#include "JSSPInstance.h"
#include "SimpleLoader.h"
#include "RandomSolver.h"
#include "GreedySolver.h"

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


    GreedySolver greedy;
    greedy.solve(loader.operacje, loader.liczbaJobow, loader.liczbaMaszyn);
    greedy.printSchedule();
    greedy.zapiszDoCSV("harmonogram_greedy.csv");


    return 0;
}
