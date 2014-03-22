#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"


/*****************************************************************************
 Name:		string_edit
 Purpose:	Clears string and puts player into editing mode.
 Called by:	none
 ****************************************************************************/
void string_edit( CHAR_DATA *ch, char **pString )
{
    send_to_char( "-========- Entering EDIT Mode -=========-\n\r", ch );
    send_to_char( "    Type .h on a new line for help\n\r", ch );
    send_to_char( " Terminate with a ~ or @ on a blank line.\n\r", ch );
    send_to_char( "-=======================================-\n\r", ch );
   send_to_char("{x000000000{W1{x111111111{W2{x222222222{W3{x333333333{W4{x444444444{W5{x555555555{W6{x666666666{W7{x777777777{W8{x\n\r", ch);
   send_to_char("{x123456789{W0{x123456789{W0{x123456789{W0{x123456789{W0{x123456789{W0{x123456789{W0{x123456789{W0{x123456789{W0{x\n\r", ch);

    if ( *pString == NULL )
    {
        *pString = str_dup( "" );
    }
    else
    {
        **pString = '\0';
    }

    ch->desc->pString = pString;

    return;
}



/*****************************************************************************
 Name:		string_append
 Purpose:	Puts player into append mode for given string.
 Called by:	(many)olc_act.c
 ****************************************************************************/
void string_append( CHAR_DATA *ch, char **pString )
{
    send_to_char( "-=========================================-\n\r", ch );
    send_to_char( "    Type .h on a new line for help\n\r", ch );
    send_to_char( " Terminate with a .q or /q on a blank line.\n\r", ch );
    send_to_char( "-==========================================-\n\r", ch );
   send_to_char("{x000000000{W1{x111111111{W2{x222222222{W3{x333333333{W4{x444444444{W5{x555555555{W6{x666666666{W7{x777777777{W8{x\n\r", ch);
   send_to_char("{x123456789{W0{x123456789{W0{x123456789{W0{x123456789{W0{x123456789{W0{x123456789{W0{x123456789{W0{x123456789{W0{x\n\r", ch);

    if ( *pString == NULL )
    {
        *pString = str_dup( "" );
    }
    send_to_char( *pString, ch );
    
    if ( *(*pString + strlen( *pString ) - 1) != '\r' )
    send_to_char( "\n\r", ch );

    ch->desc->pString = pString;

    return;
}



/*****************************************************************************
 Name:		string_replace
 Purpose:	Substitutes one string for another.
 Called by:	string_add(string.c) (aedit_builder)olc_act.c.
 ****************************************************************************/
char * string_replace( char * orig, char * old, char * new )
{
    char xbuf[MAX_STRING_LENGTH];
    int i;

    xbuf[0] = '\0';
    strcpy( xbuf, orig );
    if ( strstr( orig, old ) != NULL )
    {
        i = strlen( orig ) - strlen( strstr( orig, old ) );
        xbuf[i] = '\0';
        strcat( xbuf, new );
        strcat( xbuf, &orig[i+strlen( old )] );
        free_string( orig );
    }

    return str_dup( xbuf );
}



/*****************************************************************************
 Name:		string_add
 Purpose:	Interpreter for string editing.
 Called by:	game_loop_xxxx(comm.c).
 ****************************************************************************/
