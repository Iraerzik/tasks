#include "chat.h"

Client clients[MAX_CLIENTS];
struct pollfd fds[MAX_CLIENTS + 1]; // +1 для слушающего
int nfds = 1; // кол-во отслеживаемых дескр

void cleanup(int sig){
    (void)sig;
    printf("\nServer shutting down...\n");
    fflush(stdout);  // сбрасываем буферы

    // закрываем все соединения
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].active) {
            close(clients[i].fd);
        }
    }

    exit(0);
}

// инициализация клиентов
void init_clients() {
    for(int i = 0; i < MAX_CLIENTS; i++) {
        clients[i].fd = -1;
        clients[i].active = 0;
        clients[i].contact_count = 0;
        memset(clients[i].name, 0, NAME_SIZE);
        memset(clients[i].private_contacts, 0, sizeof(clients[i].private_contacts));
        fds[i + 1].fd = -1;
        fds[i + 1].events = POLLIN;
    }
}

// отправка сообщения всем клиентам
void send_msg(char *message, int sender_fd) {
    for(int i = 0; i < MAX_CLIENTS; i++) {
        if(clients[i].active && clients[i].fd != sender_fd) {
            write(clients[i].fd, message, strlen(message));
        }
    }
}

// поиск клиента по fd
int find_client_by_fd(int fd) {
    for(int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].active && clients[i].fd == fd) {
            return i;
        }
    }
    return -1;
}

// поиск свободного места для нового клиента
int find_free_slot() {
    for(int i = 0; i < MAX_CLIENTS; i++) {
        if(!clients[i].active) {
            return i;
        }
    }
    return -1;
}

