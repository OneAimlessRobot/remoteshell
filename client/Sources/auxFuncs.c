#include "../Includes/preprocessor.h"
#include "../Includes/auxFuncs.h"

double genRanddouble(double min, double max){

    if (max < min) {
        fprintf(stderr, "Error: max must be greater than or equal to min\n");
        return 0.0f;
    }

    // Generate a random double between 0 and 1
    double random = ((double)rand() / RAND_MAX);

    // Scale and shift the random value to fit within the desired range
    return (random * (max - min)) + min;

}

int genRandInt(int min, int max) {
    if (max < min) {
        fprintf(stderr, "Error: max must be greater than or equal to min\n");
        return 0;
        }

    // Generate a random integer between min and max
    return (rand() % (max - min + 1)) + min;
}


char* randStr(int size){

        char* result= malloc(size+1);
        memset(result,0,size+1);
        for(int i=0;i<size;i++){

                result[i]=(char)genRandInt((int)97,(int)112);

        }
	result[size]=0;
        return result;



}

char** randStrArr(int sizeOfStrs,int size){

	char** result= malloc(sizeof(char*)*size);

	for(int i=0;i<size;i++){

		result[i]=randStr(sizeOfStrs);

	}

	return result;


}

void freeStrArr(char** arr,int size){
	if(!arr){

		return;

	}
	
	for(int i=0;i<size;i++){
		if(arr[i]){
		free(arr[i]);
		}
	}

	free(arr);


}

int* getRandIntArr(int min,int max,int size){

	int* result= malloc(sizeof(int)*size);
	for(int i=0;i<size;i++){

		result[i]=genRandInt(min,max);
	}
	return result;

}

void swap(void** var1, void** var2){

void* tmp=*var1;
*var1=*var2;
*var2=tmp;




}


int64_t min(int64_t arg1, int64_t arg2){

	if(arg1<arg2){

		return arg1;

	}
	return arg2;


}
