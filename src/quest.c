#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "tables.h"

DECLARE_DO_FUN( do_say );

/*
#define QUEST_ITEM1 9224
#define QUEST_ITEM2 6013
#define QUEST_ITEM3 5230
#define QUEST_ITEM4 6506
#define QUEST_ITEM5 5632
*/

#define QUEST_OBJQUEST1 25403
#define QUEST_OBJQUEST2 25404
#define QUEST_OBJQUEST3 25405
#define QUEST_OBJQUEST4 25406
#define QUEST_OBJQUEST5 25407

/* Local functions */

void generate_quest	args(( CHAR_DATA *ch, CHAR_DATA *questman ));
void quest_update	args(( void ));
bool quest_level_diff   args(( int clevel, int mlevel));
bool chance		args(( int num ));
ROOM_INDEX_DATA 	*find_location( CHAR_DATA *ch, char *arg );

/* CHANCE function. I use this everywhere in my code, very handy :> */

bool IsQuestObjVnum(OBJ_DATA * obj)
{
    int i;
    
    if(!obj || !obj->pIndexData || !obj->pIndexData->vnum)
        return FALSE;
    
    for(i=0;quest_table[i].obj_name;i++)
    {
        if(quest_table[i].vnum==obj->pIndexData->vnum)
            return TRUE;
    }
    
    return FALSE;    
}

bool chance(int num)
{
    if (number_range(1,100) <= num) 
      return TRUE;
    else 
      return FALSE;
}

/* The main quest function */

