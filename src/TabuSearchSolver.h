#ifndef TABU_SEARCH_SOLVER_H
#define TABU_SEARCH_SOLVER_H

#include "OperationSchedule.h"
#include <vector>
#include <map>
#include <string>

class TabuSearchSolver
{
public:
    TabuSearchSolver(int liczbaIteracji = 1000, int dlugoscTabu = 50);

    void solve(const std::vector<OperationSchedule>& operacje, int liczbaJobow, int liczbaMaszyn);
    void printSchedule() const;
    int getMakespan() const;
    void zapiszDoCSV(const std::string& nazwaPliku) const;

private:
    std::vector<OperationSchedule> schedule; // najlepszy znaleziony harmonogram
    int makespan;                            // jego czas trwania
    int liczbaIteracji;
    int dlugoscTabu;

    // pomocnicze:
    std::vector<int> generujLosowyGenotyp(int liczbaJobow, int liczbaMaszyn);
    int obliczMakespan(std::vector<OperationSchedule>& harmonogram) const;
};

#endif
