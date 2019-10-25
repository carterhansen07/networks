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
static octet iptosend[4];
static unsigned char     MY_MAC[6];
static unsigned char      MY_IP[4];
static unsigned char TARGET_MAC[6];
static unsigned char  TARGET_IP[4];
int placeholder;
int placeholder1;
int placeholder2;
int placeholder3;
void getmac(unsigned char* mac)
{

char my_mac[6];
char temp;
char semicolon;
char useless;
std::ifstream addressfile;
addressfile.open("/sys/class/net/eno1/address");

for (unsigned int i = 0; i<6; i++)
{
  addressfile >> my_mac[i];
  addressfile >> temp;
  my_mac[i] = (my_mac[i]>= 'a')?(my_mac[i]-'a'+10) : (my_mac[i] - '0');
  my_mac[i] = my_mac[i] <<4;
  temp = (temp>= 'a')?(temp-'a'+10) : (temp - '0');
  my_mac[i] = temp + my_mac[i];
  addressfile >> useless;
  mac[i] = my_mac[i];
}

addressfile.close();


}




void PrintArray_NewLine(unsigned char* arraytoprint, unsigned int length)
{
  for (unsigned int i =0; i < length; i++ )
          {
              printf("%d \n",arraytoprint[i]);
          }
}



void copyarray(unsigned char * destarray, unsigned char * arraytocopy, unsigned int length )
{
  for (unsigned int i =0; i < length; i++ )
  {
      destarray[i] = arraytocopy[i];

  }
}


frameio net;             // gives us access to the raw network
message_queue ip_queue;  // message queue for the IP protocol stack
message_queue arp_queue; // message queue for the ARP protocol stack
message_queue arp_ether_frame;
message_queue cache_timer;


struct ether_frame       // handy template for 802.3/DIX frames
{
   octet dst_mac[6];     // destination MAC address
   octet src_mac[6];     // source MAC address
   octet prot[2];        // protocol (or length)
   octet data[1500];     // payload
};

struct mac_ip_pair{
  octet ip[4];
  octet mac[6];
};

struct macrequest
{
  octet ip[4];
  bool requestsent;
};
//
// This thread sits around and receives frames from the network.
// When it gets one, it dispatches it to the proper protocol stack.
//



void *protocol_loop(void *arg)
{
   ether_frame buf;
   while(1)
   {
      int n = net.recv_frame(&buf,sizeof(buf));
      if ( n < 42 ) continue; // bad frame!
      switch ( buf.prot[0]<<8 | buf.prot[1] )
      {
          case 0x800:
	      ip_queue.send(PACKET,buf.data,n);
	      break;
          case 0x806:
	    
             arp_queue.send(PACKET,buf.data,n);
             

             break;
      }
   }
}

//
// Toy function to print something interesting when an IP frame arrives
//
void *ip_protocol_loop(void *arg)
{
   octet buf[1500];
   event_kind event;
   int timer_no = 1;

   // for fun, fire a timer each time we get a frame
   while ( 1 )
   {
      ip_queue.recv(&event, buf, sizeof(buf));
      if ( event != TIMER )
      {
         //printf("got an IP frame from %d.%d.%d.%d, queued timer %d\n",   buf[12],buf[13],buf[14],buf[15],timer_no);
         ip_queue.timer(10,timer_no);
         timer_no++;
      }
      else
      {
         //printf("timer %d fired\n",*(int *)buf);
      }
   }
}

//
// Toy function to print something interesting when an ARP frame arrives
//