void do_quest(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *questman;
    OBJ_DATA *obj=NULL, *obj_next;
    OBJ_INDEX_DATA *questinfoobj;
    MOB_INDEX_DATA *questinfo;
    char buf [MAX_STRING_LENGTH];
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

if(!IS_NPC(ch))
{
    if (arg1[0] == '\0')
    {
        send_to_char("\n\r{cQUEST Commands{w: {x",ch); 
        send_to_char("\n\r{CPOINTS INFO TIME REQUEST COMPLETE STOP LIST BUY{x\n\r",ch);
        send_to_char("\n\r{cFor more information, type '{WHELP QUEST{c'.{x\n\r",ch);
	return;
    }

if(!strcmp(arg1, "list") 
&& (!IS_IMP(ch)))
{
  if (IS_IMMORTAL(ch))
    {
     send_to_char("{RLeave questing for the mortals...{x\n\r", ch);
     return;
    }
}


if(!strcmp(arg1, "stop"))
{
	if(IS_SET(ch->pact, PLR_QUESTOR))
	{
         send_to_char("\n\r{WSo be it! Your quest is over!{x\n\r", ch);
         REMOVE_BIT(ch->pact, PLR_QUESTOR);
         ch->questgiver = NULL;
         ch->countdown = 0;
         ch->questmob = 0;
         ch->questobj = 0;
         ch->nextquest = 5;
	}
}

    if (!strcmp(arg1, "info"))
    {
	if (IS_SET(ch->pact, PLR_QUESTOR))
	{
	    if (ch->questmob == -1 && ch->questgiver->short_descr != NULL)
	    {
send_to_char("\n\r{cYour {WQUEST {cis {WALMOST {ccomplete!!!{x",ch);
sprintf(buf,"\n\r{cGet back to {W%s {cbefore your time runs out!{x\n\r",
ch->questgiver->short_descr);
send_to_char(buf,ch);
return;
	    }
	    else if (ch->questobj > 0)
	    {
                questinfoobj = get_obj_index(ch->questobj);
		if (questinfoobj != NULL)
		{
sprintf(buf, "{cYou are on a quest to recover the fabled {C%s{c!{x\n\r",questinfoobj->name);
		    send_to_char(buf, ch);
		}
		else send_to_char("{WYou aren't currently on a quest.{x\n\r",ch);
		return;
	    }
	    else if (ch->questmob > 0)
	    {
                questinfo = get_mob_index(ch->questmob);
		if (questinfo != NULL)
		  {
sprintf(buf, "{cYou are on a quest to slay the dreaded {C%s{c!{x\n\r",questinfo->short_descr);
		    send_to_char(buf, ch);
		}
		else send_to_char("{cYou aren't currently on a quest.{x\n\r",ch);
		return;
	    }
	}
	else
	    send_to_char("{cYou aren't currently on a quest.{x\n\r",ch);
	return;
    }
    if (!strcmp(arg1, "points"))
    {
	sprintf(buf, "\n\r{cYou have {W%d {cquest points.{x\n\r",ch->questpoints);
	send_to_char(buf, ch);
	return;
    }
    else if (!strcmp(arg1, "time"))
    {
	if (!IS_SET(ch->pact, PLR_QUESTOR))
	{
	    send_to_char("\n\r{CYou aren't currently on a Quest.{x\n\r",ch);
	    if (ch->nextquest > 1)
	    {
sprintf(buf, "\n\r{cThere are {W%d {cminutes remaining until you can go on another Quest.{x\n\r",ch->nextquest);
		send_to_char(buf, ch);
	    }
	    else if (ch->nextquest == 1)
	    {
sprintf(buf, "\n\r{cThere is less than {WONE {cminute remaining until you can go on another Quest.{x\n\r");
		send_to_char(buf, ch);
	    }
	}
        else if (ch->countdown > 0)
        {
	    sprintf(buf, "\n\r{cTime left for current quest{w: {W%d {cminutes{x\n\r",ch->countdown);
	    send_to_char(buf, ch);
	}
	return;
    }

/* Checks for a character in the room with spec_questmaster set. This special
   procedure must be defined in special.c. You could instead use an 
   ACT_QUESTMASTER flag instead of a special procedure. */

    for ( questman = ch->in_room->people; questman != NULL; questman = questman->next_in_room )

    {
	if (!IS_NPC(questman)) continue;
        if (IS_SET(questman->act, ACT_QUESTMASTER)) break;
        if( IS_NPC(ch))
        return;
    }

    if (questman == NULL)
    {
        send_to_char("\n\r{RYou can't do that here!{x\n\r",ch);
        return;
    }

    if ( questman->fighting != NULL)
    {
	send_to_char("\n\r{GWait until the fighting stops!{x\n\r",ch);
        return;
    }

    ch->questgiver = questman;

/* And, of course, you will need to change the following lines for YOUR
   quest item information. Quest items on Moongate are unbalanced, very
   very nice items, and no one has one yet, because it takes awhile to
   build up quest points :> Make the item worth their while. */

    if (!strcmp(arg1, "list"))
    {
	int iCtr;
     act( "\n\r{W$n {casks {W$N {cfor a list of Quest Items.{x\n\r", ch, NULL,questman, TO_ROOM); 
     act ("\n\r{cYou ask {W$N {cfor a list of Quest Items.{x\n\r",ch, NULL, questman,TO_CHAR);
sprintf( buf,"{c[{CObject Description        {c] [{WLvl{c] [{wQPs {c]{x\n\r");
     send_to_char( buf, ch );
	for ( iCtr = 0; quest_table[iCtr].obj_name; iCtr++ )
	  {
	  sprintf( buf, "{c[{C%30.30s{c] [{W%3d{c] [{w%4d{c]{x\n\r",
		   get_obj_index( quest_table[iCtr].vnum )->short_descr,
		   quest_table[iCtr].level, quest_table[iCtr].qp_cost );
	  send_to_char( buf, ch );
	  }
	send_to_char( "\n\r{CTo buy an item, type '{WQUEST BUY {c<{WITEM{c>{C'.{x\n\r", ch );
	return;
    }

    else if (!strcmp(arg1, "buy"))
    {
	int iCtr;

	  if (arg2[0] == '\0')
		 {
	send_to_char( "\n\r{cTo buy an item, type '{WQUEST BUY {c<{WITEM{c>'.{x\n\r", ch );
		  return;
		 }


	  else for ( iCtr = 0; quest_table[iCtr].obj_name; iCtr++ )
			 if ( is_name( arg2, quest_table[iCtr].obj_name ) )
				{

					if ( ch->questpoints < quest_table[iCtr].qp_cost)
					{
sprintf(buf, "{W%s{m, you are not Worthy of this item!!  Earn more Quest Points, and then return!{x\n\r", ch->name);
						send_to_char (buf, ch);
						return;
					}

				 obj = create_object( get_obj_index( quest_table[iCtr].vnum ), quest_table[iCtr].level );
				 break;
				}


			 if ( !obj )
	         {
	          sprintf(buf, "\n\r{YI don't have that item, {W%s{y.\n\r{x",ch->name);
	          do_say(questman, buf);
	         }
               ch->questpoints -= quest_table[iCtr].qp_cost;
	       if (obj != NULL)
	         {
    	      act( "\n\r{W$N {cgives {C$p {cto {W$n{c.{x\n\r", ch, obj, questman,TO_ROOM);
    	      act( "\n\r{W$N {cgives you {C$p{c.{x\n\r",ch, obj, questman, TO_CHAR );
	          obj_to_char(obj, ch);
	         }
	        return;
    }
    
	else if (!strcmp(arg1, "request"))
    {

        act( "\n\r{W$n {casks {W$N {cfor a quest.{x\n\r", ch, NULL, questman,TO_ROOM); 
	act ("\n\r{cYou ask {W$N {cfor a quest.{x",ch, NULL, questman, TO_CHAR);
	if (IS_SET(ch->pact, PLR_QUESTOR))
	{
	    sprintf(buf, "{RBut you're already on a quest!{x\n\r");
	    do_say(questman, buf);
	    return;
	}
	if (ch->nextquest > 0)
	{
sprintf(buf, "{cYou're very brave, {W%s{c, but let someone else have a chance.{x\n\r",ch->name);
	    do_say(questman, buf);
	    sprintf(buf, "{cCome back later.{x");
	    do_say(questman, buf);
	    return;
	}

	sprintf(buf, "{YThank you, brave {W%s{c!{x",ch->name);
	do_say(questman, buf);
        ch->questmob = 0;
	ch->questobj = 0;

	generate_quest(ch, questman);

        if (ch->questmob > 0 || ch->questobj > 0)
	{
            ch->countdown = number_range(10,30);
	    SET_BIT(ch->pact, PLR_QUESTOR);
sprintf(buf, "\n\r{cYou have {W%d {cminutes to complete this quest.{x\n\r",ch->countdown);
	    do_say(questman, buf);
	    sprintf(buf, "\n\r{cMay the gods go with you!{x\n\r");
	    do_say(questman, buf);
	}
	return;
    }
    else if (!strcmp(arg1, "complete"))
    {
act( "\n\r{W$n {cinforms {W$N {c$e has completed $s Quest.{x\n\r", ch, NULL,questman,
TO_ROOM); 
act ("\n\r{cYou inform {W$N {cyou have completed $s Quest.{x\n\r",ch, NULL, questman,
TO_CHAR);
	if (ch->questgiver != questman)
	{
sprintf(buf, "\n\r{YI never sent you on a quest! Perhaps you're thinking of someone else.{x\n\r");
	    do_say(questman,buf);
	    return;
	}

	if (IS_SET(ch->pact, PLR_QUESTOR))
	{
	    if (ch->questmob == -1 && ch->countdown > 0)
	    {
		int reward, pointreward;

	    	reward = number_range(ch->level,ch->level * 4);
  
		pointreward = ((ch->level*0.8) + 10 + (number_range(1, 21)-1));
if(pointreward<25)
	pointreward=25;
else if(pointreward>100)
	pointreward=100;
		//(level x 0.8) + (10) + (rnd(1-21)-1)
		//pointreward = number_range(UMAX(20, ch->level/2), UMIN(90, ch->level*1.5));
/*
                if (ch->level <= 80 )
  	    	  pointreward = ch->level;
                 else
	    	  pointreward = ch->level * 3/2;*/

		sprintf(buf, "\n\r{CCongratulations on completing your Quest!{x\n\r");
		do_say(questman,buf);
sprintf(buf,"\n\r{cAs a reward, I am giving you [{W%d{c] quest points, and [{W%d{c] {YGold{c.{x\n\r",pointreward,reward);
		do_say(questman,buf);

	        REMOVE_BIT(ch->pact, PLR_QUESTOR);
	        ch->questgiver = NULL;
	        ch->countdown = 0;
	        ch->questmob = 0;
		ch->questobj = 0;
	        ch->nextquest = 5;
		ch->gold += reward;
		ch->questpoints += pointreward;

	        return;
	    }
	    else if (ch->questobj > 0 && ch->countdown > 0)
	    {
		bool obj_found = FALSE;

    		for (obj = ch->carrying; obj != NULL; obj= obj_next)
    		{
        	    obj_next = obj->next_content;
        
		    if (obj != NULL && obj->pIndexData->vnum == ch->questobj)
		    {
			obj_found = TRUE;
            	        break;
		    }
        	}
		if (obj_found == TRUE)
		{
		    int reward, pointreward;

                    reward = number_range(ch->level,ch->level * 4);
                
                    /*if (ch->level <= 80 )
                      pointreward = ch->level;
                     else
                      pointreward = ch->level * 3/2;*/
pointreward = ((ch->level*0.8) + 10 + (number_range(1, 21)-1));
//pointreward = number_range(UMAX(20, ch->level/2), UMIN(90, ch->level*1.5));

if(pointreward<25)
        pointreward=25;
else if(pointreward>100)
        pointreward=100;
		    act("You hand $p to $N.",ch, obj, questman, TO_CHAR);
		    act("$n hands $p to $N.",ch, obj, questman, TO_ROOM);

	    	    sprintf(buf, "Congratulations on completing your quest!");
		    do_say(questman,buf);
		    sprintf(buf,"As a reward, I am giving you %d quest points, and %d gold.",pointreward,reward);
		    do_say(questman,buf);
	            REMOVE_BIT(ch->pact, PLR_QUESTOR);
	            ch->questgiver = NULL;
	            ch->countdown = 0;
	            ch->questmob = 0;
		    ch->questobj = 0;
	            ch->nextquest = 5;
		    ch->gold += reward;
		    ch->questpoints += pointreward;
		    extract_obj(obj);
		    return;
		}
		else
		{
		    sprintf(buf, "You haven't completed the quest yet, but there is still time!");
		    do_say(questman, buf);
		    return;
		}
		return;
	    }
	    else if ((ch->questmob > 0 || ch->questobj > 0) && ch->countdown > 0)
	    {
		sprintf(buf, "You haven't completed the quest yet, but there is still time!");
		do_say(questman, buf);
		return;
	    }
	}
	if (ch->nextquest > 0)
	    sprintf(buf,"But you didn't complete your quest in time!");
	else sprintf(buf, "You have to REQUEST a quest first, %s.",ch->name);
	do_say(questman, buf);
	return;
    }

    send_to_char("QUEST commands: POINTS INFO TIME REQUEST COMPLETE LIST BUY.\n\r",ch);
    send_to_char("For more information, type 'HELP QUEST'.\n\r",ch);
    return;
 }
}

