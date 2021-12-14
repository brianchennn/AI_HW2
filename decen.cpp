#include <unistd.h>
#include <iostream>
#include <fstream>
#include <cstring>

#include <string.h>
#include <stdio.h>
#include <string>
#include <algorithm>
#include <vector>
#include <cstdlib>
#include <float.h>

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

#include <pthread.h>
#include <mutex>
#include <thread>
using std::cout; using std::endl;
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::seconds;
using std::chrono::system_clock;
# define PAD 64
using namespace std;

auto t3 = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();

typedef struct{
    string np;
    int g1;
    string n;
}triplet;

typedef struct node{
	string ID;
	double g;
	double h;
    double f;
	bool visited;
	node* previous;
    bool open;
    bool closed;
    pthread_mutex_t mut;
}node;
typedef struct{
	string start;
	double end1;
	double end2;
	double end3;
}heuristic;

typedef struct{
	string start;
	string end;
	double distance;
	double speed_limit;
}edge;

static vector<node> nodes;
static vector<edge> edges;
static string start;
static string endd;

bool mask[32][PAD] = {1};
pthread_mutex_t lo,li;

double incumbent_cost = DBL_MAX;

double find_dist(vector<edge> edges, node *a, node *b){
	for(int i=0;i<edges.size();i++){
		if(edges[i].start == a->ID && edges[i].end == b->ID)
			return edges[i].distance;
	}
	return DBL_MAX;
}

node* find_n(string a){
    for(int i=0;i<nodes.size();i++){
        if(nodes[i].ID == a){
            return &nodes[i];
        }
    }
    return NULL;
}
unsigned long long rdtsc(){
    unsigned int lo,hi;
    __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
    return ((unsigned long long)hi << 32) | lo;
}

bool find_closed(vector<node*> &closed, string np){
    for(int i=0;i<closed.size();i++){
        if(closed[i]->ID == np)
            return true;
    }
    return false;
}

bool find_open(vector<node*> &open, string np){
    for(int i=0;i<open.size();i++){
        if(open[i]->ID == np)
            return true;
    }
    return false;
}

vector<triplet> buffer[10];
bool finish = false;
void *job(void *args){
    int rank = *(int *)args;
    cout<<rank<<endl;
	int pthread_num = *((int *)args + 1);
    
    vector<node*> open, closed;
    if(rank==0){
        node* n=find_n(start);
        open.push_back(n);
    }
	while(1){
        if(finish) return NULL;
        while(buffer[rank].size() != 0){
            triplet t = buffer[rank].at(0);
            pthread_mutex_lock(&li);
            buffer[rank].erase(buffer[rank].begin());
            pthread_mutex_unlock(&li);
            node* np = find_n(t.np);
            //pthread_mutex_lock(&np->mut);
            if(find_closed(closed, t.np)){
                if(t.g1<np->g){
                    //np->closed=0; 
                    //np->open=1; 
                    open.push_back(np);
                    for(int i=0;i<closed.size();i++){
                        if(closed[i]->ID == t.np){
                            closed.erase(closed.begin()+i);
                            break;
                        }
                    }
                }
                else{
                    continue;
                }
            }
            else{
                if(!find_open(open, t.np)){
                    //np->open=1;
                    open.push_back(np);
                }
                else if(t.g1>=np->g){
                    continue;
                }
            }
            np->g=t.g1;
            np->f=np->g+np->h;
            node* nn = find_n(t.n);
            np->previous=nn;
           // pthread_mutex_unlock(&np->mut);
        }

        if(open.size()==0){
            continue;
        }
        
        double min = DBL_MAX;
        string id;
        int idx=-1;
        // find smallest f from open
        for(int i=0;i<open.size();i++){
            if(open[i]->f<min){
                min=open[i]->f;
                id=open[i]->ID;
                idx=i;
            }
        }
        if(min>=incumbent_cost) continue;


        node* n = find_n(id);
        n->visited = 1;
        closed.push_back(n);
        open.erase(open.begin()+idx);
        //n->closed=1;
        if(n->ID==endd){
            if(n->g<incumbent_cost){
                //pthread_mutex_lock(&li);
                incumbent_cost=n->g;
                finish = true;
                //pthread_mutex_unlock(&li);
                printf("cost: %lf\n", incumbent_cost);
                auto t4 = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
                printf("\n %.3lf sec \n\n",(double)(t4 - t3)/1000);
                return NULL;
            }
        }
        else if(n->g+n->h>incumbent_cost){
            continue;
        }
        for(int i=0;i<edges.size();i++){
            if(edges[i].start==n->ID){
                //pthread_mutex_lock(&n->mut);
                double g1=n->g + edges[i].distance;
                triplet t;
                t.np=edges[i].end;
                t.g1=g1;
                t.n=n->ID;
                //pthread_mutex_unlock(&n->mut);
                //cout<<rand_r(&seed)%pthread_num<<endl;
                unsigned int seed = rdtsc();
                pthread_mutex_lock(&li);
                buffer[rand_r(&seed)%pthread_num].push_back(t);
                pthread_mutex_unlock(&li);
            }
        }
    }
    return NULL;
}


