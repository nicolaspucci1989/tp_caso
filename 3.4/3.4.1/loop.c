#include <stdio.h>
#define IDGRUPO 200

int main(){

  int i,c;

  while(1){
    c = 48 + i;

    printf("%c", c);

    i++;
    i = i % IDGRUPO;
  }

  return 0;
}
