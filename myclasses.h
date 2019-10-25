
#ifndef MYCLASSES_H
#define MYCLASSES_H

typedef unsigned char octet;
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <iostream> 
#include <iomanip>
#include <stdlib.h>
#include <vector>
#include <array>


class cache{

public:
cache(std::array<octet,4>& iptoadd , std::array<octet,6>&  mactoadd, int position)
{
	ip = iptoadd;
	mac = mactoadd;
	vector_position = position;
	isvalid = true;
}

~cache();

bool check(std::array<octet,6>& iptocheck, std:;array<octet, 6> & mactocheck);
bool isvalid()
	{
		return isvalid;
	}

int get_location()
{
	return vector_position;
}

void printpair();


private: 
bool isvalid;
std::array<octet, 4> ip;

std::array<octet, 6> mac;
int vector_position;

};


#endif