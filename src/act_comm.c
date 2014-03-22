#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "merc.h"
#include "interp.h"
#include "recycle.h"
#include "tables.h"
#include "lookup.h"

extern bool is_ignoring(CHAR_DATA *ch, CHAR_DATA *victim);
void talk_channel( CHAR_DATA *ch, char *argument, int channel, const char *verb );



/* RT code to delete yourself */

void do_delet( CHAR_DATA *ch, char *argument)
{
    send_to_char("You must type the full command to delete yourself.\n\r",ch);
}

void do_delete( CHAR_DATA *ch, char *argument)
{
   char strsave[MAX_INPUT_LENGTH];

   if (IS_NPC(ch))
	return;
  
if (IS_SET(ch->in_room->room_flags, ROOM_DRAGONPIT)) 
  {
send_to_char( "\n\r{CWait until after you are out of the DragonPIT!{x\n\r", ch );
  return;
  }

if ( ch->pk_timer > 0 ) 
  {
send_to_char( "\n\r{WYou must wait {R5 ticks{W before you can {RDELETE {Wafter a player fight.{x\n\r", ch );
  return;
  }


   if (ch->pcdata->confirm_delete)
   {
	if (argument[0] != '\0')
	{
	    send_to_char("Delete status removed.\n\r",ch);
	    ch->pcdata->confirm_delete = FALSE;
	    return;
	}
	else
	{
    	    sprintf( strsave, "%s%s", PLAYER_DIR, capitalize( ch->name ) );
	    wiznet("$N turns $Mself into line noise.",ch,NULL,0,0,0);
	    stop_fighting(ch,TRUE);

           if ( ch->clan != 0)
              {
               DECLARE_DO_FUN( do_guild );
               do_guild( ch, "self none" );
              } 
	    do_function(ch, &do_quit, "");
	    unlink(strsave);
	    return;
 	}
    }

    if (argument[0] != '\0')
    {
	send_to_char("Just type delete. No argument.\n\r",ch);
	return;
    }

    send_to_char("Type delete again to confirm this command.\n\r",ch);
    send_to_char("WARNING: this command is irreversible.\n\r",ch);
    send_to_char("Typing delete with an argument will undo delete status.\n\r",
	ch);
    ch->pcdata->confirm_delete = TRUE;
    wiznet("$N is contemplating deletion.",ch,NULL,0,0,ch->level);
  }


void do_choose( CHAR_DATA *ch, char *argument)
{
   if (IS_NPC(ch))
	return;
  
      if (ch->level < 20)
       {
         send_to_char("\n\r{WThis is not available until you are at least 20th level.{x\n\r",ch);
         return;
        }  

       
      if (ch->level > 40)
        {
          send_to_char("\n\r{WThis is not available after the 40th level.{x\n\r",ch);
          return;
         }  

 if (IS_SET(ch->pact, PLR_PKILLER))
   {
  send_to_char("\n\r{WYou have already CHOOSEN, there is no going back now!{x\n\r",ch);
  return;
   }  


   if (ch->pcdata->confirm_choose)
   {
	if (argument[0] != '\0')
	{
	    send_to_char("\n\r{WCHOOSE status removed.{x\n\r",ch);
	    ch->pcdata->confirm_choose = FALSE;
	    return;
	}
	else
	{
    SET_BIT (ch->pact, PLR_PKILLER);
    send_to_char("\n\r{RYou have choosen to become a PKILLER.{x\n\r",ch);
    send_to_char("{RYou can now engage other players who have choosen.{x\n\r",ch);
    send_to_char("{RGood luck and Good Hunting!{x\n\r",ch);
	    return;
 	}
    }

    if (argument[0] != '\0')
    {
	send_to_char("\n\r{WJust type choose. No argument.{x\n\r",ch);
	return;
    }


    send_to_char("\n\r{WType choose again to confirm this command.{x\n\r",ch);
    send_to_char("{RWARNING: this command is irreversible.{x\n\r",ch);
    send_to_char("{WTyping choose with an argument will undo choose status.{x\n\r",ch);
    ch->pcdata->confirm_choose = TRUE;
}


void do_channels( CHAR_DATA *ch, char *argument)
{
/*    char buf[MAX_STRING_LENGTH]; */

    send_to_char("\n\r{CChannel        {WStatus{x\n\r",ch);
    send_to_char("{b---------------------{x\n\r",ch);
   
  if(is_owner(ch))
   {
     send_to_char("{cOWNER{x            ",ch);
      if ( !IS_SET( ch->deaf, CHANNEL_OWNER ) )
        send_to_char("{gON{x\n\r",ch);
      else
        send_to_char("{rOFF{x\n\r",ch);
    }

  if (ch->level >= 499)  
    {
     send_to_char("{cIMP{x              ",ch);
      if ( !IS_SET( ch->deaf, CHANNEL_IMP ) )
        send_to_char("{gON{x\n\r",ch);
      else
        send_to_char("{rOFF{x\n\r",ch);
    }

     if (IS_IMMORTAL(ch) || granted(ch, "immtalk"))
       {
        send_to_char("{cIMM{x              ",ch);
         if( !IS_SET( ch->deaf, CHANNEL_IMMTALK ) )
           send_to_char("{gON{x\n\r",ch);
         else
           send_to_char("{rOFF{x\n\r",ch);
       }

  if (IS_IMMORTAL(ch) || granted(ch, "builder"))
    {
      send_to_char("{cBUILDER{x          ",ch);
       if( !IS_SET( ch->deaf, CHANNEL_BUILDER ) )
         send_to_char("{gON{x\n\r",ch);
       else
         send_to_char("{rOFF{x\n\r",ch);
    }

    if (ch->clan > 0 )
    {
      send_to_char("{cClan{x             ",ch);
      if( !IS_SET( ch->deaf, CHANNEL_CLAN ) )
        send_to_char("{gON{x\n\r",ch);
      else
        send_to_char("{rOFF{x\n\r",ch);

      send_to_char("{cCWar{x             ",ch);
      if( !IS_SET( ch->deaf, CHANNEL_CWAR ) )
        send_to_char("{gON{x\n\r",ch);
      else
        send_to_char("{rOFF{x\n\r",ch);
    }

    if (ch->level >= 100 )
    {
    send_to_char("{cHERO{x          ",ch);
    if ( !IS_SET( ch->deaf, CHANNEL_HERO ) )
      send_to_char("{gON{x\n\r",ch);
    else
      send_to_char("{rOFF{x\n\r",ch);
    } 

    send_to_char("{cIMMHELP{x          ",ch);
    if ( !IS_SET( ch->deaf, CHANNEL_IMMHELP ) )
      send_to_char("{gON{x\n\r",ch);
    else
      send_to_char("{rOFF{x\n\r",ch);

    send_to_char("{cRolePlay{x         ",ch);
    if ( !IS_SET( ch->deaf, CHANNEL_RP ) )
      send_to_char("{gON{x\n\r",ch);
    else
      send_to_char("{rOFF{x\n\r",ch);

    send_to_char("{cChat{x             ",ch);
    if ( !IS_SET( ch->deaf, CHANNEL_CHAT ) )
      send_to_char("{gON{x\n\r",ch);
    else
      send_to_char("{rOFF{x\n\r",ch);

    send_to_char("{cGrats{x             ",ch);
    if ( !IS_SET( ch->deaf, CHANNEL_GRAT ) )
      send_to_char("{gON{x\n\r",ch);
    else
      send_to_char("{rOFF{x\n\r",ch);
    
    
    send_to_char("{cTells{x            ",ch);
    if (!IS_SET(ch->comm,COMM_DEAF))
        send_to_char("{gON{x\n\r",ch);
    else
        send_to_char("{rOFF{x\n\r",ch);

    send_to_char("{cVent{x             ",ch);
    if ( !IS_SET( ch->deaf, CHANNEL_VENT ) )
      send_to_char("{gON{x\n\r",ch);
    else
      send_to_char("{rOFF{x\n\r",ch);

    send_to_char("{cAuction{x          ",ch);
    if ( !IS_SET( ch->deaf, CHANNEL_AUCTION ) )
        send_to_char("{gON{x\n\r",ch);
    else
        send_to_char("{rOFF{x\n\r",ch);

	//////////////////////////////////////////////////////////////////////////
	// Acrophobia Stuff
    send_to_char("{cAcrophobia{x       ",ch);
    if ( !IS_SET( ch->deaf, CHANNEL_ACRO ) )
        send_to_char("{gON{x\n\r",ch);
    else
        send_to_char("{rOFF{x\n\r",ch);

    send_to_char("{cYells{x            ",ch);
    if ( !IS_SET( ch->deaf, CHANNEL_YELL ) )
      send_to_char("{gON{x\n\r",ch);
    else
      send_to_char("{rOFF{x\n\r",ch);

    send_to_char("{cDragonPIT{x        ",ch);
    if (!IS_SET(ch->deaf,CHANNEL_DPTALK))
      send_to_char("{gON{x\n\r", ch);
    else
      send_to_char("{rOFF{x\n\r", ch);
      
    if(ch->race == race_lookup("Nephilim"))
    {
        send_to_char("{cNephilim{x         ",ch);
        if (!IS_SET(ch->deaf,CHANNEL_NEPHILIM))
          send_to_char("{gON{x\n\r", ch);
        else
          send_to_char("{rOFF{x\n\r", ch);
    }
    
    send_to_char("{b---------------------{x\n\r",ch);


/*
    if (IS_SET(ch->comm,COMM_SNOOP_PROOF))
        send_to_char("{cYou are {WIMMUNE{c to snooping.{x\n\r",ch);

    if (ch->lines != PAGELEN)
    {
        if (ch->lines)
        {
            sprintf(buf,"{cYou display {W%d{c lines of scroll.{x\n\r",ch->lines+2);
            send_to_char(buf,ch);
        }
        else
            send_to_char("{cScroll Buffering {rOFF{x\n\r",ch);
    }

    send_to_char("{cQuiet mode{x       ",ch);
    if (IS_SET(ch->comm,COMM_QUIET))
      send_to_char("{gON{x\n\r",ch);
    else
      send_to_char("{rOFF{x\n\r",ch);

    if (IS_SET(ch->comm,COMM_AFK))
        send_to_char("{WYou are {RAFK{W.{x  \n\r",ch);

    if (ch->prompt != NULL)
    {
      sprintf(buf,"\n\r{cYour current prompt is{W:{x %s{x\n\r",ch->prompt);
      send_to_char(buf,ch);
    }

    if (!IS_NPC(ch))
      {
    if (IS_SET(ch->comm, COMM_COMBAT))
        send_to_char("{CCOMBAT {cinformation is {gON{x\n\r",ch);
      else
        send_to_char("{CCOMBAT {cinformation is {rOFF{x\n\r",ch);
      }


    if (IS_SET(ch->comm,COMM_NOTELL))
      send_to_char("You cannot use tell.\n\r",ch);

    if (IS_SET(ch->comm,COMM_NOCHANNELS))
     send_to_char("You cannot use channels.\n\r",ch);

    if (IS_SET(ch->comm,COMM_NOEMOTE))
      send_to_char("You cannot show emotions.\n\r",ch);
  
    if (IS_SET(ch->comm,COMM_DPITKILL))
      send_to_char("\n\r{RYou are unable to use DPTALK or DRAGONPIT while your DragonPIT Privlages are revoked..{x\n\r",ch);
   */

    send_to_char("{cSee '{WHELP CHANNELS{c' for more information...{x\n\r", ch); 




 return;
}

	    
/* RT deaf blocks out all shouts */

