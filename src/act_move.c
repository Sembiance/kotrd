#if defined(macintosh)
#include <types.h>
#include <time.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include "merc.h"
#include "interp.h"
#include "tables.h"

DECLARE_DO_FUN (do_recall );
DECLARE_DO_FUN (do_look );
 

char *	const	dir_name	[]		=
{
    "north", "east", "south", "west", "up", "down", "northeast",
"southeast", "southwest", "northwest" 
};

const	sh_int	rev_dir		[]		=
{
    2, 3, 0, 1, 5, 4, 8, 9, 6, 7
};

const	sh_int	movement_loss	[SECT_MAX]	=
{
    1, 2, 2, 3, 4, 6, 4, 1, 6, 10, 6
};



/*
 * Local functions.
 */
void  recall  args((CHAR_DATA *ch ));
int	find_door	args( ( CHAR_DATA *ch, char *arg ) );
bool	has_key		args( ( CHAR_DATA *ch, int key ) );
bool    is_guild        args( ( int class, int vnum ) );
void	set_fighting	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );

// Stones of Wisdom Stuff
bool stones_try_moving(CHAR_DATA * ch);

bool is_guild (int class, int vnum)
  {
   int iGuild;
   
   for (iGuild = 0; iGuild < MAX_GUILD; iGuild++)

   if (class_table[class].guild[iGuild] == vnum)
     return TRUE;
   return FALSE;
  }


/* New Move   Arioch */
void move_char( CHAR_DATA *ch, int door, bool follow )
{
  CHAR_DATA *fch;
  CHAR_DATA *fch_next;
  ROOM_INDEX_DATA *in_room;
  ROOM_INDEX_DATA *to_room = NULL;
  EXIT_DATA *pexit;

  if ( door < 0 || door > 9 )
    {
    bug( "Do_move: bad door %d.", door );
    return;
    }


  if (IS_AFFECTED(ch, AFF_HIDE))
    {
    affect_strip( ch, gsn_chameleon_power );
    affect_strip( ch, gsn_hide );
    REMOVE_BIT   ( ch->affected_by, AFF_HIDE            );
    }

  if (IS_SET(ch->comm, COMM_AFK))
    {
     send_to_char("AFK mode removed due to MOVEMENT. Type 'replay' to see tells.\n\r",ch);
      REMOVE_BIT(ch->comm,COMM_AFK);
    }


  if ( !IS_NPC(ch) && mp_exit_trigger( ch, door ) )
    return;

  in_room = ch->in_room;
  if ( ( pexit   = in_room->exit[door] ) == NULL
  ||   ( to_room = pexit->u1.to_room   ) == NULL
  ||   !can_see_room(ch,pexit->u1.to_room))
    {
    send_to_char( "Alas, you cannot go that way.\n\r", ch );
    return;
    }

  if (IS_SET(pexit->exit_info, EX_CLOSED)
  &&  (!IS_AFFECTED(ch, AFF_PASS_DOOR) 
  || IS_SET(pexit->exit_info,EX_NOPASS))
  && (!IS_IMMORTAL(ch)))
    {
    act( "The $d is closed.", ch, NULL, pexit->keyword, TO_CHAR );
    return;
    }

  if ( IS_AFFECTED(ch, AFF_CHARM)
  &&   ch->master != NULL
  &&   in_room == ch->master->in_room )
    {
    send_to_char( "What?  And leave your beloved master?\n\r", ch );
    return;
    }

  if(!IS_IMP(ch))
    {
    if ( !is_room_owner(ch,to_room) && room_is_private( to_room ) )
      {
      send_to_char( "That room is private right now.\n\r", ch );
      return;
      }
    }

  if(!IS_IMP(ch))
    {
    if ( !IS_NPC(ch) )
      {
      int iClass;
      int move;
      for ( iClass = 0; iClass < MAX_CLASS; iClass++ )
        {
        if ( is_guild( iClass, to_room->vnum )
        &&   !is_guild( ch->class, to_room->vnum ) )
          {
          send_to_char( "You aren't allowed in there.\n\r", ch );
          return;
          }
        }
	
      if ( in_room->sector_type == SECT_AIR
      ||   to_room->sector_type == SECT_AIR )
        {
        if ( !IS_AFFECTED(ch, AFF_FLYING) && !IS_IMMORTAL(ch))
          {
          send_to_char( "You can't fly.\n\r", ch );
          return;
          }
        }
      if (( in_room->sector_type == SECT_WATER_NOSWIM
      ||    to_room->sector_type == SECT_WATER_NOSWIM )
      &&    !IS_AFFECTED(ch,AFF_FLYING))
        {
        OBJ_DATA *obj;
        bool found;
  	    /*
  	     * Look for a boat.
  	     */
        found = FALSE;
        if (IS_IMMORTAL(ch))
          found = TRUE;
        for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
          {
          if ( obj->item_type == ITEM_BOAT )
            {
            found = TRUE;
            break;
            }
          }
        if ( !found )
          {
          send_to_char( "You need a boat to go there.\n\r", ch );
          return;
          }
        }

    	move = movement_loss[UMIN(SECT_MAX-1, in_room->sector_type)]
    	     + movement_loss[UMIN(SECT_MAX-1, to_room->sector_type)];
      move /= 2; 

      if ( IS_AFFECTED( ch, AFF_FLYING )
      ||   IS_AFFECTED( ch, AFF_HASTE )) 
        move /= 2;

      if (IS_AFFECTED(ch,AFF_SLOW))
         move *= 2;
    
      if ( ch->move < move )
        {
        send_to_char( "You are too exhausted.\n\r", ch );
        return;
        }

      WAIT_STATE( ch, 1 );
      ch->move -= move;
    }
  }  

    // Stones of Wisdom Stuff
    if(!stones_try_moving(ch))
        return;
  
  if ( !IS_AFFECTED(ch, AFF_SNEAK)
  &&   ch->invis_level < LEVEL_HERO)
    act( "{c$n leaves $T.{x", ch, NULL, dir_name[door], TO_ROOM );

  char_from_room( ch );
  char_to_room( ch, to_room );
  if ( !IS_AFFECTED(ch, AFF_SNEAK)
  &&   ch->invis_level < LEVEL_HERO)
    act( "{c$n has arrived.{x", ch, NULL, NULL, TO_ROOM );

  do_look( ch, "auto" );

  if (in_room == to_room) /* no circular follows */
    return;

  for ( fch = in_room->people; fch != NULL; fch = fch_next )
    {
    fch_next = fch->next_in_room;
    if ( fch->master == ch && IS_AFFECTED(fch,AFF_CHARM) 
    &&   fch->position < POS_STANDING)
      do_stand(fch,"");
    if ( fch->master == ch && fch->position == POS_STANDING 
    &&   can_see_room(fch,to_room))
      {
	    if (  IS_SET(ch->in_room->room_flags,ROOM_LAW)
	    &&  ( IS_NPC(fch) && IS_SET(fch->act,ACT_AGGRESSIVE) ) )
        {
        act("You can't bring $N into the city.", ch,NULL,fch,TO_CHAR);
        act("You aren't allowed in the city.", fch,NULL,NULL,TO_CHAR);
        continue;
        }

      act( "You follow $N.", fch, NULL, ch, TO_CHAR );
      move_char( fch, door, TRUE );
      }
    }
    if ( IS_NPC( ch ) && HAS_TRIGGER( ch, TRIG_ENTRY ) )
        mp_percent_trigger( ch, NULL, NULL, NULL, TRIG_ENTRY );
    if ( !IS_NPC( ch ) )
        mp_greet_trigger( ch );

  return;
}




