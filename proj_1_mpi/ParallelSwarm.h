#pragma once
#include "Swarm.h"

class ParallelSwarm : public Swarm
{
public:
    ParallelSwarm(int robots, Antenna *antenna, Function *function);

    void run(int steps);
    void single_step();

    void evaluate_function();
    void find_neighbours_and_remember_best();
    void move();
    void fit_antenna_range();
    void initialize_antennas();

    void before_first_run();
    void before_get_position() {};

    void set_position(int dimension, int robot, double position_value);
    double get_position(int robot, int dimension);

private:
    void allocate_memory();

    int rank, size;             // Identyfikator procesu i liczba procesów
    int local_start, local_end; // Zakres robotów, za które odpowiada dany proces
    int step;
    int *neighbour_id;
    int *nearest_neighbours;
    double *function_value;
    double *antenna_range_sq;

    double **position;
    double **new_position;
};
