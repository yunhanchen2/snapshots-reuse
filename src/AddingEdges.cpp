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

int begin_edge=0;
int end_edge=0;
int begin_position=0;
int end_position=0;

pthread_mutex_t mu;

vector<int> processDuplicateVector(vector<int>& input, int size) {
    set< vector<int> > uniqueSet;

    for (int i = 0; i < input.size(); i += size) {
        vector<int> row(input.begin() + i, input.begin() + i + size);
        uniqueSet.insert(row);
    }

    vector< vector<int> > output(uniqueSet.begin(), uniqueSet.end());

    vector <int> return_num;
    return_num.clear();

    for(int i=0;i<output.size();i++){
        for(int j=0;j<size;j++){
            return_num.push_back(output[i][j]);
        }
    }

    return return_num;
}

vector<int> getThePureNode(vector<int>& input){
    set<int> uniqueSet;
    for (int num : input) {
        uniqueSet.insert(num);
    }
    vector<int> output(uniqueSet.begin(), uniqueSet.end());
    return output;
}

static vector<int> vectors_intersection(vector<int> v1,vector<int> v2){
    vector<int> v;
    sort(v1.begin(),v1.end());
    sort(v2.begin(),v2.end());
    set_intersection(v1.begin(),v1.end(),v2.begin(),v2.end(),back_inserter(v));//求交集
    return v;
}