void do_north( CHAR_DATA *ch, char *argument )
{
    move_char( ch, DIR_NORTH, FALSE );
    return;
}

void do_east( CHAR_DATA *ch, char *argument )
{
    move_char( ch, DIR_EAST, FALSE );
    return;
}

void do_south( CHAR_DATA *ch, char *argument )
{
    move_char( ch, DIR_SOUTH, FALSE );
    return;
}

void do_west( CHAR_DATA *ch, char *argument )
{
    move_char( ch, DIR_WEST, FALSE );
    return;
}

void do_up( CHAR_DATA *ch, char *argument )
{
    move_char( ch, DIR_UP, FALSE );
    return;
}

void do_down( CHAR_DATA *ch, char *argument )
{
    move_char( ch, DIR_DOWN, FALSE );
    return;
}

void do_northeast( CHAR_DATA *ch, char *argument )
{
    move_char( ch, DIR_NORTHEAST, FALSE );
    return;
}

void do_southeast( CHAR_DATA *ch, char *argument )
{
    move_char( ch, DIR_SOUTHEAST, FALSE );
    return;
}

void do_southwest( CHAR_DATA *ch, char *argument )
{
    move_char( ch, DIR_SOUTHWEST, FALSE );
    return;
}

void do_northwest( CHAR_DATA *ch, char *argument )
{
    move_char( ch, DIR_NORTHWEST, FALSE );
    return;
}




int find_door( CHAR_DATA *ch, char *arg )
{
    EXIT_DATA *pexit;
    int door;

	 if ( !str_cmp( arg, "n" ) || !str_cmp( arg, "north" ) ) door = 0;
    else if ( !str_cmp( arg, "e" ) || !str_cmp( arg, "east"  ) ) door = 1;
    else if ( !str_cmp( arg, "s" ) || !str_cmp( arg, "south" ) ) door = 2;
    else if ( !str_cmp( arg, "w" ) || !str_cmp( arg, "west"  ) ) door = 3;
    else if ( !str_cmp( arg, "u" ) || !str_cmp( arg, "up"    ) ) door = 4;
    else if ( !str_cmp( arg, "d" ) || !str_cmp( arg, "down"  ) ) door = 5;
    else if ( !str_cmp( arg, "ne" ) || !str_cmp( arg, "northeast" ) ) door= 6;
    else if ( !str_cmp( arg, "se" ) || !str_cmp( arg, "southeast"  ) ) door = 7;
    else if ( !str_cmp( arg, "sw" ) || !str_cmp( arg, "southwest" ) ) door = 8;
    else if ( !str_cmp( arg, "nw" ) || !str_cmp( arg, "northwest"  ) ) door = 9;
    else
    {
	for ( door = 0; door <= 9; door++ )
	{
	    if ( ( pexit = ch->in_room->exit[door] ) != NULL
	    &&   IS_SET(pexit->exit_info, EX_ISDOOR)
	    &&   pexit->keyword != NULL
	    &&   is_name( arg, pexit->keyword ) )
		return door;
	}
	act( "I see no $T here.", ch, NULL, arg, TO_CHAR );
	return -1;
    }

    if ( ( pexit = ch->in_room->exit[door] ) == NULL )
    {
	act( "I see no door $T here.", ch, NULL, arg, TO_CHAR );
	return -1;
    }

    if ( !IS_SET(pexit->exit_info, EX_ISDOOR) )
    {
	send_to_char( "You can't do that.\n\r", ch );
	return -1;
    }

    return door;
}

void do_push( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
    EXIT_DATA *pexit;
    int door;
    char    buf[MAX_STRING_LENGTH];
    int doorFound=0;
    int chosenDoor=0;

  one_argument( argument, arg );

  if ( arg[0] == '\0' )
    {
    send_to_char( "Push whom?\n\r", ch );
    return;
    }

  if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
    send_to_char( "They aren't here.\n\r", ch );
    return;
    }

  if ( victim == ch )
    {
    send_to_char( "You push yourself around.\n\r", ch );
    return;
    }
    
    if(!IS_NPC(victim))
    {
        send_to_char("You can't push other players.\n\r", ch);
        return;
    }

    if (victim->pIndexData->pShop != NULL)
      {
	    send_to_char("The shopkeeper wouldn't like that.\n\r",ch);
	    return;
      }

    if (IS_SET(victim->act,ACT_TRAIN)
    ||  IS_SET(victim->act,ACT_PRACTICE)
    ||  IS_SET(victim->act,ACT_BANKER)
    ||  IS_SET(victim->act,ACT_FORGER)
    ||  IS_SET(victim->act,ACT_PRACTICE)
    ||  IS_SET(victim->act,ACT_QUESTMASTER)
    ||  IS_SET(victim->act,ACT_IS_HEALER)
    ||  IS_SET(victim->act,ACT_GAIN)
    ||  (IS_SET(victim->act,ACT_PET) && victim->master != ch))
      {
	  send_to_char("I don't think Mota would approve.\n\r",ch);
	  return;
       }

	for ( door = 0; door <= 9; door++ )
	{
	    if ( ( pexit = ch->in_room->exit[door] ) != NULL )
	    {
	       if(doorFound==0 || (doorFound==1 && number_range(1, 3)==2))
	       {
	           doorFound = 1;
	           chosenDoor = door;
	        }
    	}
	}
	
	if(doorFound==0)
	{
	   send_to_char("There are no doors to push that creature through!\n\r", ch);
	   return;
	}
	
	if(victim->level>ch->level && victim->level-ch->level>75)
	{
	   send_to_char("That mob is way too powerful for you to push!\n\r", ch);
	   return;
	}
	
	if(victim->level<=ch->level || (number_range(1, 100)<=(95-(victim->level-ch->level))))
	{
	   sprintf(buf, "You push %s out of the room.\n\r", victim->name);
	   send_to_char(buf, ch);
	   
       act( "$n pushed $N out of the room", ch, NULL, victim, TO_ROOM );
        move_char(victim, chosenDoor, TRUE);
    }
    else
    {
        
        if (!IS_AWAKE(victim))
            do_function(victim, &do_wake, "");
        
        sprintf(buf, "%s says 'Try and push me around will ya!\n\r'", victim->name);
        send_to_char(buf, ch);
        sprintf(buf, "%s says 'Try and push me around will ya!'", victim->name);
        act(buf, ch, NULL, victim, TO_ROOM);
        
        set_fighting(victim, ch);
    }
}

