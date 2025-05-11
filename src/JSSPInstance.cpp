#include "JSSPInstance.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>

bool JSSPInstance::wczytajPlik(const std::string& sciezka) {
    std::ifstream file(sciezka);
    if (!file.is_open()) {
        std::cerr << "Nie mozna otworzyc pliku: " << sciezka << "\n";
        return false;
    }

    file >> liczbaJobow >> liczbaMaszyn; //zczytywanie job i maszyn 

    std::string line;
    std::getline(file, line); // usuwamy znak nowej linii po liczbach

    int jobID = 0;
    while (jobID < liczbaJobow && std::getline(file, line)) { //kazda linia to job
        if (line.empty()) continue;

        std::istringstream iss(line);
        int maszyna, czas;
        int operationID = 0;

        while (iss >> maszyna >> czas) 
        {
            if (maszyna == -1 && czas == -1) 
            {
                break;
            }

            OperationSchedule op;
            op.job_id = jobID;
            op.operation_id = operationID;
            op.machine_id = maszyna;
            op.processing_time = czas;
            op.start_time = 0;
            op.end_time = 0;

            operacje.push_back(op);
            ++operationID;
        }
        ++jobID;
    }

    return true;
}

void JSSPInstance::wypiszOperacje() const {
    std::cout << "Liczba Jobow: " << liczbaJobow << ", Liczba Maszyn: " << liczbaMaszyn << "\n";
    for (int i = 0; i < operacje.size(); ++i) 
    {
        const OperationSchedule& op = operacje[i];
        std::cout << "Job " << op.job_id
                  << ", Operacja " << op.operation_id
                  << ", Maszyna " << op.machine_id
                  << ", Czas: " << op.processing_time << "\n";
    }
}
