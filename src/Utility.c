///////////////////////////////////////////////////////////////////////////
// Utility.c
///////////////////////////////////////////////////////////////////////////


// Standard Includes
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <dirent.h>

#include <time.h>

// Additional Includes
#include "Utility.h"
#include "StringUtility.h"
#include "ArrayUtility.h"

//////////////////////////////////////////////////////////////////////////
// MakeTime
long				MakeTime(int month, int day, int year)
{
	struct tm	timeTM;

	// Month, of year, 0 - 11
	timeTM.tm_mon = month-1;

	// Day, of month, 1 - 31
	timeTM.tm_mday = day;

	// Year, number of since 1900
	timeTM.tm_year = year-1900;

	// Hour, of day, 0 - 23
	timeTM.tm_hour = 0;
	timeTM.tm_min = 0;
	timeTM.tm_sec = 0;
	timeTM.tm_wday = 0;
	timeTM.tm_yday = 0;
	timeTM.tm_isdst = -1;

	return mktime(&timeTM);
}


//////////////////////////////////////////////////////////////////////////
// chartoi
int					chartoi(char theChar)
{
	char	two[2];

	two[0] = theChar;
	two[1] = '\0';

	return atoi(two);
}

//////////////////////////////////////////////////////////////////////////
// memcut
unsigned char *		memcut(unsigned char ** dataP, unsigned long * dataLengthP, unsigned char * ripStart, unsigned long ripLength)
{
	unsigned char *		data=0;
	unsigned char *		segment=0;
	unsigned char *		newData=0;
	unsigned long		newDataLength=0, dataLength=0;

	if(!dataP || !dataLengthP || !ripStart || !ripLength)
		return 0;

	data = *dataP;
	dataLength = *dataLengthP;

	if(ripStart<data || ripStart>=(data+dataLength) || ripStart+ripLength>(data+dataLength))
		return 0;

	segment = (unsigned char *)malloc(ripLength);
	memcpy(segment, ripStart, ripLength);

	if(dataLength>ripLength)
	{
		newData = (unsigned char *)malloc(dataLength-ripLength);
		newDataLength = 0;

		if(ripStart>data)
		{
			memcpy(newData, data, ripStart-data);
			newDataLength+=ripStart-data;
		}

		if((ripStart+ripLength)<(data+dataLength))
		{
			memcpy(newData+newDataLength, ripStart+ripLength, (data+dataLength)-(ripStart+ripLength));
			newDataLength+=(data+dataLength)-(ripStart+ripLength);
		}
	}

	*dataP = memfree(*dataP);
	data = 0;
	*dataLengthP = 0;

	if(newData && newDataLength)
	{
		*dataP = newData;
		*dataLengthP = newDataLength;
	}

	return segment;
}


//////////////////////////////////////////////////////////////////////////
// memfind
unsigned char *		memfind(unsigned char * data, unsigned long dataLength, unsigned char * match, unsigned long matchLength)
{
	unsigned char *		p=0;

	if(!data || !dataLength || !match || !matchLength || matchLength>dataLength)
		return 0;

	for(p=data;p<=data+(dataLength-matchLength);p++)
	{
		if(!memcmp(p, match, matchLength))
			return p;
	}

	return 0;
}


//////////////////////////////////////////////////////////////////////////
// memdup
/*unsigned char *	memdup(unsigned char * data, unsigned long size)
{
	unsigned char *	newData=0;

	if(!data || !size)
		return 0;

	newData = (unsigned char *)malloc(size);

	memcpy(newData, data, size);

	return newData;
}*/

//////////////////////////////////////////////////////////////////////////
// memfree
unsigned char *	memfree(unsigned char * data)
{
	if(data)
		free(data);

	return 0;
}

//////////////////////////////////////////////////////////////////////////
// MinimizeBytes
Boolean		MinimizeBytes(double source, double * targetNum, char ** targetString)
{
	if(!source || !targetNum || !targetString)
		return false;

	if(source>=1073741824)		// GB in bytes
	{
		*targetNum = source/1024;		// MB
		*targetNum = *targetNum/1024;	// KB
		*targetNum = *targetNum/1024;	// bytes

		*targetString = strdup("GB");
	}
	else if(source>=1048576)	// MB in bytes
	{
		*targetNum = source/1024;		// KB
		*targetNum = *targetNum/1024;	// bytes

		*targetString = strdup("MB");
	}
	else if(source>=1024)		// KB in bytes
	{
		*targetNum = source/1024;		// KB
		*targetString = strdup("KB");
	}
	else
	{
		*targetNum = source;			// bytes
		*targetString = strdup("bytes");
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////
// GetTicks
double		GetTicks(void)
{
	struct timeval		tv;

	#ifdef _WINDOWS
		return (double)GetTickCount();
	#else
		gettimeofday(&tv, 0);

		return (double)((((double)tv.tv_sec)*1000) + (((double)tv.tv_usec)/1000));
	#endif
}


//////////////////////////////////////////////////////////////////////////
// memcharpop
unsigned char memcharpop(unsigned char * memory, unsigned long memoryLength, unsigned char ** memoryDest)
{
	unsigned char		c;
	unsigned char *		newMemory;

	if(!memory || !memoryLength)
		return 0;

	c = *memory;

	if((memoryLength-1)==0)
	{
		memory = memfree(memory);
		*memoryDest = 0;
	}

	newMemory = (unsigned char *)malloc(memoryLength-1);
	memcpy(newMemory, memory+1, memoryLength-1);
	
	memory = memfree(memory);

	*memoryDest = newMemory;

	return c;
}


//////////////////////////////////////////////////////////////////////////
// memappend
unsigned char *	memappend(unsigned char * start, unsigned long startLength, unsigned char * appendage, unsigned long appendageLength)
{
	if(!start && !appendage)
		return start;

	if(!startLength && !appendageLength)
		return start;

	if(!appendage || !appendageLength)
		return start;

	if(!start)
	{
		start = (unsigned char *)malloc(appendageLength);
		memcpy(start, appendage, appendageLength);
	}
	else
	{
		start = (unsigned char *)realloc(start, startLength+appendageLength);
		memcpy(start+startLength, appendage, appendageLength);
	}

	return start;
}


//////////////////////////////////////////////////////////////////////////
// udiff
unsigned long		udiff(unsigned long one, unsigned long two)
{
	if(one>two)
		return one-two;

	return two-one;
}


//////////////////////////////////////////////////////////////////////////
// GetDaysInMonth
unsigned char		GetDaysInMonth(unsigned char monthNum, unsigned long yearNum)
{
	if(monthNum==0 || monthNum==2 || monthNum==4 || monthNum==6 || monthNum==7 || monthNum==9 || monthNum==11)
		return 31;
	else if(monthNum==3 || monthNum==5 || monthNum==8 || monthNum==10)
		return 30;
	else if(monthNum==1)
	{
		if((((yearNum % 4)==0) && ((yearNum % 100)!=0)) || ((yearNum % 400)==0))
			return 29;
		else
			return 28;
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////
// GetDirListing
char ** GetDirListing(char * dirPath)
{
    DIR *           dir=0;
    struct dirent * entry=0;
    char **         listing=0;
    
    if(!(dir=opendir(dirPath)))
        return 0;
    
    while((entry=readdir(dir)))
    {
        listing = array_append(listing, entry->d_name);
    }
    
    closedir(dir);
    
    return listing;
}