// поиск клиента по имени
int find_client_by_name(char *name) {
    for(int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].active && strcmp(clients[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

// удаление клиента
void remove_client(int index) {
    if (index >= 0 && index < MAX_CLIENTS && clients[index].active) {
        printf("Client %s disconnected\n", clients[index].name);
        close(clients[index].fd);
        clients[index].active = 0;
        clients[index].fd = -1;
        clients[index].contact_count = 0;
        memset(clients[index].name, 0, NAME_SIZE);
        memset(clients[index].private_contacts, 0, sizeof(clients[index].private_contacts));
        
        // Очищаем pollfd
        for (int i = 1; i < nfds; i++) {
            if (fds[i].fd == clients[index].fd) {
                fds[i].fd = -1;
                fds[i].events = 0;
                fds[i].revents = 0;
                break;
            }
        }
    }
}

// добавить контакт в историю приватных сообщений
void add_private_contact(int client_idx, char *contact_name) {
    if (client_idx < 0 || client_idx >= MAX_CLIENTS) return;

    // Проверяем, есть ли уже такой контакт
    for (int i = 0; i < clients[client_idx].contact_count; i++) {
        if (strcmp(clients[client_idx].private_contacts[i], contact_name) == 0) {
            return; // уже есть
        }
    }

    // Добавляем новый контакт
    if (clients[client_idx].contact_count < MAX_CLIENTS) {
        strcpy(clients[client_idx].private_contacts[clients[client_idx].contact_count], contact_name);
        clients[client_idx].contact_count++;
    }
}

// отправка приватного сообщения
void send_private_message(int sender_idx, char *recipient_name, char *msg_text) {
    if (sender_idx < 0) return;

    int recipient_idx = find_client_by_name(recipient_name);

    if (recipient_idx < 0) {
        char error_msg[BUFFER_SIZE];
        sprintf(error_msg, "Error: User '%s' not found\n", recipient_name);
        write(clients[sender_idx].fd, error_msg, strlen(error_msg));
        return;
    }

    if (recipient_idx == sender_idx) {
        char error_msg[BUFFER_SIZE];
        sprintf(error_msg, "Error: Cannot send private message to yourself\n");
        write(clients[sender_idx].fd, error_msg, strlen(error_msg));
        return;
    }

    // Добавляем в историю отправителя
    add_private_contact(sender_idx, recipient_name);
    // Добавляем в историю получателя (для симметрии)
    add_private_contact(recipient_idx, clients[sender_idx].name);

    // Отправляем приватное сообщение получателю
    char private_msg[BUFFER_SIZE];
    sprintf(private_msg, "[Private from %s]: %s\n",
            clients[sender_idx].name, msg_text);
    write(clients[recipient_idx].fd, private_msg, strlen(private_msg));

    // Подтверждение отправителю
    char confirm_msg[BUFFER_SIZE];
    sprintf(confirm_msg, "[Private to %s]: %s\n", recipient_name, msg_text);
    write(clients[sender_idx].fd, confirm_msg, strlen(confirm_msg));
}

// показать список приватных контактов
void show_private_contacts(int client_idx) {
    if (client_idx < 0) return;

    char response[BUFFER_SIZE];

    if (clients[client_idx].contact_count == 0) {
        sprintf(response, "You haven't sent any private messages yet.\n");
        write(clients[client_idx].fd, response, strlen(response));
        return;
    }

    sprintf(response, "Your private message contacts (%d):\n",
            clients[client_idx].contact_count);
    write(clients[client_idx].fd, response, strlen(response));

    for (int i = 0; i < clients[client_idx].contact_count; i++) {
        sprintf(response, "  - %s\n", clients[client_idx].private_contacts[i]);
        write(clients[client_idx].fd, response, strlen(response));
    }
}

// показать список доступных команд
void show_help(int client_fd) {
    char *help_text =
        "\n=== Available commands ===\n"
        "  \\users                    - show online users\n"
        "  \\private <name> <msg>     - send private message to user\n"
        "  \\privates                 - show your private message contacts\n"
        "  \\quit <message>           - exit with farewell message\n"
        "  \\help                      - show this help\n"
        "===========================\n"
        "Just type anything to send a public message\n\n";

    write(client_fd, help_text, strlen(help_text));
}

// обработка команд клиента
void handle_command(int client_idx, char *buf) {
    if (client_idx < 0) return;
    
    char answer[BUFFER_SIZE];
    // Удаляем символы новой строки
    buf[strcspn(buf, "\n")] = 0;
    buf[strcspn(buf, "\r")] = 0;
    
    if (strlen(buf) == 0) return;

    // Проверяем, является ли это командой (начинается с \)
    if (buf[0] == '\\') {
        // Команда \help
        if (strcmp(buf, "\\help") == 0) {
            show_help(clients[client_idx].fd);
        }
        // Команда \users
        else if (strcmp(buf, "\\users") == 0) {
            int online_count = 0;
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (clients[i].active) online_count++;
            }
            
            sprintf(answer, "Online users (%d):\n", online_count);
            write(clients[client_idx].fd, answer, strlen(answer));

            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (clients[i].active) {
                    sprintf(answer, "  - %s\n", clients[i].name);
                    write(clients[client_idx].fd, answer, strlen(answer));
                }
            }
        }   
		else if (strcmp(buf, "\\privates") == 0) {
            show_private_contacts(client_idx);
        }
        // Команда \private
        else if (strncmp(buf, "\\private", 8) == 0) {
            // Проверяем, что после команды есть пробел и аргументы
            if (strlen(buf) < 9 || buf[8] != ' ') {
                char *usage = "Usage: \\private <nickname> <message>\n";
                write(clients[client_idx].fd, usage, strlen(usage));
                return;
            }
            
            // Пропускаем "\\private "
            char *p = buf + 9; // +9 чтобы пропустить "\\private "
            while (*p == ' ') p++; // пропускаем дополнительные пробелы
            
            // Копируем никнейм
            char recipient[NAME_SIZE] = {0};
            int i = 0;
            while (*p && *p != ' ' && i < NAME_SIZE - 1) {
                recipient[i++] = *p++;
            }
            recipient[i] = '\0';
            
            if (strlen(recipient) == 0) {
                char *usage = "Usage: \\private <nickname> <message>\n";
                write(clients[client_idx].fd, usage, strlen(usage));
                return;
            }
            
            // Пропускаем пробелы перед сообщением
            while (*p == ' ') p++;
            
            if (strlen(p) == 0) {
                char *empty_msg = "Error: Empty message\n";
                write(clients[client_idx].fd, empty_msg, strlen(empty_msg));
                return;
            }
            
            send_private_message(client_idx, recipient, p);
        }
        // Команда \quit
        else if (strncmp(buf, "\\quit", 5) == 0) {
            char *msg = buf + 5;
            while (*msg == ' ') msg++;
            
            // Сообщаем всем об уходе
            char quit_msg[BUFFER_SIZE];
            sprintf(quit_msg, "--- %s покинул чат: %s\n", 
                    clients[client_idx].name, 
                    (strlen(msg) > 0) ? msg : "без причины");
            send_msg(quit_msg, clients[client_idx].fd);
            
            // Удаляем клиента
            remove_client(client_idx);
        }
        else {
            // Неизвестная команда
            sprintf(answer, "Unknown command: %s. Type \\help for available commands.\n", buf);
            write(clients[client_idx].fd, answer, strlen(answer));
        }
    }
    else if (strlen(buf) > 0) {
        // Обычное сообщение
        char message[BUFFER_SIZE];
        sprintf(message, "%s: %s\n", clients[client_idx].name, buf);
        send_msg(message, -1); // -1 означает всем (включая автора)
    }
}

// обработка нового подключения
void handle_new_connection(int lfd) {
    struct sockaddr_in a_addr;
    socklen_t a_len = sizeof(a_addr);

    int a_fd = accept(lfd, (struct sockaddr*) &a_addr, &a_len);
    if (a_fd < 0) {
        perror("accept");
        return;
    }

    int slot = find_free_slot();
    if (slot < 0) {
        char *msg = "Server full, try again later.\n";
        write(a_fd, msg, strlen(msg));
        close(a_fd);
        return;
    }
    
    // Запрашиваем имя
    char *name_prompt = "Enter your nickname: ";
    write(a_fd, name_prompt, strlen(name_prompt));

    char name[NAME_SIZE];
    int n = read(a_fd, name, sizeof(name) - 1);
    if (n <= 0) {
        close(a_fd);
        return;
    }
    name[n] = '\0';
    name[strcspn(name, "\n")] = 0;
    name[strcspn(name, "\r")] = 0;

    // Проверяем пустое имя
    if (strlen(name) == 0) {
        char *msg = "Nickname cannot be empty.\n";
        write(a_fd, msg, strlen(msg));
        close(a_fd);
        return;
    }

    // Проверяем уникальность имени
    for(int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].active && strcmp(clients[i].name, name) == 0) {
            char *msg = "Nickname already exists.\n";
            write(a_fd, msg, strlen(msg));
            close(a_fd);
            return;
        }
    }
    
    // Добавляем нового клиента
    clients[slot].fd = a_fd;
    strcpy(clients[slot].name, name);
    clients[slot].addr = a_addr;
    clients[slot].active = 1;
    clients[slot].contact_count = 0;
    memset(clients[slot].private_contacts, 0, sizeof(clients[slot].private_contacts));

    fds[nfds].fd = a_fd;
    fds[nfds].events = POLLIN;
    fds[nfds].revents = 0;
    nfds++;

    printf("New client connected: %s from %s:%d (slot %d, nfds=%d)\n",
           name, inet_ntoa(a_addr.sin_addr), ntohs(a_addr.sin_port), slot, nfds);

    // Сообщаем всем о новом участнике
    char welcome[BUFFER_SIZE];
    sprintf(welcome, "--- %s вошел в комнату\n", name);
    send_msg(welcome, a_fd);

    // Отправляем приветствие
    char greeting[BUFFER_SIZE];
    sprintf(greeting, "Welcome to the chat, %s! Type \\help for available commands.\n", name);
    write(a_fd, greeting, strlen(greeting));
}

