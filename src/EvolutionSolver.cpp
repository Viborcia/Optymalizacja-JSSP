#include "EvolutionSolver.h"
#include <random>
#include <algorithm>
#include <limits>
#include <iostream>
#include <numeric>
#include <fstream>
#include <utility>

EvolutionSolver::EvolutionSolver(int populacja, int pokolenia, double prawdopodobMutacji, double prawdopodobKrzyzowania, int tourSize)
    : rozmiarPopulacji(populacja),
      liczbaPokolen(pokolenia),
      prawdopodobienstwoMutacji(prawdopodobMutacji),
      prawdopodobienstwoKrzyzowania(prawdopodobKrzyzowania),
      tourSize(tourSize),
      najlepszyMakespan(std::numeric_limits<int>::max()),
      liczbaJobow(0),
      liczbaMaszyn(0) {}

void EvolutionSolver::solve(const std::vector<OperationSchedule>& operacje, int lj, int lm) {
    std::mt19937 gen(std::random_device{}());
    std::ofstream plik("ewolucyjny.csv");
    plik << "Pokolenie;Populacja;Fitness;Priorytety\n";

    liczbaJobow = lj;
    liczbaMaszyn = lm;
    int liczbaOperacji = operacje.size();

    populacja.clear();
    kosztyIteracji.clear();
    avgIteracji.clear();
    worstIteracji.clear();

    for (int i = 0; i < rozmiarPopulacji; ++i) {
        Individual individual = stworzLosowyOsobnik(liczbaOperacji);
        individual.fitness = ocenOsobnik(individual, operacje);
        populacja.push_back(individual);
    }

    for (int epoka = 0; epoka < liczbaPokolen; ++epoka) {
        std::cout << "\n=== Epoka " << epoka + 1 << " ===\n";

        std::vector<Individual> nowaPopulacja;

        while ((int)nowaPopulacja.size() < rozmiarPopulacji) {
            Individual r1 = turniej(populacja, tourSize, gen);
            Individual r2 = turniej(populacja, tourSize, gen);

            std::uniform_real_distribution<> disProb(0.0, 1.0);
            double probabilityCrossover = disProb(gen);
            Individual child1 = r1;
            Individual child2 = r2;

            if (probabilityCrossover < prawdopodobienstwoKrzyzowania) {
                std::cout<<"Krzyżujemy "<<"\n";
                std::pair<Individual, Individual> children = krzyzowanieOX(r1, r2, gen);
                child1 = children.first;
                child2 = children.second;
            }

            double probabilityMutation = disProb(gen);
            if (probabilityMutation < prawdopodobienstwoMutacji){
                std::cout << "MUTUJEMY 1 ";
                for (int p : child1.priorytety) {
                    std::cout << p << " ";
                }
                std::cout << "\n";
                mutacjaSwap(child1);
            } 
            if (probabilityMutation < prawdopodobienstwoMutacji){
                std::cout << "MUTUJEMY 2 ";
                for (int p : child2.priorytety) {
                    std::cout << p << " ";
                }
                std::cout << "\n";
                mutacjaSwap(child2); 
            } 

            child1.fitness = ocenOsobnik(child1, operacje);
            child2.fitness = ocenOsobnik(child2, operacje);

            nowaPopulacja.push_back(child1);
            nowaPopulacja.push_back(child2);
        }

        // Zapis i wyświetlenie populacji
        for (size_t i = 0; i < nowaPopulacja.size(); ++i) {
            const Individual& ind = nowaPopulacja[i];

            // CSV
            plik << epoka + 1 << ";" << i << ";" << ind.fitness << ";";
            for (size_t j = 0; j < ind.priorytety.size(); ++j) {
                plik << ind.priorytety[j];
                if (j < ind.priorytety.size() - 1) plik << "-";
            }
            plik << "\n";

            // Konsola
            std::cout << "Osobnik " << i << ": Fitness = " << ind.fitness << ", Priorytety = ";
            for (int p : ind.priorytety) std::cout << p << " ";
            std::cout << "\n";
        }

        populacja = nowaPopulacja;

        int best = std::numeric_limits<int>::max();
        int worst = 0;
        int sum = 0;

        for (const auto& individual : populacja) {
            if (individual.fitness < najlepszyMakespan) {
                najlepszyMakespan = individual.fitness;
                najlepszyHarmonogram = budujHarmonogram(individual, operacje);
                std::cout << "Nowy najlepszy makespan: " << najlepszyMakespan
                          << " (epoka " << epoka + 1 << ")\n";
            }
            best = std::min(best, individual.fitness);
            worst = std::max(worst, individual.fitness);
            sum += individual.fitness;
        }

        int avg = sum / populacja.size();

        kosztyIteracji.push_back(best);
        avgIteracji.push_back(avg);
        worstIteracji.push_back(worst);

        std::cout << "Statystyki epoki " << epoka + 1 << " — Best: " << best 
                  << ", Avg: " << avg << ", Worst: " << worst << "\n";
    }

    std::cout << "\n=== Zakończono działanie algorytmu ===\n";
    std::cout << "Najlepszy znaleziony makespan: " << najlepszyMakespan << "\n";
}

