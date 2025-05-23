#include "RandomSolver.h"
#include "OperationSchedule.h"
#include <iostream>
#include <fstream>
#include <random>
#include <algorithm>
#include <numeric> // std::accumulate
#include <fstream>


RandomSolver::RandomSolver(int liczbaProb)
    : liczbaProb(liczbaProb), makespan(0)
{}


void RandomSolver::solve(const std::vector<OperationSchedule>& operacje, int liczbaJobow, int liczbaMaszyn)
{
    std::mt19937 gen(std::random_device{}());

    // Ustawiamy początkowy makespan na bardzo dużą wartość (żeby każdy harmonogram był lepszy)
    makespan = std::numeric_limits<int>::max();

    // Czyścimy wektor kosztów prób (dla statystyk)
    kosztyProb.clear();

    // Liczba wszystkich operacji we wszystkich jobach
    int liczbaOperacji = operacje.size();

    // Wykonujemy wiele prób (losowych harmonogramów)
    for (int prob = 0; prob < liczbaProb; ++prob)
    {

        // === KROK 1: Skopiuj operacje i nadaj im losowe, unikalne priorytety ===
        std::vector<OperationSchedule> kandydaci = operacje;

        // Tworzymy wektor liczb 0,1,2,...,N-1
        std::vector<int> priorytety(liczbaOperacji);
        for (int i = 0; i < liczbaOperacji; ++i)
        {
            priorytety[i] = i;
        }

        // Tasujemy priorytety, aby każda operacja dostała inny, losowy numer
        std::shuffle(priorytety.begin(), priorytety.end(), gen);

        // Przypisujemy każdej operacji losowy priorytet oraz zerujemy czasy
        for (int i = 0; i < liczbaOperacji; ++i)
        {
            kandydaci[i].priority = priorytety[i];
            kandydaci[i].start_time = 0;
            kandydaci[i].end_time = 0;
        }
        /* std::cout << "Przypisane priorytety:\n";
    for (const auto& op : kandydaci)
    {
        std::cout << "Job " << op.job_id << ", Op " << op.operation_id 
                  << ", Maszyna " << op.machine_id << ", Priorytet: " << op.priority << "\n";
    }*/
        // === KROK 2: Sortujemy operacje po priorytecie rosnąco ===
        std::vector<int> indeksy(liczbaOperacji);
        for (int i = 0; i < liczbaOperacji; ++i)
        {
            indeksy[i] = i;
        }
        
        //Sortowanie kubełkowe
        std::sort(indeksy.begin(), indeksy.end(), PorownywaczIndeksowPoPriorytecie(&kandydaci));

        // === KROK 3: Tworzymy harmonogram, trzymając ograniczenia technologiczne ===
        std::vector<OperationSchedule> harmonogram;

        std::vector<int> maszyna_wolna_od(liczbaMaszyn, 0);
        std::vector<int> job_gotowy_od(liczbaJobow, 0);
        std::vector<bool> czy_zrobione(liczbaOperacji, false);

        // Dopóki nie zaplanujemy wszystkich operacji
        while (harmonogram.size() < kandydaci.size())
        {
            bool dodano = false;

            for (int ii = 0; ii < indeksy.size(); ++ii)
            {
                int i = indeksy[ii]; // ← Używamy wcześniej posortowanych indeksów!
                OperationSchedule& op = kandydaci[i];

                if (czy_zrobione[i]) continue; // już zaplanowana

                bool poprzedniaWykonana = false;

                if (op.operation_id == 0)
                {
                    poprzedniaWykonana = true;
                }
                else
                {

                // szukamy indeksu poprzedniej operacji w kandydaci
                for (int j = 0; j < kandydaci.size(); ++j)
                {
                    if (kandydaci[j].job_id == op.job_id && kandydaci[j].operation_id == op.operation_id - 1 && czy_zrobione[j])
                    {
                            poprzedniaWykonana = true;
                            break;
                    }
                }
                }

                if (poprzedniaWykonana)
                {
                    int czasMaszyny = maszyna_wolna_od[op.machine_id]; //od kiedy maszyna, na której ma się odbyć ta operacja, jest dostępna.
                    int czasJoba = job_gotowy_od[op.job_id]; //od kiedy job może kontynuować (czyli kiedy skończył poprzednią operację).
                    int czasStart = (czasMaszyny > czasJoba) ? czasMaszyny : czasJoba;

                    op.start_time = czasStart;
                    op.end_time = czasStart + op.processing_time;

                    maszyna_wolna_od[op.machine_id] = op.end_time; //ta maszyna będzie znów wolna dopiero po tej operacji
                    job_gotowy_od[op.job_id] = op.end_time; // job będzie gotowy do kolejnej operacji też dopiero wtedy


                    czy_zrobione[i] = true;
                    harmonogram.push_back(op);
                    dodano = true;
                }
            }


              /*    std::cout << "Zaplanowane Job " << op.job_id << ", Op " << op.operation_id
                          << " na maszynie " << op.machine_id
                          << " od " << op.start_time << " do " << op.end_time << "\n";
           */ }


            if (!dodano)
            {
                std::cerr << "Błąd: Nie udało się zaplanować żadnej operacji.\n";
                break;
            }
        }

        // === KROK 4: Liczymy makespan – koniec najpóźniejszej operacji ===
        int wynik = 0;
        for (int i = 0; i < harmonogram.size(); ++i)
        {
            if (harmonogram[i].end_time > wynik)
            {
                wynik = harmonogram[i].end_time;
            }
        }

           // std::cout << "Makespan tej próby: " << wynik << "\n";


        // === KROK 5: Jeśli ten harmonogram jest najlepszy dotąd – zapamiętaj go ===
        if (wynik < makespan)
        {
            makespan = wynik;
            schedule = harmonogram;
                   // std::cout << "=> Nowy najlepszy harmonogram (makespan = " << makespan << ")\n";

        }

        // Zapisz wynik tej próby (dla statystyk)
        kosztyProb.push_back(wynik);
    }
}


