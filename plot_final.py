import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

def load_results():
    """Загрузка результатов"""
    df = pd.read_csv('results.csv')
    print("Загружено результатов:", len(df))
    return df

def plot_speedup(df):
    """График ускорения"""
    plt.figure(figsize=(10, 6))
    
    # Для каждой задачи
    tests = df['Test'].unique()
    colors = ['blue', 'green', 'red']
    markers = ['o', 's', '^']
    
    for test, color, marker in zip(tests, colors, markers):
        test_data = df[df['Test'] == test]
        K = test_data['K'].iloc[0]
        N = test_data['N'].iloc[0]
        
        label = f'K={K}, N={N:,} ({K*N:,} эл.)'
        plt.plot(test_data['Threads'], test_data['Speedup'], 
                marker=marker, linewidth=2, markersize=8,
                color=color, label=label)
    
    # Идеальное ускорение
    threads = sorted(df['Threads'].unique())
    plt.plot(threads, threads, 'k--', alpha=0.5, label='Идеальное ускорение')
    
    plt.xlabel('Количество потоков')
    plt.ylabel('Ускорение (T_seq / T_par)')
    plt.title('Зависимость ускорения от количества потоков')
    plt.legend()
    plt.grid(True, alpha=0.3)
    plt.xticks(threads)
    plt.savefig('speedup.png', dpi=150, bbox_inches='tight')
    print("График сохранен: speedup.png")
    plt.show()

def plot_efficiency(df):
    """График эффективности"""
    plt.figure(figsize=(10, 6))
    
    tests = df['Test'].unique()
    colors = ['blue', 'green', 'red']
    markers = ['o', 's', '^']
    
    for test, color, marker in zip(tests, colors, markers):
        test_data = df[df['Test'] == test]
        K = test_data['K'].iloc[0]
        N = test_data['N'].iloc[0]
        
        label = f'K={K}, N={N:,}'
        plt.plot(test_data['Threads'], test_data['Efficiency'], 
                marker=marker, linewidth=2, markersize=8,
                color=color, label=label)
    
    # Линия 100% эффективности
    plt.axhline(y=100, color='k', linestyle='--', alpha=0.5, label='100% эффективность')
    
    plt.xlabel('Количество потоков')
    plt.ylabel('Эффективность (%)')
    plt.title('Зависимость эффективности от количества потоков')
    plt.legend()
    plt.grid(True, alpha=0.3)
    plt.xticks(sorted(df['Threads'].unique()))
    plt.ylim(0, 110)
    plt.savefig('efficiency.png', dpi=150, bbox_inches='tight')
    print("График сохранен: efficiency.png")
    plt.show()

def plot_time_comparison(df):
    """Сравнение времени"""
    fig, axes = plt.subplots(1, 3, figsize=(15, 5))
    
    tests = df['Test'].unique()
    
    for idx, test in enumerate(tests):
        ax = axes[idx]
        test_data = df[df['Test'] == test]
        K = test_data['K'].iloc[0]
        N = test_data['N'].iloc[0]
        
        # Время параллельных версий
        ax.plot(test_data['Threads'], test_data['ParTime_ms'], 'bo-', 
               linewidth=2, markersize=8, label='Параллельное время')
        
        # Последовательное время (горизонтальная линия)
        seq_time = test_data['SeqTime_ms'].iloc[0]
        ax.axhline(y=seq_time, color='r', linestyle='--', 
                  linewidth=2, label=f'Последовательное: {seq_time:.1f} мс')
        
        ax.set_title(f'K={K}, N={N:,}\n({K*N:,} элементов)')
        ax.set_xlabel('Потоки')
        ax.set_ylabel('Время (мс)')
        ax.legend()
        ax.grid(True, alpha=0.3)
        ax.set_xticks(test_data['Threads'])
    
    plt.tight_layout()
    plt.savefig('time_comparison.png', dpi=150, bbox_inches='tight')
    print("График сохранен: time_comparison.png")
    plt.show()

