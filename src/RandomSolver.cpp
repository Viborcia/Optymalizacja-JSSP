#include "RandomSolver.h"
#include <iostream>
#include <fstream>
#include <random>
#include <map>
#include <algorithm>

RandomSolver::RandomSolver(int liczbaProb)
    : liczbaProb(liczbaProb), makespan(0)
{}

void RandomSolver::solve(const std::vector<OperationSchedule>& operacje, int liczbaJobow, int liczbaMaszyn)
{
    // Tworzymy generator liczb losowych
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

        // === KROK 2: Sortujemy operacje po priorytecie rosnąco ===

        for (int i = 0; i < liczbaOperacji - 1; ++i)
        {
            for (int j = 0; j < liczbaOperacji - i - 1; ++j)
            {
                if (kandydaci[j].priority > kandydaci[j + 1].priority)
                {
                    OperationSchedule temp = kandydaci[j];
                    kandydaci[j] = kandydaci[j + 1];
                    kandydaci[j + 1] = temp;
                }
            }
        }

        // === KROK 3: Tworzymy harmonogram, trzymając ograniczenia technologiczne ===

        std::vector<OperationSchedule> harmonogram; // wynikowa lista operacji w kolejności wykonywania

        std::map<int, int> dostepMaszyny; // kiedy dana maszyna jest dostępna (np. maszyna 1 wolna od czasu 10)
        std::map<int, int> dostepJoba;    // kiedy dany job może wykonywać kolejną operację
        std::map<std::pair<int, int>, bool> wykonane; // które operacje już zostały wykonane

        // Pętla działa, aż wszystkie operacje zostaną zaplanowane
        while (harmonogram.size() < kandydaci.size())
        {
            bool dodano = false; // czy w tej iteracji udało się dodać operację?

            // Iterujemy po wszystkich operacjach w kolejności priorytetu
            for (int i = 0; i < kandydaci.size(); ++i)
            {
                OperationSchedule& op = kandydaci[i];
                std::pair<int, int> klucz = std::make_pair(op.job_id, op.operation_id);

                // Jeśli ta operacja już została zaplanowana, to pomijamy
                if (wykonane[klucz]) continue;

                // Sprawdzamy, czy poprzednia operacja z tego joba została wykonana
                bool poprzedniaWykonana = false;
                if (op.operation_id == 0)
                {
                    poprzedniaWykonana = true; // jeśli to pierwsza operacja w jobie – można ją wykonać
                }
                else
                {
                    std::pair<int, int> poprzednia = std::make_pair(op.job_id, op.operation_id - 1);
                    if (wykonane[poprzednia]) poprzedniaWykonana = true;
                }

                // Jeśli możemy ją wykonać – harmonogramujemy
                if (poprzedniaWykonana)
                {
                    int czasMaszyny = dostepMaszyny[op.machine_id]; // od kiedy maszyna wolna
                    int czasJoba = dostepJoba[op.job_id];           // od kiedy job gotowy

                    // start_time to max(z obu), żeby nie kolidować
                    int czasStart = (czasMaszyny > czasJoba) ? czasMaszyny : czasJoba;

                    op.start_time = czasStart;
                    op.end_time = czasStart + op.processing_time;

                    // Aktualizujemy dostępność maszyny i joba
                    dostepMaszyny[op.machine_id] = op.end_time;
                    dostepJoba[op.job_id] = op.end_time;

                    // Zaznaczamy, że ta operacja została wykonana
                    wykonane[klucz] = true;

                    // Dodajemy operację do harmonogramu
                    harmonogram.push_back(op);
                    dodano = true;
                }
            }

            // Jeśli nie udało się zaplanować nic – błąd
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

        // === KROK 5: Jeśli ten harmonogram jest najlepszy dotąd – zapamiętaj go ===
        if (wynik < makespan)
        {
            makespan = wynik;
            schedule = harmonogram;
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
