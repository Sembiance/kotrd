///////////////////////////////////////////////////////////////////////////
// StringUtility.c
///////////////////////////////////////////////////////////////////////////


// Standard Includes
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

// Additional Includes
#include "Utility.h"
#include "StringUtility.h"
#include "ArrayUtility.h"

///////////////////////////////////////////////////////////////////////////
// widetothin
char *			widetothin(unsigned char * wide)
{
	unsigned char * c=0;
	char *			thin=0;

	for(c=wide;*c!='\0';c+=2)
	{
		thin = strchrappend(thin, *c);
	}

	return thin;
}


///////////////////////////////////////////////////////////////////////////
// strtolower
void	strtolower(char * string)
{
	if(!string || !strlen(string))
		return;

	for(;*string;string++)
	{
		*string = tolower(*string);
	}
}


///////////////////////////////////////////////////////////////////////////
// strdump
void	strdump(char * text, char * file)
{
	FILE *	fp=0;

	if(!text || !file)
		return;

	fp = fopen(file, "wb");
	if(!fp)
		return;

	fprintf(fp, text);

	fclose(fp);
}


///////////////////////////////////////////////////////////////////////////
// strendswith
Boolean strendswith(char * string, char * match)
{
	if(!string || !(*string) || !match || !(*match))
		return false;

	if(strlen(match)>strlen(string))
		return false;

	if(!strcmp(string+(strlen(string)-strlen(match)), match))
		return true;

	return false;
}


///////////////////////////////////////////////////////////////////////////
// strstartswith
Boolean strstartswith(char * string, char * match)
{
	if(!string || !(*string) || !match || !(*match))
		return false;

	if(strlen(match)>strlen(string))
		return false;

	if(strstr(string, match)==string)
		return true;

	return false;
}


///////////////////////////////////////////////////////////////////////////
// strchrtrim
char * strchrtrim(char * str, char letter)
{
	if(!str)
		return str;

	while(*str==letter && *str)
		strcpy(str, str+1);

	return str;
}


///////////////////////////////////////////////////////////////////////////
// strrchrtrim
char * strrchrtrim(char * str, char letter)
{
	if(!str)
		return str;

	while(str[(strlen(str)-1)] == letter && strlen(str))
		str[(strlen(str)-1)] = '\0';

	return str;
}


///////////////////////////////////////////////////////////////////////////
// strchrstrip
char * strchrstrip(char * str, char letter)
{
	char *	c=0;

	if(!str)
		return str;

	for(c=str;*c;c++)
	{
		while(*c==letter)
			strcpy(c, c+1);
	}

	return str;
}


///////////////////////////////////////////////////////////////////////////
// strstrstrip
char * strstrstrip(char * str, char * strip)
{
	char *	c=0;

	if(!str || !strip)
		return str;

	for(c=str;*c;c++)
	{
		while(strstr(c, strip)==c)
			strcpy(c, c+strlen(strip));
	}

	return str;
}


///////////////////////////////////////////////////////////////////////////
// strchrrep
char * strchrrep(char * str, char match, char replacement)
{
	char *	c=0;
	int		length=0;

	if(!str)
		return str;

	for(c=str,length=strlen(str);length;length--,c++)
	{
		if(*c==match)
			*c = replacement;
	}

	return str;
}


///////////////////////////////////////////////////////////////////////////
// strstrrep
char * strstrrep(char * str, char * match, char * replacement)
{
	char *	newString=0;
	char *	newStringPointer=0;
	char *	originalString=0;
	char *	c=0;
	char	singleChar[2];
	long	newSize=0;

	if(!str || !match || !replacement || !strstr(str, match))
		return str;

	originalString = strdup(str);

	newSize = strlen(str)+(strstrcount(str, match)*(strlen(replacement)-strlen(match)))+1;

	newString = (char *)realloc(str, newSize);
	memset(newString, 0, newSize);
	newStringPointer = newString;

	for(c=originalString;*c;c++)
	{
		if(strstr(c, match)==c)
		{
			c+=strlen(match)-1;
			memcpy(newStringPointer, replacement, strlen(replacement));
			newStringPointer+=strlen(replacement);
		}
		else
		{
			singleChar[0] = *c;
			singleChar[1] = '\0';

			memcpy(newStringPointer, singleChar, strlen(singleChar));
			newStringPointer++;
		}
	}

	originalString= strfree(originalString);

	return newString;
}


///////////////////////////////////////////////////////////////////////////
// hextostr
char * hextostr(char * str)
{
	char *		c;
	char *		d;
	long		value;
	char		oneLetter[3];

	if(!str)
		return str;

	strchrstrip(str, ' ');

	if(strstr(str, "0x")==str)
		strcpy(str, str+2);

	if((strlen(str)%2)!=0)
		str[(strlen(str)-1)] = '\0';

	for(c=str,d=str;*c;c++,d++)
	{
		oneLetter[0] = *c;
		oneLetter[1] = *(++c);
		oneLetter[2] = '\0';
		value = strtol(oneLetter, 0, 16);

		sprintf(oneLetter, "%c", (char)value);
		strcpy(d, oneLetter);
	}

	return str;
}