void string_add( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    /*
     * Thanks to James Seng
     */
    smash_tilde( argument );

    if ( *argument == '.' || *argument == '/' )
    {
        char arg1 [MAX_INPUT_LENGTH];
        char arg2 [MAX_INPUT_LENGTH];
        char arg3 [MAX_INPUT_LENGTH];

        argument = one_argument( argument, arg1 );

	if ( !str_cmp( arg1, ".i" ) || !str_cmp( arg1, "/i" ) )
	{
	    *ch->desc->pString = string_insline( ch, argument, *ch->desc->pString );
	    return;
	}

        argument = first_arg( argument, arg2, FALSE );
        argument = first_arg( argument, arg3, FALSE );

        if ( !str_cmp( arg1, ".c" ) || !str_cmp( arg1, "/c" ) )
        {
            send_to_char( "\n\r{CString cleared{x\n\r", ch );
            **ch->desc->pString = '\0';
            return;
        }
	if ( !str_cmp( arg1, ".d" ) || !str_cmp( arg1, "/d" ) )
	{
    	    *ch->desc->pString = string_delline( ch, arg2, *ch->desc->pString );
	    return;
	}

        if ( !str_cmp( arg1, ".s" ) || !str_cmp( arg1, "/s" ) )
        {
            send_to_char( "\n\r{CString so far{x:{x\n\r", ch );
	    string_show( ch, *ch->desc->pString );
            return;
        }

        if ( !str_cmp( arg1, ".r" ) || !str_cmp( arg1, "/r" ) )
        {
            if ( arg2[0] == '\0' )
            {
                send_to_char(
            "\n\r{GSyntax{x:  {D({W/r {cor {W.r{D) {c<{W\"OLD STRING\"{c> <{W\"NEW STRING\"{c>{x\n\r",ch);
             send_to_char(
            "{cEXAMPLE{x: {C/r \"OLD TEXT\" \"NEW TEXT\" {c- {WText MUST be enclosed within{R\"{C...{x\n\r",ch);
                return;
            }

            *ch->desc->pString =
                string_replace( *ch->desc->pString, arg2, arg3 );
            sprintf( buf, "\n\r{CThe string {c<{W%s{c> {Chas been replaced with {c<{W%s{c>{x\n\r", arg2, arg3 );
            send_to_char( buf, ch );
            return;
        }

        if ( !str_cmp( arg1, ".f" ) || !str_cmp( arg1, "/f" ) )
        {
            *ch->desc->pString = format_string( *ch->desc->pString );
            send_to_char( "\n\r{CString formatted{x\n\r", ch );
            return;
        }
        if ( !str_cmp( arg1, ".q" ) || !str_cmp( arg1, "/q" ) )
        {
	    ch->desc->pString = NULL;
	    send_to_char( "\n\r{WDONE\n\r{cType {CREPLAY {cto see any missed tells.{x\n\r", ch );
	    return;
        }        
        if ( !str_cmp( arg1, ".h" ) || !str_cmp( arg1, "/h" ) )
        {
            send_to_char( "Sedit help (commands on blank line):   \n\r", ch );
            send_to_char( ".r 'old' 'new'   - replace a substring \n\r", ch );
            send_to_char( "                   (requires '', \"\") \n\r", ch );
	    send_to_char( ".d #             - delete line #       \n\r", ch );
	    send_to_char( ".i # <string>    - insert at line #    \n\r", ch );
            send_to_char( ".h               - get help (this info)\n\r", ch );
            send_to_char( ".s               - show string so far  \n\r", ch );
            send_to_char( ".f               - (word wrap) string  \n\r", ch );
            send_to_char( ".c               - clear string so far \n\r", ch );
            send_to_char( ".q               - end string          \n\r", ch );
	    send_to_char( "     '.' can be substituted for '/'    \n\r", ch );
            return;
        }
            

        send_to_char( "SEdit:  Invalid dot command.\n\r", ch );
        return;
    }

    if ( *argument == '~' )
    {
        ch->desc->pString = NULL;
        return;
    }

    strcpy( buf, *ch->desc->pString );

    /*
     * Truncate strings to MAX_STRING_LENGTH.
     * --------------------------------------
     */
    if ( strlen( buf ) + strlen( argument ) >= ( MAX_STRING_LENGTH - 4 ) )
    {
        send_to_char( "String too long, last line skipped.\n\r", ch );

	/* Force character out of editing mode. */
        ch->desc->pString = NULL;
        return;
    }

    /*
     * Ensure no tilde's inside string.
     * --------------------------------
     */
    smash_tilde( argument );

    strcat( buf, argument );
    strcat( buf, "\n\r" );
    free_string( *ch->desc->pString );
    *ch->desc->pString = str_dup( buf );
    return;
}



/*
 * Thanks to Kalgen for the new procedure (no more bug!)
 * Original wordwrap() written by Surreality.
 */
/*****************************************************************************
 Name:		format_string
 Purpose:	Special string formating and word-wrapping.
 Called by:	string_add(string.c) (many)olc_act.c
 ****************************************************************************/
