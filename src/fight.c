#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include "merc.h"
#include "interp.h"
#include "dpit.h"

#include "Utility.h"
#include "StringUtility.h"
#include "ArrayUtility.h"

DECLARE_DO_FUN(do_restore);
DECLARE_DO_FUN(do_assassinate);
DECLARE_DO_FUN(do_multiburst);
DECLARE_DO_FUN(do_whirlwind);
DECLARE_DO_FUN(do_circle);
DECLARE_DO_FUN(do_critical_strike);

void talk_channel( CHAR_DATA *ch, char *argument, int channel, const char *verb );

// Explorer/Killer Percentages
bool valid_explorer_killer(CHAR_DATA *ch);

/*
 * Local functions.
 */
 int new_xp_compute(int moblevel, int playerlevel, int aligndifference);
void	check_assist	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	check_dodge	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
/*	void	check_killer	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) ); */
bool	check_parry	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	check_valid_attack args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool    check_shield_block     args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void    dam_message 	args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dam,
                            int dt, bool immune ) );
void	death_cry	args( ( CHAR_DATA *ch ) );
void	group_gain	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
int	xp_compute	args( ( CHAR_DATA *gch, CHAR_DATA *victim, 
			    int total_levels ) );
bool	is_safe		args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	make_corpse	args( ( CHAR_DATA *ch, CHAR_DATA *killer ) );
void	one_hit		args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt, bool secondary ) );
void    mob_hit		args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt ) );
void	raw_kill	args( ( CHAR_DATA *victim, CHAR_DATA *killer ) );
void	set_fighting	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	disarm		args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	update_pkinfo	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );

bool	is_fighting_pc	args( ( CHAR_DATA *ch ) );

/*
extern bool remove_obj (CHAR_DATA *ch, int iWear, bool fReplace);

    OBJ_DATA *obj_next;
    OBJ_DATA *obj;

      for (obj = victim->carrying; obj; obj = obj_next)
      {
       obj_next = obj->next_content;
       
       if (obj->wear_loc != WEAR_INVENTORY && can_see_obj (victim, obj))
       remove_obj (victim, obj->wear_loc, TRUE);

       save_char_obj(victim);
      }
*/

void do_assassinate( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    OBJ_DATA  *weapon = NULL;
    int bingoChance;
    char    buf[MAX_STRING_LENGTH];
    // Explorer/Killer Percentages
    char    killerBuf[MAX_STRING_LENGTH];

    one_argument( argument, arg );

    if ( !IS_NPC(ch)
    &&   !can_use_skpell( ch, gsn_assassinate ) )    
    {
    send_to_char( "\n\r{CWhy don't you try killing them instead?{x\n\r", ch );
    return;
    }

    if (arg[0] == '\0')
    {
        send_to_char("Assassinate whom?\n\r",ch);
        return;
    }

    if (ch->fighting != NULL)
    {
	send_to_char("You're already fighting!.\n\r",ch);
	return;
    }

    else if ((victim = get_char_room(ch,arg)) == NULL)
    {
        send_to_char("They aren't here.\n\r",ch);
        return;
    }

    if ( victim == ch )
    {
	send_to_char( "How can you assassinate yourself?\n\r", ch );
	return;
    }

    if(IS_SET(ch->in_room->room_flags, ROOM_SAFE))
    {
        send_to_char("\n\r{RNot in a SAFE room!{x\n\r",ch);
        return;
    }

  weapon = get_eq_char( ch, WEAR_WIELD );
  if ( !weapon
  || ( weapon->value[3] != 11
  &&   weapon->value[3] != 34
  &&   weapon->value[3] != 2    
  &&   weapon->value[3] != 1 ) )
    {
    send_to_char( "\n\r{WYou need a weapon that thrusts, pierces, stabs or slices to assassinate with.{x\n\r",ch );
    return;
    }

    if ( is_safe( ch, victim ) )
      return;

    if ( !IS_NPC(victim) && victim->desc == NULL )
    {
        send_to_char("He is link dead. Do not attack him.\n\r", ch);
        return;
    }

     if ( IS_IMMORTAL ( victim ))
     {
        send_to_char( "You think you can assassinate Immortals!!!\n\r", ch);
        act( "$n just tried to assassinate the immortal $N!  What a fool...", ch, NULL, victim, TO_ROOM );
        return;
     }

    if (IS_NPC(victim) &&
	 victim->fighting != NULL && 
	!is_same_group(ch,victim->fighting))
    {
        send_to_char("Kill stealing is not permitted.\n\r",ch);
        return;
    }

    if ( ( obj = get_eq_char( ch, WEAR_WIELD ) ) == NULL)
    {
	send_to_char( "You need to wield a weapon to assassinate.\n\r", ch );
	return;
    }

    if ( victim->hit < victim->max_hit / 5)
    {
	act( "$N is hurt and suspicious ... you can't sneak up.",
	    ch, NULL, victim, TO_CHAR );
	return;
    }
    
    // Check for BINGO
    bingoChance = number_range(1, 10);   // 1 in 10 shot
    if(IS_NPC(victim) && !IS_SET(victim->act, ACT_NOBINGO) && bingoChance==3)
    {
        sprintf(buf, "{GBINGO!!! {WYou have {RASSASSINATED {G%s {W!!\n\r{x", capitalize(victim->name) );
        send_to_char(buf,ch);
        act( "\n\r{WYou see a glint from the shadows and $N falls to the ground, a corpse...\n\r{x", ch, NULL, victim, TO_NOTVICT );

        sprintf ( buf, "{YYou receive {w%d {Yexperience points.\n\r{x", (int)( victim->level * 2.25));
        send_to_char (buf, ch);
        gain_exp(ch, ( victim->level * 2.25)); 

        if (IS_SET(ch->pact, PLR_QUESTOR)
         && IS_NPC(victim))
        {
            if (ch->questmob == victim->pIndexData->vnum)
            {
                send_to_char("You have almost completed your QUEST!\n\r",ch);
                send_to_char("Return to the questmaster before your time runs out!\n\r",ch);
                ch->questmob = -1;
            }
        }

        // Explorer/Killer Percentages
        if(IS_NPC(victim) && victim->pIndexData && victim->pIndexData->vnum && ch->pcdata)
        {
            sprintf(killerBuf, "%d", victim->pIndexData->vnum);
            //sprintf(buf, "you just killed %s", killerBuf);
            //send_to_char(buf, ch);
            if(valid_explorer_killer(ch) && (!ch->pcdata->killed || array_find(ch->pcdata->killed, killerBuf)==-1))
            {
                if(ch->pcdata->alertMe)
                    send_to_char("\n\r{C***{M*** {GYou just killed a new monster!!! {M***{C***{x\n\r", ch);
                ch->pcdata->killed = array_append(ch->pcdata->killed, killerBuf);
            }
        }

        raw_kill(victim,ch);    
    }
    else
    {
        WAIT_STATE( ch, skill_table[gsn_assassinate].beats );
        if ( number_percent( ) < get_skill(ch,gsn_assassinate)
        || ( get_skill(ch,gsn_assassinate) >= 2 && !IS_AWAKE(victim) ) )
        {
        if ( ch->level >= victim->level )
        {
        	if ( get_skill(ch,gsn_assassinate) == 100 )
    		{
        	    
        	    	/* otherwise, attack regular */
        	    	check_improve(ch,gsn_assassinate,TRUE,1);
        	    	multi_hit( ch, victim, gsn_assassinate );
        	    
        	}
        	else
        	{
    
        	    	/* otherwise, attack regular */
        	    	check_improve(ch,gsn_assassinate,TRUE,1);
        	    	multi_hit( ch, victim, gsn_assassinate );
        	    
        	}
        }
        else
        {    	    	
            /* attack regular */
        	check_improve(ch,gsn_assassinate,TRUE,1);
        	multi_hit( ch, victim, gsn_assassinate );
        }
    
        }
        else
        {
    	check_improve(ch,gsn_backstab,FALSE,1);
    	damage( ch, victim, 0, gsn_assassinate,DAM_NONE,TRUE);
        }
    }

    return;
}

void spell_null args(( int sn, int level, CHAR_DATA *ch, void *vo, int target));
/*
 * Control the fights going on.
 * Called periodically by update_handler.
 */
void violence_update( void )
{
    CHAR_DATA *ch;
    CHAR_DATA *ch_next;
    CHAR_DATA *victim;

    for ( ch = char_list; ch != NULL; ch = ch_next )
    {
	ch_next	= ch->next;

	if ( ( victim = ch->fighting ) == NULL || ch->in_room == NULL )
	    continue;

	if ( IS_AWAKE(ch) && ch->in_room == victim->in_room )
	    multi_hit( ch, victim, TYPE_UNDEFINED );
	else
	    stop_fighting( ch, FALSE );

	if ( ( victim = ch->fighting ) == NULL )
	    continue;

	/*
	 * Fun for the whole family!
	 */
	check_assist(ch,victim);

	if ( IS_NPC( ch ) )
	{
	    if ( HAS_TRIGGER( ch, TRIG_FIGHT ) )
		mp_percent_trigger( ch, victim, NULL, NULL, TRIG_FIGHT );
	    if ( HAS_TRIGGER( ch, TRIG_HPCNT ) )
		mp_hprct_trigger( ch, victim );
	}
    }

    return;
}

/* for auto assisting */
void check_assist(CHAR_DATA *ch,CHAR_DATA *victim)
{
    CHAR_DATA *rch, *rch_next;

    for (rch = ch->in_room->people; rch != NULL; rch = rch_next)
    {
	rch_next = rch->next_in_room;
	
	if (IS_AWAKE(rch) && rch->fighting == NULL)
	{

	    /* quick check for ASSIST_PLAYER */
	    if (!IS_NPC(ch) && IS_NPC(rch) 
	    && IS_SET(rch->off_flags,ASSIST_PLAYERS)
	    &&  rch->level + 6 > victim->level)
	    {
		do_function(rch, &do_emote, "screams and attacks!");
		multi_hit(rch,victim,TYPE_UNDEFINED);
		continue;
	    }

	    /* PCs next */
	    if (!IS_NPC(ch) || IS_AFFECTED(ch,AFF_CHARM))
	    {
		if ( ( (!IS_NPC(rch) && IS_SET(rch->pact,PLR_AUTOASSIST))
		||     IS_AFFECTED(rch,AFF_CHARM)) 
		&&   is_same_group(ch,rch) 
		&&   !is_safe(rch, victim))
		    multi_hit (rch,victim,TYPE_UNDEFINED);
		
		continue;
	    }
  	
	    /* now check the NPC cases */
	    
 	    if (IS_NPC(ch) && !IS_AFFECTED(ch,AFF_CHARM))
	
	    {
		if ( (IS_NPC(rch) && IS_SET(rch->off_flags,ASSIST_ALL))

		||   (IS_NPC(rch) && rch->group && rch->group == ch->group)

		||   (IS_NPC(rch) && rch->race == ch->race 
		   && IS_SET(rch->off_flags,ASSIST_RACE))

		||   (IS_NPC(rch) && IS_SET(rch->off_flags,ASSIST_ALIGN)
		   &&   ((IS_GOOD(rch)    && IS_GOOD(ch))
		     ||  (IS_EVIL(rch)    && IS_EVIL(ch))
		     ||  (IS_NEUTRAL(rch) && IS_NEUTRAL(ch)))) 

		||   (rch->pIndexData == ch->pIndexData 
		   && IS_SET(rch->off_flags,ASSIST_VNUM)))

	   	{
		    CHAR_DATA *vch;
		    CHAR_DATA *target;
		    int number;

		    if (number_bits(1) == 0)
			continue;
		
		    target = NULL;
		    number = 0;
		    for (vch = ch->in_room->people; vch; vch = vch->next)
		    {
			if (can_see(rch,vch)
			&&  is_same_group(vch,victim)
			&&  number_range(0,number) == 0)
			{
			    target = vch;
			    number++;
			}
		    }

		    if (target != NULL)
		    {
			do_function(rch, &do_emote, "screams and attacks!");
			multi_hit(rch,target,TYPE_UNDEFINED);
		    }
		}	
	    }
	}
    }
}


/*
 * Do one group of attacks.
 */