void do_deaf( CHAR_DATA *ch, char *argument)
{
    
   if (IS_SET(ch->comm,COMM_DEAF))
   {
     send_to_char("\n\r{CYou can now hear tells again.{x\n\r",ch);
     REMOVE_BIT(ch->comm,COMM_DEAF);
   }
   else 
   {
     send_to_char("\n\r{RFrom now on, you won't hear tells.{x\n\r",ch);
     SET_BIT(ch->comm,COMM_DEAF);
   }
}

/* RT quiet blocks out all communication */

void do_quiet ( CHAR_DATA *ch, char * argument)
{
    if (IS_SET(ch->comm,COMM_QUIET))
    {
      send_to_char("Quiet mode removed.\n\r",ch);
      REMOVE_BIT(ch->comm,COMM_QUIET);
    }
   else
   {
     send_to_char("From now on, you will only hear says and emotes.\n\r",ch);
     SET_BIT(ch->comm,COMM_QUIET);
   }
}


void do_combat ( CHAR_DATA *ch, char * argument)
{
    if (!IS_NPC(ch))
      {
    if (IS_SET(ch->comm,COMM_COMBAT))
    {
      send_to_char("\n\r{cYou {RDISABLE {cextra combat information.{x\n\r",ch);
      REMOVE_BIT(ch->comm,COMM_COMBAT);
    }
   else
   {
     send_to_char("\n\r{cYou {GENABLE {cextra combat information.{x\n\r",ch);
     SET_BIT(ch->comm,COMM_COMBAT);
   }
     }
   else
     send_to_char("\n\r{cHuh?{x\n\r",ch);
}

/* afk command */

void do_afk ( CHAR_DATA *ch, char * argument)
{
 if(!IS_NPC(ch))
   {
    if (IS_SET(ch->comm,COMM_AFK))
    {
      send_to_char("AFK mode removed. Type 'replay' to see tells.\n\r",ch);
      REMOVE_BIT(ch->comm,COMM_AFK);
    }
   else
   {
     send_to_char("You are now in AFK mode.\n\r",ch);
     SET_BIT(ch->comm,COMM_AFK);
   }
 }
}

void do_clanwhoshow ( CHAR_DATA *ch, char * argument)
{
 if(!IS_NPC(ch))
   {
    if (IS_SET(ch->comm,COMM_CLANSHOW))
    {
      send_to_char("\n\r{CYou will NO LONGER show your CLAN NAME on the WHO list.\n\r",ch);
      REMOVE_BIT(ch->comm,COMM_CLANSHOW);
    }
   else
   {
     send_to_char("\n\r{CYou will now show your CLAN NAME on the WHO list.\n\r",ch);
     SET_BIT(ch->comm,COMM_CLANSHOW);
   }
 }
}


void do_replay (CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
    {
	send_to_char("You can't replay.\n\r",ch);
	return;
    }

    if (buf_string(ch->pcdata->buffer)[0] == '\0')
    {
	send_to_char("You have no tells to replay.\n\r",ch);
	return;
    }

    page_to_char(buf_string(ch->pcdata->buffer),ch);
    clear_buf(ch->pcdata->buffer);
}

/* _ALL_ channels recoded --ENVY-- style. -- Rage */


void do_dptalk(CHAR_DATA *ch, char *argument )
{
  char buf[MAX_STRING_LENGTH];

  if (!IS_SET(ch->comm,COMM_DPITKILL))
    {

if ( argument[0] == '\0' )
    {
    if ( IS_SET( ch->deaf, CHANNEL_DPTALK ) )
      {
      REMOVE_BIT( ch->deaf, CHANNEL_DPTALK );
      sprintf( buf, "\n\r{wDragonPIT (DPTALK) Channel is now {gON{w.\n\r{x");
      send_to_char( buf, ch );
      }
    else
      {
       if (!IS_SET(ch->pact, PLR_DRAGONPIT))  
         {
          SET_BIT( ch->deaf, CHANNEL_DPTALK );
          send_to_char("\n\r{wDragonPIT (DPTALK) Channel is now {rOFF{w.\n\r{x",ch);
          return;
         }
       else
         {
          send_to_char("\n\r{GDPTALK cannot be turned off if you are in an DragonPIT.\n\r{x",ch);
          return;
         }
      }
    }
  else
   {
  if(IS_SET(ch->in_room->room_flags, ROOM_DRAGONPIT )
  || (IS_IMP(ch)))
    {
     talk_channel( ch, argument, CHANNEL_DPTALK, "dptalk" );
     return;
    }
   else
    {
     send_to_char( "\n\r{RYou must be in a DragonPIT to use this Channel{x.\n\r", ch );
     return;
    }
   }
  }
   else
    {
     send_to_char( "\n\r{RYour DRAGONPIT and DPTALK privileges were revoked.{x.\n\r", ch);
     return;
    }


 return;
}


void do_imp( CHAR_DATA *ch, char *argument )
{
  if (ch->level >= 499)  
    {
    talk_channel( ch, argument, CHANNEL_IMP, "imp" );
    return;
    }
}

void do_owner( CHAR_DATA *ch, char *argument )
{
  if (is_owner(ch))  
    {
    talk_channel( ch, argument, CHANNEL_OWNER, "owner" );
    return;
    }
}




void do_immhelp( CHAR_DATA *ch, char *argument )
{
    if ( ch->level < 3 )
      send_to_char( "\n\r{WThis channel is not available until level 3.{x\n\r", ch);
    else
      talk_channel( ch, argument, CHANNEL_IMMHELP, "immhelp" );

    return;
}


void do_rp( CHAR_DATA *ch, char *argument )
{
 if (!IS_NPC(ch))
   {
    if ( ch->level < 5 )
      send_to_char( "\n\r{WThis channel is not available until level 5.{x\n\r", ch);
    else
      talk_channel( ch, argument, CHANNEL_RP, "rp" );
    return;
   }
 return;
}

void do_gossip( CHAR_DATA *ch, char *argument )
{
 if (!IS_NPC(ch))
   {
    if ( ch->level >= 1
    ||   argument[0] == '\0' )
      talk_channel( ch, argument, CHANNEL_CHAT, "chat" );
    else
      send_to_char( "You may not chat before level 1.\n\r", ch );
    return;
 }
return;
}

void do_hero( CHAR_DATA *ch, char *argument )
{
 if (!IS_NPC(ch))
   {
    if ( (ch->level >= 100 || ch->pcdata->oldcl != -1)
    ||   argument[0] == '\0' )
      talk_channel( ch, argument, CHANNEL_HERO, "hero" );
    else
      send_to_char( "You may not HeroTalk before level 100.\n\r", ch );
    return;
 }
return;
}



void do_grats( CHAR_DATA *ch, char *argument )
{
 if (!IS_NPC(ch))
   {
    if ( ch->level >= 1
    ||   argument[0] == '\0' )
      talk_channel( ch, argument, CHANNEL_GRAT, "grats" );
    else
      send_to_char( "You may not Grats before level 1.\n\r", ch );
    return;
 }
return;
}

void do_vent(CHAR_DATA *ch, char *argument )
{
 if (!IS_NPC(ch))
   {
    if ( ch->level < 10 )
      {
      send_to_char( "\n\r{WYou may not {RVENT{W before level 10.{x\n\r",ch);
      SET_BIT( ch->deaf, CHANNEL_VENT );
      }
    else
      talk_channel( ch, argument, CHANNEL_VENT, "vent" );
    return;

  }
return;
}

void do_clantalk( CHAR_DATA *ch, char *argument )
{
    if (!IS_SET(ch->comm,COMM_NOCHANNELS))
      {
    if(!IS_IMP(ch))
      {
    if (( ch->clan > 0)
    ||   argument[0] == '\0' )
      talk_channel( ch, argument, CHANNEL_CLAN, "clan" );
     else
      send_to_char( "\n\r{GClanTalk is only available to Clan Members.{x\n\r", ch );
      }
    else
      {
    if (( ch->ctimp_clan > 0)
    ||   argument[0] == '\0' )
      talk_channel( ch, argument, CHANNEL_CLAN, "clan" );
      }

    return;
      }
}

void do_cwar( CHAR_DATA *ch, char *argument )
{
    if (!IS_SET(ch->comm,COMM_NOCHANNELS))
      {
 
    if (ch->clan == 8)
      {
       send_to_char( "\n\r{CClan {gLegacy {CMembers have no reason to use WAR channels since you are NON-PK...\n\r", ch );
       return;
      }

    if (( ch->clan > 0)
    ||   argument[0] == '\0' )
      talk_channel( ch, argument, CHANNEL_CWAR, "cwar" );
    else
      send_to_char( "\n\r{GClanWar is only available to Clan Members.{x\n\r", ch );
    return;
      }
}
/*terror*/


void do_immtalk( CHAR_DATA *ch, char *argument )
{
     talk_channel( ch, argument, CHANNEL_IMMTALK, "immtalk" );
     return;
}

void do_builder( CHAR_DATA *ch, char *argument )
{
     talk_channel( ch, argument, CHANNEL_BUILDER, "builder" );
	return;
}

void do_yell( CHAR_DATA *ch, char *argument )
{
  if(!IS_NPC(ch))
   { 
    if ( ch->level >= 3
    ||   argument[0] == '\0' )
      {

       if (ch->in_room->clanowner > 0)
         {   
          send_to_char( "\n\r{RUse CTALK to talk to your other clannies in your home.{x\n\r", ch );
          return;
         }

       if (IS_SET(ch->comm,COMM_NOCHANNELS))
        send_to_char( "You can't yell.\n\r", ch );
       else
        talk_channel( ch, argument, CHANNEL_YELL, "yell" );
      }
    else
      send_to_char( "You must be level 3 to yell.\n\r", ch );
    return;
   }
}

void do_ntalk( CHAR_DATA *ch, char *argument )
{
  if(((!IS_NPC(ch)) && ( ch->race == RACE_NEPHILIM)) || IS_IMP(ch))
   { 
       if (IS_SET(ch->comm,COMM_NOCHANNELS))
        send_to_char( "You can't talk on nephilim.\n\r", ch );
       else
        talk_channel( ch, argument, CHANNEL_NEPHILIM, "nephilim" );
   }
}

void do_nephilim( CHAR_DATA *ch, char *argument)
  {
  CHAR_DATA *och;

 if ((ch->race != RACE_NEPHILIM)
 && (!IS_IMP(ch)))
  {
  send_to_char("\n\r{RYou don't know how to speak in the Nephilim Tongue!{x\n\r",ch);
  return;
  }
 
  if(argument[0] == '\0')
  {
  send_to_char("\n\r{RSay WHAT in the Nephilim Tongue?{x\n\r",ch);
  return;
  }
   
        if((ch->race == RACE_NEPHILIM)
        || (IS_IMP(ch)))
          {
    act("\n\r{YIn Nephilian, you say '$t'{x",ch,argument,NULL,TO_CHAR);
    
     for(och = ch->in_room->people; och != NULL; och = och->next_in_room )
       {
        if ((och->race != RACE_NEPHILIM)
        && (!IS_IMP(och)))
          {
           act("\n\r{Y$n says something in a strange tongue.{x",ch,NULL,och,TO_VICT);
          }
  
        if((och->race == RACE_NEPHILIM)
        || (IS_IMP(och)))
          {
           act("\n\r{YIn Nephilian, $n says '$t'{x",ch,argument,och,TO_VICT);
          }
       }
   }
 return;
}

