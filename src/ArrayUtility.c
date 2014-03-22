///////////////////////////////////////////////////////////////////////////
// ArrayUtility.c
///////////////////////////////////////////////////////////////////////////


// Standard Includes
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "Utility.h"
#include "ArrayUtility.h"
#include "StringUtility.h"

int	number_range(int from, int to);

///////////////////////////////////////////////////////////////////////////
// array_trim_char
char **			array_trim_char(char ** array, char c)
{
	char **		ar;
	if(!array || array_len(array)==0)
		return array;

	for(ar=array;*ar;ar++)
	{
		*ar = strchrtrim(*ar, c);
		*ar = strrchrtrim(*ar, c);
	}

	return array;
}


///////////////////////////////////////////////////////////////////////////
// array_cmp
unsigned char	array_cmp(char ** firstArray, char ** secondArray)
{
	char **	ar;
	char ** arTwo;

	if(!firstArray || !secondArray)
		return 0;

	if(array_len(firstArray)==0 || array_len(secondArray)==0)
		return 0;

	if(array_len(firstArray)!=array_len(secondArray))
		return 1;

	for(ar=firstArray,arTwo=secondArray;ar && arTwo && *ar && *arTwo;ar++,arTwo++)
	{
		if(strcmp(*ar, *arTwo))
			return 1;
	}

	return 0;
}


///////////////////////////////////////////////////////////////////////////
// array_free
char **	array_free(char ** array)
{
	char **		ac=0;

	if(!array)
		return array;

	for(ac=array;*ac;ac++)
	{
		*ac = strfree(*ac);
	}

	free(array);

	return 0;
}


///////////////////////////////////////////////////////////////////////////
// array_find
long	array_find(char ** array, char * match)
{
	long	i=0;

	if(!array || !match || !(*match))
		return -1;

	for(i=0;*array;array++,i++)
	{
		if(!strcmp(*array, match))
			return i;
	}

	return -1;
}


///////////////////////////////////////////////////////////////////////////
// array_find_in
long	array_find_in(char ** array, char * match)
{
	long	i=0;

	if(!array || !match || !(*match))
		return -1;

	for(i=0;*array;array++,i++)
	{
		if(strstr(*array, match))
			return i;
	}

	return -1;
}


///////////////////////////////////////////////////////////////////////////
// array_len
unsigned long	array_len(char ** array)
{
	char **			ar=0;
	unsigned long	i=0;

	if(!array || !(*array))
		return 0;

	for(ar=array,i=0;*ar;ar++,i++)
		;

	return i;
}


///////////////////////////////////////////////////////////////////////////
// array_new
char ** array_new(char * data)
{
	char **		newArray=0;

	if(!data)
	{
		newArray = (char **)malloc(sizeof(char *));
		memset(newArray, 0, (sizeof(char *)));
	}
	else
	{
		newArray = (char **)malloc(sizeof(char *)*2);
		memset(newArray, 0, (sizeof(char *)*2));
		newArray[0] = strdup(data);
	}		

	return newArray;
}


///////////////////////////////////////////////////////////////////////////
// array_append
char **	array_append(char ** oldArray, char * data)
{
	unsigned long	oldLength=0;
	char **			newArray=0;

	if(!oldArray)
		return array_new(data);

	oldLength = array_len(oldArray);

	newArray = (char **)realloc(oldArray, (sizeof(char *)*(oldLength+2)));

	newArray[oldLength] = strdup(data);
	newArray[oldLength+1] = 0;

	return newArray;
}


///////////////////////////////////////////////////////////////////////////
// array_insert
char ** array_insert(char ** oldArray, char * data)
{
	char **			ar=0;
	char **			newArray=0;

	if(!oldArray)
		return array_new(data);
	
	newArray = array_new(data);

	for(ar=oldArray;ar && *ar;ar++)
	{
		newArray = array_append(newArray, *ar);
	}

	oldArray = array_free(oldArray);

	return newArray;
}


///////////////////////////////////////////////////////////////////////////
// array_dump
void	array_dump(char ** array, char * file)
{
	FILE *			fp=0;
	char **			ac=0;

	if(!array || !(*array))
		return;

	if(file && *file)
	{
		fp = fopen(file, "wb");
		if(!fp)
			return;
	}

	for(ac=array;*ac;ac++)
	{
		if(file && *file)
			fprintf(fp, "%s\n", *ac);
		else
			fprintf(stderr, "%s\n", *ac);
	}

	if(file && *file)
		fclose(fp);
}


