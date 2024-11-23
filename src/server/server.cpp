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
#include "utils/read.hpp"
#include "utils/write.hpp"
#include "log.hpp"

// consts
const size_t k_max_msg = 4096;
//
// utility functions
static void die(const char *msg)
{
    int err = errno; // errno is global value which set by any system call when it fails.
    fprintf(stderr, "[%d] %s\n", err, msg);
    abort();
}

static void send_server_response(int client_conn_fd)
{
    char read_buff[64] = {0};

    ssize_t n = read(client_conn_fd, &read_buff, sizeof(read_buff) - 1); // replace with recv, returns -1 for error

    if (n < 0)
    {
        msg("read() error");
        return;
    }

    std::cout << "From client => " << read_buff << std::endl;

    char write_buff[] = "Hello from server";
    write(client_conn_fd, &write_buff, sizeof(write_buff)); // replace with send
}

// memcpy can also throw errors but we don't handle them

static int32_t handle_one_request(int client_conn_fd)
{
    char read_buff[4 + k_max_msg + 1]; // one for /0 null character
    errno = 0;
    int32_t err = read_full(client_conn_fd, read_buff, 4); // read first 4 bytes to examine the incoming msg length
    if (err)
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

    // try to check what is the len of incoming data that we read
    uint32_t len = 0;
    memcpy(&len, read_buff, 4);
    if (len > k_max_msg)
    {
        msg("too long");
        return -1;
    }
    // send &read_buff[4] to write from 4th byte and not 1st byte.
    err = read_full(client_conn_fd, &read_buff[4], len);

    if (err)
    {
        msg("error reading data from request");
        return err;
    }

    read_buff[4 + len] = '\0'; // add null terminated char to last index.
    std::cout << "client request => " << &read_buff[4] << std::endl;
    // do something with request
    // write response back to client
    char reply[] = "Hello from server";
    char write_buff[4 + k_max_msg + 1];
    len = (uint32_t)(strlen(reply));
    memcpy(write_buff, &len, 4);
    memcpy(&write_buff[4], reply, len);
    return write_all(client_conn_fd, write_buff, 4 + len);
}

int main()
{

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    int val = 1;

    // setting for socket only(SOL_SOCKET) set val(1) for option SO_REUSEADDR so that when this address is in TIME_WAIT state, for some reason still ask kernel to assign this address if in any other state then throw address_in_use
    int opt_set = setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

    struct sockaddr_in bind_server = {};

    bind_server.sin_family = AF_INET;
    bind_server.sin_port = htons(1234);
    bind_server.sin_addr.s_addr = htonl(0);

    int rv = bind(server_fd, (const sockaddr *)&bind_server, sizeof(bind_server)); // returns 0 for success - 1  for error

    if (rv)
    {
        die("bind :: Failed to bind ");
    }

    rv = listen(server_fd, SOMAXCONN);

    if (rv)
    {
        die("listen :: Failed to listen ");
    }
    std::cout << "sever is listening at port 1234" << std::endl;

    while (true)
    {

        sockaddr_in client_conn = {};
        socklen_t addrlen = sizeof(client_conn);
        int connfd = accept(server_fd, (sockaddr *)&client_conn, &addrlen); // returns -1 for error

        struct in_addr ip_addr = {};
        ip_addr.s_addr = client_conn.sin_addr.s_addr;
        char *ip_addr_hr = inet_ntoa(ip_addr);

        std::cout << ip_addr_hr << std::endl;

        if (connfd < 0)
        {
            continue;
        }
        while (true)
        {
            int32_t err = handle_one_request(connfd);
            if (err)
                break;
        }

        close(connfd);
    }

    close(server_fd);
}
