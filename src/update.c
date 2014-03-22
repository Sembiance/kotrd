#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "interp.h"

void talk_auction (char *argument, CHAR_DATA * ch);
void raw_kill( CHAR_DATA *victim, CHAR_DATA *killer );
void save_helps(void);
bool isdp;
int remvalue;

/*
 * Local functions.
 */
int	hit_gain	args( ( CHAR_DATA *ch ) );
int	mana_gain	args( ( CHAR_DATA *ch ) );
int	move_gain	args( ( CHAR_DATA *ch ) );
void	mobile_update	args( ( void ) );
void	weather_update	args( ( void ) );
void	char_update	args( ( void ) );
void	obj_update	args( ( void ) );
void	aggr_update	args( ( void ) );
void    auction_update  args( ( void ) );
void    quest_update    args( ( void ) ); /* Vassago - quest.c */
void    dtrap_update    args( ( void ) );
void    dp_update    args( ( void ) );
void    remort_update   args( ( void ) );
void    write_online args( ( void ) );
//////////////////////////////////////////////////////////////////////////
// Acrophobia Stuff
void	acro_update  args( ( void ) );

// Explorer/Killer Percentages
void do_updatetop(CHAR_DATA *ch, char *argument );

// Stones of Wisdom Stuff
void	stones_update(void);

void psycho_update(void);

/* used for saving */

int	save_number = 0;

// LEGEND SYSTEM
bool legend_can_raise_level(CHAR_DATA *ch);

/*
 * Advancement stuff.
 */
void advance_level( CHAR_DATA *ch, bool hide, bool restore )
{
    char buf[MAX_STRING_LENGTH];
    int add_hp;
    int add_mana;
    int add_move;
    int add_prac;

  ch->pcdata->share_level = ch->level;
  ch->pcdata->shares_bought = 0;

    ch->pcdata->last_level = 
	( ch->played + (int) (current_time - ch->logon) ) / 3600;

    add_hp	= ((con_app[get_curr_stat(ch,STAT_CON)].hitp) + number_range(
		    class_table[ch->class].hp_min,
		    class_table[ch->class].hp_max ));
    add_mana 	= number_range(2,(2*get_curr_stat(ch,STAT_INT)
				  + get_curr_stat(ch,STAT_WIS))/5);
    if (!class_table[ch->class].fMana)
	add_mana /= 2;

    add_move	= number_range( 1, (get_curr_stat(ch,STAT_CON)
				  + get_curr_stat(ch,STAT_DEX))/6 );
    add_prac	= wis_app[get_curr_stat(ch,STAT_WIS)].practice;

    add_hp   = add_hp * 9/10;
    add_mana = add_mana * 9/10;
    add_move = add_move * 9/10;

    add_hp	= UMAX(  2, add_hp   );
    add_mana	= UMAX(  2, add_mana );
    add_move	= UMAX(  6, add_move );

    if(!IS_HARDCORE(ch) || (IS_HARDCORE(ch) && ch->level>ch->pcdata->previousHCLevel))
    {    
        ch->max_hit 	+= add_hp;
        ch->max_mana	+= add_mana;
        ch->max_move	+= add_move;
        ch->practice	+= add_prac;
        ch->train		+= 1;
    
        ch->pcdata->perm_hit	+= add_hp;
        ch->pcdata->perm_mana	+= add_mana;
        ch->pcdata->perm_move	+= add_move;
    }

    if (!hide)
    {
        if(!IS_HARDCORE(ch) || (IS_HARDCORE(ch) && ch->level>ch->pcdata->previousHCLevel))
        {
        	sprintf(buf,
    	    "{CYou gain {R%d hit point%s{C, {B%d mana{C, {Y%d moves{C, and {G%d practice%s.{x\n\r",
    
    	    add_hp, add_hp == 1 ? "" : "s", add_mana, add_move,
    	    add_prac, add_prac == 1 ? "" : "s");
    	   send_to_char( buf, ch );
    	}
    	else
    	{
    	   sprintf(buf, "{CBecause you are {RHardcore{C you will not gain trains and pracs again until you reach Level %d.{x\n\r", ch->pcdata->previousHCLevel+1);
    	   send_to_char(buf, ch);
    	}
    }
    if(restore)
    {
	   do_function(ch, &do_restore, ch->name);
       send_to_char( "{CThe Gods have given you the power to continue.{x\n\r", ch );
    }
    return;
}   



