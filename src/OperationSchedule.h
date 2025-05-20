#ifndef OPERATION_SCHEDULE_H
#define OPERATION_SCHEDULE_H
#include <vector>

// Struktura opisująca jedną operację w harmonogramie JSSP
struct OperationSchedule {
    int job_id;          // Numer joba
    int operation_id;    // Numer operacji w jobie
    int machine_id;      // Numer maszyny
    int processing_time; // Czas trwania operacji
    int start_time;      // Czas rozpoczęcia operacji
    int end_time;        // Czas zakończenia operacji
    int priority;
};

inline bool porownajPoPriorytecie(const OperationSchedule& a, const OperationSchedule& b)
{
    return a.priority < b.priority;
}

// Pomocniczy porównywacz do sortowania indeksów operacji według priorytetu
struct PorownywaczIndeksowPoPriorytecie
{
    const std::vector<OperationSchedule>* operacje;

    PorownywaczIndeksowPoPriorytecie(const std::vector<OperationSchedule>* op)
    {
        operacje = op;
    }

    bool operator()(int a, int b) const
    {
        return (*operacje)[a].priority < (*operacje)[b].priority;
    }
};


#endif // OPERATION_SCHEDULE_H
