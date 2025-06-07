# Используем официальный образ Debian
FROM debian:bullseye

# Устанавливаем необходимые пакеты
RUN apt-get update && apt-get install -y gcc libmicrohttpd-dev && apt-get clean

# Устанавливаем рабочую директорию
WORKDIR /app

# Копируем код сервера
COPY server.c .

# Указываем команду для запуска сервера
RUN gcc server.c -o server -lmicrohttpd

# Открываем порт
EXPOSE 8080

# Запуск сервера
CMD ["./server"]
