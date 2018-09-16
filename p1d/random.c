#include <stdio.h>
#include <stdlib.h>

int main(){
  int c, n;

  for(c = 1; c<=300000; c++){
    n = rand() % 1000;
    printf("%d\n", n);
  }

  return 0;
}
