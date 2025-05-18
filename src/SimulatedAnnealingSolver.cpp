#include "SimulatedAnnealingSolver.h"
#include <iostream>
#include <fstream>
#include <random>
#include <algorithm>
#include <numeric>
#include <cmath> // dla exp()

// Konstruktor: ustawiamy wszystkie parametry
SimulatedAnnealingSolver::SimulatedAnnealingSolver(double startTemp, double endTemp, double coolingRate, int maxIter)
{
    temperaturaStartowa = startTemp; temperaturaKoncowa = endTemp; wspolczynnikChlodzenia = coolingRate; maksLiczbaIteracji = maxIter;
    makespan = 0;
}

std::vector<OperationSchedule> SimulatedAnnealingSolver::zbudujHarmonogramZPriorytetami(
    const std::vector<int>& priorytety,
    const std::vector<OperationSchedule>& operacje,
    int liczbaJobow,
    int liczbaMaszyn)
{
    std::vector<OperationSchedule> kandydaci = operacje;
    for (int i = 0; i < kandydaci.size(); ++i)
    {
        kandydaci[i].priority = priorytety[i];
    }

    std::sort(kandydaci.begin(), kandydaci.end(), porownajPoPriorytecie);

    // Wylicz ile operacji ma każdy job
    std::vector<int> liczbaOperacjiNaJob(liczbaJobow, 0);
    for (int i = 0; i < kandydaci.size(); ++i)
    {
        const OperationSchedule& op = kandydaci[i];
        if (op.operation_id + 1 > liczbaOperacjiNaJob[op.job_id])
        {
            liczbaOperacjiNaJob[op.job_id] = op.operation_id + 1;
        }
    }

    std::vector<std::vector<bool> > czy_zrobione(liczbaJobow);
    for (int i = 0; i < liczbaJobow; ++i)
    {
        czy_zrobione[i].resize(liczbaOperacjiNaJob[i], false);
    }

    std::vector<int> maszyna_wolna_od(liczbaMaszyn, 0);
    std::vector<int> job_gotowy_od(liczbaJobow, 0);
    std::vector<OperationSchedule> harmonogram;

    int liczbaZaplanowanych = 0;
    int N = kandydaci.size();

    while (liczbaZaplanowanych < N)
    {
        bool dodano = false;

        for (int i = 0; i < N; ++i)
        {
            OperationSchedule& op = kandydaci[i];

            if (czy_zrobione[op.job_id][op.operation_id]) continue;

            bool poprzedniaOK = (op.operation_id == 0) || czy_zrobione[op.job_id][op.operation_id - 1];

            if (poprzedniaOK)
            {
                int start = (maszyna_wolna_od[op.machine_id] > job_gotowy_od[op.job_id])
                          ? maszyna_wolna_od[op.machine_id]
                          : job_gotowy_od[op.job_id];

                int end = start + op.processing_time;

                op.start_time = start;
                op.end_time = end;

                maszyna_wolna_od[op.machine_id] = end;
                job_gotowy_od[op.job_id] = end;
                czy_zrobione[op.job_id][op.operation_id] = true;

                harmonogram.push_back(op);
                liczbaZaplanowanych++;
                dodano = true;
            }
        }

        if (!dodano)
        {
            std::cerr << "[SA] Błąd: Deadlock – nie da się zaplanować żadnej operacji\n";
            break;
        }
    }

    return harmonogram;
}



void SimulatedAnnealingSolver::solve(const std::vector<OperationSchedule>& operacje, int liczbaJobow, int liczbaMaszyn)
{
    std::mt19937 gen(std::random_device{}());
    int liczbaOperacji = operacje.size();

    // === Krok 1: Losowe rozwiązanie początkowe ===
    std::vector<int> aktualnyGenotyp(liczbaOperacji);
    for (int i = 0; i < liczbaOperacji; ++i)
        aktualnyGenotyp[i] = i;

    std::shuffle(aktualnyGenotyp.begin(), aktualnyGenotyp.end(), gen);
    std::vector<OperationSchedule> aktualnyHarmonogram = zbudujHarmonogramZPriorytetami(aktualnyGenotyp, operacje, liczbaJobow, liczbaMaszyn);
    int aktualnyKoszt = obliczMakespan(aktualnyHarmonogram, aktualnyGenotyp, liczbaJobow, liczbaMaszyn);

    std::vector<int> najlepszyGenotyp = aktualnyGenotyp;
    std::vector<OperationSchedule> najlepszyHarmonogram = aktualnyHarmonogram;
    int najlepszyKoszt = aktualnyKoszt;

    // === Krok 2: Parametry SA ===
    double T = temperaturaStartowa;
    int iteracja = 0;

    std::uniform_int_distribution<> dist(0, liczbaOperacji - 1);

    while (T > temperaturaKoncowa && iteracja < maksLiczbaIteracji)
    {
        // === Krok 3: Tworzymy sąsiada przez zamianę dwóch operacji ===
        std::vector<int> nowyGenotyp = aktualnyGenotyp;
        int i = dist(gen);
        int j = dist(gen);
        while (i == j) j = dist(gen); // upewnij się, że różne indeksy

        std::swap(nowyGenotyp[i], nowyGenotyp[j]);

        // === Krok 4: Obliczamy koszt sąsiada ===
        std::vector<OperationSchedule> nowyHarmonogram = zbudujHarmonogramZPriorytetami(nowyGenotyp, operacje, liczbaJobow, liczbaMaszyn);
        int nowyKoszt = obliczMakespan(nowyHarmonogram, nowyGenotyp, liczbaJobow, liczbaMaszyn);

        // === Krok 5: Różnica kosztów ===
        int delta = nowyKoszt - aktualnyKoszt;

        // === Krok 6: Czy zaakceptować nowego? ===
        if (delta < 0 || (std::exp(-delta / T) > ((double)rand() / RAND_MAX)))
        {
            aktualnyGenotyp = nowyGenotyp;
            aktualnyHarmonogram = nowyHarmonogram;
            aktualnyKoszt = nowyKoszt;
        }

        // === Krok 7: Aktualizacja najlepszego ===
        if (aktualnyKoszt < najlepszyKoszt)
        {
            najlepszyKoszt = aktualnyKoszt;
            najlepszyGenotyp = aktualnyGenotyp;
            najlepszyHarmonogram = aktualnyHarmonogram;
        }

        // === Krok 8: Chłodzenie ===
        kosztyIteracji.push_back(aktualnyKoszt);
        T *= wspolczynnikChlodzenia;
        iteracja++;
    }

    // Zapisz najlepsze rozwiązanie
    schedule = najlepszyHarmonogram;
    makespan = najlepszyKoszt;
}


