#include <stdio.h>
#include "pipe_helper.c"

char* is_palindrome(const char *string, int size) {
    int left = 0;
    int right = size - 1;

    while (right - left > 0) {
        if (string[left++] != string[right--]) {
            return "0";
        }
    }

    return "1";
}
void run() {
    int fd;
    if ((fd = open("main.fifo", O_RDONLY)) < 0) {
        puts("Second process: can't open main.fifo for reading");
        return;
    }

    int size;
    char *str = read_bytes_fd(fd, &size);
    if (str == NULL) {
        return;
    }

    if (close(fd) < 0) {
        puts("Second process: can't close main.fifo after reading");
        return;
    }

    char *result = is_palindrome(str, size);
    if ((fd = open("back.fifo", O_WRONLY)) < 0) {
        puts("Second process: can't open back.fifo for writing");
        return;
    }

    ssize_t written_size = write(fd, result, 1);
    if (written_size != 1) {
        puts("Second process: can't write all string to back.fifo");
        return;
    }

    if (close(fd) < 0) {
        puts("Second process: can't close back.fifo for writing");
    }

    free(str);
}

int main() {
    run();
    return 0;
}
