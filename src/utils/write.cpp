#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <cassert>
#include <log.hpp>
// memcpy can also throw errors but we don't handle them
int32_t write_all(int client_conn_fd, char *write_buff, int n)
{
    // while we get n bytes
    while (n > 0)
    {
        ssize_t recvd = write(client_conn_fd, write_buff, n);

        if (recvd <= 0) // if we get < 0 then it's some kind of error
        {
            msg("write err()");
            return -1;
        }
        // we should always get recvd bytes <= n
        assert((size_t)recvd <= n);
        n -= (size_t)recvd;
    }
    return 0;
}
