#define NUM_OF_NODES 8

void dijkstra(int weight[][NUM_OF_NODES], int s, int t, int *pd, int precede[]);
void print_path(int s, int t, int precede[]);
