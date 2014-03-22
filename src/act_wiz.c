#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"
#include "interp.h"
#include "recycle.h"
#include "tables.h"
#include "lookup.h"
#include "olc.h"

#include "Utility.h"
#include "StringUtility.h"
#include "ArrayUtility.h"


/*
 * Local functions.
 */
DECLARE_DO_FUN ( do_asave );

ROOM_INDEX_DATA *	find_location	args( ( CHAR_DATA *ch, char *arg ) );
void raw_kill( CHAR_DATA *victim, CHAR_DATA *killer );
void show_granted_to_char(CHAR_DATA * victim, CHAR_DATA * ch);


void do_wiznet( CHAR_DATA *ch, char *argument )
{
    int flag;
    char buf[MAX_STRING_LENGTH];

    if ( argument[0] == '\0' )
    {
      	if (IS_SET(ch->wiznet,WIZ_ON))
      	{
            send_to_char("\n\r{cSigning {rOFF{c of Wiznet.{x\n\r",ch);
            REMOVE_BIT(ch->wiznet,WIZ_ON);
      	}
      	else
      	{
            send_to_char("\n\r{cSigning {WON{c to Wiznet.{x\n\r",ch);
            SET_BIT(ch->wiznet,WIZ_ON);
      	}
      	return;
    }

    if (!str_prefix(argument,"on"))
    {
            send_to_char("\n\r{cSigning {WON{c to Wiznet.{x\n\r",ch);
	SET_BIT(ch->wiznet,WIZ_ON);
	return;
    }

    if (!str_prefix(argument,"off"))
    {
            send_to_char("\n\r{cSigning {rOFF{c of Wiznet.{x\n\r",ch);
	REMOVE_BIT(ch->wiznet,WIZ_ON);
	return;
    }

    /* show wiznet status */
    if (!str_prefix(argument,"status")) 
    {
	buf[0] = '\0';

	if (!IS_SET(ch->wiznet,WIZ_ON))
	    strcat(buf,"off ");

	for (flag = 0; wiznet_table[flag].name != NULL; flag++)
	    if (IS_SET(ch->wiznet,wiznet_table[flag].flag))
	    {
		strcat(buf,wiznet_table[flag].name);
		strcat(buf," ");
	    }

	strcat(buf,"{x\n\r");

	send_to_char("\n\r{cWiznet status{w:{C\n\r",ch);
	send_to_char(buf,ch);
	return;
    }

    if (!str_prefix(argument,"show"))
    /* list of all wiznet options */
    {
	buf[0] = '\0';

	for (flag = 0; wiznet_table[flag].name != NULL; flag++)
	{
	    if (wiznet_table[flag].level <= ch->level)
	    {
	    	strcat(buf,wiznet_table[flag].name);
	    	strcat(buf," ");
	    }
	}

	strcat(buf,"{x\n\r");

	send_to_char("\n\r{cWiznet options available to you are{w:{C\n\r",ch);
	send_to_char(buf,ch);
	return;
    }
   
    flag = wiznet_lookup(argument);

    if (flag == -1 || ch->level < wiznet_table[flag].level)
    {
	send_to_char("\n\r{RNo such option.{x\n\r",ch);
	return;
    }
   
    if (IS_SET(ch->wiznet,wiznet_table[flag].flag))
    {
	sprintf(buf,"\n\r{gYou will no longer see {W%s {gon wiznet.{x\n\r",
	        wiznet_table[flag].name);
	send_to_char(buf,ch);
	REMOVE_BIT(ch->wiznet,wiznet_table[flag].flag);
    	return;
    }
    else
    {
    	sprintf(buf,"{GYou will now see {W%s {Gon wiznet.{x\n\r",
		wiznet_table[flag].name);
	send_to_char(buf,ch);
    	SET_BIT(ch->wiznet,wiznet_table[flag].flag);
	return;
    }

}

void do_auto_shutdown( void )
{
    FILE *fp;
    extern bool merc_down;
    DESCRIPTOR_DATA *d,*d_next;
    merc_down = TRUE;

    /* This is to write to the file. */
    fclose(fpReserve);
    if((fp = fopen(LAST_COMMAND,"a")) == NULL)
      bug("Error in do_auto_save opening last_command.txt",0);
   
      fprintf(fp,"Last Command: %s\n",
            last_command);

    fclose( fp );
    fpReserve = fopen( NULL_FILE, "r" );

    for ( d = descriptor_list; d != NULL; d = d_next)
      {
      if(d->character)
        do_save (d->character, "");
      d_next = d->next;
      close_socket(d);
      }
    return;
}


void wiznet(char *string, CHAR_DATA *ch, OBJ_DATA *obj,
	    long flag, long flag_skip, int min_level) 
{
  DESCRIPTOR_DATA *d;
  for ( d = descriptor_list; d != NULL; d = d->next )
    {
    if (d->connected == CON_PLAYING
    &&  IS_IMMORTAL(d->character) 
    &&  IS_SET(d->character->wiznet,WIZ_ON) 
    &&  (!flag || IS_SET(d->character->wiznet,flag))
    &&  (!flag_skip || !IS_SET(d->character->wiznet,flag_skip))
    &&  d->character->level >= min_level
    &&  d->character != ch)
      {
	    if (IS_SET(d->character->wiznet,WIZ_PREFIX))
  	  	send_to_char("{RWiznet> ",d->character);
      act_new(string,d->character,obj,ch,TO_CHAR,POS_DEAD);
      }
    }
 
    return;
}


/* RT nochannels command, for those spammers */
void do_nochan( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
 
    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "\n\r{RNochannel whom?\n\r{x", ch );
        return;
    }
 
    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "\n\r{RThey aren't here.{x\n\r", ch );
        return;
    }
 
    if (!IS_IMP(ch))
      {
       if ( victim->level >= ch->level )
         {
          send_to_char( "\n\r{RYou failed.{x\n\r", ch );
          return;
         }
       } 

    if ( IS_SET(victim->comm, COMM_NOCHANNELS) )
    {
        REMOVE_BIT(victim->comm, COMM_NOCHANNELS);
        send_to_char( "\n\r{CThe gods have restored your channel priviliges.{x\n\r", 
		      victim );
        send_to_char( "\n\r{WNOCHANNELS removed.{x\n\r", ch );
	sprintf(buf,"\n\r{W$N restores channels to %s.\n\r{x",victim->name);
	wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,ch->level);
    }
    else
    {
        SET_BIT(victim->comm, COMM_NOCHANNELS);
        send_to_char( "\n\r{RThe gods have revoked your channel priviliges.{x\n\r", 
		       victim );
        send_to_char( "\n\r{RNOCHANNELS set.{x\n\r", ch );
	sprintf(buf,"\n\r{R$N revokes %s's channels.{x\n\r",victim->name);
	wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,ch->level);
    }
 
    return;
}


void do_bamfin( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    if ( !IS_NPC(ch) )
    {
	smash_tilde( argument );

	if (argument[0] == '\0')
	{
	    sprintf(buf,"\n\r{CYour poofin is {x%s{x\n\r",ch->pcdata->bamfin);
	    send_to_char(buf,ch);
	    return;
	}
	     
	free_string( ch->pcdata->bamfin );
	ch->pcdata->bamfin = str_dup( argument );

        sprintf(buf,"\n\r{CYour poofin is now {x%s\n\r",ch->pcdata->bamfin);
        send_to_char(buf,ch);
    }
    return;
}

void do_bamfout( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
 
    if ( !IS_NPC(ch) )
    {
        smash_tilde( argument );
 
        if (argument[0] == '\0')
        {
            sprintf(buf,"\n\r{cYour poofout is {x%s\n\r",ch->pcdata->bamfout);
            send_to_char(buf,ch);
            return;
        }
 
        free_string( ch->pcdata->bamfout );
        ch->pcdata->bamfout = str_dup( argument );
 
        sprintf(buf,"\n\r{cYour poofout is now {x%s\n\r",ch->pcdata->bamfout);
        send_to_char(buf,ch);
    }
    return;
}


void do_deny( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "\n\r{cDeny whom?{x\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "\n\r{RThey aren't here.{x\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "\n\r{RNot on NPC's.{x\n\r", ch );
	return;
    }

    if (!IS_IMP(ch))
      {   
       if (victim->level >= ch->level )
         {
	  send_to_char( "\n\r{RYou failed.{x\n\r", ch );
	  return;
         }
      }

    SET_BIT(victim->pact, PLR_DENY);
    send_to_char( "\n\rYou are denied access!\n\r", victim );
    sprintf(buf,"\n\r{c$N denies access to %s{x\n\r",victim->name);
    wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,ch->level);
    send_to_char( "\n\r{COK{x\n\r", ch );
    save_char_obj(victim);
    stop_fighting(victim,TRUE);
    do_function(victim, &do_quit, "" );

    return;
}



void do_discon( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "\n\r{GSyntax{w: {WDISCON {c<{WCHAR NAME {cor {WSOCKET{c>{x\n\r",ch );
        send_to_char( "\n\r{BUsed to DISCONNECT PCs from the MUD by NAME or SOCKET.{x\n\r",ch);
        return;
    }

    if (is_number(arg))
    {
	int desc;

	desc = atoi(arg);
    	for ( d = descriptor_list; d != NULL; d = d->next )
    	{
            if ( d->descriptor == desc )
            {
            	close_socket( d );
            	send_to_char( "\n\r{COK{x\n\r", ch );
            	return;
            }
	}
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "\n\r{RThey aren't here.{x\n\r", ch );
	return;
    }

    if ( victim->desc == NULL )
    {
	act( "\n\r{R$N doesn't have a descriptor.{x\n\r", ch, NULL, victim, TO_CHAR);
	return;
    }

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	if ( d == victim->desc )
	{
	    close_socket( d );
	    send_to_char( "\n\r{COK{x\n\r", ch );
	    return;
	}
    }

    bug( "Do_discon: desc not found.", 0 );
    send_to_char( "\n\r{RDescriptor not found!{x\n\r", ch );
    return;
}


void do_gecho( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;

    if ( argument[0] == '\0' )
    {
	send_to_char( "\n\r{GSyntax{w: {WGECHO {c<{WTEXT TO SEND{c>{x\n\r",ch );
	return;
    }
    
    for ( d = descriptor_list; d; d = d->next )
    {
	if ( d->connected == CON_PLAYING )
	{
	    send_to_char( argument, d->character );
	    send_to_char( "\n\r",   d->character );
	}
    }

    return;
}



void do_recho( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;    

    if ( argument[0] == '\0' )
    {
        send_to_char( "\n\r{GSyntax{w: {WRECHO {c<{WTEXT TO SEND{c>{x\n\r",ch ); 
        return;
    }

    for ( d = descriptor_list; d; d = d->next )
    {
	if ( d->connected == CON_PLAYING
	&&   d->character->in_room == ch->in_room )
	{
            if (d->character->level >= ch->level)
                send_to_char( "\n\r{CLOCAL{w> {x",d->character);
	    send_to_char( argument, d->character );
	    send_to_char( "\n\r",   d->character );
	}
    }

    return;
}

void do_pecho( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    argument = one_argument(argument, arg);
 
    if ( argument[0] == '\0'|| arg[0] == '\0' )
    {
        send_to_char( "\n\r{GSyntax{w: {WPECHO {c<{WTARGET{c> <{WTEXT TO SEND{c>{x\n\r",ch ); 
        return;
    }   
    if  ( (victim = get_char_world(ch, arg) ) == NULL )
    {
	send_to_char("\n\r{RTarget not found.{x\n\r",ch);
	return;
    }

    if (victim->level >= ch->level && ch->level != MAX_LEVEL)
        send_to_char( "\n\r{rPERSONAL{w> {x",victim);

    send_to_char(argument,victim);
    send_to_char("\n\r",victim);
    send_to_char( "\n\r{rPERSONAL{w> {x",ch);
    send_to_char(argument,ch);
    send_to_char("\n\r",ch);
}


ROOM_INDEX_DATA *find_location( CHAR_DATA *ch, char *arg )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;

    if ( is_number(arg) )
	return get_room_index( atoi( arg ) );

    if ( ( victim = get_char_world( ch, arg ) ) != NULL )
	return victim->in_room;

    if ( ( obj = get_obj_world( ch, arg ) ) != NULL )
	return obj->in_room;

    return NULL;
}



void do_transfer( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
	send_to_char( "\n\r{cTransfer whom (and where)?{x\n\r", ch );
	return;
    }

    if ( !str_cmp( arg1, "all" ) )
    {
	for ( d = descriptor_list; d != NULL; d = d->next )
	{
	    if ( d->connected == CON_PLAYING
	    &&   d->character != ch
	    &&   d->character->in_room != NULL
	    &&   can_see( ch, d->character ) )
	    {
		char buf[MAX_STRING_LENGTH];
		sprintf( buf, "%s %s", d->character->name, arg2 );
		do_function(ch, &do_transfer, buf );
	    }
	}
	return;
    }

    /*
     * Thanks to Grodyn for the optional location parameter.
     */
    if ( arg2[0] == '\0' )
    {
	location = ch->in_room;
    }
    else
    {
	if ( ( location = find_location( ch, arg2 ) ) == NULL )
	{
	    send_to_char( "No such location.\n\r", ch );
	    return;
	}

	if ( !is_room_owner(ch,location) && room_is_private( location ) 
	&&  (!IS_IMP(ch)))
	{
	    send_to_char( "That room is private right now.\n\r", ch );
	    return;
	}
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim->in_room == NULL )
    {
	send_to_char( "They are in limbo.\n\r", ch );
	return;
    }

    if (!IS_NPC( victim ) && IS_SET (victim->pact, PLR_JAIL))
      {
        send_to_char( "They are in jail.\n\r", ch );
        return;
      }  

    if (victim == ch)
      {
       send_to_char("\n\r{rIMMs can use {RGOTO{r so there is no reason to use {RTRANSFER{r on yourself.{x\n\r",victim);
       return;
      }

    if (IS_IMP(victim)
    && (!IS_IMP(ch)))
    {
	send_to_char( "\n\r{RYou Failed{x\n\r", ch );
	return;
    }
    
    if ( victim->fighting != NULL )
	stop_fighting( victim, TRUE );
    act( "$n disappears in a mushroom cloud.", victim, NULL, NULL, TO_ROOM );
    char_from_room( victim );
    char_to_room( victim, location );
    act( "$n arrives from a puff of smoke.", victim, NULL, NULL, TO_ROOM );
    if ( ch != victim )
	act( "$n has transferred you.", ch, NULL, victim, TO_VICT );
    do_function(victim, &do_look, "auto" );
    send_to_char( "\n\r{COK{x\n\r", ch );
}



void do_at( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    ROOM_INDEX_DATA *original;
    OBJ_DATA *on;
    CHAR_DATA *wch;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
        send_to_char( "\n\r {GSyntax{w: {WAT {c<{WTARGET{c> <{WCOMMAND{c>{x\n\r",ch );
        send_to_char( "\n\r{cTargets{w: {CROOM VNUM{c, {CMOB NAME{c, {CCHAR NAME{x\n\r",ch);
        send_to_char( 
"\n\r\n\r{BAllows COMMANDs, POWERs, SPELLs, etc. to be done {WAT {Brange{b.{x\n\r",ch);
        return;
    }

    if ( IS_SET( ch->pact, PLR_JAIL ))
        {
        send_to_char( "\n\r{RThere's {rNO {Rescaping {WJAIL{r!{x\n\r", ch );
        return;
        }
    else
        REMOVE_BIT( ch->pact, PLR_JAIL );


    if ( ( location = find_location( ch, arg ) ) == NULL )
    {
	send_to_char( "\n\r{CNo such location.{x\n\r", ch );
	return;
    }

    if (!is_room_owner(ch,location) && room_is_private( location ) 
    && (!IS_IMP(ch)))
    {
	send_to_char( "\n\r{cThat room is private right now.{x\n\r", ch );
	return;
    }

    original = ch->in_room;
    on = ch->on;
    char_from_room( ch );
    char_to_room( ch, location );
    interpret( ch, argument );

    /*
     * See if 'ch' still exists before continuing!
     * Handles 'at XXXX quit' case.
     */
    for ( wch = char_list; wch != NULL; wch = wch->next )
    {
	if ( wch == ch )
	{
	    char_from_room( ch );
	    char_to_room( ch, original );
	    ch->on = on;
	    break;
	}
    }

    return;
}

