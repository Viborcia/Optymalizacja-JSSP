#include "EvolutionarySolver.h"
#include <algorithm>
#include <random>
#include <iostream>
#include <map>
#include <fstream>
#include <queue>      
#include <stdexcept> 


EvolutionarySolver::EvolutionarySolver(int populacja, int pokolen, double pKrzyzowania, double pMutacji)
    : rozmiarPopulacji(populacja), liczbaPokolen(pokolen), prawdopKrzyzowania(pKrzyzowania), prawdopMutacji(pMutacji)
{}

EvolutionarySolver::Individual EvolutionarySolver::turniej(const std::vector<Individual>& populacja) {
    std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<> dist(0, populacja.size() - 1);

    int a = dist(gen);
    int b = dist(gen);

    return (populacja[a].fitness < populacja[b].fitness) ? populacja[a] : populacja[b];
}

void EvolutionarySolver::krzyzowanieOX(const std::vector<int>& rodzic1,
                                       const std::vector<int>& rodzic2,
                                       std::vector<int>& potomek1,
                                       std::vector<int>& potomek2) {
    int size = rodzic1.size();
    std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<> dist(0, size - 1);

    int start = dist(gen);
    int end = dist(gen);
    if (start > end) std::swap(start, end);

    potomek1 = std::vector<int>(size, -1);
    potomek2 = std::vector<int>(size, -1);

    // Kopiujemy segment
    for (int i = start; i <= end; ++i) {
        potomek1[i] = rodzic1[i];
        potomek2[i] = rodzic2[i];
    }

    // Wypełniamy resztę potomek1
    int index = (end + 1) % size;
    for (int i = 0; i < size; ++i) {
        int kandydat = rodzic2[(end + 1 + i) % size];
        if (std::find(potomek1.begin(), potomek1.end(), kandydat) == potomek1.end()) {
            potomek1[index] = kandydat;
            index = (index + 1) % size;
        }
    }

    // Wypełniamy resztę potomek2
    index = (end + 1) % size;
    for (int i = 0; i < size; ++i) {
        int kandydat = rodzic1[(end + 1 + i) % size];
        if (std::find(potomek2.begin(), potomek2.end(), kandydat) == potomek2.end()) {
            potomek2[index] = kandydat;
            index = (index + 1) % size;
        }
    }
}

void EvolutionarySolver::mutacja(std::vector<int>& genotyp) {
    std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<> dist(0, genotyp.size() - 1);

    int a = dist(gen);
    int b = dist(gen);
    std::swap(genotyp[a], genotyp[b]);
}


void EvolutionarySolver::solve(const std::vector<OperationSchedule>& operacje, int lJobow, int lMaszyn)
{
    liczbaJobow = lJobow;
    liczbaMaszyn = lMaszyn;
    operacjeRef = operacje;

    std::mt19937 gen(std::random_device{}());
    int liczbaOperacji = operacje.size();

    // === Inicjalizacja populacji ===
    std::vector<Individual> populacja;

    for (int i = 0; i < rozmiarPopulacji; ++i)
    {
        std::vector<int> genotyp(liczbaOperacji);
        for (int j = 0; j < liczbaOperacji; ++j)
            genotyp[j] = j;

        std::shuffle(genotyp.begin(), genotyp.end(), gen);

        Individual chromo = dekoduj(genotyp, operacje, liczbaJobow, liczbaMaszyn);
        populacja.push_back(chromo);
    }

    // === Ewolucja ===
    for (int pok = 0; pok < liczbaPokolen; ++pok)
    {
        std::vector<Individual> nowaPopulacja;

        while (nowaPopulacja.size() < rozmiarPopulacji)
        {
            Individual rodzic1 = turniej(populacja);
            Individual rodzic2 = turniej(populacja);

            std::vector<int> potomek1 = rodzic1.genotyp;
            std::vector<int> potomek2 = rodzic2.genotyp;

            double losKrzyz = (double)rand() / RAND_MAX;
            if (losKrzyz < prawdopKrzyzowania)
            {
                krzyzowanieOX(rodzic1.genotyp, rodzic2.genotyp, potomek1, potomek2);
            }

            double losMut1 = (double)rand() / RAND_MAX;
            if (losMut1 < prawdopMutacji)
                mutacja(potomek1);

            double losMut2 = (double)rand() / RAND_MAX;
            if (losMut2 < prawdopMutacji)
                mutacja(potomek2);

            nowaPopulacja.push_back(dekoduj(potomek1, operacje, liczbaJobow, liczbaMaszyn));
            if (nowaPopulacja.size() < rozmiarPopulacji)
                nowaPopulacja.push_back(dekoduj(potomek2, operacje, liczbaJobow, liczbaMaszyn));
        }

        populacja = nowaPopulacja;
    }

    // === Najlepszy osobnik ===
    Individual best = populacja[0];
    for (int i = 1; i < populacja.size(); ++i)
    {
        if (populacja[i].fitness < best.fitness)
        {
            best = populacja[i];
        }
    }

    makespan = best.fitness;
    schedule = best.harmonogram;
}