void gain_exp( CHAR_DATA *ch, int gain )
{
    char buf[MAX_STRING_LENGTH]; 
    DESCRIPTOR_DATA *d;
    CHAR_DATA *wch;
CHAR_DATA * original;
    CHAR_DATA * lastMortal=0;
    
    if ( IS_NPC(ch) || ch->level >= LEVEL_HERO )
	return;
   
   // 1.5x bonus exp for bottom of who list < 100 mortal: addict code
  for ( d = descriptor_list; d; d = d->next )
    {
        if ( d->connected != CON_PLAYING)
            continue;

        wch   = ( d->original != NULL ) ? d->original : d->character;
        
        if(wch->level<LEVEL_HERO && ((current_time - wch->logon)/60 >= 15) && !IS_SET(wch->comm, COMM_AFK))
            lastMortal = wch;

    }
    
    if(ch==lastMortal)
        gain*=1.5;
   
   
    if (gain >= 551)
     gain = 550;

    ch->exp = UMAX( exp_per_level(ch,ch->pcdata->points), ch->exp + gain );
    while ( ch->level < LEVEL_HERO && ch->exp >= exp_per_level(ch,ch->pcdata->points) * (ch->level+1) )
    {
        if(IS_LEGEND(ch) && !legend_can_raise_level(ch))
        {
            sprintf(buf, "\n\r{RYou have not yet found your {cLevel %d {YLegend Shard{R and may not advance to {cLevel %d{R until you find it.{x\n\r", ch->level, ch->level+1);
            send_to_char(buf, ch);
            
            ch->exp = (exp_per_level(ch,ch->pcdata->points) * (ch->level+1))-1;
            
            return;
            
        }
        
    
	send_to_char( "{CYou raise a level!!  {x\n\r", ch );
	ch->level += 1;

    if(IS_LEGEND(ch))
    {
        if(ch->level==LEVEL_HERO)
            sprintf(buf, "{W[{RG{YR{GA{CT{MS{W] {xMota{W: {G%s has become a %sLevel {C%d {W%s {M{{{YLegend{M}{R!{Y!{R!{Y!{R!{Y!{R!{Y!{R!{Y!{R!{Y!{R!{Y!{R!{Y!{R!{Y!{R!{Y!{R!{Y!{R!{Y!{R!{Y!{R!{Y!{R!{Y! {x\n\r", ch->name, IS_HARDCORE(ch) ? "{RHardcore{G " : "", ch->level, legend_table[ch->pcdata->legend].name);
        else
            sprintf(buf, "{W[{RG{YR{GA{CT{MS{W] {xMota{W: {G%s has %sattained {M{{{YLegend{M}{G %sLevel {C%d{W!!!!!!!{x\n\r", ch->name, IS_HARDCORE(ch) && ch->level < ch->pcdata->previousHCLevel ? "re-" : "", IS_HARDCORE(ch) ? "{RHardcore{G " : "", ch->level);
    }
    else if(IS_REMORT(ch))
    {
        if(ch->level==LEVEL_HERO)
            sprintf(buf, "{W[{RG{YR{GA{CT{MS{W] {xMota{W: {G%s has attained {MREMORT{G %sLevel {C%d{W!! THEY ARE NOW A {MREMORT{W HERO{R!{Y!{R!{Y!{R!{Y!{R!{Y!{R!{Y!{x\n\r", ch->name, IS_HARDCORE(ch) ? "{RHardcore{G " : "", ch->level);
        else
            sprintf(buf, "{W[{RG{YR{GA{CT{MS{W] {xMota{W: {G%s has %sattained {MREMORT{G %sLevel {C%d{W!!{x\n\r", ch->name, IS_HARDCORE(ch) && ch->level < ch->pcdata->previousHCLevel ? "re-" : "", IS_HARDCORE(ch) ? "{RHardcore{G " : "", ch->level);
    }
    else
    {
        if(ch->level==LEVEL_HERO)
            sprintf(buf, "{W[{RG{YR{GA{CT{MS{W] {xMota{W: {G%s has attained %sLevel {C%d{W!! THEY ARE NOW A HERO!!!{x\n\r", ch->name, IS_HARDCORE(ch) ? "{RHardcore{G " : "", ch->level);
        else
            sprintf(buf, "{W[{RG{YR{GA{CT{MS{W] {xMota{W: {G%s has %sattained %sLevel {C%d{W!!{x\n\r", ch->name, IS_HARDCORE(ch) && ch->level < ch->pcdata->previousHCLevel ? "re-" : "", IS_HARDCORE(ch) ? "{RHardcore{G " : "", ch->level);
    }

	for(d=descriptor_list;d;d=d->next)
	{
		original = d->original ? d->original : d->character; /* if switched */
		if(d->connected==CON_PLAYING)
		  act_new(buf, original, NULL, NULL, TO_CHAR, POS_DEAD);
	}

    //talk_channel( ch, buf, CHANNEL_GRAT, "grats" );
   // do_gecho (ch, buf);
        
        wiznet(buf,ch,NULL,WIZ_LEVELS,0,0); 
	advance_level(ch,FALSE, TRUE);
	save_char_obj(ch);
    }

    return;
}



/*
 * Regeneration stuff.
 */
int hit_gain( CHAR_DATA *ch )
{
    int gain;
    int number;

    if (ch->in_room == NULL)
	return 0;

    if ( IS_NPC(ch) )
    {
	gain =  5 + ch->level;
 	if (IS_AFFECTED(ch,AFF_REGENERATION))
	    gain *= 2;

	switch(ch->position)
	{
	    default : 		gain /= 2;			break;
	    case POS_SLEEPING: 	gain = 3 * gain/2;		break;
	    case POS_RESTING:  					break;
	    case POS_FIGHTING:	gain /= 3;		 	break;
 	}

	
    }
    else
    {
	gain = UMAX(3,get_curr_stat(ch,STAT_CON) - 3 + ch->level/2); 
	gain += class_table[ch->class].hp_max - 10;
 	number = number_percent();
	if (number < get_skill(ch,gsn_fast_healing))
	{
	    gain += number * gain / 100;
	    if (ch->hit < ch->max_hit)
		check_improve(ch,gsn_fast_healing,TRUE,8);
	}

	switch ( ch->position )
	{
	    default:	   	gain /= 4;			break;
	    case POS_SLEEPING: 					break;
	    case POS_RESTING:  	gain /= 2;			break;
	    case POS_FIGHTING: 	gain /= 6;			break;
	}

	if ( ch->pcdata->condition[COND_HUNGER]   == 0 )
	    gain /= 2;

	if ( ch->pcdata->condition[COND_THIRST] == 0 )
	    gain /= 2;

    }

    gain = gain * ch->in_room->heal_rate / 100;
    
    if (ch->on != NULL && ch->on->item_type == ITEM_FURNITURE)
	gain = gain * ch->on->value[3] / 100;

    if ( IS_AFFECTED(ch, AFF_POISON) )
	gain /= 6;

    if (IS_AFFECTED(ch, AFF_PLAGUE))
	gain /= 8;

    if (IS_AFFECTED(ch,AFF_HASTE) || IS_AFFECTED(ch,AFF_SLOW))
	gain /=2 ;
	
	// Explorer/Killer Percentages
    if(gTopKiller && !strcasecmp(gTopKiller, ch->name))
        gain*=1.15;
    if(gTopExplorer && !strcasecmp(gTopExplorer, ch->name))
        gain*=1.15;
    if(gTopTreasureHunter && !strcasecmp(gTopTreasureHunter, ch->name))
        gain*=1.15;

    return UMIN(gain, ch->max_hit - ch->hit);
}



