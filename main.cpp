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
# define PAD 64
using namespace std;

typedef struct node{
	string ID;
	double g;
	double h;
	bool visited;
	node *previous;
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
bool compare(node *a, node *b){
	return a->g + a->h < b->g + b->h; // 升冪
}

double find_dist(vector<edge> edges, node *a, node *b){
	for(int i=0;i<edges.size();i++){
		if(edges[i].start == a->ID && edges[i].end == b->ID)
			return edges[i].distance;
	}
	return DBL_MAX;
}

void *job(void *args){
    int rank = *(int *)args;
	int pthread_num = *((int *)args + 1);
	while(1){
        int cont_flag = 0;
        if(mask[rank][0] == 0){
			return NULL; // 表示自己的thread 該結束了
        }
        node *n;
        pthread_mutex_lock(&lo);
        double min = DBL_MAX;
        int is_empty = 1;
        for (int i = 0 ; i < nodes.size(); i++){ // 檢查 OPEN 是否 empty 順便求出最佳的 node
            if(nodes[i].open == 1){
                if(nodes[i].g + nodes[i].h < min){
                    is_empty = 0;
                    min = nodes[i].g + nodes[i].h;
                    n = &nodes[i];
                }
            }
        }
        pthread_mutex_unlock(&lo);
        if(is_empty){
            int accu = 0;
            for (int i = 0 ; i < pthread_num ; i++){
                if(mask[i][0] == 0) accu++;
            }
            if(accu == 1) return NULL; // 如果只剩自己1個 thread => 結束A* algorithm
            else continue; // 如果還有其他 thread => busy waiting
        }

		pthread_mutex_lock(&n->mut);
		if(n->g + n->h >= incumbent_cost){ // OPEN 裡面的都很爛 表示差不多可以降低thread數量了
            mask[rank][0] = 0;
            cont_flag = 1;
        }
        if(n->ID == endd){
			if(n->g < incumbent_cost){ // 最佳路徑
                pthread_mutex_lock(&li);
				incumbent_cost = n->g; // 更新 incumbent_cost
                pthread_mutex_unlock(&li);
            }
            mask[rank][0] = 0;
            cont_flag = 1;
		}
        n->open = 0;
        n->closed = 1;
        n->visited = 1;
        pthread_mutex_unlock(&n->mut);

	    if(cont_flag == 1) continue;	
		int i = 0;
		for( ; i < edges.size(); i++){
			if(edges[i].start == n->ID){
				break;
			}
		}
		for(; i < edges.size() && edges[i].start == n->ID; i++){
			for(int j = 0; j < nodes.size(); j++){
				if(nodes[j].ID == edges[i].end){
					int flag = 0;
					node *neighbor = &nodes[j];
                    pthread_mutex_lock(&neighbor->mut);
					double g1 = n->g + edges[i].distance;
					if(neighbor->closed == 1){
						if(g1 < neighbor->g){
                            neighbor->open = 1;
                            neighbor->closed = 0;
						}else{
							flag = 1;
						}
					}else{	
						if(neighbor->open == 0){
                            neighbor->open = 1;
						}else if(g1 >= neighbor->g){
							flag = 1;
						}
					}
					if(flag == 0){
						neighbor->g = g1;
						neighbor->previous = n;
					}
                    pthread_mutex_unlock(&neighbor->mut);
					break;
				}
			}
		}
	}
    return NULL;
}
using std::cout; using std::endl;
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::seconds;
using std::chrono::system_clock;


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
	auto t3 = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();

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
	for (int i = 0 ; i < pthread_num ; i++){
		struct Args args;
        args.rank = i;
		args.pthread_num = pthread_num;
		pthread_create(&t[i], NULL, &job, &args);
	}
	for (int i = 0 ; i < pthread_num ; i++){
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
		fp = fopen("path.txt","w");
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
