#include <iostream>
#include <algorithm>
#include <fstream>
#include <vector>
#include <iterator>
#include <set>
#include <pthread.h>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <random>
#include <unordered_set>

using namespace std;

struct Edge
{
    int source;
    int destination;
#ifdef WEIGHT
    int weights;
#endif
};

class CSRGraph {
private:
    int node; // the number of current nodes
    int edge;   // the number of current edges
    int **array_origin;
    static int max_index; // the maximum number of all nodes
    int max_degree; // the maximum degree among all nodes in the entire graph

    vector <vector <int> > ss_adding_edge;
    vector<vector<int> > ss_deleting_edge;

    int *col_indices;    // delete this??? edge array in the CSR for the original graph
    int *row_offsets;    //  delete this??? vertex array in the CSR for the original graph

    int *true_index;    //  real vertex ID -> organized vertex ID
    static int *query_list; // organized vertex ID -> real vertex ID


    static int ** ss_neighbor_number; // the number of neighbors (including deleted) for each node in each snapshot; size = [16][#Nodes]
    static vector <int> ss_col_indices; // the edge array for the latest snapshots
    static int * ss_row_offsets; // the vertex array for the latest snapshots

    int m_snapshots; // ??? the number of snapshots; default = 16;

    static bool compare(const int* a, const int* b);
    void removeDuplicates();
    vector<int> generateUniqueRandomInts(int n, int m);

    char* filename; // where the data graph comes from
    int original_ratio; // default = 10
public:
    CSRGraph(); // constructor
    ~CSRGraph() { clear(); }

    int getNode();
    int getEdge();
    int getNumber_of_ss(){return m_snapshots;}
    int getMaxDegree();
    static int getMaxIndex();
    int * getCol_indices();
    int * getRow_offsets();
    int * getTrue_index();
    int * getQuery_list();
    vector <vector <int> > getSS_adding_edge(){return ss_adding_edge;}
    vector <vector <int> > getSS_deleting_edge(){return ss_deleting_edge;}

    int getTheNumberOfSnapshots();

    void setSizeOfSnapshot(int s);
    void setOriginal_Ratio(int r);

    void loadTheGraph(char *pathname); // read the graph from a file in "pathname", generate the original graph
    void clear(); // delete all arrays created;
    void generate_one_snapshot(int _ins_ratio, int _del_ratio,int round_index); // ????
    void Generate_Snapshots(int _num_snapshots, int _original_ratio);
    bool check_neighbor_exist(int _v_a,int _v_b,int index_of_snapshot); // checking whether two vertices are neighbours in a snapshot
    static vector <int> getTheNeighbor(int _v_a, int index_of_snapshot); // return all neighbours of node _v_a_ in a snapshot
    bool check_deleting_status(int _v_a, int _v_b, int ss_index);
};