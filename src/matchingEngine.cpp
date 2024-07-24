#include <iostream>
#include <algorithm>
#include <fstream>
#include <vector>
#include <iterator>
#include <set>
#include <pthread.h>
#include <chrono>
#include "../include/matchingEngine.h"

using namespace std;
using namespace chrono;

static vector<int> vectors_intersection(vector<int> v1,vector<int> v2){
    vector<int> v;
    sort(v1.begin(),v1.end());
    sort(v2.begin(),v2.end());
    set_intersection(v1.begin(),v1.end(),v2.begin(),v2.end(),back_inserter(v));//求交集
    return v;
}

void* graph_matching_threads(void *n){
    //in threads: get the neighbor and check degree store them in the vectors
    //record time

    ThreadData* dataT=(ThreadData*) n;
    DataPassingToThreads *dataPassingToThreads=dataT->data;

    DataForPassingBack *passingBack=new DataForPassingBack();

    passingBack->number_of_matching_node=0;

    int round=dataPassingToThreads->index_of_snapshot;

    CSRGraph * graph=dataPassingToThreads->graph;

    if(dataPassingToThreads->round_index==0){
        //only check the degree
        for (int j = 0; j < dataPassingToThreads->number_of_matching; j++) {//满足其邻居条件以后,j为candidate node中的jth元素

            //get the neighbor
            vector <int> neighbor = graph->getTheNeighbor(dataPassingToThreads->passing_node_to_thread_of_each[j],round);

            if ( neighbor.size() >= dataPassingToThreads->num_of_neighbor[dataPassingToThreads->order[dataPassingToThreads->round_index]] ) {//degree也满足了
                passingBack->matching_node.push_back(dataPassingToThreads->passing_node_to_thread_of_each[j]);//存新match的
                passingBack->number_of_matching_node++;
            }

        }

    } else {
        int *tem;
        for(int i=0;i<dataPassingToThreads->number_of_matching;i++){
            //each time pick one group
            tem=new int[dataPassingToThreads->round_index];
            for(int j=0;j<dataPassingToThreads->round_index;j++){
                tem[j]=dataPassingToThreads->passing_node_to_thread_of_each[i*dataPassingToThreads->round_index+j];
            }

            //get the neighbors
            vector<int> back;
            vector< vector<int> > neibor(dataPassingToThreads->size_of_neighbor_of_prenode_pattern);
            for(int k=0;k<dataPassingToThreads->size_of_neighbor_of_prenode_pattern;k++){//将邻居放入vector中
                vector <int> neighbor=graph->getTheNeighbor(tem[dataPassingToThreads->neighbor_of_prenode_pattern[k]],round);
                for(int r=0;r<neighbor.size();r++){
                    neibor[k].push_back(neighbor[r]);//放入的是对应的编号而非第几个
                }
//                //testing
//                pthread_mutex_lock(&mu);
//                for(int r=0;r<neighbor.size();r++){
//                    cout<<neighbor[r]<<" ";
//                }
//                cout<<endl;
//                pthread_mutex_unlock(&mu);

                //join the vector
                if(k==0){
                    back=neibor[0];
                } else {
                    back= vectors_intersection(back,neibor[k]);
                }
            }

            //cut off the node before the node and have a new matching
            vector<int>::iterator it;
            for(it=back.begin();it!=back.end();){
                bool check=true;
                for(int j=0;j<dataPassingToThreads->round_index;j++){
                    if(*it==tem[j]){
                        it=back.erase(it);
                        check= false;
                    }
                }
                if(check){
                    it++;
                }
            }

            //check the degree
            for (int j = 0; j < back.size(); j++) {//满足其邻居条件以后,j为candidate node中的jth元素
                if (graph->getTheNeighbor(back[j],round).size() >= dataPassingToThreads->num_of_neighbor[dataPassingToThreads->order[dataPassingToThreads->round_index]]) {//degree也满足了
                    for (int k = 0; k < dataPassingToThreads->round_index; k++) {
                        passingBack->matching_node.push_back(tem[k]);//将原来的存回去

                    }
                    passingBack->matching_node.push_back(back[j]);//存新match的
                    passingBack->number_of_matching_node++;
                }
            }

            delete [] tem;
        }
    }

    pthread_exit(passingBack);
}


