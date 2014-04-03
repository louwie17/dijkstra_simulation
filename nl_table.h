#include <cnet.h>
#include "Stack.h"

#define	ALL_LINKS	(-1)
#define NNODE       7

extern	void	reboot_NL_table(void);

extern  void get_columns(int* table, int column);
extern  void get_full_table(int table[NNODE+1][NNODE+1]);

extern  int NL_path(int source, int destination);
extern  int NL_link(CnetAddr destination);
/*
extern	int	NL_ackexpected(CnetAddr address);
extern	int	NL_nextpackettosend(CnetAddr address);
extern	int	NL_packetexpected(CnetAddr address);

extern	void	inc_NL_packetexpected(CnetAddr address);
extern	void	inc_NL_ackexpected(CnetAddr address);

extern	int	NL_linksofminhops(CnetAddr address);
*/
extern  int    NL_updateroutingtable(CnetAddr address, int link, int last_node,
        int node_table[NNODE+1]);
extern  int    NL_updatefulltable(int full_table[NNODE+1][NNODE+1]);
extern	void	NL_savehopcount(CnetAddr address, int hops, int link,
        int total_cost, StackT nodes_visited);