void do_say( CHAR_DATA *ch, char *argument )
{
    if ( argument[0] == '\0' )
    {
	send_to_char( "Say what?\n\r", ch );
	return;
    }


    if (IS_SET(ch->comm,COMM_NOCHANNELS)
    && (!IS_NPC(ch)))
    {
        send_to_char("You open your mouth, but nothing comes out.\n\r",ch);
        return;
    }

	/* Make the words drunk if needed */
	if (!IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK] > 10)
	argument = makedrunk(argument,ch);

	if (IS_SET(ch->comm,COMM_DUMMY))
		argument = makedummy(argument, ch);
		
		fix_player_text(ch, argument);

    act( "\n\r{Y$n says '$T'{x", ch, NULL, argument, TO_ROOM );
    act( "\n\r{YYou say '$T'{x", ch, NULL, argument, TO_CHAR );

    if ( !IS_NPC(ch) )
    {
	CHAR_DATA *mob, *mob_next;
	for ( mob = ch->in_room->people; mob != NULL; mob = mob_next )
	{
	    mob_next = mob->next_in_room;
	    if ( IS_NPC(mob) && HAS_TRIGGER( mob, TRIG_SPEECH )
	    &&   mob->position == mob->pIndexData->default_pos )
		mp_act_trigger( argument, mob, ch, NULL, NULL, TRIG_SPEECH );
	}
    }
    return;
}




void do_tell( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    if (IS_NPC(ch))
      return;

    if ( IS_SET(ch->comm, COMM_NOTELL) || IS_SET(ch->comm,COMM_DEAF))
    {
	send_to_char( "Your message didn't get through.\n\r", ch );
	return;
    }

    if ( IS_SET(ch->comm, COMM_QUIET) )
    {
	send_to_char( "You must turn off quiet mode first.\n\r", ch);
	return;
    }

    if (IS_SET(ch->comm,COMM_DEAF))
    {
	send_to_char("You must turn off deaf mode first.\n\r",ch);
	return;
    }

    if (IS_SET(ch->comm,COMM_NOCHANNELS)
    && (!IS_NPC(ch)))
      {
        send_to_char("\n\r{RYou open your mouth to speak but nothing comes out{x.\n\r",ch);
        return;
      }

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "Tell whom what?\n\r", ch );
	return;
    }

    /*
     * Can tell to PC's anywhere, but NPC's only in same room.
     * -- Furey
     */
    if ( ( victim = get_char_world( ch, arg ) ) == NULL
    || ( IS_NPC(victim) && victim->in_room != ch->in_room ) )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if (IS_NPC(victim))
    {
	return;
    }

    if ( victim->desc == NULL && !IS_NPC(victim))
    {
	act("$N seems to have misplaced $S link...try again later.",
	    ch,NULL,victim,TO_CHAR);
        sprintf(buf,"%s tells you '%s'\n\r",PERS(ch,victim),argument);
        buf[0] = UPPER(buf[0]);
        add_buf(victim->pcdata->buffer,buf);
	return;
    }

    if ( !(IS_IMMORTAL(ch) && ch->level > LEVEL_IMMORTAL) && !IS_AWAKE(victim) )
    {
	act( "$E can't hear you.", ch, 0, victim, TO_CHAR );
	return;
    }
if(IS_SET(ch->comm, COMM_DUMMY))
  argument = makedummy(argument, ch);  

    if (IS_SET(victim->comm,COMM_QUIET) 
    || IS_SET(victim->comm,COMM_DEAF))
    {
      if (!IS_IMMORTAL(ch))
        {
 	 act( "$E is not receiving tells.", ch, 0, victim, TO_CHAR );
   	 return;
        }
      else
      if (ch->level < victim->level)
        {
    	 act( "$E is not receiving tells.", ch, 0, victim, TO_CHAR );
   	 return;
        } 
    }

/*
  if (is_ignoring(victim, ch) && !IS_IMMORTAL(ch)) 
  {
    sprintf(buf, "%s is ignoring you.\n\r", victim->name);
    send_to_char(buf, ch);
    return;
  }
*/

    if (IS_SET(victim->comm,COMM_AFK))
    {
	if (IS_NPC(victim))
	{
	    act("$E is AFK, and not receiving tells.",ch,NULL,victim,TO_CHAR);
	    return;
	}

	act("$E is AFK, but your tell will go through when $E returns.",
	    ch,NULL,victim,TO_CHAR);
	sprintf(buf,"{Y%s tells you '%s'\n\r{x",PERS(ch,victim),argument);
	buf[0] = UPPER(buf[0]);
	add_buf(victim->pcdata->buffer,buf);
	return;
    }

    act( "\n\r{YYou tell $N '$t'{x", ch, argument, victim, TO_CHAR );
    act_new("\n\r{Y$n tells you '$t'{x",ch,argument,victim,TO_VICT,POS_DEAD);
    victim->reply	= ch;

    if ( !IS_NPC(ch) && IS_NPC(victim) && HAS_TRIGGER(victim,TRIG_SPEECH) )
	mp_act_trigger( argument, victim, ch, NULL, NULL, TRIG_SPEECH );

    return;

}



void do_reply( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char buf[MAX_STRING_LENGTH];

    if ( IS_SET(ch->comm, COMM_NOTELL) )
    {
	send_to_char( "Your message didn't get through.\n\r", ch );
	return;
    }

    if ( ( victim = ch->reply ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim->desc == NULL && !IS_NPC(victim))
    {
        act("$N seems to have misplaced $S link...try again later.",
            ch,NULL,victim,TO_CHAR);
        sprintf(buf,"{Y%s tells you '%s'\n\r{x",PERS(ch,victim),argument);
        buf[0] = UPPER(buf[0]);
        add_buf(victim->pcdata->buffer,buf);
        return;
    }

    if ( !IS_IMMORTAL(ch) && !IS_AWAKE(victim) )
    {
	act( "$E can't hear you.", ch, 0, victim, TO_CHAR );
	return;
    }

    if ((IS_SET(victim->comm,COMM_QUIET) || IS_SET(victim->comm,COMM_DEAF))
    &&  !IS_IMMORTAL(ch) && !IS_IMMORTAL(victim))
    {
        act_new( "$E is not receiving tells.", ch, 0, victim, TO_CHAR,POS_DEAD);
        return;
    }

    if (!IS_IMMORTAL(victim) && !IS_AWAKE(ch))
    {
	send_to_char( "In your dreams, or what?\n\r", ch );
	return;
    }

    if (IS_SET(victim->comm,COMM_AFK))
    {
        if (IS_NPC(victim))
        {
            act_new("$E is AFK, and not receiving tells.",
		ch,NULL,victim,TO_CHAR,POS_DEAD);
            return;
        }
 
        act_new("$E is AFK, but your tell will go through when $E returns.",
            ch,NULL,victim,TO_CHAR,POS_DEAD);
        sprintf(buf,"{Y%s tells you '%s'\n\r{x",PERS(ch,victim),argument);
	buf[0] = UPPER(buf[0]);
        add_buf(victim->pcdata->buffer,buf);
        return;
    }

    act_new("{YYou tell $N '$t'{x",ch,argument,victim,TO_CHAR,POS_DEAD);
    act_new("{Y$n tells you '$t'{x",ch,argument,victim,TO_VICT,POS_DEAD);
    victim->reply	= ch;

    return;
}

void do_emote( CHAR_DATA *ch, char *argument )
{
    if ( !IS_NPC(ch) && IS_SET(ch->comm, COMM_NOEMOTE) )
    {
        send_to_char( "You can't show your emotions.\n\r", ch );
        return;
    }
 
    if (IS_SET(ch->comm,COMM_NOCHANNELS))
    {
        send_to_char( "\n\r{RYou can't use {WEMOTE{R until your channels are restored.{x\n\r", ch );
        return;
    }


    if ( argument[0] == '\0' )
    {
        send_to_char( "Emote what?\n\r", ch );
        return;
    }
 
    MOBtrigger = FALSE;
    act( "{m$n $T{x", ch, NULL, argument, TO_ROOM );
    act( "{m$n $T{x", ch, NULL, argument, TO_CHAR );
    MOBtrigger = TRUE;
    return;
}

void do_qui( CHAR_DATA *ch, char *argument )
{
    send_to_char( "If you want to QUIT, you have to spell it out.\n\r", ch );
    return;
}

void do_quit( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d,*d_next;
    int id;
    CHAR_DATA * original;
    char buf[MAX_STRING_LENGTH];

    if ( IS_NPC(ch) )
	return;

if (IS_SET(ch->in_room->room_flags, ROOM_DRAGONPIT)) 
  {
send_to_char( "\n\r{CWait until after you are out of the DragonPIT!{x\n\r", ch );
  return;
  }


if ( ch->pk_timer > 0 ) 
  {
send_to_char( "\n\r{WYou must wait {R5 ticks{W before you can QUIT after a player fight.{x\n\r", ch );
  return;
  }
  else 
   if ( ch->position == POS_FIGHTING )
    {
	send_to_char( "\n\r{RNo way! You are fighting!{x\n\r", ch );
	return;
    }

    if ( ch->position  < POS_STUNNED  )
    {
	send_to_char( "\n\r{RYou're not DEAD yet!{x\n\r", ch );
	return;
    }


    if ( auction->item != NULL && ((ch == auction->buyer) || (ch == auction->seller)) )
    {
        send_to_char ("\n\r{RWait till you have sold/bought the item on auction!{x\n\r",ch);
        return;
    }
    
  if(ch->invis_level <= 100 && ch->incog_level <= 100)
  {
    sprintf(buf, "{WIt appears {G%s {Wcould no longer stand the {rheat.{x\n\r", ch->name);
	for(d=descriptor_list;d;d=d->next)
	{
		original = d->original ? d->original : d->character; /* if switched */
		if(d->connected==CON_PLAYING)
		  act_new(buf, original, NULL, NULL, TO_CHAR, POS_DEAD);
	}
  }

    act( "\n\r{W$n {chas left the Red Dragon.{x\n\r", ch, NULL, NULL, TO_ROOM );
    sprintf( log_buf, "%s has quit.", ch->name );
    log_string( log_buf );
     wiznet("\n\r{W$N {crejoins the real world.{x\n\r",ch,NULL,WIZ_LOGINS,0,ch->level);

    /*
     * After extract_char the ch is no longer valid!
     */

    if ( IS_SET( ch->in_room->room_flags, ROOM_PRIVATE ) 
    && !IS_SET( ch->pact, PLR_JAIL )
    && ch->level < LEVEL_IMMORTAL )
	{
	char_from_room( ch );
	char_to_room( ch, get_room_index( ROOM_VNUM_TEMPLE ) );
	}

    save_char_obj( ch );

    /* Free note that might be there somehow */
    if (ch->pcdata->in_progress)
	free_note (ch->pcdata->in_progress);
    
    id = ch->id;
    d = ch->desc;
    extract_char( ch, TRUE );
    if ( d != NULL )
    close_socket( d );

    /* toast evil cheating bastards */
    for (d = descriptor_list; d != NULL; d = d_next)
    {
	CHAR_DATA *tch;

	d_next = d->next;
	tch = d->original ? d->original : d->character;
	if (tch && tch->id == id)
	{
	    extract_char(tch,TRUE);
	    close_socket(d);
	} 
    }
    
    crn_players--;
    return;
}



void do_save( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC(ch) )
	return;

    save_char_obj( ch );
    send_to_char("\n\r{CSAVING PFile...{x\n\r", ch);
    return;
}



