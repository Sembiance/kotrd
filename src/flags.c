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
#include "tables.h"

int flag_lookup args( ( const char *name, const struct flag_type *flag_table) );

void do_flag(CHAR_DATA *ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH],arg2[MAX_INPUT_LENGTH],arg3[MAX_INPUT_LENGTH];
    char word[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    long *flag, old = 0, new = 0, marked = 0, pos;
    char type;
    const struct flag_type *flag_table;

    argument = one_argument(argument,arg1);
    argument = one_argument(argument,arg2);
    argument = one_argument(argument,arg3);

    type = argument[0];

    if (type == '=' || type == '-' || type == '+')
        argument = one_argument(argument,word);

    if (arg1[0] == '\0')
    {
	send_to_char("\n\r{GSyntax{w: {WFLAG CHAR {c<{WNAME{c> <{WFIELD{c> <{WFLAG{c>{x\n\r",ch);
	send_to_char("         {WFLAG MOB {c<{WNAME{c> <{WFIELD{c> <{WFLAG{c>{x\n\r\n\r",ch);
	send_to_char("{cCHAR FIELDS{w: {CPLR COMM AFF IMM RES VULN{x\n\r",ch);
	send_to_char(" {cMOB FIELDS{w: {CACT AFF OFF IMM RES VULN FORM PART{x\n\r",ch);
/*
	send_to_char("  +: add flag, -: remove flag, = set equal to\n\r",ch);
	send_to_char("  otherwise flag toggles the flags listed.\n\r",ch);
*/
	return;
    }

    if (arg2[0] == '\0')
    {
	send_to_char("\n\r{cWhat do you wish to set flags on?{x\n\r",ch);
	return;
    }

    if (arg3[0] == '\0')
    {
	send_to_char("\n\r{cYou need to specify a flag to set.{x\n\r",ch);
	return;
    }

    if (argument[0] == '\0')
    {
	send_to_char("\n\r{cWhich flags do you wish to change?{x\n\r",ch);
	return;
    }

    if (!str_prefix(arg1,"mob") || !str_prefix(arg1,"char"))
    {
	victim = get_char_world(ch,arg2);
	if (victim == NULL)
	{
	    send_to_char("\n\r{RYou can't find them.{x\n\r",ch);
	    return;
	}

        /* select a flag to set */
	if (!str_prefix(arg3,"act"))
	{
	    if (!IS_NPC(victim))
	    {
		send_to_char("\n\r{RUse {WPLR{R for PCs.{x\n\r",ch);
		return;
	    }

	    flag = &victim->act;
	    flag_table = act_flags;
	}

	else if (!str_prefix(arg3,"plr"))
	{
	    if (IS_NPC(victim))
	    {
		send_to_char("\n\r{RUse {WACT{R for NPCs.{x\n\r",ch);
		return;
	    }

	    flag = &victim->pact;
	    flag_table = plr_flags;
	}

 	else if (!str_prefix(arg3,"aff"))
	{
	    flag = &victim->affected_by;
	    flag_table = affect_flags;
	}

  	else if (!str_prefix(arg3,"immunity"))
	{
	    flag = &victim->imm_flags;
	    flag_table = imm_flags;
	}

	else if (!str_prefix(arg3,"resist"))
	{
	    flag = &victim->res_flags;
	    flag_table = res_flags;
	}

	else if (!str_prefix(arg3,"vuln"))
	{
	    flag = &victim->vuln_flags;
	    flag_table = vuln_flags;
	}

	else if (!str_prefix(arg3,"form"))
	{
	    if (!IS_NPC(victim))
	    {
	 	send_to_char("\n\r{WFORM {Rcan't be set on PCs.{x\n\r",ch);
		return;
	    }

	    flag = &victim->form;
	    flag_table = form_flags;
	}

	else if (!str_prefix(arg3,"parts"))
	{
	    if (!IS_NPC(victim))
	    {
		send_to_char("\n\r{WPARTS {Rcan't be set on PCs.{x\n\r",ch);
		return;
	    }

	    flag = &victim->parts;
	    flag_table = part_flags;
	}

	else if (!str_prefix(arg3,"comm"))
	{
	    if (IS_NPC(victim))
	    {
		send_to_char("\n\r{WCOMM {Rcan't be set on NPCs.{x\n\r",ch);
		return;
	    }

	    flag = &victim->comm;
	    flag_table = comm_flags;
	}

	else 
	{
	    send_to_char("\n\r{RThat's not an acceptable flag.{x\n\r",ch);
	    return;
	}

	old = *flag;
	victim->zone = NULL;

	if (type != '=')
	    new = old;

        /* mark the words */
        for (; ;)
        {
	    argument = one_argument(argument,word);

	    if (word[0] == '\0')
		break;

	    pos = flag_lookup(word,flag_table);
	    if (pos == 0)
	    {
		send_to_char("\n\r{RThat flag doesn't exist!{x\n\r",ch);
		return;
	    }
	    else
		SET_BIT(marked,pos);
	}

	for (pos = 0; flag_table[pos].name != NULL; pos++)
	{
	    if (!flag_table[pos].settable && IS_SET(old,flag_table[pos].bit))
	    {
		SET_BIT(new,flag_table[pos].bit);
		continue;
	    }

	    if (IS_SET(marked,flag_table[pos].bit))
	    {
		switch(type)
		{
		    case '=':
		    case '+':
			SET_BIT(new,flag_table[pos].bit);
			break;
		    case '-':
			REMOVE_BIT(new,flag_table[pos].bit);
			break;
		    default:
			if (IS_SET(new,flag_table[pos].bit))
			    REMOVE_BIT(new,flag_table[pos].bit);
			else
			    SET_BIT(new,flag_table[pos].bit);
		}
	    }
	}
	*flag = new;
	return;
    }
}
