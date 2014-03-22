#if defined(macintosh)
#include <types.h>
#include <time.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"
#include "interp.h"

#include "Utility.h"
#include "StringUtility.h"
#include "ArrayUtility.h"

bool granted(CHAR_DATA *ch, char * command)
{
    if(!ch || !command || command[0]=='\0' || IS_NPC(ch) || !ch->pcdata || !ch->pcdata->granted || array_len(ch->pcdata->granted)==0)
        return FALSE;
    
    if(array_find(ch->pcdata->granted, command)!=-1)
        return TRUE;
        
    return FALSE;
}

void show_granted_to_char(CHAR_DATA * victim, CHAR_DATA * ch)
{
    char **     ar=0;
    char        buf[MAX_STRING_LENGTH];
    int         i=0;
    
    if(!victim->pcdata->granted)
    {
        send_to_char("None\n\r", ch);
        return;
    }
    for(i=0,ar=victim->pcdata->granted;ar && *ar;i++,ar++)
    {
        sprintf(buf, "{B[{W%-10s{B]", *ar);
        send_to_char(buf, ch);
        
        if(i>0 && (i%5)==0)
            send_to_char("{x\n\r", ch);
    }
}

void do_grant(CHAR_DATA *ch, char *argument)
{
    int             cmd;
    CHAR_DATA *     victim;
    char            arg1[MAX_INPUT_LENGTH];
    char            arg2[MAX_INPUT_LENGTH];
    char            buf[MAX_STRING_LENGTH];
    
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    
    if(arg1[0]=='\0')
    {
        send_to_char("\n\r{WSyntax:{C grant {Y<player>{B <command>{x\n\r", ch);
        return;
    }
    
    if((victim=get_char_world(ch, arg1))==NULL)
    {
        send_to_char("\n\r{CThat Player is not online.{x\n\r", ch);
        return;
    }
    
    if(IS_NPC(victim))
    {
    	send_to_char("\n\r{RNot on NPC's.{x\n\r", ch );
    	return;
    }
    
    if(arg2[0]=='\0')
    {
        sprintf(buf, "\n\r{CPlayer {W%s{C has been granted the following commands:{x\n\r", victim->name);
        send_to_char(buf, ch);
        show_granted_to_char(victim, ch);
        return;
    }
    
    if(!strcmp(arg2, "builder"))
    {
        sprintf(buf, "%s alist", victim->name);     do_grant(ch, buf);
        sprintf(buf, "%s asave", victim->name);		do_grant(ch, buf);
        sprintf(buf, "%s force", victim->name);		do_grant(ch, buf);
        sprintf(buf, "%s fvlist", victim->name);	do_grant(ch, buf);
        sprintf(buf, "%s goto", victim->name);		do_grant(ch, buf);
        sprintf(buf, "%s grab", victim->name);		do_grant(ch, buf);
        sprintf(buf, "%s hedit", victim->name);		do_grant(ch, buf);
        sprintf(buf, "%s holylight", victim->name);	do_grant(ch, buf);
        sprintf(buf, "%s medit", victim->name);		do_grant(ch, buf);
        sprintf(buf, "%s mlevel", victim->name);	do_grant(ch, buf);
        sprintf(buf, "%s mload", victim->name);		do_grant(ch, buf);
        sprintf(buf, "%s mpedit", victim->name);	do_grant(ch, buf);
        sprintf(buf, "%s msearch", victim->name);	do_grant(ch, buf);
        sprintf(buf, "%s mstat", victim->name);		do_grant(ch, buf);
        sprintf(buf, "%s oedit", victim->name);		do_grant(ch, buf);
        sprintf(buf, "%s oload", victim->name);		do_grant(ch, buf);
        sprintf(buf, "%s olevel", victim->name);	do_grant(ch, buf);
        sprintf(buf, "%s osearch", victim->name);	do_grant(ch, buf);
        sprintf(buf, "%s ostat", victim->name);		do_grant(ch, buf);
        sprintf(buf, "%s otype", victim->name);		do_grant(ch, buf);
        sprintf(buf, "%s peace", victim->name);		do_grant(ch, buf);
        sprintf(buf, "%s purge", victim->name);		do_grant(ch, buf);
        sprintf(buf, "%s redit", victim->name);		do_grant(ch, buf);
        sprintf(buf, "%s resets", victim->name);	do_grant(ch, buf);
        sprintf(buf, "%s rstat", victim->name);		do_grant(ch, buf);
        sprintf(buf, "%s slay", victim->name);		do_grant(ch, buf);
        sprintf(buf, "%s stat", victim->name);		do_grant(ch, buf);
        sprintf(buf, "%s vlist", victim->name);		do_grant(ch, buf);		
        sprintf(buf, "%s vnumlist", victim->name);	do_grant(ch, buf);
    }
    
    for(cmd=0;cmd_table[cmd].name[0]!='\0';cmd++)
    {
	   if(!str_prefix(arg2, cmd_table[cmd].name))
           break;
	}
	
	if(cmd_table[cmd].name[0]=='\0')
	{
	   send_to_char("\n\r{RNo valid command found.{x\n\r", ch);
	   return;
	}
	
	if(strcmp(cmd_table[cmd].name, "wizhelp") && array_find(victim->pcdata->granted, "wizhelp")==-1)
	{
	    sprintf(buf, "%s wizhelp", victim->name);	do_grant(ch, buf);
    }
    	
    if(array_find(victim->pcdata->granted, cmd_table[cmd].name)!=-1)
    {
        sprintf(buf, "\n\r{W%s{R already has the {W%s{R command!{x\n\r", victim->name, cmd_table[cmd].name);
        send_to_char(buf, ch);
        return;
    }
    
    victim->pcdata->granted = array_append(victim->pcdata->granted, cmd_table[cmd].name);

    sprintf(buf, "\n\r{GCommand {W%s{G was granted to player {W%s{x\n\r", cmd_table[cmd].name, victim->name);
    send_to_char(buf, ch);
    
    sprintf(buf, "\n\r{GYou have been granted the command: {W%s{x\n\r", cmd_table[cmd].name);
    send_to_char(buf, victim);
    
    return;
}

