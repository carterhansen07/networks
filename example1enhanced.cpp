#include "frameio.h"
#include "util.h"
#include <stdio.h>
#include <iostream> 
#include <iomanip>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <fstream>
#include <iomanip>







int main()
{
    
    getmac(MY_MAC);
    MY_IP[0] = 129;
    MY_IP[1] = 123;
    MY_IP[2] =   4;
    MY_IP[3] = 189;
    TARGET_MAC[0] = 0xa0;
    TARGET_MAC[1] = 0xb3;
    TARGET_MAC[2] = 0xcc;
    TARGET_MAC[3] = 0xf7;
    TARGET_MAC[4] = 0xe3;
    TARGET_MAC[5] = 0x59;
    TARGET_IP[0]  = 129;
    TARGET_IP[1]  = 123;
    TARGET_IP[2]  = 85;
    TARGET_IP[3]  = 248;
  
   net.assignmac(MY_MAC);
   net.open_net("eno1");
   

   pthread_create(&loop_thread,NULL,protocol_loop,NULL);
   pthread_create(&arp_thread,NULL,arp_protocol_loop,NULL);
   pthread_create(&ip_thread,NULL,ip_protocol_loop,NULL);
   for ( ; ; )
      sleep(1);
}


