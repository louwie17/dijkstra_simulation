#include <cnet.h>
#include <stdlib.h>
#include <limits.h>

#include "Stack.h"
#include "nl_table.h"
#include "dll_basic.h"

#define MAXHOPS     4
#define NNODES      7

typedef enum    	{ NL_DATA, NL_ACK }   NL_PACKETKIND;

typedef struct {
    char            header;
    CnetAddr		src;
    CnetAddr		dest;
    NL_PACKETKIND	kind;      	/* only ever NL_DATA or NL_ACK */
    int			seqno;		/* 0, 1, 2, ... */
    int			hopcount;
    size_t		length;       	/* the length of the msg portion only */
    char		msg[MAX_MESSAGE_SIZE];

    StackT      nodes_visited;
    int         total_cost;
} NL_PACKET;

typedef struct {
    char        header;
    CnetAddr    src;
    int         seqno;
    int         link_cost;
    int         node_table[NNODES+1];
} NL_UPD;

#define PACKET_HEADER_SIZE  (sizeof(NL_PACKET) - MAX_MESSAGE_SIZE)
#define PACKET_SIZE(p)	    (PACKET_HEADER_SIZE + p.length)

/* ----------------------------------------------------------------------- */

static void update_routing_tables(NL_UPD packet)
{
    int link;
    for (link=1; link<=nodeinfo.nlinks; ++link)
    {
        packet.link_cost = linkinfo[link].costperframe;
        CHECK(down_to_datalink(link, (char*)&packet, sizeof(NL_UPD)));
    }
}
void print_array(int node_table[NNODES+1])
{
    int j;
    printf("Node table \n");
    for (j = 0; j< (NNODES+1); j++)
        printf("%4d\n", node_table[j]);
}

EVENT_HANDLER(update_tables)
{
    // create update table packet
    printf("Calling update routing tables \n");
    NL_UPD  p;
    p.header = 'u';
    p.seqno = nodeinfo.nodenumber;
    p.src   = nodeinfo.address;
    printf("Before memcpy\n");
    print_array(p.node_table);
    get_columns(p.node_table, 1);
    printf("After memcpy\n");
    print_array(p.node_table);
    update_routing_tables(p);
    // call update_routing_tables with the created packet.
    CNET_start_timer(EV_TIMER1, (CnetTime)5000000, 0);
}

EVENT_HANDLER(down_to_network)
{
}
/*  up_to_network() IS CALLED FROM THE DATA LINK LAYER (BELOW) TO ACCEPT
    A PACKET FOR THIS NODE, OR TO RE-ROUTE IT TO THE INTENDED DESTINATION.
    */
int up_to_network(char *packet, size_t length, int arrived_on_link)
{
    if (packet[0] == 'u')
    {
        NL_UPD      *p = (NL_UPD *)packet;
        printf("God a NL_UPD From: %4d\n",p->src);
        //p->total_cost += linkinfo[arrived_on_link].costperframe;
        //return 0;
        printf("=============================\n");
        printf("Link Cost: %4d\n", linkinfo[arrived_on_link].costperframe);
        if (p->node_table[nodeinfo.nodenumber] > p->link_cost)
            p->node_table[nodeinfo.nodenumber] = p->link_cost;
        print_array(p->node_table);
        int updated = NL_updateroutingtable(p->src,
                arrived_on_link, p->seqno, p->node_table);
        if (updated == 1)
        {
            // continue sending packet around
            printf("Updated Table\n");
            p->seqno = nodeinfo.nodenumber;
            p->src = nodeinfo.address;
            get_columns(p->node_table, 1);
            update_routing_tables(*p);
        }
        else
            //discard packet;
        return 0;
    }
    //else
    //    NL_PACKET	*p = (NL_PACKET *)packet;

    //++p->hopcount;			/* took 1 hop to get here */
    //p->total_cost += linkinfo[arrived_on_link].costperframe;
    // stack_push(p->nodes_visited, nodeinfo.nodenumber);
    /*  IS THIS PACKET IS FOR ME? */
    /*
    if(p->dest == nodeinfo.address)

        switch (p->kind) {
            case NL_DATA :
                break;
            case NL_ACK :
                break;
        }
        */
    /* OTHERWISE, THIS PACKET IS FOR SOMEONE ELSE */
    /*
    else {
        ;
    }*/
    return(0);
}

/* ----------------------------------------------------------------------- */

EVENT_HANDLER(reboot_node)
{
    reboot_DLL();
    reboot_NL_table();

    CHECK(CNET_set_handler(EV_APPLICATIONREADY, down_to_network, 0));
    CHECK(CNET_set_handler(EV_TIMER1, update_tables, 0));
    CNET_start_timer(EV_TIMER1, (CnetTime)1000, 0);
    CNET_enable_application(ALLNODES);
}
