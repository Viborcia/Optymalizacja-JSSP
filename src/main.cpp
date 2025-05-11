#include "JSSPInstance.h"
#include "SimpleLoader.h"
#include "RandomSolver.h"

#include <iostream>
using namespace std;

int main() 
{

    SimpleLoader loader;
    loader.load("ta/ta01.txt");
    loader.wypisz();

   /* JSSPInstance loader;
    loader.wczytajPlik("Known-Optima/long-js-600000-100-10000-1.data");
    loader.wypiszOperacje();*/
   

    RandomSolver solver;
    solver.solve(loader.operacje, loader.liczbaJobow, loader.liczbaMaszyn);
    solver.printSchedule();
    solver.zapiszDoCSV("harmonogram_random.csv");

    return 0;
}
