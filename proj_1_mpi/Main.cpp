/*
 * Main.cpp
 */

#include "Antenna.h"
#include "Function.h"
#include "MathHelper.h"
#include "SequentialSwarm.h"
#include "ParallelSwarm.h"
#include "SimpleAntenna.h"
#include "SimpleExpFunction.h"
#include "Swarm.h"
#include "consts.h"

#include <fstream>
#include <cstring>
#include <iostream>
#include <stdlib.h>
#include <mpi.h>

using namespace std;

double rnd() {
	return random() % 1000000 / 1000000.0;
}

void initialize_swarm(Swarm *swarm, Function *function, int robots) {
	int dims = function->dimensions();
	double *size = new double[dims];

	for (int d = 0; d < dims; d++)
		size[d] = function->get_max_position()[d]
				- function->get_min_position()[d];

	srandom(SEED);

	for (int robot = 0; robot < robots; robot++) {
		for (int d = 0; d < dims; d++)
			swarm->set_position(d, robot,
					function->get_min_position()[d] + size[d] * rnd());
	}

	delete[] size;
}

void show_report(int step, Swarm *swarm, int robots, int dims) {
	cout << "WywoĹanow show_report" << endl;
}


Function* initialize_function() {
	int DIMS = 4;
	SimpleExpFunction *f = new SimpleExpFunction(DIMS, 12);

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

	f->set_exp_location(2, 0, 1);
	f->set_exp_location(2, 1, 4);
	f->set_exp_location(2, 2, 0.5);
	f->set_exp_location(2, 3, 1.5);

	f->set_exp_location(3, 0, 4);
	f->set_exp_location(3, 1, 1);
	f->set_exp_location(3, 2, 0.5);
	f->set_exp_location(3, 3, 3.5);

	f->set_exp_location(4, 0, 1);
	f->set_exp_location(4, 1, 1);
	f->set_exp_location(4, 2, 2.5);
	f->set_exp_location(4, 3, 1.5);

	f->set_exp_location(5, 0, 4);
	f->set_exp_location(5, 1, 4);
	f->set_exp_location(5, 2, 2.5);
	f->set_exp_location(5, 3, 3.5);

	f->set_exp_location(6, 0, 1);
	f->set_exp_location(6, 1, 4);
	f->set_exp_location(6, 2, 2.5);
	f->set_exp_location(6, 3, 1.5);

	f->set_exp_location(7, 0, 4);
	f->set_exp_location(7, 1, 1);
	f->set_exp_location(7, 2, 2.5);
	f->set_exp_location(7, 3, 3.5);

	f->set_exp_location(8, 0, 1);
	f->set_exp_location(8, 1, 1);
	f->set_exp_location(8, 2, 2.5);
	f->set_exp_location(8, 3, 1.5);

	f->set_exp_location(9, 0, 4);
	f->set_exp_location(9, 1, 4);
	f->set_exp_location(9, 2, 2.5);
	f->set_exp_location(9, 3, 3.5);

	f->set_exp_location(10, 0, 1);
	f->set_exp_location(10, 1, 4);
	f->set_exp_location(10, 2, 2.5);
	f->set_exp_location(10, 3, 1.5);

	f->set_exp_location(11, 0, 4);
	f->set_exp_location(11, 1, 1);
	f->set_exp_location(11, 2, 2.5);
	f->set_exp_location(11, 3, 3.5);

	return f;
}

