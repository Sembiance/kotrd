///////////////////////////////////////////////////////////////////////////
// Utility.h
///////////////////////////////////////////////////////////////////////////

// Standard Boolean type definition
typedef char	Boolean;

// Data truth values
#ifndef true
	#define true 1
#endif

#ifndef false
	#define false 0
#endif

// General Macros
#define ISLETTER(a)					(((((a) >= 0x41) && ((a) <= 0x5A)) || (((a) >= 0x61) && ((a) <= 0x7A))) ? 1 : 0)	// Returns true if a is a letter (non symbol, non number)
#define ISNUMBER(a)					((((a) >= 0x30) && ((a) <= 0x39)) ? 1 : 0)		// Returns true if a is a ASCII number
#define ISSPACE(a)					(((a) == 0x20) ? 1 : 0)		// Returns true if a is a ASCII space character
#define ISPRINTABLE(a)				((((a) >= 0x20) && ((a) <= 0x7E)) ? 1 : 0)		// Returns true if a is ASCII printable value
#define rint(X)						(((X-floor(X))<0.5)?floor(X):ceil(X))

// Assorted Functions
//unsigned char *		memdup(unsigned char * data, unsigned long size);
unsigned char *		memfree(unsigned char * data);
double				GetTicks(void);
Boolean				MinimizeBytes(double source, double * targetNum, char ** targetString);
unsigned char *		memfind(unsigned char * data, unsigned long dataLength, unsigned char * match, unsigned long matchLength);
unsigned char *		memcut(unsigned char ** dataP, unsigned long * dataLengthP, unsigned char * ripStart, unsigned long ripLength);
unsigned char *		memappend(unsigned char * start, unsigned long startLength, unsigned char * appendage, unsigned long appendageLength);
unsigned char		memcharpop(unsigned char * memory, unsigned long memoryLength, unsigned char ** memoryDest);
unsigned long		udiff(unsigned long one, unsigned long two);
unsigned char		GetDaysInMonth(unsigned char monthNum, unsigned long yearNum);
long				MakeTime(int month, int day, int year);
int					chartoi(char theChar);
char **             GetDirListing(char * dirPath);