///////////////////////////////////////////////////////////////////////////
// strstrexplode
char ** strstrexplode(char * strP, char * match)
{
	char *				str;
	char *				lastMatch;
	char *				currentMatch;
	char **				array=0;
	char *				temp=0;
	char *				originalstr;

	if(!strP || !match)
		return 0;

	str = strdup(strP);
	originalstr = str;

	for(currentMatch=strstr(str, match),lastMatch=str;currentMatch;currentMatch=strstr(lastMatch, match))
	{
		if(currentMatch!=lastMatch)
		{
			temp = (char *)malloc((currentMatch-lastMatch)+1);
			memset(temp, 0, (currentMatch-lastMatch)+1);
			strncpy(temp, lastMatch, currentMatch-lastMatch);
			array = array_append(array, temp);
			temp = strfree(temp);

			lastMatch+=currentMatch-lastMatch;
		}

		lastMatch += strlen(match);
	}

	if(lastMatch<(str+strlen(str)))
		array = array_append(array, lastMatch);

	free(originalstr);

	return array;
}

///////////////////////////////////////////////////////////////////////////
// strchrexplode
char ** strchrexplode(char * strP, char sep)
{
	unsigned long	count, i;
	char **			array;
	char *			str;
	char *			originalstr;

	if(!strP)
		return 0;

	str = strdup(strP);
	originalstr = str;

	//strchrtrim(str, sep);
	//strrchrtrim(str, sep);

	if(strlen(str)==0)
	{
		originalstr = strfree(originalstr);
		return 0;
	}

	count = strchrcount(str, sep)+1;

	array = (char **)malloc(sizeof(char *)*(count+1));
	memset(array, 0, (sizeof(char *)*(count+1)));

	strchrrep(str, sep, '\0');

	for(i=0;i<count;i++)
	{
		if(!strlen(str))
		{
			array[i] = (char *)malloc(1);
			array[i][0] = '\0';
		}
		else
			array[i] = strdup(str);

		str = str+strlen(str)+1;
	}

	free(originalstr);

	return array;
}


///////////////////////////////////////////////////////////////////////////
// strstrcount
unsigned long strstrcount(char * str, char * match)
{
	unsigned long	i=0;

	if(!str)
		return 0;

	while(*str)
	{
		if(strstr(str, match)==str)
			i++;

		str++;
	}

	return i;
}

///////////////////////////////////////////////////////////////////////////
// strchrcount
unsigned long strchrcount(char * str, char letter)
{
	unsigned long		i=0;

	if(!str)
		return 0;

	while(*str)
	{
		if(*str==letter)
			i++;

		str++;
	}

	return i;
}


///////////////////////////////////////////////////////////////////////////
// strchrappend
char *	strchrappend(char * original, char more)
{
	char	buf[2];

	buf[0] = more;
	buf[1] = '\0';
	
	return strappend(original, buf);
}


///////////////////////////////////////////////////////////////////////////
// strappend
char *	strappend(char * original, char * more)
{
	char *	endResult=0;

	if(!more)
		return (original ? original : 0);

	if(!original)
		return strdup(more);

	endResult = (char *)realloc(original, strlen(original)+strlen(more)+1);
	memset(endResult+strlen(endResult), 0, strlen(more)+1);
	strncpy(endResult+strlen(endResult), more, strlen(more));

	return endResult;
}


///////////////////////////////////////////////////////////////////////////
// strrstr
char *	strrstr(char * haystack, char * needle)
{
	char *	c=0;

	if(!haystack || !needle || strlen(haystack)<strlen(needle))
		return 0;

	c = haystack+(strlen(haystack)-strlen(needle));

	for(;strstr(c, needle)!=c && c>haystack;c--)
		;

	if(c==haystack && strstr(haystack, needle)!=haystack)
		return 0;

	return c;
}


///////////////////////////////////////////////////////////////////////////
// strfree
char *	strfree(char * str)
{
	if(str)
		free(str);

	return 0;
}


///////////////////////////////////////////////////////////////////////////
// memtostr
char *			memtostr(unsigned char * memory, unsigned long memoryLength)
{
	char *	str=0;

	if(!memory || !memoryLength)
		return 0;

	str = (char *)malloc(memoryLength+1);
	memset(str, 0, memoryLength+1);
	memcpy(str, memory, memoryLength);

	return str;
}


///////////////////////////////////////////////////////////////////////////
// strenquote
char *	strenquote(char * str)
{
	char *				newStr=0;
	unsigned long		len=0;
	
	if(!str)
		return str;

	len = strlen(str);

	newStr = (char *)malloc(len+3);
	newStr[0] = '"';
	memcpy(newStr+1, str, len);
	newStr[len+1] = '"';
	newStr[len+2] = '\0';

	free(str);

	return newStr;
}
///////////////////////////////////////////////////////////////////////////
// strndup
char *		strndup(char * str, unsigned long length)
{
	char *		result;

	if(!str || !strlen(str) || length>strlen(str))
		return 0;

	result = (char *)malloc(length+1);
	memset(result, 0, length+1);
	memcpy(result, str, length);

	return result;
}

///////////////////////////////////////////////////////////////////////////
// strchrrepeat
char *  strchrrepeat(char letter, unsigned long count)
{
    char *          newData=0;

    if(!count)
        return 0;
    
    newData = (char *)malloc(count+1);
    memset(newData, 0, count+1);
    memset(newData, letter, count);
    
    return newData;
}