void saveFile(int step, Swarm *swarm, int robots, int dims, double time, const char *filename )
{
	ofstream report;

	report.open(filename);

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

int compare(int step, Swarm *swarm, int robots, int dims, double time, int procs, const char *filename) {
	ifstream report;
	report.open(filename);

	int step_loaded, robots_loaded, dims_loaded ;
	double timeSeq;
	report >> step_loaded;
	report >> robots_loaded;
	report >> dims_loaded;
	report >> timeSeq;

	if (( step != step_loaded) || ( robots != robots_loaded ) ||
			( dims != dims_loaded ) )
	{
		cout << "BĹÄd. ZaĹadowano bĹÄdny plik z danymi" << endl;
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
			if ( fabs( position_from_swarm - position_expected ) > 0.0001 ) {
				cout << "BĹad wersja sekwencyjna daje inne poĹoĹźenia niĹź wspĂłĹbieĹźna" << endl;
				cout << "Krok        : " << step << endl;
				cout << "Robot       : "  << r << endl;
				cout << "Powinno byÄ : " << position_expected << endl;
				cout << "MPI daĹo    : " << position_from_swarm << endl;
				cout << "delta       : " << fabs( position_from_swarm - position_expected ) << endl;
				return 1;
			}
		}
	}
	report.close();

	double speed_up = timeSeq / time;
	double efficiency = speed_up / procs;

	cout << "Czas pracy programu sekwencyjnego : " << timeSeq << endl;
	cout << "Czas pracy programu rĂłwnolegĹego  : " << time << endl;
	cout << "Uzykane przyspieszenie obliczeĹ   : " << speed_up << "x" << endl;
	cout << "Uzyskana efektywnoĹÄ pracy        : " << 100.0 * efficiency << "%" << endl;

	if ( efficiency < 0.6 ) {
		cout << "Zbyt maĹa efektywnoĹÄ obliczeĹ wspĂłĹbieĹźnych" << endl;
		return 2;
	}

	if ( efficiency > 1.0 ) {
		cout << "EfektywnoĹÄ obliczeĹ przekroczyĹa 100%. JAK????????????" << endl;
		cout << "EfektywnoĹÄ obliczeĹ przekroczyĹa 100%. JAK????????????" << endl;
		cout << "EfektywnoĹÄ obliczeĹ przekroczyĹa 100%. JAK????????????" << endl;

		timeSeq = time * procs / 0.99; 
		cout << "Korekta czasu pracy programu sekwencyjnego : " << timeSeq << endl;
		cout << "Korekta czasu pracy programu sekwencyjnego : " << timeSeq << endl;
		cout << "Korekta czasu pracy programu sekwencyjnego : " << timeSeq << endl;
	}

	return 0;
}

int main(int argc, char **argv) {
	double start;
	int procs, rank = 0;

	MPI_Init(&argc, &argv);
	cout << "MPI version " << MPI_VERSION << endl;
	MPI_Comm_size(MPI_COMM_WORLD, &procs);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	Function *function = initialize_function();
	double max_distance = MathHelper::distance(function->get_max_position(),
			function->get_min_position(), function->dimensions());

	Antenna *antenna = new SimpleAntenna(TARGET_NEIGHBOURS,
			max_distance / ANTENNA_MIN_RANGE_DIV,
			max_distance / ANTENNA_MAX_RANGE_DIV, ANTENNA_RANGE_MODIFIER);
	Swarm *swarm;

	if ( procs == 1 )
		swarm = new SequentialSwarm(ROBOTS, antenna, function);
	else
		swarm = new ParallelSwarm(ROBOTS, antenna, function);

	if (!rank) {
		initialize_swarm(swarm, function, ROBOTS);
	}

	int step = 0;
	double startAt = MPI_Wtime();
	swarm->before_first_run();
	do {
		swarm->run(REPORT_PERIOD);
		step += REPORT_PERIOD;
   		swarm->before_get_position();
		if ( rank == 0 ) 
			cout << "Wykonano " << step << " krokow symulacji" << endl;
	} while (step < STEPS);
	double endAt = MPI_Wtime();
	double delta = endAt - startAt;

	if ( procs == 1 ) {
		saveFile( step, swarm, ROBOTS, function->dimensions(), delta, "last_positions.txt");
	} else {
		if (!rank) {
			if ( ! compare(step, swarm, ROBOTS, function->dimensions(), delta, procs, "last_positions.txt") ) {
				cout << "!!!!!!!!!!!!! SUKCES !!!!!!!!!!!" << endl;
			} else {
				cout << "! PORAZKA ! PORAZKA ! PORAZKA ! PORAZKA ! PORAZKA ! PORAZKA !" << endl;
			}
		}
	}

	MPI_Finalize();

	return 0;
}