void RandomSolver::printSchedule() const
{
    // Nagłówek – informacja o harmonogramie
    std::cout << "\n=== Najlepszy harmonogram (RandomSolver) ===\n";
    std::cout << "Makespan: " << makespan << "\n";
    std::cout << "Operacje:\n";
    std::cout << "Job\tOpID\tMaszyna\tPriory\tStart\tEnd\n";

    // Iterujemy po wszystkich operacjach w zapisanym harmonogramie
    for (int i = 0; i < schedule.size(); ++i)
    {
        const OperationSchedule& op = schedule[i]; // jawnie deklarujemy typ

        // Wypisujemy dane jednej operacji w formie tabeli
        std::cout << op.job_id << "\t"
                  << op.operation_id << "\t"
                  << op.machine_id << "\t"
                  << op.priority << "\t"
                  << op.start_time << "\t"
                  << op.end_time << "\n";
    }
}

void RandomSolver::zapiszDoCSV(const std::string& nazwaPliku) const
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

void RandomSolver::zapiszStatystykiDoCSV(const std::string& nazwaPliku, int run) const
{
    if (kosztyProb.empty()) 
    {
        std::cerr << "Brak danych do zapisania statystyk.\n";
        return;
    }

    double best = *std::min_element(kosztyProb.begin(), kosztyProb.end());
    double worst = *std::max_element(kosztyProb.begin(), kosztyProb.end());
    double avg = std::accumulate(kosztyProb.begin(), kosztyProb.end(), 0.0) / kosztyProb.size();

    std::ofstream out;
    bool istnieje = std::ifstream(nazwaPliku).good();
    out.open(nazwaPliku, std::ios::app); // dopisujemy


    if (!out.is_open()) {
        std::cerr << "Nie można otworzyć pliku do zapisu: " << nazwaPliku << "\n";
        return;
    }


    if (!istnieje) {
        out << "run;best;average;worst\n"; // nagłówek tylko jeśli plik nie istniał
    }

    out << run << ";" << best << ";" << avg << ";" << worst << "\n";
    out.close();
}

