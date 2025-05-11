#ifndef SIMPLE_LOADER_H
#define SIMPLE_LOADER_H
#include "OperationSchedule.h"
#include <vector>
#include <string>

class SimpleLoader {
public:
    int liczbaJobow;
    int liczbaMaszyn;
    std::vector<OperationSchedule> operacje; // Wczytane operacje w nowej strukturze!

    bool load(const std::string& filename);
    void wypisz() const;
};

#endif // SIMPLE_LOADER_H