void multi_hit( CHAR_DATA *ch, CHAR_DATA *victim, int dt )
{
  int     chance;
  int diceroll;
  OBJ_DATA  *weapon = NULL;

  /* decrement the wait */
  if (ch->desc == NULL)
    ch->wait = UMAX(0,ch->wait - PULSE_VIOLENCE);

  if (ch->daze > 0)
    {
    ch->daze = UMAX(0,ch->daze - PULSE_VIOLENCE); 
    ch->position = POS_STUNNED;
    if (ch->daze <= 0)
      ch->position = POS_SITTING;
    }

  /* no attacks for stunnies -- just a check */
  if (ch->position < POS_RESTING)
    return;


    if ((victim->fighting = ch)
    && (ch->daze <= 0)
    && ((ch->position == POS_SITTING)
    || (ch->position == POS_RESTING)))
      {
      act("{CYou clamber back to your feet!{x",ch,NULL,victim,TO_CHAR);
      act("{C$n{C clambers back to their feet!{x",ch,NULL,victim,TO_ROOM);
      ch->position = POS_STANDING;
      }

  if (IS_NPC(ch))
    {
  	mob_hit(ch,victim,dt);
    return;
    }

  if (IS_NPC(victim) 
  && victim->pIndexData->pShop != NULL)
    {
     send_to_char("\n\r\n\r{RThe Implementors would not like you attacking their SHOPKEEPERS!{x\n\r\n\r",ch); 
     return;
    }

  if (!IS_NPC(victim))
    {
    if (!IS_SET(victim->in_room->room_flags, ROOM_DRAGONPIT))
      ch->pk_timer = 5;
    }

  if (!IS_NPC(ch))
    {

    if (IS_SET(ch->comm, COMM_COMBAT))
      {
       send_to_char("{WFIRST ATTACK RESULTS{w:{x\n\r",ch); 

    if ( (get_skill(ch,gsn_enhanced_damage) > 2 )
    && ( IS_CLASS(ch, CLASS_WARRIOR)
    ||   IS_CLASS(ch, CLASS_RANGER) 
    ||   IS_CLASS(ch, CLASS_PALADIN) 
    ||   IS_CLASS(ch, CLASS_ANTI_PALADIN) 
    ||   IS_CLASS(ch, CLASS_THIEF) 
    ||   IS_CLASS(ch, CLASS_ASSASSIN)))
      {
       diceroll = number_percent();

      if (diceroll <= get_skill(ch,gsn_enhanced_damage))
        {
         send_to_char ("{rYour {RPHYSICAL {rdamage is {RENHANCED{r.{x\n\r",ch); 
        }
      }
    }

    one_hit( ch, victim, dt, FALSE );

    if (get_eq_char (ch, WEAR_SECONDARY)
    && ch->pcdata->learned[gsn_dualwield] > number_percent( ) )
      {
       weapon = get_eq_char( ch, WEAR_SECONDARY );

       if ((dt == gsn_whirlwind)
       && ( weapon->value[3] != 1
       &&   weapon->value[3] != 3
       &&   weapon->value[3] != 5
       &&   weapon->value[3] != 22 ))
         { 
       
         }
       else
         {
          one_hit( ch, victim, dt, TRUE );
          check_improve(ch,gsn_dualwield,TRUE,5);
         }
   }

    if ( ch->fighting != victim )
      return;
     }
    
    if (ch->pcdata->learned[gsn_second_attack] >= 1)
      {
      chance = get_skill(ch,gsn_second_attack);
      if (IS_AFFECTED(ch,AFF_SLOW))
        chance = 0;
      if ( number_percent( ) < chance )
        {


    if (!IS_NPC(ch))
      {
       if (IS_SET(ch->comm, COMM_COMBAT))
      {
       send_to_char("{MSECOND ATTACK RESULTS{w:{x\n\r",ch); 
      }
      }
        one_hit( ch, victim, dt,FALSE );
        check_improve(ch,gsn_second_attack,TRUE,5);
        if (dt != gsn_backstab 
        && dt != gsn_circle 
        && dt != gsn_assassinate
        && dt != gsn_whirlwind )
          {
          if ((get_eq_char (ch, WEAR_SECONDARY))
          && ( IS_NPC( ch )
          || (ch->pcdata->learned[gsn_dualwield] > number_percent( )) ) )
            {
            one_hit( ch, victim, dt,TRUE );
            check_improve(ch,gsn_dualwield,TRUE,5);
            }
          } 
        if ( ch->fighting != victim )
          return;
        }
      }


    if ((ch->pcdata->learned[gsn_third_attack] >= 1) 
    && (ch->class == 4 || ch->pcdata->oldcl == 4 ||
        ch->class == 5 || ch->pcdata->oldcl == 5 ||
        ch->class == 6 || ch->pcdata->oldcl == 6 ||
        ch->class == 7 || ch->pcdata->oldcl == 7 ||
        ch->class == 8 || ch->pcdata->oldcl == 8) )
      {
      chance = get_skill(ch,gsn_third_attack)/2;
      if (IS_AFFECTED(ch,AFF_SLOW))
        chance = 0;
      if ( number_percent( ) < chance )
        {

    if (!IS_NPC(ch))
      {
    if (IS_SET(ch->comm, COMM_COMBAT))
      { 
       send_to_char("{GTHIRD ATTACK RESULTS{w:{x\n\r",ch); 
      }
      }
      	  one_hit( ch, victim, dt, FALSE );
    	  check_improve(ch,gsn_third_attack,TRUE,6);
        if (dt != gsn_backstab 
        && dt != gsn_circle 
        && dt != gsn_assassinate
        && dt != gsn_whirlwind )
          {
          if ((get_eq_char (ch, WEAR_SECONDARY))
          && ( IS_NPC( ch )
          || (ch->pcdata->learned[gsn_dualwield]/2 > number_percent( )) ) )
            {
            one_hit( ch, victim, dt, TRUE );
            check_improve(ch,gsn_dualwield,TRUE,5);
            }
          } 

        if ( ch->fighting != victim )
          return;
        }
      }

    if ((ch->pcdata->learned[gsn_fourth_attack] >= 1) 
    && (ch->class == 5 || ch->pcdata->oldcl == 5) )
      {
       chance = get_skill(ch,gsn_fourth_attack)/1.5;

       if (IS_AFFECTED(ch,AFF_SLOW))
       chance = 0;

       if ( number_percent( ) < chance )
         {

        if (!IS_NPC(ch))
      {
    if (IS_SET(ch->comm, COMM_COMBAT))
      { 
       send_to_char("{BFOURTH ATTACK RESULTS{w:{x\n\r",ch); 
      }
       }
          one_hit( ch, victim, dt,FALSE );
          check_improve(ch,gsn_fourth_attack,TRUE,5);
	  
          if (dt != gsn_backstab 
          && dt != gsn_circle 
          && dt != gsn_assassinate
          && dt != gsn_whirlwind )
            {
             if ((get_eq_char (ch, WEAR_SECONDARY))
             && ( IS_NPC( ch )
             || (ch->pcdata->learned[gsn_dualwield]/2 > number_percent( )) ) )
               {
                one_hit( ch, victim, dt, TRUE );
                check_improve(ch,gsn_dualwield,TRUE,5);
               }
            }
                    
          if ( ch->fighting != victim )
          return;
         }
      }


    if (IS_AFFECTED(ch,AFF_HASTE))
      {

    if (!IS_NPC(ch))
      {
    if (IS_SET(ch->comm, COMM_COMBAT))
      { 
       send_to_char("{CBONUS{D({WHASTE{D) {CATTACK RESULTS{w:{x\n\r",ch);
      }
      }

           
      one_hit(ch,victim,dt,FALSE);

       if (dt != gsn_backstab 
       && dt != gsn_circle 
       && dt != gsn_assassinate
       && dt != gsn_whirlwind) 
         {
          if ((get_eq_char (ch, WEAR_SECONDARY))
          && ( IS_NPC( ch )
          || ch->pcdata->learned[gsn_dualwield] > number_percent( ) ) )
            {
             one_hit(ch,victim,dt, TRUE );
            }
         }

       if ( ch->fighting != victim )
	 return;
      }

    return;
}