void do_open( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int door;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Open what?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
 	/* open portal */
	if (obj->item_type == ITEM_PORTAL)
	{
	    if (!IS_SET(obj->value[1], EX_ISDOOR))
	    {
		send_to_char("You can't do that.\n\r",ch);
		return;
	    }

	    if (!IS_SET(obj->value[1], EX_CLOSED))
	    {
		send_to_char("It's already open.\n\r",ch);
		return;
	    }

	    if (IS_SET(obj->value[1], EX_LOCKED))
	    {
		send_to_char("It's locked.\n\r",ch);
		return;
	    }

	    REMOVE_BIT(obj->value[1], EX_CLOSED);
	    act("You open $p.",ch,obj,NULL,TO_CHAR);
	    act("$n opens $p.",ch,obj,NULL,TO_ROOM);
	    return;
 	}

	/* 'open object' */
	if ( obj->item_type != ITEM_CONTAINER)
	    { send_to_char( "That's not a container.\n\r", ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_CLOSED) )
	    { send_to_char( "It's already open.\n\r",      ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_CLOSEABLE) )
	    { send_to_char( "You can't do that.\n\r",      ch ); return; }
	if ( IS_SET(obj->value[1], CONT_LOCKED) )
	    { send_to_char( "It's locked.\n\r",            ch ); return; }

	REMOVE_BIT(obj->value[1], CONT_CLOSED);
	act("You open $p.",ch,obj,NULL,TO_CHAR);
	act( "$n opens $p.", ch, obj, NULL, TO_ROOM );
	return;
    }

    if ( ( door = find_door( ch, arg ) ) >= 0 )
    {
	/* 'open door' */
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;

	pexit = ch->in_room->exit[door];
	if ( !IS_SET(pexit->exit_info, EX_CLOSED) )
	    { send_to_char( "It's already open.\n\r",      ch ); return; }
	if (  IS_SET(pexit->exit_info, EX_LOCKED) )
	    { send_to_char( "It's locked.\n\r",            ch ); return; }

	REMOVE_BIT(pexit->exit_info, EX_CLOSED);
	act( "$n opens the $d.", ch, NULL, pexit->keyword, TO_ROOM );
	send_to_char( "Ok.\n\r", ch );

	/* open the other side */
	if ( ( to_room   = pexit->u1.to_room            ) != NULL
	&&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != NULL
	&&   pexit_rev->u1.to_room == ch->in_room )
	{
	    CHAR_DATA *rch;

	    REMOVE_BIT( pexit_rev->exit_info, EX_CLOSED );
	    for ( rch = to_room->people; rch != NULL; rch = rch->next_in_room )
		act( "The $d opens.", rch, NULL, pexit_rev->keyword, TO_CHAR );
	}
    }

    return;
}



void do_close( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int door;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Close what?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
	/* portal stuff */
	if (obj->item_type == ITEM_PORTAL)
	{

	    if (!IS_SET(obj->value[1],EX_ISDOOR)
	    ||   IS_SET(obj->value[1],EX_NOCLOSE))
	    {
		send_to_char("You can't do that.\n\r",ch);
		return;
	    }

	    if (IS_SET(obj->value[1],EX_CLOSED))
	    {
		send_to_char("It's already closed.\n\r",ch);
		return;
	    }

	    SET_BIT(obj->value[1],EX_CLOSED);
	    act("You close $p.",ch,obj,NULL,TO_CHAR);
	    act("$n closes $p.",ch,obj,NULL,TO_ROOM);
	    return;
	}

	/* 'close object' */
	if ( obj->item_type != ITEM_CONTAINER )
	    { send_to_char( "That's not a container.\n\r", ch ); return; }
	if ( IS_SET(obj->value[1], CONT_CLOSED) )
	    { send_to_char( "It's already closed.\n\r",    ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_CLOSEABLE) )
	    { send_to_char( "You can't do that.\n\r",      ch ); return; }

	SET_BIT(obj->value[1], CONT_CLOSED);
	act("You close $p.",ch,obj,NULL,TO_CHAR);
	act( "$n closes $p.", ch, obj, NULL, TO_ROOM );
	return;
    }

    if ( ( door = find_door( ch, arg ) ) >= 0 )
    {
	/* 'close door' */
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;

	pexit	= ch->in_room->exit[door];
	if ( IS_SET(pexit->exit_info, EX_CLOSED) )
	    { send_to_char( "It's already closed.\n\r",    ch ); return; }

	SET_BIT(pexit->exit_info, EX_CLOSED);
	act( "$n closes the $d.", ch, NULL, pexit->keyword, TO_ROOM );
	send_to_char( "Ok.\n\r", ch );

	/* close the other side */
	if ( ( to_room   = pexit->u1.to_room            ) != NULL
	&&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != 0
	&&   pexit_rev->u1.to_room == ch->in_room )
	{
	    CHAR_DATA *rch;

	    SET_BIT( pexit_rev->exit_info, EX_CLOSED );
	    for ( rch = to_room->people; rch != NULL; rch = rch->next_in_room )
		act( "The $d closes.", rch, NULL, pexit_rev->keyword, TO_CHAR );
	}
    }

    return;
}



bool has_key( CHAR_DATA *ch, int key )
{
    OBJ_DATA *obj;

    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
	if ( obj->pIndexData->vnum == key )
	    return TRUE;
    }

    return FALSE;
}



