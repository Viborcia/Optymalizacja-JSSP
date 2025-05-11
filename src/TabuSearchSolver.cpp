#include "TabuSearchSolver.h"
#include <iostream>
#include <fstream>
#include <random>
#include <algorithm>

// Konstruktor – zapamiętujemy liczbę iteracji i długość listy tabu
TabuSearchSolver::TabuSearchSolver(int liczbaIteracji, int dlugoscTabu)
{
    this->liczbaIteracji = liczbaIteracji;
    this->dlugoscTabu = dlugoscTabu;
    this->makespan = 0;
}

// Główna funkcja rozwiązująca JSSP metodą tabu search
void TabuSearchSolver::solve(const std::vector<OperationSchedule>& operacje, int liczbaJobow, int liczbaMaszyn)
{
    std::mt19937 gen(std::random_device{}());

    // Punkt startowy – bierzemy permutację operacji i losujemy priorytety
    std::vector<OperationSchedule> aktualny = operacje;
    int liczbaOperacji = aktualny.size();

    std::vector<int> priorytety(liczbaOperacji);
    for (int i = 0; i < liczbaOperacji; ++i)
    {
        priorytety[i] = i;
    }

    std::shuffle(priorytety.begin(), priorytety.end(), gen);

    for (int i = 0; i < liczbaOperacji; ++i)
    {
        aktualny[i].priority = priorytety[i];
        aktualny[i].start_time = 0;
        aktualny[i].end_time = 0;
    }

    // Sortujemy startowy harmonogram według priorytetu (będziemy to powtarzać później)
    for (int i = 0; i < liczbaOperacji - 1; ++i)
    {
        for (int j = 0; j < liczbaOperacji - i - 1; ++j)
        {
            if (aktualny[j].priority > aktualny[j + 1].priority)
            {
                OperationSchedule temp = aktualny[j];
                aktualny[j] = aktualny[j + 1];
                aktualny[j + 1] = temp;
            }
        }
    }

        // === KROK 4: Oblicz makespan startowego harmonogramu ===

    int najlepszyMakespan = obliczMakespan(aktualny); // funkcja licząca makespan
    std::vector<OperationSchedule> najlepszy = aktualny; // zapamiętujemy jako najlepszy dotąd

    // === KROK 5: Przygotuj listę tabu ===
    std::vector<std::pair<int, int> > tabuLista; // lista zakazanych ruchów (zamian)

    // === KROK 6: Pętla główna Tabu Search ===

    for (int iter = 0; iter < liczbaIteracji; ++iter)
    {
        int najlepszySasiadKoszt = std::numeric_limits<int>::max();
        std::vector<OperationSchedule> najlepszySasiad;
        int najlepszyI = -1;
        int najlepszyJ = -1;

        for (int i = 0; i < aktualny.size() - 1; ++i)
{
    for (int j = i + 1; j < aktualny.size(); ++j)
    {
        // Zamieniaj tylko operacje na tej samej maszynie
        if (aktualny[i].machine_id != aktualny[j].machine_id)
            continue;

        // Sprawdź czy ruch (i,j) nie jest tabu
        std::pair<int, int> ruch = std::make_pair(i, j);
        bool tabu = false;

        for (int k = 0; k < tabuLista.size(); ++k)
        {
            if (tabuLista[k] == ruch)
            {
                tabu = true;
                break;
            }
        }
        if (tabu) continue;

        // Tworzymy sąsiada
        std::vector<OperationSchedule> sasiad = aktualny;
int tmp = sasiad[i].priority;
sasiad[i].priority = sasiad[j].priority;
sasiad[j].priority = tmp;

        // Oblicz koszt
        int koszt = obliczMakespan(sasiad);

        if (koszt < najlepszySasiadKoszt)
        {
            najlepszySasiad = sasiad;
            najlepszySasiadKoszt = koszt;
            najlepszyI = i;
            najlepszyJ = j;
        }
    }
}


        // Jeśli znaleźliśmy sensownego sąsiada – użyj go
        if (najlepszySasiad.size() > 0)
        {
            aktualny = najlepszySasiad;

            // Aktualizujemy tabu listę
            std::pair<int, int> wykonanyRuch = std::make_pair(najlepszyI, najlepszyJ);
            tabuLista.push_back(wykonanyRuch);
            if (tabuLista.size() > dlugoscTabu)
            {
                tabuLista.erase(tabuLista.begin()); // usuwamy najstarszy (FIFO)
            }

            // Jeśli jest lepszy od najlepszego dotąd – zapamiętaj
            if (najlepszySasiadKoszt < najlepszyMakespan)
            {
                najlepszy = najlepszySasiad;
                najlepszyMakespan = najlepszySasiadKoszt;
            }
        }
        else
        {
            std::cerr << "Brak dostępnych sąsiadów (lub wszystko na tabu).\n";
            break;
        }
    }

    // === KROK 7: Zapisz wynik ===
    makespan = najlepszyMakespan;
    schedule = najlepszy;
}