void generate_quest(CHAR_DATA *ch, CHAR_DATA *questman)
{
    CHAR_DATA *victim;
    MOB_INDEX_DATA *vsearch;
    ROOM_INDEX_DATA *room;
    ROOM_INDEX_DATA *proom;
    OBJ_DATA *questitem;
    char buf [MAX_STRING_LENGTH];
    long mcounter;
    int mob_vnum;

    /*  Randomly selects a mob from the world mob list. If you don't
	want a mob to be selected, make sure it is immune to summon.
	Or, you could add a new mob flag called ACT_NOQUEST. The mob
	is selected for both mob and obj quests, even tho in the obj
	quest the mob is not used. This is done to assure the level
	of difficulty for the area isn't too great for the player. */

    for (mcounter = 0; mcounter < 45000; mcounter ++)
    {
	mob_vnum = number_range(1300, 45000);

	if ( (vsearch = get_mob_index(mob_vnum) ) != NULL )
	{

            if (quest_level_diff(ch->level, vsearch->level) == TRUE
                && vsearch->pShop == NULL
		&& !IS_SET(vsearch->area->area_flags, AREA_PROTO)
		&& !IS_SET(vsearch->imm_flags, IMM_SUMMON)
    		&& !IS_SET(vsearch->act, ACT_TRAIN)
		&& !IS_SET(vsearch->act, ACT_PRACTICE)
    		&& !IS_SET(vsearch->act, ACT_IS_HEALER)
		&& !IS_SET(vsearch->act, ACT_PET)
		&& !IS_SET(vsearch->act, ACT_BANKER)
		&& !IS_SET(vsearch->act, ACT_FORGER)
		&& !IS_SET(vsearch->act, ACT_QUESTMASTER)
		&& !IS_SET(vsearch->act, ACT_GAIN)
		&& !IS_SET(vsearch->affected_by, AFF_CHARM)
		&& !IS_SET(vsearch->affected_by, AFF_INVISIBLE)
                && (vsearch->spec_fun != spec_lookup( "spec_draglord" ))
                && (vsearch->spec_fun != spec_lookup( "spec_questmaster" ))
                && (vsearch != get_mob_index(MOB_VNUM_CAT))
                && (vsearch != get_mob_index(MOB_VNUM_FIDO))
                && (vsearch != get_mob_index(MOB_VNUM_COW))
                && (vsearch != get_mob_index(MOB_VNUM_WOLF))
                && (vsearch != get_mob_index(MOB_VNUM_BEAR))
                && (vsearch != get_mob_index(MOB_VNUM_RABBIT))
                && (vsearch != get_mob_index(MOB_VNUM_SNAIL))
                && (vsearch != get_mob_index(MOB_VNUM_BOAR))
                && (vsearch != get_mob_index(MOB_VNUM_SLIME))
                && (vsearch != get_mob_index(MOB_VNUM_ZOMBIE))
                && (vsearch != get_mob_index(MOB_VNUM_COMPANION))
		&& chance(40)) break;
		else vsearch = NULL;
	}
    }

    if ( vsearch == NULL || ( victim = get_char_world( ch, vsearch->player_name ) ) == NULL )
    {
	sprintf(buf, "I'm sorry, but I don't have any quests for you at this time.");
	do_say(questman, buf);
	sprintf(buf, "Try again later.");
	do_say(questman, buf);
	ch->nextquest = 2;
        return;
    }

    if ( ( room = find_location( ch, victim->name ) ) == NULL )
    {
	sprintf(buf, "I'm sorry, but I don't have any quests for you at this time.");
	do_say(questman, buf);
	sprintf(buf, "Try again later.");
	do_say(questman, buf);
	ch->nextquest = 2;
        return;
    }


    /*  40% chance it will send the player on a 'recover item' quest. */

    if (chance(40))
    {
	int objvnum = 0;

        proom = room +1;

	switch(number_range(0,4))
	{
	    case 0:
	    objvnum = QUEST_OBJQUEST1;
	    break;

	    case 1:
	    objvnum = QUEST_OBJQUEST2;
	    break;

	    case 2:
	    objvnum = QUEST_OBJQUEST3;
	    break;

	    case 3:
	    objvnum = QUEST_OBJQUEST4;
	    break;

	    case 4:
	    objvnum = QUEST_OBJQUEST5;
	    break;
	}

        questitem = create_object( get_obj_index(objvnum), ch->level );
	questitem->timer = 30;

        if  (can_see_room(ch,room)
        &&  (!room_is_private(room))
        &&  (room != get_room_index(ROOM_VNUM_LIMBO))
        &&  (room != get_room_index(ROOM_VNUM_IMM))   
        &&  (room != get_room_index(ROOM_VNUM_SCHOOL))
        &&  (room != get_room_index(ROOM_VNUM_JAIL)) 
        &&  (room != get_room_index(ROOM_VNUM_DRAGONPIT))
        &&  (room != get_room_index(ROOM_VNUM_DRAGONPIT_RETURN))
        &&  (room->clanowner < 1)
        &&  !IS_SET(room->room_flags, ROOM_PRIVATE)
        &&  !IS_SET(room->room_flags, ROOM_SOLITARY)
        &&  !IS_SET(room->room_flags, ROOM_MUD_SCHOOL)
        &&  !IS_SET(room->room_flags, ROOM_DEATHTRAP)
        &&  !IS_SET(room->room_flags, ROOM_IMP_ONLY)
        &&  !IS_SET(room->room_flags, ROOM_GODS_ONLY)
        &&  !IS_SET(room->room_flags, ROOM_NEWBIES_ONLY)
        &&  !IS_SET(room->room_flags, ROOM_HEROES_ONLY)
        &&  !IS_SET(room->room_flags, ROOM_NOWHERE)
        &&  !IS_SET(room->room_flags, ROOM_PET_SHOP)
        &&  !IS_SET(proom->room_flags, ROOM_PET_SHOP)
        &&  !IS_SET(room->room_flags, ROOM_DRAGONPIT)
        &&  !IS_SET(room->area->area_flags, AREA_PROTO ))
          obj_to_room(questitem, room);
        else
          {
           room = get_random_room(ch);
           obj_to_room(questitem, room);
          }

	ch->questobj = questitem->pIndexData->vnum;

	sprintf(buf, "Vile pilferers have stolen %s from the royal treasury!",questitem->short_descr);
	do_say(questman, buf);
	do_say(questman, "My court wizardess, with her magic mirror, has pinpointed its location.");

	/* I changed my area names so that they have just the name of the area
	   and none of the level stuff. You may want to comment these next two
	   lines. - Vassago */

	sprintf(buf, "Look in the general area of %s for %s!",room->area->name, room->name);
	do_say(questman, buf);
	return;
    }

    /* Quest to kill a mob */

    else 
    {

 if(IS_SET(room->room_flags, ROOM_SAFE)
|| IS_SET(room->room_flags, ROOM_PRIVATE)
|| IS_SET(room->room_flags, ROOM_SOLITARY)
|| IS_SET(room->room_flags, ROOM_NO_RECALL))
{
        sprintf(buf, "I'm sorry, but I don't have any quests for you at this time.");
        do_say(questman, buf);
        sprintf(buf, "Try again later.");
        do_say(questman, buf);
        ch->nextquest = 1;
        return;
}

switch(number_range(0,1))
    {
	case 0:
        sprintf(buf, "An enemy of mine, %s, is making vile threats against the crown.",victim->short_descr);
        do_say(questman, buf);
        sprintf(buf, "This threat must be eliminated!");
        do_say(questman, buf);
	break;

	case 1:
	sprintf(buf, "Rune's most heinous criminal, %s, has escaped from the dungeon!",victim->short_descr);
	do_say(questman, buf);
	sprintf(buf, "Since the escape, %s has murdered %d civillians!",victim->short_descr, number_range(2,20));
	do_say(questman, buf);
	do_say(questman,"The penalty for this crime is death, and you are to deliver the sentence!");
	break;
    }

    if (room->name != NULL)
    {
        sprintf(buf, "Seek %s out somewhere in the vicinity of %s!",victim->short_descr,room->name);
        do_say(questman, buf);

	/* I changed my area names so that they have just the name of the area
	   and none of the level stuff. You may want to comment these next two
	   lines. - Vassago */

	sprintf(buf, "That location is in the general area of %s.",room->area->name);
	do_say(questman, buf);
    }
    ch->questmob = victim->pIndexData->vnum;
    }
    return;
}