void do_lock( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int door;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Lock what?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
	/* portal stuff */
	if (obj->item_type == ITEM_PORTAL)
	{
	    if (!IS_SET(obj->value[1],EX_ISDOOR)
	    ||  IS_SET(obj->value[1],EX_NOCLOSE))
	    {
		send_to_char("You can't do that.\n\r",ch);
		return;
	    }
	    if (!IS_SET(obj->value[1],EX_CLOSED))
	    {
		send_to_char("It's not closed.\n\r",ch);
	 	return;
	    }

	    if (obj->value[4] < 0 || IS_SET(obj->value[1],EX_NOLOCK))
	    {
		send_to_char("It can't be locked.\n\r",ch);
		return;
	    }

	    if (!has_key(ch,obj->value[4]))
	    {
		send_to_char("You lack the key.\n\r",ch);
		return;
	    }

	    if (IS_SET(obj->value[1],EX_LOCKED))
	    {
		send_to_char("It's already locked.\n\r",ch);
		return;
	    }

	    SET_BIT(obj->value[1],EX_LOCKED);
	    act("You lock $p.",ch,obj,NULL,TO_CHAR);
	    act("$n locks $p.",ch,obj,NULL,TO_ROOM);
	    return;
	}

	/* 'lock object' */
	if ( obj->item_type != ITEM_CONTAINER )
	    { send_to_char( "That's not a container.\n\r", ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_CLOSED) )
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( obj->value[2] < 0 )
	    { send_to_char( "It can't be locked.\n\r",     ch ); return; }
	if ( !has_key( ch, obj->value[2] ) )
	    { send_to_char( "You lack the key.\n\r",       ch ); return; }
	if ( IS_SET(obj->value[1], CONT_LOCKED) )
	    { send_to_char( "It's already locked.\n\r",    ch ); return; }

	SET_BIT(obj->value[1], CONT_LOCKED);
	act("You lock $p.",ch,obj,NULL,TO_CHAR);
	act( "$n locks $p.", ch, obj, NULL, TO_ROOM );
	return;
    }

    if ( ( door = find_door( ch, arg ) ) >= 0 )
    {
	/* 'lock door' */
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;

	pexit	= ch->in_room->exit[door];
	if ( !IS_SET(pexit->exit_info, EX_CLOSED) )
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( pexit->key < 0 )
	    { send_to_char( "It can't be locked.\n\r",     ch ); return; }
	if ( !has_key( ch, pexit->key) )
	    { send_to_char( "You lack the key.\n\r",       ch ); return; }
	if ( IS_SET(pexit->exit_info, EX_LOCKED) )
	    { send_to_char( "It's already locked.\n\r",    ch ); return; }

	SET_BIT(pexit->exit_info, EX_LOCKED);
	send_to_char( "*Click*\n\r", ch );
	act( "$n locks the $d.", ch, NULL, pexit->keyword, TO_ROOM );

	/* lock the other side */
	if ( ( to_room   = pexit->u1.to_room            ) != NULL
	&&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != 0
	&&   pexit_rev->u1.to_room == ch->in_room )
	{
	    SET_BIT( pexit_rev->exit_info, EX_LOCKED );
	}
    }

    return;
}



void do_unlock( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int door;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Unlock what?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
 	/* portal stuff */
	if (obj->item_type == ITEM_PORTAL)
	{
	    if (!IS_SET(obj->value[1],EX_ISDOOR))
	    {
		send_to_char("You can't do that.\n\r",ch);
		return;
	    }

	    if (!IS_SET(obj->value[1],EX_CLOSED))
	    {
		send_to_char("It's not closed.\n\r",ch);
		return;
	    }

	    if (obj->value[4] < 0)
	    {
		send_to_char("It can't be unlocked.\n\r",ch);
		return;
	    }

	    if (!has_key(ch,obj->value[4]))
	    {
		send_to_char("You lack the key.\n\r",ch);
		return;
	    }

	    if (!IS_SET(obj->value[1],EX_LOCKED))
	    {
		send_to_char("It's already unlocked.\n\r",ch);
		return;
	    }

	    REMOVE_BIT(obj->value[1],EX_LOCKED);
	    act("You unlock $p.",ch,obj,NULL,TO_CHAR);
	    act("$n unlocks $p.",ch,obj,NULL,TO_ROOM);
	    return;
	}

	/* 'unlock object' */
	if ( obj->item_type != ITEM_CONTAINER )
	    { send_to_char( "That's not a container.\n\r", ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_CLOSED) )
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( obj->value[2] < 0 )
	    { send_to_char( "It can't be unlocked.\n\r",   ch ); return; }
	if ( !has_key( ch, obj->value[2] ) )
	    { send_to_char( "You lack the key.\n\r",       ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_LOCKED) )
	    { send_to_char( "It's already unlocked.\n\r",  ch ); return; }

	REMOVE_BIT(obj->value[1], CONT_LOCKED);
	act("You unlock $p.",ch,obj,NULL,TO_CHAR);
	act( "$n unlocks $p.", ch, obj, NULL, TO_ROOM );
	return;
    }

    if ( ( door = find_door( ch, arg ) ) >= 0 )
    {
	/* 'unlock door' */
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;

	pexit = ch->in_room->exit[door];
	if ( !IS_SET(pexit->exit_info, EX_CLOSED) )
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( pexit->key < 0 )
	    { send_to_char( "It can't be unlocked.\n\r",   ch ); return; }
	if ( !has_key( ch, pexit->key) )
	    { send_to_char( "You lack the key.\n\r",       ch ); return; }
	if ( !IS_SET(pexit->exit_info, EX_LOCKED) )
	    { send_to_char( "It's already unlocked.\n\r",  ch ); return; }

	REMOVE_BIT(pexit->exit_info, EX_LOCKED);
	send_to_char( "*Click*\n\r", ch );
	act( "$n unlocks the $d.", ch, NULL, pexit->keyword, TO_ROOM );

	/* unlock the other side */
	if ( ( to_room   = pexit->u1.to_room            ) != NULL
	&&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != NULL
	&&   pexit_rev->u1.to_room == ch->in_room )
	{
	    REMOVE_BIT( pexit_rev->exit_info, EX_LOCKED );
	}
    }

    return;
}