matchingEngine::matchingEngine(int aNumber_of_thread,CSRGraph* aGraph,PatternGraph * aPatternGraph,AddingEdges * aAddingEdges){
    number_of_thread=aNumber_of_thread;
    args=new ThreadData[aNumber_of_thread];
    graph=aGraph;
    patternGraph=aPatternGraph;
    addingEdges=aAddingEdges;
    number_of_node_for_last_matching=aGraph->getNode();
}

ThreadData * matchingEngine::get_the_data_prepared(int matching_round,int index_of_snapshot){

    int id=patternGraph->getOrder()[matching_round];
    args=new ThreadData[number_of_thread]();
    //preparing data
    //query graph中的neighbor限制
    int size_of_neighbor_of_prenode=0;

    if(patternGraph->getNeighbor_restriction()[id].size()!=0){
        neighbor_of_prenode=new int[patternGraph->getNeighbor_restriction()[id].size()]();
        for(int p=0;p<patternGraph->getNeighbor_restriction()[id].size();p++){
            neighbor_of_prenode[p]=patternGraph->getNeighbor_restriction()[id][p];
        }
        size_of_neighbor_of_prenode=patternGraph->getNeighbor_restriction()[id].size();
    }

    //lunch the threads
    int full_node_for_each_thread=number_of_node_for_last_matching/number_of_thread;
    int remaining=number_of_node_for_last_matching-full_node_for_each_thread*number_of_thread;
    if(remaining==0){
        remaining=number_of_thread;
    } else {
        full_node_for_each_thread++;
    }
    int sharing_node_ptr=0;

    passing_node_to_thread_of_each=new int* [number_of_thread];
    dataPassingToThreads=new DataPassingToThreads * [number_of_thread];
    number_of_matching=new int[number_of_thread]();

    //prepare for the data
    for(int p=0;p<number_of_thread; p++){
        if(p<remaining){
            number_of_matching[p]=full_node_for_each_thread;
        }else {
            number_of_matching[p]=full_node_for_each_thread-1;
        }

        if(matching_round==0){
            passing_node_to_thread_of_each[p]=new int[number_of_matching[p]];
            for(int t=0;t<number_of_matching[p];t++){
                passing_node_to_thread_of_each[p][t]=graph->getTrue_index()[sharing_node_ptr];
                sharing_node_ptr++;
            }
        } else {
            passing_node_to_thread_of_each[p]=new int[number_of_matching[p]*matching_round];
            for(int t=0;t<number_of_matching[p];t++){
                for(int k=0;k<matching_round;k++){
                    passing_node_to_thread_of_each[p][t*matching_round+k]=node_of_matching[matching_round*sharing_node_ptr+k];
                }
                sharing_node_ptr++;
            }
        }

        dataPassingToThreads[p]=new DataPassingToThreads(passing_node_to_thread_of_each[p],matching_round,neighbor_of_prenode,size_of_neighbor_of_prenode,number_of_matching[p],index_of_snapshot,patternGraph->getNum_of_neighbor(),patternGraph->getOrder(),graph);

        args[p].data = dataPassingToThreads[p];
    }

    return args;
}

int matchingEngine::receiving_the_data(DataForPassingBack* ptr_get,int round_index){
    int counter=0;
    node_of_matching.clear();

    for(int p=0;p<number_of_thread;p++){
        counter+=ptr_get[p].number_of_matching_node;
        node_of_matching.insert(node_of_matching.end(),ptr_get[p].matching_node.begin(),ptr_get[p].matching_node.end());
    }
    number_of_node_for_last_matching=counter;

//    //testing
//    cout<<"the testing"<<endl;
//    for(int t=0;t<node_of_matching.size();t++){
//        cout<<node_of_matching[t]<<" ";
//    }
//    cout<<endl;

    return counter;
}

