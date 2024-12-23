#include "ParallelSwarm.h"
#include "MathHelper.h"
#include "Swarm.h"
#include "consts.h"
#include <cstring>
#include <mpi.h> 
#include <cmath>
#include <iostream>
#include <fstream>

using namespace std;

ParallelSwarm::ParallelSwarm(int robots, Antenna *antenna, Function *function)
    : Swarm(robots, antenna, function)
{
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size == 1)
    {
        local_start = 0;
        local_end = robots;
    }
    else
    {
        int robots_per_proc = robots / size;
        int remainder = robots % size;
        local_start = rank * robots_per_proc + std::min(rank, remainder);
        local_end = local_start + robots_per_proc + (rank < remainder ? 1 : 0);
    }

    step = 0;
    allocate_memory();
    initialize_antennas();
}

void ParallelSwarm::before_first_run()
{
    if (size == 1)
        return;
    for (int r = 0; r < robots; r++)
    {
        MPI_Bcast(position[r], dimensions, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    }
}

void ParallelSwarm::run(int steps)
{
    before_first_run();

    for (int i = 0; i < steps; i++)
    {
        single_step();
    }
}

void ParallelSwarm::single_step()
{
    step++;

    evaluate_function();
    find_neighbours_and_remember_best();
    move();
    fit_antenna_range();
}

void ParallelSwarm::evaluate_function()
{
    for (int robot = local_start; robot < local_end; robot++)
    {
        function_value[robot] = function->value(position[robot]);
    }

    if (size == 1)
    {
        return;
    }

    int local_count = (local_end - local_start);
    double *tmp = new double[local_count];
    memcpy(tmp, function_value + local_start, local_count * sizeof(double));

    int *counts = new int[size];
    int *displs = new int[size];

    for (int r = 0; r < size; r++)
    {
        int rp = robots / size;
        int rem = robots % size;
        int rs = r * rp + std::min(r, rem);
        int re = rs + rp + (r < rem ? 1 : 0);
        counts[r] = (re - rs);
    }

    displs[0] = 0;
    for (int r = 1; r < size; r++)
    {
        displs[r] = displs[r - 1] + counts[r - 1];
    }

    MPI_Allgatherv(
        tmp,
        local_count,
        MPI_DOUBLE,
        function_value,
        counts,
        displs,
        MPI_DOUBLE,
        MPI_COMM_WORLD);

    delete[] tmp;
    delete[] counts;
    delete[] displs;
}


void ParallelSwarm::move()
{
    for (int robot = local_start; robot < local_end; robot++)
    {
        MathHelper::move(position[robot],
                         position[neighbour_id[robot]],
                         new_position[robot],
                         dimensions,
                         STEP_SIZE / sqrt(step));
    }

    for (int robot = local_start; robot < local_end; robot++)
    {
        for (int d = 0; d < dimensions; d++)
        {
            position[robot][d] = new_position[robot][d];
        }
    }

    if (size > 1)
    {
        int local_count = (local_end - local_start) * dimensions;
        double *tmp = new double[local_count];

        {
            int idx = 0;
            for (int robot = local_start; robot < local_end; robot++)
            {
                memcpy(tmp + idx, position[robot], dimensions * sizeof(double));
                idx += dimensions;
            }
        }

        int *counts = new int[size];
        int *displs = new int[size];

        for (int r = 0; r < size; r++)
        {
            int rp = robots / size;
            int rem = robots % size;
            int rs = r * rp + std::min(r, rem);
            int re = rs + rp + (r < rem ? 1 : 0);
            counts[r] = (re - rs) * dimensions;
        }
        displs[0] = 0;
        for (int r = 1; r < size; r++)
        {
            displs[r] = displs[r - 1] + counts[r - 1];
        }

        double *pos_1d = new double[robots * dimensions];
        {
            int idx = 0;
            for (int robot = 0; robot < robots; robot++)
            {
                memcpy(pos_1d + idx, position[robot], dimensions * sizeof(double));
                idx += dimensions;
            }
        }

        MPI_Allgatherv(
            tmp,         // sendbuf
            local_count, // sendcount
            MPI_DOUBLE,  // sendtype
            pos_1d,      // recvbuf
            counts,
            displs,
            MPI_DOUBLE,
            MPI_COMM_WORLD);
        {
            int idx = 0;
            for (int robot = 0; robot < robots; robot++)
            {
                memcpy(position[robot], pos_1d + idx, dimensions * sizeof(double));
                idx += dimensions;
            }
        }

        delete[] tmp;
        delete[] pos_1d;
        delete[] counts;
        delete[] displs;
    }
}

void ParallelSwarm::find_neighbours_and_remember_best()
{
    for (int robot = local_start; robot < local_end; robot++)
    {
        int best_id = robot;
        double best_val = function_value[robot];
        double my_antenna_range_sq = antenna_range_sq[robot];
        double *my_position = position[robot];

        nearest_neighbours[robot] = 0;

        for (int other_robot = 0; other_robot < robots; other_robot++)
        {
            if (robot != other_robot)
            {
                double distance = MathHelper::distanceSQ(my_position, position[other_robot], dimensions);
                if (distance < my_antenna_range_sq)
                {
                    nearest_neighbours[robot]++;
                    if (function_value[other_robot] > best_val)
                    {
                        best_val = function_value[other_robot];
                        best_id = other_robot;
                    }
                }
            }
        }
        neighbour_id[robot] = best_id;
    }
}

void ParallelSwarm::fit_antenna_range()
{
    for (int robot = 0; robot < robots; robot++)
    {
        double range = antenna->range(sqrt(antenna_range_sq[robot]), nearest_neighbours[robot]);
        antenna_range_sq[robot] = range * range;
    }
}

void ParallelSwarm::initialize_antennas()
{
    double vSQ = antenna->initial_range();
    vSQ *= vSQ;
    for (int r = 0; r < robots; r++)
    {
        antenna_range_sq[r] = vSQ;
    }
}

void ParallelSwarm::set_position(int dimension, int robot, double position_value)
{
    position[robot][dimension] = position_value;
}

double ParallelSwarm::get_position(int robot, int dimension)
{
    return position[robot][dimension];
}

void ParallelSwarm::allocate_memory()
{
    position = new double *[robots];
    new_position = new double *[robots];
    for (int i = 0; i < robots; i++)
    {
        position[i] = new double[dimensions];
        new_position[i] = new double[dimensions];
    }

    neighbour_id = new int[robots];
    nearest_neighbours = new int[robots];
    function_value = new double[robots];
    antenna_range_sq = new double[robots];
}
