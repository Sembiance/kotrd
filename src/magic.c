#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "interp.h"
#include "magic.h"
#include "recycle.h"

/*
 * Local functions.
 */
void	say_spell	args( ( CHAR_DATA *ch, int sn ) );
void sonic_effect(void *vo, int level, int dam, int target);
void spell_imprint(int sn, int level, CHAR_DATA *ch, void *vo);
DECLARE_DO_FUN ( do_peace );

/* imported functions */
bool    remove_obj      args( ( CHAR_DATA *ch, int iWear, bool fReplace ) );
void 	wear_obj	args( ( CHAR_DATA *ch, OBJ_DATA *obj, bool fReplace ) );


// Stones of Wisdom Stuff
bool stones_try_moving(CHAR_DATA * ch);

/*
 * Lookup a skill by name.
 */
int skill_lookup( const char *name )
{
    int sn;

    for ( sn = 0; sn < MAX_SKILL; sn++ )
    {
	if ( skill_table[sn].name == NULL )
	    break;
	if ( LOWER(name[0]) == LOWER(skill_table[sn].name[0])
	&&   !str_prefix( name, skill_table[sn].name ) )
	    return sn;
    }

    return -1;
}

int find_spell( CHAR_DATA *ch, const char *name )
{
    /* finds a spell the character can cast if possible */
    int sn, found = -1;

    if (IS_NPC(ch))
	return skill_lookup(name);

    for ( sn = 0; sn < MAX_SKILL; sn++ )
    {
	if (skill_table[sn].name == NULL)
	    break;
	if (LOWER(name[0]) == LOWER(skill_table[sn].name[0])
	&&  !str_prefix(name,skill_table[sn].name))
	{
	    if ( found == -1)
		found = sn;
	    if (ch->level >= skill_table[sn].skill_level[ch->class]
	    &&  ch->pcdata->learned[sn] > 0)
		    return sn;
	}
    }
    return found;
}



/*
 * Lookup a skill by slot number.
 * Used for object loading.
 */
int slot_lookup( int slot )
{
    extern bool fBootDb;
    int sn;

    if ( slot <= 0 )
	return -1;

    for ( sn = 0; sn < MAX_SKILL; sn++ )
    {
	if ( slot == skill_table[sn].slot )
	    return sn;
    }

    if ( fBootDb )
    {
	bug( "Slot_lookup: bad slot %d.", slot );
	abort( );
    }

    return -1;
}



/*
 * Utter mystical words for an sn.
 */
void say_spell( CHAR_DATA *ch, int sn )
{
    char buf  [MAX_STRING_LENGTH];
    char buf2 [MAX_STRING_LENGTH];
    CHAR_DATA *rch;
    char *pName;
    int iSyl;
    int length;

    struct syl_type
    {
	char *	old;
	char *	new;
    };

    static const struct syl_type syl_table[] =
    {
	{ " ",		" "		},
	{ "ar",		"abra"		},
	{ "au",		"kada"		},
	{ "bless",	"fido"		},
	{ "blind",	"nose"		},
	{ "bur",	"mosa"		},
	{ "cu",		"judi"		},
	{ "de",		"oculo"		},
	{ "en",		"unso"		},
	{ "light",	"dies"		},
	{ "lo",		"hi"		},
	{ "mor",	"zak"		},
	{ "move",	"sido"		},
	{ "ness",	"lacri"		},
	{ "ning",	"illa"		},
	{ "per",	"duda"		},
	{ "ra",		"gru"		},
	{ "fresh",	"ima"		},
	{ "re",		"candus"	},
	{ "son",	"sabru"		},
	{ "tect",	"infra"		},
	{ "tri",	"cula"		},
	{ "ven",	"nofo"		},
	{ "a", "a" }, { "b", "b" }, { "c", "q" }, { "d", "e" },
	{ "e", "z" }, { "f", "y" }, { "g", "o" }, { "h", "p" },
	{ "i", "u" }, { "j", "y" }, { "k", "t" }, { "l", "r" },
	{ "m", "w" }, { "n", "i" }, { "o", "a" }, { "p", "s" },
	{ "q", "d" }, { "r", "f" }, { "s", "g" }, { "t", "h" },
	{ "u", "j" }, { "v", "z" }, { "w", "x" }, { "x", "n" },
	{ "y", "l" }, { "z", "k" },
	{ "", "" }
    };

    buf[0]	= '\0';
    for ( pName = skill_table[sn].name; *pName != '\0'; pName += length )
    {
	for ( iSyl = 0; (length = strlen(syl_table[iSyl].old)) != 0; iSyl++ )
	{
	    if ( !str_prefix( syl_table[iSyl].old, pName ) )
	    {
		strcat( buf, syl_table[iSyl].new );
		break;
	    }
	}

	if ( length == 0 )
	    length = 1;
    }

    sprintf( buf2, "$n utters the words, '%s'.", buf );
    sprintf( buf,  "$n utters the words, '%s'.", skill_table[sn].name );

    for ( rch = ch->in_room->people; rch; rch = rch->next_in_room )
    {
	if ( rch != ch )
	    act((!IS_NPC(rch) && ch->class==rch->class) ? buf : buf2,
	        ch, NULL, rch, TO_VICT );
    }

    return;
}

bool saves_spell( int level, CHAR_DATA *victim, int dam_type )
{
    int save;

    save = ( 50 + ((( victim->level - level) * 5) - (victim->saving_throw)) );

    if (IS_AFFECTED(victim,AFF_BERSERK))
	save += victim->level/2;

    switch(check_immune(victim,dam_type))
    {
	case IS_IMMUNE:		return TRUE;
	case IS_RESISTANT:	save += (victim->level);	break;
	case IS_VULNERABLE:	save -= (victim->level);	break;
    }

    if (!IS_NPC(victim) 
    && (!(class_table[victim->class].fMana)))
      {
	save = ((9 * save) / 10);
      }
 
    save = URANGE( 5, save, 95 );
    return number_percent( ) < save;
}

bool saves_dispel( int dis_level, int spell_level, int duration)
{
    int save;
    
    if (duration == -1)
      spell_level += 5;  


    save = 50 + (spell_level - dis_level) * 5;
    save = URANGE( 5, save, 95 );
    return number_percent( ) < save;
}


bool check_dispel( int dis_level, CHAR_DATA *victim, int sn)
{
    AFFECT_DATA *af;

    if (is_affected(victim, sn))
    {
        for ( af = victim->affected; af != NULL; af = af->next )
        {
            if ( af->type == sn )
              {
                if(IS_IMP(victim))
                  {
                   affect_strip(victim,sn);
        	   if ( skill_table[sn].msg_off )
        	     {
            	      send_to_char( skill_table[sn].msg_off, victim );
            	      send_to_char( "\n\r", victim );
        	     }
                  }
                 else
                  {
                   if (!saves_dispel(dis_level,af->level,af->duration))
                     {
                      affect_strip(victim,sn);
        	      if ( skill_table[sn].msg_off )
        	        {
            		 send_to_char( skill_table[sn].msg_off, victim );
            		 send_to_char( "\n\r", victim );
        	        }
                     }
                   }
                               return TRUE;
}
  	           else
                    af->level--;


            }
        }
    return FALSE; 
}

int MANA_COST( CHAR_DATA *ch, int sn )
{
  int level = 1;
 
 if ( IS_NPC( ch ) )
    return 0;

if ( ch->pcdata->oldcl == -1 )
    level = skill_table[sn].skill_level[ch->class];
  else
    level = skill_table[sn].skill_level[ch->class] <=
            skill_table[sn].skill_level[ch->pcdata->oldcl] 
          ? skill_table[sn].skill_level[ch->class] 
          : skill_table[sn].skill_level[ch->pcdata->oldcl];

  return UMIN( skill_table[sn].min_mana, 5000 ); 
/*  return UMIN( skill_table[sn].min_mana, 100 / ( 2 + ch->level - level ) ); */
}


/*
 * The kludgy global is for spells who want more stuff from command line.
 */
char *target_name;

void do_cast( CHAR_DATA *ch, char *argument )
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  OBJ_DATA *obj;
  ROOM_INDEX_DATA *location;
  void *vo;
  int mana;
  int sn;
  int target;
  int diceroll;

  location=NULL;
 
  send_to_char("\n\r",ch);

  /*
   * Switched NPC's can cast spells, but others can't.
   */
  if ( IS_NPC(ch) && ch->desc == NULL)
    return;

  target_name = one_argument( argument, arg1 );
  one_argument( target_name, arg2 );

  if ( arg1[0] == '\0' )
    {
    send_to_char( "\n\r{GCast which what where?{x\n\r", ch );
    return;
    }

  if ((sn = find_spell(ch,arg1)) < 1
  ||  skill_table[sn].spell_fun == spell_null
  || (!IS_NPC(ch) && (!can_use_skpell( ch, sn )
  ||                   ch->pcdata->learned[sn] == 0)))
    {
    send_to_char( "\n\r{RYou don't know any spells of that name.{x\n\r", ch );  
    return;
    }

  
  if (ch->level <= ASSTIMP)
    { 
  location = get_room_index(ROOM_VNUM_DRAGONPIT);

  if (ch->in_room == location)
    {
    send_to_char( "\n\r{GYou must wait until the DragonPIT begins to start casting!{x\n\r", ch );
    return;
    }

if (!IS_NPC(ch))
   {
  if (IS_SET(ch->comm, COMM_AFK))
    {
     send_to_char("AFK mode removed due to CASTING. Type 'replay' to see tells.\n\r",ch);
      REMOVE_BIT(ch->comm,COMM_AFK);
    }
   }

  if ( ch->position < skill_table[sn].minimum_position )
    {
    send_to_char( "You can't concentrate enough.\n\r", ch );
    return;
    }
   }

  mana = MANA_COST (ch, sn );

  /*
   * Locate targets.
   */
  victim	= NULL;
  obj		= NULL;
  vo		= NULL;
  target	= TARGET_NONE;
     

  switch ( skill_table[sn].target )
    {
    default:
      bug( "Do_cast: bad target for sn %d.", sn );
      return;

    case TAR_IGNORE:
      break;

    case TAR_CHAR_OFFENSIVE:

       if (IS_SET(ch->in_room->room_flags, ROOM_SAFE)
       && (ch != victim))
         {
         send_to_char( "{RYou can only casts spells on yourself in Safe Rooms.{x\n\r", ch );
         return;
         }

      if ( arg2[0] == '\0' )
        {
        if ( ( victim = ch->fighting ) == NULL )
          {
          send_to_char( "Cast the spell on whom?\n\r", ch );
          return;
          }
        }
      else
        {
        if ( ( victim = get_char_room( ch, target_name ) ) == NULL )
          {
          send_to_char( "They aren't here.\n\r", ch );
          return;
          }
        }

      /* Check if players are CHOOSEN */
     

if (!IS_NPC(victim))
{
if (!IS_NPC(ch))
  { 
   if (ch->level < MAX_LEVEL)
     {
      if (!IS_SET(victim->in_room->room_flags, ROOM_DRAGONPIT))
        {
         if (!IS_SET(ch->in_room->room_flags,ROOM_DRAGONPIT))    
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
}


      if ( !IS_NPC(ch) )
        {
        if (is_safe(ch,victim) && victim != ch)
          {
          send_to_char("\n\r{RNot on that target.{x\n\r",ch);
          return; 
          }
        }

      if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
        {
  	    send_to_char( "You can't do that on your own follower.\n\r", ch );
        return;
        }
      vo = (void *) victim;
      target = TARGET_CHAR;


    if (!IS_NPC(ch))
      {
    if (IS_SET(ch->comm, COMM_COMBAT))
      {
   if (( get_skill(ch,gsn_spellcraft) >= 2 )
   && ( IS_CLASS( ch, CLASS_MAGE )
   ||   IS_CLASS( ch, CLASS_CLERIC ) ) )
      {
       diceroll = number_percent();
  
       if (diceroll <= get_skill(ch,gsn_spellcraft))
        {
        send_to_char ("{MSPELLCRAFT{m boosts the {MMAGICAL {mpower of your spell.{x\n\r",ch);
        }
      }
     }   
    }
      break;

    case TAR_CHAR_DEFENSIVE:
      if ( arg2[0] == '\0' )
	      victim = ch;
      else
        {
        if ( ( victim = get_char_room( ch, target_name ) ) == NULL )
          {
          send_to_char( "They aren't here.\n\r", ch );
          return;
          }
        }

	vo = (void *) victim;
	target = TARGET_CHAR;
	break;

    case TAR_CHAR_SELF:

         if ( arg2[0] == '\0' )
	   victim = ch;
         else
           {
            if ( ( victim = get_char_room( ch, target_name ) ) == NULL )
              {
               send_to_char( "\n\r{RThey aren't here!{x\n\r", ch );
               return;
              }
           }

	 vo = (void *) victim;
  	 target = TARGET_CHAR;
	 break;
/*

	 if ( arg2[0] != '\0' && !is_name( target_name, ch->name ) )
	   {
	    send_to_char( "\n\r{RYou cannot cast this spell on another!{x\n\r", ch );
	    return;
	   }

	vo = (void *) ch;
	target = TARGET_CHAR;
	break;
*/
    case TAR_OBJ_INV:
	if ( arg2[0] == '\0' )
	{
	    send_to_char( "What should the spell be cast upon?\n\r", ch );
	    return;
	}

	if ( ( obj = get_obj_carry( ch, target_name, ch ) ) == NULL )
	{
	    send_to_char( "You are not carrying that.\n\r", ch );
	    return;
	}

	vo = (void *) obj;
	target = TARGET_OBJ;
	break;

    case TAR_OBJ_CHAR_OFF:

       if (IS_SET(ch->in_room->room_flags, ROOM_SAFE)
       && (ch != victim))
         {
          send_to_char( "{RYou can only casts spells on yourself in Safe Rooms.{x\n\r", ch );
          return;
         }

	if (arg2[0] == '\0')
	{
	    if ((victim = ch->fighting) == NULL)
	    {
		send_to_char("Cast the spell on whom or what?\n\r",ch);
		return;
	    }
	
	    target = TARGET_CHAR;
	}
	else if ((victim = get_char_room(ch,target_name)) != NULL)
	{
	    target = TARGET_CHAR;
	}

	if (target == TARGET_CHAR) /* check the sanity of the attack */
	{
	    if(is_safe_spell(ch,victim,FALSE) && victim != ch)
	    {
		send_to_char("Not on that target.\n\r",ch);
		return;
	    }

            if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
            {
                send_to_char( "You can't do that on your own follower.\n\r",
                    ch );
                return;
            }

	    if (!IS_NPC(ch))
	      /* check_killer(ch,victim); */

	    vo = (void *) victim;
 	}
	else if ((obj = get_obj_here(ch,target_name)) != NULL)
	{
	    vo = (void *) obj;
	    target = TARGET_OBJ;
	}
	else
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}
	break; 

    case TAR_OBJ_CHAR_DEF:

        if (arg2[0] == '\0')
        {
            vo = (void *) ch;
            target = TARGET_CHAR;                                                 
        }
        else if ((victim = get_char_room(ch,target_name)) != NULL)
        {
            vo = (void *) victim;
            target = TARGET_CHAR;
	}
	else if ((obj = get_obj_carry(ch,target_name,ch)) != NULL)
	{
	    vo = (void *) obj;
	    target = TARGET_OBJ;
	}
	else
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}
	break;
    }
	    
    if ( !IS_NPC(ch) && ch->mana < mana )
    {
	send_to_char( "You don't have enough mana.\n\r", ch );
	return;
    }
      
    if ( str_cmp( skill_table[sn].name, "ventriloquate" ) )
	say_spell( ch, sn );
    
    if(skill_table[sn].spell_fun==spell_word_of_recall && IS_REMORT(ch))
        WAIT_STATE(ch, 6);
    else
        WAIT_STATE( ch, skill_table[sn].beats );
      
    if ( number_percent( ) > get_skill(ch,sn) )
    {
	send_to_char( "You lost your concentration.\n\r", ch );
	check_improve(ch,sn,FALSE,1);
	ch->mana -= mana / 2;
    }
    else
    { 
        ch->mana -= mana;

        if (IS_NPC(ch) 
        || (class_table[ch->class].fMana))
          {
            (*skill_table[sn].spell_fun) ( sn, ch->level, ch, vo,target);
          }
        else
          {
            (*skill_table[sn].spell_fun) (sn, (3 * (ch->level/4)), ch, vo,target);
          }

        check_improve(ch,sn,TRUE,1);
    }


    if ((skill_table[sn].target == TAR_CHAR_OFFENSIVE
    ||   (skill_table[sn].target == TAR_OBJ_CHAR_OFF && target == TARGET_CHAR))
    &&   victim != ch
    &&   victim->master != ch)
    {
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;

	for ( vch = ch->in_room->people; vch; vch = vch_next )
	{
	    vch_next = vch->next_in_room;
	    if ( victim == vch && victim->fighting == NULL )
	    {	
                /* check_killer(victim,ch); */
 		multi_hit( victim, ch, TYPE_UNDEFINED );
		break;
	    }
	}
    }

    return;
}



/*
 * Cast spells at targets using a magical object.
 */
void obj_cast_spell( int sn, int level, CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *obj )
{
    void *vo;
    int target = TARGET_NONE;

    if ( sn <= 0 )
	return;

    if ( sn >= MAX_SKILL || skill_table[sn].spell_fun == 0 )
    {
	bug( "Obj_cast_spell: bad sn %d.", sn );
	return;
    }

    switch ( skill_table[sn].target )
    {
    default:
	bug( "Obj_cast_spell: bad target for sn %d.", sn );
	return;

    case TAR_IGNORE:
	vo = NULL;
	break;

    case TAR_CHAR_OFFENSIVE:
	if ( victim == NULL )
	    victim = ch->fighting;
	if ( victim == NULL )
	{
	    send_to_char( "You can't do that.\n\r", ch );
	    return;
	}
	if (is_safe(ch,victim) && ch != victim)
	{
	    send_to_char("Something isn't right...\n\r",ch);
	    return;
	}
	vo = (void *) victim;
	target = TARGET_CHAR;
	break;

    case TAR_CHAR_DEFENSIVE:
    case TAR_CHAR_SELF:
	if ( victim == NULL )
	    victim = ch;
	vo = (void *) victim;
	target = TARGET_CHAR;
	break;

    case TAR_OBJ_INV:
	if ( obj == NULL )
	{
	    send_to_char( "You can't do that.\n\r", ch );
	    return;
	}
	vo = (void *) obj;
	target = TARGET_OBJ;
	break;

    case TAR_OBJ_CHAR_OFF:
        if ( victim == NULL && obj == NULL)
		{
			if (ch->fighting != NULL)
			victim = ch->fighting;
			else
			{
			send_to_char("You can't do that.\n\r",ch);
			return;
			}
		}

	    if (victim != NULL)
	    {
		if (is_safe_spell(ch,victim,FALSE) && ch != victim)
		{
		    send_to_char("Somehting isn't right...\n\r",ch);
		    return;
		}

		vo = (void *) victim;
		target = TARGET_CHAR;
	    }
	    else
	    {
	    	vo = (void *) obj;
	    	target = TARGET_OBJ;
	    }
        break;


    case TAR_OBJ_CHAR_DEF:
	if (victim == NULL && obj == NULL)
	{
	    vo = (void *) ch;
	    target = TARGET_CHAR;
	}
	else if (victim != NULL)
	{
	    vo = (void *) victim;
	    target = TARGET_CHAR;
	}
	else
	{
	    vo = (void *) obj;
	    target = TARGET_OBJ;
	}
	
	break;
    }

    target_name = "";
    (*skill_table[sn].spell_fun) ( sn, level, ch, vo,target);

    

    if ( (skill_table[sn].target == TAR_CHAR_OFFENSIVE
    ||   (skill_table[sn].target == TAR_OBJ_CHAR_OFF && target == TARGET_CHAR))
    &&   victim != ch
    &&   victim->master != ch )
    {
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;

	for ( vch = ch->in_room->people; vch; vch = vch_next )
	{
	    vch_next = vch->next_in_room;
	    if ( victim == vch && victim->fighting == NULL )
	    {
		/* check_killer(victim,ch); */
		multi_hit( victim, ch, TYPE_UNDEFINED );
		break;
	    }
	}
    }

    return;
}



/*
 * Spell functions.
 */





void spell_armor( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
    {
	if (victim == ch)
	  send_to_char("\n\r{cYou already have {CARMOR{c.{x\n\r",ch);
	else
	  act("\n\r{C$N {cis already protected by an {CARMOR {cspell.{x\n\r",ch,NULL,victim,TO_CHAR);
	return;
    }
    af.where	 = TO_AFFECTS;
    af.type      = sn;
    af.level	 = ch->level;
    af.duration  = 8+(ch->level/3);
    af.modifier  = -20;
    af.location  = APPLY_AC;
    af.bitvector = 0;
    affect_to_char( victim, &af );
    send_to_char( "\n\r{cYou are now protected by an {CARMOR {cspell.{x\n\r", victim );
    if ( ch != victim )
	act("\n\r{C$N {cis protected by your {CARMOR{c spell.{x\n\r",ch,NULL,victim,TO_CHAR);
    return;
}