int mana_gain( CHAR_DATA *ch )
{
    int gain;
    int number;

    if (ch->in_room == NULL)
	return 0;

    if ( IS_NPC(ch) )
    {
	gain = 5 + ch->level;
	switch (ch->position)
	{
	    default:		gain /= 2;		break;
	    case POS_SLEEPING:	gain = 3 * gain/2;	break;
   	    case POS_RESTING:				break;
	    case POS_FIGHTING:	gain /= 3;		break;
    	}
    }
    else
    {
	gain = (get_curr_stat(ch,STAT_WIS) 
	      + get_curr_stat(ch,STAT_INT) + ch->level) / 2;
	number = number_percent();
	if (number < get_skill(ch,gsn_meditation))
	{
	    gain += number * gain / 100;
	    if (ch->mana < ch->max_mana)
	        check_improve(ch,gsn_meditation,TRUE,8);
	}
	if (!class_table[ch->class].fMana)
	    gain /= 2;

	switch ( ch->position )
	{
	    default:		gain /= 4;			break;
	    case POS_SLEEPING: 					break;
	    case POS_RESTING:	gain /= 2;			break;
	    case POS_FIGHTING:	gain /= 6;			break;
	}

	if ( ch->pcdata->condition[COND_HUNGER]   == 0 )
	    gain /= 2;

	if ( ch->pcdata->condition[COND_THIRST] == 0 )
	    gain /= 2;

    }

    gain = gain * ch->in_room->mana_rate / 100;

    if (ch->on != NULL && ch->on->item_type == ITEM_FURNITURE)
	gain = gain * ch->on->value[4] / 100;

    if ( IS_AFFECTED( ch, AFF_POISON ) )
	gain /= 6;

    if (IS_AFFECTED(ch, AFF_PLAGUE))
        gain /= 8;

    if (IS_AFFECTED(ch,AFF_HASTE) || IS_AFFECTED(ch,AFF_SLOW))
        gain /=2 ;

	// Explorer/Killer Percentages
    if(gTopKiller && !strcasecmp(gTopKiller, ch->name))
        gain*=1.15;
    if(gTopExplorer && !strcasecmp(gTopExplorer, ch->name))
        gain*=1.15;
    if(gTopTreasureHunter && !strcasecmp(gTopTreasureHunter, ch->name))
        gain*=1.15;

    return UMIN(gain, ch->max_mana - ch->mana);
}



int move_gain( CHAR_DATA *ch )
{
    int gain;

    if (ch->in_room == NULL)
	return 0;

    if ( IS_NPC(ch) )
    {
	gain = ch->level;
    }
    else
    {
	gain = UMAX( 15, ch->level );

	switch ( ch->position )
	{
	case POS_SLEEPING: gain += get_curr_stat(ch,STAT_DEX);		break;
	case POS_RESTING:  gain += get_curr_stat(ch,STAT_DEX) / 2;	break;
	}

	if ( ch->pcdata->condition[COND_HUNGER]   == 0 )
	    gain /= 2;

	if ( ch->pcdata->condition[COND_THIRST] == 0 )
	    gain /= 2;
    }

    gain = gain * ch->in_room->heal_rate/100;

    if (ch->on != NULL && ch->on->item_type == ITEM_FURNITURE)
	gain = gain * ch->on->value[3] / 100;

    if ( IS_AFFECTED(ch, AFF_POISON) )
	gain /= 6;

    if (IS_AFFECTED(ch, AFF_PLAGUE))
        gain /= 8;

    if (IS_AFFECTED(ch,AFF_HASTE) || IS_AFFECTED(ch,AFF_SLOW))
        gain /=2 ;

	// Explorer/Killer Percentages
    if(gTopKiller && !strcasecmp(gTopKiller, ch->name))
        gain*=1.15;
    if(gTopExplorer && !strcasecmp(gTopExplorer, ch->name))
        gain*=1.15;
    if(gTopTreasureHunter && !strcasecmp(gTopTreasureHunter, ch->name))
        gain*=1.15;

    return UMIN(gain, ch->max_move - ch->move);
}



void gain_condition( CHAR_DATA *ch, int iCond, int value )
{
    int condition;

    if ( value == 0 || IS_NPC(ch) || ch->level >= LEVEL_IMMORTAL)
	return;

    condition				= ch->pcdata->condition[iCond];
    if (condition == -1)
        return;
    ch->pcdata->condition[iCond]	= URANGE( 0, condition + value, 48 );

    if ( ch->pcdata->condition[iCond] == 0 )
    {
	switch ( iCond )
	{
	case COND_HUNGER:
	    send_to_char( "You are hungry.\n\r",  ch );
	    break;

	case COND_THIRST:
	    send_to_char( "You are thirsty.\n\r", ch );
	    break;

	case COND_DRUNK:
	    if ( condition != 0 )
		send_to_char( "You are sober.\n\r", ch );
	    break;
	}
    }

    return;
}



/*
 * Mob autonomous action.
 * This function takes 25% to 35% of ALL Merc cpu time.
 * -- Furey
 */
void mobile_update( void )
{
    CHAR_DATA *ch;
    CHAR_DATA *ch_next;
    EXIT_DATA *pexit;
    int door;

    /* Examine all mobs. */
    for ( ch = char_list; ch != NULL; ch = ch_next )
    {
	ch_next = ch->next;

	if ( !IS_NPC(ch) || ch->in_room == NULL || IS_AFFECTED(ch,AFF_CHARM))
	    continue;


	/* Examine call for special procedure */
	if ( ch->spec_fun != 0 )
	{
	    if ( (*ch->spec_fun) ( ch ) )
		continue;
	}

	if (ch->pIndexData->pShop != NULL) /* give him some gold */
	    if ((ch->gold * 100 + ch->silver) < ch->pIndexData->wealth)
	    {
		ch->gold += ch->pIndexData->wealth * number_range(1,20)/5000000;
		ch->silver += ch->pIndexData->wealth * number_range(1,20)/50000;
	    }
	 
	/*
	 * Check triggers only if mobile still in default position
	 */
	if ( ch->position == ch->pIndexData->default_pos )
	{
	    /* Delay */
	    if ( HAS_TRIGGER( ch, TRIG_DELAY) 
	    &&   ch->mprog_delay > 0 )
	    {
		if ( --ch->mprog_delay <= 0 )
		{
		    mp_percent_trigger( ch, NULL, NULL, NULL, TRIG_DELAY );
		    continue;
		}
	    } 
	    if ( HAS_TRIGGER( ch, TRIG_RANDOM) )
	    {
		if( mp_percent_trigger( ch, NULL, NULL, NULL, TRIG_RANDOM ) )
		continue;
	    }
	}

	/* That's all for sleeping / busy monster, and empty zones */
	if ( ch->position != POS_STANDING )
	    continue;

	/* Scavenge */
	if ( IS_SET(ch->act, ACT_SCAVENGER)
	&&   ch->in_room->contents != NULL
	&&   number_bits( 6 ) == 0 )
	{
	    OBJ_DATA *obj;
	    OBJ_DATA *obj_best;
	    int max;

	    max         = 1;
	    obj_best    = 0;
	    for ( obj = ch->in_room->contents; obj; obj = obj->next_content )
	    {
		if ( CAN_WEAR(obj, ITEM_TAKE) && can_loot(ch, obj)
		     && obj->cost > max  && obj->cost > 0)
		{
		    obj_best    = obj;
		    max         = obj->cost;
		}
	    }

	    if ( obj_best )
	    {
		obj_from_room( obj_best );
		obj_to_char( obj_best, ch );
		act( "$n gets $p.", ch, obj_best, NULL, TO_ROOM );
	    }
	}

	/* Wander */
	if ( !IS_SET(ch->act, ACT_SENTINEL) 
	&& number_bits(3) == 0
	&& ( door = number_bits( 9 ) ) <= 9
	&& ( pexit = ch->in_room->exit[door] ) != NULL
	&&   pexit->u1.to_room != NULL
	&&   !IS_SET(pexit->exit_info, EX_CLOSED)
	&&   !IS_SET(pexit->u1.to_room->room_flags, ROOM_NO_MOB)
	&& ( !IS_SET(ch->act, ACT_STAY_AREA)
	||   pexit->u1.to_room->area == ch->in_room->area ) 
	&& ( !IS_SET(ch->act, ACT_OUTDOORS)
	||   !IS_SET(pexit->u1.to_room->room_flags,ROOM_INDOORS)) 
	&& ( !IS_SET(ch->act, ACT_INDOORS)
	||   IS_SET(pexit->u1.to_room->room_flags,ROOM_INDOORS)))
	{
	    move_char( ch, door, FALSE );
	}
    }

    return;
}



