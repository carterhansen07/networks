
#include <stdio.h>
#include <iostream> 
#include <iomanip>
#include <fstream>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>







void copyarray(unsigned char * destarray, unsigned char * arraytocopy, unsigned int length )
{
  for (unsigned int i =0; i < length; i++ )
  {
  		destarray[i] = arraytocopy[i];

  }
}


void getmac(unsigned char* mac)
{
unsigned char my_mac[8];
unsigned char temp;
unsigned char semicolon;
unsigned char useless;
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


int main(){






unsigned char my_mac[8];

getmac(my_mac);

printf("Mac is: ");
for (unsigned int i = 0; i < 6; i++)
	{
		printf("%02x:",my_mac[i]);


	}
printf("\n");





return 0;


}


