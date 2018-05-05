#include <time.h>
#include <iostream>
#include <stdlib.h>


int main(){
int iterations = 0;
int msec = 0, trigger = 5000; /* 5000ms */
clock_t before = clock();

for(;;){
  /*
   * Do something to busy the CPU just here while you drink a coffee
   * Be sure this code will not take more than `trigger` ms
   */



  clock_t difference = clock() - before;
  msec = difference * 1000 / CLOCKS_PER_SEC;
  iterations++;



  if ( msec >= trigger ){

  	printf("Time taken %d seconds %d milliseconds (%d iterations)\n",msec/1000, msec%1000, iterations);
  	break;
  }
} 



}