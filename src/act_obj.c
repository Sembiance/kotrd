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
#include "magic.h"
#include "bet.h"

#include "Utility.h"
#include "StringUtility.h"
#include "ArrayUtility.h"

void talk_auction (char *argument, CHAR_DATA * ch);
void spell_imprint (int sn, int level, CHAR_DATA *ch, void *vo);
/*
 * Local functions.
 */
#define CD CHAR_DATA
#define OD OBJ_DATA
bool	remove_obj	args( (CHAR_DATA *ch, int iWear, bool fReplace ) );
void	wear_obj	args( (CHAR_DATA *ch, OBJ_DATA *obj, bool fReplace ) );
CD *	find_keeper	args( (CHAR_DATA *ch ) );
int	get_cost	args( (CHAR_DATA *keeper, OBJ_DATA *obj, bool fBuy ) );
void 	obj_to_keeper	args( (OBJ_DATA *obj, CHAR_DATA *ch ) );
OD *	get_obj_keeper	args( (CHAR_DATA *ch,CHAR_DATA *keeper,char *argument));
AUCTION_DATA  *auction;
bool IsQuestObjVnum(OBJ_DATA * obj);

DECLARE_DO_FUN(do_visible);

#undef OD
#undef	CD

/* RT part of the corpse looting code */

bool can_loot(CHAR_DATA *ch, OBJ_DATA *obj)
{
    CHAR_DATA *owner, *wch;

    if (IS_IMMORTAL(ch))
	return TRUE;


    if (!obj->owner || obj->owner == NULL)
	return TRUE;

    owner = NULL;
    for ( wch = char_list; wch != NULL ; wch = wch->next )
        if (!str_cmp(wch->name,obj->owner))
            owner = wch;


    if (owner == NULL)
	return TRUE;

    if (!str_cmp(ch->name,owner->name))
	return TRUE;

    if (!IS_NPC(owner) && IS_SET(owner->pact,PLR_CANLOOT))
	return TRUE;

    if (is_same_group(ch,owner))
	return TRUE;

    return FALSE;
}


void get_obj( CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *container )
{
    /* variables for AUTOSPLIT */
    EXTRA_DESCR_DATA *  ed;
    CHAR_DATA *gch;
    char buf[MAX_STRING_LENGTH];
    int members;
    char buffer[100];

    if ( !CAN_WEAR(obj, ITEM_TAKE) )
    {
	send_to_char( "You can't take that.\n\r", ch );
	return;
    }
    
    // See if it's joined with anything
    if(GetObjExtraDesc(obj, "=joinedwith="))
    {
        send_to_char("That object is currently joined and you can't get it.\n\r", ch);
        return;
    }
    

    if ( ch->carry_number + get_obj_number( obj ) > can_carry_n( ch ) )
    {
	act( "$d: you can't carry that many items.",
	    ch, NULL, obj->name, TO_CHAR );
	return;
    }

    if ((!obj->in_obj || obj->in_obj->carried_by != ch)
    &&  (get_carry_weight(ch) + get_obj_weight(obj) > can_carry_w(ch)))
    {
	act( "$d: you can't carry that much weight.",
	    ch, NULL, obj->name, TO_CHAR );
	return;
    }

    if (!can_loot(ch,obj))
    {
	act("Corpse looting is not permitted.",ch,NULL,NULL,TO_CHAR );
	return;
    }

    if (obj->in_room != NULL)
    {
	for (gch = obj->in_room->people; gch != NULL; gch = gch->next_in_room)
	    if (gch->on == obj)
	    {
		act("$N appears to be using $p.",
		    ch,obj,gch,TO_CHAR);
		return;
	    }
    }

        if(same_obj(obj, ch->carrying) >=30)
        {
            send_to_char("You already have enough.\n\r", ch);
            return;
        }
		

    if ( container != NULL )
    {
      {
         sprintf(buf,"corpse %s", ch->name);

         if ( container->item_type == ITEM_CORPSE_PC
         &&  IS_SET(ch->in_room->room_flags, ROOM_NOLOOT)
         &&  !str_cmp(container->name, buf) )
          {  }
         else
          if IS_SET(ch->in_room->room_flags, ROOM_NOLOOT)
            {
             send_to_char( "{RYou can't loot in this room.{x\n\r", ch );
             return;
            }
       }

    	if (container->pIndexData->vnum == OBJ_VNUM_PIT
	&&  ch->level < obj->level)
	{
	    send_to_char("You are not powerful enough to use it.\n\r",ch);
	    return;
	}

    	if (container->pIndexData->vnum == OBJ_VNUM_PIT
	&&  !CAN_WEAR(container, ITEM_TAKE)
	&&  !IS_OBJ_STAT(obj,ITEM_HAD_TIMER))
	    obj->timer = 0;	
	act( "You get $p from $P.", ch, obj, container, TO_CHAR );
	act( "$n reaches into $P and gets $p.", ch, obj, container, TO_ROOM );
    if(obj->extra_descr != NULL || obj->pIndexData->extra_descr != NULL)
    {
        for (ed = obj->extra_descr; ed != NULL; ed = ed->next )
        {
            if(!str_cmp("pickupmessage", ed->keyword))
                send_to_char(ed->description, ch);
        }

        for ( ed = obj->pIndexData->extra_descr; ed != NULL; ed = ed->next )
        {
            if(!str_cmp("pickupmessage", ed->keyword))
                send_to_char(ed->description, ch);
        }
    }

	REMOVE_BIT(obj->extra_flags,ITEM_HAD_TIMER);
	obj_from_obj( obj );
    }
    else
    {
	act( "You get $p.", ch, obj, container, TO_CHAR );
	act( "$n gets $p.", ch, obj, container, TO_ROOM );
    if(obj->extra_descr != NULL || obj->pIndexData->extra_descr != NULL)
    {
        for (ed = obj->extra_descr; ed != NULL; ed = ed->next )
        {
            if(!str_cmp("pickupmessage", ed->keyword))
                send_to_char(ed->description, ch);
        }

        for ( ed = obj->pIndexData->extra_descr; ed != NULL; ed = ed->next )
        {
            if(!str_cmp("pickupmessage", ed->keyword))
                send_to_char(ed->description, ch);
        }
    }

	obj_from_room( obj );
    }

    if ( obj->item_type == ITEM_MONEY)
    {
	ch->silver += obj->value[0];
	ch->gold += obj->value[1];
        if (IS_SET(ch->pact,PLR_AUTOSPLIT))
        { /* AUTOSPLIT code */
    	  members = 0;
    	  for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    	  {
            if (!IS_AFFECTED(gch,AFF_CHARM) && is_same_group( gch, ch ) )
              members++;
    	  }

	  if ( members > 1 && (obj->value[0] > 1 || obj->value[1]))
	  {
	    sprintf(buffer,"%d %d",obj->value[0],obj->value[1]);
	    do_function(ch, &do_split, buffer);	
	  }
        }
 
	extract_obj( obj );
    }
    else
    {
	obj_to_char( obj, ch );
    }

    return;
}

void do_identify( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    CHAR_DATA *mob;

    for ( mob = ch->in_room->people; mob; mob = mob->next_in_room )
    {
        if ( IS_NPC(mob) && IS_SET(mob->act, ACT_IS_HEALER) )
            break;
    }
 
    if ( mob == NULL )
    {
        send_to_char( "You can't do that here.\n\r", ch );
        return;
    }

    if ( ( obj = get_obj_carry( ch, argument, ch ) ) == NULL )
    {
       send_to_char( "You are not carrying that.\n\r", ch );
       return;
    }

    if (IS_IMMORTAL(ch))
       act( "$n cackles and comps you!\n\r", mob, obj, ch, TO_VICT );
    else if (ch->level <= 5)
       ;
    else if (ch->gold < 10)
       {
       act( "$n goes about his business without looking at $p.",
             mob, obj, ch, TO_VICT );
       return;
       }
    else
       {
       ch->gold -= 10;
       send_to_char("Your purse feels lighter.\n\r", ch);
       }

    act( "$n fondles $p and examines it closely.",
       mob, obj, 0, TO_ROOM );
    
	spell_identify(0,100,ch,obj,TARGET_OBJ);
}


void do_junk(CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *obj;

    if ( ( obj = get_obj_carry( ch, argument, ch ) ) == NULL )
    {
        send_to_char( "\n\r{RYou do not have that item!{x\n\r", ch );
        return;
    }

    if ( !can_drop_obj( ch, obj ) )
    {
        send_to_char( "\n\r{RYou can't let go of it!{x\n\r", ch );
        return;
    }

    obj_from_char( obj );
    act( "\n\r{C$n {cjunks {C$p{c.{x", ch, obj, NULL, TO_ROOM );
    act( "\n\r{cYou junk {C$p{c.{x", ch, obj, NULL, TO_CHAR );
        extract_obj(obj);
    
   return;
}


void do_get( CHAR_DATA *ch, char *argument )
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  int counter=0;
  OBJ_DATA *obj;
  OBJ_DATA *obj_next;
  OBJ_DATA *container;
  bool found;

  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );


  if (!str_cmp(arg2,"from"))
    argument = one_argument(argument,arg2);

  /* Get type. */
  if ( arg1[0] == '\0' )
    {
    send_to_char( "Get what?\n\r", ch );
    return;
    }


  if ( arg2[0] == '\0' )
    {
    if ( !str_cmp( arg1, "corpse"))
      {
      if (!IS_NPC(ch))
        {
        if (!IS_IMP(ch))
          {
          obj = get_obj_list( ch, arg1, ch->in_room->contents );

          /* Need the following otherwise mud will crash when
           * any non-imp types: "get corpse" and there isnt one.
           * -- Rage
           */
          if ( !obj )
            {
            act( "I see no $T here.", ch, NULL, arg1, TO_CHAR );
            return;
            }

          sprintf(buf,"corpse %s", ch->name);
          if ( obj->item_type == ITEM_CORPSE_PC
          &&   IS_SET(obj->in_room->room_flags, ROOM_NOLOOT)
          &&   str_cmp(obj->name, buf) )
            {
            send_to_char("\n\r{RThat is not your corpse!{x\n\r", ch);
            return;
            }
          }
        }
      }  
    if ( str_cmp( arg1, "all" ) && str_prefix( "all.", arg1 ) )
	{

           /* 'get obj' */
	    obj = get_obj_list( ch, arg1, ch->in_room->contents );


	    if ( obj == NULL )
	    {
		act( "I see no $T here.", ch, NULL, arg1, TO_CHAR );
		return;
	    }

      if (!IS_NPC(ch))
        {
       if (!IS_IMP(ch))
         {
           sprintf(buf,"corpse %s", ch->name);
           if ( obj->item_type == ITEM_CORPSE_PC
           &&  IS_SET(obj->in_room->room_flags, ROOM_NOLOOT)
           &&  str_cmp(obj->name, buf) )
	    {
		send_to_char("\n\r{RThat is not your corpse!{x\n\r", ch);
		return;
	    }
          }
         }      

      if (!IS_NPC(ch))
        {
      if (!IS_IMP(ch))
        {
       if (IS_SET (obj->extra_flags, ITEM_CLAN_EQ)
       && (!IS_SET (ch->pact, PLR_PKILLER)))
          {
send_to_char( "\n\r{WOnly those who PKill can use Clan Equipment.{x\n\r", ch );
	  return;
           }
         }
        }
	    get_obj( ch, obj, NULL );
	}
	else
	{
	    /* 'get all' or 'get all.obj' */
	    found = FALSE;
	    for ( obj = ch->in_room->contents; obj != NULL; obj = obj_next )
	    {
		obj_next = obj->next_content;


      if (!IS_NPC(ch))
        {
       if (!IS_IMP(ch))
          {
            sprintf(buf,"corpse %s", ch->name);

           if ( obj->item_type == ITEM_CORPSE_PC
           &&  IS_SET(obj->in_room->room_flags, ROOM_NOLOOT)
           &&  str_cmp(obj->name, buf) )
	    {
		send_to_char("\n\r{RThat is not your corpse!{x\n\r", ch);
		return;
	    }
           }
         }

      if (!IS_NPC(ch))
        {
       if (!IS_IMP(ch))
         {
       if (IS_SET (obj->extra_flags, ITEM_CLAN_EQ)
       && (!IS_SET (ch->pact, PLR_PKILLER)))
         {
send_to_char( "\n\r{WOnly those who PKill can use Clan Equipment.{x\n\r", ch );
	  return;
         }
         }
       }

		if ( ( arg1[3] == '\0' || is_name( &arg1[4], obj->name ) )
		&&   can_see_obj( ch, obj ) )
		{
		    found = TRUE;
		    get_obj( ch, obj, NULL );
		    counter++;
		    if(counter>=50)
			obj=NULL;
		}
	    }

	    if ( !found ) 
	    {
		if ( arg1[3] == '\0' )
		    send_to_char( "I see nothing here.\n\r", ch );
		else
		    act( "I see no $T here.", ch, NULL, &arg1[4], TO_CHAR );
	    }
	}
    }
    else
    {
	/* 'get ... container' */
	if ( !str_cmp( arg2, "all" ) || !str_prefix( "all.", arg2 ) )
	{
	    send_to_char( "You can't do that.\n\r", ch );
	    return;
	}

	if ( ( container = get_obj_here( ch, arg2 ) ) == NULL )
	{
	    act( "I see no $T here.", ch, NULL, arg2, TO_CHAR );
	    return;
	}

	switch ( container->item_type )
	{
	default:
	    send_to_char( "That's not a container.\n\r", ch );
	    return;

	case ITEM_CONTAINER:
	case ITEM_CORPSE_NPC:
	    break;

	case ITEM_CORPSE_PC:
	    {

		if (!can_loot(ch,container))
		{
		    send_to_char( "You can't do that.\n\r", ch );
		    return;
		}
	    }
	}

	if ( IS_SET(container->value[1], CONT_CLOSED) )
	{
	    act( "The $d is closed.", ch, NULL, container->name, TO_CHAR );
	    return;
	}

	if ( str_cmp( arg1, "all" ) && str_prefix( "all.", arg1 ) )
	{
	    /* 'get obj container' */
	    obj = get_obj_list( ch, arg1, container->contains );


	    if ( obj == NULL )
	    {
		act( "I see nothing like that in the $T.",
		    ch, NULL, arg2, TO_CHAR );
		return;
	    }

      if (!IS_NPC(ch))
       {
       if (!IS_IMP(ch))
         {
       if (IS_SET (obj->extra_flags, ITEM_CLAN_EQ)
       && (!IS_SET (ch->pact, PLR_PKILLER)))
         {
send_to_char( "\n\r{WOnly those who PKill can use Clan Equipment.{x\n\r", ch );
	  return;
         }
         }
         }

	    get_obj( ch, obj, container );
	}
	else
	{
	    /* 'get all container' or 'get all.obj container' */
	    found = FALSE;
	    for ( obj = container->contains; obj != NULL; obj = obj_next )
	    {
		obj_next = obj->next_content;




		if ( ( arg1[3] == '\0' || is_name( &arg1[4], obj->name ) )
		&&   can_see_obj( ch, obj ) )
		{
		    found = TRUE;
		    if (container->pIndexData->vnum == OBJ_VNUM_PIT
		    &&  !IS_IMMORTAL(ch))
		    {
			send_to_char("Don't be so greedy!\n\r",ch);
			return;
		    }


      if (!IS_NPC(ch))
         {
       if (!IS_IMP(ch))
         {
       if (IS_SET (obj->extra_flags, ITEM_CLAN_EQ)
       && (!IS_SET (ch->pact, PLR_PKILLER)))
         {
send_to_char( "\n\r{WOnly those who PKill can use Clan Equipment.{x\n\r", ch );
	  return;
         }
         }
     }
		    get_obj( ch, obj, container );
		}
	    }

	    if ( !found )
	    {
		if ( arg1[3] == '\0' )
		    act( "I see nothing in the $T.",
			ch, NULL, arg2, TO_CHAR );
		else
		    act( "I see nothing like that in the $T.",
			ch, NULL, arg2, TO_CHAR );
	    }
	}
    }

    return;
}



void do_put( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *container;
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if (!str_cmp(arg2,"in") || !str_cmp(arg2,"on"))
	argument = one_argument(argument,arg2);

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "Put what in what?\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "all" ) || !str_prefix( "all.", arg2 ) )
    {
	send_to_char( "You can't do that.\n\r", ch );
	return;
    }

    if ( ( container = get_obj_here( ch, arg2 ) ) == NULL )
    {
	act( "I see no $T here.", ch, NULL, arg2, TO_CHAR );
	return;
    }

    if ( container->item_type != ITEM_CONTAINER )
    {
	send_to_char( "That's not a container.\n\r", ch );
	return;
    }

    if ( IS_SET(container->value[1], CONT_CLOSED) )
    {
	act( "The $d is closed.", ch, NULL, container->name, TO_CHAR );
	return;
    }

    if ( str_cmp( arg1, "all" ) && str_prefix( "all.", arg1 ) )
    {
	/* 'put obj container' */
	if ( ( obj = get_obj_carry( ch, arg1, ch ) ) == NULL )
	{
	    send_to_char( "You do not have that item.\n\r", ch );
	    return;
	}

	if ( obj == container )
	{
	    send_to_char( "You can't fold it into itself.\n\r", ch );
	    return;
	}

	if ( !can_drop_obj( ch, obj ) )
	{
	    send_to_char( "You can't let go of it.\n\r", ch );
	    return;
	}

    	if (WEIGHT_MULT(obj) != 100)
    	{
           send_to_char("You have a feeling that would be a bad idea.\n\r",ch);
            return;
        }

/*
	if (get_obj_weight( obj ) + get_true_weight( container )
	     > (container->value[0] * 10) 
	||  get_obj_weight(obj) > (container->value[3] * 10))
*/
	if (get_obj_weight( obj ) + get_true_weight( container )
	     > (container->value[0]) 
	||  get_obj_weight(obj) > (container->value[3]))
	{
	    send_to_char( "It won't fit.\n\r", ch );
	    return;
	}
	
	if (container->pIndexData->vnum == OBJ_VNUM_PIT &&  !CAN_WEAR(container,ITEM_TAKE))
	{
	    if (obj->timer)
		SET_BIT(obj->extra_flags,ITEM_HAD_TIMER);
	    else
	        obj->timer = number_range(100,200);
	}

	obj_from_char( obj );
	obj_to_obj( obj, container );

	if (IS_SET(container->value[1],CONT_PUT_ON))
	{
	    act("$n puts $p on $P.",ch,obj,container, TO_ROOM);
	    act("You put $p on $P.",ch,obj,container, TO_CHAR);
	}
	else
	{
	    act( "$n puts $p in $P.", ch, obj, container, TO_ROOM );
	    act( "You put $p in $P.", ch, obj, container, TO_CHAR );
	}
    }
    else
    {
	/* 'put all container' or 'put all.obj container' */
	for ( obj = ch->carrying; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;

	    if ( ( arg1[3] == '\0' || is_name( &arg1[4], obj->name ) )
	    &&   can_see_obj( ch, obj )
	    &&   WEIGHT_MULT(obj) == 100
	    &&   obj->wear_loc == WEAR_INVENTORY
	    &&   obj != container
	    &&   can_drop_obj( ch, obj )
	    &&   get_obj_weight( obj ) + get_true_weight( container )
		 <= (container->value[0] * 10) 
	    &&   get_obj_weight(obj) < (container->value[3] * 10))
	    {
	    	if (container->pIndexData->vnum == OBJ_VNUM_PIT &&  !CAN_WEAR(obj, ITEM_TAKE) )
			{
	    	    if (obj->timer)
			SET_BIT(obj->extra_flags,ITEM_HAD_TIMER);
	    	    else
	    	    	obj->timer = number_range(100,200);
			}
		obj_from_char( obj );
		obj_to_obj( obj, container );

        	if (IS_SET(container->value[1],CONT_PUT_ON))
        	{
            	    act("$n puts $p on $P.",ch,obj,container, TO_ROOM);
            	    act("You put $p on $P.",ch,obj,container, TO_CHAR);
        	}
		else
		{
		    act( "$n puts $p in $P.", ch, obj, container, TO_ROOM );
		    act( "You put $p in $P.", ch, obj, container, TO_CHAR );
		}
	    }
	}
    }

    return;
}