void do_stand( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj = NULL;

    if (argument[0] != '\0')
    {
	if (ch->position == POS_FIGHTING)
	{
	    send_to_char("Maybe you should finish fighting first?\n\r",ch);
	    return;
	}
	obj = get_obj_list(ch,argument,ch->in_room->contents);
	if (obj == NULL)
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}
	if (obj->item_type != ITEM_FURNITURE
	||  (!IS_SET(obj->value[2],STAND_AT)
	&&   !IS_SET(obj->value[2],STAND_ON)
	&&   !IS_SET(obj->value[2],STAND_IN)))
	{
	    send_to_char("You can't seem to find a place to stand.\n\r",ch);
	    return;
	}
	if (ch->on != obj && count_users(obj) >= obj->value[0])
	{
	    act_new("There's no room to stand on $p.",
		ch,obj,NULL,TO_CHAR,POS_DEAD);
	    return;
	}
 	ch->on = obj;
    }
    
if (!IS_NPC(ch))
   {
  if (IS_SET(ch->comm, COMM_AFK))
    {
     send_to_char("AFK mode removed due to PLAYER INPUT. Type 'replay' to see tells.\n\r",ch);
      REMOVE_BIT(ch->comm,COMM_AFK);
    }
   }

    switch ( ch->position )
    {
    case POS_SLEEPING:
	if ( IS_AFFECTED(ch, AFF_SLEEP) )
	    { send_to_char( "You can't wake up!\n\r", ch ); return; }
	
	if (obj == NULL)
	{
	    send_to_char( "You wake and stand up.\n\r", ch );
	    act( "$n wakes and stands up.", ch, NULL, NULL, TO_ROOM );
	    ch->on = NULL;
	}
	else if (IS_SET(obj->value[2],STAND_AT))
	{
	   act_new("You wake and stand at $p.",ch,obj,NULL,TO_CHAR,POS_DEAD);
	   act("$n wakes and stands at $p.",ch,obj,NULL,TO_ROOM);
	}
	else if (IS_SET(obj->value[2],STAND_ON))
	{
	    act_new("You wake and stand on $p.",ch,obj,NULL,TO_CHAR,POS_DEAD);
	    act("$n wakes and stands on $p.",ch,obj,NULL,TO_ROOM);
	}
	else 
	{
	    act_new("You wake and stand in $p.",ch,obj,NULL,TO_CHAR,POS_DEAD);
	    act("$n wakes and stands in $p.",ch,obj,NULL,TO_ROOM);
	}
	ch->position = POS_STANDING;
	do_function(ch, &do_look, "auto");
	break;

    case POS_RESTING: case POS_SITTING:
	if (obj == NULL)
	{
	    send_to_char( "You stand up.\n\r", ch );
	    act( "$n stands up.", ch, NULL, NULL, TO_ROOM );
	    ch->on = NULL;
	}
	else if (IS_SET(obj->value[2],STAND_AT))
	{
	    act("You stand at $p.",ch,obj,NULL,TO_CHAR);
	    act("$n stands at $p.",ch,obj,NULL,TO_ROOM);
	}
	else if (IS_SET(obj->value[2],STAND_ON))
	{
	    act("You stand on $p.",ch,obj,NULL,TO_CHAR);
	    act("$n stands on $p.",ch,obj,NULL,TO_ROOM);
	}
	else
	{
	    act("You stand in $p.",ch,obj,NULL,TO_CHAR);
	    act("$n stands on $p.",ch,obj,NULL,TO_ROOM);
	}
	ch->position = POS_STANDING;
	break;

    case POS_STANDING:
	send_to_char( "You are already standing.\n\r", ch );
	break;

    case POS_FIGHTING:
	send_to_char( "You are already fighting!\n\r", ch );
	break;
    }

    return;
}



void do_rest( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj = NULL;

    if (ch->position == POS_FIGHTING)
    {
	send_to_char("You are already fighting!\n\r",ch);
	return;
    }

    /* okay, now that we know we can rest, find an object to rest on */
    if (argument[0] != '\0')
    {
	obj = get_obj_list(ch,argument,ch->in_room->contents);
	if (obj == NULL)
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}
    }
    else obj = ch->on;

    if (obj != NULL)
    {
        if (obj->item_type != ITEM_FURNITURE
    	||  (!IS_SET(obj->value[2],REST_ON)
    	&&   !IS_SET(obj->value[2],REST_IN)
    	&&   !IS_SET(obj->value[2],REST_AT)))
    	{
	    send_to_char("You can't rest on that.\n\r",ch);
	    return;
    	}

        if (obj != NULL && ch->on != obj && count_users(obj) >= obj->value[0])
        {
	    act_new("There's no more room on $p.",ch,obj,NULL,TO_CHAR,POS_DEAD);
	    return;
    	}
	
	ch->on = obj;
    }

    switch ( ch->position )
    {
    case POS_SLEEPING:
	if (IS_AFFECTED(ch,AFF_SLEEP))
	{
	    send_to_char("You can't wake up!\n\r",ch);
	    return;
	}

	if (obj == NULL)
	{
	    send_to_char( "You wake up and start resting.\n\r", ch );
	    act ("$n wakes up and starts resting.",ch,NULL,NULL,TO_ROOM);
	}
	else if (IS_SET(obj->value[2],REST_AT))
	{
	    act_new("You wake up and rest at $p.",
		    ch,obj,NULL,TO_CHAR,POS_SLEEPING);
	    act("$n wakes up and rests at $p.",ch,obj,NULL,TO_ROOM);
	}
        else if (IS_SET(obj->value[2],REST_ON))
        {
            act_new("You wake up and rest on $p.",
                    ch,obj,NULL,TO_CHAR,POS_SLEEPING);
            act("$n wakes up and rests on $p.",ch,obj,NULL,TO_ROOM);
        }
        else
        {
            act_new("You wake up and rest in $p.",
                    ch,obj,NULL,TO_CHAR,POS_SLEEPING);
            act("$n wakes up and rests in $p.",ch,obj,NULL,TO_ROOM);
        }
	ch->position = POS_RESTING;
	break;

    case POS_RESTING:
	send_to_char( "You are already resting.\n\r", ch );
	break;

    case POS_STANDING:
	if (obj == NULL)
	{
	    send_to_char( "You rest.\n\r", ch );
	    act( "$n sits down and rests.", ch, NULL, NULL, TO_ROOM );
	}
        else if (IS_SET(obj->value[2],REST_AT))
        {
	    act("You sit down at $p and rest.",ch,obj,NULL,TO_CHAR);
	    act("$n sits down at $p and rests.",ch,obj,NULL,TO_ROOM);
        }
        else if (IS_SET(obj->value[2],REST_ON))
        {
	    act("You sit on $p and rest.",ch,obj,NULL,TO_CHAR);
	    act("$n sits on $p and rests.",ch,obj,NULL,TO_ROOM);
        }
        else
        {
	    act("You rest in $p.",ch,obj,NULL,TO_CHAR);
	    act("$n rests in $p.",ch,obj,NULL,TO_ROOM);
        }
	ch->position = POS_RESTING;
	break;

    case POS_SITTING:
	if (obj == NULL)
	{
	    send_to_char("You rest.\n\r",ch);
	    act("$n rests.",ch,NULL,NULL,TO_ROOM);
	}
        else if (IS_SET(obj->value[2],REST_AT))
        {
	    act("You rest at $p.",ch,obj,NULL,TO_CHAR);
	    act("$n rests at $p.",ch,obj,NULL,TO_ROOM);
        }
        else if (IS_SET(obj->value[2],REST_ON))
        {
	    act("You rest on $p.",ch,obj,NULL,TO_CHAR);
	    act("$n rests on $p.",ch,obj,NULL,TO_ROOM);
        }
        else
        {
	    act("You rest in $p.",ch,obj,NULL,TO_CHAR);
	    act("$n rests in $p.",ch,obj,NULL,TO_ROOM);
	}
	ch->position = POS_RESTING;
	break;
    }