void do_goto( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *rch;
    int count = 0;
    ROOM_INDEX_DATA *location;

    if ( ( location = find_location( ch, argument ) ) == NULL )
    {
	send_to_char( "\n\r{RNo such location!{x\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' )
    {
	send_to_char( "\n\r{cGOTO where?{x\n\r", ch );
	return;
    }

  if(!IS_IMP(ch))
    {
     if(IS_SET(location->area->area_flags, AREA_IMP))
       {
        send_to_char(
        "\n\r{RYou {WCAN NOT{R use {WGOTO{R to reach this room1!{x\n\r",ch);
        return;
       }

     if(ch->level < ASSTIMP && IS_SET(location->area->area_flags, AREA_NOIMM))
       {
              send_to_char(
              "\n\r{RYou {WCAN NOT{R use {WGOTO{R to reach this room2!{x\n\r",ch);
              return;
      }
   }

if ( IS_SET( ch->pact, PLR_JAIL ))
        {
        send_to_char( "\n\r{RThere's {rNO {Rescaping {rJAIL{R!{x\n\r", ch );
        return;
        }
    else
        REMOVE_BIT( ch->pact, PLR_JAIL );

    count = 0;
    for ( rch = location->people; rch != NULL; rch = rch->next_in_room )
        count++;


    if (!is_room_owner(ch,location) && room_is_private(location) 
    &&  (count > 1 
    || (!IS_IMP(ch)))  )
    {
	send_to_char( "\n\r{RThat room is private right now!{x\n\r", ch );
	return;
    }


    if ( ch->fighting != NULL )
	stop_fighting( ch, TRUE );

    for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
    {
	if (rch->level >= ch->invis_level)
	{
	    if (ch->pcdata != NULL && ch->pcdata->bamfout[0] != '\0')
		act("$t",ch,ch->pcdata->bamfout,rch,TO_VICT);
	    else
		act("\n\r{C$n {cleaves in a swirling mist.{x\n\r",ch,NULL,rch,TO_VICT);
	}
    }

    char_from_room( ch );
    char_to_room( ch, location );

    for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
    {
        if (rch->level >= ch->invis_level)
        {
            if (ch->pcdata != NULL && ch->pcdata->bamfin[0] != '\0')
                act("$t",ch,ch->pcdata->bamfin,rch,TO_VICT);
            else
                act("\n\r{cThe God {C$n {cappears in a swirling mist.{x\n\r",ch,NULL,rch,TO_VICT);
        }
    }

    do_function(ch, &do_look, "auto" );

    return;
}

void do_violate( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *location;
    CHAR_DATA *rch;
 
    if ( argument[0] == '\0' )
    {
        send_to_char( "Goto where?\n\r", ch );
        return;
    }
 
    if ( ( location = find_location( ch, argument ) ) == NULL )
    {
        send_to_char( "No such location.\n\r", ch );
        return;
    }

    if (!room_is_private( location ))
    {
        send_to_char( "That room isn't private, use goto.\n\r", ch );
        return;
    }
 
    if ( ch->fighting != NULL )
        stop_fighting( ch, TRUE );

    for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
    {
        if (rch->level >= ch->invis_level)
        {
            if (ch->pcdata != NULL && ch->pcdata->bamfout[0] != '\0')
                act("$t",ch,ch->pcdata->bamfout,rch,TO_VICT);
            else
                act("$n leaves in a swirling mist.",ch,NULL,rch,TO_VICT);
        }
    }
 
    char_from_room( ch );
    char_to_room( ch, location );
 
 
    for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
    {
        if (rch->level >= ch->invis_level)
        {
            if (ch->pcdata != NULL && ch->pcdata->bamfin[0] != '\0')
                act("$t",ch,ch->pcdata->bamfin,rch,TO_VICT);
            else
                act("$n appears in a swirling mist.",ch,NULL,rch,TO_VICT);
        }
    }
 
    do_look( ch, "auto" );
    return;
} 


void do_rstat( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    OBJ_DATA *obj;
    CHAR_DATA *rch;
    int door;

    one_argument( argument, arg );

    location = ( arg[0] == '\0' ) ? ch->in_room : find_location( ch, arg );
    if ( location == NULL )
    {
	send_to_char( "No such location.\n\r", ch );
	return;
    }

    if (!is_room_owner(ch,location) && ch->in_room != location 
    &&  room_is_private( location ) && (!IS_IMP(ch)))
    {
	send_to_char( "That room is private right now.\n\r", ch );
	return;
    }

    sprintf( buf, "Name: '%s'\n\rArea: '%s'\n\r",
	location->name,
	location->area->name );
    send_to_char( buf, ch );

    sprintf( buf,
	"Vnum: %d  Sector: %d  Light: %d  Healing: %d  Mana: %d\n\r",
	location->vnum,
	location->sector_type,
	location->light,
	location->heal_rate,
	location->mana_rate );
    send_to_char( buf, ch );

    sprintf( buf,
	"Room flags: %d.\n\rDescription:\n\r%s",
	location->room_flags,
	location->description );
    send_to_char( buf, ch );

    if ( location->extra_descr != NULL )
    {
	EXTRA_DESCR_DATA *ed;

	send_to_char( "Extra description keywords: '", ch );
	for ( ed = location->extra_descr; ed; ed = ed->next )
	{
	    send_to_char( ed->keyword, ch );
	    if ( ed->next != NULL )
		send_to_char( " ", ch );
	}
	send_to_char( "'.\n\r", ch );
    }

    send_to_char( "Characters:", ch );
    for ( rch = location->people; rch; rch = rch->next_in_room )
    {
	if (can_see(ch,rch))
        {
	    send_to_char( " ", ch );
	    one_argument( rch->name, buf );
	    send_to_char( buf, ch );
	}
    }

    send_to_char( ".\n\rObjects:   ", ch );
    for ( obj = location->contents; obj; obj = obj->next_content )
    {
	send_to_char( " ", ch );
	one_argument( obj->name, buf );
	send_to_char( buf, ch );
    }
    send_to_char( ".\n\r", ch );

    for ( door = 0; door <= 9; door++ )
    {
	EXIT_DATA *pexit;

	if ( ( pexit = location->exit[door] ) != NULL )
	{
	    sprintf( buf,
		"Door: %d.  To: %d.  Key: %d.  Exit flags: %d.\n\rKeyword: '%s'.  Description: %s",

		door,
		(pexit->u1.to_room == NULL ? -1 : pexit->u1.to_room->vnum),
	    	pexit->key,
	    	pexit->exit_info,
	    	pexit->keyword,
	    	pexit->description[0] != '\0'
		    ? pexit->description : "(none).\n\r" );
	    send_to_char( buf, ch );
	}
    }

    return;
}


void do_reboo( CHAR_DATA *ch, char *argument )
{
    send_to_char( "\n\r{rTo do a {WREBOOT{r you {RMUST{r spell it out.{x\n\r", ch );
    return;
}

void do_reboot( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    extern bool merc_down;
    DESCRIPTOR_DATA *d,*d_next;
    CHAR_DATA *vch;

    	sprintf( buf, "{CI Am ReBooting The MUD and Saving Your Area Files.{x");
    	do_function(ch, &do_builder, buf );

    	sprintf( buf, "{rAn {RIMPLEMENTOR {rhas {WREBOOTED{r the MUD.{x\n\r"
                      "{GYou will be able to log back on in a couple of minutes... {C;){x\n\r\n\r");
    	do_function(ch, &do_gecho, buf );

        do_asave(ch,"world");
        save_clans();

    for ( d = descriptor_list; d != NULL; d = d_next )
    {
	d_next = d->next;
	vch = d->original ? d->original : d->character;

	if (vch != NULL)
            do_save (vch, "");
	    save_char_obj(vch);
    	close_socket(d);
    }

    merc_down = TRUE;
    return;
}


void do_shutdow( CHAR_DATA *ch, char *argument )
{
    send_to_char( "If you want to SHUTDOWN, spell it out.\n\r", ch );
    return;
}

void do_shutdown( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    extern bool merc_down;
    DESCRIPTOR_DATA *d,*d_next;
    CHAR_DATA *vch;

    if (ch->invis_level < LEVEL_HERO)
    sprintf( buf, "Shutdown by %s.", ch->name );
    append_file( ch, SHUTDOWN_FILE, buf );
    strcat( buf, "\n\r" );
    if (ch->invis_level < LEVEL_HERO)
    {
    	do_function(ch, &do_gecho, buf );
    }
    save_clans();
    merc_down = TRUE;
    for ( d = descriptor_list; d != NULL; d = d_next)
    {
	d_next = d->next;
	vch = d->original ? d->original : d->character;
	if (vch != NULL)
	    save_char_obj(vch);
	close_socket(d);
    }
    return;
}

void do_snoop( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Snoop whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim->desc == NULL )
    {
	send_to_char( "\n\r{RNo descriptor to snoop.{x\n\r", ch );
	return;
    }

    if(!str_cmp(victim->name, "Venus") && str_cmp(ch->name, "Venus"))
	{
		send_to_char("\n\r{RSnoop Doggy Dogg?{x\n\r", ch);
		return;
	}

    if ((IS_IMP(victim))
    && (!IS_IMP(ch)))
    { 
        send_to_char( "\n\r{RNo descriptor to snoop.{x\n\r", ch );
        return;
    }


    if ( victim == ch )
    {
	send_to_char( "Cancelling all snoops.\n\r", ch );
	for ( d = descriptor_list; d != NULL; d = d->next )
	{
	    if ( d->snoop_by == ch->desc )
		d->snoop_by = NULL;
	}
	return;
    }

    if ( victim->desc->snoop_by != NULL )
    {
	send_to_char( "\n\r{BBusy already.{x\n\r", ch );
	return;
    }



    if (!is_room_owner(ch,victim->in_room) && ch->in_room != victim->in_room 
    &&  room_is_private(victim->in_room) && (!IS_IMP(ch)))
    {
        send_to_char("That character is in a private room.\n\r",ch);
        return;
    }

       if ( ( victim->level >= ch->level ) 
       ||   IS_SET(victim->comm,COMM_SNOOP_PROOF))
         {
	  send_to_char( "\n\r{RYou failed!{x\n\r", ch );
	  return;
         }
    

    if ( ch->desc != NULL )
    {
	for ( d = ch->desc->snoop_by; d != NULL; d = d->snoop_by )
	{
	    if ( d->character == victim || d->original == victim )
	    {
		send_to_char( "No snoop loops.\n\r", ch );
		return;
	    }
	}
    }

    victim->desc->snoop_by = ch->desc;
    return;
}


void do_switch( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );
    
    if ( arg[0] == '\0' )
    {
	send_to_char( "Switch into whom?\n\r", ch );
	return;
    }

    if ( ch->desc == NULL )
	return;
    
    if ( ch->desc->original != NULL )
    {
	send_to_char( "You are already switched.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "\n\r{COk{X\n\r", ch );
	return;
    }

    if (!IS_NPC(victim))
    {
	send_to_char("You can only switch into mobiles.\n\r",ch);
	return;
    }

    if (!is_room_owner(ch,victim->in_room) && ch->in_room != victim->in_room 
    &&  room_is_private(victim->in_room) && (!IS_IMP(ch)))
    {
	send_to_char("That character is in a private room.\n\r",ch);
	return;
    }

    if ( victim->desc != NULL )
    {
	send_to_char( "Character in use.\n\r", ch );
	return;
    }

    ch->desc->character = victim;
    ch->desc->original  = ch;
    victim->desc        = ch->desc;
    ch->desc            = NULL;
    victim->oldlvl = ch->oldlvl;
    /* change communications to match */
    if (ch->prompt != NULL)
        victim->prompt = str_dup(ch->prompt);
    victim->comm = ch->comm;
    victim->lines = ch->lines;
    send_to_char( "\n\r{COK{x\n\r", victim );
    return;
}



void do_return( CHAR_DATA *ch, char *argument )
{
    if ( ch->desc == NULL )
	return;

    if ( ch->desc->original == NULL )
    {
	send_to_char( "You aren't switched.\n\r", ch );
	return;
    }

    send_to_char( 
"You return to your original body. Type replay to see any missed tells.\n\r", 
	ch );
    if (ch->prompt != NULL)
    {
	free_string(ch->prompt);
	ch->prompt = NULL;
    }

    ch->desc->character       = ch->desc->original;
    ch->desc->original        = NULL;
    ch->desc->character->desc = ch->desc; 
    ch->desc                  = NULL;
    return;
}


bool obj_check (CHAR_DATA *ch, OBJ_DATA *obj)
{
/*
    if (( ch->level >= GGOD 
    && ch->level <= (MAX_LEVEL - 370) )
    || ((ch->level >= IMMORTAL) 
    && obj->level <= 20 
    && obj->cost <= 1000))

*/

 //   if ( ch->level >= AI )
	return TRUE;
   // else
//	return FALSE;
}

/* for clone, to insure that cloning goes many levels deep */
void recursive_clone(CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *clone)
{
    OBJ_DATA *c_obj, *t_obj;


    for (c_obj = obj->contains; c_obj != NULL; c_obj = c_obj->next_content)
    {
	if (obj_check(ch,c_obj))
	{
	    t_obj = create_object(c_obj->pIndexData,0);
	    clone_object(c_obj,t_obj);
	    obj_to_obj(t_obj,clone);
	    recursive_clone(ch,c_obj,t_obj);
	}
    }
}

/* command that is similar to load */
void do_clone(CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char *rest;
    CHAR_DATA *mob;
    OBJ_DATA  *obj;

    rest = one_argument(argument,arg);

    if (arg[0] == '\0')
    {
	send_to_char("Clone what?\n\r",ch);
	return;
    }

    if (!str_prefix(arg,"object"))
    {
	mob = NULL;
	obj = get_obj_here(ch,rest);
	if (obj == NULL)
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}
    }
    else if (!str_prefix(arg,"mobile") || !str_prefix(arg,"character"))
    {
	obj = NULL;
	mob = get_char_room(ch,rest);
	if (mob == NULL)
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}
    }
    else /* find both */
    {
	mob = get_char_room(ch,argument);
	obj = get_obj_here(ch,argument);
	if (mob == NULL && obj == NULL)
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}
    }

    /* clone an object */
    if (obj != NULL)
    {
	OBJ_DATA *clone;

	if (!obj_check(ch,obj))
	{
	    send_to_char(
		"Your powers are not great enough for such a task.\n\r",ch);
	    return;
	}

	if(same_obj(obj, ch->carrying)>=30)
	{
	send_to_char("You already have enough.\n\r", ch);
	return;
	}

	clone = create_object(obj->pIndexData,0); 
	clone_object(obj,clone);
	if (obj->carried_by != NULL)
	    obj_to_char(clone,ch);
	else
	    obj_to_room(clone,ch->in_room);
 	recursive_clone(ch,obj,clone);

	act("$n has created $p.",ch,clone,NULL,TO_ROOM);
	act("You clone $p.",ch,clone,NULL,TO_CHAR);
	return;
    }
    else if (mob != NULL)
    {
	CHAR_DATA *clone;
	OBJ_DATA *new_obj;

	if (!IS_NPC(mob))
	{
	    send_to_char("You can only clone mobiles.\n\r",ch);
	    return;
	}

	if ( (mob->level > 20 && ch->level >= GGOD && ch->level <= (MAX_LEVEL - 370))
	||  (mob->level > 10 && ch->level >= LGOD && ch->level < GGOD))
	{
	    send_to_char(
		"Your powers are not great enough for such a task.\n\r",ch);
	    return;
	}

	clone = create_mobile(mob->pIndexData);
	clone_mobile(mob,clone); 
	
	for (obj = mob->carrying; obj != NULL; obj = obj->next_content)
	{
	    if (obj_check(ch,obj))
	    {
		new_obj = create_object(obj->pIndexData,0);
		clone_object(obj,new_obj);
		recursive_clone(ch,obj,new_obj);
		obj_to_char(new_obj,clone);
		new_obj->wear_loc = obj->wear_loc;
	    }
	}
	char_to_room(clone,ch->in_room);
        act("$n has created $N.",ch,NULL,clone,TO_ROOM);
        act("You clone $N.",ch,NULL,clone,TO_CHAR);
        return;
    }
}

void do_mload( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    MOB_INDEX_DATA *pMobIndex;
    CHAR_DATA *rch;
    CHAR_DATA *victim;
    
    one_argument( argument, arg );

    if ( arg[0] == '\0' || !is_number(arg) )
    {
	send_to_char( "\n\r{GSyntax{w: {WMLOAD {c<{WMOB VNUM{c>{x\n\r",ch );
	send_to_char( "{G     or {WLOAD MOB {c<{WMOB VNUM{c>{x\n\r",ch );
	return;
    }

    if ( ( pMobIndex = get_mob_index( atoi( arg ) ) ) == NULL )
    {
	send_to_char( "\n\r{RNo mob has that vnum.{x\n\r", ch );
	return;
    }

    rch = GET_CHAR( ch );
    if ( IS_NPC( rch ) )
      return;



  if(!IS_IMP(rch))
    {
     if(IS_SET(pMobIndex->area->area_flags, AREA_IMP))
       {
        send_to_char(
        "\n\r{RYou {WCAN NOT{R use {WGOTO{R to reach this room!{x\n\r",ch);
        return;
       }
      
     if(rch->level < ASSTIMP)
       {
        if(IS_SET(pMobIndex->area->area_flags, AREA_NOIMM))
          {
           if(!IS_SET(rch->comm, COMM_HELPER))
             {
              send_to_char(
              "\n\r{RYou {WCAN NOT{R use {WGOTO{R to reach this room!{x\n\r",ch);
              return;
             }
          }
       }
    }

    victim = create_mobile( pMobIndex );
    char_to_room( victim, ch->in_room );
    act( "\n\r{W$n has created $N!{x\n\r", ch, NULL, victim, TO_ROOM );
    send_to_char( "\n\r{COK{x\n\r", ch );
    return;
}



