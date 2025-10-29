# Компилятор и флаги
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -pthread
TARGET = lab
SOURCE = main.c

# Сборка по умолчанию
all: $(TARGET)

# Сборка основной программы
$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCE)

# Запуск программы
run: $(TARGET)
	./$(TARGET)

# Очистка собранных файлов
clean:
	rm -f $(TARGET)

# Пересборка
rebuild: clean all

# Отладочная сборка
debug: CFLAGS += -g -O0
debug: $(TARGET)

# Профилировочная сборка
profile: CFLAGS += -pg
profile: $(TARGET)

.PHONY: all run clean rebuild debug profile