void do_follow( CHAR_DATA *ch, char *argument )
{
/* RT changed to allow unlimited following and follow the NOFOLLOW rules */
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Follow whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master != NULL )
    {
	act( "But you'd rather follow $N!", ch, NULL, ch->master, TO_CHAR );
	return;
    }

    if ( victim == ch )
    {
	if ( ch->master == NULL )
	{
	    send_to_char( "You already follow yourself.\n\r", ch );
	    return;
	}
	stop_follower(ch);
	return;
    }

    if (!IS_NPC(victim) && IS_SET(victim->pact,PLR_NOFOLLOW) && !IS_IMMORTAL(ch))
    {
	act("$N doesn't seem to want any followers.\n\r",
             ch,NULL,victim, TO_CHAR);
        return;
    }

      if (IS_SET(ch->in_room->room_flags, ROOM_DRAGONPIT) || !IS_SET(ch->pact, PLR_DRAGONPIT))
        {
    REMOVE_BIT(ch->pact,PLR_NOFOLLOW);

    if ( ch->master != NULL )
	stop_follower( ch );

    add_follower( ch, victim );
    return;
        }
     else
      {
	send_to_char( "\n\r{RNot while you are in an DRAGONPIT!{x\n\r", ch );
	return;
      }

}


void add_follower( CHAR_DATA *ch, CHAR_DATA *master )
{
    if ( ch->master != NULL )
    {
	bug( "Add_follower: non-null master.", 0 );
	return;
    }

    ch->master        = master;
    ch->leader        = NULL;

    if ( can_see( master, ch ) )
	act( "$n now follows you.", ch, NULL, master, TO_VICT );

    act( "You now follow $N.",  ch, NULL, master, TO_CHAR );

    return;
}



void stop_follower( CHAR_DATA *ch )
{
    if ( ch->master == NULL )
    {
	bug( "Stop_follower: null master.", 0 );
	return;
    }

    if ( IS_AFFECTED(ch, AFF_CHARM) )
    {
	REMOVE_BIT( ch->affected_by, AFF_CHARM );
	affect_strip( ch, gsn_charm_person );
    }

    if ( can_see( ch->master, ch ) && ch->in_room != NULL)
    {
	act( "$n stops following you.",     ch, NULL, ch->master, TO_VICT    );
    	act( "You stop following $N.",      ch, NULL, ch->master, TO_CHAR    );
    }
    if (ch->master->pet == ch)
	ch->master->pet = NULL;

    ch->master = NULL;
    ch->leader = NULL;
    return;
}

/* nukes charmed monsters and pets */
void nuke_pets( CHAR_DATA *ch )
{    
    CHAR_DATA *pet;

    if ((pet = ch->pet) != NULL)
    {
    	stop_follower(pet);
    	if (pet->in_room != NULL)
    	    act("$N slowly fades away.",ch,NULL,pet,TO_NOTVICT);
    	extract_char(pet,TRUE);
    }
    ch->pet = NULL;

    return;
}



void die_follower( CHAR_DATA *ch )
{
    CHAR_DATA *fch;

    if ( ch->master != NULL )
    {
    	if (ch->master->pet == ch)
    	    ch->master->pet = NULL;
	stop_follower( ch );
    }

    ch->leader = NULL;

    for ( fch = char_list; fch != NULL; fch = fch->next )
    {
	if ( fch->master == ch )
	    stop_follower( fch );
	if ( fch->leader == ch )
	    fch->leader = fch;
    }

    return;
}



void do_order( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH],arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *och;
    CHAR_DATA *och_next;
    bool found;
    bool fAll;

    argument = one_argument( argument, arg );
    one_argument(argument,arg2);

    if (!str_cmp(arg2,"delete") || !str_cmp(arg2,"mob"))
    {
        send_to_char("That will NOT be done.\n\r",ch);
        return;
    }

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "Order whom to do what?\n\r", ch );
	return;
    }

    if ( IS_AFFECTED( ch, AFF_CHARM ) )
    {
	send_to_char( "You feel like taking, not giving, orders.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
	fAll   = TRUE;
	victim = NULL;
    }
    else
    {
	fAll   = FALSE;
	if ( ( victim = get_char_room( ch, arg ) ) == NULL )
	{
	    send_to_char( "They aren't here.\n\r", ch );
	    return;
	}

	if ( victim == ch )
	{
	    send_to_char( "Aye aye, right away!\n\r", ch );
	    return;
	}

	if (!IS_AFFECTED(victim, AFF_CHARM) || victim->master != ch 
	||  (IS_IMMORTAL(victim) && victim->level > ch->level))
	{
	    send_to_char( "Do it yourself!\n\r", ch );
	    return;
	}
    }

    found = FALSE;
    for ( och = ch->in_room->people; och != NULL; och = och_next )
    {
	och_next = och->next_in_room;

	if ( IS_AFFECTED(och, AFF_CHARM)
	&&   och->master == ch
	&& ( fAll || och == victim ) )
	{
	    found = TRUE;
	    sprintf( buf, "$n orders you to '%s'.", argument );
	    act( buf, ch, NULL, och, TO_VICT );
	    interpret( och, argument );
	}
    }

    if ( found )
    {
	WAIT_STATE(ch,PULSE_VIOLENCE);
	send_to_char( "Ok.\n\r", ch );
    }
    else
	send_to_char( "You have no followers here.\n\r", ch );
    return;
}



void do_group( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

      if (!IS_SET(ch->pact, PLR_DRAGONPIT))
        {
    if ( arg[0] == '\0' )
    {
	CHAR_DATA *gch;
	CHAR_DATA *leader;

	leader = (ch->leader != NULL) ? ch->leader : ch;
	sprintf( buf, "\n\r{C%s's group{w:{x\n\r", PERS(leader, ch) );
	send_to_char( buf, ch );


	for ( gch = char_list; gch != NULL; gch = gch->next )
	{
	    if ( is_same_group( gch, ch ) )
	    {
		sprintf( buf,
		"{c[{W%3d %3.3s{c] %-15.15s {cHP{w:{W%5d{c({C%-5d{c) {cMANA{w:{W%5d{c({C%-5d{c) {cMV{w:{W%5d{c({C%-5d{c)\n\r",
		    gch->level,
		    IS_NPC(gch) ? "Mob" : class_table[gch->class].who_name,
		    capitalize( PERS(gch, ch)),
		    gch->hit,   gch->max_hit,
		    gch->mana,  gch->max_mana,
		    gch->move,  gch->max_move);
		send_to_char( buf, ch );
	    }
	}
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "\n\r{RThey aren't here.{x\n\r", ch );
	return;
    }

    if ( ch->master != NULL || ( ch->leader != NULL && ch->leader != ch ) )
    {
	send_to_char( "\n\r{RBut you are following someone else!{x\n\r", ch );
	return;
    }

    if ( victim->master != ch && ch != victim )
    {
	act_new("\n\r{c$N isn't following you.{x",ch,NULL,victim,TO_CHAR,POS_SLEEPING);
	return;
    }
    
    if (victim->level - ch->level >= 10
    || victim->level - ch->level <= -10)
    {
        send_to_char("\n\r{RThey are not within 10 levels of you{x!\n\r",ch);
        return;
    }

    if (IS_AFFECTED(victim,AFF_CHARM))
    {
        send_to_char("\n\r{RYou can't remove charmed mobs from your group{x.\n\r",ch);
        return;
    }
    
    if (IS_AFFECTED(ch,AFF_CHARM))
    {
    	act_new("\n\r{WYou like $m to leave them!{x\n\r",
	    ch,NULL,victim,TO_VICT,POS_SLEEPING);
    	return;
    }

    if ( is_same_group( victim, ch ) && ch != victim )
    {
	victim->leader = NULL;
	act_new("\n\r{c$n removes $N from $s group.{x\n\r",
	    ch,NULL,victim,TO_NOTVICT,POS_RESTING);
	act_new("\n\r{c$n removes you from $s group.{x\n\r",
	    ch,NULL,victim,TO_VICT,POS_SLEEPING);
	act_new("\n\r{cYou remove $N from your group.{x\n\r",
	    ch,NULL,victim,TO_CHAR,POS_SLEEPING);
	return;
    }

    victim->leader = ch;
    act_new("\n\r{C$N joins $n's group.{x\n\r",ch,NULL,victim,TO_NOTVICT,POS_RESTING);
    act_new("\n\r{CYou join $n's group.{x\n\r",ch,NULL,victim,TO_VICT,POS_SLEEPING);
    act_new("\n\r{C$N joins your group.",ch,NULL,victim,TO_CHAR,POS_SLEEPING);
    return;
  }
 else
    {
     send_to_char("\n\r{RGrouping is not allowed in FREE-FOR-ALL DragonPITs!{x\n\r",ch);
     return;
    }
}



/*
 * 'Split' originally by Gnort, God of Chaos.
 */
void do_split( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH],arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *gch;
    int members;
    int amount_gold = 0, amount_silver = 0;
    int share_gold, share_silver;
    int extra_gold, extra_silver;

    argument = one_argument( argument, arg1 );
	       one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Split how much?\n\r", ch );
	return;
    }
    
    amount_silver = atoi( arg1 );

    if (arg2[0] != '\0')
	amount_gold = atoi(arg2);

    if ( amount_gold < 0 || amount_silver < 0)
    {
	send_to_char( "Your group wouldn't like that.\n\r", ch );
	return;
    }

    if ( amount_gold == 0 && amount_silver == 0 )
    {
	send_to_char( "You hand out zero coins, but no one notices.\n\r", ch );
	return;
    }

    if ( ch->gold <  amount_gold || ch->silver < amount_silver)
    {
	send_to_char( "You don't have that much to split.\n\r", ch );
	return;
    }
  
    members = 0;
    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
	if ( is_same_group( gch, ch ) && !IS_AFFECTED(gch,AFF_CHARM))
	    members++;
    }

    if ( members < 2 )
    {
	send_to_char( "Just keep it all.\n\r", ch );
	return;
    }
	    
    share_silver = amount_silver / members;
    extra_silver = amount_silver % members;

    share_gold   = amount_gold / members;
    extra_gold   = amount_gold % members;

    if ( share_gold == 0 && share_silver == 0 )
    {
	send_to_char( "Don't even bother, cheapskate.\n\r", ch );
	return;
    }

    ch->silver	-= amount_silver;
    ch->silver	+= share_silver + extra_silver;
    ch->gold 	-= amount_gold;
    ch->gold 	+= share_gold + extra_gold;

    if (share_silver > 0)
    {
	sprintf(buf,
	    "You split %d silver coins. Your share is %d silver.\n\r",
 	    amount_silver,share_silver + extra_silver);
	send_to_char(buf,ch);
    }

    if (share_gold > 0)
    {
	sprintf(buf,
	    "You split %d gold coins. Your share is %d gold.\n\r",
	     amount_gold,share_gold + extra_gold);
	send_to_char(buf,ch);
    }

    if (share_gold == 0)
    {
	sprintf(buf,"$n splits %d silver coins. Your share is %d silver.",
		amount_silver,share_silver);
    }
    else if (share_silver == 0)
    {
	sprintf(buf,"$n splits %d gold coins. Your share is %d gold.",
		amount_gold,share_gold);
    }
    else
    {
	sprintf(buf,
"$n splits %d silver and %d gold coins, giving you %d silver and %d gold.\n\r",
	 amount_silver,amount_gold,share_silver,share_gold);
    }

    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
	if ( gch != ch && is_same_group(gch,ch) && !IS_AFFECTED(gch,AFF_CHARM))
	{
	    act( buf, ch, NULL, gch, TO_VICT );
	    gch->gold += share_gold;
	    gch->silver += share_silver;
	}
    }

    return;
}



