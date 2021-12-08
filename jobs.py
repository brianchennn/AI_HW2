import numpy as np
import pandas as pd
import threading
from multiprocessing import Process,Lock,Manager

def job(ns,lock1,lock2):
    
    #global dist,accu_time,for_loop_time,sort_time,mid_time,nodes_query_time,edges_query_time,dist,done,sort_time
    #global edges, nodes, heuristics, visited, previous,distance,incumbent_cost
    #print("job")
    
    ns.distance.sort(key=lambda x: x[1]+x[2])
    if(ns.distance == []):
        return
    s = ns.distance[0][0]
    d = ns.distance[0][1]
    if(s == ns.end):
        ns.done = 1
        if(ns.incumbent_cost > d):
            d = ns.incumbent_cost
        return
    best_dist = ns.distance[0]
    ns.distance.pop(0)
    if(best_dist[1] + best_dist[2] > ns.incumbent_cost):
        return
    #print(best_dist)
    #if nodes.at[s,'visited'] == 0:
    #    nodes.at[s,'visited'] = 1
    #    nodes.at[s,'previous'] = best_dist[3]

    try:
        edge = ns.edges.loc[int(s),:]
    except:
        return
    if not isinstance(edge, pd.DataFrame): 
        edge = edge.to_frame().transpose() # 轉成 DataFrame

    for index,edg in edge.iterrows():
        e = int(edg['end'])
        if(ns.nodes.at[e,'visited'] == 0):
            #print("fewfew")
            ns.distance.append([e, d + edg['distance'], ns.heuristics.at[e,str(ns.end)] , s])
            ns.nodes.at[e,'visited'] = 1
            ns.nodes.at[e,'g'] = d + edg['distance']
            ns.nodes.at[e,'previous'] = s
            ns.dist = d + edg['distance']
            #if e == end:
            #    nodes.at[end,'previous'] = s
            #    break
        '''else:
            if(nodes.at[e,'g'] > d + edg['distance']):
                distance.append([e, d + edg['distance'], heuristics.at[e,str(end)] , s])
                nodes.at[e,'g'] = d + edg['distance']
                dist = d + edg['distance']
                nodes.at[e,'previous'] = s
            else:
                pass '''
        #lock2.release()


