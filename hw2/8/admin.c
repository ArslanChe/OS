#include "ipc_resources.c"

int main() {
    int shm_id = init_resources();
    int sem_id = get_semaphore();
    Hotel *hotel = attach_memory(shm_id);

    // Пример работы: инициализация номеров
    for (int i = 0; i < ROOM_COUNT; i++) {
        hotel->rooms[i] = 0; // Все номера свободны
    }

    printf("Администратор запустил инициализацию номеров.\n");
    detach_memory(hotel);
    return 0;
}
