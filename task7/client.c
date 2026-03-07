#include "chat.h"

int main(int argc, char const *argv[]) {
    int port, n, sfd;
    struct hostent *phe;
    struct sockaddr_in sin;
    fd_set readfds;
    char buffer[BUFFER_SIZE];
    
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <hostname> <port>\n", argv[0]);
        exit(1);
    }
    
    // получим адрес компьютера
    if (!(phe = gethostbyname(argv[1]))) {
        fprintf(stderr, "Bad host name: %s\n", argv[1]);
        exit(1);
    }
    
    // прочитаем номер порта
    if (sscanf(argv[2], "%d%n", &port, &n) != 1
           || argv[2][n] || port <= 0 || port > 65535) {
        fprintf(stderr, "Bad port number: %s\n", argv[2]);
        exit(1);
    }
    
    // создаём сокет
    if ((sfd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }

    // устанавливаем адрес подключения
    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);
    memcpy(&sin.sin_addr, phe->h_addr_list[0], sizeof(sin.sin_addr));
    
    // подсоединяемся к серверу
    if (connect(sfd, (struct sockaddr*) &sin, sizeof(sin)) < 0) {
        perror("connect");
        exit(1);
    }
    
    printf("Connected to chat server. Enter your nickname when prompted.\n");
    printf("Type \\help for available commands.\n");
	while(1) {
        FD_ZERO(&readfds);
        FD_SET(0, &readfds);  // stdin
        FD_SET(sfd, &readfds); // socket
        
        if (select(sfd + 1, &readfds, NULL, NULL, NULL) < 0) {
            perror("select");
            break;
        }

        // данные от сервера
        if (FD_ISSET(sfd, &readfds)) {
            int n = read(sfd, buffer, sizeof(buffer) - 1);
            if (n <= 0) {
                printf("Server disconnected\n");
                break;
            }
            buffer[n] = '\0';
            printf("%s", buffer);
            fflush(stdout);
        }

        // данные пользователя
        if (FD_ISSET(0, &readfds)) {
            if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
                write(sfd, buffer, strlen(buffer));
            }
        }
    }
    
    close(sfd);
    return 0;
}
