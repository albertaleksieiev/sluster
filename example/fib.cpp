#include "../sluster.h"
#include <vector>
#include <utility>

std::vector<std::pair<int,long long>> results;
int value = 38;
char buf[16394];
void on_message(int sockfd, char* msg, struct sockaddr *their_addr, socklen_t addr_len){

    int offset = 0;
    for(auto& p: results){
        offset += sprintf(buf+offset,"%d,%lld\n",p.first, p.second);
    }

    sendto(sockfd, buf, offset, 0, their_addr, addr_len);
}

long long fib(int n){
    if(n < 2){
        return 1;
    }
    return fib(n - 1) + fib(n - 2);
}

void calcFunc(void*){
    while(true) {
        results.push_back({value, fib(value)});
        value++;
    }
}


int main(){
    CalculationSocketManager calculationSocketManager("4950",on_message);

    calculationSocketManager.start(calcFunc);
}
