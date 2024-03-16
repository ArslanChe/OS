#include <stdio.h>
#include "pipe_helper.c"

void run(const char *input, const char *output) {
    create_fifo("main.fifo");
    create_fifo("back.fifo");

    int size;
    char *str = read_bytes(input, &size);
    if (str == NULL) {
        return;
    }

    int fd;
    if ((fd = open("main.fifo", O_WRONLY)) < 0) {
        puts("First process: can't open main.fifo for writing");
        return;
    }

    ssize_t written_size = write(fd, str, size);
    if (size != written_size) {
        puts("First process: can't write all string to main.fifo");
        return;
    }

    if (close(fd) < 0) {
        puts("First process: can't close main.fifo after writing");
        return;
    }

    if ((fd = open("back.fifo", O_RDONLY)) < 0) {
        puts("First process: can't open back.fifo for reading");
        return;
    }

    free(str);
    str = read_bytes_fd(fd, &size);
    if (str == NULL) {
        return;
    }

    if (close(fd) < 0) {
        puts("First process: can't close back.fifo after reading");
        return;
    }

    write_bytes_create(output, str, size);
    free(str);
}

int main(int argc, char **argv) {
    if (argc != 3) {
        puts("Invalid console argument count!");
        return 0;
    }

    run(argv[1], argv[2]);
    return 0;
}
