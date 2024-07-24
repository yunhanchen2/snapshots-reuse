#include <iostream>
#include <algorithm>
#include <fstream>
#include <vector>
#include <iterator>
#include <set>
#include <time.h>
#include <pthread.h>

#include "../include/PatternGraph.h"

using namespace std;


PatternGraph::PatternGraph(int e,int n){
    //set the size
    edge=e;
    node=n;
    index_ptr_of_pattern=new int[n+1]();
    indices_of_pattern=new int[e*2]();
    num_of_neighbor=new int[n];
    order=new int[n];
    neighbor_restriction.resize(node);

    //init the array
    cout<<"input the index_ptr_of_pattern(from 0 to n)"<<endl;
    for(int i=0;i<node+1;i++){
        cin>>index_ptr_of_pattern[i];
    }

    cout<<"input the indices_of_pattern(from 0 to n)"<<endl;
    for(int i=0;i<edge*2;i++){
        cin>>indices_of_pattern[i];
    }

}
void PatternGraph::getTheNeighborOfEachNode(){
    for(int i=0;i<node;i++){
        num_of_neighbor[i]=index_ptr_of_pattern[i+1]-index_ptr_of_pattern[i];
    }
}

void PatternGraph::getTheMatchingOrder(){
    int max=node-1;
    int marker=0;
    while(marker!=node){
        for(int i=0;i<node;i++){
            if(max==num_of_neighbor[i]){
                order[marker]=i;
                marker++;
            }
        }
        max--;
    }
}

void PatternGraph::getNeighborRestriction(){
    for(int i=0;i<node;i++){
        for(int j=0;j<i;j++){//node before them in the order
            for(int k=index_ptr_of_pattern[order[i]];k<index_ptr_of_pattern[order[i]+1];k++){
                if(indices_of_pattern[k]==order[j]){
                    neighbor_restriction[order[i]].push_back(j);
                }
            }
        }
    }
}

void PatternGraph::clear(){
    if (index_ptr_of_pattern != nullptr) {
        delete[] index_ptr_of_pattern;
    }

    if (indices_of_pattern != nullptr) {
        delete[] indices_of_pattern;
    }

    if (num_of_neighbor != nullptr) {
        delete[] num_of_neighbor;
    }

    if (order != nullptr) {
        delete[] order;
    }

    for(int i=0;i<edge;i++){
        delete [] order_of_matching[i];
    }
    delete [] order_of_matching;
}

int PatternGraph::getEdge(){
    return edge;
};

int PatternGraph::getNode(){
    return node;
};

int * PatternGraph::getIndex_ptr_of_pattern(){
    return index_ptr_of_pattern;
};

int * PatternGraph::getIndices_of_pattern(){
    return indices_of_pattern;
};

int * PatternGraph::getOrder(){
    return order;
};

int * PatternGraph::getNum_of_neighbor(){
    return num_of_neighbor;
}

vector < vector<int> > PatternGraph::getNeighbor_restriction(){
    return neighbor_restriction;
};

void PatternGraph::generateEdgeMatchingOrder(){
    //generate order
    order_of_matching=new int*[edge];
    for(int i=0;i<edge;i++){
        order_of_matching[i]=new int[node]();
    }

    int ptr=0;
    for(int i=0;i<node;i++){
        int begin=i;
        for(int j=index_ptr_of_pattern[i];j<index_ptr_of_pattern[i+1];j++){
            if(indices_of_pattern[j]>begin){
                int end=indices_of_pattern[j];
                order_of_matching[ptr][0]=begin;
                order_of_matching[ptr][1]=end;
                int ptr2=2;
                for(int k=0;k<node;k++){
                    if(order[k]!=begin&&order[k]!=end){
                        order_of_matching[ptr][ptr2]=order[k];
                        ptr2++;
                    }
                }
                ptr++;
            }
        }
    }

//    //testing
//    for(int i=0;i<edge;i++){
//        for(int j=0;j<node;j++){
//            cout<<order_of_matching[i][j]<<" ";
//        }
//        cout<<endl;
//    }

    //generate restriction
    for(int i=0;i<edge;i++){
        vector<vector<int> > temp_out;
        for(int j=2;j<node;j++){
            vector <int> temp_in;
            int temp=order_of_matching[i][j];
            for(int k=index_ptr_of_pattern[temp];k<index_ptr_of_pattern[temp+1];k++){
                for(int k2=0;k2<j;k2++){
                    if(order_of_matching[i][k2]==indices_of_pattern[k]){
                        temp_in.push_back(k2);
                    }
                }
            }
            temp_out.push_back(temp_in);
        }
        matching_nei_restriction.push_back(temp_out);
    }

//    //testing
//    cout<<"the neighbor restriction:"<<endl;
//    for(int i=0;i<edge;i++){
//        for(int j=0;j<node-2;j++){
//            for(int t=0;t<matching_nei_restriction[i][j].size();t++){
//                cout<<matching_nei_restriction[i][j][t]<<" ";
//            }
//            cout<<endl;
//        }
//        cout<<endl;
//    }
}

vector <int> PatternGraph::getEdges_Numbers(){
    vector <int> return_edges;
    return_edges.clear();
    for(int i=0;i<node;i++){
        for(int j=index_ptr_of_pattern[i];j<index_ptr_of_pattern[i+1];j++){
            if(i<indices_of_pattern[j]){
                return_edges.push_back(i);
                return_edges.push_back(indices_of_pattern[j]);
            }
        }
    }
    return return_edges;
}