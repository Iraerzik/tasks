#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <sys/types.h>

// Структура для хранения информации о файле для сортировки
typedef struct {
    char name[256];
    char permissions[11];
    struct stat file_stat;
} file_entry;

/*преобразование st_mode в строку прав доступа*/
void get_permissions(mode_t mode, char *str) {
    // Первый символ - тип файла
    if (S_ISREG(mode))      str[0] = '-';
    else if (S_ISDIR(mode)) str[0] = 'd';
    else if (S_ISLNK(mode)) str[0] = 'l';
    else if (S_ISCHR(mode)) str[0] = 'c';
    else if (S_ISBLK(mode)) str[0] = 'b';
    else if (S_ISFIFO(mode)) str[0] = 'p';
    else if (S_ISSOCK(mode)) str[0] = 's';
    else str[0] = '?';

    static const char xtbl[9] = "rwxrwxrwx";
    for (int i = 0, j = (1 << 8); i < 9; i++, j >>= 1){
        str[i + 1] = (mode & j) ? xtbl[i] : '-';
    }

    // Специальные биты
    if (mode & S_ISUID) str[3] = (mode & S_IXUSR) ? 's' : 'S';
    if (mode & S_ISGID) str[6] = (mode & S_IXGRP) ? 's' : 'S';
    if (mode & S_ISVTX) str[9] = (mode & S_IXOTH) ? 't' : 'T';

    str[10] = '\0';
}

/*преобразование времени*/
void format_time(time_t mtime, char *time_str) {
    struct tm *timeinfo = localtime(&mtime);
    strftime(time_str, 20, "%b %d %H:%M", timeinfo);
}

/*вывод для ls -l*/
void print_long_format(const char *filename, const struct stat *file_stat) {
    char permissions[11];
    char time_str[20];
    struct passwd *pw = getpwuid(file_stat->st_uid);
    struct group *gr = getgrgid(file_stat->st_gid);

    get_permissions(file_stat->st_mode, permissions);
    format_time(file_stat->st_mtime, time_str);

    printf("%s %2ld %s %s %6ld %s %s\n",
                permissions,
                file_stat->st_nlink,
                pw ? pw->pw_name : "?",
                gr ? gr->gr_name : "?",
                file_stat->st_size,
                time_str,
                filename);
}

// Функция сравнения для сортировки по правам доступа
int compare_by_permissions(const void *a, const void *b) {
    const file_entry *entry_a = (const file_entry *)a;
    const file_entry *entry_b = (const file_entry *)b;
    
    return strcmp(entry_a->permissions, entry_b->permissions);
}

// Функция для сбора файлов из директории (без вывода)
int collect_files(const char *path, file_entry **files_ptr, int *file_count_ptr) {
    DIR *dir = opendir(path);
    if (dir == NULL) {
        perror("opendir");
        return -1;
    }

    file_entry *files = NULL;
    int file_count = 0;
    int capacity = 0;
    
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        int len = strlen(entry->d_name);
        if ((len >= 4 && strcmp(entry->d_name + len - 4, ".swp") == 0) ||
            (len >= 4 && strcmp(entry->d_name + len - 4, ".swo") == 0)) {
            continue;
        }

        if (file_count >= capacity) {
            capacity = capacity == 0 ? 16 : capacity * 2;
            files = realloc(files, capacity * sizeof(file_entry));
        }
        
        strncpy(files[file_count].name, entry->d_name, sizeof(files[file_count].name) - 1);
        files[file_count].name[sizeof(files[file_count].name) - 1] = '\0';
        
        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
        
        if (lstat(full_path, &files[file_count].file_stat) == -1) {
            perror("lstat");
            continue;
        }
        
        get_permissions(files[file_count].file_stat.st_mode, files[file_count].permissions);
        file_count++;
    }

    *files_ptr = files;
    *file_count_ptr = file_count;
    closedir(dir);
    return 0;
}

// Функция для вывода собранных файлов
void print_files(file_entry *files, int file_count, int long_format, int flag_g) {
    for (int i = 0; i < file_count; i++) {
        if (long_format || flag_g) {
            print_long_format(files[i].name, &files[i].file_stat);
        } else {
            printf("%s\n", files[i].name);
        }
    }
}

void list_directory(const char *path, int recursive, int long_format, int flag_g) {
    file_entry *files = NULL;
    int file_count = 0;
    
    if (collect_files(path, &files, &file_count) == -1) {
        return;
    }

    // Выводим текущую директорию
    printf("%s:\n", path);
    print_files(files, file_count, long_format, flag_g);

    // Рекурсивный обход
    if (recursive) {
        for (int i = 0; i < file_count; i++) {
            if (S_ISDIR(files[i].file_stat.st_mode)) {
                char full_path[1024];
                snprintf(full_path, sizeof(full_path), "%s/%s", path, files[i].name);
                printf("\n");
                list_directory(full_path, recursive, long_format, flag_g);
            }
        }
    }

    free(files);
}

void parse_arguments(int argc, char *argv[], int *recursive, int *long_format, int *flag_g, char **target_dir) {
    *recursive = 0;
    *long_format = 0;
    *flag_g = 0;
    *target_dir = ".";

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-R") == 0) {
            *recursive = 1;
        } else if (strcmp(argv[i], "-l") == 0) {
            *long_format = 1;
        } else if (strcmp(argv[i], "-g") == 0) {
            *flag_g = 1;
        } else {
            *target_dir = argv[i];
        }
    }
}

int main(int argc, char *argv[]) {
    int long_format = 0;
    int flag_g = 0;
    int recursive = 0;
    char *target_dir = ".";

    parse_arguments(argc, argv, &recursive, &long_format, &flag_g, &target_dir);

    if (recursive) {
        list_directory(target_dir, recursive, long_format, flag_g);
    } else {
        file_entry *files = NULL;
        int file_count = 0;
        
        if (collect_files(target_dir, &files, &file_count) == 0) {
            if (long_format && file_count > 0) {
                qsort(files, file_count, sizeof(file_entry), compare_by_permissions);
            }
            
            print_files(files, file_count, long_format, flag_g);
            free(files);
        }
    }

    return 0;
}