void do_gtell( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *gch;

    if ( argument[0] == '\0' )
    {
	send_to_char( "\n\r{cTell your group what?{x\n\r", ch );
	return;
    }

    if ( IS_SET( ch->comm, COMM_NOTELL ) )
    {
	send_to_char( "\n\r{RYour message didn't get through!{x\n\r", ch );
	return;
    }

    act( "\n\r{mYou tell the Group '{W$T{m'{x", ch, NULL, argument, TO_CHAR );


    for ( gch = char_list; gch != NULL; gch = gch->next )
    {
	if ( is_same_group( gch, ch ) )

        act_new("\n\r{m[{WGROUP{m] {w$n: {W$t{x",ch,argument,gch,TO_VICT,POS_SLEEPING);
    }

    return;
}



/*
 * It is very important that this be an equivalence relation:
 * (1) A ~ A
 * (2) if A ~ B then B ~ A
 * (3) if A ~ B  and B ~ C, then A ~ C
 */
bool is_same_group( CHAR_DATA *ach, CHAR_DATA *bch )
{
    if ( ach == NULL || bch == NULL)
	return FALSE;

    if ( ach->leader != NULL ) ach = ach->leader;
    if ( bch->leader != NULL ) bch = bch->leader;
    return ach == bch;
}

/*
 * Colour setting and unsetting, way cool, Lope Oct '94
 */
void do_colour( CHAR_DATA *ch, char *argument )
{
    char 	arg[ MAX_STRING_LENGTH ];

    argument = one_argument( argument, arg );

    if( !*arg )
    {
	if( !IS_SET( ch->pact, PLR_COLOUR ) )
	{
	    ch->desc->fcolour=TRUE;
	    SET_BIT( ch->pact, PLR_COLOUR );
	    send_to_char( "{bC{ro{yl{co{mu{gr{x is now {rON{x, Way Cool!\n\r", ch );
	}
	else
	{
	    ch->desc->fcolour=FALSE;
	    send_to_char_bw( "Colour is now OFF, <sigh>\n\r", ch );
	    REMOVE_BIT( ch->pact, PLR_COLOUR );
	}
	return;
    }
    else
    {
	send_to_char_bw( "Colour Configuration is unavailable in this\n\r", ch );
	send_to_char_bw( "version of colour, sorry\n\r", ch );
    }

    return;
}

/*
 * Generic Channel function.  Ported from Envy. MUCH simpler.
 */
void talk_channel( CHAR_DATA *ch, char *argument, int channel, const char *verb )
{
  DESCRIPTOR_DATA *d;
  char             buf[ MAX_STRING_LENGTH ];
  CLAN_DATA *pClan;  

  if ( argument[0] == '\0' )
    {
        if(channel==CHANNEL_NEPHILIM && ch->race!=race_lookup("Nephilim"))
            return;
    if ( IS_SET( ch->deaf, channel ) )
      {
/*      REMOVE_BIT( ch->comm, channel ); */
      REMOVE_BIT( ch->deaf, channel );
      sprintf( buf, "\n\r{w%s channel is now {gON{w.\n\r{x", capitalize( verb ) );
      send_to_char( buf, ch );
      }
    else
      {
      SET_BIT( ch->deaf, channel );
      sprintf( buf, "\n\r{w%s channel is now {rOFF{w.\n\r{x", capitalize( verb ) );
      send_to_char( buf, ch );
      }
    return;
    }



  if ( !IS_NPC( ch ) && IS_SET( ch->comm, COMM_NOCHANNELS) )
    {
    sprintf( buf, "\n\r{RYou can't %s.{x\n\r{x", verb );
    send_to_char( buf, ch );
    return;
    }

  REMOVE_BIT( ch->deaf, channel );
  REMOVE_BIT( ch->comm, COMM_QUIET );
	if (!IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK] > 10)
	argument = makedrunk(argument,ch);
if (IS_SET(ch->comm,COMM_DUMMY))
	argument = makedummy(argument, ch);

fix_player_text(ch, argument);

  switch ( channel )
    {
    default:
      sprintf( buf, "\n\r{CYou %s '%s'\n\r{x", verb, argument );
      send_to_char( buf, ch );
      sprintf( buf, "\n\r{C$n %ss '$t'{x\n\r", verb );
      break;
    case CHANNEL_CHAT:
      sprintf( buf, "\n\r{C[{WCHAT{C] {x$n{w: {C$t{x" );
      act_new( buf, ch, argument, NULL, TO_CHAR, POS_DEAD );
      break;
    
    case CHANNEL_GRAT:
      sprintf( buf, "\n\r{W[{RG{YR{GA{CT{MS{W] {x$n{w: {W$t{x" );
      act_new( buf, ch, argument, NULL, TO_CHAR, POS_DEAD );
      break; 

    case CHANNEL_VENT:
      sprintf( buf, "\n\r{B[{WVENT{B]{x $n{w: {B$t{x" );
      act_new( buf, ch, argument, NULL, TO_CHAR, POS_DEAD );
      break;
    case CHANNEL_DPTALK:
      sprintf( buf, "\n\r{D[{RDragonPIT{D] {x$n{w: {R$t{x" );
      act_new( buf, ch, argument, NULL, TO_CHAR, POS_DEAD );
      break;
    case CHANNEL_IMMTALK:
      sprintf( buf, "\n\r{c[{CIMM{c]{x $n{w: {M$t{x" );
      act_new( buf, ch, argument, NULL, TO_CHAR, POS_DEAD );
      break;
    case CHANNEL_BUILDER:
      sprintf( buf, "\n\r{Y[{WBUILDER{Y]{x $n{w: {W$t{x" );
      act_new( buf, ch, argument, NULL, TO_CHAR, POS_DEAD );
      break;
    case CHANNEL_CLAN:
     if(!IS_IMP(ch))
       {
        pClan = get_clan_index(ch->clan);

        sprintf( buf, "\n\r{w[{W%s{w]{x $n{w: {g$t{x",pClan->who_name );
        act_new( buf, ch, argument, NULL, TO_CHAR, POS_DEAD );
        break;
       } 
      else
       {
        pClan = get_clan_index(ch->ctimp_clan);

        sprintf( buf, "\n\r{w[{W%s{w]{x $n{w: {g$t{x",pClan->who_name );
        act_new( buf, ch, argument, NULL, TO_CHAR, POS_DEAD );
        break;
       }
    case CHANNEL_CWAR:
      sprintf( buf, "\n\r{R[{WCWAR{R] {x$n{w: {R$t{x" );
      act_new( buf, ch, argument, NULL, TO_CHAR, POS_DEAD );
      break;
    case CHANNEL_IMP:
      sprintf( buf, "\n\r{c[{CIMP{c]{x $n{C: {c$t{x" );
      act_new( buf, ch, argument, NULL, TO_CHAR, POS_DEAD );
      break;
    case CHANNEL_HERO:
      sprintf( buf, "\n\r{D[{WHero{D]{x $n{C: {c$t{x" );
      act_new( buf, ch, argument, NULL, TO_CHAR, POS_DEAD );
      break;
    case CHANNEL_OWNER:
      sprintf( buf, "\n\r{g[{GOWNER{g]{x $n{w: {G$t{x" );
      act_new( buf, ch, argument, NULL, TO_CHAR, POS_DEAD );
      break;
    case CHANNEL_YELL:
      sprintf( buf, "\n\r{wYou yell '%s'\n\r{x", strip_color(argument) );
      send_to_char( buf, ch );
      sprintf( buf, "{w$n %ss '$t'{x", verb );
      break;
    case CHANNEL_NEPHILIM:
      sprintf( buf, "\n\r{w{D[{RNeph{D]{x $n{w: {D$t{x");
      act_new( buf, ch, argument, NULL, TO_CHAR, POS_DEAD );
      break;
      
   case CHANNEL_AUCTION:
        break;
	//////////////////////////////////////////////////////////////////////////
	// Acrophobia Stuff
   case CHANNEL_ACRO:
        break;

    case CHANNEL_IMMHELP:

      sprintf( buf, "\n\r{r[{WDIVINE AID{r]{x $n{r: {W$t{x" );
      if (!IS_IMMORTAL(ch))
        {
         act( "\n\r{r[{WDIVINE AID{r]{x $n{r: {W$T{x", ch, NULL, argument, TO_CHAR );
        }
      else    
      act_new( buf, ch, argument, NULL, TO_CHAR, POS_DEAD );

      break;
   case CHANNEL_RP:      
       if (!str_cmp (ch->name, "Myserie"))
        {
         sprintf( buf, "\n\r{m[{WThe {RC{Yo{Cd{Mi{Bn{Gg {WGod{m]{x$n{m: {m$t{x");
         act_new( buf, ch, argument, NULL, TO_CHAR, POS_DEAD );
         break;
        }

       if(!str_cmp(ch->name, "Argon"))
        {
        sprintf(buf, "\n\r{m[{wThe {WSh{bad{Dow {wGod{m] {x$n{m: {W$t{x");
        act_new( buf, ch, argument, NULL, TO_CHAR, POS_DEAD );
        break;
        }

       if (!str_cmp (ch->name, "Dreamslayer"))
        {
         sprintf( buf, "\n\r{m[{WThe {BG{br{Ba{bn{Bd {MKnight{m]$n{m: {W$t{x");
         act_new( buf, ch, argument, NULL, TO_CHAR, POS_DEAD );
         break;
        }

       
       if (!str_cmp (ch->name, "Venus"))
        {
         sprintf( buf, "\n\r{m[{WThe {MMUD {WQueen{m] {x$n{m: {W$t{x");
         act_new( buf, ch, argument, NULL, TO_CHAR, POS_DEAD );
         break;
        }

       if (!str_cmp (ch->name, "Divia"))
        {
         sprintf( buf, "\n\r{m[{WFirst {rRot{RR{rD {WHero{m] {x$n{m: {W$t{x");
         act_new( buf, ch, argument, NULL, TO_CHAR, POS_DEAD );
         break;
        }

       if(IS_IMMORTAL(ch))
        {
         sprintf( buf, "\n\r{m[{WImmortal{m] {x$n{m: {W$t{x");
         act_new( buf, ch, argument, NULL, TO_CHAR, POS_DEAD );
         break;
        }
        else
        {
        sprintf( buf, "\n\r{m[{W%s{m]{x $n{m: {W$t{x", pc_race_table[ch->race].name);
        act_new( buf, ch, argument, NULL, TO_CHAR, POS_DEAD );
        break;
        }
    }


  for ( d = descriptor_list; d; d = d->next )
    {
    CHAR_DATA *och;
    CHAR_DATA *vch;
    och = d->original ? d->original : d->character;
    vch = d->character;

    if ( d->connected == CON_PLAYING
    &&   vch != ch
    &&   !IS_SET( och->deaf, channel ) )
      {
          if ( IS_SET( och->comm, COMM_QUIET ) && channel != CHANNEL_OWNER )
            continue;
    
          if (is_ignoring(och, ch))
    	continue;
    
            if(IS_IMP(och))
            {
            act_new(buf, ch, argument, vch, TO_VICT, POS_DEAD );
            continue;
            }
    
    
          if (IS_IMP(ch))
            {
          if (( channel == CHANNEL_IMMTALK && !IS_IMMORTAL( och ) && !granted(och, "immtalk"))
          ||  ( channel == CHANNEL_IMP && (!(och->level >= 499)))
          ||  ( channel == CHANNEL_OWNER && (!is_owner(och)))
          ||  ( channel == CHANNEL_YELL && ch->in_room->area != och->in_room->area )
          ||  ( channel == CHANNEL_HERO && ( och->level < 100 && och->pcdata->oldcl == -1 ) )
          ||  (channel == CHANNEL_CLAN && ch->ctimp_clan != och->clan) 
          ||  (channel == CHANNEL_BUILDER && och->level <= LEVEL_HERO) 
          ||  (channel == CHANNEL_IMMHELP && och->level <= LEVEL_HERO) 
          ||  (channel == CHANNEL_CWAR && och->clan == 0 ))
            continue;
          if (( channel == CHANNEL_YELL )
          &&    och->position <= POS_SLEEPING )
            continue;
           }
        else
         {
          if (( channel == CHANNEL_IMMTALK && !IS_IMMORTAL( och ) && !granted(och, "immtalk"))
          ||  ( channel == CHANNEL_IMP && (!(och->level >= 499)))
          ||  ( channel == CHANNEL_OWNER && (!is_owner(och)))
          ||  ( channel == CHANNEL_YELL && ch->in_room->area != och->in_room->area )
          ||  ( channel == CHANNEL_HERO && ( och->level < 100 && och->pcdata->oldcl == -1 ) )
          ||  (channel == CHANNEL_CLAN && ( (ch->clan != och->clan && ch->clan != och->ctimp_clan)) ) 
          ||  (channel == CHANNEL_BUILDER && !IS_IMMORTAL(och)) 
          ||  (channel == CHANNEL_IMMHELP && och->level <= LEVEL_HERO) 
          ||  (channel == CHANNEL_CWAR && och->clan == 0 ))
            continue;
          if (( channel == CHANNEL_YELL )
          &&    och->position <= POS_SLEEPING )
            continue;
           if(channel==CHANNEL_NEPHILIM && och->race != RACE_NEPHILIM)
             continue;
         }
         
         act_new( buf, ch, argument, vch, TO_VICT, POS_DEAD );
      }
    }
 return;
}



