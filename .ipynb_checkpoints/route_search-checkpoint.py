import csv
import time
import threading
from multiprocessing import Process,Lock,Manager
def bfs(start = 2270143902, end = 1079387396):
    with open('edges.csv', newline='') as csvfile:
        edges = csv.DictReader(csvfile)
        nodes = []
        edgess = []
        for edge in edges:
            edge['start'] = int(edge['start'])
            edge['end'] = int(edge['end'])
            edge['distance'] = float(edge['distance'])
            edge['speed limit'] = float(edge['speed limit'])
            nodes.append(edge['start'])
            nodes.append(edge['end'])
            edgess.append(edge)
        nodes = list(dict.fromkeys(nodes))
        nodes.sort()
        visited = {}
        previous = {}
        distance = {}
        for node in nodes:
            visited[node] = 0
            previous[node] = 0
            distance[node] = 0
        queue = []
        queue.append(start)
        visited[start] = 1
        num_visited = 0
        while queue:
            s = queue.pop(0)
            for edge in edgess:
                if edge['start'] == s:
                    if visited[edge['end']] == 0:
                        num_visited += 1
                        e = edge['end']
                        queue.append(e)
                        visited[e] = 1
                        previous[e] = edge['start']
                        distance[e] = edge['distance']
                        if e == end:
                            queue.clear()
                            break
        num_visited = sum(list(visited.values()))-1
        path = []
        dist = 0.0
        path.append(end)
        current_node = end
        while previous[current_node] != 0:
            path.append(previous[current_node])
            dist += distance[current_node]
            current_node = previous[current_node]
        path.reverse()
        return path, dist, num_visited

def dfs(start = 2270143902, end = 1079387396):
    with open('edges.csv', newline='') as csvfile:
        edges = csv.DictReader(csvfile)
        nodes = []
        edgess = []
        for edge in edges:
            edge['start'] = int(edge['start'])
            edge['end'] = int(edge['end'])
            edge['distance'] = float(edge['distance'])
            edge['speed limit'] = float(edge['speed limit'])
            nodes.append(edge['start'])
            nodes.append(edge['end'])
            edgess.append(edge)
        nodes = list(dict.fromkeys(nodes))
        nodes.sort()
        visited = {}
        previous = {}
        distance = {}
        for node in nodes:
            visited[node] = 0
            previous[node] = 0
            distance[node] = 0
        stack = []
        stack.append(start)
        visited[start] = 1
        num_visited = 0
        while stack:
            s = stack.pop(-1)
            for edge in edgess:
                if edge['start'] == s:
                    if visited[edge['end']] == 0:
                        num_visited += 1
                        e = edge['end']
                        stack.append(e)
                        visited[e] = 1
                        previous[e] = edge['start']
                        distance[e] = edge['distance']
                        if e == end:
                            stack.clear()
                            break
        num_visited = sum(list(visited.values()))-1
        path = []
        dist = 0.0
        path.append(end)
        current_node = end
        while previous[current_node] != 0:
            path.append(previous[current_node])
            dist += distance[current_node]
            current_node = previous[current_node]
        path.reverse()
        return path, dist, num_visited

def ucs(start = 2270143902, end = 1079387396):
    with open('edges.csv', newline='') as csvfile:
        edges = csv.DictReader(csvfile)
        nodes = []
        edgess = []
        for edge in edges:
            edge['start'] = int(edge['start'])
            edge['end'] = int(edge['end'])
            edge['distance'] = float(edge['distance'])
            edge['speed limit'] = float(edge['speed limit'])
            nodes.append(edge['start'])
            nodes.append(edge['end'])
            edgess.append(edge)
        nodes = list(dict.fromkeys(nodes))
        nodes.sort()
        visited = {}
        previous = {}
        distance = []
        for node in nodes:
            visited[node] = 0
            previous[node] = 0
        distance.append([start, 0.0, 0])
        visited[start] = 1
        num_visited = 0
        dist = 0.0
        while distance:
            distance.sort(key=lambda x: x[1])
            s = distance[0][0]
            d = distance[0][1]
            p = distance[0][2]
            if visited[s] == 0:
                visited[s] = 1
                previous[s] = p
            distance.pop(0)
            for edge in edgess:
                if edge['start'] == s:
                    if visited[edge['end']] == 0:
                        e = edge['end']
                        distance.append([edge['end'], d+edge['distance'], s])
                        dist = d+edge['distance']
                        if e == end:
                            previous[e] = s
                            distance.clear()
                            break
        num_visited += sum(list(visited.values()))-1
        path = []
        path.append(end)
        current_node = end
        while previous[current_node] != 0:
            path.append(previous[current_node])
            current_node = previous[current_node]
        path.reverse()
        return path, dist, num_visited