void dtrap_update( void )
{
   CHAR_DATA *ch;
   CHAR_DATA *ch_next;

   for ( ch = char_list; ch != NULL; ch = ch_next)
   {
       ch_next = ch->next;
             
       if (!IS_NPC(ch)
       && !IS_IMMORTAL(ch)
       && IS_SET(ch->in_room->room_flags, ROOM_DEATHTRAP) )
       {
           do_look( ch, "dt" );

          if (ch->position == POS_STANDING && ch->hit > 20)
             {
             ch->position = POS_RESTING;
              ch->hit /= 2;
             send_to_char("You better get out of here fast!!!!\n\r", ch);
             }
             else
             {
             ch->hit = 1;
             raw_kill(ch,ch);   
             send_to_char("You are dead!!!!", ch);
             }
      }
    }


}




/*
 * Update the weather.
 */
void weather_update( void )
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
    int diff;

    buf[0] = '\0';

    switch ( ++time_info.hour )
    {
    case  5:
	weather_info.sunlight = SUN_LIGHT;
	strcat( buf, "The day has begun.\n\r" );
	break;

    case  6:
	weather_info.sunlight = SUN_RISE;
	strcat( buf, "The sun rises in the east.\n\r" );
	break;

    case 19:
	weather_info.sunlight = SUN_SET;
	strcat( buf, "The sun slowly disappears in the west.\n\r" );
	break;

    case 20:
	weather_info.sunlight = SUN_DARK;
	strcat( buf, "The night has begun.\n\r" );
	break;

    case 24:
	time_info.hour = 0;
	time_info.day++;
	break;
    }

    if ( time_info.day   >= 35 )
    {
	time_info.day = 0;
	time_info.month++;
    }

    if ( time_info.month >= 17 )
    {
	time_info.month = 0;
	time_info.year++;
    }

    /*
     * Weather change.
     */
    if ( time_info.month >= 9 && time_info.month <= 16 )
	diff = weather_info.mmhg >  985 ? -2 : 2;
    else
	diff = weather_info.mmhg > 1015 ? -2 : 2;

    weather_info.change   += diff * dice(1, 4) + dice(2, 6) - dice(2, 6);
    weather_info.change    = UMAX(weather_info.change, -12);
    weather_info.change    = UMIN(weather_info.change,  12);

    weather_info.mmhg += weather_info.change;
    weather_info.mmhg  = UMAX(weather_info.mmhg,  960);
    weather_info.mmhg  = UMIN(weather_info.mmhg, 1040);

    switch ( weather_info.sky )
    {
    default: 
	bug( "Weather_update: bad sky %d.", weather_info.sky );
	weather_info.sky = SKY_CLOUDLESS;
	break;

    case SKY_CLOUDLESS:
	if ( weather_info.mmhg <  990
	|| ( weather_info.mmhg < 1010 && number_bits( 2 ) == 0 ) )
	{
	    strcat( buf, "The sky is getting cloudy.\n\r" );
	    weather_info.sky = SKY_CLOUDY;
	}
	break;

    case SKY_CLOUDY:
	if ( weather_info.mmhg <  970
	|| ( weather_info.mmhg <  990 && number_bits( 2 ) == 0 ) )
	{
	    strcat( buf, "It starts to rain.\n\r" );
	    weather_info.sky = SKY_RAINING;
	}

	if ( weather_info.mmhg > 1030 && number_bits( 2 ) == 0 )
	{
	    strcat( buf, "The clouds disappear.\n\r" );
	    weather_info.sky = SKY_CLOUDLESS;
	}
	break;

    case SKY_RAINING:
	if ( weather_info.mmhg <  970 && number_bits( 2 ) == 0 )
	{
	    strcat( buf, "Lightning flashes in the sky.\n\r" );
	    weather_info.sky = SKY_LIGHTNING;
	}

	if ( weather_info.mmhg > 1030
	|| ( weather_info.mmhg > 1010 && number_bits( 2 ) == 0 ) )
	{
	    strcat( buf, "The rain stopped.\n\r" );
	    weather_info.sky = SKY_CLOUDY;
	}
	break;

    case SKY_LIGHTNING:
	if ( weather_info.mmhg > 1010
	|| ( weather_info.mmhg >  990 && number_bits( 2 ) == 0 ) )
	{
	    strcat( buf, "The lightning has stopped.\n\r" );
	    weather_info.sky = SKY_RAINING;
	    break;
	}
	break;
    }

    if ( buf[0] != '\0' )
    {
	for ( d = descriptor_list; d != NULL; d = d->next )
	{
	    if ( d->connected == CON_PLAYING
	    &&   IS_OUTSIDE(d->character)
	    &&   IS_AWAKE(d->character) )
		send_to_char( buf, d->character );
	}
    }

    return;
}


