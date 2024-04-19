#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <string.h>

#define ROOM_COUNT 10

typedef struct {
    int rooms[ROOM_COUNT];
} Hotel;


union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

int init_resources() {
    key_t key = ftok("shmfile", 65);
    if (key == -1) {
        perror("ftok failed");
        exit(1);
    }
    int shm_id = shmget(key, sizeof(Hotel), 0666|IPC_CREAT);
    if (shm_id == -1) {
        perror("shmget failed");
        exit(1);
    }
    return shm_id;
}

int get_semaphore() {
    key_t key = ftok("shmfile", 65);
    if (key == -1) {
        perror("ftok failed");
        exit(1);
    }
    int sem_id = semget(key, 2, 0666|IPC_CREAT); // Создаем два семафора
    if (sem_id == -1) {
        perror("semget failed");
        exit(1);
    }

    // Настройка начальных значений семафоров
    union semun u;
    u.val = 1;
    if (semctl(sem_id, 0, SETVAL, u) == -1) { // Мьютекс
        perror("semctl mutex failed");
        exit(1);
    }

    u.val = ROOM_COUNT;
    if (semctl(sem_id, 1, SETVAL, u) == -1) { // Семафор номеров
        perror("semctl room sem failed");
        exit(1);
    }

    return sem_id;
}


Hotel* attach_memory(int shm_id) {
    Hotel* hotel = (Hotel*) shmat(shm_id, NULL, 0);
    if (hotel == (void*) -1) {
        perror("shmat failed");
        exit(1);
    }
    return hotel;
}

void detach_memory(Hotel* hotel) {
    if (shmdt(hotel) == -1) {
        perror("shmdt failed");
        exit(1);
    }
}

void cleanup_resources(int shm_id, int sem_id) {
    if (shmctl(shm_id, IPC_RMID, NULL) == -1) {
        perror("shmctl failed");
    }
    if (semctl(sem_id, 0, IPC_RMID, NULL) == -1) {
        perror("semctl failed");
    }
}
