#!/bin/bash

# Путь к исполняемому файлу клиента
CLIENT_EXECUTABLE="./client"

# Количество клиентов для запуска
CLIENT_COUNT=20

echo "Запуск $CLIENT_COUNT клиентов..."

# Запуск указанного количества клиентов в фоновом режиме
for (( i=1; i<=CLIENT_COUNT; i++ ))
do
    $CLIENT_EXECUTABLE $i &
    echo "Клиент $i запущен."
    sleep 1
done

echo "Все клиенты были запущены."
wait