void remort_update( void )
{
 OBJ_DATA *obj;
 OBJ_INDEX_DATA *pObjIndex;
 ROOM_INDEX_DATA *rRoom;
 ROOM_INDEX_DATA *pRoom;
 int room_vnum;
 int proom_vnum;
 
 room_vnum = number_range(1300, 31500);

 if ( remvalue == 0 )
   {
    if ( (rRoom = get_room_index(room_vnum) ) != NULL )
      {
         proom_vnum = room_vnum + 1;

         pRoom = get_room_index(proom_vnum);

        if  ((!room_is_private(rRoom))
        &&  (rRoom != get_room_index(ROOM_VNUM_LIMBO))
        &&  (rRoom != get_room_index(ROOM_VNUM_IMM))
        &&  (rRoom != get_room_index(ROOM_VNUM_SCHOOL))
        &&  (rRoom != get_room_index(ROOM_VNUM_JAIL))
        &&  (rRoom != get_room_index(ROOM_VNUM_DRAGONPIT))
        &&  (rRoom != get_room_index(ROOM_VNUM_DRAGONPIT_RETURN))
        &&  (rRoom->clanowner < 1)
        &&  !IS_SET(rRoom->room_flags, ROOM_PRIVATE)
        &&  !IS_SET(rRoom->room_flags, ROOM_SOLITARY)
        &&  !IS_SET(rRoom->room_flags, ROOM_MUD_SCHOOL)
        &&  !IS_SET(rRoom->room_flags, ROOM_DEATHTRAP)
        &&  !IS_SET(rRoom->room_flags, ROOM_IMP_ONLY)
        &&  !IS_SET(rRoom->room_flags, ROOM_GODS_ONLY)
        &&  !IS_SET(rRoom->room_flags, ROOM_NEWBIES_ONLY)
        &&  !IS_SET(rRoom->room_flags, ROOM_HEROES_ONLY)
        &&  !IS_SET(rRoom->room_flags, ROOM_NOWHERE)
        &&  !IS_SET(rRoom->room_flags, ROOM_DRAGONPIT)
        &&  !IS_SET(rRoom->area->area_flags, AREA_PROTO )
        &&  (!IS_SET(rRoom->room_flags, ROOM_PET_SHOP)
        ||   !IS_SET(pRoom->room_flags, ROOM_PET_SHOP)))
           {
            remvalue = room_vnum;
            
            if (( pObjIndex = get_obj_index(OBJ_VNUM_REMORT_PORTAL ) ) != NULL )
              {
               obj = create_object( pObjIndex, 1 );
               obj_to_room( obj, rRoom );
              }
           }
          else
           { 
            remvalue = 0; 
           }
      }
     else
      remvalue = 0;
  }
 return;
}


