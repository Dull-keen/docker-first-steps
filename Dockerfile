# Используем официальный образ Python
FROM python:3.9-slim

# Устанавливаем рабочую директорию
WORKDIR /app

# Копируем файл с зависимостями (если есть)
COPY requirements.txt .

# Устанавливаем зависимости
RUN pip install --no-cache-dir -r requirements.txt

# Копируем код сервера
COPY server.py .

# Указываем команду для запуска сервера
CMD ["python", "server.py"]

# Открываем порт
EXPOSE 5000
