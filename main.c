#include<stdio.h>
#include<sys/select.h>
#include<unistd.h>
#include<stdbool.h>

int main(void) {
        bool flag = true;
        fd_set fdset;

        FD_ZERO(&fdset);
        printf("%d", &fdset);
        return 0;
}