if (!IS_NPC(ch))
   {
  if (IS_SET(ch->comm, COMM_AFK))
    {
     send_to_char("AFK mode removed due to PLAYER INPUT. Type 'replay' to see tells.\n\r",ch);
      REMOVE_BIT(ch->comm,COMM_AFK);
    }
   }

    return;
}


void do_sit (CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj = NULL;

    if (ch->position == POS_FIGHTING)
    {
	send_to_char("Maybe you should finish this fight first?\n\r",ch);
	return;
    }

    /* okay, now that we know we can sit, find an object to sit on */
    if (argument[0] != '\0')
    {
	obj = get_obj_list(ch,argument,ch->in_room->contents);
	if (obj == NULL)
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}
    }
    else obj = ch->on;

    if (obj != NULL)                                                              
    {
	if (obj->item_type != ITEM_FURNITURE
	||  (!IS_SET(obj->value[2],SIT_ON)
	&&   !IS_SET(obj->value[2],SIT_IN)
	&&   !IS_SET(obj->value[2],SIT_AT)))
	{
	    send_to_char("You can't sit on that.\n\r",ch);
	    return;
	}

	if (obj != NULL && ch->on != obj && count_users(obj) >= obj->value[0])
	{
	    act_new("There's no more room on $p.",ch,obj,NULL,TO_CHAR,POS_DEAD);
	    return;
	}

	ch->on = obj;
    }
    switch (ch->position)
    {
	case POS_SLEEPING:
	    if (IS_AFFECTED(ch,AFF_SLEEP))
	    {
		send_to_char("You can't wake up!\n\r",ch);
		return;
	    }

            if (obj == NULL)
            {
            	send_to_char( "You wake and sit up.\n\r", ch );
            	act( "$n wakes and sits up.", ch, NULL, NULL, TO_ROOM );
            }
            else if (IS_SET(obj->value[2],SIT_AT))
            {
            	act_new("You wake and sit at $p.",ch,obj,NULL,TO_CHAR,POS_DEAD);
            	act("$n wakes and sits at $p.",ch,obj,NULL,TO_ROOM);
            }
            else if (IS_SET(obj->value[2],SIT_ON))
            {
            	act_new("You wake and sit on $p.",ch,obj,NULL,TO_CHAR,POS_DEAD);
            	act("$n wakes and sits at $p.",ch,obj,NULL,TO_ROOM);
            }
            else
            {
            	act_new("You wake and sit in $p.",ch,obj,NULL,TO_CHAR,POS_DEAD);
            	act("$n wakes and sits in $p.",ch,obj,NULL,TO_ROOM);
            }

	    ch->position = POS_SITTING;
	    break;
	case POS_RESTING:
	    if (obj == NULL)
		send_to_char("You stop resting.\n\r",ch);
	    else if (IS_SET(obj->value[2],SIT_AT))
	    {
		act("You sit at $p.",ch,obj,NULL,TO_CHAR);
		act("$n sits at $p.",ch,obj,NULL,TO_ROOM);
	    }

	    else if (IS_SET(obj->value[2],SIT_ON))
	    {
		act("You sit on $p.",ch,obj,NULL,TO_CHAR);
		act("$n sits on $p.",ch,obj,NULL,TO_ROOM);
	    }
	    ch->position = POS_SITTING;
	    break;
	case POS_SITTING:
	    send_to_char("You are already sitting down.\n\r",ch);
	    break;
	case POS_STANDING:
	    if (obj == NULL)
    	    {
		send_to_char("You sit down.\n\r",ch);
    	        act("$n sits down on the ground.",ch,NULL,NULL,TO_ROOM);
	    }
	    else if (IS_SET(obj->value[2],SIT_AT))
	    {
		act("You sit down at $p.",ch,obj,NULL,TO_CHAR);
		act("$n sits down at $p.",ch,obj,NULL,TO_ROOM);
	    }
	    else if (IS_SET(obj->value[2],SIT_ON))
	    {
		act("You sit on $p.",ch,obj,NULL,TO_CHAR);
		act("$n sits on $p.",ch,obj,NULL,TO_ROOM);
	    }
	    else
	    {
		act("You sit down in $p.",ch,obj,NULL,TO_CHAR);
		act("$n sits down in $p.",ch,obj,NULL,TO_ROOM);
	    }
    	    ch->position = POS_SITTING;
    	    break;
    }

if (!IS_NPC(ch))
   {
  if (IS_SET(ch->comm, COMM_AFK))
    {
     send_to_char("AFK mode removed due to PLAYER INPUT. Type 'replay' to see tells.\n\r",ch);
      REMOVE_BIT(ch->comm,COMM_AFK);
    }
   }

    return;
}


