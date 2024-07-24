#include <iostream>
#include <algorithm>
#include <fstream>
#include <vector>
#include <iterator>
#include <set>
#include <pthread.h>

using namespace std;

class PatternGraph{
private:
    int edge;
    int node;
    int *index_ptr_of_pattern;
    int *indices_of_pattern;
    int *num_of_neighbor;
    int *order;
    int ** order_of_matching;
    vector < vector<int> > neighbor_restriction;
    vector < vector <vector< int > > > matching_nei_restriction;

public:
    int getEdge();
    int getNode();
    int * getIndex_ptr_of_pattern();
    int * getIndices_of_pattern();
    int * getNum_of_neighbor();
    int * getOrder();
    vector < vector<int> > getNeighbor_restriction();
    vector < vector <vector< int > > > getMatching_Restriction(){ return matching_nei_restriction;}
    int ** getOder_of_matching(){return order_of_matching;}

    PatternGraph(int e,int n);
    ~PatternGraph() { clear(); }
    void getTheNeighborOfEachNode();
    void generateEdgeMatchingOrder(); //contain edges number of the matching order
    void getTheMatchingOrder();
    void getNeighborRestriction();
    void clear();
    vector <int> getEdges_Numbers();
};