void do_oload( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH] ,arg2[MAX_INPUT_LENGTH];
    OBJ_INDEX_DATA *pObjIndex;
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *rch;
    OBJ_DATA *obj;
    int number;
    
    argument = one_argument( argument, arg1 );
    one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || !is_number(arg1))
    {
	send_to_char( "\n\r{GSyntax{w: {WOLOAD {c<{WOBJ VNUM{c>{x\n\r",ch );
	send_to_char( "{G     or {WLOAD OBJ {c<{WOBJ VNUM{c>{x\n\r",ch );
	return;
    }
    
	number=0;
  
    if ( arg2[0] != '\0')  /* load with a level */
    {
	if (!is_number(arg2))
        {
	send_to_char( "\n\r{GSyntax{w: {WOLOAD {c<{WOBJ VNUM{c>{x\n\r",ch );
	send_to_char( "{G     or {WLOAD OBJ {c<{WOBJ VNUM{c>{x\n\r",ch );
	  return;
	}
        number = atoi(arg2);
	if(number<1 || number>20)
{
	send_to_char("\n\r{RInvalid number.{x\n\r", ch);
	return;
}
    }

    if ( ( pObjIndex = get_obj_index( atoi( arg1 ) ) ) == NULL )
    {
	send_to_char( "\n\r{RNo object has that vnum.{x\n\r", ch );
	return;
    }

    rch = GET_CHAR( ch );
    if ( IS_NPC( rch ) )
      return;

  if(!IS_IMP(rch))
    {   
     if(IS_SET(pObjIndex->area->area_flags, AREA_IMP))
       {
        send_to_char(
        "\n\r{RYou {WCAN NOT{R use {WGOTO{R to reach this room!{x\n\r",ch); 
        return;
       }
        
     if(rch->level < ASSTIMP)
       {
        if(IS_SET(pObjIndex->area->area_flags, AREA_NOIMM))
          {
           if(!IS_SET(rch->comm, COMM_HELPER))
             {
              send_to_char(
              "\n\r{RYou {WCAN NOT{R use {WGOTO{R to reach this room!{x\n\r",ch);
              return;
             }
          }
    
       }
    }

    obj = create_object( pObjIndex, pObjIndex->level );
        if(same_obj(obj, ch->carrying) >=30)
        {
            send_to_char("\n\r{RYou already have enough.{x\n\r", ch);
	    obj_to_room( obj, ch->in_room );
    act( "\n\r{W$n has created $p!\n\r{x", ch, obj, NULL, TO_ROOM );
            return;
        }

    if(!IS_IMP(ch))
      {
       sprintf(buf, "%s %s", obj->name, ch->name);
       obj->name = str_dup( buf );
      }

    if ( CAN_WEAR(obj, ITEM_TAKE) )
	obj_to_char( obj, ch );
    else
	obj_to_room( obj, ch->in_room );
    act( "\n\r{W$n has created $p!{x\n\r", ch, obj, NULL, TO_ROOM );
    send_to_char( "\n\r{COK{x\n\r", ch );
    return;
}



void do_purge( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[100];
    OBJ_DATA *obj;
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	/* 'purge' */
	CHAR_DATA *vnext;
	OBJ_DATA  *obj_next;

	for ( victim = ch->in_room->people; victim != NULL; victim = vnext )
	{
	    vnext = victim->next_in_room;

          if(!IS_IMP(ch))
            {
	    if ( IS_NPC(victim) 
            && !IS_SET(victim->act,ACT_NOPURGE) 
	    && victim != ch
            && (IS_BUILDER(ch,victim->in_room->area)) 
            && (!IS_SET(victim->in_room->area->area_flags, AREA_IMP))
            && (!IS_SET(victim->in_room->area->area_flags, AREA_NOIMM))
            && (IS_SET( ch->comm, COMM_HELPER)))
		extract_char( victim, TRUE );

	    }
          else
            {
	    if ( IS_NPC(victim) 
	    &&  victim != ch)
		extract_char( victim, TRUE );
	    }

      }

	for ( obj = ch->in_room->contents; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;

          if(!IS_IMP(ch))
            {
	     if (!IS_OBJ_STAT(obj,ITEM_NOPURGE)
             && (IS_BUILDER(ch,obj->in_room->area)) 
             && (!IS_SET(obj->in_room->area->area_flags, AREA_IMP))
             && (!IS_SET(obj->in_room->area->area_flags, AREA_NOIMM))
             && (IS_SET( ch->comm, COMM_HELPER)))
               extract_obj( obj );
           }
          else
            {
             extract_obj( obj );
            }
	}

	act( "\n\r{R$n {WPURGEs {Rthe room!{x\n\r", ch, NULL, NULL, TO_ROOM);
	send_to_char( "\n\r{COK{x\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "\n\r{RThey aren't here!{x\n\r", ch );
	return;
    }

    if ( !IS_NPC(victim) )
    {

	if (ch == victim)
	{
	  send_to_char("\n\r{GYou are either tryin' to be funny or tryin' to {WCRASH{G the MUD!{x\n\r",ch);
	  send_to_char("{GIt had better be tryin' to be {WFUNNY{G for your Sake!!{x\n\r",ch);
	  return;
	}

	if (ch->level <= victim->level)
	{
	  send_to_char("\n\r{RMaybe that wasn't a good idea...{x\n\r",ch);
	  sprintf(buf,"\n\r{W%s {Rtried to purge you!{x\n\r",ch->name);
	  send_to_char(buf,victim);
	  return;
	}

	act("\n\r{W$n {cdisintegrates {C$N{c.{x\n\r",ch,0,victim,TO_NOTVICT);

    	if (victim->level > 1)
	    save_char_obj( victim );
    	d = victim->desc;
    	extract_char( victim, TRUE );
    	if ( d != NULL )
          close_socket( d );

	return;
    }
    act( "\n\r{W$n {gpurges {G$N{g.{x\n\r", ch, NULL, victim, TO_NOTVICT );
    extract_char( victim, TRUE );

 return;
}

void do_dbag( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA       *c_obj, *obj;
    CHAR_DATA      *victim;
    OBJ_INDEX_DATA *pObjIndex;
    char            arg [ MAX_INPUT_LENGTH ];
    char            buf [ MAX_STRING_LENGTH ];
    int		    iCount;
    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Syntax: dbag <player>\n\r", ch );
        return;
    }

    if ( !( victim = get_char_world( ch, arg ) ) )
    {
        send_to_char( "That player is not here.\n\r", ch);
        return;
    }
    
    /* Create the container , ie bag */
    if ( !( pObjIndex = get_obj_index( 3729 ) ) )
    {
        send_to_char( "There has been a slight problem creating the bag.\n\r", ch );
        return;
    }
    c_obj = create_object( pObjIndex, 1 );
    free_string( c_obj->name );
    c_obj->name = str_dup("dragon bag");

    free_string( c_obj->short_descr );
    c_obj->short_descr = str_dup("a {rDragon{x bag");

    free_string( c_obj->description );
    c_obj->description = str_dup("a {rDragon{x bag is on the floor");

    /*------ water skin -----------*/
    if ( !( pObjIndex = get_obj_index( 3728 ) ) )
    {
        send_to_char( "There has been a slight problem creating water skin.\n\r",ch); 
        return;
    }
    obj = create_object( pObjIndex, 1 );
    obj_to_obj( obj, c_obj );

    /*---------- cloak -----------*/
    if ( !( pObjIndex = get_obj_index( 3725 ) ) )
    {
        send_to_char( "There has been a slight problem creating the necklace.\n\r",ch);
        return;
    }
        obj = create_object( pObjIndex, 1 );
	obj_to_obj( obj, c_obj );
 
    /*------ linen robe ----------*/
    if ( !( pObjIndex = get_obj_index( 3726 ) ) )
    {
        send_to_char( "There has been a slight problem creating a linen robe.\n\r",ch);
        return;
    }
    obj = create_object( pObjIndex, 1 );
    obj_to_obj( obj, c_obj );

    /*--------- cookies ----------*/
    if ( !( pObjIndex = get_obj_index( 3009 ) ) )
    {
        send_to_char( "There has been a slight problem creating the cookies.\n\r",ch);
        return;
    }
    for ( iCount = 0; iCount < 3; iCount++ )
	{
        obj = create_object( pObjIndex, 1 );
        free_string( obj->name );
        obj->name = str_dup("lybre cookies");
        free_string( obj->short_descr );
        obj->short_descr = str_dup("a batch of Lybre's delicious cookies");
        free_string( obj->description );
        obj->description = str_dup("a batch of Lybre's delicious cookies");
	obj_to_obj( obj, c_obj );
	}
    obj = create_object( pObjIndex, 1 );
    free_string( obj->name );
    obj->name = str_dup("argon taco salad");
    free_string( obj->short_descr );
    obj->short_descr = str_dup("Argon's taco salad");
    free_string( obj->description );
    obj->description = str_dup("Argon's taco salad");
    obj_to_obj( obj, c_obj );
   
    obj = create_object( pObjIndex, 1 );
    free_string( obj->name );
    obj->name = str_dup("druh box peanut");
    free_string( obj->short_descr );
    obj->short_descr = str_dup("Druh's large box of peanuts");
    free_string( obj->description );
    obj->description = str_dup("Druh's large box of peanuts");
    obj_to_obj( obj, c_obj );

    obj = create_object( pObjIndex, 1 );
    free_string( obj->name );
    obj->name = str_dup("amberdrake cupcake");
    free_string( obj->short_descr );
    obj->short_descr = str_dup("Amberdrake's cupcake");
    free_string( obj->description );
    obj->description = str_dup("Amberdrake's cupcake");
    obj_to_obj( obj, c_obj );

    /*------ midgaard maps ----------*/
    if ( !( pObjIndex = get_obj_index( 3162 ) ) )
    {
        send_to_char( "There has been a problem creating tha map.\n\r",ch);
        return;
    }
    obj = create_object( pObjIndex, 1 );
    obj_to_obj( obj, c_obj );

    if ( !( pObjIndex = get_obj_index( 3164 ) ) )
    {
        send_to_char( "There has been a problem creating the map.\n\r",ch);
        return;
    }
    obj = create_object( pObjIndex, 1 );
    obj_to_obj( obj, c_obj );


    {
    static char *dragon = "{xA {rRed Dragon{x";
    sprintf( buf, "%s swoops down from the skies and lands in front of you.", 
	     dragon );
    act_new( buf, victim, NULL, NULL, TO_CHAR, POS_DEAD );
    sprintf( buf, "%s swoops down from the skies and lands in front of $n.",
	     dragon );
    act_new( buf, victim, NULL, NULL, TO_ROOM, POS_DEAD );
    sprintf( buf, "%s {Ysays to you, 'Here you go me child.'{x", dragon );
    act_new( buf, victim, NULL, NULL, TO_CHAR, POS_DEAD );
    sprintf( buf, "%s {Ysays to $n, 'Here you go me child.'{x", dragon );
    act_new( buf, victim, NULL, NULL, TO_ROOM, POS_DEAD );
    sprintf( buf, "%s hands you $p and flies away!{x", dragon );
    act_new( buf, victim, c_obj, NULL, TO_CHAR, POS_DEAD );
    sprintf( buf, "%s hands $n $p and flies away!{x", dragon );
    act_new( buf, victim, c_obj, NULL, TO_NOTVICT, POS_DEAD );
    }
    /* Now let the newbie has it */
    obj_to_char( c_obj, victim );

    return;
}

void do_pnote( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "\n\r{GGive note privilage to whom?{x\n\r", ch );
        return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "\n\r{RThey aren't here.{x\n\r", ch );
        return;
    }

    if ( IS_NPC(victim) )
    {
        send_to_char( "\n\r{GNot on NPC's.{x\n\r", ch );
        return;
    }

    if ( victim->level > ch->level )
    {
        send_to_char( "\n\r{RYou failed.{x\n\r", ch );
        return;
    }
 
    if ( victim->level < 5 && !IS_IMP(ch))
    {
        send_to_char( "\n\r{rThe player {RMUST{r be atleast {W5th {rlevel to get {WNOTE PRIVILEGEs{r..{x\n\r", ch );
        return;
    }


    if ( IS_SET(victim->pact, PLR_PNOTE) )
    {
        sprintf(buf, "\n\r{W%s {galready has {WNOTE PRIVILEGEs{g.{x\n\r",victim->name);
        send_to_char(buf, ch);
    }
  else
    {
     if(!IS_SET(victim->comm, COMM_NOPNOTE))
       {
        SET_BIT(victim->pact, PLR_PNOTE);
        send_to_char( "\n\r{gYou have been {GGRANTED {WNOTE PRIVILEGEs{g.{x\n\r",victim);
        sprintf(buf, "\n\r{cYou have {CGRANTED {W%s {WNOTE PRIVILEGEs{c.{x\n\r",victim->name);
        send_to_char(buf, ch);
       }
     else
       {
        sprintf(buf, "\n\r{W%s {ris set as {WNOPNOTE{r, so double check before granting it to them!{x\n\r",victim->name);
        send_to_char(buf, ch);
       }
    }

    save_char_obj( victim );

    return;
}