/* procedure for all mobile attacks */
void mob_hit (CHAR_DATA *ch, CHAR_DATA *victim, int dt)
{

    OBJ_DATA *dual;
    int chance,number;
    CHAR_DATA *vch, *vch_next;
    dual = get_eq_char( ch, WEAR_SECONDARY );

    if (!IS_NPC(victim))
    {
        if (IS_SET(victim->comm, COMM_COMBAT))
        {
            send_to_char("{RENEMY ATTACK RESULTS{w:{x\n\r",victim); 
        }
    }

    one_hit(ch,victim,dt,FALSE);
    if (ch->fighting != victim)
	return;

    if ( dual )
      {
       one_hit(ch,victim,dt,TRUE);
       if (ch->fighting != victim)
       return;
      }

    /* Area attack -- BALLS nasty! */
 
    if (IS_SET(ch->off_flags,OFF_AREA_ATTACK))
    {
	for (vch = ch->in_room->people; vch != NULL; vch = vch_next)
	{
	    vch_next = vch->next;
	    if ((vch != victim && vch->fighting == ch))
              one_hit(ch,vch,dt,FALSE);
            
            if ( dual )
	      one_hit(ch,vch,dt,TRUE);
	}
    }

    if (IS_AFFECTED(ch,AFF_HASTE) 
    ||  (IS_SET(ch->off_flags,OFF_FAST) 
    && !IS_AFFECTED(ch,AFF_SLOW)))
      {
       one_hit(ch,victim,dt,FALSE);
       if (ch->fighting != victim || dt == gsn_backstab || dt == gsn_circle)
	 return;

       if ( dual )
         {
          one_hit(ch,victim,dt,TRUE);
          if (ch->fighting != victim)
	    return;
         }
      }

    chance = (ch->level);

    if (IS_AFFECTED(ch,AFF_SLOW) 
    && !IS_SET(ch->off_flags,OFF_FAST))
	chance /= 2;

    if (number_percent() < chance)
    {
	one_hit(ch,victim,dt,FALSE);
	if (ch->fighting != victim)
	    return;
    
        if ( dual )
          {
           one_hit(ch,victim,dt,TRUE);
           if (ch->fighting != victim)
             return;
          }
       } 

  if (IS_SET(ch->act,ACT_WARRIOR)
  || IS_SET(ch->act,ACT_PALADIN)
  || IS_SET(ch->act,ACT_ASSASSIN))
    {
     chance = (ch->level/1.15);

     if (IS_AFFECTED(ch,AFF_SLOW) 
     && !IS_SET(ch->off_flags,OFF_FAST))
     chance = 0;

     if (number_percent() < chance)
       {
        one_hit(ch,victim,dt,FALSE);
        if (ch->fighting != victim)
        return;
       
        if ( dual )
          {
           one_hit(ch,victim,dt,TRUE);
           if (ch->fighting != victim)
           return;
          }
       }
    }

  if (IS_SET(ch->act,ACT_WARRIOR))
    {
     chance = (ch->level/1.25);

     if (IS_AFFECTED(ch,AFF_SLOW) 
     && !IS_SET(ch->off_flags,OFF_FAST))
     chance = 0;

     if (number_percent() < chance)
       {
        one_hit(ch,victim,dt,FALSE);
        if (ch->fighting != victim)
        return;
    
        if ( dual )
          {
           one_hit(ch,victim,dt,TRUE);
           if (ch->fighting != victim)
           return;
          }
       }
    }

    /* oh boy!  Fun stuff! */

    if (ch->wait > 0)
	return;

    number = number_range(0,2);

    if (number == 1 && IS_SET(ch->act,ACT_MAGE))
    {
	/*  { mob_cast_mage(ch,victim); return; } */ ;
    }

    if (number == 2 && IS_SET(ch->act,ACT_CLERIC))
    {	
	/* { mob_cast_cleric(ch,victim); return; } */ ;
    }

    /* now for the skills */

    number = number_range(0,8);

    switch(number) 
    {
    case (0) :
	if (IS_SET(ch->off_flags,OFF_BASH))
	    do_function(ch, &do_bash, "");
	break;

    case (1) :
	if (IS_SET(ch->off_flags,OFF_BERSERK) && !IS_AFFECTED(ch,AFF_BERSERK))
	    do_function(ch, &do_berserk, "");
	break;


    case (2) :
	if (IS_SET(ch->off_flags,OFF_DISARM) 
	|| (get_weapon_sn(ch, FALSE) != gsn_hand_to_hand 
	&& (IS_SET(ch->act,ACT_WARRIOR)
   	||  IS_SET(ch->act,ACT_THIEF))))
	    do_function(ch, &do_disarm, "");
	break;

    case (3) :
	if (IS_SET(ch->off_flags,OFF_KICK))
	    do_function(ch, &do_kick, "");
	break;

    case (4) :
	if (IS_SET(ch->off_flags,OFF_KICK_DIRT))
	    do_function(ch, &do_dirt, "");
	break;

    case (5) :
	if (IS_SET(ch->off_flags,OFF_TAIL))
	{
	    /* do_function(ch, &do_tail, "") */ ;
	}
	break; 

    case (6) :
	if (IS_SET(ch->off_flags,OFF_TRIP))
	    do_function(ch, &do_trip, "");
	break;

    case (7) :
	if (IS_SET(ch->off_flags,OFF_CRUSH))
	{
	    /* do_function(ch, &do_crush, "") */ ;
	}
	break;
    case (8) :
	if (IS_SET(ch->off_flags,OFF_BACKSTAB))
	{
	    do_function(ch, &do_backstab, "");
	}
    }
}

	
/* new version of one_hit, includes DUALWIELDing */
void one_hit( CHAR_DATA *ch, CHAR_DATA *victim, int dt, bool secondary )
{
    OBJ_DATA *wield;
    OBJ_DATA *weapon; 
    int victim_ac;
    int thac0;
    int thac0_00;
    int thac0_32;
    int dam;
    int clvl;
    int diceroll;
    int sn,skill;
    int dam_type;
    bool result;

    sn = -1;

    clvl = ch->level;

		/* just in case */
    if (victim == ch || ch == NULL || victim == NULL)
     return;

    if ( victim->position == POS_DEAD || ch->in_room != victim->in_room )
     return;

	if (check_valid_attack == FALSE)
	{
		send_to_char("{RYou may not attack{x\n\r",ch);
		return;
	}

/* 
    if (!IS_IMP(ch))
      {
    if(IS_SET(victim->in_room->room_flags, ROOM_SAFE))
      {
       send_to_char("\n\r{CNot in this room!{x\n\r",ch);
       return; 
      }
      }
*/

    if(IS_SET(victim->in_room->room_flags, ROOM_SAFE))
      {
       send_to_char("\n\r{CNot in this room!{x\n\r",ch);
       return; 
      }

/* Check if players are CHOOSEN */


if (!IS_NPC(ch))
   {
  if (IS_SET(ch->comm, COMM_AFK))
    {
     send_to_char("AFK mode removed due to COMBAT. Type 'replay' to see tells.\n\r",ch);
      REMOVE_BIT(ch->comm,COMM_AFK);
    }
   }


if (!IS_NPC(victim))
{
if (!IS_NPC(ch))
  {
   if (ch->level < MAX_LEVEL)
     {
      if(!IS_SET(ch->in_room->room_flags, ROOM_DRAGONPIT))
      {
          if (!IS_SET(victim->pact, PLR_DRAGONPIT))
            {
             if (!IS_SET(ch->pact, PLR_DRAGONPIT))
               {
                if (!IS_SET(ch->pact, PLR_PKILLER))
                  {
                   send_to_char("\n\r{WYou must type {RCHOOSE{W and follow those{x\n\r",ch);
                   send_to_char("{Winstructions if you wish to PKILL.{x\n\r",ch);
                   stop_fighting (ch, TRUE);
                   return;
                  } 
    
                if (!IS_SET(victim->pact, PLR_PKILLER))
                  {
                   send_to_char("\n\r{WYour target has not used the {RCHOOSE{W command yet.{x\n\r",ch);
                   stop_fighting (ch, TRUE);
                   return;
                  }
               }
            }
        }
     } 
  }}

    /*
	Figure out the type of damage message.
        if secondary == true, use the second weapon.
     */

    if (!secondary)
        wield = get_eq_char( ch, WEAR_WIELD );
    else  
        wield = get_eq_char( ch, WEAR_SECONDARY );

    if ( dt == TYPE_UNDEFINED )
	 {
     dt = TYPE_HIT;
	  if ( wield != NULL && wield->item_type == ITEM_WEAPON )
			dt += wield->value[3];
     else 
         dt += ch->dam_type;
         }


    if (IS_IMP(ch)
    && IS_SET(ch->comm, COMM_COMBAT))
      dam_type = DAM_IMP;
    else
      {    
    if (dt < TYPE_HIT)
	  if (wield != NULL)
         dam_type = attack_table[wield->value[3]].damage;
     else
         dam_type = attack_table[ch->dam_type].damage;
    else
     dam_type = attack_table[dt - TYPE_HIT].damage;

	 if (dam_type == -1)
     dam_type = DAM_PUNCH;
      }


      /* get the weapon skill */
    sn = get_weapon_sn(ch,secondary);
    skill = 20 + get_weapon_skill(ch,sn);

     /*
     * Calculate to-hit-armor-class-0 versus armor.
     */

    if ( IS_NPC(ch) )
	 {
     thac0_00 = 10;
     thac0_32 = -10;

     if (IS_SET(ch->act,ACT_WARRIOR))
                 thac0_32 = -26;
     else if (IS_SET(ch->act,ACT_ANTIPALADIN))
                 thac0_32 = -20;
     else if (IS_SET(ch->act,ACT_PALADIN))
                 thac0_32 = -18;
     else if (IS_SET(ch->act,ACT_RANGER))
                 thac0_32 = -18;
     else if (IS_SET(ch->act,ACT_ASSASSIN))
                 thac0_32 = -10;
     else if (IS_SET(ch->act,ACT_THIEF))
                 thac0_32 = -10;
     else if (IS_SET(ch->act,ACT_PSIONIC))
                 thac0_32 = -5;
     else if (IS_SET(ch->act,ACT_CLERIC))
                 thac0_32 = 0;
     else if (IS_SET(ch->act,ACT_MAGE))
                 thac0_32 = 4;

    	}
    else
    {
	  thac0_00 = class_table[ch->class].thac0_00;
     thac0_32 = class_table[ch->class].thac0_32;
    }


    thac0  = interpolate( ch->level, thac0_00, thac0_32 );

    if (thac0 < 0)
        thac0 = thac0/2;

    if (thac0 < -5)
        thac0 = -5 + (thac0 + 5) / 2;

    thac0 -= GET_HITROLL(ch) * skill/100;
	 thac0 += 5 * (100 - skill) / 100;

    if (dt == gsn_critical_strike)
	thac0 -= 20 * (100 - get_skill(ch,gsn_critical_strike));
    if (dt == gsn_assassinate)
	thac0 -= 14 * (100 - get_skill(ch,gsn_assassinate));
    if (dt == gsn_whirlwind)
	thac0 -= 9 * (100 - get_skill(ch,gsn_whirlwind));
    if (dt == gsn_backstab)
	thac0 -= 8 * (100 - get_skill(ch,gsn_backstab));
    if (dt == gsn_circle)
	thac0 -= 8 * (100 - get_skill(ch,gsn_circle));


/* wizzle  add other DAM_ types */

    switch(dam_type)
      {
     case(DAM_PIERCE):victim_ac = GET_AC(victim,AC_PIERCE)/10;   break;
     case(DAM_BASH):      victim_ac = GET_AC(victim,AC_BASH)/10;      break;
     case(DAM_SLASH): victim_ac = GET_AC(victim,AC_SLASH)/10;    break;
     default:   victim_ac = GET_AC(victim,AC_EXOTIC)/10;    break;
      }

     
    if (victim_ac < -15)
     victim_ac = (victim_ac + 15) / 5 - 15;
     
	 if ( !can_see( ch, victim ) )
     victim_ac -= 300;

	 if ( victim->position < POS_FIGHTING)
     victim_ac += 100;

    if (victim->position < POS_RESTING)
     victim_ac += 300;

	 while ( ( diceroll = number_bits( 5 ) ) >= 20 );

    if (!IS_IMP(ch))
      {
    if ( diceroll == 0
	 || ( diceroll != 19 && diceroll < thac0 - victim_ac ) )
    {
	  /* Miss. */
     damage( ch, victim, 0, dt, dam_type, TRUE );
     tail_chain( );
	  return;
    }
      }

    /*
     * Hit.                   
     * Calc damage.
     */

/*
    if ( IS_NPC(ch) 
    && (!ch->pIndexData->new_format 
    || wield == NULL)) 
*/

    if ( IS_NPC(ch)) 
      {
       if (!ch->pIndexData->new_format)
	 {
          dam = number_range( ch->level / 2, ch->level * 1.5 );
	
          if ( wield != NULL )
             dam += dam / 2;
          }
        else
         dam = dice(ch->damage[DICE_NUMBER],ch->damage[DICE_TYPE]);
      }
     else
	 {
          if (sn != -1)
          check_improve(ch,sn,TRUE,5);
     
          if ( wield != NULL )
            {
             if (wield->pIndexData->new_format)
             dam = dice(wield->value[1],wield->value[2]) * skill/100;
            else
             dam = number_range( wield->value[1], wield->value[2]) * skill/100;
            }
          else
           if ((get_skill(ch,gsn_hand_to_hand) >= 75)
           && (IS_CLASS(ch, CLASS_WARRIOR)
           || IS_CLASS(ch, CLASS_PALADIN)
           || IS_CLASS(ch, CLASS_ANTI_PALADIN)
           || IS_CLASS(ch, CLASS_RANGER)) )
dam = number_range(((get_skill(ch,gsn_hand_to_hand) /10) * (ch->level /20)),( (get_skill(ch,gsn_hand_to_hand)/4) * (ch->level /20))); 
          else   
dam = number_range(((get_skill(ch,gsn_hand_to_hand) /20) * (ch->level /33)),( (get_skill(ch,gsn_hand_to_hand)/10) * (ch->level /20))); 
	 }

	 /*
	  * Bonuses.
         */

  


  if ( dt == gsn_assassinate && wield != NULL)
    {
      if ( wield->value[0] != 2 )
	dam -= dam/2;
      else 
      {
        diceroll = number_percent();

	if (diceroll <= get_skill(ch,gsn_assassinate))
          {
           if (ch->level >= 50)
              dam += ((clvl * 3) + (get_skill(ch,gsn_assassinate) * 6));
            else
              dam += (clvl * 2);

           check_improve(ch,gsn_assassinate,TRUE,6);
          }
       }  
     }

  if ( dt == gsn_backstab && wield != NULL)
     {
      if ( wield->value[0] != 2 )
	  {
		  	
		  dam += dam*2;
	  }
      else 
      {
		  
		  diceroll = number_percent();

	if (diceroll <= get_skill(ch,gsn_backstab))
          {
           if (ch->level >= 50)
              dam += ((clvl * 3) + (get_skill(ch,gsn_backstab) * (3)) ); //Yavi 6/30/2004 *Update
		   else
              dam += (clvl);

           check_improve(ch,gsn_backstab,TRUE,6);
          }
       }  
     }

    if ( dt == gsn_circle && wield != NULL)
      {       
       if ( wield->value[0] != 2 )
 	  dam -= dam/2;
        else
        {
        diceroll = number_percent();

	if (diceroll <= get_skill(ch,gsn_circle))
          {
           if (ch->level >= 60)
              dam += ((clvl * 2) + (get_skill(ch,gsn_circle) * (3/2)) );
            else
              dam += (clvl / 2);

           check_improve(ch,gsn_circle,TRUE,6);
          }
       }  
    }

  if ( dt == gsn_whirlwind && wield != NULL)
    {
     if ( ch->fighting == NULL )
       dam += 0;
      else
          {
        diceroll = number_percent();

	if (diceroll <= get_skill(ch,gsn_whirlwind))
          {

           if (ch->level >= 45)
              dam += ((clvl * 2) + (get_skill(ch,gsn_whirlwind) * (3/2)) );
            else
              dam += (clvl / 2);

           check_improve(ch,gsn_whirlwind,TRUE,6);
          }
       }  
    } 

   if ( dt == gsn_critical_strike && wield != NULL)
     { 
     if ( ch->fighting == NULL )
       dam += 0;
     else
      {
        diceroll = number_percent();

	if (diceroll <= get_skill(ch,gsn_critical_strike))
          {
           if (ch->level >= 65)
              dam += ((clvl * 3) + get_skill(ch,gsn_critical_strike));
            else
              dam += (clvl / 2);

           check_improve(ch,gsn_critical_strike,TRUE,6);
          }
       }  
    }


   if (get_eq_char(ch, WEAR_WIELD) != NULL)
    {

    if (!secondary)
        weapon = get_eq_char( ch, WEAR_WIELD );
    else  
        weapon = get_eq_char( ch, WEAR_SECONDARY );
       
     if (IS_WEAPON_STAT(weapon,WEAPON_VORPAL)
     ||  IS_WEAPON_STAT(weapon,WEAPON_SHARP))
       {
        if (IS_WEAPON_STAT(weapon,WEAPON_VORPAL)
        && ( weapon->value[3] == 1 
        ||   weapon->value[3] == 3 
        ||   weapon->value[3] == 21))
          dam += ( (wield->level/2) * 2 );
      else
        if (IS_WEAPON_STAT(weapon,WEAPON_SHARP)
        && ( weapon->value[3] == 2 
        ||   weapon->value[3] == 5 
        ||   weapon->value[3] == 11 
        ||   weapon->value[3] == 26 
        ||   weapon->value[3] == 34 ))
          dam += ( (wield->level/4) * 2 );
       }
     }


    if ( (get_skill(ch,gsn_enhanced_damage) > 2 )
    && ( IS_CLASS(ch, CLASS_WARRIOR)
    ||   IS_CLASS(ch, CLASS_RANGER) 
    ||   IS_CLASS(ch, CLASS_PALADIN) 
    ||   IS_CLASS(ch, CLASS_ANTI_PALADIN) 
    ||   IS_CLASS(ch, CLASS_THIEF) 
    ||   IS_CLASS(ch, CLASS_ASSASSIN)))
      {
       if ( IS_CLASS(ch, CLASS_WARRIOR))
        {

    if (dam_type == DAM_BASH
    ||  dam_type == DAM_PIERCE
    ||  dam_type == DAM_SLASH
    ||  dam_type == DAM_CRUSH
    ||  dam_type == DAM_CLEAVE
    ||  dam_type == DAM_PUNCH
    ||  dam_type == DAM_BITE
    ||  dam_type == DAM_CLAW
    ||  dam_type == DAM_IMP ) 
      {
         diceroll = number_percent();
  
	 if (diceroll <= get_skill(ch,gsn_enhanced_damage))
         {
         if (ch->level >= 50)
  	   dam += ( (clvl * 2 ) + (8 * str_app[get_curr_stat(ch,STAT_STR)].todam));
         else
  	   dam += ( (clvl / 2) + (2 * str_app[get_curr_stat(ch,STAT_STR)].todam));

        check_improve(ch,gsn_enhanced_damage,TRUE,6);
         }
      }

        }
     else
    if ( IS_CLASS(ch, CLASS_RANGER) 
    ||   IS_CLASS(ch, CLASS_PALADIN) 
    ||   IS_CLASS(ch, CLASS_ANTI_PALADIN))
      {

    if (dam_type == DAM_BASH
    ||  dam_type == DAM_PIERCE
    ||  dam_type == DAM_SLASH
    ||  dam_type == DAM_CRUSH
    ||  dam_type == DAM_CLEAVE
    ||  dam_type == DAM_PUNCH
    ||  dam_type == DAM_BITE
    ||  dam_type == DAM_CLAW
    ||  dam_type == DAM_IMP ) 
      {
        diceroll = number_percent();
	if (diceroll <= get_skill(ch,gsn_enhanced_damage))
          {

         if (ch->level >= 65)
  	   dam += ((clvl / 5)  + ( 4 * str_app[get_curr_stat(ch,STAT_STR)].todam));
         else
  	   dam += ((clvl / 10) + ( 2 * str_app[get_curr_stat(ch,STAT_STR)].todam));

         check_improve(ch,gsn_enhanced_damage,TRUE,6);
          }
      }
    }
  else
    if ( IS_CLASS(ch, CLASS_THIEF) 
    ||   IS_CLASS(ch, CLASS_ASSASSIN) )
      {
    if (dam_type == DAM_BASH
    ||  dam_type == DAM_PIERCE
    ||  dam_type == DAM_SLASH
    ||  dam_type == DAM_CRUSH
    ||  dam_type == DAM_CLEAVE
    ||  dam_type == DAM_PUNCH
    ||  dam_type == DAM_BITE
    ||  dam_type == DAM_CLAW
    ||  dam_type == DAM_IMP ) 
      {
        diceroll = number_percent();
	if (diceroll <= get_skill(ch,gsn_enhanced_damage))
        {
         if (ch->level >= 65)
  	   dam += ((clvl / 8)  + ( 2 * str_app[get_curr_stat(ch,STAT_STR)].todam));
         else
  	   dam += ((clvl / 10) + ( str_app[get_curr_stat(ch,STAT_STR)].todam));

         check_improve(ch,gsn_enhanced_damage,TRUE,6);
        }
      }

      }
    }

    result = damage( ch, victim, dam, dt, dam_type, TRUE );

    if ( result && secondary )
      check_improve(ch,gsn_dualwield,TRUE,3);
    else if ( secondary )
      check_improve(ch,gsn_dualwield,FALSE,4);


    /* but do we have a funky weapon? */
    if (result && wield != NULL)
	 {

     int dam;

     if (ch->fighting == victim && IS_WEAPON_STAT(wield,WEAPON_POISON))
	  {
         int level;
           AFFECT_DATA *poison, af;

	   if ((poison = affect_find(wield->affected,gsn_poison)) == NULL)
	     level = wield->level;
           else
             level = poison->level;
     
         if (!saves_spell(level / 2,victim,DAM_POISON)) 
         {
          send_to_char("{GYou feel poison coursing through your veins.{x",
				  victim);
          act("{G$n is poisoned by the venom on $p.{x",
              victim,wield,NULL,TO_ROOM);

          af.where     = TO_AFFECTS;
          af.type      = gsn_poison;
          af.level     = level;
          af.duration  = level;
          af.location  = APPLY_STR;
          af.modifier  = -(wield->level/ 7 );
          af.bitvector = AFF_POISON;
          affect_join( victim, &af );
         }

         /* weaken the poison if it's temporary */
         if (poison != NULL)
         {
          poison->level = UMAX(0,poison->level - 2);
			 poison->duration = UMAX(0,poison->duration - 1);
     
          if (poison->level == 0 || poison->duration == 0)
              act("{GThe poison on $p has worn off.{x",ch,wield,NULL,TO_CHAR);
         }
       }


     if (ch->fighting == victim && IS_WEAPON_STAT(wield,WEAPON_VAMPIRIC))
        {
         dam = number_range(1, wield->level);
         act("$p draws life from $n.",victim,wield,NULL,TO_ROOM);
         act("You feel $p drawing your life away.",
         victim,wield,NULL,TO_CHAR);
         damage(ch,victim,dam,0,DAM_NEGATIVE,FALSE);
         ch->alignment = UMAX(-1000,ch->alignment - 1);
         ch->hit += dam/2;
        }

     if (ch->fighting == victim && IS_WEAPON_STAT(wield,WEAPON_FLAMING))
     {
         dam = number_range(1,wield->level);
         act("$n is {Rburned{x by $p.",victim,wield,NULL,TO_ROOM);
         act("$p sears your flesh.",victim,wield,NULL,TO_CHAR);
         fire_effect( (void *) victim,wield->level/2,dam,TARGET_CHAR);
         damage(ch,victim,dam,0,DAM_FIRE,FALSE);
     }

     if (ch->fighting == victim && IS_WEAPON_STAT(wield,WEAPON_FROST))
	  {
         dam = number_range(1,wield->level);
         act("{B$p freezes $n.{x",victim,wield,NULL,TO_ROOM);
         act("{BThe cold touch of $p surrounds you with ice.{x",
          victim,wield,NULL,TO_CHAR);
         cold_effect(victim,wield->level/2,dam,TARGET_CHAR);
         damage(ch,victim,dam,0,DAM_COLD,FALSE);
	  }

     if (ch->fighting == victim && IS_WEAPON_STAT(wield,WEAPON_SHOCKING))
     {
         dam = number_range(1,wield->level);
         act("{Y$n is struck by lightning from $p.{x",victim,wield,NULL,TO_ROOM);
         act("{YYou are shocked by $p.{x",victim,wield,NULL,TO_CHAR);
         shock_effect(victim,wield->level/2,dam,TARGET_CHAR);
         damage(ch,victim,dam,0,DAM_LIGHTNING,FALSE); 
     }

   }

	 tail_chain( );
	 return;
}