// w tym miejscu kopiujemy pełny harmonogram, a nie tylko goły genotyp
EvolutionarySolver::Individual EvolutionarySolver::dekoduj(
    const std::vector<int>& genotyp,
    const std::vector<OperationSchedule>& operacje,
    int liczbaJobow,
    int liczbaMaszyn
)
{
    EvolutionarySolver::Individual osobnik;
    osobnik.genotyp = genotyp;

    // Ustaw priorytety operacjom według genotypu
    std::vector<OperationSchedule> operacjeZPriorytetami = operacje;
    for (int i = 0; i < genotyp.size(); ++i)
    {
        operacjeZPriorytetami[i].priority = genotyp[i];
    }

    // Sortujemy operacje rosnąco po priorytecie
    for (int i = 0; i < operacjeZPriorytetami.size() - 1; ++i)
    {
        for (int j = 0; j < operacjeZPriorytetami.size() - i - 1; ++j)
        {
            if (operacjeZPriorytetami[j].priority > operacjeZPriorytetami[j + 1].priority)
            {
                OperationSchedule temp = operacjeZPriorytetami[j];
                operacjeZPriorytetami[j] = operacjeZPriorytetami[j + 1];
                operacjeZPriorytetami[j + 1] = temp;
            }
        }
    }

    // Przygotowanie struktur pomocniczych
    std::vector<OperationSchedule> harmonogram;
    std::map<int, int> dostepMaszyny;
    std::map<int, int> dostepJoba;
    std::map<std::pair<int, int>, bool> wykonane;

    int wykonanych = 0;
    while (wykonanych < operacjeZPriorytetami.size())
    {
        bool cosDodano = false;

        for (int i = 0; i < operacjeZPriorytetami.size(); ++i)
        {
            OperationSchedule& op = operacjeZPriorytetami[i];
            std::pair<int, int> klucz = std::make_pair(op.job_id, op.operation_id);

            if (wykonane[klucz]) continue;

            bool moznaWykonac = false;
            if (op.operation_id == 0)
            {
                moznaWykonac = true;
            }
            else
            {
                std::pair<int, int> poprzednia = std::make_pair(op.job_id, op.operation_id - 1);
                if (wykonane[poprzednia]) moznaWykonac = true;
            }

            if (moznaWykonac)
            {
                int czasStart = (dostepMaszyny[op.machine_id] > dostepJoba[op.job_id])
                              ? dostepMaszyny[op.machine_id]
                              : dostepJoba[op.job_id];

                op.start_time = czasStart;
                op.end_time = czasStart + op.processing_time;

                dostepMaszyny[op.machine_id] = op.end_time;
                dostepJoba[op.job_id] = op.end_time;
                wykonane[klucz] = true;

                harmonogram.push_back(op);
                wykonanych++;
                cosDodano = true;
            }
        }

        if (!cosDodano)
        {
            throw std::runtime_error("Błąd: Nie można zbudować poprawnego harmonogramu na podstawie priorytetów.");
        }
    }

    // Liczymy makespan
    int maksCzas = 0;
    for (int i = 0; i < harmonogram.size(); ++i)
    {
        if (harmonogram[i].end_time > maksCzas)
        {
            maksCzas = harmonogram[i].end_time;
        }
    }

    osobnik.fitness = maksCzas;
    osobnik.harmonogram = harmonogram;
    return osobnik;
}



void EvolutionarySolver::printSchedule() const {
    std::cout << "\n=== Najlepszy harmonogram (EvolutionarySolver) ===\n";
    std::cout << "Makespan: " << makespan << "\n";
    std::cout << "Operacje:\n";
    std::cout << "Job\tOpID\tMaszyna\tPriory\tStart\tEnd\n";
    for (const auto& op : schedule) {
        std::cout << op.job_id << "\t"
                  << op.operation_id << "\t"
                  << op.machine_id << "\t"
                  << op.priority << "\t"
                  << op.start_time << "\t"
                  << op.end_time << "\n";
    }
}

void EvolutionarySolver::zapiszDoCSV(const std::string& nazwaPliku) const
{
    // Tworzymy obiekt plikowy do zapisu
    std::ofstream out(nazwaPliku);

    // Sprawdzamy, czy plik otworzył się poprawnie
    if (!out.is_open()) {
        std::cerr << "Nie można otworzyć pliku do zapisu: " << nazwaPliku << "\n";
        return; // kończymy funkcję, jeśli nie udało się otworzyć pliku
    }

    // Nagłówek pliku CSV
    out << "job_id,operation_id,machine_id,start_time,end_time\n";

    // Iterujemy po wszystkich operacjach i zapisujemy ich dane do pliku
    for (int i = 0; i < schedule.size(); ++i)
    {
        const OperationSchedule& op = schedule[i];

        // Wypisujemy dane w formacie CSV (czyli oddzielone przecinkami)
        out << op.job_id << ","
            << op.operation_id << ","
            << op.machine_id << ","
            << op.start_time << ","
            << op.end_time << "\n";
    }

    // Plik zostanie zamknięty automatycznie po zakończeniu funkcji (RAII)
}

