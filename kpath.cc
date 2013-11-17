/*
 * =====================================================================================
 *
 *       Filename:  kpath.cc
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  11/15/2013 09:47:06 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */


#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <deque>
#include <stack>
#include <queue>
#include <algorithm>
#include <utility>
#include <tuple>

#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstdlib>
#include <cstdio>
#include <chrono>

#ifdef _OPENMP
    #include <omp.h>
#endif

#ifndef MAX_NUM_THREADS
    #define MAX_NUM_THREADS 16
#endif
#define K_LIMIT 4
#ifndef TASK_LIMIT
    #define TASK_LIMIT 32
#endif

#define TID omp_get_thread_num()

using std::size_t;
typedef size_t Id;
typedef std::vector<std::vector<Id>> AdjacencyList;
typedef std::deque<Id> PathType;

#ifdef RECURSIVE
AdjacencyList adj_list;
size_t num_vertices, num_edges;
size_t final_length;
Id start_vertex, finish_vertex;
#ifdef _OPENMP
//    std::vector<std::vector<bool>> visited;
//    std::vector<size_t> num_paths;
    std::vector<bool> visited[MAX_NUM_THREADS];
    size_t num_paths[MAX_NUM_THREADS];

    #define VISITED visited[TID]
    #define NUM_PATHS num_paths[TID]
#else
    std::vector<bool> visited;
    size_t num_paths = 0;

    #define VISITED visited
    #define NUM_PATHS num_paths
#endif // _OPENMP
#endif // RECURSIVE

