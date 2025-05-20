#include "TabuSearchSolver.h"
#include <iostream>
#include <fstream>
#include <random>
#include <numeric>
#include <algorithm>


// Funkcja sprawdzająca, czy zamiana dwóch operacji w genotypie jest legalna
bool czySwapLegalny(const std::vector<int>& genotyp, const std::vector<OperationSchedule>& operacje, int i, int j)
{
    int idx1 = genotyp[i];
    int idx2 = genotyp[j];

    const OperationSchedule& op1 = operacje[idx1];
    const OperationSchedule& op2 = operacje[idx2];

    // Jeśli operacje są z różnych jobów – swap jest zawsze dozwolony
    if (op1.job_id != op2.job_id)
        return true;

    // Jeśli swap zmienia kolejność operacji w tym samym jobie – jest nielegalny
    return !( (op1.operation_id < op2.operation_id && i > j) ||
              (op2.operation_id < op1.operation_id && j > i) );
}

// Konstruktor klasy – zapisuje parametry algorytmu
TabuSearchSolver::TabuSearchSolver(int liczbaIteracji, int dlugoscTabu)
{
    this->liczbaIteracji = liczbaIteracji;
    this->dlugoscTabu = dlugoscTabu;
    this->makespan = 0;
}

// Funkcja pomocnicza: buduje harmonogram na podstawie priorytetów
std::vector<OperationSchedule> TabuSearchSolver::zbudujHarmonogramZPriorytetami(
    const std::vector<int>& priorytety,
    const std::vector<OperationSchedule>& operacje,
    int liczbaJobow,
    int liczbaMaszyn)
{
    std::vector<OperationSchedule> kopia = operacje;

    // Przypisz priorytety i wyzeruj start/end_time
    for (int i = 0; i < priorytety.size(); ++i)
    {
        kopia[i].priority = priorytety[i];
        kopia[i].start_time = 0;
        kopia[i].end_time = 0;
    }

    // Zbuduj harmonogram na podstawie aktualnych priorytetów
    obliczMakespanOgolny(kopia, &priorytety, true, liczbaJobow, liczbaMaszyn);

    return kopia;
}