vector<int> matchingEngine::getNode_of_matching(){
    return node_of_matching;
}

void matchingEngine::Round_cleaner(int matching_round){
    int id=patternGraph->getOrder()[matching_round];

    for(int d=0;d<number_of_thread;d++){
        if(passing_node_to_thread_of_each[d]!=nullptr){
            delete [] passing_node_to_thread_of_each[d];
        }
    }
    delete [] passing_node_to_thread_of_each;

    if(patternGraph->getNeighbor_restriction()[id].size()!=0){
        delete [] neighbor_of_prenode;
    }

    if(number_of_matching!=nullptr){
        delete [] number_of_matching;
    }

    delete [] args;
    delete [] dataPassingToThreads;
}

void matchingEngine::Matching_ss0() {
    pthread_t tid[number_of_thread];

    auto start = system_clock::now();

    for(int i=0;i<patternGraph->getNode();i++){
        ThreadData * arggs;
        arggs=get_the_data_prepared(i,0);

        for(int p=0;p<number_of_thread;p++){
            pthread_create(&tid[p], NULL, graph_matching_threads, &arggs[p]);
        }

        //get vectors in each thread and merge them together
        DataForPassingBack* ptr_get=new DataForPassingBack[number_of_thread];
        void ** ptr=new void * [number_of_thread];

        for (int p = 0; p < number_of_thread; p++) {
            pthread_join(tid[p], &(ptr[p]));
            ptr_get[p]=*((DataForPassingBack*) (ptr[p]));
        }

        receiving_the_data(ptr_get,i);

        delete [] ptr;

        Round_cleaner(i);

        delete [] ptr_get;
    }
    current_matching = removeDuplicatesMatching(node_of_matching, patternGraph->getNode()); //get the final matching

    auto end = system_clock::now();
    auto duration= duration_cast<microseconds>(end-start);

    cout<<"time of matching for the snapshot#"<<0<<" is: "<<double (duration.count())*microseconds ::period ::num/microseconds::period::den<<endl;
}

vector< vector<int> > matchingEngine::removeDuplicatesMatching(vector<int>& input, int size) {
    set< vector<int> > uniqueSet;

    for (int i = 0; i < input.size(); i += size) {
        vector<int> row(input.begin() + i, input.begin() + i + size);
        uniqueSet.insert(row);
    }

    vector< vector<int> > output(uniqueSet.begin(), uniqueSet.end());

    return output;
}

void matchingEngine::Matching_ssi() {
    vector<int> edges_of_pattern=patternGraph->getEdges_Numbers();
    for(int i=1;i<graph->getNumber_of_ss();i++){

        auto start = system_clock::now();

        //get the matching adding
        addingEdges->Matching_Add(i);
        vector <int> adding_matching=addingEdges->getMatching_of_addings();

        //delete the matching
        vector <int> after_deleting_matching;
        after_deleting_matching.clear();
        for(int j=0;j<current_matching.size();j++){
            bool status= false;
            for(int k=0;k<patternGraph->getEdge();k++){
                status=graph->check_deleting_status(current_matching[j][edges_of_pattern[k*2]],current_matching[j][edges_of_pattern[k*2+1]],i);
                if(status){
                    break;
                }
            }
            if(!status){
                after_deleting_matching.insert(after_deleting_matching.end(),current_matching[j].begin(),current_matching[j].end());
            }
        }

        //merging adding part
        after_deleting_matching.insert(after_deleting_matching.end(),adding_matching.begin(),adding_matching.end());

        //get the current matching
        current_matching=removeDuplicatesMatching(after_deleting_matching, patternGraph->getNode());

        auto end = system_clock::now();
        auto duration= duration_cast<microseconds>(end-start);

        cout<<"time of matching for the snapshot#"<<i<<" is: "<<double (duration.count())*microseconds ::period ::num/microseconds::period::den<<endl;

    }

    return;
}