int SimulatedAnnealingSolver::obliczMakespan(
    std::vector<OperationSchedule>& kandydaci,
    const std::vector<int>& priorytety,
    int liczbaJobow,
    int liczbaMaszyn)
{
    for (int i = 0; i < kandydaci.size(); ++i)
    {
        kandydaci[i].priority = priorytety[i];
    }

    std::sort(kandydaci.begin(), kandydaci.end(), porownajPoPriorytecie);

    std::vector<int> liczbaOperacjiNaJob(liczbaJobow, 0);
    for (int i = 0; i < kandydaci.size(); ++i)
    {
        const OperationSchedule& op = kandydaci[i];
        if (op.operation_id + 1 > liczbaOperacjiNaJob[op.job_id])
        {
            liczbaOperacjiNaJob[op.job_id] = op.operation_id + 1;
        }
    }

    std::vector<std::vector<bool> > czy_zrobione(liczbaJobow);
    for (int i = 0; i < liczbaJobow; ++i)
    {
        czy_zrobione[i].resize(liczbaOperacjiNaJob[i], false);
    }

    std::vector<int> maszyna_wolna_od(liczbaMaszyn, 0);
    std::vector<int> job_gotowy_od(liczbaJobow, 0);

    int maksCzas = 0;
    int zaplanowane = 0;
    int N = kandydaci.size();

    while (zaplanowane < N)
    {
        bool cosDodano = false;

        for (int i = 0; i < N; ++i)
        {
            OperationSchedule& op = kandydaci[i];

            if (czy_zrobione[op.job_id][op.operation_id]) continue;

            bool poprzedniaOK = (op.operation_id == 0) || czy_zrobione[op.job_id][op.operation_id - 1];

            if (poprzedniaOK)
            {
                int start = (maszyna_wolna_od[op.machine_id] > job_gotowy_od[op.job_id])
                          ? maszyna_wolna_od[op.machine_id]
                          : job_gotowy_od[op.job_id];

                int end = start + op.processing_time;

                op.start_time = start;
                op.end_time = end;

                maszyna_wolna_od[op.machine_id] = end;
                job_gotowy_od[op.job_id] = end;
                czy_zrobione[op.job_id][op.operation_id] = true;

                if (end > maksCzas)
                {
                    maksCzas = end;
                }

                zaplanowane++;
                cosDodano = true;
            }
        }

        if (!cosDodano)
        {
            std::cerr << "[SA] Błąd: Deadlock – nie da się zaplanować żadnej operacji\n";
            return std::numeric_limits<int>::max();
        }
    }

    return maksCzas;
}





// Wypisuje harmonogram (tak jak w innych solverach)
void SimulatedAnnealingSolver::printSchedule() const
{
    std::cout << "\n=== Najlepszy harmonogram (SimulatedAnnealingSolver) ===\n";
    std::cout << "Makespan: " << makespan << "\n";
    std::cout << "Operacje:\n";
    std::cout << "Job\tOpID\tMaszyna\tPriory\tStart\tEnd\n";

    for (const auto& op : schedule)
    {
        std::cout << op.job_id << "\t"
                  << op.operation_id << "\t"
                  << op.machine_id << "\t"
                  << op.priority << "\t"
                  << op.start_time << "\t"
                  << op.end_time << "\n";
    }
}

// Zapis do CSV
void SimulatedAnnealingSolver::zapiszDoCSV(const std::string& nazwaPliku) const
{
    std::ofstream out(nazwaPliku);
    if (!out.is_open())
    {
        std::cerr << "Nie można otworzyć pliku do zapisu: " << nazwaPliku << "\n";
        return;
    }

    out << "job_id,operation_id,machine_id,start_time,end_time,priority\n";
    for (const auto& op : schedule)
    {
        out << op.job_id << ","
            << op.operation_id << ","
            << op.machine_id << ","
            << op.start_time << ","
            << op.end_time << ","
            << op.priority << "\n";
    }
}

void SimulatedAnnealingSolver::zapiszStatystykiDoCSV(const std::string& nazwaPliku, int run) const
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