void do_advance( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int level;
    int iLevel;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );


    if (!is_owner(ch))
    {
	send_to_char( "\n\r\n\r{RIf you are seeing this you do not have permission to use advance..!{x\n\r", ch );
	return;
    }

    if ( arg1[0] == '\0' || arg2[0] == '\0' || !is_number( arg2 ) )
    {
	send_to_char( "\n\r{GSyntax{x: {WADVANCE {c<{WCHAR NAME{c> <{WLEVEL{c>{x\n\r", ch );
	send_to_char( "\n\r{BUsed to RAISE and LOWER a PCs level.{x\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "\n\r{RThat player is NOT here!{x\n\r", ch);
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "\n\r{RNOT on NPC's!{x\n\r", ch );
	return;
    }

    if ( ( level = atoi( arg2 ) ) < 1 || level > MAX_LEVEL )
    {
	sprintf(buf,"\n\r{RLevel {WMUST {Rbe {W1 {Rto {W500{R!{x\n\r");
	send_to_char(buf, ch);
	return;
    }

    if ( level > ch->oldlvl )
    {
	send_to_char( "\n\r{RLimited to YOUR level!{x\n\r", ch );
	return;
    }

    /*
     * Lower level:
     *   Reset to level 1.
     *   Then raise again.
     *   Currently, an imp can lower another imp.
     *   -- Swiftest
     */

    if ( level <= victim->level )
      {
       sprintf(buf, "\n\r{CLOWERING {W%s{c's LEVEL to {C%d{c!{x\n\r", victim->name, level);
       send_to_char(buf,ch);
       sprintf(buf, "\n\r\n\r{g**** {GYour {WLEVEL {Ghas been {WLOWERED {Gby an {cIMPLEMENTOR {Gto {W%d {g****{x\n\r\n\r",level);
       send_to_char(buf,victim);
       victim->level    = 1;
       victim->oldlvl   = level;
       victim->practice = 0;
       victim->train	= 0;	
       victim->exp      = exp_per_level(victim,victim->pcdata->points);
       victim->max_hit  = 10;
       victim->max_mana = 100;
       victim->max_move = 100;
       victim->practice = 0;
       victim->hit      = victim->max_hit;
       victim->mana     = victim->max_mana;
       victim->move     = victim->max_move;
       advance_level( victim, TRUE, FALSE );
      }
    else
      {
       sprintf(buf, "\n\r{CRAISING {W%s{c's LEVEL to {C%d{c!{x\n\r",victim->name,level );
       send_to_char(buf,ch);
       sprintf(buf, "\n\r{g**** {GYour {WLEVEL {Ghas been {WRAISED {Gby an {cIMPLEMENTOR {Gto {W%d {g****{x\n\r\n\r", level);
       send_to_char(buf,victim);
      }

    for ( iLevel = victim->level ; iLevel < level; iLevel++ )
    {
	victim->level  += 1;
	advance_level( victim,TRUE, FALSE);
    }

    victim->exp   = exp_per_level(victim,victim->pcdata->points) 
		  * UMAX( 1, victim->level );

    save_char_obj(victim);
    return;
}


void do_restore( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *vch;
    CHAR_DATA *victim;
    DESCRIPTOR_DATA *d;
    AFFECT_DATA af;

    one_argument( argument, arg );

    if (arg[0] == '\0' || !str_cmp(arg,"room"))
    {
        for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room) 
        {
   
          if(!IS_SET(vch->in_room->room_flags, ROOM_DRAGONPIT))
           { 

    if (IS_SET(vch->pact, PLR_NORESTORE))
      {
      return;
      }

    while ( vch->affected )
        affect_remove( vch, vch->affected );
    vch->affected_by = race_table[vch->race].aff;
    vch->affected2_by = race_table[vch->race].aff2;


            vch->hit 	= vch->max_hit;
            vch->mana	= vch->max_mana;
            vch->move	= vch->max_move;
            update_pos( vch);
            act(
"\n\r{w$n {chas {CRESTORED {cyou completely and {CREMOVED {call ill-effects!{x",ch,NULL,vch,TO_VICT);
           }
       }
        send_to_char("\n\r{CROOM RESTORED{x\n\r",ch);
        return;
    }

if (( ch->level >= ASSTIMP) 
    && (!str_cmp(arg,"all")
    ||  !str_cmp(arg,"all+"))) 
    {
        bool fPlus = ( arg[3] == '+' );
        for (d = descriptor_list; d != NULL; d = d->next)
        {
	    victim = d->character;

	    if (victim == NULL 
            || IS_NPC(victim)
            || (victim->desc->connected != CON_PLAYING)
            || (IS_SET(victim->in_room->room_flags, ROOM_DRAGONPIT) ))
	     continue;
 
    if (IS_SET(victim->pact, PLR_NORESTORE))
      {
      return;
      }
               
    while ( victim->affected )
        affect_remove( victim, victim->affected );
    victim->affected_by = race_table[victim->race].aff;

    while ( victim->affected )
        affect_remove( victim, victim->affected );
    victim->affected2_by = race_table[victim->race].aff2;
            
            victim->hit 	= victim->max_hit;
            victim->mana	= victim->max_mana;
            victim->move	= victim->max_move;

	    if ( fPlus )
	      {
               if(IS_IMP(ch))
                 {

                if ( !IS_AFFECTED( victim, AFF_HASTE ) )
		          {       
              	        affect_strip(victim,skill_lookup("haste"));
              	        af.where	= TO_AFFECTS;
            		af.type		= skill_lookup( "haste" );
            		af.level	= 500;
            		af.duration 	= 15;
            		af.location	= APPLY_DEX;
            		af.modifier	= 5;
            		af.bitvector	= AFF_HASTE;
            		affect_to_char( victim, &af );
                            
                            if (victim != ch)
             		  send_to_char( "\n\r{cYou have been {CHASTED {cby an {CIMMORTAL{c!{x\n\r", victim );
            		}
 

                if ( !IS_AFFECTED( victim, AFF_SANCTUARY ) )
        		{       
                      affect_strip(victim,skill_lookup("sanctuary"));
        	      af.where	= TO_AFFECTS;
        	      af.type	= skill_lookup( "sanctuary" );
        	      af.level	= 500;
        	      af.duration  = 15;
        /*	      af.location  = APPLY_NONE; */
        	      af.modifier  = 0;
        	      af.bitvector = AFF_SANCTUARY;
                      affect_to_char( victim, &af );
                  
                      if(victim != ch)
            	        send_to_char( "\n\r{WYou have been granted {cSANCTUARY {Wby an {cIMMORTAL{W!{x\n\r",victim);
                 }

	           }
         }
            update_pos( victim);
	    if (victim->in_room != NULL)
            act(
"\n\r{w$n {chas {CRESTORED {cyou completely and {CREMOVED {call ill-effects!{x",ch,NULL,victim,TO_VICT);
          }
	send_to_char("\n\r{cAll active players restored.{x\n\r",ch);
	return;
      }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "\n\r{RThey aren't here.{x\n\r", ch );
	return;
    }


    if(IS_SET(victim->in_room->room_flags, ROOM_DRAGONPIT))
      {
       send_to_char( "\n\r{GThat Player is engaged in a DragonPIT.{x\n\r", ch );
       return;
      }

    victim->hit  = victim->max_hit;
    victim->mana = victim->max_mana;
    victim->move = victim->max_move;
    update_pos( victim );

    return;
}


 	
void do_freeze( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Freeze whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }


       if ( victim->level >= ch->level )
         {
	  send_to_char( "You failed.\n\r", ch );
	  return;
         }

    if ( IS_SET(victim->pact, PLR_FREEZE) )
    {
	REMOVE_BIT(victim->pact, PLR_FREEZE);
	send_to_char( "You can play again.\n\r", victim );
	send_to_char( "FREEZE removed.\n\r", ch );
	sprintf(buf,"$N unFREEZES %s.",victim->name);
        wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,ch->level);
    }
    else
    {
	SET_BIT(victim->pact, PLR_FREEZE);
	send_to_char( "You can't do ANYthing!\n\r", victim );
	send_to_char( "FREEZE set.\n\r", ch );
	sprintf(buf,"$N FREEZEs %s.",victim->name);
        wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,ch->level);
    }

    save_char_obj( victim );

    return;
}



void do_log( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "\n\r{RLog whom?{x\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
	if ( fLogAll )
	{
	    fLogAll = FALSE;
	    send_to_char( "\n\r{CLog ALL off.{x\n\r", ch );
	}
	else
	{
	    fLogAll = TRUE;
	    send_to_char( "\n\r{GLog ALL on.{x\n\r", ch );
	}
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "\n\r{RThey aren't here!{x\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "\n\r{RNot on NPC's!{x\n\r", ch );
	return;
    }

    /*
     * No level check, gods can log anyone.
     */
    if ( IS_SET(victim->pact, PLR_LOG) )
    {
	REMOVE_BIT(victim->pact, PLR_LOG);
	send_to_char( "\n\r{GLOG removed.{x\n\r", ch );
    }
    else
    {
	SET_BIT(victim->pact, PLR_LOG);
	send_to_char( "\n\r{CLOG set.{x\n\r", ch );
    }

    return;
}


void do_peace( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *rch;

    for ( rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room )
     {
      if (ch->level >= rch->level) 
        {
         if ( rch->fighting != NULL )
	   stop_fighting( rch, TRUE );
	 if (IS_NPC(rch) && IS_SET(rch->act,ACT_AGGRESSIVE))
	   REMOVE_BIT(rch->act,ACT_AGGRESSIVE);
         }
     }

    send_to_char( "\n\r{COK{x\n\r", ch );
    return;
}

void do_wizlock( CHAR_DATA *ch, char *argument )
{
    extern bool wizlock;
    wizlock = !wizlock;

    if ( wizlock )
    {
	send_to_char( "\n\r{CGame WIZLOCKed!{x\n\r", ch );
    }
    else
    {
	send_to_char( "\n\r{GGame un-WIZLOCKed.{x\n\r", ch );
    }
    return;
}

/* RT anti-newbie code */

void do_newlock( CHAR_DATA *ch, char *argument )
{
    extern bool newlock;
    newlock = !newlock;
 
    if ( newlock )
    {
	wiznet("$N locks out new characters.",ch,NULL,0,0,0);
        send_to_char( "New characters have been locked out.\n\r", ch );
    }
    else
    {
	wiznet("$N allows new characters back in.",ch,NULL,0,0,0);
        send_to_char( "Newlock removed.\n\r", ch );
    }
 
    return;
}


void do_string( CHAR_DATA *ch, char *argument )
{
    char type [MAX_INPUT_LENGTH];
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    char buf[MAX_STRING_LENGTH];

    smash_tilde( argument );
    argument = one_argument( argument, type );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    strcpy( arg3, argument );

    if ( type[0] == '\0' || arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
	send_to_char("\n\r{GSyntax{w: {WSTRING CHAR {c<{WNAME{c> <{WFIELD{c> <{WSTRING{c>\n\r",ch);
	send_to_char("         {WSTRING MOB {c<{WNAME{c> <{WFIELD{c> <{WSTRING{c>\n\r",ch);
	send_to_char("         {WSTRING OBJ {c<{WNAME{c> <{WFIELD{c> <{WSTRING{c>\n\r",ch);
	send_to_char("\n\r{cMOB & OBJ FIELDS{w: {CNAME SHORT LONG\n\r",ch);
	send_to_char("     {cCHAR FIELDS{w: {CTITLE\n\r",ch);
	return;
    }
    


    if (!str_prefix(type,"character") || !str_prefix(type,"mobile"))
    {

    	if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
    	{
	    send_to_char( "\n\r{RThey are {WNOT{R in this room.{x\n\r", ch );
	    return;
    	}

	/* clear zone for mobs */
	victim->zone = NULL;

	/* string something */


     	if ( !str_prefix( arg2, "name" ) )
    	{
	    if ( !IS_NPC(victim) )
	    {
	    	send_to_char( "\n\r{RUse RENAME to do this!{x\n\r", ch );
	    	return;
	    }
     
	    free_string( victim->name );
            sprintf(buf, "%s", arg3);
            victim->name = str_dup(buf);
	    return;
    	}

    	if ( !str_prefix( arg2, "short" ) )
    	{
	    if ( !IS_NPC(victim) )
	    {
	    	send_to_char( "\n\r{RNOT on Player Characters!{x\n\r", ch );
	    	return;
	    }

	    free_string( victim->short_descr );
            sprintf(buf, "%s", victim->name);
            if(!strstr(victim->name, ch->name))
            {
            free_string(victim->name);
            victim->name = str_dup(buf);
            }
	    victim->short_descr = str_dup( arg3 );
	    return;
           
    	}

    	if ( !str_prefix( arg2, "long" ) )
    	{
	    if ( !IS_NPC(victim) )
	    {
	    	send_to_char( "\n\r{RNOT on Player Characters!{x\n\r", ch );
	    	return;
	    }

	    free_string( victim->long_descr );
	    strcat(arg3,"\n\r");
	    victim->long_descr = str_dup( arg3 );
            sprintf(buf, "%s", victim->name);
            if(!strstr(victim->name, ch->name))
            {
            free_string(victim->name);
            victim->name = str_dup(buf);
	       }
	      return;

    	}

    	if ( !str_prefix( arg2, "title" ) )
    	{
	    if ( IS_NPC(victim) )
	    {
	    	send_to_char( "\n\r{RNOT on Non-Player Characters!{x\n\r", ch );
	    	return;
	    }

	    set_title( victim, arg3 );
	    return;
    	}
    }
    
    if (!str_prefix(type,"object"))
      {

    	if ( ( obj = get_obj_here( ch, arg1 ) ) == NULL )
    	{
	    send_to_char( "\n\r{RThat is {WNOT{R in this room.{x\n\r", ch );
	    return;
    	}

       if (( obj = get_obj_world( ch, arg1 )) == NULL )
    	 {
	  send_to_char( "\n\r{CNothing like that in heaven or earth.{x\n\r", ch );
	  return;
    	 }
    	
       if ( !str_prefix( arg2, "name" ) )
    	 {

	  free_string( obj->name );
	  sprintf(buf, "%s", arg3);	
	  obj->name = str_dup( buf );

	  return;


    	 }

       if ( !str_prefix( arg2, "short" ) )
    	 {
	  free_string( obj->short_descr );
          sprintf(buf, "%s", obj->name);

  	  if(!strstr(obj->name, ch->name))
	    {
	     free_string(obj->name);
	     obj->name = str_dup(buf);
	    }
        
	  obj->short_descr = str_dup( arg3);
	  return;
    	 }

    	if ( !str_prefix( arg2, "long" ) )
    	  {

	   free_string( obj->description );
        
           if(!strstr(obj->name, ch->name))
             {
              sprintf(buf, "%s", obj->name);
              free_string(obj->name);
              obj->name = str_dup(buf);
	     }

	   obj->description = str_dup( arg3);
        
	   return;
    	  }

   }
    do_function(ch, &do_string, "");
}

void do_force( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH]; 
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
      send_to_char( "\n\r{GSyntax{w: {c'{WFORCE {c<{WCHARACTER{c> <{WCOMMAND{c>{c'\n\r", ch );

     if(ch->level >= ASSTIMP)
       {
      send_to_char( "        {c'{WFORCE {c<{WTYPE{c> <{WCHARACTER{c> <{WCOMMAND{c>{c'{x\n\r",ch);
       }

     if(IS_IMP(ch))
       {
      send_to_char( "\n\r{RIMP ONLY {cTypes{w: {CALL IMMS PCS{x\n\r", ch );
       }

     if(ch->level == ASSTIMP)
       {
      send_to_char( "\n\r{RASST IMP ONLY {cTypes{w: {CPCS{x\n\r", ch );
       }
	return;
    }

    one_argument(argument,arg2);
  
    if (!str_cmp(arg2,"delete") || !str_prefix(arg2,"mob"))
    {
	send_to_char("\n\r{RThat will {WNOT {Rbe done{r!{x\n\r",ch);
	return;
    }

/*    sprintf( buf, "\n\r{C$n {cforces you to {D'{W%s{D'{c.{x\n\r", argument ); */

    if ( !str_cmp( arg, "all" ) )
    {
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;

	if (ch->level < MAX_LEVEL )
	{
	    send_to_char("\n\r{WNOT {Rat your level!\n\r{x",ch);
	    return;
	}

	for ( vch = char_list; vch != NULL; vch = vch_next )
	{
	    vch_next = vch->next;

	    if ( !IS_NPC(vch) && vch->level < ch->level )
	    {
/*		act( buf, ch, NULL, vch, TO_VICT ); */
		interpret( vch, argument );
	    }
	}
    }
    else if (!str_cmp(arg,"pcs"))
    {
        CHAR_DATA *vch;
        CHAR_DATA *vch_next;
 
        if (ch->level < ASSTIMP)
        {
	    send_to_char("\n\r{WNOT {Rat your level!\n\r{x",ch);
            return;
        }
 
        for ( vch = char_list; vch != NULL; vch = vch_next )
        {
            vch_next = vch->next;
 
            if ( (!IS_NPC(vch) && vch->level < ch->level)
	    &&	 vch->level < LEVEL_HERO )
            {
/*                act( buf, ch, NULL, vch, TO_VICT ); */
                interpret( vch, argument );
            }
        }
    }
    else if (!str_cmp(arg,"imms"))
    {
        CHAR_DATA *vch;
        CHAR_DATA *vch_next;
 
        if (ch->level < MAX_LEVEL )
        {
            send_to_char("\n\r{WNOT {Rat your level!\n\r{x",ch);
            return;
        }
 
        for ( vch = char_list; vch != NULL; vch = vch_next )
        {
            vch_next = vch->next;
 
            if ( !IS_NPC(vch) 
            && ( vch->level < ch->level )
            &&   vch->level > LEVEL_HERO)
            {
                act( buf, ch, NULL, vch, TO_VICT );
                interpret( vch, argument );
            }
        }
    }
    else
    {
	CHAR_DATA *victim;

	if ( ( victim = get_char_world( ch, arg ) ) == NULL )
	{
	    send_to_char( "\n\r{RThey are {WNOT {Rhere{r!{x\n\r", ch );
	    return;
	}

	if ( victim == ch )
	{
	    send_to_char( "\n\r{CAye aye{c, {Cright away{c!{x\n\r", ch );
	    return;
	}
	if(!str_cmp(victim->name, "Venus"))
	{
		send_to_char("\n\r{MAwww... couldn't you just ask her nicely?{x\n\r", ch);
		return;
	}

    	if (!is_room_owner(ch,victim->in_room) 
	&&  ch->in_room != victim->in_room 
        &&  room_is_private(victim->in_room) 
        &&  (!IS_IMP(ch)))
    	{
            send_to_char("\n\r{RThat character is in a {WPRIVATE{R room{r!{x\n\r",ch);
            return;
        }

	if ( victim->level > ch->level )
	{
	    send_to_char( "\n\r{RThat character is beyond your power level{r!{x\n\r", ch );
	    return;
	}

/*	act( buf, ch, NULL, victim, TO_VICT ); */
	interpret( victim, argument );
    }

    send_to_char( "\n\r{COK{x\n\r", ch );
    return;
}



/*
 * New routines by Dionysos.
 */
