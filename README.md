# Sluster
Simple computation cluster, with easy integration. Read more in my [blog](http://albert.guru/).

Include `sluster.h` into project and implement 2 functions - response on message and computation function.

```
#include "sluster.h"

char buf[16394];
int value1 = 0, value2 = 20;
void on_message(int sockfd, char* msg, struct sockaddr *their_addr, socklen_t addr_len){

    int n = sprintf(buf, "hello from here: %d,%d", value1,value2);

    sendto(sockfd, buf, n, 0, their_addr, addr_len);
}

void calcFunc(void*){
	while(true){
		value1+=10;
		value2 += value1;
		value2 %= 5;
		value1 %=100;
	}
}
int main(){
    CalculationSocketManager calculationSocketManager("4950",on_message);

    calculationSocketManager.start(calcFunc);
}

```
***File main.cpp***

Compile with `pthread` flag, and run.

```
> g++ main.cpp -pthread -o main.o
> ./main.o
starting UDP server on port 4950
```

Try it by using netcat.

```
> nc -u localhost  4950
> 
hello from here: 100,0
```