void do_drop( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  OBJ_DATA *obj_next;
  bool found;
  EXTRA_DESCR_DATA *  ed;

  argument = one_argument( argument, arg );

  if ( arg[0] == '\0' )
    {
    send_to_char( "Drop what?\n\r", ch );
    return;
    }

  if ( is_number( arg ) )
    {
    /* 'drop NNNN coins' */
    int amount, gold = 0, silver = 0;
    amount   = atoi(arg);
    argument = one_argument( argument, arg );
    if ( amount <= 0
    || ( str_cmp( arg, "coins" ) && str_cmp( arg, "coin" ) && 
      str_cmp( arg, "gold"  ) && str_cmp( arg, "silver") ) )
      {
	    send_to_char( "Sorry, you can't do that.\n\r", ch );
	    return;
      }

	if ( !str_cmp( arg, "coins") || !str_cmp(arg,"coin") 
	||   !str_cmp( arg, "silver"))
	{
	    if (ch->silver < amount)
	    {
		send_to_char("You don't have that much silver.\n\r",ch);
		return;
	    }

	    ch->silver -= amount;
	    silver = amount;
	}

	else
	{
	    if (ch->gold < amount)
	    {
		send_to_char("You don't have that much gold.\n\r",ch);
		return;
	    }

	    ch->gold -= amount;
  	    gold = amount;
	}

	for ( obj = ch->in_room->contents; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;

	    switch ( obj->pIndexData->vnum )
	    {
	    case OBJ_VNUM_SILVER_ONE:
		silver += 1;
		extract_obj(obj);
		break;

	    case OBJ_VNUM_GOLD_ONE:
		gold += 1;
		extract_obj( obj );
		break;

	    case OBJ_VNUM_SILVER_SOME:
		silver += obj->value[0];
		extract_obj(obj);
		break;

	    case OBJ_VNUM_GOLD_SOME:
		gold += obj->value[1];
		extract_obj( obj );
		break;

	    case OBJ_VNUM_COINS:
		silver += obj->value[0];
		gold += obj->value[1];
		extract_obj(obj);
		break;
	    }
	}

	obj_to_room( create_money( gold, silver ), ch->in_room );
	act( "$n drops some coins.", ch, NULL, NULL, TO_ROOM );
	send_to_char( "OK.\n\r", ch );
	return;
    }

    if ( str_cmp( arg, "all" ) && str_prefix( "all.", arg ) )
    {
        ROOM_INDEX_DATA *pToRoom;       /* Line added! */
        int i = 0;                      /* Line added! */

	/* 'drop obj' */
	if ( ( obj = get_obj_carry( ch, arg, ch ) ) == NULL )
	{
	    send_to_char( "You do not have that item.\n\r", ch );

	    return;
	}

	if ( !can_drop_obj( ch, obj ) )
	{
	    send_to_char( "You can't let go of it.\n\r", ch );
	    return;
	}

	obj_from_char( obj );
	obj_to_room( obj, ch->in_room );
	act( "$n drops $p.", ch, obj, NULL, TO_ROOM );
	act( "You drop $p.", ch, obj, NULL, TO_CHAR );
    if(obj->extra_descr != NULL || obj->pIndexData->extra_descr != NULL)
    {
        for (ed = obj->extra_descr; ed != NULL; ed = ed->next )
        {
            if(!str_cmp("dropmessage", ed->keyword))
                send_to_char(ed->description, ch);
        }

        for ( ed = obj->pIndexData->extra_descr; ed != NULL; ed = ed->next )
        {
            if(!str_cmp("dropmessage", ed->keyword))
                send_to_char(ed->description, ch);
        }
    }

	if (IS_OBJ_STAT(obj,ITEM_MELT_DROP)
	||  ch->level == BUILDER )
	{
	    act("$p dissolves into smoke.",ch,obj,NULL,TO_ROOM);
	    act("$p dissolves into smoke.",ch,obj,NULL,TO_CHAR);
	    extract_obj(obj);
	    return;
	}
        /* Phoenix: Start NEW BIT */
        for ( ; obj->in_room->exit[DIR_DOWN]
             && (pToRoom = obj->in_room->exit[DIR_DOWN]->u1.to_room)
             && obj->in_room->sector_type == SECT_AIR
             && i < 10; i++ ) {
           static ROOM_INDEX_DATA *visited_rooms[10];
           int j;
           
           if ( obj->in_room->people )
              act("$p falls away through the air below you.",
              obj->in_room->people, obj, NULL, TO_ALL);
           visited_rooms[i] = obj->in_room;
           for ( j = 0; j <= i; j++ )
              if ( visited_rooms[j] == pToRoom )
                 break;
           obj_from_room(obj);
           obj_to_room(obj, pToRoom);

           if ( obj->in_room->people && obj->in_room->exit[DIR_DOWN]
             && obj->in_room->exit[DIR_DOWN]->u1.to_room
             && obj->in_room->sector_type == SECT_AIR ) {
              if ( obj->in_room->people )
              act( "$p falls through the air.\n\r",
              obj->in_room->people, obj, NULL, TO_ALL);
           } else
              /* switch statement */
              act("$p falls from the sky and lands on the ground in "
              "front of you.", obj->in_room->people, obj, NULL, TO_ALL);
        }
        /* Phoenix: End NEW BIT */

    }
    else
    {
	/* 'drop all' or 'drop all.obj' */
	found = FALSE;
	for ( obj = ch->carrying; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;

	    if ( ( arg[3] == '\0' || is_name( &arg[4], obj->name ) )
	    &&   can_see_obj( ch, obj )
	    &&   obj->wear_loc == WEAR_INVENTORY
	    &&   can_drop_obj( ch, obj ) )
	    {
		found = TRUE;
		obj_from_char( obj );
		obj_to_room( obj, ch->in_room );
		act( "$n drops $p.", ch, obj, NULL, TO_ROOM );
		act( "You drop $p.", ch, obj, NULL, TO_CHAR );
    if(obj->extra_descr != NULL || obj->pIndexData->extra_descr != NULL)
    {
        for (ed = obj->extra_descr; ed != NULL; ed = ed->next )
        {
            if(!str_cmp("dropmessage", ed->keyword))
                send_to_char(ed->description, ch);
        }

        for ( ed = obj->pIndexData->extra_descr; ed != NULL; ed = ed->next )
        {
            if(!str_cmp("dropmessage", ed->keyword))
                send_to_char(ed->description, ch);
        }
    }

        	if (IS_OBJ_STAT(obj,ITEM_MELT_DROP))
        	{
             	    act("$p dissolves into smoke.",ch,obj,NULL,TO_ROOM);
            	    act("$p dissolves into smoke.",ch,obj,NULL,TO_CHAR);
            	    extract_obj(obj);
        	}
	    }
	}

	if ( !found )
	{
	    if ( arg[3] == '\0' )
		act( "You are not carrying anything.",
		    ch, NULL, arg, TO_CHAR );
	    else
		act( "You are not carrying any $T.",
		    ch, NULL, &arg[4], TO_CHAR );
	}
    }

    return;
}



void do_give( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char new_arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA  *obj;
    OBJ_DATA *obj_next;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

	if ( ch->level == BUILDER )
	  {
	  send_to_char( "\n\r{RYou get to BUILD it, not GIVE it.{x\n\r", ch );
	  return;
	  }

  if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
    send_to_char( "\n\r{CGive what to whom?{x\n\r", ch );
    return;
    }


    if ( is_number( arg1 ) )
    {
	/* 'give NNNN coins victim' */
	int amount;
	bool silver;

	amount   = atoi(arg1);
	if ( amount <= 0
	|| ( str_cmp( arg2, "coins" ) && str_cmp( arg2, "coin" ) && 
	     str_cmp( arg2, "gold"  ) && str_cmp( arg2, "silver")) )
	{
	    send_to_char( "\n\r{RSorry, you can't do that!{x\n\r", ch );
	    return;
	}

	silver = str_cmp(arg2,"gold");

	argument = one_argument( argument, arg2 );
	
	
	if ( arg2[0] == '\0' )
	{
	    send_to_char( "\n\r{CGive what to whom?{x\n\r", ch );
	    return;
	}


	if ( ( victim = get_char_room( ch, arg2 ) ) == NULL )
	{
	    send_to_char( "\n\r{CThey aren't here.{x\n\r", ch );
	    return;
	}

/* Removed by request of Argon (Yavi 7-1-04)
   if (!IS_IMP(ch))
     {
      if (IS_NPC(victim))
        {
         send_to_char( "\n\r{RThey can get their own!{x\n\r", ch );
         return;
        }
     }
*/
	if ( (!silver && ch->gold < amount) || (silver && ch->silver < amount) )
	{
	    send_to_char( "\n\r{RYou haven't got that much.{x\n\r", ch );
	    return;
	}

	if (silver)
	{
	    ch->silver		-= amount;
	    victim->silver 	+= amount;
	}
	else
	{
	    ch->gold		-= amount;
	    victim->gold	+= amount;
	}

	sprintf(buf,"\n\r{C$n {cgives you {C%d %s{c.\n\r{x",amount, silver ? "{WSILVER" : "{YGOLD");
	act( buf, ch, NULL, victim, TO_VICT    );
	act( "\n\r{C$n {cgives {W$N {csome coins.\n\r{x",  ch, NULL, victim, TO_NOTVICT );
	sprintf(buf,"\n\r{cYou give {W$N {C%d %s{c.\n\r{x",amount, silver ? "{WSILVER" : "{YGOLD");
	act( buf, ch, NULL, victim, TO_CHAR    );

	/*
	 * Bribe trigger
	 */
	if ( IS_NPC(victim) && HAS_TRIGGER( victim, TRIG_BRIBE ) )
	    mp_bribe_trigger( victim, ch, silver ? amount : amount * 100 );

	return;
    }


  if(!str_cmp(arg1, "all"))
    {
    if(!str_cmp(ch->name, arg2))
      {
	    send_to_char("\n\r{RYou already have everything!{x\n\r", ch);
	    return;
      }
    for(obj=ch->carrying;obj!=NULL;obj=obj_next)
      {
	    obj_next=obj->next_content;
	    if(!is_exact_name("all", obj->name))
        {
        one_argument(obj->name, new_arg);
        sprintf(buf, "%s %s", new_arg, arg2);
        do_function(ch, &do_give, buf);
        }
      }
    return;
    }

    if ( ( obj = get_obj_carry( ch, arg1, ch ) ) == NULL )
    {
	send_to_char( "\n\r{RYou do not have that item!{x\n\r", ch );
	return;
    }

    if ( obj->wear_loc != WEAR_INVENTORY )
    {
	send_to_char( "\n\r{CYou must remove it first.{x\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg2 ) ) == NULL )
    {
	send_to_char( "\n\r{RThey aren't here!{x\n\r", ch );
	return;
    }

    if (IS_NPC(victim) && victim->pIndexData->pShop != NULL)
    {
	act("\n\r{Y$N tells you 'Sorry, you'll have to sell that.'{x\n\r",
	    ch,NULL,victim,TO_CHAR);
	ch->reply = victim;
	return;
    }

    if ( !can_drop_obj( ch, obj ) )
    {
	send_to_char( "\n\r{RYou can't let go of it!{x\n\r", ch );
	return;
    }

  if (!IS_IMP(ch))
     {
    if (IS_NPC(victim))
    {
    send_to_char( "\n\r{RThey can get their own.{x\n\r", ch );
    return;
    }
     }

    if ( victim->carry_number + get_obj_number( obj ) > can_carry_n( victim ) )
    {
	act( "\n\r{C$N {chas $S hands full!{x\n\r", ch, NULL, victim, TO_CHAR );
	return;
    }

    if (get_carry_weight(victim) + get_obj_weight(obj) > can_carry_w( victim ) )
    {
	act( "\n\r{R$N {rcan't carry that much weight!{x\n\r", ch, NULL, victim, TO_CHAR );
	return;
    }

    if ( !can_see_obj( victim, obj ) )
    {
	act( "\n\r{R$N {rcan't see it!{x\n\r", ch, NULL, victim, TO_CHAR );
	return;
    }

        if(same_obj(obj, victim->carrying) >=30)
        {
	    act("\n\r{C$N {calready has enough!{x\n\r", ch, NULL, victim, TO_CHAR);
            return;
        }

             if (!IS_NPC(ch))
          {
       if (!IS_IMP(ch))
         {
       if (IS_SET (obj->extra_flags, ITEM_CLAN_EQ)
       && (!IS_SET (victim->pact, PLR_PKILLER)))
         {
send_to_char( "\n\r{ROnly those who PKill can use Clan Equipment.{x\n\r", ch );
	  return;
         }
        }
     }


    obj_from_char( obj );
    obj_to_char( obj, victim );
    MOBtrigger = FALSE;
    act( "\n\r{C$n {cgives {W$p {cto {C$N{c.{x\n\r", ch, obj, victim, TO_NOTVICT );
    act( "\n\r{C$n {cgives you {W$p{c.{x\n\r",   ch, obj, victim, TO_VICT    );
    act( "\n\r{cYou give {W$p {cto {C$N{c.{x\n\r", ch, obj, victim, TO_CHAR    );
    MOBtrigger = TRUE;

    /*
     * Give trigger
     */
    if ( IS_NPC(victim) && HAS_TRIGGER( victim, TRIG_GIVE ) )
	mp_give_trigger( victim, ch, obj );

    return;
}


/* for poisoning weapons and food/drink */
void do_envenom(CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *obj;
    AFFECT_DATA af;
    int percent,skill;

    /* find out what */

    if ( !IS_NPC(ch)
    &&   !can_use_skpell( ch, gsn_envenom) )          
      {
        send_to_char( "\n\r{BYou put lots of Tabassco on your blade, surely that will poison someone!{x\n\r",ch);
        return;   
      }

    if (argument[0] == '\0')
    {
	send_to_char("Envenom what item?\n\r",ch);
	return;
    }

    obj =  get_obj_list(ch,argument,ch->carrying);

    if (obj== NULL)
    {
	send_to_char("You don't have that item.\n\r",ch);
	return;
    }

    if ((skill = get_skill(ch,gsn_envenom)) < 1)
    {
	send_to_char("Are you crazy? You'd poison yourself!\n\r",ch);
	return;
    }

    if (obj->item_type == ITEM_FOOD || obj->item_type == ITEM_DRINK_CON)
    {
	if (IS_OBJ_STAT(obj,ITEM_BLESS) 
        || IS_OBJ_STAT(obj,ITEM_FORTIFIED))
	{
	    act("You fail to poison $p.",ch,obj,NULL,TO_CHAR);
	    return;
	}

	if (number_percent() < skill)  /* success! */
	{
	    act("$n treats $p with deadly poison.",ch,obj,NULL,TO_ROOM);
	    act("You treat $p with deadly poison.",ch,obj,NULL,TO_CHAR);
	    if (!obj->value[3])
	    {
		obj->value[3] = 1;
		check_improve(ch,gsn_envenom,TRUE,4);
	    }
	    WAIT_STATE(ch,skill_table[gsn_envenom].beats);
	    return;
	}

	act("You fail to poison $p.",ch,obj,NULL,TO_CHAR);
	if (!obj->value[3])
	    check_improve(ch,gsn_envenom,FALSE,4);
	WAIT_STATE(ch,skill_table[gsn_envenom].beats);
	return;
     }

    if (obj->item_type == ITEM_WEAPON)
    {
        if (IS_WEAPON_STAT(obj,WEAPON_FLAMING)
        ||  IS_WEAPON_STAT(obj,WEAPON_FROST)
        ||  IS_WEAPON_STAT(obj,WEAPON_VAMPIRIC)
        ||  IS_WEAPON_STAT(obj,WEAPON_SHARP)
        ||  IS_WEAPON_STAT(obj,WEAPON_VORPAL)
        ||  IS_WEAPON_STAT(obj,WEAPON_SHOCKING)
        ||  IS_OBJ_STAT(obj,ITEM_BLESS) 
        || IS_OBJ_STAT(obj,ITEM_FORTIFIED))
        {
            act("You can't seem to envenom $p.",ch,obj,NULL,TO_CHAR);
            return;
        }

	if (obj->value[3] < 0 
	||  attack_table[obj->value[3]].damage == DAM_BASH)
	{
	    send_to_char("You can only envenom edged weapons.\n\r",ch);
	    return;
	}

        if (IS_WEAPON_STAT(obj,WEAPON_POISON))
        {
            act("$p is already envenomed.",ch,obj,NULL,TO_CHAR);
            return;
        }

	percent = number_percent();
	if (percent < skill)
	{
 
            af.where     = TO_WEAPON;
            af.type      = gsn_poison;
            af.level     = ch->level * percent / 100;
            af.duration  = ch->level/2 * percent / 100;
            af.location  = 0;
            af.modifier  = 0;
            af.bitvector = WEAPON_POISON;
            affect_to_obj(obj,&af);
 
            act("$n coats $p with deadly venom.",ch,obj,NULL,TO_ROOM);
	    act("You coat $p with venom.",ch,obj,NULL,TO_CHAR);
	    check_improve(ch,gsn_envenom,TRUE,3);
	    WAIT_STATE(ch,skill_table[gsn_envenom].beats);
            return;
        }
	else
	{
	    act("You fail to envenom $p.",ch,obj,NULL,TO_CHAR);
	    check_improve(ch,gsn_envenom,FALSE,3);
	    WAIT_STATE(ch,skill_table[gsn_envenom].beats);
	    return;
	}
    }
 
    act("You can't poison $p.",ch,obj,NULL,TO_CHAR);
    return;
}

void do_fill( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *fountain;
    bool found;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Fill what?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_carry( ch, arg, ch ) ) == NULL )
    {
	send_to_char( "You do not have that item.\n\r", ch );
	return;
    }

    found = FALSE;
    for ( fountain = ch->in_room->contents; fountain != NULL;
	fountain = fountain->next_content )
    {
	if ( fountain->item_type == ITEM_FOUNTAIN )
	{
	    found = TRUE;
	    break;
	}
    }

    if ( !found )
    {
	send_to_char( "There is no fountain here!\n\r", ch );
	return;
    }

    if ( obj->item_type != ITEM_DRINK_CON )
    {
	send_to_char( "You can't fill that.\n\r", ch );
	return;
    }

    if ( obj->value[1] != 0 && obj->value[2] != fountain->value[2] )
    {
	send_to_char( "There is already another liquid in it.\n\r", ch );
	return;
    }

    if ( obj->value[1] >= obj->value[0] )
    {
	send_to_char( "Your container is full.\n\r", ch );
	return;
    }

    sprintf(buf,"You fill $p with %s from $P.",
	liq_table[fountain->value[2]].liq_name);
    act( buf, ch, obj,fountain, TO_CHAR );
    sprintf(buf,"$n fills $p with %s from $P.",
	liq_table[fountain->value[2]].liq_name);
    act(buf,ch,obj,fountain,TO_ROOM);
    obj->value[2] = fountain->value[2];
    obj->value[1] = obj->value[0];
    return;
}

