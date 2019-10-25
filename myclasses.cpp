#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <iostream> 
#include <iomanip>
#include <stdlib.h>
#include "myclasses.h"



bool cache::check(std::array<octet,6>& iptocheck, std:;array<octet, 6> & mactocheck)

{
	if (iptocheck == ip & mactocheck == mac)
		{
			return 	true;

		}

	else 
		{
			return false;
		}
}





void cache::printpair()
		{
			for (int i = 0; i < 4; i++)
				printf("%d ",ip[i]);

			printf("\n");

			for (int j = 0; j < 6; j++)
				printf("%02x:",mac[j]);

			printf("\n");


		}