int main(int argc, char *argv[]){

	auto t1 = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
	if(argc != 4)
		perror("Usage : ./main [start] [end] [num of thread]\n");
	const int pthread_num = atoi(argv[3]);
	pthread_t t[pthread_num];
	printf("\n===== Run program with %d thread =====\n",pthread_num);
	printf("\n[Reading csv file and generate nodes table] \n");
	
	start = argv[1];
	endd = argv[2];
	char str[128];
	FILE *fp = fopen("edges.csv","r");
	edge e;
	fgets(str,256,fp);

	while(fgets(str,256,fp)){
		const char s[2] = ",";
		char *token = strtok(str, s);
		string tmp1(token);
		e.start = tmp1;
		token = strtok(NULL, s);
		string tmp2(token);
		e.end = tmp2;
		token = strtok(NULL, s);
		e.distance = atof(token);
		token = strtok(NULL, s);
		e.speed_limit = atof(token);
		edges.push_back(e);
	}
	fclose(fp);

	FILE *fp2 = fopen("heuristic.csv","r");
	vector<heuristic> heuristics;
	heuristic heu;
	fgets(str,256,fp2);
	while(fgets(str,256,fp2)){
		const char s[2] = ",";
		char *token = strtok(str, s);
		string tmp1(token);
		heu.start = tmp1;
		token = strtok(NULL, s);
		heu.end1 = atof(token);
		token = strtok(NULL, s);
		heu.end2 = atof(token);
		token = strtok(NULL, s);
		heu.end3 = atof(token);
		heuristics.push_back(heu);
	}
	fclose(fp2);

	for(int i = 0 ; i < heuristics.size(); i++){
		node n;
		n.ID = heuristics[i].start;
		n.g = 0;
		if(endd == "1079387396")
			n.h = heuristics[i].end1;
		else if(endd == "1737223506")
			n.h = heuristics[i].end2;
		else if(endd == "8513026827")
			n.h = heuristics[i].end3;
		n.visited = 0;
		n.previous = NULL;
        n.open = 0;
        n.closed = 0;
        pthread_mutex_init(&n.mut,NULL);
		nodes.push_back(n);
	}
	auto t2 = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
	printf("\n %.3lf sec\n\n", (double)(t2 - t1)/1000);

	printf("[Starting A* algorithm]\n");
	t3 = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();

	node *root;
	for (int i = 0 ; i < nodes.size(); i++){
		if(nodes[i].ID == start){
			root = &nodes[i];
			break;
		}
	}
    root->open = 1;
    pthread_mutex_init(&lo, NULL);
    pthread_mutex_init(&li, NULL);
	struct Args{
		int rank;
		int pthread_num;
	};
	for (int i = 0 ; i < pthread_num ; i++){
        mask[i][0] = 1;
    }
    struct Args args[10];
	for (int i = 0 ; i < pthread_num ; i++){
        args[i].rank = i;
		args[i].pthread_num = pthread_num;
        //cout<<"create"<<endl;
		pthread_create(&t[i], NULL, &job, &args[i]);
	}
	for (int i = 0 ; i < pthread_num ; i++){
        //cout<<"join"<<endl;
		pthread_join(t[i], NULL);
	}
    pthread_mutex_destroy(&lo);
    pthread_mutex_destroy(&li);
    for(int i = 0 ; i < nodes.size(); i++){
        pthread_mutex_destroy(&(nodes[i].mut));
    }
	auto t4 = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
	printf("\n %.3lf sec \n\n",(double)(t4 - t3)/1000);
	
	printf("[Calculating statistics Data]\n");
	auto t5 = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
	vector<string> path;
	double dist = 0.f;
	if(incumbent_cost == DBL_MAX){
		printf("Fail to find a path.\n");
	}else{
        node *cur;
        for(int i = 0 ; i < nodes.size(); i++){
            if(nodes[i].ID == endd)
                cur = &nodes[i];
        }
		for (;cur->previous != NULL;){
            cout<<cur->ID<<endl;
			path.push_back(cur->ID);
			dist += find_dist(edges, cur->previous, cur);
			cur = cur->previous;
		}
		int visited_nodes = 0;
		for(int i=0;i<nodes.size();i++){
			if(nodes[i].visited == 1)
				visited_nodes ++;
		}
		reverse(path.begin(), path.end());
		fp = fopen("path2.txt","w");
		for(int i=0;i<path.size();i++){
			fprintf(fp,"%s\n",path[i].c_str());
		}
		fclose(fp);
		auto t6 = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
		printf("\n %.3lf sec \n\n",(double)(t6 - t5)/1000);
		printf("\n==== Statistics =====\n\n");
		printf("Visited_nodes: %d\n",visited_nodes);
		printf("Total distance: %.3lf m\n",dist);
        printf("Execution time: %.3lf sec\n", (double)(t6-t1)/1000);
	}
	
}