/*
 * Update all chars, including mobs.
*/
void char_update( void )
{   
    CHAR_DATA *ch;
    CHAR_DATA *ch_next;
    CHAR_DATA *ch_quit;

    ch_quit	= NULL;

    /* update save counter */
    save_number++;

    if (save_number > 15)
	save_number = 0;

    for ( ch = char_list; ch != NULL; ch = ch_next )
    {
	AFFECT_DATA *paf;
	AFFECT_DATA *paf_next;

	ch_next = ch->next;


       if (!IS_SET(ch->pact, PLR_DRAGONPIT))
         {
          if ( ch->timer > 20 )
            ch_quit = ch;
         }

      if (!IS_NPC(ch))
         {
        if ((ch->in_room->room_flags == ROOM_DRAGONPIT)
        && (isdp == FALSE))
          {
           if (IS_SET(ch->pact, PLR_DRAGONPIT))
             { REMOVE_BIT(ch->pact, PLR_DRAGONPIT); }           

             char_from_room(ch);
             char_to_room(ch, (get_room_index(ROOM_VNUM_DRAGONPIT_RETURN)));
          }
         }
       
        if (ch->pk_timer > 0)
          ch->pk_timer -= 1;
    if(ch->position == POS_INCAP && !IS_NPC(ch) && ch->pcdata)
    {
        ch->pcdata->incap_duration++;
        if(ch->pcdata->incap_duration>30)   // kill them after 30 ticks
        {
             ch->hit = 1;
            raw_kill(ch, ch);
        }
            
    }
    else if ( ch->position >= POS_STUNNED )
	{
            /* check to see if we need to go home */
            if (IS_NPC(ch) && ch->zone != NULL && ch->zone != ch->in_room->area
            && ch->desc == NULL &&  ch->fighting == NULL 
	    && !IS_AFFECTED(ch,AFF_CHARM) && number_percent() < 5)
            {
            	act("$n wanders on home.",ch,NULL,NULL,TO_ROOM);
            	extract_char(ch,TRUE);
            	continue;
            }

	    if ( ch->hit  < ch->max_hit )
		ch->hit  += hit_gain(ch);
	    else
		ch->hit = ch->max_hit;

	    if ( ch->mana < ch->max_mana )
		ch->mana += mana_gain(ch);
	    else
		ch->mana = ch->max_mana;

	    if ( ch->move < ch->max_move )
		ch->move += move_gain(ch);
	    else
		ch->move = ch->max_move;
	}

	if ( ch->position == POS_STUNNED )
	    update_pos( ch );

       if ( !IS_NPC(ch) && ch->level < LEVEL_IMMORTAL )
	{
	    OBJ_DATA *obj;

	    if ( ( (obj = get_eq_char( ch, WEAR_LIGHT ) ) != NULL)
	    &&   obj->item_type == ITEM_LIGHT_SOURCE
	    &&   obj->value[2] > 0 )
	    {
		if ( --obj->value[2] == 0 && ch->in_room != NULL )
		{
		    --ch->in_room->light;
		    act( "$p goes out.", ch, obj, NULL, TO_ROOM );
		    act( "$p flickers and goes out.", ch, obj, NULL, TO_CHAR );
		    extract_obj( obj );
		}
	 	else if ( obj->value[2] <= 5 && ch->in_room != NULL)
		    act("$p flickers.",ch,obj,NULL,TO_CHAR);
	    }

	    if (IS_IMMORTAL(ch))
		ch->timer = 0;

	    if ( ++ch->timer >= 15 )
	    {
		if ( ch->was_in_room == NULL && ch->in_room != NULL )
		{
		    ch->was_in_room = ch->in_room;

		    if ( ch->fighting != NULL )
			stop_fighting( ch, TRUE );

		    if (ch->level > 1)
		        save_char_obj( ch );

		    char_from_room( ch );
		    char_to_room( ch, get_room_index( ROOM_VNUM_LIMBO ) );
		do_quit( ch, "" );
		}
	    }

	    gain_condition( ch, COND_DRUNK,  -1 );
	    gain_condition( ch, COND_FULL, ch->size > SIZE_MEDIUM ? -4 : -2 );
	    gain_condition( ch, COND_THIRST, -1 );
	    gain_condition( ch, COND_HUNGER, ch->size > SIZE_MEDIUM ? -2 : -1);
	}

	for ( paf = ch->affected; paf != NULL; paf = paf_next )
	{
	    paf_next	= paf->next;
	    if ( paf->duration > 0 )
	    {
		paf->duration--;
		if (number_range(0,4) == 0 && paf->level > 0)
		  paf->level--;  /* spell strength fades with time */
            }
	    else if ( paf->duration < 0 )
		;
	    else
	    {
		if ( paf_next == NULL
		||   paf_next->type != paf->type
		||   paf_next->duration > 0 )
		{
		    if ( paf->type > 0 && skill_table[paf->type].msg_off )
		    {
			send_to_char( skill_table[paf->type].msg_off, ch );
			send_to_char( "\n\r", ch );
		    }
		}
	  
		affect_remove( ch, paf );
	    }
	}

	/*
	 * Careful with the damages here,
	 *   MUST NOT refer to ch after damage taken,
	 *   as it may be lethal damage (on NPC).
	 */

        if (is_affected(ch, gsn_plague) && ch != NULL)
        {
            AFFECT_DATA *af, plague;
            CHAR_DATA *vch;
            int dam;

	    if (ch->in_room == NULL)
		continue;
            
	    act("$n writhes in agony as plague sores erupt from $s skin.",
		ch,NULL,NULL,TO_ROOM);
	    send_to_char("You writhe in agony from the plague.\n\r",ch);
            for ( af = ch->affected; af != NULL; af = af->next )
            {
            	if (af->type == gsn_plague)
                    break;
            }
        
            if (af == NULL)
            {
            	REMOVE_BIT(ch->affected_by,AFF_PLAGUE);
            	continue;
            }
        
            if (af->level == 1)
            	continue;
        
	    plague.where		= TO_AFFECTS;
            plague.type 		= gsn_plague;
            plague.level 		= af->level - 1; 
            plague.duration 	= number_range(1,2 * plague.level);
            plague.location		= APPLY_STR;
            plague.modifier 	= -5;
            plague.bitvector 	= AFF_PLAGUE;
        
            for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
            {
                if (!saves_spell(plague.level - 2,vch,DAM_DISEASE) 
		&&  !IS_IMMORTAL(vch)
            	&&  !IS_AFFECTED(vch,AFF_PLAGUE) && number_bits(4) == 0)
            	{
            	    send_to_char("You feel hot and feverish.\n\r",vch);
            	    act("$n shivers and looks very ill.",vch,NULL,NULL,TO_ROOM);
            	    affect_join(vch,&plague);
            	}
            }

	    dam = UMIN(ch->level,af->level/5+1);
	    ch->mana -= dam;
	    ch->move -= dam;
	    damage( ch, ch, dam, gsn_plague,DAM_DISEASE,FALSE);
        }
	else if ( IS_AFFECTED(ch, AFF_POISON) && ch != NULL
	     &&   !IS_AFFECTED(ch,AFF_SLOW))

	{
	    AFFECT_DATA *poison;

	    poison = affect_find(ch->affected,gsn_poison);

	    if (poison != NULL)
	    {
	        act( "$n shivers and suffers.", ch, NULL, NULL, TO_ROOM );
	        send_to_char( "You shiver and suffer.\n\r", ch );
	        damage(ch,ch,poison->level + 1,gsn_poison,
		    DAM_POISON,FALSE);
	    }
	}

	else if ( ch->position == POS_INCAP && number_range(0,1) == 0)
	{
	    damage( ch, ch, 1, TYPE_UNDEFINED, DAM_NONE,FALSE);
	}
	else if ( ch->position == POS_MORTAL )
	{
	    damage( ch, ch, 1, TYPE_UNDEFINED, DAM_NONE,FALSE);
	}
    }

    /*
     * Autosave and autoquit.
     * Check that these chars still exist.
     */
    for ( ch = char_list; ch != NULL; ch = ch_next )
    {
        ch_next = ch->next;

	if (ch->desc != NULL && ch->desc->descriptor % 30 == save_number)
	{
	    save_char_obj(ch);
	}

        if (ch == ch_quit)
	{
            do_function(ch, &do_quit, "" );
	}
    }

    return;
}






/*
 * Update all objs.
 * This function is performance sensitive.
 */
void obj_update( void )
{   
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    AFFECT_DATA *paf, *paf_next;

    for ( obj = object_list; obj != NULL; obj = obj_next )
    {
	CHAR_DATA *rch;
	char *message;

	obj_next = obj->next;

	/* go through affects and decrement */
        for ( paf = obj->affected; paf != NULL; paf = paf_next )
        {
            paf_next    = paf->next;
            if ( paf->duration > 0 )
            {
                paf->duration--;
                if (number_range(0,4) == 0 && paf->level > 0)
                  paf->level--;  /* spell strength fades with time */
            }
            else if ( paf->duration < 0 )
                ;
            else
            {
                if ( paf_next == NULL
                ||   paf_next->type != paf->type
                ||   paf_next->duration > 0 )
                {
                    if ( paf->type > 0 && skill_table[paf->type].msg_obj )
                    {
			if (obj->carried_by != NULL)
			{
			    rch = obj->carried_by;
			    act(skill_table[paf->type].msg_obj,
				rch,obj,NULL,TO_CHAR);
			}
			if (obj->in_room != NULL 
			&& obj->in_room->people != NULL)
			{
			    rch = obj->in_room->people;
			    act(skill_table[paf->type].msg_obj,
				rch,obj,NULL,TO_ALL);
			}
                    }
                }

                affect_remove_obj( obj, paf );
            }
        }


	if ( obj->timer <= 0 || --obj->timer > 0 )
	    continue;

	switch ( obj->item_type )
	{
	default:              message = "$p crumbles into dust.";  break;
	case ITEM_FOUNTAIN:   message = "$p dries up.";         break;
	case ITEM_CORPSE_NPC: message = "$p decays into dust."; break;
	case ITEM_CORPSE_PC:  message = "$p decays into dust."; break;
	case ITEM_FOOD:       message = "$p decomposes.";	break;
	case ITEM_POTION:     message = "$p has evaporated from disuse.";	
								break;
	case ITEM_PORTAL:     message = "$p fades out of existence."; break;
	case ITEM_CONTAINER: 
	    if (CAN_WEAR(obj,ITEM_WEAR_FLOAT))
		if (obj->contains)
		    message = 
		"$p flickers and vanishes, spilling its contents on the floor.";
		else
		    message = "$p flickers and vanishes.";
	    else
		message = "$p crumbles into dust.";
	    break;
	}

	if ( obj->carried_by != NULL )
	{
	    if (IS_NPC(obj->carried_by) 
	    &&  obj->carried_by->pIndexData->pShop != NULL)
		obj->carried_by->silver += obj->cost/5;
	    else
	    {
	    	act( message, obj->carried_by, obj, NULL, TO_CHAR );
		if ( obj->wear_loc == WEAR_FLOAT)
		    act(message,obj->carried_by,obj,NULL,TO_ROOM);
	    }
	}
	else if ( obj->in_room != NULL
	&&      ( rch = obj->in_room->people ) != NULL )
	{
	    if (! (obj->in_obj && obj->in_obj->pIndexData->vnum == OBJ_VNUM_PIT
	           && !CAN_WEAR(obj->in_obj,ITEM_TAKE)))
	    {
	    	act( message, rch, obj, NULL, TO_ROOM );
	    	act( message, rch, obj, NULL, TO_CHAR );
	    }
	}

        if ((obj->item_type == ITEM_CORPSE_PC || obj->wear_loc == WEAR_FLOAT)
	&&  obj->contains)
	{   /* save the contents */
     	    OBJ_DATA *t_obj, *next_obj;

	    for (t_obj = obj->contains; t_obj != NULL; t_obj = next_obj)
	    {
		next_obj = t_obj->next_content;
		obj_from_obj(t_obj);

		if (obj->in_obj) /* in another object */
		    obj_to_obj(t_obj,obj->in_obj);

		else if (obj->carried_by)  /* carried */
		    if (obj->wear_loc == WEAR_FLOAT)
			if (obj->carried_by->in_room == NULL)
			    extract_obj(t_obj);
			else
			    obj_to_room(t_obj,obj->carried_by->in_room);
		    else
		    	obj_to_char(t_obj,obj->carried_by);

		else if (obj->in_room == NULL)  /* destroy it */
		    extract_obj(t_obj);

		else /* to a room */
		    obj_to_room(t_obj,obj->in_room);
	    }
	}

	extract_obj( obj );
    }

    return;
}



