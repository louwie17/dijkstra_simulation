#include <cnet.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include "Stack.h"
#include "nl_table.h"

// ---- A SIMPLE NETWORK LAYER SEQUENCE TABLE AS AN ABSTRACT DATA TYPE ----
#define NNODES      7
#define MAX_INT     99

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
/*
static int find_address(CnetAddr address)
{
//  ATTEMPT TO LOCATE A KNOWN ADDRESS
    for(int t=0 ; t<NL_table_size ; ++t)
	if(NL_table[t].address == address)
	    return t;

//  UNNOWN ADDRESS, SO WE MUST CREATE AND INITIALIZE A NEW ENTRY
    NL_table	= realloc(NL_table, (NL_table_size+1)*sizeof(NLTABLE));
    memset(&NL_table[NL_table_size], 0, sizeof(NLTABLE));
    NL_table[NL_table_size].address	= address;
    //NL_table[NL_table_size].path = stack_new();
    NL_table[NL_table_size].total_cost = INT_MAX;
    return NL_table_size++;
}
*/
void get_columns(int* table, int column)
{
    printf("Routing table  given table\n");
    int j;
    for (j = 0; j < (NNODES + 1); j++)
        printf("%8d %8d\n",NL_routingtable[column][j], table[j]);
    memcpy(&table[0], &NL_routingtable[column], sizeof(NL_routingtable[0]));
}
/*
int NL_ackexpected(CnetAddr address) {
    int t   = find_address(address);
    return NL_table[t].ackexpected;
}

void inc_NL_ackexpected(CnetAddr address) {
    int t   = find_address(address);
    NL_table[t].ackexpected++;
}

int NL_nextpackettosend(CnetAddr address) {
    int t   = find_address(address);
    return NL_table[t].nextpackettosend++;
}

int NL_packetexpected(CnetAddr address) {
    int t   = find_address(address);
    return NL_table[t].packetexpected;
}

void inc_NL_packetexpected(CnetAddr address) {
    int t   = find_address(address);
    NL_table[t].packetexpected++;
}
*/
// -----------------------------------------------------------------

//  FIND THE LINK ON WHICH PACKETS OF MINIMUM HOP COUNT WERE OBSERVED.
//  IF THE BEST LINK IS UNKNOWN, WE RETURN ALL_LINKS.
/*
int NL_linksofminhops(CnetAddr address) {
    int	t	= find_address(address);
    int link	= NL_table[t].minhop_link;
    return (link == 0) ? ALL_LINKS : (1 << link);
}
*/
// static	bool	given_stats	= false;

int NL_updateroutingtable(CnetAddr address, int link, int last_node, 
        int node_table[NNODES+1])
{
    /*
    if (hops == 1)
        NL_routingtable[address][nodenum] = linkinfo[link].costperframe;
    */
    //int t   = find_address(address);
    if (NL_routingtable[0][address] > linkinfo[link].costperframe)
    {
        NL_routingtable[0][address] = linkinfo[link].costperframe;
        NL_routingtable[1][address] = address;
    }
    int j;
    int updated = 0;
    for (j = 0; j < (NNODES + 1); j++)
    {
        printf("Compare %4d with %4d\n", NL_routingtable[0][j], (node_table[j]+NL_routingtable[0][address]));
        if (NL_routingtable[0][j] > (node_table[j] + NL_routingtable[0][address]))
        {
            NL_routingtable[1][j] = address;
            NL_routingtable[0][j] = node_table[j] + NL_routingtable[0][address];
            updated = 1;
        }
    }
    /*
    if (NL_table[t].total_cost > total_cost) {
        NL_table[t].total_cost = linkinfo[link].costperframe;
        NL_table[t].next_node = last_node;
        given_stats = true;
        return 1;
    }*/
    return updated;
}

// -----------------------------------------------------------------

static EVENT_HANDLER(show_NL_table)
{
    CNET_clear();
    printf("Node #%3d table\n",nodeinfo.nodenumber);
    printf("%8s %8s %8s\n","Dest","Cost","Next Node");
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
    memset(NL_routingtable, 50, sizeof(NL_routingtable[0][0]) * 2 * (NNODES+1));
    NL_routingtable[0][nodeinfo.nodenumber] = 0;
    NL_routingtable[1][nodeinfo.nodenumber] = nodeinfo.nodenumber;
    NL_table		= calloc(1, sizeof(NLTABLE));
    NL_table_size	= 0;
}