char *format_string( char *oldstring /*, bool fSpace */)
{
  char xbuf[MAX_STRING_LENGTH];
  char xbuf2[MAX_STRING_LENGTH];
  char *rdesc;
  int i=0;
  bool cap=TRUE;
  
  xbuf[0]=xbuf2[0]=0;
  
  i=0;
  
  for (rdesc = oldstring; *rdesc; rdesc++)
  {
    if (*rdesc=='\n')
    {
      if (xbuf[i-1] != ' ')
      {
        xbuf[i]=' ';
        i++;
      }
    }
    else if (*rdesc=='\r') ;
    else if (*rdesc==' ')
    {
      if (xbuf[i-1] != ' ')
      {
        xbuf[i]=' ';
        i++;
      }
    }
    else if (*rdesc==')')
    {
      if (xbuf[i-1]==' ' && xbuf[i-2]==' ' && 
          (xbuf[i-3]=='.' || xbuf[i-3]=='?' || xbuf[i-3]=='!'))
      {
        xbuf[i-2]=*rdesc;
        xbuf[i-1]=' ';
        xbuf[i]=' ';
        i++;
      }
      else
      {
        xbuf[i]=*rdesc;
        i++;
      }
    }
    else if (*rdesc=='.' || *rdesc=='?' || *rdesc=='!') {
      if (xbuf[i-1]==' ' && xbuf[i-2]==' ' && 
          (xbuf[i-3]=='.' || xbuf[i-3]=='?' || xbuf[i-3]=='!')) {
        xbuf[i-2]=*rdesc;
        if (*(rdesc+1) != '\"')
        {
          xbuf[i-1]=' ';
          xbuf[i]=' ';
          i++;
        }
        else
        {
          xbuf[i-1]='\"';
          xbuf[i]=' ';
          xbuf[i+1]=' ';
          i+=2;
          rdesc++;
        }
      }
      else
      {
        xbuf[i]=*rdesc;
        if (*(rdesc+1) != '\"')
        {
          xbuf[i+1]=' ';
          xbuf[i+2]=' ';
          i += 3;
        }
        else
        {
          xbuf[i+1]='\"';
          xbuf[i+2]=' ';
          xbuf[i+3]=' ';
          i += 4;
          rdesc++;
        }
      }
      cap = TRUE;
    }
    else
    {
      xbuf[i]=*rdesc;
      if ( cap )
        {
          cap = FALSE;
          xbuf[i] = UPPER( xbuf[i] );
        }
      i++;
    }
  }
  xbuf[i]=0;
  strcpy(xbuf2,xbuf);
  
  rdesc=xbuf2;
  
  xbuf[0]=0;
  
  for ( ; ; )
  {
    for (i=0; i<77; i++)
    {
      if (!*(rdesc+i)) break;
    }
    if (i<77)
    {
      break;
    }
    for (i=(xbuf[0]?76:73) ; i ; i--)
    {
      if (*(rdesc+i)==' ') break;
    }
    if (i)
    {
      *(rdesc+i)=0;
      strcat(xbuf,rdesc);
      strcat(xbuf,"\n\r");
      rdesc += i+1;
      while (*rdesc == ' ') rdesc++;
    }
    else
    {
      bug ("No spaces", 0);
      *(rdesc+75)=0;
      strcat(xbuf,rdesc);
      strcat(xbuf,"-\n\r");
      rdesc += 76;
    }
  }
  while (*(rdesc+i) && (*(rdesc+i)==' '||
                        *(rdesc+i)=='\n'||
                        *(rdesc+i)=='\r'))
    i--;
  *(rdesc+i+1)=0;
  strcat(xbuf,rdesc);
  if (xbuf[strlen(xbuf)-2] != '\n')
    strcat(xbuf,"\n\r");

  free_string(oldstring);
  return(str_dup(xbuf));
}



/*
 * Used above in string_add.  Because this function does not
 * modify case if fCase is FALSE and because it understands
 * parenthesis, it would probably make a nice replacement
 * for one_argument.
 */
/*****************************************************************************
 Name:		first_arg
 Purpose:	Pick off one argument from a string and return the rest.
 		Understands quates, parenthesis (barring ) ('s) and
 		percentages.
 Called by:	string_add(string.c)
 ****************************************************************************/
char *first_arg( char *argument, char *arg_first, bool fCase )
{
    char cEnd;

    while ( *argument == ' ' )
	argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"'
      || *argument == '%'  || *argument == '(' )
    {
        if ( *argument == '(' )
        {
            cEnd = ')';
            argument++;
        }
        else cEnd = *argument++;
    }

    while ( *argument != '\0' )
    {
	if ( *argument == cEnd )
	{
	    argument++;
	    break;
	}
    if ( fCase ) *arg_first = LOWER(*argument);
            else *arg_first = *argument;
	arg_first++;
	argument++;
    }
    *arg_first = '\0';

    while ( *argument == ' ' )
	argument++;

    return argument;
}




