// memcpy can also throw errors but we don't handle them
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <cassert>
#include <log.hpp>

int32_t read_full(int client_conn_fd, char *read_buff, size_t n)
{ // while we get n bytes
    while (n > 0)
    {
        ssize_t recvd = read(client_conn_fd, read_buff, n);

        if (recvd == 0)
        {
            msg("EOF");
            return -1;
        }

        if (recvd < 0) // if we get < 0 then it's some kind of error
        {
            if (recvd == EINTR)
            {
                msg("read() recieved EINTR");
                continue;
            }
            msg("read err() 0");
            return -1;
        }
        // we should always get recvd bytes <= n
        assert((size_t)recvd <= n);
        n -= (size_t)recvd;
        read_buff += recvd; // where should next read call start writing to buf from
        // so this is pointer arithmetic
        // lets say read_buff was on pointer 0x1234 then read got 3 bytes not we need to write after that 3 bytes
        // so we increment the read_buff pointer to 0x1237 so read will write the next data from here
        // otherwise it will overwrite it.
    }
    return 0;
}