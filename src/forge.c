#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "magic.h"

CHAR_DATA *  find_forger	args( ( CHAR_DATA * ch ) );

CHAR_DATA * find_forger ( CHAR_DATA *ch )
{
    CHAR_DATA * forger;

    for ( forger = ch->in_room->people; forger != NULL; forger = forger->next_in_room ) {
	if (!IS_NPC(forger))
	    continue;

        if ( IS_NPC(forger) && IS_SET(forger->act, ACT_FORGER) )
	    return forger;
    }

    if ( forger == NULL ) {
	send_to_char("\n\r{WOnly a Master WeaponSmith can do a Forge.{x\n\r", ch);
	return NULL;
    }

    return NULL;
}


void do_forge( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA  *	obj;
    CHAR_DATA * forger;
    char	weapon[MAX_INPUT_LENGTH];
    char *	words;
    int		cost;

    forger = find_forger(ch);
				    
    if (!forger)
	return;

    if (argument[0] == '\0') 
      {
	send_to_char("\n\r{WYou have to tell me the weapon and type of forging you want.{x\n\r",ch);
	send_to_char("\n\r{GMy possible forges are{w:{x\n\r",ch);
	send_to_char("\n\r  {cTYPE      {w: {CEFFECT                   {YCOST{x\n\r",ch);
	send_to_char("  {c-----------------------------------------------{x\n\r",ch);
	send_to_char("  {Rflame     {w: {Cflaming weapon           {Y80000 gold{x\n\r",ch);
	send_to_char("  {ddrain     {w: {Cvampiric weapon         {Y100000 gold{x\n\r",ch);
	send_to_char("  {Bshocking  {w: {Celectric weapon          {Y65000 gold{x\n\r",ch);
	send_to_char("  {Cfrost     {w: {Cfrost weapon             {Y70000 gold{x\n\r",ch);
	send_to_char("  {Msharp     {w: {Csharp weapon             {Y55000 gold{x\n\r",ch);
	send_to_char("  {mvorpal    {w: {Cvorpal weapon            {Y65000 gold{x\n\r",ch);
	send_to_char("  {Gpoison    {w: {Cpoison weapon            {Y45000 gold{x\n\r",ch);
	send_to_char("  {c-----------------------------------------------{x\n\r",ch);
	send_to_char("\n\r{GType '{WFORGE {c<WEAPON NAME{c> <{WTYPE{c>{G' to forge the weapon.{x\n\r",ch);
	return;
    }
    
    argument = one_argument ( argument, weapon);

    if ( ( obj = get_obj_carry( ch, weapon, ch ) ) == NULL ) {
        act ("\n\r{Y$N says 'You're not carrying that.'\n\r{x", ch, NULL, forger,
TO_CHAR);
        return;
    }
    
    if(obj->item_type != ITEM_WEAPON) {
        act ("\n\r{Y$N says 'This is not a weapon!.'{x\n\r", ch, NULL, forger,
TO_CHAR);
        return;
    }
  
    if ( argument[0] == '\0' ) {
        act ("{Y\n\r$N says 'Pardon? type 'forge' to see the list of modifications.'{x\n\r", ch, NULL,forger, TO_CHAR);
	return;
    }    	  
   

    if (!str_prefix(argument,"flame")) 
    {
	if (IS_WEAPON_STAT( obj, WEAPON_FLAMING) ) {
	    act("\n\r{Y$N says '$p is already flaming.'{x\n\r", ch, obj, forger,
TO_CHAR);
	    return;
	}

	words = "yrawz braoculo";
	cost  = 80000;

	if ( cost > ch->gold ) {
	    act ("\n\r{Y$N says 'You do not have enough gold for my services'\n\r{x",ch,NULL, forger,TO_CHAR);
	    return;
	}

    	act("\n\r{W$n utters the words '{R$T{W'.\n\r{x", forger, NULL, words,TO_ROOM);
    	act("\n\r{W$n utters the words '{R$T{W'.\n\r{x", forger, NULL, words,TO_CHAR);

	spell_pflame_blade( skill_lookup("pflame blade"), 50, forger, obj, 0 );

	if ( IS_WEAPON_STAT ( obj, WEAPON_FLAMING) ) 
          {
	    act ("\n\r{W$N gives $p to $n.{x\n\r", ch, obj, forger, TO_ROOM);
	    act ("\n\r{Y$N says 'Take care with $p, now it is a lot more powerful.'\n\r{x", ch, obj, forger,TO_CHAR);
	    ch->gold 	    -= cost;
	  }
	 else
	    act ("\n\r{Y$N says 'I'm sorry I can't help you.'{x\n\r", ch, NULL,forger,TO_CHAR); 
	return;
    }
    
    if (!str_prefix(argument,"drain")) 
    {
	if (IS_WEAPON_STAT( obj, WEAPON_VAMPIRIC) ) {
	    act("\n\r{Y$N says '$p is already vampiric.'{x\n\r", ch, obj, forger,TO_CHAR);
	    return;
	}

	words = "egruui braoculo";
	cost  = 100000;

	if ( cost > ch->gold ) {
	    act ("\n\r{Y$N says 'You do not have enough gold for my services'{x\n\r",ch,NULL, forger, TO_CHAR);
	    return;
	}
    
    	act("\n\r{W$n utters the words '{d$T{W'.{x\n\r", forger, NULL, words,TO_ROOM);
    	act("\n\r{W$n utters the words '{d$T{W'.{x\n\r", forger, NULL,words,TO_CHAR);

	spell_pdrain_blade( skill_lookup("pdrain blade"), 50, forger, obj, 0 );

	if ( IS_WEAPON_STAT ( obj, WEAPON_VAMPIRIC) ) {
	    act ("\n\r{W$N gives $p to $n.\n\r{x", ch, obj, forger, TO_ROOM);
	    act ("\n\r{Y$N says 'Take care with $p, now it is a lot more powerful.'\n\r{x", ch, obj, forger,TO_CHAR);
	    ch->gold        -= cost;
	    forger->gold  += cost;
	}
	else
	    act ("\n\r{Y$N says 'I'm sorry I can't help you.'\n\r{x", ch, NULL,forger,TO_CHAR); 
	return;
    }

    if (!str_prefix(argument,"shocking")) {
	if (IS_WEAPON_STAT( obj, WEAPON_SHOCKING) ) 
        {
	 act("\n\r{Y$N says '$p is already electrical.'{x\n\r", ch, obj, forger,TO_CHAR);
	    return;
	}

	words = "gpaqtuio braoculo";
	cost  = 65000;

	if ( cost > ch->gold ) {
	    act ("\n\r{Y$N says 'You do not have enough gold for my services'\n\r{x",
ch, NULL, forger,TO_CHAR);
	    return;
	}

    	act("\n\r{W$n utters the words '{B$T{W'.{x\n\r", forger, NULL,words,TO_ROOM);
    	act("\n\r{W$n utters the words '{B$T{W'.{x\n\r", forger, NULL,words,TO_CHAR);

	spell_pshocking_blade( skill_lookup("pshocking blade"), 50, forger, obj, 0 );

	if ( IS_WEAPON_STAT ( obj, WEAPON_SHOCKING) ) {
	    act ("\n\r{W$N gives $p to $n.{x\n\r", ch, obj, forger, TO_ROOM);
	    act ("\n\r{Y$N says 'Take care with $p, now it is a lot more powerful.'\n\r{x", ch, obj, forger,TO_CHAR);
	    ch->gold        -= cost;
	    forger->gold  += cost;
	}
	else
	    act ("\n\r{Y$N says 'I'm sorry I can't help you.'\n\r{x", ch, NULL,forger, TO_CHAR); 
	return;
    }

    if (!str_prefix(argument,"frost")) {
	if (IS_WEAPON_STAT( obj, WEAPON_FROST) ) {
	    act("\n\r{Y$N says '$p is already frost.'{x\n\r", ch, obj, forger,TO_CHAR);
	    return;
	}

	words = "yfagh braoculo";
	cost  = 70000;

	if ( cost > ch->gold ) {
	    act ("\n\r{Y$N says 'You do not have enough gold for my services'{x\n\r",ch,NULL, forger,TO_CHAR);
	    return;
	}

    	act("\n\r{W$n utters the words '{C$T{W'.{x\n\r", forger, NULL, words,TO_ROOM);
    	act("\n\r{W$n utters the words '{C$T{W'.{x\n\r", forger, NULL,words,TO_CHAR);

	spell_pfrost_blade( skill_lookup("pfrost blade"), 50, forger, obj, 0 );
	
	if ( IS_WEAPON_STAT ( obj, WEAPON_FROST) ) {
	    act ("\n\r{W$N gives $p to $n.{x\n\r", ch, obj, forger, TO_ROOM);
	    act ("\n\r{Y$N says 'Take care with $p, now it is a lot more powerful.'\n\r{x", ch, obj, forger,TO_CHAR);
	    ch->gold 	    -= cost;
	    forger->gold  += cost;
	}
	else
	    act ("\n\r{Y$N says 'I'm sorry I can't help you.'{x\n\r", ch, NULL,forger, TO_CHAR); 
	return;
    }

    if (!str_prefix(argument,"sharp")) {
	if (IS_WEAPON_STAT( obj, WEAPON_SHARP) ) {
	    act("\n\r{Y$N says '$p is already sharp.'\n\r{x", ch, obj, forger,TO_CHAR);
	    return;
	}

	words = "gpabras braoculo";
	cost  = 55000;

	if ( cost > ch->gold ) {
	    act ("\n\r{Y$N says 'You do not have enough gold for my services'{x\n\r",ch,NULL, forger,TO_CHAR);
	    return;
	}

    	act("\n\r{W$n utters the words '{M$T{W'.\n\r{x", forger, NULL, words,TO_ROOM);
    	act("\n\r{W$n utters the words '{M$T{W'.\n\r{x", forger, NULL,words,TO_CHAR);

	spell_psharp_blade( skill_lookup("psharp blade"), 50, forger, obj, 0 );
	
	if ( IS_WEAPON_STAT ( obj, WEAPON_SHARP) ) {
	    act ("\n\r{W$N gives $p to $n.{x\n\r", ch, obj, forger, TO_ROOM);
	    act ("\n\r{Y$N says 'Take care with $p, now it is a lot more powerful.'\n\r{x", ch, obj, forger,TO_CHAR);
	    ch->gold 	    -= cost;
	    forger->gold  += cost;
	}
	else
	    act ("\n\r{Y$N says 'I'm sorry I can't help you.'\n\r{x", ch, NULL,forger, TO_CHAR); 
	return;
    }

    if (!str_prefix(argument,"vorpal")) {
	if (IS_WEAPON_STAT( obj, WEAPON_VORPAL) ) {
	    act("\n\r{Y$N says '$p is already vorpal.'\n\r{x", ch, obj, forger,TO_CHAR);
	    return;
	}

	words = "zafsar braoculo";
	cost  = 65000;

	if ( cost > ch->gold ) {
	    act ("\n\r{Y$N says 'You do not have enough gold for my services'{x\n\r",ch, NULL, forger,TO_CHAR);
	    return;
	}

    	act("\n\r{W$n utters the words '${mT{W'.\n\r{x", forger, NULL, words,TO_ROOM);
    	act("\n\r{W$n utters the words '${mT{W'.\n\r{x", forger, NULL,words,TO_CHAR);

	spell_pvorpal_blade( skill_lookup("pvorpal blade"), 50, forger, obj, 0 );
	
	if ( IS_WEAPON_STAT ( obj, WEAPON_VORPAL) ) {
	    act ("\n\r{W$N gives $p to $n.\n\r{x", ch, obj, forger, TO_ROOM);
	    act ("\n\r{Y$N says 'Take care with $p, now it is a lot more powerful.'\n\r{x", ch, obj, forger,TO_CHAR);

	    ch->gold 	    -= cost;
	    forger->gold  += cost;
	}
	else
	    act ("\n\r{Y$N says 'I'm sorry I can't help you.'\n\r{x", ch, NULL,forger, TO_CHAR); 
	return;
    } 

    if (!str_prefix(argument,"poison")) {
	if (IS_WEAPON_STAT( obj, WEAPON_POISON) ) {
	    act("\n\r{Y$N says '$p is already poisoned.'\n\r{x", ch, obj, forger,TO_CHAR);
	    return;
	}

	words = "entorque braoculo";
	cost  = 45000;

	if ( cost > ch->gold ) {
	    act ("\n\r{Y$N says 'You do not have enough gold for my services'{x\n\r",ch, NULL, forger,TO_CHAR);
	    return;
	}

    	act("\n\r{W$n utters the words '${GT{W'.\n\r{x", forger, NULL,words,TO_ROOM);
    	act("\n\r{W$n utters the words '${GT{W'.\n\r{x", forger, NULL,words,TO_CHAR);

	spell_ppoison_blade( skill_lookup("ppoison blade"), 50, forger, obj, 0 );
	
	if ( IS_WEAPON_STAT ( obj, WEAPON_POISON) ) {
	    act ("\n\r{W$N gives $p to $n.\n\r{x", ch, obj, forger, TO_ROOM);
	    act ("\n\r{Y$N says 'Take care with $p, now it is a lot more powerful.'\n\r{x", ch, obj, forger,TO_CHAR);

	    ch->gold 	    -= cost;
	    forger->gold  += cost;
	}
	else
	    act ("\n\r{Y$N says 'I'm sorry I can't help you.'\n\r{x", ch, NULL,forger, TO_CHAR); 
	return;
    } 


    act ("\n\r{Y$N says 'Pardon? Type 'forge' to see the list of modifications.'\n\r{x",ch, NULL, forger,TO_CHAR);
    return;								 
}