EvolutionSolver::Individual EvolutionSolver::stworzLosowyOsobnik(int liczbaOperacji) {
    Individual individual;
    individual.priorytety.resize(liczbaOperacji);
    //std::iota(individual.priorytety.begin(), individual.priorytety.end(), 0);
    for (int i = 0; i < (int)individual.priorytety.size(); ++i) {
    individual.priorytety[i] = i;
}
    std::mt19937 gen(std::random_device{}());
    std::shuffle(individual.priorytety.begin(), individual.priorytety.end(), gen);
    return individual;
}

int EvolutionSolver::ocenOsobnik(Individual& individual, const std::vector<OperationSchedule>& operacje) {
    std::vector<OperationSchedule> kandydujace = operacje;
    for (int i = 0; i < (int)individual.priorytety.size(); ++i) {
        kandydujace[i].priority = individual.priorytety[i];
    }

    auto harmonogram = budujHarmonogram(individual, kandydujace);
    int maks = 0;
    for (const auto& o : harmonogram) {
        if (o.end_time > maks) maks = o.end_time;
    }
    return maks;
}

EvolutionSolver::Individual EvolutionSolver::turniej(const std::vector<Individual>& populacja, int tourSize, std::mt19937& gen) {
    std::uniform_int_distribution<> dist(0, (int)populacja.size() - 1);
    Individual best = populacja[dist(gen)];

    for (int i = 1; i < tourSize; ++i) {
        Individual kandydat = populacja[dist(gen)];
        if (kandydat.fitness < best.fitness)
            best = kandydat;
    }

    return best;
}

void EvolutionSolver::mutacjaSwap(Individual& individual) {
    std::mt19937 gen(std::random_device{}());
    std::uniform_real_distribution<> probDist(0.0, 1.0);
    std::uniform_int_distribution<> geneDist(0, (int)individual.priorytety.size() - 1);

    for (int i = 0; i < (int)individual.priorytety.size(); ++i) {
        if (probDist(gen) < prawdopodobienstwoMutacji) {
            int j = geneDist(gen);
            if (i != j) {
                std::swap(individual.priorytety[i], individual.priorytety[j]);
            }
        }
    }
}


