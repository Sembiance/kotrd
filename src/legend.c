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
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include "merc.h"
#include "interp.h"
#include "recycle.h"

#include "Utility.h"
#include "StringUtility.h"
#include "ArrayUtility.h"

bool has_item( CHAR_DATA *ch, sh_int vnum, sh_int item_type, bool fWear );

const struct legend_type		legend_table	[]	=
{ 
    { "Nephilim", 23799, 23799 },
    { "Eidolon", 24099, 24099 },
    { NULL, 0, 0}
};

bool legend_top_top(CHAR_DATA *ch)
{
    AREA_DATA *     pArea;
    char **         ar=0;          
    unsigned long   totalMobsInArea=0, killedInThisArea=0, totalMobsKilled=0;
    unsigned long   totalRoomsInArea=0, exploredInThisArea=0, totalRoomsExplored=0;
    unsigned long   totalObjectsInArea=0, foundInThisArea=0, totalObjectsFound=0;

    if(!ch || !ch->pcdata)
        return FALSE;
        
    // Hack so immortals can become legends
    if(!strcmp(ch->name, "Ezra"))
        return TRUE;

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

    if(((totalMobsKilled*100)/gTotalMobs)>=90 && ((totalRoomsExplored*100)/gTotalRooms)>=90 && ((totalObjectsFound*100)/gTotalObjects)>=90)
        return TRUE;
        
    return FALSE;
}


void do_legend( CHAR_DATA *ch, char *argument )
{
    int                 i, validRoom;
    AFFECT_DATA *       paf;
    AFFECT_DATA *       paf_next;
    OBJ_DATA *          obj;
    OBJ_DATA *          obj_next;
    CHAR_DATA *         vch;
    CHAR_DATA *         pch;
    DESCRIPTOR_DATA *   d;
    char                buf[MAX_STRING_LENGTH];
        
    // Check valid level
    if(ch->level!=100 || ch->pcdata->oldcl==-1)
    {
        send_to_char("\n\r{RYou must be a remort hero in order to become a Legend.{x\n\r", ch);
        return;
    }
    
    // Must be a player
    if(IS_NPC(ch))
    {
        send_to_char("\n\r{RBut your not a player!{x\n\r", ch);
        return;
    }
    
    // Must not be a legend already
    if(ch->pcdata->legend>0)
    {
        send_to_char("\n\r{RYou are already a Legend!{x\n\r", ch);
        return;
    }
    
    // Check enough quest points
    if(ch->questpoints < 4000)
    {
        send_to_char( "\n\r{RYou do not have enough quest points to become a Legend.{x\n\r", ch );
        return;
    }
    
    // 100% in all three top lists
    if(!legend_top_top(ch))
    {
        send_to_char("\n\r{RYou must have 100% in each of the three Top lists in order to become a Legend.{x\n\r", ch);
        return;
    }    
    
    // Check valid room
    for(i=0,validRoom=-1;legend_table[i].name;i++)
    {
        if(ch->in_room && ch->in_room->vnum==legend_table[i].room_vnum)
        {
            validRoom = i;
            break;
        }
    }
    
    if(validRoom==-1)
    {
        send_to_char("\n\r{RYou are not in the correct room to become a Legend.{x\n\r", ch);
        return;
    }
    
    // Check valid obejct
    if(!has_item(ch, legend_table[validRoom].obj_vnum, -1, FALSE))
    {
        send_to_char("\n\r{RYou are not holding the correct item to become a Legend.{x\n\r", ch);
        return;
    }
    
    // All checks passed! They can now become a legend!!
    send_to_char("\n\r{WA MASSIVE beam of light rips through the sky above and down to you...{x\n\r", ch);
    ch->level = 0;
    ch->pcdata->legending = TRUE;
    ch->pcdata->legend = validRoom;

    send_to_char("\n\r{GAll your equipment disintegrates...{x\n\r", ch);
    for(obj=ch->carrying;obj;obj=obj_next)
    {
        obj_next = obj->next_content;
        extract_obj(obj);
    }
    
    send_to_char("\n\r{GYou feel all magic seeping form you...{x\n\r", ch);
    for(paf=ch->affected;paf;paf=paf_next)
    {
        paf_next = paf->next;
        affect_remove(ch, paf);
    }
    
    d = ch->desc;
    
    write_to_buffer(d, "{cChoose a {CWEAPON {cfrom this list.\n\r"
                       "{cThe {CWEAPON {cyou choose is the one your {CCHARACTER {cwill begin the {CGAME {cwith\n\r"
                       "{cAND the corresponding {CWEAPON {cskill will start with a {W40% {CBASE CHANCE{c.{x\n\r\n\r",0);
    buf[0] = '\0';
    
    for(i=0;weapon_table[i].name;i++)
    {
        if(ch->pcdata->learned[*weapon_table[i].gsn]>0)
        {
            strcat(buf, weapon_table[i].name);
            strcat(buf, " ");
        }
    }
    
    strcat(buf, "\n\r\n\r{cYour choice? {w:{W ");
    write_to_buffer(d, buf, 0);    
    
    act_new( "\n\r{W$n is engulfed in a MASSIVE beam of light and in a flash is gone.{x", ch,NULL,NULL,TO_ROOM, POS_RESTING );
    extract_char(ch, FALSE);
    save_char_obj(ch);
    char_from_room(ch);
    crn_players--;
    
  for ( vch = char_list, pch = NULL; vch; pch = vch, vch = vch->next )
    if ( vch == ch )
      {
      if ( pch )
	pch->next = vch->next;
      else
	char_list = vch->next;
      break;
      }
    
    d->connected = CON_PICK_WEAPON;

    return;
}

