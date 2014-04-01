#include <cnet.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include "Stack.h"
#include "nl_table.h"

// ---- A SIMPLE NETWORK LAYER SEQUENCE TABLE AS AN ABSTRACT DATA TYPE ----
#define NNODES

typedef struct {
    CnetAddr	address;		// ... of remote node
    int     ackexpected;
    int     nextpackettosend;
    int     packetexpected;

    int		minhops;		// minimum known hops to remote node
    int		minhop_link;		// link via which minhops path observed
    int     total_cost;
    StackT  path;
} NLTABLE;

static	NLTABLE	*NL_table	= NULL;
static	int	NL_table_size	= 0;

static int NL_routingtable[NNODES][NNODES];

// -----------------------------------------------------------------

//  GIVEN AN ADDRESS, LOCATE OR CREATE ITS ENTRY IN THE NL_table
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
    NL_table[NL_table_size].minhops	= INT_MAX;
    NL_table[NL_table_size].total_cost = INT_MAX;
    return NL_table_size++;
}

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

// -----------------------------------------------------------------

//  FIND THE LINK ON WHICH PACKETS OF MINIMUM HOP COUNT WERE OBSERVED.
//  IF THE BEST LINK IS UNKNOWN, WE RETURN ALL_LINKS.

int NL_linksofminhops(CnetAddr address) {
    int	t	= find_address(address);
    int link	= NL_table[t].minhop_link;
    return (link == 0) ? ALL_LINKS : (1 << link);
}

static	bool	given_stats	= false;

void NL_updateroutingtable(CnetAddr address, int hops, int link, int nodenum)
{
    if (hops == 1)
        NL_routingtable[address][nodenum] = linkinfo[link].costperframe;
}

void NL_savehopcount(CnetAddr address, int hops, int link, int total_cost,
        StackT nodes_visited)
{
    int	t	= find_address(address);

    if(NL_table[t].minhops > hops && NL_table[t].total_cost >= total_cost) {
	NL_table[t].minhops	= hops;
	NL_table[t].minhop_link	= link;
    NL_table[t].total_cost = linkinfo[link].costperframe;
    //NL_table[t].path    = nodes_visited;
	given_stats		= true;
    } else if (NL_table[t].total_cost > total_cost) {
        NL_table[t].minhops = hops;
        NL_table[t].minhop_link = link;
        NL_table[t].total_cost = linkinfo[link].costperframe;
        //NL_table[t].path = nodes_visited;
        given_stats = true;
    }

}

// -----------------------------------------------------------------

static EVENT_HANDLER(show_NL_table)
{
    CNET_clear();
    printf("\n%12s", "destination");
    if(given_stats)
	printf(" %8s %8s %8s", "minhops", "bestlink", "min cost");
    printf("\n");

    for(int t=0 ; t<NL_table_size ; ++t)
	if(NL_table[t].address != nodeinfo.address) {
	    printf("%12d", (int)NL_table[t].address);
	    if(NL_table[t].minhop_link != 0)
		printf(" %8d %8d %8d", NL_table[t].minhops,NL_table[t].minhop_link,
                NL_table[t].total_cost);
	    printf("\n");
	}
    printf("NNODES value: %d \n",NNODES);
    int i, j;
    for (i = 0; i < NNODES; i++) {
        for (j = 0; j < NNODES; j++) {
            printf("%4d", NL_routingtable[i][j]);
        }
        printf("\n");
    }
}

void reboot_NL_table(void)
{
    CHECK(CNET_set_handler(EV_DEBUG0, show_NL_table, 0));
    CHECK(CNET_set_debug_string(EV_DEBUG0, "NL info"));

    NL_table		= calloc(1, sizeof(NLTABLE));
    NL_table_size	= 0;
}
