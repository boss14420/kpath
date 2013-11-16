#!/usr/bin/env python3

import random
import sys


def generate(n, m):
    G = [[] for i in range(n)]
    ratio = m / ((n - 1) * n)
    print(ratio)
    edges = 0
    for i in range(0, n):
        for j in range(0, n):
            if random.random() <= ratio and i != j:
                G[i].append(j)
                edges += 1

    return (G, edges)


def writefile(G, n, edges, fn):
    with open(fn, 'wt') as f:
        f.write('{0} {1}\n'.format(n, edges))
        for i in range(n):
            for j in G[i]:
                f.write('{0} {1}\n'.format(i, j))


def main():
    n = int(sys.argv[1])
    m = int(sys.argv[2])
    fn = sys.argv[3]

    (G, edges) = generate(n, m)
    writefile(G, n, edges, fn)

if __name__ == "__main__":
    main()
