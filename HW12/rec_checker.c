#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define MAX_RECURSION_DEPTH 1000
#define BASE_FILENAME "a"
#define BUFFER_SIZE 64
#define DIR_NAME "test_files"

void check(int ret, const char* msg) {
    if (ret == -1) {
        perror(msg);
        exit(EXIT_FAILURE);
    }
}

int main() {
    // Создаем новую директорию.
    int ret = mkdir(DIR_NAME, 0755);
    check(ret, "Ошибка создания директории");

    // Меняем текущую рабочую директорию.
    ret = chdir(DIR_NAME);
    check(ret, "Ошибка изменения рабочей директории");

    // Создаем базовый файл.
    int fd = open(BASE_FILENAME, O_CREAT, S_IRUSR | S_IWUSR);
    check(fd, "Ошибка создания файла");
    close(fd);

    char src[BUFFER_SIZE] = BASE_FILENAME;
    char dst[BUFFER_SIZE] = "";
    int depth;

    // Создаем символьные связи и проверяем их доступность.
    for (depth = 0; depth < MAX_RECURSION_DEPTH; ++depth) {
        sprintf(dst, "%s%d", BASE_FILENAME, depth);

        // Создаем символьную связь.
        ret = symlink(src, dst);
        check(ret, "Ошибка создания символьной связи");

        // Пытаемся открыть файл через символьную связь.
        fd = open(dst, O_RDONLY);
        if (fd == -1) {
            break;  // Достигли глубины рекурсии.
        }
        close(fd);

        // Обновляем исходный файл.
        strncpy(src, dst, BUFFER_SIZE);
    }

    printf("Глубина рекурсии для символьных связей: %d\n", depth);

    return 0;
}

