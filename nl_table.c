#include <cnet.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include "Stack.h"
#include "nl_table.h"

// ---- A SIMPLE NETWORK LAYER SEQUENCE TABLE AS AN ABSTRACT DATA TYPE ----
#define NNODES      7
#define MAX_INT     10000

typedef struct {
    CnetAddr	address;		// ... of remote node
    CnetAddr    next_node;
    int     total_cost;
} NLTABLE;

static	NLTABLE	*NL_table	= NULL;
static	int	NL_table_size	= 0;

static int NL_fullroutingtable[NNODES+1][NNODES+1];
static int NL_routingtable[2][NNODES+1];

void get_columns(int* table, int column)
{
    memcpy(&table[0], &NL_routingtable[column], sizeof(NL_routingtable[0]));
}

void get_full_table(int table[NNODE+1][NNODE+1])
{
    int i;
    for (i = 0; i < (NNODES+1); i++)
    {
        memcpy(&table[i], &NL_fullroutingtable[i], sizeof(NL_fullroutingtable[0]));
    }
}

int NL_link(CnetAddr destination)
{
    return NL_routingtable[1][destination];
}

int NL_updateroutingtable(CnetAddr address, int link, int last_node, 
        int node_table[NNODES+1])
{
    int updated = 0;
    if (NL_routingtable[0][address] != linkinfo[link].costperframe)
    {
        NL_routingtable[0][address] = linkinfo[link].costperframe;
        NL_routingtable[1][address] = link;
        updated = 1;
    }
    int j;
    for (j = 0; j < (NNODES + 1); j++)
    {
        if (NL_routingtable[0][j] > (node_table[j] + NL_routingtable[0][address]))
        {
            NL_routingtable[1][j] = link;
            NL_routingtable[0][j] = node_table[j] + NL_routingtable[0][address];
            updated = 1;
        }
    }
    return updated;
}

int NL_updatefulltable(int full_table[NNODE+1][NNODE+1])
{
    int i,j;
    int updated = 0;
    for (i = 0; i < (NNODE+1); i++)
    {
        for (j = 0; j < (NNODE+1); j++)
        {
            if (full_table[i][j] == MAX_INT)
                continue;
            if (full_table[i][j] != NL_fullroutingtable[i][j])
            {
                NL_fullroutingtable[i][j] = full_table[i][j];
                updated = 1;
            }
        }
    }
    return updated;
}

static EVENT_HANDLER(show_NL_table)
{
    CNET_clear();
    printf("Node #%3d table\n",nodeinfo.nodenumber);
    printf("%8s %8s %8s\n","Dest","Cost","Next Link");
    int i, j;
    for (i = 0; i < (NNODES+1); i++) {
        printf("%8d",i);
        for (j = 0; j < 2; j++) {
            printf("%8d", NL_routingtable[j][i]);
        }
        printf("\n");
    }
    printf("Full table    from: \n");
    for (i = 0; i < (NNODES+1); i++)
    {
        for (j = 0; j < (NNODES+1); j++)
        {
            printf("%9d", NL_fullroutingtable[i][j]);
        }
        printf("\n");
    }
}

void reboot_NL_table(void)
{
    CHECK(CNET_set_handler(EV_DEBUG0, show_NL_table, 0));
    CHECK(CNET_set_debug_string(EV_DEBUG0, "NL info"));
    int i,j;
    for (i = 0; i < 2; i++)
    {
        for (j = 0; j < (NNODES+1); j++)
            NL_routingtable[i][j] = MAX_INT;
    }
    for (i = 0; i < (NNODES+1); i++)
    {
        for (j = 0; j < (NNODES+1); j++)
        {
            NL_fullroutingtable[i][j] = MAX_INT;
        }
    }
    NL_routingtable[0][nodeinfo.nodenumber] = 0;
    NL_routingtable[1][nodeinfo.nodenumber] = nodeinfo.nodenumber;
    NL_table		= calloc(1, sizeof(NLTABLE));
    NL_table_size	= 0;
}