std::pair<EvolutionSolver::Individual, EvolutionSolver::Individual>
EvolutionSolver::krzyzowanieOX(const Individual& p1, const Individual& p2, std::mt19937& gen) {
    int n = (int)p1.priorytety.size();
    std::uniform_int_distribution<> dist(0, n - 1);
    int start = dist(gen), end = dist(gen);
    if (start > end) std::swap(start, end);

   // std::cout << "Rozmiar: " << n << "\n";
   // std::cout << "Zakres krzyżowania: start=" << start << ", end=" << end << "\n";
   // std::cout << "Rodzic 1: ";
   // for (int x : p1.priorytety) std::cout << x << " ";
   // std::cout << "\nRodzic 2: ";
   // for (int x : p2.priorytety) std::cout << x << " ";
   // std::cout << "\n";

    // Dziecko 1: segment od p1, reszta z p2
    Individual child1;
    child1.priorytety = std::vector<int>(n, -1);
    for (int i = start; i <= end; ++i)
        child1.priorytety[i] = p1.priorytety[i];
    
  //  std::cout << "Child1 po skopiowaniu segmentu: ";
   // for (int x : child1.priorytety) std::cout << x << " ";
   // std::cout << "\n";

    int index1 = (end + 1) % n;
    for (int i = 0; i < n; ++i) {
        int val = p2.priorytety[(end + 1 + i) % n];
        if (std::find(child1.priorytety.begin(), child1.priorytety.end(), val) == child1.priorytety.end()) {
            child1.priorytety[index1] = val;
           // std::cout << "Dodano " << val << " do child1 na pozycję " << index1 << "\n";
            index1 = (index1 + 1) % n;
        }
    }

    //std::cout << "Child1 końcowy: ";
   // for (int x : child1.priorytety) std::cout << x << " ";
   // std::cout << "\n";

    // Dziecko 2: segment od p2, reszta z p1
    Individual child2;
    child2.priorytety = std::vector<int>(n, -1);
    for (int i = start; i <= end; ++i)
        child2.priorytety[i] = p2.priorytety[i];

   // std::cout << "Child2 po skopiowaniu segmentu: ";
   // for (int x : child2.priorytety) std::cout << x << " ";
   // std::cout << "\n";   
        
    int index2 = (end + 1) % n;
    for (int i = 0; i < n; ++i) {
        int val = p1.priorytety[(end + 1 + i) % n];
        if (std::find(child2.priorytety.begin(), child2.priorytety.end(), val) == child2.priorytety.end()) {
            child2.priorytety[index2] = val;
      //      std::cout << "Dodano " << val << " do child2 na pozycję " << index2 << "\n";
            index2 = (index2 + 1) % n;
        }
    }

   // std::cout << "Child2 końcowy: ";
   // for (int x : child2.priorytety) std::cout << x << " ";
   // std::cout << "\n";

    return std::make_pair(child1, child2);
}

std::vector<OperationSchedule> EvolutionSolver::budujHarmonogram(const Individual& ch, const std::vector<OperationSchedule>& operacje) {
    std::vector<OperationSchedule> kandydaci = operacje;
    for (int i = 0; i < (int)ch.priorytety.size(); ++i) {
        kandydaci[i].priority = ch.priorytety[i];
        kandydaci[i].start_time = 0;
        kandydaci[i].end_time = 0;
    }

    std::sort(kandydaci.begin(), kandydaci.end(), [](const auto& a, const auto& b) {
        return a.priority < b.priority;
    });

    std::vector<OperationSchedule> harmonogram;
    std::vector<int> maszyna_wolna_od(liczbaMaszyn, 0);
    std::vector<int> job_gotowy_od(liczbaJobow, 0);
    std::vector<bool> czy_zrobione(kandydaci.size(), false);

    while ((int)harmonogram.size() < (int)kandydaci.size()) {
        for (int i = 0; i < (int)kandydaci.size(); ++i) {
            if (czy_zrobione[i]) continue;
            auto& op = kandydaci[i];

            bool poprzedniaWykonana = (op.operation_id == 0);
            if (!poprzedniaWykonana) {
                for (int j = 0; j < (int)kandydaci.size(); ++j) {
                    if (kandydaci[j].job_id == op.job_id &&
                        kandydaci[j].operation_id == op.operation_id - 1 &&
                        czy_zrobione[j]) {
                        poprzedniaWykonana = true;
                        break;
                    }
                }
            }

            if (poprzedniaWykonana) {
                int start = std::max(maszyna_wolna_od[op.machine_id], job_gotowy_od[op.job_id]);
                op.start_time = start;
                op.end_time = start + op.processing_time;

                maszyna_wolna_od[op.machine_id] = op.end_time;
                job_gotowy_od[op.job_id] = op.end_time;

                czy_zrobione[i] = true;
                harmonogram.push_back(op);
            }
        }
    }

    return harmonogram;
}