void spell_pdrain_blade( int sn, int level, CHAR_DATA *ch, void *vo, int target )
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

    if(IS_OBJ_STAT(obj,ITEM_BLESS) && !IS_IMP(ch)) {
	send_to_char("This weapon is too blessed.\n\r", ch);
	return;
    }

    af.where    = TO_WEAPON;
    af.type     = sn;
    af.level    = 0;
    af.duration = -1;
    af.location = 0;
    af.modifier = 0;
    af.bitvector = WEAPON_VAMPIRIC;
    affect_to_obj(obj, &af);

    act("$p carried by $n turns dark and vampiric.", ch, obj, NULL, TO_ROOM);
    act("$p carried by you turns dark and vampiric.", ch, obj, NULL, TO_CHAR);
    return;
}

void spell_pshocking_blade( int sn, int level, CHAR_DATA *ch, void *vo, int target )
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
    af.level    = 0;
    af.duration = -1;
    af.location = 0;
    af.modifier = 0;
    af.bitvector = WEAPON_SHOCKING;
    affect_to_obj(obj, &af);

    act("$p carried by $n sparks with electricity.", ch, obj, NULL, TO_ROOM);
    act("$p carried by you sparks with electricity.", ch, obj, NULL, TO_CHAR);
    return;
}


void spell_pflame_blade( int sn, int level, CHAR_DATA *ch, void *vo, int target )
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
    af.level    = 0;
    af.duration = -1;
    af.location = 0;
    af.modifier = 0;
    af.bitvector = WEAPON_FLAMING;
    affect_to_obj(obj, &af);

    act("$p carried by $n gets a fiery aura.", ch, obj, NULL, TO_ROOM);
    act("$p carried by you gets a fiery aura.", ch, obj, NULL, TO_CHAR);
    return;
}