// This is called after they have picked the weapon they want to keep when becoming a legend
void legend_weapon_picked(CHAR_DATA * ch, int weaponChosen)
{
    int sn;
    
    // Wipe all skills except the weaponChosen
    for(sn=0;sn<MAX_SKILL;sn++)
    {
        if(sn==weaponChosen)
            continue;
        
        ch->pcdata->learned[sn] = 0;
        ch->pcdata->points -= skill_table[sn].rating[ch->class];
    }
    
	for(sn=0;sn<MAX_GROUP;sn++)
	{
		ch->pcdata->group_known[sn]=FALSE;
	}    
    
    return;
}

void legend_logging_in_first_time(CHAR_DATA * ch)
{
    int     i;
    
    ch->questpoints = 0;
    ch->gold = 0;
    ch->silver = 0;
    ch->pcdata->gold_bank = 0;
    ch->pcdata->silver_bank = 0;
    ch->alignment = 0;
    ch->level = 1;
    ch->train = 0;
    ch->practice = 0;
    ch->max_hit = 500;
    ch->max_mana = 250;
    ch->max_move = 350;
    ch->pcdata->condition[COND_THIRST] = -1;
    ch->pcdata->condition[COND_HUNGER] = -1;
    ch->hit = ch->max_hit;
    ch->mana = ch->max_mana;
    ch->move = ch->max_move;
    ch->exp = exp_per_level(ch, ch->pcdata->points);
    ch->pcdata->perm_hit = ch->max_hit;
    ch->pcdata->perm_mana = ch->max_mana;
    ch->pcdata->perm_move = ch->max_move;
	for(i=0;i<MAX_STATS;i++)
	{
		ch->perm_stat[i]=20;
	}
	ch->nextquest = 15;
    SET_BIT( ch->deaf, CHANNEL_VENT );
	set_title(ch, "says 'I have not used the TITLE command yet!'{x");
    char_to_room( ch, get_room_index( ROOM_VNUM_SCHOOL ) );
    send_to_char("\n\r",ch);
    send_to_char("\n\r",ch);

	do_outfit(ch, "");
    do_dbag (ch, "self");
}

