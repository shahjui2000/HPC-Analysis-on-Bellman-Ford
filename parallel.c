
#include<stdio.h>
#include<math.h>
#include<omp.h>
#include<time.h>
#include<string.h>
#include<stdlib.h>
#include<limits.h>


#define min(x, y) (((x) < (y)) ? (x) : (y))
//  Using the MONOTONIC clock 
#define CLK CLOCK_MONOTONIC

/* Function to compute the difference between two points in time */
struct timespec diff(struct timespec start, struct timespec end);

/* 
   Function to computes the difference between two time instances

   Taken from - http://www.guyrutenberg.com/2007/09/22/profiling-code-using-clock_gettime/ 

   Further reading:
http://stackoverflow.com/questions/6749621/how-to-create-a-high-resolution-timer-in-linux-to-measure-program-performance
http://stackoverflow.com/questions/3523442/difference-between-clock-realtime-and-clock-monotonic
 */
struct timespec diff(struct timespec start, struct timespec end){
	struct timespec temp;
	if((end.tv_nsec-start.tv_nsec)<0){
		temp.tv_sec = end.tv_sec-start.tv_sec-1;
		temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
	}
	else{
		temp.tv_sec = end.tv_sec-start.tv_sec;
		temp.tv_nsec = end.tv_nsec-start.tv_nsec;
	}
	return temp;
}


int main(int argc, char* argv[])
{
	struct timespec start_e2e, end_e2e, start_alg, end_alg, e2e, alg;
	/* Should start before anything else */
	clock_gettime(CLK, &start_e2e);

	/* Check if enough command-line arguments are taken in. */
	if(argc < 3){
		printf( "Usage: %s n p \n", argv[0] );
		return -1;
	}

	int N=atoi(argv[1]);	/* size of input array */
	int P=atoi(argv[2]);	/* number of processors*/
	char *problem_name = "matrix_multiplication";
	char *approach_name = "block";
//	char buffer[10];
//	FILE* inputFile;
	FILE* outputFile;
	//	inputFile = fopen(argv[3],"r");

	char outputFileName[50];		
	sprintf(outputFileName,"output/%s_%s_%s_%s_output.txt",problem_name,approach_name,argv[1],argv[2]);	

	//**********************
	srand ( time(NULL) ); //for randomization everytime the code is runned
	
	int noV = N; //number of vertices in graph is set equal to problem size N
	int noE = (noV*(noV-1))/2; //number of edges in graph is set equal to N(N-1)/2
	int i,j;
	
	int edge[noE][3]; //Defining edge having the structure of (A,B,wight) where A and B are 
	 		  //the two nodes between which the edge exists

	i = 0;
	while(i < noE) //loop goes on until noE number of edges are not created
	{
		// printf("heya:%d ", i);
		edge[i][0] = rand()%noV ; //randomly assigning node A
		edge[i][1] = rand()%noV ; //randomly assigning node B
		edge[i][2] = rand()%50 - 10; //randomly assigning weight in range -10 to 40 (negative weights allowed)
 
		if(edge[i][0] == edge[i][1]) //if it is a self-edge, weight is 0 that is A-A or B-B
		{
			edge[i][2] = 0;
			continue;
		}
		else
		{
			for(j = 0; j < i; j++)  //checks that the new edge is not a duplicate of the old.
			{
				if((edge[i][0] == edge[j][0] && edge[i][1] == edge[j][1]) || (edge[i][0] == edge[j][1] && edge[i][1] == edge[j][0]))
				{
					i--; //if it is a duplicate, decreases the counter so as to disregard the edge
				}
			}
		}
		
		i++; //updates counter of number of edges formed as of yet
	}
    printf("---------------------------------------------\n");
    printf("Edge Description:\n");
    for(i = 0; i< noE; i++) //prints the edge description
    {
        printf("NoE %d: \t %d ----- %d: \t %d\n",i+1,edge[i][0] + 1,edge[i][1] + 1, edge[i][2] );
    }
    printf("---------------------------------------------\n");

	int* dist = (int*)malloc(sizeof(int)*noV);

    for (i = 0; i < noV; i++) //allocate initial distances of each vertex as infinity
        dist[i]   = INT_MAX;
	
	int src = 0;
    dist[src] = 0;//allocate distance of source vertex as 0, since we are finding distance all vertices to that of source

	int k;
	//**********************
	omp_set_num_threads(P); //sets the number of processors
	clock_gettime(CLK, &start_alg);	/* Start the algo timer */

	/*----------------------Core algorithm starts here----------------------------------------------*/

	
	#pragma omp parallel for private(i, k) //for parallelism
	for(i=0; i< noV; i++) //iteration for noV-1 number of times needed in Bellman-Ford
	{
		for (k = 0; k < noE; k++)//iterate over each edge
		{
            
            if( dist[edge[k][0]] != INT_MAX)//If A is not infinity 
            {
                if(dist[edge[k][0]] + edge[k][2] < dist[edge[k][1]] && edge[k][1]!=0) //Bellman-Ford algo: if dis(A)+weight<dist(B) 
                    dist[edge[k][1]] = dist[edge[k][0]]+edge[k][2];//update distance of B
            }
            if( dist[edge[k][1]] != INT_MAX)//Same logic for B since our graph is bidirectional
            {
                if(dist[edge[k][1]] + edge[k][2] < dist[edge[k][0]] && edge[k][0]!=0)
                    dist[edge[k][0]] = dist[edge[k][1]]+edge[k][2];
            }
		}
	}

	
	/*----------------------Core algorithm finished--------------------------------------------------*/

	clock_gettime(CLK, &end_alg);	/* End the algo timer */
	/* Ensure that only the algorithm is present between these two
	   timers. Further, the whole algorithm should be present. */


	/* Should end before anything else (printing comes later) */
	clock_gettime(CLK, &end_e2e);
	e2e = diff(start_e2e, end_e2e);
	alg = diff(start_alg, end_alg);
	

	// printf("Vertex Distance from Source\n");

    // for (i = 0; i < noV; ++i)
    //     printf("%d \t\t %d\n", i+1, dist[i]);

		
	/* problem_name,approach_name,n,p,e2e_sec,e2e_nsec,alg_sec,alg_nsec
	   Change problem_name to whatever problem you've been assigned
	   Change approach_name to whatever approach has been assigned
	   p should be 0 for serial codes!! 
	 */
	printf("%s,%s,%d,%d,%d,%ld,%d,%ld\n", problem_name, approach_name, N, P, e2e.tv_sec, e2e.tv_nsec, alg.tv_sec, alg.tv_nsec);

	return 0;

}