void do_invis( CHAR_DATA *ch, char *argument )
{
    int level;
    char arg[MAX_STRING_LENGTH];

    /* RT code for taking a level argument */
    one_argument( argument, arg );

       if ( arg[0] == '\0' ) 
         {
          if ( ch->invis_level)
            {
	     ch->invis_level = 0; 
 	     act( "\n\r{C$n slowly fades into existence.\n\r{x", ch, NULL, NULL,TO_ROOM );
 	     send_to_char( "\n\r{CYou slowly fade back into existence.{x\n\r", ch );
            }
           else
            {
	     ch->invis_level = ch->level;
	     act( "\n\r{C$n slowly fades into thin air.\n\r{x", ch, NULL, NULL,TO_ROOM );
	     send_to_char( "\n\r{CYou slowly vanish into thin air.{x\n\r", ch );
            }
         }
    else
      {
       level = atoi(arg);
       
       if (!is_owner(ch))
         {
          if (level < 2 || level > ch->level)
            {
	     send_to_char("\n\r{RInvis level must be between 2 and your level.{x\n\r",ch);
             return;
            }
          else
            {
             ch->reply = NULL;
             ch->invis_level = level;
             act( "\n\r{C$n slowly fades into thin air.{x\n\r", ch, NULL,NULL,TO_ROOM);
             send_to_char( "\n\r{CYou slowly vanish into thin air{x.\n\r", ch );
            }
         }
       else
         {
          if (level < 2 || level > 500)
            {
	     send_to_char("\n\r{RInvis level must be between 2 and 500.{x\n\r",ch);
             return;
            }
          else
            {
	     ch->reply = NULL;
             ch->invis_level = level;
             act( "\n\r{C$n slowly fades into thin air.{x\n\r", ch, NULL,NULL,TO_ROOM );
             send_to_char( "\n\r{CYou slowly vanish into thin air{x.\n\r", ch );
            }
         }
      }

    return;
}


void do_incog( CHAR_DATA *ch, char *argument )
{
    int level;
    char arg[MAX_STRING_LENGTH];
 
    /* RT code for taking a level argument */
    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    /* take the default path */
 
      if ( ch->incog_level)
      {
          ch->incog_level = 0;
          act( "$n is no longer cloaked.", ch, NULL, NULL, TO_ROOM );
          send_to_char( "You are no longer cloaked.\n\r", ch );
      }
      else
      {
          ch->incog_level = ch->level;
          act( "$n cloaks $s presence.", ch, NULL, NULL, TO_ROOM );
          send_to_char( "You cloak your presence.\n\r", ch );
      }
    else
    /* do the level thing */
    {
      level = atoi(arg);
      if (level < 2 || level > ch->level)
      {
        send_to_char("Incog level must be between 2 and your level.\n\r",ch);
        return;
      }
      else
      {
          ch->reply = NULL;
          ch->incog_level = level;
          act( "$n cloaks $s presence.", ch, NULL, NULL, TO_ROOM );
          send_to_char( "You cloak your presence.\n\r", ch );
      }
    }
 
    return;
}



void do_holylight( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC(ch) )
	return;

    if ( IS_SET(ch->pact, PLR_HOLYLIGHT) )
    {
	REMOVE_BIT(ch->pact, PLR_HOLYLIGHT);
	send_to_char( "Holy light mode off.\n\r", ch );
    }
    else
    {
	SET_BIT(ch->pact, PLR_HOLYLIGHT);
	send_to_char( "Holy light mode on.\n\r", ch );
    }

    return;
}

/* prefix command: it will put the string typed on each line typed */

void do_prefi (CHAR_DATA *ch, char *argument)
{
    send_to_char("You cannot abbreviate the prefix command.\n\r",ch);
    return;
}

void do_prefix (CHAR_DATA *ch, char *argument)
{
    char buf[MAX_INPUT_LENGTH];

    if (argument[0] == '\0')
    {
	if (ch->prefix[0] == '\0')
	{
	    send_to_char("\n\r{RYou have no prefix to clear.{x\n\r",ch);
	    return;
	}

	send_to_char("\n\r{CPrefix removed.{x\n\r",ch);
	free_string(ch->prefix);
	ch->prefix = str_dup("");
	return;
    }

    if (ch->prefix[0] != '\0')
    {
	sprintf(buf,"\n\r{CPrefix changed to {W%s{c.{x\n\r",argument);
	free_string(ch->prefix);
    }
    else
    {
	sprintf(buf,"\n\r{cPrefix set to {W%s{c.{x\n\r",argument);
    }

    ch->prefix = str_dup(argument);
}

void do_bonus( CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;
    char       buf  [ MAX_STRING_LENGTH ];
    char       arg1 [ MAX_INPUT_LENGTH ];
    char       arg2 [ MAX_INPUT_LENGTH ];
     char       arg3 [ MAX_INPUT_LENGTH ];
   char		verb[MAX_STRING_LENGTH];
    int      value;
   DESCRIPTOR_DATA *d;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );


    if ( arg1[0] == '\0' || arg2[0] == '\0' || !is_number( arg2 ) )
    {
	send_to_char( "\n\r{GSyntax{w: {WBONUS {c<{WCHAR NAME{c> <{WNUMBER of POINTS{c> {c[{Gquest or qp{c]{x\n\r", ch );
	send_to_char( "\n\r{BADDs {WX{B amount of EXP Points to TARGET CHAR.{x\n\r", ch );
	send_to_char( "\n\r{BADDs {WX{B amount of Quest Points to TARGET CHAR if you specify {Gquest{B or{G qp{B at the end.{x\n\r", ch );
	 return;
    }

    if ( !str_cmp( arg1, "all" ) )
    {
        for ( d = descriptor_list; d != NULL; d = d->next )
        {
            if ( d->connected == CON_PLAYING
            &&   d->character != ch
            &&   d->character->in_room != NULL
            &&   can_see( ch, d->character ) )
            {
                char buf[MAX_STRING_LENGTH];
		if(arg3[0]!='\0')
	                sprintf( buf, "%s %s %s", d->character->name, arg2, arg3 );
		else
                        sprintf( buf, "%s %s", d->character->name, arg2);

                do_function(ch, &do_bonus, buf );
            }
        }
        return;
    }
    
    if (( victim = get_char_world ( ch, arg1 ) ) == NULL )
    {
      send_to_char("\n\r{CThat Player is not here.{x\n\r", ch);
      return;
    }
       
    if ( IS_NPC( victim ) )
    {
	send_to_char( "\n\r{RNot on NPC's.{x\n\r", ch );
	return;
    }

    if ( ch == victim )
    {
	send_to_char( "\n\r{RYou may not bonus yourself.{x\n\r", ch );
	return;
    }

   /* if (IS_IMMORTAL(victim) || victim->level >= LEVEL_IMMORTAL)
    {
    send_to_char("\n\r{RYou can't bonus immortals silly!{x\n\r", ch);
    return;
    }       */

    value = atoi( arg2 );

    if ( value == 0 )
    {
	send_to_char( "\n\r{RThe value must not be equal to {W0{R.{x\n\r", ch );
	return;
    }

	if(!strcmp(arg3, "quest") || !strcmp(arg3, "qp"))
	{
		if ( value < -200 || value > 200 )
		{
			send_to_char( "\n\r{cValue range is {C-200 {cto {C200{c questpoints.{x\n\r", ch );
			return;
		}		
		sprintf(verb, "quest");
		victim->questpoints+=value;
	}
	else
	{
		if ( value < -500 || value > 500 )
		{
			send_to_char( "\n\r{cValue range is {C-500 {cto {C500{c experience points.{x\n\r", ch );
			return;
		}		
		sprintf(verb, "experience");
	    gain_exp(victim, value);
	}
   
    sprintf( buf,"\n\r{cYou have bonused {W%s {ca whopping {C%d {c%s points.{x\n\r",
    		victim->name, value, verb);
    		send_to_char(buf, ch);

    if ( value > 0 )
    { 
      sprintf( buf,"\n\r{GYou have been bonused {W%d {G%s points!!{x\n\r",
value, verb );
      send_to_char( buf, victim );
    }
    else
    {
      sprintf( buf,"\n\r{RYou have been penalized {W%d {R%s points.{x\n\r",
value, verb );
      send_to_char( buf, victim );
    }
  return;
}

void do_sockets( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA       *vch;
    DESCRIPTOR_DATA *d;
    char            buf  [ MAX_STRING_LENGTH ];
    char            buf2 [ MAX_STRING_LENGTH ];
    int             count;
    char *          st;
    char            s[100];
    char            idle[10];

    count       = 0;
    buf[0]      = '\0';
    buf2[0]     = '\0';

    strcat( buf2, "\n\r{R[{WNum Connec_State Login  Idl{R] {WPlayer       Host{x\n\r" );
    strcat( buf2,
"{R--------------------------------------------------------------------------------------{x\n\r");  
    for ( d = descriptor_list; d; d = d->next )
    {
        if ( d->character && can_see( ch, d->character ) )
        {
           /* NB: You may need to edit the CON_ values */
           switch( d->connected )
           {
              case CON_PLAYING:              st = "  PLAYING   ";    break;
              case CON_GET_NAME:             st = "  Get Name  ";    break;
              case CON_GET_OLD_PASSWORD:     st = " Old Passwd ";    break;
              case CON_CONFIRM_NEW_NAME:     st = "Confirm Name";    break;
              case CON_GET_NEW_PASSWORD:     st = "   New PW   ";    break;
              case CON_CONFIRM_NEW_PASSWORD: st = " Confirm PW ";    break;
              case CON_GET_HARDCORE: st = " Hardcore ";    break;
              case CON_GET_NEW_RACE:         st = "  New Race  ";    break;
              case CON_GET_NEW_SEX:          st = "  New Sex   ";    break;
              case CON_GET_NEW_CLASS:        st = " New Class  ";    break;
              case CON_GET_ALIGNMENT:  	     st = " New Align  ";	break;
              case CON_DEFAULT_CHOICE:       st = "Choose Cust ";	break;
              case CON_GEN_GROUPS:	     st = "Customizing ";	break;
              case CON_PICK_WEAPON:	     st = "Pick Weapon ";	break;
	      case CON_READ_IMOTD:	     st = " Read IMOTD "; break;
	      case CON_BREAK_CONNECT:	     st = "  LINKDEAD  "; break;
              case CON_READ_MOTD:            st = " Read MOTD  ";    break;
	      case CON_GET_COLOUR:   	     st = " Get Color  "; break;
	      case CON_ROLL_STATS: 	     st = " Roll Stats "; break;
	      case CON_NOTE_TO:
	      case CON_NOTE_SUBJECT:
	      case CON_NOTE_EXPIRE:
	      case CON_NOTE_TEXT:
	      case CON_NOTE_FINISH:
	       st = " Write Note "; break;
	      
              default:                       st = " {c!{RUNKNOWN{c!{x  ";    break;
           }
           count++;
           
           /* Format "login" value... */
           vch = d->original ? d->original : d->character;
           strftime( s, 100, "{C%I{w:{C%M{c%p{x", localtime( &vch->logon ) );
           
           if ( vch->timer > 0 )
              sprintf( idle, "{R%-2d{x", vch->timer );
           else
              sprintf( idle, "  {x" );
           
           sprintf( buf, "{R[{C%3d {W%s{x %7s %2s{R] {C%-12s{x {c%-38.38s{x\n\r",
              d->descriptor,st,s,idle,
              ( d->original ) ? d->original->name
                              : ( d->character )  ? d->character->name
                                                  : "(None!)",
              (IS_IMP(d->character) && !IS_IMP(ch)) ? "{c!{RUNKNOWN{c!{x" : d->host );
              
           strcat( buf2, buf );

        }
    }

    sprintf( buf, "\n\r{c[ {W%d {c] {CTotal user%s{x\n\r", count, count == 1 ? "" : "s" );
    strcat( buf2, buf );
    send_to_char( buf2, ch );
    return;

}

void do_rsearch(CHAR_DATA *ch, char *argument)
{
    char                arg[MAX_INPUT_LENGTH];
    char                buf[MAX_STRING_LENGTH];
    int                 counter, vnum;
    bool                found;
    BUFFER *            buffer1;
    ROOM_INDEX_DATA *   room;
    
    if(arg[0] == '\0')
    {
        send_to_char("\n\r{GSyntax{w: {Wrsearch {c<{Wobject{c>{x\n\r",ch);
        return;
    }
    
    counter = 0;
    found = FALSE;
    buffer1 = new_buf();
    
    for(vnum=1; vnum<=32766; vnum++)
    {
        if((room = get_room_index(vnum)))
        {
            if(!IS_IMP(ch))
            {
                if(IS_SET(room->area->area_flags, AREA_IMP))
                    continue;
            }
      
            if(ch->level < ASSTIMP)
            {
                if(IS_SET(room->area->area_flags, AREA_NOIMM) && !IS_SET(ch->comm, COMM_HELPER))
                    continue;
            }

            if(strstr(room->name, arg) || is_name(arg, room->name))
            {
                if(!found)
                    found = TRUE;
                
                counter++;
                sprintf(buf, "{c[{C%5d{c]{x %s{x\n\r", room->vnum, room->name);
                add_buf(buffer1, buf);
            }
        }
    }
    
    if(!found)
    {
        send_to_char("\n\r{RNo rooms were found matching that name.{x\n\r", ch);
        free_buf(buffer1);
        return;
    } 

    send_to_char("\n\r{c[{CVnum {c] {WRoom Name{x", ch);
    send_to_char("\n\r{c-------------------------------------------------------------------{x\n\r",ch);
    add_buf(buffer1,"{c-------------------------------------------------------------------{x\n\r");
       
    sprintf(buf, "{cNumber of Rooms Found{w: {W%d\n\r", counter);
    add_buf(buffer1, buf);
    page_to_char(buf_string(buffer1), ch);
    free_buf(buffer1);    
    
    return;
}

void do_osearch(CHAR_DATA *ch, char *argument)
  {
    char                arg[MAX_INPUT_LENGTH];
    char                buf[MAX_STRING_LENGTH];
    int                 counter, vnum;
    OBJ_INDEX_DATA *    pObjIndex;
    bool                found;
    BUFFER *            buffer1;
    
    argument = one_argument( argument, arg );

    if(arg[0] == '\0')
    {
        send_to_char("\n\r{GSyntax{w: {WOSEARCH {c<{WOBJECT{c>{x\n\r",ch);
	return;
    }

    counter = 0;
    found = FALSE;
    buffer1 = new_buf();

        for(vnum=1; vnum<=32766; vnum++)
        {
            if((pObjIndex = get_obj_index(vnum)))
            {

  if(!IS_IMP(ch))
    {
     if(IS_SET(pObjIndex->area->area_flags, AREA_IMP))
       {
        continue;
       }
      
     if(ch->level < ASSTIMP)
       {
        if(IS_SET(pObjIndex->area->area_flags, AREA_NOIMM))
          {
           if(!IS_SET(ch->comm, COMM_HELPER))
             {
              continue;
             }
          }
       }
    }

                if(is_name(arg, pObjIndex->name))
                {
                    if(!found)
                        found = TRUE;
                    counter++;
sprintf(buf, "{c[{C%5d{c][{W%4d{c]{x %s{x\n\r",pObjIndex->vnum,pObjIndex->level,
strip_color(pObjIndex->short_descr));
                    add_buf(buffer1, buf);
                }
            }
        }
        if(!found)
        {
            send_to_char("\n\r{RNo objects were found matching that name.{x\n\r", ch);
            free_buf(buffer1);
            return;
        } 

        send_to_char("\n\r{c[{CVnum {c][{WLevL{c] {WItem Name{x", ch);
        send_to_char("\n\r{c-------------------------------------------------------------------{x\n\r",ch);
        add_buf(buffer1,"{c-------------------------------------------------------------------{x\n\r");
       
    sprintf(buf, "{cNumber of Objects Found{w: {W%d\n\r", counter);
    add_buf(buffer1, buf);
    page_to_char(buf_string(buffer1), ch);
    free_buf(buffer1);
    
    return;
}

