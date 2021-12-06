import csv

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
    
def astar(start = 2270143902, end = 1079387396):
    edges = pd.read_csv('./edges.csv').astype(float)
    nodes = pd.DataFrame(edges)['start'].tolist()
    nodes.extend(pd.DataFrame(edges)['end'].tolist())
    nodes = list(dict.fromkeys(nodes)) # remove duplicate
    edges = edges.set_index('start')
    nodes = pd.DataFrame(nodes,columns=['node']).astype(float)
    nodes = nodes.set_index('node')
    heuristics = pd.DataFrame(pd.read_csv('./heuristic.csv'),columns=['node', str(end)]).set_index('node').astype(float)
    
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
    while distance:
        distance.sort(key=lambda x: x[1]+x[2])
        s = distance[0][0]
        d = distance[0][1]
        if nodes.at[int(s),'visited'] == 0:
            nodes.at[int(s),'visited'] = 1
            nodes.at[int(s),'previous'] = distance[0][3] 
        distance.pop(0)
        edge = edges.loc[s,:]
        if not isinstance(edge, pd.DataFrame):
            edge = edge.to_frame().transpose()
        for index,edg in edge.iterrows():
            if(nodes.at[edg['end'],'visited'] == 0):
                e = edg['end']
                distance.append([e, d + edg['distance'], heuristics.at[int(e),str(end)] , s])
                dist = d + edge['distance']
                if e == end:
                    nodes.at[end,'previous'] = s
                    distance.clear()
                    break
    num_visited = sum(list(visited.values()))-1
    path = []
    path.append(end)
    current_node = end
    while nodes.at[current_node,'previous'] != 0:
        path.append(nodes.at[current_node,'previous'])
        current_node = nodes.at[current_node,'previous']
    path.reverse()
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