/*
 * Inflict damage from a hit.
 */
bool damage(CHAR_DATA *ch,CHAR_DATA *victim,int dam,int dt,int dam_type, bool show) 
{
  char buf[MAX_STRING_LENGTH];
  int diceroll;
  int clvl;
  OBJ_DATA *corpse;
  bool immune;
    DESCRIPTOR_DATA *d;
    CHAR_DATA * original;
  

  if ( victim->position == POS_DEAD )
    return FALSE;

   immune = FALSE;

    if ( dam <= 0 )
	  dam = 1;

	if (check_valid_attack(ch,victim) == FALSE)
	{
		send_to_char("{RAttack not allowed.{x\n\r",ch);
		 if ( ch->fighting != NULL )
			stop_fighting( ch, TRUE );
		 return FALSE;
	}

   if (!IS_IMP(ch))
     {
      if(!IS_NPC(ch))
        {
      if (ch->level > 100)
        clvl = 100;
        }
     }
	
    if ( victim != ch )
    {
	if ( is_safe( ch, victim ) )
	    return FALSE;

	if ( victim->position > POS_STUNNED )
	{
	    if ( victim->fighting == NULL )
	    {
		set_fighting( victim, ch );
		if ( IS_NPC( victim ) && HAS_TRIGGER( victim, TRIG_KILL ) )
		    mp_percent_trigger( victim, ch, NULL, NULL, TRIG_KILL );
	    }
	    if (victim->timer <= 4)
	    	victim->position = POS_FIGHTING;
	}

	if ( victim->position > POS_STUNNED )
	{
	    if ( ch->fighting == NULL )
		set_fighting( ch, victim );
	}

	if ( victim->master == ch )
	    stop_follower( victim );
    }

    /*
     * Inviso attacks ... not.
     */

    if (!IS_IMP(ch))
      {
    if ( IS_AFFECTED(ch, AFF_INVISIBLE) 
    ||   IS_AFFECTED(ch, AFF_HIDE) 
    ||   IS_AFFECTED(ch, AFF_SNEAK))
    {
	affect_strip( ch, gsn_invisibility );
	affect_strip( ch, gsn_hide );
	affect_strip( ch, gsn_sneak );
	affect_strip( ch, gsn_obfuscate );
	affect_strip( ch, gsn_mass_invis );
	affect_strip( ch, gsn_chameleon_power);
	affect_strip( ch, gsn_shadow_form );
	REMOVE_BIT( ch->affected_by, AFF_INVISIBLE );
	REMOVE_BIT( ch->affected_by, AFF_HIDE);
	REMOVE_BIT( ch->affected_by, AFF_SNEAK );
	act( "$n fades into existence.", ch, NULL, NULL, TO_ROOM );
    }
      }

    /*
     * Damage modifiers.
     */

  if ( is_sn( dt )
  &&   !IS_NPC( ch )
  && (dam_type == DAM_MAGIC)
  &&   skill_table[dt].spell_fun != spell_null
  && ( IS_CLASS( ch, CLASS_MAGE )
  ||   IS_CLASS( ch, CLASS_CLERIC ) ) )
    {
    if( get_skill(ch,gsn_spellcraft) >= 2 )
      {
      diceroll = number_percent();
      if (diceroll <= get_skill(ch,gsn_spellcraft))
        {
        dam += dam/3;
        check_improve(ch,gsn_spellcraft,TRUE,2);
        }
      }
    }


    if ( dam > 1 && !IS_NPC(victim) 
    &&   victim->pcdata->condition[COND_DRUNK]  > 10 )
	dam += (victim->pcdata->condition[COND_DRUNK] / 25) * (victim->level);

    if ( !IS_AWAKE(victim) )
	  dam += (dam/2);
     else 
    if (victim->position < POS_FIGHTING)
     dam += (dam/4);


if (!IS_IMP(ch))
  {
if (!IS_NPC(ch))
  {
  if ( dam_type == DAM_BASH
  || dam_type == DAM_PIERCE
  || dam_type == DAM_SLASH
  || dam_type == DAM_MAIM
  || dam_type == DAM_CRUSH
  || dam_type == DAM_CLEAVE
  || dam_type == DAM_PUNCH
  || dam_type == DAM_BITE
  || dam_type == DAM_CLAW
  || dam_type == DAM_IMP)
    {
     dam += GET_DAMROLL(ch); 
    }
  }
 else
   dam += GET_DAMROLL(ch); 
 }


    if (!IS_IMP(ch))
      {
    if ( dt >= TYPE_HIT && ch != victim)
    {
        if ( check_parry( ch, victim ) )
	    return FALSE;
	if ( check_dodge( ch, victim ) )
	    return FALSE;
	if ( check_shield_block(ch,victim))
	    return FALSE;

    }

    immune = FALSE;
    switch(check_immune(victim,dam_type))
    {
	case(IS_IMMUNE):
	    immune = TRUE;
	    dam = 0;
	    break;
	case(IS_RESISTANT):	
	    dam -= dam/3;
	    break;
    }
      }


    if ( dam > 1 && ((IS_AFFECTED(victim, AFF_PROTECT_EVIL) && IS_EVIL(ch) )
    ||		     (IS_AFFECTED(victim, AFF_PROTECT_GOOD) && IS_GOOD(ch) )))
	dam -= dam / 3;

    if ( dam > 1 && IS_AFFECTED(victim, AFF_SANCTUARY) )
	dam /= 2;

    if ( dam > 1 && IS_AFFECTED( victim, AFF_GOLDEN ) )
      dam /= 4;



   if (!IS_IMP(ch))
     {
    immune = FALSE;
    switch(check_immune(victim,dam_type))
    {
	case(IS_VULNERABLE):
	    dam += dam/2;
	    break;
    }
     }
/*player vs player cutdown*/

if (!IS_NPC(ch) && !IS_NPC(victim))   { dam *= 0.8; }
    
/** Check for parry, and dodge.*/

    if (dam < 0)
        dam = 0;


    if (show)
    	dam_message( ch, victim, dam, dt, immune );

    if (dam <= 0)
	return FALSE;

    /*
     * Hurt the victim.
     * Inform the victim of his new state.
     */

if (victim->in_room != get_room_index(ROOM_VNUM_DRAGONPIT_RETURN))
  {
    victim->hit -= dam;

    if ( !IS_NPC(victim)
    &&   victim->level >= LEVEL_IMMORTAL
    &&   victim->hit < 1 
    && (!IS_SET(victim->pact, PLR_DRAGONPIT)))
	victim->hit = 1;

    update_pos( victim );

    switch( victim->position )
    {
    case POS_MORTAL:
	act( "$n is mortally wounded, and will die soon, if not aided.",
	    victim, NULL, NULL, TO_ROOM );
	send_to_char( 
	    "You are mortally wounded, and will die soon, if not aided.\n\r",
	    victim );
	break;

    case POS_INCAP:
	act( "$n is incapacitated and will slowly die, if not aided.", victim, NULL, NULL, TO_ROOM );
	send_to_char( "You are incapacitated and will slowly die, if not aided.\n\r", victim );
	sprintf(buf, "Help me! I have become incapacitated! If someone doesn't get to {x%s{C quickly, I will die!!!", victim->in_room->name);
	talk_channel(victim, buf, CHANNEL_CHAT, "chat" );
	if(number_range(1, 100)<=5)
	{
	   do_restore(victim,"self");
       send_to_char("The Gods hear your plea and grant you mercy!", victim);
	   talk_channel(victim, "The gods have heard my plea and have granted me mercy!", CHANNEL_CHAT, "chat" );
	}
	break;

    case POS_STUNNED:
	act( "$n is stunned, but will probably recover.",
	    victim, NULL, NULL, TO_ROOM );
	send_to_char("You are stunned, but will probably recover.\n\r",
	    victim );
	break;

    case POS_DEAD:
	act( "$n is DEAD!!", victim, 0, 0, TO_ROOM );
	send_to_char( "{rYou have been KILLED!!{x\n\r\n\r", victim );
	break;

    default:
	if ( dam > victim->max_hit / 4 )
	    send_to_char( "That really did {RHURT{x!\n\r", victim );
	if ( victim->hit < victim->max_hit / 4 )
	    send_to_char( "You sure are {rBLEEDING{x!\n\r", victim );
	break;
    }
 }

    /*
     * Sleep spells and extremely wounded folks.
     */
    if ( !IS_AWAKE(victim) )
	stop_fighting( victim, FALSE );


    /*
     * Payoff for killing things.
     */

if (!IS_NPC(ch)
&& !IS_NPC(victim)
&& victim->pcdata->bounty > 0)
{
    sprintf(buf,"\n\r{CYou receive a {Y%d GOLD {CBounty, for Killing {W%s{C!{x\n\r\n\r",
    victim->pcdata->bounty, victim->name);
    send_to_char(buf, ch);
    ch->gold += victim->pcdata->bounty;
    victim->pcdata->bounty =0;
}


    if ( victim->position == POS_DEAD )
    {
        group_gain( ch, victim ); 

    if (!IS_NPC(victim))
      {
      if(!IS_SET(victim->in_room->room_flags, ROOM_DRAGONPIT))
      {
          if (!IS_SET(victim->pact, PLR_DRAGONPIT))
            {
    
             if (!IS_NPC( ch ))
              {
                update_pkinfo( ch, victim );
    	  }
     
            if(!IS_NPC(ch) && !IS_NPC(victim))
            {
                sprintf(buf, "{WThe grim reaper shouts, \"{G%s{W has been slaughtered by {G%s{W\"!!!{x\n\r", victim->name, ch->name);
                for(d=descriptor_list;d;d=d->next)
                {
                	original = d->original ? d->original : d->character; /* if switched */
                	if(d->connected==CON_PLAYING)
                	  act_new(buf, original, NULL, NULL, TO_CHAR, POS_DEAD);
                }
            }
                
            
            
               sprintf( log_buf, "%s killed by %s at %d",
    		victim->name,
    		(IS_NPC(ch) ? ch->short_descr : ch->name),
    		ch->in_room->vnum );
    	    log_string( log_buf );
    
    
          if (!IS_SET(victim->in_room->room_flags, ROOM_DRAGONPIT) && !IS_SET(victim->pact, PLR_DRAGONPIT) &&
    		  !IS_SET(victim->in_room->room_flags,ROOM_GAUNTLET))
                        { 
    if ( victim->exp > exp_per_level(victim,victim->pcdata->points) * victim->level )
     	gain_exp( victim, (2 * (exp_per_level(victim,victim->pcdata->points)
    			         * victim->level - victim->exp)/3) + 50 );
    					}
    
    	}
    	}
      }

      if (!IS_SET(victim->in_room->room_flags, ROOM_DRAGONPIT) && !IS_SET(victim->pact, PLR_DRAGONPIT))
       {
         /*sprintf( log_buf, "%s got toasted by %s at %s [room %d]",
            (IS_NPC(victim) ? victim->short_descr : victim->name),
            (IS_NPC(ch) ? ch->short_descr : ch->name),
            ch->in_room ? ch->in_room->name : "somewhere", ch->in_room ? ch->in_room->vnum : 0);*/
	    //log_string( log_buf );
       }
     else if(IS_SET(victim->pact, PLR_DRAGONPIT))
       {
          DESCRIPTOR_DATA *d;
          char buf[MAX_STRING_LENGTH];
        
          indp --;
        
          for (d = descriptor_list; d != NULL; d = d->next)
             {
              if (d->connected == CON_PLAYING)
                {
                 if(!IS_SET(d->character->deaf, CHANNEL_DPTALK))
                   {
        sprintf(buf, "\n\r\n\r              {RThe DragonLord announces that....{x\n\r"
                  "{W%s {rwas just {RELIMINATED {rby {W%s {rfrom the DragonPIT!!\r"
                  "{rLooks like {W%s {rwas {RUNWORTHY{r of this DragonPIT!!\r"
                  "{rBetter Luck next time {GLOSER{r... !!{x\n\r\n\r",
        victim->name,ch->name,victim->name);
        send_to_char(buf, d->character);
                   } 
                }
             }
        
          if (indp == 1)
          {
          for (d = descriptor_list; d != NULL; d = d->next)
             {   
              if (d->connected == CON_PLAYING)
                {
               if(!IS_SET(d->character->deaf, CHANNEL_DPTALK))
                 {
        sprintf(buf, "\n\r\n\r              {RThe DragonLord announces that...\n\r"
                  "{W%s {rhas emerged VICTORIOUS!!!\r "
        	  "{r%s is a true {RDragonWarrior{r of The Knights of the Red Dragon!!!\r"
                  "{rThe Jackpot Prize %s receives is a total of {W%d {Ygp{r!!{x\n\r\n\r",
        ch->name,ch->sex == 1 ? "He" : ch->sex == 2 ? "She" : "It",ch->sex==1?"He":ch->sex == 2 ?"She":"It",
         (jackpot-(jackpot/10)));
        send_to_char(buf, d->character);
                 }
               }
             }
          
            ch->gold += (jackpot-(jackpot/10));
            indp = -1;
            dptimer = -1;
            dptimeleft = -1;
            dptype = 0;
            min_level = 0;           
            max_level = 0;
            cost_level = 0;
            jackpot = 0;
        
          if (IS_SET(ch->pact, PLR_DRAGONPIT))
          { 
             REMOVE_BIT(ch->pact, PLR_DRAGONPIT);
            }
        
            char_from_room(ch); 
            char_to_room(ch, get_room_index(ROOM_VNUM_DRAGONPIT_RETURN));
            do_restore(ch,"self");
            do_look(ch, "auto");
          }
        }


//Begin Gauntlet Death Code

   if (IS_SET(victim->in_room->room_flags,ROOM_GAUNTLET) && (!IS_NPC(victim)))
   {
			char_from_room(victim);
			char_to_room( victim, get_room_index(ROOM_VNUM_GAUNTLET_RETURN));
			do_restore(victim,"self");
            do_look(victim, "auto");
			sprintf(buf, "{RThe Gauntlet Master shouts '{W%s is no match for me, find me a warrior who is!{R'{x\n\r",victim->name);
			for (d = descriptor_list; d != NULL; d = d->next)
			{   
				if (d->connected == CON_PLAYING)
				{
					if(!IS_SET(d->character->deaf, CHANNEL_DPTALK))
					{
					
						send_to_char(buf, d->character);
					}
				}
			}
			
			return TRUE;			
   }

        /*
         * Death trigger
         */
        if ( IS_NPC( victim ) && HAS_TRIGGER( victim, TRIG_DEATH) )
        {
            victim->position = POS_STANDING;
            mp_percent_trigger( victim, ch, NULL, NULL, TRIG_DEATH );
        }

    if (!IS_NPC(victim))
      {
    sprintf( log_buf, "{r%s got toasted by %s at %s [room %d]{x",
    NAME( victim ), NAME( ch ), ch->in_room->name, ch->in_room->vnum);
    wiznet(log_buf,NULL,NULL,WIZ_DEATHS,0,0);
      }

    if (IS_SET(victim->pact,PLR_TARGET))
      REMOVE_BIT(victim->pact,PLR_TARGET);

        raw_kill( victim, ch ); 

      
        /* RT new auto commands */

	if (!IS_NPC(ch)
	&&  (corpse = get_obj_list(ch,"corpse",ch->in_room->contents)) != NULL
	&&  corpse->item_type == ITEM_CORPSE_NPC && can_see_obj(ch,corpse))
	{
	    OBJ_DATA *coins;

	    corpse = get_obj_list( ch, "corpse", ch->in_room->contents ); 

	    if ( IS_SET(ch->pact, PLR_AUTOLOOT) &&
		 corpse && corpse->contains) /* exists and not empty */
            {
		do_function(ch, &do_get, "all corpse");
	    }


 	    if (IS_SET(ch->pact,PLR_AUTOGOLD) &&
	        corpse && corpse->contains  && /* exists and not empty */
		!IS_SET(ch->pact,PLR_AUTOLOOT))
	    {
		if ((coins = get_obj_list(ch,"gcash",corpse->contains))
		     != NULL)
		{
		    do_function(ch, &do_get, "all.gcash corpse");
	      	}
	    }
            
	    if (IS_SET(ch->pact, PLR_AUTOSAC))
	    {
       	        if (IS_SET(ch->pact,PLR_AUTOLOOT) && corpse && corpse->contains)
       	      	{
		    return TRUE;  /* leave if corpse has treasure */
	      	}
	        else
		{
		    do_function(ch, &do_sacrifice, "corpse");
		}
	    }


	return TRUE;
    }



    if ( victim == ch )
	return TRUE;

    /*
     * Take care of link dead people.
     */
    if ( !IS_NPC(victim) && victim->desc == NULL )
    {
	if ( number_range( 0, victim->wait ) == 0 )
	{
	    do_function(victim, &do_recall, "" );
	    return TRUE;
	}
    }
  }
  
    /*
     * Wimp out?
     */
    if ( IS_NPC(victim) && dam > 0 && victim->wait < PULSE_VIOLENCE / 2)
    {
	if ( ( IS_SET(victim->act, ACT_WIMPY) && number_bits( 2 ) == 0
	&&   victim->hit < victim->max_hit / 5) 
	||   ( IS_AFFECTED(victim, AFF_CHARM) && victim->master != NULL
	&&     victim->master->in_room != victim->in_room ) )
	{
	    do_function(victim, &do_flee, "" );
	}
    }

    if ( !IS_NPC(victim)
    &&   victim->hit > 0
    &&   victim->hit <= victim->wimpy
    &&   victim->wait < PULSE_VIOLENCE / 2 )
    {
	do_function (victim, &do_flee, "" );
    }

  
    tail_chain( );
    return TRUE;
 }