void do_pour (CHAR_DATA *ch, char *argument)
{
    char arg[MAX_STRING_LENGTH],buf[MAX_STRING_LENGTH];
    OBJ_DATA *out, *in;
    CHAR_DATA *vch = NULL;
    int amount;

    argument = one_argument(argument,arg);
    
    if (arg[0] == '\0' || argument[0] == '\0')
    {
	send_to_char("Pour what into what?\n\r",ch);
	return;
    }
    

    if ((out = get_obj_carry(ch,arg, ch)) == NULL)
    {
	send_to_char("You don't have that item.\n\r",ch);
	return;
    }

    if (out->item_type != ITEM_DRINK_CON)
    {
	send_to_char("That's not a drink container.\n\r",ch);
	return;
    }

    if (!str_cmp(argument,"out"))
    {
	if (out->value[1] == 0)
	{
	    send_to_char("It's already empty.\n\r",ch);
	    return;
	}

	out->value[1] = 0;
	out->value[3] = 0;
	sprintf(buf,"You invert $p, spilling %s all over the ground.",
		liq_table[out->value[2]].liq_name);
	act(buf,ch,out,NULL,TO_CHAR);
	
	sprintf(buf,"$n inverts $p, spilling %s all over the ground.",
		liq_table[out->value[2]].liq_name);
	act(buf,ch,out,NULL,TO_ROOM);
	return;
    }

    if ((in = get_obj_here(ch,argument)) == NULL)
    {
	vch = get_char_room(ch,argument);

	if (vch == NULL)
	{
	    send_to_char("Pour into what?\n\r",ch);
	    return;
	}

	in = get_eq_char(vch,WEAR_HOLD);

	if (in == NULL)
	{
	    send_to_char("They aren't holding anything.",ch);
 	    return;
	}
    }

    if (in->item_type != ITEM_DRINK_CON)
    {
	send_to_char("You can only pour into other drink containers.\n\r",ch);
	return;
    }
    
    if (in == out)
    {
	send_to_char("You cannot change the laws of physics!\n\r",ch);
	return;
    }

    if (in->value[1] != 0 && in->value[2] != out->value[2])
    {
	send_to_char("They don't hold the same liquid.\n\r",ch);
	return;
    }

    if (out->value[1] == 0)
    {
	act("There's nothing in $p to pour.",ch,out,NULL,TO_CHAR);
	return;
    }

    if (in->value[1] >= in->value[0])
    {
	act("$p is already filled to the top.",ch,in,NULL,TO_CHAR);
	return;
    }

    amount = UMIN(out->value[1],in->value[0] - in->value[1]);

    in->value[1] += amount;
    out->value[1] -= amount;
    in->value[2] = out->value[2];
    
    if (vch == NULL)
    {
    	sprintf(buf,"You pour %s from $p into $P.",
	    liq_table[out->value[2]].liq_name);
    	act(buf,ch,out,in,TO_CHAR);
    	sprintf(buf,"$n pours %s from $p into $P.",
	    liq_table[out->value[2]].liq_name);
    	act(buf,ch,out,in,TO_ROOM);
    }
    else
    {
        sprintf(buf,"You pour some %s for $N.",
            liq_table[out->value[2]].liq_name);
        act(buf,ch,NULL,vch,TO_CHAR);
	sprintf(buf,"$n pours you some %s.",
	    liq_table[out->value[2]].liq_name);
	act(buf,ch,NULL,vch,TO_VICT);
        sprintf(buf,"$n pours some %s for $N.",
            liq_table[out->value[2]].liq_name);
        act(buf,ch,NULL,vch,TO_NOTVICT);
	
    }
}

void do_drink( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int amount;
    int liquid;
    ROOM_INDEX_DATA *location;

    one_argument( argument, arg );

    if ( ch->fighting != NULL )
      {   
       send_to_char( "\n\r{WDrinking is much to difficult while beating something up.\n\r{x", ch );
       return;
      }

    if ( arg[0] == '\0' )
    {
	for ( obj = ch->in_room->contents; obj; obj = obj->next_content )
	{
	    if ( obj->item_type == ITEM_FOUNTAIN )
		break;
	}

	if ( obj == NULL )
	{
	    send_to_char( "Drink what?\n\r", ch );
	    return;
	}
    }
    else
    {
	if ( ( obj = get_obj_here( ch, arg ) ) == NULL )
	{
	    send_to_char( "You can't find it.\n\r", ch );
	    return;
	}
    }

if (!IS_NPC(ch))
   {
  if (IS_SET(ch->comm, COMM_AFK))
    {
     send_to_char("AFK mode removed due to PLAYER INPUT. Type 'replay' to see tells.\n\r",ch);
      REMOVE_BIT(ch->comm,COMM_AFK);
    }
   }

	 location = get_room_index(ROOM_VNUM_DRAGONPIT);

        if ( ch->in_room == location )
         {
	  send_to_char( "\n\r{GNot until the DragonPIT Begins.{x\n\r", ch );
  	  return;
         }


    if ( !IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK] > 10 )
    {
	send_to_char( "You fail to reach your mouth.  *Hic*\n\r", ch );
	return;
    }

    switch ( obj->item_type )
    {
    default:
	send_to_char( "You can't drink from that.\n\r", ch );
	return;

    case ITEM_FOUNTAIN:
        if ( ( liquid = obj->value[2] )  < 0 )
        {
            bug( "Do_drink: bad liquid number %d.", liquid );
            liquid = obj->value[2] = 0;
        }
	amount = liq_table[liquid].liq_affect[4] * 3;
	break;

    case ITEM_DRINK_CON:
	if ( obj->value[1] <= 0 )
	{
	    send_to_char( "It is already empty.\n\r", ch );
	    return;
	}

	if ( ( liquid = obj->value[2] )  < 0 )
	{
	    bug( "Do_drink: bad liquid number %d.", liquid );
	    liquid = obj->value[2] = 0;
	}

        amount = liq_table[liquid].liq_affect[4];
        amount = UMIN(amount, obj->value[1]);
	break;
     }
    if (!IS_NPC(ch) && !IS_IMMORTAL(ch) 
    &&  ch->pcdata->condition[COND_FULL] > 45)
    {
	send_to_char("You're too full to drink more.\n\r",ch);
	return;
    }

    act( "$n drinks $T from $p.",
	ch, obj, liq_table[liquid].liq_name, TO_ROOM );
    act( "You drink $T from $p.",
	ch, obj, liq_table[liquid].liq_name, TO_CHAR );

    gain_condition( ch, COND_DRUNK,
	amount * liq_table[liquid].liq_affect[COND_DRUNK] / 36 );
    gain_condition( ch, COND_FULL,
	amount * liq_table[liquid].liq_affect[COND_FULL] / 4 );
    gain_condition( ch, COND_THIRST,
	amount * liq_table[liquid].liq_affect[COND_THIRST] / 10 );
    gain_condition(ch, COND_HUNGER,
	amount * liq_table[liquid].liq_affect[COND_HUNGER] / 2 );

    if ( !IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK]  > 10 )
	send_to_char( "You feel drunk.\n\r", ch );
    if ( !IS_NPC(ch) && ch->pcdata->condition[COND_FULL]   > 40 )
	send_to_char( "You are full.\n\r", ch );
    if ( !IS_NPC(ch) && ch->pcdata->condition[COND_THIRST] > 40 )
	send_to_char( "Your thirst is quenched.\n\r", ch );
	
    if ( obj->value[3] != 0 )
    {
	/* The drink was poisoned ! */
	AFFECT_DATA af;

	act( "$n chokes and gags.", ch, NULL, NULL, TO_ROOM );
	send_to_char( "You choke and gag.\n\r", ch );
	af.where     = TO_AFFECTS;
	af.type      = gsn_poison;
	af.level	 = number_fuzzy(amount); 
	af.duration  = 3 * amount;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = AFF_POISON;
	affect_join( ch, &af );
    }
	
    if (obj->value[0] > 0)
        obj->value[1] -= amount;

    return;
}



void do_eat( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    ROOM_INDEX_DATA *location;

    location = NULL;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Eat what?\n\r", ch );
	return;
    }

    if(!str_cmp(arg, "all"))
    {
        for (obj = ch->carrying; obj; obj = obj->next_content)
            if (can_see_obj(ch,obj) && obj->wear_loc != WEAR_INVENTORY)
		if(str_cmp(obj->name, "all"))
                    do_function(ch, &do_eat, obj->name);
	return;
    }

    if ( ( obj = get_obj_carry( ch, arg, ch ) ) == NULL )
    {
	send_to_char( "You do not have that item.\n\r", ch );
	return;
    }

    if ( ch->fighting != NULL )
      {   
       send_to_char( "\n\r{WYou are to distracted by combat to eat lunch.\n\r{x", ch );
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

    if ( !IS_IMMORTAL(ch) )
    {
	if ( obj->item_type != ITEM_FOOD && obj->item_type != ITEM_PILL )
	{
	    send_to_char( "That's not edible.\n\r", ch );
	    return;
	}

	if ( !IS_NPC(ch) && ch->pcdata->condition[COND_FULL] > 40 )
	{   
	    send_to_char( "You are too full to eat more.\n\r", ch );
	    return;
	}
    }

	 location = get_room_index(ROOM_VNUM_DRAGONPIT);

        if ( ch->in_room == location )
         {
	  send_to_char( "\n\r{GNot until the DragonPIT Begins.{x\n\r", ch );
  	  return;
         }



    obj->owner = NULL;
    act( "$n eats $p.",  ch, obj, NULL, TO_ROOM );
    act( "You eat $p.", ch, obj, NULL, TO_CHAR );

    switch ( obj->item_type )
    {

    case ITEM_FOOD:
	if ( !IS_NPC(ch) )
	{
	    int condition;

	    condition = ch->pcdata->condition[COND_HUNGER];
	    gain_condition( ch, COND_FULL, obj->value[0] );
	    gain_condition( ch, COND_HUNGER, obj->value[1]);
	    if ( condition == 0 && ch->pcdata->condition[COND_HUNGER] > 0 )
		send_to_char( "You are no longer hungry.\n\r", ch );
	    else if ( ch->pcdata->condition[COND_FULL] > 40 )
		send_to_char( "You are full.\n\r", ch );
	}

	if ( obj->value[3] != 0 )
	{
	    /* The food was poisoned! */
	    AFFECT_DATA af;

	    act( "$n chokes and gags.", ch, 0, 0, TO_ROOM );
	    send_to_char( "You choke and gag.\n\r", ch );

	    af.where	 = TO_AFFECTS;
	    af.type      = gsn_poison;
	    af.level 	 = number_fuzzy(obj->value[0]);
	    af.duration  = 2 * obj->value[0];
	    af.location  = APPLY_NONE;
	    af.modifier  = 0;
	    af.bitvector = AFF_POISON;
	    affect_join( ch, &af );
	}
	break;

    case ITEM_PILL: 

    if (!IS_IMP(ch))
      {
       if (ch->level < obj->level)
         {
	  send_to_char("\n\r{GThis pill is too powerful for you to eat.{x\n\r",ch);
	  return;
         }
       else
        {
	 obj_cast_spell( obj->value[1], obj->value[0], ch, ch, NULL );
	 obj_cast_spell( obj->value[2], obj->value[0], ch, ch, NULL );
	 obj_cast_spell( obj->value[3], obj->value[0], ch, ch, NULL );
	 break;
        }
      }
    }

    extract_obj( obj );
    return;
}



/*
 * Remove an object.
 */
bool remove_obj( CHAR_DATA *ch, int iWear, bool fReplace )
{
    OBJ_DATA *obj;

    if ( ( obj = get_eq_char( ch, iWear ) ) == NULL )
	return TRUE;

    if ( !fReplace )
	return FALSE;

    if ( IS_SET(obj->extra_flags, ITEM_NOREMOVE) )
    {
	act( "You can't remove $p.", ch, obj, NULL, TO_CHAR );
	return FALSE;
    }

    unequip_char( ch, obj );
    act( "$n stops using $p.", ch, obj, NULL, TO_ROOM );
    act( "You stop using $p.", ch, obj, NULL, TO_CHAR );
    return TRUE;
}



/*
 * Wear one object.
 * Optional replacement of existing objects.
 * Big repetitive code, ick.
 */
