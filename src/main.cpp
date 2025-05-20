#include "JSSPInstance.h"
#include "SimpleLoader.h"
#include "RandomSolver.h"
#include "TabuSearchSolver.h"
#include "SimulatedAnnealingSolver.h"
#include "EvolutionarySolver.h"
#include <chrono>
#include <iostream>
using namespace std;

int main() 
{
   SimpleLoader loader;

   //loader.load("C:\\Users\\MICHA~1\\Desktop\\opt2\\jssp2\\Optymalizacja-JSSP\\ta\\ta01.txt");
    loader.load("ta\\ta01.txt");
    //loader.wypisz(); 
    

    
   //JSSPInstance loader;
   //loader.wczytajPlik("C:\\Users\\MICHA~1\\Desktop\\opt2\\jssp2\\Optymalizacja-JSSP\\Known-Optima\\short-js-600000-100-10000-1.data");
   //loader.wczytajPlik("Known-Optima\\short-js-600000-100-10000-1.data");
    //loader.wypiszOperacje();
   

    auto startwsio = std::chrono::high_resolution_clock::now();
    
    int liczbaUruchomien = 2;

    /**/
    // === RANDOM SOLVER===
    auto startRand = std::chrono::high_resolution_clock::now();

    int randIteracji = 1000;
    int najlepszyRun = -1;
    int najlepszyKoszt = std::numeric_limits<int>::max();

    for (int run = 0; run < liczbaUruchomien; ++run)
    {
        RandomSolver solver(randIteracji);
        solver.solve(loader.operacje, loader.liczbaJobow, loader.liczbaMaszyn);

        // Zapisz statystyki z tego runa do pliku CSV
        solver.zapiszStatystykiDoCSV("wyniki_random.csv", run);

        // Jeśli koszt lepszy niż dotychczas – zapamiętaj numer i wartość
        if (solver.getMakespan() < najlepszyKoszt)
        {
            najlepszyKoszt = solver.getMakespan();
            najlepszyRun = run;
            solver.zapiszDoCSV("harmonogram_random.csv"); // tylko najlepszy
        }
    }
    std::cout << "Najlepszy RANDOM run: #" << najlepszyRun << "\n";
    std::cout << "Koszt (makespan): " << najlepszyKoszt << "\n";

    auto stopRand = std::chrono::high_resolution_clock::now();
   std::chrono::duration<double> elapsedRandom = stopRand - startRand;
   std::cout << "[RandomSolver] Czas wykonania: " << elapsedRandom.count() << " sekund\n";
    
    // === TABU SEARCH ===
    auto startTS = std::chrono::high_resolution_clock::now();

    int tabuIteracje = 1000;
    int dlugoscTabu = 100;
    int najlepszyRunTS = -1;
    int najlepszyKosztTS = std::numeric_limits<int>::max();
    TabuSearchSolver najlepszyTabu(tabuIteracje, dlugoscTabu);

    for (int run = 0; run < liczbaUruchomien; ++run)
    {
        TabuSearchSolver solver(tabuIteracje, dlugoscTabu);
        solver.solve(loader.operacje, loader.liczbaJobow, loader.liczbaMaszyn);

        // Zapis statystyk z tego runa
        solver.zapiszStatystykiDoCSV("wyniki_tabu.csv", run);

        // Jeśli lepszy niż dotąd – zapamiętaj
        if (solver.getMakespan() < najlepszyKosztTS)
        {
            najlepszyKosztTS = solver.getMakespan();
            najlepszyRunTS = run;
            najlepszyTabu = solver;
        }
    }

    // Zapisz tylko najlepszy harmonogram
    najlepszyTabu.zapiszDoCSV("harmonogram_tabu.csv");
    najlepszyTabu.zapiszBestVsCurrentCSV("best_vs_current_tabu.csv");
    najlepszyTabu.zapiszKosztyNajlepszegoRunCSV("koszty_tabu.csv");



    std::cout << "Najlepszy TABU run: #" << najlepszyRunTS << "\n";
    std::cout << "Koszt (makespan): " << najlepszyKosztTS << "\n";

    auto stopTS = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsedTS = stopTS - startTS;
    std::cout << "[TabuSearch] Czas wykonania: " << elapsedTS.count() << " sekund\n";

    

    // === SIMULATED ANNEALING ===
auto startSA = std::chrono::high_resolution_clock::now();

double startTemp = 1000.0;
double endTemp = 0.01;
double coolingRate = 0.800;
int maxIter = 10000;

int najlepszyRunSA = -1;
int najlepszyKosztSA = std::numeric_limits<int>::max();
SimulatedAnnealingSolver najlepszySA(startTemp, endTemp, coolingRate, maxIter);

for (int run = 0; run < liczbaUruchomien; ++run)
{
    SimulatedAnnealingSolver solver(startTemp, endTemp, coolingRate, maxIter);
    solver.solve(loader.operacje, loader.liczbaJobow, loader.liczbaMaszyn);

    solver.zapiszStatystykiDoCSV("wyniki_sa.csv", run);

    if (solver.getMakespan() < najlepszyKosztSA)
    {
        najlepszyKosztSA = solver.getMakespan();
        najlepszyRunSA = run;
        najlepszySA = solver;
    }
}

// Zapisz tylko harmonogram najlepszego rozwiązania
najlepszySA.zapiszDoCSV("harmonogram_sa.csv");
najlepszySA.zapiszKosztyNajlepszegoRunCSV("koszty_sa.csv");
najlepszySA.zapiszBestVsCurrentCSV("best_vs_current_sa.csv");



std::cout << "Najlepszy SA run: #" << najlepszyRunSA << "\n";
std::cout << "Koszt (makespan): " << najlepszyKosztSA << "\n";

auto stopSA = std::chrono::high_resolution_clock::now();
std::chrono::duration<double> elapsedSA = stopSA - startSA;
std::cout << "[SimulatedAnnealing] Czas wykonania: " << elapsedSA.count() << " sekund\n";



/*
    EvolutionarySolver solverAE(100, 500, 0.8, 0.2); // populacja, pokolenia, prawd. krzyÅ¼., mutacji
    solverAE.solve(loader.operacje, loader.liczbaJobow, loader.liczbaMaszyn);
    solverAE.printSchedule();
    solverAE.zapiszDoCSV("harmonogram_evolution.csv");
*/

   

auto stopwsio = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsedwsio = stopwsio - startwsio;
    std::cout << "[Caly Algorytm] Czas wykonania: " << elapsedwsio.count() << " sekund\n";



    return 0;
}