import numpy as np
import pandas as pd


global dist,accu_time,for_loop_time,sort_time,mid_time,nodes_query_time,edges_query_time,dist,done,sort_time
global edges, nodes, heuristics, visited, previous


def job(lock1,lock2,distance):
    global dist,accu_time,for_loop_time,sort_time,mid_time,nodes_query_time,edges_query_time,dist,done,sort_time
    global edges, nodes, heuristics, visited, previous
    
    t17 = time.time()
    lock1.acquire()
    distance.sort(key=lambda x: x[1]+x[2])
    t18 = time.time()
    sort_time += t18 - t17
    if(distance == []):
        print(" empty")
        return
    #print(distance)
    s = distance[0][0]
    if(s == end):
        done = 1
    d = distance[0][1]
    best_dist = distance.pop(0)
    lock1.release()
    t50 = time.time()
    lock2.acquire()
    if nodes.at[s,'visited'] == 0:
        nodes.at[s,'visited'] = 1
        nodes.at[s,'previous'] = best_dist[3]
    lock2.release()
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
            lock1.acquire()
            distance.append([e, d + edg['distance'], heuristics.at[e,str(end)] , s])
            lock1.release()
            dist = d + edg['distance']
            if e == end:
                lock2.acquire()
                nodes.at[end,'previous'] = s
                lock2.release()
                #print("clear")
                #done = 1
                #distance.clear()

                break
    t5 = time.time()
    for_loop_time += t5 - t4



def astar(start = 2270143902, end = 1079387396):
    
    global dist,accu_time,for_loop_time,sort_time,mid_time,nodes_query_time,edges_query_time,dist,done,sort_time
    global edges, nodes, heuristics, visited, previous
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
    
    #print(" ===== edges table =====\n",edges)
    #print(" ===== nodes table =====\n",nodes)
    #print(" ===== heuristics table =====\n",heuristics)
    
    
    distance.append([start, 0, heuristics.at[start,str(end)], 0])
    nodes.loc[str(start), 'visited'] = 1
    nodes.loc[str(start), 'previous'] = 1
    num_visited = 0
    #global dist,accu_time,for_loop_time,sort_time,mid_time,nodes_query_time,edges_query_time,dist,done
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
    manager = Manager() # Lock 的 manager
    lock1 = manager.Lock()
    lock2 = manager.Lock()
    done = 0
    
    
    while not done:
        if(len(distance) == 0):
            continue
        t_pool.append(Process(target = job,args=(lock1,lock2,distance)))
        t_pool[-1].start()
        #job()
    for i, t in enumerate(t_pool):
        t_pool[i].join()
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
    
    
    

            
def astar_time(start = 2270143902, end = 1079387396):
    with open('edges.csv', newline='') as csvfile:
        edges = csv.DictReader(csvfile)
        nodes = []
        edgess = []
        for edge in edges:
            edge['start'] = int(edge['start'])
            edge['end'] = int(edge['end'])
            edge['distance'] = float(edge['distance'])
            edge['speed limit'] = float(edge['speed limit'])
            edge['time'] = float(edge['distance'])/float(edge['speed limit'])*3.6
            nodes.append(edge['start'])
            nodes.append(edge['end'])
            edgess.append(edge)
        with open('heuristic.csv', newline='') as csvfile2:
            heuristics = csv.DictReader(csvfile2)
            heuristicss = {}
            for heuristic in heuristics:
                heuristicss[int(heuristic['node'])] = float(heuristic[str(end)])
            nodes = list(dict.fromkeys(nodes))
            nodes.sort()
            visited = {}
            previous = {}
            distance = []
            for node in nodes:
                visited[node] = 0
                previous[node] = 0
            distance.append([start, 0, heuristicss[start], 0])
            visited[start] = 1
            num_visited = 0
            time = 0.0
            while distance:
                distance.sort(key=lambda x: x[1]+x[2])
                s = distance[0][0]
                d = distance[0][1]
                if visited[s] == 0:
                    visited[s] = 1
                    previous[s] = distance[0][3]
                distance.pop(0)
                for edge in edgess:
                    if edge['start'] == s:
                        if visited[edge['end']] == 0:
                            e = edge['end']
                            distance.append([edge['end'], edge['time']+d, heuristicss[edge['end']]/60*3.6, edge['start']])
                            if e == end:
                                time = edge['time']+d
                                previous[end] = edge['start']
                                distance.clear()
                                break
            num_visited = sum(list(visited.values()))-1
            path = []
            path.append(end)
            current_node = end
            while previous[current_node] != 0:
                path.append(previous[current_node])
                current_node = previous[current_node]
            path.reverse()
            return path, time, num_visited