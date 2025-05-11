#ifndef JSSP_INSTANCE_H
#define JSSP_INSTANCE_H

#include <vector>
#include <string>
#include "OperationSchedule.h"

class JSSPInstance {
public:
    int liczbaJobow;
    int liczbaMaszyn;
    std::vector<OperationSchedule> operacje;

    // Wczytuje instancje z pliku .data
    bool wczytajPlik(const std::string& sciezka);

    // Wypisuje operacje (do testow)
    void wypiszOperacje() const;
};

#endif // JSSP_INSTANCE_H