void spell_bless( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    AFFECT_DATA af;

    /* deal with the object case first */
    if (target == TARGET_OBJ)
    {
	obj = (OBJ_DATA *) vo;
	if (IS_OBJ_STAT(obj,ITEM_BLESS))
	{
	    act("$p is already blessed.",ch,obj,NULL,TO_CHAR);
	    return;
	}

	if (IS_OBJ_STAT(obj,ITEM_EVIL))
	{
	    AFFECT_DATA *paf;

	    paf = affect_find(obj->affected,gsn_curse);
	    if (!saves_dispel(level,paf != NULL ? paf->level : obj->level,0))
	    {
		if (paf != NULL)
		    affect_remove_obj(obj,paf);
		act("$p glows a pale blue.",ch,obj,NULL,TO_ALL);
		REMOVE_BIT(obj->extra_flags,ITEM_EVIL);
		return;
	    }
	    else
	    {
		act("The evil of $p is too powerful for you to overcome.",
		    ch,obj,NULL,TO_CHAR);
		return;
	    }
	}
	
	af.where	= TO_OBJECT;
	af.type		= sn;
	af.level	= level;
	af.duration	= 6 + level;
	af.location	= APPLY_SAVES;
	af.modifier	= (level/2);
	af.bitvector	= ITEM_BLESS;
	affect_to_obj(obj,&af);

	act("$p glows with a holy aura.",ch,obj,NULL,TO_ALL);

	if (obj->wear_loc != WEAR_INVENTORY)
	    ch->saving_throw -= 1;
	return;
    }

    /* character target */
    victim = (CHAR_DATA *) vo;


    if ( victim->position == POS_FIGHTING || is_affected( victim, sn ) )
    {
	if (victim == ch)
	  send_to_char("You are already blessed.\n\r",ch);
	else
	  act("$N already has divine favor.",ch,NULL,victim,TO_CHAR);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    af.duration  = (level/2);
    af.location  = APPLY_HITROLL;
    af.modifier  = level / 4;
    af.bitvector = 0;
    affect_to_char( victim, &af );

    af.location  = APPLY_SAVES;
    af.modifier  = 0 - level / 8;
    affect_to_char( victim, &af );
    send_to_char( "You feel righteous.\n\r", victim );
    if ( ch != victim )
	act("You grant $N the favor of your god.",ch,NULL,victim,TO_CHAR);
    return;
}



void spell_blindness( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (IS_CLASS(ch,CLASS_CLERIC))
      {
    if ( IS_AFFECTED(victim, AFF_BLIND) || saves_spell(level,victim,DAM_HOLY))
	return;
      }
    else
      {
    if ( IS_AFFECTED(victim, AFF_BLIND) || saves_spell(level,victim,DAM_MAGIC))
	return;
      }



    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.location  = APPLY_HITROLL;
    af.modifier  = -4;
    af.duration  = 1+level;
    af.bitvector = AFF_BLIND;
    affect_to_char( victim, &af );
    send_to_char( "You are blinded!\n\r", victim );
    act("$n appears to be blinded.",victim,NULL,NULL,TO_ROOM);
    return;
}







void spell_call_lightning( int sn, int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    int dam;

    if ( !IS_OUTSIDE(ch) )
    {
	send_to_char( "You must be out of doors.\n\r", ch );
	return;
    }

    if ( weather_info.sky < SKY_RAINING )
    {
	send_to_char( "You need bad weather.\n\r", ch );
	return;
    }

    dam = dice(level/2, 8);

    send_to_char( "Mota's lightning strikes your foes!\n\r", ch );
    act( "$n calls Mota's lightning to strike $s foes!",
	ch, NULL, NULL, TO_ROOM );

    for ( vch = char_list; vch != NULL; vch = vch_next )
    {
	vch_next	= vch->next;
	if ( vch->in_room == NULL )
	    continue;
	if ( vch->in_room == ch->in_room )
	{
           if (ch->class == CLASS_CLERIC)
             {
	    if ( vch != ch && ( IS_NPC(ch) ? !IS_NPC(vch) : IS_NPC(vch) ) )
		damage( ch, vch, saves_spell( level,vch,DAM_LIGHTNING) 
		? dam / 2 : dam, sn,DAM_LIGHTNING,TRUE);
             }
           else
             {
 	    if ( vch != ch && ( IS_NPC(ch) ? !IS_NPC(vch) : IS_NPC(vch) ) )
		damage( ch, vch, saves_spell( level,vch,DAM_MAGIC) 
		? dam / 2 : dam, sn,DAM_MAGIC,TRUE);
             }

	    continue;
	}

	if ( vch->in_room->area == ch->in_room->area
	&&   IS_OUTSIDE(vch)
	&&   IS_AWAKE(vch) )
	    send_to_char( "Lightning flashes in the sky.\n\r", vch );
    }

    return;
}

/* RT calm spell stops all fighting in the room */

void spell_calm( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *vch;
    int mlevel = 0;
    int count = 0;
    int high_level = 0;    
    int chance;
    AFFECT_DATA af;

    /* get sum of all mobile levels in the room */
    for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
    {
	if (vch->position == POS_FIGHTING)
	{
	    count++;
	    if (IS_NPC(vch))
	      mlevel += vch->level;
	    else
	      mlevel += vch->level/2;
	    high_level = UMAX(high_level,vch->level);
	}
    }

    /* compute chance of stopping combat */
    chance = 4 * level - high_level + 2 * count;

    if (IS_IMMORTAL(ch)) /* always works */
      mlevel = 0;

    if (number_range(0, chance) >= mlevel)  /* hard to stop large fights */
    {
	for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
   	{
	    if (IS_NPC(vch) && (IS_SET(vch->imm_flags,IMM_MAGIC) ||
				IS_SET(vch->act,ACT_UNDEAD)))
	      return;

	    if (IS_AFFECTED(vch,AFF_CALM) || IS_AFFECTED(vch,AFF_BERSERK)
	    ||  is_affected(vch,skill_lookup("frenzy")))
	      return;
	    
	    send_to_char("A wave of calm passes over you.\n\r",vch);

	    if (vch->fighting || vch->position == POS_FIGHTING)
	      stop_fighting(vch,FALSE);


	    af.where = TO_AFFECTS;
	    af.type = sn;
  	    af.level = level;
	    af.duration = level/4;
	    af.location = APPLY_HITROLL;
	    if (!IS_NPC(vch))
	      af.modifier = -5;
	    else
	      af.modifier = -2;
	    af.bitvector = AFF_CALM;
	    affect_to_char(vch,&af);

	    af.location = APPLY_DAMROLL;
	    affect_to_char(vch,&af);
	}
    }
}

void spell_cancellation( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    bool found = FALSE;
 
    level += 2; 

    if ((!IS_NPC(ch) && IS_NPC(victim) && 
	 !(IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim) ) ||
        (IS_NPC(ch) && !IS_NPC(victim)) )
    {
	send_to_char("You failed, try dispel magic.\n\r",ch);
	return;
    }

    if (check_dispel(level,victim,skill_lookup("armor")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("bless")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("blindness")))
    {
        found = TRUE;
        act("$n is no longer blinded.",victim,NULL,NULL,TO_ROOM);
    }

    if (check_dispel(level,victim,skill_lookup("calm")))
    {
	found = TRUE;
	act("$n no longer looks so peaceful...",victim,NULL,NULL,TO_ROOM);
    }
 
    if (check_dispel(level,victim,skill_lookup("change sex")))
    {
        found = TRUE;
        act("$n looks more like $mself again.",victim,NULL,NULL,TO_ROOM);
    }
 
    if (check_dispel(level,victim,skill_lookup("charm person")))
    {
        found = TRUE;
        stop_follower( victim );
        victim->leader = NULL;
        act("$n regains $s free will.",victim,NULL,NULL,TO_ROOM);
    }
 
    if (check_dispel(level,victim,skill_lookup("chill touch")))
    {
        found = TRUE;
        act("$n looks warmer.",victim,NULL,NULL,TO_ROOM);
    }
 
    if (check_dispel(level,victim,skill_lookup("curse")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("detect evil")))
        found = TRUE;

    if (check_dispel(level,victim,skill_lookup("detect good")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("detect hidden")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("detect invis")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("detect magic")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("faerie fire")))
    {
        act("$n's outline fades.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("fly")))
    {
        act("$n falls to the ground!",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }

    if (check_dispel(level,victim,skill_lookup("frenzy")))
    {
	act("$n no longer looks so wild.",victim,NULL,NULL,TO_ROOM);;
	found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("giant strength")))
    {
        act("$n no longer looks so mighty.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }

    if (check_dispel(level,victim,skill_lookup("haste")))
    {
	act("$n is no longer moving so quickly.",victim,NULL,NULL,TO_ROOM);
	found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("infravision")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("invis")))
    {
        act("$n fades into existance.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("mass invis")))
    {
        act("$n fades into existance.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("pass door")))
        found = TRUE;

    if (check_dispel(level,victim,skill_lookup("protection evil")))
        found = TRUE;

    if (check_dispel(level,victim,skill_lookup("protection good")))
        found = TRUE; 
 
    if (check_dispel(level,victim,skill_lookup("sanctuary")))
    {
        act("The white aura around $n's body vanishes.",
            victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }

    if (check_dispel(level,victim,skill_lookup("golden aura")))
      {
      act( "The golden aura around $n's body fades away.",
            victim, NULL, NULL, TO_ROOM );
      found = TRUE;
      }
 
    if (check_dispel(level,victim,skill_lookup("shield")))
    {
        act("The shield protecting $n vanishes.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("sleep")))
        found = TRUE;

    if (check_dispel(level,victim,skill_lookup("slow")))
    {
        act("$n is no longer moving so slowly.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("stone skin")))
    {
        act("$n's skin regains its normal texture.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("weaken")))
    {
        act("$n looks stronger.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (found)
        send_to_char("Ok.\n\r",ch);
    else
        send_to_char("Spell failed.\n\r",ch);
}

void spell_cause_light( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    damage( ch, (CHAR_DATA *) vo, dice(1, 8) + level / 3, sn,DAM_HOLY,TRUE);
    return;
}



void spell_cause_critical(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    damage( ch, (CHAR_DATA *) vo, dice(3, 8) + level - 6, sn,DAM_HOLY,TRUE);
    return;
}



void spell_cause_serious(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    damage( ch, (CHAR_DATA *) vo, dice(2, 8) + level / 2, sn,DAM_HOLY,TRUE);
    return;
}


	  

void spell_change_sex( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ))
    {
	if (victim == ch)
	  send_to_char("You've already been changed.\n\r",ch);
	else
	  act("$N has already had $s(?) sex changed.",ch,NULL,victim,TO_CHAR);
	return;
    }
    if (saves_spell(level , victim,DAM_HOLY))
	return;	
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = 2 * level;
    af.location  = APPLY_SEX;
    do
    {
	af.modifier  = number_range( 0, 2 ) - victim->sex;
    }
    while ( af.modifier == 0 );
    af.bitvector = 0;
    affect_to_char( victim, &af );
    send_to_char( "You feel different.\n\r", victim );
    act("$n doesn't look like $mself anymore...",victim,NULL,NULL,TO_ROOM);
    return;
}



void spell_charm_person( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo, *mch;
    AFFECT_DATA af;
    int max_charm = 0;


 if (!IS_AFFECTED(victim, AFF_CHARM))
   {
    if (is_safe(ch,victim)) 
      return;

    if (victim->master != NULL)
    {
	send_to_char( "\n\r{WThat target was just recently charmed, maybe a different one would be better.\n\r{x", ch);
	return;
    }

    if ( !IS_NPC (victim))
    {
	send_to_char( "\n\r{WOther players may not be charmed!\n\r{x", ch);
	return;
    }

    if (victim->perm_stat[STAT_INT] > ( ch->perm_stat[STAT_INT] + 2) )
    {
	send_to_char( "\n\r{RYour victim is much to intelligent to be enslaved by you!\n\r{x", ch);
	return;
    }


   if (ch->class == 0 || ch->class == 2)
     {
    for (mch = char_list; mch != NULL; mch = mch->next) 
       {
        if ((max_charm <= 2)
        && (ch->pet != NULL))  
           max_charm += 1;
      
        if ((max_charm >= 3)
        && (ch->pet != NULL))
          {
         send_to_char( "\n\r{RAttempting to take control of more than 3 minds{x\n\r",ch);
         send_to_char( "{Rcauses you to lose control of the Magical Energies{x\n\r",ch);
         send_to_char( "{Rreleasing {WONE{r of your adoring fans!{x\n\r",ch);
	 nuke_pets(ch);
           return;
          }

        if (mch->master == ch || mch->leader == ch)
          {
           if (max_charm <= 2 )  
             { max_charm += 1; }
          }

        }
     }
   else
     {
      if (ch->pet != NULL)
          {
         send_to_char( "\n\r{ROnly MAGEs & PSIs can CONTROL more than ONE slave!{x\n\r",ch);
	 nuke_pets(ch);
           return;
          }
     }

    if ( victim == ch )
    {
	send_to_char( "\n\r{CYou like yourself even better!{x\n\r", ch );
	return;
    }

    if (IS_CLASS(ch, CLASS_CLERIC))
       {
    if ( IS_AFFECTED(victim, AFF_CHARM)
    ||   IS_AFFECTED(ch, AFF_CHARM)
    ||   level < victim->level
    ||   IS_SET(victim->imm_flags,IMM_CHARM)
    ||   saves_spell( level, victim,DAM_HOLY) )
	return;
       }
     else
       {
    if ( IS_AFFECTED(victim, AFF_CHARM)
    ||   IS_AFFECTED(ch, AFF_CHARM)
    ||   level < victim->level
    ||   IS_SET(victim->imm_flags,IMM_CHARM)
    ||   saves_spell( level, victim,DAM_MAGIC) )
	return;
       }

    if ( victim->master )
      { stop_follower( victim ); }

    add_follower( victim, ch );
    victim->leader = ch;


   if (ch->class == 0 || ch->class == 2)
     {
      if ((max_charm >= 2)
      && (ch->pet == NULL))
        ch->pet = victim;
     }
   else
     {
      ch->pet = victim;
     }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    af.duration  = number_fuzzy( level / 4 );
    af.location  = 0;
    af.modifier  = 0;
    af.bitvector = AFF_CHARM;
    affect_to_char( victim, &af );
    act( "\n\r{cIsn't {C$n {cjust so {CNICE{c?{x\n\r", ch, NULL, victim, TO_VICT );
    if ( ch != victim )
	act("\n\r{C$N {clooks at you with adoring eyes.{x\n\r",ch,NULL,victim,TO_CHAR);
    return;
  }
 else
  {
   send_to_char("{RThey are already under the influence of a CHARM!{x\n\r",ch);
    return;
  }
return;
}


void spell_continual_light(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    OBJ_DATA *light;

    if (target_name[0] != '\0')  /* do a glow on some object */
    {
	light = get_obj_carry(ch,target_name,ch);
	
	if (light == NULL)
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}

	if (IS_OBJ_STAT(light,ITEM_GLOW))
	{
	    act("$p is already glowing.",ch,light,NULL,TO_CHAR);
	    return;
	}

	SET_BIT(light->extra_flags,ITEM_GLOW);
	act("$p glows with a white light.",ch,light,NULL,TO_ALL);
	return;
    }

    light = create_object( get_obj_index( OBJ_VNUM_LIGHT_BALL ), 0 );
    obj_to_room( light, ch->in_room );
    act( "$n twiddles $s thumbs and $p appears.",   ch, light, NULL, TO_ROOM );
    act( "You twiddle your thumbs and $p appears.", ch, light, NULL, TO_CHAR );
    return;
}



void spell_control_weather(int sn,int level,CHAR_DATA *ch,void *vo,int target) 
{
    if ( !str_cmp( target_name, "better" ) )
	weather_info.change += dice( level / 3, 4 );
    else if ( !str_cmp( target_name, "worse" ) )
	weather_info.change -= dice( level / 3, 4 );
    else
	send_to_char ("Do you want it to get better or worse?\n\r", ch );

    send_to_char( "Ok.\n\r", ch );
    return;
}



void spell_create_food( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    OBJ_DATA *mushroom;

    mushroom = create_object( get_obj_index( OBJ_VNUM_MUSHROOM ), 0 );
    mushroom->value[0] = level / 2;
    mushroom->value[1] = level;
    obj_to_room( mushroom, ch->in_room );
    act( "$p suddenly appears.", ch, mushroom, NULL, TO_ROOM );
    act( "$p suddenly appears.", ch, mushroom, NULL, TO_CHAR );
    return;
}

void spell_create_rose( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    OBJ_DATA *rose;
    rose = create_object(get_obj_index(OBJ_VNUM_ROSE), 0);
    act("$n has created a beautiful red rose.",ch,rose,NULL,TO_ROOM);
    send_to_char("You create a beautiful red rose.\n\r",ch);
    obj_to_char(rose,ch);
    return;
}

void spell_create_spring(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    OBJ_DATA *spring;

    spring = create_object( get_obj_index( OBJ_VNUM_SPRING ), 0 );
    spring->timer = level;
    obj_to_room( spring, ch->in_room );
    act( "$p flows from the ground.", ch, spring, NULL, TO_ROOM );
    act( "$p flows from the ground.", ch, spring, NULL, TO_CHAR );
    return;
}



void spell_create_water( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    int water;

    if ( obj->item_type != ITEM_DRINK_CON )
    {
	send_to_char( "It is unable to hold water.\n\r", ch );
	return;
    }

    if ( obj->value[2] != LIQ_WATER && obj->value[1] != 0 )
    {
	send_to_char( "It contains some other liquid.\n\r", ch );
	return;
    }

    water = UMIN(
		level * (weather_info.sky >= SKY_RAINING ? 4 : 2),
		obj->value[0] - obj->value[1]
		);
  
    if ( water > 0 )
    {
	obj->value[2] = LIQ_WATER;
	obj->value[1] += water;
	if ( !is_name( "water", obj->name ) )
	{
	    char buf[MAX_STRING_LENGTH];

	    sprintf( buf, "%s water", obj->name );
	    free_string( obj->name );
	    obj->name = str_dup( buf );
	}
	act( "$p is filled.", ch, obj, NULL, TO_CHAR );
    }

    return;
}



void spell_cure_blindness(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    if ( !is_affected( victim, gsn_blindness ) && !is_affected(victim, skill_lookup("fire breath")) )
    {
        if (victim == ch)
          send_to_char("You aren't blind.\n\r",ch);
        else
          act("$N doesn't appear to be blinded.",ch,NULL,victim,TO_CHAR);
        return;
    }
 
    if (check_dispel(level,victim,gsn_blindness) || check_dispel(level,victim,skill_lookup("fire breath")))
    {
        send_to_char("Their vision returns!\n\r", ch);
        send_to_char( "Your vision returns!\n\r", victim );
        act("$n is no longer blinded.",victim,NULL,NULL,TO_ROOM);
    }
    else
        send_to_char("Spell failed.\n\r",ch);
}



void spell_cure_critical( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int heal;

    heal = dice(8, 8) + UMIN(ch->level, 55);
    victim->hit = UMIN( victim->hit + heal, victim->max_hit );
    update_pos( victim );
    send_to_char( "You feel better!\n\r", victim );
    if ( ch != victim )
	send_to_char( "Ok.\n\r", ch );
    return;
}

/* RT added to cure plague */
void spell_cure_disease( int sn, int level, CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    if ( !is_affected( victim, gsn_plague ) )
    {
        if (victim == ch)
          send_to_char("You aren't ill.\n\r",ch);
        else
          act("$N doesn't appear to be diseased.",ch,NULL,victim,TO_CHAR);
        return;
    }
    
    if (check_dispel(level,victim,gsn_plague))
    {
	send_to_char("Your sores vanish.\n\r",victim);
	act("$n looks relieved as $s sores vanish.",victim,NULL,NULL,TO_ROOM);
    }
    else
	send_to_char("Spell failed.\n\r",ch);
}



void spell_cure_light( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int heal;

    heal = dice(2, 8) + UMIN(ch->level, 15);
    victim->hit = UMIN( victim->hit + heal, victim->max_hit );
    update_pos( victim );
    send_to_char( "You feel better!\n\r", victim );
    if ( ch != victim )
	send_to_char( "Ok.\n\r", ch );
    return;
}



void spell_cure_poison( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
 
    if ( !is_affected( victim, gsn_poison ) )
    {
        if (victim == ch)
          send_to_char("You aren't poisoned.\n\r",ch);
        else
          act("$N doesn't appear to be poisoned.",ch,NULL,victim,TO_CHAR);
        return;
    }
 
    if (check_dispel(level,victim,gsn_poison))
    {
        send_to_char("A warm feeling runs through your body.\n\r",victim);
        act("$n looks much better.",victim,NULL,NULL,TO_ROOM);
    }
    else
        send_to_char("Spell failed.\n\r",ch);
}

void spell_cure_serious( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int heal;

    heal = dice(4, 8) + UMIN(ch->level, 30);
    victim->hit = UMIN( victim->hit + heal, victim->max_hit );
    update_pos( victim );
    send_to_char( "You feel better!\n\r", victim );
    if ( ch != victim )
	send_to_char( "Ok.\n\r", ch );
    return;
}



void spell_curse( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    AFFECT_DATA af;

    /* deal with the object case first */
    if (target == TARGET_OBJ)
    {
        obj = (OBJ_DATA *) vo;
        if (IS_OBJ_STAT(obj,ITEM_EVIL))
        {
            act("$p is already filled with evil.",ch,obj,NULL,TO_CHAR);
            return;
        }

        if (IS_OBJ_STAT(obj,ITEM_BLESS))
        {
            AFFECT_DATA *paf;

            paf = affect_find(obj->affected,skill_lookup("bless"));

            if (!saves_dispel(level,paf != NULL ? paf->level : obj->level,0))
            {
                if (paf != NULL)
                    affect_remove_obj(obj,paf);
                act("$p glows with a red aura.",ch,obj,NULL,TO_ALL);
                REMOVE_BIT(obj->extra_flags,ITEM_BLESS);
                return;
            }
            else
            {
                act("The holy aura of $p is too powerful for you to overcome.",
                    ch,obj,NULL,TO_CHAR);
                return;
            }
        }

        af.where        = TO_OBJECT;
        af.type         = sn;
        af.level        = level;
        af.duration     = 2 * level;
        af.location     = APPLY_HITROLL;
        af.modifier     = -3 * (level / 2);
        af.bitvector    = ITEM_EVIL;
        affect_to_obj(obj,&af);

        af.location     = APPLY_SAVES;
        af.modifier     = +(3 * (level / 2));
        affect_to_obj(obj,&af);

        act("$p glows with a malevolent aura.",ch,obj,NULL,TO_ALL);

	if (obj->wear_loc != WEAR_INVENTORY)
	    ch->saving_throw += 1;
        return;
    }

    /* character curses */
    victim = (CHAR_DATA *) vo;

    if (IS_CLASS(ch, CLASS_CLERIC))
      {
    if (IS_AFFECTED(victim,AFF_CURSE) || saves_spell(level,victim,DAM_HOLY))
	return;
      }
     else
      {
    if (IS_AFFECTED(victim,AFF_CURSE) || saves_spell(level,victim,DAM_MAGIC))
	return;
      }
      

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = 2*level;
    af.location  = APPLY_HITROLL;
    af.modifier  = -3 * (level / 2);
    af.bitvector = AFF_CURSE;
    affect_to_char( victim, &af );

    af.location  = APPLY_SAVES;
    af.modifier  = (3 * (level / 2));
    affect_to_char( victim, &af );

    send_to_char( "You feel unclean.\n\r", victim );
    if ( ch != victim )
	act("$N looks very uncomfortable.",ch,NULL,victim,TO_CHAR);
    return;
}

/* RT replacement demonfire spell */

void spell_demonfire(int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    if ( !IS_NPC(ch) && !IS_EVIL(ch) )
    {
        victim = ch;
	send_to_char("The demons turn upon you!\n\r",ch);
    }

/*
    ch->alignment = UMAX(-1000, ch->alignment - 50);
*/
    if (victim != ch)
    {
	act("$n calls forth the demons of Hell upon $N!",
	    ch,NULL,victim,TO_ROOM);
        act("$n has assailed you with the demons of Hell!",
	    ch,NULL,victim,TO_VICT);
	send_to_char("You conjure forth the demons of hell!\n\r",ch);
    }
    dam = dice( level, 14 );

    if (IS_CLASS(ch, CLASS_CLERIC))
      {
    if ( saves_spell( level, victim,DAM_NEGATIVE) )
        dam /= 2;
    damage( ch, victim, dam, sn, DAM_NEGATIVE ,TRUE);
      }
    else
      {
    if ( saves_spell( level, victim,DAM_MAGIC) )
        dam /= 2;
    damage( ch, victim, dam, sn, DAM_MAGIC ,TRUE);
      }


    spell_curse(gsn_curse, 3 * level / 4, ch, (void *) victim,TARGET_CHAR);
}

void spell_detect_evil( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_DETECT_EVIL) )
    {
	if (victim == ch)
	  send_to_char("You can already sense evil.\n\r",ch);
	else
	  act("$N can already detect evil.",ch,NULL,victim,TO_CHAR);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    af.duration  = level;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_DETECT_EVIL;
    affect_to_char( victim, &af );
    send_to_char( "Your eyes tingle.\n\r", victim );
    if (victim != ch)
{ act("\n\r{C$N is now affected by DETECT EVIL.{x\n\r",ch,NULL,victim,TO_CHAR); }
    return;
}


void spell_detect_good( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
 
    if ( IS_AFFECTED(victim, AFF_DETECT_GOOD) )
    {
        if (victim == ch)
          send_to_char("You can already sense good.\n\r",ch);
        else
          act("$N can already detect good.",ch,NULL,victim,TO_CHAR);
        return;
    }
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_DETECT_GOOD;
    affect_to_char( victim, &af );
    send_to_char( "Your eyes tingle.\n\r", victim );
    if (victim != ch)
{ act("\n\r{C$N is now affected by DETECT GOOD.{x\n\r",ch,NULL,victim,TO_CHAR); } 
    return;
}



void spell_detect_hidden(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;


    if ( is_affected( victim, sn ) )
    {
        if (victim == ch)
          send_to_char("You are already as alert as you can be. \n\r",ch);
        else
          act("$N can already sense hidden lifeforms.",ch,NULL,victim,TO_CHAR);
        return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_DETECT_HIDDEN;
    affect_to_char( victim, &af );
    send_to_char( "Your awareness improves.\n\r", victim );
    if ( ch != victim )
        act("\n\r{C$N is now affected by DETECT HIDDEN.{x\n\r",ch,NULL,victim,TO_CHAR);
    return;
}



void spell_detect_invis( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;


    if ( is_affected( victim, sn ) )
    {
        if (victim == ch)
          send_to_char("\n\r{RYou can already see invisible.{x\n\r",ch);
        else
          act("\n\r{r$N {Rcan already see invisible things.{x\n\r",ch,NULL,victim,TO_CHAR);
        return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_DETECT_INVIS;
    affect_to_char( victim, &af );
    send_to_char( "\n\r{CYour eyes tingle.{x\n\r", victim );
    if ( ch != victim )
        act("\n\r{C$N can now SEE INVISIBLE.{x\n\r",ch,NULL,victim,TO_CHAR);
    return;
}



void spell_detect_magic( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_DETECT_MAGIC) )
    {
        if (victim == ch)
          send_to_char("You can already sense magical auras.\n\r",ch);
        else
          act("$N can already detect magic.",ch,NULL,victim,TO_CHAR);
        return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    af.duration  = level;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_DETECT_MAGIC;
    affect_to_char( victim, &af );
    send_to_char( "Your eyes tingle.\n\r", victim );
    if ( ch != victim )
        act("\n\r{C$N is now affected by DETECT MAGIC.{x\n\r",ch,NULL,victim,TO_CHAR);
    return;
}



void spell_detect_poison( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;

    if ( obj->item_type == ITEM_DRINK_CON || obj->item_type == ITEM_FOOD )
    {
	if ( obj->value[3] != 0 )
	    send_to_char( "You smell poisonous fumes.\n\r", ch );
	else
	    send_to_char( "It looks delicious.\n\r", ch );
    }
    else
    {
	send_to_char( "It doesn't look poisoned.\n\r", ch );
    }

    return;
}



void spell_dispel_evil( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
  
    if ( !IS_NPC(ch) && IS_EVIL(ch) )
	victim = ch;
  
    if ( IS_GOOD(victim) )
    {
	act( "Mota protects $N.", ch, NULL, victim, TO_ROOM );
	return;
    }

    if ( IS_NEUTRAL(victim) )
    {
	act( "$N does not seem to be affected.", ch, NULL, victim, TO_CHAR );
	return;
    }

    if (victim->hit > (ch->level * 4))
      dam = dice( level, 4 );
    else
      dam = UMAX(victim->hit, dice(level,4));

    if (IS_CLASS(ch, CLASS_CLERIC))
      {
    if ( saves_spell( level, victim,DAM_HOLY) )
	dam /= 2;
    damage( ch, victim, dam, sn, DAM_HOLY ,TRUE);
    return;
      }
    else
      {
    if ( saves_spell( level, victim,DAM_MAGIC) )
	dam /= 2;
    damage( ch, victim, dam, sn, DAM_MAGIC ,TRUE);
    return;
      }

}


void spell_dispel_good( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
 
    if ( !IS_NPC(ch) && IS_GOOD(ch) )
        victim = ch;
 
    if ( IS_EVIL(victim) )
    {
        act( "$N is protected by $S evil.", ch, NULL, victim, TO_ROOM );
        return;
    }
 
    if ( IS_NEUTRAL(victim) )
    {
        act( "$N does not seem to be affected.", ch, NULL, victim, TO_CHAR );
        return;
    }
 
    if (victim->hit > (ch->level * 4))
      dam = dice( level, 4 );
    else
      dam = UMAX(victim->hit, dice(level,4));

    if (IS_CLASS(ch, CLASS_CLERIC))
      {
    if ( saves_spell( level, victim,DAM_HOLY) )
	dam /= 2;
    damage( ch, victim, dam, sn, DAM_HOLY ,TRUE);
    return;
      }
    else
      {
    if ( saves_spell( level, victim,DAM_MAGIC) )
	dam /= 2;
    damage( ch, victim, dam, sn, DAM_MAGIC ,TRUE);
    return;
      }

    return;
}


/* modified for enhanced use */

void spell_dispel_magic( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    bool found = FALSE;



    if (IS_CLASS(ch, CLASS_CLERIC))
      {
    if (saves_spell(level, victim,DAM_HOLY))
    {
	send_to_char( "You feel a brief tingling sensation.\n\r",victim);
	send_to_char( "You failed.\n\r", ch);
	return;
    }
      }
  else
      {
    if (saves_spell(level, victim,DAM_MAGIC))
    {
	send_to_char( "You feel a brief tingling sensation.\n\r",victim);
	send_to_char( "You failed.\n\r", ch);
	return;
    }
      }



    /* begin running through the spells */ 

    if (check_dispel(level,victim,skill_lookup("armor")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("bless")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("blindness")))
        act("$n is no longer blinded.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("calm")))
        act("$n no longer looks so peaceful...",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("change sex")))
        act("$n looks more like $mself again.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
 
    if (IS_NPC(victim))
    {
        if (check_dispel(level,victim,skill_lookup("charm person")))
        {
            stop_follower( victim );
            victim->leader = NULL;
            act("$n regains $s free will.",victim,NULL,NULL,TO_ROOM);
            found = TRUE;
        }
    }

    if (check_dispel(level,victim,skill_lookup("chill touch")))
        act("$n looks warmer.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("curse")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("detect evil")))
        found = TRUE;

    if (check_dispel(level,victim,skill_lookup("detect good")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("detect hidden")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("detect invis")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("detect magic")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("faerie fire")))
        act("$n's outline fades.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("fly")))
        act("$n falls to the ground!",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("frenzy")))
        act("$n no longer looks so wild.",victim,NULL,NULL,TO_ROOM);;
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("giant strength")))
        act("$n no longer looks so mighty.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("haste")))
        act("$n is no longer moving so quickly.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("infravision")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("invis")))
        act("$n fades into existance.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("mass invis")))
        act("$n fades into existance.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("pass door")))
        found = TRUE;
 

    if (check_dispel(level,victim,skill_lookup("protection evil")))
        found = TRUE;

    if (check_dispel(level,victim,skill_lookup("protection good")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("golden aura")))
      act( "The golden aura around $N's body fades away.", victim, NULL, NULL, TO_ROOM );
      found = TRUE;

    if (check_dispel(level,victim,skill_lookup("sanctuary")))
        act("The white aura around $n's body vanishes.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;

    if (check_dispel(level,victim,skill_lookup("shield")))
        act("The shield protecting $n vanishes.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("sleep")))
        found = TRUE;

    if (check_dispel(level,victim,skill_lookup("slow")))
        act("$n is no longer moving so slowly.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("stone skin")))
        act("$n's skin regains its normal texture.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("weaken")))
        act("$n looks stronger.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
 
    if (found)
        send_to_char("Ok.\n\r",ch);
    else
        send_to_char("Spell failed.\n\r",ch);
	return;
}

void spell_earthquake( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;

    send_to_char( "The earth trembles beneath your feet!\n\r", ch );
    act( "$n makes the earth tremble and shiver.", ch, NULL, NULL, TO_ROOM );

    for ( vch = char_list; vch != NULL; vch = vch_next )
    {
	vch_next	= vch->next;
	if ( vch->in_room == NULL )
	    continue;
	if ( vch->in_room == ch->in_room )
	{
	    if ( vch != ch && !is_safe_spell(ch,vch,TRUE))
		{
			if (IS_AFFECTED(vch,AFF_FLYING))
				damage(ch,vch,0,sn,DAM_BASH,TRUE);
			else
				damage( ch,vch,level + dice(2, 8), sn, DAM_BASH,TRUE);
		}
		continue;
	}

	if ( vch->in_room->area == ch->in_room->area )
	    send_to_char( "The earth trembles and shivers.\n\r", vch );
    }

    return;
}

/* New Enchant Armor */
void spell_enchant_armor( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    char *msg = "$p is imbued in a magenta aura.";
    int chance = number_percent( );
  
    if ( get_skill( ch, sn ) == 0 )
      {
       send_to_char( "You are unable to make the magic work.\n\r", ch );
       return;
      }
 
    if (obj->item_type != ITEM_ARMOR)
      {
       send_to_char("That isn't an armor.\n\r",ch);
       return;
      }

    if (obj->wear_loc != -1)
      {
       send_to_char("The item must be carried to be enchanted.\n\r",ch);
       return;
      }

    if ( IS_SET( obj->extra_flags, ITEM_MAGIC ) )
      {
       send_to_char( "This item is already enchanted.\n\r", ch );
       return;
      }

    if (IS_OBJ_STAT(obj,ITEM_BLESS))
      chance -= 10;
    
    if (IS_OBJ_STAT(obj,ITEM_GLOW))
      chance -= 5;

    if ( chance > ( get_skill( ch, sn ) ) )
      {  
       send_to_char( "Nothing seemed to happen.\n\r", ch );
       return;
      }
      else 
    if ( chance <= ( get_skill( ch, sn ) * .05 ))
      {
       obj->value[0] *= 2;
       obj->value[1] *= 2;
       obj->value[2] *= 2;
       obj->value[3] *= 2;
       SET_BIT( obj->extra_flags, ITEM_MAGIC );
       msg = "$p is imbued in a radiant magenta aura.";          
       act( msg, ch, obj, NULL, TO_CHAR );
       return;
      }
      else 
    if ( chance <= ( get_skill( ch, sn ) * .15 ))
      {
       obj->value[0] *= 1.75;
       obj->value[1] *= 1.75;
       obj->value[2] *= 1.75;
       obj->value[3] *= 1.75;
       SET_BIT( obj->extra_flags, ITEM_MAGIC );
       msg = "$p is imbued in a shimmering magenta aura.";          
       act( msg, ch, obj, NULL, TO_CHAR );
       return;
      }
      else
    if ( chance <= ( get_skill( ch, sn ) * .25 ))
      {
       obj->value[0] *= 1.5;
       obj->value[1] *= 1.5;
       obj->value[2] *= 1.5;
       obj->value[3] *= 1.5;
       SET_BIT( obj->extra_flags, ITEM_MAGIC );
       msg = "$p is imbued in a bright magenta aura.";          
       act( msg, ch, obj, NULL, TO_CHAR );
       return;
      }
      else
      {
       obj->value[0] *= 1.25;
       obj->value[1] *= 1.25;
       obj->value[2] *= 1.25;
       obj->value[3] *= 1.25;
       SET_BIT( obj->extra_flags, ITEM_MAGIC );
       msg = "$p is imbued in a magenta aura.";          
       act( msg, ch, obj, NULL, TO_CHAR );
       return;
      }
 return;   
}

void spell_enchant_weapon(int sn,int level,CHAR_DATA *ch, void *vo,int target)
{
  OBJ_DATA *obj = (OBJ_DATA *) vo;
  char buf[MAX_STRING_LENGTH];
  AFFECT_DATA *paf;
  int result;
  int hit_bonus, dam_bonus;
  bool hit_found = FALSE, dam_found = FALSE;

  if (obj->item_type != ITEM_WEAPON)
    {
        send_to_char("\n\r{RThat isn't a weapon!{x\n\r",ch);
        return;
    }

  if (obj->wear_loc != -1)
    {
        send_to_char("\n\r{RThe item must be in your INVENTORY to be enchanted!{x\n\r",ch);
        return;
    }
  if ( IS_SET( obj->extra_flags, ITEM_MAGIC ) )
        {
        send_to_char( "\n\r{RThis item is already MAGICAL!{x\n\r", ch );
        return;
        }

  /* this means they have no bonus */
  hit_bonus = 0;
  dam_bonus = 0;
  for ( paf = obj->affected; paf; paf = paf->next )
        {
        if ( paf->location == APPLY_HITROLL )
      { hit_found = TRUE; continue; }
    if ( paf->location == APPLY_DAMROLL )
      { dam_found = TRUE; continue; }
        }
  result = number_percent();

  if (result < 10) /* item disenchanted */
    {
    AFFECT_DATA *paf_next;

    act("\n\r{C$p {Cglows brightly, then fades...oops.{x\n\r",ch,obj,NULL,TO_CHAR);
    act("\n\r{C$p {Cglows brightly, then fades.\n\r{x",ch,obj,NULL,TO_ROOM);
    obj->enchanted = TRUE;
        /* remove all affects */
        for (paf = obj->affected; paf != NULL; paf = paf_next)
      {
            paf_next = paf->next;
            free_affect(paf);
      }
    obj->affected = NULL;
        /* clear all flags */
        obj->extra_flags = 0;
          return;
    }

  /* okay, move all the old flags into new vectors if we have to */
  if (!obj->enchanted)
    {
        AFFECT_DATA *af_new;
        obj->enchanted = TRUE;

        for (paf = obj->pIndexData->affected; paf != NULL; paf = paf->next)
      {
            af_new = new_affect();

            af_new->next = obj->affected;
            obj->affected = af_new;

            af_new->where       = paf->where;
            af_new->type        = UMAX(0,paf->type);
            af_new->level       = paf->level;
            af_new->duration    = paf->duration;
            af_new->location    = paf->location;
            af_new->modifier    = paf->modifier;
            af_new->bitvector   = paf->bitvector;
        }
    }

  if (result <= 90)  /* success! */
    {
     if (ch->level >= 499)
       { 
        if (ch->level == 500)
          {
           hit_bonus = (level-350) / 5;
           dam_bonus = (level-350) / 10;
          }
        else
        if (ch->level == 499)
          {
           if(!strstr(obj->name, ch->name))
             { 
              sprintf(buf, "%s %s", obj->name, ch->name);
              free_string(obj->name);
              obj->name = str_dup(buf);
             }
           hit_bonus = (level-359) / 5;
           dam_bonus = (level-359) / 10;
          }

        act("\n\r{B$p {Bglows blue.\n\r{x",ch,obj,NULL,TO_CHAR);
        act("\n\r{B$p {Bglows blue.{x\n\r",ch,obj,NULL,TO_ROOM);
        SET_BIT(obj->extra_flags, ITEM_MAGIC);
       }
      else
       {

        if (ch->level >= IMMORTAL)
          {
           if(!strstr(obj->name, ch->name))
             { 
              sprintf(buf, "%s %s", obj->name, ch->name);
              free_string(obj->name);
              obj->name = str_dup(buf);
             }
          } 

        act("\n\r{B$p {Bglows blue.{x\n\r",ch,obj,NULL,TO_CHAR);
        act("\n\r{B$p {Bglows blue.{x\n\r",ch,obj,NULL,TO_ROOM);
        SET_BIT(obj->extra_flags, ITEM_MAGIC);
        hit_bonus = level / 5;
        dam_bonus = level / 10;
       }
    }
  else  /* exceptional enchant */
    {

     if (ch->level >= 499)
       { 
        if (ch->level == 500)
          { 
           hit_bonus = (level-350) / 2;
           dam_bonus = (level-350) / 4;
          }

        if (ch->level == 499)
          { 
           if(!strstr(obj->name, ch->name))
             { 
              sprintf(buf, "%s %s", obj->name, ch->name);
              free_string(obj->name);
              obj->name = str_dup(buf);
             }
           hit_bonus = (level-359) / 4;
           dam_bonus = (level-359) / 6.666667;
          }
        act("\n\r{B$p {Bglows a {WBRILLIANT {BBLUE!{x\n\r",ch,obj,NULL,TO_CHAR);
        act("\n\r{B$p {Bglows a {WBRILLIANT {BBLUE!{x\n\r",ch,obj,NULL,TO_ROOM);
        SET_BIT(obj->extra_flags,ITEM_MAGIC);
        SET_BIT(obj->extra_flags,ITEM_GLOW);
      }
     else
       { 
        if (ch->level >= IMMORTAL)
          {
           if(!strstr(obj->name, ch->name))
             { 
              sprintf(buf, "%s %s", obj->name, ch->name);
              free_string(obj->name);
              obj->name = str_dup(buf);
             }
          } 

        hit_bonus = level / 4;
        dam_bonus = level / 6.666667;

        act("\n\r{B$p {Bglows a {WBRILLIANT {BBLUE!{x\n\r",ch,obj,NULL,TO_CHAR);
        act("\n\r{B$p {Bglows a {WBRILLIANT {BBLUE!{x\n\r",ch,obj,NULL,TO_ROOM);
        SET_BIT(obj->extra_flags,ITEM_MAGIC);
        SET_BIT(obj->extra_flags,ITEM_GLOW);
      }
    }

  /* now add the enchantments */

  if (obj->level < LEVEL_HERO - 1)
        obj->level = UMIN(LEVEL_HERO - 1,obj->level + 1);
  if (dam_found)
    {
        for ( paf = obj->affected; paf != NULL; paf = paf->next)
      {
            if ( paf->location == APPLY_DAMROLL )
        {
        paf->type = sn;
        paf->modifier += dam_bonus;
        paf->level = UMAX(paf->level,level);
        if (paf->modifier > 4)
                      SET_BIT(obj->extra_flags,ITEM_HUM);
        }
      }
    }
  else /* add a new affect */
    {
    paf = new_affect();
        paf->where      = TO_OBJECT;
    paf->type   = sn;
    paf->level  = level;
    paf->duration       = -1;
    paf->location       = APPLY_DAMROLL;
    paf->modifier       = dam_bonus;
    paf->bitvector  = 0;
    paf->next   = obj->affected;
    obj->affected       = paf;
    }

  if (hit_found)
    {
    for ( paf = obj->affected; paf != NULL; paf = paf->next)
      {
      if ( paf->location == APPLY_HITROLL )
        {
        paf->type = sn;
        paf->modifier += hit_bonus;
        paf->level = UMAX(paf->level,level);
        if (paf->modifier > 4)
          SET_BIT(obj->extra_flags,ITEM_HUM);
        }
        }
    }
  else /* add a new affect */
    {
    paf = new_affect();
    paf->type       = sn;
    paf->level      = level;
    paf->duration   = -1;
    paf->location   = APPLY_HITROLL;
    paf->modifier   = hit_bonus;
    paf->bitvector  = 0;
    paf->next       = obj->affected;
    obj->affected   = paf;
    }
  return;

}



/*
 * Drain XP, MANA, HP.
 * Caster gains HP.
 */
void spell_energy_drain( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

/*
    if (victim != ch)
        ch->alignment = UMAX(-1000, ch->alignment - 50);
*/

    if (IS_CLASS(ch, CLASS_CLERIC))
      {
    if ( saves_spell( level, victim,DAM_HOLY) )
    {
	send_to_char("You feel a momentary chill.\n\r",victim);  	
	return;
    }
      }
   else
      {
    if ( saves_spell( level, victim,DAM_MAGIC) )
    {
	send_to_char("You feel a momentary chill.\n\r",victim);  	
	return;
    }
      }


    if ( victim->level <= 2 )
    {
	dam		 = ch->hit + 1;
    }
    else
    {
	gain_exp( victim, 0 - number_range( level/2, 3 * level / 2 ) );
	victim->mana	/= 2;
	victim->move	/= 2;
	dam		 = dice(1, level);
	ch->hit		+= dam;
    }

    send_to_char("You feel your life slipping away!\n\r",victim);
    send_to_char("Wow....what a rush!\n\r",ch);


    if (IS_CLASS(ch, CLASS_CLERIC))  
      damage( ch, victim, dam, sn, DAM_HOLY ,TRUE);
    else
      damage( ch, victim, dam, sn, DAM_MAGIC ,TRUE);

    return;
}


void spell_flamestrike( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = dice(6 + level / 2, 8);


    if (IS_CLASS(ch, CLASS_CLERIC))
      {
    if ( saves_spell( level, victim,DAM_HOLY) )
	dam /= 2;
    damage( ch, victim, dam, sn, DAM_HOLY ,TRUE); 
      }
    else
      {
    if ( saves_spell( level, victim,DAM_FIRE) )
	dam /= 2;
    damage( ch, victim, dam, sn, DAM_FIRE ,TRUE); 
      }

    return;
}



void spell_faerie_fire( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_FAERIE_FIRE) )
	return;
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    af.duration  = level;
    af.location  = APPLY_AC;
    af.modifier  = 2 * level;
    af.bitvector = AFF_FAERIE_FIRE;
    affect_to_char( victim, &af );
    send_to_char( "You are surrounded by a pink outline.\n\r", victim );
    act( "$n is surrounded by a pink outline.", victim, NULL, NULL, TO_ROOM );
    return;
}



void spell_faerie_fog( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *ich;

    act( "$n conjures a cloud of purple smoke.", ch, NULL, NULL, TO_ROOM );
    send_to_char( "You conjure a cloud of purple smoke.\n\r", ch );

    for ( ich = ch->in_room->people; ich != NULL; ich = ich->next_in_room )
    {
	if (ich->invis_level > 0)
	    continue;


    if (IS_CLASS(ch, CLASS_CLERIC))
      {
	if ( ich == ch || saves_spell( level, ich,DAM_HOLY) )
	    continue;
      }
    else
      {
	if ( ich == ch || saves_spell( level, ich,DAM_MAGIC) )
	    continue;
      }

	affect_strip ( ich, gsn_invisibility		);
	affect_strip ( ich, gsn_obfuscate		);
	affect_strip ( ich, gsn_mass_invis		);
	affect_strip ( ich, gsn_sneak			);
	affect_strip ( ich, gsn_shadow_form		);
	affect_strip ( ich, gsn_chameleon_power		);
	REMOVE_BIT   ( ich->affected_by, AFF_HIDE	);
	REMOVE_BIT   ( ich->affected_by, AFF_INVISIBLE	);
	REMOVE_BIT   ( ich->affected_by, AFF_SNEAK	);
	act( "$n is revealed!", ich, NULL, NULL, TO_ROOM );
	send_to_char( "You are revealed!\n\r", ich );
    }

    return;
}

void spell_floating_disc( int sn, int level,CHAR_DATA *ch,void *vo,int target )
{
    OBJ_DATA *disc, *floating;

    floating = get_eq_char(ch,WEAR_FLOAT);
    if (floating != NULL && IS_OBJ_STAT(floating,ITEM_NOREMOVE))
    {
	act("You can't remove $p.",ch,floating,NULL,TO_CHAR);
	return;
    }

    disc = create_object(get_obj_index(OBJ_VNUM_DISC), 0);
    disc->value[0]	= ch->level * 10; /* 10 pounds per level capacity */
    disc->value[3]	= ch->level * 5; /* 5 pounds per level max per item */
    disc->timer		= ch->level * 2 - number_range(0,level / 2); 

    act("$n has created a floating black disc.",ch,NULL,NULL,TO_ROOM);
    send_to_char("You create a floating disc.\n\r",ch);
    obj_to_char(disc,ch);
    wear_obj(ch,disc,TRUE);
    return;
}


void spell_fly( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_FLYING) )
    {
	if (victim == ch)
	  send_to_char("You are already airborne.\n\r",ch);
	else
	  act("$N doesn't need your help to fly.",ch,NULL,victim,TO_CHAR);
	return;
    }
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    af.duration  = level + 3;
    af.location  = 0;
    af.modifier  = 0;
    af.bitvector = AFF_FLYING;
    affect_to_char( victim, &af );
    send_to_char( "Your feet rise off the ground.\n\r", victim );
    act( "$n's feet rise off the ground.", victim, NULL, NULL, TO_ROOM );
    return;
}

/* RT clerical berserking spell */

void spell_frenzy(int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (is_affected(victim,sn) || IS_AFFECTED(victim,AFF_BERSERK))
    {
	if (victim == ch)
	  send_to_char("You are already in a frenzy.\n\r",ch);
	else
	  act("$N is already in a frenzy.",ch,NULL,victim,TO_CHAR);
	return;
    }

    if (is_affected(victim,skill_lookup("calm")))
    {
	if (victim == ch)
	  send_to_char("Why don't you just relax for a while?\n\r",ch);
	else
	  act("$N doesn't look like $e wants to fight anymore.",
	      ch,NULL,victim,TO_CHAR);
	return;
    }

    if ((IS_GOOD(ch) 
    && !IS_GOOD(victim)) 
    || (IS_NEUTRAL(ch) 
    && !IS_NEUTRAL(victim)) 
    || (IS_EVIL(ch) && !IS_EVIL(victim)))
    {
	act("Your god doesn't seem to like $N",ch,NULL,victim,TO_CHAR);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type 	 = sn;
    af.level	 = level;
    af.duration	 = level / 3;
    af.modifier  = level / 6;
    af.bitvector = 0;

    af.location  = APPLY_HITROLL;
    affect_to_char(victim,&af);

    af.location  = APPLY_DAMROLL;
    affect_to_char(victim,&af);

    af.modifier  = 10 * (level / 12);
    af.location  = APPLY_AC;
    affect_to_char(victim,&af);

    send_to_char("You are filled with holy wrath!\n\r",victim);
    act("$n gets a wild look in $s eyes!",victim,NULL,NULL,TO_ROOM);
}

/* RT ROM-style gate */
    
void spell_gate( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim;
    bool gate_pet;


    if (IS_CLASS(ch, CLASS_CLERIC))
      {
    if ( ( victim = get_char_world( ch, target_name ) ) == NULL
    ||   victim == ch
    ||   victim->in_room == NULL
    ||   !can_see_room(ch,victim->in_room) 
    ||   IS_SET(victim->in_room->room_flags, ROOM_SAFE)
    ||   IS_SET(victim->in_room->room_flags, ROOM_PRIVATE)
    ||   IS_SET(victim->in_room->room_flags, ROOM_SOLITARY)
    ||   IS_SET(victim->in_room->room_flags, ROOM_DRAGONPIT)
    ||   IS_SET(ch->in_room->room_flags, ROOM_DRAGONPIT)
    ||   IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
    ||   IS_SET(victim->in_room->area->area_flags, AREA_PROTO )
    ||   IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
    ||   victim->level >= level + 3
    ||   (is_clan(victim) && !is_same_clan(ch,victim))
    ||   (!IS_NPC(victim) && victim->level >= LEVEL_HERO) 
    ||   (IS_NPC(victim) && IS_SET(victim->imm_flags,IMM_SUMMON))
    ||   (IS_NPC(victim) && saves_spell( level, victim,DAM_HOLY) ) )
    {
        send_to_char( "You failed.\n\r", ch );
        return;
    }
     }
   else
      {
    if ( ( victim = get_char_world( ch, target_name ) ) == NULL
    ||   victim == ch
    ||   victim->in_room == NULL
    ||   !can_see_room(ch,victim->in_room) 
    ||   IS_SET(victim->in_room->room_flags, ROOM_SAFE)
    ||   IS_SET(victim->in_room->room_flags, ROOM_PRIVATE)
    ||   IS_SET(victim->in_room->room_flags, ROOM_SOLITARY)
    ||   IS_SET(victim->in_room->room_flags, ROOM_DRAGONPIT)
    ||   IS_SET(ch->in_room->room_flags, ROOM_DRAGONPIT)
    ||   IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
    ||   IS_SET(victim->in_room->area->area_flags, AREA_PROTO )
    ||   IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
    ||   victim->level >= level + 3
    ||   (is_clan(victim) && !is_same_clan(ch,victim))
    ||   (!IS_NPC(victim) && victim->level >= LEVEL_HERO) 
    ||   (IS_NPC(victim) && IS_SET(victim->imm_flags,IMM_SUMMON))
    ||   (IS_NPC(victim) && saves_spell( level, victim,DAM_MAGIC) ) )
    {
        send_to_char( "You failed.\n\r", ch );
        return;
    }
     }

   if (!IS_IMP(ch))
     {
      if (IS_IMMORTAL(ch))
        {
         send_to_char("\n\r{rIMMs can use {RGOTO{r so there is no reason to be {RGATEING{r.{x\n\r",ch);
         return;
        }
     }

	if ((IS_SET(victim->imm_flags,IMM_SUMMON)) && !IS_NPC(victim))
	{
		send_to_char("You failed.",ch);
		return;
	}
	
    // Stones of Wisdom Stuff
    if(!stones_try_moving(ch))
        return;

    if (ch->pet != NULL && ch->in_room == ch->pet->in_room)
	gate_pet = TRUE;
    else
	gate_pet = FALSE;
    
    act("$n steps through a gate and vanishes.",ch,NULL,NULL,TO_ROOM);
    send_to_char("You step through a gate and vanish.\n\r",ch);
    char_from_room(ch);
    char_to_room(ch,victim->in_room);

    act("$n has arrived through a gate.",ch,NULL,NULL,TO_ROOM);
    do_function(ch, &do_look, "auto");

    if (gate_pet)
    {
	act("$n steps through a gate and vanishes.",ch->pet,NULL,NULL,TO_ROOM);
	send_to_char("You step through a gate and vanish.\n\r",ch->pet);
	char_from_room(ch->pet);
	char_to_room(ch->pet,victim->in_room);
	act("$n has arrived through a gate.",ch->pet,NULL,NULL,TO_ROOM);
	do_function(ch->pet, &do_look, "auto");
    }
}



void spell_giant_strength(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
    {
	if (victim == ch)
	  send_to_char("You are already as strong as you can get!\n\r",ch);
	else
	  act("$N can't get any stronger.",ch,NULL,victim,TO_CHAR);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    af.duration  = level;
    af.location  = APPLY_STR;
    af.modifier  = 1 + (level >= 18) + (level >= 25) + (level >= 32);
    af.bitvector = 0;
    affect_to_char( victim, &af );
    send_to_char( "Your muscles surge with heightened power!\n\r", victim );
    act("$n's muscles surge with heightened power.",victim,NULL,NULL,TO_ROOM);
    return;
}



void spell_harm( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = UMAX(  20, victim->hit - dice(1,4) );


    if (IS_CLASS(ch, CLASS_CLERIC))
      {
    if ( saves_spell( level, victim,DAM_HOLY) )
	dam = UMIN( 50, dam / 2 );
    dam = UMIN( 100, dam );
    damage( ch, victim, dam, sn, DAM_HOLY ,TRUE);
    return;
      }
    else
      {
    if ( saves_spell( level, victim,DAM_MAGIC) )
	dam = UMIN( 50, dam / 2 );
    dam = UMIN( 100, dam );
    damage( ch, victim, dam, sn, DAM_MAGIC ,TRUE);
    return;
      }

}

/* RT haste spell */

void spell_haste( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
 
    if ( is_affected( victim, sn ) || IS_AFFECTED(victim,AFF_HASTE)
    ||   IS_SET(victim->off_flags,OFF_FAST))
    {
	if (victim == ch)
	  send_to_char("You can't move any faster!\n\r",ch);
 	else
	  act("$N is already moving as fast as $E can.",
	      ch,NULL,victim,TO_CHAR);
        return;
    }

    if (IS_AFFECTED(victim,AFF_SLOW))
    {
	if (!check_dispel(level,victim,skill_lookup("slow")))
	{
	    if (victim != ch)
	        send_to_char("Spell failed.\n\r",ch);
	    send_to_char("You feel momentarily faster.\n\r",victim);
	    return;
	}
        act("$n is moving less slowly.",victim,NULL,NULL,TO_ROOM);
        return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    if (victim == ch)
      af.duration  = level/2;
    else
      af.duration  = level/4;
    af.location  = APPLY_DEX;
    af.modifier  = 1 + (level >= 18) + (level >= 25) + (level >= 32);
    af.bitvector = AFF_HASTE;
    affect_to_char( victim, &af );
    send_to_char( "You feel yourself moving more quickly.\n\r", victim );
    act("$n is moving more quickly.",victim,NULL,NULL,TO_ROOM);
    if ( ch != victim )
        send_to_char( "Ok.\n\r", ch );
    return;
}



void spell_heal( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int heal;

    heal = 100 + ch->level;
    victim->hit = UMIN( victim->hit + heal, victim->max_hit );
    update_pos( victim );
    send_to_char( "A warm feeling fills your body.\n\r", victim );
    if ( ch != victim )
	send_to_char( "Ok.\n\r", ch );
    return;
}

void spell_heat_metal( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    OBJ_DATA *obj_lose, *obj_next;
    int dam = 0;
    bool fail = TRUE;
 
   if (!saves_spell(level + 2,victim,DAM_FIRE) 
   &&  !IS_SET(victim->imm_flags,IMM_FIRE))
   {
        for ( obj_lose = victim->carrying;
	      obj_lose != NULL; 
	      obj_lose = obj_next)
        {
	    obj_next = obj_lose->next_content;
            if ( number_range(1,2 * level) > obj_lose->level 
	    &&   !saves_spell(level,victim,DAM_FIRE)
	    &&   !IS_OBJ_STAT(obj_lose,ITEM_NONMETAL)
	    &&   !IS_OBJ_STAT(obj_lose,ITEM_FORTIFIED)
	    &&   !IS_OBJ_STAT(obj_lose,ITEM_FIREPROOF))
            {
                switch ( obj_lose->item_type )
                {
               	case ITEM_ARMOR:
		if (obj_lose->wear_loc != -1) /* remove the item */
		{
		    if (can_drop_obj(victim,obj_lose)
		    &&  (obj_lose->weight / 10) < 
			number_range(1,2 * get_curr_stat(victim,STAT_DEX))
		    &&  remove_obj( victim, obj_lose->wear_loc, TRUE ))
		    {
		        act("$n yelps and throws $p to the ground!",
			    victim,obj_lose,NULL,TO_ROOM);
		        act("You remove and drop $p before it burns you.",
			    victim,obj_lose,NULL,TO_CHAR);
			dam += (number_range(1,obj_lose->level) / 3);
                        obj_from_char(obj_lose);

                        if (IS_SET(victim->in_room->room_flags, ROOM_DRAGONPIT) || IS_SET(victim->pact, PLR_DRAGONPIT))
                           obj_to_char( obj_lose, victim );
                        else
                           obj_to_room( obj_lose, victim->in_room);

                        fail = FALSE;
                    }
		    else /* stuck on the body! ouch! */
		    {
			act("Your skin is seared by $p!",
			    victim,obj_lose,NULL,TO_CHAR);
			dam += (number_range(1,obj_lose->level));
			fail = FALSE;
		    }

		}
		else /* drop it if we can */
		{
		    if (can_drop_obj(victim,obj_lose))
		    {
                        act("$n yelps and throws $p to the ground!",
                            victim,obj_lose,NULL,TO_ROOM);
                        act("You and drop $p before it burns you.",
                            victim,obj_lose,NULL,TO_CHAR);
                        dam += (number_range(1,obj_lose->level) / 6);
                        obj_from_char(obj_lose);

                        if (IS_SET(victim->in_room->room_flags, ROOM_DRAGONPIT) || IS_SET(victim->pact, PLR_DRAGONPIT))
                           obj_to_char( obj_lose, victim ); 
                        else
                           obj_to_room( obj_lose, victim->in_room);

			fail = FALSE;
                    }
		    else /* cannot drop */
		    {
                        act("Your skin is seared by $p!",
                            victim,obj_lose,NULL,TO_CHAR);
                        dam += (number_range(1,obj_lose->level) / 2);
			fail = FALSE;
                    }
		}
                break;
                case ITEM_WEAPON:
		if (obj_lose->wear_loc != -1) /* try to drop it */
		{
		    if (IS_WEAPON_STAT(obj_lose,WEAPON_FLAMING))
			continue;

		    if (can_drop_obj(victim,obj_lose) 
		    &&  remove_obj(victim,obj_lose->wear_loc,TRUE))
		    {
			act("$n is burned by $p, and throws it to the ground.",
			    victim,obj_lose,NULL,TO_ROOM);
			send_to_char(
			    "You throw your red-hot weapon to the ground!\n\r",
			    victim);
			dam += 1;
			obj_from_char(obj_lose);

                        if (IS_SET(victim->in_room->room_flags, ROOM_DRAGONPIT) || IS_SET(victim->pact, PLR_DRAGONPIT))
                           obj_to_char( obj_lose, victim ); 
                        else
                           obj_to_room( obj_lose, victim->in_room);

			fail = FALSE;
		    }
		    else /* YOWCH! */
		    {
			send_to_char("Your weapon sears your flesh!\n\r",
			    victim);
			dam += number_range(1,obj_lose->level);
			fail = FALSE;
		    }
		}
                else /* drop it if we can */
                {
                    if (can_drop_obj(victim,obj_lose))
                    {
                        act("$n throws a burning hot $p to the ground!",
                            victim,obj_lose,NULL,TO_ROOM);
                        act("You and drop $p before it burns you.",
                            victim,obj_lose,NULL,TO_CHAR);
                        dam += (number_range(1,obj_lose->level) / 6);
                        obj_from_char(obj_lose);

                        if (IS_SET(victim->in_room->room_flags, ROOM_DRAGONPIT) || IS_SET(victim->pact, PLR_DRAGONPIT))
                           obj_to_char( obj_lose, victim ); 
                        else
                           obj_to_room( obj_lose, victim->in_room);

                        fail = FALSE;
                    }
                    else /* cannot drop */
                    {
                        act("Your skin is seared by $p!",
                            victim,obj_lose,NULL,TO_CHAR);
                        dam += (number_range(1,obj_lose->level) / 2);
                        fail = FALSE;
                    }
                }
                break;
		}
	    }
	}
    } 
    if (fail)
    {
        send_to_char("Your spell had no effect.\n\r", ch);
	send_to_char("You feel momentarily warmer.\n\r",victim);
    }
    else /* damage! */
    {
	if (saves_spell(level,victim,DAM_FIRE))
	    dam = 2 * dam / 3;
	damage(ch,victim,dam,sn,DAM_FIRE,TRUE);
    }
}

/* RT really nasty high-level attack spell */
void spell_holy_word(int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    int dam;
    int bless_num, curse_num, frenzy_num;
   
    bless_num = skill_lookup("bless");
    curse_num = skill_lookup("curse"); 
    frenzy_num = skill_lookup("frenzy");

    act("$n utters a word of divine power!",ch,NULL,NULL,TO_ROOM);
    send_to_char("You utter a word of divine power.\n\r",ch);
 
    for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
    {
        vch_next = vch->next_in_room;

	if ((IS_GOOD(ch) && IS_GOOD(vch)) ||
	    (IS_EVIL(ch) && IS_EVIL(vch)) ||
	    (IS_NEUTRAL(ch) && IS_NEUTRAL(vch)) )
	{
 	  send_to_char("You feel full more powerful.\n\r",vch);
	  spell_frenzy(frenzy_num,level,ch,(void *) vch,TARGET_CHAR); 
	  spell_bless(bless_num,level,ch,(void *) vch,TARGET_CHAR);
	}

	else if ((IS_GOOD(ch) && IS_EVIL(vch)) ||
		 (IS_EVIL(ch) && IS_GOOD(vch)) )
	{
	  if (!is_safe_spell(ch,vch,TRUE))
	  {
            spell_curse(curse_num,level,ch,(void *) vch,TARGET_CHAR);
	    send_to_char("You are struck down!\n\r",vch);
	    dam = dice(level,6);
	    damage(ch,vch,dam,sn,DAM_HOLY,TRUE);
	  }
	}

        else if (IS_NEUTRAL(ch))
	{
	  if (!is_safe_spell(ch,vch,TRUE))
	  {
            spell_curse(curse_num,level/2,ch,(void *) vch,TARGET_CHAR);
	    send_to_char("You are struck down!\n\r",vch);
	    dam = dice(level,4);
	    damage(ch,vch,dam,sn,DAM_HOLY,TRUE);
   	  }
	}
    }  
    
    send_to_char("You feel drained.\n\r",ch);
    ch->move = 0;
    ch->hit /= 2;
}

void spell_identify( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    char buf[MAX_STRING_LENGTH];
    AFFECT_DATA *paf;

   
    sprintf( buf," {cItem Name{w: {C%s\n\r", obj->pIndexData->short_descr);
    send_to_char( buf, ch );

    if (ch->level >= 100)
      {
       sprintf( buf,"{cItem Level{w: {C%d\n\r",obj->level);
       send_to_char( buf, ch );
      }
     else 
    if (ch->level >= obj->level)
      {
      sprintf( buf,"{cItem Level{w: {C%d\n\r",obj->level);
      send_to_char( buf, ch );
      }

    sprintf( buf," {cItem Type{w: {C%s\n\r", item_name(obj->item_type));
    send_to_char( buf, ch );

    if (ch->level >= 100)
      {
    sprintf( buf,"{cProperties{w: {C%s\n\r",extra_bit_name( obj->extra_flags ));
    send_to_char( buf, ch );
      }    
    else
    if (ch->level >= obj->level)
      {
    sprintf( buf,"{cProperties{w: {C%s\n\r",extra_bit_name( obj->extra_flags ));
    send_to_char( buf, ch );
      }    

    sprintf( buf,"  {cMaterial{w: {C%s\n\r",material_table[obj->material_type].desc);
    send_to_char( buf, ch );

    sprintf( buf," {cCondition{w: {C%d\n\r",obj->pIndexData->condition);
    send_to_char( buf, ch );

    sprintf( buf,"    {cWeight{w: {C%d lbs\n\r",obj->weight);
    send_to_char( buf, ch );

    if (ch->level >= 100)
      {
    sprintf( buf,"     {cValue{w: {C%d{x\n\r\n\r",obj->cost);
    send_to_char( buf, ch );
      }
    else
    if (ch->level >= obj->level)
      {
    sprintf( buf,"     {cValue{w: {C%d{x\n\r\n\r",obj->cost);
    send_to_char( buf, ch );
      }

    switch ( obj->item_type )
    {
    case ITEM_SCROLL: 
    case ITEM_POTION:
    case ITEM_PILL:

       if (ch->level >= 100)
         {
	sprintf( buf, "{cSpell Level{w: {C%d  {cEmbedded Spells{w: ", obj->value[0] );
	send_to_char( buf, ch );

	if ( obj->value[1] >= 0 && obj->value[1] < MAX_SKILL )
	{
	    send_to_char( " {c'{W", ch );
	    send_to_char( skill_table[obj->value[1]].name, ch );
	    send_to_char( "{c'", ch );
	}

	if ( obj->value[2] >= 0 && obj->value[2] < MAX_SKILL )
	{
	    send_to_char( " {c'{W", ch );
	    send_to_char( skill_table[obj->value[2]].name, ch );
	    send_to_char( "{c'", ch );
	}

	if ( obj->value[3] >= 0 && obj->value[3] < MAX_SKILL )
	{
	    send_to_char( " {C'{W", ch );
	    send_to_char( skill_table[obj->value[3]].name, ch );
	    send_to_char( "{c'", ch );
	}

	if (obj->value[4] >= 0 && obj->value[4] < MAX_SKILL)
	{
	    send_to_char(" {c'{W",ch);
	    send_to_char(skill_table[obj->value[4]].name,ch);
	    send_to_char("{c'",ch);
	}

	send_to_char( "{x\n\r", ch );
	break;
      }
    else
      {
        if (ch->level >= obj->value[0])
         {
	sprintf( buf, "{cSpell Level{w: {C%d  {cEmbedded Spells{w: ", obj->value[0] );
	send_to_char( buf, ch );

	if ( obj->value[1] >= 0 && obj->value[1] < MAX_SKILL )
	{
	    send_to_char( " {c'{W", ch );
	    send_to_char( skill_table[obj->value[1]].name, ch );
	    send_to_char( "{c'", ch );
	}

	if ( obj->value[2] >= 0 && obj->value[2] < MAX_SKILL )
	{
	    send_to_char( " {c'{W", ch );
	    send_to_char( skill_table[obj->value[2]].name, ch );
	    send_to_char( "{c'", ch );
	}

	if ( obj->value[3] >= 0 && obj->value[3] < MAX_SKILL )
	{
	    send_to_char( " {C'{W", ch );
	    send_to_char( skill_table[obj->value[3]].name, ch );
	    send_to_char( "{c'", ch );
	}

	if (obj->value[4] >= 0 && obj->value[4] < MAX_SKILL)
	{
	    send_to_char(" {c'{W",ch);
	    send_to_char(skill_table[obj->value[4]].name,ch);
	    send_to_char("{c'",ch);
	}

	send_to_char( "{x\n\r", ch );
       }
     else
	{
	    send_to_char("{RThis ITEM is beyond your ability to IDENTIFY completely{x\n\r",ch);
	}
	break;
     }


    case ITEM_WAND: 
    case ITEM_STAFF: 


       if (ch->level >= 100)
         {
	if ( obj->value[3] >= 0 && obj->value[3] < MAX_SKILL )
          {
	sprintf( buf, "{cSpell Effect{w: {C%s  {cLevel{w: {C%d  {cCharges{w: {C%d{x",
	   skill_table[obj->value[3]].name, obj->value[0], obj->value[2] );
	send_to_char( buf, ch );
	send_to_char( "{x\n\r", ch );
	break;
         }
      }
   else
    {
       if (ch->level >= obj->value[0])
         {
	if ( obj->value[3] >= 0 && obj->value[3] < MAX_SKILL )
          {
	sprintf( buf, "{cSpell Effect{w: {C%s  {cLevel{w: {C%d  {cCharges{w: {C%d{x",
	   skill_table[obj->value[3]].name, obj->value[0], obj->value[2] );
	send_to_char( buf, ch );
	send_to_char( "{x\n\r", ch );
         }
        }
     else
	{
	    send_to_char("{RThis ITEM is beyond your ability to IDENTIFY completely{x\n\r",ch);
	}
	break;
     }


    case ITEM_DRINK_CON:
        sprintf(buf,"{cLiquid Type{w: {C%s  {cLiquid Color{w: {C%s{x\n\r",
            liq_table[obj->value[2]].liq_name,
            liq_table[obj->value[2]].liq_color);
        send_to_char(buf,ch);
        break;

    case ITEM_CONTAINER:
    
     if (ch->level >= 100)
        {
	sprintf(buf,
"         {cCapacity{w: {C%d{c#\n\r   Maximum Weight{w: {C%d{c#\n\r       Properties{w: {C%s{x\n\r",
	    obj->value[0], obj->value[3], cont_bit_name(obj->value[1]));
	send_to_char(buf,ch);

	if (obj->value[4] != 100)
	{
	    sprintf(buf,"{cWeight Multiplier{w: {C%d{c%%{x\n\r",
		obj->value[4]);
	    send_to_char(buf,ch);
	}
	break;
     }
 else 
   {
     if (ch->level >= obj->level)
        {
	sprintf(buf,
"         {cCapacity{w: {C%d{c#\n\r   Maximum Weight{w: {C%d{c#\n\r       Properties{w: {C%s{x\n\r",
	    obj->value[0], obj->value[3], cont_bit_name(obj->value[1]));
	send_to_char(buf,ch);

	if (obj->value[4] != 100)
	{
	    sprintf(buf,"{cWeight Multiplier{w: {C%d{c%%{x\n\r",
		obj->value[4]);
	    send_to_char(buf,ch);
	}
       }
     else
	{
	    send_to_char("{RThis ITEM is beyond your ability to IDENTIFY completely{x\n\r",ch);
	}
	break;
    }

		
    case ITEM_WEAPON:
 	send_to_char("     {cWeapon Class{w:{C ",ch);
	switch (obj->value[0])
	{
	    case(WEAPON_SWORD)  : send_to_char("Sword\n\r",ch);	break;	
	    case(WEAPON_DAGGER) : send_to_char("Dagger\n\r",ch);	break;
	    case(WEAPON_SPEAR)	: send_to_char("Spear/Staff\n\r",ch);	break;
	    case(WEAPON_MACE) 	: send_to_char("Mace/Club\n\r",ch);	break;
	    case(WEAPON_AXE)	: send_to_char("Axe\n\r",ch);		break;
	    case(WEAPON_FLAIL)	: send_to_char("Flail\n\r",ch);	break;
	    case(WEAPON_WHIP)	: send_to_char("Whip\n\r",ch);		break;
	    case(WEAPON_POLEARM): send_to_char("Polearm\n\r",ch);	break;
	    case(WEAPON_EXOTIC) : send_to_char("Exotic\n\r",ch);	break;
	    default		: send_to_char("{RUNKNOWN\n\r",ch);	break;
 	}

if (ch->level >= 100)
   {
sprintf(buf,"      {cDamage Type{w: {C%s{x\n\r",
(obj->value[3] > 0 && obj->value[3] < MAX_DAMAGE_MESSAGE) ? attack_table[obj->value[3]].noun :
"{R!!UNDEFINED!!{x");
send_to_char(buf,ch);
   }
else
if (ch->level >= obj->level)
   {
sprintf(buf,"      {cDamage Type{w: {C%s{x\n\r",
(obj->value[3] > 0 && obj->value[3] < MAX_DAMAGE_MESSAGE) ? attack_table[obj->value[3]].noun :
"{R!!UNDEFINED!!{x");
send_to_char(buf,ch);
   }

/*
	if (obj->pIndexData->new_format)
	    sprintf(buf,"    {cDamage Amount{w: {C%d{cd{C%d {D({WAVERAGE{w: {R%d{D){x\n\r",
		obj->value[1],obj->value[2],
		(1 + obj->value[2]) * obj->value[1] / 2);
	else
	    sprintf( buf, "     {cDamage Range{w: {C%d {cto {C%d {D({WAVERAGE{w: {R%d{D){x\n\r",
	    	obj->value[1], obj->value[2],
	    	( obj->value[1] + obj->value[2] ) / 2 );
*/
            if (ch->level >= 100)
              {
	       sprintf( buf, "     {cDamage Range{w: {C%d {cto {C%d {D({WAVERAGE{w: {R%d{D){x\n\r",
	    	obj->value[1], (obj->value[1] * obj->value[2]),
		((obj->value[1]) * ( (obj->value[2] + 1) / 2)) );
  	       send_to_char( buf, ch );
              }
         else
           {
            if (ch->level >= obj->level)
              {
	       sprintf( buf, "     {cDamage Range{w: {C%d {cto {C%d {D({WAVERAGE{w: {R%d{D){x\n\r",
	    	obj->value[1], (obj->value[1] * obj->value[2]),
		((obj->value[1]) * ( (obj->value[2] + 1) / 2)) );
  	       send_to_char( buf, ch );
              }
            else
   	      {
	       send_to_char("{RThis ITEM is beyond your ability to IDENTIFY completely{x\n\r",ch);
	      }
            }


        
        if (obj->value[4])  /* weapon flags */
        {

         if (ch->level >= 100)
           {
            sprintf(buf,"{cWeapon Properties{w: {C%s{x\n\r",weapon_bit_name(obj->value[4]));
            send_to_char(buf,ch);
           }
        else
         if (ch->level >= obj->level)
           {
            sprintf(buf,"{cWeapon Properties{w: {C%s{x\n\r",weapon_bit_name(obj->value[4]));
            send_to_char(buf,ch);
           }
           else
  	    {
	     send_to_char("{RThis ITEM is beyond your ability to IDENTIFY completely{x\n\r",ch);
	    }
   	  break;
        }

    case ITEM_ARMOR:

     if (ch->level >= 100)
       {
	sprintf( buf, 
	"{cArmor Classes{w: {CPierce{w: {W%d   {CBash{w: {W%d   {CSlash{w: {W%d   {CExotic{w: {C%d{x\n\r", 
	    obj->value[0], obj->value[1], obj->value[2], obj->value[3] );
	send_to_char( buf, ch );
       }
     else
     if (ch->level >= obj->level)
       {
	sprintf( buf, 
	"{cArmor Classes{w: {CPierce{w: {W%d   {CBash{w: {W%d   {CSlash{w: {W%d   {CExotic{w: {C%d{x\n\r", 
	    obj->value[0], obj->value[1], obj->value[2], obj->value[3] );
	send_to_char( buf, ch );
       }
     else
        {
            send_to_char("{RThis ITEM is beyond your ability to IDENTIFY completely{x\n\r",ch);
        }

	break;
    }

    if (!obj->enchanted)
    for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
    {
	if ( paf->location != APPLY_NONE && paf->modifier != 0 )
	{
	    sprintf( buf, "{cAdded Modifier To{w: {C%s  {cAmount{w: {C%d{x\n\r",
		affect_loc_name( paf->location ), paf->modifier );
	    send_to_char(buf,ch);
            if (paf->bitvector)
            {
                switch(paf->where)
                {
                    case TO_AFFECTS:
                        sprintf(buf,"{cAdded Affect Of{w: {C%s{x\n",
                            affect_bit_name(paf->bitvector));
                        break;
                    case TO_OBJECT:
                        sprintf(buf,"{cAdded OBJ Flag Of{w: {C%s{x\n",
                            extra_bit_name(paf->bitvector));
                        break;
                    case TO_IMMUNE:
                        sprintf(buf,"{cAdded Immunity To{w: {C%s\n",
                            imm_bit_name(paf->bitvector));
                        break;
                    case TO_RESIST:
                        sprintf(buf,"{cAdded Resistance To{w: {C%s{x\n\r",
                            imm_bit_name(paf->bitvector));
                        break;
                    case TO_VULN:
                        sprintf(buf,"{cAdded Vulnerability To{w: {C%s{x\n\r",
                            imm_bit_name(paf->bitvector));
                        break;
                    default:
                        sprintf(buf,"{RUNKNOWN BIT {W%d {r: {W%d{c - {CReport to ARIOCH{x\n\r",
                            paf->where,paf->bitvector);
                        break;
                }
	        send_to_char( buf, ch );
	    }
	}
    }

    for ( paf = obj->affected; paf != NULL; paf = paf->next )
    {
	if ( paf->location != APPLY_NONE && paf->modifier != 0 )
	{
	    sprintf( buf, "{cAdded Modifier To{w: {C%s  {cAmount{w: {C%d{x",
	    	affect_loc_name( paf->location ), paf->modifier );
	    send_to_char( buf, ch );

            if ( paf->duration > -1)
                sprintf(buf,"  {cHours{w: {C%d{x\n\r",paf->duration);
            else
                sprintf(buf,"\n\r");
	    send_to_char(buf,ch);
            if (paf->bitvector)
            {
                switch(paf->where)
                {
                    case TO_AFFECTS:
                        sprintf(buf,"{cAdded Affect Of{w: {C%s{x\n",
                            affect_bit_name(paf->bitvector));
                        break;
                    case TO_OBJECT:
                        sprintf(buf,"{cAdded OBJ Flag Of{w: {C%s{x\n",
                            extra_bit_name(paf->bitvector));
                        break;
		    case TO_WEAPON:
            sprintf(buf,"{cWeapon Properties{w: {C%s{x\n\r",weapon_bit_name(obj->value[4]));
			break;
                    case TO_IMMUNE:
                        sprintf(buf,"{cAdded Immunity To{w: {C%s\n",
                            imm_bit_name(paf->bitvector));
                        break;
                    case TO_RESIST:
                        sprintf(buf,"{cAdded Resistance To{w: {C%s{x\n\r",
                            imm_bit_name(paf->bitvector));
                        break;
                    case TO_VULN:
                        sprintf(buf,"{cAdded Vulnerability To{w: {C%s{x\n\r",
                            imm_bit_name(paf->bitvector));
                        break;
                    default:
                        sprintf(buf,"{RUNKNOWN BIT {W%d {r: {W%d{c - {CReport to ARIOCH{x\n\r",
                            paf->where,paf->bitvector);
                        break;
                }
                send_to_char(buf,ch);
            }
	}
    }

    return;
}



void spell_infravision( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_INFRARED) )
    {
	if (victim == ch)
	  send_to_char("You can already see in the dark.\n\r",ch);
	else
	  act("$N already has infravision.\n\r",ch,NULL,victim,TO_CHAR);
	return;
    }
    act( "$n's eyes glow red.\n\r", ch, NULL, NULL, TO_ROOM );

    af.where	 = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    af.duration  = 2 * level;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_INFRARED;
    affect_to_char( victim, &af );
    send_to_char( "Your eyes glow red.\n\r", victim );
    return;
}



void spell_invis( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim;
    AFFECT_DATA af;

    victim = (CHAR_DATA *) vo;

    if ( IS_AFFECTED(victim, AFF_INVISIBLE) )
	return;

    act( "$n fades out of existence.", victim, NULL, NULL, TO_ROOM );

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level + 12;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_INVISIBLE;
    affect_to_char( victim, &af );
    send_to_char( "You fade out of existence.\n\r", victim );
    if ( ch != victim )
        act("\n\r{C$N is now affected by INVISIBLITY.{x\n\r",ch,NULL,victim,TO_CHAR);
    return;
}



void spell_know_alignment(int sn,int level,CHAR_DATA *ch,void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    char *msg;
    int ap;

    ap = victim->alignment;

         if ( ap >  700 ) msg = "$N has a pure and good aura.";
    else if ( ap >  350 ) msg = "$N is of excellent moral character.";
    else if ( ap >  100 ) msg = "$N is often kind and thoughtful.";
    else if ( ap > -100 ) msg = "$N doesn't have a firm moral commitment.";
    else if ( ap > -350 ) msg = "$N lies to $S friends.";
    else if ( ap > -700 ) msg = "$N is a black-hearted murderer.";
    else msg = "$N is the embodiment of pure evil!.";

    act( msg, ch, NULL, victim, TO_CHAR );
    return;
}







void spell_locate_object( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    char buf[MAX_INPUT_LENGTH];
    BUFFER *buffer;
    OBJ_DATA *obj;
    OBJ_DATA *in_obj;
    bool found;
    int number = 0, max_found;

    found = FALSE;
    number = 0;
    max_found = IS_IMMORTAL(ch) ? 200 : 2 * level;

    buffer = new_buf();
 
    for ( obj = object_list; obj != NULL; obj = obj->next )
    {
	if ( !can_see_obj( ch, obj ) 
        || !is_name( target_name, obj->name ) 
    	||   IS_OBJ_STAT(obj,ITEM_NOLOCATE) 
        || number_percent() > (2 * level)
	||   ch->level < obj->level)
	    continue;

	found = TRUE;
        number++;

	for ( in_obj = obj; in_obj->in_obj != NULL; in_obj = in_obj->in_obj )
	    ;

	if ( in_obj->carried_by != NULL && can_see(ch,in_obj->carried_by))
	{
	    sprintf( buf, "one is carried by %s\n\r",
		PERS(in_obj->carried_by, ch) );
	}
	else
	{
	    if (IS_IMMORTAL(ch) && in_obj->in_room != NULL)
		sprintf( buf, "one is in %s [Room %d]\n\r",
		    in_obj->in_room->name, in_obj->in_room->vnum);
	    else 
	    	sprintf( buf, "one is in %s\n\r",
		    in_obj->in_room == NULL
		    	? "somewhere" : in_obj->in_room->name );
	}

	buf[0] = UPPER(buf[0]);
	add_buf(buffer,buf);

    	if (number >= max_found)
	    break;
    }

    if ( !found )
	send_to_char( "Nothing like that in heaven or earth.\n\r", ch );
    else
	page_to_char(buf_string(buffer),ch);

    free_buf(buffer);

    return;
}





void spell_mass_healing(int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *gch;
    int heal_num, refresh_num;
    
    heal_num = skill_lookup("heal");
    refresh_num = skill_lookup("refresh"); 

    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
	if ((IS_NPC(ch) && IS_NPC(gch)) ||
	    (!IS_NPC(ch) && !IS_NPC(gch)))
	{
	    spell_heal(heal_num,level,ch,(void *) gch,TARGET_CHAR);
	    spell_refresh(refresh_num,level,ch,(void *) gch,TARGET_CHAR);  
	}
    }
}
	    

void spell_mass_invis( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    AFFECT_DATA af;
    CHAR_DATA *gch;

    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
	if ( !is_same_group( gch, ch ) || IS_AFFECTED(gch, AFF_INVISIBLE) )
	    continue;
	act( "$n slowly fades out of existence.", gch, NULL, NULL, TO_ROOM );
	send_to_char( "You slowly fade out of existence.\n\r", gch );

	af.where     = TO_AFFECTS;
	af.type      = sn;
    	af.level     = level/2;
	af.duration  = 24;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = AFF_INVISIBLE;
	affect_to_char( gch, &af );
    }
    send_to_char( "Ok.\n\r", ch );

    return;
}



void spell_null( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    send_to_char( "That's not a spell!\n\r", ch );
    return;
}



void spell_pass_door( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_PASS_DOOR) )
    {
	if (victim == ch)
	  send_to_char("You are already out of phase.\n\r",ch);
	else
	  act("$N is already shifted out of phase.",ch,NULL,victim,TO_CHAR);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = number_fuzzy( level / 4 );
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_PASS_DOOR;
    affect_to_char( victim, &af );
    act( "$n turns translucent.", victim, NULL, NULL, TO_ROOM );
    send_to_char( "You turn translucent.\n\r", victim );
    if ( ch != victim )
        act("\n\r{C$N is now affected by PASS DOOR.{x\n\r",ch,NULL,victim,TO_CHAR);
    return;
}

/* RT plague spell, very nasty */

void spell_plague( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (saves_spell(level,victim,DAM_DISEASE) || 
        (IS_NPC(victim) && IS_SET(victim->act,ACT_UNDEAD)))
    {
	if (ch == victim)
	  send_to_char("You feel momentarily ill, but it passes.\n\r",ch);
	else
	  act("$N seems to be unaffected.",ch,NULL,victim,TO_CHAR);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type 	  = sn;
    af.level	  = level * 3/4;
    af.duration  = level;
    af.location  = APPLY_STR;
    af.modifier  = -5; 
    af.bitvector = AFF_PLAGUE;
    affect_join(victim,&af);
   
    send_to_char
      ("You scream in agony as plague sores erupt from your skin.\n\r",victim);
    act("$n screams in agony as plague sores erupt from $s skin.",
	victim,NULL,NULL,TO_ROOM);
}

void spell_poison( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    AFFECT_DATA af;


    if (target == TARGET_OBJ)
    {
	obj = (OBJ_DATA *) vo;

	if (obj->item_type == ITEM_FOOD || obj->item_type == ITEM_DRINK_CON)
	{
	    if (IS_OBJ_STAT(obj,ITEM_BLESS) || IS_OBJ_STAT(obj,ITEM_FORTIFIED))
	    {
		act("Your spell fails to corrupt $p.",ch,obj,NULL,TO_CHAR);
		return;
	    }
	    obj->value[3] = 1;
	    act("$p is infused with poisonous vapors.",ch,obj,NULL,TO_ALL);
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
	    ||  IS_OBJ_STAT(obj,ITEM_BLESS) || IS_OBJ_STAT(obj,ITEM_FORTIFIED))
	    {
		act("You can't seem to envenom $p.",ch,obj,NULL,TO_CHAR);
		return;
	    }

	    if (IS_WEAPON_STAT(obj,WEAPON_POISON))
	    {
		act("$p is already envenomed.",ch,obj,NULL,TO_CHAR);
		return;
	    }

	    af.where	 = TO_WEAPON;
	    af.type	 = sn;
	    af.level	 = level / 2;
	    af.duration	 = level/8;
 	    af.location	 = 0;
	    af.modifier	 = 0;
	    af.bitvector = WEAPON_POISON;
	    affect_to_obj(obj,&af);

	    act("$p is coated with deadly venom.",ch,obj,NULL,TO_ALL);
	    return;
	}

	act("You can't poison $p.",ch,obj,NULL,TO_CHAR);
	return;
    }

    victim = (CHAR_DATA *) vo;

    if ( saves_spell( level, victim,DAM_POISON) )
    {
	act("$n turns slightly green, but it passes.",victim,NULL,NULL,TO_ROOM);
	send_to_char("You feel momentarily ill, but it passes.\n\r",victim);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level;
    af.location  = APPLY_STR;
    af.modifier  = -5;
    af.bitvector = AFF_POISON;
    affect_join( victim, &af );
    send_to_char( "{GYou feel very sick.{x\n\r", victim );
    act("{G$n looks very ill.{x",victim,NULL,NULL,TO_ROOM);
    return;
}



void spell_protection_evil(int sn,int level,CHAR_DATA *ch,void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
 
   if (!IS_IMP(ch))
     {
      if ( IS_AFFECTED(victim, AFF_PROTECT_EVIL) 
      ||   IS_AFFECTED(victim, AFF_PROTECT_GOOD))
        {
         if (victim == ch)
           send_to_char("You are already protected.\n\r",ch);
         else
           act("$N is already protected.",ch,NULL,victim,TO_CHAR);
         return;
        }
     }
   else
     {
      if ( IS_AFFECTED(victim, AFF_PROTECT_EVIL))
        {
         if (victim == ch)
           send_to_char("You are already protected.\n\r",ch);
         else
           act("$N is already protected.",ch,NULL,victim,TO_CHAR);
         return;
        }
     }


    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = 24;
    af.location  = APPLY_SAVES;
    af.modifier  = (level/2);
    af.bitvector = AFF_PROTECT_EVIL;
    affect_to_char( victim, &af );
    send_to_char( "You feel holy and pure.\n\r", victim );
    if ( ch != victim )
        act("\n\r{C$N is now affected by PROTECTION FROM EVIL.{x\n\r",ch,NULL,victim,TO_CHAR);
    return;
}
 
void spell_protection_good(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
 
   if (!IS_IMP(ch))
     {
      if ( IS_AFFECTED(victim, AFF_PROTECT_GOOD) 
      ||   IS_AFFECTED(victim, AFF_PROTECT_EVIL))
        {
         if (victim == ch)
           send_to_char("You are already protected.\n\r",ch);
         else
           act("$N is already protected.",ch,NULL,victim,TO_CHAR);
         return;
        }
     }
    else
     {
      if ( IS_AFFECTED(victim, AFF_PROTECT_GOOD))
        {
         if (victim == ch)
           send_to_char("You are already protected.\n\r",ch);
         else
           act("$N is already protected.",ch,NULL,victim,TO_CHAR);
         return;
        }
     }


    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = 24;
    af.location  = APPLY_SAVES;
    af.modifier  = -1;
    af.bitvector = AFF_PROTECT_GOOD;
    affect_to_char( victim, &af );
    send_to_char( "You feel aligned with darkness.\n\r", victim );
    if ( ch != victim )
        act("\n\r{C$N is now affected by PROTECTION FROM GOOD.{x\n\r",ch,NULL,victim,TO_CHAR);
    return;
}


void spell_ray_of_truth (int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam, align;
 
    if (IS_EVIL(ch) )
    {
        victim = ch;
        send_to_char("The energy explodes inside you!\n\r",ch);
    }
 
    if (victim != ch)
    {
        act("$n raises $s hand, and a blinding ray of light shoots forth!",
            ch,NULL,NULL,TO_ROOM);
        send_to_char(
	   "You raise your hand and a blinding ray of light shoots forth!\n\r",
	   ch);
    }

    if (IS_GOOD(victim))
    {
	act("$n seems unharmed by the light.",victim,NULL,victim,TO_ROOM);
	send_to_char("The light seems powerless to affect you.\n\r",victim);
	return;
    }

    dam = dice( level, 10 );
    if ( saves_spell( level, victim,DAM_HOLY) )
        dam /= 2;

    align = victim->alignment;
    align -= 350;

    if (align < -1000)
	align = -1000 + (align + 1000) / 3;

    dam = (dam * align * align) / 1000000;

    damage( ch, victim, dam, sn, DAM_HOLY ,TRUE);
    spell_blindness(gsn_blindness, 
	3 * level / 4, ch, (void *) victim,TARGET_CHAR);
}


void spell_recharge( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    int chance, percent;

    if (obj->item_type != ITEM_WAND && obj->item_type != ITEM_STAFF)
    {
	send_to_char("That item does not carry charges.\n\r",ch);
	return;
    }

    if (obj->value[3] >= 3 * level / 2)
    {
	send_to_char("Your skills are not great enough for that.\n\r",ch);
	return;
    }

    if (obj->value[1] == 0)
    {
	send_to_char("That item has already been recharged once.\n\r",ch);
	return;
    }

    chance = 40 + 2 * level;

    chance -= obj->value[3]; /* harder to do high-level spells */
    chance -= (obj->value[1] - obj->value[2]) *
	      (obj->value[1] - obj->value[2]);

    chance = UMAX(level/2,chance);

    percent = number_percent();

    if (percent < chance / 2)
    {
	act("$p glows softly.",ch,obj,NULL,TO_CHAR);
	act("$p glows softly.",ch,obj,NULL,TO_ROOM);
	obj->value[2] = UMAX(obj->value[1],obj->value[2]);
	obj->value[1] = 0;
	return;
    }

    else if (percent <= chance)
    {
	int chargeback,chargemax;

	act("$p glows softly.",ch,obj,NULL,TO_CHAR);
	act("$p glows softly.",ch,obj,NULL,TO_CHAR);

	chargemax = obj->value[1] - obj->value[2];
	
	if (chargemax > 0)
	    chargeback = UMAX(1,chargemax * percent / 100);
	else
	    chargeback = 0;

	obj->value[2] += chargeback;
	obj->value[1] = 0;
	return;
    }	

    else if (percent <= UMIN(95, 3 * chance / 2))
    {
	send_to_char("Nothing seems to happen.\n\r",ch);
	if (obj->value[1] > 1)
	    obj->value[1]--;
	return;
    }

    else /* whoops! */
    {
	act("$p glows brightly and explodes!",ch,obj,NULL,TO_CHAR);
	act("$p glows brightly and explodes!",ch,obj,NULL,TO_ROOM);
	extract_obj(obj);
    }
}

void spell_refresh( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    victim->move = UMIN( victim->move + level, victim->max_move );
    if (victim->max_move == victim->move)
        send_to_char("You feel fully refreshed!\n\r",victim);
    else
        send_to_char( "You feel less tired.\n\r", victim );
    if ( ch != victim )
        send_to_char( "Ok.\n\r", ch );
    return;
}

void spell_remove_curse( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    OBJ_DATA *obj;
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    OBJ_DATA *obj_next;

     victim = (CHAR_DATA *) vo;

    if (check_dispel(level,victim,gsn_curse))
    {
	send_to_char("\n\r{WYou feel better.{x\n\r",victim);
	act("\n\r{C$n looks more relaxed.{x\n\r",victim,NULL,NULL,TO_ROOM);
    }
    
    if(ch==victim || IS_NPC(ch))
    {
        for (obj = victim->carrying; obj; obj = obj->next_content)
        {
            if(!can_see_obj(ch,obj) || !IS_SET(obj->extra_flags, ITEM_NOREMOVE) || obj->wear_loc == WEAR_INVENTORY)
                continue;
            
            unequip_char( victim, obj );
            act( "$n removes $p.", victim, obj, NULL, TO_ROOM );
            act( "$n removes $p.", victim, obj, NULL, TO_CHAR );
        }
        
    	for ( obj = victim->carrying; obj != NULL; obj = obj_next )
    	{
    	    obj_next = obj->next_content;
    	    
            if(!can_see_obj(ch,obj) || !IS_SET(obj->extra_flags,ITEM_NODROP) || obj->wear_loc != WEAR_INVENTORY)
                continue;
            
    	    obj_from_char( obj );
    	    obj_to_room( obj, victim->in_room );
    	    act( "$n drops $p.", victim, obj, NULL, TO_ROOM );
    	    act( "$n drops $p.", victim, obj, NULL, TO_CHAR );
        }
    }
    
 return;
}

void spell_golden( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim = ( target != TARGET_OBJ ) ? (CHAR_DATA *) vo : NULL;
  AFFECT_DATA af;
  
  if ( get_skill( ch, sn ) == 0 )
    {
    send_to_char( "{RYou are unable to make the magic work!{x\n\r", ch );
    return;
    }
 
if (!IS_IMP(ch))
  {
   if ( victim != ch )
     {
      send_to_char( "{RYou cannot cast this spell on another!{x\n\r", ch );
      return;
     }
  }


   if ( (is_affected(victim, skill_lookup("bio fortress")))
   || (is_affected(victim, skill_lookup("sanctuary"))) )
     {
      if (victim == ch)
        { send_to_char("{RGolden Aura is not compatiable with Sanctuary or Bio-Fortress!{x\n\r",ch); }
      else
        { send_to_char("{RThey have Sanctuary or Bio-Fortress, Golden Aura will not work!{x\n\r",ch); }
      return;
     }
        
   if (is_affected(victim, skill_lookup("golden aura")))
     {
      if (victim == ch)
        { send_to_char("{RYou are already protected by Golden Aura!{x\n\r",ch); }
      else
        { send_to_char("{RThey are already protected by Golden Aura!{x\n\r",ch); }
      return;
     }

  af.where     = TO_AFFECTS;
  af.type      = sn;
  af.level     = level;
  af.duration  = ( level / 4.5 );
  af.location  = APPLY_NONE;
  af.modifier  = 0;
  af.bitvector = AFF_GOLDEN;
  affect_to_char( victim, &af );


    if (victim != ch)
     { 
act("\n\r{C$N is now affected by GOLDEN AURA.{x\n\r",ch,NULL,victim,TO_CHAR); 
  act( "{Y$n is surrounded by a crisp golden aura.{x", victim, NULL, NULL, TO_NOTVICT );
  send_to_char( "{YYou are surrounded by a crisp golden aura.\n\r{c", victim );
     }
    else
     {
  act( "{Y$n is surrounded by a crisp golden aura.{x", victim, NULL, NULL, TO_ROOM );
  send_to_char( "{YYou are surrounded by a crisp golden aura.\n\r{c", victim );
     }
  return;
}

void spell_sanctuary( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

   if ( (is_affected(victim, skill_lookup("golden aura")))
   || (is_affected(victim, skill_lookup("bio fortress"))) )
     {
      if (victim == ch)
        { send_to_char("{RSanctuary is not compatiable with Bio-Fortress or Golden Aura!{x\n\r",ch); }
      else
        { send_to_char("{RThey have Bio-Fortress or Golden Aura, Sanctuary will not work!{x\n\r",ch); }

      return;
     }
        
   if (is_affected(victim, skill_lookup("sanctuary")))
     {
      if (victim == ch)
        { send_to_char("{RYou are already protected by Sanctuary!{x\n\r",ch); }
      else
        { send_to_char("{RThey are already protected by Sanctuary!{x\n\r",ch); }
      return;
     }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level / 6;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_SANCTUARY;
    affect_to_char( victim, &af );
    act( "{W$n is surrounded by a white aura.{x", victim, NULL, NULL, TO_ROOM );
    send_to_char( "{WYou are surrounded by a white aura.{x\n\r", victim );
    return;
}



void spell_shield( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
    {
	if (victim == ch)
	  send_to_char("You are already protected by an energy shield.\n\r",ch);
	else
	  act("$N is already protected by an energy shield.",ch,NULL,victim,TO_CHAR);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = 8 + level;
    af.location  = APPLY_AC;
    af.modifier  = -20;
    af.bitvector = 0;
    affect_to_char( victim, &af );
    act( "$n is surrounded by an energy shield.", victim, NULL, NULL, TO_ROOM );
    send_to_char( "You are surrounded by an energy shield.\n\r", victim );
    return;
}


void spell_sleep( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
  
    if ( IS_AFFECTED(victim, AFF_SLEEP)
    ||   (IS_NPC(victim) && IS_SET(victim->act,ACT_UNDEAD))
    ||   (level + 2) < victim->level
    ||   saves_spell( level-4, victim,DAM_MAGIC) )
	return;

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = 4 + level;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_SLEEP;
    affect_join( victim, &af );

    if ( IS_AWAKE(victim) )
    {
	send_to_char( "You feel very sleepy ..... zzzzzz.\n\r", victim );
	act( "$n goes to sleep.", victim, NULL, NULL, TO_ROOM );
	victim->position = POS_SLEEPING;
    }
    return;
}

void spell_slow( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
 
    if ( is_affected( victim, sn ) || IS_AFFECTED(victim,AFF_SLOW))
    {
        if (victim == ch)
          send_to_char("You can't move any slower!\n\r",ch);
        else
          act("$N can't get any slower than that.",
              ch,NULL,victim,TO_CHAR);
        return;
    }
 
    if (saves_spell(level,victim,DAM_MAGIC) 
    ||  IS_SET(victim->imm_flags,IMM_MAGIC))
    {
	if (victim != ch)
            send_to_char("Nothing seemed to happen.\n\r",ch);
        send_to_char("You feel momentarily lethargic.\n\r",victim);
        return;
    }
 
    if (IS_AFFECTED(victim,AFF_HASTE))
    {
        if (!check_dispel(level,victim,skill_lookup("haste")))
        {
	    if (victim != ch)
            	send_to_char("Spell failed.\n\r",ch);
            send_to_char("You feel momentarily slower.\n\r",victim);
            return;
        }

        act("$n is moving less quickly.",victim,NULL,NULL,TO_ROOM);
        return;
    }
 

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level/2;
    af.location  = APPLY_DEX;
    af.modifier  = -1 - (level >= 18) - (level >= 25) - (level >= 32);
    af.bitvector = AFF_SLOW;
    affect_to_char( victim, &af );
    send_to_char( "You feel yourself slowing d o w n...\n\r", victim );
    act("$n starts to move in slow motion.",victim,NULL,NULL,TO_ROOM);
    return;
}

void spell_stone_skin( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( ch, sn ) )
    {
	if (victim == ch)
	  send_to_char("Your skin is already as hard as a rock.\n\r",ch); 
	else
	  act("$N is already as hard as can be.",ch,NULL,victim,TO_CHAR);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level;
    af.location  = APPLY_AC;
    af.modifier  = -40;
    af.bitvector = 0;
    affect_to_char( victim, &af );
    act( "$n's skin turns to stone.", victim, NULL, NULL, TO_ROOM );
    send_to_char( "Your skin turns to stone.\n\r", victim );
    if ( ch != victim )
        act("\n\r{C$N is now affected by STONE SKIN.{x\n\r",ch,NULL,victim,TO_CHAR);
    return;
}



void spell_summon( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim;
    char   log_buf[MAX_STRING_LENGTH];


    if (IS_CLASS(ch, CLASS_CLERIC))
      {
    if ( ( victim = get_char_world( ch, target_name ) ) == NULL
    ||   victim == ch
    ||   victim->in_room == NULL
    ||   IS_SET(ch->in_room->room_flags, ROOM_SAFE)
    ||   IS_SET(victim->in_room->room_flags, ROOM_SAFE)
    ||   IS_SET(victim->in_room->room_flags, ROOM_PRIVATE)
    ||   IS_SET(victim->in_room->room_flags, ROOM_SOLITARY)
    ||   IS_SET(victim->in_room->room_flags, ROOM_DRAGONPIT)
    ||   IS_SET(ch->in_room->room_flags, ROOM_DRAGONPIT)
    ||   IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
    ||   IS_SET(victim->in_room->area->area_flags, AREA_PROTO )
    ||   (IS_NPC(victim) && IS_SET(victim->act,ACT_AGGRESSIVE))
    ||   victim->level >= level + 3
    ||   (!IS_NPC(victim) && victim->level >= LEVEL_IMMORTAL)
    ||   victim->fighting != NULL
    ||   (IS_NPC(victim) && IS_SET(victim->imm_flags,IMM_SUMMON))
    ||	 (IS_NPC(victim) && victim->pIndexData->pShop != NULL)
    ||   (!IS_NPC(victim) && IS_SET(victim->pact,PLR_NOSUMMON))
    ||   (IS_NPC(victim) && saves_spell( level, victim,DAM_HOLY)) )

    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }
     }
   else
      {
    if ( ( victim = get_char_world( ch, target_name ) ) == NULL
    ||   victim == ch
    ||   victim->in_room == NULL
    ||   IS_SET(ch->in_room->room_flags, ROOM_SAFE)
    ||   IS_SET(victim->in_room->room_flags, ROOM_SAFE)
    ||   IS_SET(victim->in_room->room_flags, ROOM_PRIVATE)
    ||   IS_SET(victim->in_room->room_flags, ROOM_SOLITARY)
    ||   IS_SET(victim->in_room->room_flags, ROOM_DRAGONPIT)
    ||   IS_SET(ch->in_room->room_flags, ROOM_DRAGONPIT)
    ||   IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
    ||   IS_SET(victim->in_room->area->area_flags, AREA_PROTO )
    ||   (IS_NPC(victim) && IS_SET(victim->act,ACT_AGGRESSIVE))
    ||   victim->level >= level + 3
    ||   (!IS_NPC(victim) && victim->level >= LEVEL_IMMORTAL)
    ||   victim->fighting != NULL
    ||   (IS_NPC(victim) && IS_SET(victim->imm_flags,IMM_SUMMON))
    ||	 (IS_NPC(victim) && victim->pIndexData->pShop != NULL)
    ||   (!IS_NPC(victim) && IS_SET(victim->pact,PLR_NOSUMMON))
    ||   (IS_NPC(victim) && saves_spell( level, victim,DAM_MAGIC)) )

    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }
     }

        sprintf( log_buf, "Log %s: cast summon %s", ch->name, victim->name);
        wiznet(log_buf,ch,NULL,WIZ_SECURE,0,ch->level);
        log_string( log_buf );

    act( "$n disappears suddenly.", victim, NULL, NULL, TO_ROOM );
    char_from_room( victim );
    char_to_room( victim, ch->in_room );
    act( "$n arrives suddenly.", victim, NULL, NULL, TO_ROOM );
    act( "$n has summoned you!", ch, NULL, victim,   TO_VICT );
    do_look( victim, "auto" );
    return;
} 


void spell_teleport( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    ROOM_INDEX_DATA *pRoomIndex;
    //CHAR_DATA * vch;
    //CHAR_DATA * vch_next;


    if (IS_CLASS(ch, CLASS_CLERIC))
      {
    if ( ( victim = get_char_world( ch, target_name ) ) == NULL
    //||   victim == ch
    ||   victim->in_room == NULL
    ||   !can_see_room(ch,victim->in_room) 
    ||   IS_SET(victim->in_room->room_flags, ROOM_SAFE)
    ||   IS_SET(victim->in_room->room_flags, ROOM_PRIVATE)
    ||   IS_SET(victim->in_room->room_flags, ROOM_SOLITARY)
    ||   IS_SET(victim->in_room->room_flags, ROOM_DRAGONPIT)
    ||   IS_SET(ch->in_room->room_flags, ROOM_DRAGONPIT)
    ||   IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
    ||   IS_SET(victim->in_room->area->area_flags, AREA_PROTO )
    ||   IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
    ||   victim->level >= level + 3
    ||   (is_clan(victim) && !is_same_clan(ch,victim))
    ||   (!IS_NPC(victim) && victim->level > LEVEL_HERO)   
    ||   (IS_NPC(victim) && IS_SET(victim->imm_flags,IMM_SUMMON))
    ||   (IS_NPC(victim) && saves_spell( level, victim,DAM_HOLY) ) )
    {
        send_to_char( "You failed.\n\r", ch );
        return;
    }
      }
   else
      {
    if ( ( victim = get_char_world( ch, target_name ) ) == NULL
    //||   victim == ch
    ||   victim->in_room == NULL
    ||   !can_see_room(ch,victim->in_room) 
    ||   IS_SET(victim->in_room->room_flags, ROOM_SAFE)
    ||   IS_SET(victim->in_room->room_flags, ROOM_PRIVATE)
    ||   IS_SET(victim->in_room->room_flags, ROOM_SOLITARY)
    ||   IS_SET(victim->in_room->room_flags, ROOM_DRAGONPIT)
    ||   IS_SET(ch->in_room->room_flags, ROOM_DRAGONPIT)
    ||   IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
    ||   IS_SET(victim->in_room->area->area_flags, AREA_PROTO )
    ||   IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
    ||   victim->level >= level + 3
    ||   (is_clan(victim) && !is_same_clan(ch,victim))
    ||   (!IS_NPC(victim) && victim->level > LEVEL_HERO)  
    ||   (IS_NPC(victim) && IS_SET(victim->imm_flags,IMM_SUMMON))
    ||   (IS_NPC(victim) && saves_spell( level, victim,DAM_MAGIC) ) )
    {
        send_to_char( "You failed.\n\r", ch );
        return;
    }
      }

 if (!IS_IMP(ch))
    {
    if (IS_IMMORTAL(ch))
      {
       send_to_char("\n\r{rIMMs can use {RGOTO{r so there is no reason to be using {RTELEPORT{r.{x\n\r",ch);
       return;
      }
    }

    pRoomIndex = get_random_room(victim);

    if ( IS_SET( pRoomIndex->area->area_flags, AREA_PROTO ) )
    {
	send_to_char( "\n\r{RYou failed!{x\n\r", ch );
	return;
    }

    if (victim != ch)
	send_to_char("You have been teleported!\n\r",victim);
	
	act( "$n vanishes!", victim, NULL, NULL, TO_ROOM );
        char_from_room( victim );
        char_to_room( victim, pRoomIndex );
        act( "$n slowly fades into existence.", victim, NULL, NULL, TO_ROOM );
        do_function(victim, &do_look, "auto" );
	
	// teleport all friends in room
	/*
    for (vch = ch->in_room->people; vch != NULL; vch = vch_next)
    {
	   vch_next = vch->next_in_room;
	   
	   if(vch!=victim && !is_same_group(vch, ch) )
	       continue;
	   
        act( "$n vanishes!", vch, NULL, NULL, TO_ROOM );
        char_from_room( vch );
        char_to_room( vch, pRoomIndex );
        act( "$n slowly fades into existence.", vch, NULL, NULL, TO_ROOM );
        do_function(vch, &do_look, "auto" );
	   
	}
	*/
    return;
}



void spell_ventriloquate( int sn, int level, CHAR_DATA *ch,void *vo,int target)
{
    char buf1[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char speaker[MAX_INPUT_LENGTH];
    CHAR_DATA *vch;

    target_name = one_argument( target_name, speaker );

    sprintf( buf1, "%s says '%s'.\n\r",              speaker, target_name );
    sprintf( buf2, "Someone makes %s say '%s'.\n\r", speaker, target_name );
    buf1[0] = UPPER(buf1[0]);

    for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room )
    {
	if (!is_exact_name( speaker, vch->name) && IS_AWAKE(vch))
	    send_to_char( saves_spell(level,vch,DAM_MAGIC) ? buf2 : buf1, vch );
    }

    return;
}



void spell_weaken( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;


    if (IS_CLASS(ch, CLASS_CLERIC))
      {
    if ( is_affected( victim, sn ) || saves_spell( level,victim,DAM_HOLY) )
	return;
      }
    else
      {
    if ( is_affected( victim, sn ) || saves_spell( level,victim,DAM_MAGIC) )
	return;
      }


    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level / 2;
    af.location  = APPLY_STR;
    af.modifier  = -1 * (level / 5);
    af.bitvector = AFF_WEAKEN;
    affect_to_char( victim, &af );
    send_to_char( "You feel your strength slip away.\n\r", victim );
    act("$n looks tired and weak.",victim,NULL,NULL,TO_ROOM);
    return;
}



/* RT recall spell is back */

void spell_word_of_recall( int sn, int level, CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    ROOM_INDEX_DATA *location;
    
    if (IS_NPC(victim))
      return;
    
    if (!victim->clan)
     {   
    if ((location = get_room_index( ROOM_VNUM_TEMPLE)) == NULL)
    {
	send_to_char("You are completely lost.\n\r",victim);
	return;
    } 

    if (IS_SET(victim->in_room->room_flags,ROOM_NO_RECALL) ||
	IS_AFFECTED(victim,AFF_CURSE))
    {
	send_to_char("Spell failed.\n\r",victim);
	return;
    }


  if (!IS_IMP(ch))
    {
    if (IS_IMMORTAL(ch))
      {
       send_to_char("\n\r{rIMMs can use {RGOTO{r so there is no reason to cast {RWORD OF RECALL{r.{x\n\r",ch);
       return;
      }
    }

    if (victim->fighting != NULL)
	stop_fighting(victim,TRUE);
    
    ch->move /= 2;
    act("$n disappears.",victim,NULL,NULL,TO_ROOM);
    char_from_room(victim);
    char_to_room(victim,location);
    act("$n appears in the room.",victim,NULL,NULL,TO_ROOM);
    do_function(victim, &do_look, "auto");
    }
    else
      {
        void crecall (CHAR_DATA *ch);
         crecall (ch);
      }

}


/*
 * NPC spells.
 */

void spell_acid_breath( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam,hp_dam,dice_dam,hpch;

  if (!IS_IMMORTAL(victim))
     {
      if (IS_NPC(victim))
      {
         act("$n spits acid at $N.",ch,NULL,victim,TO_NOTVICT);
         act("$n spits a stream of corrosive acid at you.",ch,NULL,victim,TO_VICT);
         act("You spit acid at $N.",ch,NULL,victim,TO_CHAR);

         hpch = UMAX(12,ch->hit);
         hp_dam = number_range(hpch/11 + 1, hpch/6);
         dice_dam = dice(level,16);

          dam = UMAX(hp_dam + dice_dam/10,dice_dam + hp_dam/10);
    
      if (saves_spell(level,victim,DAM_ACID))
       {
	acid_effect(victim,level/2,dam/4,TARGET_CHAR);
	damage(ch,victim,dam/2,sn,DAM_ACID,TRUE);
       }
      else
       {
	acid_effect(victim,level,dam,TARGET_CHAR);
	damage(ch,victim,dam,sn,DAM_ACID,TRUE);
       }
     }
   else
     {
      if ((IS_SET(ch->pact, PLR_PKILLER))
      && ((!IS_NPC(victim))
      && (IS_SET(victim->pact, PLR_PKILLER))))
        {
    act("$n spits acid at $N.",ch,NULL,victim,TO_NOTVICT);
    act("$n spits a stream of corrosive acid at you.",ch,NULL,victim,TO_VICT);
    act("You spit acid at $N.",ch,NULL,victim,TO_CHAR);

    hpch = UMAX(12,ch->hit);
    hp_dam = number_range(hpch/11 + 1, hpch/6);
    dice_dam = dice(level,16);

    dam = UMAX(hp_dam + dice_dam/10,dice_dam + hp_dam/10);
    
    if (saves_spell(level,victim,DAM_ACID))
    {
	acid_effect(victim,level/2,dam/4,TARGET_CHAR);
	damage(ch,victim,dam/2,sn,DAM_ACID,TRUE);
    }
    else
    {
	acid_effect(victim,level,dam,TARGET_CHAR);
	damage(ch,victim,dam,sn,DAM_ACID,TRUE);
    }
   }
  }

 }

 }

void spell_fire_breath( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    CHAR_DATA *vch, *vch_next;
    int dam,hp_dam,dice_dam;
    int hpch;

  if (!IS_IMMORTAL(victim))
     {
      if (IS_NPC(victim))
      {
    act("$n breathes forth a cone of fire.",ch,NULL,victim,TO_NOTVICT);
    act("$n breathes a cone of hot fire over you!",ch,NULL,victim,TO_VICT);
    act("You breath forth a cone of fire.",ch,NULL,NULL,TO_CHAR);

    hpch = UMAX( 10, ch->hit );
    hp_dam  = number_range( hpch/9+1, hpch/5 );
    dice_dam = dice(level,20);

    dam = UMAX(hp_dam + dice_dam /10, dice_dam + hp_dam / 10);
    fire_effect(victim->in_room,level,dam/2,TARGET_ROOM);


    for (vch = victim->in_room->people; vch != NULL; vch = vch_next)
    {
	vch_next = vch->next_in_room;

	if (is_safe_spell(ch,vch,TRUE) 
	||  (IS_NPC(vch) && IS_NPC(ch) 
	&&   (ch->fighting != vch || vch->fighting != ch)))
	    continue;

	if (vch == victim) /* full damage */
	{
	    if (saves_spell(level,vch,DAM_FIRE))
	    {
		fire_effect(vch,level/2,dam/4,TARGET_CHAR);
		damage(ch,vch,dam/2,sn,DAM_FIRE,TRUE);
	    }
	    else
	    {
		fire_effect(vch,level,dam,TARGET_CHAR);
		damage(ch,vch,dam,sn,DAM_FIRE,TRUE);
	    }
	}
	else /* partial damage */
	{
	    if (saves_spell(level - 2,vch,DAM_FIRE))
	    {
		fire_effect(vch,level/4,dam/8,TARGET_CHAR);
		damage(ch,vch,dam/4,sn,DAM_FIRE,TRUE);
	    }
	    else
	    {
		fire_effect(vch,level/2,dam/4,TARGET_CHAR);
		damage(ch,vch,dam/2,sn,DAM_FIRE,TRUE);
	    }
	  }
        }
      }
     else
      {
      if ((IS_SET(ch->pact, PLR_PKILLER))
      && ((!IS_NPC(victim))
      && (IS_SET(victim->pact, PLR_PKILLER)))
      && !is_same_group(ch, victim))
        {
    act("$n breathes forth a cone of fire.",ch,NULL,victim,TO_NOTVICT);
    act("$n breathes a cone of hot fire over you!",ch,NULL,victim,TO_VICT);
    act("You breath forth a cone of fire.",ch,NULL,NULL,TO_CHAR);

    hpch = UMAX( 10, ch->hit );
    hp_dam  = number_range( hpch/9+1, hpch/5 );
    dice_dam = dice(level,20);

    dam = UMAX(hp_dam + dice_dam /10, dice_dam + hp_dam / 10);
    fire_effect(victim->in_room,level,dam/2,TARGET_ROOM);


    for (vch = victim->in_room->people; vch != NULL; vch = vch_next)
    {
	vch_next = vch->next_in_room;

	if (is_safe_spell(ch,vch,TRUE) 
	||  (IS_NPC(vch) && IS_NPC(ch) 
	&&   (ch->fighting != vch || vch->fighting != ch)))
	    continue;

	if (vch == victim) /* full damage */
	{
	    if (saves_spell(level,vch,DAM_FIRE))
	    {
		fire_effect(vch,level/2,dam/4,TARGET_CHAR);
		damage(ch,vch,dam/2,sn,DAM_FIRE,TRUE);
	    }
	    else
	    {
		fire_effect(vch,level,dam,TARGET_CHAR);
		damage(ch,vch,dam,sn,DAM_FIRE,TRUE);
	    }
	}
	else /* partial damage */
	{
	    if (saves_spell(level - 2,vch,DAM_FIRE))
	    {
		fire_effect(vch,level/4,dam/8,TARGET_CHAR);
		damage(ch,vch,dam/4,sn,DAM_FIRE,TRUE);
	    }
	    else
	    {
		fire_effect(vch,level/2,dam/4,TARGET_CHAR);
		damage(ch,vch,dam/2,sn,DAM_FIRE,TRUE);
	    }
	  }
        }

        }
      }




    }
  }

void spell_frost_breath( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    CHAR_DATA *vch, *vch_next;
    int dam,hp_dam,dice_dam, hpch;

  if (!IS_IMMORTAL(victim))
     {  
      if (IS_NPC(victim))
      { 
    act("$n breathes out a freezing cone of frost!",ch,NULL,victim,TO_NOTVICT);
    act("$n breathes a freezing cone of frost over you!",
	ch,NULL,victim,TO_VICT);
    act("You breath out a cone of frost.",ch,NULL,NULL,TO_CHAR);

    hpch = UMAX(12,ch->hit);
    hp_dam = number_range(hpch/11 + 1, hpch/6);
    dice_dam = dice(level,16);

    dam = UMAX(hp_dam + dice_dam/10,dice_dam + hp_dam/10);
    cold_effect(victim->in_room,level,dam/2,TARGET_ROOM); 

    for (vch = victim->in_room->people; vch != NULL; vch = vch_next)
    {
	vch_next = vch->next_in_room;

	if (is_safe_spell(ch,vch,TRUE)
	||  (IS_NPC(vch) && IS_NPC(ch) 
	&&   (ch->fighting != vch || vch->fighting != ch)))
	    continue;

	if (vch == victim) /* full damage */
	{
	    if (saves_spell(level,vch,DAM_COLD))
	    {
		cold_effect(vch,level/2,dam/4,TARGET_CHAR);
		damage(ch,vch,dam/2,sn,DAM_COLD,TRUE);
	    }
	    else
	    {
		cold_effect(vch,level,dam,TARGET_CHAR);
		damage(ch,vch,dam,sn,DAM_COLD,TRUE);
	    }
	}
	else
	{
	    if (saves_spell(level - 2,vch,DAM_COLD))
	    {
		cold_effect(vch,level/4,dam/8,TARGET_CHAR);
		damage(ch,vch,dam/4,sn,DAM_COLD,TRUE);
	    }
	    else
	    {
		cold_effect(vch,level/2,dam/4,TARGET_CHAR);
		damage(ch,vch,dam/2,sn,DAM_COLD,TRUE);
	    }
	}
      }
    } 
  else
    {
      if ((IS_SET(ch->pact, PLR_PKILLER))
      && ((!IS_NPC(victim))
      && (IS_SET(victim->pact, PLR_PKILLER)))
      && !is_same_group(ch, victim))
        {

    act("$n breathes out a freezing cone of frost!",ch,NULL,victim,TO_NOTVICT);
    act("$n breathes a freezing cone of frost over you!",
	ch,NULL,victim,TO_VICT);
    act("You breath out a cone of frost.",ch,NULL,NULL,TO_CHAR);

    hpch = UMAX(12,ch->hit);
    hp_dam = number_range(hpch/11 + 1, hpch/6);
    dice_dam = dice(level,16);

    dam = UMAX(hp_dam + dice_dam/10,dice_dam + hp_dam/10);
    cold_effect(victim->in_room,level,dam/2,TARGET_ROOM); 

    for (vch = victim->in_room->people; vch != NULL; vch = vch_next)
    {
	vch_next = vch->next_in_room;

	if (is_safe_spell(ch,vch,TRUE)
	||  (IS_NPC(vch) && IS_NPC(ch) 
	&&   (ch->fighting != vch || vch->fighting != ch)))
	    continue;

	if (vch == victim) /* full damage */
	{
	    if (saves_spell(level,vch,DAM_COLD))
	    {
		cold_effect(vch,level/2,dam/4,TARGET_CHAR);
		damage(ch,vch,dam/2,sn,DAM_COLD,TRUE);
	    }
	    else
	    {
		cold_effect(vch,level,dam,TARGET_CHAR);
		damage(ch,vch,dam,sn,DAM_COLD,TRUE);
	    }
	}
	else
	{
	    if (saves_spell(level - 2,vch,DAM_COLD))
	    {
		cold_effect(vch,level/4,dam/8,TARGET_CHAR);
		damage(ch,vch,dam/4,sn,DAM_COLD,TRUE);
	    }
	    else
	    {
		cold_effect(vch,level/2,dam/4,TARGET_CHAR);
		damage(ch,vch,dam/2,sn,DAM_COLD,TRUE);
	    }
	}
      }
     }




    }


  }
}

    
void spell_gas_breath( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    int dam,hp_dam,dice_dam,hpch;

    act("$n breathes out a cloud of poisonous gas!",ch,NULL,NULL,TO_ROOM);
    act("You breath out a cloud of poisonous gas.",ch,NULL,NULL,TO_CHAR);

    hpch = UMAX(16,ch->hit);
    hp_dam = number_range(hpch/15+1,8);
    dice_dam = dice(level,12);

    dam = UMAX(hp_dam + dice_dam/10,dice_dam + hp_dam/10);
    poison_effect(ch->in_room,level,dam,TARGET_ROOM);

    for (vch = ch->in_room->people; vch != NULL; vch = vch_next)
    {
	vch_next = vch->next_in_room;


  if (!IS_IMMORTAL(vch))
     {
      if (IS_NPC(vch))
      {	
        if (is_safe_spell(ch,vch,TRUE)
	||  (IS_NPC(ch) && IS_NPC(vch) 
	&&   (ch->fighting == vch || vch->fighting == ch)))
	    continue;

	if (saves_spell(level,vch,DAM_POISON))
	{
	    poison_effect(vch,level/2,dam/4,TARGET_CHAR);
	    damage(ch,vch,dam/2,sn,DAM_POISON,TRUE);
	}
	else
	{
	    poison_effect(vch,level,dam,TARGET_CHAR);
	    damage(ch,vch,dam,sn,DAM_POISON,TRUE);
	}
      }
     else
      {
      if ((IS_SET(ch->pact, PLR_PKILLER))
      && ((!IS_NPC(vch))
      && (IS_SET(vch->pact, PLR_PKILLER))))
        {
        if (is_safe_spell(ch,vch,TRUE)
        || is_same_group(ch, vch)
	||  (IS_NPC(ch) && IS_NPC(vch) 
	&&   (ch->fighting == vch || vch->fighting == ch)))
	    continue;

	if (saves_spell(level,vch,DAM_POISON))
	{
	    poison_effect(vch,level/2,dam/4,TARGET_CHAR);
	    damage(ch,vch,dam/2,sn,DAM_POISON,TRUE);
	}
	else
	{
	    poison_effect(vch,level,dam,TARGET_CHAR);
	    damage(ch,vch,dam,sn,DAM_POISON,TRUE);
	}
      }
  
      }
     }
 }
}

void spell_lightning_bolt(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = dice( level, 18 );


    if (IS_CLASS(ch, CLASS_CLERIC))
      {
    if ( saves_spell( level, victim, DAM_HOLY ) )
	dam /= 2;
    damage( ch, victim, dam, sn, DAM_HOLY, TRUE);
      }
     else
      {
    if ( saves_spell( level, victim, DAM_MAGIC ) )
	dam /= 2;
    damage( ch, victim, dam, sn, DAM_MAGIC, TRUE);
      }

    return;
}

void spell_lightning_breath(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam,hp_dam,dice_dam,hpch;


  if (!IS_IMMORTAL(victim))
     {
      if (IS_NPC(victim))
      {
    act("$n breathes a bolt of lightning at $N.",ch,NULL,victim,TO_NOTVICT);
    act("$n breathes a bolt of lightning at you!",ch,NULL,victim,TO_VICT);
    act("You breathe a bolt of lightning at $N.",ch,NULL,victim,TO_CHAR);

    hpch = UMAX(10,ch->hit);
    hp_dam = number_range(hpch/9+1,hpch/5);
    dice_dam = dice(level,20);

    dam = UMAX(hp_dam + dice_dam/10,dice_dam + hp_dam/10);

    if (saves_spell(level,victim,DAM_LIGHTNING))
    {
	shock_effect(victim,level/2,dam/4,TARGET_CHAR);
	damage(ch,victim,dam/2,sn,DAM_LIGHTNING,TRUE);
    }
    else
    {
	shock_effect(victim,level,dam,TARGET_CHAR);
	damage(ch,victim,dam,sn,DAM_LIGHTNING,TRUE); 
    }
      }
   else
      {
      if ((IS_SET(ch->pact, PLR_PKILLER))
      && ((!IS_NPC(victim))
      && (IS_SET(victim->pact, PLR_PKILLER))))
        {
    act("$n breathes a bolt of lightning at $N.",ch,NULL,victim,TO_NOTVICT);
    act("$n breathes a bolt of lightning at you!",ch,NULL,victim,TO_VICT);
    act("You breathe a bolt of lightning at $N.",ch,NULL,victim,TO_CHAR);

    hpch = UMAX(10,ch->hit);
    hp_dam = number_range(hpch/9+1,hpch/5);
    dice_dam = dice(level,20);

    dam = UMAX(hp_dam + dice_dam/10,dice_dam + hp_dam/10);

    if (saves_spell(level,victim,DAM_LIGHTNING))
    {
	shock_effect(victim,level/2,dam/4,TARGET_CHAR);
	damage(ch,victim,dam/2,sn,DAM_LIGHTNING,TRUE);
    }
    else
    {
	shock_effect(victim,level,dam,TARGET_CHAR);
	damage(ch,victim,dam,sn,DAM_LIGHTNING,TRUE); 
    }


        }

      }
 }
}

/*
 * Spells for mega1.are from Glop/Erkenbrand.
 */
void spell_general_purpose(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
 
    dam = number_range( 25, 100 );
    if ( saves_spell( level, victim, DAM_OTHER) )
        dam /= 2;
    damage( ch, victim, dam, sn, DAM_OTHER ,TRUE);
    return;
}

void spell_high_explosive(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
 
    dam = number_range( 30, 120 );

    if ( saves_spell( level, victim, DAM_ENERGY) ) 
        dam /= 2;
    damage( ch, victim, dam, sn, DAM_ENERGY,TRUE);
    return;
}

/* Psionic Powers - Non-Combat*/

void spell_domination( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo, *mch;
    AFFECT_DATA af;
    int max_charm = 0;

/*
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
*/


 if (!IS_AFFECTED(victim, AFF_CHARM))
   {
    if (is_safe(ch,victim)) 
    return;

    if ( victim == ch )
    {
        send_to_char( "Dominate yourself? That's kinda wierd!\n\r", ch );
	return;
    }

    if ( IS_AFFECTED(victim, AFF_CHARM)
    ||   IS_AFFECTED(ch, AFF_CHARM)
    ||   level < victim->level
    ||   IS_SET(victim->imm_flags,IMM_CHARM)
    ||   saves_spell( level, victim,DAM_MENTAL) )
	return;


    if (ch->pet != NULL)
      nuke_pets(ch);


    if ( victim->position == POS_SLEEPING )
    {
        send_to_char( "You can not dominate a mind while it sleeps.\n\r", ch );
        send_to_char( "Your slumbers are briefly troubled.\n\r", victim );
        return;
    }


    if (victim->perm_stat[STAT_INT] > ( ch->perm_stat[STAT_INT] + 2) )
    {
	send_to_char( "\n\r{RYour victim is much to intelligent to be enslaved by you!\n\r{x", ch);
	return;
    }


   if (ch->class == 0 || ch->class == 2)
     {
    for (mch = char_list; mch != NULL; mch = mch->next) 
       {
        if ((max_charm <= 2)
        && (ch->pet != NULL))  
           max_charm += 1;
      
        if ((max_charm >= 3)
        && (ch->pet != NULL))
          {
         send_to_char( "\n\r{RAttempting to take control of more than 3 minds{x\n\r",ch);
         send_to_char( "{Rcauses you to lose control of the Magical Energies{x\n\r",ch);
         send_to_char( "{Rreleasing {WONE{r of your adoring fans!{x\n\r",ch);
	 nuke_pets(ch);
           return;
          }

        if (mch->master == ch || mch->leader == ch)
          {
           if (max_charm <= 2 )  
             { max_charm += 1; }
          }

        }
     }
   else
     {
      if (ch->pet != NULL)
          {
         send_to_char( "\n\r{ROnly MAGEs & PSIs can CONTROL more than ONE slave!{x\n\r",ch);
	 nuke_pets(ch);
           return;
          }
     }

  
    if ( victim->master )
      stop_follower( victim );

    add_follower( victim, ch );
    victim->leader = ch;

   if (ch->class == 0 || ch->class == 2)
     {
      if ((max_charm >= 2)
      && (ch->pet == NULL))
        ch->pet = victim;
     }
   else
     {
      ch->pet = victim;
     }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    af.duration  = level;
    af.location  = 0;
    af.modifier  = 0;
    af.bitvector = AFF_CHARM;
    affect_to_char( victim, &af );
    act( "{M\n\rYour will dominates $N!\n\r{x", ch, NULL, victim, TO_CHAR );
    act( "{G\n\rYour will is dominated by $n!\n\r{x", ch, NULL, victim, TO_VICT );
    if ( ch != victim )
    act("{C\n\r$N looks at you with a blank stare in their eyes.\n\r{x",ch,NULL,victim,TO_CHAR);
    return;
   }
 else
   {
    send_to_char("{RThey are already under the influence of a CHARM!{x\n\r",ch);
    return;
   }
return;
}

void spell_apport( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim;

    if ( ( victim = get_char_world( ch, target_name ) ) == NULL
    ||   victim == ch
    ||   victim->in_room == NULL
    ||   IS_SET(ch->in_room->room_flags, ROOM_SAFE)
    ||   IS_SET(victim->in_room->room_flags, ROOM_SAFE)
    ||   IS_SET(victim->in_room->room_flags, ROOM_PRIVATE)
    ||   IS_SET(victim->in_room->room_flags, ROOM_SOLITARY)
    ||   IS_SET(victim->in_room->room_flags, ROOM_DRAGONPIT)
    ||   IS_SET(ch->in_room->room_flags, ROOM_DRAGONPIT)
    ||   IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
    ||   (IS_NPC(victim) && IS_SET(victim->act,ACT_AGGRESSIVE))
    ||   victim->level >= level + 3
    ||   (!IS_NPC(victim) && victim->level >= LEVEL_IMMORTAL)
    ||   victim->fighting != NULL
    ||   (IS_NPC(victim) && IS_SET(victim->imm_flags,IMM_SUMMON))
    ||	 (IS_NPC(victim) && victim->pIndexData->pShop != NULL)
    ||   (!IS_NPC(victim) && IS_SET(victim->pact,PLR_NOSUMMON))
    ||   (IS_NPC(victim) && saves_spell( level, victim,DAM_MENTAL)) )

    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    act( "$n disappears suddenly.", victim, NULL, NULL, TO_ROOM );
    char_from_room( victim );
    char_to_room( victim, ch->in_room );
    act( "$n arrives with a {R**POP**{x.", victim, NULL, NULL, TO_ROOM );
    act( "$n has apported you to them!", ch, NULL, victim,   TO_VICT );
    do_look( victim, "auto" );
    return;
}

void spell_combat_mind ( int sn,int level,CHAR_DATA *ch,void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ))
    {
     if ( victim == ch )
       send_to_char( "You already understand battle tactics.\n\r",victim );
     else
       act( "$N already understands battle tactics.", ch, NULL, victim, TO_CHAR );
       return;
     }
  
    if ((is_affected(victim, skill_lookup("frenzy")))
    || IS_AFFECTED(victim,AFF_BERSERK))
      {
        if (victim == ch)
        send_to_char("You are already at your peak.\n\r",victim);
        else
        act("$N's is already at their peak.", ch, NULL, victim, TO_CHAR);
        return;
      }

  
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level /3;
    af.location  = APPLY_HITROLL;
    af.modifier  = level /5;
    af.bitvector = 0;
    affect_to_char( victim, &af );

    af.location = APPLY_AC;
    af.modifier = - level/2 - 5;
    affect_to_char( victim, &af );

    af.location = APPLY_DAMROLL;
    af.modifier = level /5;
    affect_to_char( victim, &af );

    if ( victim == ch)
      send_to_char( "\x1B[1m\x1B[34mYou gain a keen understanding of battle tactics.\x1B[0m\n\r",victim );
    else
     {
    send_to_char( "\x1B[1m\x1B[34mYou gain a keen understanding of battle tactics.\x1B[0m\n\r",victim );
 act("\n\r{C$N is now affected by COMBAT MIND {c({WPSIONIC POWER{c){C.{x\n\r",ch,NULL,victim,TO_CHAR);
     }

    return;
}

void spell_adrenaline_control (int sn,int level,CHAR_DATA *ch,void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
    {
     if ( victim == ch )
       send_to_char( "You already pumped up on adrenaline.\n\r",victim );
     else
       act( "$N is already pumped on adrenaline.", ch, NULL, victim,
TO_CHAR );
       return;
     }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level - 5;
    af.location  = APPLY_DEX;
    af.modifier  = level/10 +2;
    af.bitvector = 0;
    affect_to_char( victim, &af );

    af.location = APPLY_CON;
    af.modifier = level/10 +2;
    affect_to_char( victim, &af );

    af.where     = TO_RESIST;
    af.type      = sn;
    af.level     = level;
    af.duration  = level - 5;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = RES_BASH;
    affect_to_char( victim, &af );  
    
    if (victim == ch)
      {
    send_to_char( "\x1B[1m\x1B[34mYou have given yourself an adrenaline rush!\x1B[0m\n\r", ch );
      }
    else
      {
    send_to_char( "\x1B[1m\x1B[34mYou have been given an adrenaline rush!\x1B[0m\n\r", victim );
 act("\n\r{C$N is now affected by ADRENALINE RUSH {c({WPSIONIC POWER{c){C.{x\n\r",ch,NULL,victim,TO_CHAR);
      }
    return;
}

void spell_enhanced_strength(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
    {
     if ( victim == ch )
       send_to_char( "You are as strong as you can get.\n\r",victim );
     else
       act( "$N is as strong as they will get.", ch, NULL, victim, TO_CHAR );
       return;
     }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level;
    af.location  = APPLY_STR;
    af.modifier  = level/10 +5;
    af.bitvector = 0;
    affect_to_char( victim, &af );
    send_to_char( "\x1B[1m\x1B[34mYou are HUGE!\x1B[0m\n\r", victim );
    if ( ch != victim )
 act("\n\r{C$N is now affected by ENHANCED STRENGTH {c({WPSIONIC POWER{c){C.{x\n\r",ch,NULL,victim,TO_CHAR);
    return;
}

void spell_aura_sight ( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    char *msg;
    int ap;

    ap = victim->alignment;

    if ( ap > 700 ) msg = "$N has an aura as white as the driven snow.";
    else if ( ap > 350 ) msg = "$N is of excellent moral character.";
    else if ( ap > 100 ) msg = "$N is often kind and thoughtful.";
    else if ( ap > -100 ) msg = "$N doesn't have a firm moral commitment.";
    else if ( ap > -350 ) msg = "$N lies to $S friends.";
    else if ( ap > -700 ) msg = "Don't bring $N home to meet your family.";
    else msg = "Uh, check please!";

    act( msg, ch, NULL, victim, TO_CHAR );
    return;
}


void spell_awe ( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{ 
    
    send_to_char( "\n\r{CYou reach out with your mind in an attempt\n\r{x",ch );
    send_to_char( "{Cto bring order to the world around you.\n\r{x",ch);


    if ( number_percent( ) < get_skill(ch,sn) )
     {
      CHAR_DATA *rch;

      for ( rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room )
         {
          if (ch->level >= rch->level)
            {
             if ( rch->fighting != NULL )
             stop_fighting( rch, TRUE );
            }

         }
         act( "\n\r{WThe room is in {rAWE{W of you!{x\n\r",ch,NULL,NULL, TO_CHAR);
         act( "\n\r{WYou are in complete {rAWE{W of $n!{x\n\r",ch,NULL,NULL, TO_ROOM );
         return;
     }
     else
      {
    send_to_char( "\n\r{CYou FAILED to {WAWE{Cthe room with your presence!.\n\r{x",ch );
    return;
      }
 return;
}

void spell_thought_shield ( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
    {
     if ( victim == ch )
       send_to_char( "You are already protected by your mind.\n\r",victim );
     else
       act( "$N is as protected as they are going to get.", ch, NULL, victim, TO_CHAR );
       return;
     }
  
    if (is_affected(victim, skill_lookup("mental barrier")))
      {
        if (victim == ch)
        send_to_char("Your mind is already protected by a stronger Mental Barrier.\n\r",victim);
        else
        act("$N's mind is already protected by a stronger Mental Barrier.", ch, NULL, victim, TO_CHAR);
        return;
      }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level/2;
    af.location  = APPLY_AC;
    af.modifier  = - level/2; 
    af.bitvector = 0;
    affect_to_char( victim, &af );

    af.where     = TO_RESIST;
    af.type      = sn;
    af.level     = level;
    af.duration  = level/2;
    af.location  = APPLY_NONE;
    af.modifier  = 0; 
    af.bitvector = RES_MENTAL;
    affect_to_char( victim, &af );

  if (victim == ch)
    send_to_char( "\x1B[1m\x1B[34mYou have created a thought shield around yourself.\x1B[0m\n\r", ch );
  else
    {
    send_to_char( "\x1B[1m\x1B[34mA Thought Shield has been created around you.\x1B[0m\n\r", victim );
    act("\n\r{C$N is now affected by THOUGHT SHIELD {c({WPSIONIC POWER{c){C.{x\n\r",ch,NULL,victim,TO_CHAR);
    }
    return;
}

void spell_psychic_healing ( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int heal;

    heal = dice( 6, 12 ) + 2 * level /3 ;
    victim->hit = UMIN( victim->hit + heal, victim->max_hit );
    update_pos( victim );

    send_to_char( "\x1B[1m\x1B[34mYou feel better!\x1B[0m\n\r", victim );
    return;
}

void spell_mental_barrier ( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
    {
     if ( victim == ch )
       send_to_char( "You are already protected by your mind.\n\r",victim );
     else
       act( "$N is as protected as they are going to get.", ch, NULL, victim, TO_CHAR );
       return;
     }

     if (is_affected(victim, skill_lookup("thought shield")))
      {
        if (victim == ch)
        send_to_char("Your mind is already protected by a Thought Shield.\n\r",victim);
        else
        act("$N's mind is already protected by a Thought Shield.", ch, NULL, victim, TO_CHAR);
        return;
      }
   
    af.where     = TO_IMMUNE;
    af.type      = sn;
    af.level     = level;
    af.duration  = level/2;
    af.location  = APPLY_NONE;
    af.modifier  = 0; 
    af.bitvector = IMM_MENTAL;
    affect_to_char( victim, &af );
  
    if (victim == ch)
      send_to_char( "\x1B[1m\x1B[34mYou erect a mental barrier around yourself.\x1B[0m\n\r",victim );
  else
    {
     send_to_char( "\x1B[1m\x1B[34mA Mental Barrier has been erected around you.\x1B[0m\n\r",victim);
act("\n\r{C$N is now affected by MENTAL BARRIER {c({WPSIONIC POWER{c){C.{x\n\r",ch,NULL,victim,TO_CHAR);
    }

    return;
}


void spell_share_strength ( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( victim == ch )
    {
        send_to_char( "You can't share strength with yourself.\n\r", ch );
        return;
    }
    if ( is_affected( victim, sn ) )
    {
        act( "$N already shares someone's strength.", ch, NULL, victim,
        TO_CHAR );
        return;
    }
    if ( get_curr_stat( ch, STAT_STR ) <= 5 )
    {
        send_to_char( "You are too weak to share your strength.\n\r", ch );
        return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level;
    af.location  = APPLY_STR;
    af.modifier  = level /5 +1; 
    af.bitvector = 0;
    affect_to_char( victim, &af );

    af.modifier = -level /10 +2;
    affect_to_char( ch, &af );

    act( "\x1B[1m\x1B[34mYou share your strength with $N.\x1B[0m", ch, NULL, victim, TO_CHAR );
    act( "\x1B[1m\x1B[34m$n shares $s strength with you.\x1B[0m", ch, NULL, victim, TO_VICT );
    return;
}

void spell_levitation ( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED( victim, AFF_FLYING ) )
        return;

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level/1.25;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_FLYING;
    affect_to_char( victim, &af );

    send_to_char( "\x1B[1m\x1B[34mYour feet rise off the ground.\x1B[0m\n\r", victim );
    act( "\x1B[1m\x1B[34m$n's feet rise off the ground.\x1B[0m", victim, NULL, NULL, TO_ROOM );

     if (victim != ch)
   { act("\n\r{C$N is now affected by LEVITATION {c({WPSIONIC POWER{c){C.{x\n\r",ch,NULL,victim,TO_CHAR);}
    return;
}

void spell_obfuscate( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED( victim, AFF_INVISIBLE ) )
        return;

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = 48;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_INVISIBLE;
    affect_to_char( victim, &af );

    send_to_char( "\x1B[1m\x1B[34mYou fade out of existence.\x1B[0m\n\r", victim );
    act( "\x1B[1m\x1B[34m$n fades out of existence.\x1B[0m", victim, NULL, NULL, TO_ROOM );

   if (victim != ch)
    { act("\n\r{C$N is now affected by OBFUSCATE {c({WPSIONIC POWER{c){C.{x\n\r",ch,NULL,victim,TO_CHAR); }

    return;
}

void spell_bio_fortress ( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;


   if ( (is_affected(victim, skill_lookup("golden aura")))
   || (is_affected(victim, skill_lookup("sanctuary"))) )
     {
      if (victim == ch)
       { send_to_char("{RBio-Fortress is not compatiable with Sanctuary or Golden Aura!{x\n\r",ch); }
      else
       { send_to_char("{RThey have Sanctuary or Golden Aura, Bio-Fortress will not work!{x\n\r",ch); }

      return; 
     }

   if (is_affected(victim, skill_lookup("bio fortress")))
     {
      if (victim == ch)
        { send_to_char("{RYou are already protected by a Bio-Fortress!{x\n\r",ch); }
      else
        { send_to_char("{RThey are already protected by a Bio-Fortress!{x\n\r",ch); }

      return; 
     }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level /3;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_SANCTUARY;
    affect_to_char( victim, &af );

    send_to_char( "\x1B[1m\x1B[34mYou are surrounded by a form of Bio-Psionic Energy.\x1B[0m\n\r", victim);
    act( "\x1B[1m\x1B[34m$n is surrounded by an unusual energy field .\x1B[0m", victim, NULL, NULL,TO_ROOM );
    if (victim != ch)
{ act("\n\r{C$N is now affected by BIO FORTRESS {c({WPSIONIC POWER{c){C.{x\n\r",ch,NULL,victim,TO_CHAR); }

    return;

}

void spell_cell_adjustment ( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    if ( is_affected( victim, gsn_poison ) )
    {
        affect_strip( victim, gsn_poison );
        send_to_char( "\x1B[1m\x1B[34mA warm feeling runs through your body.\x1B[0m\n\r", victim );
        act( "\x1B[1m\x1B[34m$N looks better.\x1B[0m", ch, NULL, victim, TO_NOTVICT );
    }

    if ( is_affected( victim, gsn_curse ) )
    {
        affect_strip( victim, gsn_curse );
        send_to_char( "\x1B[1m\x1B[34mYou feel better, the curse is lifted.\x1B[0m\n\r", victim );
    }

    send_to_char( "Ok.\n\r", ch );
    return;
}

void spell_displacement ( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
        return;

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level - 4;
    af.location  = APPLY_AC;
    af.modifier  = 4 - level; 
    af.bitvector = 0;
    affect_to_char( victim, &af );

    af.location  = APPLY_DEX;
    af.modifier  = +level /8; 
    affect_to_char( victim, &af );
   
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level /2;
    af.location  = APPLY_NONE;
    af.modifier  = 0; 
    af.bitvector = AFF_PASS_DOOR;
    affect_to_char( victim, &af );

  if (victim == ch)
    {
    send_to_char( "\x1B[1m\x1B[34mYour form shimmers, and you appear displaced.\x1B[0m\n\r", victim );
    act( "\x1B[1m\x1B[34m$N shimmers and appears in a different location.\x1B[0m",
    ch, NULL, victim, TO_NOTVICT );
    }
  else
    {
    send_to_char( "\x1B[1m\x1B[34mYour form shimmers, and you appear displaced.\x1B[0m\n\r", victim );
    act( "\x1B[1m\x1B[34m$N shimmers and appears in a different location.\x1B[0m",
    ch, NULL, victim, TO_NOTVICT );
 act("\n\r{C$N is now affected by DISPLACEMENT {c({WPSIONIC POWER{c){C.{x\n\r",ch,NULL,victim,TO_CHAR); 
    } 
    return;

}

void spell_ectoplasmic_form ( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED( victim, AFF_PASS_DOOR ) )
    {
    send_to_char( "\x1B[1m\x1B[34mYour form shimmers, but nothing happens.\n\rMore than likely it is because you are already out of phase.\x1B[0m\n\r", victim );
    return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level /2;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_PASS_DOOR;
    affect_to_char( victim, &af );

    send_to_char( "\x1B[1m\x1B[34mYou turn translucent.\x1B[0m\n\r", victim );
    act( "\x1B[1m\x1B[34m$n turns translucent.\x1B[0m", victim, NULL, NULL, TO_ROOM );
    if (victim != ch)
{ act("\n\r{C$N is now affected by ECTOPLASMIC FORM {c({WPSIONIC POWER{c){C.{x\n\r",ch,NULL,victim,TO_CHAR); }
    return;
}
void spell_lend_health ( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int hpch;

    if ( ch == victim )
    {
        send_to_char( "Lend health to yourself?  What a weirdo.\n\r", ch );
        return;
    }
    hpch = UMIN( 50, victim->max_hit - victim->hit );
    if ( hpch == 0 )
    {
        act( "Nice thought, but $N doesn't need healing.", ch, NULL,
        victim, TO_CHAR );
        return;
    }
    if ( ch->hit-hpch < 50 )
    {
        send_to_char( "You aren't healthy enough yourself!\n\r", ch );
        return;
    }
    victim->hit += hpch;
    ch->hit -= hpch;
    update_pos( victim );
    update_pos( ch );

    act( "\x1B[1m\x1B[34mYou lend some of your health to $N.\x1B[0m", ch, NULL, victim, TO_CHAR );
    act( "\x1B[1m\x1B[34m$n lends you some of $s health.\x1B[0m", ch, NULL, victim, TO_VICT );

    return;
}

void spell_complete_healing ( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    victim->hit = victim->max_hit;
    victim->move = victim->max_move;
    update_pos( victim );
    if ( ch != victim )
        send_to_char( "Ok.\n\r", ch );
    send_to_char( "\x1B[1m\x1B[34mAhhhhhh...You are completely healed!\x1B[0m\n\r", victim );
    return;
}

void spell_inertial_barrier ( int sn, int level, CHAR_DATA *ch, void *vo, int target  )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( ch, sn ) )
    {
      if (victim == ch)
         send_to_char( "You are already protected by your mind.\n\r",victim );
      else
         act( "$N is as protected as they are going to get.", ch, NULL, victim, TO_CHAR );
      return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level /1.2;
    af.location  = APPLY_AC;
    af.modifier  = -level;
    af.bitvector = 0;
    affect_to_char( ch, &af );
   
    send_to_char( "\x1B[1m\x1B[34mA shimmering barrier forms around you.\x1B[0m\n\r", victim );
    act( "\x1B[1m\x1B[34mA shimmering barrier forms around $N.\x1B[0m", victim, NULL, ch, TO_ROOM );
    if (victim != ch)
{ act("\n\r{C$N is now affected by INERTIAL BARRIER {c({WPSIONIC POWER{c){C.{x\n\r",ch,NULL,victim,TO_CHAR); }
    return;
}

void spell_flesh_armor ( int sn, int level, CHAR_DATA *ch, void *vo, int target  )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
        return;


    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level;
    af.location  = APPLY_AC;
    af.modifier  = -20;
    af.bitvector = 0;
    affect_to_char( victim, &af );

    af.where     = TO_RESIST;
    af.type      = sn;
    af.level     = level;
    af.duration  = level;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = RES_PIERCE;
    affect_to_char( victim, &af );

    af.where     = TO_RESIST;
    af.type      = sn;
    af.level     = level;
    af.duration  = level;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = RES_SLASH;
    affect_to_char( victim, &af );


    
    send_to_char( "\x1B[1m\x1B[34mYour flesh turns to steel.\x1B[0m\n\r", victim );
    act( "\x1B[1m\x1B[34m$N's flesh turns to steel.\x1B[0m", ch, NULL, victim, TO_NOTVICT);
    if (victim != ch)
{ act("\n\r{C$N is now affected by FLESH ARMOR {c({WPSIONIC POWER{c){C.{x\n\r",ch,NULL,victim,TO_CHAR); }
    return;
}

void spell_energy_containment ( int sn, int level, CHAR_DATA *ch, void *vo, int target  )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
        return;

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level /1.5;
    af.location  = APPLY_SAVES;
    af.modifier  = -level /2;
    af.bitvector = 0;
    affect_to_char( victim, &af );

    af.where     = TO_RESIST;
    af.type      = sn;
    af.level     = level;
    af.duration  = level /2;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = RES_ENERGY;
    affect_to_char( victim, &af );

    af.where     = TO_RESIST;
    af.type      = sn;
    af.level     = level;
    af.duration  = level /2;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = RES_LIGHTNING;
    affect_to_char( victim, &af );

    send_to_char( "\x1B[1m\x1B[34mYou can deflect some damage caused by magic easier.\x1B[0m\n\r", victim );
    if (victim != ch)
{ act("\n\r{C$N is now affected by ENERGY CONTAINMENT {c({WPSIONIC POWER{c){C.{x\n\r",ch,NULL,victim,TO_CHAR); }
    return;
}


void spell_enhance_armor (int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    char *msg = "\n\r{W$p is surrounded in a clear aura.{x\n\r";
    int chance = number_percent( );
  
    if ( get_skill( ch, sn ) == 0 )
      {
       send_to_char( "\n\r{WYour mind is not disciplined enough for this.{x\n\r", ch );
       return;
      }
 
    if (obj->item_type != ITEM_ARMOR)
      {
       send_to_char("\n\r{WThat isn't an armor.{x\n\r",ch);
       return;
      }

    if (obj->wear_loc != -1)
      {
       send_to_char("\n\r{WThe item must be carried to be enhanced.{x\n\r",ch);
       return;
      }

    if ( IS_SET( obj->extra_flags, ITEM_MAGIC ) )
      {
       send_to_char( "\n\r{WThis item is already empowered with a different type of energy.{x\n\r", ch );
       return;
      }

    if (IS_OBJ_STAT(obj,ITEM_BLESS))
      chance -= 10;
    
    if (IS_OBJ_STAT(obj,ITEM_GLOW))
      chance -= 5;

    if ( chance > ( get_skill( ch, sn ) ) )
      {  
       send_to_char( "\n\r{WYour mind is in to much turmoil, try meditating before trying this again.{x\n\r", ch );
       return;
      }
      else 
    if ( chance <= ( get_skill( ch, sn ) * .05 ))
      {
       obj->value[0] *= 2;
       obj->value[1] *= 2;
       obj->value[2] *= 2;
       obj->value[3] *= 2;
       SET_BIT( obj->extra_flags, ITEM_MAGIC );
       msg = "\n\r{W$p flares radiantly for a brief moment.{x\n\r";          
       act( msg, ch, obj, NULL, TO_CHAR );
       return;
      }
      else 
    if ( chance <= ( get_skill( ch, sn ) * .15 ))
      {
       obj->value[0] *= 1.75;
       obj->value[1] *= 1.75;
       obj->value[2] *= 1.75;
       obj->value[3] *= 1.75;
       SET_BIT( obj->extra_flags, ITEM_MAGIC );
       msg = "\n\r{W$p shimmers brightly for a brief moment.{x\n\r";          
       act( msg, ch, obj, NULL, TO_CHAR );
       return;
      }
      else
    if ( chance <= ( get_skill( ch, sn ) * .25 ))
      {
       obj->value[0] *= 1.5;
       obj->value[1] *= 1.5;
       obj->value[2] *= 1.5;
       obj->value[3] *= 1.5;
       SET_BIT( obj->extra_flags, ITEM_MAGIC );
       msg = "\n\r{W$p glows warmly for a brief moment.{x\n\r";          
       act( msg, ch, obj, NULL, TO_CHAR );
       return;
      }
      else
      {
       obj->value[0] *= 1.25;
       obj->value[1] *= 1.25;
       obj->value[2] *= 1.25;
       obj->value[3] *= 1.25;
       SET_BIT( obj->extra_flags, ITEM_MAGIC );
       msg = "\n\r{W$p shimmers  for a brief moment.{x\n\r";          
       act( msg, ch, obj, NULL, TO_CHAR );
       return;
      }
    return;
}

/* PSIONIC POWERS - Combat */

void spell_disintegrate( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    char buf  [MAX_STRING_LENGTH];
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    OBJ_DATA *obj, *obj_next;
    int xp;

    if(!IS_NPC(victim))
     {
      send_to_char("Sorry, not on players.\n\r", ch);
      return;
     }
  
   
    if ( number_percent( ) < (2 * level) && !saves_spell(level,victim,DAM_MENTAL ))
        for ( obj = victim->carrying; obj ;obj = obj_next )
        {
         obj_next = obj ->next_content;
         if (obj != NULL)

         if (!IS_OBJ_STAT(obj,ITEM_FORTIFIED))
           extract_obj (obj);
            
         continue;

         if ( number_bits( 2 ) != 0 )
         continue;
   act( "\x1B[1m\x1B[32m$p disintegrates!\x1B[0m", victim, obj, NULL,TO_CHAR );
   act( "\x1B[1m\x1B[32m$n's $p disintegrates!\x1B[0m", victim, obj, NULL, TO_ROOM );

         if (!IS_OBJ_STAT(obj,ITEM_FORTIFIED))
           extract_obj( obj ) ;
        }

    if ( !saves_spell( level, victim, DAM_MENTAL ) )
    {
      act( "\x1B[1m\x1B[32mYou have DISINTEGRATED $N!\x1B[0m", ch, NULL,victim, TO_CHAR );
      act( "\x1B[1m\x1B[32mYou have been DISINTEGRATED by $n!\x1B[0m", ch, NULL, victim, TO_VICT );
      act( "\x1B[1m\x1B[32m$n's spell DISINTEGRATES $N!\x1B[0m", ch, NULL, victim, TO_ROOM );
      
       xp = ( number_range( victim->level /5, 1.5 * (victim->level / 2 ) ));
       sprintf ( buf, "{YYou receive {w%d {Yexperience points.\n\r{x", xp);
       {
        send_to_char (buf, ch);
        gain_exp(ch, xp); 
       }

       if ( IS_NPC( victim ) )
       extract_char( victim, TRUE );
       else
       extract_char( victim, FALSE );

     }
    return;
}


void spell_essence_drain( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    if ( saves_spell( level, victim, DAM_MENTAL ) )
        return;

/*
    ch->alignment = UMAX(-1000, ch->alignment - 200);
*/
    if ( victim->level <= 2 )
    {
        dam = ch->hit + 50;
    }
    else
    {
        gain_exp( victim, 0 - number_range( level /2, 3 * level /2 ) );
        victim->mana /= 2;
        victim->move /= 2;
        dam = dice( 1, level );
        ch->hit += dam;
    }

    damage( ch, victim, dam, sn, DAM_MENTAL, TRUE );

    return;
}

void spell_agitation ( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    static const int dam_each [ ] =
        {
        0,
        0, 0, 0, 0, 0, 50, 50, 50, 51, 51,
        51, 52, 52, 52, 53, 53, 53, 54, 54, 54,
        55, 55, 55, 56, 56, 56, 57, 57, 57, 58,
        58, 58, 59, 59, 59, 60, 60, 60, 61, 61,
        61, 62, 62, 62, 63, 63, 63, 64, 64, 64
    };
    int dam;
    level = UMIN( level, sizeof( dam_each ) /sizeof( dam_each[0] ) - 1 );
    level = UMAX( 0, level );
    dam = number_range( dam_each[level] /2, dam_each[level] * 2 );

    if ( saves_spell( level, victim, DAM_MENTAL ) )
        dam /= 2;
    damage( ch, victim, dam, sn, DAM_MENTAL, TRUE );

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level /3;
    af.location  = APPLY_DEX;
    af.modifier  = -3 + level/20;
    af.bitvector = 0;
    affect_to_char( victim, &af );

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level /3;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_SLOW;
    affect_to_char( victim, &af );
    return;
}

void spell_ballistic_attack ( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    static const int dam_each [ ] =
        {
        0,
        3, 4, 4, 5, 6, 6, 6, 7, 7, 7,
        7, 7, 8, 8, 8, 9, 9, 9, 10, 10,
        10, 11, 11, 11, 12, 12, 12, 13, 13, 13,
        14, 14, 14, 15, 15, 15, 16, 16, 16, 17,
        17, 17, 18, 18, 18, 19, 19, 19, 20, 20
    };
    int dam;

    level = UMIN( level, sizeof( dam_each ) /sizeof( dam_each[0] ) - 1 );
    level = UMAX( 0, level );
    dam = number_range( dam_each[level] /2, dam_each[level] * 2 );
    if ( saves_spell( level, victim, DAM_MENTAL ) )
        dam /= 2;
    act( "You chuckle as a stone strikes $N.", ch, NULL, victim,
    TO_CHAR );
    damage( ch, victim, dam, sn, DAM_MENTAL, TRUE );
    return;
}

void spell_control_flames ( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
            AFFECT_DATA af;

    static const int dam_each [ ] = 
        {
        0,
        0, 0, 0, 0, 0, 0, 0, 64, 66, 66,
        66, 68, 68, 68, 70, 70, 70, 72, 72, 72,
        74, 74, 74, 76, 76, 76, 78, 78, 78, 80,
        80, 80, 82, 82, 82, 84, 84, 84, 86, 86,
        86, 88, 88, 88, 89, 89, 89, 90, 90, 90
    };
    int dam;

    if (!get_eq_char( ch, WEAR_LIGHT )) 
    {
        send_to_char( "\n\r{CYou must be carrying a light source...{x\n\r", ch );
        return;
    }

    level = UMIN( level, sizeof( dam_each ) /sizeof( dam_each[0] ) - 1 );
    level = UMAX( 0, level );
    dam = number_range( dam_each[level] /2, dam_each[level] * 2 );
    if ( saves_spell( level, victim, DAM_MENTAL ) )
        dam /= 2;
    damage( ch, victim, dam, sn, DAM_MENTAL, TRUE  );

    af.where     = TO_VULN;
    af.type      = sn;
    af.level     = level;
    af.duration  = level /2;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = VULN_FIRE;
    affect_to_char( victim, &af );
    return;
}

void spell_death_field ( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    int dam;
    int hpch;

    if ( !IS_EVIL( ch ) && !IS_NEUTRAL(ch) )
    {
        send_to_char( "You are not evil enough to do that!\n\r", ch);
        return;
    }

    send_to_char( "\x1B[1m\x1B[32mA black haze emanates from you!\x1B[0m\n\r", ch );
    act ( "\x1B[1m\x1B[32mA black haze emanates from $n!\x1B[0m", ch,
NULL, ch, TO_ROOM );

for ( vch = ch->in_room->people; vch; vch = vch_next )
    {
        
        vch_next = vch->next_in_room;

	if (is_safe_spell(ch,vch,TRUE) 
	||  (IS_NPC(vch) && IS_NPC(ch) 
	&&   (ch->fighting != vch || vch->fighting != ch)))
	    continue;

        if ( !IS_NPC( ch ) ? IS_NPC( vch ) : IS_NPC( vch ) )
        {
            hpch = URANGE( 20, ch->hit, 1500 );
            if ( !saves_spell( level, vch, DAM_MENTAL )
                && ( level <= vch->level + 5
                && level >= vch->level - 5 ) )
            {
                dam = 8; /* Enough to compensate for sanct. and prot. */
                vch->hit = 1;
                update_pos( vch );
                send_to_char( "\x1B[1m\x1B[32mThe haze envelops you!\x1B[0m\n\r", vch );
                act( "\x1B[1m\x1B[32mThe haze envelops $N!\x1B[0m", ch, NULL, vch, TO_ROOM );
            }
            else
                dam = number_range( hpch /6 + 1, hpch /3);

            damage( ch, vch, dam, sn, DAM_MENTAL, TRUE  );
        }
    }
    return;
}

void spell_detonate ( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    static const int dam_each [ ] =
        {
        0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 148,
        150, 150, 152, 152, 154, 154, 156, 156, 158, 158,
        160, 160, 162, 162, 164, 164, 166, 166, 168, 168,
        170, 170, 172, 172, 174, 174, 176, 176, 178, 180
    };
    int dam;

    level = UMIN( level, sizeof( dam_each ) /sizeof( dam_each[0] ) - 1 );
    level = UMAX( 0, level );
    dam = number_range( dam_each[level] /2, dam_each[level] * 2 );
    if ( saves_spell( level, victim, DAM_MENTAL ) )
        dam /= 2;
    damage( ch, victim, dam, sn, DAM_MENTAL, TRUE  );
    return;
}

void spell_ego_whip ( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) || saves_spell( level, victim, DAM_MENTAL ) )
        return;

    af.where     = TO_AFFECTS;
    af.type = sn;
    af.level     = level;
    af.duration = level;
    af.location = APPLY_HITROLL;
    af.modifier = -level /6;
    af.bitvector = 0;
    affect_to_char( victim, &af );

    af.location = APPLY_DAMROLL;
    af.modifier = -level /6;
    affect_to_char( victim, &af );

    af.location = APPLY_SAVES;
    af.modifier = level/6;
    affect_to_char( victim, &af );

    af.location = APPLY_AC;
    af.modifier = level *1.5;
    affect_to_char( victim, &af );

    act( "\x1B[1m\x1B[32mYou ridicule $N about $S childhood.\x1B[0m", ch, NULL, victim, TO_CHAR );
    send_to_char( "\x1B[1m\x1B[32mYour ego takes a beating.\x1B[0m\n\r", victim );
    act( "\x1B[1m\x1B[32m$N's ego is crushed by $n!\x1B[0m", ch, NULL, victim, TO_NOTVICT );
    return;
}

void spell_inflict_pain ( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    damage( ch, (CHAR_DATA *) vo, dice( 4, 20 ) + level *1.75, sn, DAM_MENTAL, TRUE  );
    return;
}

void spell_mind_thrust ( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    damage( ch, (CHAR_DATA *) vo, dice( 2, 12 ) + level, sn, DAM_MENTAL, TRUE  );
    return;
}

void spell_project_force ( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    damage( ch, (CHAR_DATA *) vo, dice( 10, 34 ) + level * 3, sn, DAM_MENTAL, TRUE  );
    return;
}

void spell_psionic_blast ( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    static const int dam_each [ ] =
        {
        0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 225, 225, 227, 227,
        229, 229, 231, 231, 233, 233, 235, 235, 237, 237,
        239, 239, 241, 241, 243, 243, 245, 245, 247, 247,
        259, 259, 251, 251, 253, 253, 255, 255, 257, 260

    };
    int dam;


    level = UMIN( level, sizeof( dam_each ) /sizeof( dam_each[0] ) - 1 );
    level = UMAX( 0, level );
    dam = number_range( dam_each[level] /2, dam_each[level] * 2 );
    if ( saves_spell( level, victim, DAM_MENTAL ) )
        dam /= 2;
    damage( ch, victim, dam, sn, DAM_MENTAL, TRUE  );
    
    if ( !is_affected( victim, sn ) )
     {
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level /2;
    af.location  = APPLY_INT;
    af.modifier  = -level /6;
    af.bitvector = 0;
    affect_to_char( victim, &af );

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level /2;
    af.location  = APPLY_WIS;
    af.modifier  = -level /6;
    af.bitvector = 0;
    affect_to_char( victim, &af );
     }

    return;
}

void spell_psychic_crush ( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    damage( ch, (CHAR_DATA *) vo, dice( 8, 28 ) + level *2.25, sn, DAM_MENTAL, TRUE  );
    return;
}

void spell_psychic_drain ( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) || saves_spell( level, victim, DAM_MENTAL ) )
        return;

    af.type = sn;
    af.duration = level /2;
    af.location = APPLY_STR;
    af.modifier = - level/8+5;
    af.bitvector = 0;
    affect_to_char( victim, &af );

    send_to_char( "\x1B[1m\x1B[32mYou feel drained.\x1B[0m\n\r", victim );
    act( "\x1B[1m\x1B[32m$n appears drained of strength.\x1B[0m", victim, NULL, NULL, TO_ROOM );
    return;
}

void spell_ultrablast ( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    int dam;
    int hpch;

    for ( vch = ch->in_room->people; vch; vch = vch_next )
    {
        vch_next = vch->next_in_room;

	if (is_safe_spell(ch,vch,TRUE) 
	||  (IS_NPC(vch) && IS_NPC(ch) 
	&&   (ch->fighting != vch || vch->fighting != ch)))
	    continue;

        if ( IS_NPC( ch ) ? !IS_NPC( vch ) : IS_NPC( vch ) )
        {
            hpch = UMAX( 10, ch->hit );
            dam = number_range( hpch /6+1, hpch /3 );
            if ( saves_spell( level, vch, DAM_MENTAL ) )
                dam /= 2;
            damage( ch, vch, dam, sn, DAM_MENTAL, TRUE  );
        }
    }
    return;
}

/* New Spells */
void spell_forsake( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim = (CHAR_DATA *)vo;
  AFFECT_DATA af;
  if ( is_affected( victim, sn ) )
    {
    act( "\n\r{G$N has already been forsaken.{x\n\r", ch, NULL, victim, TO_CHAR );
    return;
    }
  if ( IS_EVIL( ch ) )
    level += 5;
  if ( IS_GOOD( victim ) )
    level += 5;
  if ( saves_spell( level, victim, DAM_NEGATIVE ) )
    {
    act( "\n\r{G$N's God protects them from your unholy wrath!\n\r{x", ch, NULL, victim, TO_CHAR);
    return;
    }
  af.where     = TO_AFFECTS;
  af.type      = sn;
  af.level     = level;
  af.duration  = 10;
  af.location  = APPLY_MOVE;
  af.modifier  = 0 - ( victim->move - victim->level );
  af.bitvector = AFF_CURSE;
  affect_to_char( victim, &af );
  
  af.where     = TO_AFFECTS;
  af.type      = sn;
  af.level     = level;
  af.duration  = UMAX( 3, level / 25 );
  af.location  = APPLY_HITROLL;
  af.modifier  = 0 - ( 10 + ( af.duration * 4 ) );
  af.bitvector = AFF_BLIND;
  affect_to_char( victim, &af );

  if ( ch->move > ch->max_move )
    ch->move = ch->max_move;

  act( "\n\r{D$n calls upon the wrath of $s Unholy Gods and forsakes you!{x\n\r", ch,NULL,victim,TO_VICT );
  act( "\n\r{D$n calls upon $s Unholy Gods to forsake $N!{x\n\r", ch, NULL, victim, TO_NOTVICT);
  act( "\n\r{DYou call upon the wrath of your Unholy Gods and forsake $N!{x\n\r", ch, NULL,victim, TO_CHAR );
  return;
}

void spell_divine_wrath( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( is_affected( victim, sn ) )
    return;

  if ( saves_spell( level, victim, DAM_HOLY ) )
    {
    act( "\n\r{W$N's God protects him from the Wrath of Yours!{x\n\r", ch, NULL, victim, TO_CHAR);
    return;
    }

  af.where     = TO_AFFECTS;
  af.type      = sn;
  af.level     = level;
  af.duration  = UMAX( 3, level / 20 );
  af.location  = APPLY_HITROLL;
  af.modifier  = 0 - ( 10 + ( af.duration * 5 ) );
  af.bitvector = AFF_BLIND;
  affect_to_char( victim, &af );

  af.location  = APPLY_SAVES;
  af.modifier  = 5 + af.duration;
  af.bitvector = AFF_CURSE;
  affect_to_char( victim, &af );

  af.location  = APPLY_STR;
  af.modifier  = 0 - ( af.duration * 2 );
  af.bitvector = AFF_WEAKEN;
  affect_to_char( victim, &af );

  send_to_char("\n\r{G%n's God looks upon you with {WGREAT{G disfavor!{x\n\r", victim );
  act( "\n\r{GYour God unleashes their full {WWRATH{G upon $N !{x\n\r", ch, NULL, victim,TO_CHAR );
  return;
} 



void spell_immdis_magic( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
/*    bool found = FALSE; */


if (IS_IMP(ch))
  {
    level += 400; 

/*
if (check_dispel(level,victim,skill_lookup("berserk")))
found = TRUE;

if (check_dispel(level,victim,skill_lookup("chill touch")))
found = TRUE;

if (check_dispel(level,victim,skill_lookup("adrenaline control")))
found = TRUE;

if (check_dispel(level,victim,skill_lookup("armor")))
found = TRUE;

if (check_dispel(level,victim,skill_lookup("aura sight")))
found = TRUE;

if (check_dispel(level,victim,skill_lookup("bio fortress")))
found = TRUE;

if (check_dispel(level,victim,skill_lookup("bless")))
found = TRUE;

if (check_dispel(level,victim,skill_lookup("blindness")))
found = TRUE;

if (check_dispel(level,victim,skill_lookup("calm")))
found = TRUE;

if (check_dispel(level,victim,skill_lookup("changE sex")))
found = TRUE;

if (check_dispel(level,victim,skill_lookup("charm person")))
found = TRUE;

if (check_dispel(level,victim,skill_lookup("combat mind")))
found = TRUE;

if (check_dispel(level,victim,skill_lookup("curse")))
found = TRUE;

if (check_dispel(level,victim,skill_lookup("death field")))
found = TRUE;

if (check_dispel(level,victim,skill_lookup("detect evil")))
found = TRUE;

if (check_dispel(level,victim,skill_lookup("detect good")))
found = TRUE;

if (check_dispel(level,victim,skill_lookup("detect hidden")))
found = TRUE;

if (check_dispel(level,victim,skill_lookup("detect invis")))
found = TRUE;

if (check_dispel(level,victim,skill_lookup("detect magic")))
found = TRUE;

if (check_dispel(level,victim,skill_lookup("displacement")))
found = TRUE;

if (check_dispel(level,victim,skill_lookup("divine wrath")))
found = TRUE;

if (check_dispel(level,victim,skill_lookup("domination")))
found = TRUE;

if (check_dispel(level,victim,skill_lookup("ectoplasmic form")))
found = TRUE;

if (check_dispel(level,victim,skill_lookup("energy containment")))
found = TRUE;

if (check_dispel(level,victim,skill_lookup("enhanced strength")))
found = TRUE;

if (check_dispel(level,victim,skill_lookup("faerie fire")))
found = TRUE;

if (check_dispel(level,victim,skill_lookup("faerie fog")))
found = TRUE;

if (check_dispel(level,victim,skill_lookup("flesh armor")))
found = TRUE;

if (check_dispel(level,victim,skill_lookup("fly")))
found = TRUE;

if (check_dispel(level,victim,skill_lookup("frenzy")))
found = TRUE;

if (check_dispel(level,victim,skill_lookup("giant strength")))
found = TRUE;

if (check_dispel(level,victim,skill_lookup("golden aura")))
found = TRUE;

if (check_dispel(level,victim,skill_lookup("haste")))
found = TRUE;

if (check_dispel(level,victim,skill_lookup("inertial barrier")))
found = TRUE;

if (check_dispel(level,victim,skill_lookup("infravision")))
found = TRUE;

if (check_dispel(level,victim,skill_lookup("invisibility")))
found = TRUE;

if (check_dispel(level,victim,skill_lookup("know alignment")))
found = TRUE;

if (check_dispel(level,victim,skill_lookup("lend health")))
found = TRUE;

if (check_dispel(level,victim,skill_lookup("levitation")))
found = TRUE;

if (check_dispel(level,victim,skill_lookup("mass invis")))
found = TRUE;

if (check_dispel(level,victim,skill_lookup("mental barrier")))
found = TRUE;

if (check_dispel(level,victim,skill_lookup("obfuscate")))
found = TRUE;

if (check_dispel(level,victim,skill_lookup("pass door")))
found = TRUE;

if (check_dispel(level,victim,skill_lookup("plague")))
found = TRUE;

if (check_dispel(level,victim,skill_lookup("poison")))
found = TRUE;

if (check_dispel(level,victim,skill_lookup("protection evil")))
found = TRUE;

if (check_dispel(level,victim,skill_lookup("protection good")))
found = TRUE;

if (check_dispel(level,victim,skill_lookup("sanctuary")))
found = TRUE;

if (check_dispel(level,victim,skill_lookup("share strength")))
found = TRUE;

if (check_dispel(level,victim,skill_lookup("shield")))
found = TRUE;

if (check_dispel(level,victim,skill_lookup("sleep")))
found = TRUE;

if (check_dispel(level,victim,skill_lookup("slow")))
found = TRUE;

if (check_dispel(level,victim,skill_lookup("stone skin")))
found = TRUE;

if (check_dispel(level,victim,skill_lookup("thought shield")))
found = TRUE;

if (check_dispel(level,victim,skill_lookup("weaken")))
found = TRUE;

    if (found)
        send_to_char("Ok.\n\r",ch);
    else
        send_to_char("Spell failed.\n\r",ch);
  }
 else
  {
    while ( victim->affected )
        affect_remove( victim, victim->affected );
    victim->affected_by = race_table[victim->race].aff;
    victim->affected2_by = race_table[victim->race].aff2;
  }
*/

    while ( victim->affected )
        affect_remove( victim, victim->affected );
    victim->affected_by = race_table[victim->race].aff;
    victim->affected2_by = race_table[victim->race].aff2;
 
    act("\n\r{GALL Affects have been stripped from TARGET.{x\n\r",ch,NULL,victim,TO_CHAR);
  }
 else
   {
    send_to_char("\n\r{GH{gu{GH{g!!{x\n\r",ch);
    return;
   }

    return;
}

void spell_imprint( int sn, int level, CHAR_DATA *ch, void *vo )
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    int       sp_slot, i, mana;
    char      buf[ MAX_STRING_LENGTH ];

      if (skill_table[sn].spell_fun == spell_null )
     {
	send_to_char("\n\r{WThat is not a spell.{x\n\r",ch);
	return;
     }

    /* counting the number of spells contained within */

    for (sp_slot = i = 1; i < 4; i++) 
	if (obj->value[i] != -1)
	    sp_slot++;

    if (sp_slot > 1)
    {
	act ("\n\r{B$p cannot contain any more spells.{x\n\r", ch, obj, NULL, TO_CHAR);
	return;
    }

    if(2+ch->level-skill_table[sn].skill_level[ch->class] != 0)
{
mana = 3*UMAX(skill_table[sn].min_mana, 100 / ( 2 + ch->level - 
		skill_table[sn].skill_level[ch->class] ) );
}
else
{
mana=3*UMAX(skill_table[sn].min_mana, 100);
}
	    
    if ( !IS_NPC(ch) && ch->mana < mana )
    {
	send_to_char( "\n\r{GYou don't have enough mana.{x\n\r", ch );
	return;
    }
 
    if ( number_percent( ) > ch->pcdata->learned[sn] )
    {
	send_to_char( "{W\n\rYou lost your concentration.{x\n\r", ch );
	ch->mana -= mana / 2;
	return;
    }

    /* executing the imprinting process */
    ch->mana -= mana;
    obj->value[sp_slot] = sn;

    /* Making it successively harder to pack more spells into potions or 
       scrolls - JH */ 

    switch( sp_slot )
   {
   
    default:
	bug( "sp_slot has more than %d spells.", sp_slot );
	return;

    case 1:
        if ( number_percent() > 90 )
       { 
        sprintf(buf, "\n\r{RThe magic enchantment has failed --- the %s vanishes.{x\n\r",
          item_name(obj->item_type) );
	  send_to_char( buf, ch );
	  extract_obj( obj );
	  return;
	 }     
	break;
    case 2:
        if ( number_percent() > 55 )
       { 
        sprintf(buf, "\n\r{RThe magic enchantment has failed --- the %s vanishes.{x\n\r", 
          item_name(obj->item_type) );
	  send_to_char( buf, ch );
	  extract_obj( obj );
	  return;
	 }     
	break;

    case 3:
        if ( number_percent() > 35 )
       { 
        sprintf(buf, "\n\r{RThe magic enchantment has failed --- the %s vanishes.{x\n\r",
          item_name(obj->item_type) );
	  send_to_char( buf, ch );
	  extract_obj( obj );
	  return;
  	 }      
	break;
    } 
  

    /* labeling the item */

    free_string (obj->short_descr);
    sprintf ( buf, "a %s of ", item_name(obj->item_type) ); 
    for (i = 1; i <= sp_slot ; i++)
      if (obj->value[i] != -1)
      {
	 strcat (buf, skill_table[obj->value[i]].name);
       (i != sp_slot ) ? strcat (buf, ", ") : strcat (buf, "") ; 
      }
    obj->short_descr = str_dup(buf);
	
    sprintf( buf, "%s %s", obj->name, item_name(obj->item_type) );
    free_string( obj->name );
    obj->name = str_dup( buf );        

    sprintf(buf, "\n\r{MYou have imbued a new spell to the %s.{x\n\r", 
    item_name(obj->item_type) );
    send_to_char( buf, ch );

    return;
}  


void spell_create_zombie( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    OBJ_DATA *obj;
    CHAR_DATA *mob;
    int i;

    obj = get_obj_here( ch, target_name );

    if ( obj == NULL )
    {
        send_to_char( "\n\r{GCreate Zombie from what?{x\n\r", ch );
        return;
    }

    /* Nothing but NPC corpses. */


    if ( !str_cmp (obj->short_descr, "the corpse of a zombie"))
    {
        send_to_char( "\n\r{GThis corpse has previously been animated!{x\n\r", ch );
        return;
    }

        if (IS_SET(ch->in_room->room_flags, ROOM_DRAGONPIT))
          {
send_to_char("\n\r{GSomething is blocking your magic, maybe it is the DragonLord.{x\n\r",ch);
           return;
          }

    if( obj->item_type != ITEM_CORPSE_NPC )
    {
        if( obj->item_type == ITEM_CORPSE_PC )
            send_to_char( "\n\r{GYou can't create zombies from player corpses.{x\n\r", ch );
        else
            send_to_char( "\n\r{GIt would serve no purpose...{x\n\r", ch );
        return;
    }

    if (!IS_IMP(ch))
      {
    if( obj->level > (ch->level + 5) 
    || (obj->level > 100) )
    {
        send_to_char( "\n\r{GYou couldn't call forth such a great spirit.{x\n\r", ch );
        return;
    }
      }

    if( ch->pet != NULL )
    {
        send_to_char( "\n\r{GYou already have a pet.{x\n\r", ch );
        return;
    }

    /* Chew on the zombie a little bit, recalculate level-dependant stats */

    mob = create_mobile( get_mob_index( MOB_VNUM_ZOMBIE ) );
    mob->level                  = obj->level;
    mob->max_hit                = mob->level * 10 + number_range(
                                        mob->level * mob->level/4,
                                        mob->level * mob->level);

    mob->max_hit *= 1.1;
    mob->hit                    = mob->max_hit;
    mob->max_mana               = 100 + dice(mob->level,10);
    mob->mana                   = mob->max_mana;
    mob->hitroll		= mob->level * 4.5;
    mob->damroll		=(mob->hitroll / 3.5);


    for (i = 0; i < 3; i++)
    mob->armor[i]           = interpolate(mob->level,100,-100);
    mob->armor[3]               = interpolate(mob->level,100,0);

    for (i = 0; i < MAX_STATS; i++)
    mob->perm_stat[i] = 20 + mob->level/3;

    /* You rang? */
    char_to_room( mob, ch->in_room );
    act( "\n\r{G$p springs to life as a hideous zombie!{x\n\r", ch, obj, NULL, TO_ROOM );
    act( "\n\r{G$p springs to life as a hideous zombie!{x\n\r", ch, obj, NULL, TO_CHAR );
    extract_obj(obj);

 
    /* Yessssss, massssssster... */
    SET_BIT(mob->affected_by, AFF_CHARM);
    SET_BIT(mob->act, ACT_PET);
    mob->comm = COMM_NOCHANNELS;
    add_follower( mob, ch );
    mob->leader = ch;
    ch->pet = mob;

    /* For a little flavor... */
    send_to_char ("\n\r",ch);
    do_say( mob, "How may I serve you, master?" );
    return;
}


void spell_drain_blade( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    OBJ_DATA * obj = (OBJ_DATA *) vo;
    AFFECT_DATA af;
   
    if(obj->item_type != ITEM_WEAPON) {
	send_to_char("You can only cast this spell on weapons.\n\r",ch);
	return;
    }

    if(IS_WEAPON_STAT(obj,WEAPON_VAMPIRIC)) {
	send_to_char("This weapon is already vampiric.\n\r", ch);
	return;
    }

    if(IS_OBJ_STAT(obj,ITEM_BLESS)) {
	send_to_char("This weapon is too blessed.\n\r", ch);
	return;
    }

    af.where    = TO_WEAPON;
    af.type     = sn;
    af.level    = 20;
    af.duration = 50;
    af.location = 0;
    af.modifier = 0;
    af.bitvector = WEAPON_VAMPIRIC;
    affect_to_obj(obj, &af);

    act("$p carried by $n turns dark and vampiric.", ch, obj, NULL, TO_ROOM);
    act("$p carried by you turns dark and vampiric.", ch, obj, NULL, TO_CHAR);
    return;
}

void spell_shocking_blade( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    OBJ_DATA * obj = (OBJ_DATA *) vo;
    AFFECT_DATA af;

    if(obj->item_type != ITEM_WEAPON) {
	send_to_char("You can only cast this spell on weapons.\n\r",ch);
	return;
    }

    if(IS_WEAPON_STAT(obj,WEAPON_SHOCKING)) {
	send_to_char("This weapon is already electrical.\n\r", ch);
	return;
    }

    af.where    = TO_WEAPON;
    af.type     = sn;
    af.level    = 20;
    af.duration = 50;
    af.location = 0;
    af.modifier = 0;
    af.bitvector = WEAPON_SHOCKING;
    affect_to_obj(obj, &af);

    act("$p carried by $n sparks with electricity.", ch, obj, NULL, TO_ROOM);
    act("$p carried by you sparks with electricity.", ch, obj, NULL, TO_CHAR);
    return;
}


void spell_flame_blade( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    AFFECT_DATA af;

    if(obj->item_type != ITEM_WEAPON) {
	send_to_char("You can only cast this spell on weapons.\n\r",ch);
	return;
    }

    if(IS_WEAPON_STAT(obj,WEAPON_FLAMING)) {
	send_to_char("This weapon is already flaming.\n\r", ch);
	return;
    }

    if(IS_WEAPON_STAT(obj,WEAPON_FROST)) {
	send_to_char("This weapon is too frost to handle this magic.\n\r", ch);
	return;
    }

    af.where    = TO_WEAPON;
    af.type     = sn;
    af.level    = 20;
    af.duration = 50;
    af.location = 0;
    af.modifier = 0;
    af.bitvector = WEAPON_FLAMING;
    affect_to_obj(obj, &af);

    act("$p carried by $n gets a fiery aura.", ch, obj, NULL, TO_ROOM);
    act("$p carried by you gets a fiery aura.", ch, obj, NULL, TO_CHAR);
    return;
}

void spell_frost_blade( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    AFFECT_DATA af;

    if(obj->item_type != ITEM_WEAPON) {
	send_to_char("You can only cast this spell on weapons.\n\r",ch);
	return;
    }

    if(IS_WEAPON_STAT(obj,WEAPON_FROST)) {
	send_to_char("This weapon is already frost.\n\r", ch);
	return;
      }

    if(IS_WEAPON_STAT(obj,WEAPON_FLAMING)) {
	send_to_char("This weapon is too cold to handle this magic.\n\r", ch);
	return;
    }

    af.where    = TO_WEAPON;
    af.type     = sn;
    af.level    = 20;
    af.duration = 50;
    af.location = 0;
    af.modifier = 0;
    af.bitvector = WEAPON_FROST;
    affect_to_obj(obj, &af);

    act("$p carried by $n grows wickedly cold.", ch, obj, NULL, TO_ROOM);
    act("$p carried by you grows wickedly cold.", ch, obj, NULL, TO_CHAR);
    return;
}

void spell_sharp_blade( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    AFFECT_DATA af;

    if(obj->item_type != ITEM_WEAPON) {
	send_to_char("\n\r{WYou can only cast this spell on weapons.{x\n\r",ch);
	return;
    }

    if(IS_WEAPON_STAT(obj,WEAPON_SHARP)) 
      {
	send_to_char("\n\r{WThis weapon is already sharp.{x\n\r", ch);
	return;
    }

    af.where    = TO_WEAPON;
    af.type     = sn;
    af.level    = 20;
    af.duration = 50;
    af.location = 0;
    af.modifier = 0;
    af.bitvector = WEAPON_SHARP;
    affect_to_obj(obj, &af);

    act("\n\r{W$p carried by $n looks newly honed.{x\n\r", ch, obj, NULL, TO_ROOM);
    act("\n\r{W$p carried by you looks newly honed.{x\n\r", ch, obj, NULL, TO_CHAR);
    return;
}

void spell_vorpal_blade( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    AFFECT_DATA af;

    if(obj->item_type != ITEM_WEAPON) {
	send_to_char("You can only cast this spell on weapons.\n\r",ch);
	return ;
    }

    if(IS_WEAPON_STAT(obj,WEAPON_VORPAL)) {
	send_to_char("This weapon is already vorpal.\n\r", ch);
	return;
    }

    af.where    = TO_WEAPON;
    af.type     = sn;
    af.level    = level/2;
    af.duration = level;
    af.location = 0;
    af.modifier = 0;
    af.bitvector = WEAPON_VORPAL;
    affect_to_obj(obj, &af);

    act("$p carried by $n gleams with magical strengh.", ch, obj, NULL, TO_ROOM);
    act("$p carried by you gleams with magical strengh.", ch, obj, NULL, TO_CHAR);
    return;
} 
 

void spell_poison_blade( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    AFFECT_DATA af;

    if(obj->item_type != ITEM_WEAPON) {
	send_to_char("You can only cast this spell on weapons.\n\r",ch);
	return ;
    }

    if(IS_WEAPON_STAT(obj,WEAPON_POISON)) {
	send_to_char("This weapon is already poisoned.\n\r", ch);
	return;
    }

    af.where    = TO_WEAPON;
    af.type     = sn;
    af.level    = level;
    af.duration = level;
    af.location = 0;
    af.modifier = 0;
    af.bitvector = WEAPON_POISON;
    affect_to_obj(obj, &af);

    act("$p carried by $n drips with a deadly venom.", ch, obj, NULL, TO_ROOM);
    act("$p carried by you drips with a deadly venom.", ch, obj, NULL, TO_CHAR);
    return;
} 

void spell_metamorph( int sn, int level, CHAR_DATA *ch, void *vo, int target )
 {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
     int vnum;
 

    if (victim == ch)
     {
      send_to_char("\n\r{RWhy would you want to do this to yourself?{x\n\r", ch);
      return;
     }

     if (victim->level > ch->level)
     {
 	send_to_char( "\n\r{RYou failed.{x\n\r", ch );
 	return;
     }

    if(!IS_NPC(victim))
     {
      send_to_char("\n\r{RSorry, not on players.{x\n\r", ch);
      return;
     }
 
     if ( saves_spell( level, victim,DAM_MAGIC) )
       {
  	  send_to_char( "\n\r{RYou failed.{x\n\r", ch );
        return;
       }

     switch( number_bits(4) )
     {
 	case 0: vnum = MOB_VNUM_CAT; break;
 	case 1: vnum = MOB_VNUM_FIDO; break;
 	case 2: vnum = MOB_VNUM_COW; break;
 	case 3: vnum = MOB_VNUM_WOLF; break;
 	case 4: vnum = MOB_VNUM_BEAR; break;
 	case 5: vnum = MOB_VNUM_RABBIT; break;
 	case 6: vnum = MOB_VNUM_SNAIL; break;
 	case 7: vnum = MOB_VNUM_BOAR; break;
 	default: vnum = MOB_VNUM_SLIME; break;
     }
 
     extract_char( victim, TRUE );
     victim = create_mobile( get_mob_index( vnum ) );
     char_to_room( victim, ch->in_room );
     act( "\n\r{W$N has been transformed.{x\n\r", ch, NULL, victim ,TO_ROOM );
     do_look( ch, "auto" );
     return;
 }




/* Mage Combat Spells */
void spell_magic_missile( int sn, int level, CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
    
    dam = dice( level, 3 );
    if ( saves_spell( level, victim, DAM_MAGIC) )     
        dam /= 2;
    damage( ch, victim, dam, sn, DAM_MAGIC, TRUE);     
    return;
}

void spell_chill_touch( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
  int dam;
    
    dam = dice( level, 4 );

    if ( saves_spell( level, victim, DAM_COLD ) )     
        dam /= 2;

    if ( !saves_spell( level, victim,DAM_COLD ) )
    {
	act("\n\t{B$n turns blue and shivers.{x\n\r",victim,NULL,NULL,TO_ROOM);
	af.where     = TO_AFFECTS;
	af.type      = sn;
        af.level     = level;
	af.duration  = level/5;
	af.location  = APPLY_STR;
	af.modifier  = 0 - (level/20);
	af.bitvector = 0;
	affect_to_char( victim, &af );
    }
    else
    {
	dam /= 2;
    }

/*    damage( ch, victim, dam, sn, DAM_COLD, TRUE);     */
    damage( ch, victim, dam, sn, DAM_MAGIC, TRUE);     
    return;
}

void spell_burning_hands(int sn,int level, CHAR_DATA *ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
    
    dam = dice( level, 6 );
/*
    if ( saves_spell( level, victim, DAM_FIRE ) )     
        dam /= 2;
    damage( ch, victim, dam, sn, DAM_FIRE, TRUE);     
*/
    if ( saves_spell( level, victim, DAM_MAGIC ) )     
        dam /= 2;
    damage( ch, victim, dam, sn, DAM_MAGIC, TRUE);     

    return;
} 

void spell_shocking_grasp(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
    
    dam = dice( level, 8 );
/*
    if ( saves_spell( level, victim, DAM_LIGHTNING ) )     
        dam /= 2;
    damage( ch, victim, dam, sn, DAM_LIGHTNING, TRUE);     
*/
    if ( saves_spell( level, victim, DAM_MAGIC ) )     
        dam /= 2;
    damage( ch, victim, dam, sn, DAM_MAGIC, TRUE);     
    return;
}

void spell_colour_spray( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
    
    dam = dice( level, 12 );

    if ( saves_spell( level, victim, DAM_LIGHT2 ) )     
        dam /= 2;
    else 
	spell_blindness(skill_lookup("blindness"),
	    level/2,ch,(void *) victim,TARGET_CHAR);

/*    damage( ch, victim, dam, sn, DAM_LIGHT2, TRUE);     */
    damage( ch, victim, dam, sn, DAM_MAGIC, TRUE);     
    return;
}

void spell_fireball( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = dice( level, 16 );

/*
    if ( saves_spell( level, victim, DAM_FIRE ) )
	dam /= 2;
    damage( ch, victim, dam, sn, DAM_FIRE, TRUE);
*/
    if ( saves_spell( level, victim, DAM_MAGIC ) )
	dam /= 2;
    damage( ch, victim, dam, sn, DAM_MAGIC, TRUE);

    return;
}


void spell_acid_blast( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = dice( level, 20 );
/*
    if ( saves_spell( level, victim, DAM_ACID ) )
	dam /= 2;
    damage( ch, victim, dam, sn, DAM_ACID, TRUE);
*/
    if ( saves_spell( level, victim, DAM_MAGIC ) )
	dam /= 2;
    damage( ch, victim, dam, sn, DAM_MAGIC, TRUE);

    return;
}

void spell_spellfire( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = dice( level, 23 );

    if ( saves_spell( level, victim, DAM_MAGIC ) )
	dam /= 2;
    else 
     victim->mana -= (ch->level);


    damage( ch, victim, dam, sn, DAM_MAGIC, TRUE);
    return;
}

void spell_orb_chaos( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    int dam;

    dam = dice( level, 28 );

/*
    if ( saves_spell( level, victim, DAM_NEGATIVE ) )
	dam /= 2;
*/
    if ( saves_spell( level, victim, DAM_MAGIC ) )
	dam /= 2;

/*    if ( !saves_spell( level, victim,DAM_NEGATIVE ) ) */

    if ( !saves_spell( level, victim,DAM_MAGIC ) )
    {

    if ( !is_affected( victim, sn ) )
      {
	act("\n\r{W$n is struck by {RPure CHAOS{W!{x\n\r",victim,NULL,NULL,TO_ROOM);

	victim->move -= (ch->level*1.5);

  af.where     = TO_AFFECTS;
  af.type      = sn;
  af.level     = level;
  af.duration  = level/10;
  af.location  = APPLY_HITROLL;
  af.modifier  = 0 - level/10;
  af.bitvector = AFF_BLIND;
  affect_to_char( victim, &af );

  af.location  = APPLY_SAVES;
  af.modifier  = 0 - level/2;
  af.bitvector = AFF_CURSE;
  affect_to_char( victim, &af );
   }
     }
    else
    {
	dam /= 2;
    }

/*    damage( ch, victim, dam, sn, DAM_NEGATIVE, TRUE); */
    damage( ch, victim, dam, sn, DAM_MAGIC, TRUE);
    return;
}


void spell_chain_lightning(int sn,int level,CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    CHAR_DATA *tmp_vict,*last_vict,*next_vict;
    bool found;
    int dam;

    /* first strike */

    act("A lightning bolt leaps from $n's hand and arcs to $N.",
        ch,NULL,victim,TO_ROOM);
    act("A lightning bolt leaps from your hand and arcs to $N.",
	ch,NULL,victim,TO_CHAR);
    act("A lightning bolt leaps from $n's hand and hits you!",
	ch,NULL,victim,TO_VICT);  

    dam = dice(level,10);

    if ( is_same_group( victim, ch ) )
      {
       send_to_char("\n\r{RYou can NOT cast this on members of your OWN GROUP!{x\n\r", ch);
       return;
      }

    if (saves_spell(level,victim,DAM_MAGIC))
 	dam /= 3;
    damage(ch,victim,dam,sn,DAM_MAGIC,TRUE);


    last_vict = victim;
    level -= 10;   /* decrement damage */


  /* new targets */
    while (level > 0)
    {
	found = FALSE;
	for (tmp_vict = ch->in_room->people; 
	     tmp_vict != NULL; 
	     tmp_vict = next_vict) 
	{

	  next_vict = tmp_vict->next_in_room;
	  if (!is_safe_spell(ch,tmp_vict,TRUE) && tmp_vict != last_vict)
	  {
	    found = TRUE;
	    last_vict = tmp_vict;
	    dam = dice(level,6);

        if (IS_SET(tmp_vict->in_room->room_flags,ROOM_SAFE)
        || (IS_SET(ch->in_room->room_flags,ROOM_SAFE)))
          {
           send_to_char("\n\r{RThis Room is a Safe Room!{x\n\r", ch);
           found = FALSE;
          }
        else
         {
        if ( is_same_group( tmp_vict, ch ) )
          {
	    act("The bolt arcs around $n!",tmp_vict,NULL,NULL,TO_ROOM);

	    act("\n\r{GThe Bolt arcs around your body, missing you completely!{x\n\r",tmp_vict,NULL,NULL,TO_CHAR);
          }
         else
          {
	    act("The bolt arcs to $n!",tmp_vict,NULL,NULL,TO_ROOM);
	    act("The bolt hits you!",tmp_vict,NULL,NULL,TO_CHAR);

	    if (saves_spell(level,tmp_vict,DAM_MAGIC))
		dam /= 3;
	    damage(ch,tmp_vict,dam,sn,DAM_MAGIC,TRUE);

	    level -= 10;  /* decrement damage */
          }
         }

/*            found = FALSE; */
	  }
	}   /* end target searching loop */


    if (!found) /* no target found, hit the caster */
      {

      if (ch == NULL)
        return;

      last_vict = ch;

      if (last_vict == ch) /* no double hits */
        {
	    act("The bolt seems to have fizzled out.",ch,NULL,NULL,TO_ROOM);
	    act("The bolt grounds out through your body.",
		ch,NULL,NULL,TO_CHAR);
        return;
        }

      if (ch == NULL) 
        return;
	}
    }
} 

void spell_wyldstryke( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    int dam;

    dam = dice( level, 40 );

    if ( saves_spell( level, victim, DAM_MAGIC ) )
	dam /= 2;

    if ( !saves_spell( level, victim,DAM_MAGIC ) )
    {
      if ( !is_affected( victim, sn ) )
        {
	act("\n\r{W$n is struck by Random Magical Energies{W!{x\n\r",victim,NULL,NULL,TO_ROOM);

	victim->mana -= (ch->level*2);
	victim->move -= (ch->level*1.5);

	af.where     = TO_AFFECTS;
	af.type      = sn;
        af.level     = level;
	af.duration  = level/5;
	af.location  = APPLY_STR;
	af.modifier  = 0 - (level/20);
	af.bitvector = 0;
	affect_to_char( victim, &af );

    	af.location  = APPLY_DAMROLL;
    	af.modifier  = 0 - (level / 20);
    	af.bitvector = 0;
    	affect_to_char( victim, &af );

    	af.location  = APPLY_HITROLL;
    	af.modifier  = 0 - (level / 20);
    	af.bitvector = 0;
    	affect_to_char( victim, &af );

  	af.location  = APPLY_INT;
  	af.modifier  = 0 - (level/20);
  	af.bitvector = 0;
  	affect_to_char( victim, &af );


	af.location  = APPLY_WIS;
  	af.modifier  = 0 - (level/20);
  	af.bitvector = 0;
  	affect_to_char( victim, &af );
      }
    }
    else
    {
	dam /= 2;
    }

    damage( ch, victim, dam, sn, DAM_MAGIC, TRUE);
    return;
} 

void spell_acid_storm(int sn,int level,CHAR_DATA *ch, void *vo,int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  CHAR_DATA *tmp_vict,*last_vict,*next_vict;
  bool found;
  int dam;

  /* first strike */

  act("\n\r{G$n summons an Acid Storm to the room!{x\n\r", ch,NULL,victim,TO_ROOM);
  act("\n\r{GYou summon an Acid Storm to the room!{x\n\r", ch,NULL,victim,TO_CHAR);
  act("\n\r{G$n's Acid Storm engulfs you!{x\n\r",          ch,NULL,victim,TO_VICT);  

  dam = dice(level,12);

  if ( is_same_group( victim, ch ) )
    {
     send_to_char("\n\r{RYou can NOT cast this on members of your OWN GROUP!{x\n\r", ch);
     return;
    }

  if (saves_spell(level,victim,DAM_ACID))
    dam /= 2;

  damage(ch,victim,dam,sn,DAM_ACID,TRUE);
  last_vict = victim;
  level -= 5;   /* decrement damage */

  /* new targets */
  while (level > 0)
    {
    found = FALSE;
    for (tmp_vict = ch->in_room->people; 
         tmp_vict != NULL; 
         tmp_vict = next_vict) 
      {
      next_vict = tmp_vict->next_in_room;

      if (!is_safe_spell(ch,tmp_vict,TRUE) && tmp_vict != last_vict)
        {
        found = TRUE;
        last_vict = tmp_vict;
        dam = dice(level,8);


        if (IS_SET(tmp_vict->in_room->room_flags,ROOM_SAFE)
        || (IS_SET(ch->in_room->room_flags,ROOM_SAFE)))
          {
           send_to_char("\n\r{RThis Room is a Safe Room!{x\n\r", ch);
           found = FALSE;  
          }
        else
          {
        if ( is_same_group( tmp_vict, ch ))
          {
           act("\n\r{GThe Acid Storm washes over $n harmlessly!{x\n\r", tmp_vict,NULL,NULL,TO_ROOM);

           act("\n\r{G$n's Acid Storm slides harmlessly off your body!{x\n\r",ch,NULL,tmp_vict,TO_VICT);  
          }
         else
          {
           act("\n\r{G$n's Acid Storm engulfs you!{x\n\r",ch,NULL,tmp_vict,TO_VICT);  

           if (saves_spell(level,tmp_vict,DAM_ACID))
            dam /= 2;
           damage(ch,tmp_vict,dam,sn,DAM_ACID,TRUE);
           level -= 5;  /* decrement damage */
          }
         }
/*        found = FALSE;  */
        }
      }   /* end target searching loop */


    if (!found) /* no target found, hit the caster */
      {

      if (ch == NULL)
        return;

      last_vict = ch;

      if (last_vict == ch) /* no double hits */
        {
        act("{GThe Acid Storm dries up!{x\n\r",ch,NULL,NULL,TO_ROOM);
        act("{GYour Acid Storm has dried up!{x\n\r",ch,NULL,NULL,TO_CHAR);
        return;
        }

      if (ch == NULL) 
        return;
	}
    }
} 

void spell_wail_banshee( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    CHAR_DATA *vch, *vch_next;
    int dam,hp_dam,dice_dam;
    int hpch;

    act("\n\r{M$n Wails like a Banshee!{x\n\r",ch,NULL,victim,TO_NOTVICT);
    act("\n\r{M$n's Banshees Wail is deafening you!{x\n\r",ch,NULL,victim,TO_VICT);
    act("\n\r{MYou Wail like a Banshee!{x\n\r",ch,NULL,NULL,TO_CHAR);

    hpch = UMAX( 5,ch->hit);
    hp_dam = number_range(hpch/6+1,3);
    dice_dam = dice(level,12);

    dam = UMAX(hp_dam + dice_dam /15, dice_dam + hp_dam / 15);
    sonic_effect(victim->in_room,level,dam/2,TARGET_ROOM);

    for (vch = victim->in_room->people; vch != NULL; vch = vch_next)
    {
	vch_next = vch->next_in_room;

	if (is_safe_spell(ch,vch,TRUE) 
	||  (IS_NPC(vch) && IS_NPC(ch) 
	&&   (ch->fighting != vch || vch->fighting != ch)))
	    continue;
/*
	if (vch == victim) 
	{
            if (saves_spell(level,vch,DAM_SOUND))
	    {
                sonic_effect(vch,level/4,dam/4,TARGET_CHAR);
                damage(ch,vch,dam/4,sn,DAM_SOUND,TRUE);
	    }
	    else
	    {
                sonic_effect(vch,level,dam,TARGET_CHAR);
                damage(ch,vch,dam,sn,DAM_SOUND,TRUE);
	    }
	}
	else
	{
            if (saves_spell(level - 2,vch,DAM_SOUND))
	    {
                sonic_effect(vch,level/5,dam/5,TARGET_CHAR);
                damage(ch,vch,dam/5,sn,DAM_SOUND,TRUE);
	    }
	    else
	    {
                sonic_effect(vch,level/4,dam/4,TARGET_CHAR);
                damage(ch,vch,dam/4,sn,DAM_SOUND,TRUE);
	    }
	}
*/

	if (vch == victim) 
	{
            if (saves_spell(level,vch,DAM_MAGIC))
	    {
                sonic_effect(vch,level/4,dam/4,TARGET_CHAR);
                damage(ch,vch,dam/4,sn,DAM_MAGIC,TRUE);
	    }
	    else
	    {
                sonic_effect(vch,level,dam,TARGET_CHAR);
                damage(ch,vch,dam,sn,DAM_MAGIC,TRUE);
	    }
	}
	else
	{
            if (saves_spell(level - 2,vch,DAM_MAGIC))
	    {
                sonic_effect(vch,level/5,dam/5,TARGET_CHAR);
                damage(ch,vch,dam/5,sn,DAM_MAGIC,TRUE);
	    }
	    else
	    {
                sonic_effect(vch,level/4,dam/4,TARGET_CHAR);
                damage(ch,vch,dam/4,sn,DAM_MAGIC,TRUE);
	    }
	}
    }
 return;
} 


void spell_torment_cold( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    int dam;

    dam = dice( level, 75 );

    if ( saves_spell( level, victim, DAM_COLD ) )
	dam /= 2;

    if ( !saves_spell( level, victim,DAM_COLD ) )
    {
      if ( !is_affected( victim, sn ) )
        {   
	act("\n\r{W$n is engulfed in a bone chilling cold!{x\n\r",victim,NULL,NULL,TO_ROOM);

	spell_blindness(skill_lookup("blindness"),level,ch,(void *) victim,TARGET_CHAR);

	af.where     = TO_AFFECTS;
	af.type      = sn;
        af.level     = level;
	af.duration  = level/5;
	af.location  = APPLY_STR;
	af.modifier  = 0 - (level/5);
	af.bitvector = 0;
	affect_to_char( victim, &af );

    	af.location  = APPLY_DAMROLL;
    	af.modifier  = 0 - (level / 2);
    	af.bitvector = 0;
    	affect_to_char( victim, &af );

  	af.location  = APPLY_DEX;
  	af.modifier  = 0 - (level/5);
  	af.bitvector = 0;
  	affect_to_char( victim, &af );

    	af.location  = APPLY_HITROLL;
    	af.modifier  = 0 - (level / 2);
    	af.bitvector = 0;
    	affect_to_char( victim, &af );

  	af.location  = APPLY_SAVES;
  	af.modifier  = 0 - level/2;
  	af.bitvector = AFF_CURSE;
  	affect_to_char( victim, &af );
      }
    }
    else
    {
	dam /= 2;
    }

    damage( ch, victim, dam, sn, DAM_COLD, TRUE); 
/*    damage( ch, victim, dam, sn, DAM_MAGIC, TRUE); */
    return;
} 



void spell_sonic_scream( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    CHAR_DATA *vch, *vch_next;
    int dam,hp_dam,dice_dam;
    int hpch;

if (ch->in_room != get_room_index(ROOM_VNUM_DRAGONPIT_RETURN))
  {
    act("\n\r{G$n releases a teeth rettling scream.{x\n\r",ch,NULL,victim,TO_NOTVICT);
    act("\n\r{G$n blasts you with concentrated sound!{x\n\r",ch,NULL,victim,TO_VICT);
    act("\n\r{GYou release a blast of ultra sound at your target!{x\n\r",ch,NULL,NULL,TO_CHAR);
  }

    hpch = UMAX( 7,ch->hit);
    hp_dam = number_range(hpch/4+1,3);
    dice_dam = dice(level,30);

    dam = UMAX(hp_dam + dice_dam /10, dice_dam + hp_dam / 10);
    sonic_effect(victim->in_room,level,dam/2,TARGET_ROOM);

    for (vch = victim->in_room->people; vch != NULL; vch = vch_next)
    {
	vch_next = vch->next_in_room;

	if (is_safe_spell(ch,vch,TRUE)  || is_same_group(ch, vch)
	||  (IS_NPC(vch) && IS_NPC(ch) 
	&&   (ch->fighting != vch || vch->fighting != ch)))
	    continue;

/*
	if (vch == victim) 
	{
            if (saves_spell(level,vch,DAM_SOUND))
	    {
                sonic_effect(vch,level/2,dam/2,TARGET_CHAR);
                damage(ch,vch,dam/2,sn,DAM_SOUND,TRUE);
	    }
	    else
	    {
                sonic_effect(vch,level,dam,TARGET_CHAR);
                damage(ch,vch,dam,sn,DAM_SOUND,TRUE);
	    }
	}
	else
	{
            if (saves_spell(level - 10,vch,DAM_SOUND))
	    {
                sonic_effect(vch,level/3,dam/3,TARGET_CHAR);
                damage(ch,vch,dam/3,sn,DAM_SOUND,TRUE);
	    }
	    else
	    {
                sonic_effect(vch,level/2,dam/2,TARGET_CHAR);
                damage(ch,vch,dam/2,sn,DAM_SOUND,TRUE);
	    }
	}
*/

	if (vch == victim) 
	{
            if (saves_spell(level,vch,DAM_MAGIC))
	    {
                sonic_effect(vch,level/2,dam/2,TARGET_CHAR);
                damage(ch,vch,dam/2,sn,DAM_MAGIC,TRUE);
	    }
	    else
	    {
                sonic_effect(vch,level,dam,TARGET_CHAR);
                damage(ch,vch,dam,sn,DAM_MAGIC,TRUE);
	    }
	}
	else
	{
            if (saves_spell(level - 10,vch,DAM_MAGIC))
	    {
                sonic_effect(vch,level/3,dam/3,TARGET_CHAR);
                damage(ch,vch,dam/3,sn,DAM_MAGIC,TRUE);
	    }
	    else
	    {
                sonic_effect(vch,level/2,dam/2,TARGET_CHAR);
                damage(ch,vch,dam/2,sn,DAM_MAGIC,TRUE);
	    }
	}
    }
 return;
}



/* ENCHANTER spells */
void spell_fortify_object(int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    AFFECT_DATA af;
 
    if (IS_OBJ_STAT(obj,ITEM_FORTIFIED))
    {
        act("{G$p {gis already {GIMPERVIOUS{g to harm.{x",ch,obj,NULL,TO_CHAR);
        return;
    }
 
    if (ch->level <= 90)
      {
       af.where     = TO_OBJECT;
       af.type      = sn;
       af.level     = ch->level * 2;
       af.duration  = number_fuzzy(ch->level * 2);
       af.location  = APPLY_NONE;
       af.modifier  = 0;
       af.bitvector = ITEM_FORTIFIED;

      affect_to_obj(obj,&af);

      }
    else 
      {
       SET_BIT(obj->extra_flags,ITEM_FORTIFIED);
      }

 
    act("{CYou have made {c$p {CIMPERVIOUS to harm.{x",ch,obj,NULL,TO_CHAR);
    act("\n\r{W$p {Chums briefly but looks unchanged.{x",ch,obj,NULL,TO_ROOM);
}

void spell_fireproof(int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    AFFECT_DATA af;

    if (IS_OBJ_STAT(obj,ITEM_FIREPROOF))
    {
        act("{R$p {ris already WARDED against fire.{x",ch,obj,NULL,TO_CHAR);
        return;
    }
    
    af.where     = TO_OBJECT;
    af.type      = sn;
    af.level     = level;
    af.duration  = number_fuzzy(level / 4);
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = ITEM_FIREPROOF;

    affect_to_obj(obj,&af);
    
    act("{rYou WARD {R$p {ragainst fire.{x",ch,obj,NULL,TO_CHAR);
    act("{R$p {rglows with a {RRED {raura briefly.{x",ch,obj,NULL,TO_ROOM);
}
