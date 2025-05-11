#ifndef OPERATION_SCHEDULE_H
#define OPERATION_SCHEDULE_H

// Struktura opisująca jedną operację w harmonogramie JSSP
struct OperationSchedule {
    int job_id;          // Numer joba
    int operation_id;    // Numer operacji w jobie
    int machine_id;      // Numer maszyny
    int processing_time; // Czas trwania operacji
    int start_time;      // Czas rozpoczęcia operacji
    int end_time;        // Czas zakończenia operacji
};

#endif // OPERATION_SCHEDULE_H