///////////////////////////////////////////////////////////////////////////
// array_fuse
char *	array_fuse(char ** array, char * seperator)
{
	char *			result=0;
	char **			ac=0;
	unsigned long	size=0;

	if(!array || !(*array))
		return 0;

	for(size=0,ac=array;*ac;ac++)
	{
		size+=strlen(*ac);
		if(seperator)
			size+=strlen(seperator);
	}
	size++;		// Term zero

	if(!size)
		return 0;

	result = (char *)malloc(size);
	memset(result, 0, size);

	for(ac=array;*ac;ac++)
	{
		strcat(result, *ac);
		if(seperator)
			strcat(result, seperator);
	}

	if(seperator)
		*strrstr(result, seperator) = '\0';

	return result;
}


///////////////////////////////////////////////////////////////////////////
// array_drop_last
char **	array_drop_last(char ** array)
{
	char **			ac=0;
	unsigned long	size=0;

	if(!array || !(*array))
		return array;

	size = array_len(array);
	if(size==1)
		return array_free(array);

	for(ac=array;*ac;ac++)
		;

	ac--;
	free(*ac);
	*ac = 0;

	array = (char **)realloc(array, sizeof(char *)*size);

	return array;
}


///////////////////////////////////////////////////////////////////////////
// array_dup
char ** array_dup(char ** array)
{
	char **			newArray=0;
	char **			ac=0;

	if(!array || !(*array))
		return 0;

	for(ac=array;*ac;ac++)
	{
		newArray = array_append(newArray, *ac);
	}

	return newArray;
}


///////////////////////////////////////////////////////////////////////////
// array_count_matches
unsigned long			array_count_matches(char ** array, char * match)
{
	char **			ar=0;
	unsigned long	count=0;

	for(ar=array;*ar;ar++)
	{
		if(!strcmp(*ar, match))
			count++;
	}

	return count;
}

///////////////////////////////////////////////////////////////////////////
// array_group_count
char **			array_group_count_matches(char ** array)
{
	char **		ar=0;
	char **		newArray=0;
	char *		lastItem=0;
	char		count[32];

	if(!array || !(*array))
		return 0;

	for(ar=array;*ar;ar++)
	{
		if(!lastItem || strcmp(lastItem, *ar))
		{
			lastItem = strfree(lastItem);
			lastItem = strdup(*ar);

			newArray = array_append(newArray, *ar);
			sprintf(count, "%lu", array_count_matches(array, *ar));
			newArray = array_append(newArray, count);
		}
	}

	return newArray;
}


///////////////////////////////////////////////////////////////////////////
// array_sort
void	array_sort(char ** array)
{
	if(!array || !array_len(array))
		return;

	qsort(array, array_len(array), sizeof(char *), array_alpha_sort);
}

// array_alpha_sort - INTERNAL USE ONLY
int array_alpha_sort(const void *a, const void *b)
{
  return (strcmp(*(char **)a, *(char **)b));
}


///////////////////////////////////////////////////////////////////////////
// array_sort_on_len
void	array_sort_on_len(char ** array)
{
	if(!array || !array_len(array))
		return;

	qsort(array, array_len(array), sizeof(char *), array_len_sort);
}

// array_len_sort
int array_len_sort(const void *a, const void *b)
{
  return (strlen(*(char **)a)-strlen(*(char **)b));
}


///////////////////////////////////////////////////////////////////////////
// array_sort_randomly
void	array_sort_randomly(char ** array)
{
	if(!array || !array_len(array))
		return;

	qsort(array, array_len(array), sizeof(char *), array_random_sort);
}

// array_random_sort - INTERNAL USE ONLY
int array_random_sort(const void *a, const void *b)
{
  return (number_range(0, 2)-1);
}


///////////////////////////////////////////////////////////////////////////
// array_sort_numerically
void			array_sort_numerically(char ** array)
{
	if(!array || !array_len(array))
		return;

	qsort(array, array_len(array), sizeof(char *), array_numeric_sort);
}

// array_numeric_sort - INTERNAL USE ONLY
int				array_numeric_sort(const void *a, const void *b)
{
	if(atoi(*(char **)a)<atoi(*(char **)b))
		return -1;
	else if(atoi(*(char **)a)==atoi(*(char **)b))
		return 0;
	
	return 1;
}