void    do_object(CHAR_DATA *ch, char *argument)
{
    char                arg1[MAX_INPUT_LENGTH];
    char                arg2[MAX_INPUT_LENGTH];
    char                buf[MAX_STRING_LENGTH];
    int                 counter, vnum, type, level;
    OBJ_INDEX_DATA *    pObjIndex;
    OBJ_DATA *          obj;
    OBJ_DATA *          in_obj;
    bool                found;
    BUFFER *            buffer1;
    
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );


    if(arg1[0] == '\0' || arg2[0] == '\0')
    {
        send_to_char("Syntax: object <action> <argument>\n\r"
                     "\n\r"
                     "search  - Searches all possible vnums for <argument>\n\r"
                     "where   - Shows you where on mud all the <argument>'s are\n\r"
                     "level   - <argument> is the level you want to search for\n\r"
                     "type    - <argument> is the type of object to search for\n\r"
                     "material- <argument> is the material type to search for\n\r", ch); 
	return;
    }

    if(str_cmp(arg1, "search") && str_cmp(arg1, "where") &&
       str_cmp(arg1, "level") && str_cmp(arg1, "type")
	&& str_cmp(arg1, "material"))
    {
        send_to_char("Syntax: object <action> <argument>\n\r"
                     "\n\r"
                     "search  - Searches all possible vnums for <argument>\n\r"
                     "where   - Shows you where on mud all the <argument>'s are\n\r"
                     "level   - <argument> is the level you want to search for\n\r"
                     "type    - <argument> is the type of object to search for\n\r" 
                     "material- <argument> is the material type to search for\n\r", ch); 
	return;
    }
    
    counter = 0;
    found = FALSE;
    buffer1 = new_buf();

    if(!str_cmp(arg1, "type"))
    {
        type=flag_value(type_flags, arg2);
        if(type==NO_FLAG)
        {
            send_to_char("Not a valid item type.\n\r", ch);
            free_buf(buffer1);
            return;
        }

        for(vnum=1; vnum<=32766; vnum++)
        {
            if((pObjIndex = get_obj_index(vnum)))
            {
                if(pObjIndex->item_type == type)
                {
                    if(!found)
                        found = TRUE;
                    counter++;
                    sprintf(buf, "{y[%5d]{x %s{x\n\r",
                            pObjIndex->vnum, capitalize(pObjIndex->short_descr));
                    add_buf(buffer1, buf);
                }
            }
        }
        if(!found)
        {
            send_to_char("No objects were found matching that name.\n\r", ch);
            free_buf(buffer1);
            return;
        }

        send_to_char("\n\r Vnum    Item Name\n\r--------------------------------------------------------------------------------\n\r", ch);
        add_buf(buffer1, "--------------------------------------------------------------------------------\n\r");
    }

    if(!str_cmp(arg1, "material"))
    {
        type=flag_value(material_flags, arg2);
        if(type==NO_FLAG)
        {
            send_to_char("Not a valid material type.\n\r", ch);
            free_buf(buffer1);
            return;
        }

        for(vnum=1; vnum<=32766; vnum++)
        {
            if((pObjIndex = get_obj_index(vnum)))
            {
                if(pObjIndex->material_type == type)
                {
                    if(!found)
                        found = TRUE;
                    counter++;
                    sprintf(buf, "{y[%5d]{x %s{x\n\r",
                            pObjIndex->vnum, capitalize(pObjIndex->short_descr));
                    add_buf(buffer1, buf);
                }
            }
        }
        if(!found)
        {
            send_to_char("No objects made of that material were found.\n\r", ch);
            free_buf(buffer1);
            return;
        }

        send_to_char("\n\r Vnum    Item Name\n\r--------------------------------------------------------------------------------\n\r", ch);
        add_buf(buffer1, "--------------------------------------------------------------------------------\n\r");
    }

    if(!str_cmp(arg1, "where"))
    {
        for(obj=object_list; obj; obj=obj->next)
        {
           
           if(!can_see_obj(ch,obj) 
           || !is_name(arg2, obj->name) 
           || (ch->level != (MAX_LEVEL)
           &&  ch->level < obj->level))
                continue;

            found = TRUE;
            counter++;

            for(in_obj=obj; in_obj->in_obj; in_obj=in_obj->in_obj);

            if(in_obj->carried_by && can_see(ch,in_obj->carried_by) &&
               in_obj->carried_by->in_room)
            {
              if (IS_SET (obj->item_type, ITEM_CORPSE_PC))
               {
                sprintf(buf, "{c[{y%5d{c]{x %s is carried by %s {c[{yRoom %d{c]{x\n\r",
                        obj->pIndexData->vnum, capitalize(obj->short_descr),
                        PERS(in_obj->carried_by, ch), in_obj->carried_by->in_room->vnum);
                add_buf(buffer1, buf);
               }
              else
               {
                sprintf(buf, "{c[{y%5d{c]{x %s is carried by %s {c[{yRoom %d{c]{x\n\r",
                        obj->pIndexData->vnum, capitalize(obj->pIndexData->short_descr),
                        PERS(in_obj->carried_by, ch), in_obj->carried_by->in_room->vnum);
                add_buf(buffer1, buf);
               }
            }
            else if(in_obj->in_room && can_see_room(ch,in_obj->in_room))
            {
              if (IS_SET (obj->item_type, ITEM_CORPSE_PC))
               {
                sprintf(buf, "{c[{y%5d{c]{x %s is in %s {c[{yRoom %d{c]{x\n\r",
                        obj->pIndexData->vnum, capitalize(obj->short_descr),
                        in_obj->in_room->name, in_obj->in_room->vnum);
                add_buf(buffer1, buf);
               }
              else
               {
                sprintf(buf, "{c[{y%5d{c]{x %s is in %s {c[{yRoom %d{c]{x\n\r",
                        obj->pIndexData->vnum, capitalize(obj->pIndexData->short_descr),
                        in_obj->in_room->name, in_obj->in_room->vnum);
                add_buf(buffer1, buf);
               }
            }
            else
            {
              if (IS_SET (obj->item_type, ITEM_CORPSE_PC))
               {
                sprintf(buf, "{c[{y%5d{c]{x %s is somewhere{x\n\r",
                        obj->pIndexData->vnum, capitalize(obj->short_descr));
                add_buf(buffer1, buf);
               }
              else
               {
                sprintf(buf, "{c[{y%5d{c]{x %s is somewhere{x\n\r",
                        obj->pIndexData->vnum, capitalize(obj->pIndexData->short_descr));
                add_buf(buffer1, buf);
               }
            }
        }
        if(!found)
        {
            send_to_char("No objects were found matching that name.\n\r", ch);
            free_buf(buffer1);
            return;
        }

        send_to_char("\n\r Vnum    Item Name     Where          Room Vnum\n\r--------------------------------------------------------------------------------\n\r", ch);
        add_buf(buffer1, "--------------------------------------------------------------------------------\n\r");
    }
    

    if(!str_cmp(arg1, "level"))
    {
    	if(!is_number(arg2))
    	{
    		send_to_char("Thats not a valid level.\n\r", ch);
    		free_buf(buffer1);
    		return;
    	}
    	level=atoi(arg2);
        for(vnum=1; vnum<=32766; vnum++)
        {
            if((pObjIndex = get_obj_index(vnum)))
            {
                if( pObjIndex->level==level)
                {
                    if(!found)
                        found = TRUE;
                    counter++;
                    sprintf(buf, "{y[%5d]{x %s{x\n\r", pObjIndex->vnum, capitalize(pObjIndex->short_descr));
                    add_buf(buffer1, buf);
                }
            }
        }

        if(!found)
        {
            send_to_char("No objects were found matching that level.\n\r", ch);
            free_buf(buffer1);
            return;
        }

        send_to_char("\n\r Vnum    Item Name\n\r--------------------------------------------------------------------------------\n\r", ch);
        add_buf(buffer1, "--------------------------------------------------------------------------------\n\r");
    }
    
    if(!str_cmp(arg1, "search"))
    {
        for(vnum=1; vnum<=32766; vnum++)
        {
            if((pObjIndex = get_obj_index(vnum)))
            {
                if(is_name(arg2, pObjIndex->name))
                {
                    if(!found)
                        found = TRUE;
                    counter++;
                    sprintf(buf, "{y[%5d]{x %s{x\n\r",
                            pObjIndex->vnum, capitalize(pObjIndex->short_descr));
                    add_buf(buffer1, buf);
                }
            }
        }
        if(!found)
        {
            send_to_char("No objects were found matching that name.\n\r", ch);
            free_buf(buffer1);
            return;
        } 

        send_to_char("\n\r Vnum    Item Name\n\r--------------------------------------------------------------------------------\n\r", ch);
        add_buf(buffer1, "--------------------------------------------------------------------------------\n\r");
    }
    
    sprintf(buf, "Number of Objects Found: %d\n\r", counter);
    add_buf(buffer1, buf);
    page_to_char(buf_string(buffer1), ch);
    free_buf(buffer1);
    
    return;
}


void do_msearch(CHAR_DATA *ch, char *argument)
  {
    char                arg[MAX_INPUT_LENGTH];
    char                buf[MAX_STRING_LENGTH];
    int                 counter, vnum;
    MOB_INDEX_DATA *    pMobIndex;
    bool                found;
    BUFFER *            buffer1;

    argument = one_argument( argument, arg );

    if(arg[0] == '\0')
    {
        send_to_char("\n\r{GSyntax{w: {WMSEARCH {c<{WMOB NAME{c>{x\n\r",ch);
	return;
    }

    counter = 0;
    found = FALSE;
    buffer1 = new_buf();

        for(vnum=1; vnum<=32766; vnum++)
        {
            if((pMobIndex = get_mob_index(vnum)))
            {
 
  if(!IS_IMP(ch))
    {
     if(IS_SET(pMobIndex->area->area_flags, AREA_IMP))
       {
        continue;
       }
        
     if(ch->level < ASSTIMP)
       {
        if(IS_SET(pMobIndex->area->area_flags, AREA_NOIMM))
          {
           if(!IS_SET(ch->comm, COMM_HELPER))
             {
              continue;
             }
          }  
       }
    }

               if(is_name(arg, pMobIndex->player_name))
                {
                    if(!found)
                        found = TRUE;
                    counter++;
                    sprintf(buf, "{c[{C%5d{c][{W%4d{c] {x %s{x\n\r",
                            pMobIndex->vnum,pMobIndex->level,
		strip_color(pMobIndex->short_descr));
                    add_buf(buffer1, buf);
                }
            }
        }
        if(!found)
        {
            send_to_char("\n\r{RNo mobiles were found matching that name.{x\n\r", ch);
            free_buf(buffer1);
            return;
        } 

        send_to_char("\n\r{c[{CVNum {c][{WLevL{c] {WMobile Name{x", ch);
        send_to_char("\n\r{c-------------------------------------------------------------------{x\n\r",ch);
        add_buf(buffer1,"{c-------------------------------------------------------------------{x\n\r");

    sprintf(buf, "{cNumber of Mobiles Found{w: {W%d{x\n\r", counter);
    add_buf(buffer1, buf);
    page_to_char(buf_string(buffer1), ch);
    free_buf(buffer1);
    
    return;
}


void do_mlevel(CHAR_DATA *ch, char *argument)
{
    char                arg1[MAX_INPUT_LENGTH];
    char                buf[MAX_STRING_LENGTH];
    int                 counter, vnum, level;
    MOB_INDEX_DATA *    pMobIndex;
    bool                found;
    BUFFER *            buffer1;
        	
	argument = one_argument( argument, arg1 );

    level=atoi(arg1);

    if(arg1[0] == '\0')
    {
        send_to_char("\n\r{GSyntax{w: {WMLEVEL {c<{WTARGET LEVEL{c>{x\n\r",ch);
        return;
    }

    counter = 0;
    found = FALSE;
    buffer1 = new_buf();
    
        if(!is_number(arg1))
        {
            send_to_char("Thats not a valid level.\n\r", ch);
            free_buf(buffer1);
            return;
        }
 
    send_to_char("\n\r Vnum    Mobile Name\n\r--------------------------------------------------------------------------------\n\r", ch);

        for(vnum=1; vnum<=32766; vnum++)
        {
            if((pMobIndex = get_mob_index(vnum)))
            {
                if(pMobIndex->level==level)
                {
                    if(!found)
                        found = TRUE;
                    counter++;
                    sprintf(buf, "{y[%5d]{x %s{x\n\r", pMobIndex->vnum,
                            capitalize(pMobIndex->short_descr));
                    add_buf(buffer1, buf);
                }
            }
        }

        if(!found)
        {
            send_to_char("No mobiles were found matching that level.\n\r", ch);
            free_buf(buffer1);
            return;
        }


    add_buf(buffer1, "--------------------------------------------------------------------------------\n\r");
    
    sprintf(buf, "Number of Mobiles Found: %d\n\r", counter);
    add_buf(buffer1, buf);
    page_to_char(buf_string(buffer1), ch);
    free_buf(buffer1);
    return;
}

/** Function: do_pload
  * Descr   : Loads a player object into the mud, bringing them (and their
  *           pet) to you for easy modification.  Player must not be connected.
  *           Note: be sure to send them back when your done with them.
  * Returns : (void)
  * Syntax  : pload (who)
  * Written : v1.0 12/97
  * Author  : Gary McNickle <gary@dharvest.com>
  */
void do_pload( CHAR_DATA *ch, char *argument )
{
  DESCRIPTOR_DATA d;
  bool isChar = FALSE;
  char name[MAX_INPUT_LENGTH];

  if (argument[0] == '\0')
  {
    send_to_char("Load who?\n\r", ch);
    return;
  }

  argument[0] = UPPER(argument[0]);
  argument = one_argument(argument, name);

  /* Dont want to load a second copy of a player who's allready online! */
  if ( get_char_world( ch, name ) != NULL )
  {
    send_to_char( "That person is already connected!\n\r", ch );
    return;
  }

	if(!str_cmp(name, "Venus"))
	{
	send_to_char("Venus doesn't want to be on right now, Sorry.", ch);
		return;

	}

  isChar = load_char_obj(&d, name); /* char pfile exists? */

  if (!isChar)
  {
    send_to_char("Load Who? Are you sure? I cant seem to find them.\n\r", ch);
    return;
  }

  d.character->desc     = NULL;
  d.character->next     = char_list;
  char_list             = d.character;
  d.connected           = CON_PLAYING; 
  reset_char(d.character);
 

  /* bring player to imm */
  if ( d.character->in_room != NULL )
  {
    char_to_room( d.character, ch->in_room); /* put in room imm is in */
  }

  act( "$n has pulled $N from the pattern!",
        ch, NULL, d.character, TO_ROOM );

  if (d.character->pet != NULL)
   {
     char_to_room(d.character->pet,d.character->in_room);
     act("$n has entered the game.",d.character->pet,NULL,NULL,TO_ROOM);
   }

}

/** Function: do_punload
  * Descr   : Returns a player, previously 'ploaded' back to the void from
  *           whence they came.  This does not work if the player is actually 
  *           connected.
  * Returns : (void)
  * Syntax  : punload (who)
  * Written : v1.0 12/97
  * Author  : Gary McNickle <gary@dharvest.com>
  */
void do_punload( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *victim;
  char who[MAX_INPUT_LENGTH];

  argument = one_argument(argument, who);

  if ( ( victim = get_char_world( ch, who ) ) == NULL )
  {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }

  /** Person is legitametly logged on... was not ploaded.
   */
  if (victim->desc != NULL)
  {
    send_to_char("I dont think that would be a good idea...\n\r", ch);
    return;
  }

  if (victim->was_in_room != NULL) /* return player and pet to orig room */
  {
    char_to_room(victim, victim->was_in_room);
    if (victim->pet != NULL)
      char_to_room(victim->pet, victim->was_in_room);
  }

  save_char_obj(victim);
  do_quit(victim,"");

  act("$n has released $N back to the Pattern.",
       ch, NULL, victim, TO_ROOM);
}

/*
void    do_seepower(CHAR_DATA *ch, char *argument)
{
    char            arg1[MAX_INPUT_LENGTH];
    char            buf[MAX_STRING_LENGTH];
    CHAR_DATA *     victim;
    bool            found;
    int cmd;
    int col;
    int i;

    argument = one_argument( argument, arg1 );

    if(arg1[0] == '\0')
    {
        send_to_char("\n\r{GSyntax{w: {WSEEPOWER {c<{WCHAR NAME{c>{x\n\r", ch);
        return;
    }
    
    victim = get_char_world(ch, arg1);
    if(victim==NULL)
    {
        send_to_char("\n\r{MThat player is not online.{x\n\r", ch);
        return;
    }

    if (victim->desc == NULL)
        victim = victim;
    else
        victim = victim->desc->original ? victim->desc->original : victim;

    if(!victim->pcdata)
        return;

    if(IS_NPC(victim))
    {
        send_to_char("\n\r{RNot on NPC's.{x\n\r", ch);
        return;
    }
    
    if(victim->level > ch->level)
    {
        send_to_char("\n\r{That player is ABOVE your level.{x\n\r", ch);
        return;
    }

    sprintf(buf, "\n\r{cImmortal Commands{w: {W%s {x\n\r\n\r", victim->name);
    send_to_char(buf, ch);
    found=FALSE;



    col = 0;
    for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
              {
                  if ( cmd_table[cmd].level >= LEVEL_HERO     
                  &&   cmd_table[cmd].level <= victim->level
                  &&   cmd_table[cmd].show)
                     {
                 sprintf( buf, "{r[{W%-10s{r]", cmd_table[cmd].name );
                 send_to_char( buf, ch );
                 if ( ++col % 6 == 0 )
                 send_to_char( "{x\n\r", ch );
                     }

               }

    if ( col % 6 != 0 )
    send_to_char( "{x\n\r", ch );
    return;
}
*/


/*
 * do_rename renames a player to another name.
 * PCs only. Previous file is deleted, if it exists.
 * Char is then saved to new file.
 * New name is checked against std. checks, existing offline players and
 * online players. 
 * .gz files are checked for too, just in case.
 */

bool check_parse_name (char* name);  /* comm.c */

