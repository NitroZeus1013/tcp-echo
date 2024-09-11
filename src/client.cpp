#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <iostream>
#include <cassert>
#include <cstring>
#include <string.h>
#include <csignal>

// consts
const size_t k_max_msg = 4096;
//
// utility functions
static void msg(const char *msg)
{
    fprintf(stderr, "%s\n", msg);
}

static void die(const char *msg)
{
    int err = errno;
    fprintf(stderr, "[%d] %s\n", err, msg);
    abort();
}

static int32_t read_full(int client_conn_fd, char *read_buff, size_t n)
{ // while we get n bytes, as sometimes read may return < n bytes even if we asked for n bytes
    while (n > 0)
    {
        ssize_t recvd = read(client_conn_fd, read_buff, n);

        /*
            returns 0 for EOF
            return > 0 for successfull read
            return -1 for err and errno is set to appropriate error
         */

        if (recvd == 0)
        {
            msg("read EOF");
            return -1;
        }

        if (recvd < 0) // if we get < 0 then it's some kind of error
        {
            msg("read err()");
            return -1;
        }
        // we should always get recvd bytes <= n
        assert((size_t)recvd <= n);
        n -= (size_t)recvd;
        read_buff += recvd;
    }
    return 0;
}
static int32_t write_all(int client_conn_fd, char *write_buff, int n)
{
    // while we get n bytes
    while (n > 0)
    {
        ssize_t recvd = write(client_conn_fd, write_buff, n);

        if (recvd <= 0) // if we get < 0 then it's some kind of error and errno is set
        {
            msg("read err()");
            return -1;
        }
        // we should always get recvd bytes <= n
        assert((size_t)recvd <= n);
        n -= (size_t)recvd;
    }

    return 0;
}

int32_t query(int client_conn_fd, char *data)
{
    uint32_t len = (uint32_t)strlen(data); // get length of data being sent
    if (len > k_max_msg)                   // if data is more than max size defined by our protocol then return - we don't have any chunking here
    {
        msg("tooo long ");
        return -1;
    }
    char write_buff[4 + k_max_msg + 1]; // write buffer of size 4-data length, k_max_msg - actual data, 1 - \0

    memcpy(write_buff, &len, 4); // copy N bytes from src(len) to destination
    memcpy(&write_buff[4], data, len);
    int32_t err = write_all(client_conn_fd, write_buff, 4 + len);

    if (err)
    {
        msg("write err");
        return -1;
    }

    // read response
    char read_buff[4 + k_max_msg + 1];
    errno = 0;
    err = read_full(client_conn_fd, read_buff, 4);
    // read can trigger EOF and set the global errno to 0
    if (err < 0)
    {
        if (errno == 0)
        {
            msg("EOF");
        }
        else
        {
            msg(strcat("read() error", std::strerror(errno)));
        }
        return err;
    }
    memcpy(&len, read_buff, 4);
    if (len > k_max_msg)
    {
        msg("too long to read");
        return -1;
    }

    err = read_full(client_conn_fd, &read_buff[4], len);

    if (err < 0)
    {
        msg("read error");
        return -1;
    }
    read_buff[4 + len] = '\0';
    std::cout << "Server => " << &read_buff[4] << std::endl;

    return 0;
}

int client_conn_fd;

void handle_singal(int sig)
{
    std::cout << "\n Closing connection \n";
    close(client_conn_fd);
    exit(0);
}

int main()
{
    signal(SIGINT, handle_singal);
    client_conn_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (client_conn_fd < 0)
    {
        die("socket()");
    }

    struct sockaddr_in client_addr = {};
    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = ntohl(INADDR_LOOPBACK);
    client_addr.sin_port = ntohs(1234);

    int rv = connect(client_conn_fd, (sockaddr *)&client_addr, sizeof(client_addr));

    if (rv)
    {
        die("connect()");
    }

    // char msg[] = "hello from client";
    // write(client_conn_fd, &msg, sizeof(msg));

    // char read_buffer[64] = {0};

    // ssize_t n = read(client_conn_fd, &read_buffer, sizeof(read_buffer));

    // if (n < 0)
    // {
    //     die("read()");
    // }

    // std::cout << "From server => " << read_buffer << std::endl;
    char input[4096];
    while (true)
    {
        std::cin.getline(input, 50); // getline bocks the further code until user enters input and inserts a new line using enter key
        int32_t err = query(client_conn_fd, input);
        if (err)
        {
            goto L_DONE;
        }
    }
L_DONE:
    close(client_conn_fd);
    return 0;
}