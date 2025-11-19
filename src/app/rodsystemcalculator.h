#ifndef RODSYSTEMCALCULATOR_H
#define RODSYSTEMCALCULATOR_H

#include <cmath>
#include <vector>

class RodSystemCalculator {
private:
    struct Rod {
        double L;           // Длина стержня
        double A;           // Площадь поперечного сечения
        double E;           // Модуль упругости
        double q;           // Распределенная нагрузка
        double sigma_allow; // Допустимое напряжение
    };

    std::vector<Rod> rods;
    std::vector<double> F; // Сосредоточенные силы в узлах
    int n;                 // Количество узлов

    std::vector<double> solveLinearSystem(
        const std::vector<std::vector<double>> &A,
        const std::vector<double> &b);

public:
    RodSystemCalculator(int num_nodes);

    void setRod(int p, double L, double A, double E, double q,
                double sigma_allow);
    void setForce(int node, double force);
    void calculate(std::vector<double> & displacements,
                   std::vector<double> & forces, std::vector<double> & stresses,
                   bool leftAnchor, bool rightAnchor);

    // Геттеры для получения данных о стержнях
    int getNodeCount() const { return n; }
    int getRodCount() const { return n - 1; }
    double getRodLength(int index) const {
        return (index >= 0 && index < rods.size()) ? rods[index].L : 0.0;
    }
    double getRodArea(int index) const {
        return (index >= 0 && index < rods.size()) ? rods[index].A : 0.0;
    }
};

#endif // RODSYSTEMCALCULATOR_H