bool legend_has_race_chat(CHAR_DATA * legend, CHAR_DATA * target)
{
    if(IS_NPC(target))
        return TRUE;
        
    if(IS_IMMORTAL(target) || IS_IMP(target) || IS_LEGEND(target))
        return TRUE;
        
    if(((target->race == RACE_SUCCUBUS)
    || (target->race == RACE_INCCUBUS) 
    || (target->race == RACE_CAMBION)
    || (IS_CLASS(target, CLASS_ENCHANTER))
    || (target->race == RACE_MYRDDRAAL)) && IS_SET(legend->racechan, RACECHAN_DEMONIC))
        return TRUE;
    
    if(((target->race == RACE_ELF)
    || (IS_CLASS(target, CLASS_ENCHANTER))
    || (target->race == RACE_SIDHE_ELF)) && IS_SET(legend->racechan, RACECHAN_ELVEN))
        return TRUE;
        
    if(((target->race == RACE_DWARF)
    || (IS_CLASS(target, CLASS_ENCHANTER)))&& IS_SET(legend->racechan, RACECHAN_DWARVEN))
        return TRUE;
    
    if(target->race == RACE_DRACONIAN && IS_SET(legend->racechan, RACECHAN_DRACONIAN))
        return TRUE;
    
    if(((target->race == RACE_RAPTOR)
    ||  (target->race == RACE_LIZARDMAN)
    ||  (target->race == RACE_LAMIA)) && IS_SET(legend->racechan, RACECHAN_REPTILIAN))
        return TRUE;
    
    if(((target->race == RACE_DARKELF)
    || (IS_CLASS(target, CLASS_ENCHANTER))) && IS_SET(legend->racechan, RACECHAN_DROW))
        return TRUE;
    
    if(((target->race == RACE_SIDHE_ELF)
    || (IS_CLASS(target, CLASS_ENCHANTER))) && IS_SET(legend->racechan, RACECHAN_HIGH_ELVEN))
        return TRUE;
    
    if(target->race == RACE_MYRDDRAAL && IS_SET(legend->racechan, RACECHAN_UNDEAD))
        return TRUE;

    if(target->race == RACE_KENDER && IS_SET(legend->racechan, RACECHAN_KENDER))
        return TRUE;
    
    if(((target->race == RACE_GOLEM)
    || (IS_CLASS(target, CLASS_ENCHANTER))
    || (IS_CLASS(target, CLASS_MAGE))) && IS_SET(legend->racechan, RACECHAN_MAGESPEAK))
        return TRUE;
    
    if(((target->race == RACE_SIDHE_ELF)
    || (IS_CLASS(target, CLASS_RANGER))) && IS_SET(legend->racechan, RACECHAN_FORESTSIGN))
        return TRUE;
    
    if(((target->race == RACE_MYRDDRAAL)
    || (target->race == RACE_KENDER)
    || (IS_CLASS(target, CLASS_THIEF))) && IS_SET(legend->racechan, RACECHAN_THIEVES_CANT))
        return TRUE;
        
    if(((target->race == RACE_MYRDDRAAL)
    ||  (IS_CLASS(target, CLASS_ASSASSIN))
    ||  (IS_CLASS(target, CLASS_ANTI_PALADIN))) && IS_SET(legend->racechan, RACECHAN_HANDTALK))
        return TRUE;

    if(((IS_CLASS(target, CLASS_WARRIOR))
    || (target->race == RACE_NEPHILIM)) && IS_SET(legend->racechan, RACECHAN_WAR_CHANT))
        return TRUE;

    return FALSE;
}

void legend_garble_text(char * original, char * destination)
{
    int i;
    
    for(i=0;i<strlen(original);i++)
    {
        if(original[i]==' ')
            destination[i] = original[i];
        else
            destination[i] = number_range(97, 122);   
    }
    destination[i] = '\0';
    
    return;
}

bool legend_can_raise_level(CHAR_DATA *ch)
{
    OBJ_DATA *  obj;
    
    if((ch->level+1)%5!=0)
        return TRUE;
    
    for(obj=ch->carrying;obj;obj=obj->next_content)
    {
        if(IS_OBJ_STAT(obj, ITEM_LEGEND) && obj->level==ch->level)
            return TRUE;
    }
    
    return FALSE;
}