void do_revoke(CHAR_DATA *ch, char *argument)
{
    int             cmd;
    CHAR_DATA *     victim;
    char            arg1[MAX_INPUT_LENGTH];
    char            arg2[MAX_INPUT_LENGTH];
    char **         newArray=0;
    char **         ar=0;
    char            buf[MAX_STRING_LENGTH];

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    
    if(arg1[0]=='\0' || arg2[0]=='\0')
    {
        send_to_char("\n\r{WSyntax:{C revoke {Y<player>{B <command>{x\n\r", ch);
        return;
    }
    
    if((victim=get_char_world(ch, arg1))==NULL)
    {
        send_to_char("\n\r{CThat Player is not online.{x\n\r", ch);
        return;
    }
    
    if(IS_NPC(victim))
    {
    	send_to_char("\n\r{RNot on NPC's.{x\n\r", ch );
    	return;
    }
    
    for(cmd=0;cmd_table[cmd].name[0]!='\0';cmd++)
    {
	   if(!str_prefix(arg2, cmd_table[cmd].name))
           break;
	}
	
	if(cmd_table[cmd].name[0]=='\0')
	{
	   send_to_char("\n\r{RNo valid command found.{x\n\r", ch);
	   return;
	}
	
    if(array_find(victim->pcdata->granted, cmd_table[cmd].name)==-1)
    {
        sprintf(buf, "\n\r{W%s{R has not yet been granted the {W%s{R command!{x\n\r", victim->name, cmd_table[cmd].name);
        send_to_char(buf, ch);
        return;
    }
    
    for(ar=victim->pcdata->granted;ar && *ar;ar++)
    {
        if(strcmp(*ar, cmd_table[cmd].name))
            newArray = array_append(newArray, *ar);
    }
    
    victim->pcdata->granted = array_free(victim->pcdata->granted);
    victim->pcdata->granted = newArray;
    
    sprintf(buf, "\n\r{GCommand {W%s{G was revoked from player {W%s{x\n\r", cmd_table[cmd].name, victim->name);
    send_to_char(buf, ch);
    
    sprintf(buf, "\n\r{GYou have been revoked from using the command: {W%s{x\n\r", cmd_table[cmd].name);
    send_to_char(buf, victim);

    if(array_len(victim->pcdata->granted)==1 && !strcmp(victim->pcdata->granted[0], "wizhelp"))
    {
	    sprintf(buf, "%s wizhelp", victim->name);	do_revoke(ch, buf);
    }    
    
    return;
}

