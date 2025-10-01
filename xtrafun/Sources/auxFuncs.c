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
int64_t max(int64_t arg1, int64_t arg2){

	if(arg1>arg2){

		return arg1;

	}
	return arg2;


}
int64_t clamp(int64_t value,int64_t arg1, int64_t arg2){
	
	if(arg1>arg2){
	int64_t tmp_var=arg1;
	arg1=arg2;
	arg2=tmp_var;

	}
	if(value<arg1){

		return arg1;

	}
	if(value>arg2){

		return arg2;
	}
	return value;


}
void print_addr_aux(char* prompt,struct sockaddr_in* addr){
         printf("%s\nEndereço: \n%s Porta: %u\n",prompt,inet_ntoa(addr->sin_addr),ntohs(addr->sin_port));

}

void print_sock_addr(int socket){

        struct sockaddr_in addr={0};

        getsockname(socket,(struct sockaddr*)(&addr),&socklenvar[0]);

        print_addr_aux("O endereço desta socket é:\n",&addr);

}
void init_addr(struct sockaddr_in* addr, char* hostname_str,uint16_t port){

         addr->sin_family=AF_INET;
        struct addrinfo *addr_info_struct=NULL;
        int error=0;
         if((error=getaddrinfo(hostname_str, NULL, NULL, &addr_info_struct))){
                printf("Erro a obter address a partir de hostname!!\nErro: %s\n",gai_strerror(error));
                if(addr_info_struct){
                        freeaddrinfo(addr_info_struct);
                }
        }

        memcpy(addr,(struct sockaddr_in*)addr_info_struct->ai_addr,sizeof(struct sockaddr_in));

        addr->sin_port= htons(port);

        print_addr_aux("ip address: ",addr);

        if(addr_info_struct){
                freeaddrinfo(addr_info_struct);
        }
}
WINDOW *create_newwin(int height, int width, int starty, int startx)
{	WINDOW *local_win;

	local_win = newwin(height, width, starty, startx);
	box(local_win, 0 , 0);		/* 0, 0 gives default characters 
					 * for the vertical and horizontal
					 * lines			*/
	wrefresh(local_win);		/* Show that box 		*/

	return local_win;
}

void destroy_win(WINDOW *local_win)
{
	wborder(local_win, ' ', ' ', ' ',' ',' ',' ',' ',' ');
	wrefresh(local_win);
	delwin(local_win);
}

void clear_screen_with_printf(void){
	printf("\e[1;1H\e[2J");

}