bool is_safe(CHAR_DATA *ch, CHAR_DATA *victim)
{
  if (victim->in_room == NULL || ch->in_room == NULL)
    return TRUE;

  if (victim->fighting == ch || victim == ch)
    return FALSE;

  if (IS_IMMORTAL(ch) && ch->level > LEVEL_IMMORTAL)
    return FALSE;
    
    // Opposing legend factions hate each other
    if(!IS_NPC(ch) && !IS_NPC(victim) && IS_LEGEND(ch) && IS_LEGEND(victim) && ch->pcdata->legend!=victim->pcdata->legend)
        return FALSE;

    /* killing mobiles */
  if (IS_NPC(victim))
    {

  	/* safe room? */
    if (IS_SET(victim->in_room->room_flags,ROOM_SAFE))
      {
	    send_to_char("Not in this room.\n\r",ch);
	    return TRUE;
      }

    if (victim->pIndexData->pShop != NULL)
      {
	    send_to_char("The shopkeeper wouldn't like that.\n\r",ch);
	    return TRUE;
      }

    /* no killing healers, trainers, etc */

    if (IS_SET(victim->act,ACT_TRAIN)
    ||  IS_SET(victim->act,ACT_PRACTICE)
    ||  IS_SET(victim->act,ACT_BANKER)
    ||  IS_SET(victim->act,ACT_FORGER)
    ||  IS_SET(victim->act,ACT_PRACTICE)
    ||  IS_SET(victim->act,ACT_QUESTMASTER)
    ||  IS_SET(victim->act,ACT_IS_HEALER)
    ||  IS_SET(victim->act,ACT_GAIN))
      {

       if (IS_SET(victim->act,ACT_PET))
         {
          if (victim->master != NULL)
            {
	     return FALSE;
            }
         }

	  send_to_char("I don't think Mota would approve.\n\r",ch);
	  return TRUE;
       }


  
	if (!IS_NPC(ch))
      {
      if (IS_SET(victim->act,ACT_PET)
      && (victim->master == NULL))
        {
        act("But $N looks so cute and cuddly...",
            ch,NULL,victim,TO_CHAR);
        return TRUE;
  	    }


      if (IS_AFFECTED(victim,AFF_CHARM) && ch != victim->master)
  	    {
        send_to_char("You don't own that monster.\n\r",ch);
        return TRUE;
  	    }
      }
    }
  else
    {
  	if (IS_NPC(ch))
    	{
	    if (IS_SET(victim->in_room->room_flags,ROOM_SAFE))
  	    {
/*             send_to_char("\n\r{RNot in a SAFE room!{x\n\r",ch); */
             return TRUE;
            }

	    /* charmed mobs and pets cannot attack players while owned */
	    if ((IS_AFFECTED(ch,AFF_CHARM) && ch->master != NULL)
            && (!IS_SET(victim->pact, PLR_PKILLER)))
  	    {
/*             send_to_char("\n\r{CPlayers are your friends!{x\n\r",ch); */
             return TRUE;
  	    }
  	  }
        else
        { 
         if (IS_SET(ch->pact, PLR_DRAGONPIT))
           {
            if (IS_SET(victim->pact, PLR_DRAGONPIT))
              {
               return FALSE;
              }
           }
           
        if(IS_SET(ch->in_room->room_flags, ROOM_DRAGONPIT))
            return FALSE;
         
         if(!IS_NPC(victim))
           {
            if (IS_SET(victim->pact,PLR_TARGET) || IS_SET(victim->pact,PLR_THIEF))
                return FALSE;


        if (victim->level >= ch->level+11) 
  	      {
               if(!IS_SET(ch->pact,PLR_TARGET))
                 {
            send_to_char("\n\{CSo you think you are ready to play with the big characters?  Better hope so!{x\n\r",ch);
            SET_BIT(ch->pact, PLR_TARGET);
                 }
               return FALSE;
  	      }


        if (victim->fighting != ch)
          {
           if (ch->pcdata->oldcl > -1
           || victim->pcdata->oldcl > -1)
             {
	      if(IS_REMORT(victim))
		{
		 if(IS_REMORT(ch))
		  {
		   if(victim->level < ch->level-15)
               	    {
                     send_to_char("\n\r{RPick on someone your own size.{x\n\r",ch);
                     return TRUE;
                    }
		  }
		  else
		  {
		    if(victim->level < ch->level-20)
	            {
                	send_to_char("\n\r{RPick on someone your own size.{x\n\r",ch);
                	return TRUE;
                    }
		  }
		}
		else
		{
		  if(IS_REMORT(ch))
		  {
		    if(victim->level < ch->level-5)
                    {
                        send_to_char("\n\r{RPick on someone your own size.{x\n\r",ch);
                        return TRUE;
                    }
		  }
		  else
		  {
		    if (victim->level < ch->level-10)
                    {
                	send_to_char("\n\r{RPick on someone your own size.{x\n\r",ch);
                	return TRUE;
                    }
		  }
		}
              }
	    else
              {
             if (victim->level < ch->level-10)
               {
                send_to_char("\n\r{RPick on someone your own size.{x\n\r",ch);
                return TRUE;
               }
             }
          }
        }
      }
    }
  return FALSE;
}
 
bool is_safe_spell(CHAR_DATA *ch, CHAR_DATA *victim, bool area )
{
    if (victim->in_room == NULL 
    || ch->in_room == NULL)
        return TRUE;

    if (victim == ch 
    && area)
	return TRUE;

    if (victim->fighting == ch || victim == ch)
	return FALSE;

    if (IS_IMMORTAL(ch) 
    && ch->level > LEVEL_IMMORTAL 
    && !area)
	return FALSE;

    /* killing mobiles */
    if (IS_NPC(victim))
    {
	/* safe room? */
	if (IS_SET(victim->in_room->room_flags,ROOM_SAFE))
	    return TRUE;

	if (victim->pIndexData->pShop != NULL)
	    return TRUE;

	/* no killing healers, trainers, etc */
    if (IS_SET(victim->act,ACT_TRAIN)
    ||  IS_SET(victim->act,ACT_PRACTICE)
    ||  IS_SET(victim->act,ACT_BANKER)
    ||  IS_SET(victim->act,ACT_FORGER)
    ||  IS_SET(victim->act,ACT_PRACTICE)
    ||  IS_SET(victim->act,ACT_QUESTMASTER)
    ||  IS_SET(victim->act,ACT_IS_HEALER)
    ||  IS_SET(victim->act,ACT_GAIN))
      {
    
       if (IS_SET(victim->act,ACT_PET))
         {
          if (victim->master != NULL)
            {
             return FALSE;
            }
         }
            
          send_to_char("I don't think Mota would approve.\n\r",ch);
          return TRUE;
       }


	if (!IS_NPC(ch))
	{
	    /* no pets */
	    if (IS_SET(victim->act,ACT_PET)
            && (victim->master == NULL))
	   	return TRUE;

	    /* no charmed creatures unless owner */
	    if (IS_AFFECTED(victim,AFF_CHARM) && (area || ch != victim->master))
		return TRUE;

	    /* legal kill? -- cannot hit mob fighting non-group member */
	    if (victim->fighting != NULL && !is_same_group(ch,victim->fighting))
		return TRUE;
	}
	else
	{
	    /* area effect spells do not hit other mobs */
	    if (area && !is_same_group(victim,ch->fighting))
		return TRUE;
	}
    }
    /* killing players */
    else
    {
	if (area && IS_IMMORTAL(victim) && victim->level > LEVEL_IMMORTAL)
	    return TRUE;

	/* NPC doing the killing */
	if (IS_NPC(ch))
	{
	    /* charmed mobs and pets cannot attack players while owned */
	    if (IS_AFFECTED(ch,AFF_CHARM) && ch->master != NULL
	    &&  ch->master->fighting != victim)
		return TRUE;
	
	    /* safe room? */
	    if (IS_SET(victim->in_room->room_flags,ROOM_SAFE))
		return TRUE;

	    /* legal kill? -- mobs only hit players grouped with opponent*/
	    if (ch->fighting != NULL && !is_same_group(ch->fighting,victim))
		return TRUE;
	}
	else
	{
    	if (check_valid_attack(ch, victim) == FALSE)
    	   return TRUE;

         if (IS_SET(ch->in_room->room_flags, ROOM_DRAGONPIT))
           {
            if (IS_SET(victim->in_room->room_flags, ROOM_DRAGONPIT))
              {
               return FALSE;
              }
           }

	    if (IS_SET(victim->pact,PLR_TARGET) || IS_SET(victim->act,PLR_THIEF))
		return FALSE;

	    if (ch->level > victim->level + 10)
		return TRUE;
	}

    }
    return FALSE;
}
/*
 * See if an attack justifies a KILLER flag.
 */
/*
void check_killer( CHAR_DATA *ch, CHAR_DATA *victim )
{
    char buf[MAX_STRING_LENGTH];

    while ( IS_AFFECTED(victim, AFF_CHARM) && victim->master != NULL )
	victim = victim->master;


    if ( IS_NPC(victim)
    ||   IS_SET(victim->pact, PLR_THIEF))
	return;


    if ( IS_SET(ch->affected_by, AFF_CHARM) )
    {
	if ( ch->master == NULL )
	{
	    char buf[MAX_STRING_LENGTH];

	    sprintf( buf, "Check_killer: %s bad AFF_CHARM",
		IS_NPC(ch) ? ch->short_descr : ch->name );
	    bug( buf, 0 );
	    affect_strip( ch, gsn_charm_person );
	    REMOVE_BIT( ch->affected_by, AFF_CHARM );
	    return;
	}
	stop_follower( ch );
	return;
    }


    if ( IS_NPC(ch)
    ||   ch == victim
    ||   IS_IMMORTAL(ch)
    ||   IS_SET(ch->pact, PLR_PKILLER) 
    ||	 ch->fighting  == victim)
        return;


    if (((IS_SET(ch->pact, PLR_PKILLER))
    || (IS_SET(victim->pact, PLR_PKILLER)))
    && (!IS_NPC(ch)))
      {
    send_to_char( "*** You are now a KILLER!! ***\n\r", ch );
    SET_BIT(ch->pact, PLR_KILLER);
    save_char_obj( ch );
    return;
      }
    else
      {
     return;
       }
 return;
}
*/


/*
 * Check for parry.
 */
bool check_parry( CHAR_DATA *ch, CHAR_DATA *victim )
{
    int chance;

    if ( !IS_AWAKE(victim) )
	return FALSE;

    chance = get_skill(victim,gsn_parry) / 2;

    if ( get_eq_char( victim, WEAR_WIELD ) == NULL )
    {
	if (IS_NPC(victim))
	    chance /= 2;
	else
	    return FALSE;
    }

    if (!can_see(ch,victim))
	chance /= 2;

    if ( number_percent( ) >= chance + victim->level - ch->level )
	return FALSE;

    act( "You parry $n's attack.",  ch, NULL, victim, TO_VICT    );
    act( "$N parries your attack.", ch, NULL, victim, TO_CHAR    );
    check_improve(victim,gsn_parry,TRUE,6);
    return TRUE;
}

/*
 * Check for shield block.
 */