/* Level differences to search for. Moongate has 350
   levels, so you will want to tweak these greater or
   less than statements for yourself. - Vassago */

bool quest_level_diff(int clevel, int mlevel)
{
if (clevel >  1 && clevel <  6 && mlevel >  1 && mlevel <  6) return TRUE;else
if (clevel >  5 && clevel < 11 && mlevel >  5 && mlevel < 11) return TRUE;else
if (clevel > 10 && clevel < 16 && mlevel > 10 && mlevel < 16) return TRUE;else
if (clevel > 15 && clevel < 21 && mlevel > 15 && mlevel < 21) return TRUE;else
if (clevel > 20 && clevel < 26 && mlevel > 20 && mlevel < 31) return TRUE;else
if (clevel > 25 && clevel < 31 && mlevel > 25 && mlevel < 36) return TRUE;else
if (clevel > 30 && clevel < 36 && mlevel > 30 && mlevel < 41) return TRUE;else
if (clevel > 35 && clevel < 41 && mlevel > 35 && mlevel < 46) return TRUE;else
if (clevel > 40 && clevel < 46 && mlevel > 40 && mlevel < 56) return TRUE;else
if (clevel > 45 && clevel < 51 && mlevel > 45 && mlevel < 66) return TRUE;else
if (clevel > 50 && clevel < 56 && mlevel > 50 && mlevel < 71) return TRUE;else
if (clevel > 55 && clevel < 61 && mlevel > 55 && mlevel < 76) return TRUE;else
if (clevel > 60 && clevel < 66 && mlevel > 60 && mlevel < 81) return TRUE;else
if (clevel > 65 && clevel < 71 && mlevel > 65 && mlevel < 86) return TRUE;else
if (clevel > 70 && clevel < 76 && mlevel > 70 && mlevel < 96) return TRUE;else
if (clevel > 75 && clevel < 81 && mlevel > 75 && mlevel < 96) return TRUE;else
if (clevel > 80 && clevel < 86 && mlevel > 80 && mlevel < 101) return TRUE;else
if (clevel > 85 && clevel < 91 && mlevel > 85 && mlevel < 106) return TRUE;else
if (clevel > 90 && clevel < 96 && mlevel > 95 && mlevel < 121) return TRUE;else
if (clevel > 96 && clevel < 100 && mlevel > 115 && mlevel < 141) return TRUE;else
if (clevel == 100 && mlevel > 1 && mlevel < 141) return TRUE;else
return FALSE;
}
		
