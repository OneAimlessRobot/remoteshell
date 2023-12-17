#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include <string.h>
#include <time.h>
#include <unistd.h>


int randInteger(int min, int max) {
    // Seed the random number generator with the current time.


    // Generate a random number between 0 and RAND_MAX.
    int random = rand();

    // Scale the random number to fit within the specified range.
    int result = (random % (max - min + 1)) + min;

    return result;
}


char* randStr(int size){
	
	char* result= malloc(size+1);
	memset(result,0,size+1);
	for(int i=0;i<=size;i++){

		result[i]=(char)randInteger((int)0,(int)255);
	
	}
	write(1,result,size+1);
	return result;
	


}



int main(int argc, char ** argv){


                struct timespec time;

                clock_gettime(CLOCK_REALTIME, &time);
                srand(time.tv_nsec);


	if(argc<2){

		perror("Mete um numero como arg ou n vai dar :/\n");
		exit(1);
	}
	
	free(randStr(atoi(argv[1])));

	return 0;
}