void do_sleep( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj = NULL;

    switch ( ch->position )
    {
    case POS_SLEEPING:
	send_to_char( "You are already sleeping.\n\r", ch );
	break;

    case POS_RESTING:
    case POS_SITTING:
    case POS_STANDING: 
	if (argument[0] == '\0' && ch->on == NULL)
	{
	    send_to_char( "You go to sleep.\n\r", ch );
	    act( "$n goes to sleep.", ch, NULL, NULL, TO_ROOM );
	    ch->position = POS_SLEEPING;
	}
	else  /* find an object and sleep on it */
	{
	    if (argument[0] == '\0')
		obj = ch->on;
	    else
	    	obj = get_obj_list( ch, argument,  ch->in_room->contents );

	    if (obj == NULL)
	    {
		send_to_char("You don't see that here.\n\r",ch);
		return;
	    }
	    if (obj->item_type != ITEM_FURNITURE
	    ||  (!IS_SET(obj->value[2],SLEEP_ON) 
	    &&   !IS_SET(obj->value[2],SLEEP_IN)
	    &&	 !IS_SET(obj->value[2],SLEEP_AT)))
	    {
		send_to_char("You can't sleep on that!\n\r",ch);
		return;
	    }

	    if (ch->on != obj && count_users(obj) >= obj->value[0])
	    {
		act_new("There is no room on $p for you.",
		    ch,obj,NULL,TO_CHAR,POS_DEAD);
		return;
	    }

	    ch->on = obj;
	    if (IS_SET(obj->value[2],SLEEP_AT))
	    {
		act("You go to sleep at $p.",ch,obj,NULL,TO_CHAR);
		act("$n goes to sleep at $p.",ch,obj,NULL,TO_ROOM);
	    }
	    else if (IS_SET(obj->value[2],SLEEP_ON))
	    {
	        act("You go to sleep on $p.",ch,obj,NULL,TO_CHAR);
	        act("$n goes to sleep on $p.",ch,obj,NULL,TO_ROOM);
	    }
	    else
	    {
		act("You go to sleep in $p.",ch,obj,NULL,TO_CHAR);
		act("$n goes to sleep in $p.",ch,obj,NULL,TO_ROOM);
	    }
	    ch->position = POS_SLEEPING;
	}
	break;

    case POS_FIGHTING:
	send_to_char( "You are already fighting!\n\r", ch );
	break;
    }

if (!IS_NPC(ch))
   {
  if (IS_SET(ch->comm, COMM_AFK))
    {
     send_to_char("AFK mode removed due to PLAYER INPUT. Type 'replay' to see tells.\n\r",ch);
      REMOVE_BIT(ch->comm,COMM_AFK);
    }
   }

    return;
}



void do_wake( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
	{ do_function(ch, &do_stand, ""); return; }

    if ( !IS_AWAKE(ch) )
	{ send_to_char( "You are asleep yourself!\n\r",       ch ); return; }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
	{ send_to_char( "They aren't here.\n\r",              ch ); return; }

    if ( IS_AWAKE(victim) )
	{ act( "$N is already awake.", ch, NULL, victim, TO_CHAR ); return; }

    if ( IS_AFFECTED(victim, AFF_SLEEP) )
	{ act( "You can't wake $M!",   ch, NULL, victim, TO_CHAR );  return; }

    act_new( "$n wakes you.", ch, NULL, victim, TO_VICT,POS_SLEEPING );
    do_function(victim, &do_sit, "");
    return;
}


/*
 * Contributed by Alander.
 */

void do_recall( CHAR_DATA *ch, char *argument )
{
  ROOM_INDEX_DATA *location;
  
  location=NULL;

  location = get_room_index(ROOM_VNUM_DRAGONPIT);
  
  if (ch->in_room == location)
    {
send_to_char( "\n\r{GWait until the DragonPIT begins so it will be fair...{x\n\r",ch);
    return;
    }

 if (!IS_IMP(ch))
    {
    if (IS_IMMORTAL(ch))
      {
       send_to_char("\n\r{rIMMs can use {RGOTO{r so there is no reason to be using {RRECALL{r.{x\n\r",ch);
       return;
      }
    }

  if ( ch->pk_timer > 0 )
    send_to_char( "\n\r{WYou must wait {R5 ticks{W before you can recall after a player fight.{x\n\r", ch );
  else if ( ch->fighting )
    send_to_char( "\n\r{WYou must use {RWORD OF RECALL{W during battle.{x\n\r", ch );    
  else
   recall( ch );

  return;
}

void recall( CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    ROOM_INDEX_DATA *location;

   location=NULL;
  

    if ((IS_SET (ch->pact, PLR_PKILLER) || (ch->clan))
    && (ch->level > 20))
      {
 send_to_char("\n\r{WThis option is not available beyond 20th level for {GPKILLER{Ws.{x\n\r",ch);
 send_to_char("{R*** {WIf you are in a {cCLAN{w try using {c'{RCRECALL{c'{W instead.{x\n\r",ch);
	  return;
      }

    if (IS_NPC(ch) && !IS_SET(ch->act,ACT_PET))
    {
	send_to_char("Only players can recall.\n\r",ch);
	return;
    }
  
    act( "$n prays for transportation!", ch, 0, 0, TO_ROOM );

    if (IS_NPC(ch) && IS_SET(ch->act,ACT_PET))
      location=get_room_index(ROOM_VNUM_TEMPLE);
    else if (ch->pcdata->house == 0)
      location=get_room_index(ROOM_VNUM_TEMPLE);
    else
      location=get_room_index(ch->pcdata->house);

    if (  location == NULL )
    {
	send_to_char( "You are completely lost.\n\r", ch );
	return;
    }

    if ( ch->in_room == location )
	return;

    if ( IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
    ||   IS_AFFECTED(ch, AFF_CURSE))
    {
	send_to_char( "Mota has forsaken you.\n\r", ch );
	return;
    }

    if ( ( victim = ch->fighting ) != NULL )
    {
	int lose,skill;

	skill = get_skill(ch,gsn_recall);

	if ( number_percent() < 80 * skill / 100 )
	{
	    check_improve(ch,gsn_recall,FALSE,6);
	    WAIT_STATE( ch, 4 );
	    sprintf( buf, "You failed!.\n\r");
	    send_to_char( buf, ch );
	    return;
	}

	lose = (ch->desc != NULL) ? 25 : 50;
	gain_exp( ch, 0 - lose );
	check_improve(ch,gsn_recall,TRUE,4);
	sprintf( buf, "You recall from combat!  You lose %d exps.\n\r", lose );
	send_to_char( buf, ch );
	stop_fighting( ch, TRUE );
	
    }

    // Stones of Wisdom Stuff
    if(!stones_try_moving(ch))
        return;

    ch->move /= 2;
    act( "$n disappears.", ch, NULL, NULL, TO_ROOM );
    char_from_room( ch );
    char_to_room( ch, location );
    act( "$n appears in the room.", ch, NULL, NULL, TO_ROOM );
    do_function(ch, &do_look, "auto" );
    
    if (ch->pet != NULL)
	do_function(ch->pet, &do_recall, "");

    return;
}