unsigned int count = 0;
void *arp_protocol_loop(void *arg)
{

  
  bool sendbroadcast;

  unsigned int indexforip = 0;
   //We will let this cache hold the MAC 
   octet broadcast[60];
   macrequest mac_request[100];
   unsigned int macrequestsize = 1;
   unsigned int bytes_sent = 0;
   unsigned int cache_length = 1;
   bool tocache = false;
   bool incache = false;
   bool isrequest;
   octet buf[1500];
   octet reply[42];
   struct mac_ip_pair cache[100];
   //construct our broadcast base


   event_kind event; // Enum for PACKET = 0 or TIMER = 1
   broadcast[6]  = MY_MAC[0];
   broadcast[7]  = MY_MAC[1];
   broadcast[8]  = MY_MAC[2];
   broadcast[9]  = MY_MAC[3];
   broadcast[10] = MY_MAC[4];
   broadcast[11] = MY_MAC[5];

   broadcast[22] = MY_MAC[0];
   broadcast[23] = MY_MAC[1];
   broadcast[24] = MY_MAC[2];
   broadcast[25] = MY_MAC[3];
   broadcast[26] = MY_MAC[4];
   broadcast[27] = MY_MAC[5];
   //our ip
   broadcast[28] = MY_IP[0];
   broadcast[29] = MY_IP[1];
   broadcast[30] = MY_IP[2];
   broadcast[31] = MY_IP[3];
   while ( 1 )
   {
     broadcast[0]  = 0xff;
   broadcast[1]  = 0xff;
   broadcast[2]  = 0xff;
   broadcast[3]  = 0xff;
   broadcast[4]  = 0xff;
   broadcast[5]  = 0xff;
   broadcast[12] = 0x08;
   broadcast[13] = 0x06;
   broadcast[14] = 0x00;
   broadcast[15] = 0x01;
   broadcast[16] = 0x08;
   broadcast[17] = 0x00;
   broadcast[18] = 0x06;
   broadcast[19] = 0x04;
   broadcast[20] = 0x00;
   broadcast[21] = 0x01;
   //our mac
   // broadcast[22] = MY_MAC[0];
   // broadcast[23] = MY_MAC[1];
   // broadcast[24] = MY_MAC[2];
   // broadcast[25] = MY_MAC[3];
   // broadcast[26] = MY_MAC[4];
   // broadcast[27] = MY_MAC[5];
   // //our ip
   // broadcast[28] = MY_IP[0];
   // broadcast[29] = MY_IP[1];
   // broadcast[30] = MY_IP[2];
   // broadcast[31] = MY_IP[3];
   //this next sequence is 0 for target mac since we don't have it
   broadcast[32] = 0x00;
   broadcast[33] = 0x00;
   broadcast[34] = 0x00;
   broadcast[35] = 0x00;
   broadcast[36] = 0x00;
   broadcast[37] = 0x00;
   //this is the targeted ip
   broadcast[38] = 0x00;
   broadcast[39] = 0x00;
   broadcast[40] = 0x00;
   broadcast[41] = 0x00;

    broadcast[42] = 0x00;
    broadcast[43] = 0x00;
    broadcast[44] = 0x00;
    broadcast[45] = 0x00;
    broadcast[46] = 0x00;
    broadcast[47] = 0x00;
    broadcast[48] = 0x00;
    broadcast[49] = 0x00;
    broadcast[50] = 0x00;
    broadcast[51] = 0x00;
    broadcast[52] = 0x00;
    broadcast[53] = 0x00;
    broadcast[54] = 0x00;
    broadcast[55] = 0x00;
    broadcast[56] = 0x00;
    broadcast[57] = 0x00;
    broadcast[58] = 0x00;
    broadcast[59] = 0x00;
    incache = false;
      arp_queue.recv(&event, buf, sizeof(buf));
      //printf("got an ARP %s\n", buf[7]==1? "request":"reply");
        //request
      
        if (buf[7] == 1)
          {

            isrequest = true;
        
            // for (int i = 24; i<28; i++)
            //     {
            //         printf("Here is some buff information: %02x ", buf[i]);
            //     }
            broadcast[38] = buf[24];
            broadcast[39] = buf[25];
            broadcast[40] = buf[26];
            broadcast[41] = buf[27];

            if (buf[24] == MY_IP[0] & buf[25] == MY_IP[1] & buf[26] == MY_IP[2] & buf[27] == MY_IP[3])
              {
                      
                      reply [0]  = TARGET_MAC[0];
                      reply [1]  = TARGET_MAC[1];
                      reply [2]  = TARGET_MAC[2];
                      reply [3]  = TARGET_MAC[3];
                      reply [4]  = TARGET_MAC[4];
                      reply [5]  = TARGET_MAC[5];
                      reply [6]  = MY_MAC[0];
                      reply [7]  = MY_MAC[1];
                      reply [8]  = MY_MAC[2];
                      reply [9]  = MY_MAC[3];
                      reply [10] = MY_MAC[4];
                      reply [11] = MY_MAC[5];
                      reply [12] = 0x08;
                      reply [13] = 0x06;
                      reply [14]  = 0x00;
                      reply [15]  = 0x01; 
                      reply [16]  = 0x08;
                      reply [17]  = 0x00; 
                      reply [18]  = 0x06; 
                      reply [19]  = 0x04; 

                      reply [20]  = 0x00;
                      reply [21]  = 0x02; //opcode is 0x0002 for reply

                      //These next are the sender MAC which is us 
                      reply [22]  = MY_MAC[0];
                      reply [23] =  MY_MAC[1];
                      reply [24] =  MY_MAC[2];
                      reply [25] =  MY_MAC[3];
                      reply [26] =  MY_MAC[4];
                      reply [27] =  MY_MAC[5];
                      //these next are the sender ip address (Hey that is this computer which is (129.123.5.55 or 
                      // 81 7b 05 37 in hex)
                      reply [28] = MY_IP[0]; 
                      reply [29] = MY_IP[1];
                      reply [30] = MY_IP[2];
                      reply [31] = MY_IP[3];
                      //this i for the computer that was next to me in the lab
                      // probably would be much better to grab these values from the buffer. But what is done is done

                      // target MAC
                      reply [32] = TARGET_MAC[0];
                      reply [33] = TARGET_MAC[1];
                      reply [34] = TARGET_MAC[2];
                      reply [35] = TARGET_MAC[3];
                      reply [36] = TARGET_MAC[4];
                      reply [37] = TARGET_MAC[5];
                      //target ip
                      reply [38] = TARGET_IP[0];
                      reply [39] = TARGET_IP[1];
                      reply [40] = TARGET_IP[2];
                      reply [41] = TARGET_IP[3];
                      
                      //printf("  here is the reference address: %u",&reply);
                   
                      //printf("  here is what we get for size %u \n",sizeof(reply));
                     bytes_sent = net.send_frame(&reply, sizeof(reply));
                     //printf("This is how many bytes were sent: %d \n", bytes_sent);
                }



          }
        //reply

        
        else 
        {
          isrequest = false;
          
            //here is where we will handle the caching
            //These iterates through all the cache values we have
            for (unsigned int i = 0; i< cache_length; i++)
                {
                  //this checks to see if the ip's match
                    
                    if (cache[i].ip[0] == buf[14] & cache[i].ip[1] == buf[15] & cache[i].ip[2] == buf[16]&cache[i].ip[3] == buf[17])
                      { 
                        //printf("Found in cache \n");
                        //it was found 
                        tocache = false;
                        i = cache_length;
                        incache = true;
                        indexforip = i;
                      }
                      else 
                      {
                        tocache = true;
                        //printf("do we get here2 \n");
                      }


                }

          if (tocache)
            {
              printf("cached ip is: ");
              //cache the ip
              for (unsigned int j = 0 ; j<4; j++)
                  {
                   printf(" %d", buf[14+j]);
                    cache[cache_length-1].ip[j] = buf[14+j];
                  }  
                  printf("\n");
              //add the accompanying mac
              for (unsigned int l = 0; l<6; l++)
                  {

                    cache[cache_length-1].mac[l] = buf[8+l];
                  }
                  cache_length++;
                 // printf("Cache Successful\n");

                  incache = true;
            }
            else{}

          }


        if (incache == true)
        {
          if (isrequest == true)
          {
            printf("Request for MAC-IP pair that we have in cache \n");
            // target MAC
                  reply [32] = cache[indexforip].mac[0];
                  reply [33] = cache[indexforip].mac[1];
                  reply [34] = cache[indexforip].mac[2];
                  reply [35] = cache[indexforip].mac[3];
                  reply [36] = cache[indexforip].mac[4];
                  reply [37] = cache[indexforip].mac[5];
                  //target ip
                  reply [38] = cache[indexforip].ip[0];
                  reply [39] = cache[indexforip].ip[1];
                  reply [40] = cache[indexforip].ip[2];
                  reply [41] = cache[indexforip].ip[3];
                  bytes_sent = net.send_frame(&reply, sizeof(reply));
                  printf("Reply was sent for %d bytes \n",bytes_sent);
          }
          else 
          {
            // In this scenario it is in the cache and it is a reply telling us the mac which isn't necessary
            printf("MAC-IP Pair Already Cached \n");
          }

        }
        else 
        {
          if (isrequest == true)
            {
              //printf("Request for MAC-IP pair that we do not have in cache\n");
              for (unsigned int m = 0; m<(macrequestsize); m++)
              { 
                        if (broadcast[38] == mac_request[m].ip[0] & broadcast[39] == mac_request[m].ip[1] & broadcast[40] == mac_request[m].ip[2] & broadcast[41] == mac_request[m].ip[3])
                        {
                            sendbroadcast = false;
                            m = macrequestsize;
                        
                        }
                        else{
                          sendbroadcast = true;
                        }


              }
              if (sendbroadcast == true)
              { 
                printf("Broadcast request sent\n");
                mac_request[macrequestsize-1].ip[0] = broadcast[38];
                mac_request[macrequestsize-1].ip[1] = broadcast[39];
                mac_request[macrequestsize-1].ip[2] = broadcast[40];
                mac_request[macrequestsize-1].ip[3] = broadcast[41];
                mac_request[macrequestsize-1].requestsent = true;

                bytes_sent = net.send_frame(&broadcast, sizeof(broadcast));
                macrequestsize= macrequestsize+1;
              }
              //printf("Broadcast for MAC sent bytes sent was %d \n",bytes_sent);



            }
          //this is the scenario where it is not in the cache and it is a reply Shouldn't happen
          else 
          {
            printf("this is area of the code we should not be reaching \n");
          }
        }

count = count + 1;
if (count == 50){
  count = 0;
    printf("Please enter IP: \n");

    
    scanf("%i %i %i %i",iptosend,(iptosend+1),(iptosend+2),(iptosend+3));
    printf("\n");

    printf("ip entered was: %d  %d  %d  %d \n",iptosend[0],iptosend[1],iptosend[2],iptosend[3]);
        incache = false;

        for (unsigned int i = 0; i< cache_length; i++)
                {
                  //this checks to see if the ip's match
                    if (cache[i].ip[0] == iptosend[0] & cache[i].ip[1] == iptosend[1] & cache[i].ip[2] == iptosend[2]&cache[i].ip[3] == iptosend[3])
                      { 
                        printf("Ip entered  request Found in cache Broadcasting it's MAC\n");
                        //it was found 
                            tocache = false;
                            indexforip = i;
                            i = cache_length;
                            incache = true;
                             broadcast[0]  = cache[indexforip].mac[0];
                             broadcast[1]  = cache[indexforip].mac[1];
                             broadcast[2]  = cache[indexforip].mac[2];
                             broadcast[3]  = cache[indexforip].mac[3];
                             broadcast[4]  = cache[indexforip].mac[4];
                             broadcast[5]  = cache[indexforip].mac[5];

                             broadcast[21] = 0x02;//for reply
                             //This is the value for what we are sharing
                             broadcast[32] = cache[indexforip].mac[0];
                             broadcast[33] = cache[indexforip].mac[1];
                             broadcast[34] = cache[indexforip].mac[2];
                             broadcast[35] = cache[indexforip].mac[3];
                             broadcast[36] = cache[indexforip].mac[4];
                             broadcast[37] = cache[indexforip].mac[5];
                             //this is the targeted ip
                             broadcast[38] = cache[indexforip].ip[0];
                             broadcast[39] = cache[indexforip].ip[1];
                             broadcast[40] = cache[indexforip].ip[2];
                             broadcast[41] = cache[indexforip].ip[3];
                        printf("here is our payload: \n");   
                        for (int a = 0; a <60; a++)
                        {
                          printf ("%02x ",broadcast[a]);
                          if (a == 15 | a == 31 | a == 47)
                            printf("\n");
                        }
                        printf("\n");     
                        bytes_sent = net.send_frame(&broadcast, sizeof(broadcast));
                        printf("Reply was sent for %d bytes \n",bytes_sent);

                      }
                      else 
                      {
                        tocache = true;
                        //printf("do we get here2 \n");
                      }
                }
                if (incache == false)
                {
                  printf("Ip entered not in cache\n");
                  broadcast[38] = iptosend[0];
                  broadcast[39] = iptosend[1];
                  broadcast[40] = iptosend[2];
                  broadcast[41] = iptosend[3];
                  bytes_sent = net.send_frame(&broadcast, sizeof(broadcast));
                  printf("\nBroadcast sent for %d bytes \n",bytes_sent);


                }
              }

   }
}

//
// if you're going to have pthreads, you'll need some thread descriptors
//
pthread_t loop_thread, arp_thread, ip_thread;

//
// start all the threads then step back and watch (actually, the timer
// thread will be started later, but that is invisible to us.)
//

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

