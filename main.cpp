#include <iostream>
#include <fstream>
#include <cstring>

#include <string.h>
#include <stdio.h>
#include <string>

#include <vector>
#include <cstdlib>
#include <float.h>

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

bool compare(node *a, node *b){
	return a->g + a->h > b->g + b->h;
}


int main(){
	int Case;
	printf("Input case: \n");
	scanf("%d", &Case);
	string start, end;
	switch(Case){
		case 1:
			start = "2270143902";
			end = "1079387396";
			break;
		case 2:
			start = "426882161";
			end = "1737223506";
			break;
		case 3:
			start = "1718165260";
			end = "8513026827";
			break;
		default:
			start = "0";
			end = "0";
			printf("wrong case\n");
			break;
	}
	
	char str[128];
	FILE *fp = fopen("edges.csv","r");
	vector<edge> edges;
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
	vector<node> nodes;
	for(int i = 0 ; i < heuristics.size(); i++){
		node n;
		n.ID = heuristics[i].start;
		n.g = 0;
		switch(Case){
			case 1:
				n.h = heuristics[i].end1;
				break;
			case 2:
				n.h = heuristics[i].end2;
				break;
			case 3:
				n.h = heuristics[i].end3;
				break;
			default:
				break;
		}
		n.visited = 0;
		n.previous = NULL;
		nodes.push_back(n);
	}
	vector<node *> OPEN;
	vector<node *> CLOSED;
	double incumbent_cost = DBL_MAX;
	node *root;
	for (int i = 0 ; i < nodes.size(); i++){
		if(nodes[i].ID == start){
			root = &nodes[i];
			break;
		}
	}
	OPEN.push_back(root);
	node *n;
	while(1){
		if(OPEN.size() == 0)
			return 0;
			//continue;
		sort(OPEN.begin(), OPEN.end(), compare);
		n = OPEN[OPEN.size() - 1];
		OPEN.pop_back();
		CLOSED.push_back(n);
		if(n->ID == end){
			if(n->g < incumbent_cost){
				incumbent_cost = n->g;
			}
			break;
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
					double g1 = neighbor->g + edges[i].distance;
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
	vector<string> path;
	if(incumbent_cost == DBL_MAX){
		printf("fail\n");
	}else{
		node *cur = n;
		for (;cur->previous != NULL;){
			path.push_back(cur->ID);
			//cout << cur->ID << endl;
			cur = cur->previous;
		}
		reverse(path.begin(), path.end());
		fp = fopen("path.txt","w");
		for(int i=0;i<path.size();i++){
			fprintf(fp,"%s\n",path[i].c_str());
		}
		fclose(fp);
		printf("success\n");
	}
	
}
