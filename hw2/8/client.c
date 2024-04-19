#include "ipc_resources.c"
#include <unistd.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s client_id\n", argv[0]);
        exit(1);
    }

    int client_id = atoi(argv[1]);
    int shm_id = init_resources();
    int sem_id = get_semaphore();
    Hotel *hotel = attach_memory(shm_id);

    struct sembuf buf;

    int waiting = 0;
    while (1) {
        // Захват мьютекса
        buf.sem_num = 0;
        buf.sem_op = -1;
        buf.sem_flg = 0;
        semop(sem_id, &buf, 1);

        int found_room = -1;
        for (int i = 0; i < ROOM_COUNT; i++) {
            if (hotel->rooms[i] == 0) {
                hotel->rooms[i] = 1;
                found_room = i;
                break;
            }
        }

        if (found_room != -1) {
            printf("Клиент %d занял номер %d.\n", client_id, found_room + 1);
            // Освобождение мьютекса
            buf.sem_num = 0;
            buf.sem_op = 1;
            semop(sem_id, &buf, 1);

            sleep(15);  // Представим, что клиент остается на ночь

            // Захват мьютекса снова
            buf.sem_num = 0;
            buf.sem_op = -1;
            semop(sem_id, &buf, 1);

            hotel->rooms[found_room] = 0;
            printf("Клиент %d освободил номер %d.\n", client_id, found_room + 1);
            
            // Освобождение мьютекса и выход
            buf.sem_num = 0;
            buf.sem_op = 1;
            semop(sem_id, &buf, 1);
            break;
        } else {
            // Освобождение мьютекса
            buf.sem_num = 0;
            buf.sem_op = 1;
            semop(sem_id, &buf, 1);
            
            if (!waiting) {
                printf("Клиент %d ожидает на скамейке.\n", client_id);
                waiting = 1;
            }
        }
    }

    detach_memory(hotel);
    return 0;
}
