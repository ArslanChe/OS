#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#define ROOM_COUNT 10

typedef struct {
    int rooms[ROOM_COUNT]; // 0 - свободен, 1 - занят
    sem_t mutex; // Мьютекс для защиты данных в разделяемой памяти
    sem_t room_sem; // Семафор для контроля доступа к номерам
} Hotel;

Hotel *hotel; // Указатель на разделяемую память

void init_resources() {
    int shm_fd = shm_open("/hotel_shm", O_CREAT | O_RDWR, 0644);
    ftruncate(shm_fd, sizeof(Hotel));
    hotel = mmap(0, sizeof(Hotel), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    // Инициализация семафоров в разделяемой памяти
    sem_init(&hotel->mutex, 1, 1);
    sem_init(&hotel->room_sem, 1, ROOM_COUNT);

    // Инициализация данных в разделяемой памяти
    memset(hotel->rooms, 0, sizeof(hotel->rooms));
}

void cleanup_resources() {
    // Удаление семафоров
    sem_destroy(&hotel->mutex);
    sem_destroy(&hotel->room_sem);

    // Освобождение разделяемой памяти
    munmap(hotel, sizeof(Hotel));
    shm_unlink("/hotel_shm");
}

void client_process(int id) {
    printf("Клиент %d пришел в гостиницу.\n", id + 1);
    int waiting = 0; // Флаг, отслеживающий, ожидает ли клиент на скамейке
    while (1) {
        sem_wait(&hotel->mutex); // Блокируем доступ к разделяемой памяти
        int found_room = -1;
        for (int i = 0; i < ROOM_COUNT; i++) {
            if (hotel->rooms[i] == 0) { // Если номер свободен
                found_room = i;
                hotel->rooms[i] = 1; // Занимаем номер
                break;
            }
        }

        if (found_room != -1) { // Если найден свободный номер
            if (waiting) {
                printf("Клиент %d перестал ожидать и занял номер %d.\n", id + 1, found_room + 1);
                waiting = 0; // Сброс флага ожидания
            } else {
                printf("Клиент %d занял номер %d.\n", id + 1, found_room + 1);
            }
            sem_post(&hotel->mutex); // Освобождаем доступ к разделяемой памяти
            sleep(5); // Имитация пребывания в номере

            sem_wait(&hotel->mutex);
            hotel->rooms[found_room] = 0;
            printf("Клиент %d освободил номер %d.\n", id + 1, found_room + 1);
            sem_post(&hotel->room_sem); // Освобождаем номер для других клиентов
            sem_post(&hotel->mutex);
            exit(0); // Завершаем процесс
        } else {
            sem_post(&hotel->mutex); // Освобождаем доступ к разделяемой памяти
            if (!waiting) {
                printf("Клиент %d ожидает на скамейке.\n", id + 1);
                waiting = 1; // Устанавливаем флаг ожидания
            }
            sem_wait(&hotel->room_sem); // Ожидаем свободный номер
        }
    }
}

void signal_handler(int sig) {
    cleanup_resources();
    exit(0);
}

int main(int argc, char *argv[]) {
    signal(SIGINT, signal_handler);

    int client_count = 20; // Количество клиентов по умолчанию
    if (argc > 1) client_count = atoi(argv[1]);

    init_resources();

    for (int i = 0; i < client_count; i++) {
        if (fork() == 0) {
            client_process(i);
        }
    }

    while (wait(NULL) > 0); // Ожидаем завершения всех дочерних процессов

    cleanup_resources();
    return 0;
}
