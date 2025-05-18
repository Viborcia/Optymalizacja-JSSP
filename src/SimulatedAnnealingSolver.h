#ifndef SIMULATED_ANNEALING_SOLVER_H
#define SIMULATED_ANNEALING_SOLVER_H

#include "OperationSchedule.h"
#include <vector>
#include <string>

class SimulatedAnnealingSolver
{
public:
    // Konstruktor z parametrami algorytmu
    SimulatedAnnealingSolver(double startTemp, double endTemp, double coolingRate, int maxIter);

    // Główna funkcja uruchamiająca algorytm
    void solve(const std::vector<OperationSchedule>& operacje, int liczbaJobow, int liczbaMaszyn);

    // Wypisanie najlepszego harmonogramu
    void printSchedule() const;

    // Zapis harmonogramu do pliku CSV
    void zapiszDoCSV(const std::string& nazwaPliku) const;
    void zapiszStatystykiDoCSV(const std::string& nazwaPliku, int run) const;

    int getMakespan() const { return makespan; }
    const std::vector<OperationSchedule>& getSchedule() const { return schedule; }



private:
    // Najlepszy znaleziony harmonogram i jego koszt (makespan)
    std::vector<OperationSchedule> schedule;
    int makespan;

    // Parametry algorytmu SA
    double temperaturaStartowa;
    double temperaturaKoncowa;
    double wspolczynnikChlodzenia;
    int maksLiczbaIteracji;
    std::vector<int> kosztyIteracji; // historia kosztów


    // Pomocnicze do obliczeń
    int obliczMakespan(std::vector<OperationSchedule>& kandydaci, const std::vector<int>& priorytety, int liczbaJobow, int liczbaMaszyn);
    std::vector<OperationSchedule> zbudujHarmonogramZPriorytetami(const std::vector<int>& priorytety, const std::vector<OperationSchedule>& operacje, int liczbaJobow, int liczbaMaszyn);
};

#endif