def create_report(df):
    """Создание текстового отчета"""
    print("\n" + "="*70)
    print("                 АНАЛИТИЧЕСКИЙ ОТЧЕТ")
    print("="*70)
    
    print("\nОБЩАЯ СТАТИСТИКА:")
    print(f"Количество тестов: {len(df['Test'].unique())}")
    print(f"Количество измерений: {len(df)}")
    print(f"Диапазон потоков: от {df['Threads'].min()} до {df['Threads'].max()}")
    
    print("\n" + "="*70)
    print("РЕЗУЛЬТАТЫ ПО ТЕСТАМ:")
    
    for test in sorted(df['Test'].unique()):
        test_data = df[df['Test'] == test]
        K = test_data['K'].iloc[0]
        N = test_data['N'].iloc[0]
        elements = K * N
        
        print(f"\nТЕСТ {test}: K={K}, N={N:,} ({elements:,} элементов)")
        print("-" * 65)
        print(f"Последовательная версия: {test_data['SeqTime_ms'].iloc[0]:.2f} мс")
        print("\nПараллельные версии:")
        print("Потоки | Время(мс) | Ускорение | Эффективность | Элементов/мс")
        print("-" * 65)
        
        for _, row in test_data.iterrows():
            throughput = elements / row['ParTime_ms']
            print(f"{row['Threads']:6d} | {row['ParTime_ms']:9.2f} | {row['Speedup']:9.2f} | "
                  f"{row['Efficiency']:12.1f}% | {throughput:13.0f}")
    
    # Анализ
    print("\n" + "="*70)
    print("ВЫВОДЫ И АНАЛИЗ:")
    
    # Лучшие результаты
    best_speedup = df.loc[df['Speedup'].idxmax()]
    best_efficiency = df.loc[df['Efficiency'].idxmax()]
    
    print(f"\n1. МАКСИМАЛЬНОЕ УСКОРЕНИЕ: {best_speedup['Speedup']:.2f}x")
    print(f"   Достигнуто при: K={best_speedup['K']}, N={best_speedup['N']:,}, "
          f"{best_speedup['Threads']} потоков")
    print(f"   Эффективность: {best_speedup['Efficiency']:.1f}%")
    
    print(f"\n2. МАКСИМАЛЬНАЯ ЭФФЕКТИВНОСТЬ: {best_efficiency['Efficiency']:.1f}%")
    print(f"   Достигнуто при: K={best_efficiency['K']}, N={best_efficiency['N']:,}, "
          f"{best_efficiency['Threads']} потоков")
    print(f"   Ускорение: {best_efficiency['Speedup']:.2f}x")
    
    print(f"\n3. АНАЛИЗ МАСШТАБИРУЕМОСТИ:")
    for threads in sorted(df['Threads'].unique()):
        subset = df[df['Threads'] == threads]
        avg_eff = subset['Efficiency'].mean()
        avg_speedup = subset['Speedup'].mean()
        print(f"   {threads:2d} потоков: средняя эффективность {avg_eff:.1f}%, "
              f"среднее ускорение {avg_speedup:.2f}x")
    
    print(f"\n4. РЕКОМЕНДАЦИИ:")
    print("   - Для достижения максимальной эффективности используйте 2-8 потоков")
    print("   - Для максимальной производительности используйте 16-32 потока")
    print("   - Эффективность падает при увеличении потоков из-за накладных расходов")

def main():
    """Главная функция"""
    print("Построение графиков для лабораторной работы")
    print("="*50)
    
    try:
        df = load_results()
    except FileNotFoundError:
        print("Ошибка: файл results.csv не найден!")
        print("Сначала запустите программу: ./lab2_final")
        return
    
    # Построение графиков
    plot_speedup(df)
    plot_efficiency(df)
    plot_time_comparison(df)
    
    # Создание отчета
    create_report(df)
    
    print("\n" + "="*50)
    print("ФАЙЛЫ ДЛЯ ОТЧЕТА:")
    print("1. results.csv - таблица с результатами")
    print("2. speedup.png - график ускорения")
    print("3. efficiency.png - график эффективности")
    print("4. time_comparison.png - сравнение времени")
    print("="*50)

if __name__ == "__main__":
    main()