void wear_obj( CHAR_DATA *ch, OBJ_DATA *obj, bool fReplace )
{
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *weapon;
          if (!IS_NPC(ch))
      {
    if (!IS_IMP(ch))
      {
       if ( ch->level < obj->level)
       {
	sprintf( buf, "You must be level %d to use this object.\n\r",
	    obj->level );
	send_to_char( buf, ch );
	act( "$n tries to use $p, but is too inexperienced.",
	    ch, obj, NULL, TO_ROOM );
	return;
       }
      }
     }



    if ( CAN_WEAR( obj, ITEM_WEAR_FINGER ) )
    {
	if ( get_eq_char( ch, WEAR_FINGER_L ) != NULL
	&&   get_eq_char( ch, WEAR_FINGER_R ) != NULL
	&&   !remove_obj( ch, WEAR_FINGER_L, fReplace )
	&&   !remove_obj( ch, WEAR_FINGER_R, fReplace ) )
	    return;

	if ( get_eq_char( ch, WEAR_FINGER_L ) == NULL )
	{
	    act( "$n wears $p on $s left finger.",    ch, obj, NULL, TO_ROOM );
	    act( "You wear $p on your left finger.",  ch, obj, NULL, TO_CHAR );
	    equip_char( ch, obj, WEAR_FINGER_L );
	    return;
	}

	if ( get_eq_char( ch, WEAR_FINGER_R ) == NULL )
	{
	    act( "$n wears $p on $s right finger.",   ch, obj, NULL, TO_ROOM );
	    act( "You wear $p on your right finger.", ch, obj, NULL, TO_CHAR );
	    equip_char( ch, obj, WEAR_FINGER_R );
	    return;
	}

	bug( "Wear_obj: no free finger.", 0 );
	send_to_char( "You already wear two rings.\n\r", ch );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_NECK ) )
    {
	if ( get_eq_char( ch, WEAR_NECK_1 ) != NULL
	&&   get_eq_char( ch, WEAR_NECK_2 ) != NULL
	&&   !remove_obj( ch, WEAR_NECK_1, fReplace )
	&&   !remove_obj( ch, WEAR_NECK_2, fReplace ) )
	    return;

	if ( get_eq_char( ch, WEAR_NECK_1 ) == NULL )
	{
	    act( "$n wears $p around $s neck.",   ch, obj, NULL, TO_ROOM );
	    act( "You wear $p around your neck.", ch, obj, NULL, TO_CHAR );
	    equip_char( ch, obj, WEAR_NECK_1 );
	    return;
	}

	if ( get_eq_char( ch, WEAR_NECK_2 ) == NULL )
	{
	    act( "$n wears $p around $s neck.",   ch, obj, NULL, TO_ROOM );
	    act( "You wear $p around your neck.", ch, obj, NULL, TO_CHAR );
	    equip_char( ch, obj, WEAR_NECK_2 );
	    return;
	}

	bug( "Wear_obj: no free neck.", 0 );
	send_to_char( "You already wear two neck items.\n\r", ch );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_BODY ) )
    {
	if ( !remove_obj( ch, WEAR_BODY, fReplace ) )
	    return;
	act( "$n wears $p on $s torso.",   ch, obj, NULL, TO_ROOM );
	act( "You wear $p on your torso.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_BODY );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_HEAD ) )
    {
	if ( !remove_obj( ch, WEAR_HEAD, fReplace ) )
	    return;
	act( "$n wears $p on $s head.",   ch, obj, NULL, TO_ROOM );
	act( "You wear $p on your head.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_HEAD );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_LEGS ) )
    {
	if ( !remove_obj( ch, WEAR_LEGS, fReplace ) )
	    return;
	act( "$n wears $p on $s legs.",   ch, obj, NULL, TO_ROOM );
	act( "You wear $p on your legs.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_LEGS );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_FEET ) )
    {
	if ( !remove_obj( ch, WEAR_FEET, fReplace ) )
	    return;
	act( "$n wears $p on $s feet.",   ch, obj, NULL, TO_ROOM );
	act( "You wear $p on your feet.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_FEET );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_HANDS ) )
    {
	if ( !remove_obj( ch, WEAR_HANDS, fReplace ) )
	    return;
	act( "$n wears $p on $s hands.",   ch, obj, NULL, TO_ROOM );
	act( "You wear $p on your hands.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_HANDS );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_ARMS ) )
    {
	if ( !remove_obj( ch, WEAR_ARMS, fReplace ) )
	    return;
	act( "$n wears $p on $s arms.",   ch, obj, NULL, TO_ROOM );
	act( "You wear $p on your arms.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_ARMS );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_ABOUT ) )
    {
	if ( !remove_obj( ch, WEAR_ABOUT, fReplace ) )
	    return;
	act( "$n wears $p about $s torso.",   ch, obj, NULL, TO_ROOM );
	act( "You wear $p about your torso.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_ABOUT );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_WAIST ) )
    {
	if ( !remove_obj( ch, WEAR_WAIST, fReplace ) )
	    return;
	act( "$n wears $p about $s waist.",   ch, obj, NULL, TO_ROOM );
	act( "You wear $p about your waist.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_WAIST );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_WRIST ) )
    {
	if ( get_eq_char( ch, WEAR_WRIST_L ) != NULL
	&&   get_eq_char( ch, WEAR_WRIST_R ) != NULL
	&&   !remove_obj( ch, WEAR_WRIST_L, fReplace )
	&&   !remove_obj( ch, WEAR_WRIST_R, fReplace ) )
	    return;

	if ( get_eq_char( ch, WEAR_WRIST_L ) == NULL )
	{
	    act( "$n wears $p around $s left wrist.",
		ch, obj, NULL, TO_ROOM );
	    act( "You wear $p around your left wrist.",
		ch, obj, NULL, TO_CHAR );
	    equip_char( ch, obj, WEAR_WRIST_L );
	    return;
	}

	if ( get_eq_char( ch, WEAR_WRIST_R ) == NULL )
	{
	    act( "$n wears $p around $s right wrist.",
		ch, obj, NULL, TO_ROOM );
	    act( "You wear $p around your right wrist.",
		ch, obj, NULL, TO_CHAR );
	    equip_char( ch, obj, WEAR_WRIST_R );
	    return;
	}

	bug( "Wear_obj: no free wrist.", 0 );
	send_to_char( "You already wear two wrist items.\n\r", ch );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_SHIELD ) )
    {
	OBJ_DATA *weapon;

	if ( !remove_obj( ch, WEAR_SHIELD, fReplace ) )
	    return;

	weapon = get_eq_char(ch,WEAR_WIELD);
	if (weapon != NULL && ch->size < SIZE_GIANT
	&&  IS_WEAPON_STAT(weapon,WEAPON_TWO_HANDS))
	{
	    send_to_char("\n\r{MYour hands are tied up with your weapon!{x\n\r",ch);
	    return;
	}

        if (get_eq_char (ch, WEAR_SECONDARY) != NULL)
        {
            send_to_char ("You cannot use a shield while using 2 weapons.\n\r",ch);
            return;
        }

	act( "$n wears $p as a shield.", ch, obj, NULL, TO_ROOM );
	act( "You wear $p as a shield.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_SHIELD );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WIELD ) )
    {
	int sn,skill;

	if ( !remove_obj( ch, WEAR_WIELD, fReplace ) )
	    return;

	if ( !IS_NPC(ch) 
	&& get_obj_weight(obj) > (str_app[get_curr_stat(ch,STAT_STR)].wield  
		* 10))
	{
	    send_to_char( "It is too heavy for you to wield.\n\r", ch );
	    return;
	}

	if (!IS_NPC(ch) 
	&&  ch->size < SIZE_GIANT 
	&&  IS_WEAPON_STAT(obj,WEAPON_TWO_HANDS)
 	&&  (get_eq_char(ch,WEAR_SHIELD) != NULL
        ||  get_eq_char(ch,WEAR_HOLD) != NULL
        ||  get_eq_char(ch,WEAR_SECONDARY) != NULL))
	{
	    send_to_char("\n\r{RYou need two hands free for that weapon.{x\n\r",ch);
	    return;
	}

      if ( get_eq_char(ch, WEAR_SECONDARY) != NULL)
      {
       remove_obj(ch, WEAR_SECONDARY, TRUE);
      }

	act( "$n wields $p.", ch, obj, NULL, TO_ROOM );
	act( "You wield $p.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_WIELD );

        sn = get_weapon_sn(ch, FALSE);

	if (sn == gsn_hand_to_hand)
	   return;

        skill = get_weapon_skill(ch,sn);
 
        if (skill >= 100)
            act("\n\r{C$p feels like a part of you!{x\n\r",ch,obj,NULL,TO_CHAR);
        else if (skill > 85)
            act("\n\r{MYou feel quite confident with $p{x\n\r.",ch,obj,NULL,TO_CHAR);
        else if (skill > 70)
            act("\n\r{RYou are skilled with $p.{x\n\r",ch,obj,NULL,TO_CHAR);
        else if (skill > 50)
            act("\n\r{BYour skill with $p is adequate.{x\n\r",ch,obj,NULL,TO_CHAR);
        else if (skill > 25)
            act("\n\r{G$p feels a little clumsy in your hands.{x\n\r",ch,obj,NULL,TO_CHAR);
        else if (skill > 1)
            act("\n\r{WYou fumble and almost drop $p.{x\n\r",ch,obj,NULL,TO_CHAR);
        else
            act("\n\r{yYou don't even know which end is up on $p.{x\n\r",
                ch,obj,NULL,TO_CHAR);
	return;
    }

    if ( CAN_WEAR( obj, ITEM_HOLD ))
    {
	weapon = get_eq_char(ch,WEAR_WIELD);

	if ( !remove_obj( ch, WEAR_HOLD, fReplace ) )
	    return;

       if ( obj->item_type == ITEM_LIGHT_SOURCE )
        {
        if ( !remove_obj( ch, WEAR_LIGHT, fReplace ) )
            return;
        act( "$n lights $p and holds it.", ch, obj, NULL, TO_ROOM );
        act( "You light $p and hold it.",  ch, obj, NULL, TO_CHAR );
        equip_char( ch, obj, WEAR_LIGHT );
        return;
        }

        if ((get_eq_char (ch, WEAR_SECONDARY) != NULL)
	||  (weapon != NULL
        &&  IS_WEAPON_STAT(weapon,WEAPON_TWO_HANDS)
        &&  ch->size < SIZE_GIANT))
        {
            send_to_char ("\n\r{GYour hands are full from your weapons.{x\n\r",ch);
            return;
        }

	act( "$n holds $p in $s hand.",   ch, obj, NULL, TO_ROOM );
	act( "You hold $p in your hand.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_HOLD );
	return;
    }

    if ( CAN_WEAR(obj,ITEM_WEAR_FLOAT) )
    {
	if (!remove_obj(ch,WEAR_FLOAT, fReplace) )
	    return;
	act("$n releases $p to float next to $m.",ch,obj,NULL,TO_ROOM);
	act("You release $p and it floats next to you.",ch,obj,NULL,TO_CHAR);
	equip_char(ch,obj,WEAR_FLOAT);
	return;
    }

   if ( obj->item_type == ITEM_LIGHT_SOURCE )
    {
        if ( !remove_obj( ch, WEAR_LIGHT, fReplace ) )
            return;
        act( "$n lights $p and holds it.", ch, obj, NULL, TO_ROOM );
        act( "You light $p and hold it.",  ch, obj, NULL, TO_CHAR );
        equip_char( ch, obj, WEAR_LIGHT );
        return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_BACK ) )
    {
	if ( !remove_obj( ch, WEAR_BACK, fReplace ) )
	    return;
	act( "$n wears $p on $s back.",   ch, obj, NULL, TO_ROOM );
	act( "You wear $p on your back.", ch, obj, NULL, TO_CHAR );
	equip_char( ch, obj, WEAR_BACK );
	return;
    }

    if ( fReplace )
	send_to_char( "You can't wear, wield, or hold that.\n\r", ch );

    return;
}



void do_wear( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *weapon;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Wear, wield, or hold what?\n\r", ch );
	return;
    }


    if ( !str_cmp( arg, "all" ) )
    {
	OBJ_DATA *obj_next;

	for ( obj = ch->carrying; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;


    if (!IS_NPC(ch))
      {
    if (!IS_IMP(ch))
         {
          if (IS_SET (obj->extra_flags, ITEM_CLAN_EQ) 
          && (!IS_SET (ch->pact, PLR_PKILLER)))
          { 
           send_to_char( "\n\r{WOnly those who PKill can use Clan Equipment.{x\n\r", ch ); 
	   return;
          }
         }
       }

        weapon = get_eq_char(ch,WEAR_WIELD);
        if ((weapon != NULL)
        && ( IS_WEAPON_STAT(weapon,WEAPON_TWO_HANDS)
        && ch->size < SIZE_GIANT)
        && ((obj->wear_loc == WEAR_SHIELD)
        || (obj->wear_loc == WEAR_HOLD)
        || (obj->wear_loc == WEAR_SECONDARY)))
          { 
   send_to_char( "\n\r{RYour hands are full with your two-handed weapon.{x\n\r", ch ); 
	   return;
          }

	    if ( obj->wear_loc == WEAR_INVENTORY && can_see_obj( ch, obj ) )
		wear_obj( ch, obj, FALSE );
	}
	return;
    }
    else
    {
	if ( ( obj = get_obj_carry( ch, arg, ch ) ) == NULL )
	{
	    send_to_char( "You do not have that item.\n\r", ch );
	    return;
	}

    if (!IS_NPC(ch))
      {
       if (!IS_IMP(ch))
         {
       if (IS_SET (obj->extra_flags, ITEM_CLAN_EQ) 
       && (!IS_SET (ch->pact, PLR_PKILLER)))
         { 
send_to_char( "\n\r{WOnly those who PKill can use Clan Equipment.{x\n\r", ch ); 
	  return;
         }
         }
        }
     
	wear_obj( ch, obj, TRUE );
    }

    return;
}



void do_remove( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "\n\r{WRemove what?{x\n\r", ch );
	return;
    }

    if(!str_cmp(argument, "all"))
    {
       send_to_char("\n\r",ch);


        for (obj = ch->carrying; obj; obj = obj->next_content)
            if (can_see_obj(ch,obj) && obj->wear_loc != WEAR_INVENTORY)
                remove_obj (ch, obj->wear_loc, TRUE);
	return;
    }

    if ( ( obj = get_obj_wear( ch, arg ) ) == NULL )
    {
	send_to_char( "\n\r{RYou do not have that item.{x\n\r", ch );
	return;
    }



    if (IS_SET(obj->wear_flags, ITEM_WIELD)  
    && ( get_eq_char(ch, WEAR_SECONDARY) != NULL))
      {
       send_to_char("\n\r{WWithout a primary weapon, you are unable to wield a secondary one.{x\n\r\n\r",ch);
       remove_obj(ch, WEAR_WIELD, TRUE);
       remove_obj(ch, WEAR_SECONDARY, TRUE);
       return;
      }



       send_to_char("\n\r",ch);
    remove_obj( ch, obj->wear_loc, TRUE );
    return;
}



void do_sacrifice( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *obj2;
    OBJ_DATA *obj_next;
    int silver;
     int counter = 0;    

    /* variables for AUTOSPLIT */
    CHAR_DATA *gch;
    int members;
    char buffer[100];


    one_argument( argument, arg );

    if ( arg[0] == '\0' || !str_cmp( arg, ch->name ) )
    {
	act( "$n offers $mself to Mota, who graciously declines.",
	    ch, NULL, NULL, TO_ROOM );
	send_to_char(
	    "Mota appreciates your offer and may accept it later.\n\r", ch );
	return;
    }

    counter = 0;

    if(!str_cmp(arg, "all"))
    {
	for ( obj2 = ch->in_room->contents; obj2 != NULL; obj2 = obj_next )
        {
            obj_next = obj2->next_content;
            if(str_cmp(obj2->name, "all"))
	    do_function(ch, &do_sacrifice, obj2->name);
            counter++;

            if (counter > 20)
            break;
        }
	return;
    }

    obj = get_obj_list( ch, arg, ch->in_room->contents );
    if ( obj == NULL )
    {
	send_to_char( "You can't find it.\n\r", ch );
	return;
    }

    if ( obj->item_type == ITEM_CORPSE_PC )
    {
	if (obj->contains)
        {
	   send_to_char(
	     "Mota wouldn't like that.\n\r",ch);
	   return;
        }
    }


    if ( !CAN_WEAR(obj, ITEM_TAKE) || CAN_WEAR(obj, ITEM_NO_SAC) || IsQuestObjVnum(obj))
    {
	act( "$p is not an acceptable sacrifice.", ch, obj, 0, TO_CHAR );
	return;
    }

    if (obj->in_room != NULL)
    {
	for (gch = obj->in_room->people; gch != NULL; gch = gch->next_in_room)
	    if (gch->on == obj)
	    {
		act("$N appears to be using $p.",
		    ch,obj,gch,TO_CHAR);
		return;
	    }
    }
		
    silver = UMAX(1,obj->level * 3);

    if (obj->item_type != ITEM_CORPSE_NPC && obj->item_type != ITEM_CORPSE_PC)
    	silver = UMIN(silver,obj->cost);

    if (silver == 1)
        send_to_char(
	    "Mota gives you one silver coin for your sacrifice.\n\r", ch );
    else
    {
	sprintf(buf,"Mota gives you %d silver coins for your sacrifice.\n\r",
		silver);
	send_to_char(buf,ch);
    }
    
    ch->silver += silver;
    
    if (IS_SET(ch->pact,PLR_AUTOSPLIT) )
    { /* AUTOSPLIT code */
    	members = 0;
	for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    	{
    	    if ( is_same_group( gch, ch ) )
            members++;
    	}

	if ( members > 1 && silver > 1)
	{
	    sprintf(buffer,"%d",silver);
	    do_function(ch, &do_split, buffer);	
	}
    }

    act( "$n sacrifices $p to Mota.", ch, obj, NULL, TO_ROOM );
    extract_obj( obj );
    return;
}



void do_quaff( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    ROOM_INDEX_DATA *location;
    
    location = NULL;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Quaff what?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_carry( ch, arg, ch ) ) == NULL )
    {
	send_to_char( "You do not have that potion.\n\r", ch );
	return;
    }

    location = get_room_index(ROOM_VNUM_DRAGONPIT);


    if (ch->in_room == location)
    {
	send_to_char( "\n\r{GNot until the DragonPIT Begins.{x\n\r", ch );
	return;
    }


    if ( obj->item_type != ITEM_POTION )
    {
	send_to_char( "You can quaff only potions.\n\r", ch );
	return;
    }

    if (!IS_IMP(ch))
      {
    if (ch->level < obj->level)
    {
	send_to_char("This liquid is too powerful for you to drink.\n\r",ch);
	return;
    }


    /*bobif ( ch->fighting != NULL )
      {   
       send_to_char( "\n\r{WYou can not drink a potion and fight at the same time.\n\r{x", ch );
       return;
      }*/
     }

    act( "$n quaffs $p.", ch, obj, NULL, TO_ROOM );
    act( "You quaff $p.", ch, obj, NULL ,TO_CHAR );

    obj_cast_spell( obj->value[1], obj->value[0], ch, ch, NULL );
    obj_cast_spell( obj->value[2], obj->value[0], ch, ch, NULL );
    obj_cast_spell( obj->value[3], obj->value[0], ch, ch, NULL );

    extract_obj( obj );
    return;
}


void do_recite( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *victim2;
    OBJ_DATA *scroll;
    OBJ_DATA *obj;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( ( scroll = get_obj_carry( ch, arg1, ch ) ) == NULL )
    {
	send_to_char( "You do not have that scroll.\n\r", ch );
	return;
    }

    if ( scroll->item_type != ITEM_SCROLL )
    {
	send_to_char( "You can recite only scrolls.\n\r", ch );
	return;
    }



    if (!IS_IMP(ch))
     {
      if ( ch->level < scroll->level)
        {
	send_to_char("This scroll is too complex for you to comprehend.\n\r",ch);
	return;
        }

    if ( ch->fighting != NULL )
      {   
       send_to_char( "\n\r{WYou are unable to read and fight at the same time.\n\r{x", ch );
       return;
      }

     } 

    obj = NULL;

    if ( arg2[0] == '\0' )
    {
	victim = ch;
    }
    else
    {
	if ( ( victim = get_char_room ( ch, arg2 ) ) == NULL
	&&   ( obj    = get_obj_here  ( ch, arg2 ) ) == NULL )
	{
	    send_to_char( "You can't find it.\n\r", ch );
	    return;
	}
    }

    act( "$n recites $p.", ch, scroll, NULL, TO_ROOM );
    act( "You recite $p.", ch, scroll, NULL, TO_CHAR );

    if (number_percent() >= 20 + get_skill(ch,gsn_scrolls) * 4/5)
    {
	send_to_char("You mispronounce a syllable.\n\r",ch);
	check_improve(ch,gsn_scrolls,FALSE,2);
    }
    else
       if((skill_table[scroll->value[1]].target) == TAR_CHAR_SELF)
        {
         victim2 = ch;
         obj_cast_spell( scroll->value[1], scroll->value[0], ch, victim2, obj );
   
       obj_cast_spell( scroll->value[2], scroll->value[0], ch, victim, obj );
       obj_cast_spell( scroll->value[3], scroll->value[0], ch, victim, obj );
       check_improve(ch,gsn_scrolls,TRUE,2);
        }
   else     
       if((skill_table[scroll->value[2]].target) == TAR_CHAR_SELF)
        {
         victim2 = ch;
         obj_cast_spell( scroll->value[2], scroll->value[0], ch, victim2, obj );
   
       obj_cast_spell( scroll->value[1], scroll->value[0], ch, victim, obj );
       obj_cast_spell( scroll->value[3], scroll->value[0], ch, victim, obj );
       check_improve(ch,gsn_scrolls,TRUE,2);
        }
   else     
       if((skill_table[scroll->value[3]].target) == TAR_CHAR_SELF)
        {
         victim2 = ch;
         obj_cast_spell( scroll->value[3], scroll->value[0], ch, victim2, obj );
   
       obj_cast_spell( scroll->value[1], scroll->value[0], ch, victim, obj );
       obj_cast_spell( scroll->value[2], scroll->value[0], ch, victim, obj );
       check_improve(ch,gsn_scrolls,TRUE,2);
        }
   else     
     {
       obj_cast_spell( scroll->value[1], scroll->value[0], ch, victim, obj );
       obj_cast_spell( scroll->value[2], scroll->value[0], ch, victim, obj );
       obj_cast_spell( scroll->value[3], scroll->value[0], ch, victim, obj );
       check_improve(ch,gsn_scrolls,TRUE,2);
      }

    extract_obj( scroll );
    return;
}


void do_brandish( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    OBJ_DATA *staff;
    int sn;

    if ( ( staff = get_eq_char( ch, WEAR_HOLD ) ) == NULL )
    {
	send_to_char( "You hold nothing in your hand.\n\r", ch );
	return;
    }

    if ( staff->item_type != ITEM_STAFF )
    {
	send_to_char( "You can brandish only with a staff.\n\r", ch );
	return;
    }

    if ( ( sn = staff->value[3] ) < 0
    ||   sn >= MAX_SKILL
    ||   skill_table[sn].spell_fun == 0 )
    {
	bug( "Do_brandish: bad sn %d.", sn );
	return;
    }

    WAIT_STATE( ch, 2 * PULSE_VIOLENCE );

    if ( staff->value[2] > 0 )
    {
	act( "$n brandishes $p.", ch, staff, NULL, TO_ROOM );
	act( "You brandish $p.",  ch, staff, NULL, TO_CHAR );
	if ( ch->level < staff->level 
	||   number_percent() >= 20 + get_skill(ch,gsn_staves) * 4/5)
 	{
	    act ("You fail to invoke $p.",ch,staff,NULL,TO_CHAR);
	    act ("...and nothing happens.",ch,NULL,NULL,TO_ROOM);
	    check_improve(ch,gsn_staves,FALSE,2);
	}
	
	else for ( vch = ch->in_room->people; vch; vch = vch_next )
	{
	    vch_next	= vch->next_in_room;

	    switch ( skill_table[sn].target )
	    {
	    default:
		bug( "Do_brandish: bad target for sn %d.", sn );
		return;

	    case TAR_IGNORE:
		if ( vch != ch )
		    continue;
		break;

	    case TAR_CHAR_OFFENSIVE:
		if ( IS_NPC(ch) ? IS_NPC(vch) : !IS_NPC(vch) )
		    continue;
		break;
		
	    case TAR_CHAR_DEFENSIVE:
		if ( IS_NPC(ch) ? !IS_NPC(vch) : IS_NPC(vch) )
		    continue;
		break;

	    case TAR_CHAR_SELF:
		if ( vch != ch )
		    continue;
		break;
	    }

	    obj_cast_spell( staff->value[3], staff->value[0], ch, vch, NULL );
	    check_improve(ch,gsn_staves,TRUE,2);
	}
    }

    if ( --staff->value[2] <= 0 )
    {
	act( "$n's $p blazes bright and is gone.", ch, staff, NULL, TO_ROOM );
	act( "Your $p blazes bright and is gone.", ch, staff, NULL, TO_CHAR );
	extract_obj( staff );
    }

    return;
}



void do_zap( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *wand;
    OBJ_DATA *obj;
 
   one_argument( argument, arg );
    if ( arg[0] == '\0' && ch->fighting == NULL )
    {
	send_to_char( "Zap whom or what?\n\r", ch );
	return;
    }

    if ( ( wand = get_eq_char( ch, WEAR_HOLD ) ) == NULL )
    {
	send_to_char( "You hold nothing in your hand.\n\r", ch );
	return;
    }

    if ( wand->item_type != ITEM_WAND )
    {
	send_to_char( "You can zap only with a wand.\n\r", ch );
	return;
    }

    obj = NULL;
    if ( arg[0] == '\0' )
    {
	if ( ch->fighting != NULL )
	{
	    victim = ch->fighting;
	}
	else
	{
	    send_to_char( "Zap whom or what?\n\r", ch );
	    return;
	}
    }
    else
    {
	if ( ( victim = get_char_room ( ch, arg ) ) == NULL
	&&   ( obj    = get_obj_here  ( ch, arg ) ) == NULL )
	{
	    send_to_char( "You can't find it.\n\r", ch );
	    return;
	}
    }

    WAIT_STATE( ch, 2 * PULSE_VIOLENCE );

    if ( wand->value[2] > 0 )
    {
	if ( victim != NULL )
	{
	    act( "$n zaps $N with $p.", ch, wand, victim, TO_NOTVICT );
	    act( "You zap $N with $p.", ch, wand, victim, TO_CHAR );
	    act( "$n zaps you with $p.",ch, wand, victim, TO_VICT );
	}
	else
	{
	    act( "$n zaps $P with $p.", ch, wand, obj, TO_ROOM );
	    act( "You zap $P with $p.", ch, wand, obj, TO_CHAR );
	}

 	if (ch->level < wand->level
	||  number_percent() >= 20 + get_skill(ch,gsn_wands) * 4/5) 
	{
	    act( "Your efforts with $p produce only smoke and sparks.",
		 ch,wand,NULL,TO_CHAR);
	    act( "$n's efforts with $p produce only smoke and sparks.",
		 ch,wand,NULL,TO_ROOM);
	    check_improve(ch,gsn_wands,FALSE,2);
	}
	else
	{
	    obj_cast_spell( wand->value[3], wand->value[0], ch, victim, obj );
	    check_improve(ch,gsn_wands,TRUE,2);
	}
    }

    if ( --wand->value[2] <= 0 )
    {
	act( "$n's $p explodes into fragments.", ch, wand, NULL, TO_ROOM );
	act( "Your $p explodes into fragments.", ch, wand, NULL, TO_CHAR );
	extract_obj( wand );
    }

    return;
}



void do_steal( CHAR_DATA *ch, char *argument )
{
    char buf  [MAX_STRING_LENGTH];
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    int percent, perbase;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "\n\r{RSteal what from whom?{x\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg2 ) ) == NULL )
    {
	send_to_char( "\n\r{RThey aren't here.{x\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "\n\r{GThat's pointless.{x\n\r", ch );
	return;
    }

    if ((IS_SET(victim->in_room->room_flags, ROOM_SAFE))
    ||  (IS_SET(victim->in_room->room_flags, ROOM_DRAGONPIT)) )
      {
       send_to_char("\n\r{CNot in this room!{x\n\r",ch);
       return;
      }

    if ( victim->position == POS_FIGHTING)
    {
	send_to_char(  "\n\r{RYou'd better not {r-- {RYou might get hit{r.{x\n\r",ch);
	return;
    }


      if((ch->level + 10 < victim->level) 
      || (ch->level - 5 > victim->level)) 
        {
         send_to_char("\n\r{RYou don't think that would be a good idea{r.{x\n\r",ch);
         return;
        }


   if(!IS_NPC(victim))
     {
      if(!IS_SET(ch->pact, PLR_PKILLER)) 
        {
         send_to_char(
"\n\r{rYou must be a {WPKILLER {rto steal from other {RPLAYERs{r.{x\n\r",ch);
         return;
        }

      if(!IS_SET(victim->pact, PLR_PKILLER)) 
        {
         send_to_char(
"\n\r{rYour victim must be a {WPKILLER{r if you are to {RSTEAL {rfrom them.{x\n\r",ch);
         return;
        }
    }

    WAIT_STATE( ch, skill_table[gsn_steal].beats );
    perbase  = number_percent();

    percent = perbase;

    if (!IS_AWAKE(victim))
    	percent -= 25;
     else    
    if (!can_see(victim,ch))
    	percent -= 25;
      else
        percent += 15;

   if(IS_SET(ch->pact, PLR_THIEF))
     percent += 50;


    if(!IS_CLASS(ch, CLASS_THIEF))
      {
       percent += 150;
      }


   if (ch->level >= victim->level)
      percent -= ((ch->level - victim->level) * 3);
   else
      percent += ((victim->level - ch->level) * 3);


   if(!IS_NPC(victim))
     {
     if (percent > get_skill(ch,gsn_steal))
      {
	send_to_char( "\n\r{RBUSTED{r!!!{x\n\r", ch );

           sprintf(buf,"{GYour chance {D({WBASE{c/{CMODIFIED{D) {Gwas{w: {D({W%d{c/{C%d{D){x\n\r",perbase,percent); 
           send_to_char(buf,ch);

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
        do_visible(ch," ");
        act( "\n\r{W$n {Gfades into existence{g!{x\n\r", ch, NULL, NULL, TO_ROOM );
    }

	act( "\n\r{W$n {Rtried to steal from you!{x\n\r", ch, NULL, victim, TO_VICT);
	act( "\n\r{W$n {rtried to steal from {R$N{r!{x\n\r",  ch, NULL, victim,TO_NOTVICT);

	switch(number_range(0,3))
	{
	case 0 :
	   sprintf( buf, "%s is a lousy thief!", ch->name );
	   break;
        case 1 :
	   sprintf( buf, "%s couldn't rob %s way out of a paper bag!",
		    ch->name,(ch->sex == 2) ? "her" : "his");
	   break;
	case 2 :
	    sprintf( buf,"%s tried to rob me!",ch->name );
	    break;
	case 3 :
	    sprintf(buf,"Keep your hands out of there, %s!",ch->name);
	    break;
        }

        if (!IS_AWAKE(victim))
            do_function(victim, &do_wake, "");
         else
	    do_function(victim, &do_yell, buf );


	if (!IS_NPC(ch) )
	{
	 if (!IS_SET(ch->pact, PLR_THIEF) )
	   {
	    SET_BIT(ch->pact, PLR_THIEF);
	    send_to_char( "\n\r{r*** {RYou are now a {WTHIEF{R!! {r***{x\n\r", ch );
            save_char_obj( ch );
           }
        }

        check_improve(ch,gsn_steal,FALSE,2);
	return;
     }
    }
  else
     {
     if (percent > get_skill(ch,gsn_steal))
      {
	send_to_char( "\n\r{RBUSTED{r!!!{x\n\r", ch );

           sprintf(buf,"{GYour chance {D({WBASE{c/{CMODIFIED{D) {Gwas{w: {D({W%d{c/{C%d{D){x\n\r",perbase,percent);
           send_to_char(buf,ch);           

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
        do_visible(ch," ");
        act( "\n\r{W$n {Gfades into existence{g!{x\n\r", ch, NULL, NULL, TO_ROOM );
    }

	act( "\n\r{W$n {rtried to steal from {R$N{r!{x\n\r",  ch, NULL, victim,TO_NOTVICT);

	switch(number_range(0,3))
	{
	case 0 :
	   sprintf( buf, "%s is a lousy thief!", ch->name );
	   break;
        case 1 :
	   sprintf( buf, "%s couldn't rob %s way out of a paper bag!",
		    ch->name,(ch->sex == 2) ? "her" : "his");
	   break;
	case 2 :
	    sprintf( buf,"%s tried to rob me!",ch->name );
	    break;
	case 3 :
	    sprintf(buf,"Keep your hands out of there, %s!",ch->name);
	    break;
        }

        if (!IS_AWAKE(victim))
            do_function(victim, &do_wake, "");
         else
	    do_function(victim, &do_yell, buf );


	if (!IS_NPC(ch) )
	{
	 if (!IS_SET(ch->pact, PLR_THIEF) )
	   {
	    SET_BIT(ch->pact, PLR_THIEF);
	    send_to_char( "\n\r{r*** {RYou are now a {WTHIEF{R!! {r***{x\n\r", ch );
            save_char_obj( ch );
           }
        }

        check_improve(ch,gsn_steal,FALSE,2);
        multi_hit( victim, ch, TYPE_UNDEFINED );
	return;
      }
   }


    if ( !str_cmp( arg1, "gold" )
    ||	 !str_cmp( arg1, "silver"))
    {
	int gold, silver;

	if ((!str_cmp(arg1, "gold") 
	&& victim->gold <= 0 )
	|| ( !str_cmp(arg1, "silver")
        && victim->silver <= 0 ))
	  {
	   send_to_char( "\n\r{RYou couldn't get any coins!{x\n\r", ch );
	   return;
	  }
        else
          {
	if ((!str_cmp(arg1, "gold") 
	&& victim->gold >= 1 )
	|| ( !str_cmp(arg1, "silver")
        && victim->silver >= 1 ))
          {

   	   if (!str_cmp(arg1, "gold")) 
             {
	      gold = number_range(1, victim->gold);

     	      ch->gold     	+= gold;
	      victim->gold 	-= gold;
  	      sprintf( buf, "\n\r{CBingo!  You got %d gold coins.{x\n\r", gold );
             }

   	   if (!str_cmp(arg1, "silver")) 
             {
    	      silver = number_range(1,victim->silver);

   	      ch->silver   	+= silver;
	      victim->silver 	-= silver;
  	      sprintf( buf, "\n\r{CBingo!  You got %d silver coins.{x\n\r",silver);
             }
           }
         }

	send_to_char( buf, ch );

           sprintf(buf,"{GYour chance {D({WBASE{c/{CMODIFIED{D) {Gwas{w: {D({W%d{c/{C%d{D){x\n\r",perbase,percent);
           send_to_char(buf,ch);           

	check_improve(ch,gsn_steal,TRUE,2);
	return;
      }


    if ( ( obj = get_obj_carry( victim, arg1, ch ) ) == NULL )
      {
       send_to_char( "\n\r{RYou can't find it!{x\n\r", ch );
       return;
      }

    if (get_obj_weight( obj ) > 30)
      {
       send_to_char(
"\n\r{RThat item is way to heavy for you to steal and not be noticed!{x\n\r", ch );
       return;
      }
	
    if ( !can_drop_obj( victim, obj )
    ||   IS_SET(obj->extra_flags, ITEM_INVENTORY)
    ||   obj->level > ch->level )
      {
       send_to_char( "\n\r{RYou can't pry it away!{x\n\r", ch );
       return;
      }

    if ( ch->carry_number + get_obj_number( obj ) > can_carry_n( ch ) )
      {
       send_to_char( "\n\r{RYou have your hands full!{x\n\r", ch );
       return;
      }

    if ( ch->carry_weight + get_obj_weight( obj ) > can_carry_w( ch ) )
      {
       send_to_char( "\n\r{RYou can't carry that much weight!{x\n\r", ch );
       return;
      }

    if (same_obj(obj, ch->carrying) >=30)
      {
       send_to_char("\n\r{RYou already have enough!{x\n\r", ch);
       return;
      }

    obj_from_char( obj );
    obj_to_char( obj, ch );
    act("\n\r{CYou pocket $p!{x\n\r",ch,obj,NULL,TO_CHAR);

           sprintf(buf,"{GYour chance {D({WBASE{c/{CMODIFIED{D) {Gwas{w: {D({W%d{c/{C%d{D){x\n\r",perbase,percent);
           send_to_char(buf,ch);           

    check_improve(ch,gsn_steal,TRUE,2);
    return;
}



/*
 * Shopping commands.
 */
CHAR_DATA *find_keeper( CHAR_DATA *ch )
{
    /*char buf[MAX_STRING_LENGTH];*/
    CHAR_DATA *keeper;
    SHOP_DATA *pShop;

    pShop = NULL;
    for ( keeper = ch->in_room->people; keeper; keeper = keeper->next_in_room )
    {
	if ( IS_NPC(keeper) && (pShop = keeper->pIndexData->pShop) != NULL )
	    break;
    }

    if ( pShop == NULL )
    {
	send_to_char( "You can't do that here.\n\r", ch );
	return NULL;
    }

    /*
     * Undesirables.
     *
    if ( !IS_NPC(ch) && IS_SET(ch->pact, PLR_THIEF) )
    {
	do_function(keeper, &do_say, "Thieves are not welcome!");
	sprintf(buf, "%s the THIEF is over here!\n\r", ch->name);
	do_function(keeper, &do_yell, buf ); 
	return NULL;
    }
	*/
    /*
     * Shop hours.
     */
    if ( time_info.hour < pShop->open_hour )
    {
	do_function(keeper, &do_say, "Sorry, I am closed. Come back later.");
	return NULL;
    }
    
    if ( time_info.hour > pShop->close_hour )
    {
	do_function(keeper, &do_say, "Sorry, I am closed. Come back tomorrow.");
	return NULL;
    }

    /*
     * Invisible or hidden people.
     */
    if ( !can_see( keeper, ch ) )
    {
	do_function(keeper, &do_say, "I don't trade with folks I can't see.");
	return NULL;
    }

    return keeper;
}

/* insert an object at the right spot for the keeper */
void obj_to_keeper( OBJ_DATA *obj, CHAR_DATA *ch )
{
    OBJ_DATA *t_obj, *t_obj_next;

    /* see if any duplicates are found */
    for (t_obj = ch->carrying; t_obj != NULL; t_obj = t_obj_next)
    {
	t_obj_next = t_obj->next_content;

	if (obj->pIndexData == t_obj->pIndexData 
	&&  !str_cmp(obj->short_descr,t_obj->short_descr))
	{
	    /* if this is an unlimited item, destroy the new one */
	    if (IS_OBJ_STAT(t_obj,ITEM_INVENTORY))
	    {
		extract_obj(obj);
		return;
	    }
	    obj->cost = t_obj->cost; /* keep it standard */
	    break;
	}
    }

    if (t_obj == NULL)
    {
	obj->next_content = ch->carrying;
	ch->carrying = obj;
    }
    else
    {
	obj->next_content = t_obj->next_content;
	t_obj->next_content = obj;
    }

    obj->carried_by      = ch;
    obj->in_room         = NULL;
    obj->in_obj          = NULL;
    ch->carry_number    += get_obj_number( obj );
    ch->carry_weight    += get_obj_weight( obj );
}

/* get an object from a shopkeeper's list */
OBJ_DATA *get_obj_keeper( CHAR_DATA *ch, CHAR_DATA *keeper, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int number;
    int count;
 
    number = number_argument( argument, arg );
    count  = 0;
    for ( obj = keeper->carrying; obj != NULL; obj = obj->next_content )
    {
        if (obj->wear_loc == WEAR_INVENTORY
        &&  can_see_obj( keeper, obj )
	&&  can_see_obj(ch,obj)
        &&  is_name( arg, obj->name ) )
        {
            if ( ++count == number )
                return obj;
	
	    /* skip other objects of the same name */
	    while (obj->next_content != NULL
	    && obj->pIndexData == obj->next_content->pIndexData
	    && !str_cmp(obj->short_descr,obj->next_content->short_descr))
		obj = obj->next_content;
        }
    }
 
    return NULL;
}

int get_cost( CHAR_DATA *keeper, OBJ_DATA *obj, bool fBuy )
{
    SHOP_DATA *pShop;
    int cost;

    if ( obj == NULL || ( pShop = keeper->pIndexData->pShop ) == NULL )
	return 0;

    if ( fBuy )
    {
	cost = obj->cost * pShop->profit_buy  / 100;
    }
    else
    {
	OBJ_DATA *obj2;
	int itype;

	cost = 0;
	for ( itype = 0; itype < MAX_TRADE; itype++ )
	{
	    if ( obj->item_type == pShop->buy_type[itype] )
	    {
		cost = obj->cost * pShop->profit_sell / 100;
		break;
	    }
	}

	if (!IS_OBJ_STAT(obj,ITEM_SELL_EXTRACT))
	    for ( obj2 = keeper->carrying; obj2; obj2 = obj2->next_content )
	    {
	    	if ( obj->pIndexData == obj2->pIndexData  &&   !str_cmp(obj->short_descr,obj2->short_descr) )
			{
	 			if (IS_OBJ_STAT(obj2,ITEM_INVENTORY))
					cost /= 2;
			    else
                   	cost = cost * 3 / 4;
			}
	    }
    }

    if ( obj->item_type == ITEM_STAFF || obj->item_type == ITEM_WAND )
    {
	if (obj->value[1] == 0)
	    cost /= 4;
	else
	    cost = cost * obj->value[2] / obj->value[1];
    }

    return cost;
}



void do_buy( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    int cost,roll;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Buy what?\n\r", ch );
	return;
    }

    if ( IS_SET(ch->in_room->room_flags, ROOM_PET_SHOP) )
    {
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *pet;
	ROOM_INDEX_DATA *pRoomIndexNext;
	ROOM_INDEX_DATA *in_room;

	smash_tilde(argument);

	if ( IS_NPC(ch) )
	    return;

	argument = one_argument(argument,arg);

	/* hack to make new thalos pets work */
	if (ch->in_room->vnum == 9621)
	    pRoomIndexNext = get_room_index(9706);
	else
	    pRoomIndexNext = get_room_index( ch->in_room->vnum + 1 );
	if ( pRoomIndexNext == NULL )
	{
	    bug( "Do_buy: bad pet shop at vnum %d.", ch->in_room->vnum );
	    send_to_char( "Sorry, you can't buy that here.\n\r", ch );
	    return;
	}

	in_room     = ch->in_room;
	ch->in_room = pRoomIndexNext;
	pet         = get_char_room( ch, arg );
	ch->in_room = in_room;

	if ( pet == NULL || !IS_SET(pet->act, ACT_PET) )
	{
	    send_to_char( "Sorry, you can't buy that here.\n\r", ch );
	    return;
	}

	if ( ch->pet != NULL )
	{
	    send_to_char("You already own a pet.\n\r",ch);
	    return;
	}

 	cost = 10 * pet->level * pet->level;

	if ( (ch->silver + 100 * ch->gold) < cost )
	{
	    send_to_char( "You can't afford it.\n\r", ch );
	    return;
	}

	if ( ch->level < pet->level )
	{
	    send_to_char(
		"You're not powerful enough to master this pet.\n\r", ch );
	    return;
	}

	/* haggle */
	roll = number_percent();
	if (roll < get_skill(ch,gsn_haggle))
	{
	    cost -= cost / 2 * roll / 100;
	    sprintf(buf,"You haggle the price down to %d coins.\n\r",cost);
	    send_to_char(buf,ch);
	    check_improve(ch,gsn_haggle,TRUE,4);
	
	}

	deduct_cost(ch,cost);
	pet			= create_mobile( pet->pIndexData );
	SET_BIT(pet->act, ACT_PET);
	SET_BIT(pet->affected_by, AFF_CHARM);
	pet->comm = COMM_NOCHANNELS;

	argument = one_argument( argument, arg );
	if ( arg[0] != '\0' )
	{
	    sprintf( buf, "%s %s", pet->name, arg );
	    free_string( pet->name );
	    pet->name = str_dup( buf );
	}

	sprintf( buf, "%sA neck tag says 'I belong to %s'.\n\r",
	    pet->description, ch->name );
	free_string( pet->description );
	pet->description = str_dup( buf );

	char_to_room( pet, ch->in_room );
	add_follower( pet, ch );
	pet->leader = ch;
	ch->pet = pet;
	send_to_char( "Enjoy your pet.\n\r", ch );
	act( "$n bought $N as a pet.", ch, NULL, pet, TO_ROOM );
	return;
    }
    else
    {
	CHAR_DATA *keeper;
	OBJ_DATA *obj,*t_obj;
	char arg[MAX_INPUT_LENGTH];
	int number, count = 1;

	if ( ( keeper = find_keeper( ch ) ) == NULL )
	    return;

	number = mult_argument(argument,arg);
	obj  = get_obj_keeper( ch,keeper, arg );
	cost = get_cost( keeper, obj, TRUE );

	if (number < 1 || number > 30)
	{
	    act("$n tells you 'Get real!",keeper,NULL,ch,TO_VICT);
	    return;
	}

	if ( cost <= 0 || !can_see_obj( ch, obj ) )
	{
	    act( "$n tells you 'I don't sell that -- try 'list''.",
		keeper, NULL, ch, TO_VICT );
	    ch->reply = keeper;
	    return;
	}

	if (!IS_OBJ_STAT(obj,ITEM_INVENTORY))
	{
	    for (t_obj = obj->next_content;
	     	 count < number && t_obj != NULL; 
	     	 t_obj = t_obj->next_content) 
	    {
	    	if (t_obj->pIndexData == obj->pIndexData
	    	&&  !str_cmp(t_obj->short_descr,obj->short_descr))
		    count++;
	    	else
		    break;
	    }

	    if (count < number)
	    {
	    	act("$n tells you 'I don't have that many in stock.",
		    keeper,NULL,ch,TO_VICT);
	    	ch->reply = keeper;
	    	return;
	    }
	}

	if ( (ch->silver + ch->gold * 100) < cost * number )
	{
	    if (number > 1)
		act("$n tells you 'You can't afford to buy that many.",
		    keeper,obj,ch,TO_VICT);
	    else
	    	act( "$n tells you 'You can't afford to buy $p'.",
		    keeper, obj, ch, TO_VICT );
	    ch->reply = keeper;
	    return;
	}
	
	if ( obj->level > ch->level )
	{
	    act( "$n tells you 'You can't use $p yet'.",
		keeper, obj, ch, TO_VICT );
	    ch->reply = keeper;
	    return;
	}

	if (ch->carry_number +  number * get_obj_number(obj) > can_carry_n(ch))
	{
	    send_to_char( "You can't carry that many items.\n\r", ch );
	    return;
	}

	if ( ch->carry_weight + number * get_obj_weight(obj) > can_carry_w(ch))
	{
	    send_to_char( "You can't carry that much weight.\n\r", ch );
	    return;
	}

	if(same_obj(obj, ch->carrying) >=30)
	{
	    send_to_char("You already have enough.\n\r", ch);
	    return;
	}

	/* haggle */
	roll = number_percent();
	if (!IS_OBJ_STAT(obj,ITEM_SELL_EXTRACT) 
	&& roll < get_skill(ch,gsn_haggle))
	{
	    cost -= obj->cost / 2 * roll / 100;
	    act("You haggle with $N.",ch,NULL,keeper,TO_CHAR);
	    check_improve(ch,gsn_haggle,TRUE,4);
	}

	if (number > 1)
	{
	    sprintf(buf,"$n buys $p[%d].",number);
	    act(buf,ch,obj,NULL,TO_ROOM);
	    sprintf(buf,"You buy $p[%d] for %d silver.",number,cost * number);
	    act(buf,ch,obj,NULL,TO_CHAR);
	}
	else
	{
	    act( "$n buys $p.", ch, obj, NULL, TO_ROOM );
	    sprintf(buf,"You buy $p for %d silver.",cost);
	    act( buf, ch, obj, NULL, TO_CHAR );
	}
	deduct_cost(ch,cost * number);
	keeper->gold += cost * number/100;
	keeper->silver += cost * number - (cost * number/100) * 100;

	for (count = 0; count < number; count++)
	{
	    if ( IS_SET( obj->extra_flags, ITEM_INVENTORY ) )
	    	t_obj = create_object( obj->pIndexData, obj->level );
	    else
	    {
		t_obj = obj;
		obj = obj->next_content;
	    	obj_from_char( t_obj );
	    }

	    if (t_obj->timer > 0 && !IS_OBJ_STAT(t_obj,ITEM_HAD_TIMER))
	    	t_obj->timer = 0;
	    REMOVE_BIT(t_obj->extra_flags,ITEM_HAD_TIMER);
	    obj_to_char( t_obj, ch );
	    if (cost < t_obj->cost)
	    	t_obj->cost = cost;
	}
    }
}



void do_list( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    if ( IS_SET(ch->in_room->room_flags, ROOM_PET_SHOP) )
    {
	ROOM_INDEX_DATA *pRoomIndexNext;
	CHAR_DATA *pet;
	bool found;

        /* hack to make new thalos pets work */
        if (ch->in_room->vnum == 9621)
            pRoomIndexNext = get_room_index(9706);
        else
            pRoomIndexNext = get_room_index( ch->in_room->vnum + 1 );

	if ( pRoomIndexNext == NULL )
	{
	    bug( "Do_list: bad pet shop at vnum %d.", ch->in_room->vnum );
	    send_to_char( "You can't do that here.\n\r", ch );
	    return;
	}

	found = FALSE;
	for ( pet = pRoomIndexNext->people; pet; pet = pet->next_in_room )
	{
	    if ( IS_SET(pet->act, ACT_PET) )
	    {
		if ( !found )
		{
		    found = TRUE;
		    send_to_char( "Pets for sale:\n\r", ch );
		}
		sprintf( buf, "[%2d] %8d - %s\n\r",
		    pet->level,
		    10 * pet->level * pet->level,
		    pet->short_descr );
		send_to_char( buf, ch );
	    }
	}
	if ( !found )
	    send_to_char( "Sorry, we're out of pets right now.\n\r", ch );
	return;
    }
    else
    {
	CHAR_DATA *keeper;
	OBJ_DATA *obj;
	int cost,count;
	bool found;
	char arg[MAX_INPUT_LENGTH];

	if ( ( keeper = find_keeper( ch ) ) == NULL )
	    return;
        one_argument(argument,arg);

	found = FALSE;
	for ( obj = keeper->carrying; obj; obj = obj->next_content )
	{
	    if ( obj->wear_loc == WEAR_INVENTORY
	    &&   can_see_obj( ch, obj )
	    &&   ( cost = get_cost( keeper, obj, TRUE ) ) > 0 
	    &&   ( arg[0] == '\0'  
 	       ||  is_name(arg,obj->name) ))
	    {
		if ( !found )
		{
		    found = TRUE;
		    send_to_char( "[Lv Price Qty] Item\n\r", ch );
		}

		if (IS_OBJ_STAT(obj,ITEM_INVENTORY))
		    sprintf(buf,"[%2d %5d -- ] %s\n\r",
			obj->level,cost,obj->short_descr);
		else
		{
		    count = 1;

		    while (obj->next_content != NULL 
		    && obj->pIndexData == obj->next_content->pIndexData
		    && !str_cmp(obj->short_descr,
			        obj->next_content->short_descr))
		    {
			obj = obj->next_content;
			count++;
		    }
		    sprintf(buf,"[%2d %5d %2d ] %s\n\r",
			obj->level,cost,count,obj->short_descr);
		}
		send_to_char( buf, ch );
	    }
	}

	if ( !found )
	    send_to_char( "You can't buy anything here.\n\r", ch );
	return;
    }
}



void do_sell( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *keeper;
    OBJ_DATA *obj;
    int cost,roll;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Sell what?\n\r", ch );
	return;
    }

    if ( ( keeper = find_keeper( ch ) ) == NULL )
	return;

    if ( ( obj = get_obj_carry( ch, arg, ch ) ) == NULL )
    {
	act( "$n tells you 'You don't have that item'.",
	    keeper, NULL, ch, TO_VICT );
	ch->reply = keeper;
	return;
    }

    if ( !can_drop_obj( ch, obj ) )
    {
	send_to_char( "You can't let go of it.\n\r", ch );
	return;
    }

    if (!can_see_obj(keeper,obj))
    {
	act("$n doesn't see what you are offering.",keeper,NULL,ch,TO_VICT);
	return;
    }

    if ( IS_SET( obj->pIndexData->area->area_flags, AREA_PROTO ))
    {
	act("\n\r{W$n {cdoesn't want {CPROTO{c Items!{x\n\r",keeper,NULL,ch,TO_VICT);
	return;
    }

    if ( ( cost = get_cost( keeper, obj, FALSE ) ) <= 0 )
    {
	act( "$n looks uninterested in $p.", keeper, obj, ch, TO_VICT );
	return;
    }
    if ( cost > (keeper-> silver + 100 * keeper->gold) )
    {
	act("$n tells you 'I'm afraid I don't have enough wealth to buy $p.",
	    keeper,obj,ch,TO_VICT);
	return;
    }

    act( "$n sells $p.", ch, obj, NULL, TO_ROOM );
    /* haggle */
    roll = number_percent();
    if (!IS_OBJ_STAT(obj,ITEM_SELL_EXTRACT) && roll < get_skill(ch,gsn_haggle))
    {
        send_to_char("You haggle with the shopkeeper.\n\r",ch);
        cost += obj->cost / 2 * roll / 100;
        cost = UMIN(cost,95 * get_cost(keeper,obj,TRUE) / 100);
	cost = UMIN(cost,(keeper->silver + 100 * keeper->gold));
        check_improve(ch,gsn_haggle,TRUE,4);
    }
    sprintf( buf, "You sell $p for %d silver and %d gold piece%s.",
	cost - (cost/100) * 100, cost/100, cost == 1 ? "" : "s" );
    act( buf, ch, obj, NULL, TO_CHAR );
    ch->gold     += cost/100;
    ch->silver 	 += cost - (cost/100) * 100;
    deduct_cost(keeper,cost);
    if ( keeper->gold < 0 )
	keeper->gold = 0;
    if ( keeper->silver< 0)
	keeper->silver = 0;

    if ( obj->item_type == ITEM_TRASH || IS_OBJ_STAT(obj,ITEM_SELL_EXTRACT))
    {
	extract_obj( obj );
    }
    else
    {
	obj_from_char( obj );
	if (obj->timer)
	    SET_BIT(obj->extra_flags,ITEM_HAD_TIMER);
	else
	    obj->timer = number_range(50,100);
	obj_to_keeper( obj, keeper );
    }

    return;
}


/*
void do_value( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *keeper;
    OBJ_DATA *obj;
    int cost;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Value what?\n\r", ch );
	return;
    }

    if ( ( keeper = find_keeper( ch ) ) == NULL )
	return;

    if ( ( obj = get_obj_carry( ch, arg, ch ) ) == NULL )
    {
	act( "$n tells you 'You don't have that item'.",
	    keeper, NULL, ch, TO_VICT );
	ch->reply = keeper;
	return;
    }

    if (!can_see_obj(keeper,obj))
    {
        act("$n doesn't see what you are offering.",keeper,NULL,ch,TO_VICT);
        return;
    }

    if ( !can_drop_obj( ch, obj ) )
    {
	send_to_char( "You can't let go of it.\n\r", ch );
	return;
    }

    if ( ( cost = get_cost( keeper, obj, FALSE ) ) <= 0 )
    {
	act( "$n looks uninterested in $p.", keeper, obj, ch, TO_VICT );
	return;
    }

    sprintf( buf, 
	"$n tells you 'I'll give you %d silver and %d gold coins for $p'.", 
	cost - (cost/100) * 100, cost/100 );
    act( buf, keeper, obj, ch, TO_VICT );
    ch->reply = keeper;

    return;
}
*/


void do_second (CHAR_DATA *ch, char *argument)
/* wear object as a secondary weapon */
{
    OBJ_DATA *obj;
    OBJ_DATA *wield;
    OBJ_DATA *weapon;
    int obj_weight = 0;
    int str_weight = 0;
    char buf[MAX_STRING_LENGTH];


 if ( !IS_NPC(ch)
    &&   !can_use_skpell( ch, gsn_dualwield ) )
      {
      send_to_char( "\n\r{cYou do not know how to dual wield.{x\n\r", ch );
      return;
      }

    if (argument[0] == '\0')
    {
        send_to_char ("\n\r{cWear which weapon in your off-hand?{x\n\r",ch);
        return;
    }

    obj = get_obj_carry (ch, argument, ch);

    if (!obj)
    {
        send_to_char ("\n\r{cYou have no such thing in your backpack.{x\n\r",ch);
        return;
    }

    if ( obj->item_type != ITEM_WEAPON )
      {
      send_to_char( "\n\r{cThat's not a weapon.{x\n\r", ch );
      return;
      }
    /* check if the char is using a shield or a held weapon */
    if ( !(weapon = get_eq_char(ch,WEAR_WIELD)) )
      {
      send_to_char( "You must wield a weapon to dual wield another.\n\r", ch );
      return;
      }

    if (get_eq_char (ch,WEAR_SHIELD) != NULL) 
   {
send_to_char ("\n\r{CYou are wearing a shield, try removing it to free up your off-hand!{x\n\r",ch);
        return;
    }



    if ( get_eq_char (ch,WEAR_HOLD)   != NULL) 
   {
send_to_char ("\n\r{CYou are holding something, try removing it to free up your off-hand!{x\n\r",ch);
        return;
    }

    if ( IS_WEAPON_STAT(weapon,WEAPON_TWO_HANDS)
    && ch->size < SIZE_GIANT)
   {
send_to_char ("\n\r{CYou are not big enough to Dual wield while using a Two-Handed weapon!{x\n\r",ch);
        return;
    }

    if ( obj->level > ch->level )
    {
        sprintf( buf, "\n\r{gYou must be level {W%d {gto use this object.{x\n\r",
            obj->level );
        send_to_char( buf, ch );
        act( "\n\r{C$n tries to use $p, but is too inexperienced.{x\n\r",
            ch, obj, NULL, TO_ROOM );
        return;
    }

    if ( !(wield = get_eq_char( ch, WEAR_WIELD )) ) 
    {
        send_to_char ("\n\r{RYou need to wield a primary weapon, before using a secondary one!{x\n\r",ch);
        return;
    }

    obj_weight = get_obj_weight( obj );
    str_weight = str_app[get_curr_stat( ch, STAT_STR )].wield;
    str_weight *= 5;

    if ( obj_weight > str_weight)
    {
        send_to_char ("\n\r{WYou are not strong enough to dual this weapon.{x\n\r",ch);
        return;
    }


/* check if the secondary weapon is at least 
   half as light as the primary weapon 
   New: Daggers & Stuff under 5 pounds don't count. */



   if (!IS_CLASS(ch, CLASS_WARRIOR) && get_obj_weight( wield )<=obj_weight)
   {
        send_to_char("\n\n{WSecondary weapon must weigh less than primary.{x\n\r", ch);
        return;
   }
   
   /*if (IS_CLASS(ch, CLASS_WARRIOR)
   ||  IS_CLASS(ch, CLASS_PALADIN)
   ||  IS_CLASS(ch, CLASS_ANTI_PALADIN))
     {

      if ( (obj_weight * 2 > get_obj_weight( wield ))
      || (obj_weight > 40)
      || (weapon_table[obj->value[0]].type == WEAPON_SPEAR)
      || (weapon_table[obj->value[0]].type == WEAPON_POLEARM))
        {
send_to_char ("\n\r{WA Secondary weapon must be 1/2 the weight of your Primary not exceeding{x\n\r",ch);
send_to_char ("{W40 lbs AND/OR may not be of the SPEAR or POLEARM weapon types.{x\n\r",ch);
        return;
        }

     }
     else if(IS_CLASS(ch, CLASS_RANGER))
     {
      if ( (obj_weight * 2 > get_obj_weight( wield ))
      || (obj_weight > 40)
      || (weapon_table[obj->value[0]].type != WEAPON_DAGGER && weapon_table[obj->value[0]].type != WEAPON_SWORD))
        {
        send_to_char ("\n\r{WA Secondary weapon must be 1/2 the weight of your Primary not exceeding{x\n\r",ch);
         send_to_char ("{W40 lbs AND must fall into the weapon catagory of DAGGER or SWORD.{x\n\r",ch);
         return;
        }
     }
   else
     {
      if ( (obj_weight * 2 > get_obj_weight( wield ))
      || (obj_weight > 40)
      || (weapon_table[obj->value[0]].type != WEAPON_DAGGER))
        {
        send_to_char ("\n\r{WA Secondary weapon must be 1/2 the weight of your Primary not exceeding{x\n\r",ch);
         send_to_char ("{W40 lbs AND must fall into the weapon catagory of DAGGER.{x\n\r",ch);
         return;
        }
     }*/


    if (!remove_obj(ch, WEAR_SECONDARY, TRUE)) /* remove the current weapon if any */
        return;                                /* remove obj tells about any no_remove */


    act ("\n\r{W$n {Wwields $p {Win $s off-hand.{x\n\r",ch,obj,NULL,TO_ROOM);
    act ("\n\r{GYou wield $p {Gin your off-hand.{x",ch,obj,NULL,TO_CHAR);
    equip_char ( ch, obj, WEAR_SECONDARY);
    return;
}


void do_donate( CHAR_DATA *ch, char *argument )
{
  char arg1 [MAX_INPUT_LENGTH];
  ROOM_INDEX_DATA *don_room;
  OBJ_DATA  *obj;
  //OBJ_DATA  *pit;

  argument = one_argument( argument, arg1 );

  if ( arg1[0] == '\0' )
    {
    send_to_char( "\n\r{CDonate what?{x\n\r", ch );
    return;
    }

    if ( !( obj = get_obj_carry( ch, arg1, ch ) ) )
    {
	send_to_char( "\n\r{RYou do not have that item!{x\n\r", ch );
	return;
    }

    if ( IS_OBJ_STAT( obj, ITEM_MELT_DROP )
    || obj->item_type == ITEM_FOOD
    || obj->item_type == ITEM_KEY
    || obj->item_type == ITEM_CORPSE_NPC
    || obj->cost == 0 )
      {
      send_to_char( "\n\r{RDonate something worthwile!{x\n\r", ch );
      return;
      }

    if (obj->level >= 91)
    {
	send_to_char( "\n\r{GYou {WCAN NOT {Gdonate items that are over {WLEVEL 70{G!{x\n\r", ch );
	return;
    }

    if ( obj->wear_loc != WEAR_INVENTORY )
    {
	send_to_char( "\n\r{RYou must remove it first.{x\n\r", ch );
	return;
    }
    
    if ( !can_drop_obj( ch, obj ) )
    {
	send_to_char( "\n\r{RYou can't let go of it.{x\n\r", ch );
	return;
    }

    /*if ( ch->clan > 0 )
      {
      CLAN_DATA *pClan = get_clan_index( ch->clan );
      
      if ( !pClan
      ||   !(don_room = get_room_index( pClan->hall )) )
      	don_room = get_room_index( 3039 );
      
      if ( ! don_room )
      	{
      	send_to_char( "\n\r{RCan't find the room with the pit.{x\n\r", ch );
      	return;
      	}
      
        for ( pit = don_room->contents; pit; pit = pit->next_content )
	if ( pit->item_type == ITEM_CONTAINER
	&&   is_name( pit->name, "clan donation pit") )
	  break;
      }      
    else*/
      {

       if ( !(don_room = get_room_index( 3039 )) )
	{
	send_to_char( "\n\r{RCannot find Donation Room.\{xn\r", ch );
	return;
	}
   
  act("\n\r{MA tiny demon runs upto you, grabs $p{M,\n\rand runs off to the donation room!{x\n\r",
	ch, obj, NULL, TO_CHAR );
  act("\n\r{GA tiny demon runs upto $n, grabs $p {Gfrom them,\n\rthen dashes off.{x\n\r",
        ch, obj, NULL, TO_ROOM );

if (don_room->people)
    {
act ("\n\r{cA tiny demon suddenly darts into the room, drops $p \n\r{con the heap and then disappears in a puff of brimstone!{x\n\r",
don_room->people,obj,NULL,TO_ALL);
    }

  obj->cost = 0;
  obj->timer = 1000;
  obj_from_char( obj );
  obj_to_room( obj,don_room);

  return;
  }  


return;
}

void do_auction (CHAR_DATA *ch, char *argument)
{
  OBJ_DATA *obj;
  int  min=10;
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];

  argument = one_argument (argument, arg1);
  if (IS_NPC(ch)) /* NPC can be extracted at any time and thus can't auction! */
    return;

  if (arg1[0] == '\0')
    {
    talk_auction(arg1, ch);
    return;
    }

  if(!str_cmp(arg1, "info"))
    {
    if(auction->item != NULL)
      {
            if (auction->bet > 0)
              {
    sprintf (buf, "\n\r     {gCurrent BID{w: {Y%d GOLD{x\n\r",auction->bet);
        send_to_char(buf, ch);
              }
          else
                {
                sprintf (buf, "\n\r{RNO {rbets on this item have been received.{x\n\r");
            send_to_char (buf,ch);

           sprintf(buf, "\n\r  {gMinimum bet is{w: {Y%d GOLD{x\n\r", auction->min);
          send_to_char(buf, ch);

                }

if (auction->item->level <= ch->level)
  {
       sprintf(buf, "{gIs useable Level{w: {GYES{x\n\r");
send_to_char(buf, ch);
  }
else
  {
       sprintf(buf, "{gIs useable Level{w: {RNO{x\n\r"); 
send_to_char(buf, ch);
  }

       sprintf(buf, "  {gWeight of item{w: {W%d{x\n\r",auction->item->weight); 
send_to_char(buf, ch);

        sprintf(buf, "    {gType of Item{w: {W%s{x\n\r", capitalize(item_name(auction->item->item_type)));
        send_to_char(buf, ch);

        if(auction->item->enchanted)
        sprintf(buf,"{gObject Enchanted{w: {BYES{x\n\r");
        else
        sprintf(buf,"{gObject Enchanted{w: {RNO{x\n\r");
        send_to_char(buf, ch);

                sprintf(buf,"          {gSeller{w: {W%s{g\n\r     High Bidder{w: {W%s{g\n\r ",
                        auction->seller->name,
                        auction->buyer ? auction->buyer->name : "none");
        send_to_char(buf, ch);

            if(IS_IMMORTAL(ch))
            {
        sprintf(buf, "\n\r          {rVnum{w: {R%d{x\n\r", auction->item->pIndexData->vnum);
        send_to_char(buf, ch);
            }

            return;
        }
        else
        {
            send_to_char ("\n\r{RAuction WHAT?{x\n\r",ch);
            return;
        }
    }


    if (IS_IMMORTAL(ch) && !str_cmp(arg1,"stop"))
      {
    if (auction->item == NULL)
    {
        send_to_char ("\n\r{RThere is no auction going on you can stop.{x\n\r",ch);
        return;
    }
    else /* stop the auction */
    {
        sprintf (buf,"{GSale of {x%s{g has been stopped by God. {rItem confiscated.{x",
                        auction->item->short_descr);
        talk_auction (buf, ch);

        obj_to_char (auction->item, ch);
        auction->item = NULL;
        if (auction->buyer != NULL) /* return money to the buyer */
        {
            auction->buyer->gold += auction->bet;
            send_to_char (
"{BA small demon appears next to you and returns your money!{x\n\r",auction->buyer);
        }
        return;
    }
      }

    if (!str_cmp(arg1,"bet") )
      {
        if (auction->item != NULL)
        {
            int newbet;
 
         if (ch->level != MAX_LEVEL && IS_IMMORTAL(ch))
           {
            send_to_char("\n\r{RImmortals should not be buying things...{x\n\r", ch);
            return;
           }


        if(same_obj(auction->item, ch->carrying) >=30)
        {
            send_to_char("\n\r{RYou already have enough.{x\n\r", ch);
            return;
        }

   if(!IS_IMP(ch))
     {
             if(auction->seller == ch)
               {
        send_to_char("\n\r{gYou may {rnot{g bid on your own item.{x\n\r", ch);
        return;
               }
     }

            /* make - perhaps - a bet now */
            if (argument[0] == '\0')
            {
                send_to_char ("{gBet how much?{x\n\r",ch);
                return;
            }

            newbet = parsebet (auction->bet, argument);

        if(auction->bet==0 && newbet<auction->min) 
         {
sprintf(buf, "\n\r{gMinimum opening bid on this item is {y%d gold{g.{x\n\r",
auction->min);
send_to_char(buf, ch);
return;
          }
            if (newbet < (auction->bet + 5))
            {
 send_to_char ("\n\r{gYou must at least bid {y5 gold{g over the current bet.{x\n\r",ch);
                return;
            }

            if (newbet > ch->gold)
            {
                send_to_char ("\n\r{gYou don't have that much money!{x\n\r",ch);
                return;
            }

            /* the actual bet is OK! */

            /* return the gold to the last buyer, if one exists */
            if (auction->buyer != NULL)
                auction->buyer->gold += auction->bet;

            ch->gold -= newbet; /* substract the gold - important :) */
            auction->buyer = ch;
            auction->bet   = newbet;
            auction->going = 0;
            auction->pulse = PULSE_AUCTION; /* start the auction over again */

           sprintf (buf,"{gA bet of {y%d gold{g has been received on {x%s{x.",
newbet,auction->item->short_descr); 
talk_auction (buf, ch);
            return;
        }
        else
        {
            send_to_char ("\n\r{gThere isn't anything being auctioned right now.{x\n\r",ch);
            return;
        }
      }
  

/* finally... */
argument = one_argument (argument, arg2);
if(arg2[0] != '\0')
{
        if(is_number(arg2))
          min=atoi(arg2);
        if(min<10)
        {
        send_to_char("\n\r{gMinimum bet must be at least {y10 gold{g.{x\n\r", ch);
        return;
        }
     if(!IS_IMP(ch))
       {
        if(min>100000)
        {
        send_to_char("\n\r{gMinimum bet can not be more than {Y100,000 gold{g.{x\n\r", ch);
        return;
        }
       }
}

    obj = get_obj_list (ch, arg1, ch->carrying); /* does char have the item ? */

    if (obj == NULL)
    {
        send_to_char ("\n\r{gYou aren't carrying that.{x\n\r",ch);
        return;
    }

    if (obj->item_type == ITEM_CONTAINER)
      {
       if (obj->contains)
         {
          send_to_char( "\n\r{CYou must empty it out first.{x\n\r", ch );
          return;
         }
       }


    if ( (IS_SET(obj->extra_flags, ITEM_NOREMOVE)
    ||   IS_SET(obj->extra_flags, ITEM_NODROP)
    ||   IS_SET(obj->extra_flags, ITEM_NOUNCURSE)
    ||   IS_SET(obj->extra_flags, ITEM_NOPURGE)
    ||   IS_SET(obj->extra_flags, ITEM_CLAN_EQ)) && !IS_IMP(ch))
    {
        send_to_char( "\n\r{RThis ITEM is CURSED or is CLAN EQ, either way it can not be auctioned...{x\n\r", ch );
        return;
    }
    

    if ( obj->wear_loc != WEAR_INVENTORY )
    {
        send_to_char( "\n\r{RYou must remove it first.{x\n\r", ch );
        return;
    }

        if(min>ch->gold)
        {
        send_to_char("\n\r{RYou can not afford to auction that, at that price.{x\n\r",
ch);
        return;
        }



    if (auction->item == NULL)
    {
//    switch (obj->item_type)	// commented out by sembiance to allow all item types
//    {

//    default:
//        act ("\n\r{RYou cannot auction $Ts.{x\n\r", ch, NULL, item_name(obj->item_type), TO_CHAR);
//        return;

/*    case ITEM_WEAPON:
    case ITEM_GEM:
    case ITEM_CONTAINER:
    case ITEM_LIGHT_SOURCE:
    case ITEM_TREASURE:
    case ITEM_SEGMENT:
    case ITEM_JEWELRY:
    case ITEM_MAP:
    case ITEM_ARMOR:
    case ITEM_STAFF:
    case ITEM_WAND:
*/

        obj_from_char (obj);
        auction->item = obj;
        auction->bet = 0;
        auction->buyer = NULL;
        auction->seller = ch;
        auction->min = min;
        auction->pulse = PULSE_AUCTION;
        auction->going = 0;

        sprintf (buf, "{GA new item has been received: {x%s{g. Min Bet: {y%d gold{g.{x",
obj->short_descr, auction->min);
        talk_auction (buf, ch);

        sprintf (buf,
"{BA small demon appears next to you, grabs {W%s{B from you, & disappears quickly!{x\n\r",
auction->item->short_descr);
        send_to_char(buf, ch);
        return;

//    } /* switch */
    }
    else
    {
        act ("\n\r{gTry again later - $p is being auctioned right now!{x\n\r",
ch,auction->item,NULL,TO_CHAR);
        return;
    }
}

/*
void do_balance ( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    if (IS_NPC(ch))
	return;

    sprintf( buf, "You have %ld coins in the bank.\n\r", ch->bank );
    send_to_char( buf, ch );
    return;
}

void do_deposit ( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *banker;
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int amnt;

    if (IS_NPC(ch))
	return;

    if (!IS_SET(ch->in_room->room_flags, ROOM_BANK) ) 
    {
	sprintf( buf, "But you are not in a bank.\n\r" );
	send_to_char( buf, ch );
	return;
    }

    banker = NULL;
    for ( banker = ch->in_room->people; banker; banker = banker->next_in_room )
    {
	if ( IS_NPC( banker ) && IS_SET(banker->pIndexData->act, ACT_BANKER) )
	    break;
    }

    if ( !banker )
    {
	sprintf( buf, "The banker is currently not available.\n\r" );
	send_to_char( buf, ch );
	return;
    }
 
    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	sprintf( buf, "How much gold do you wish to deposit?\n\r" );
	send_to_char( buf, ch );
	return;
    }

    amnt = atoi( arg );
    
    if ( amnt >= (ch->gold + 1) )
    {
	sprintf( buf, "%s, you do not have %d gold coins.", ch->name, amnt );
	do_say( banker, buf );
	return;
    }

    ch->bank += amnt;
    ch->gold -= amnt;
    sprintf( buf, "%s, your account now contains: %ld coins,", ch->name, ch->bank );
    do_say( banker, buf );
    sprintf( buf, "after depositing: %d coins.", amnt );
    do_say( banker, buf );
    return;
}

void do_withdraw ( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *banker;
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int amnt;

    if (IS_NPC(ch))
	return;

    if (!IS_SET(ch->in_room->room_flags, ROOM_BANK) ) 
    {
	sprintf( buf, "But you are not in a bank.\n\r" );
	send_to_char( buf, ch );
	return;
    }

    banker = NULL;
    for ( banker = ch->in_room->people; banker; banker = banker->next_in_room )
    {
	if ( IS_NPC( banker ) && IS_SET(banker->pIndexData->act, ACT_BANKER) )
	    break;
    }

    if ( !banker )
    {
	 sprintf( buf, "The banker is currently not available.\n\r" );
	 send_to_char( buf, ch );
	 return;
    }
 
    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	 sprintf( buf, "How much gold do you wish to withdraw?\n\r" );
	 send_to_char( buf, ch );
	 return;
    }

    amnt = atoi( arg );
    
    if ( amnt >= (ch->bank + 1) )
    {
	sprintf( buf, "%s, you do not have %d gold coins in the bank.", ch->name, amnt );
	do_say( banker, buf );
	return;
    }

    ch->gold += amnt;
    ch->bank -= amnt;
    sprintf( buf, "%s, your account now contains: %ld coins,", ch->name, ch->bank );
    do_say( banker, buf );
    sprintf( buf, "after withdrawing: %d coins.", amnt );
    do_say( banker, buf );
    return;
}
*/

void do_brew ( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int sn;
    int absn;
    int mana;

	absn=0;

    if ( !IS_NPC(ch)
    &&   !can_use_skpell( ch, gsn_brew) )
    {                                          
	send_to_char( "\n\r{WYou do not know how to brew potions.{x\n\r", ch );
	return;
    }

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char("\n\r{WBrew what spell?{x\n\r", ch );
	return;
    }

	if(!str_prefix(arg, "cure critical") 
        || !str_prefix(arg, "heal")
        || !str_prefix(arg, "cure serious")
	|| !str_prefix(arg, "mass healing") 
	|| !str_prefix(arg, "divine wrath") 
	|| !str_prefix(arg, "complete healing") )
		{
			send_to_char("\n\r{RYou can't brew that spell.{x\n\r",ch);
			return;
		}

    /* Do we have a vial to brew potions? */
    for ( obj = ch->carrying; obj; obj = obj->next_content )
    {
	if ( obj->item_type == ITEM_POTION && obj->wear_loc == WEAR_HOLD )
	    break;
    }

    if ( !obj )
    {
	send_to_char( "\n\r{RYou are not holding a vial.{x\n\r", ch );
	return;
    }
    
    if (obj->pIndexData->vnum != OBJ_VNUM_VIAL_SINGLE)
     {
	send_to_char( "\n\r{RThis is not the correct type of vial for brewing.{x\n\r", ch );
        return;
     }

    if ( ( sn = skill_lookup(arg) )  < 0)
    {
	send_to_char( "\n\r{RYou don't know any spells by that name.{x\n\r", ch );
	return;
    }

    /* preventing potions of gas breath, acid blast, etc.; doesn't make sense
       when you quaff a gas breath potion, and then the mobs in the room are
       hurt. Those TAR_IGNORE spells are a mixed blessing. - JH */
  
    if ( (skill_table[sn].target != TAR_CHAR_DEFENSIVE) && 
         (skill_table[sn].target != TAR_CHAR_SELF)              ) 
    {
	send_to_char( "\n\r{RYou cannot brew that spell.{x\n\r", ch );
	return;
    }

    act( "{Y\n\r$n begins preparing a potion.\n\r{x", ch, obj, NULL, TO_ROOM );
    WAIT_STATE( ch, skill_table[gsn_brew].beats );

	mana = 3*UMAX(skill_table[sn].min_mana,
	    100 / ( 2 + ch->level - 
		skill_table[sn].skill_level[ch->class] ) );
	check_improve(ch,sn,FALSE,1);

    /* Check the skill percentage, fcn(wis,int,skill) */

   if ( !IS_NPC(ch) 
         && ( number_percent( ) > ch->pcdata->learned[gsn_brew] ||
              number_percent( ) > ((get_curr_stat(ch,STAT_INT)-8)*5 + 
                                   (get_curr_stat(ch,STAT_WIS)-8)*3)))

    {   
        act( "\n\r{G$p explodes violently!{x\n\r", ch, obj, NULL, TO_CHAR );
	act( "\n\r{G$p explodes violently!{x\n\r", ch, obj, NULL, TO_ROOM );
	extract_obj( obj );
	return;
    }

    /* took this outside of imprint codes, so I can make do_brew differs from
       do_scribe; basically, setting potion level and spell level --- JH */

    obj->level = ch->level/2;
    obj->value[0] = ch->level/2;
    spell_imprint(sn, ch->level, ch, obj); 

}

void do_scribe ( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int sn;
  
    if ( !IS_NPC(ch)
    &&   !can_use_skpell( ch, gsn_scribe) )          
    {                                          
	send_to_char( "\n\r{BYou do not know how to scribe scrolls.{x\n\r", ch );
	return;
    }

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "\n\r{WScribe what spell?{x\n\r", ch );
	return;
    }
   
    if(!str_prefix(arg, "cure critical") 
        || !str_prefix(arg, "heal")
        || !str_prefix(arg, "cure serious")
        || !str_prefix(arg, "golden aura")
	|| !str_prefix(arg, "mass healing")
        || !str_prefix(arg, "remove curse") 
        || !str_prefix(arg, "divine wrath") 
        || !str_prefix(arg, "sonic scream") 
        || !str_prefix(arg, "complete healing") )
		{
			send_to_char("{R\n\rYou can't scribe that spell.{x\n\r",ch);
			return;
		}
   
    /* Do we have a parchment to scribe spells? */
    for ( obj = ch->carrying; obj; obj = obj->next_content )
    {
	if ( obj->item_type == ITEM_SCROLL && obj->wear_loc == WEAR_HOLD )
	    break;
    }
    if ( !obj )
    {
	send_to_char( "\n\r{BYou are not holding a parchment.{x\n\r", ch );
	return;
    }
    
    if (obj->pIndexData->vnum != OBJ_VNUM_SCROLL_SINGLE)
     {
	send_to_char( 
"\n\r{BYou must use a particular type of parchment to scribe to, this is not it.{x\n\r", ch );
        return;
     }

    if ( ( sn = skill_lookup(arg) )  < 0)
    {
	send_to_char("\n\r{BYou don't know any spells by that name.{x\n\r", ch );
	return;
    }
    
    act( "{Y\n\r$n begins writing a scroll.{x\n\r", ch, obj, NULL, TO_ROOM );
    WAIT_STATE( ch, skill_table[gsn_scribe].beats );

    /* Check the skill percentage, fcn(int,wis,skill) */
    if ( !IS_NPC(ch) 
         && ( number_percent( ) > ch->pcdata->learned[gsn_scribe] ||
              number_percent( ) > ((get_curr_stat(ch,STAT_INT)-8)*5 + 
                                   (get_curr_stat(ch,STAT_WIS)-8)*3)))
    {
	act( "\n\r{R$p bursts in flames!{x\n\r", ch, obj, NULL, TO_CHAR );
	act( "\n\r{R$p bursts in flames!{x\n\r", ch, obj, NULL, TO_ROOM );
	extract_obj( obj );
	return;
    }

    /*
 basically, making scrolls more potent than potions; also, scrolls
       are not limited in the choice of spells, i.e. scroll of enchant weapon
       has no analogs in potion forms --- JH */

    obj->level = ch->level*2/3;
    obj->value[0] = ch->level/1.5;
    spell_imprint(sn, ch->level, ch, obj); 

} 


bool ProcessObjectJoin(CHAR_DATA * ch, OBJ_DATA * obj1, OBJ_DATA * obj2)
{
    char **             segmentScript=0;
    char **             line=0;
    char **             subCommands=0;
    char **             ar=0;
    char **             joinedWith=0;
    char *              tempBuffer=0;
    char **             joinsWith=0;
    char **             ifJoinedWith=0;
    bool                matched=FALSE, joinsWithMatchFound=FALSE;
    OBJ_INDEX_DATA *    newObjectIndex;
    OBJ_DATA *          newObject;
    EXTRA_DESCR_DATA *  ed=0;
    char                buf[MAX_STRING_LENGTH];

    if(!GetObjExtraDesc(obj1, "=segment="))
        return TRUE;
        
    segmentScript = strstrexplode(GetObjExtraDescText(GetObjExtraDesc(obj1, "=segment=")), "\n\r");
        
    for(line=segmentScript;line && *line;line++)
    {
        if(strstartswith(*line, "joinswith") && !GetObjExtraDesc(obj1, "=joinedwith="))
        {
            subCommands = strchrexplode(*line, ' ');
            if(!subCommands || array_len(subCommands)<2)
            {
                subCommands = array_free(subCommands);
                continue;
            }
            
            joinsWith = strchrexplode(subCommands[1], ',');
            for(joinsWithMatchFound=FALSE,ar=joinsWith;ar && *ar;ar++)
            {
                // Check to see what vnum this joins with, it must be obj2
                if(is_number(*ar) && atoi(*ar)==obj2->pIndexData->vnum)
                {
                    joinsWithMatchFound = TRUE;
                    break;
                }
            }
            joinsWith = array_free(joinsWith);
            
            if(joinsWithMatchFound==FALSE)
            {
                subCommands = array_free(subCommands);
                continue;
            }
            
            // Valid join! First send out our message to our char
            for(line++;*line && strcmp(*line, "`");line++)
            {
                tempBuffer = strappend(tempBuffer, *line);
                tempBuffer = strappend(tempBuffer, "\n\r");
            }
            act_new(tempBuffer, ch, obj1, obj2, TO_CHAR, POS_DEAD);
            tempBuffer = strfree(tempBuffer);

            // Now send out the to_room message
            if(*line)
            {
                for(line++;*line && strcmp(*line, "`");line++)
                {
                    tempBuffer = strappend(tempBuffer, *line);
                    tempBuffer = strappend(tempBuffer, "\n\r");
                }
                act_new(tempBuffer, ch, obj1, obj2, TO_ROOM, POS_DEAD);
                tempBuffer = strfree(tempBuffer);
            }
            
            // Now see if we are supposed to create a new object (trash both current ones) or just join them
            if(subCommands[2] && !strcmp(subCommands[2], "creates") && subCommands[3] && is_number(subCommands[3]))
            {
                // Create object vnum
                if(!(newObjectIndex=get_obj_index(atoi(subCommands[3]))) || !(newObject=create_object(newObjectIndex, 0)))
                {
                    subCommands = array_free(subCommands);
                    return TRUE;
                }
                    
                // If the object it's being joined too (obj2) is in the room then create new object in room, else on char
                if(!obj2->carried_by || !CAN_WEAR(newObject, ITEM_TAKE))
                    obj_to_room(newObject, ch->in_room);
                else
                    obj_to_char(newObject, ch);
                
                
                extract_obj(obj1);
                extract_obj(obj2);
                
                return FALSE;
            }
            else
            {
                // Just join the objects
                if(!CAN_WEAR(obj1, ITEM_TAKE) || !CAN_WEAR(obj2, ITEM_TAKE))
                {
                    if(obj1->carried_by)
                    {
                        obj_from_char(obj1);
                        obj_to_room(obj1, ch->in_room);
                    }
                    
                    if(obj2->carried_by)
                    {
                        obj_from_char(obj2);
                        obj_to_room(obj2, ch->in_room);
                    }
                }
                else
                {
                    if(!obj2->carried_by)
                    {
                        if(obj1->carried_by)
                            obj_from_char(obj1);
                        obj_to_room(obj1, ch->in_room);
                    }
                    else
                    {
                        if(obj1->carried_by!=obj2->carried_by)    // should not be possible!
                        {
                            obj_from_char(obj1);
                            obj_to_char(obj1, obj2->carried_by);
                        }
                    }
                }
                
                // Join the objects together
                sprintf(buf, "%d\n\r", obj2->pIndexData->vnum);
                if((ed=GetObjExtraDesc(obj1, "=joinedwith=")))
                {
                    tempBuffer = strdup(ed->description);
                    tempBuffer = strappend(tempBuffer, buf);
                    
                    free_string(ed->description);
                    ed->description = str_dup(tempBuffer);
                    
                    tempBuffer = strfree(tempBuffer);
                }
                else
                {
                    AddObjExtraDesc(obj1, "=joinedwith=", buf);
                }
                
                sprintf(buf, "%d\n\r", obj1->pIndexData->vnum);
                if((ed=GetObjExtraDesc(obj2, "=joinedwith=")))
                {
                    tempBuffer = strdup(ed->description);
                    tempBuffer = strappend(tempBuffer, buf);
                    
                    free_string(ed->description);
                    ed->description = str_dup(tempBuffer);
                    
                    tempBuffer = strfree(tempBuffer);
                }
                else
                {
                    AddObjExtraDesc(obj2, "=joinedwith=", buf);
                }
            }
            
            subCommands = array_free(subCommands);
        }
        else if(strstartswith(*line, "ifjoinedwith") && GetObjExtraDesc(obj1, "=joinedwith="))
        {
            // Creates an object
            subCommands = strchrexplode(*line, ' ');
            if(!subCommands || array_len(subCommands)<4)
                continue;
            
            ifJoinedWith = strchrexplode(subCommands[1], ',');
            joinedWith = strstrexplode(GetObjExtraDescText(GetObjExtraDesc(obj1, "=joinedwith=")), "\n\r");
            
            for(matched=TRUE,ar=ifJoinedWith,ar++;ar && *ar;ar++)
            {
                if(array_find(joinedWith, *ar)==-1)
                {
                    matched = FALSE;
                    break;
                }
            }
            
            ifJoinedWith = array_free(ifJoinedWith);
            joinedWith = array_free(joinedWith);
            
            if(!matched)    // didn't match or didn't find a non-number (command)
            {
                subCommands = array_free(subCommands);
                continue;
            }
            
            // First send out our message to our char
            for(line++;*line && strcmp(*line, "`");line++)
            {
                tempBuffer = strappend(tempBuffer, *line);
                tempBuffer = strappend(tempBuffer, "\n\r");
            }
            act_new(tempBuffer, ch, obj1, obj2, TO_CHAR, POS_DEAD);
            tempBuffer = strfree(tempBuffer);

            // Now send out the to_room message
            if(*line)
            {
                for(line++;*line && strcmp(*line, "`");line++)
                {
                    tempBuffer = strappend(tempBuffer, *line);
                    tempBuffer = strappend(tempBuffer, "\n\r");
                }
                act_new(tempBuffer, ch, obj1, obj2, TO_ROOM, POS_DEAD);
                tempBuffer = strfree(tempBuffer);
            }
            
            if(!strcmp(subCommands[2], "creates") && is_number(subCommands[3]))
            {
                 // Create object vnum
                if(!(newObjectIndex=get_obj_index(atoi(subCommands[3]))) || !(newObject=create_object(newObjectIndex, 0)))
                {
                    subCommands = array_free(subCommands);
                    continue;
                }
                    
                // If the object it's being joined too (obj2) is in the room then create new object in room, else on char
                if(!obj2->carried_by || !CAN_WEAR(newObject, ITEM_TAKE))
                    obj_to_room(newObject, ch->in_room);
                else
                    obj_to_char(newObject, ch);
                
                joinedWith = strstrexplode(GetObjExtraDescText(GetObjExtraDesc(obj1, "=joinedwith=")), "\n\r");
                
                if(obj1->carried_by)
                    extract_objs_in_list(ch->carrying, joinedWith);
                else
                    extract_objs_in_list(ch->in_room->contents, joinedWith);
                    
                joinedWith = array_free(joinedWith);
                    
                // Check if destroys itself, if so extract_obj(obj1);  otherwise just remove joins
                {
                    DeleteObjExtraDesc(obj1, "=joinedwith=");
                }
                
                subCommands = array_free(subCommands);
                
                return FALSE;
            }

            subCommands = array_free(subCommands);
        }
        
        if(!*line)
            break;
    }
    
    return TRUE;
}





void do_join(CHAR_DATA *ch, char * argument)
{
    char                arg1[MAX_INPUT_LENGTH];
    char                arg2[MAX_INPUT_LENGTH];
    char                buf[MAX_STRING_LENGTH];
    OBJ_DATA *          obj1=0;
    OBJ_DATA *          obj2=0;
    char *              message=0;
    OBJ_DATA *          new_obj;
    OBJ_INDEX_DATA *    new_obj_index;
    int                 i;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if(arg1[0] == '\0' || arg2[0] == '\0')
    {
        send_to_char("Usage: join <object1> <object2>\n\r",ch);
        return;
    }

    obj1 = get_obj_here_me_first(ch, arg1);
    if(!obj1)
    {
        sprintf(buf, "{RYou don't have, and do not see {W%s {Rin the room.{x\n\r", arg1 ? arg1 : "{Rthat");
        send_to_char(buf, ch);
        return;
    }
    
    obj2 = get_obj_here_me_first(ch, arg2);
    if(!obj2)
    {
        sprintf(buf, "{RYou don't have, and do not see {W%s {Rin the room.{x\n\r", arg2 ? arg2 : "{Rthat");
        send_to_char(buf, ch);
        return;
    }

    if(obj1==obj2)
    {
        send_to_char("{RYou can not join an item to itself.{x\n\r", ch);
        return;
    }
    
    // Handle the legacy segments
    if(obj1->item_type==ITEM_SEGMENT && obj2->item_type==ITEM_SEGMENT)
    {
        for(i=1;i<=4;i++)
        {
            if(obj1->value[i] == obj2->pIndexData->vnum)
                break;
        }
    
        if(i==5)
        {
            sprintf(buf, "%s does not fit into %s\n\r", obj1->short_descr, obj2->short_descr);
            send_to_char(buf, ch);
            return;
        }
    
        // Join works
        if(obj1->value[0] == obj2->value[0])
        {
            if((message=GetObjExtraDescText(GetObjExtraDesc(obj1, "joinmsg"))))
                send_to_char(message, ch);
            if((message=GetObjExtraDescText(GetObjExtraDesc(obj2, "joinmsg"))))
                send_to_char(message, ch);
                
            new_obj_index = get_obj_index(obj1->value[0]);
            if(new_obj_index==NULL)
            {
                send_to_char("Could not complete joining. err1\n\r", ch);
                return;
            }
    
            new_obj = create_object(new_obj_index, 0);
    
            if (CAN_WEAR(new_obj, ITEM_TAKE))
                obj_to_char( new_obj, ch );
            else
                obj_to_room( new_obj, ch->in_room );
            
            if((message=GetObjExtraDescText(GetObjExtraDesc(new_obj, "mademsg"))))
                send_to_char(message, ch);
    
            extract_obj(obj1);
            extract_obj(obj2);
    
            return;
        }
        
        // Join failed
        if((message=GetObjExtraDescText(GetObjExtraDesc(obj1, "failmsg"))))
            send_to_char(message, ch);
        if((message=GetObjExtraDescText(GetObjExtraDesc(obj2, "failmsg"))))
            send_to_char(message, ch);
            
        extract_obj(obj1);
        extract_obj(obj2);   
        
        return;
    } 
    
    // Handle new segments
    if((!GetObjExtraDesc(obj1, "=segment=") && !GetObjExtraDesc(obj2, "=segment=")) || !obj1->pIndexData || !obj2->pIndexData)
    {
        sprintf(buf, "{R{x%s{R is not capable of being joined with {x%s{R.{x\n\r", obj1->short_descr, obj2->short_descr);
        send_to_char(buf, ch);
        return;
    }
    
    if(ProcessObjectJoin(ch, obj1, obj2))
        ProcessObjectJoin(ch, obj2, obj1);

    return;
}