/*
 * this function sends raw argument over the AUCTION: channel
 * I am not too sure if this method is right..
 */

void talk_auction (char *argument, CHAR_DATA * ch)
{
    DESCRIPTOR_DATA *d;
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *original;

  if ( argument[0] == '\0' )
    {
        if ( IS_SET( ch->deaf, CHANNEL_AUCTION) )
          {
           REMOVE_BIT( ch->comm, CHANNEL_AUCTION );
           REMOVE_BIT( ch->deaf, CHANNEL_AUCTION);
           sprintf( buf, "\n\r{cAUCTION Channel is now {GON{c.\n\r{x");
           send_to_char( buf, ch );
          }
        else
          {
           SET_BIT( ch->deaf, CHANNEL_AUCTION );
           sprintf( buf, "\n\r{cAUCTION Channel is now {ROFF{c.\n\r{x");
           send_to_char( buf, ch );
          }
      return;
    }

    sprintf (buf,"\n\r{C[{cAUCTION{C]{w: {W%s{x\n\r", argument);

    for (d = descriptor_list; d != NULL; d = d->next)
    {
        original = d->original ? d->original : d->character; /* if switched */
        if ((d->connected == CON_PLAYING) && !IS_SET(original->deaf,CHANNEL_AUCTION))
            act_new(buf, original, NULL, NULL, TO_CHAR, POS_DEAD);
    }
}

void do_beep( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;

    if ( argument[0]=='\0' )
    {
        if ( IS_SET( ch->comm, COMM_BEEP ) )
        {
            REMOVE_BIT( ch->comm, COMM_BEEP );
            send_to_char( "You no longer receive beeps.\n\r", ch );
        }
        else
        {
            SET_BIT( ch->comm, COMM_BEEP );
            send_to_char( "You now accept beeps.\n\r", ch );
        }
        return;
    }
    if ( !IS_SET( ch->comm, COMM_BEEP ) )
    {
        send_to_char( "You have to turn on the beep channel first.\n\r", ch);
        return;
    }

    if ( ( victim=get_char_world( ch, argument ) )==NULL )
    {
        send_to_char( "Nobody like that.\n\r", ch );
        return;
    }

    if ( !IS_SET( victim->comm, COMM_BEEP ) )
    {
        act_new("$N is not receiving beeps.", ch, NULL, victim, TO_CHAR, POS_DEAD);
        return;
    }

    act_new( "\aYou {Ybeep{x to $N.", ch, NULL, victim, TO_CHAR, POS_DEAD );
    act_new( "\a$n {Gbeeps{x you.", ch, NULL, victim, TO_VICT, POS_DEAD );

    return;
}


/*
char    as_string[MAX_NUM_AUTOSNOOP][70];

void do_asnoop(CHAR_DATA *ch, char *argument)
{ 
    char arg[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];

    int position=1;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
      {   
       send_to_char( "\n\r{RThat is not a valid number!{x\n\r", ch );
       return;
      }

    position = atoi(arg);
    if ((position < 1)||(position >= MAX_NUM_AUTOSNOOP))
      {
       send_to_char("\n\r{RPosition is outta range!{x\n\r", ch);
       return;
      }

    sprintf(buf,"\n\r{gSwitching from AutoSnooping{w: {W%s {gto AutoSnooping{w: {W%s {gin slot #{W%d {g.{x\n\r",
as_string[position], argument != NULL ? argument : "{D({WNONE{D){x",position);

    strcpy(as_string[position],argument);
    send_to_char(buf,ch);

    return;
}

void do_aslist(CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    int i;
    int counter;

    send_to_char  ("\n\r{RAUTOSNOOPING THE FOLLOWING{w:{x\n\r", ch);
   
    counter = -1;
    for (i=0; i < MAX_NUM_AUTOSNOOP; i++) 
     {
        counter++;
        sprintf(buf,"{C%d  {w:  {C%s{x\n\r",counter,as_string[i]);
        send_to_char(buf,ch);
    }
}

*/


/* RACIAL and GOD LANGUAGEs */

 void do_godspeak( CHAR_DATA *ch, char *argument)
  {
  CHAR_DATA *och;
 
  if ((!IS_IMMORTAL(ch))
  && (!IS_CLASS(ch, CLASS_CLERIC))
  && (!IS_CLASS(ch, CLASS_PALADIN))
  && (!IS_IMP(ch)))
  {
  send_to_char("\n\r{RYou don't know how to speak in the Godly Tongue!{x\n\r",ch);
  return; 
  }

  if(argument[0] == '\0')
  {
  send_to_char("\n\r{RSay WHAT in the Divine Tongue?{x\n\r",ch);
  return; 
  }


  if((IS_IMMORTAL(ch))
  || (IS_CLASS(ch, CLASS_CLERIC))
  || (IS_CLASS(ch, CLASS_PALADIN))
  || (IS_IMP(ch)))
    {
     act("\n\r{YIn GodSpeak, you say '$t'{x",ch,argument,NULL,TO_CHAR);
     
     for(och = ch->in_room->people; och != NULL; och = och->next_in_room )
       {
        if((!IS_IMMORTAL(och))
        && (!IS_CLASS(och, CLASS_CLERIC))
        && (!IS_CLASS(och, CLASS_PALADIN))
        && (!IS_IMP(och)))
          {
           act("\n\r{Y$n says something in a strange tongue.{x",ch,NULL,och,TO_VICT);
          }

        if((IS_IMMORTAL(och))
        || (IS_CLASS(och, CLASS_CLERIC))
        || (IS_CLASS(och, CLASS_PALADIN))
        || (IS_IMP(och)))
          {
           act("\n\r{YIn GodSpeak, $n says '$t'{x",ch,argument,och,TO_VICT);
          }
       }
   }

 return;
}

 void do_elven( CHAR_DATA *ch, char *argument)
  {
  CHAR_DATA *och;
 
 if ((ch->race != RACE_ELF)
 &&  (!IS_CLASS(ch, CLASS_ENCHANTER))
 &&  (ch->race != RACE_SIDHE_ELF)
 &&  (!IS_IMP(ch)) && !IS_SET(ch->racechan, RACECHAN_ELVEN))

  {
  send_to_char("\n\r{RYou don't know how to speak in the Elven Tongue!{x\n\r",ch);
  return; 
  }

  if(argument[0] == '\0')
  {
  send_to_char("\n\r{RSay WHAT in the Elven Tongue?{x\n\r",ch);
  return; 
  }

 if((ch->race == RACE_ELF)
 || (IS_CLASS(ch, CLASS_ENCHANTER))
 || (ch->race == RACE_SIDHE_ELF)
 || (IS_IMP(ch)) || IS_SET(ch->racechan, RACECHAN_ELVEN))
   {
    act("\n\r{YIn Elven, you say '$t'{x",ch,argument,NULL,TO_CHAR);
     
     for(och = ch->in_room->people; och != NULL; och = och->next_in_room )
       {

        if((och->race != RACE_ELF)
        && (!IS_CLASS(och, CLASS_ENCHANTER))
        && (och->race != RACE_SIDHE_ELF) 
        && (!IS_IMP(och)) && !IS_SET(ch->racechan, RACECHAN_ELVEN))
          {
           act("\n\r{Y$n says something in a strange tongue.{x",ch,NULL,och,TO_VICT);
          }

        if((och->race == RACE_ELF)
        ||  (IS_CLASS(och, CLASS_ENCHANTER))
        || (och->race == RACE_SIDHE_ELF) 
        || (IS_IMP(och)) || IS_SET(ch->racechan, RACECHAN_ELVEN))
          {
           act("\n\r{YIn Elven, $n says '$t'{x",ch,argument,och,TO_VICT);
          }
       }
   }
 return;
}


void do_demonic( CHAR_DATA *ch, char *argument)
  {
  CHAR_DATA *och;
    
        if ((ch->race != RACE_SUCCUBUS)
        && (!IS_CLASS(ch, CLASS_ENCHANTER))
        && (ch->race != RACE_INCCUBUS) 
        && (ch->race != RACE_CAMBION)  
        && (ch->race != RACE_MYRDDRAAL)
        && (!IS_IMP(ch)) && !IS_SET(ch->racechan, RACECHAN_DEMONIC))
  {
  send_to_char("\n\r{RYou don't know how to speak in the Demonic Tongue!{x\n\r",ch);
  return;
  }
   
  if(argument[0] == '\0')
   {
    send_to_char("\n\r{RSay WHAT in the Demonic Tongue?{x\n\r",ch);
    return;
  }
   
        if ((ch->race == RACE_SUCCUBUS)
        || (IS_CLASS(ch, CLASS_ENCHANTER))
        || (ch->race == RACE_INCCUBUS) 
        || (ch->race == RACE_CAMBION) 
        || (ch->race == RACE_MYRDDRAAL) 
        || (IS_IMP(ch)) || IS_SET(ch->racechan, RACECHAN_DEMONIC))
          {
    act("\n\r{YIn Demonic, you say '$t'{x",ch,argument,NULL,TO_CHAR);
    
     for(och = ch->in_room->people; och != NULL; och = och->next_in_room )
       {
        if ((och->race != RACE_SUCCUBUS)
        && (!IS_CLASS(och, CLASS_ENCHANTER))
        && (och->race != RACE_INCCUBUS) 
        && (och->race != RACE_CAMBION)  
        && (och->race != RACE_MYRDDRAAL) 
        && (!IS_IMP(och)) && !IS_SET(ch->racechan, RACECHAN_DEMONIC))
          {
           act("\n\r{Y$n says something in a strange tongue.{x",ch,NULL,och,TO_VICT);
          }
  
        if ((och->race == RACE_SUCCUBUS)
        || (IS_CLASS(och, CLASS_ENCHANTER))
        || (och->race == RACE_INCCUBUS) 
        || (och->race == RACE_CAMBION) 
        || (och->race == RACE_MYRDDRAAL) 
        || (IS_IMP(och)) || IS_SET(ch->racechan, RACECHAN_DEMONIC))
          {
           act("\n\r{YIn Demonic, $n says '$t'{x",ch,argument,och,TO_VICT);
          }
       }
   }
 return;
}


