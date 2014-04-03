#include <cnet.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#define INF 10000
#define NUM_OF_NODES 8
#define MEMBER 1
#define NOMEMBER 0

void dijkstra(int weight[][NUM_OF_NODES],int s,int t,int *pd, int precede[])
{
    int distance[NUM_OF_NODES],perm[NUM_OF_NODES],prev[NUM_OF_NODES];
    int current,k,dc;
    int smalldist,newdist;
    for (int i = 0; i < NUM_OF_NODES; i++)
    {
        perm[i] = NOMEMBER;
        distance[i] = INF;
        prev[i] = -1;
    }

    perm[s] = MEMBER;
    distance[s] = 0;
    current = s;

    while(current != t)
    {
        smalldist = INF;
        dc = distance[current];
        for (int i = 0; i < NUM_OF_NODES; i++)
        {
            if (perm[i] == NOMEMBER)
            {
                newdist = dc + weight[current][i];
                if (newdist < distance[i])
                {
                    distance[i] = newdist; // Count the updated distance
                    precede[i] = current;

                }
                if (distance[i] < smalldist)
                {
                    smalldist = distance[i];
                    k = i;
                }

            }
        }//end of for and if

        current = k;
        perm[current] = MEMBER;

    }//end while
    *pd = distance[t];
} //end of function

void print_path(int s, int t, int precede[]) {
    int current = t;

    while (current != s) {
        printf("%d -> ", current);
        current = precede[current];
    }

    printf("%d\n",current);
}
