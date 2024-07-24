#include <iostream>
#include <algorithm>
#include <fstream>
#include <vector>
#include <iterator>
#include <set>
#include <pthread.h>
#include <chrono>
#include "../include/PatternGraph.h"
#include "../include/CSRGraph.h"

class DataPassingToThreads_Add{
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
    DataPassingToThreads_Add(int *aPassing_node_to_thread_of_each,int aRound_index,int * aNeighbor_of_prenode_pattern,int aSize_of_neighbor_of_prenode_pattern,int aNumber_of_matching,int aIndex_of_snapshot,int * aNum_of_neighbor,int * aOrder,CSRGraph * aGraph){
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

struct DataForPassingBack_Add{
    int number_of_matching_node;
    vector <int> matching_node;
};

struct ThreadData_Add{
    DataPassingToThreads_Add *data;
};

class AddingEdges{
private:
    vector<int> node_of_matching;
    ThreadData_Add *args;
    int number_of_thread;
    int number_of_node_for_last_matching;
    CSRGraph *graph;
    PatternGraph * patternGraph;
    vector <int> matching_of_addings;

    //for preparing data
    int **passing_node_to_thread_of_each;
    DataPassingToThreads_Add **dataPassingToThreads;
    int *number_of_matching;
    int* neighbor_of_prenode;
public:
    AddingEdges(int aNumber_of_thread,CSRGraph* aGraph,PatternGraph * aPatternGraph);
    ThreadData_Add * get_the_add_data_prepared(int matching_round,int index_of_snapshot,int index_of_edge);
    void Matching_Add(int ss_index);
    int receiving_the_add_data(DataForPassingBack_Add* ptr_get,int round_index);
    void Round_cleaner(int matching_round,int edge_index);

    vector <int> getMatching_of_addings(){
        return matching_of_addings;
    }
};