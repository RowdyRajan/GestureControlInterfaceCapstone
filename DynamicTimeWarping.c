#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/*
Test algorithm for Dynamic Time Warping between a sequence of points will have to be adjusted 
for accleration data 
*/
int main(int argc, char** argv){

}

int min(int x , int y){
	if(x < y){
		return x;
	}else{
		return y;
	}
}

void DTW(int *x, int xlenght, int *y, int ylength, int*** result){
	*result = malloc(xlenght * sizeof(int));
	for(int i =0; i< xlenght; i++){
		*result[i] = malloc(ylength * sizeof(int));
	}

	for(int i =1; i<xlenght; i++){
		*result[i][0] = INFINITY;
	}

	for(int i =1; i<xlenght; i++){
		*result[0][i] = INFINITY;
	}
	*result[0][0] = 0;

	for(int i = 1; i<xlenght; i++ ){
		for(int j = 1; j<ylength;j++){
			int cost = x[i] - y[j];
			int min = min(*result[i-1, j-1] ,min(*result[i-1,j], *result[i, j-1]));
			*result[i,j] = cost + min; 
		}
	}
}
