#ifndef EVOLUTIONARY_SOLVER_H
#define EVOLUTIONARY_SOLVER_H

#include "OperationSchedule.h"
#include <vector>
#include <string>

class EvolutionarySolver {
public:
    EvolutionarySolver(int populacja = 100, int pokolen = 500, double pKrzyzowania = 0.8, double pMutacji = 0.2);

    void solve(const std::vector<OperationSchedule>& operacje, int liczbaJobow, int liczbaMaszyn);
    void printSchedule() const;
    void zapiszDoCSV(const std::string& nazwaPliku) const;

private:
    struct Individual 
    {
        std::vector<int> genotyp; // lista indeksów operacji
        int fitness;              // makespan
        std::vector<OperationSchedule> harmonogram;
    };

      Individual dekoduj(const std::vector<int>& genotyp,
                       const std::vector<OperationSchedule>& operacje,
                       int liczbaJobow,
                       int liczbaMaszyn);

    Individual turniej(const std::vector<Individual>& populacja);

    void krzyzowanieOX(const std::vector<int>& rodzic1,
                       const std::vector<int>& rodzic2,
                       std::vector<int>& potomek1,
                       std::vector<int>& potomek2);

    void mutacja(std::vector<int>& genotyp);

    std::vector<OperationSchedule> schedule;
    int makespan;
    int liczbaJobow;
    int liczbaMaszyn;
    std::vector<OperationSchedule> operacjeRef;

    int rozmiarPopulacji;
    int liczbaPokolen;
    double prawdopKrzyzowania;
    double prawdopMutacji;
};

#endif