bool check_shield_block( CHAR_DATA *ch, CHAR_DATA *victim )
{
    int chance;

    if ( !IS_AWAKE(victim) )
        return FALSE;


    chance = get_skill(victim,gsn_shield_block) / 5 + 3;


    if ( get_eq_char( victim, WEAR_SHIELD ) == NULL )
        return FALSE;

    if ( number_percent( ) >= chance + victim->level - ch->level )
        return FALSE;

    act( "You block $n's attack with your shield.",  ch, NULL, victim, 
TO_VICT    );
    act( "$N blocks your attack with a shield.", ch, NULL, victim, 
TO_CHAR    );
    check_improve(victim,gsn_shield_block,TRUE,6);
    return TRUE;
}


/*
 * Check for dodge.
 */
bool check_dodge( CHAR_DATA *ch, CHAR_DATA *victim )
{
    int chance;

    if ( !IS_AWAKE(victim) )
	return FALSE;

    chance = get_skill(victim,gsn_dodge) / 2;

    if (!can_see(victim,ch))
	chance /= 2;

    if ( number_percent( ) >= chance + victim->level - ch->level )
        return FALSE;

    act( "You dodge $n's attack.", ch, NULL, victim, TO_VICT    );
    act( "$N dodges your attack.", ch, NULL, victim, TO_CHAR    );
    check_improve(victim,gsn_dodge,TRUE,6);
    return TRUE;
}



/*
 * Set position of a victim.
 */
void update_pos( CHAR_DATA *victim )
{
    if ( victim->hit > 0 )
    {
    	if ( victim->position <= POS_STUNNED )
	    victim->position = POS_STANDING;
	return;
    }

    if ( IS_NPC(victim) && victim->hit < 1 )
    {
	victim->position = POS_DEAD;
	return;
    }

    if ( victim->hit <= -11 )
    {
	victim->position = POS_DEAD;
	return;
    }

         if ( victim->hit <= -6 )
            victim->position = POS_MORTAL;
        else if ( victim->hit <= -3 )
        {
            victim->position = POS_INCAP;
            if(!IS_NPC(victim) && victim->pcdata)
                victim->pcdata->incap_duration = 1;
        }
        else
            victim->position = POS_STUNNED;

    return;
}



/*
 * Start fights.
 */
void set_fighting( CHAR_DATA *ch, CHAR_DATA *victim )
{
    if ( ch->fighting != NULL )
    {
	bug( "Set_fighting: already fighting", 0 );
	return;
    }

    if ( IS_AFFECTED(ch, AFF_SLEEP) )
	affect_strip( ch, gsn_sleep );

    ch->fighting = victim;
    ch->position = POS_FIGHTING;

    return;
}



/*
 * Stop fights.
 */
void stop_fighting( CHAR_DATA *ch, bool fBoth )
{
    CHAR_DATA *fch;

    for ( fch = char_list; fch != NULL; fch = fch->next )
    {
	if ( fch == ch || ( fBoth && fch->fighting == ch ) )
	{
	    fch->fighting	= NULL;
	    fch->position	= IS_NPC(fch) ? fch->default_pos : POS_STANDING;
	    update_pos( fch );
	}
    }

    return;
}


/*
 * Make a corpse out of a character.
 */
void make_corpse( CHAR_DATA *ch, CHAR_DATA *killer )
{
  ROOM_INDEX_DATA *corpse_room = NULL;
  char buf[MAX_STRING_LENGTH];
  OBJ_DATA *corpse;
  OBJ_DATA *obj;
  OBJ_DATA *obj_next;
  char *name;

  if ( IS_NPC(ch) )
    {
    name	= ch->short_descr;
    corpse		= create_object(get_obj_index(OBJ_VNUM_CORPSE_NPC), 0);
    corpse->timer	= number_range( 3, 6 );

    if ( ch->gold > 0
    || ch->silver > 0)
      {
	    obj_to_obj( create_money( ch->gold, ch->silver ), corpse );

        lottery += ch->gold / 10;

	    ch->gold = 0;
	    ch->silver = 0;
      }

    corpse->cost = 0;
    }
  else
    {
    name		= ch->name;
    corpse		= create_object(get_obj_index(OBJ_VNUM_CORPSE_PC), 0);
    corpse->timer	= number_range( 25, 40 );
    REMOVE_BIT(ch->pact,PLR_CANLOOT);

    corpse->owner = NULL;

    if (ch->gold > 1 || ch->silver > 1)
      {
      obj_to_obj(create_money(ch->gold / 2, ch->silver/2), corpse);

      lottery += 2 * ch->gold / 10;

      ch->gold = 0;
      ch->silver = 0;
	    }
    corpse->cost = 0;
    }

  corpse->level = ch->level;

  sprintf( buf, corpse->name, name );
  free_string( corpse->name );
  corpse->name = str_dup( buf );

  sprintf( buf, corpse->short_descr, name );
  free_string( corpse->short_descr );
  corpse->short_descr = str_dup( buf );

  sprintf( buf, corpse->description, name );
  free_string( corpse->description );
  corpse->description = str_dup( buf );

  for ( obj = ch->carrying; obj != NULL; obj = obj_next )
    {
    bool floating = FALSE;
    obj_next = obj->next_content;
    if (obj->wear_loc == WEAR_FLOAT)
      floating = TRUE;
    obj_from_char( obj );
    if (obj->item_type == ITEM_POTION)
      obj->timer = number_range(500,1000);
    if (obj->item_type == ITEM_SCROLL)
      obj->timer = number_range(1000,2500);

    if (IS_SET(obj->extra_flags,ITEM_ROT_DEATH) && !floating)
      {
      obj->timer = number_range((UMIN(25,(obj->level))),(obj->level*10));
      REMOVE_BIT(obj->extra_flags,ITEM_ROT_DEATH);
      }
   
     REMOVE_BIT(obj->extra_flags,ITEM_VIS_DEATH);

    if ( IS_SET( obj->extra_flags, ITEM_INVENTORY ) )
      extract_obj( obj );
    else 
   if (floating)
      {
	    if (IS_OBJ_STAT(obj,ITEM_ROT_DEATH)) /* get rid of it! */
        { 
        if (obj->contains != NULL)
          {
          OBJ_DATA *in, *in_next;
          act("$p evaporates,scattering its contents.", ch,obj,NULL,TO_ROOM);
          for (in = obj->contains; in != NULL; in = in_next)
            {
            in_next = in->next_content;
            obj_from_obj(in);
            obj_to_room(in,ch->in_room);
            }
          }
        else
		      act("$p evaporates.", ch,obj,NULL,TO_ROOM);
        extract_obj(obj);
        }
	    else
        {
        act("$p falls to the floor.",ch,obj,NULL,TO_ROOM);
        obj_to_room(obj,ch->in_room);
        }
      }
    else
	    obj_to_obj( obj, corpse );
    }
  if ( ( !IS_NPC( killer ) && killer != ch )
  || ( IS_NPC( killer ) && killer->master && !IS_NPC( killer->master ) )
  || IS_NPC( ch ) )
    corpse_room = ch->in_room;
  else
    corpse_room = get_room_index( ROOM_VNUM_MORGUE );

  if(IS_SET(ch->form,FORM_INSTANT_DECAY))
    {
     extract_obj(corpse);
     return;
    }

  if (!IS_SET(ch->in_room->room_flags, ROOM_DEATHTRAP))
    { 
     obj_to_room( corpse, corpse_room );
     return;
    }
   else
    {
     extract_obj(corpse);
     return;
    } 
 }



/*
 * Improved Death_cry contributed by Diavolo.
 */
void death_cry( CHAR_DATA *ch )
{
    ROOM_INDEX_DATA *was_in_room;
    char *msg;
    int door;
    int vnum;

    vnum = 0;
    msg = "You hear $n's death cry.";

    switch ( number_bits(4))
    {
    case  0: msg  = "$n hits the ground ... DEAD.";			break;
    case  1: 
	if (ch->material == 0)
	{
	    msg  = "$n splatters blood on your armor.";		
	    break;
	}
    case  2: 							
	if (IS_SET(ch->parts,PART_GUTS))
	{
	    msg = "$n spills $s guts all over the floor.";
	    vnum = OBJ_VNUM_GUTS;
	}
	break;
    case  3: 
	if (IS_SET(ch->parts,PART_HEAD))
	{
	    msg  = "$n's severed head plops on the ground.";
	    vnum = OBJ_VNUM_SEVERED_HEAD;				
	}
	break;
    case  4: 
	if (IS_SET(ch->parts,PART_HEART))
	{
	    msg  = "$n's heart is torn from $s chest.";
	    vnum = OBJ_VNUM_TORN_HEART;				
	}
	break;
    case  5: 
	if (IS_SET(ch->parts,PART_ARMS))
	{
	    msg  = "$n's arm is sliced from $s dead body.";
	    vnum = OBJ_VNUM_SLICED_ARM;				
	}
	break;
    case  6: 
	if (IS_SET(ch->parts,PART_LEGS))
	{
	    msg  = "$n's leg is sliced from $s dead body.";
	    vnum = OBJ_VNUM_SLICED_LEG;				
	}
	break;
    case 7:
	if (IS_SET(ch->parts,PART_BRAINS))
	{
	    msg = "$n's head is shattered, and $s brains splash all over you.";
	    vnum = OBJ_VNUM_BRAINS;
	}
    }

    act( msg, ch, NULL, NULL, TO_ROOM );

    if ( vnum != 0 )
    {
	char buf[MAX_STRING_LENGTH];
	OBJ_DATA *obj;
	char *name;

	name		= IS_NPC(ch) ? ch->short_descr : ch->name;
	obj		= create_object( get_obj_index( vnum ), 0 );
	obj->timer	= number_range( 4, 7 );

	sprintf( buf, obj->short_descr, name );
	free_string( obj->short_descr );
	obj->short_descr = str_dup( buf );

	sprintf( buf, obj->description, name );
	free_string( obj->description );
	obj->description = str_dup( buf );

	if (obj->item_type == ITEM_FOOD)
	{
	    if (IS_SET(ch->form,FORM_POISON))
		obj->value[3] = 1;
	    else if (!IS_SET(ch->form,FORM_EDIBLE))
		obj->item_type = ITEM_TRASH;
	}

	obj_to_room( obj, ch->in_room );
    }

    if ( IS_NPC(ch) )
	msg = "You hear something's death cry.";
    else
	msg = "You hear someone's death cry.";

    was_in_room = ch->in_room;
    for ( door = 0; door <= 9; door++ )
    {
	EXIT_DATA *pexit;

	if ( ( pexit = was_in_room->exit[door] ) != NULL
	&&   pexit->u1.to_room != NULL
	&&   pexit->u1.to_room != was_in_room )
	{
	    ch->in_room = pexit->u1.to_room;
	    act( msg, ch, NULL, NULL, TO_ROOM );
	}
    }
    ch->in_room = was_in_room;

    return;
}

  

void raw_kill( CHAR_DATA *victim, CHAR_DATA *killer )
{
    int i;
    char    buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
    CHAR_DATA * original;
    
    stop_fighting( victim, TRUE );
    death_cry( victim );

 if (!IS_SET(victim->in_room->room_flags, ROOM_DRAGONPIT) || !IS_SET(victim->pact, PLR_DRAGONPIT))
   {
     make_corpse( victim, killer );

    if ( IS_NPC(victim) )
    {
	victim->pIndexData->killed++;
	kill_table[URANGE(0, victim->level, MAX_LEVEL-391)].killed++;
	extract_char( victim, TRUE );

       if ( !IS_SET(victim->form,FORM_UNDEAD))
	 check_spirit(killer,victim);

	return;
    }

    if(IS_SET(victim->pact, PLR_THIEF) && victim!=killer)
        REMOVE_BIT( victim->pact, PLR_THIEF );

    extract_char( victim, FALSE );

    while ( victim->affected )
	affect_remove( victim, victim->affected );
    victim->affected_by	= race_table[victim->race].aff;
    victim->affected2_by = race_table[victim->race].aff2;

    for (i = 0; i < 4; i++)
    	victim->armor[i]= 100;
 
    if (victim->race == RACE_MINOTAUR)
      {
       for (i = 0; i < 4; i ++)
         victim->armor[i] += UMAX(10,10*(victim->level/5));
      }

    victim->position	= POS_RESTING;
    victim->hit		= UMAX( 1, victim->hit  );
    victim->mana	= UMAX( 1, victim->mana );
    victim->move	= UMAX( 1, victim->move );
    
    if(IS_HARDCORE(victim) && !(IS_LEGEND(victim) && victim->level==LEVEL_HERO))
    {
       if(victim->level>victim->pcdata->previousHCLevel)
           victim->pcdata->previousHCLevel = victim->level;

       victim->level    = 1;
       victim->oldlvl   = 1;
       victim->exp      = exp_per_level(victim,victim->pcdata->points);

       if(victim->pcdata->previousHCLevel==LEVEL_HERO)
       {
            while(victim->level<LEVEL_HERO-25)
            {
            	victim->level  += 1;
            	advance_level( victim,TRUE, FALSE);
            }
            victim->exp   = exp_per_level(victim,victim->pcdata->points)  * UMAX( 1, victim->level );
       }
       
       save_char_obj(victim);
       
       if(victim->pcdata->previousHCLevel>=5)
       {
           sprintf(buf, "\n\r{W[{RG{YR{GA{CT{MS{W] {xMota{W: {GEveryone CONGRATULATE {W%s{G for KILLING the {RHARDCORE {Gplayer {W%s{R!{Y!{R!{Y!{R!{Y!{R!{Y!{x\n\r", IS_NPC(killer) ? killer->short_descr : killer->name, victim->name);
        	for(d=descriptor_list;d;d=d->next)
        	{
        		original = d->original ? d->original : d->character; /* if switched */
        		if(d->connected==CON_PLAYING)
        		  act_new(buf, original, NULL, NULL, TO_CHAR, POS_DEAD);
        	}
        	
        	sprintf(buf, "\n\r{ROHHHH NOOO!!!! {W%s{C who is a {RHARDCORE PLAYER{C has just DIED at Level %d! They must now start over at Level %d{R!{Y!{R!{Y!{R!{Y!{x\n\r", victim->name, victim->pcdata->previousHCLevel, victim->level);
        	for(d=descriptor_list;d;d=d->next)
        	{
        		original = d->original ? d->original : d->character; /* if switched */
        		if(d->connected==CON_PLAYING)
        		  act_new(buf, original, NULL, NULL, TO_CHAR, POS_DEAD);
        	}
        }
    }
    
    return;
   }
 else
   {

    if ( IS_SET(victim->pact, PLR_DRAGONPIT))
      {
       
    if ( IS_NPC(victim) )
    {
	victim->pIndexData->killed++;
	kill_table[URANGE(0, victim->level, MAX_LEVEL-391)].killed++;
	extract_char( victim, TRUE );
	return;
    }

    while ( victim->affected )
	affect_remove( victim, victim->affected );
    victim->affected_by	= race_table[victim->race].aff;
    victim->affected2_by	= race_table[victim->race].aff2;

/*
    for (i = 0; i < 4; i++)
    	victim->armor[i]= 100;
 
    if (victim->race == RACE_MINOTAUR)
      {
       for (i = 0; i < 4; i ++)
         victim->armor[i] += UMAX(10,10*(victim->level/5));
      }
*/
         victim->position	= POS_STANDING;
         char_from_room(victim);
         char_to_room(victim, get_room_index(ROOM_VNUM_DRAGONPIT_RETURN));
         do_restore(victim,"self");
         do_look(victim, "auto");
  
         if (victim->in_room == get_room_index(ROOM_VNUM_DRAGONPIT_RETURN))
           {
            REMOVE_BIT(victim->pact, PLR_DRAGONPIT);
           }

         return;
      }
   }
}