void do_rename (CHAR_DATA* ch, char* argument)
{
	char old_name[MAX_INPUT_LENGTH], 
	     new_name[MAX_INPUT_LENGTH],
	     strsave [MAX_INPUT_LENGTH];

	CHAR_DATA* victim;
	FILE* file;
	
	argument = one_argument(argument, old_name); /* find new/old name */
	one_argument (argument, new_name);

	/* Trivial checks */
	if (!old_name[0])
	{
		send_to_char ("Rename who?\n\r",ch);
		return;
	}
	
	victim = get_char_world (ch, old_name);
	
	if (!victim)
	{
		send_to_char ("There is no such a person online.\n\r",ch);
		return;
	}
	
	if (IS_NPC(victim))
	{   
		send_to_char ("You cannot use Rename on NPCs.\n\r",ch);
		return;
	}

	/* allow rename self new_name,but otherwise only lower level */	
	if ( (victim != ch) && (victim->level >= ch->level) )
	{
		send_to_char ("You failed.\n\r",ch);
		return;
	}
	
	if (!victim->desc || (victim->desc->connected != CON_PLAYING) )
	{
		send_to_char ("This player has lost his link or is inside a pager or the like.\n\r",ch);
		return;
	}

	if (!new_name[0])
	{
		send_to_char ("Rename to what new name?\n\r",ch);
		return;
	}
	
	if (!check_parse_name(new_name))
	{
		send_to_char ("The new name is illegal.\n\r",ch);
		return;
	}

	/* First, check if there is a player named that off-line */	

    sprintf( strsave, "%s%s", PLAYER_DIR, capitalize( new_name ) );

	fclose (fpReserve); /* close the reserve file */
	file = fopen (strsave, "r"); /* attempt to to open pfile */
	if (file)
	{
		send_to_char ("A player with that name already exists!\n\r",ch);
		fclose (file);
    	fpReserve = fopen( NULL_FILE, "r" ); /* is this really necessary these days? */
		return;		
	}
   	fpReserve = fopen( NULL_FILE, "r" );  /* reopen the extra file */

	/* Check .gz file ! */
    sprintf( strsave, "%s%s.gz", PLAYER_DIR, capitalize( new_name ) );

	fclose (fpReserve); /* close the reserve file */
	file = fopen (strsave, "r"); /* attempt to to open pfile */
	if (file)
	{
		send_to_char ("A player with that name already exists in a compressed file!\n\r",ch);
		fclose (file);
    	fpReserve = fopen( NULL_FILE, "r" ); 
		return;		
	}
   	fpReserve = fopen( NULL_FILE, "r" );  /* reopen the extra file */

	if (get_char_world(ch,new_name)) /* check for playing level-1 non-saved */
	{
		send_to_char ("A player with the name you specified already exists!\n\r",ch);
		return;
	}

	/* Save the filename of the old name */
    sprintf( strsave, "%s%s", PLAYER_DIR, capitalize( victim->name ) );


	/* Rename the character and save him to a new file */
	/* NOTE: Players who are level 1 do NOT get saved under a new name */

	free_string (victim->name);
	victim->name = str_dup (capitalize(new_name));
	
	save_char_obj (victim);
	
	/* unlink the old file */
	unlink (strsave); /* unlink does return a value.. but we do not care */

	/* That's it! */
	
	send_to_char ("Character renamed.\n\r",ch);

	victim->position = POS_STANDING; /* I am laaazy */
	act ("$n has renamed you to $N!",ch,NULL,victim,TO_VICT);
			
} /* do_rename */

const char * name_expand (CHAR_DATA *ch)
{
	int count = 1;
	CHAR_DATA *rch;
	char name[MAX_INPUT_LENGTH]; /*  HOPEFULLY no mob has a name longer than THAT */

	static char outbuf[MAX_INPUT_LENGTH];	
	
	if (!IS_NPC(ch))
		return ch->name;
		
	one_argument (ch->name, name); /* copy the first word into name */
	
	if (!name[0]) /* weird mob .. no keywords */
	{
		strcpy (outbuf, ""); /* Do not return NULL, just an empty buffer */
		return outbuf;
	}
		
	for (rch = ch->in_room->people; rch && (rch != ch);rch = rch->next_in_room)
		if (is_name (name, rch->name))
			count++;
			

	sprintf (outbuf, "%d.%s", count, name);
	return outbuf;
}

void do_doat (CHAR_DATA *ch, char *argument)
{
	char range[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	bool fGods = FALSE, fMortals = FALSE, found;
	ROOM_INDEX_DATA *room, *old_room;
	CHAR_DATA *p, *p_next;
	int i;
	
	argument = one_argument (argument, range);
	
	if (!range[0] || !argument[0]) /* invalid usage? */
	{

	send_to_char("\n\r{GSyntax{w: {WDOAT {c<{WTARGET TYPE{c> <{WCOMMAND{c>{x\n\r",ch);
	send_to_char("\n\r{cTarget Types{w: {CMORTALS GODS MOBS EVERYWHERE{x\n\r",ch);
		return;
	}
	
	if (!str_prefix("quit", argument))
	{
		send_to_char ("Are you trying to crash the MUD or something?\n\r",ch);
		return;
	}
	
	
	if (!str_cmp (range, "all"))
	{
		fMortals = TRUE;
		fGods = TRUE;
	}
	else if (!str_cmp (range, "gods"))
		fGods = TRUE;
	else if (!str_cmp (range, "mortals"))
		fMortals = TRUE;
	else
		do_help (ch, "for"); /* show syntax */


	if (strchr (argument, '#')) /* replace # ? */
	{ 
		for (p = char_list; p ; p = p_next)
		{
			p_next = p->next; /* In case someone DOES try to AT MOBS SLAY # */
			found = FALSE;
			
			if (p == ch || IS_NPC(p) || (!(p->in_room)) )
			   continue;

/*
			if (!(p->in_room) || room_is_private(p->in_room) || (p == ch))
				continue;
*/
			
			if (!IS_NPC(p) && p->level >= LEVEL_IMMORTAL && fGods)
				found = TRUE;
			else if (!IS_NPC(p) && p->level < LEVEL_IMMORTAL && fMortals)
				found = TRUE;

			/* It looks ugly to me.. but it works :) */				
			if (found) /* p is 'appropriate' */
			{
				char *pSource = argument; /* head of buffer to be parsed */
				char *pDest = buf; /* parse into this */
				
				while (*pSource)
				{
					if (*pSource == '#') /* Replace # with name of target*/
					{
						const char *namebuf = name_expand (p);
						
						if (namebuf) /* in case there is no mob name?? */
							while (*namebuf) /* copy name over */
								*(pDest++) = *(namebuf++);

						pSource++;
					}
					else
						*(pDest++) = *(pSource++);
				} /* while */
				*pDest = '\0'; /* Terminate */
				
				/* Execute */
				old_room = ch->in_room;
				char_from_room (ch);
				char_to_room (ch,p->in_room);
				interpret (ch, buf);
				char_from_room (ch);
				char_to_room (ch,old_room);
				
			} /* if found */
		} /* for every char */
	}
	else /* just for every room with the appropriate people in it */
	{
		for (i = 0; i < MAX_KEY_HASH; i++) /* run through all the buckets */
			for (room = room_index_hash[i] ; room ; room = room->next)
			{
				found = FALSE;
				
				/* Anyone in here at all? */
				if (!room->people) /* Skip it if room is empty */
					continue;
					
					
				/* Check if there is anyone here of the requried type */
				/* Stop as soon as a match is found or there are no more ppl in room */
				for (p = room->people; p && !found; p = p->next_in_room)
				{

					if (p == ch || IS_NPC(p) || (!(p->in_room)) )
						continue;
						
					if (!IS_NPC(p) && (p->level >= LEVEL_IMMORTAL) && fGods)
						found = TRUE;
					else if (!IS_NPC(p) && (p->level <= LEVEL_IMMORTAL) && fMortals)
						found = TRUE;
				} /* for everyone inside the room */
						
				if (found && !room_is_private(room))
/* Any of the required type here AND room not private? */
				{
					old_room = ch->in_room;
					char_from_room (ch);
					char_to_room (ch, room);
					interpret (ch, argument);
					char_from_room (ch);
					char_to_room (ch, old_room);
				} 
			} 
	} 
}

void do_addlag(CHAR_DATA *ch, char *argument)
{

	CHAR_DATA *victim;
	char arg1[MAX_STRING_LENGTH];
	int x;

	argument = one_argument(argument, arg1);

    if ( argument[0] == '\0'|| arg1[0] == '\0' )
    {
        send_to_char( "\n\r{GSyntax{w: {WADDLAG {c<{WTARGET{c> <{WTICKs of LAG{c>{x\n\r",ch );
        send_to_char( "\n\r{BLAGs the TARGET for {WX{B number of ticks.{x\n\r",ch );
        return;
    }

	if ((victim = get_char_world(ch, arg1)) == NULL)
	{
		send_to_char("They're not here.", ch);
		return;
	}

	if ((x = atoi(argument)) <= 0)
	{
		send_to_char("That makes a LOT of sense.", ch);
		return;
	}
 
	if (x > 100)
	{
		send_to_char("There's a limit to cruel and unusual punishment", ch);
		return;
	 }

	if(ch->level != MAX_LEVEL)
            send_to_char("Somebody REALLY didn't like you", victim);
	WAIT_STATE(victim, x);
	send_to_char("Adding lag now...", ch);
	return;
}

void do_grab (CHAR_DATA *ch, char *argument)
{
    CHAR_DATA  *victim;
    OBJ_DATA   *obj;
    char        arg1 [ MAX_INPUT_LENGTH ];
    char        arg2 [ MAX_INPUT_LENGTH ];
    char        arg3 [ MAX_INPUT_LENGTH ];

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );

    if ( ( arg1[0] == '\0' ) || ( arg2[0] == '\0' ) )
    {
        send_to_char( "Syntax : grab <object> <player>\n\r", ch );
        return;
    }
    if ( !( victim = get_char_world( ch, arg2 ) ) )
    {
        send_to_char( "They are not here!\n\r", ch );
        return;
    }
	if(!str_cmp(victim->name, "Venus"))
	{
		send_to_char("Boy aren't you a grabby one...\n\r", ch);
		return;
	}
    if ( !( obj = get_obj_list( ch, arg1, victim->carrying ) ) )
    {
        send_to_char( "They do not have that item.\n\r", ch );
        return;
    }

    if (!IS_IMP(ch))
      {
    if ( victim->level >= ch->level )
    {
        send_to_char( "You Failed.\n\r", ch );
        return;
    }
     }

    if ( obj->wear_loc != WEAR_INVENTORY )
        unequip_char( victim, obj );

    obj_from_char( obj );
    obj_to_char( obj, ch );

    act( "You grab $p from $N.", ch, obj, victim, TO_CHAR );
    if ( arg3[0] == '\0'
        || !str_cmp( arg3, "yes" ) || !str_cmp( arg3, "true" ) )
           act( "You no longer own $p.", ch, obj, victim, TO_VICT );
    return;
}


void do_outfit ( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    int i,sn,vnum;

    if (ch->level <= ASSTIMP)
      {
    if (ch->level > 5 || IS_NPC(ch))
    {
	send_to_char("Find it yourself!\n\r",ch);
	return;
    }
      }

    if ( ( obj = get_eq_char( ch, WEAR_LIGHT ) ) == NULL )
    {
        obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_BANNER), 0 );
	obj->cost = 0;
	obj_to_char( obj, ch );
        equip_char( ch, obj, WEAR_LIGHT );
    }
 
    if ( ( obj = get_eq_char( ch, WEAR_BODY ) ) == NULL )
    {
	obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_VEST), 0 );
	obj->cost = 0;
        obj_to_char( obj, ch );
        equip_char( ch, obj, WEAR_BODY );
    }

    /* do the weapon thing */
    if ((obj = get_eq_char(ch,WEAR_WIELD)) == NULL)
	 {
    	sn = 1; 
    	vnum = OBJ_VNUM_SCHOOL_SWORD; /* just in case! */

    	for (i = 1; weapon_table[i].name != NULL; i++)
    	{
         if (ch->pcdata->learned[sn] < ch->pcdata->learned[*weapon_table[i].gsn])
	    {
	    	sn = *weapon_table[i].gsn;
	    	vnum = weapon_table[i].vnum;
	    }
    	}

	obj = create_object(get_obj_index(vnum),0);
     	obj_to_char(obj,ch);
    	equip_char(ch,obj,WEAR_WIELD);
    }

    if (((obj = get_eq_char(ch,WEAR_WIELD)) == NULL 
    ||   !IS_WEAPON_STAT(obj,WEAPON_TWO_HANDS))
    &&  (obj = get_eq_char( ch, WEAR_SHIELD ) ) == NULL )
    {
        obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_SHIELD), 0 );
	obj->cost = 0;
        obj_to_char( obj, ch );
        equip_char( ch, obj, WEAR_SHIELD );
    }

    send_to_char("You have been equipped by Mota.\n\r",ch);
}


void do_scatter(CHAR_DATA *ch, char * argument)
{
    char                arg1[MAX_INPUT_LENGTH];
    char                buf[MAX_STRING_LENGTH];
    OBJ_DATA *          obj;
    AREA_DATA *         area;
    ROOM_INDEX_DATA *   room;
    EXIT_DATA *		pexit;
    int                 i, max_rooms, target_room;
    
    argument = one_argument( argument, arg1 );

    if(arg1[0] == '\0')
    {
        send_to_char("Usage: scatter <object>\n\r",ch);
        return;
    }
    
    obj = get_obj_carry(ch, arg1, ch);
    
    if(obj == NULL)
    {
        send_to_char( "You do not have that item.\n\r", ch );
        return;
    }

    area=ch->in_room->area;
    max_rooms = area->max_vnum - area->min_vnum;
    i=0;
    
    do
    {
        target_room = number_range(area->min_vnum, area->max_vnum);
        room = get_room_index(target_room);
        if(room)
        {
            if( room->status >= 1 || room_is_private(room) || 
                IS_SET(room->room_flags, ROOM_IMP_ONLY) ||
                IS_SET(room->room_flags, ROOM_GODS_ONLY) ||
                IS_SET(room->room_flags, ROOM_HEROES_ONLY) ||
                IS_SET(room->room_flags, ROOM_NEWBIES_ONLY))
                room=NULL;
            for(pexit=NULL;pexit==NULL&&i<9;i++)
            	pexit=room->exit[i];
            if(pexit==NULL)
                room=NULL;
        }
        i++;
    }while(room==NULL && i<max_rooms);

    if(i>=max_rooms)
    {
        send_to_char("No rooms found in area.\n\r", ch);
        return;
    }
    
    
    obj_from_char(obj);
    obj_to_room(obj, room);
    
    sprintf(buf, "{WSent: {x%s {Wto:{x %d{x\n\r", obj->short_descr, room->vnum);
    send_to_char(buf, ch);
}

void do_shatter(CHAR_DATA *ch, char * argument)
{
    char                arg1[MAX_INPUT_LENGTH];
    char                buf[MAX_STRING_LENGTH];
    OBJ_DATA *          obj;
    int                 i;
    
    argument = one_argument( argument, arg1 );

    if(arg1[0] == '\0')
    {
        send_to_char("Usage: shatter <object>\n\r",ch);
        return;
    }
    
    i=0;
    for (obj=object_list;obj!=NULL;obj=obj->next)
    {
        if(is_name(arg1, obj->name) && IS_OBJ_STAT(obj, ITEM_FRAGILE))
        {
            i++;
            if(obj->carried_by != NULL)
            {
                sprintf(buf, "%s is shattered.\n\r", obj->short_descr);
                send_to_char(buf, obj->carried_by);
                obj_from_char(obj);
            }
            extract_obj(obj);
        }
    }
    
    sprintf(buf, "%d items were shattered.\n\r", i);
    send_to_char(buf, ch);
    
    return;
}


