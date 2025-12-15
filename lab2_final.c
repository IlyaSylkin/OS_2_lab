#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>
#include <string.h>

typedef struct {
    double **arrays;
    double *result;
    int start, end;
    int K;
} ThreadData;

void *sum_part(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    for (int i = data->start; i < data->end; i++) {
        double s = 0;
        for (int k = 0; k < data->K; k++)
            s += data->arrays[k][i];
        data->result[i] = s;
    }
    pthread_exit(NULL);
}

// Последовательная версия (эталон для сравнения)
double sequential_sum(double **arrays, double *result, int K, int N) {
    struct timeval start, end;
    gettimeofday(&start, NULL);
    
    for (int i = 0; i < N; i++) {
        double s = 0;
        for (int k = 0; k < K; k++)
            s += arrays[k][i];
        result[i] = s;
    }
    
    gettimeofday(&end, NULL);
    return (end.tv_sec - start.tv_sec) * 1000.0 +
           (end.tv_usec - start.tv_usec) / 1000.0;
}

int main(int argc, char *argv[]) {
    printf("====================================================\n");
    printf("   Исследование ускорения параллельного суммирования\n");
    printf("====================================================\n\n");
    
    // Тестовые конфигурации (ваши данные)
    int tests[][2] = {
        // K,    N
        {10,    10000},     // Малая задача: 100K элементов
        {100,   100000},    // Средняя задача: 10M элементов  
        {1000,  1000000},   // Большая задача: 1G элементов
    };
    
    int num_tests = sizeof(tests) / sizeof(tests[0]);
    int thread_counts[] = {2, 4, 8, 16, 32};  // Только параллельные варианты
    int num_threads = sizeof(thread_counts) / sizeof(thread_counts[0]);
    
    // Открываем файл для записи результатов
    FILE *csv = fopen("results.csv", "w");
    fprintf(csv, "Test,K,N,TotalElements,Threads,SeqTime_ms,ParTime_ms,Speedup,Efficiency\n");
    
    for (int test_idx = 0; test_idx < num_tests; test_idx++) {
        int K = tests[test_idx][0];
        int N = tests[test_idx][1];
        long total_elements = (long)K * N;
        
        const char *description;
        if (test_idx == 0) description = "Малая задача";
        else if (test_idx == 1) description = "Средняя задача";
        else description = "Большая задача";
        
        printf("\n%s\n", description);
        printf("K=%d, N=%d, Всего элементов: %'ld\n", K, N, total_elements);
        for (int i = 0; i < 50; i++) printf("=");
        printf("\n\n");
        
        // === 1. Выделение памяти ===
        double **arrays = malloc(K * sizeof(double *));
        if (!arrays) {
            printf("Ошибка выделения памяти для arrays!\n");
            return 1;
        }
        
        for (int i = 0; i < K; i++) {
            arrays[i] = malloc(N * sizeof(double));
            if (!arrays[i]) {
                printf("Ошибка выделения памяти для arrays[%d]!\n", i);
                return 1;
            }
        }
        
        double *result_seq = malloc(N * sizeof(double));
        double *result_par = malloc(N * sizeof(double));
        if (!result_seq || !result_par) {
            printf("Ошибка выделения памяти для результатов!\n");
            return 1;
        }
        
        // === 2. Генерация данных ===
        srand(time(NULL) + test_idx);
        printf("Генерация данных...\n");
        for (int i = 0; i < K; i++) {
            for (int j = 0; j < N; j++) {
                arrays[i][j] = (double)(rand() % 100);
            }
        }
        
        // === 3. Последовательная версия (ЭТАЛОН) ===
        printf("\nПоследовательная версия (эталон):\n");
        double seq_time = sequential_sum(arrays, result_seq, K, N);
        printf("  Время: %'.2f мс\n", seq_time);
        printf("  Производительность: %'.0f эл/мс\n\n", total_elements / seq_time);
        
        // === 4. Параллельные версии ===
        printf("Параллельные версии:\n");
        printf("┌───────┬─────────────┬────────────┬─────────────┬──────────┐\n");
        printf("│ Потоки│ Время(мс)   │ Ускорение  │ Эффективность│ Эл/мс    │\n");
        printf("├───────┼─────────────┼────────────┼─────────────┼──────────┤\n");
        
        for (int t_idx = 0; t_idx < num_threads; t_idx++) {
            int threads = thread_counts[t_idx];
            
            // Проверка: нельзя иметь больше потоков чем элементов
            if (threads > N) {
                printf("│ %5d │   N/A        │    N/A      │     N/A      │   N/A    │\n", threads);
                continue;
            }
            
            // Очистка массива результатов
            memset(result_par, 0, N * sizeof(double));
            
            // === ЗАМЕР ВРЕМЕНИ ПАРАЛЛЕЛЬНОЙ ВЕРСИИ ===
            struct timeval start, end;
            gettimeofday(&start, NULL);
            
            // Создание и настройка потоков
            pthread_t *thread_list = malloc(threads * sizeof(pthread_t));
            ThreadData *data_list = malloc(threads * sizeof(ThreadData));
            
            if (!thread_list || !data_list) {
                printf("Ошибка выделения памяти для потоков!\n");
                return 1;
            }
            
            // Распределение работы между потоками
            int base_chunk = N / threads;
            int remainder = N % threads;
            int current_start = 0;
            
            for (int t = 0; t < threads; t++) {
                data_list[t].arrays = arrays;
                data_list[t].result = result_par;
                data_list[t].K = K;
                data_list[t].start = current_start;
                
                int chunk_size = base_chunk + (t < remainder ? 1 : 0);
                data_list[t].end = current_start + chunk_size;
                current_start = data_list[t].end;
                
                pthread_create(&thread_list[t], NULL, sum_part, &data_list[t]);
            }
            
            // Ожидание завершения всех потоков
            for (int t = 0; t < threads; t++) {
                pthread_join(thread_list[t], NULL);
            }
            
            gettimeofday(&end, NULL);
            
            // Освобождение памяти потоков
            free(thread_list);
            free(data_list);
            
            // Расчет времени выполнения
            double par_time = (end.tv_sec - start.tv_sec) * 1000.0 +
                            (end.tv_usec - start.tv_usec) / 1000.0;
            
            // === ПРОВЕРКА КОРРЕКТНОСТИ ===
            int correct = 1;
            for (int i = 0; i < N && correct; i++) {
                if (fabs(result_seq[i] - result_par[i]) > 0.001) {
                    correct = 0;
                }
            }
            
            if (!correct) {
                printf("│ %5d │   ERROR      │    ERROR    │     ERROR    │   ERROR  │\n", threads);
                continue;
            }
            
            // === РАСЧЕТ МЕТРИК ===
            double speedup = seq_time / par_time;
            double efficiency = (speedup / threads) * 100;
            double throughput = total_elements / par_time;
            
            // === ВЫВОД РЕЗУЛЬТАТОВ ===
            printf("│ %5d │ %11.2f │ %10.2f │ %11.1f%% │ %8.0f │\n", 
                   threads, par_time, speedup, efficiency, throughput);
            
            // === СОХРАНЕНИЕ В CSV ===
            fprintf(csv, "Test%d,%d,%d,%ld,%d,%.2f,%.2f,%.3f,%.2f\n",
                    test_idx+1, K, N, total_elements, threads, 
                    seq_time, par_time, speedup, efficiency);
        }
        
        printf("└───────┴─────────────┴────────────┴─────────────┴──────────┘\n\n");
        
        // === ОСВОБОЖДЕНИЕ ПАМЯТИ ===
        for (int i = 0; i < K; i++) {
            free(arrays[i]);
        }
        free(arrays);
        free(result_seq);
        free(result_par);
    }
    
    fclose(csv);
    
    // === ФИНАЛЬНЫЙ ВЫВОД ===
    printf("\n====================================================\n");
    printf("           ТЕСТИРОВАНИЕ ЗАВЕРШЕНО\n");
    printf("====================================================\n");
    printf("\nРезультаты сохранены в файл: results.csv\n");
    printf("Формат CSV: Test,K,N,TotalElements,Threads,SeqTime_ms,ParTime_ms,Speedup,Efficiency\n");
    printf("\nДля построения графиков запустите: python3 plot_final.py\n");
    printf("\nКраткая статистика:\n");
    printf("- Протестировано: %d различных размеров задач\n", num_tests);
    printf("- Потоки: 2, 4, 8, 16, 32\n");
    printf("- Всего измерений: %d\n", num_tests * num_threads);
    
    return 0;
}