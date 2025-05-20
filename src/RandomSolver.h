#ifndef RANDOM_SOLVER_H
#define RANDOM_SOLVER_H

#include "OperationSchedule.h"
#include <vector>
#include <string>


class RandomSolver {
public:
    RandomSolver(int liczbaProb);

    void solve(const std::vector<OperationSchedule>& operacje, int liczbaJobow, int liczbaMaszyn);
    void printSchedule() const;
    void zapiszDoCSV(const std::string& nazwaPliku) const;
    void zapiszStatystykiDoCSV(const std::string& nazwaPliku, int run) const;

    int getMakespan() const { return makespan; }


    void zapiszMakespanDoCSV(const std::string &nazwaPliku) const;

private:
    std::vector<OperationSchedule> schedule;
    int makespan;
    int liczbaProb;
    std::vector<double> kosztyProb;
};

#endif 
