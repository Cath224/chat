#include <stdio.h>
#include <winsock.h>
#include <pthread.h>

#pragma comment(lib, "ws2_32.lib")


typedef struct CONN {
    SOCKET socket;
    struct sockaddr_in address;
    pthread_mutex_t *mutex;
} CONN;

void *handle_connection(void *args);

void *handle_response(void *args);

int starts_with(const char *start, const char *str) {
    size_t start_length = strlen(start),
            str_length = strlen(str);
    return str_length < start_length ? 0 : memcmp(start, str, start_length) == 0 ? 1 : 0;
}

int main() {
    WSADATA wsa;
    SOCKET s;
    struct sockaddr_in server;
    struct in_addr **addr_list;
    char *message, server_reply[2000];
    int recv_size;
    char ip[100];
    struct hostent *he;
    pthread_t thread1, thread2;

    printf("\nInitialising Winsock...");
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Failed. Error Code : %d", WSAGetLastError());
        return 1;
    }

    printf("Initialised.\n");


    if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET) {
        printf("Could not create socket : %d", WSAGetLastError());
    }

    printf("Socket created.\n");

    if ((he = gethostbyname("localhost")) == NULL) {

        printf("gethostbyname failed : %d", WSAGetLastError());
        return 1;
    }

    addr_list = (struct in_addr **) he->h_addr_list;

    for (int i = 0; addr_list[i] != NULL; i++) {
        strcpy(ip, inet_ntoa(*addr_list[i]));
    }


    server.sin_addr.s_addr = inet_addr(ip);
    server.sin_family = AF_INET;
    server.sin_port = htons(8082);

    puts("Connected");

    CONN *conn = (CONN *) malloc(sizeof(CONN));

    conn->socket = s;
    conn->address = server;
    conn->mutex = (pthread_mutex_t*) malloc(sizeof (pthread_mutex_t));
    pthread_mutex_init(conn->mutex, NULL);
    int slen = sizeof(conn->address);

    char name[256];

    printf("Please, enter your name: \n");
    gets(name);

    char* start_conn_with_name = (char*) malloc(sizeof (char) * 264);

    strcpy(start_conn_with_name, "CLIENT:");
    strcat(start_conn_with_name, name);


    if (sendto(conn->socket, start_conn_with_name, strlen(start_conn_with_name), 0, (struct sockaddr *) &conn->address, slen) ==
        SOCKET_ERROR) {
        printf("sendto() failed with error code : %d", WSAGetLastError());
        return 1;
    }

    pthread_create(&thread1, NULL, handle_connection, (void *) conn);
    pthread_create(&thread2, NULL, handle_response, (void *) conn);
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    closesocket(conn->socket);
    free(conn);
    free(start_conn_with_name);
    WSACleanup();
    return 0;
}

void *handle_connection(void *args) {
    CONN *conn = (CONN *) args;
    char *message = (char *) malloc(sizeof(char *) * 1000);
    char *buffer = (char *) malloc(sizeof(char *) * 1024);
    int slen = sizeof(conn->address);
    puts("Start chatting, enter message (max 1000 symbols): ");
    while (1) {
        gets(message);
        if (sendto(conn->socket, message, strlen(message), 0, (struct sockaddr *) &conn->address, slen) ==
            SOCKET_ERROR) {
            pthread_mutex_lock(conn->mutex);
            printf("sendto() failed with error code : %d", WSAGetLastError());
            pthread_mutex_unlock(conn->mutex);
            break;
        }

        memset(message, '\0', 1024);
    }
    free(message);
    free(buffer);

}

void *handle_response(void *args) {
    CONN *conn = (CONN *) args;
    char *buffer = (char *) malloc(sizeof(char *) * 1286);
    int slen = sizeof(conn->address);

    while (1) {

        memset(buffer, '\0', 1286);
        if (recvfrom(conn->socket, buffer, 1286, 0, (struct sockaddr *) &conn->address, &slen) == SOCKET_ERROR) {
            pthread_mutex_lock(conn->mutex);
            printf("recvfrom() failed with error code : %d", WSAGetLastError());
            pthread_mutex_unlock(conn->mutex);
            break;
        }

        if (starts_with("NAME:", buffer) == 0) {
            continue;
        }




        int i = 0;
        char* name_pointer = buffer + 5;
        while(*name_pointer != '\n') {
            name_pointer++;
            i++;
        }

        char *name = (char*) malloc(sizeof (char) * i);

        memcpy(name, buffer + 5, sizeof (char) * i);

        name_pointer++;
        pthread_mutex_lock(conn->mutex);
        printf("%s: %s\n", name, name_pointer);
        pthread_mutex_unlock(conn->mutex);
    }
    free(buffer);
}
