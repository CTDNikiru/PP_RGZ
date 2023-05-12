#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <err.h>
#include <string.h>
#include <pthread.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <chrono>
#include "cube.cpp"

using namespace std;

// Размер символьного буфера для ответа клиенту
const unsigned int BUF_LENGTH = 1000;
// Порядковый номер запроса клиента
int request_number = 1;

struct request_handler_params
{
    // Дескриптор сокета клиента
    int socket_desk;
    // Порядковый номер запроса клиента
    int request_number;
};

// Функция-обработчик запроса
void *request_handler(void *arg)
{
    request_handler_params *params = (request_handler_params *)arg;

    // Формируем строку
    char response_buf[BUF_LENGTH];
    snprintf(
        response_buf,
        BUF_LENGTH,
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html; charset=UTF-8\r\n\r\n"
        "Request number %d has been processed. ",
        params->request_number);
    request_number++;
    // Отправляем строку пользователю о начале вычисления
    write(params->socket_desk, response_buf, strlen(response_buf) * sizeof(char));

    // Начинаем вычисления
    cout << "start counting pairs in thread " << pthread_self() << endl;

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    
    start();
    
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    string answ = "Execution time = " + to_string(std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count()) + " milliseconds\n";

    // Отображаем результат
    write(params->socket_desk, answ.c_str(), strlen(answ.c_str()) * sizeof(char));

    // Закрываем соединение
    shutdown(params->socket_desk, SHUT_WR);

    // close(params->socket_desk);
    delete params;
    return NULL;
}

int main()
{
    int res;
    int one = 1, client_fd;
    struct sockaddr_in svr_addr, cli_addr;
    socklen_t sin_len = sizeof(cli_addr);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        err(1, "can't open socket");
    }

    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int));

    int port = 8080;
    svr_addr.sin_family = AF_INET;
    svr_addr.sin_addr.s_addr = INADDR_ANY;
    svr_addr.sin_port = htons(port);

    if (bind(sock, (struct sockaddr *)&svr_addr, sizeof(svr_addr)) == -1)
    {
        close(sock);
        err(1, "Can't bind");
    }

    listen(sock, 5);

    // Проинициализируем переменную для хранения атрибутов потока
    pthread_attr_t thread_attr;
    res = pthread_attr_init(&thread_attr);
    if (res != 0)
    {
        cout << "Cannot create thread attribute: " << strerror(res) << endl;
        exit(-1);
    }

    size_t stack_size = 2 * 1024 * 1024;

    res = pthread_attr_setstacksize(&thread_attr, stack_size);
    if (res != 0)
    {
        cout << "Setting stack size attribute failed: " << strerror(res)
             << endl;
        exit(-1);
    }

    while (1)
    {
        client_fd = accept(sock, (struct sockaddr *)&cli_addr, &sin_len);
        if (client_fd == -1)
        {
            perror("Can't accept");
            continue;
        }

        cout << "main thread with id:  " << pthread_self() << " accepted signal" << endl;

        request_handler_params *params = new request_handler_params;
        params->request_number = request_number;
        params->socket_desk = client_fd;

        pthread_t thread;
        res = pthread_create(&thread, &thread_attr, request_handler, (void *)params);
        if (res != 0)
        {
            cout << "Cannot create a thread: " << strerror(res) << endl;
            close(client_fd);
            exit(-1);
        }
    }
}