/*
 * Aggress.
 *
 * for each mortal PC
 *     for each mob in room
 *         aggress on some random PC
 *
 * This function takes 25% to 35% of ALL Merc cpu time.
 * Unfortunately, checking on each PC move is too tricky,
 *   because we don't the mob to just attack the first PC
 *   who leads the party into the room.
 *
 * -- Furey
 */
void aggr_update( void )
{
    CHAR_DATA *wch;
    CHAR_DATA *wch_next;
    CHAR_DATA *ch;
    CHAR_DATA *ch_next;
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    CHAR_DATA *victim;

    for ( wch = char_list; wch != NULL; wch = wch_next )
    {
	wch_next = wch->next;
	if ( IS_NPC(wch)
	||   wch->level >= LEVEL_IMMORTAL
	||   wch->in_room == NULL 
	||   wch->in_room->area->empty)
	    continue;

	for ( ch = wch->in_room->people; ch != NULL; ch = ch_next )
	{
	    int count;

	    ch_next	= ch->next_in_room;

	    if ( !IS_NPC(ch)
	    ||   !IS_SET(ch->act, ACT_AGGRESSIVE)
	    ||   IS_SET(ch->in_room->room_flags,ROOM_SAFE)
	    ||   IS_AFFECTED(ch,AFF_CALM)
	    ||   ch->fighting != NULL
	    ||   IS_AFFECTED(ch, AFF_CHARM)
	    ||   !IS_AWAKE(ch)
	    ||   ( IS_SET(ch->act, ACT_WIMPY) && IS_AWAKE(wch) )
	    ||   !can_see( ch, wch ) 
	    ||   number_bits(1) == 0)
		continue;

	    /*
	     * Ok we have a 'wch' player character and a 'ch' npc aggressor.
	     * Now make the aggressor fight a RANDOM pc victim in the room,
	     *   giving each 'vch' an equal chance of selection.
	     */
	    count	= 0;
	    victim	= NULL;
	    for ( vch = wch->in_room->people; vch != NULL; vch = vch_next )
	    {
		vch_next = vch->next_in_room;

		if ( !IS_NPC(vch)
		&&   vch->level < LEVEL_IMMORTAL
		&&   ch->level >= vch->level - 5 
		&&   ( !IS_SET(ch->act, ACT_WIMPY) || !IS_AWAKE(vch) )
		&&   can_see( ch, vch ) )
		{
		    if ( number_range( 0, count ) == 0 )
			victim = vch;
		    count++;
		}
	    }

	    if ( victim == NULL )
		continue;

	    multi_hit( ch, victim, TYPE_UNDEFINED );
	}
    }

    return;
}

void write_online( void )
{
    DESCRIPTOR_DATA *d;
    char buf[MAX_STRING_LENGTH];
    int nMatch;
    
    FILE *fp;

    sprintf(buf,"/home/phoenix5/public_html/online.html");    // This is for new mud home, live
    //sprintf(buf,"/usr/data/httpd/users/phoenix/online.html");  //This is for the live mud
	//sprintf(buf,"/usr/users/mud/phoenix/test/fullkotrd/online.html"); //This is for the test mud

    if ( ( fp = fopen( buf, "w" ) ) == NULL )
    {
        perror( buf );
        return;
    }

    fprintf(fp, "<HTML>\n");
    fprintf(fp, "<TITLE>Characters Logged on KotRD</TITLE>\n");
    fprintf(fp, "<HEAD></HEAD>\n");

    fprintf(fp, "<BODY BGCOLOR=\"black\">\n");
    fprintf(fp, "<FONT COLOR=\"write\"><b>\n");
    fprintf(fp, "<IMG SRC=\"./tpo.jpg\"></IMG> <h3><FONT COLOR=\"RED\">\n");
    fprintf(fp, "<CENTER>\n");

    nMatch = 0;

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
		CHAR_DATA *wch2;

        if ( d->connected != CON_PLAYING )
            continue;

		wch2   = ( d->original != NULL ) ? d->original : d->character;
		if ((wch2->invis_level >= 101) || (wch2->incog_level >= 101))
			continue;

        nMatch++;
    }

    fprintf(fp, "%d\n", nMatch);

    fprintf(fp, "</FONT></h3><p>\n");

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
        CHAR_DATA *wch;
			
		if ( d->connected != CON_PLAYING )
            continue;

        wch   = ( d->original != NULL ) ? d->original : d->character;

		if ((wch->invis_level >= 101) || (wch->incog_level >= 101))
			continue;

        fprintf(fp, "%s<br>\n", wch->name);

    }

    fprintf(fp, "<hr width=\"40%%\"><FONT COLOR=\"008080\">\n");
    fprintf(fp, "<h5>Updated every minute.</h5>\n");
    fprintf(fp, "</CENTER></FONT></BODY></HTML>");

    fclose(fp);
    return;
}