int TabuSearchSolver::obliczMakespan(std::vector<OperationSchedule>& harmonogram) const
{
    std::map<int, int> dostepMaszyny; // dostępność maszyn
    std::map<int, int> dostepJoba;    // dostępność jobów
    std::map<std::pair<int, int>, bool> wykonane; // wykonane operacje (job, opID)

    int maxCzas = 0;

    // KROK 1: Sortujemy operacje po priorytecie (ale nie wykonujemy ich jeszcze!)
    for (int i = 0; i < harmonogram.size() - 1; ++i)
    {
        for (int j = 0; j < harmonogram.size() - i - 1; ++j)
        {
            if (harmonogram[j].priority > harmonogram[j + 1].priority)
            {
                OperationSchedule temp = harmonogram[j];
                harmonogram[j] = harmonogram[j + 1];
                harmonogram[j + 1] = temp;
            }
        }
    }

    // KROK 2: Pętla wykonująca operacje tylko wtedy, gdy mogą być wykonane
    int wykonanych = 0;
    while (wykonanych < harmonogram.size())
    {
        bool cosDodano = false;

        for (int i = 0; i < harmonogram.size(); ++i)
        {
            OperationSchedule& op = harmonogram[i];
            std::pair<int, int> klucz = std::make_pair(op.job_id, op.operation_id);

            // Jeżeli już wykonana – pomijamy
            if (wykonane[klucz]) continue;

            // Sprawdzamy, czy poprzednia operacja joba została już wykonana
            bool moznaWykonac = false;
            if (op.operation_id == 0)
            {
                moznaWykonac = true; // pierwszą operację można zawsze wykonać
            }
            else
            {
                std::pair<int, int> poprzednia = std::make_pair(op.job_id, op.operation_id - 1);
                if (wykonane[poprzednia])
                {
                    moznaWykonac = true;
                }
            }

            if (moznaWykonac)
            {
                int czasMaszyny = dostepMaszyny[op.machine_id];
                int czasJoba = dostepJoba[op.job_id];

                int start = (czasMaszyny > czasJoba) ? czasMaszyny : czasJoba;
                int end = start + op.processing_time;

                op.start_time = start;
                op.end_time = end;

                dostepMaszyny[op.machine_id] = end;
                dostepJoba[op.job_id] = end;
                wykonane[klucz] = true;

                if (end > maxCzas)
                {
                    maxCzas = end;
                }

                wykonanych++;
                cosDodano = true;
            }
        }

        // Jeśli w tej pętli nie udało się nic zaplanować – coś poszło nie tak
        if (!cosDodano)
        {
            std::cerr << "Błąd: Deadlock przy obliczaniu makespan – nie można wykonać żadnej operacji.\n";
            break;
        }
    }

    return maxCzas;
}


void TabuSearchSolver::printSchedule() const
{
    // Wypisujemy nagłówek
    std::cout << "\n=== Najlepszy harmonogram (TabuSearchSolver) ===\n";
    std::cout << "Makespan: " << makespan << "\n";
    std::cout << "Operacje:\n";
    std::cout << "Job\tOpID\tMaszyna\tPriory\tStart\tEnd\n";

    // Iterujemy po wszystkich operacjach w harmonogramie
    for (int i = 0; i < schedule.size(); ++i)
    {
        const OperationSchedule& op = schedule[i]; // jawne użycie typu

        // Wypisujemy dane operacji w czytelnej formie tabeli
        std::cout << op.job_id << "\t"
                  << op.operation_id << "\t"
                  << op.machine_id << "\t"
                  << op.priority << "\t"
                  << op.start_time << "\t"
                  << op.end_time << "\n";
    }
}

void TabuSearchSolver::zapiszDoCSV(const std::string& nazwaPliku) const
{
    // Otwieramy plik do zapisu
    std::ofstream out(nazwaPliku);

    // Sprawdzamy, czy plik otworzył się poprawnie
    if (!out.is_open())
    {
        std::cerr << "Nie można otworzyć pliku do zapisu: " << nazwaPliku << "\n";
        return;
    }

    // Nagłówek CSV
    out << "job_id,operation_id,machine_id,start_time,end_time,priority\n";

    // Zapisujemy dane każdej operacji w formacie CSV
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

    // Plik zostanie zamknięty automatycznie po wyjściu z funkcji (RAII)
}