void group_gain( CHAR_DATA *ch, CHAR_DATA *victim )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *gch;
    CHAR_DATA *lch;
    int xp;
    int members;
    int group_levels;
    // Explorer/Killer Percentages
    char    killerBuf[MAX_STRING_LENGTH];
    float totalXP = 0;

    /*
     * Monsters don't get kill xp's or alignment changes.
     * P-killing doesn't help either.
     * Dying of mortal wounds or poison doesn't give xp to anyone!
     */
    if ( victim == ch )
	return;
    

if(!IS_SET(ch->in_room->room_flags, ROOM_DRAGONPIT))
  {
   if (!IS_SET(victim->in_room->room_flags,ROOM_DRAGONPIT))
     {  
    members = 0;
    group_levels = 0;
    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
	if ( is_same_group( gch, ch ) )
        {
	    members++;
	    group_levels += IS_NPC(gch) ? gch->level / 2 : gch->level;
	}
    }
        if (IS_SET(ch->pact, PLR_QUESTOR)
         && IS_NPC(victim))
        {
            if (ch->questmob == victim->pIndexData->vnum)
            {
                send_to_char("{WYou have almost completed your {RQUEST!{x\n\r",ch);
                send_to_char("{WReturn to the {Rquestmaster {Wbefore your time runs out!{x\n\r",ch);
                ch->questmob = -1;
            }
        }

        // Explorer/Killer Percentages
        if(IS_NPC(victim) && victim->pIndexData && victim->pIndexData->vnum && ch->pcdata)
        {
            sprintf(killerBuf, "%d", victim->pIndexData->vnum);
            //sprintf(buf, "you just killed %s", killerBuf);
            //send_to_char(buf, ch);
            if(valid_explorer_killer(ch) && (!ch->pcdata->killed || array_find(ch->pcdata->killed, killerBuf)==-1))
            {
                if(ch->pcdata->alertMe)
                    send_to_char("\n\r{C***{M*** {GYou just killed a new monster!!! {M***{C***{x\n\r", ch);
                ch->pcdata->killed = array_append(ch->pcdata->killed, killerBuf);
            }
        }


    if ( members == 0 )
    {
	bug( "Group_gain: members.", members );
	members = 1;
	group_levels = ch->level ;
    }

    lch = (ch->leader != NULL) ? ch->leader : ch;

    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
	OBJ_DATA *obj;
	OBJ_DATA *obj_next;

	if ( !is_same_group( gch, ch ) || IS_NPC(gch))
	    continue;

	if ( gch->level - lch->level >= 10 )
	{
	    send_to_char( "\n\r{RYou are too high for this group.{x\n\r", gch );
	    continue;
	}

	if ( gch->level - lch->level <= -10 )
	{
	    send_to_char( "\n\r{RYou are too low for this group.{x\n\r", gch );
	    continue;
	}

        float aligndifference;
        totalXP = 0;
        
        if(gch->alignment>=victim->alignment)
            aligndifference = (float)(gch->alignment-victim->alignment);
        else
            aligndifference = (float)(victim->alignment-gch->alignment);
        
        //sprintf(buf, "Calculating for player %s [%d] against mob %s [%d] with align difference %f\n\r", gch->name, gch->level, victim->name, victim->level, aligndifference);
        //send_to_char(buf, gch);
        
        totalXP = (((float)((float)((float)victim->level - (float)gch->level)+(float)1) / (float)gch->level) * (float)100);
        //sprintf(buf, "XP after first step ((((moblevel - playerlevel)+1) / playerlevel) * 100)): %f\n\r", totalXP);
        //send_to_char(buf, gch);
        
        totalXP *= (float)(((float)number_range(75, 125)/(float)100.0));
        //sprintf(buf, "XP after second step (xp * (0.75 to 1.25)): %f\n\r", totalXP);
        //send_to_char(buf, gch);
        
        if(gch->level<=90)
            totalXP += (float)((float)aligndifference / (float)40.0);
        //sprintf(buf, "XP after third step (xp + (aligndifference / 40)): %f\n\r", totalXP);
        //send_to_char(buf, gch);
        
         if(number_range(1, 20)==7)
            totalXP+=(float)50.0;
        
        //sprintf(buf, "XP after fourth step (xp + (1 in 20 chance of +50)): %f\n\r", totalXP);
        //send_to_char(buf, gch);
        
        //if(members>1)
        //    totalXP*=(float)((float)1.0-(float)((float)((float)members-(float)1.0)*(float)0.1));   // Reduce XP by 10% per party member (other than character)

        xp = (int)totalXP;
        
        if(IS_HARDCORE(ch))
            xp*=1.2;
        //sprintf(buf, "Final XP Bonus: %d\n\r", (int)xp);
        //send_to_char(buf, gch);

        if(xp<0)
        {
            send_to_char("\n\r{RThat mob is much too low level for you. Find something tougher to fight.{x\n\r", gch);
            xp = 0;
        }
    
    // Change alignment
    if(gch->alignment==victim->alignment)
    {
        if(gch->alignment<=0)
            gch->alignment += abs(gch->alignment + victim->alignment)*.025;
        else
            gch->alignment += abs(gch->alignment + victim->alignment)*-.025;
    }
    else if(gch->alignment<0)
    {
        if(gch->alignment<victim->alignment)
            gch->alignment += abs(victim->alignment - gch->alignment)*-.025;
        else
            gch->alignment += abs(gch->alignment + victim->alignment)*.025;
    }
    else
    {
        if(gch->alignment<victim->alignment)
            gch->alignment += abs(gch->alignment + victim->alignment)*-.025;
        else
            gch->alignment += abs(victim->alignment - gch->alignment)*.025;
    }
    
    if(gch->alignment<=-1000)
        gch->alignment = -1000;
    else if(gch->alignment>=1000)
        gch->alignment = 1000;
        
	//xp = xp_compute( gch, victim, group_levels );  
	sprintf( buf, "{YYou receive {w%d {Yexperience points.{x\n\r", xp );
	send_to_char( buf, gch );
	gain_exp( gch, xp );

	for ( obj = ch->carrying; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;
	    if ( obj->wear_loc == WEAR_INVENTORY )
		continue;

	    if ( ( IS_OBJ_STAT(obj, ITEM_ANTI_EVIL)    && IS_EVIL(ch)    )
	    ||   ( IS_OBJ_STAT(obj, ITEM_ANTI_GOOD)    && IS_GOOD(ch)    )
	    ||   ( IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(ch) ) )
	    {
		act( "{GYou are zapped by $p.{x", ch, obj, NULL, TO_CHAR );
		act( "{G$n is zapped by $p.{x",   ch, obj, NULL, TO_ROOM );
		obj_from_char( obj );
		obj_to_room( obj, ch->in_room );
	    }
	}
     }
   }
 }
 return;
}



/*
 * Compute xp for a kill.
 * Also adjust alignment of killer.
 * Edit this function to change xp computations.
 */
int xp_compute( CHAR_DATA *gch, CHAR_DATA *victim, int total_levels )
{
    int xp,base_exp;
    int align,level_range,time_per_level;
 /*   int change;    REMOVED BECAUSE OF STATIC ALIGNMENTs */

    level_range = victim->level - gch->level;
 
  switch (level_range)
    {
 	default : 	base_exp =   0;		break;
	case -9 :	base_exp =   1;		break;
	case -8 :	base_exp =   2;		break;
	case -7 :	base_exp =   5;		break;
	case -6 : 	base_exp =   9;		break;
	case -5 :	base_exp =  11;		break;
	case -4 :	base_exp =  22;		break;
	case -3 :	base_exp =  33;		break;
	case -2 :	base_exp =  50;		break;
	case -1 :	base_exp =  66;		break;
	case  0 :	base_exp =  83;		break;
	case  1 :	base_exp =  99;		break;
	case  2 :	base_exp = 121;		break;
	case  3 :	base_exp = 143;		break;
	case  4 :	base_exp = 165;		break;
    }

    if (level_range > 4)
	base_exp = 160 + (15 * (level_range - 4));

    /* do alignment computations */
   
    align = victim->alignment - gch->alignment;

    if (IS_SET(victim->act,ACT_NOALIGN))
    {
	/* no change */
    }

/*
    else if (align > 500) 
    {
	change = (align - 500) * base_exp / 500 * gch->level/total_levels; 
	change = UMAX(1,change);
        gch->alignment = UMAX(-1000,gch->alignment - change);
    }

    else if (align < -500)
    {
	change =  ( -1 * align - 500) * base_exp/500 * gch->level/total_levels;
	change = UMAX(1,change);
	gch->alignment = UMIN(1000,gch->alignment + change);
    }

    else
    {
	change =  gch->alignment * base_exp/400 * gch->level/total_levels;  
	gch->alignment -= change;
    }
*/
    
    /* calculate exp multiplier */
    if (IS_SET(victim->act,ACT_NOALIGN))
	xp = base_exp;

/*
    else if (gch->alignment > 500) 
*/
    if (gch->alignment > 500) 
    {
	if (victim->alignment < -750)
	    xp = (base_exp *5)/4;
   
 	else if (victim->alignment < -500)
	    xp = (base_exp * 11)/10;

        else if (victim->alignment > 750)
	    xp = base_exp / 4;

   	else if (victim->alignment > 500)
	    xp = (base_exp *3)/4;

        else if (victim->alignment > 250)
	    xp = (base_exp * 9)/10; 

	else
	    xp = base_exp;
    }

/*
    else if (gch->alignment < -500)
*/
    if (gch->alignment < -500)
    {
	if (victim->alignment > 750)
	    xp = (base_exp * 5)/4;
	
  	else if (victim->alignment > 500)
	    xp = (base_exp * 11)/10; 

   	else if (victim->alignment < -750)
	    xp = base_exp/4;

	else if (victim->alignment < -500)
	    xp = (base_exp * 3)/4;

	else if (victim->alignment < -250)
	    xp = (base_exp * 9)/10;

	else
	    xp = base_exp;
    }

/*
    else if (gch->alignment > 200)
    {

	if (victim->alignment < -750)
	    xp = (base_exp * 6)/5;

 	else if (victim->alignment > 750)
	    xp = base_exp/2;

	else if (victim->alignment > 0)
	    xp = (base_exp * 3)/4; 
	
	else
	    xp = base_exp;
    }

    else if (gch->alignment < -200)
    {
	if (victim->alignment > 750)
	    xp = (base_exp * 6)/5;
 
	else if (victim->alignment < -750)
	    xp = base_exp/2;

	else if (victim->alignment < 0)
	    xp = (base_exp * 3)/4;

	else
	    xp = base_exp;
    }
*/
    else /* neutral */
    {

	if (victim->alignment > 500 || victim->alignment < -500)
	    xp = (base_exp * 5)/4;

	else if (victim->alignment < 200 && victim->alignment > -200)
	    xp = base_exp/3;

 	else
	    xp = base_exp;
    } 

    /*Exp Setting by Venus */
    if (gch->level < 15)
    	xp = 1 * xp * (gch->level + 10)* 2 / (gch->level);
     
   /* if (gch->level > 15 && gch->level < 21)
        xp = 1 * xp * (gch->level - 10);*/

    /*if (gch->level > 41 && gch->level < 71)
        xp = 10 * xp / (gch->level - 4);

    if (gch->level > 71 && gch->level < 90)
        xp = 10 * xp / (gch->level - 8);*/

    /* less at high */
    if (gch->level > 90 )
	xp =  10 * xp / (gch->level - 45 );

    /* reduce for playing time */
    /* compute quarter-hours per level */
    /* make it a curve */

    {
	time_per_level = 4 *
			 (gch->played + (int) (current_time - gch->logon))/3600
			 / gch->level;

	time_per_level = URANGE(2,time_per_level,12);
	if (gch->level < 15)  
	    time_per_level = UMAX(time_per_level,(15 - gch->level));
	xp = xp * time_per_level / 12;
    }
    
   
    /* randomize the rewards */
    xp = number_range (xp * 3/4, xp * 5/4);

    /* adjust for grouping */
    if(total_levels>gch->level)
    {
        xp*=0.8;
    }
    //xp = xp * gch->level/( UMAX(1,total_levels -1) );


    if(xp >= 551)
      xp = 550;   

    if (IS_SET(victim->act,ACT_PET)
    && (victim->master == NULL))
      {
       xp = 0;
       return xp;
      }
     else
       return xp;
}


