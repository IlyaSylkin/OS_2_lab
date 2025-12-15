#!/bin/bash

echo "========================================"
echo " Лабораторная работа: Параллельное суммирование"
echo "========================================"
echo ""

# 1. Компиляция
echo "1. Компиляция программы..."
gcc -o lab2_final lab2_final.c -lpthread -lm
if [ $? -ne 0 ]; then
    echo "Ошибка компиляции!"
    exit 1
fi
echo "   [OK] Программа скомпилирована"

# 2. Запуск тестов
echo ""
echo "2. Запуск тестов..."
echo "   Тест 1: K=10, N=10000 (100K элементов)"
echo "   Тест 2: K=100, N=100000 (10M элементов)"
echo "   Тест 3: K=1000, N=1000000 (1G элементов)"
echo ""
./lab2_final
if [ $? -ne 0 ]; then
    echo "Ошибка выполнения!"
    exit 1
fi
echo "   [OK] Тесты выполнены"

# 3. Проверка зависимостей Python
echo ""
echo "3. Проверка зависимостей Python..."
python3 -c "import pandas, matplotlib" 2>/dev/null
if [ $? -ne 0 ]; then
    echo "   Установка зависимостей: pip install pandas matplotlib"
    pip install pandas matplotlib --quiet
fi

# 4. Построение графиков
echo ""
echo "4. Построение графиков..."
python3 plot_final.py
if [ $? -ne 0 ]; then
    echo "Ошибка построения графиков!"
    exit 1
fi
echo "   [OK] Графики построены"

# 5. Итог
echo ""
echo "========================================"
echo "          ТЕСТИРОВАНИЕ ЗАВЕРШЕНО"
echo "========================================"
echo ""
echo "Созданы файлы:"
echo "  results.csv        - таблица результатов"
echo "  speedup.png        - график ускорения"
echo "  efficiency.png     - график эффективности"
echo "  time_comparison.png - сравнение времени"
echo ""
echo "Для просмотра результатов:"
echo "  cat results.csv"
echo "  eog *.png  # или open *.png на macOS"