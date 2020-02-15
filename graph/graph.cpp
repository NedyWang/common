/**
 *  Basic Graph algorithm.
 *  @author wgsmail@163.com
 *  @date   2020/02/15
 */

#include <deque>
#include <iostream>
#include <vector>

struct Graph {
    Graph(int nodes_num) {
        neighbor.resize(nodes_num);
        indegree.resize(nodes_num);
        outdegree.resize(nodes_num);
    }

    void put(int s, int e);

    std::vector<std::vector<int>> neighbor;
    std::vector<int> indegree;
    std::vector<int> outdegree;
};

void Graph::put(int s, int e) {
    neighbor[s].push_back(e);
    ++indegree[s];
    ++outdegree[e];
}

enum { WHITE, GRAY, BLACK };

// DFS graph from v
void dfs(const Graph& g, int v, std::vector<int>* visited) {
    if (visited->at(v) != WHITE) {
        return;
    }

    visited->at(v) = GRAY;
    std::cout << v << " - ";

    for (auto n : g.neighbor[v]) {
        if (visited->at(n) == WHITE) {
            dfs(g, n, visited);
        }
    }

    visited->at(v) = BLACK;
}

// topo sort. echo topo sort if no cycle fine, else print impossible
// Reference:
//  https://leetcode.com/problems/course-schedule-ii/solution/
bool topo_sort_dfs(const Graph& g, int v, std::vector<int>* visited,
                   std::vector<int>* list) {
    if (visited->at(v) == GRAY) {
        return false;
    }

    visited->at(v) = GRAY;

    for (auto n : g.neighbor[v]) {
        if (visited->at(n) == WHITE) {
            topo_sort_dfs(g, n, visited, list);
        } else if (visited->at(n) == GRAY) {
            return false;
        }
    }

    visited->at(v) = BLACK;
    list->push_back(v);

    return true;
}

void topo_sort_dfs(const Graph& g) {
    std::vector<int> list;
    std::vector<int> visited;
    visited.resize(g.neighbor.size());

    for (int i = 0; i < g.neighbor.size(); ++i) {
        if (visited[i] == WHITE && !topo_sort_dfs(g, i, &visited, &list)) {
            std::cout << "impossbile" << std::endl;
            return;
        }
    }

    for (auto it = list.rbegin(); it != list.rend(); ++it) {
        std::cout << *it << " ";
    }

    std::cout << std::endl;
}

// topo sort with indegree
void topo_sort_with_indegree(Graph* g) {
    std::deque<int> q;
    for (int i = 0; i < g->neighbor.size(); ++i) {
        if (g->indegree[i] == 0) {
            q.push_back(i);
        }
    }

    if (q.empty()) {
        std::cout << "impossible" << std::endl;
    }

    std::vector<int> list;

    while (!q.empty()) {
        int v = q.front();
        q.pop_front();

        list.push_back(v);

        for (auto n : g->neighbor[v]) {
            if (--g->indegree[n] == 0) {
                q.push_back(n);
            }
        }
    }

    if (list.size() < g->neighbor.size()) {
        std::cout << "impossible" << std::endl;
        return;
    }

    for (auto v : list) {
        std::cout << v << " ";
    }

    std::cout << std::endl;
}