void do_train( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    CHAR_DATA *mob;
    sh_int stat = - 1;
    char *pOutput = NULL;
    int cost=0;
    int i=0;
    int numRaceChansSoFar=0;

    if ( IS_NPC(ch) )
	return;

    /*
     * Check for trainer.
     */
    for ( mob = ch->in_room->people; mob; mob = mob->next_in_room )
    {
	if ( IS_NPC(mob) && IS_SET(mob->act, ACT_TRAIN) )
	    break;
    }

    if ( mob == NULL )
    {
	send_to_char( "You can't do that here.\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' )
    {
	sprintf( buf, "You have %d training sessions.\n\r", ch->train );
	send_to_char( buf, ch );
	argument = "foo";
    }

    cost = 1;

    if ( !str_cmp( argument, "str" ) )
    {
	if ( class_table[ch->class].attr_prime == STAT_STR )
	    cost    = 1;
	stat        = STAT_STR;
	pOutput     = "strength";
    }

    else if ( !str_cmp( argument, "int" ) )
    {
	if ( class_table[ch->class].attr_prime == STAT_INT )
	    cost    = 1;
	stat	    = STAT_INT;
	pOutput     = "intelligence";
    }

    else if ( !str_cmp( argument, "wis" ) )
    {
	if ( class_table[ch->class].attr_prime == STAT_WIS )
	    cost    = 1;
	stat	    = STAT_WIS;
	pOutput     = "wisdom";
    }

    else if ( !str_cmp( argument, "dex" ) )
    {
	if ( class_table[ch->class].attr_prime == STAT_DEX )
	    cost    = 1;
	stat  	    = STAT_DEX;
	pOutput     = "dexterity";
    }

    else if ( !str_cmp( argument, "con" ) )
    {
	if ( class_table[ch->class].attr_prime == STAT_CON )
	    cost    = 1;
	stat	    = STAT_CON;
	pOutput     = "constitution";
    }
    else if ( !str_cmp(argument, "hp" ) )
    {
	cost = 1;
    }
    else if ( !str_cmp(argument, "mana" ) )
    {
	cost = 1;
    }
    else
    {
        cost = 0;
        strcpy(buf2, "{GYou can also train:{Y");
        
        for(i=0,numRaceChansSoFar=0;racechan_flags[i].name;i++)
        {
            if(IS_SET(ch->racechan, racechan_flags[i].bit))
                numRaceChansSoFar++;
            else
            {
                strcat(buf2, " ");
                strcat(buf2, racechan_flags[i].name);
            }
        }
        
        strcat(buf2, "{x\n\r");
        
        for(i=0;racechan_flags[i].name;i++)
        {
            if(!str_cmp(argument, racechan_flags[i].name))
            {
                if(IS_LEGEND(ch))
                {
                    if(IS_HARDCORE(ch))
                        cost = 2;
                    else
                        cost = 3;
                }
                else
                {
                    if(numRaceChansSoFar==0)
                        cost = 3;
                    else if(numRaceChansSoFar==1)
                        cost = 6;
                    else
                    {
                        send_to_char("\n\r{RYou have already trained in two racial languages.{x\n\r{RYou may not train in any more.{x\n\r", ch);
                        return;
                    }
                }
                break;
            }
        }
        
        if(cost==0)
        {
        	strcpy( buf, "{GYou can train:{Y" );
        	if ( ch->perm_stat[STAT_STR] < get_max_train(ch,STAT_STR)) 
        	    strcat( buf, " str" );
        	if ( ch->perm_stat[STAT_INT] < get_max_train(ch,STAT_INT))  
        	    strcat( buf, " int" );
        	if ( ch->perm_stat[STAT_WIS] < get_max_train(ch,STAT_WIS)) 
        	    strcat( buf, " wis" );
        	if ( ch->perm_stat[STAT_DEX] < get_max_train(ch,STAT_DEX))  
        	    strcat( buf, " dex" );
        	if ( ch->perm_stat[STAT_CON] < get_max_train(ch,STAT_CON))  
        	    strcat( buf, " con" );
        	strcat( buf, " hp mana");
        
     	    strcat( buf, "{x\n\r" );
    	    send_to_char( buf, ch );
    	    
    	    if(numRaceChansSoFar<2 || IS_LEGEND(ch))
    	       send_to_char(buf2, ch);
    
        	return;
        }
    }

    if (!str_cmp("hp",argument))
    {
    	if ( cost > ch->train )
    	{
    send_to_char( "\n\r{RYou don't have enough training sessions.{x\n\r", ch);
            return;
        }
 
	ch->train -= cost;
        ch->pcdata->perm_hit += 10;
        ch->max_hit += 10;
        ch->hit +=10;
        act( "\n\r{WYour durability increases!{x\n\r",ch,NULL,NULL,TO_CHAR);
/*        act( "$n's durability increases!",ch,NULL,NULL,TO_ROOM); */
        return;
    }
 
    if (!str_cmp("mana",argument))
    {
        if ( cost > ch->train )
        {
    send_to_char( "\n\r{RYou don't have enough training sessions.{x\n\r", ch);
            return;
        }

	ch->train -= cost;
        ch->pcdata->perm_mana += 10;
        ch->max_mana += 12;
        ch->mana += 12;
        act( "\n\r{CYour power increases!{x\n\r",ch,NULL,NULL,TO_CHAR);
/*        act( "$n's power increases!",ch,NULL,NULL,TO_ROOM); */
        return;
    }
    
   for(i=0;racechan_flags[i].name;i++)
    {
        if(!str_cmp(argument, racechan_flags[i].name))
        {   
            if ( cost > ch->train )
            {
                send_to_char( "\n\r{RYou don't have enough training sessions.{x\n\r", ch);
                return;
            }
            
            ch->train -= cost;
            SET_BIT(ch->racechan, racechan_flags[i].bit);
            sprintf(buf, "\n\r{GYou now know how to speak {Y%s{x\n\r", racechan_flags[i].name);
            send_to_char(buf, ch);
            
            return;
        }
    }
    

    if ( ch->perm_stat[stat]  >= get_max_train(ch,stat) )
    {
	act( "\n\r{cYour {W$T {cis already at maximum.{x\n\r", ch, NULL, pOutput,TO_CHAR );
	return;
    }

    if ( cost > ch->train )
    {
    send_to_char( "\n\r{RYou don't have enough training sessions.{x\n\r", ch);
	return;
    }

    ch->train		-= cost;
  
    ch->perm_stat[stat]		+= 1;
    act( "\n\r{cYour {W$T {cincreases{x\n\r!", ch, NULL, pOutput, TO_CHAR );
/*    act( "$n's $T increases!", ch, NULL, pOutput, TO_ROOM ); */
    return;
}
