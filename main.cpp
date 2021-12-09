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

static vector<node *> OPEN;
static vector<node *> CLOSED;
static vector<node> nodes;
static vector<edge> edges;
static string start;
static string endd;

# define pthread_num 4
# define PAD 64
pthread_t t[pthread_num];
bool mask[pthread_num][PAD] = {1};
pthread_t root_tid = 0;
pthread_mutex_t lo,li;
pthread_mutex_t l1;
pthread_mutex_t l[8];

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

void *job(void *Rank){
    int rank = *(int *)Rank;
	while(1){
        //printf("%d\n",OPEN.size());
        //if(rank != 0)
        //    printf("Rank %d\n",rank);
        int cont_flag = 0;
        if(mask[rank][0] == 0){
            printf("rank %d finished\n",rank);
            return NULL;
        }
        int is_empty = 1;
        for (int i = 0 ; i < nodes.size() ; i++){
            if(nodes[i].open == 1){
                is_empty = 0;
                break;
            }
        }
		//if(OPEN.size() == 0){
        if(is_empty){
            int accu = 0;
            for (int i = 0 ; i < pthread_num ; i++){
                if(mask[i][0] == 0) accu++;
            }
            if(accu == 1) return NULL;
            else continue;
        }
        node *n;
        pthread_mutex_lock(&lo);
        double min = DBL_MAX;
        for (int i = 0 ; i < nodes.size(); i++){
            if(nodes[i].g + nodes[i].h < min && nodes[i].open == 1){
                min = nodes[i].g + nodes[i].h;
                n = &nodes[i];
            }
        }
		//sort(OPEN.begin(), OPEN.end(), compare);
		//node *n = OPEN[0];
        //for (int i = 0 ; i < OPEN.size() - 1 ; i++)
            //OPEN[i] = OPEN[i+1];
		//OPEN.pop_back();
        //pthread_mutex_unlock(&lo);
        n->visited = 1;
		if(n->g + n->h >= incumbent_cost){
            mask[rank][0] = 0;
            cont_flag = 1;
        }
        if(n->ID == endd){
			if(n->g < incumbent_cost){
                //pthread_mutex_lock(&li);
				incumbent_cost = n->g;
                //pthread_mutex_unlock(&li);
            }
            printf("mask to 0\n");
            mask[rank][0] = 0;
            cont_flag = 1;
		}
        //pthread_mutex_lock(&lo);
		//CLOSED.push_back(n);
        n->open = 0;
        n->closed = 1;
        pthread_mutex_unlock(&lo);
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
                    pthread_mutex_lock(&lo);
					double g1 = n->g + edges[i].distance;
					if(neighbor->closed == 1){
						if(g1 < neighbor->g){
                            //pthread_mutex_lock(&lo);
							//OPEN.push_back(neighbor);
                            neighbor->open = 1;
                            neighbor->closed = 0;
							CLOSED.erase(find(CLOSED.begin(),CLOSED.end(),neighbor));
                            //pthread_mutex_unlock(&lo);
						}else{
							flag = 1;
						}
					}else{	
						if(neighbor->open == 0){
                            //pthread_mutex_lock(&lo);
                            neighbor->open = 1;
							//OPEN.push_back(neighbor);
                            //pthread_mutex_unlock(&lo);
						}else if(g1 >= neighbor->g){
							flag = 1;
						}
					}
					if(flag == 0){
						neighbor->g = g1;
                        //neighbor->visited = 1;
						neighbor->previous = n;
					}
                    pthread_mutex_unlock(&lo);
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
	if(argc != 3)
		perror("Usage : ./main [start] [end]");
	printf("\nReading csv file and generate nodes table...\n");
	
	start = argv[1];
	endd = argv[2];
	char str[128];
	FILE *fp = fopen("edges.csv","r");
	//vector<edge> edges;
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
	fp = fopen("heuristic.csv","r");
	vector<heuristic> heuristics;
	heuristic heu;
	fgets(str,256,fp);
	while(fgets(str,256,fp)){
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
	fclose(fp);
	//vector<node> nodes;
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
	printf(" %.3lfs used\n\n", (double)(t2 - t1)/1000);


	printf("Starting A* algorithm...\n");
	auto t3 = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();

	//vector<node *> OPEN;
	//vector<node *> CLOSED;
	//double incumbent_cost = DBL_MAX;
	node *root;
	for (int i = 0 ; i < nodes.size(); i++){
		if(nodes[i].ID == start){
			root = &nodes[i];
			break;
		}
	}
	//OPEN.push_back(root);
    root->open = 1;
	//node *n;
    pthread_mutex_init(&lo, NULL);
    pthread_mutex_init(&li, NULL);
	// OPEN, CLOSED, nodes, edges, 
    int rank[pthread_num];
	for (int i = 0 ; i < pthread_num ; i++){
        mask[i][0] = 1;
    }
	for (int i = 0 ; i < pthread_num ; i++){
        rank[i] = i;
		pthread_create(&t[i], NULL, &job, &rank[i]);
        sleep(1);
	}
    root_tid = t[0];
	for (int i = 0 ; i < pthread_num ; i++){
        printf("join\n");
		pthread_join(t[i], NULL);
	}
    pthread_mutex_destroy(&lo);
    pthread_mutex_destroy(&li);
    for(int i = 0 ; i < nodes.size(); i++){
        pthread_mutex_destroy(&(nodes[i].mut));
    }
		auto t4 = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
	printf(" %.3lfs used.\n\n",(double)(t4 - t3)/1000);
	
	printf("Calculate statistic data...\n");
	auto t5 = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
	vector<string> path;
	double dist = 0.f;
	if(incumbent_cost == DBL_MAX){
		printf("fail\n");
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
		printf(" %.3lf used.\n\n\n",(double)(t6 - t5)/1000);
		printf("A* Success\n");
		printf("Visited_nodes: %d\n",visited_nodes);
		printf("Total distance: %lf\n",dist);
	}
	
}
