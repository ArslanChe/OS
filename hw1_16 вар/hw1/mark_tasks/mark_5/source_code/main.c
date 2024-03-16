#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
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
const int buf_size = 5000;

char *read_bytes_fd(int file_descriptor, int *size) {
    char *buffer = (char*)malloc(buf_size);
    ssize_t read_bytes;
    char *result = (char*)malloc(buf_size * 2);
    int result_size = buf_size * 2;
    int ptr = 0;
    while (1) {
        read_bytes = read(file_descriptor, buffer, buf_size);
        if (read_bytes == -1) {
            free(result);
            free(buffer);
            close(file_descriptor);
            return NULL;
        }

        if (ptr < result_size) {
            memcpy(result + ptr, buffer, read_bytes);
            ptr += read_bytes;
        } else {
            char *new_buffer = (char*)malloc(result_size * 2);
            memcpy(new_buffer, result, result_size);
            free(result);
            result = new_buffer;
            ptr = result_size;
            memcpy(result + ptr, buffer, read_bytes);
            result_size *= 2;
            ptr += read_bytes;
        }

        if (read_bytes != buf_size) {
            break;
        }
    }

    result[ptr] = '\0';
    (*size) = ptr;
    free(buffer);
    return result;
}

char *read_bytes(const char *name, int *size) {
    int file_descriptor;
    char *buffer = (char*)malloc(buf_size);

    if ((file_descriptor = open(name, O_RDONLY)) < 0) {
        puts("Can't open for reading");
        return NULL;
    }

    ssize_t read_bytes;
    char *result = (char*)malloc(buf_size * 2);
    int result_size = buf_size * 2;
    int ptr = 0;
    while (1) {
        read_bytes = read(file_descriptor, buffer, buf_size);
        if (read_bytes == -1) {
            free(result);
            free(buffer);
            close(file_descriptor);
            return NULL;
        }

        if (ptr < result_size) {
            memcpy(result + ptr, buffer, read_bytes);
            ptr += read_bytes;
        } else {
            char *new_buffer = (char*)malloc(result_size * 2);
            memcpy(new_buffer, result, result_size);
            free(result);
            result = new_buffer;
            ptr = result_size;
            memcpy(result + ptr, buffer, read_bytes);
            result_size *= 2;
            ptr += read_bytes;
        }

        if (read_bytes != buf_size) {
            break;
        }
    }

    result[ptr] = '\0';
    (*size) = ptr;
    free(buffer);
    return result;
}

void write_bytes(const char *name, const char *buf, int size) {
    int file_descriptor;
    if ((file_descriptor = open(name, O_WRONLY)) < 0) {
        puts("Can't open for writing");
        return;
    }

    ssize_t write_size = write(file_descriptor, buf, size);
    if (write_size != size) {
        puts("Can't write all string");
        return;
    }

    if (close(file_descriptor) < 0) {
        puts("Can't close");
    }
}

void write_bytes_create(const char *name, const char *buf, int size) {
    int file_descriptor;
    if ((file_descriptor = open(name, O_WRONLY | O_CREAT, 0666)) < 0) {
        puts("Can't open for writing");
        return;
    }

    ssize_t write_size = write(file_descriptor, buf, size);
    if (write_size != size) {
        puts("Can't write all string");
        return;
    }

    if (close(file_descriptor) < 0) {
        puts("Can't close");
    }
}

int* create_pipe() {
    int *fd = malloc(sizeof(int) * 2);
    fd[0] = fd[1] = -1;
    if (pipe(fd) < 0) {
        puts("Failed to create pipe");
    }

    return fd;
}

void create_fifo(const char *fifo_name) {
    (void) umask(0);
    mkfifo(fifo_name, 0666);
}
void run(const char *input, const char *output) {
    create_fifo("to_second.fifo");
    create_fifo("to_third.fifo");

    pid_t chpid = fork();
    if (chpid < 0) {
        puts("Failed to fork main process");
        return;
    }

    if (chpid > 0) {
        int size;
        char *str = read_bytes(input, &size);
        if (str == NULL) {
            return;
        }

        int fd;
        if ((fd = open("to_second.fifo", O_WRONLY)) < 0) {
            puts("First process: can't open to_second.fifo for writing");
            return;
        }

        ssize_t written_size = write(fd, str, size);
        if (size != written_size){
            puts("First process: can't write all string to fifo");
            return;
        }

        if (close(fd) < 0) {
            puts("First process: can't close to_second.fifo after writing");
            return;
        }

        free(str);
    } else {
        pid_t chpid_second = fork();
        if (chpid_second < 0) {
            puts("Failed to fork second process");
            return;
        }

        if (chpid_second > 0) {
            int fd;
            if ((fd = open("to_second.fifo", O_RDONLY)) < 0) {
                puts("Second process: can't open to_second.fifo for reading");
                return;
            }

            int size;
            char *str = read_bytes_fd(fd, &size);
            if (str == NULL) {
                return;
            }

            if (close(fd) < 0) {
                puts("Second process: can't close to_second.fifo after reading");
                return;
            }

            char *result = is_palindrome(str, size);
            if ((fd = open("to_third.fifo", O_WRONLY)) < 0) {
                puts("Second process: can't open to_third.fifo for writing");
                return;
            }

            ssize_t written_size = write(fd, result, 1);
            if (written_size != 1) {
                puts("Second process: can't write all string to fifo");
                return;
            }

            if (close(fd) < 0) {
                puts("Second process: can't close to_third.fifo for writing");
            }

            free(str);
        } else {
            int fd;
            if ((fd = open("to_third.fifo", O_RDONLY)) < 0) {
                puts("Third process: can't open to_third.fifo for reading");
                return;
            }

            int size;
            char *str = read_bytes_fd(fd, &size);
            if (str == NULL) {
                return;
            }

            if (close(fd) < 0) {
                puts("Third process: can't close to_third.fifo after reading");
                return;
            }

            write_bytes_create(output, str, size);
            free(str);
        }
    }
}

int main(int argc, char **argv) {
    if (argc != 3) {
        puts("Invalid console argument count!");
        return 0;
    }

    run(argv[1], argv[2]);
    return 0;
}
