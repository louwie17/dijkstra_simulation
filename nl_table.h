#include <cnet.h>
#include "Stack.h"

#define	ALL_LINKS	(-1)

extern	void	reboot_NL_table(void);

extern  void get_columns(int* table, int column);
extern	int	NL_ackexpected(CnetAddr address);
extern	int	NL_nextpackettosend(CnetAddr address);
extern	int	NL_packetexpected(CnetAddr address);

extern	void	inc_NL_packetexpected(CnetAddr address);
extern	void	inc_NL_ackexpected(CnetAddr address);

extern	int	NL_linksofminhops(CnetAddr address);
extern  int    NL_updateroutingtable(CnetAddr address, int link, int last_node,
        int node_table[8]);
extern	void	NL_savehopcount(CnetAddr address, int hops, int link,
        int total_cost, StackT nodes_visited);