void do_flame( CHAR_DATA *ch, char *argument )
{
  char buf[1024];
  CHAR_DATA *victim = NULL;
  DESCRIPTOR_DATA *d;

  if ( argument[0] == '\0' )
	{
	send_to_char( "{xFlame whom?\n\r", ch );
	return;
	}
  if ( !str_cmp( argument, "all" ) )
	{
	send_to_char( "Not yet implemented.\n\r", ch );
	return;
	}
  for ( d = descriptor_list; d; d = d->next )
	if ( d->connected == CON_PLAYING 
	&& is_name( argument, d->character->name )
	&& can_see( ch, d->character ) )
	  victim = d->character;
  if ( !victim )
	{
	send_to_char( "No such player online at this time.\n\r", ch );
	return;
	}
  if ( victim->in_room != ch->in_room )
   {
   sprintf( buf, "You summon the {Ddemons{x from {rhell{x to claim the soul of {R$N!" );
   act( buf, ch, NULL, victim, TO_CHAR );
   sprintf( buf, "{xThousands of lighting bolts {xerupt from $n's hands towards an unknown target." );
   act( buf, ch, NULL, victim, TO_ALL );
   sprintf( buf, "{xThe earth trembles and opens to release the greater powers from {rhell{x" );
   for ( d = descriptor_list; d; d = d->next )
	if ( d->connected == CON_PLAYING )
	  {
	  act( buf, d->character, NULL, NULL, TO_CHAR );
	  if ( victim->in_room != d->character->in_room )
	    {
	    act( "The {Rflames{x rise to seek their prey.", d->character, NULL, NULL, TO_CHAR );
	    act( "The {Rflames{x quickly consume their victim", d->character, NULL, NULL, TO_CHAR );
	    }
	  }
   act( "The {Rfires{x quickly find you as you are frieghtened from their awe.", victim, NULL, NULL, TO_CHAR );
   act( "7  Above you, the {Clightning {xconverges, forming a massive {Cbolt {xof pure {Clightning{x.", victim, NULL, NULL, TO_ALL );
   act( "Suddenly the {Rfire{x rages towards YOU!", victim, NULL, NULL, TO_CHAR );
   act( "9 Suddenly the {Clightning {xplunges towards YOU!", victim, NULL, NULL, TO_ALL );
   act( "10 Sharply, the {Clightning {xbends towards $n and hits him dead center in the head!", victim, NULL, NULL, TO_ALL );
   act( "You feel the {Yheat{x as it closes in...", victim, NULL, NULL, TO_CHAR );
   }
  else
   {
   act( "You send forth a huge {DBall of {RFire {xtowards $N.", ch, NULL, victim, TO_CHAR );
   act( "{RFire {xerupts from $n's hands crackling towards $N.", ch, NULL, victim, TO_NOTVICT );
   act( "{RFire {xerupts from $n's hands crackling towards YOU!", ch, NULL, victim, TO_VICT );
   }
   send_to_char( "\n\rYou scream out in pain as the {Rfire {xburns.\n\r", victim );
   send_to_char( "You feel your skin bubble from the {Devil {Rfires{x cast upon you.\n\r", victim );
   act( "$n sreams in pain and falls to $s knees.", victim, NULL, NULL, TO_ALL );
   sprintf( buf, "{CStop the {rburning{C, please have mercy upon my soul!");
   do_gecho( victim, buf );
   {
   AFFECT_DATA af;
   af.where	= TO_AFFECTS;
   af.type	= skill_lookup( "blindness" );
   af.level	= 110;
   af.duration	= 25;
   af.location  = APPLY_HITROLL;
   af.modifier  = -25;
   af.bitvector = AFF_BLIND;
   affect_to_char( victim, &af );
   af.location	= APPLY_AC;
   af.modifier	= 25;
   affect_to_char( victim, &af );

   af.type	= skill_lookup( "weaken" );
   af.level	= 110;
   af.duration	= 25;
   af.location	= APPLY_STR;
   af.modifier  = -5;
   af.bitvector	= 0;
   affect_to_char( victim, &af );

   af.type	= skill_lookup( "plague" );
   af.level	= 110;
   af.duration	= victim->level;
   af.location  = APPLY_STR;
   af.modifier  = -5;
   af.bitvector = AFF_PLAGUE;
   affect_to_char( victim, &af );

   af.type	= skill_lookup( "curse" );
   af.level	= 110;
   af.duration	= 25;
   af.location	= APPLY_SAVES;
   af.modifier	= 15;
   af.bitvector = AFF_CURSE;
   affect_to_char( victim, &af );

   victim->hit /= 2;
   victim->mana /= 2;
	victim->move /= 2;
	}
return;
}  

void do_bolt( CHAR_DATA *ch, char *argument )
{
  char buf[1024];
  CHAR_DATA *victim = NULL;
  DESCRIPTOR_DATA *d;
  if ( str_cmp( ch->name, "Venus" ) )
	{
	send_to_char( "{xHuh?\n\r", ch );
	return;
	}
  if ( argument[0] == '\0' )
	{
	send_to_char( "{xBolt whom?\n\r", ch );
	return;
	}
  if ( !str_cmp( argument, "all" ) )
	{
	send_to_char( "Not yet implemented.\n\r", ch );
	return;
	}
  for ( d = descriptor_list; d; d = d->next )
	if ( d->connected == CON_PLAYING 
	&& is_name( argument, d->character->name )
	&& can_see( ch, d->character ) )
	  victim = d->character;
  if ( !victim )
	{
	send_to_char( "No such player online at this time.\n\r", ch );
	return;
	}
  if ( victim->in_room != ch->in_room )
   {
   sprintf( buf, "{xYou send forth thousands of {Clightning bolts {xto chase $N from the lands!" );
   act( buf, ch, NULL, victim, TO_CHAR );
   sprintf( buf, "{xThousands of {Clighting bolts {xerupt from $n's hands towards an unknown target." );
   act( buf, ch, NULL, victim, TO_ROOM );
   sprintf( buf, "{xThe sky lightens as thousands of {Clighting bolts {xbegin dancing in it." );
   for ( d = descriptor_list; d; d = d->next )
	if ( d->connected == CON_PLAYING )
	  {
	  act( buf, d->character, NULL, NULL, TO_CHAR );
	  if ( victim->in_room != d->character->in_room )
	    {
	    act( "Slowly, the {Clightning bolts {xstart to unite.", d->character, NULL, NULL, TO_CHAR );
	    act( "Suddenly, the immense new {Clightning bolt {xplunges to the earth!", d->character, NULL, NULL, TO_CHAR );
	    }
	  }
   act( "Above you, the {Clightning {xconverges, forming a massive {Cbolt {xof pure {Clightning{x.", victim, NULL, NULL, TO_CHAR );
   act( "Above you, the {Clightning {xconverges, forming a massive {Cbolt {xof pure {Clightning{x.", victim, NULL, NULL, TO_ROOM );
   act( "Suddenly the {Clightning {xplunges towards YOU!", victim, NULL, NULL, TO_CHAR );
   act( "Suddenly the {Clightning {xplunges towards YOU!", victim, NULL, NULL, TO_ROOM );
   act( "Sharply, the {Clightning {xbends towards $n and hits him dead center in the head!", victim, NULL, NULL, TO_ROOM );
   act( "Fear strikes you as the {Clightning {xbends towards you...", victim, NULL, NULL, TO_CHAR );
   }
  else
   {
   act( "You send forth a huge {Clightning bolt {xtowards $N.", ch, NULL, victim, TO_CHAR );
   act( "{CLightning {xerupts from $n's hands crackling towards $N.", ch, NULL, victim, TO_NOTVICT );
   act( "{CLightning {xerupts from $n's hands crackling towards YOU!", ch, NULL, victim, TO_VICT );
   }
   send_to_char( "\n\rYou scream out in pain as the {Clightning {xstrikes.\n\r", victim );
   send_to_char( "You feel your skin sear and your eyes melt.\n\r", victim );
   act( "$n screams in pain and falls to $s knees.", victim, NULL, NULL, TO_ROOM );
   sprintf( buf, "The PAIN, make it go away, please %s, have mercy upon my soul!",
	    ( ch->invis_level ) ? "GODS" : "Venus" );
   do_gecho( victim, buf );
   {
   AFFECT_DATA af;
   af.where	= TO_AFFECTS;
   af.type	= skill_lookup( "blindness" );
   af.level	= 110;
   af.duration	= 25;
   af.location  = APPLY_HITROLL;
   af.modifier  = -25;
   af.bitvector = AFF_BLIND;
   affect_to_char( victim, &af );

   af.location	= APPLY_AC;
   af.modifier	= 25;
   affect_to_char( victim, &af );

   af.type	= skill_lookup( "weaken" );
   af.level	= 110;
   af.duration	= 25;
   af.location	= APPLY_STR;
   af.modifier  = -5;
   af.bitvector	= 0;
   affect_to_char( victim, &af );

   af.type	= skill_lookup( "plague" );
   af.level	= 110;
   af.duration	= victim->level;
   af.location  = APPLY_STR;
   af.modifier  = -5;
   af.bitvector = AFF_PLAGUE;
   affect_to_char( victim, &af );

   af.type	= skill_lookup( "curse" );
   af.level	= 110;
   af.duration	= 25;
   af.location	= APPLY_SAVES;
   af.modifier	= 15;
   af.bitvector = AFF_CURSE;
   affect_to_char( victim, &af );

   victim->hit /= 2;
   victim->mana /= 2;
	victim->move /= 2;
	}
return;
}


void do_wizhelp( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    int cmd;
    int col;
    col = 0;

if((ch->level == IMPLEMENTOR) || (ch->oldlvl == IMPLEMENTOR))
  { 
   sprintf (buf,"\n\r{CImmortal Commands for an{w: {WIMPLEMENTOR{x\n\r\n\r" ); 
   send_to_char( buf, ch ); 
  }
else if (ch->level == ASSTIMP)
  { 
   sprintf (buf,"\n\r{CImmortal Commands for an{w: {WASSISTANT IMPLEMENTOR{x\n\r\n\r" );          
   send_to_char( buf, ch );
  } 
else if (ch->level >= GGOD && ch->level <= (MAX_LEVEL - 370))
  { 
   sprintf (buf,"\n\r{CImmortal Commands for a{w: {WGREATER GOD{x\n\r\n\r" );          
   send_to_char( buf, ch );
  } 
else if (ch->level >= LGOD && ch->level < GGOD)
  { 
   sprintf (buf,"\n\r{CImmortal Commands for a{w: {WLESSER GOD{x\n\r\n\r" );          
   send_to_char( buf, ch );
  } 
else if (ch->level >= DGOD && ch->level < LGOD)
  { 
   sprintf (buf,"\n\r{CImmortal Commands for a{w: {WDEMI-GOD{x\n\r\n\r" );          
   send_to_char( buf, ch );
  } 
else if (ch->level >= VETIMM && ch->level < DGOD)
  { 
   sprintf (buf,"\n\r{CImmortal Commands for a{w: {WVETERAN IMMORTAL{x\n\r\n\r" );          
   send_to_char( buf, ch );
  } 
else if (ch->level == NEWIMM)
  { 
   sprintf (buf,"\n\r{CImmortal Commands for a{w: {WNEWBIE IMMORTAL{x\n\r\n\r" );          
   send_to_char( buf, ch );
  } 
else if (ch->level == BUILDER)
  { 
   sprintf (buf,"\n\r{CImmortal Commands for a{w: {WBUILDER{x\n\r\n\r" );          
   send_to_char( buf, ch );
  } 
else if (ch->level == IMMORTAL || ch->level == VISIMM)
  { 
   sprintf (buf,"\n\r{CImmortal Commands for a{w: {WVISITING IMMORTAL{x\n\r\n\r" );          
   send_to_char( buf, ch );
  } 

  if(ch->level>=LEVEL_IMMORTAL)
  {
    for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
         {
             if(!IS_IMP(ch))
               {
             if ( cmd_table[cmd].level >= LEVEL_HERO
             && ( cmd_table[cmd].level <= ch->level )
             &&   cmd_table[cmd].show)
               {
                sprintf( buf, "{b[{W%-10s{b]", cmd_table[cmd].name );
                send_to_char( buf, ch );
 
                if ( ++col % 6 == 0 )
                send_to_char( "{x\n\r", ch );
               }
             }
            else
               {
             if ( cmd_table[cmd].level >= LEVEL_HERO
             && ( cmd_table[cmd].level <= ch->oldlvl )
             &&   cmd_table[cmd].show)
               {
                sprintf( buf, "{b[{W%-10s{b]", cmd_table[cmd].name );
                send_to_char( buf, ch );
 
                if ( ++col % 6 == 0 )
                send_to_char( "{x\n\r", ch );
               }
             }

         }

    if ( col % 6 != 0 )
    send_to_char( "{x\n\r", ch );

       send_to_char("\n\r{BUnListed Commands{w:{x\n\r",ch);
     send_to_char("{b[{C@         {b][{Cdump      {b][{Cemboot    {b][{Cfileident {b][{Cmpdump    {b][{Cmpedit    {b]{x\n\r",ch);
     send_to_char("{b[{Cmpstat    {b][{Cobject    {b][{Cinvis     {b][{C]         {b][{Cimmhelp   {b]{x\n\r",ch);
    }
       
       send_to_char("\n\r{BEmpowered Commands{w:{x\n\r",ch);
show_granted_to_char(ch, ch);

 return;
}

int  skilltable_array_sort(const void *a, const void *b)
{
    char bufA[MAX_STRING_LENGTH];
    char bufB[MAX_STRING_LENGTH];
    
    strcpy(bufA, *(char **)a);
    strchrrep(bufA, ',', '\0');
    strcpy(bufB, *(char **)b);
    strchrrep(bufB, ',', '\0');
    
    if(atoi(bufA)<atoi(bufB))
        return -1;
    else if(atoi(bufA)==atoi(bufB))
        return 0;

    return 1;
}


void do_skilltable(CHAR_DATA *ch, char *argument)
{
    int         sn=0, i=0, count=0, total=0, averageLevel=0, averageTrain=0, numCL=0;
    char **     skills=0;
    char **     cols=0;
    char **     ar=0;
    BUFFER *    buffer;
    char        buf[MAX_STRING_LENGTH];

    buffer = new_buf();
    
    add_buf(buffer, "\n\r{WThe level and trains here are the AVERAGE across all classes.{x\n\r");
    add_buf(buffer, "{WIf there is a {C*{W by the skill, then it is actually a SPELL.{x\n\r");
    add_buf(buffer, "{WCl is the number of classes that get this spell/skill.{x\n\r");
    add_buf(buffer, "{WThe last 4 columns, Lv, Tr, QP and Gold are what is required for {YLegend's{W to purchase.{x\n\r\n\r");
    
    add_buf(buffer, "{D[{RSkill Name          {D][{RLvl{D][{RTr{D][{C*{D][{RCl{D][{RLv{D][{RTr{D][{G  QP  {D][{Y  Gold  {D]{x\n\r");
    for(sn=0;sn<MAX_SKILL && skill_table[sn].name;sn++)
    {
        for(i=0,count=0,total=0;i<MAX_CLASS;i++)
        {
            if(skill_table[sn].skill_level[i]<=100)
            {
                count++;
                total+=skill_table[sn].skill_level[i];
            }
        }
        if(count>0)
            averageLevel = (total/count);
        else
            averageLevel = 101;
        numCL = count;
        
        for(i=0,count=0,total=0;i<MAX_CLASS;i++)
        {
            if(skill_table[sn].skill_level[i]<=100)
            {
                count++;
                total+=skill_table[sn].rating[i];
            }
        }
        if(count>0)
            averageTrain = (total/count);
        else
            averageTrain = 0;
        
        sprintf(buf, "%d,%d,%s,%s,%d,%d,%d,%d,%d",
                averageLevel, averageTrain, (skill_table[sn].spell_fun==spell_null) ? "0" : "1", skill_table[sn].name, numCL,
                skill_table[sn].legendLevel, skill_table[sn].legendTrainCost, skill_table[sn].legendQPCost, skill_table[sn].legendGoldCost);
        skills = array_append(skills, buf);
    }
    
    qsort(skills, array_len(skills), sizeof(char *), skilltable_array_sort);
    
    for(ar=skills;ar && *ar;ar++)
    {
        cols = strchrexplode(*ar, ',');
        
        sprintf(buf, "{D[{W%-20s{D][{W%3d{D][{W%2d{D][{C%s{D][{W%2d{D][{W%2d{D][{W%2d{D][{G%6d{D][{Y%8d{D]{x\n\r",
                cols[3], atoi(cols[0]), atoi(cols[1]), !strcmp(cols[2], "0") ? " " : "*", atoi(cols[4]), atoi(cols[5]), atoi(cols[6]), atoi(cols[7]), atoi(cols[8]));
        add_buf(buffer, buf);
        
        cols = array_free(cols);
    }
    
    skills = array_free(skills);
    
    page_to_char(buf_string(buffer), ch);
    free_buf(buffer);
    
    return;
}

void do_fixage( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int age;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || !is_number( arg2 ) )
    {
        send_to_char( "\n\r{GSyntax{w: {WFIXAGE {c<{WCHAR NAME{c> <{WAGE{c>{x\n\r",ch );
        return;
    }
    if ( (age=atoi( arg2 )) < 17 )
      {
      send_to_char( "\n\r{RAge cannot be lower than 17.{x\n\r", ch );
      return;
      }
    if ( !(victim = get_char_world( ch, arg1 ))
    ||   IS_NPC( victim ) )
    {
        send_to_char( "\n\r{cThat player is not here.{x\n\r", ch);
        return;
    }
    victim->played =  72000 * ( age - 17 ) + ch->logon - current_time;
    return;
}  
