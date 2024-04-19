#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <semaphore.h>
#include <string.h>

#define ROOM_COUNT 10

typedef struct {
    int rooms[ROOM_COUNT]; // 0 - свободен, 1 - занят
} Hotel;

Hotel *hotel; // Указатель на разделяемую память
sem_t *room_sem; // Семафор для контроля доступа к номерам
sem_t *mutex; // Мьютекс для защиты данных в разделяемой памяти

void init_resources() {
    // Открываем именованные семафоры
    room_sem = sem_open("/room_sem", O_CREAT, 0644, ROOM_COUNT);
    mutex = sem_open("/mutex", O_CREAT, 0644, 1);

    // Создаем разделяемую память
    int shm_fd = shm_open("/hotel_shm", O_CREAT | O_RDWR, 0644);
    ftruncate(shm_fd, sizeof(Hotel));
    hotel = mmap(0, sizeof(Hotel), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    // Инициализируем данные в разделяемой памяти
    memset(hotel->rooms, 0, sizeof(hotel->rooms)); // Все номера свободны
}

void cleanup_resources() {
    // Закрываем и удаляем семафоры
    sem_close(room_sem);
    sem_close(mutex);
    sem_unlink("/room_sem");
    sem_unlink("/mutex");

    // Освобождаем разделяемую память
    munmap(hotel, sizeof(Hotel));
    shm_unlink("/hotel_shm");
}

void client_process(int id) {
    printf("Клиент %d пришел в гостиницу.\n", id + 1);
    int waiting = 0; // Флаг, отслеживающий, ожидает ли клиент на скамейке
    while (1) {
        sem_wait(mutex); // Блокируем доступ к разделяемой памяти
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
            sem_post(mutex); // Освобождаем доступ к разделяемой памяти
            sleep(5); // Имитация пребывания в номере

            sem_wait(mutex);
            hotel->rooms[found_room] = 0;
            printf("Клиент %d освободил номер %d.\n", id + 1, found_room + 1);
            sem_post(room_sem); // Освобождаем номер для других клиентов
            sem_post(mutex);
            exit(0); // Завершаем процесс
        } else {
            sem_post(mutex); // Освобождаем доступ к разделяемой памяти
            if (!waiting) {
                printf("Клиент %d ожидает на скамейке.\n", id + 1);
                waiting = 1; // Устанавливаем флаг ожидания
            }
            sem_wait(room_sem); // Ожидаем свободный номер
        }
    }
}

void signal_handler(int sig) {
    cleanup_resources();
    exit(0);
}

int main(int argc, char *argv[]) {
    signal(SIGINT, signal_handler); // Обрабатываем сигнал прерывания
    
    int client_count = 20; // Количество клиентов по умолчанию
    if (argc > 1) client_count = atoi(argv[1]);

    init_resources();

    for (int i = 0; i < client_count; i++) {
        if (fork() == 0) {
            client_process(i);
        }
    }

    // Ожидаем завершения всех дочерних процессов
    for (int i = 0; i < client_count; i++) {
        wait(NULL);
    }

    cleanup_resources();
    return 0;
}

