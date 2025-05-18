#include "JSSPInstance.h"
#include "SimpleLoader.h"
#include "RandomSolver.h"
#include "TabuSearchSolver.h"
#include "EvolutionarySolver.h"
#include "EvolutionSolver.h"


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
    solver.zapiszMakespanDoCSV("makespan_random.csv");

  /* // Parametry Tabu Search
    int liczbaIteracji = 1000;
    int dlugoscTabu = 50;

    // Tworzymy solver
    TabuSearchSolver tabuSolver(liczbaIteracji, dlugoscTabu);

// Uruchamiamy algorytm
    tabuSolver.solve(loader.operacje, loader.liczbaJobow, loader.liczbaMaszyn);
    tabuSolver.printSchedule();
    tabuSolver.zapiszDoCSV("harmonogram_tabu.csv");*/

   EvolutionSolver solverAE(10, 20, 0.01, 0.7, 3); // populacja, pokolenia, mutacja, krzyżowanie, turniej
    solverAE.solve(loader.operacje, loader.liczbaJobow, loader.liczbaMaszyn);
    solverAE.printSchedule();
   // solverAE.zapiszDoCSV("harmonogram_evolution.csv");

    
    return 0;
}