int main(int argc, char const *argv[]) {
    int lfd, n, port, sopt;
    struct sockaddr_in baddr;
    
    signal(SIGINT, cleanup);
    signal(SIGTERM, cleanup);
    
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(1);
    }
    
    // читаем номер порта
    if (sscanf(argv[1], "%d%n", &port, &n) != 1
            || argv[1][n] || port <= 0 || port > 65535) {
        fprintf(stderr, "Bad port number: %s\n", argv[1]);
        exit(1);
    }
    
    // создаём сокет
    if ((lfd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }
    
    // задаём режим сокета
    sopt = 1;
    if (setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &sopt, sizeof(sopt)) < 0) {
        perror("setsockopt");
        exit(1);
    }
    
    // задаём адрес сокета
    baddr.sin_family = AF_INET;
    baddr.sin_port = htons(port);
    baddr.sin_addr.s_addr = INADDR_ANY;

    // привязываем сокет
    if (bind(lfd, (struct sockaddr *) &baddr, sizeof(baddr)) < 0) {
        perror("bind");
        exit(1);
    }
    
    // включаем режим прослушивания
    if (listen(lfd, 5) < 0) {
        perror("listen");
        exit(1);
    }

    init_clients();
    fds[0].fd = lfd;
    fds[0].events = POLLIN;
    fds[0].revents = 0;
    
    printf("Chat server started on port %d\n", port);
    printf("Commands: \\help, \\users, \\private, \\privates, \\quit\n");
    fflush(stdout);
    
    while (1) {
        int poll_res = poll(fds, nfds, POLL_TIMEOUT);

        if (poll_res < 0) {
            perror("poll");
            continue;
        }
        if (poll_res == 0) { 
            continue;
        }
        
        // проверка новых подключений
        if (fds[0].revents & POLLIN) {
            handle_new_connection(lfd);
        }

        for (int i = 1; i < nfds; i++) {
            if (fds[i].fd < 0) continue;

            if (fds[i].revents & POLLIN) {
                char buffer[BUFFER_SIZE];
                int n = read(fds[i].fd, buffer, sizeof(buffer) - 1);

                if (n <= 0) {
                    // клиент отключился
                    int client_idx = find_client_by_fd(fds[i].fd);
                    if (client_idx >= 0) {
                        char quit[BUFFER_SIZE];
                        sprintf(quit, "--- %s покинул чат (отключился)\n", 
                                clients[client_idx].name);
                        send_msg(quit, fds[i].fd);
                        remove_client(client_idx);
                    }

                    // закрываем сокет
                    close(fds[i].fd);
                    
                    // удаляем из pollfd
                    for(int j = i; j < nfds - 1; j++) {
                        fds[j] = fds[j+1];
                    }
                    nfds--;
                    i--;
                } else {
                    buffer[n] = '\0';
                    int client_idx = find_client_by_fd(fds[i].fd);
                    if (client_idx >= 0) {
                        handle_command(client_idx, buffer);

                        // если клиент удалил себя сам (через \quit)
                        if (!clients[client_idx].active) {
                            close(fds[i].fd);
                            for (int j = i; j < nfds - 1; j++) {
                                fds[j] = fds[j+1];
                            }
                            nfds--;
                            i--;
                        }
                    }
                }
            }
            else if (fds[i].revents & (POLLHUP | POLLERR | POLLNVAL)) {
                int client_idx = find_client_by_fd(fds[i].fd);
                if (client_idx >= 0) {
                    char quit[BUFFER_SIZE];
                    sprintf(quit, "--- %s покинул чат (проблема с соединением)\n", 
                            clients[client_idx].name);
                    send_msg(quit, fds[i].fd);
                    remove_client(client_idx);
                }
                
                close(fds[i].fd);
                
                // удаляем из pollfd
                for (int j = i; j < nfds - 1; j++) {
                    fds[j] = fds[j + 1];
                }
                nfds--;
                i--;
            }
        }
    }
    
    close(lfd);
    return 0;
}