void dam_message( CHAR_DATA *ch, CHAR_DATA *victim,int dam,int dt,bool immune )
{
    char buf1[256], buf2[256], buf3[256];
    const char *vs;
    const char *vp;
    const char *attack;
    char punct;

    if (ch == NULL || victim == NULL)
	return;

/*
    if (dam >= 4001)
     dam = 4000;
*/

	 if ( dam ==   0 ) { vs = "{Rmiss{x";	        vp = "{Rmisses{x";	}
    else if ( dam <=   5 ) { vs = "{Rscratch{x";	vp = "{Rscratches{x";	}
    else if ( dam <=  10 ) { vs = "{Rgraze{x";	        vp = "{Rgrazes{x";	}
    else if ( dam <=  15 ) { vs = "{Rhit{x";	        vp = "{Rhits{x";	}
    else if ( dam <=  20 ) { vs = "{Rinjure{x";	        vp = "{Rinjures{x";	}
    else if ( dam <=  25 ) { vs = "{Rwound{x";	        vp = "{Rwounds{x";	}
    else if ( dam <=  35 ) { vs = "{Rmaul{x";           vp = "{Rmauls{x";       }
    else if ( dam <=  45 ) { vs = "{Rdecimate{x";	vp = "{Rdecimates{x";	}
    else if ( dam <=  60 ) { vs = "{Rdevastate{x";	vp = "{Rdevastates{x";	}
    else if ( dam <=  80 ) { vs = "{Rmaim{x";	        vp = "{Rmaims{x";	}
    else if ( dam <= 100 ) { vs = "{RMUTILATE{x";	vp = "{RMUTILATES{x";	}
    else if ( dam <= 125 ) { vs = "{RDISEMBOWEL{x";	vp = "{RDISEMBOWELS{x";	}
    else if ( dam <= 150 ) { vs = "{RDISMEMBER{x";	vp = "{RDISMEMBERS{x";	}
    else if ( dam <= 175 ) { vs = "{RMASSACRE{x";	vp = "{RMASSACRES{x";	}
    else if ( dam <= 200 ) { vs = "{RMANGLE{x";	        vp = "{RMANGLES{x";	}
    else if ( dam <= 225 ) { vs = "{R--- {WDEMOLISH {R---{x";
			     vp = "{R--- {WDEMOLISHES {R---{x";		}
    else if ( dam <= 250 ) { vs = "{R*** {WDEVASTATE {R***{x";
			     vp = "{R*** {WDEVASTATES {R***{x";		}
    else if ( dam <= 275 ) { vs = "{R<<< {WOBLITERATE {R>>>{x";
			     vp = "{R<<< {WOBLITERATES {R>>>{x";		}
    else if ( dam <= 300 ) { vs = "{R=== {WANNIHILATE {R==={x";
			     vp = "{R=== {WANNIHILATES {R==={x";		}
    else if ( dam <= 325 ) { vs = "{R!!! {WERADICATE {R!!!{x";
			     vp = "{R!!! {WERADICATES {R!!!{x";		}
    else if ( dam <= 750 ) { vs = "{Rdo {WUNSPEAKABLE {Rthings to{x";
			     vp = "{Rdoes {WUNSPEAKABLE {Rthings to{x";	}
    else                   { vs = "{Rdo {WGODLIKE {Rdamage to{x";
			     vp = "{Rdoes {WGODLIKE {Rdamage to{x";	}

    punct   = (dam <= 100) ? '.' : '!';

    if ( dt == TYPE_HIT )
    {
	if (ch  == victim)
	{
	    sprintf( buf1, "$n %s $melf%c",vp,punct);
                 if (dam!=0) 
                     sprintf( buf2, "You %s yourself for {g<({W%d{g)>{x hp%c",vs,dam,punct);
                 else
                        sprintf( buf2, "You %s yourself%c",vs,punct);
	}
	else
	{
	    sprintf( buf1, "$n %s $N%c",  vp, punct );
                 if (dam!=0)
                 {
            sprintf( buf2, "You %s $N for {c<[{R%d{c]>{x hp%c", vs,dam,punct );
            sprintf( buf3, "$n %s you for {g<({W%d{g)>{x hp%c", vp, dam,punct);
                 }
                 else
                 {
                        sprintf( buf2, "You %s $N%c", vs,punct );
                        sprintf( buf3, "$n %s you%c", vp,punct );
                 }
	}
    }
    else
    {
	if ( dt >= 0 && dt < MAX_SKILL )
	    attack	= skill_table[dt].noun_damage;
	else if ( dt >= TYPE_HIT
	&& dt < TYPE_HIT + MAX_DAMAGE_MESSAGE) 
	    attack	= attack_table[dt - TYPE_HIT].noun;
	else
	{
	    bug( "Dam_message: bad dt %d.", dt );
	    dt  = TYPE_HIT;
	    attack  = attack_table[0].name;
	}

     if (immune)
     {
         if (ch == victim)
         {
          sprintf(buf1,"{G$n is unaffected by $s own %s.{x",attack);
          sprintf(buf2,"{WLuckily, you are immune to that.{x");
         }
         else
         {
          sprintf(buf1,"{W$N is unaffected by $n's %s!{x",attack);
          sprintf(buf2,"\n\r{G$N is unaffected by your %s!{x\n\r",attack);
          sprintf(buf3,"{W$n's %s is powerless against you.{x",attack);
         }
     }
          else
     {
         if (ch == victim)
         {
          sprintf( buf1, "$n's %s %s $m%c",attack,vp,punct);
                  if(dam!=0)
                        sprintf( buf2, "Your %s %s you for {g<({W%d{g)>{x hp%c",attack,vp,dam,punct);
                  else
                          sprintf( buf2, "Your %s %s you%c",attack,vp,punct);
                        }
         else
         {
          sprintf( buf1, "$n's %s %s $N%c",  attack, vp, punct );
                  if(dam!=0)
                  {
 sprintf( buf2, "Your %s %s $N for {c<[{R%d{c]> {Whp{x%c",  attack, vp, dam,punct );
 sprintf( buf3, "$n's %s %s you for {g<({W%d{g)> {Whp{x%c", attack, vp, dam,punct );
                  }
                  else
                  {
                          sprintf( buf2, "Your %s %s $N%c",  attack, vp,punct );
                        sprintf( buf3, "$n's %s %s you%c", attack, vp,punct );
                  }
         }
     }
    }

    if (ch == victim)
         {
     act(buf1,ch,NULL,NULL,TO_ROOM);
     act(buf2,ch,NULL,NULL,TO_CHAR);
    }
    else
         {
     act( buf1, ch, NULL, victim, TO_NOTVICT );
     act( buf2, ch, NULL, victim, TO_CHAR );
     act( buf3, ch, NULL, victim, TO_VICT );
    }

    return;
}



/*
 * Disarm a creature.
 * Caller must check for successful attack.
 */
void disarm( CHAR_DATA *ch, CHAR_DATA *victim )
{
    OBJ_DATA *obj;

    if ( ( obj = get_eq_char( victim, WEAR_WIELD ) ) == NULL )
	return;

    if ( IS_OBJ_STAT(obj,ITEM_NOREMOVE))
    {
	act("$S weapon won't budge!",ch,NULL,victim,TO_CHAR);
	act("$n tries to disarm you, but your weapon won't budge!",
	    ch,NULL,victim,TO_VICT);
	act("$n tries to disarm $N, but fails.",ch,NULL,victim,TO_NOTVICT);
	return;
    }

    act( "$n {GDISARMS{x you and sends your weapon flying!", 
	 ch, NULL, victim, TO_VICT    );
    act( "You disarm $N!",  ch, NULL, victim, TO_CHAR    );
    act( "$n disarms $N!",  ch, NULL, victim, TO_NOTVICT );

    obj_from_char( obj );
    if ( IS_OBJ_STAT(obj,ITEM_NODROP) || IS_OBJ_STAT(obj,ITEM_INVENTORY) )
	obj_to_char( obj, victim );
    else
    if (IS_SET(victim->in_room->room_flags, ROOM_DRAGONPIT) || IS_SET(victim->pact, PLR_DRAGONPIT))
	obj_to_char( obj, victim );
    else
    {
	obj_to_room( obj, victim->in_room );
	if (IS_NPC(victim) && victim->wait == 0 && can_see_obj(victim,obj))
	    get_obj(victim,obj,NULL);
    }

    return;
}

void do_suicide (CHAR_DATA *ch, char *argument )
{
    if(strcmp(argument, "yes kill myself"))
    {
        send_to_char("{RThis will cause you to commity suicide! Type {W'suicide yes kill myself'{R if you are sure!!!{x\n\r", ch);
        return;
    }
    
    send_to_char("{R\n\rYOU HAVE KILLED YOURSELF!!!!!{x\n\r", ch);

    ch->mana	= 1;
    ch->move	= 1;    
    raw_kill(ch, ch);

    ch->mana	= 1;
    ch->move	= 1;    
    ch->hit	= 1;    
    
    //char_to_room(ch,get_room_index(ROOM_VNUM_ALTAR));
    //update_pos( ch);
    
    return;
}

void do_kill( CHAR_DATA *ch, char *argument )
{

  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;

  one_argument( argument, arg );

  if ( arg[0] == '\0' )
    {
    send_to_char( "Kill whom?\n\r", ch );
    return;
    }
    
  if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
    send_to_char( "They aren't here.\n\r", ch );
    return;
    }

  if ( victim == ch )
    {
    send_to_char( "You hit yourself.  Ouch!\n\r", ch );
    return;
    }

  if(IS_SET(victim->in_room->room_flags, ROOM_SAFE))
    {
    send_to_char("\n\r{RNot in a SAFE room!{x\n\r",ch);
    return;
    }

	if (check_valid_attack(ch, victim) == FALSE)
	{
		send_to_char("{RYou may not attack.{x\n\r",ch);
		return;
	}

/*
      if (!IS_SET(victim->in_room->room_flags, ROOM_DRAGONPIT))
      {    
       if (victim->fighting != NULL
       && !is_same_group(ch,victim->fighting));
         {
          send_to_char("\n\r{GKill stealing is not permitted.{x\n\r",ch);
          return;
         }
      }
*/
  if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
    {
    act( "$N is your beloved master.", ch, NULL, victim, TO_CHAR );
    return;
    }

  if ( ch->position == POS_FIGHTING )
    {
    send_to_char( "You do the best you can!\n\r", ch );
    return;
    }

  /*
   * Prevents high-level players from charming
   * low-level mobs and having them kill low
   * level players.
   */
  if ( IS_AFFECTED( ch, AFF_CHARM )
  && ch->master
  && is_safe( ch->master, victim ) )
    return;    
    
    WAIT_STATE( ch, 1 * PULSE_VIOLENCE );
/*    check_killer( ch, victim ); */
    multi_hit( ch, victim, TYPE_UNDEFINED );
    return;
}



void do_flee( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *was_in;
    ROOM_INDEX_DATA *now_in;
    CHAR_DATA *victim;
    int attempt;

    if ( ( victim = ch->fighting ) == NULL )
    {
        if ( ch->position == POS_FIGHTING )
            ch->position = POS_STANDING;
	send_to_char( "You aren't fighting anyone.\n\r", ch );
	return;
    }

    was_in = ch->in_room;
    for ( attempt = 0; attempt < 6; attempt++ )
    {
	EXIT_DATA *pexit;
	int door;

	door = number_door( );
	if ( ( pexit = was_in->exit[door] ) == 0
	||   pexit->u1.to_room == NULL
	||   IS_SET(pexit->exit_info, EX_CLOSED)
	||   number_range(0,ch->daze) != 0
	|| ( IS_NPC(ch)
	&&   IS_SET(pexit->u1.to_room->room_flags, ROOM_NO_MOB) ) )
	    continue;

	move_char( ch, door, FALSE );
	if ( ( now_in = ch->in_room ) == was_in )
	    continue;

	ch->in_room = was_in;
	act( "$n has fled!", ch, NULL, NULL, TO_ROOM );
	ch->in_room = now_in;

	if ( !IS_NPC(ch) )
	{
	    send_to_char( "You flee from combat!\n\r", ch );
	if( (ch->class == 2) 
	    && (number_percent() < 3*(ch->level/2) ) )
		send_to_char( "You snuck away safely.\n\r", ch);
	else
	    {
	       if(IS_HARDCORE(ch))
	       {
        	    send_to_char( "You lost 100 exp.\n\r", ch); 
        	    gain_exp( ch, -100 );	       
	       }
	       else
	       {
        	    send_to_char( "You lost 10 exp.\n\r", ch); 
        	    gain_exp( ch, -10 );
    	   }
	    }
	}

	stop_fighting( ch, TRUE );
	return;
    }

    send_to_char( "PANIC! You couldn't escape!\n\r", ch );
    return;
}


void do_surrender( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *mob;

    if ( (mob = ch->fighting) == NULL )
    {
	send_to_char( "But you're not fighting!\n\r", ch );
	return;
    }

   if(!IS_NPC(mob)
   && (!IS_NPC(ch)))
     {
      if ((!IS_SET(ch->pact, PLR_DRAGONPIT))
      && (!IS_SET(mob->pact, PLR_DRAGONPIT)) && !IS_SET(mob->in_room->room_flags, ROOM_DRAGONPIT))
        {
      act( "\n\r{rYou {RSURRENDER {rto {W$N{r!{x\n\r", ch, NULL, mob, TO_CHAR );
      act( "\n\r{W$n {RSURRENDERs {rto you!{x\n\r", ch, NULL, mob, TO_VICT );
      act( "\n\r{W$n {rhas SURRENDERed to {R$N{r!{x\n\r", ch, NULL, mob, TO_NOTVICT );
      update_pkinfo( mob, ch );
      group_gain( mob, ch ); 
 
       if ( ch->exp > exp_per_level(ch,ch->pcdata->points) * ch->level )
 	gain_exp( ch, (2 * (exp_per_level(ch,ch->pcdata->points) * ch->level - ch->exp)/3) + 50 );

      stop_fighting( ch, TRUE );
        }
      else
        {
         send_to_char("\n\r{RYou may not SURRENDER during a DragonPIT!  They are to the DEATH!{x\n\r",ch);
        }
     }


    if (!IS_NPC( ch ) 
    && IS_NPC( mob ) 
    && ( !HAS_TRIGGER( mob, TRIG_SURR ) 
    || !mp_percent_trigger( mob, ch, NULL, NULL, TRIG_SURR ) ) )
    {
	act( "\n\r{G$N {gseems to ignore your {WCOWARDLY {gact!{x\n\r", ch, NULL, mob, TO_CHAR );
	multi_hit( mob, ch, TYPE_UNDEFINED );
    }

 return;
}

void do_sla( CHAR_DATA *ch, char *argument )
{
    send_to_char( "If you want to SLAY, spell it out.\n\r", ch );
    return;
}


void update_pkinfo( CHAR_DATA *ch, CHAR_DATA *victim )
{
  CLAN_DATA *pClanK = get_clan_index( ch->clan );
  CLAN_DATA *pClanV = get_clan_index( victim->clan );

  if ( ch == victim )
    return;

  if ( !ch->pcdata->member
  ||   !victim->pcdata->member )
    return;

  if ( ch->level > victim->level )
    {
    ch->pcdata->member->pks_dwn++;
    victim->pcdata->member->pkd_up++;
    pClanK->pks_dwn++;
    pClanV->pkd_up++;
    }
  else
    {
    ch->pcdata->member->pks_up++;
    victim->pcdata->member->pkd_dwn++;
    pClanK->pks_up++;
    pClanV->pkd_dwn++;
    }
  save_clans( );
  save_char_obj( ch );
  save_char_obj( victim );
  return;
}


bool is_fighting_pc( CHAR_DATA *ch )
{
  CHAR_DATA *fch;
  if ( ch->fighting
  &&  !IS_NPC( ch->fighting ) )
    return TRUE;
  for ( fch = ch->in_room->people; fch; fch = fch->next )
    if ( fch != ch
    &&   fch->fighting == ch
    &&  !IS_NPC( fch ) )
      return TRUE;
  return FALSE;
}

//This is to address valid PvP attacks.  Added by Yavi July 2004
bool check_valid_attack(CHAR_DATA *ch, CHAR_DATA *victim)
{
	int clevel;
	int vlevel;
	int gap;
	
	if (IS_NPC(ch) || IS_NPC(victim))
		return TRUE;
	if (IS_IMMORTAL(ch) || IS_IMMORTAL(victim))
		return TRUE;

	clevel = ch->level;
	vlevel = victim->level;
	gap = abs(clevel - vlevel);

	if (gap <= 10 || (IS_SET(victim->pact, PLR_DRAGONPIT) && IS_SET(ch->pact, PLR_DRAGONPIT)) || IS_SET(ch->in_room->room_flags, ROOM_DRAGONPIT))
		return TRUE;

	return FALSE;
}




