#include <iostream>
#include <fstream>
#include <cmath>
#include <cstdlib>   // for srandom, random
#include <cstring>
#include <omp.h>     // <-- używamy OpenMP, zamiast MPI

#include "Antenna.h"
#include "Function.h"
#include "MathHelper.h"
#include "SequentialSwarm.h"
#include "ParallelSwarm.h"
#include "SimpleAntenna.h"
#include "SimpleExpFunction.h"
#include "Swarm.h"
#include "consts.h"

using namespace std;

// ============== FUNKCJE POMOCNICZE ==============

// Zwrot losowej liczby [0..1)
double rnd() {
    return random() % 1000000 / 1000000.0;
}

// Inicjalizacja pozycji robotów w zakresie funkcji
void initialize_swarm(Swarm* swarm, Function* function, int robots) {
    int dims = function->dimensions();
    double* size = new double[dims];

    for (int d = 0; d < dims; d++) {
        size[d] = function->get_max_position()[d] - function->get_min_position()[d];
    }

    srandom(SEED);

    for (int robot = 0; robot < robots; robot++) {
        for (int d = 0; d < dims; d++) {
            double pos = function->get_min_position()[d] + size[d] * rnd();
            swarm->set_position(d, robot, pos);
        }
    }

    delete[] size;
}

// Prosta funkcja raportująca (tutaj tylko sygnalizacyjnie)
void show_report(int step, Swarm* swarm, int robots, int dims) {
    cout << "Wywołano show_report: step=" << step << endl;
    // Jeśli chcesz, możesz tu dodać wypisanie pozycji itp.
}

// Inicjalizacja testowej funkcji (jak w Twoim kodzie)
Function* initialize_function() {
    int DIMS = 4;
    SimpleExpFunction* f = new SimpleExpFunction(DIMS, 12);

    f->set_exp_params(0, 2.0, 0.25);
    f->set_exp_params(1, 2.0, 0.3);
    f->set_exp_params(2, 2.0, 0.4);
    f->set_exp_params(3, 2.0, 0.3);
    f->set_exp_params(4, 2.5, 0.25);
    f->set_exp_params(5, 2.4, 0.3);
    f->set_exp_params(6, 2.3, 0.4);
    f->set_exp_params(7, 2.2, 0.3);
    f->set_exp_params(8, 1.5, 0.25);
    f->set_exp_params(9, 1.4, 0.3);
    f->set_exp_params(10, 1.3, 0.4);
    f->set_exp_params(11, 1.2, 0.3);

    f->set_exp_location(0, 0, 1);
    f->set_exp_location(0, 1, 1);
    f->set_exp_location(0, 2, 0.5);
    f->set_exp_location(0, 3, 1.5);

    f->set_exp_location(1, 0, 4);
    f->set_exp_location(1, 1, 4);
    f->set_exp_location(1, 2, 0.5);
    f->set_exp_location(1, 3, 3.5);

    // ... analogicznie dla pozostałych 10 lokalizacji ...
    // (pomijam, bo są w Twoim kodzie – przeklej identycznie)

    return f;
}

// Zapis wyników do pliku (analogicznie do Twojej metody saveFile)
void saveFile(int step, Swarm* swarm, int robots, int dims, double time, const char* filename) {
    ofstream report(filename);
    report << step << endl;
    report << robots << endl;
    report << dims << endl;
    report << time << endl;

    for (int r = 0; r < robots; r++) {
        report << step << " " << r << " ";
        for (int d = 0; d < dims; d++) {
            report << swarm->get_position(r, d) << " ";
        }
        report << endl;
    }
    report.close();
}

