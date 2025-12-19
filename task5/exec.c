#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include "tree.h"
#include "exec.h"  

#define MAX_BG_PROCESSES 100

static BackgroundProcess bg_processes[MAX_BG_PROCESSES];
static int bg_count = 0;


static int execute_command(tree node);/* Исполнение простой команды */
static int execute_redirect(tree node);/* Исполнение перенаправления */
static int execute_pipeline(tree node);/* Исполнение конвейера */
static int execute_sequence(tree node);/* Исполнение последовательности */
static int execute_logical(tree node);/* Исполнение логических операторов */
static int execute_background(tree node);/* Исполнение фоновой задачи */
static int execute_subshell(tree node);/* Исполнение подоболочки */

/* ============= обработка сигналов ============= */

static void sigchld_handler(int sig) {
    /* Вызываем cleanup_zombies при завершении дочернего процесса */
    cleanup_zombies();
}

void setup_signal_handlers(void) {
    struct sigaction sa_int, sa_chld;
    
    /* Обработчик SIGINT - игнорируем в родительском процессе */
    sa_int.sa_handler = SIG_IGN;
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags = SA_RESTART;
    sigaction(SIGINT, &sa_int, NULL);
    
    /* Обработчик SIGCHLD */
    sa_chld.sa_handler = sigchld_handler;
    sigemptyset(&sa_chld.sa_mask);
    sa_chld.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    sigaction(SIGCHLD, &sa_chld, NULL);
}

void reset_signal_handlers(void) {
    struct sigaction sa_default;
    
    /* Восстанавливаем обработчики по умолчанию */
    sa_default.sa_handler = SIG_DFL;
    sigemptyset(&sa_default.sa_mask);
    sa_default.sa_flags = 0;
    
    sigaction(SIGINT, &sa_default, NULL);
    sigaction(SIGCHLD, &sa_default, NULL);
}

/* ============= управление фоновыми проц  ============= */

void cleanup_zombies(void) {
    int status;
    pid_t pid;
    
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        for (int i = 0; i < bg_count; i++) {
            if (bg_processes[i].pid == pid) {
                if (WIFEXITED(status)) {
                    printf("[%d] Завершен %s с кодом %d\n", 
                           pid, bg_processes[i].command, WEXITSTATUS(status));
                } else if (WIFSIGNALED(status)) {
                    printf("[%d] Завершен %s сигналом %d\n", 
                           pid, bg_processes[i].command, WTERMSIG(status));
                }
                
                free(bg_processes[i].command);
                for (int j = i; j < bg_count - 1; j++) {
                    bg_processes[j] = bg_processes[j + 1];
                }
                bg_count--;
                break;
            }
        }
    }
}

void add_background_process(pid_t pid, const char *cmd) {
    if (bg_count < MAX_BG_PROCESSES) {
        bg_processes[bg_count].pid = pid;
        bg_processes[bg_count].command = strdup(cmd);
        printf("[%d] %s\n", pid, cmd);
        bg_count++;
    }
}

void wait_for_background(void) {
    for (int i = 0; i < bg_count; i++) {
        waitpid(bg_processes[i].pid, NULL, 0);
    }
    bg_count = 0;
}

/* ============= исполнение  ============= */

/* Исполнение простой команды */
static int execute_command(tree node) {
    if (!node || node->type != NODE_COMMAND) return -1;
    
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return -1;
    }
    
    if (pid == 0) {
        reset_signal_handlers();
        execvp(node->args[0], node->args);
        fprintf(stderr, "shell: команда не найдена: %s\n", node->args[0]);
        exit(127);
    }
    /* В родительском процессе */
    int status;
    waitpid(pid, &status, 0);
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    
    return -1;
}

/* Исполнение перенаправления */
static int execute_redirect(tree node) {
    if (!node || node->type != NODE_REDIRECT) return -1;
    
    int flags = 0;
    int fd_target = 0;
    
    if (strcmp(node->command, ">") == 0) {
        flags = O_WRONLY | O_CREAT | O_TRUNC;
        fd_target = STDOUT_FILENO;
    } else if (strcmp(node->command, ">>") == 0) {
        flags = O_WRONLY | O_CREAT | O_APPEND;
        fd_target = STDOUT_FILENO;
    } else if (strcmp(node->command, "<") == 0) {
        flags = O_RDONLY;
        fd_target = STDIN_FILENO;
    }
    
    int fd = open(node->args[0], flags, 0644);
    if (fd < 0) {
        perror(node->args[0]);
        return -1;
    }
 
    dup2(fd, fd_target);
    close(fd);
    return 0;
}