// Główna funkcja algorytmu Tabu Search
void TabuSearchSolver::solve(const std::vector<OperationSchedule>& operacje, int liczbaJobow, int liczbaMaszyn)
{
    std::mt19937 gen(std::random_device{}());
    int liczbaOperacji = operacje.size();


    const int liczbaLosowychSasiadow = 600;

    // Inicjalizacja losowych priorytetów
    std::vector<int> priorytety(liczbaOperacji);
    for (int i = 0; i < liczbaOperacji; ++i)
        priorytety[i] = i;
    std::shuffle(priorytety.begin(), priorytety.end(), gen);

    // Tworzymy harmonogram startowy
    std::vector<OperationSchedule> aktualny = zbudujHarmonogramZPriorytetami(priorytety, operacje, liczbaJobow, liczbaMaszyn);
    
    int najlepszyMakespan = obliczMakespanOgolny(aktualny, nullptr, false, liczbaJobow, liczbaMaszyn);
    std::vector<OperationSchedule> najlepszy = aktualny;

    // Inicjalizacja listy tabu
    std::vector<std::pair<int, int>> tabuLista;
    int bezPoprawy = 0;
    const int limitBezPoprawy = 100;
    int liczbaRestartow = 0;
    std::uniform_int_distribution<> dist(0, liczbaOperacji - 1);

    // Główna pętla iteracji
    for (int iter = 0; iter < liczbaIteracji; ++iter)
    {
        //std::cout << "\n[DEBUG] Iteracja " << iter << " | bez poprawy: " << bezPoprawy
                //  << " | obecny makespan: " << najlepszyMakespan << "\n";

        std::vector<int> najlepszySasiadGenotyp;
        int najlepszySasiadKoszt = std::numeric_limits<int>::max();
        int najlepszyI = -1;
        int najlepszyJ = -1;

        // Szukamy najlepszego sąsiada (spośród losowych zamian)
        for (int s = 0; s < liczbaLosowychSasiadow; ++s)
        {
            int i = dist(gen);
            int j = dist(gen);
            if (i == j) continue;
            if (i > j) std::swap(i, j);

            std::pair<int, int> ruch = std::make_pair(i, j);
            std::vector<int> sasiadGenotyp = priorytety;
            std::swap(sasiadGenotyp[i], sasiadGenotyp[j]);

            if (!czySwapLegalny(sasiadGenotyp, operacje, i, j))
                continue;

            std::vector<OperationSchedule> kopia = operacje;
            int koszt = obliczMakespanOgolny(kopia, &sasiadGenotyp, true, liczbaJobow, liczbaMaszyn);

            bool jestNaTabu = (std::find(tabuLista.begin(), tabuLista.end(), ruch) != tabuLista.end());

            if (koszt < najlepszySasiadKoszt)
            {
                najlepszySasiadGenotyp = sasiadGenotyp;
                najlepszySasiadKoszt = koszt;
                najlepszyI = i;
                najlepszyJ = j;
            }
        }

        // Jeśli znaleziono sąsiada – sprawdzamy, czy warto go zaakceptować
            if (!najlepszySasiadGenotyp.empty())
   {priorytety = najlepszySasiadGenotyp;
aktualny = zbudujHarmonogramZPriorytetami(priorytety, operacje, liczbaJobow, liczbaMaszyn);

// TO TU WŁAŚCIWE OBLICZAMY koszt aktualnego rozwiązania
int aktualnyKoszt = obliczMakespanOgolny(aktualny, nullptr, false, liczbaJobow, liczbaMaszyn);



    // Dodaj ruch do listy tabu
    tabuLista.push_back(std::make_pair(najlepszyI, najlepszyJ));
    if (tabuLista.size() > dlugoscTabu)
        tabuLista.erase(tabuLista.begin());

        // Zawsze zapisz aktualny koszt
kosztyIteracji.push_back(aktualnyKoszt);

// Oblicz avg i worst na podstawie dotychczasowych kosztów
int suma = 0;
int najgorszy = aktualnyKoszt;
for (int k = 0; k < kosztyIteracji.size(); ++k)
{
    suma += kosztyIteracji[k];
    if (kosztyIteracji[k] > najgorszy)
        najgorszy = kosztyIteracji[k];
}
double avg = static_cast<double>(suma) / kosztyIteracji.size();
avgIteracji.push_back(avg);
worstIteracji.push_back(najgorszy);

// Jeśli rozwiązanie jest lepsze – zapisz je jako nowe najlepsze
if (najlepszySasiadKoszt < najlepszyMakespan)
{
    najlepszy = aktualny;
    najlepszyMakespan = najlepszySasiadKoszt;
    bezPoprawy = 0;
}
else
{
    bezPoprawy++;
}


    // Zapisz do historii: current i best_so_far
    historiaCurrent.push_back(aktualnyKoszt);
    historiaBestSoFar.push_back(najlepszyMakespan);

    // Restart, jeśli za długo nie ma poprawy
    if (bezPoprawy >= limitBezPoprawy)
    {
        liczbaRestartow++;

        std::shuffle(priorytety.begin(), priorytety.end(), gen);
        aktualny = zbudujHarmonogramZPriorytetami(priorytety, operacje, liczbaJobow, liczbaMaszyn);
        bezPoprawy = 0;
        tabuLista.clear();
    }
}

        else
        {
            std::cerr << "[DEBUG] Brak dostępnych sąsiadów (lub wszystko na tabu).\n";
            break;
        }
    }

    makespan = najlepszyMakespan;
    schedule = najlepszy;
    if (kosztyIteracji.empty())
        kosztyIteracji.push_back(najlepszyMakespan); // zabezpieczenie, jeśli żadna iteracja nie poprawiła

}


int TabuSearchSolver::obliczMakespanOgolny(std::vector<OperationSchedule>& kandydaci,
                                           const std::vector<int>* priorytety,
                                           bool czySortowac,
                                           int liczbaJobow,
                                           int liczbaMaszyn) const
{
    const int N = kandydaci.size();

    // Wyznacz ile operacji ma każdy job
    std::vector<int> liczbaOperacjiNaJob(liczbaJobow, 0);
    for (int i = 0; i < N; ++i)
    {
        const OperationSchedule& op = kandydaci[i];
        if (op.operation_id + 1 > liczbaOperacjiNaJob[op.job_id])
        {
            liczbaOperacjiNaJob[op.job_id] = op.operation_id + 1;
        }
    }

    if (czySortowac && priorytety != nullptr)
    {
        for (int i = 0; i < N; ++i)
        {
            kandydaci[i].priority = (*priorytety)[i];
        }
    }

    // Bucket sort
    std::vector<int> indeksy(N);
    std::vector<int> bucket(N, -1);
    for (int i = 0; i < N; ++i)
    {
        int p = kandydaci[i].priority;
        if (p < 0 || p >= N)
        {
            std::cerr << "[Błąd] Zły priorytet: " << p << " (i=" << i << ")\n";
        }
        bucket[p] = i;
    }
    for (int i = 0; i < N; ++i)
        indeksy[i] = bucket[i];

    // Struktury pomocnicze
    std::vector<int> maszyna_wolna_od(liczbaMaszyn, 0);
    std::vector<int> job_gotowy_od(liczbaJobow, 0);
    std::vector<std::vector<char>> czy_zrobione(liczbaJobow);
    for (int i = 0; i < liczbaJobow; ++i)
        czy_zrobione[i].resize(liczbaOperacjiNaJob[i], 0);

    std::vector<char> zaplanowane(N, 0);
    int zaplanowanych = 0;
    int maksEnd = 0;

    int numerIteracji = 0;

    while (zaplanowanych < N)
    {
        bool dodano = false;

        for (int ii = 0; ii < N; ++ii)
        {
            int i = indeksy[ii];
            if (zaplanowane[i]) continue;

            OperationSchedule& op = kandydaci[i];

            bool poprzedniaOK = (op.operation_id == 0) || (czy_zrobione[op.job_id][op.operation_id - 1]);

            if (!poprzedniaOK)
                continue;

            int start = std::max(maszyna_wolna_od[op.machine_id], job_gotowy_od[op.job_id]);
            int end = start + op.processing_time;

            op.start_time = start;
            op.end_time = end;

            maszyna_wolna_od[op.machine_id] = end;
            job_gotowy_od[op.job_id] = end;
            czy_zrobione[op.job_id][op.operation_id] = 1;
            zaplanowane[i] = 1;
            zaplanowanych++;
            dodano = true;

            // 🔍 Debug: pokaż tylko pierwsze i ostatnie operacje
            /*if (zaplanowanych <= 10 || zaplanowanych > N - 5)
            {
                std::cout << "[DEBUG] #" << zaplanowanych
                          << " job=" << op.job_id
                          << " op=" << op.operation_id
                          << " maszyna=" << op.machine_id
                          << " czas=" << op.processing_time
                          << " start=" << op.start_time
                          << " end=" << op.end_time
                          << "\n";
            }*/

            if (end > maksEnd)
            {
                maksEnd = end;
            }
        }

        if (!dodano)
        {
            std::cerr << "[TS] DEADLOCK – nie udało się zaplanować operacji.\n";
            return std::numeric_limits<int>::max();
        }

        numerIteracji++;
    }

    return maksEnd;
}


