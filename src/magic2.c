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


extern char *target_name;

void spell_farsight( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    if (IS_AFFECTED(ch,AFF_BLIND))
    {
        send_to_char("Maybe it would help if you could see?\n\r",ch);
        return;
    }
 
    do_function(ch, &do_scan, target_name);
}


void spell_portal( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim;
    OBJ_DATA *portal, *stone;

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
    ||   (!IS_NPC(victim) && victim->level >= LEVEL_HERO) 
    ||   (IS_NPC(victim) && IS_SET(victim->imm_flags,IMM_SUMMON))
    ||   (IS_NPC(victim) && saves_spell( level, victim,DAM_NONE) )
    ||   (is_clan(victim) && !is_same_clan(ch,victim)))
    {
        send_to_char( "You failed.\n\r", ch );
        return;
    }   

 if (!IS_IMP(ch))
    {
    if (IS_IMMORTAL(ch))
      {
       send_to_char("\n\r{rIMMs can use {RGOTO{r so there is no reason to be opening {RPORTAL{r.{x\n\r",ch);
       return;
      }
    }

    stone = get_eq_char(ch,WEAR_HOLD);
    if (!IS_IMMORTAL(ch) 
    &&  (stone == NULL || stone->item_type != ITEM_WARP_STONE))
    {
	send_to_char("You lack the proper component for this spell.\n\r",ch);
	return;
    }

    if (stone != NULL && stone->item_type == ITEM_WARP_STONE)
    {
     	act("You draw upon the power of $p.",ch,stone,NULL,TO_CHAR);
     	//act("It flares brightly and vanishes!",ch,stone,NULL,TO_CHAR);
     	//extract_obj(stone);
    }

    portal = create_object(get_obj_index(OBJ_VNUM_PORTAL),0);
    portal->timer = 2 + level / 25; 
    portal->value[3] = victim->in_room->vnum;

    obj_to_room(portal,ch->in_room);

    act("$p rises up from the ground.",ch,portal,NULL,TO_ROOM);
    act("$p rises up before you.",ch,portal,NULL,TO_CHAR);
}

void spell_nexus( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *victim;
    OBJ_DATA *portal, *stone;
    ROOM_INDEX_DATA *to_room, *from_room;

    from_room = ch->in_room;
 victim=get_char_world(ch, target_name);
        if( victim== NULL)
    {
        send_to_char( "You failed.\n\r", ch );
        return;
    }
    else if(victim == ch
    ||   (to_room = victim->in_room) == NULL
    ||   !can_see_room(ch,to_room) || !can_see_room(ch,from_room)
    ||   IS_SET(to_room->room_flags, ROOM_SAFE)
    ||	 IS_SET(from_room->room_flags,ROOM_SAFE)
    ||   IS_SET(to_room->room_flags, ROOM_PRIVATE)
    ||   IS_SET(to_room->room_flags, ROOM_SOLITARY)
    ||   IS_SET(to_room->room_flags, ROOM_DRAGONPIT)
    ||   IS_SET(ch->in_room->room_flags, ROOM_DRAGONPIT)
    ||   IS_SET(to_room->room_flags, ROOM_NO_RECALL)
    ||   IS_SET(to_room->area->area_flags, AREA_PROTO )
    ||   IS_SET(from_room->room_flags,ROOM_NO_RECALL)
    ||   victim->level >= level + 3
    ||   (!IS_NPC(victim) && victim->level >= LEVEL_HERO)
    ||   (IS_NPC(victim) && IS_SET(victim->imm_flags,IMM_SUMMON))
    ||   (IS_NPC(victim) && saves_spell( level, victim,DAM_NONE) ) 
    ||   (is_clan(victim) && !is_same_clan(ch,victim)))
    {
        send_to_char( "You failed.\n\r", ch );
        return;
    }   
 
    stone = get_eq_char(ch,WEAR_HOLD);
    if (!IS_IMMORTAL(ch)
    &&  (stone == NULL || stone->item_type != ITEM_WARP_STONE))
    {
        send_to_char("You lack the proper component for this spell.\n\r",ch);
        return;
    }
 
 if (!IS_IMP(ch))
    {
    if (IS_IMMORTAL(ch))
      {
       send_to_char("\n\r{rIMMs can use {RGOTO{r so there is no reason to open a {RNEXUS{r point.{x\n\r",ch);
       return;
      }
    }

    if (stone != NULL && stone->item_type == ITEM_WARP_STONE)
    {
        act("You draw upon the power of $p.",ch,stone,NULL,TO_CHAR);
        //act("It flares brightly and vanishes!",ch,stone,NULL,TO_CHAR);
        //extract_obj(stone);
    }

    /* portal one */ 
    portal = create_object(get_obj_index(OBJ_VNUM_PORTAL),0);
    portal->timer = 1 + level / 10;
    portal->value[3] = to_room->vnum;
 
    obj_to_room(portal,from_room);
 
    act("$p rises up from the ground.",ch,portal,NULL,TO_ROOM);
    act("$p rises up before you.",ch,portal,NULL,TO_CHAR);

    /* no second portal if rooms are the same */
    if (to_room == from_room)
	return;

    /* portal two */
    portal = create_object(get_obj_index(OBJ_VNUM_PORTAL),0);
    portal->timer = 1 + level/10;
    portal->value[3] = from_room->vnum;

    obj_to_room(portal,to_room);

    if (to_room->people != NULL)
    {
	act("$p rises up from the ground.",to_room->people,portal,NULL,TO_ROOM);
	act("$p rises up from the ground.",to_room->people,portal,NULL,TO_CHAR);
    }
}


/* SPELL BLANK

void spell_( int sn, int level, CHAR_DATA *ch, void *vo, int target )         
{
    AFFECT_DATA af;
    
    if ( IS_AFFECTED2(ch, AFF_) )         
    {
        send_to_char("\n\r{CYou are already !{x\n\r", ch);             
        return;
    }
    
    af.where     = TO_AFFECTS2;
    af.type      = sn;
    af.level     = level;   
    af.duration  = ;       
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_;         
    affect_to_char(ch, &af);

    af.where     = TO_IMMUNE;
    af.bitvector = IMM_;    
    affect_to_char(ch, &af);
    
    send_to_char("You are surrounded by a !{x\n\r", ch);                      
    act("$n is surrounded by a magical !{x", ch, NULL, NULL, TO_ROOM);              
        
    return;
}




void spell_( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    AFFECT_DATA af;

    if(obj->item_type != ITEM_WEAPON)  
      {
        send_to_char("\n\r{WYou can only cast this spell on weapons.{x\n\r",ch);
        return;
      }
    
 *  
    if(IS_WEAPON_STAT(obj,WEAPON_SHARP))
      {
        send_to_char("\n\r{WThis weapon is already sharp.{x\n\r", ch);
        return;
    }
 *  
    
    af.where    = TO_;      
    af.type     = sn;
    af.level    = ;  
    af.duration = ;  
    af.location = 0;
    af.modifier = 0;
    af.bitvector = ;            
    affect_to_obj(obj, &af);
    
    act("\n\r{W$p carried by $n looks newly honed.{x\n\r", ch, obj, NULL, TO_ROOM); 
    act("\n\r{W$p carried by you looks newly honed.{x\n\r", ch, obj, NULL, TO_CHAR);
    return;
}

*/