/* Called from update_handler() by pulse_area */


void quest_update(void)
{
    DESCRIPTOR_DATA *d;
    CHAR_DATA *ch;
 
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
        if (d->character != NULL && d->connected == CON_PLAYING)
        {
 
        ch = d->character;
 
if(!IS_NPC(ch))
  {
        if (ch->nextquest > 0)
        {
            ch->nextquest--;
            if (ch->nextquest == 0)
            {
                send_to_char("You may now quest again.\n\r",ch);
                return;
            }
        }
        else if (IS_SET(ch->pact,PLR_QUESTOR))
        {
            if (--ch->countdown <= 0)
            {
                char buf [MAX_STRING_LENGTH];
 
                ch->nextquest = 5;
                sprintf(buf, "You have run out of time for your quest!\n\rYou may quest again in %d minutes.\n\r",ch->nextquest);
                send_to_char(buf, ch);
                REMOVE_BIT(ch->pact, PLR_QUESTOR);
                ch->questgiver = NULL;
                ch->countdown = 0;
                ch->questmob = 0;
            }
            if (ch->countdown > 0 && ch->countdown < 6)
            {
                send_to_char("Better hurry, you're almost out of time for your quest!\n\r",ch);
                return;
            }
          }
        }
      }
    }
   return;
 }



