#include "ParallelSwarm.h"
#include "MathHelper.h"
#include "Swarm.h"
#include "consts.h"
#include <cmath>
#include <iostream>

#ifdef _OPENMP
  #include <omp.h>
#endif

using namespace std;

ParallelSwarm::ParallelSwarm(int robots, Antenna* antenna, Function* function)
    : Swarm(robots, antenna, function)
{
    step = 0;
    allocate_memory();      
    initialize_antennas();  
}

void ParallelSwarm::run(int steps) {
    for (int i = 0; i < steps; i++) {
        single_step();
    }
}

void ParallelSwarm::single_step() {
    step++;
    evaluate_function();
    find_neighbours_and_remember_best();
    move();
    fit_antenna_range();
}

void ParallelSwarm::evaluate_function() {
    #pragma omp parallel for
    for (int robot = 0; robot < robots; robot++) {
        function_value[robot] = function->value(position[robot]);
    }
}

void ParallelSwarm::find_neighbours_and_remember_best() {
    #pragma omp parallel for private(my_antenna_range_sq, my_position, best_function_value, best_id)
    for (int robot = 0; robot < robots; robot++) {
        double* loc_position = position[robot];  
        double loc_best_fv   = function_value[robot];
        double loc_antenna   = antenna_range_sq[robot];
        int    loc_best_id   = robot;

        nearest_neighbours[robot] = 0;

        for (int other_robot = 0; other_robot < robot; other_robot++) {
            double dist_sq = MathHelper::distanceSQ(loc_position, position[other_robot], dimensions);
            if (dist_sq < loc_antenna) {
                nearest_neighbours[robot]++;   
                if (loc_best_fv < function_value[other_robot]) {
                    loc_best_fv = function_value[other_robot];
                    loc_best_id = other_robot;
                }
            }
        }
        for (int other_robot = robot + 1; other_robot < robots; other_robot++) {
            double dist_sq = MathHelper::distanceSQ(loc_position, position[other_robot], dimensions);
            if (dist_sq < loc_antenna) {
                nearest_neighbours[robot]++;
                if (loc_best_fv < function_value[other_robot]) {
                    loc_best_fv = function_value[other_robot];
                    loc_best_id = other_robot;
                }
            }
        }

        neighbour_id[robot] = loc_best_id;
    }
}

void ParallelSwarm::move() {
    // 1. Obliczamy new_position
    #pragma omp parallel for
    for (int robot = 0; robot < robots; robot++) {
        MathHelper::move(position[robot],
                         position[neighbour_id[robot]],
                         new_position[robot],
                         dimensions,
                         STEP_SIZE / sqrt(step));
    }

    // 2. Kopiujemy do position
    #pragma omp parallel for
    for (int robot = 0; robot < robots; robot++) {
        for (int d = 0; d < dimensions; d++) {
            position[robot][d] = new_position[robot][d];
        }
    }
}

void ParallelSwarm::fit_antenna_range() {
    #pragma omp parallel for
    for (int robot = 0; robot < robots; robot++) {
        double range = antenna->range(sqrt(antenna_range_sq[robot]),
                                      nearest_neighbours[robot]);
        antenna_range_sq[robot] = range * range;
    }
}

void ParallelSwarm::allocate_memory() {
    position     = new double*[robots];
    new_position = new double*[robots];

    for (int i = 0; i < robots; i++) {
        position[i]     = new double[dimensions];  
        new_position[i] = new double[dimensions];
    }

    neighbour_id       = new int[robots];
    nearest_neighbours = new int[robots];
    function_value     = new double[robots];
    antenna_range_sq   = new double[robots];
}

void ParallelSwarm::initialize_antennas() {
    double init_range = antenna->initial_range();
    double vSQ = init_range * init_range;
    for (int r = 0; r < robots; r++) {
        antenna_range_sq[r] = vSQ;
    }
}

void ParallelSwarm::distributionOfRobots(int* histogramP0, int* histogramP1, int size, double d2idx)
{
    this->histogramD0 = histogramP0;
    this->histogramD1 = histogramP1;
    histogram_size = size;
    this->d2idx = d2idx;

    #pragma omp parallel for
    for (int robotA = 0; robotA < robots; robotA++) {
        distributionForRobot(robotA);
    }
}

void ParallelSwarm::distributionForRobot(int robotA) {
    double dxMin = 1e18;
    double dyMin = 1e18;
    double dx, dy;

    for (int robotB = 0; robotB < robots; robotB++) {
        if (robotA == robotB) continue;

        dx = fabs(position[robotA][0] - position[robotB][0]);
        dy = fabs(position[robotA][1] - position[robotB][1]);

        if (dx < dxMin) dxMin = dx;
        if (dy < dyMin) dyMin = dy;
    }

    int idxD0 = static_cast<int>(dxMin * d2idx);
    int idxD1 = static_cast<int>(dyMin * d2idx);

    if (idxD0 < histogram_size) {
        #pragma omp atomic
        histogramD0[idxD0]++;
    }

    if (idxD1 < histogram_size) {
        #pragma omp atomic
        histogramD1[idxD1]++;
    }
}


void ParallelSwarm::set_position(int dimension, int robot, double position_value) {
    position[robot][dimension] = position_value;
}

double ParallelSwarm::get_position(int robot, int dimension) {
    return position[robot][dimension];
}
