#include <stdio.h>
#include <fcntl.h>

#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

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
    unlink(fifo_name);
    (void) umask(0);
    mknod(fifo_name, S_IFIFO | 0666, 0);
}
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
void run(const char *input, const char *output) {
    int *pipe_to_second = create_pipe();
    int *pipe_to_third = create_pipe();

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

        if (close(pipe_to_second[0]) < 0) {
            puts("First process: can't close reading side of pipe");
            return;
        }

        ssize_t written_size = write(pipe_to_second[1], str, size);
        if (size != written_size){
            puts("First process: can't write all string to pipe");
            return;
        }

        if (close(pipe_to_second[1]) < 0) {
            puts("First process: can't close writing side of pipe");
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
            if (close(pipe_to_second[1]) < 0) {
                puts("Second process: can't close writing side of pipe");
                return;
            }

            int size;
            char *str = read_bytes_fd(pipe_to_second[0], &size);
            if (str == NULL) {
                return;
            }

            char *result = is_palindrome(str, size);
            if (close(pipe_to_third[0]) < 0) {
                puts("Second process: can't close reading side of pipe");
                return;
            }

            ssize_t written_size = write(pipe_to_third[1], result, 1);
            if (written_size != 1) {
                puts("Second process: can't write all string to pipe");
                return;
            }

            if (close(pipe_to_third[1]) < 0) {
                puts("Second process: can't close writing side of pipe");
            }

            free(str);
        } else {
            if (close(pipe_to_third[1]) < 0) {
                puts("Third process: can't close writing side of pipe");
                return;
            }

            int size;
            char *str = read_bytes_fd(pipe_to_third[0], &size);
            if (str == NULL) {
                return;
            }

            if (close(pipe_to_third[0]) < 0) {
                puts("Third process: can't close reading side of pipe");
                return;
            }

            write_bytes_create(output, str, size);
            free(str);
        }
    }


    free(pipe_to_second);
    free(pipe_to_third);
}

int main(int argc, char **argv) {
    if (argc != 3) {
        puts("Invalid console argument count!");
        return 0;
    }

    run(argv[1], argv[2]);
    return 0;
}
