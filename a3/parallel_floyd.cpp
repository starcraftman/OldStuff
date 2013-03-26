/**
 * Important notes on input.txt
 * First number on first line is number of nodes in graph.
 * All nodes on subsequent lines are zero indexed (i.e. node 1 -> 0).
 * Remaining lines must take following format:
 * i	j	cost
 * Reads from node i to node j the edge has cost to travel.
 * If you want undirected graph you'll put one i,j pair then j,i pair.
 * There can be NO negative cycles in the graph, else the algorithm will produce bad output.
 */
/********************* Header Files ***********************/
/* C++ Headers */
#include <iostream> /* Input/output objects. */
#include <fstream> /* File operations. */
#include <sstream> /* String stream. */
#include <string> /* C++ String class. */
//#include <new> /* Defines bad_malloc exception, new functions. */
//#include <typeinfo> /* Casting header. */
//#include <exception> /* Top level exception header. */
//#include <stdexcept> /* Derived exception classes. */

/* STL Headers */
//#include <vector>
//#include <list>
//#include <deque>
//#include <stack>
//#include <queue>
//#include <priority_queue>
//#include <bitset>
//#include <set> // multiset for multiple keys allowed.
//#include <map> // multimap for multiple keys allowed.
//#include <utility> // Has pair for map.
//#include <algorithm>
//#include <numeric>

/* C Headers */
#include <cstdlib>
//#include <cstddef>
//#include <cctype>
//#include <cstring>
//#include <cstdio>
//#include <climits>
//#include <cassert>
//#include <cmath>

/* Project Headers */
#include "lib_floyd.hpp"
#include "mpi.h"

/******************* Constants/Macros *********************/


/**************** Namespace Declarations ******************/
using std::cin;
using std::cout;
using std::endl;
using std::string;
using floyd::Matrix;

/******************* Type Definitions *********************/


/**************** Static Data Definitions *****************/
void serial_shortest(Matrix& cost, Matrix& path);
string make_path(const int i, const int j, const Matrix& dist, const Matrix& p);

/****************** Class Definitions *********************/


/****************** Static Functions **********************/


/****************** Global Functions **********************/
/**
 * Main loop of the function.
 */
int main(int argc, char **argv) {
    double start(0.0);
    int num(0), i(0), j(0), id(0), world(0);

    /* Init mpi and time function. */
    MPI_Init(&argc, &argv);
    start = MPI_Wtime();
    id = MPI::COMM_WORLD.Get_rank();
    world = MPI::COMM_WORLD.Get_size();

    /* Open input and make matrices. */
    std::ifstream fin(INPUT, std::ifstream::in);
    std::ofstream fout(OUTPUT, std::ofstream::out);
    fin >> num;
    Matrix c(num); /* Cost matrix. */
    Matrix p(num); /* Path matrix. */
    floyd::init_path(p);

    /* Ensure we have even split amongst processors. */
    if ((num % world) != 0) {
        cout << "Please choose a number of processors that divide cleanly into nodes." << endl;
        return 1;
    }

    /* Ensure we have an even sqrt(p) value. */
    int root = floyd::lib_sqrt(world);
    if (root == 0) {
        cout << "The number of processors must have even square root values." << endl
            << "For example, p = 16, root = 4." << endl;
        return 1;
    }

    /* Expect cost matrix to be in INPUT file. */
    c.read(fin);
    fout << "The original cost matrix." << endl;
    c.print(fout);

    /* Function to compute paths. */
    serial_shortest(c, p);

    /* Final result and time. */
    if (id == MASTER) {
        fout << "The shortest path matrix." << endl;
        c.print(fout);
        fout << "Time elapsed from MPI_Init to MPI_Finalize is " << MPI_Wtime() - start << " seconds.\n";
        cout << "Check output.txt for results of the operation once done with query." << endl;
    }

    fout.close();
    MPI_Finalize();

    if (id == MASTER) {
        /* Query interface, query about any shortest path to get nodes. Zero indexed as always. */
        while (true) {
            cout << "Enter an i and j and I will tell you the shortest path and cost." << endl <<
                    "Enter -1 on both to quit." << endl;

            cin >> i >> j;

            if (i > c.size || j > c.size || i < 0 || j < 0) {
                cout << "Numbers out of range. Only 0 - " << c.size-1 << " allowed." << endl;
                continue;
            }

            if (-1 == i && -1 == j)
                break;

            cout << "The path from path is: " << i << make_path(i, j, c, p) << j << " " << endl
                    << "The cost of the path is: " << c.a[i][j] << endl;
        }
    }
    return 0;
}

/*
 * Find the shortest path, path is simply used to trace back the shortest path.
 */
void serial_shortest(Matrix& cost, Matrix& path) {
    for (int k = 0; k < cost.size; ++k) {
        for (int i = 0; i < cost.size; ++i) {
            for (int j = 0; j < cost.size; ++j) {
                int new_dist = cost.a[i][k] + cost.a[k][j];
                if (new_dist < cost.a[i][j]) {
                    cost.a[i][j] = new_dist;
                    path.a[i][j] = k;
                }
            }
        }
//		cout << "Cost matrix at round " << k << endl;
//		cost.print(cout);

        MPI::COMM_WORLD.Barrier();
    }
}

/*
 * Get the path from node i to j (zero index) with distance and path matrices.
 * You need to put i and j around what is returned from this function.
 */
string make_path(const int i, const int j, const Matrix& dist, const Matrix& p) {
    if (dist.a[i][j] == INF)
        return string("NO PATH.");

    int mid = p.a[i][j];

    if (mid == INF) {
        return string(" ");
    }

    std::stringstream line;
    line << make_path(i, mid, dist, p) << mid << make_path(mid, j, dist, p);

    return line.str();
}
