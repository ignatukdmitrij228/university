#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>

// Проверка, бьет ли какой-либо ферзь клетку (r, c) 
static int is_attacked(int r, int c, int* qr, int* qc, int n) {
    for (int k = 0; k < n; k++) {
        if (qr[k] == r || qc[k] == c || abs(qr[k] - r) == abs(qc[k] - c)) {
            return 1;
        }
    }
    return 0;
}

// Подсчет свободных клеток 
static int count_free(int n, int* qr, int* qc) {
    int free_cnt = 0;
    for (int r = 0; r < n; r++) {
        for (int c = 0; c < n; c++) {
            if (!is_attacked(r, c, qr, qc, n)) {
                free_cnt++;
            }
        }
    }
    return free_cnt;
}

// Красивый вывод доски с координатной сеткой
static void print_board(int n, int* qr, int* qc) {
    printf("    ");
    for (int c = 0; c < n; c++) printf("%d ", c + 1); // Заголовок столбцов
    printf("\n   ");
    for (int c = 0; c < n; c++) printf("--");
    printf("\n");

    for (int r = 0; r < n; r++) {
        printf("%d | ", r + 1); // Заголовок строки
        for (int c = 0; c < n; c++) {
            int is_q = 0;
            for (int k = 0; k < n; k++) {
                if (qr[k] == r && qc[k] == c) { is_q = 1; break; }
            }
            if (is_q) printf("Q "); // Ферзь
            else if (is_attacked(r, c, qr, qc, n)) printf("* "); // Клетка под боем
            else printf(". "); // Свободная клетка
        }
        printf("\n");
    }
    printf("\nQ - ferz, * - bitaya kletka, . - svobodnaya kletka\n\n");
}

int main() {
    int n;
    printf("Vvedite razmer doski (N): ");
    if (scanf("%d", &n) != 1 || n <= 0 || n > 6) {
        printf("Oshibka (N ot 1 do 6)\n");
        return 1;
    }

    int* qr = (int*)malloc(n * sizeof(int));
    int* qc = (int*)malloc(n * sizeof(int));
    int* best_qr = (int*)malloc(n * sizeof(int));
    int* best_qc = (int*)malloc(n * sizeof(int));

    if (!qr || !qc || !best_qr || !best_qc) { printf("Oshibka (net pamyati)\n"); return 1; }

    int max_free = -1;

    // Общее количество комбинаций (N*N)^N 
    long long total_combos = 1;
    for (int i = 0; i < n; i++) total_combos *= (n * n);

    // ПОЛНЫЙ ПЕРЕБОР 
    for (long long i = 0; i < total_combos; i++) {
        long long temp = i;

        // Разворачиваем номер комбинации в координаты ферзей 
        for (int k = 0; k < n; k++) {
            int pos = temp % (n * n);
            temp /= (n * n);
            qr[k] = pos / n;
            qc[k] = pos % n;
        }

        // Проверка: не стоят ли два ферзя на одной клетке 
        int valid = 1;
        for (int a = 0; a < n; a++) {
            for (int b = a + 1; b < n; b++) {
                if (qr[a] == qr[b] && qc[a] == qc[b]) {
                    valid = 0;
                    break;
                }
            }
            if (!valid) break;
        }

        // Если расстановка корректна (все на разных клетках), считаем свободные поля
        if (valid) {
            int free_cells = count_free(n, qr, qc);
            // Если нашли вариант лучше предыдущего, запоминаем его
            if (free_cells > max_free) {
                max_free = free_cells;
                for (int k = 0; k < n; k++) {
                    best_qr[k] = qr[k];
                    best_qc[k] = qc[k];
                }
            }
        }
    }

    // ВЫВОД РЕЗУЛЬТАТА 
    printf("\nRazmer doski: %dx%d\n", n, n);
    printf("Luchshaya rasstanovka:\n");
    for (int k = 0; k < n; k++) {
        printf("  Ferz %d: (%d, %d)\n", k + 1, best_qr[k] + 1, best_qc[k] + 1);
    }

    printf("\nSvobodnykh poley: %d\n\n", max_free);

    // Отрисовка доски 
    print_board(n, best_qr, best_qc);

    free(qr);
    free(qc);
    free(best_qr);
    free(best_qc);

    return 0;
}