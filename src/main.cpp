#include "JSSPInstance.h"
#include "SimpleLoader.h"
#include "RandomSolver.h"
#include "TabuSearchSolver.h"
#include "EvolutionarySolver.h"

#include <iostream>
using namespace std;

int main() 
{

    SimpleLoader loader;
    loader.load("LA/toy.data");
    loader.wypisz();

   /* JSSPInstance loader;
    loader.wczytajPlik("Known-Optima/long-js-600000-100-10000-1.data");
    loader.wypiszOperacje();*/
   

    RandomSolver solver;
    solver.solve(loader.operacje, loader.liczbaJobow, loader.liczbaMaszyn);
    solver.printSchedule();
    solver.zapiszDoCSV("harmonogram_random.csv");

    // Parametry Tabu Search
    int liczbaIteracji = 1000;
    int dlugoscTabu = 50;

    // Tworzymy solver
    TabuSearchSolver tabuSolver(liczbaIteracji, dlugoscTabu);

// Uruchamiamy algorytm
    tabuSolver.solve(loader.operacje, loader.liczbaJobow, loader.liczbaMaszyn);
    tabuSolver.printSchedule();
    tabuSolver.zapiszDoCSV("wynik_tabu.csv");

    EvolutionarySolver solverAE(100, 500, 0.8, 0.2); // populacja, pokolenia, prawd. krzyÅ¼., mutacji
    solverAE.solve(loader.operacje, loader.liczbaJobow, loader.liczbaMaszyn);
    solverAE.printSchedule();
    solverAE.zapiszDoCSV("harmonogram_evolution.csv");

    return 0;
}
