#include <cnet.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>

#include "Stack.h"
#include "nl_table.h"
#include "dll_basic.h"

#define MAXHOPS     4
#define NNODES      7

typedef enum        { CENTRAL, SOURCE, DISTR } NL_ROUTING_TYPE;
typedef enum        { NL_DATA, NL_ACK }     NL_PACKETKIND;

typedef struct {
    char            header;
    CnetAddr		src;
    CnetAddr		dest;
    size_t		length;       	/* the length of the msg portion only */
    char		msg[MAX_MESSAGE_SIZE];
} NL_PACKET;

typedef struct {
    char        header;
    CnetAddr    src;
    int         seqno;
    int         link_cost;
    int         node_table[NNODES+1];
    int         full_table[NNODES+1][NNODES+1];
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
void print_array(int node_table[NNODES+1][NNODES+1])
{
    int i,j;
    printf("Node table \n");
    for (i = 0; i< (NNODES+1); i++)
    {
        for (j = 0; j < (NNODES+1); j++)
            printf("%8d", node_table[i][j]);
        printf("\n");
    }
}

EVENT_HANDLER(update_tables)
{
    // create update table packet
    NL_UPD  p;
    p.header = 'u';
    p.seqno = nodeinfo.nodenumber;
    p.src   = nodeinfo.address;
    get_columns(p.node_table, 0);
    get_full_table(p.full_table);
    update_routing_tables(p);
    // call update_routing_tables with the created packet.
    CNET_start_timer(EV_TIMER1, (CnetTime)5000000, 0);
}

EVENT_HANDLER(down_to_network)
{
    NL_PACKET   p;
    p.header    = 'd';
    p.length    = sizeof(p.msg);
    CHECK(CNET_read_application(&p.dest, p.msg, &p.length));
    CHECK(CNET_disable_application(p.dest));
    p.src   = nodeinfo.address;
    int link = NL_link(p.dest);
    // Send packet to the datalink layer
    if (link <= nodeinfo.nodenumber)
        CHECK(down_to_datalink(link, (char *)&p, PACKET_SIZE(p)));
    else
    {
        int l = ((int)rand() % nodeinfo.nlinks)+1;
        CHECK(down_to_datalink(l, (char *)&p, PACKET_SIZE(p)));
    }
}

/*  up_to_network() IS CALLED FROM THE DATA LINK LAYER (BELOW) TO ACCEPT
    A PACKET FOR THIS NODE, OR TO RE-ROUTE IT TO THE INTENDED DESTINATION.
    */
int up_to_network(char *packet, size_t length, int arrived_on_link)
{
    if (packet[0] == 'd')
    {
        NL_PACKET   *p = (NL_PACKET *)packet;

        if (p->dest == nodeinfo.address)
        {
            length = p->length;
            CHECK(CNET_write_application(p->msg, &length));
        }
        else
        {
           int link = NL_link(p->dest);
           if (link <= nodeinfo.nlinks)
               CHECK(down_to_datalink(link, packet, length));
           else
               CHECK(down_to_datalink(arrived_on_link, packet, length));
        }
    }
    else if (packet[0] == 'u')
    {
        NL_UPD      *p = (NL_UPD *)packet;
        if (p->node_table[nodeinfo.nodenumber] != p->link_cost)
            p->node_table[nodeinfo.nodenumber] = p->link_cost;
        p->full_table[p->src][nodeinfo.nodenumber] = p->link_cost;
        p->full_table[nodeinfo.nodenumber][p->src] = 
            linkinfo[arrived_on_link].costperframe;
        int updated_full_table = NL_updatefulltable(p->full_table);
        int updated = NL_updateroutingtable(p->src,
                arrived_on_link, p->seqno, p->node_table);
        if (updated == 1 || updated_full_table == 1)
        {
            // continue sending packet around
            p->seqno = nodeinfo.nodenumber;
            p->src = nodeinfo.address;
            get_columns(p->node_table, 0);
            get_full_table(p->full_table);
            update_routing_tables(*p);
        }
        else
            //discard packet;
        return 0;
    }
    return(0);
}

/* ----------------------------------------------------------------------- */

EVENT_HANDLER(reboot_node)
{
    reboot_DLL();
    reboot_NL_table();
    (void)srand( (int)time(NULL) );
    CHECK(CNET_set_handler(EV_APPLICATIONREADY, down_to_network, 0));
    CHECK(CNET_set_handler(EV_TIMER1, update_tables, 0));
    CNET_start_timer(EV_TIMER1, (CnetTime)10, 0);
    CNET_enable_application(ALLNODES);
}