/*
 * Used in olc_act.c for aedit_builders.
 */
char * string_unpad( char * argument )
{
    char buf[MAX_STRING_LENGTH];
    char *s;

    s = argument;

    while ( *s == ' ' )
        s++;

    strcpy( buf, s );
    s = buf;

    if ( *s != '\0' )
    {
        while ( *s != '\0' )
            s++;
        s--;

        while( *s == ' ' )
            s--;
        s++;
        *s = '\0';
    }

    free_string( argument );
    return str_dup( buf );
}



/*
 * Same as capitalize but changes the pointer's data.
 * Used in olc_act.c in aedit_builder.
 */
char * string_proper( char * argument )
{
    char *s;

    s = argument;

    while ( *s != '\0' )
    {
        if ( *s != ' ' )
        {
            *s = UPPER(*s);
            while ( *s != ' ' && *s != '\0' )
                s++;
        }
        else
        {
            s++;
        }
    }

    return argument;
}

char *string_delline( CHAR_DATA *ch, char *argument, char *old )
{
  char arg[MAX_INPUT_LENGTH];
  char new_str[MAX_STRING_LENGTH];
  char *ptr;
  int curr_line = 0;
  int line = 0;
  int pos = 0;
  argument = one_argument( argument, arg );
  if ( !is_number( arg ) )
    {
    send_to_char( "Delete which line?\n\r", ch );
    return old;
    }
  line = atoi( arg );
  for ( ptr = old; *ptr; ptr++, pos++ )
    {
    while ( curr_line == line )
      {
      if ( *ptr == '\n' || *ptr == '\0' )
	curr_line++;
      ptr++;
      if ( *ptr == '\r' )
	ptr++;
      }
    new_str[pos] = *ptr;
    if ( *ptr == '\r' )
      curr_line++;
    }
  new_str[pos] = '\0';
  if ( curr_line <= line )
    {
    send_to_char( "Line doesn't exist.\n\r", ch );
    return old;
    }
  send_to_char( "Line deleted.\n\r", ch );
  free_string( old );
  return str_dup( new_str );
}

void string_show( CHAR_DATA *ch, char *string )
{
  char	buf [ MAX_STRING_LENGTH ];
  char	temp[ MAX_STRING_LENGTH ];
  char	temp2[ MAX_STRING_LENGTH ];
  char *ptr;
  int	lig = 0;
  int	i;
  buf[0] = '\0';

  for( ptr = string, lig = 0; *ptr; ptr++, lig++ )
    {
    sprintf( temp, "%2d] ", lig );
    for( i = 4; *ptr != '\r'; ptr++, i++ )
    temp[i] = *ptr;
    temp[i] = '\0';
    sprintf( temp2, "%s\r",temp);
    strcat( buf, temp2);
    }
  send_to_char( buf, ch );
  send_to_char( "\n\r",ch );
  return;
}

char *string_insline( CHAR_DATA *ch, char *argument, char *old )
{
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  char *dline;
  int count = 0;
  int ln = 0;
  argument = one_argument( argument, arg );
  if ( !is_number( arg ) )
    {
    send_to_char( "Syntax: .i # <string>\n\r", ch );
    return old;
    }
  ln = atoi( arg );
  buf[0] = '\0';
  for ( dline = old; *dline != '\0'; )
    {
    char oneline[MAX_INPUT_LENGTH];
    int curr = 0;
    for ( ; *dline != '\0' && *dline != '\n'; dline++,  curr++ )
      {
      oneline[curr] = *dline;
      }
    if ( *dline == '\n' )
      {
      oneline[curr] = *dline;
      dline++, curr++;
      }
    if ( *dline == '\r' )
      {
      oneline[curr] = *dline;
      dline++, curr++;
      }
    oneline[curr] = '\0';
    curr++;
    if ( ln == count )
      {
      if ( argument[0] != '\0' )
        strcat( buf, argument);
      strcat( buf, "\n\r\0" );
      count++;
      }
    strcat(buf, oneline);
    count++;
  }
  if ( count <= ln )
    {
    send_to_char( "Line doesn't exist.\n\r", ch );
    return old;
    }
  send_to_char( "Line inserted.\n\r", ch );
  free_string( old );
  return str_dup( buf );
}