/*
 * Handle all kinds of updates.
 * Called once per pulse from game loop.
 * Random times to defeat tick-timing clients and players.
 */

void update_handler( bool forced )
{
    static  int     pulse_area;
    static  int     pulse_mobile;
    static  int     pulse_dtrap;
    static  int     pulse_violence;
    static  int     pulse_point;
    static  int     pulse_autosave;

    // Explorer/Killer Percentages
    static int      pulse_toppeople;

	if ( forced )
	{
		pulse_area = 0;
		pulse_mobile = 0;
        pulse_dtrap = 0;
		pulse_violence = 0;
		pulse_point = 0;
		pulse_autosave = PULSE_AUTOSAVE;
		// Explorer/Killer Percentages
		pulse_toppeople = 0;
	}

    if ( --pulse_area     <= 0 )
    {
	pulse_area	= PULSE_AREA;
	/* number_range( PULSE_AREA / 2, 3 * PULSE_AREA / 2 ); */
	area_update	( );
    }

    if ( --pulse_mobile   <= 0 )
    {
	pulse_mobile	= PULSE_MOBILE;
	mobile_update	( );
    }

    if ( --pulse_dtrap    <= 0)
    {
        pulse_dtrap     = PULSE_DEATHTRAP;
        dtrap_update     ( );
    }
    
    if(--pulse_autosave <= 0)
    {
        pulse_autosave = PULSE_AUTOSAVE;
        save_helps();
    }

    // Explorer/Killer Percentages
    if ( --pulse_toppeople   <= 0 )
    {
	pulse_toppeople	= PULSE_TOPPEOPLE;
	do_updatetop(0, 0);
    }

    if ( --pulse_violence <= 0 )
    {
	pulse_violence	= PULSE_VIOLENCE;
	remort_update ( );
	violence_update	( );
    }

    if ( --pulse_point    <= 0 )
    {
	wiznet("{WTICK{r!{x",NULL,NULL,WIZ_TICKS,0,0); 

	pulse_point     = PULSE_TICK;
	weather_update	( );
	char_update	( );
	obj_update	( );
	quest_update     ( );
    write_online ( );
    lottery_update ( FALSE );
    save_misc ( );
    dp_update     ( );
    }

    aggr_update( );
    auction_update();
	//////////////////////////////////////////////////////////////////////////
	// Acrophobia Stuff
	acro_update();

	// Stones of Wisdom Stuff
	stones_update();
	
	psycho_update();

    tail_chain( );
    return;
}
void auction_update (void)
{
    char buf[MAX_STRING_LENGTH];

    if (auction->item != NULL)
        if (--auction->pulse <= 0) /* decrease pulse */
        {
            auction->pulse = PULSE_AUCTION;
            switch (++auction->going) /* increase the going state */
            {
            case 1 : /* going once */
            case 2 : /* going twice */
            if (auction->bet > 0)
                sprintf (buf, "{x%s{w- {ggoing %s {gfor{Y %d GOLD{g.{x",
auction->item->short_descr,
                     ((auction->going == 1) ? "{Bonce" : "{Rtwice"), auction->bet);
            else
sprintf (buf, "{x%s{w- {ggoing %s{c ({Wno bet received yet{c){g. Min Bet{w: {Y%d GOLD{g.{x",
auction->item->short_descr,
                     ((auction->going == 1) ? "{Bonce" : "{Rtwice"), auction->min);

            talk_auction (buf, auction->buyer);
            break;

            case 3 : /* SOLD! */

            if (auction->bet > 0)
            {
                sprintf (buf, "{x%s{g sold to{w: {W%s{g for{Y %d GOLD{g.{x",
                    auction->item->short_descr,
                    IS_NPC(auction->buyer) ? auction->buyer->short_descr :
auction->buyer->name,
                    auction->bet);
                talk_auction(buf, auction->buyer);
                obj_to_char (auction->item,auction->buyer);
act ("{BA small demon appears before you in a puff of smoke and hands you{w: {W$p{g.{x",
                     auction->buyer,auction->item,NULL,TO_CHAR);
                act ("{BA small demon appears before {W$n{g, and hands $m {W$p{x",
                     auction->buyer,auction->item,NULL,TO_ROOM);

                auction->seller->gold += auction->bet; /* give him the money */
                auction->item = NULL; /* reset item */

            }
            else /* not sold */
            {
                sprintf (buf, "{gNo bets received for{w: {W%s {w- {gObject has been removed.{x",
auction->item->short_descr); 
talk_auction(buf, auction->buyer);
        if(auction->seller->gold >=auction->min)
        {
                act("{BA small demon appears & takes the service fee from you.{x",
                auction->seller,auction->item,NULL,TO_CHAR);
                act("{BA small demon appears & takes some {YGOLD{B from {W$n{B.{x",
                auction->seller,auction->item,NULL,TO_ROOM);
                auction->seller->gold-=auction->min;
                act ("{BA small demon appears before you to return{w: {W$p{B to you.{x",
                      auction->seller,auction->item,NULL,TO_CHAR);
                act ("{BA small demon appears before {W$n {Bto return {W$p{B to $m.{x",
                      auction->seller,auction->item,NULL,TO_ROOM);
obj_to_char (auction->item,auction->seller);
        }
        else
        {
        act("{gThe auctioneer can not find enough gold on {W$n{g to cover the service fee.{x",
auction->seller,auction->item,NULL,TO_ROOM);
        act("{gThe auctioneer can not find enough gold on you to cover his service fee.{x",
auction->seller,auction->item,NULL,TO_CHAR);
        act("{gThe auctioneer keeps {w$p{g from {W$n{g.{x",
auction->seller,auction->item,NULL,
TO_ROOM);
        act("{gThe auctioneer keeps {w$p{g from you.{x", auction->seller,auction->item,NULL,
TO_CHAR);
extract_obj(auction->item);
        }
                auction->item = NULL; /* clear auction */
           } /* else */

            } /* switch */
        } /* if */
} /* func */


