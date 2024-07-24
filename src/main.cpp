#include <iostream>
#include <algorithm>
#include <fstream>
#include <vector>
#include <iterator>
#include <set>
#include <pthread.h>
#include "../include/matchingEngine.h"
#include <chrono>

using namespace std;
using namespace chrono;

static int number_of_thread;

static CSRGraph graph;

//pthread_mutex_t mu;


int main(int argc,char* argv[]) {

//    pthread_mutex_init(&mu, NULL);

    if (argc > 2) {
        number_of_thread = atoi(argv[2]);
        //get the pattern graph
        int e;
        int nod;

        cout << "input the number of edge and node pattern graph" << endl;
        cin >> e >> nod;

        PatternGraph patternGraph(e, nod);

        //get neighbor of each node
        patternGraph.getTheNeighborOfEachNode();

        //get the order
        patternGraph.getTheMatchingOrder();

        //find out the restriction of nodes
        patternGraph.getNeighborRestriction();

        patternGraph.generateEdgeMatchingOrder();

        //get the data graph
        char *pathname = argv[1];

        graph.loadTheGraph(pathname);//read+sort

        //do the matching
        cout << "Input the number of the snapshots and the original ratio: " << endl;
        int num_ss, ratio;
        cin >> num_ss >> ratio;
        graph.Generate_Snapshots(num_ss, ratio);

        AddingEdges addingEdges(number_of_thread, &graph, &patternGraph);

        matchingEngine engine(number_of_thread, &graph, &patternGraph,&addingEdges);

        engine.Matching_ss0();
        engine.Matching_ssi();

    }

//    pthread_mutex_destroy(&mu);

    return 0;
}