// Wypisuje najlepszy harmonogram do konsoli
void TabuSearchSolver::printSchedule() const
{
    std::cout << "\n=== Najlepszy harmonogram (TabuSearchSolver) ===\n";
    std::cout << "Makespan: " << makespan << "\n";
    std::cout << "Operacje:\n";
    std::cout << "Job\tOpID\tMaszyna\tPriory\tStart\tEnd\n";

    for (int i = 0; i < schedule.size(); ++i)
    {
        const OperationSchedule& op = schedule[i];
        std::cout << op.job_id << "\t"
                  << op.operation_id << "\t"
                  << op.machine_id << "\t"
                  << op.priority << "\t"
                  << op.start_time << "\t"
                  << op.end_time << "\n";
    }
}

// Zapisuje harmonogram do pliku CSV
void TabuSearchSolver::zapiszDoCSV(const std::string& nazwaPliku) const
{
    std::ofstream out(nazwaPliku);
    if (!out.is_open())
    {
        std::cerr << "Nie można otworzyć pliku do zapisu: " << nazwaPliku << "\n";
        return;
    }

    // Nagłówek CSV
    out << "job_id,operation_id,machine_id,start_time,end_time,priority\n";

    for (int i = 0; i < schedule.size(); ++i)
    {
        const OperationSchedule& op = schedule[i];
        out << op.job_id << ","
            << op.operation_id << ","
            << op.machine_id << ","
            << op.start_time << ","
            << op.end_time << ","
            << op.priority << "\n";
    }
}

void TabuSearchSolver::zapiszStatystykiDoCSV(const std::string& nazwaPliku, int run) const
{
    if (kosztyIteracji.empty())
    {
        std::cerr << "Brak danych do zapisania statystyk.\n";
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


void TabuSearchSolver::zapiszBestVsCurrentCSV(const std::string& nazwaPliku) const
{
    if (historiaCurrent.empty() || historiaBestSoFar.empty()) {
        std::cerr << "[Tabu] Brak danych do zapisania best vs current.\n";
        return;
    }

    std::ofstream out;
    bool istnieje = std::ifstream(nazwaPliku).good();
    out.open(nazwaPliku, std::ios::app);

    if (!out.is_open()) {
        std::cerr << "[Tabu] Nie można otworzyć pliku: " << nazwaPliku << "\n";
        return;
    }

    if (!istnieje) {
        out << "iter;current;best_so_far\n";
    }

    for (int i = 0; i < historiaCurrent.size(); ++i) {
        out << i << ";" << historiaCurrent[i] << ";" << historiaBestSoFar[i] << "\n";
    }

    out.close();
}


void TabuSearchSolver::zapiszKosztyNajlepszegoRunCSV(const std::string& nazwaPliku) const
{
    if (kosztyIteracji.empty() || avgIteracji.empty() || worstIteracji.empty()) {
        std::cerr << "[Tabu] Brak danych do zapisania kosztów najlepszego runa.\n";
        return;
    }

    std::ofstream out;
    bool istnieje = std::ifstream(nazwaPliku).good();
    out.open(nazwaPliku, std::ios::app);

    if (!out.is_open()) {
        std::cerr << "[Tabu] Nie można otworzyć pliku: " << nazwaPliku << "\n";
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
