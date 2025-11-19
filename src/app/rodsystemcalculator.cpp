#include "rodsystemcalculator.h"
#include <stdexcept>

RodSystemCalculator::RodSystemCalculator(int num_nodes) : n(num_nodes) {
    F.resize(n, 0.0);
    rods.resize(n - 1);
}

void RodSystemCalculator::setRod(int p, double L, double A, double E, double q,
                                 double sigma_allow) {
    if (p >= 1 && p < n) {
        rods[p - 1] = {L, A, E, q, sigma_allow};
    }
}

void RodSystemCalculator::setForce(int node, double force) {
    if (node >= 1 && node <= n) {
        F[node - 1] = force;
    }
}

#include <iostream>

void RodSystemCalculator::calculate(std::vector<double> &displacements, std::vector<double> &forces,
                                    std::vector<double> &stresses, bool leftAnchor,
                                    bool rightAnchor) {

    std::cout << "RodSystemCalculator::calculate called" << std::endl;
    std::cout << "n (nodes): " << n << ", rods: " << rods.size() << std::endl;
    std::cout << "Anchors - left: " << leftAnchor << ", right: " << rightAnchor << std::endl;

    if (rods.empty()) {
        std::cout << "ERROR: No rods for calculation" << std::endl;
        throw std::runtime_error("Нет стержней для расчета");
    }

    // Проверяем данные стержней
    for (size_t i = 0; i < rods.size(); i++) {
        const Rod &rod = rods[i];
        std::cout << "Rod " << i + 1 << ": L=" << rod.L << " A=" << rod.A << " E=" << rod.E
                  << " q=" << rod.q << std::endl;
    }

    // Инициализация глобальной матрицы жесткости
    std::vector<std::vector<double>> A(n, std::vector<double>(n, 0.0));
    std::vector<double> b(n, 0.0);

    std::cout << "Building stiffness matrix..." << std::endl;

    // Сборка матрицы жесткости
    for (int p = 0; p < n - 1; p++) {
        const Rod &rod = rods[p];
        double k = rod.E * rod.A / rod.L;

        A[p][p] += k;
        A[p][p + 1] += -k;
        A[p + 1][p] += -k;
        A[p + 1][p + 1] += k;

        std::cout << "Stiffness for rod " << p + 1 << ": " << k << std::endl;
    }

    std::cout << "Building load vector..." << std::endl;

    // Сборка вектора нагрузок (сосредоточенные силы)
    for (int i = 0; i < n; i++) {
        b[i] = F[i];
        std::cout << "Node " << i + 1 << " concentrated force: " << F[i] << std::endl;
    }

    // Добавление фиксированных нагрузок от распределенных сил
    for (int p = 0; p < n - 1; p++) {
        const Rod &rod = rods[p];
        double Q = rod.q * rod.L / 2.0;

        b[p] += Q;     // Правильный знак: +Q для левого конца
        b[p + 1] += Q; // Правильный знак: +Q для правого конца

        std::cout << "Rod " << p + 1 << " distributed load: " << rod.q
                  << ", total load: " << (rod.q * rod.L) << ", fixed end forces: " << Q
                  << " at both ends" << std::endl;
    }

    std::cout << "Load vector before BC: [";
    for (int i = 0; i < n; i++) {
        std::cout << b[i] << (i < n - 1 ? ", " : "]");
    }
    std::cout << std::endl;

    std::cout << "Applying boundary conditions..." << std::endl;

    // Применение граничных условий - ПРАВИЛЬНЫЙ СПОСОБ
    if (leftAnchor) {
        std::cout << "Applying left anchor" << std::endl;
        // Для заделки: перемещение = 0, но сила реакции остается
        // Мы уже учли это в векторе b, теперь нужно только обнулить строку/столбец в матрице
        for (int i = 0; i < n; i++) {
            A[0][i] = (i == 0) ? 1.0 : 0.0;
            A[i][0] = (i == 0) ? 1.0 : 0.0;
        }
        b[0] = 0.0; // Перемещение фиксировано = 0
    }

    if (rightAnchor) {
        std::cout << "Applying right anchor" << std::endl;
        for (int i = 0; i < n; i++) {
            A[n - 1][i] = (i == n - 1) ? 1.0 : 0.0;
            A[i][n - 1] = (i == n - 1) ? 1.0 : 0.0;
        }
        b[n - 1] = 0.0; // Перемещение фиксировано = 0
    }

    std::cout << "Solving linear system..." << std::endl;

    // Решение системы уравнений
    displacements = solveLinearSystem(A, b);

    std::cout << "Displacements: [";
    for (int i = 0; i < n; i++) {
        std::cout << displacements[i] << (i < n - 1 ? ", " : "]");
    }
    std::cout << std::endl;

    // Расчет усилий в стержнях - ПРАВИЛЬНАЯ ФОРМУЛА
    forces.resize(n - 1);
    for (int p = 0; p < n - 1; p++) {
        const Rod &rod = rods[p];
        double delta_U = displacements[p + 1] - displacements[p];

        // ПРАВИЛЬНАЯ ФОРМУЛА для усилия в стержне:
        // N = (EA/L) * (u_j - u_i) - (qL/2)
        forces[p] = (rod.E * rod.A / rod.L) * delta_U - (rod.q * rod.L / 2.0);

        std::cout << "Rod " << p + 1 << " delta_U: " << delta_U << ", force: " << forces[p]
                  << std::endl;
    }

    // Расчет напряжений в УЗЛАХ
    stresses.resize(n);

    for (int i = 0; i < n; i++) {
        if (i == 0 && n > 1) {
            // Первый узел - напряжение из первого стержня
            stresses[i] = forces[0] / rods[0].A;
        } else if (i == n - 1 && n > 1) {
            // Последний узел - напряжение из последнего стержня
            stresses[i] = forces[n - 2] / rods[n - 2].A;
        } else if (i > 0 && i < n - 1) {
            // Промежуточный узел - среднее напряжение из двух соседних стержней
            double stress1 = forces[i - 1] / rods[i - 1].A;
            double stress2 = forces[i] / rods[i].A;
            stresses[i] = (stress1 + stress2) / 2.0;
        } else {
            stresses[i] = 0.0;
        }
        std::cout << "Node " << i + 1 << " stress: " << stresses[i] << std::endl;
    }

    std::cout << "RodSystemCalculator::calculate finished successfully" << std::endl;
}