void * graph_matching_threads_add(void * n){
    ThreadData_Add * dataT=(ThreadData_Add*) n;
    DataPassingToThreads_Add * dataPassingToThreads=dataT->data;

    DataForPassingBack_Add * passingBack=new DataForPassingBack_Add();

    passingBack->number_of_matching_node=0;

    int round=dataPassingToThreads->index_of_snapshot;

    CSRGraph * graph=dataPassingToThreads->graph;

    int *tem;
    for(int i=0;i<dataPassingToThreads->number_of_matching;i++){
        tem=new int[dataPassingToThreads->round_index];
        for(int j=0;j<dataPassingToThreads->round_index;j++){
            tem[j]=dataPassingToThreads->passing_node_to_thread_of_each[i*dataPassingToThreads->round_index+j];
        }

        vector<int> back;
        vector< vector<int> > neibor(dataPassingToThreads->size_of_neighbor_of_prenode_pattern);

        if(dataPassingToThreads->size_of_neighbor_of_prenode_pattern==0){
            for(int t=0;t<graph->getNode();t++){
                back.push_back(graph->getTrue_index()[t]);
            }
        }

        for(int m=0;m<dataPassingToThreads->size_of_neighbor_of_prenode_pattern;m++) {//将邻居放入vector中
            vector<int> neighbor = graph->getTheNeighbor(tem[dataPassingToThreads->neighbor_of_prenode_pattern[m]],round);
            for (int r = 0; r < neighbor.size(); r++) {
                neibor[m].push_back(neighbor[r]);//放入的是对应的编号而非第几个
            }

//                //testing
//                pthread_mutex_lock(&mu);
//                cout<<"this is the neighbor"<<endl;
//                for(int r=0;r<neighbor.size();r++){
//                    cout<<neighbor[r]<<" ";
//                }
//                cout<<endl;
//                pthread_mutex_unlock(&mu);

            //join the vector
            if (m == 0) {
                back = neibor[0];
            } else {
                back = vectors_intersection(back, neibor[m]);
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

    pthread_exit(passingBack);
}

AddingEdges::AddingEdges(int aNumber_of_thread,CSRGraph* aGraph,PatternGraph * aPatternGraph){
    number_of_thread=aNumber_of_thread;
//    args=new ThreadData_Add[aNumber_of_thread];
    graph=aGraph;
    patternGraph=aPatternGraph;
    number_of_node_for_last_matching=aGraph->getNode();
}

ThreadData_Add * AddingEdges::get_the_add_data_prepared(int matching_round,int index_of_snapshot,int index_of_edge){
    pthread_mutex_init(&mu, NULL);

    args=new ThreadData_Add[number_of_thread]();
    //preparing data
    //query graph中的neighbor限制
    int size_of_neighbor_of_prenode=0;

    if(patternGraph->getMatching_Restriction()[index_of_edge][matching_round-2].size()!=0){
        neighbor_of_prenode=new int[patternGraph->getMatching_Restriction()[index_of_edge][matching_round-2].size()]();
        for(int p=0;p<patternGraph->getMatching_Restriction()[index_of_edge][matching_round-2].size();p++){
            neighbor_of_prenode[p]=patternGraph->getMatching_Restriction()[index_of_edge][matching_round-2][p];
            size_of_neighbor_of_prenode++;
        }
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
    dataPassingToThreads=new DataPassingToThreads_Add * [number_of_thread];
    number_of_matching=new int[number_of_thread]();

    //prepare for the data
    for(int p=0;p<number_of_thread; p++){
        if(p<remaining){
            number_of_matching[p]=full_node_for_each_thread;
        }else {
            number_of_matching[p]=full_node_for_each_thread-1;
        }

        if(matching_round!=0){
            passing_node_to_thread_of_each[p]=new int[number_of_matching[p]*matching_round];
            for(int t=0;t<number_of_matching[p];t++){
                for(int k=0;k<matching_round;k++){
                    passing_node_to_thread_of_each[p][t*matching_round+k]=node_of_matching[matching_round*sharing_node_ptr+k];
                }
                sharing_node_ptr++;
            }
        }

        dataPassingToThreads[p]=new DataPassingToThreads_Add(passing_node_to_thread_of_each[p],matching_round,neighbor_of_prenode,size_of_neighbor_of_prenode,number_of_matching[p],index_of_snapshot,patternGraph->getNum_of_neighbor(),patternGraph->getOder_of_matching()[index_of_edge],graph);

        args[p].data = dataPassingToThreads[p];
    }

    return args;
}


int AddingEdges::receiving_the_add_data(DataForPassingBack_Add *ptr_get, int round_index) {
    int counter=0;
    node_of_matching.clear();

    for(int p=0;p<number_of_thread;p++){
        counter+=ptr_get[p].number_of_matching_node;
        node_of_matching.insert(node_of_matching.end(),ptr_get[p].matching_node.begin(),ptr_get[p].matching_node.end());
    }
    number_of_node_for_last_matching=counter;

//        //testing
//    cout<<"the testing"<<endl;
//    for(int t=0;t<node_of_matching.size();t++){
//        cout<<node_of_matching[t]<<" ";
//    }
//    cout<<endl;

    return counter;
}

void AddingEdges::Round_cleaner(int matching_round,int edge_index){
    int id=patternGraph->getOder_of_matching()[edge_index][matching_round];

    for(int d=0;d<number_of_thread;d++){
        if(passing_node_to_thread_of_each[d]!=nullptr){
            delete [] passing_node_to_thread_of_each[d];
        }
    }
    delete [] passing_node_to_thread_of_each;

    if(patternGraph->getMatching_Restriction()[edge_index][id-2].size()!=0){
        delete [] neighbor_of_prenode;
    }

    if(number_of_matching!=nullptr){
        delete [] number_of_matching;
    }

    delete [] args;
    delete [] dataPassingToThreads;
}

void AddingEdges::Matching_Add(int ss_index) {
    pthread_t tid[number_of_thread];

    vector<int> matching_of_adding_temp;
    matching_of_addings.clear();
    matching_of_adding_temp.clear();

//    //testing
//    cout<<"this is the "<<ss_index<<"matching:"<<endl;

    vector <int> Adding_node = getThePureNode(graph->getSS_adding_edge()[(ss_index-1)*2]);
    for(int i=0;i<patternGraph->getEdge();i++){ //ith matching order

//        //testing
//        cout<<"this is the "<<i<<"th matching order"<<endl;

        int begin_p=patternGraph->getOder_of_matching()[i][0];
        int end_p=patternGraph->getOder_of_matching()[i][1];

        //get position
        for(int q=0;q<patternGraph->getNode();q++){
            if(begin_p==patternGraph->getOrder()[q]){
                begin_position=q;
            }
        }
        for(int q=0;q<patternGraph->getNode();q++){
            if(end_p==patternGraph->getOrder()[q]){
                end_position=q;
            }
        }

        int begin_degree=patternGraph->getNum_of_neighbor()[begin_p];
        int end_degree=patternGraph->getNum_of_neighbor()[end_p];

        int num_of_add=graph->getSS_adding_edge()[(ss_index-1)*2].size();

        for(int j=0;j<num_of_add;j++){
            int _b_v=graph->getSS_adding_edge()[(ss_index-1)*2][j];
            int _e_v=graph->getSS_adding_edge()[(ss_index-1)*2+1][j];

            vector <int> neighbor_b=graph->getTheNeighbor(_b_v,ss_index);
            vector <int> neighbor_e=graph->getTheNeighbor(_e_v,ss_index);

            if(neighbor_b.size()>=begin_degree&&neighbor_e.size()>=end_degree){
                begin_edge=_b_v;
                end_edge=_e_v;

//                //testing
//                cout<<"the begin and the end: "<<begin_edge<<" "<<end_edge<<endl;

                //get the first round matching
                vector <int> candidate;
                candidate.clear();
//                cout<<"the restriction number is: "<<patternGraph->getMatching_Restriction()[i][0].size()<<"and the restriction are: "<<endl;
//                for(int t=0;t<patternGraph->getMatching_Restriction()[i][0].size();t++){
//                    cout<<patternGraph->getMatching_Restriction()[i][0][t]<<" ";
//                }
//                cout<<endl;

                if(patternGraph->getMatching_Restriction()[i][0].size()==0){
                    for(int t=0;t<graph->getNode();t++){
                        candidate.push_back(graph->getTrue_index()[t]);
                    }
                } else if(patternGraph->getMatching_Restriction()[i][0].size()==1){
                    if(patternGraph->getMatching_Restriction()[i][0][0]==0){
                        candidate=graph->getTheNeighbor(begin_edge,ss_index);
                    } else if(patternGraph->getMatching_Restriction()[i][0][0]==1){
                        candidate=graph->getTheNeighbor(end_edge,ss_index);
                    }
                } else if(patternGraph->getMatching_Restriction()[i][0].size()==2){
                    candidate = vectors_intersection(graph->getTheNeighbor(begin_edge,ss_index),graph->getTheNeighbor(end_edge,ss_index));
                }

                int degree_2=patternGraph->getNum_of_neighbor()[patternGraph->getOder_of_matching()[i][2]];

                node_of_matching.clear();
                number_of_node_for_last_matching=0;

                for(int k=0;k<candidate.size();k++){
                    if((graph->getTheNeighbor(candidate[k],ss_index).size())>=degree_2&&candidate[k]!=begin_edge&&candidate[k]!=end_edge){
                        node_of_matching.push_back(begin_edge);
                        node_of_matching.push_back(end_edge);
                        node_of_matching.push_back(candidate[k]);
                        number_of_node_for_last_matching++;
                    }
                }

//                //testing
//                cout<<"the matching is: ";
//                for(int t=0;t<number_of_node_for_last_matching;t++){
//                    cout<<node_of_matching[t*3]<<" "<<node_of_matching[t*3+1]<<" "<<node_of_matching[t*3+2]<<endl;
//                }
//                cout<<endl;

                for(int r=3;r<patternGraph->getEdge();r++){
                    ThreadData_Add * arggs=get_the_add_data_prepared(r,ss_index,i);
                    for(int p=0;p<number_of_thread;p++){
                        pthread_create(&tid[p], NULL, graph_matching_threads_add, &arggs[p]);
                    }

//                    //testing
//                    cout<<"the testing1 in the round#"<<r<<" with the edge#"<<i<<endl;

                    //get vectors in each thread and merge them together
                    DataForPassingBack_Add* ptr_get=new DataForPassingBack_Add[number_of_thread];
                    void ** ptr=new void * [number_of_thread];

                    for (int p = 0; p < number_of_thread; p++) {
                        pthread_join(tid[p], &(ptr[p]));
                        ptr_get[p]=*((DataForPassingBack_Add*) (ptr[p]));
                    }

//                    //testing
//                    cout<<"the testing2 in the round#"<<r<<" with the edge#"<<i<<endl;

                    receiving_the_add_data(ptr_get,r);

//                    //testing
//                    cout<<"the testing3 in the round#"<<r<<" with the edge#"<<i<<endl;

                    delete [] ptr;

                    Round_cleaner(r,i);

                    delete [] ptr_get;
                }

                int size=node_of_matching.size()/patternGraph->getNode();
                int ptr_match=2;
                for(int q=0;q<size;q++){
                    for(int p=0;p<patternGraph->getNode();p++){
                        if(p==begin_p){
                            matching_of_adding_temp.push_back(begin_edge);
                        } else if (p==end_position){
                            matching_of_adding_temp.push_back(end_edge);
                        } else {
                            matching_of_adding_temp.push_back(node_of_matching[q*patternGraph->getNode()+ptr_match]);
                            ptr_match++;
                        }
                    }
                }


            }
        }
    }

    matching_of_addings= processDuplicateVector(matching_of_adding_temp,patternGraph->getNode());

//    cout<<"the matching_of_adding: "<<endl;
//    for(int i=0;i<matching_of_addings.size();i++){
//        cout<<matching_of_addings[i]<<" ";
//    }
//    cout<<endl;

    pthread_mutex_destroy(&mu);
    return;
}