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

static int NL_routingtable[2][NNODES+1];
// -----------------------------------------------------------------

//  GIVEN AN ADDRESS, LOCATE OR CREATE ITS ENTRY IN THE NL_table

void get_columns(int* table, int column)
{
    memcpy(&table[0], &NL_routingtable[column], sizeof(NL_routingtable[0]));
}
// -----------------------------------------------------------------

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
            printf("Node: %4d Source: %4d Dest: %4d  %d->%d\n",nodeinfo.address,
                    address, j, NL_routingtable[0][j], 
                    node_table[j]+NL_routingtable[0][address]);
            NL_routingtable[1][j] = link;
            NL_routingtable[0][j] = node_table[j] + NL_routingtable[0][address];
            updated = 1;
        }
    }
    return updated;
}

// -----------------------------------------------------------------

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
}

void reboot_NL_table(void)
{
    CHECK(CNET_set_handler(EV_DEBUG0, show_NL_table, 0));
    CHECK(CNET_set_debug_string(EV_DEBUG0, "NL info"));
    //int max = 10000;
    //memset(NL_routingtable, , sizeof(NL_routingtable[0][0]) * 2 * (NNODES+1));
    int i,j;
    for (i = 0; i < 2; i++)
    {
        for (j = 0; j < (NNODES+1); j++)
            NL_routingtable[i][j] = MAX_INT;
    }
    //NL_routingtable
    NL_routingtable[0][nodeinfo.nodenumber] = 0;
    NL_routingtable[1][nodeinfo.nodenumber] = nodeinfo.nodenumber;
    NL_table		= calloc(1, sizeof(NLTABLE));
    NL_table_size	= 0;
}
