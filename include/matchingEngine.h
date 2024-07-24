#include <iostream>
#include <algorithm>
#include <fstream>
#include <vector>
#include <iterator>
#include <set>
#include <pthread.h>
#include <chrono>
#include "../include/AddingEdges.h"

using namespace std;
using namespace chrono;

class DataPassingToThreads{
public:
    int * num_of_neighbor;
    int * order;
    int * passing_node_to_thread_of_each;
    int round_index;
    int *neighbor_of_prenode_pattern;
    int size_of_neighbor_of_prenode_pattern;
    int number_of_matching;
    int index_of_snapshot;
    CSRGraph * graph;
    DataPassingToThreads(int *aPassing_node_to_thread_of_each,int aRound_index,int * aNeighbor_of_prenode_pattern,int aSize_of_neighbor_of_prenode_pattern,int aNumber_of_matching,int aIndex_of_snapshot,int * aNum_of_neighbor,int * aOrder,CSRGraph * aGraph){
        passing_node_to_thread_of_each=aPassing_node_to_thread_of_each;
        round_index=aRound_index;
        neighbor_of_prenode_pattern=aNeighbor_of_prenode_pattern;
        size_of_neighbor_of_prenode_pattern=aSize_of_neighbor_of_prenode_pattern;
        number_of_matching=aNumber_of_matching;
        index_of_snapshot=aIndex_of_snapshot;
        num_of_neighbor=aNum_of_neighbor;
        order=aOrder;
        graph=aGraph;
    }
};


struct DataForPassingBack{
    int number_of_matching_node;
    vector <int> matching_node;
};

struct ThreadData{
    DataPassingToThreads *data;
};



class matchingEngine{

private:
    vector<int> node_of_matching;
    ThreadData *args;
    int number_of_thread;
    int number_of_node_for_last_matching;
    CSRGraph *graph;
    PatternGraph * patternGraph;
    AddingEdges * addingEdges;

    vector <vector<int> > current_matching;

    //data used in the function of prepare for the data
    int **passing_node_to_thread_of_each;
    DataPassingToThreads **dataPassingToThreads;
    int *number_of_matching;
    int* neighbor_of_prenode;

public:
    matchingEngine(int aNumber_of_thread,CSRGraph* aGraph,PatternGraph * aPatternGraph,AddingEdges * aAddingEdges);
    ThreadData * get_the_data_prepared(int matching_round,int index_of_snapshot);
    int receiving_the_data(DataForPassingBack* ptr_get,int round_index);
    vector<int> getNode_of_matching();
    void Round_cleaner(int matching_round);
    void Matching_ss0();
    void Matching_ssi();
    vector< vector<int> >removeDuplicatesMatching(vector<int>& input, int size);
};


