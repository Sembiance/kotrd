///////////////////////////////////////////////////////////////////////////
// StringUtility.h
///////////////////////////////////////////////////////////////////////////


// Function Definitions
char *			strchrtrim(char * str, char letter);	// Optional return value
char *			strrchrtrim(char * str, char letter);	// Optional return value

void			strtolower(char * string);

char *			strchrstrip(char * str, char letter);		// Optional return value
char *			strchrrep(char * str, char match, char replacement);	// Optional return value
char *			hextostr(char * str);						// Optional return value
char **			strchrexplode(char * strP, char sep);
unsigned long	strchrcount(char * str, char letter);
char *			strappend(char * original, char * more);
char *			strchrappend(char * original, char more);
char *			strrstr(char * haystack, char * needle);
char *			strfree(char * str);
unsigned long	strstrcount(char * str, char * match);
char **			strstrexplode(char * strP, char * match);
void			strdump(char * text, char * file);
char *			strstrstrip(char * str, char * strip);		// Optional return value
Boolean			strendswith(char * string, char * match);
Boolean			strstartswith(char * string, char * match);
char *			strstrrep(char * str, char * match, char * replacement);
/*char *		strndup(char * str, unsigned long length);*/
char *          strchrrepeat(char letter, unsigned long count);

char *			strenquote(char * str);

char *			memtostr(unsigned char * memory, unsigned long memoryLength);

char *			widetothin(unsigned char * wide);

#define STRREP(a, b)				((strfree(a)==0) ? b : b)