#ifndef NDEBUG
inline void increase_num_paths(size_t &num_paths, PathType &path) {
#else
inline void increase_num_paths(size_t &num_paths) {
#endif
    ++num_paths;
#ifndef NDEBUG
#ifdef _OPENMP
#pragma omp critical
    {
#endif
    std::cerr << "Path " << std::setw(3) << num_paths << ": ";
    for(auto u : path) std::cerr << std::setw(2) << u << ' ';
    std::cerr << '\n';
#ifdef _OPENMP
    }
#endif
#endif
}

#ifdef RECURSIVE
#ifndef NDEBUG
void backtracking(size_t path_length, Id current_vertex, std::vector<bool> &visited, size_t &num_paths, PathType &path)
#else
void backtracking(size_t path_length, Id current_vertex, std::vector<bool> &visited, size_t &num_paths)
#endif // NDEBUG
{
    for (auto next_vertex : adj_list[current_vertex]) {
        if (!VISITED[next_vertex]) {
#ifndef NDEBUG
            path.push_back(next_vertex);
#endif
            if (path_length == final_length && next_vertex == finish_vertex) {
#ifndef NDEBUG
                increase_num_paths(num_paths, path);
#else
                increase_num_paths(num_paths);
#endif
            } else if (path_length != final_length && next_vertex != finish_vertex) {
                VISITED[next_vertex] = true;
#ifndef NDEBUG
                backtracking(path_length + 1, next_vertex, visited, num_paths, path);    
#else
                backtracking(path_length + 1, next_vertex, visited, num_paths);    
#endif
                VISITED[next_vertex] = false;
            }
#ifndef NDEBUG
            path.pop_back();
#endif
        }
    }
}
#endif // RECURSIVE

#ifndef RECURSIVE
#ifndef NDEBUG
//size_t backtracking_nonrecursive(size_t path_length, Id start_vertex, std::vector<bool> &visited, PathType &path)
size_t backtracking_nonrecursive(AdjacencyList const &adj_list, Id start_vertex, Id finish_vertex, size_t final_length,
                                 size_t current_length, std::vector<bool> &visited, PathType &path)
#else
//size_t backtracking_nonrecursive(size_t path_length, Id start_vertex, std::vector<bool> &visited)
size_t backtracking_nonrecursive(AdjacencyList const &adj_list, Id start_vertex, Id finish_vertex, size_t final_length,
                                 size_t current_length, std::vector<bool> &visited)
#endif // NDEBUG
{
    typedef decltype(adj_list[0].begin()) edge_iterator;
    typedef std::tuple<Id, edge_iterator> element_type;
    std::vector<element_type> Stack = { element_type { start_vertex, adj_list[start_vertex].begin() } };
    Stack.reserve(final_length+1);

    size_t num_paths = 0;
    Id current_vertex = start_vertex, next_vertex;
    edge_iterator ei = adj_list[start_vertex].begin();
    auto si = Stack.begin();

    while (si >= Stack.begin()) {
        if (ei != adj_list[current_vertex].end()) {
            next_vertex = *ei;
            if (!visited[next_vertex]) {
                if (next_vertex != finish_vertex && current_length != final_length) {
                    // backup ei
                    std::get<1>(*si) = ei;

                    // backtrack
                    current_vertex = next_vertex;
                    visited[current_vertex] = true;
                    ei = adj_list[current_vertex].begin();
                    *++si = std::make_tuple(current_vertex, ei);
                    ++current_length;

                } else if (next_vertex == finish_vertex && current_length == final_length) {
#ifndef NDEBUG
                    auto _path = path;
                    for (auto _si = Stack.begin() + 1; _si != si; ++_si)
                        _path.push_back(std::get<0>(*_si));
                    _path.push_back(finish_vertex);
                    increase_num_paths(num_paths, _path);
#else
                    increase_num_paths(num_paths);
#endif

                    // restore
                    visited[current_vertex] = false;
                    std::tie(current_vertex, ei) = *--si;
                    ++ei;
                    --current_length;
                } else
                    ++ei;
            } else
                ++ei;

        } else {
            --si;

            // restore
            if (si >= Stack.begin()) {
                visited[current_vertex] = false;
                std::tie(current_vertex, ei) = *si;
                ++ei;
                --current_length;
            }
        }
    }

    return num_paths;
}
#endif //RECURSIVE

#ifndef NDEBUG
size_t find_kpaths_task(AdjacencyList const &adj_list, Id finish_vertex, size_t final_length, PathType &initial_path)
#else
size_t find_kpaths_task(AdjacencyList const &adj_list, Id finish_vertex, size_t final_length, PathType const &initial_path)
#endif
{
    size_t num_paths = 0;
    std::vector<bool> visited(adj_list.size(), false);
    for (auto v : initial_path) visited[v] = true;
#ifndef NDEBUG
    static size_t num_tasks = 0;
    size_t task_num = num_tasks++;
#ifdef _OPENMP
#pragma omp critical
#endif // _OPENMP
    {
        std::cerr << "\nTask #" << task_num << ", initial_path: ";
        for (auto v : initial_path) std::cerr << std::setw(2) << v << ' ';
        std::cerr << '\n';
    }
#ifdef RECURSIVE
    backtracking(initial_path.size(), initial_path.back(), visited, num_paths, initial_path);
#else 
    num_paths = backtracking_nonrecursive(adj_list, initial_path.back(), finish_vertex, final_length, initial_path.size(), visited, initial_path);
#endif // RECURSIVE
#ifdef _OPENMP
#pragma omp critical
#endif // _OPENMP
    std::cerr << "\nTask #" << task_num << ": " << num_paths << " paths\n";
#else // NDEBUG
#ifdef RECURSIVE
    backtracking(initial_path.size(), initial_path.back(), visited, num_paths);
#else
    num_paths = backtracking_nonrecursive(adj_list, initial_path.back(), finish_vertex, final_length, initial_path.size(), visited);
#endif // RECURSIVE
#endif // NDEBUG
    return num_paths;
}

#ifdef _OPENMP
size_t scheduler(AdjacencyList const &adj_list, Id start_vertex, Id finish_vertex, Id final_length)
{
    std::deque<PathType> init_paths = { {start_vertex} }, tmp;

    size_t path_length = 0;

    bool changed = true;
    while (init_paths.size() < TASK_LIMIT && changed) {
        changed = false;
        ++path_length;

        for (auto &initial_path : init_paths) {
            auto last_vertex = initial_path.back();
            for (auto next_vertex : adj_list[last_vertex])
                if (next_vertex != finish_vertex && 
                        std::find(initial_path.begin(), initial_path.end(), next_vertex) == initial_path.end()) 
                {
                    initial_path.push_back(next_vertex);
                    tmp.push_back(initial_path);
                    initial_path.pop_back();
                    changed = true;
                }
        }
        init_paths = std::move(tmp);
    }

    size_t npaths = 0;
#pragma omp parallel // reduction(+:npaths)
    {
#pragma omp single
        {
            for (auto it = init_paths.begin(); it != init_paths.end(); ++it) {
#pragma omp task //shared(it)
                { 
                    auto _npaths = find_kpaths_task(adj_list, finish_vertex, final_length, *it);
#pragma omp critical
                    npaths += _npaths;
                }
            }
        }
    }
    
    return npaths;
}
#endif // _OPENMP

void read_file(char const *filename, AdjacencyList &adj_list)
{
    size_t num_vertices, num_edges;
    std::ifstream ifs(filename);
    ifs >> num_vertices >> num_edges;
    adj_list.assign(num_vertices, AdjacencyList::value_type() );
    Id u, v;
    while ( !(ifs >> u >> v).eof() ) {
        adj_list[u].push_back(v);
    }
    ifs.close();

#ifndef NDEBUG
    std::cerr << "Graph: \nnum_vertices: " << num_vertices
              << "\nnum_edges: " << num_edges << "\n\n";
    for (Id u = 0; u != num_vertices; ++u) {
        std::cerr << "vertex #" << std::setw(2) << u << " :";
        for (Id v : adj_list[u])
            std::cerr << std::setw(2) << v << ' ';
        std::cerr << '\n';
    }
    std::cerr << "\n\n";
#endif
}


using namespace std::chrono;
typedef duration<double,std::ratio<1>> real_seconds;
#define     PROCESS_TIME(COMMAND, STR)      auto b = high_resolution_clock::now();      \
                                            COMMAND;                                    \
                                            auto time = duration_cast<real_seconds>(    \
                                                high_resolution_clock::now()-b).count();\
                                            std::cout << STR << " : " << time << "s\n"



int main(int argc, char *argv[])
{
    Id start_vertex = std::atoi(argv[1]);
    Id finish_vertex = std::atoi(argv[2]);
    size_t final_length = std::atoi(argv[3]);

    AdjacencyList adj_list;
    read_file(argv[4], adj_list);

#ifdef _OPENMP
    PROCESS_TIME(auto npaths = scheduler(adj_list, start_vertex, finish_vertex, final_length), "Time: ");
#else
    PathType initial_path { start_vertex };
    PROCESS_TIME(auto npaths = find_kpaths_task(adj_list, finish_vertex, final_length, initial_path), "Time: ");
#endif
    std::cout << npaths << '\n';

    return 0;
}
