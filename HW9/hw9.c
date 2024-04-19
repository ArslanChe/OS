#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <fcntl.h>

key_t sem_key = 777;
const int BUFF_SIZE = 10000;

int createSem() {
    int sem_id;
    if ((sem_id = semget(sem_key++, 2, 0666 | IPC_CREAT)) < 0) {
        exit(-1);
    }
    printf("createSem: ID: %d\n", sem_id);
    return sem_id;
}

struct sembuf setSem(int num, int op, int flg) {
    struct sembuf s_op;
    s_op.sem_op = op;
    s_op.sem_flg = flg;
    s_op.sem_num = num;
    return s_op;
}

// Удаляем семафоры
void removeSem(int semid) {
    if (semctl(semid, 0, IPC_RMID) < 0) {
        exit(-1);
    }
}

// Сгенерировать случайное число
int getInt(int a, int b) {
    return a + rand() % (b - a);
}

int operations_num;
int shm_id;
char buffer[BUFF_SIZE];
struct sembuf semaphore_op;
int writer_id;
int reader_id;
int writer_p;
int io_pipe[2];

int main(int argc, char const *argv[]) {

    if (argc < 2) {
        operations_num = 10;
    } else {
        operations_num = atoi(argv[1]);
    }

    writer_id = createSem();
    reader_id = createSem();

    if (pipe(io_pipe) < 0) {
        exit(-1);
    }

    if ((shm_id = shm_open("/posix-shared-object", O_CREAT | O_RDWR, 0666)) == -1) {
        perror("shm_open error");
        exit(1);
    }

    if (ftruncate(shm_id, sizeof(buffer)) == -1) {
        perror("ftruncate error");
        exit(1);
    }

    char* shared_buffer = mmap(NULL, sizeof(buffer), PROT_READ | PROT_WRITE, MAP_SHARED, shm_id, 0);
    if (shared_buffer == MAP_FAILED) {
        perror("mmap error");
        exit(1);
    }

    writer_p = fork();
    if (writer_p < 0) {
        printf("Unable to create child process\n");
        exit(-1);
    } else if (writer_p == 0) {
        sleep(1);
        for (int i = 0; i < operations_num; i++) {
            int random_number = getInt(0, 100);
            sprintf(buffer, "%d", random_number);
            strncpy(shared_buffer, buffer, BUFF_SIZE);
            semaphore_op = setSem(0, 1, 0);
            if (semop(reader_id, &semaphore_op, 1) < 0) {
                printf("Semaphore error\n");
                exit(-1);
            }
            semaphore_op = setSem(0, -1, 0);
            if (semop(writer_id, &semaphore_op, 1) < 0) {
                printf("Semaphore error\n");
                exit(-1);
            }
        }

        printf("Writer has finished work \n");

    } else {
        for (int i = 0; i < operations_num; i++) {
            semaphore_op = setSem(0, -1, 0);
            if (semop(reader_id, &semaphore_op, 1) < 0) {
                printf("Semaphore error\n");
                exit(-1);
            }
            strncpy(buffer, shared_buffer, BUFF_SIZE);
            printf("Received number: %s\n", buffer);
            semaphore_op = setSem(0, 1, 0);
            if (semop(writer_id, &semaphore_op, 1) < 0) {
                printf("Semaphore error\n");
                exit(-1);
            }
        }
        printf("Reader has finished work\n");
        removeSem(writer_id);
        removeSem(reader_id);
    }
    if (munmap(shared_buffer, sizeof(buffer)) == -1) {
        perror("munmap error");
        exit(1);
    }
    if (shm_unlink("/posix-shared-object") == -1) {
        perror("shm_unlink error");
        exit(1);
    }
    return 0;
}