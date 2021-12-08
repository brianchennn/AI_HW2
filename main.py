import numpy as np
import pandas as pd
import time
#import threading

def job():
    #print(len(distance))
    t17 = time.time()
    distance.sort(key=lambda x: x[1]+x[2])
    t18 = time.time()
    sort_time += t18 - t17
    s = distance[0][0]
    d = distance[0][1]
    t50 = time.time()
    if nodes.at[s,'visited'] == 0:
        nodes.at[s,'visited'] = 1
        nodes.at[s,'previous'] = distance[0][3]
    distance.pop(0)
    t51 = time.time()
    nodes_query_time += t51 - t50
    t53 = time.time()
    try:
        edge = edges.loc[int(s),:]
    except:
        return
    if not isinstance(edge, pd.DataFrame): 
        edge = edge.to_frame().transpose() # 轉成 DataFrame
    t54 = time.time()
    edges_query_time += t54 - t53
    t19 = time.time()
    mid_time += t19 - t18
    t4 = time.time()
    for index,edg in edge.iterrows():
        e = int(edg['end'])
        if(nodes.at[e,'visited'] == 0):
            distance.append([e, d + edg['distance'], heuristics.at[e,str(end)] , s])
            if e == end:
                nodes.at[end,'previous'] = s
                dist = d + edg['distance']
                distance.clear()
                break
    t5 = time.time()
    for_loop_time += t5 - t4


def astar(start = 2270143902, end = 1079387396):
    
    edges = pd.read_csv('./edges.csv')
    edges.sort_values(by=['start'])
    edges_start_list = edges['start'].tolist()
    nodes = pd.DataFrame(edges)['start'].tolist() + pd.DataFrame(edges)['end'].tolist()
    edges.set_index(pd.Series(edges_start_list), inplace=True)
    edges.set_index('start', inplace=True)
    nodes = list(dict.fromkeys(nodes)) # remove duplicate
    nodes = pd.DataFrame(nodes,columns=['node'])
    nodes = nodes.set_index('node')
    nodes.sort_index(inplace = True)
    heuristics = pd.DataFrame(pd.read_csv('./heuristic.csv'),columns=['node', str(end)]).set_index('node')
    
    visited = {}
    previous = {}
    distance = []

    nodes['visited'] = 0
    nodes['previous'] = 0
    
    print(" ===== edges table =====\n",edges)
    print(" ===== nodes table =====\n",nodes)
    print(" ===== heuristics table =====\n",heuristics)
    
    
    distance.append([start, 0, heuristics.at[start,str(end)], 0])
    nodes.loc[str(start), 'visited'] = 1
    nodes.loc[str(start), 'previous'] = 1
    num_visited = 0
    dist = 0.0
    nodes_row,nodes_col = nodes.shape
    nodes_index = nodes.index
    print("number of nodes: ",nodes_row)
    accu_time = 0.0
    for_loop_time = 0.0
    sort_time = 0.0
    mid_time = 0.0
    nodes_query_time = 0.0
    edges_query_time = 0.0
    t12 = time.time()
    t_pool = []
    #while distance:
    #    job()
        #t_pool.append(threading.Thread(target = job))
        #t_pool[-1].start()
    #for i,t in enumerate(t_pool):
    #    t_pool[i].join()
    t13 = time.time()
    print("===== Performance =====")
    print("while loop time: ",t13 - t12)
    print("    sorting time", sort_time)
    print("    middle time", mid_time)
    print("      nodes query time", nodes_query_time)
    print("      edges query time", edges_query_time)
    print("    for loop time: ",for_loop_time)
    print("=======================")
    num_visited = nodes['visited'].sum() - 1
    path = []
    path.append(end)
    current_node = end
    t6 = time.time()
    while nodes.at[current_node,'previous'] != 0:
        path.append(nodes.at[current_node,'previous'])
        current_node = nodes.at[current_node,'previous']
    path.reverse()
    t7 = time.time()
    return path, dist, num_visited
    
import folium
import pickle
def load_path_graph(path):
    with open('graph.pkl', 'rb') as f:
        graph = pickle.load(f)

    node_pairs = list(zip(path[:-1], path[1:]))
    lines = []
    for edge in graph:
        if (edge['u'], edge['v']) in node_pairs or  (edge['v'], edge['u']) in node_pairs:
            lines.append(edge['geometry'])
    return lines
from route_search import astar
start = 2270143902; end = 1079387396

import time
st = time.time()
astar_path, astar_dist, astar_visited = astar(start, end)
en = time.time()

print(f'The number of nodes in the path found by A* search: {len(astar_path)}')
print(f'Total distance of path found by A* search: {astar_dist} m')
print(f'The number of visited nodes in A* search: {astar_visited}\n')
print("\nExecution time: ",en-st)

fmap = folium.Map(location=(24.806383132251874, 120.97685775516189), zoom_start=13)
for line in load_path_graph(astar_path):
    fmap.add_child(folium.PolyLine(locations=line, tooltip='astar', weight=4, color='red'))
fmap