void spell_pfrost_blade( int sn, int level, CHAR_DATA *ch, void *vo, int target )
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
    af.level    = 0;
    af.duration = -1;
    af.location = 0;
    af.modifier = 0;
    af.bitvector = WEAPON_FROST;
    affect_to_obj(obj, &af);

    act("$p carried by $n grows wickedly cold.", ch, obj, NULL, TO_ROOM);
    act("$p carried by you grows wickedly cold.", ch, obj, NULL, TO_CHAR);
    return;
}

void spell_psharp_blade( int sn, int level, CHAR_DATA *ch, void *vo, int target )
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
    af.level    = 0;
    af.duration = -1;
    af.location = 0;
    af.modifier = 0;
    af.bitvector = WEAPON_SHARP;
    affect_to_obj(obj, &af);

    act("\n\r{W$p carried by $n looks newly honed.{x\n\r", ch, obj, NULL, TO_ROOM);
    act("\n\r{W$p carried by you looks newly honed.{x\n\r", ch, obj, NULL, TO_CHAR);
    return;
}

void spell_pvorpal_blade( int sn, int level, CHAR_DATA *ch, void *vo, int target )
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
    af.level    = 0;
    af.duration = -1;
    af.location = 0;
    af.modifier = 0;
    af.bitvector = WEAPON_VORPAL;
    affect_to_obj(obj, &af);

    act("$p carried by $n gleams with magical strengh.", ch, obj, NULL, TO_ROOM);
    act("$p carried by you gleams with magical strengh.", ch, obj, NULL, TO_CHAR);
    return;
}

void spell_ppoison_blade( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    AFFECT_DATA af;

    if(obj->item_type != ITEM_WEAPON) {
	send_to_char("You can only cast this spell on weapons.\n\r",ch);
	return ;
    }

    if(IS_WEAPON_STAT(obj,WEAPON_VORPAL)) {
	send_to_char("This weapon is already poisoned.\n\r", ch);
	return;
    }

    af.where    = TO_WEAPON;
    af.type     = sn;
    af.level    = 0;
    af.duration = -1;
    af.location = 0;
    af.modifier = 0;
    af.bitvector = WEAPON_POISON;
    affect_to_obj(obj, &af);

    act("$p carried by $n drips with a deadly venom.", ch, obj, NULL, TO_ROOM);
    act("$p carried by you drips with a deadly venom.", ch, obj, NULL, TO_CHAR);
    return;
}



