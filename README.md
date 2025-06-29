# Веб-Сервер с Фильтром Блума (C, Docker, Postman)

Учебный проект по курсу **«Основные методы и инструменты проектирования и разработки ПО»**.

## Описание

Минималистичный веб-сервер на языке C с поддержкой API-фильтра Блума.
Поддерживает настраиваемые типы ключей (`int`, `float`, `string`) и выбор хеш-функций при инициализации через `POST /init`.

## Структура проекта

```
.
├── server.c                # Основной HTTP-сервер
├── bloom.c / bloom.h       # Реализация фильтра Блума
├── Dockerfile              # Инструкция сборки Docker-образа
├── docker-compose.yml      # Описание запуска через Docker Compose
└── tests.json              # Коллекция Postman-тестов
```

## Быстрый старт

### Сборка и запуск

1. **Запустить сервер через Docker Compose:**

   ```bash
   docker-compose up
   ```

2. **Сервер будет доступен по адресу:**

   [http://localhost:8080](http://localhost:8080)

---

## API

### `GET /ping`

Проверка доступности сервера.

**Ответ:**

```
pong
```

---

### `POST /init`

Инициализация фильтра Блума.

**Параметры (form-urlencoded):**

| Параметр | Описание                             | Пример  |
| -------- | ------------------------------------ | ------- |
| `type`   | Тип ключей: `int`, `float`, `string` | `int`   |
| `hashes` | Список хеш-функций через запятую     | `1,2,3` |

Подсказка по hash-функциям
| ID | Имя    | Алгоритм   |
| -- | ------ | ---------- |
| 1  | simple | простой    |
| 2  | djb2   | популярный |
| 3  | sdbm   | надёжный   |


**Ответ:**

```
Initialized
```

---

### `POST /add`

Добавление ключа в фильтр.

**Параметры (form-urlencoded):**

| Параметр | Описание       | Пример |
| -------- | -------------- | ------ |
| `key`    | Значение ключа | `17`   |

**Ответ:**

```
Added
```

---

### `GET /check?key=...`

Проверка, находится ли ключ в фильтре.

**Пример запроса:**
`GET /check?key=17`

**Ответ:**

* `Probably in set` — ключ, вероятно, есть в фильтре
* `No` — ключа точно нет

---

## Тестирование через Postman

В проекте прилагается коллекция `tests.json`, включающая:

* Проверку доступности (`/ping`)
* Инициализацию фильтра с разными типами (`int`, `float`, `string`)
* Добавление и проверку значений
* Проверку на отсутствие ключей
