#include<stdio.h>
#include <sys/select.h>
#include <unistd.h>
#include <stdbool.h>
#include <pty.h>
#include <limits.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

static int masterfd;

int32_t decodeutf8(const char* s, uint32_t *out_cp) {
        unsigned char c = s[0];
        if (c < 0x80) {
                *out_cp = c;
                return 1;
        }
        if ((c >> 5) == 0x6) {
                *out_cp = ((c & 0x1F) << 6) | (s[1] & 0x3F);
                return 2;
        }
        if ((c >> 4) == 0xE) {
                *out_cp = ((c & 0x0F) << 12)| ((s[1] & 0x3F << 6)) | (s[2] & 0x3F);
                return 3;
        }
        if ((c >> 3) == 0x1E) {
                *out_cp = ((c & 0x07) << 18)| ((s[2] & 0x3F) << 12) | ((s[1] & 0x3F << 6)) | (s[3] & 0x3F);
                return 4;
        }
        return -1;
}

size_t readPrompt(void) {
        static char buffer[SHRT_MAX];
        static uint32_t bufflen = 0;

        uint32_t nbytes = read(masterfd, buffer + bufflen, sizeof(buffer) - bufflen);
        bufflen += nbytes;

        uint32_t iter = 0;
        while(iter < bufflen) {
                uint32_t point;
                uint32_t len = decodeutf8(&buffer[iter], &point);
                if(len < 0 || len > bufflen)
                        break;
                printf("%i\n", point);
                iter += len;
        }
        if(iter < bufflen) {
                memmove(buffer, buffer + iter, bufflen - iter);
        }
        bufflen -= iter;
        return nbytes;
}

int main(void)
{

        if (forkpty(&masterfd, NULL, NULL, NULL) == 0) {
                execlp("bash", "bash", NULL);
                perror("execlp");
                exit(1);
        }
        bool running = true;
        fd_set fdset;

        while(running){
                FD_ZERO(&fdset);
                FD_SET(masterfd, &fdset);
                select(masterfd + 1, &fdset, NULL, NULL, NULL);

                if(FD_ISSET(masterfd, &fdset)) {
                        readPrompt();
                }
        }

        return EXIT_SUCCESS;
}