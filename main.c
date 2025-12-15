#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

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

int main() {
    int K, N, max_threads;

    printf("Введите количество массивов (K): ");
    if (scanf("%d", &K) != 1 || K <= 0) {
        printf("Ошибка: неверное значение K!\n");
        return 1;
    }

    printf("Введите длину массивов (N): ");
    if (scanf("%d", &N) != 1 || N <= 0) {
        printf("Ошибка: неверное значение N!\n");
        return 1;
    }

    printf("Введите максимальное количество потоков: ");
    if (scanf("%d", &max_threads) != 1 || max_threads <= 0) {
        printf("Ошибка: неверное количество потоков!\n");
        return 1;
    }

    double **arrays = malloc(K * sizeof(double *));
    if (arrays == NULL) {
        printf("Ошибка выделения памяти для arrays!\n");
        return 1;
    }
    
    for (int i = 0; i < K; i++) {
        arrays[i] = malloc(N * sizeof(double));
        if (arrays[i] == NULL) {
            printf("Ошибка выделения памяти для arrays[%d]!\n", i);
            // Освобождаем ранее выделенную память
            for (int j = 0; j < i; j++) free(arrays[j]);
            free(arrays);
            return 1;
        }
    }
    
    double *result = malloc(N * sizeof(double));
    if (result == NULL) {
        printf("Ошибка выделения памяти для result!\n");
        for (int i = 0; i < K; i++) free(arrays[i]);
        free(arrays);
        return 1;
    }

    srand(time(NULL));

    printf("Генерация данных...\n");
    for (int i = 0; i < K; i++) {
        for (int j = 0; j < N; j++) {
            arrays[i][j] = rand() % 100; 
        }
    }

    // Определяем кол потоков
    int threads_count = (N < max_threads) ? N : max_threads;
    if (threads_count == 0) threads_count = 1; // защита от дел на 0
    
    pthread_t threads[threads_count];
    ThreadData data[threads_count];

    int block = N / threads_count;
    printf("Используется %d потоков, блок по %d элементов\n", threads_count, block);

    for (int t = 0; t < threads_count; t++) {
        data[t].arrays = arrays;
        data[t].result = result;
        data[t].K = K;
        data[t].start = t * block;
        data[t].end = (t == threads_count - 1) ? N : (t + 1) * block;

        int rc = pthread_create(&threads[t], NULL, sum_part, &data[t]);
        if (rc) {
            printf("Ошибка создания потока %d: %d\n", t, rc);
            return 1;
        }
    }

    // Ждём завершения
    for (int t = 0; t < threads_count; t++) {
        int rc = pthread_join(threads[t], NULL);
        if (rc) {
            printf("Ошибка ожидания потока %d: %d\n", t, rc);
        }
    }

    printf("\nПервые 10 элементов результата:\n");
    for (int i = 0; i < (N < 10 ? N : 10); i++)
        printf("C[%d] = %.2f\n", i, result[i]);

    // Осв памяти
    for (int i = 0; i < K; i++)
        free(arrays[i]);
    free(arrays);
    free(result);

    printf("Программа завершена успешно!\n");
    return 0;
}