// Porównanie z danymi sekwencyjnymi (jak w Twoim kodzie: compare(...))
int compare_positions(int step, Swarm* swarm, int robots, int dims, double time, int threads, const char* filename) {
    ifstream report(filename);
    if (!report.is_open()) {
        cerr << "Nie mogę otworzyć pliku " << filename << " do porównania!" << endl;
        return -1;
    }

    int step_loaded, robots_loaded, dims_loaded;
    double timeSeq;
    report >> step_loaded;
    report >> robots_loaded;
    report >> dims_loaded;
    report >> timeSeq;

    if (step != step_loaded || robots != robots_loaded || dims != dims_loaded) {
        cout << "Błąd. Załadowano błędny plik z danymi" << endl;
        return -1;
    }

    int robot_loaded;
    double position_expected;
    double position_from_swarm;

    for (int r = 0; r < robots; r++) {
        report >> step_loaded >> robot_loaded;
        for (int d = 0; d < dims; d++) {
            report >> position_expected;
            position_from_swarm = swarm->get_position(r, d);

            if (fabs(position_from_swarm - position_expected) > 0.0001) {
                cout << "Błąd wersja sekwencyjna daje inne położenia niż równoległa" << endl;
                cout << "Krok        : " << step << endl;
                cout << "Robot       : " << r << endl;
                cout << "Powinno być : " << position_expected << endl;
                cout << "OpenMP dało : " << position_from_swarm << endl;
                cout << "delta       : " << fabs(position_from_swarm - position_expected) << endl;
                return 1;
            }
        }
    }
    report.close();

    double speed_up = timeSeq / time;
    double efficiency = speed_up / threads;

    cout << "Czas pracy programu sekwencyjnego : " << timeSeq << endl;
    cout << "Czas pracy programu równoległego  : " << time << endl;
    cout << "Uzyskane przyspieszenie obliczeń  : " << speed_up << "x" << endl;
    cout << "Uzyskana efektywność pracy        : " << 100.0 * efficiency << "%" << endl;

    if (efficiency < 0.6) {
        cout << "Zbyt mała efektywność obliczeń współbieżnych" << endl;
        return 2;
    }

    if (efficiency > 1.0) {
        cout << "Efektywność obliczeń przekroczyła 100%. JAK????????????" << endl;
        cout << "Efektywność obliczeń przekroczyła 100%. JAK????????????" << endl;
        cout << "Efektywność obliczeń przekroczyła 100%. JAK????????????" << endl;

        double timeSeq2 = time * threads / 0.99;
        cout << "Korekta czasu pracy programu sekwencyjnego : " << timeSeq2 << endl;
        cout << "Korekta czasu pracy programu sekwencyjnego : " << timeSeq2 << endl;
        cout << "Korekta czasu pracy programu sekwencyjnego : " << timeSeq2 << endl;
    }
    return 0;
}

// ============== GŁÓWNA FUNKCJA MAIN ==============
int main(int argc, char** argv) {
    // Ustalamy liczbę wątków z OMP_NUM_THREADS (ale w OpenMP i tak to automatyczne)
    // Możemy odczytać np. int threads = omp_get_max_threads();
    int threads = omp_get_max_threads();

    cout << "OpenMP version. Liczba wątków dostępnych: " << threads << endl;

    // Inicjalizacja (bez MPI)
    // Tutaj nie robimy MPI_Init, bo używamy OpenMP

    Function* function = initialize_function();
    double max_distance = MathHelper::distance(function->get_max_position(),
                                               function->get_min_position(),
                                               function->dimensions());

    // Tworzymy antenę
    Antenna* antenna = new SimpleAntenna(TARGET_NEIGHBOURS,
                                         max_distance / ANTENNA_MIN_RANGE_DIV,
                                         max_distance / ANTENNA_MAX_RANGE_DIV,
                                         ANTENNA_RANGE_MODIFIER);

    // Jeśli wątków == 1 => sekwencja, inaczej => parallel
    Swarm* swarm = nullptr;
    if (threads == 1) {
        // Wersja sekwencyjna
        swarm = new SequentialSwarm(ROBOTS, antenna, function);
        cout << "Uruchamiam wersję sekwencyjną (threads=1)" << endl;
    } else {
        // Wersja równoległa
        swarm = new ParallelSwarm(ROBOTS, antenna, function);
        cout << "Uruchamiam wersję równoległą (threads=" << threads << ")" << endl;
    }

    // Inicjalizujemy losowe pozycje robotów (tylko raz)
    initialize_swarm(swarm, function, ROBOTS);

    // Pomiar czasu (OpenMP)
    double startAt = omp_get_wtime();

    // Możesz dodać ewentualne "before_first_run()" jeśli masz w ParallelSwarm
    // swarm->before_first_run();

    // Wykonanie symulacji
    int step = 0;
    do {
        swarm->run(REPORT_PERIOD);
        step += REPORT_PERIOD;
        // swarm->before_get_position(); // jeśli chcesz
        show_report(step, swarm, ROBOTS, function->dimensions());
    } while (step < STEPS);

    double endAt = omp_get_wtime();
    double delta = endAt - startAt;
    cout << "Czas wykonania: " << delta << " s" << endl;

    // Zapis lub porównanie
    if (threads == 1) {
        // Zapis wyników do pliku last_positions.txt (dla sekwencji)
        saveFile(step, swarm, ROBOTS, function->dimensions(), delta, "last_positions.txt");
        cout << "Zapisano wyniki w last_positions.txt (wersja sekwencyjna)." << endl;
    } else {
        // Porównanie z plikiem last_positions.txt
        int status = compare_positions(step, swarm, ROBOTS, function->dimensions(),
                                       delta, threads, "last_positions.txt");
        if (!status) {
            cout << "!!!!!!!!!!!!! SUKCES !!!!!!!!!!!" << endl;
        } else {
            cout << "! PORAZKA ! PORAZKA ! PORAZKA ! PORAZKA ! PORAZKA ! PORAZKA !" << endl;
        }
    }

    // Sprzątanie
    delete swarm;
    delete antenna;
    delete function;

    return 0;
}
