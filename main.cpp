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
mutex lo,li;

double incumbent_cost = DBL_MAX;
bool compare(node *a, node *b){
	return a->g + a->h > b->g + b->h;
}

double find_dist(vector<edge> edges, node *a, node *b){
	for(int i=0;i<edges.size();i++){
		if(edges[i].start == a->ID && edges[i].end == b->ID)
			return edges[i].distance;
	}
	return DBL_MAX;
}

void *job(void *args){
	while(1){
        //printf("fwf\n");
		if(OPEN.size() == 0)
			continue;
        //lo.lock();
		sort(OPEN.begin(), OPEN.end(), compare);
		node *n = OPEN[OPEN.size() - 1];
		//n->visited = 1;
		if(n->g + n->h >= incumbent_cost)
			break;
		OPEN.pop_back();
        //lo.unlock();
		CLOSED.push_back(n);
		if(n->ID == endd){
            //li.lock();
			if(n->g < incumbent_cost){
				incumbent_cost = n->g;
			}
            //li.lock();
		}
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
					double g1 = n->g + edges[i].distance;
					vector<node *>::iterator it1 = find(CLOSED.begin(),CLOSED.end(),neighbor);
					vector<node *>::iterator it2 = find(OPEN.begin(),OPEN.end(),neighbor);
					if(it1 != CLOSED.end()){
						if(g1 < neighbor->g){
							OPEN.push_back(neighbor);
							CLOSED.erase(it1);
						}else{
							flag = 1;
						}
					}else{	
						if(it2 == OPEN.end()){
							OPEN.push_back(neighbor);
						}else if(g1 >= neighbor->g){
							flag = 1;
						}
					}
					if(flag == 0){
						neighbor->g = g1;
						neighbor->previous = n;
					}
					break;
				}
			}
		}
	}
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
	OPEN.push_back(root);
	node *n;
	node *best_n;
	// OPEN, CLOSED, nodes, edges, 
	int pthread_num = 1;
	pthread_t t[pthread_num];
	for (int i = 0 ; i < pthread_num ; i++){
		pthread_create(&t[i], NULL, &job, NULL);
	}
	for (int i = 0 ; i < pthread_num ; i++){
		pthread_join(t[i], NULL);
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