void do_dwarven( CHAR_DATA *ch, char *argument)
  {
  CHAR_DATA *och;
    
 if ((ch->race != RACE_DWARF)
 && (!IS_CLASS(ch, CLASS_ENCHANTER))
 && (!IS_IMP(ch)) && !IS_SET(ch->racechan, RACECHAN_DWARVEN))
  {
  send_to_char("\n\r{RYou don't know how to speak in the Dwarven Tongue!{x\n\r",ch);
  return;
  }
 
  if(argument[0] == '\0')
  {
  send_to_char("\n\r{RSay WHAT in the Dwarven Tongue?{x\n\r",ch);
  return;
  }
   
        if ((ch->race == RACE_DWARF)
        || (IS_CLASS(ch, CLASS_ENCHANTER))
        || (IS_IMP(ch)) || IS_SET(ch->racechan, RACECHAN_DWARVEN))
          {
    act("\n\r{YIn Dwarven, you say '$t'{x",ch,argument,NULL,TO_CHAR);
    
     for(och = ch->in_room->people; och != NULL; och = och->next_in_room )
       {
        if ((och->race != RACE_DWARF)
        && (!IS_CLASS(och, CLASS_ENCHANTER))
        && (!IS_IMP(och)) && !IS_SET(ch->racechan, RACECHAN_DWARVEN))
          {
           act("\n\r{Y$n says something in a strange tongue.{x",ch,NULL,och,TO_VICT);
          }
  
        if ((och->race == RACE_DWARF)
        || (IS_CLASS(och, CLASS_ENCHANTER))
        || (IS_IMP(och)) || IS_SET(ch->racechan, RACECHAN_DWARVEN))
          {
           act("\n\r{YIn Dwarven, $n says '$t'{x",ch,argument,och,TO_VICT);
          }
       }
   }
 return;
}

void do_draconian( CHAR_DATA *ch, char *argument)
  {
  CHAR_DATA *och;
    
 if ((ch->race != RACE_DRACONIAN)
 && (!IS_IMP(ch)) && !IS_SET(ch->racechan, RACECHAN_DRACONIAN))
  {
  send_to_char("\n\r{RYou don't know how to speak in the Draconian Tongue!{x\n\r",ch);
  return;
  }
 
  if(argument[0] == '\0')
  {
  send_to_char("\n\r{RSay WHAT in the Draconian Tongue?{x\n\r",ch);
  return;
  }

        if ((ch->race == RACE_DRACONIAN)
        || (IS_IMP(ch)) || IS_SET(ch->racechan, RACECHAN_DRACONIAN))
          {
    act("\n\r{YIn Draconian, you say '$t'{x",ch,argument,NULL,TO_CHAR);
    
     for(och = ch->in_room->people; och != NULL; och = och->next_in_room )
       {
        if ((och->race != RACE_DRACONIAN)
        && (!IS_IMP(och)) && !IS_SET(ch->racechan, RACECHAN_DRACONIAN))
          {
           act("\n\r{Y$n says something in a strange tongue.{x",ch,NULL,och,TO_VICT);
          }
  
        if ((och->race == RACE_DRACONIAN)
        || (IS_IMP(och)) || IS_SET(ch->racechan, RACECHAN_DRACONIAN))
          {
           act("\n\r{YIn Draconian, $n says '$t'{x",ch,argument,och,TO_VICT);
          }
       }
   }
 return;
}


void do_reptilian( CHAR_DATA *ch, char *argument)
  {
  CHAR_DATA *och;
    
 if ((ch->race != RACE_RAPTOR)
 &&  (ch->race != RACE_LIZARDMAN)
 &&  (ch->race != RACE_LAMIA)
 &&  (!IS_IMP(ch)) && !IS_SET(ch->racechan, RACECHAN_REPTILIAN))
  {
  send_to_char("\n\r{RYou don't know how to speak in the Reptilian Tongue!{x\n\r",ch);
  return;
  }
 
  if(argument[0] == '\0')
  {
  send_to_char("\n\r{RSay WHAT in the Reptilian Tongue?{x\n\r",ch);
  return;
  }
   

        if ((ch->race == RACE_RAPTOR)
        ||  (ch->race == RACE_LIZARDMAN)
        ||  (ch->race == RACE_LAMIA) 
        ||  (IS_IMP(ch)) || IS_SET(ch->racechan, RACECHAN_REPTILIAN))
          {
    act("\n\r{YIn Reptilian, you say '$t'{x",ch,argument,NULL,TO_CHAR);
    
     for(och = ch->in_room->people; och != NULL; och = och->next_in_room )
       {
        
        if ((och->race != RACE_RAPTOR)
        &&  (och->race != RACE_LIZARDMAN)
        &&  (och->race != RACE_LAMIA) 
        &&  (!IS_IMP(och)) && !IS_SET(ch->racechan, RACECHAN_REPTILIAN))
          {
           act("\n\r{Y$n says something in a strange tongue.{x",ch,NULL,och,TO_VICT);
          }
  
        if ((och->race == RACE_RAPTOR)
        ||  (och->race == RACE_LIZARDMAN)
        ||  (och->race == RACE_LAMIA) 
        ||  (IS_IMP(och)) || IS_SET(ch->racechan, RACECHAN_REPTILIAN))
          {
           act("\n\r{YIn Reptilian, $n says '$t'{x",ch,argument,och,TO_VICT);
          }
       }
   }
 return;
}


void do_drow( CHAR_DATA *ch, char *argument)
  {
  CHAR_DATA *och;
    
 if ((ch->race != RACE_DARKELF)
 && (!IS_CLASS(ch, CLASS_ENCHANTER))
 && (!IS_IMP(ch)) && !IS_SET(ch->racechan, RACECHAN_DROW))
  {
  send_to_char("\n\r{RYou don't know how to speak Drowish!{x\n\r",ch);
  return;
  }
 
  if(argument[0] == '\0')
  {
  send_to_char("\n\r{RSay WHAT in Drow?{x\n\r",ch);
  return;
  }
   
        if ((ch->race == RACE_DARKELF)
        || (IS_CLASS(ch, CLASS_ENCHANTER))
        || (IS_IMP(ch)) || IS_SET(ch->racechan, RACECHAN_DROW))
          {
    act("\n\r{YIn Drow, you say '$t'{x",ch,argument,NULL,TO_CHAR);
    
     for(och = ch->in_room->people; och != NULL; och = och->next_in_room )
       {
        if ((och->race != RACE_DARKELF)
        && (!IS_CLASS(och, CLASS_ENCHANTER))
        &&  (!IS_IMP(och)) && !IS_SET(ch->racechan, RACECHAN_DROW))
          {
           act("\n\r{Y$n says something in a strange tongue.{x",ch,NULL,och,TO_VICT);
          }
  
        if ((och->race == RACE_DARKELF)
        || (IS_CLASS(och, CLASS_ENCHANTER))
        || (IS_IMP(och)) || IS_SET(ch->racechan, RACECHAN_DROW))
          {
           act("\n\r{YIn Drow, $n says '$t'{x",ch,argument,och,TO_VICT);
          }
       }
   }
 return;
}


void do_high_elven( CHAR_DATA *ch, char *argument)
  {
  CHAR_DATA *och;
    
 if((ch->race != RACE_SIDHE_ELF)
 && (!IS_CLASS(ch, CLASS_ENCHANTER))
 && (!IS_IMP(ch)) && !IS_SET(ch->racechan, RACECHAN_HIGH_ELVEN))
  {
  send_to_char("\n\r{RYou don't know how to speak in the High Elven Tongue!{x\n\r",ch);
  return;
  }
 
  if(argument[0] == '\0')
  {
  send_to_char("\n\r{RSay WHAT in the High Elven Tongue?{x\n\r",ch);
  return;
  }
   
        if ((ch->race == RACE_SIDHE_ELF)
        || (IS_CLASS(ch, CLASS_ENCHANTER))
        || (IS_IMP(ch)) || IS_SET(ch->racechan, RACECHAN_HIGH_ELVEN))
          {
    act("\n\r{YIn High Elven, you say '$t'{x",ch,argument,NULL,TO_CHAR);
    
     for(och = ch->in_room->people; och != NULL; och = och->next_in_room )
       {
        
        if ((och->race != RACE_SIDHE_ELF)
        && (!IS_CLASS(och, CLASS_ENCHANTER))
        && (!IS_IMP(och)) && !IS_SET(ch->racechan, RACECHAN_HIGH_ELVEN))
          {
           act("\n\r{Y$n says something in a strange tongue.{x",ch,NULL,och,TO_VICT);
          }
  
        if ((och->race == RACE_SIDHE_ELF)
        || (IS_CLASS(och, CLASS_ENCHANTER))
        || (IS_IMP(och)) || IS_SET(ch->racechan, RACECHAN_HIGH_ELVEN))
          {
           act("\n\r{YIn High Elven, $n says '$t'{x",ch,argument,och,TO_VICT);
          }
       }
   }
 return;
}



void do_undead( CHAR_DATA *ch, char *argument)
  {
  CHAR_DATA *och;
    
 if ((ch->race != RACE_MYRDDRAAL)
 && (!IS_IMP(ch)) && !IS_SET(ch->racechan, RACECHAN_UNDEAD))
  {
  send_to_char("\n\r{RYou don't know how to speak in the Undead Tongue!{x\n\r",ch);
  return;
  }
 
  if(argument[0] == '\0')
  {
  send_to_char("\n\r{RSay WHAT in the Undead Tongue?{x\n\r",ch);
  return;
  }
   
        if ((ch->race == RACE_MYRDDRAAL)
        || (IS_IMP(ch)) || IS_SET(ch->racechan, RACECHAN_UNDEAD))
          {
    act("\n\r{YIn the Undead Language, you say '$t'{x",ch,argument,NULL,TO_CHAR);
    
     for(och = ch->in_room->people; och != NULL; och = och->next_in_room )
       {
        if ((och->race != RACE_MYRDDRAAL)
        && (!IS_IMP(och)) && !IS_SET(ch->racechan, RACECHAN_UNDEAD))
          {
           act("\n\r{Y$n says something in a strange tongue.{x",ch,NULL,och,TO_VICT);
          }
  
        if ((och->race == RACE_MYRDDRAAL)
        || (IS_IMP(och)) || IS_SET(ch->racechan, RACECHAN_UNDEAD))
          {
           act("\n\r{YIn the Undead Language, $n says '$t'{x",ch,argument,och,TO_VICT);
          }
       }
   }
 return;
}

void do_kender( CHAR_DATA *ch, char *argument)
  {
  CHAR_DATA *och;
    
 if ((ch->race != RACE_KENDER)
 && (!IS_IMP(ch)) && !IS_SET(ch->racechan, RACECHAN_KENDER))
  {
  send_to_char("\n\r{RYou don't know how to speak in the Kender Tongue!{x\n\r",ch);
  return;
  }
 
  if(argument[0] == '\0')
  {
  send_to_char("\n\r{RSay WHAT in the  Kender Tongue?{x\n\r",ch);
  return;
  }
   
        if ((ch->race == RACE_KENDER)
        || (IS_IMP(ch)) || IS_SET(ch->racechan, RACECHAN_KENDER))
          {
    act("\n\r{YIn Kender, you say '$t'{x",ch,argument,NULL,TO_CHAR);
    
     for(och = ch->in_room->people; och != NULL; och = och->next_in_room )
       {
        if ((och->race !=RACE_KENDER)
        && (!IS_IMP(och)) && !IS_SET(ch->racechan, RACECHAN_KENDER))
          {
           act("\n\r{Y$n says something in a strange tongue.{x",ch,NULL,och,TO_VICT);
          }
  
        if ((och->race == RACE_KENDER)
        || (IS_IMP(och)) || IS_SET(ch->racechan, RACECHAN_KENDER))
          {
           act("\n\r{YIn Kender, $n says '$t'{x",ch,argument,och,TO_VICT);
          }
       }
   }
 return;
}

