///////////////////////////////////////////////////////////////////////////
// ArrayUtility.h
///////////////////////////////////////////////////////////////////////////

char **			array_free(char ** array);
char **			array_new(char * data);
unsigned long	array_len(char ** array);
char **			array_append(char ** oldArray, char * data);
void			array_dump(char ** array, char * file);
char *			array_fuse(char ** array, char * seperator);
long			array_find(char ** array, char * match);
long			array_find_in(char ** array, char * match);
char **			array_drop_last(char ** array);
char **			array_dup(char ** array);
char **			array_insert(char ** oldArray, char * data);
unsigned char	array_cmp(char ** firstArray, char ** secondArray);
char **			array_trim_char(char ** array, char c);
unsigned long	array_count_matches(char ** array, char * match);
char **			array_group_count_matches(char ** array);

void			array_sort(char ** array);
int				array_alpha_sort(const void *a, const void *b);	// Internal use only

void			array_sort_on_len(char ** array);
int				array_len_sort(const void *a, const void *b);	// Internal use only

void			array_sort_randomly(char ** array);
int				array_random_sort(const void *a, const void *b);	// Internal use only

void			array_sort_numerically(char ** array);
int				array_numeric_sort(const void *a, const void *b);	// Internal use only