void EvolutionSolver::printSchedule() const {
    std::cout << "\n=== Najlepszy harmonogram (EvolutionSolver) ===\n";
    std::cout << "Makespan: " << najlepszyMakespan << "\n";
    std::cout << "Job\tOpID\tMaszyna\tStart\tEnd\n";
    for (const auto& op : najlepszyHarmonogram) {
        std::cout << op.job_id << "\t"
                  << op.operation_id << "\t"
                  << op.machine_id << "\t"
                  << op.start_time << "\t"
                  << op.end_time << "\n";
    }
}

void EvolutionSolver::zapiszDoCSV(const std::string& nazwaPliku) const
{
    // Tworzymy obiekt plikowy do zapisu
    std::ofstream out(nazwaPliku);

    // Sprawdzamy, czy plik otworzył się poprawnie
    if (!out.is_open()) {
        std::cerr << "Nie można otworzyć pliku do zapisu: " << nazwaPliku << "\n";
        return; // kończymy funkcję, jeśli nie udało się otworzyć pliku
    }

    // Nagłówek pliku CSV
    const std::string naglowek = "job_id,operation_id,machine_id,start_time,end_time\n";
    out << naglowek;
    std::cout << naglowek;  // <-- wypisanie nagłówka na konsolę

    // Iterujemy po wszystkich operacjach i zapisujemy ich dane do pliku oraz na konsolę
    for (const auto& op : najlepszyHarmonogram)
    {
        std::string linia =
            std::to_string(op.job_id) + "," +
            std::to_string(op.operation_id) + "," +
            std::to_string(op.machine_id) + "," +
            std::to_string(op.start_time) + "," +
            std::to_string(op.end_time) + "\n";

        out << linia;
        std::cout << linia;  // <-- wypisanie na konsolę
    }

    // Plik zostanie zamknięty automatycznie po zakończeniu funkcji (RAII)
}

void EvolutionSolver::zapiszStatystykiDoCSV(const std::string& nazwaPliku, int run) const
{
    if (kosztyIteracji.empty())
    {
        std::cerr << "Brak danych do zapisania statystyk (kosztyIteracji).\n";
        return;
    }

    double best = *std::min_element(kosztyIteracji.begin(), kosztyIteracji.end());
    double worst = *std::max_element(kosztyIteracji.begin(), kosztyIteracji.end());
    double avg = std::accumulate(kosztyIteracji.begin(), kosztyIteracji.end(), 0.0) / kosztyIteracji.size();

    double sumKw = 0.0;
    for (int koszt : kosztyIteracji)
    {
        double roznica = koszt - avg;
        sumKw += roznica * roznica;
    }
    double stddev = std::sqrt(sumKw / kosztyIteracji.size());

    std::ofstream out;
    bool istnieje = std::ifstream(nazwaPliku).good();
    out.open(nazwaPliku, std::ios::app);

    if (!out.is_open())
    {
        std::cerr << "Nie można otworzyć pliku do zapisu: " << nazwaPliku << "\n";
        return;
    }

    if (!istnieje)
    {
        out << "run;best;average;worst;std\n";
    }

    out << run << ";" << best << ";" << avg << ";" << worst << ";" << stddev << "\n";
    out.close();
}


void EvolutionSolver::zapiszKosztyNajlepszegoRunCSV(const std::string& nazwaPliku) const
{
    if (kosztyIteracji.empty() || avgIteracji.empty() || worstIteracji.empty()) {
        std::cerr << "[EA] Brak danych do zapisania kosztów najlepszego runa.\n";
        return;
    }

    std::ofstream out;
    bool istnieje = std::ifstream(nazwaPliku).good();
    out.open(nazwaPliku, std::ios::app);

    if (!out.is_open()) {
        std::cerr << "[EA] Nie można otworzyć pliku: " << nazwaPliku << "\n";
        return;
    }

    if (!istnieje) {
        out << "iter;best;avg;worst\n";
    }

    for (int i = 0; i < kosztyIteracji.size(); ++i) {
        out << i << ";" 
            << kosztyIteracji[i] << ";" 
            << avgIteracji[i] << ";" 
            << worstIteracji[i] << "\n";
    }

    out.close();
}