/* Исполнение конвейера */
static int execute_pipeline(tree node) {
    if (!node || node->type != NODE_PIPE) return -1;
    
    int fd[2];
    if (pipe(fd) < 0) {
        perror("pipe");
        return -1;
    }
    
    pid_t pid1 = fork();
    if (pid1 < 0) {
        perror("fork");
        close(fd[0]);
        close(fd[1]);
        return -1;
    }
    
    if (pid1 == 0) {
        /* Первый процесс в конвейере */
        reset_signal_handlers();
        close(fd[0]);
        dup2(fd[1], STDOUT_FILENO);
        close(fd[1]);
        
        /* Исполняем левую часть */
        exit(execute_tree(node->left));
    }
    
    pid_t pid2 = fork();
    if (pid2 < 0) {
        perror("fork");
        close(fd[0]);
        close(fd[1]);
        kill(pid1, SIGTERM);
        waitpid(pid1, NULL, 0);
        return -1;
    }
    
    if (pid2 == 0) {
        /* Второй процесс в конвейере */
        reset_signal_handlers();
        close(fd[1]);
        dup2(fd[0], STDIN_FILENO);
        close(fd[0]);
        
        /* Исполняем правую часть */
        exit(execute_tree(node->right));
    }
    
    /* Родительский процесс */
    close(fd[0]);
    close(fd[1]);
    
    int status1, status2;
    waitpid(pid1, &status1, 0);
    waitpid(pid2, &status2, 0);
    
    if (WIFEXITED(status2)) {
        return WEXITSTATUS(status2);
    }
    
    return -1;
}

/* Исполнение последовательности */
static int execute_sequence(tree node) {
    if (!node || node->type != NODE_SEQ) return -1;
    int result = 0;
    if (node->left) {
        result = execute_tree(node->left);
    }
    if (node->right) {
        result = execute_tree(node->right);
    }
    
    return result;
}

/* Исполнение логических операторов */
static int execute_logical(tree node) {
    if (!node || (node->type != NODE_AND && node->type != NODE_OR)) return -1;
    
    int left_result = execute_tree(node->left);
    
    if (node->type == NODE_AND) {
        /* &&: исполняем правую часть только если левая успешна */
        if (left_result == 0 && node->right) {
            return execute_tree(node->right);
        }
        return left_result;
    } else {
        /* ||: исполняем правую часть только если левая неуспешна */
        if (left_result != 0 && node->right) {
            return execute_tree(node->right);
        }
        return left_result;
    }
}

/* Исполнение фоновой задачи */
static int execute_background(tree node) {
    if (!node || node->type != NODE_BG) return -1;
    
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return -1;
    }
    
    if (pid == 0) {
        /* Дочерний процесс для фонового выполнения */
        reset_signal_handlers();
        setpgid(0, 0);  /* Новая группа процессов */
        
        if (node->left) {
            int result = execute_tree(node->left);
            exit(result);
        }
        exit(0);
    }
    
    /* Родительский процесс */
    add_background_process(pid, node->left ? node->left->command : "background");
    
    return 0;
}

/* Исполнение подоболочки */
static int execute_subshell(tree node) {
    if (!node || node->type != NODE_SUBSHELL) return -1;
    
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return -1;
    }
    
    if (pid == 0) {
        reset_signal_handlers();
        
        if (node->left) {
            int result = execute_tree(node->left);
            exit(result);
        }
        exit(0);
    }
    
    int status;
    waitpid(pid, &status, 0);
    
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    
    return -1;
}

/* Основная функция исполнения дерева */
int execute_tree(tree root) {
    if (!root) return 0;
    
    switch (root->type) {
        case NODE_COMMAND:
            return execute_command(root);
        case NODE_PIPE:
            return execute_pipeline(root);
        case NODE_REDIRECT:
            return execute_redirect(root);
        case NODE_BG:
            return execute_background(root);
        case NODE_SEQ:
            return execute_sequence(root);
        case NODE_AND:
        case NODE_OR:
            return execute_logical(root);
        case NODE_SUBSHELL:
            return execute_subshell(root);
        default:
            fprintf(stderr, "Неизвестный тип узла: %d\n", root->type);
            return -1;
    }
}