std::vector<double>
RodSystemCalculator::solveLinearSystem(const std::vector<std::vector<double>> &A,
                                       const std::vector<double> &b) {
    int size = b.size();
    if (size == 0)
        return {};

    // Создаем копии для работы
    std::vector<std::vector<double>> augmented(size, std::vector<double>(size + 1, 0.0));
    std::vector<double> result(size, 0.0);

    // Заполняем расширенную матрицу
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            augmented[i][j] = A[i][j];
        }
        augmented[i][size] = b[i];
    }

    std::cout << "Augmented matrix:" << std::endl;
    for (int i = 0; i < size; i++) {
        for (int j = 0; j <= size; j++) {
            std::cout << augmented[i][j] << "\t";
        }
        std::cout << std::endl;
    }

    // Прямой ход метода Гаусса
    for (int i = 0; i < size; i++) {
        // Поиск главного элемента
        int maxRow = i;
        for (int k = i + 1; k < size; k++) {
            if (std::abs(augmented[k][i]) > std::abs(augmented[maxRow][i])) {
                maxRow = k;
            }
        }

        // Перестановка строк
        if (maxRow != i) {
            std::swap(augmented[i], augmented[maxRow]);
        }

        // Нормализация текущей строки
        double pivot = augmented[i][i];
        if (std::abs(pivot) < 1e-15) {
            throw std::runtime_error("Система уравнений вырождена");
        }

        for (int j = i; j <= size; j++) {
            augmented[i][j] /= pivot;
        }

        // Исключение переменной из остальных строк
        for (int k = i + 1; k < size; k++) {
            double factor = augmented[k][i];
            for (int j = i; j <= size; j++) {
                augmented[k][j] -= factor * augmented[i][j];
            }
        }
    }

    // Обратный ход
    for (int i = size - 1; i >= 0; i--) {
        result[i] = augmented[i][size];
        for (int j = i + 1; j < size; j++) {
            result[i] -= augmented[i][j] * result[j];
        }
    }

    return result;
}
