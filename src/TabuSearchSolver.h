#ifndef TABU_SEARCH_SOLVER_H
#define TABU_SEARCH_SOLVER_H

#include "OperationSchedule.h"
#include <vector>
#include <map>
#include <string>

class TabuSearchSolver
{
public:
    TabuSearchSolver(int liczbaIteracji, int dlugoscTabu);

    void solve(const std::vector<OperationSchedule>& operacje, int liczbaJobow, int liczbaMaszyn);
    void printSchedule() const;

    void zapiszDoCSV(const std::string& nazwaPliku) const;

   int obliczMakespanOgolny(std::vector<OperationSchedule>& kandydaci, const std::vector<int>* priorytety, bool czySortowac, int liczbaJobow, int liczbaMaszyn) const;

    void zapiszStatystykiDoCSV(const std::string& nazwaPliku, int run) const;

    int getMakespan() const { return makespan; }
const std::vector<OperationSchedule>& getSchedule() const { return schedule; }




private:
    std::vector<OperationSchedule> schedule; // najlepszy znaleziony harmonogram
    int makespan;                            // jego czas trwania
    int liczbaIteracji;
    int dlugoscTabu;
    std::vector<int> kosztyIteracji;

    std::vector<OperationSchedule> zbudujHarmonogramZPriorytetami(
        const std::vector<int>& priorytety,
        const std::vector<OperationSchedule>& operacje,
        int liczbaJobow,
        int liczbaMaszyn
    );

};

#endif