void do_magespeak( CHAR_DATA *ch, char *argument)
  {
  CHAR_DATA *och;
    
        if ((ch->race != RACE_GOLEM)
        && (!IS_CLASS(ch, CLASS_ENCHANTER))
        && (!IS_CLASS(ch, CLASS_MAGE))
        && (!IS_IMP(ch)) && !IS_SET(ch->racechan, RACECHAN_MAGESPEAK))
  {
  send_to_char("\n\r{RYou don't know how to speak in the Language of Magic!{x\n\r",ch);
  return;
  }
 
  if(argument[0] == '\0')
  {
  send_to_char("\n\r{RSay WHAT in the Language of Magic?{x\n\r",ch);
  return;
  }
   
        if ((ch->race == RACE_GOLEM)
        || (IS_CLASS(ch, CLASS_ENCHANTER))
        || (IS_CLASS(ch, CLASS_MAGE))
        || (IS_IMP(ch)) || IS_SET(ch->racechan, RACECHAN_MAGESPEAK))
          {
    act("\n\r{YIn MageSpeak, you say '$t'{x",ch,argument,NULL,TO_CHAR);
    
     for(och = ch->in_room->people; och != NULL; och = och->next_in_room )
       {
        if ((och->race != RACE_GOLEM)
        && (!IS_CLASS(och, CLASS_ENCHANTER))
        && (!IS_CLASS(och, CLASS_MAGE))
        && (!IS_IMP(och)) && !IS_SET(ch->racechan, RACECHAN_MAGESPEAK))
          {
           act("\n\r{Y$n says something in a strange tongue.{x",ch,NULL,och,TO_VICT);
          }
  
        if ((och->race == RACE_GOLEM)
        || (IS_CLASS(och, CLASS_ENCHANTER))
        || (IS_CLASS(och, CLASS_MAGE))
        || (IS_IMP(och)) || IS_SET(ch->racechan, RACECHAN_MAGESPEAK))
          {
           act("\n\r{YIn MagESpeak, $n says '$t'{x",ch,argument,och,TO_VICT);
          }
       }
   }
 return;
}


void do_forestsign( CHAR_DATA *ch, char *argument)
  {
  CHAR_DATA *och;
    
        if ((ch->race != RACE_SIDHE_ELF)
        && (!IS_CLASS(ch, CLASS_RANGER))
        && (!IS_IMP(ch)) && !IS_SET(ch->racechan, RACECHAN_FORESTSIGN))
  {
  send_to_char("\n\r{RYou don't know how to use the Forest Sign Language!{x\n\r",ch);
  return;
  }
 
  if(argument[0] == '\0')
  {
  send_to_char("\n\r{RSay WHAT in Forest Sign Language?{x\n\r",ch);
  return;
  }
   
        if ((ch->race == RACE_SIDHE_ELF)
        || (IS_CLASS(ch, CLASS_RANGER))
        || (IS_IMP(ch)) || IS_SET(ch->racechan, RACECHAN_FORESTSIGN))
          {
    act("\n\r{YIn ForestSign, you signal '$t'{x",ch,argument,NULL,TO_CHAR);
    
     for(och = ch->in_room->people; och != NULL; och = och->next_in_room )
       {
        if ((och->race != RACE_SIDHE_ELF)
        && (!IS_CLASS(och, CLASS_RANGER))
        && (!IS_IMP(och)) && !IS_SET(ch->racechan, RACECHAN_FORESTSIGN))
          {
           act("\n\r{Y$n makes some rapid gestures with the hands.{x",ch,NULL,och,TO_VICT);
          }
  
        if ((och->race == RACE_SIDHE_ELF)
        || (IS_CLASS(och, CLASS_RANGER))
        || (IS_IMP(och)) || IS_SET(ch->racechan, RACECHAN_FORESTSIGN))
          {
           act("\n\r{YIn ForestSign, $n signals '$t'{x",ch,argument,och,TO_VICT);
          }
       }
   }
 return;
}


void do_thieves_cant( CHAR_DATA *ch, char *argument)
  {
  CHAR_DATA *och;
    
 if ((ch->race != RACE_MYRDDRAAL)
 &&  (ch->race != RACE_KENDER)
 &&  (!IS_CLASS(ch, CLASS_THIEF))
 &&  (!IS_IMP(ch)) && !IS_SET(ch->racechan, RACECHAN_THIEVES_CANT))
  {
  send_to_char("\n\r{RYou don't know how to speak in Thieves Cant!{x\n\r",ch);
  return;
  }
 
  if(argument[0] == '\0')
  {
  send_to_char("\n\r{RSay WHAT in Thieves Cant?{x\n\r",ch);
  return;
  }
   
        if ((ch->race == RACE_MYRDDRAAL)
        || (ch->race == RACE_KENDER)
        || (IS_CLASS(ch, CLASS_THIEF)) 
        || (IS_IMP(ch)) || IS_SET(ch->racechan, RACECHAN_THIEVES_CANT))
          {
    act("\n\r{YIn Thieves Cant, you say '$t'{x",ch,argument,NULL,TO_CHAR);
    
     for(och = ch->in_room->people; och != NULL; och = och->next_in_room )
       {
        if ((och->race != RACE_MYRDDRAAL)
        &&  (och->race != RACE_KENDER)
        &&  (!IS_CLASS(och, CLASS_THIEF)) 
        &&  (!IS_IMP(och)) && !IS_SET(ch->racechan, RACECHAN_THIEVES_CANT))
          {
           act("\n\r{Y$n says something in a strange tongue.{x",ch,NULL,och,TO_VICT);
          }
  
        if ((och->race == RACE_MYRDDRAAL)
        || (och->race == RACE_KENDER)
        || (IS_CLASS(och, CLASS_THIEF)) 
        || (IS_IMP(och)) || IS_SET(ch->racechan, RACECHAN_THIEVES_CANT))
          {
           act("\n\r{YIn Thieves Cant, $n says '$t'{x",ch,argument,och,TO_VICT);
          }
       }
   }
 return;
}


void do_handtalk( CHAR_DATA *ch, char *argument)
  {
  CHAR_DATA *och;

        if ((ch->race != RACE_MYRDDRAAL)
        &&  (!IS_CLASS(ch, CLASS_ASSASSIN))
        &&  (!IS_CLASS(ch, CLASS_ANTI_PALADIN)) 
        &&  (!IS_IMP(ch)) && !IS_SET(ch->racechan, RACECHAN_HANDTALK))
  {
  send_to_char("\n\r{RYou don't know how to use HandTalk!{x\n\r",ch);
  return;
  }
 
  if(argument[0] == '\0')
  {
  send_to_char("\n\r{RSign WHAT in HandTalk?{x\n\r",ch);
  return;
  }
   
        if ((ch->race == RACE_MYRDDRAAL)
        ||  (IS_CLASS(ch, CLASS_ASSASSIN))
        ||  (IS_CLASS(ch, CLASS_ANTI_PALADIN)) 
        ||  (IS_IMP(ch)) || IS_SET(ch->racechan, RACECHAN_HANDTALK))
          {
    act("\n\r{YIn HandTalk, you signal '$t'{x",ch,argument,NULL,TO_CHAR);
    
     for(och = ch->in_room->people; och != NULL; och = och->next_in_room )
       {
        if ((och->race != RACE_MYRDDRAAL)
        &&  (!IS_CLASS(och, CLASS_ASSASSIN))
        &&  (!IS_CLASS(och, CLASS_ANTI_PALADIN)) 
        &&  (!IS_IMP(och)) && !IS_SET(ch->racechan, RACECHAN_HANDTALK))
          {
           act("\n\r{Y$n makes some unusual gestures with the hands.{x",ch,NULL,och,TO_VICT);
          }
  
        if ((och->race == RACE_MYRDDRAAL)
        ||  (IS_CLASS(och, CLASS_ASSASSIN))
        ||  (IS_CLASS(och, CLASS_ANTI_PALADIN)) 
        ||  (IS_IMP(och)) || IS_SET(ch->racechan, RACECHAN_HANDTALK))
          {
           act("\n\r{YIn HandTalk, $n signals '$t'{x",ch,argument,och,TO_VICT);
          }
       }
   }
 return;
}


void do_italian( CHAR_DATA *ch, char *argument)
  {
  CHAR_DATA *och;
    
 if (!IS_IMP(ch))
  {
  send_to_char("\n\r{RYou don't know how to speak Italian!{x\n\r",ch);
  return;
  }
 
  if(argument[0] == '\0')
  {
  send_to_char("\n\r{RSay WHAT in Italian?{x\n\r",ch);
  return;
  }
   
        if (IS_IMP(ch))
          {
    act("\n\r{YIn Italian, you say '$t'{x",ch,argument,NULL,TO_CHAR);
    
     for(och = ch->in_room->people; och != NULL; och = och->next_in_room )
       {
        if (!IS_IMP(och))
          {
           act("\n\r{Y$n says something in a strange language, it sounds Italian.{x",ch,NULL,och,TO_VICT);
          }
  
        if (IS_IMP(och))
          {
           act("\n\r{YIn Italian, $n says '$t'{x",ch,argument,och,TO_VICT);
          }
       }
   }
 return;
}


void do_war_chant( CHAR_DATA *ch, char *argument)
  {
  CHAR_DATA *och;

 if ((!IS_CLASS(ch, CLASS_WARRIOR))
 && (ch->race != RACE_NEPHILIM)
 && (!IS_IMP(ch)) && !IS_SET(ch->racechan, RACECHAN_WAR_CHANT))
  {
  send_to_char("\n\r{RYou try chanting and grunting some strange noises!{x\n\r",ch);
  return;
  }
 
  if(argument[0] == '\0')
  {
  send_to_char("\n\r{RChant WHAT in the Tongue of the Warrior?{x\n\r",ch);
  return;
  }
   
        if ((IS_CLASS(ch, CLASS_WARRIOR))
        || (ch->race == RACE_NEPHILIM)
        || (IS_IMP(ch)) || IS_SET(ch->racechan, RACECHAN_WAR_CHANT))
          {
     act("\n\r{YIn War Chant, you send the message '$t'{x",ch,argument,NULL,TO_CHAR);
    
     for(och = ch->in_room->people; och != NULL; och = och->next_in_room )
       {
        if ((!IS_CLASS(och, CLASS_WARRIOR))
        && (och->race != RACE_NEPHILIM)
        && (!IS_IMP(och)) && !IS_SET(ch->racechan, RACECHAN_WAR_CHANT))
          {
           act("\n\r{Y$n makes some strange sounds in a gutteral voice!{x",ch,NULL,och,TO_VICT);
          }
  
        if ((IS_CLASS(och, CLASS_WARRIOR))
        || (och->race == RACE_NEPHILIM)
        || (IS_IMP(och)) || IS_SET(ch->racechan, RACECHAN_WAR_CHANT))
          {
           act("\n\r{YIn War Chant, $n sends the message '$t'{x",ch,argument,och,TO_VICT);
          }
       }
   }
 return;
}

