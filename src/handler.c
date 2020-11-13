#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include "merc.h"
#include "interp.h"
#include "magic.h"
#include "recycle.h"
#include "tables.h"
#include "db.h"

#include "Utility.h"
#include "StringUtility.h"
#include "ArrayUtility.h"

bool gInfiniteAffectDuration=FALSE;

// Explorer/Killer Percentages
bool valid_explorer_killer(CHAR_DATA *ch);

// Stones of Wisdom Stuff
void stones_object_leaving(OBJ_DATA * obj);
void    stones_player_quit(CHAR_DATA * ch);

/*
 * Local functions.
 */
void	affect_modify	args( ( CHAR_DATA *ch, AFFECT_DATA *paf, bool fAdd ) );
extern bool isdp;

extern int atoi  args( ( const char *string ) );

bool is_sn( int sn )
   {
    int iCtr;

    for ( iCtr = 0; iCtr < MAX_SKILL; iCtr++ )
       if ( sn == iCtr )
       return TRUE;
       
    return FALSE;
   }
   
bool file_exists(char * dynamicFilePath)
{
    int             dynamicOpen;
    struct stat     dynamicStat;
    
    if(access(dynamicFilePath, 0)!=0)
        return FALSE;

    if(!(dynamicOpen=open(dynamicFilePath, 0)))
       return FALSE;

    fstat(dynamicOpen, &dynamicStat);
    close(dynamicOpen);
    
    return TRUE;
}


/* friend stuff -- for NPC's mostly */
bool is_friend(CHAR_DATA *ch,CHAR_DATA *victim)
{
    if (is_same_group(ch,victim))
	return TRUE;

    
    if (!IS_NPC(ch))
	return FALSE;

    if (!IS_NPC(victim))
    {
	if (IS_SET(ch->off_flags,ASSIST_PLAYERS))
	    return TRUE;
	else
	    return FALSE;
    }

    if (IS_AFFECTED(ch,AFF_CHARM))
	return FALSE;

    if (IS_SET(ch->off_flags,ASSIST_ALL))
	return TRUE;

    if (ch->group && ch->group == victim->group)
	return TRUE;

    if (IS_SET(ch->off_flags,ASSIST_VNUM) 
    &&  ch->pIndexData == victim->pIndexData)
	return TRUE;

    if (IS_SET(ch->off_flags,ASSIST_RACE) && ch->race == victim->race)
	return TRUE;
     
    if (IS_SET(ch->off_flags,ASSIST_ALIGN)
    &&  !IS_SET(ch->act,ACT_NOALIGN) && !IS_SET(victim->act,ACT_NOALIGN)
    &&  ((IS_GOOD(ch) && IS_GOOD(victim))
    ||	 (IS_EVIL(ch) && IS_EVIL(victim))
    ||   (IS_NEUTRAL(ch) && IS_NEUTRAL(victim))))
	return TRUE;

    return FALSE;
}

void do_bounty( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    char buf2  [ MAX_STRING_LENGTH ];
    CHAR_DATA *victim;
    DESCRIPTOR_DATA *d;
    bool found = FALSE;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
	    
    if(ch->in_room != get_room_index(ROOM_VNUM_BOUNTY_OFFICE))
        {
	   send_to_char(
"\n\r\n\r{RYou MUST be in the Bounty Office of Midgaard to place a price{x", ch );
	   send_to_char(
"\n\r{Ron someone's head or to look at the current BOUNTY listings!{x\n\r\n\r", ch );
             return;
        }

    if(!IS_IMP(ch))
        {
         if (ch->level >= 101 && ch->level <= 498)
           {
	    send_to_char("\n\r{ROnly {WMORTALs{R may place Bounties!{x\n\r", ch );
            return;
           }
        }

        if (!str_cmp(arg1, "list"))
          {
                   if (!found)
                  {
           send_to_char( "\n\r{D[{w MOST WANTED{D - {wBOUNTY  {D]{x", ch );
                   }

           for (d = descriptor_list; d != NULL; d = d->next)
              {
                victim = d->character;

                if (victim == NULL
                || IS_NPC(victim)
                || (victim->desc->connected != CON_PLAYING)
                || (victim->pcdata->bounty <= 0))
                  continue;

                if ((victim->desc->connected == CON_PLAYING)
                && (victim->pcdata->bounty >= 0))
                  {
                   sprintf(buf2,"\n\r{D[{W%12s {D- {W%-8d{D]{x",victim->name,victim->pcdata->bounty);
                   send_to_char( buf2, ch );
                   found = TRUE;
                  }
              }

                   if (!found)
                     {
                      send_to_char("\n\r\n\r{RTHERE ARE NO OUTSTANDING WARRANTS{r.  {RTRY BACK LATER{r.{x\n\r\n\r",ch);
                      return;
                     }
                   else
                     {
                      send_to_char("\n\r{x",ch);
                      return;
                     }
     }


	if ( arg1[0] == '\0' || arg2[0] == '\0' )
          {
	   send_to_char("\n\r  {GSyntax{w: {WBOUNTY {c<{WVICTIM{c> <{WAMOUNT{c>{x\n\r", ch );
	   send_to_char("\n\r{cListings{w: {WBOUNTY LIST ALL{x\n\r", ch );
           return;
          }
	
        if ( ( victim = get_char_world( ch, arg1 ) ) == NULL)
	  {
  	   send_to_char( "\n\r{RThey are currently not logged in!{x\n\r", ch );
	   return;
          }
  

    if ((victim->level < 41)
    && (!(victim->pcdata->oldcl > -1)))
        {
	   send_to_char(
"\n\r{RThe Rotten Scoundral must be atleast {W41{Rst LeveL or a {WREMORT {rto have a Bounty placed on their head!{x\n\r",
ch );
             return;
        }

      if (IS_NPC(victim))
      {
	send_to_char( "\n\r{RYou {WCAN NOT {Rput a Bounty on NPCs!{x\n\r", ch );
	return;
      }

    if (!IS_SET(victim->pact, PLR_PKILLER))
        {
	   send_to_char(
"\n\r{ROnly PCs that are {WPKILLERs {Rmay have a Bounty placed on thier head!{x\n\r", ch );
             return;
        }



	if ( is_number( arg2 ) )
        {
	int amount;
	amount   = atoi(arg2);

        if (amount <= 0)
	{
  	   send_to_char( "\n\r{RIsn't your Price for Vengeance worth more than that?{x\n\r", ch);
	   return;
        }

        if (ch->gold < amount)
        {
		send_to_char( "\n\r{RYou don't have that much {YGOLD{R!{x\n\r", ch );
		return;
        }
	ch->gold -= amount;
	victim->pcdata->bounty +=amount;
sprintf( buf, "\n\r{GYou have placed a Bounty of {Y%d GOLD {Gon {W%s{G's Head!{x\n\r{W%s {Cnow has a Total Bounty of {Y%d GOLD{C for his carcass!{x\n\r",
	amount,victim->name,victim->name,victim->pcdata->bounty );
	send_to_char(buf,ch);
	return;
	}
}


void do_visible( CHAR_DATA *ch, char *argument )
{

    affect_strip ( ch, gsn_invisibility                 );
    affect_strip ( ch, gsn_obfuscate			);
    affect_strip ( ch, gsn_sneak                    	);
    affect_strip ( ch, gsn_hide                    	);
    affect_strip ( ch, gsn_mass_invis                   );
    affect_strip ( ch, gsn_chameleon_power              );
    affect_strip ( ch, gsn_shadow_form                  );

    REMOVE_BIT   ( ch->affected_by, AFF_HIDE            );
    REMOVE_BIT   ( ch->affected_by, AFF_INVISIBLE       );
    REMOVE_BIT   ( ch->affected_by, AFF_SNEAK           );
    send_to_char( "\n\r{WYou are completely visible now.{x\n\r", ch );
    return;
}



/* returns number of people on an object */
int count_users(OBJ_DATA *obj)
{
    CHAR_DATA *fch;
    int count = 0;

    if (obj->in_room == NULL)
	return 0;

    for (fch = obj->in_room->people; fch != NULL; fch = fch->next_in_room)
	if (fch->on == obj)
	    count++;

    return count;
}
     

int weapon_lookup (const char *name)
{
    int type;

    for (type = 0; weapon_table[type].name != NULL; type++)
    {
	if (LOWER(name[0]) == LOWER(weapon_table[type].name[0])
	&&  !str_prefix(name,weapon_table[type].name))
	    return type;
    }
    return 0;
}

int weapon_type (const char *name)
{
    int type;
 
    for (type = 0; weapon_table[type].name != NULL; type++)
    {
        if (LOWER(name[0]) == LOWER(weapon_table[type].name[0])
        &&  !str_prefix(name,weapon_table[type].name))
            return weapon_table[type].type;
    }
 
    return WEAPON_NONE;
}

int material_type (const char *name)
{
    int type;
     
    for (type = 0; material_table[type].name != NULL; type++)
    {
        if (LOWER(name[0]) == LOWER(material_table[type].name[0])
        &&  !str_prefix(name,material_table[type].name))
            return material_table[type].type;
    }
      return MATERIAL_NONE;
}


char *item_name(int item_type)
{
    int type;

    for (type = 0; item_table[type].name != NULL; type++)
	if (item_type == item_table[type].type)
	    return item_table[type].name;
    return "NONE";
}

char *material_name(int material_type)
{
    int type;

    for (type = 0; material_table[type].name != NULL; type++)
	if (material_type == material_table[type].type)
	    return material_table[type].name;
    return "SELECT_A_MATERIAL";
}

char *weapon_name( int weapon_type)
{
    int type;
 
    for (type = 0; weapon_table[type].name != NULL; type++)
        if (weapon_type == weapon_table[type].type)
            return weapon_table[type].name;
    return "NONE";
}


int attack_lookup  (const char *name)
{
    int att;

    for ( att = 0; attack_table[att].name != NULL; att++)
    {
	if (LOWER(name[0]) == LOWER(attack_table[att].name[0])
	&&  !str_prefix(name,attack_table[att].name))
	    return att;
    }

    return 0;
}

/* returns a flag for wiznet */
long wiznet_lookup (const char *name)
{
    int flag;

    for (flag = 0; wiznet_table[flag].name != NULL; flag++)
    {
	if (LOWER(name[0]) == LOWER(wiznet_table[flag].name[0])
	&& !str_prefix(name,wiznet_table[flag].name))
	    return flag;
    }

    return -1;
}

/* returns class number */
int class_lookup (const char *name)
{
   int class;
 
   for ( class = 0; class < MAX_CLASS; class++)
   {
        if (LOWER(name[0]) == LOWER(class_table[class].name[0])
        &&  !str_prefix( name,class_table[class].name))
            return class;
   }
 
   return -1;
}

/* for immunity, vulnerabiltiy, and resistant
   the 'globals' (magic and weapons) may be overriden
   three other cases -- wood, silver, and iron -- are checked in fight.c */

int check_immune(CHAR_DATA *ch, int dam_type)
{
    int immune, def;
    int bit;

    immune = -1;
    def = IS_NORMAL;

    if (dam_type == DAM_NONE)
	return immune;


   if(dam_type == 2
   || dam_type == 3
   || dam_type == 23
   || dam_type == 25
   || dam_type == 26)
    {
	if (IS_SET(ch->imm_flags,IMM_WEAPON))
	    def = IS_IMMUNE;
	else if (IS_SET(ch->res_flags,RES_WEAPON))
	    def = IS_RESISTANT;
	else if (IS_SET(ch->vuln_flags,VULN_WEAPON))
	    def = IS_VULNERABLE;
    }


   if(dam_type == 4
   || dam_type == 5
   || dam_type == 6
   || dam_type == 9
   || dam_type == 14
   || dam_type == 15
   || dam_type == 19)
    {	
	if (IS_SET(ch->imm_flags,IMM_ELEMENTAL))
	    def = IS_IMMUNE;
	else if (IS_SET(ch->res_flags,RES_ELEMENTAL))
	    def = IS_RESISTANT;
	else if (IS_SET(ch->vuln_flags,VULN_ELEMENTAL))
	    def = IS_VULNERABLE;
    }



   if(def != IS_IMMUNE
   || def != IS_RESISTANT
   || def != IS_VULNERABLE)
     {
 
      if (ch->imm_flags)
        {
      switch (dam_type)
       {
	case(DAM_BASH):		bit = IMM_BASH;		break;
	case(DAM_PIERCE):	bit = IMM_PIERCE;	break;
	case(DAM_SLASH):	bit = IMM_SLASH;	break;
	case(DAM_FIRE):		bit = IMM_FIRE;		break;
	case(DAM_COLD):		bit = IMM_COLD;		break;
	case(DAM_LIGHTNING):	bit = IMM_LIGHTNING;	break;
	case(DAM_ACID):		bit = IMM_ACID;		break;
	case(DAM_POISON):	bit = IMM_POISON;	break;
	case(DAM_NEGATIVE):	bit = IMM_NEGATIVE;	break;
	case(DAM_HOLY):		bit = IMM_HOLY;		break;
	case(DAM_ENERGY):	bit = IMM_ENERGY;	break;
	case(DAM_MENTAL):	bit = IMM_MENTAL;	break;
	case(DAM_DISEASE):	bit = IMM_DISEASE;	break;
	case(DAM_DROWNING):	bit = IMM_DROWNING;	break;
	case(DAM_LIGHT2):	bit = IMM_LIGHT2;	break;
	case(DAM_OTHER):	bit = IMM_MAGIC;	break;
	case(DAM_HARM):		bit = IMM_NEGATIVE;	break;
	case(DAM_CHARM):	bit = IMM_CHARM;	break;
	case(DAM_SOUND):	bit = IMM_SOUND;	break;
	case(DAM_MAGIC):	bit = IMM_MAGIC;	break;
	case(DAM_MAIM): 	bit = IMM_MAIM; 	break;
	case(DAM_CRUSH):	bit = IMM_CRUSH;	break;
	case(DAM_CLEAVE):	bit = IMM_CLEAVE;	break;
	case(DAM_PUNCH):	bit = IMM_PUNCH;	break;
	case(DAM_BITE):		bit = IMM_BITE;		break;
	case(DAM_CLAW):		bit = IMM_CLAW;		break;
	case(DAM_IMP):		bit = IMM_ELEMENTAL;	break;
	default:		return def;
       }
        if (IS_SET(ch->imm_flags,bit))
	  immune = IS_IMMUNE;
       }

     if ((ch->res_flags)  
     && (immune != IS_IMMUNE))
         {
       switch (dam_type)
       {
	case(DAM_BASH):		bit = RES_BASH;		break;
	case(DAM_PIERCE):	bit = RES_PIERCE;	break;
	case(DAM_SLASH):	bit = RES_SLASH;	break;
	case(DAM_FIRE):		bit = RES_FIRE;		break;
	case(DAM_COLD):		bit = RES_COLD;		break;
	case(DAM_LIGHTNING):	bit = RES_LIGHTNING;	break;
	case(DAM_ACID):		bit = RES_ACID;		break;
	case(DAM_POISON):	bit = RES_POISON;	break;
	case(DAM_NEGATIVE):	bit = RES_NEGATIVE;	break;
	case(DAM_HOLY):		bit = RES_HOLY;		break;
	case(DAM_ENERGY):	bit = RES_ENERGY;	break;
	case(DAM_MENTAL):	bit = RES_MENTAL;	break;
	case(DAM_DISEASE):	bit = RES_DISEASE;	break;
	case(DAM_DROWNING):	bit = RES_DROWNING;	break;
	case(DAM_LIGHT2):	bit = RES_LIGHT2;	break;
	case(DAM_OTHER):	bit = RES_MAGIC;	break;
	case(DAM_HARM):		bit = RES_NEGATIVE;	break;
	case(DAM_CHARM):	bit = RES_CHARM;	break;
	case(DAM_SOUND):	bit = RES_SOUND;	break;
	case(DAM_MAGIC):	bit = RES_MAGIC;	break;
	case(DAM_MAIM): 	bit = RES_MAIM; 	break;
	case(DAM_CRUSH):	bit = RES_CRUSH;	break;
	case(DAM_CLEAVE):	bit = RES_CLEAVE;	break;
	case(DAM_PUNCH):	bit = RES_PUNCH;	break;
	case(DAM_BITE):		bit = RES_BITE;		break;
	case(DAM_CLAW):		bit = RES_CLAW;		break;
	case(DAM_IMP):		bit = RES_ELEMENTAL;	break;
	default:		return def;
     }

      if (IS_SET(ch->res_flags,bit))
	immune = IS_RESISTANT;
       }

     
     if ((ch->vuln_flags)
     && (immune != IS_IMMUNE
     &&  immune != IS_RESISTANT))
        {
    switch (dam_type)
    {
	case(DAM_BASH):		bit = VULN_BASH;	break;
	case(DAM_PIERCE):	bit = VULN_PIERCE;	break;
	case(DAM_SLASH):	bit = VULN_SLASH;	break;
	case(DAM_FIRE):		bit = VULN_FIRE;	break;
	case(DAM_COLD):		bit = VULN_COLD;	break;
	case(DAM_LIGHTNING):	bit = VULN_LIGHTNING;	break;
	case(DAM_ACID):		bit = VULN_ACID;	break;
	case(DAM_POISON):	bit = VULN_POISON;	break;
	case(DAM_NEGATIVE):	bit = VULN_NEGATIVE;	break;
	case(DAM_HOLY):		bit = VULN_HOLY;	break;
	case(DAM_ENERGY):	bit = VULN_ENERGY;	break;
	case(DAM_MENTAL):	bit = VULN_MENTAL;	break;
	case(DAM_DISEASE):	bit = VULN_DISEASE;	break;
	case(DAM_DROWNING):	bit = VULN_DROWNING;	break;
	case(DAM_LIGHT2):	bit = VULN_LIGHT2;	break;
	case(DAM_OTHER):	bit = VULN_MAGIC;	break;
	case(DAM_HARM):		bit = VULN_NEGATIVE;	break;
	case(DAM_CHARM):	bit = VULN_CHARM;	break;
	case(DAM_SOUND):	bit = VULN_SOUND;	break;
	case(DAM_MAGIC):	bit = VULN_MAGIC;	break;
	case(DAM_MAIM): 	bit = VULN_MAIM; 	break;
	case(DAM_CRUSH):	bit = VULN_CRUSH;	break;
	case(DAM_CLEAVE):	bit = VULN_CLEAVE;	break;
	case(DAM_PUNCH):	bit = VULN_PUNCH;	break;
	case(DAM_BITE):		bit = VULN_BITE;	break;
	case(DAM_CLAW):		bit = VULN_CLAW;	break;
	case(DAM_IMP):		bit = VULN_ELEMENTAL;	break;
	default:		return def;
    }

    if (IS_SET(ch->vuln_flags,bit))
      immune = IS_VULNERABLE;
    }
  }
 

   if (immune == -1)
	return def;
    else
      	return immune;
 }

bool is_clan(CHAR_DATA *ch)
 {
  return ch->clan;
 }

bool is_same_clan(CHAR_DATA *ch, CHAR_DATA *victim)
  {
   CLAN_DATA *pClan = get_clan_index(ch->clan);
   if ( pClan->loners)
       return FALSE;
       return (ch->clan == victim->clan);
  }  


/* checks mob format */
bool is_old_mob(CHAR_DATA *ch)
{
    if (ch->pIndexData == NULL)
	return FALSE;
    else if (ch->pIndexData->new_format)
	return FALSE;
    return TRUE;
}
 
/* for returning skill information */
int get_skill(CHAR_DATA *ch, int sn)
{
    int skill;

    if (sn == -1) /* shorthand for level based skills */
    {
	skill = ch->level * 5 / 2;
    }

    else if (sn < -1 || sn > MAX_SKILL)
    {
        bug("Bad sn %d in get_skill.",sn);
/*	bug("HANDLER.C : GET_SKILL -  BAD SN [ %d ] ref A002",sn); */
	skill = 0;
    }

else if (!IS_NPC(ch))
    {
          if ((ch->pcdata->oldcl == -1
          &&   ch->level < skill_table[sn].skill_level[ch->class] )
          ||  (ch->pcdata->oldcl != -1
          &&   ch->level < skill_table[sn].skill_level[ch->class]
          &&   ch->level < skill_table[sn].skill_level[ch->pcdata->oldcl] ) )
            skill = 0;
          else
            skill = ch->pcdata->learned[sn];
    }


    else /* mobiles */
    {


        if (skill_table[sn].spell_fun != spell_null)
	    skill = 40 + 2 * ch->level;

	else if (sn == gsn_sneak || sn == gsn_hide)
	    skill = ch->level * 2 + 20;

        else if ((sn == gsn_dodge && IS_SET(ch->off_flags,OFF_DODGE))
 	||       (sn == gsn_parry && IS_SET(ch->off_flags,OFF_PARRY)))
	    skill = ch->level * 2;

 	else if (sn == gsn_shield_block)
	    skill = 10 + 2 * ch->level;

	else 
        if (sn == gsn_second_attack 
	&& (IS_SET(ch->act,ACT_WARRIOR) 
        || IS_SET(ch->act,ACT_ASSASSIN)
        || IS_SET(ch->act,ACT_PSIONIC)
        || IS_SET(ch->act,ACT_CLERIC)
        || IS_SET(ch->act,ACT_MAGE)
        || IS_SET(ch->act,ACT_PALADIN)
        || IS_SET(ch->act,ACT_ANTIPALADIN)
        || IS_SET(ch->act,ACT_RANGER)
        || IS_SET(ch->act,ACT_THIEF)))
	    skill = 10 + 3 * ch->level;

	else 
        if (sn == gsn_third_attack 
	&& (IS_SET(ch->act,ACT_WARRIOR) 
        || IS_SET(ch->act,ACT_ASSASSIN)
        || IS_SET(ch->act,ACT_PSIONIC)
        || IS_SET(ch->act,ACT_PALADIN)
        || IS_SET(ch->act,ACT_RANGER)
        || IS_SET(ch->act,ACT_ANTIPALADIN)
        || IS_SET(ch->act,ACT_THIEF)))
	   skill = 4 * ch->level - 40;

	else 
        if (sn == gsn_fourth_attack 
	&& (IS_SET(ch->act,ACT_WARRIOR)))
	   skill = 4 * ch->level - 40;

	else if (sn == gsn_hand_to_hand)
	    skill = 40 + 2 * ch->level;

 	else if (sn == gsn_trip && IS_SET(ch->off_flags,OFF_TRIP))
	    skill = 10 + 3 * ch->level;

 	else if (sn == gsn_bash && IS_SET(ch->off_flags,OFF_BASH))
	    skill = 10 + 3 * ch->level;

	else if (sn == gsn_disarm 
	     &&  (IS_SET(ch->off_flags,OFF_DISARM) 
	     ||   IS_SET(ch->act,ACT_WARRIOR)
             || IS_SET(ch->act,ACT_ASSASSIN)
             || IS_SET(ch->act,ACT_PALADIN)
             || IS_SET(ch->act,ACT_RANGER)
             || IS_SET(ch->act,ACT_ANTIPALADIN)
	     ||	  IS_SET(ch->act,ACT_THIEF)))
	    skill = 20 + 3 * ch->level;

	else if (sn == gsn_berserk && IS_SET(ch->off_flags,OFF_BERSERK))
	    skill = 3 * ch->level;

	else if (sn == gsn_kick)
	    skill = 10 + 3 * ch->level;

	else 
        if (sn == gsn_backstab 
        && (IS_SET(ch->act,ACT_THIEF)
	|| IS_SET(ch->act,ACT_ASSASSIN)))
        skill = 20 + 2 * ch->level;

  	else if (sn == gsn_rescue)
	    skill = 40 + ch->level; 

	else if (sn == gsn_recall)
	    skill = 40 + ch->level;

	else 
        if (sn == gsn_sword
	||  sn == gsn_dagger
	||  sn == gsn_spear
	||  sn == gsn_mace
	||  sn == gsn_axe
	||  sn == gsn_flail
	||  sn == gsn_whip
	||  sn == gsn_polearm
	||  sn == gsn_exotic)
	    skill = 40 + 5 * ch->level / 2;

	else 
	   skill = 0;
    }

    if (ch->daze > 0)
    {
	if (skill_table[sn].spell_fun != spell_null)
	    skill /= 2;
	else
	    skill = 2 * skill / 3;
    }

    if ( !IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK]  > 10 )
	skill = 9 * skill / 10;

    return URANGE(0,skill,100);
}

/* for returning weapon information */

/* new version (for Dual Wield) */
int get_weapon_sn(CHAR_DATA *ch,bool fDual)
{
  OBJ_DATA *wield;
  int sn;

  wield = get_eq_char( ch, fDual ? WEAR_SECONDARY : WEAR_WIELD );
  if (wield == NULL || wield->item_type != ITEM_WEAPON)
    sn = gsn_hand_to_hand;
  else switch (wield->value[0])
      {
      default :               sn = -1;                break;
      case(WEAPON_SWORD):     sn = gsn_sword;         break;
      case(WEAPON_DAGGER):    sn = gsn_dagger;        break;
      case(WEAPON_SPEAR):     sn = gsn_spear;         break;
      case(WEAPON_MACE):      sn = gsn_mace;          break;
      case(WEAPON_AXE):       sn = gsn_axe;           break;
      case(WEAPON_FLAIL):     sn = gsn_flail;         break;
      case(WEAPON_WHIP):      sn = gsn_whip;          break;
      case(WEAPON_POLEARM):   sn = gsn_polearm;       break;
      case(WEAPON_EXOTIC):    sn = gsn_exotic;          break;
      }
	return sn;
}


int get_weapon_skill(CHAR_DATA *ch, int sn)
{
     int skill;

     /* -1 is none */
    if (IS_NPC(ch))
    {
	if (sn == -1)
	    skill = 3 * ch->level;
	else if (sn == gsn_hand_to_hand)
	    skill = 40 + 2 * ch->level;
	else 
	    skill = 40 + 5 * ch->level / 2;
    }
    
    else
    {
	if (sn == -1)
	    skill = 3 * ch->level;
	else
	    skill = ch->pcdata->learned[sn];
    }

    return URANGE(0,skill,100);
} 


/* used to de-screw characters */
void reset_char(CHAR_DATA *ch)
{
     int loc,mod,stat;
     OBJ_DATA *obj;
     AFFECT_DATA *af;
     int i;

     if (IS_NPC(ch))
	return;

    if (ch->pcdata->perm_hit == 0 
    ||	ch->pcdata->perm_mana == 0
    ||  ch->pcdata->perm_move == 0
    ||	ch->pcdata->last_level == 0)
    {
    /* do a FULL reset */
	for (loc = 0; loc < MAX_WEAR; loc++)
	{
	    obj = get_eq_char(ch,loc);
	    if (obj == NULL)
		continue;
	    if (!obj->enchanted)
	    for ( af = obj->pIndexData->affected; af != NULL; af = af->next )
	    {
		mod = af->modifier;

		switch(af->location)
		{
		    case APPLY_SEX:	ch->sex		-= mod;
					if (ch->sex < 0 || ch->sex >2)
					    ch->sex = IS_NPC(ch) ?
						0 :
						ch->pcdata->true_sex;
									break;
		    case APPLY_MANA:	ch->max_mana	-= mod;		break;
		    case APPLY_HIT:	ch->max_hit	-= mod;		break;
		    case APPLY_MOVE:	ch->max_move	-= mod;		break;
		}
	    }

            for ( af = obj->affected; af != NULL; af = af->next )
            {
                mod = af->modifier;

                switch(af->location)
                {
                    case APPLY_SEX:     ch->sex         -= mod;         break;
                    case APPLY_MANA:    ch->max_mana    -= mod;         break;
                    case APPLY_HIT:     ch->max_hit     -= mod;         break;
                    case APPLY_MOVE:    ch->max_move    -= mod;         break;
                }
            }
	}
	/* now reset the permanent stats */
	ch->pcdata->perm_hit 	= ch->max_hit;
	ch->pcdata->perm_mana 	= ch->max_mana;
	ch->pcdata->perm_move	= ch->max_move;
	ch->pcdata->last_level	= ch->played/3600;
	if (ch->pcdata->true_sex < 0 || ch->pcdata->true_sex > 2)
		{
		if (ch->sex > 0 && ch->sex < 3)
	    	    ch->pcdata->true_sex	= ch->sex;
		else
		    ch->pcdata->true_sex 	= 0;
		}

    }

    /* now restore the character to his/her true condition */
    for (stat = 0; stat < MAX_STATS; stat++)
	ch->mod_stat[stat] = 0;

    if (ch->pcdata->true_sex < 0 || ch->pcdata->true_sex > 2)
	ch->pcdata->true_sex = 0;

    ch->sex		= ch->pcdata->true_sex;
    ch->max_hit 	= ch->pcdata->perm_hit;
    ch->max_mana	= ch->pcdata->perm_mana;
    ch->max_move	= ch->pcdata->perm_move;

    for (i = 0; i < 4; i++)
    	ch->armor[i]	= 100;

    ch->hitroll		= 0;
    ch->damroll		= 0;
    ch->saving_throw	= 0;

    /* now start adding back the effects */
    for (loc = 0; loc < MAX_WEAR; loc++)
    {
        obj = get_eq_char(ch,loc);
        if (obj == NULL)
            continue;
	for (i = 0; i < 4; i++)
	    ch->armor[i] -= apply_ac( obj, loc, i );

        if (!obj->enchanted)
	for ( af = obj->pIndexData->affected; af != NULL; af = af->next )
        {
            mod = af->modifier;

            switch(af->location)
            {

    case APPLY_STR:           
     if(mod > 4)
        mod = 4;

     ch->mod_stat[STAT_STR] += mod;	
     break;

    case APPLY_INT:           
     if(mod > 4)
        mod = 4;

     ch->mod_stat[STAT_INT] += mod;	
     break;

    case APPLY_WIS:           
     if(mod > 4)
        mod = 4;

     ch->mod_stat[STAT_WIS] += mod;	
     break;

    case APPLY_DEX:           
     if(mod > 4)
        mod = 4;

     ch->mod_stat[STAT_DEX] += mod;	
     break;

    case APPLY_CON:           
     if(mod > 4)
        mod = 4;

     ch->mod_stat[STAT_CON] += mod;	
     break;

		case APPLY_SEX:		ch->sex			+= mod; break;
		case APPLY_MANA:	ch->max_mana		+= mod; break;
		case APPLY_HIT:		ch->max_hit		+= mod; break;
		case APPLY_MOVE:	ch->max_move		+= mod; break;

		case APPLY_AC:		
		    for (i = 0; i < 4; i ++)
			ch->armor[i] += mod; 
		    break;
		case APPLY_HITROLL:	ch->hitroll		+= mod; break;
		case APPLY_DAMROLL:	ch->damroll		+= mod; break;
	
		case APPLY_SAVES:		ch->saving_throw += mod; break;
		case APPLY_SAVING_ROD: 					 break;
		case APPLY_SAVING_PETRI:				 break;
		case APPLY_SAVING_BREATH: 				 break;
		case APPLY_SAVING_SPELL:				 break;
	    }
        }
 
        for ( af = obj->affected; af != NULL; af = af->next )
        {
            mod = af->modifier;

            switch(af->location)
            {

    case APPLY_STR:
     if(mod > 4)
        mod = 4;
                
     ch->mod_stat[STAT_STR] += mod;
     break;
        
    case APPLY_INT:
     if(mod > 4)
        mod = 4;
    
     ch->mod_stat[STAT_INT] += mod;
     break;
                
    case APPLY_WIS:
     if(mod > 4)
        mod = 4;
                    
     ch->mod_stat[STAT_WIS] += mod;
     break;
                
    case APPLY_DEX:
     if(mod > 4)
        mod = 4;
                
     ch->mod_stat[STAT_DEX] += mod;
     break;
                
    case APPLY_CON:
     if(mod > 4)
        mod = 4;
        
     ch->mod_stat[STAT_CON] += mod;
     break;

                case APPLY_SEX:         ch->sex                 += mod; break;
                case APPLY_MANA:        ch->max_mana            += mod; break;
                case APPLY_HIT:         ch->max_hit             += mod; break;
                case APPLY_MOVE:        ch->max_move            += mod; break;

                case APPLY_AC:
                    for (i = 0; i < 4; i ++)
                        ch->armor[i] += mod;
                    break;
		case APPLY_HITROLL:     ch->hitroll             += mod; break;
                case APPLY_DAMROLL:     ch->damroll             += mod; break;
 
                case APPLY_SAVES:         ch->saving_throw += mod; break;
                case APPLY_SAVING_ROD:          			 break;
                case APPLY_SAVING_PETRI:        			 break;
                case APPLY_SAVING_BREATH:       			 break;
                case APPLY_SAVING_SPELL:        			 break;
            }

	}
    }
  
    /* now add back spell effects */
    for (af = ch->affected; af != NULL; af = af->next)
    {
        mod = af->modifier;

        switch(af->location)
        {
   case APPLY_STR:
     if(mod > 4)
        mod = 4;
                        
     ch->mod_stat[STAT_STR] += mod;
     break;
                
    case APPLY_INT:
     if(mod > 4)
        mod = 4;
                
     ch->mod_stat[STAT_INT] += mod;
     break;
                
    case APPLY_WIS:
     if(mod > 4)
        mod = 4;
           
     ch->mod_stat[STAT_WIS] += mod;
     break;     
            
    case APPLY_DEX:
     if(mod > 4)
        mod = 4;
                
     ch->mod_stat[STAT_DEX] += mod;
     break;     
                
    case APPLY_CON:
     if(mod > 4)
        mod = 4;
                
     ch->mod_stat[STAT_CON] += mod;
     break; 

                case APPLY_SEX:         ch->sex                 += mod; break;
                case APPLY_MANA:        ch->max_mana            += mod; break;
                case APPLY_HIT:         ch->max_hit             += mod; break;
                case APPLY_MOVE:        ch->max_move            += mod; break;
 
                case APPLY_AC:
                    for (i = 0; i < 4; i ++)
                        ch->armor[i] += mod;
                    break;
                case APPLY_HITROLL:     ch->hitroll             += mod; break;
                case APPLY_DAMROLL:     ch->damroll             += mod; break;
 
                case APPLY_SAVES:         ch->saving_throw += mod; break;
                case APPLY_SAVING_ROD:          			 break;
                case APPLY_SAVING_PETRI:        			 break;
                case APPLY_SAVING_BREATH:       			 break;
                case APPLY_SAVING_SPELL:       				 break;
        } 

}

    /* make sure sex is RIGHT!!!! */
    if (ch->sex < 0 || ch->sex > 2)
	ch->sex = ch->pcdata->true_sex;

    if (!(ch->perm_size))
      {
      ch->perm_size = pc_race_table[ch->race].size;
      ch->size = ch->perm_size;
      }
    else
      ch->size = ch->perm_size;

    if (ch->race == RACE_MINOTAUR)
      {
       ch->damroll += UMAX(1,ch->level/5);
       ch->hitroll += UMAX(1,ch->level/5);
       
       for (i = 0; i < 4; i ++)
        ch->armor[i] += UMAX(10,10*(ch->level/5));
      }

}


/*
 * Retrieve a character's age.
 */
int get_age( CHAR_DATA *ch )
{
    return 17 + ( ch->played + (int) (current_time - ch->logon) ) / 72000;
}

/* command for retrieving stats */
int get_curr_stat( CHAR_DATA *ch, int stat )
{
    int max;

    if (IS_NPC(ch) || ch->level > LEVEL_IMMORTAL)
	max = 40;
    else
    {
        if ((ch->race == race_lookup("Golem"))
        || (ch->race == race_lookup("Nephilim"))
        || (ch->race == race_lookup("Sidhe Elf"))
        || (ch->race == race_lookup("Myrddraal")))
          {
 	   max = pc_race_table[ch->race].max_stats[stat];

	   if (class_table[ch->class].attr_prime == stat)
	     max += 2;
          }
        else
          {
 	   max = pc_race_table[ch->race].max_stats[stat];

	   if (class_table[ch->class].attr_prime == stat)
	     max += 2;
          }

	if ( ch->race == race_lookup("Human"))
	    max += 1;


        if ((ch->race == race_lookup("Golem"))
        || (ch->race == race_lookup("Nephilim"))
        || (ch->race == race_lookup("Sidhe Elf"))
        || (ch->race == race_lookup("Myrddraal")))
   	  max = UMIN(max,40);
        else
          max = UMIN(max,40);
    }
  
    return URANGE(3,ch->perm_stat[stat] + ch->mod_stat[stat], max);
}

/* command for returning max training score */
int get_max_train( CHAR_DATA *ch, int stat )
{
    int max;

    if (IS_NPC(ch) || ch->level > LEVEL_IMMORTAL || IS_LEGEND(ch))
	return 40;
    else
     {
      max = pc_race_table[ch->race].max_stats[stat];

      if (class_table[ch->class].attr_prime == stat)
		 {
			if (ch->race == race_lookup("human"))
			   max += 3;
			else
			   max += 2;
		 }

        if ((ch->race == race_lookup("Golem"))
        || (ch->race == race_lookup("Nephilim"))
        || (ch->race == race_lookup("Sidhe Elf"))
        || (ch->race == race_lookup("Myrddraal")))
          return UMIN(max,27);
        else
          return UMIN(max,25);
     }    
}
   
	
/*
 * Retrieve a character's carry capacity.
 */
int can_carry_n( CHAR_DATA *ch )
{
    if ( !IS_NPC(ch) && ch->level >= LEVEL_IMMORTAL )
	return 1000;

    if ( IS_NPC(ch) && IS_SET(ch->act, ACT_PET) )
	return -1;

    return MAX_WEAR +  2 * get_curr_stat(ch,STAT_DEX) + ch->level;
}



/*
 * Retrieve a character's carry capacity.
 */
int can_carry_w( CHAR_DATA *ch )
{
    if ( !IS_NPC(ch) && ch->level >= LEVEL_IMMORTAL )
	return 10000000;

    if ( IS_NPC(ch) && IS_SET(ch->act, ACT_PET) )
	return -1;

/*    return str_app[get_curr_stat(ch,STAT_STR)].carry * 10 + ch->level * 25; */
    return ((str_app[get_curr_stat(ch,STAT_STR)].carry * 6) + (ch->level * 1.25));

}



/*
 * See if a string is one of the names of an object.
 */



bool is_name ( char *str, char *namelist )
{
    char name[MAX_INPUT_LENGTH], part[MAX_INPUT_LENGTH];
    char *list, *string;

    /* fix crash on NULL namelist */
    if (namelist == NULL || namelist[0] == '\0')
		return FALSE;

    /* fixed to prevent is_name on "" returning TRUE */
    if (str[0] == '\0')
	return FALSE;

    string = str;
    /* we need ALL parts of string to match part of namelist */
    for ( ; ; )  /* start parsing string */
    {
	str = one_argument(str,part);

	if (part[0] == '\0' )
	    return TRUE;

	/* check to see if this is part of namelist */
	list = namelist;
	for ( ; ; )  /* start parsing namelist */
	{
	    list = one_argument(list,name);
	    if (name[0] == '\0')  /* this name was not found */
		return FALSE;

	    if (!str_prefix(string,name))
		return TRUE; /* full pattern match */

	    if (!str_prefix(part,name))
		break;
	}
    }
}

bool is_exact_name(char *str, char *namelist )
{
    char name[MAX_INPUT_LENGTH];

    if (namelist == NULL)
	return FALSE;

    for ( ; ; )
    {
	namelist = one_argument( namelist, name );
	if ( name[0] == '\0' )
	    return FALSE;
	if ( !str_cmp( str, name ) )
	    return TRUE;
    }
}

/* enchanted stuff for eq */
void affect_enchant(OBJ_DATA *obj)
{
    /* okay, move all the old flags into new vectors if we have to */
    if (!obj->enchanted)
    {
        AFFECT_DATA *paf, *af_new;
        obj->enchanted = TRUE;

        for (paf = obj->pIndexData->affected;
             paf != NULL; paf = paf->next)
        {
	    af_new = new_affect();

            af_new->next = obj->affected;
            obj->affected = af_new;
 
	    af_new->where	= paf->where;
            af_new->type        = UMAX(0,paf->type);
            af_new->level       = paf->level;
            af_new->duration    = paf->duration;
            af_new->location    = paf->location;
            af_new->modifier    = paf->modifier;
            af_new->bitvector   = paf->bitvector;
        }
    }
}
           

/*
 * Apply or remove an affect to a character.
 */
void affect_modify( CHAR_DATA *ch, AFFECT_DATA *paf, bool fAdd )
{
    OBJ_DATA *wield;
    int mod,i;
EXTRA_DESCR_DATA *  ed;

    mod = paf->modifier;

    if ( fAdd )
    {
	switch (paf->where)
	{
	case TO_AFFECTS:
	    SET_BIT(ch->affected_by, paf->bitvector);
	    break;
	case TO_AFFECTS2:
	    SET_BIT(ch->affected2_by, paf->bitvector);
	    break;
	case TO_IMMUNE:
	    SET_BIT(ch->imm_flags,paf->bitvector);
	    break;
	case TO_RESIST:
	    SET_BIT(ch->res_flags,paf->bitvector);
	    break;
	case TO_VULN:
	    SET_BIT(ch->vuln_flags,paf->bitvector);
	    break;
	}
    }
    else
    {
        switch (paf->where)
        {
        case TO_AFFECTS:
            REMOVE_BIT(ch->affected_by, paf->bitvector);
            break;
        case TO_AFFECTS2:
            REMOVE_BIT(ch->affected2_by, paf->bitvector);
            break;
        case TO_IMMUNE:
            REMOVE_BIT(ch->imm_flags,paf->bitvector);
            break;
        case TO_RESIST:
            REMOVE_BIT(ch->res_flags,paf->bitvector);
            break;
        case TO_VULN:
            REMOVE_BIT(ch->vuln_flags,paf->bitvector);
            break;
        }
       mod = 0 - mod;
    }

    switch ( paf->location )
    {


    default:
        bug( "Affect_modify: unknown location %d.", paf->location );
        return;

    case APPLY_NONE:						break;
 
    case APPLY_STR:           
     if ( fAdd )
       {
        if(mod > 4)
           mod = 4;

        ch->mod_stat[STAT_STR] += mod;	
       }
     else
       {
        if(mod < -4)
         mod = -4;

        ch->mod_stat[STAT_STR] += mod;	
       }

     break;

    case APPLY_INT:           
     if ( fAdd )
       {
        if(mod > 4)
           mod = 4;
        
        ch->mod_stat[STAT_INT] += mod;
       } 
     else
       {   
        if(mod < -4)
         mod = -4;
        
        ch->mod_stat[STAT_INT] += mod;
       }
       
     break;

    case APPLY_WIS:           
     if ( fAdd )
       {
        if(mod > 4)
           mod = 4;
        
        ch->mod_stat[STAT_WIS] += mod;
       } 
     else
       {   
        if(mod < -4)
         mod = -4;
        
        ch->mod_stat[STAT_WIS] += mod;
       }
       
     break;

    case APPLY_DEX:           
     if ( fAdd )
       {
        if(mod > 4)
           mod = 4;
        
        ch->mod_stat[STAT_DEX] += mod;
       } 
     else
       {   
        if(mod < -4)
         mod = -4;
        
        ch->mod_stat[STAT_DEX] += mod;
       }
       
     break;

    case APPLY_CON:           
     if ( fAdd )
       {
        if(mod > 4)
           mod = 4;
        
        ch->mod_stat[STAT_CON] += mod;
       } 
     else
       {   
        if(mod < -4)
         mod = -4;
        
        ch->mod_stat[STAT_CON] += mod;
       }
       
     break;

    case APPLY_SEX:           ch->sex			+= mod;	break;
    case APPLY_CLASS:						break;
    case APPLY_LEVEL:						break;
    case APPLY_AGE:						break;
    case APPLY_HEIGHT:						break;
    case APPLY_WEIGHT:						break;
    case APPLY_MANA:          ch->max_mana		+= mod;	break;
    case APPLY_HIT:           ch->max_hit		+= mod;	break;
    case APPLY_MOVE:          ch->max_move		+= mod;	break;
    case APPLY_GOLD:						break;
    case APPLY_EXP:						break;
    case APPLY_AC:
        for (i = 0; i < 4; i ++)
            ch->armor[i] += mod;
        break;
    case APPLY_HITROLL:       ch->hitroll		+= mod;	break;
    case APPLY_DAMROLL:       ch->damroll		+= mod;	break;
    case APPLY_SAVES:         ch->saving_throw		+= mod;	break;
    case APPLY_SAVING_ROD:    					break;
    case APPLY_SAVING_PETRI:					break;
    case APPLY_SAVING_BREATH: 					break;
    case APPLY_SAVING_SPELL:					break;
    case APPLY_SPELL_AFFECT:  					break;
    }

    /*
     * Check for weapon wielding.
     * Guard against recursion (for weapons with affects).
     */
    if ( !IS_NPC(ch) && ( wield = get_eq_char( ch, WEAR_WIELD ) ) != NULL
    &&   get_obj_weight(wield) > (str_app[get_curr_stat(ch,STAT_STR)].wield*10))
    {
	static int depth;

	if ( depth == 0 )
	{
	    depth++;
	    act( "You drop $p.", ch, wield, NULL, TO_CHAR );
	    act( "$n drops $p.", ch, wield, NULL, TO_ROOM );
    if(wield->extra_descr != NULL || wield->pIndexData->extra_descr != NULL)
    {
        for (ed = wield->extra_descr; ed != NULL; ed = ed->next )
        {
            if(!str_cmp("dropmessage", ed->keyword))
                send_to_char(ed->description, ch);
        }

        for ( ed = wield->pIndexData->extra_descr; ed != NULL; ed = ed->next )
        {
            if(!str_cmp("dropmessage", ed->keyword))
                send_to_char(ed->description, ch);
        }
    }

	    obj_from_char( wield );
	    obj_to_room( wield, ch->in_room );
	    depth--;
	}
    }

    return;
}


/* find an effect in an affect list */
AFFECT_DATA  *affect_find(AFFECT_DATA *paf, int sn)
{
    AFFECT_DATA *paf_find;
    
    for ( paf_find = paf; paf_find != NULL; paf_find = paf_find->next )
    {
        if ( paf_find->type == sn )
	return paf_find;
    }

    return NULL;
}

/* fix object affects when removing one */
void affect_check(CHAR_DATA *ch,int where,int vector)
{
    AFFECT_DATA *paf;
    OBJ_DATA *obj;

    if (where == TO_OBJECT || where == TO_WEAPON || vector == 0)
	return;

    for (paf = ch->affected; paf != NULL; paf = paf->next)
	if (paf->where == where && paf->bitvector == vector)
	{
	    switch (where)
	    {
	        case TO_AFFECTS:
		    SET_BIT(ch->affected_by,vector);
		    break;
                case TO_AFFECTS2:
                    SET_BIT(ch->affected2_by, vector);
                    break;
	        case TO_IMMUNE:
		    SET_BIT(ch->imm_flags,vector);   
		    break;
	        case TO_RESIST:
		    SET_BIT(ch->res_flags,vector);
		    break;
	        case TO_VULN:
		    SET_BIT(ch->vuln_flags,vector);
		    break;
	    }
	    return;
	}

    for (obj = ch->carrying; obj != NULL; obj = obj->next_content)
    {
	if (obj->wear_loc == -1)
	    continue;

            for (paf = obj->affected; paf != NULL; paf = paf->next)
            if (paf->where == where && paf->bitvector == vector)
            {
                switch (where)
                {
                    case TO_AFFECTS:
                        SET_BIT(ch->affected_by,vector);
                        break;
                    case TO_AFFECTS2:
                        SET_BIT(ch->affected2_by,vector);
                        break;
                    case TO_IMMUNE:
                        SET_BIT(ch->imm_flags,vector);
                        break;
                    case TO_RESIST:
                        SET_BIT(ch->res_flags,vector);
                        break;
                    case TO_VULN:
                        SET_BIT(ch->vuln_flags,vector);
                  
                }
                return;
            }

        if (obj->enchanted)
	    continue;

        for (paf = obj->pIndexData->affected; paf != NULL; paf = paf->next)
            if (paf->where == where && paf->bitvector == vector)
            {
                switch (where)
                {
                    case TO_AFFECTS:
                        SET_BIT(ch->affected_by,vector);
                        break;
                    case TO_AFFECTS2:
                        SET_BIT(ch->affected2_by,vector);
                        break;
                    case TO_IMMUNE:
                        SET_BIT(ch->imm_flags,vector);
                        break;
                    case TO_RESIST:
                        SET_BIT(ch->res_flags,vector);
                        break;
                    case TO_VULN:
                        SET_BIT(ch->vuln_flags,vector);
                        break;
                }
                return;
            }
    }
}

/*
 * Give an affect to a char.
 */
void affect_to_char( CHAR_DATA *ch, AFFECT_DATA *paf )
{
    AFFECT_DATA *paf_new;

    paf_new = new_affect();

    *paf_new		= *paf;
    
    if(gInfiniteAffectDuration)
        paf_new->duration = -1;

    VALIDATE(paf);	/* in case we missed it when we set up paf */
    paf_new->next	= ch->affected;
    ch->affected	= paf_new;

    affect_modify( ch, paf_new, TRUE );
    return;
}

/* give an affect to an object */
void affect_to_obj(OBJ_DATA *obj, AFFECT_DATA *paf)
{
    AFFECT_DATA *paf_new;

    paf_new = new_affect();

    *paf_new		= *paf;

    VALIDATE(paf);	/* in case we missed it when we set up paf */
    paf_new->next	= obj->affected;
    obj->affected	= paf_new;

    /* apply any affect vectors to the object's extra_flags */
    if (paf->bitvector)
        switch (paf->where)
        {
        case TO_OBJECT:
    	    SET_BIT(obj->extra_flags,paf->bitvector);
	    break;
        case TO_WEAPON:
	    if (obj->item_type == ITEM_WEAPON)
	        SET_BIT(obj->value[4],paf->bitvector);
	    break;
        }
    

    return;
}



/*
 * Remove an affect from a char.
 */
void affect_remove( CHAR_DATA *ch, AFFECT_DATA *paf )
{
    int where;
    int vector;
/* for new BUG
char buf[MAX_STRING_LENGTH];
*/
    if ( ch->affected == NULL )
    {
        bug( "Affect_remove: no affect.", 0 );
/*	bug("HANDLER.C : AFFECT_REMOVED - ref A003", 0 ); */
	return;
    }

    affect_modify( ch, paf, FALSE );
    where = paf->where;
    vector = paf->bitvector;

    if ( paf == ch->affected )
    {
	ch->affected	= paf->next;
    }
    else
    {
	AFFECT_DATA *prev;

	for ( prev = ch->affected; prev != NULL; prev = prev->next )
	{
	    if ( prev->next == paf )
	    {
		prev->next = paf->next;
		break;
	    }
	}

	if ( prev == NULL )
	{

/*
      sprintf(buf, "HANDLER.C : AFFECT_REMOVE - PAF MISSING [ CH: %s ] ref A004",ch->name);
      bug(buf,0);
*/
            bug( "Affect_remove: cannot find paf.", 0 );
	    return;
	}
    }

    free_affect(paf);

    affect_check(ch,where,vector);
    return;
}

void affect_remove_obj( OBJ_DATA *obj, AFFECT_DATA *paf)
{
    int where, vector;
/* for new BUG
char buf[MAX_STRING_LENGTH];
*/

    if ( obj->affected == NULL )
    {
/*
  sprintf(buf, "HANDLER.C : AFFT_REMOVE_OBJ - NO AFFT [ OVNUM: %d ] ref A005",obj->pIndexData->vnum);
  bug(buf,0);
*/
        bug( "Affect_remove_object: no affect.", 0 );
        return;
    }

    if (obj->carried_by != NULL && obj->wear_loc != -1)
	affect_modify( obj->carried_by, paf, FALSE );

    where = paf->where;
    vector = paf->bitvector;

    /* remove flags from the object if needed */
    if (paf->bitvector)
	switch( paf->where)
        {
        case TO_OBJECT:
            REMOVE_BIT(obj->extra_flags,paf->bitvector);
            break;
        case TO_WEAPON:
            if (obj->item_type == ITEM_WEAPON)
                REMOVE_BIT(obj->value[4],paf->bitvector);
            break;
        }

    if ( paf == obj->affected )
    {
        obj->affected    = paf->next;
    }
    else
    {
        AFFECT_DATA *prev;

        for ( prev = obj->affected; prev != NULL; prev = prev->next )
        {
            if ( prev->next == paf )
            {
                prev->next = paf->next;
                break;
            }
        }

        if ( prev == NULL )
        {

/*            bug( "HANDLER.C : AFFT_REMOVE_OBJ - NO PAF  ref A006", 0 ); */
            bug( "Affect_remove_object: cannot find paf.", 0 );
            return;
        }
    }

    free_affect(paf);

    if (obj->carried_by != NULL && obj->wear_loc != -1)
	affect_check(obj->carried_by,where,vector);
    return;
}



/*
 * Strip all affects of a given sn.
 */
void affect_strip( CHAR_DATA *ch, int sn )
{
    AFFECT_DATA *paf;
    AFFECT_DATA *paf_next;

    for ( paf = ch->affected; paf != NULL; paf = paf_next )
    {
	paf_next = paf->next;
	if ( paf->type == sn )
	    affect_remove( ch, paf );
    }
    return;
}



/*
 * Return true if a char is affected by a spell.
 */

bool is_affected( CHAR_DATA *ch, int sn )
{
    AFFECT_DATA *paf;

    for ( paf = ch->affected; paf != NULL; paf = paf->next )
    {
	if ( paf->type == sn )
	    return TRUE;
    }

    return FALSE;
}

/*
 * Add or enhance an affect.
 */


void affect_join( CHAR_DATA *ch, AFFECT_DATA *paf )
{
    AFFECT_DATA *paf_old;
    bool found;
       
    found = FALSE;
    for ( paf_old = ch->affected; paf_old != NULL; paf_old = paf_old->next )
    {
        if ( paf_old->type == paf->type )
        {
            paf->level = (paf->level += paf_old->level) / 2;
            paf->duration += paf_old->duration;
            /* paf->modifier += paf_old->modifier; */
            affect_remove( ch, paf_old );
            break;
        }
    }
        
    affect_to_char( ch, paf );
    return;
}


/*
 * Move a char out of a room.
 */
void char_from_room( CHAR_DATA *ch )
{
    OBJ_DATA *obj;
/* for new BUG
char buf[MAX_STRING_LENGTH];
*/

    if ( ch->in_room == NULL )
    {
/*     bug( "HANDLER.C : CHAR_FROM_ROOM - NO ROOM  ref A007",0); */

        bug( "Char_from_room: NULL.", 0 );
	return;
    }

    if ( !IS_NPC(ch) )
      {
       if (!IS_IMMORTAL(ch))
         {
  	  --ch->in_room->area->nplayer;
         }
      }


    if  ( (obj = get_eq_char( ch, WEAR_LIGHT ) ) != NULL
    &&  obj->item_type == ITEM_LIGHT_SOURCE
    &&   obj->value[2] != 0
    &&   ch->in_room->light > 0 )
	--ch->in_room->light;

    if ( ch == ch->in_room->people )
    {
	ch->in_room->people = ch->next_in_room;
    }
    else
    {
	CHAR_DATA *prev;

	for ( prev = ch->in_room->people; prev; prev = prev->next_in_room )
	{
	    if ( prev->next_in_room == ch )
	    {
		prev->next_in_room = ch->next_in_room;
		break;
	    }
	}

	if ( prev == NULL )

/*
       sprintf( buf,"HANDLER.C : CHAR_FROM_ROOM - NO CHAR [ CH: %s ] ref A008", ch->name);
       bug(buf,0);
*/
        if ( prev == NULL )
            bug( "Char_from_room: ch not found.", 0 );
    }

    ch->in_room      = NULL;
    ch->next_in_room = NULL;
    ch->on 	     = NULL;  /* sanity check! */
    return;
}



/*
 * Move a char into a room.  BUG WIZZLE
 */
void char_to_room( CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex )
{
    OBJ_DATA *obj;
    CHAR_DATA *bch;
    // Explorer/Killer Percentages
    char    explorerBuf[1024];

    bch = GET_CHAR(ch); 


    if ( pRoomIndex != NULL )
      {
            // Explorer/Killer Percentages
            sprintf(explorerBuf, "%d", pRoomIndex->vnum);
            //sprintf(buf, "you just entered %s", explorerBuf);
            //send_to_char(buf, ch);
            if(valid_explorer_killer(ch) && (!ch->pcdata->explored || array_find(ch->pcdata->explored, explorerBuf)==-1))
            {
                if(ch->pcdata->alertMe && ch->desc->connected==CON_PLAYING)
                    send_to_char("\n\r{C***{M*** {GYou just explored a new room!!! {M***{C***{x\n\r", ch);
                ch->pcdata->explored = array_append(ch->pcdata->explored, explorerBuf);
            }


    ch->in_room		= pRoomIndex;
    ch->next_in_room	= pRoomIndex->people;
    pRoomIndex->people	= ch;

    if ( !IS_NPC(ch) )
    {
     if (!IS_IMMORTAL(ch))
       {
	if (ch->in_room->area->empty)
	{
	    ch->in_room->area->empty = FALSE;
	    ch->in_room->area->age = 0;
	}
	++ch->in_room->area->nplayer;
       }
    }

    if ( ( obj = get_eq_char( ch, WEAR_LIGHT ) ) != NULL
    &&   obj->item_type == ITEM_LIGHT_SOURCE
    &&   obj->value[2] != 0 )
	++ch->in_room->light;
	
    if (IS_AFFECTED(ch,AFF_PLAGUE))
    {
        AFFECT_DATA *af, plague;
        CHAR_DATA *vch;
        
        for ( af = ch->affected; af != NULL; af = af->next )
        {
            if (af->type == gsn_plague)
                break;
        }
        
        if (af == NULL)
        {
            REMOVE_BIT(ch->affected_by,AFF_PLAGUE);
            return;
        }
        
        if (af->level == 1)
            return;
        
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
	    &&  !IS_IMMORTAL(vch) &&
            	!IS_AFFECTED(vch,AFF_PLAGUE) && number_bits(6) == 0)
            {
            	send_to_char("You feel hot and feverish.\n\r",vch);
            	act("$n shivers and looks very ill.",vch,NULL,NULL,TO_ROOM);
            	affect_join(vch,&plague);
            }
        }
    }

  }
else
 {
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *nch;
	ROOM_INDEX_DATA *room;

       nch = GET_CHAR( bch );

       if (!IS_NPC(nch) )
        {
       sprintf(buf, "HANDLER.C : CHAR_TO_ROOM - NO RM  [ CN: %s RVNUM: ] ref A009",nch->name);       bug(buf,0);
        }
	else	
        {       
sprintf(buf, "HANDLER.C : CHAR_TO_ROOM - NO RM  [ MVNUM: %d RVNUM: ] ref A009",
nch->pIndexData->vnum);
       bug(buf,0);
        }


	if ((room = get_room_index(ROOM_VNUM_TEMPLE)) != NULL)
	    char_to_room(nch,room);
	
  }

    return;
}



/*
 * Give an obj to a char.
 */
void obj_to_char( OBJ_DATA *obj, CHAR_DATA *ch )
{
    // Explorer/Killer Percentages
    char    treasureHunterBuf[1024];
    
    // Explorer/Killer Percentages
    if(obj && obj->pIndexData)
    {
        sprintf(treasureHunterBuf, "%d", obj->pIndexData->vnum);
        if(valid_explorer_killer(ch) && (!ch->pcdata->objectsFound || array_find(ch->pcdata->objectsFound, treasureHunterBuf)==-1))
        {
            if(ch->pcdata->alertMe && ch->desc->connected==CON_PLAYING)
                send_to_char("\n\r{C***{M*** {GYou just found a new object!!! {M***{C***{x\n\r", ch);
            ch->pcdata->objectsFound = array_append(ch->pcdata->objectsFound, treasureHunterBuf);
        }
    }

    obj->next_content	 = ch->carrying;
    ch->carrying	 = obj;
    obj->carried_by	 = ch;
    obj->in_room	 = NULL;
    obj->in_obj		 = NULL;
    ch->carry_number	+= get_obj_number( obj );
    ch->carry_weight	+= get_obj_weight( obj );
}



/*
 * Take an obj from its character.
 */
void obj_from_char( OBJ_DATA *obj )
{
    CHAR_DATA *ch;

    if ( ( ch = obj->carried_by ) == NULL )
    {
	bug( "Obj_from_char: null ch.", 0 );
	return;
    }

    if ( obj->wear_loc != WEAR_INVENTORY )
	unequip_char( ch, obj );

    if ( ch->carrying == obj )
    {
	ch->carrying = obj->next_content;
    }
    else
    {
	OBJ_DATA *prev;

	for ( prev = ch->carrying; prev != NULL; prev = prev->next_content )
	{
	    if ( prev->next_content == obj )
	    {
		prev->next_content = obj->next_content;
		break;
	    }
	}

	if ( prev == NULL )
	    bug( "Obj_from_char: obj not in list.", 0 );
    }

    // Stones of Wisdom Stuff
    stones_object_leaving(obj);

    obj->carried_by	 = NULL;
    obj->next_content	 = NULL;
    ch->carry_number	-= get_obj_number( obj );
    ch->carry_weight	-= get_obj_weight( obj );
    return;
}


/*
 * Find the ac value of an obj, including position effect.
 */
int apply_ac( OBJ_DATA *obj, int iWear, int type )
{
    if ( obj->item_type != ITEM_ARMOR )
	return 0;

    switch ( iWear )
    {
    case WEAR_BODY:	return 3 * obj->value[type];
    case WEAR_HEAD:	return 2 * obj->value[type];
    case WEAR_LEGS:	return 2 * obj->value[type];
    case WEAR_FEET:	return     obj->value[type];
    case WEAR_HANDS:	return     obj->value[type];
    case WEAR_ARMS:	return     obj->value[type];
    case WEAR_SHIELD:	return     obj->value[type];
    case WEAR_NECK_1:	return     obj->value[type];
    case WEAR_NECK_2:	return     obj->value[type];
    case WEAR_ABOUT:	return 2 * obj->value[type];
    case WEAR_WAIST:	return     obj->value[type];
    case WEAR_WRIST_L:	return     obj->value[type];
    case WEAR_WRIST_R:	return     obj->value[type];
    case WEAR_HOLD:	return     obj->value[type];
    case WEAR_FLOAT:	return     obj->value[type];
    }

    return 0;
}



/*
 * Find a piece of eq on a character.
 */
OBJ_DATA *get_eq_char( CHAR_DATA *ch, int iWear )
{
    OBJ_DATA *obj;

    if (ch == NULL)
	return NULL;

    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
	if ( obj->wear_loc == iWear )
	    return obj;
    }

    return NULL;
}



/*
 * Equip a char with an obj.
 */
void equip_char( CHAR_DATA *ch, OBJ_DATA *obj, int iWear )
{
  char buf[MAX_STRING_LENGTH];
    AFFECT_DATA *paf;
    int i;
    int sn;
    char ** lines=0;
    char ** ar=0;

    if ( get_eq_char( ch, iWear ) != NULL )
    {
     sprintf(buf,"Equip_char: already equipped (%d) (%d) .", iWear, ch->pIndexData->vnum );
     bug(buf,0);
     
/*      bug( "Equip_char: already equipped (%d) (%d) .", iWear, ch->pIndexData->vnum ); */
        return;
    }

/*
    if ( get_eq_char( ch, iWear ) != NULL )
    {
     sprintf(buf,"HANDLER.C : EQUIP_CHAR - PRE-EQed [ CH: %s - WLOC: %d - OVNUM: %d ]  ref A012",
ch->name, iWear, obj->pIndexData->vnum );
     bug(buf,0);
	return;
    }
*/
    if ( ( IS_OBJ_STAT(obj, ITEM_ANTI_EVIL)    && IS_EVIL(ch)    )
    ||   ( IS_OBJ_STAT(obj, ITEM_ANTI_GOOD)    && IS_GOOD(ch)    )
    ||   ( IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(ch) ) )
    {
	/*
	 * Thanks to Morgenes for the bug fix here!
	 */
	act( "You are zapped by $p and drop it.", ch, obj, NULL, TO_CHAR );
	act( "$n is zapped by $p and drops it.",  ch, obj, NULL, TO_ROOM );
	obj_from_char( obj );
	obj_to_room( obj, ch->in_room );
	return;
    }

    for (i = 0; i < 4; i++)
    	ch->armor[i]      	-= apply_ac( obj, iWear,i );
    obj->wear_loc	 = iWear;


    if (!obj->enchanted)
	for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
	    if ( paf->location != APPLY_SPELL_AFFECT ) 
	        affect_modify( ch, paf, TRUE );
 
   for ( paf = obj->affected; paf != NULL; paf = paf->next )
	if ( paf->location == APPLY_SPELL_AFFECT )
     	    affect_to_char ( ch, paf );
	else 
          affect_modify( ch, paf, TRUE );


    if (obj->item_type == ITEM_LIGHT_SOURCE
    &&   obj->value[2] != 0
    &&   ch->in_room != NULL )
	++ch->in_room->light;
	
	if(GetObjExtraDesc(obj, "wearmessage"))
	   send_to_char(GetObjExtraDescText(GetObjExtraDesc(obj, "wearmessage")), ch);

	if(GetObjExtraDesc(obj, "=spells="))
	{
	    lines = strstrexplode(GetObjExtraDescText(GetObjExtraDesc(obj, "=spells=")), "\n\r");
	    for(ar=lines;ar && *ar;ar++)
	    {
	        sn = skill_lookup(*ar);
	        if(sn==-1)
	        {
	           sprintf(buf, "Invalid Spell on Object vnum %d spell %s char %s", obj->pIndexData->vnum, *ar, ch->name);
	           wiznet(buf, 0, 0, WIZ_ON, 0, 0);
	           continue;
	        }
	        
	        gInfiniteAffectDuration = TRUE;
	        (*skill_table[sn].spell_fun) ( sn, obj->level, ch, ch, TARGET_CHAR);
	        gInfiniteAffectDuration = FALSE;
	    }
	}

    return;
}



/*
 * Unequip a char with an obj.
 */
void unequip_char( CHAR_DATA *ch, OBJ_DATA *obj )
{
    AFFECT_DATA *paf = NULL;
    AFFECT_DATA *lpaf = NULL;
    AFFECT_DATA *lpaf_next = NULL;
	int i;
	char ** lines=0;
	char ** ar=0;
	int sn=0;
	char buf[MAX_STRING_LENGTH];

/* for new BUG
    char buf[MAX_STRING_LENGTH];

    if ( obj->wear_loc == WEAR_INVENTORY )
    {
   if (!IS_NPC(ch))
     {
     sprintf(buf,"HANDLER.C : UNEQUIP_CHAR - PRE-UNEQed [ CH: %s - OVNUM: %d ]  ref A014",
ch->name,obj->pIndexData->vnum );
     bug(buf,0);
	return;
     }
   else    
     {
     sprintf(buf,"HANDLER.C : UNEQUIP_CHAR - PRE-UNEQed [ CH: %d - OVNUM: %d ]  ref A014",
ch->pIndexData->vnum,obj->pIndexData->vnum );
     bug(buf,0);
	return;
     }
    }
*/

    if ( obj->wear_loc == WEAR_INVENTORY )
    {
        bug( "Unequip_char: already unequipped.", 0 );
        return;
    }

    for (i = 0; i < 4; i++)
    	ch->armor[i]	+= apply_ac( obj, obj->wear_loc,i );
    obj->wear_loc	 = -1;

    if (!obj->enchanted)
	{
	for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
	    if ( paf->location == APPLY_SPELL_AFFECT )
	    {
	        for ( lpaf = ch->affected; lpaf != NULL; lpaf = lpaf_next )
	        {
		    lpaf_next = lpaf->next;
		    if ((lpaf->type == paf->type) &&
		        (lpaf->level == paf->level) &&
		        (lpaf->location == APPLY_SPELL_AFFECT))
		    {
		        affect_remove( ch, lpaf );
			lpaf_next = NULL;
		    }
	        }
	    }
	    else
	    {
	        affect_modify( ch, paf, FALSE );
		affect_check(ch,paf->where,paf->bitvector);
	    }
	}

    for ( paf = obj->affected; paf != NULL; paf = paf->next )
	if ( paf->location == APPLY_SPELL_AFFECT )
	{
/*
     sprintf(buf,"HANDLER.C : NORM-APPLY - APPLY_SPELL_EFFECT [ CH: %s - OVNUM: %d ]  ref A015",
ch->name, obj->pIndexData->vnum );
     bug(buf,0);
*/
            bug ( "Norm-Apply: %d", 0 );
	    for ( lpaf = ch->affected; lpaf != NULL; lpaf = lpaf_next )
	    {
		lpaf_next = lpaf->next;
		if ((lpaf->type == paf->type) &&
		    (lpaf->level == paf->level) &&
		    (lpaf->location == APPLY_SPELL_AFFECT))
		{
/*
     sprintf(buf,"HANDLER.C : APPLY_SPELL_EFFECT [ LOC: %d - TYPE: %d ]  ref A015",
lpaf->location, lpaf->type );
     bug(buf,0);
*/

		    bug ( "location = %d", lpaf->location );
		    bug ( "type = %d", lpaf->type );
		    affect_remove( ch, lpaf );
		    lpaf_next = NULL;
		}
	    }
	}
	else
	{
	    affect_modify( ch, paf, FALSE );
	    affect_check(ch,paf->where,paf->bitvector);	
	}

    if ( obj->item_type == ITEM_LIGHT_SOURCE
    &&   obj->value[2] != 0
    &&   ch->in_room != NULL
    &&   ch->in_room->light > 0 )
	--ch->in_room->light;

	if(GetObjExtraDesc(obj, "removemessage"))
	   send_to_char(GetObjExtraDescText(GetObjExtraDesc(obj, "removemessage")), ch);

	if(GetObjExtraDesc(obj, "=spells="))
	{
	    lines = strstrexplode(GetObjExtraDescText(GetObjExtraDesc(obj, "=spells=")), "\n\r");
	    for(ar=lines;ar && *ar;ar++)
	    {
        	AFFECT_DATA *spaf;
        	AFFECT_DATA *spaf_next;
	    
	        sn = skill_lookup(*ar);
	        if(sn==-1)
	        {
	           sprintf(buf, "Invalid Spell on Object vnum %d spell %s char %s", obj->pIndexData->vnum, *ar, ch->name);
	           wiznet(buf, 0, 0, WIZ_ON, 0, 0);
	           continue;
	        }
	        
        	for(spaf=ch->affected;spaf;spaf=spaf_next)
        	{
        	    spaf_next = spaf->next;
        	    
        	    if(spaf->type==sn && spaf->level==obj->level && spaf->duration==-1)
        	    {
        	        if(skill_table[sn].msg_off)
        	        {
        	            sprintf(buf, "\n\r%s\n\r", skill_table[sn].msg_off);
            	        send_to_char(buf, ch);
            	    }
                    affect_remove(ch, spaf);
        	    }
        	}
	    }
	}

    return;
}



/*
 * Count occurrences of an obj in a list.
 */
int count_obj_list( OBJ_INDEX_DATA *pObjIndex, OBJ_DATA *list )
{
    OBJ_DATA *obj;
    int nMatch;

    nMatch = 0;
    for ( obj = list; obj != NULL; obj = obj->next_content )
    {
	if ( obj->pIndexData == pObjIndex )
	    nMatch++;
    }

    return nMatch;
}



/*
 * Move an obj out of a room.
 */
void obj_from_room( OBJ_DATA *obj )
{
    ROOM_INDEX_DATA *in_room;
    CHAR_DATA *ch;

/* for new BUG
    char buf[MAX_STRING_LENGTH];
*/

    if ( ( in_room = obj->in_room ) == NULL )
    {
/*
     sprintf(buf,"HANDLER.C : OBJ_FROM_ROOM - NULL [ OVNUM: %d - RVNUM: %d ]  ref A016", 
obj->pIndexData->vnum,obj->in_room->vnum );              
     bug(buf,0);
*/
	bug( "obj_from_room: NULL.", 0 ); 
	return;
    }

    for (ch = in_room->people; ch != NULL; ch = ch->next_in_room)
	if (ch->on == obj)
	    ch->on = NULL;

    if ( obj == in_room->contents )
    {
	in_room->contents = obj->next_content;
    }
    else
    {
	OBJ_DATA *prev;

	for ( prev = in_room->contents; prev; prev = prev->next_content )
	{
	    if ( prev->next_content == obj )
	    {
		prev->next_content = obj->next_content;
		break;
	    }
	}

	if ( prev == NULL )
	{
/*         bug("HANDLER.C : OBJ_FROM_ROOM - NO OBJ  ref A016",0 ); */

            bug( "Obj_from_room: obj not found.", 0 );
	    return;
	}
    }

    // Stones of Wisdom Stuff
    stones_object_leaving(obj);

    obj->in_room      = NULL;
    obj->next_content = NULL;
    return;
}



/*
 * Move an obj into a room.
 */
void obj_to_room( OBJ_DATA *obj, ROOM_INDEX_DATA *pRoomIndex )
{
    obj->next_content		= pRoomIndex->contents;
    pRoomIndex->contents	= obj;
    obj->in_room		= pRoomIndex;
    obj->carried_by		= NULL;
    obj->in_obj			= NULL;
    return;
}



/*
 * Move an object into an object.
 */
void obj_to_obj( OBJ_DATA *obj, OBJ_DATA *obj_to )
{
    obj->next_content		= obj_to->contains;
    obj_to->contains		= obj;
    obj->in_obj			= obj_to;
    obj->in_room		= NULL;
    obj->carried_by		= NULL;
    if (obj_to->pIndexData->vnum == OBJ_VNUM_PIT)
        obj->cost = 0; 

    for ( ; obj_to != NULL; obj_to = obj_to->in_obj )
    {
	if ( obj_to->carried_by != NULL )
	{
	    obj_to->carried_by->carry_number += get_obj_number( obj );
	    obj_to->carried_by->carry_weight += get_obj_weight( obj )
		* WEIGHT_MULT(obj_to) / 100;
	}
    }

    return;
}



/*
 * Move an object out of an object.
 */
void obj_from_obj( OBJ_DATA *obj )
{
    OBJ_DATA *obj_from;

    if ( ( obj_from = obj->in_obj ) == NULL )
    {

	bug( "Obj_from_obj: null obj_from.", 0 );
	return;
    }

    if ( obj == obj_from->contains )
    {
	obj_from->contains = obj->next_content;
    }
    else
    {
	OBJ_DATA *prev;

	for ( prev = obj_from->contains; prev; prev = prev->next_content )
	{
	    if ( prev->next_content == obj )
	    {
		prev->next_content = obj->next_content;
		break;
	    }
	}

	if ( prev == NULL )
	{
	    bug( "Obj_from_obj: obj not found.", 0 );
	    return;
	}
    }

    obj->next_content = NULL;
    obj->in_obj       = NULL;

    for ( ; obj_from != NULL; obj_from = obj_from->in_obj )
    {
	if ( obj_from->carried_by != NULL )
	{
	    obj_from->carried_by->carry_number -= get_obj_number( obj );
	    obj_from->carried_by->carry_weight -= get_obj_weight( obj ) 
		* WEIGHT_MULT(obj_from) / 100;
	}
    }

    return;
}



/*
 * Extract an obj from the world.
 */
void extract_obj( OBJ_DATA *obj )
{
    OBJ_DATA *obj_content;
    OBJ_DATA *obj_next;

    if ( obj->in_room != NULL )
	obj_from_room( obj );
    else if ( obj->carried_by != NULL )
	obj_from_char( obj );
    else if ( obj->in_obj != NULL )
	obj_from_obj( obj );

    for ( obj_content = obj->contains; obj_content; obj_content = obj_next )
    {
	obj_next = obj_content->next_content;
	extract_obj( obj_content );
    }

    if ( object_list == obj )
    {
	object_list = obj->next;
    }
    else
    {
	OBJ_DATA *prev;

	for ( prev = object_list; prev != NULL; prev = prev->next )
	{
	    if ( prev->next == obj )
	    {
		prev->next = obj->next;
		break;
	    }
	}

	if ( prev == NULL )
	{
	    bug( "Extract_obj: obj %d not found.", obj->pIndexData->vnum );
	    return;
	}
    }

    // Stones of Wisdom Stuff
    stones_object_leaving(obj);

    --obj->pIndexData->count;
    free_obj(obj);
    return;
}



/*
 * Extract a char from the world.
 */
void extract_char( CHAR_DATA *ch, bool fPull )
{
  CHAR_DATA *wch;
  OBJ_DATA *obj;
  OBJ_DATA *obj_next;

    // Stones of Wisdom Stuff
    stones_player_quit(ch);

  nuke_pets(ch);
  ch->pet = NULL; /* just in case */

  if ( fPull )
    die_follower( ch );
    
  stop_fighting( ch, TRUE );

 
  for ( obj = ch->carrying; obj != NULL; obj = obj_next )
    {
    if (IS_SET(ch->in_room->room_flags, ROOM_DRAGONPIT))
       break;
    obj_next = obj->next_content;
    extract_obj( obj );
    }
    
  if (ch->in_room != NULL)
    char_from_room( ch );

  if ( !fPull )
    {
    if (IS_SET(ch->pact, PLR_DRAGONPIT)) 
      {
      REMOVE_BIT(ch->pact, PLR_DRAGONPIT);           
      char_to_room(ch,get_room_index(ROOM_VNUM_DRAGONPIT_RETURN));
      do_restore(ch,"self");
    do_look(ch, "auto");
      return;
      }
    else
      {
      char_to_room(ch,get_room_index(ROOM_VNUM_ALTAR));
      if(ch->pcdata->legending==FALSE)
        do_look(ch, "auto");
      return;
      }
    }

  if ( IS_NPC(ch) )
    --ch->pIndexData->count;

  if ( ch->desc != NULL && ch->desc->original != NULL )
    {
    do_function(ch, &do_return, "" );
    ch->desc = NULL;
    }

  for ( wch = char_list; wch != NULL; wch = wch->next )
    {
    if ( wch->reply == ch )
      wch->reply = NULL;
    if ( ch->mprog_target == wch )
      wch->mprog_target = NULL;
    }

  if ( ch == char_list )
    char_list = ch->next;
  else
    {
    CHAR_DATA *prev;
    for ( prev = char_list; prev != NULL; prev = prev->next )
      {
	    if ( prev->next == ch )
        {
        prev->next = ch->next;
        break;
        }
      }
    if ( prev == NULL )
      {
	    bug( "Extract_char: char not found.", 0 );
	    return;
      }
    }

  if ( ch->desc != NULL )
    ch->desc->character = NULL;
  free_char( ch );
  return;
}



/*
 * Find a char in the room.
 */
CHAR_DATA *get_char_room( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *rch;
    int number;
    int count;

    number = number_argument( argument, arg );
    count  = 0;
    if ( !str_cmp( arg, "self" ) )
	return ch;
    for ( rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room )
    {
	if ( !can_see( ch, rch ) || !is_name( arg, rch->name ) )
	    continue;
	if ( ++count == number )
	    return rch;
    }

    return NULL;
}




/*
 * Find a char in the world.
 */
CHAR_DATA *get_char_world( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *wch;
    int number;
    int count;

    if ( ( wch = get_char_room( ch, argument ) ) != NULL )
	return wch;

    number = number_argument( argument, arg );
    count  = 0;
    for ( wch = char_list; wch != NULL ; wch = wch->next )
    {
	if ( wch->in_room == NULL || !can_see( ch, wch ) 
	||   !is_name( arg, wch->name ) )
	    continue;
	if ( ++count == number )
	    return wch;
    }

    return NULL;
}



/*
 * Find some object with a given index data.
 * Used by area-reset 'P' command.
 */
OBJ_DATA *get_obj_type( OBJ_INDEX_DATA *pObjIndex )
{
    OBJ_DATA *obj;

    for ( obj = object_list; obj != NULL; obj = obj->next )
    {
	if ( obj->pIndexData == pObjIndex )
	    return obj;
    }

    return NULL;
}


/*
 * Find an obj in a list.
 */
OBJ_DATA *get_obj_list( CHAR_DATA *ch, char *argument, OBJ_DATA *list )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int number;
    int count;

    number = number_argument( argument, arg );
    count  = 0;
    for ( obj = list; obj != NULL; obj = obj->next_content )
    {
	if ( can_see_obj( ch, obj ) && is_name( arg, obj->name ) )
	{
	    if ( ++count == number )
		return obj;
	}
    }

    return NULL;
}



/*
 * Find an obj in player's inventory.
 */
OBJ_DATA *get_obj_carry( CHAR_DATA *ch, char *argument, CHAR_DATA *viewer )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int number;
    int count;

    number = number_argument( argument, arg );
    count  = 0;
    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
	if ( obj->wear_loc == WEAR_INVENTORY
	&&   (can_see_obj( viewer, obj ) ) 
	&&   is_name( arg, obj->name ) )
	{
	    if ( ++count == number )
		return obj;
	}
    }

    return NULL;
}



/*
 * Find an obj in player's equipment.
 */
OBJ_DATA *get_obj_wear( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int number;
    int count;

    number = number_argument( argument, arg );
    count  = 0;
    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
	if ( obj->wear_loc != WEAR_INVENTORY
	&&   can_see_obj( ch, obj )
	&&   is_name( arg, obj->name ) )
	{
	    if ( ++count == number )
		return obj;
	}
    }

    return NULL;
}


OBJ_DATA *get_obj_here_me_first( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;

    if ( ( obj = get_obj_carry( ch, argument, ch ) ) != NULL )
	return obj;

    obj = get_obj_list( ch, argument, ch->in_room->contents );
    if ( obj != NULL )
	return obj;

    return NULL;
}



/*
 * Find an obj in the room or in inventory.
 */
OBJ_DATA *get_obj_here( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;

    obj = get_obj_list( ch, argument, ch->in_room->contents );
    if ( obj != NULL )
	return obj;

    if ( ( obj = get_obj_carry( ch, argument, ch ) ) != NULL )
	return obj;

    if ( ( obj = get_obj_wear( ch, argument ) ) != NULL )
	return obj;

    return NULL;
}



/*
 * Find an obj in the world.
 */
OBJ_DATA *get_obj_world( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int number;
    int count;

    if ( ( obj = get_obj_here( ch, argument ) ) != NULL )
	return obj;

    number = number_argument( argument, arg );
    count  = 0;
    for ( obj = object_list; obj != NULL; obj = obj->next )
    {
	if ( can_see_obj( ch, obj ) && is_name( arg, obj->name ) )
	{
	    if ( ++count == number )
		return obj;
	}
    }

    return NULL;
}

/* deduct cost from a character */

void deduct_cost(CHAR_DATA *ch, int cost)
{
    int silver = 0, gold = 0;

    silver = UMIN(ch->silver,cost); 

    if (silver < cost)
    {
	gold = ((cost - silver + 99) / 100);
	silver = cost - 100 * gold;
    }

    ch->gold -= gold;
    ch->silver -= silver;

    if (ch->gold < 0)
    {
	bug("deduct costs: gold %d < 0",ch->gold);
	ch->gold = 0;
    }
    if (ch->silver < 0)
    {
	bug("deduct costs: silver %d < 0",ch->silver);
	ch->silver = 0;
    }
}   
/*
 * Create a 'money' obj.
 */
OBJ_DATA *create_money( int gold, int silver )
{
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;

    if ( gold < 0 || silver < 0 || (gold == 0 && silver == 0) )
    {
	bug( "Create_money: zero or negative money.",UMIN(gold,silver));
	gold = UMAX(1,gold);
	silver = UMAX(1,silver);
    }

    if (gold == 0 && silver == 1)
    {
	obj = create_object( get_obj_index( OBJ_VNUM_SILVER_ONE ), 0 );
    }
    else if (gold == 1 && silver == 0)
    {
	obj = create_object( get_obj_index( OBJ_VNUM_GOLD_ONE), 0 );
    }
    else if (silver == 0)
    {
        obj = create_object( get_obj_index( OBJ_VNUM_GOLD_SOME ), 0 );
        sprintf( buf, obj->short_descr, gold );
        free_string( obj->short_descr );
        obj->short_descr        = str_dup( buf );
        obj->value[1]           = gold;
        obj->cost               = gold;
	obj->weight		= gold/5;
    }
    else if (gold == 0)
    {
        obj = create_object( get_obj_index( OBJ_VNUM_SILVER_SOME ), 0 );
        sprintf( buf, obj->short_descr, silver );
        free_string( obj->short_descr );
        obj->short_descr        = str_dup( buf );
        obj->value[0]           = silver;
        obj->cost               = silver;
	obj->weight		= silver/20;
    }
 
    else
    {
	obj = create_object( get_obj_index( OBJ_VNUM_COINS ), 0 );
	sprintf( buf, obj->short_descr, silver, gold );
	free_string( obj->short_descr );
	obj->short_descr	= str_dup( buf );
	obj->value[0]		= silver;
	obj->value[1]		= gold;
	obj->cost		= 100 * gold + silver;
	obj->weight		= gold / 5 + silver / 20;
    }

    return obj;
}



/*
 * Return # of objects which an object counts as.
 * Thanks to Tony Chamberlain for the correct recursive code here.
 */
int get_obj_number( OBJ_DATA *obj )
{
    int number;
 
    if (obj->item_type == ITEM_CONTAINER || obj->item_type == ITEM_MONEY
    ||  obj->item_type == ITEM_GEM || obj->item_type == ITEM_JEWELRY)
        number = 0;
    else
        number = 1;
 
    for ( obj = obj->contains; obj != NULL; obj = obj->next_content )
        number += get_obj_number( obj );
 
    return number;
}


/*
 * Return weight of an object, including weight of contents.
 */
int get_obj_weight( OBJ_DATA *obj )
{
    int weight;
    OBJ_DATA *tobj;

    weight = obj->weight;
    for ( tobj = obj->contains; tobj != NULL; tobj = tobj->next_content )
	weight += get_obj_weight( tobj ) * WEIGHT_MULT(obj) / 100;

    return weight;
}

int get_true_weight(OBJ_DATA *obj)
{
    int weight;
 
    weight = obj->weight;
    for ( obj = obj->contains; obj != NULL; obj = obj->next_content )
        weight += get_obj_weight( obj );
 
    return weight;
}

/*
 * True if room is dark.
 */
bool room_is_dark( ROOM_INDEX_DATA *pRoomIndex )
{
    if ( pRoomIndex->light > 0 )
	return FALSE;

    if ( IS_SET(pRoomIndex->room_flags, ROOM_DARK) )
	return TRUE;

    if ( pRoomIndex->sector_type == SECT_INSIDE
    ||   pRoomIndex->sector_type == SECT_CITY )
	return FALSE;

    if ( weather_info.sunlight == SUN_SET
    ||   weather_info.sunlight == SUN_DARK )
	return TRUE;

    return FALSE;
}


bool is_room_owner(CHAR_DATA *ch, ROOM_INDEX_DATA *room)
{
    if (room->owner == NULL || room->owner[0] == '\0')
	return FALSE;

    return is_name(ch->name,room->owner);
}

/*
 * True if room is private.
 */
bool room_is_private( ROOM_INDEX_DATA *pRoomIndex )
{
    CHAR_DATA *rch;
    int count;


    if (pRoomIndex->owner != NULL && pRoomIndex->owner[0] != '\0')
	return TRUE;

    count = 0;
    for ( rch = pRoomIndex->people; rch != NULL; rch = rch->next_in_room )
	count++;

    if ( IS_SET(pRoomIndex->room_flags, ROOM_PRIVATE)  && count >= 2 )
	return TRUE;

    if ( IS_SET(pRoomIndex->room_flags, ROOM_SOLITARY) && count >= 1 )
	return TRUE;
    
    if ( IS_SET(pRoomIndex->room_flags, ROOM_IMP_ONLY) )
	return TRUE;

    return FALSE;
}

/* visibility on a room -- for entering and exits */
bool can_see_room( CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex )
{
    if (IS_SET(pRoomIndex->room_flags, ROOM_IMP_ONLY) 
    &&  (!IS_IMP(ch)) )
	return FALSE;

    if (IS_SET(pRoomIndex->room_flags, ROOM_GODS_ONLY)
    &&  !IS_IMMORTAL(ch))
	return FALSE;

    if (IS_SET(pRoomIndex->room_flags, ROOM_HEROES_ONLY)
    &&  !IS_IMMORTAL(ch))
	return FALSE;

    if (IS_SET(pRoomIndex->room_flags,ROOM_NEWBIES_ONLY)
    &&  ch->level > 10 && !IS_IMMORTAL(ch))
	return FALSE;

    if (!IS_IMMORTAL(ch) && pRoomIndex->clanowner && ch->clan != pRoomIndex->clanowner)
        return FALSE;

    return TRUE;
}



/*
 * True if char can see victim.
 */
bool can_see( CHAR_DATA *ch, CHAR_DATA *victim )
{
/* RT changed so that WIZ_INVIS has levels */
    if ( ch == victim )
    return TRUE;
    if(!IS_NPC(victim) && victim->pcdata && victim->pcdata->gbuffered)
    return FALSE;        
    if (!is_owner(ch))
      {
       if ( ch->level < victim->invis_level)
	return FALSE;
      }
    else
     return TRUE;

    if (ch->level < victim->incog_level && ch->in_room != victim->in_room)
	return FALSE;

    if ( (!IS_NPC(ch) && IS_SET(ch->pact, PLR_HOLYLIGHT)) 
    ||   (IS_NPC(ch) && IS_IMMORTAL(ch)))
	return TRUE;

    if ( IS_AFFECTED(ch, AFF_BLIND) )
	return FALSE;

    if (  (ch->in_room && room_is_dark( ch->in_room )) && !IS_AFFECTED(ch, AFF_INFRARED) )
	return FALSE;

    if ( IS_AFFECTED(victim, AFF_INVISIBLE)
    &&   !IS_AFFECTED(ch, AFF_DETECT_INVIS) )
	return FALSE;

    /* sneaking */
    if ( IS_AFFECTED(victim, AFF_SNEAK)
    &&   !IS_AFFECTED(ch,AFF_DETECT_HIDDEN)
    &&   victim->fighting == NULL)
    {
	int chance;
	chance = get_skill(victim,gsn_sneak);
	chance += get_curr_stat(victim,STAT_DEX) * 3/2;
 	chance -= get_curr_stat(ch,STAT_INT) * 2;
	chance -= ch->level - victim->level * 3/2;

	if (number_percent() < chance)
	    return FALSE;
    }

    if ( IS_AFFECTED(victim, AFF_HIDE)
    &&   !IS_AFFECTED(ch, AFF_DETECT_HIDDEN)
    &&   victim->fighting == NULL)
	return FALSE;

    return TRUE;
}



/*
 * True if char can see obj.
 */
bool can_see_obj( CHAR_DATA *ch, OBJ_DATA *obj )
{
    if(IS_IMMORTAL(ch))
        return TRUE;
        
    if(IS_OBJ_STAT(obj, ITEM_LEGEND) && (!IS_LEGEND(ch) || ch->level<obj->level))
        return FALSE;

    if ( !IS_NPC(ch) && IS_SET(ch->pact, PLR_HOLYLIGHT) )
	return TRUE;

    if ( IS_SET(obj->extra_flags,ITEM_VIS_DEATH))
	return FALSE;

    if ( IS_OBJ_STAT(obj, ITEM_HUM))
        return TRUE;

    if ( IS_AFFECTED( ch, AFF_BLIND ) && obj->item_type != ITEM_POTION)
	return FALSE;

    if ( obj->item_type == ITEM_LIGHT_SOURCE && obj->value[2] != 0 )
	return TRUE;

    if ( (IS_SET(obj->extra_flags, ITEM_INVIS))
    && ( !IS_AFFECTED(ch, AFF_DETECT_INVIS)
    &&   !IS_SET(ch->in_room->room_flags, ROOM_SEE_INVIS)))
        return FALSE;

    if ( IS_OBJ_STAT(obj,ITEM_GLOW))
	return TRUE;

    if ( room_is_dark( ch->in_room ) && !IS_AFFECTED(ch, AFF_DARK_VISION) )
	return FALSE;

    return TRUE;
}



/*
 * True if char can drop obj.
 */
bool can_drop_obj( CHAR_DATA *ch, OBJ_DATA *obj )
{
    if ( !IS_SET(obj->extra_flags, ITEM_NODROP) )
	return TRUE;

    if ( !IS_NPC(ch) && ch->level >= LEVEL_IMMORTAL )
	return TRUE;

	if(GetObjExtraDesc(obj, "=joinedwith="))
	   return FALSE;
	
    return FALSE;
}


/*
 * Return ascii name of an affect location.
 */
char *affect_loc_name( int location )
{
  
    switch ( location )
    {
    case APPLY_NONE:		return "NONE";
    case APPLY_STR:		return "STR";
    case APPLY_DEX:		return "DEX";
    case APPLY_INT:		return "INT";
    case APPLY_WIS:		return "WIS";
    case APPLY_CON:		return "CON";
    case APPLY_SEX:		return "SEX";
    case APPLY_CLASS:		return "NONE";
    case APPLY_LEVEL:		return "NONE";
    case APPLY_AGE:		return "NONE";
    case APPLY_MANA:		return "Mana";
    case APPLY_HIT:		return "HP";
    case APPLY_MOVE:		return "Moves";
    case APPLY_GOLD:		return "NONE";
    case APPLY_EXP:		return "NONE";
    case APPLY_AC:		return "Armor";
    case APPLY_HITROLL:		return "HITRoll";
    case APPLY_DAMROLL:		return "DAMRoll";
    case APPLY_SAVES:		return "Saves";
    case APPLY_SAVING_ROD:	return "NONE";
    case APPLY_SAVING_PETRI:	return "NONE";
    case APPLY_SAVING_BREATH:	return "NONE";
    case APPLY_SAVING_SPELL:	return "NONE";
    case APPLY_SPELL_AFFECT:	return "NONE";
    }

    bug( "AFF LOC NAME: Unknown LOC %d.", location );
    return "NONE";
}

char *affect_bit_name( int vector )
{
    static char buf[512];

    buf[0] = '\0';
    if ( vector & AFF_BLIND         ) strcat( buf, " Blind"         );
    if ( vector & AFF_INVISIBLE     ) strcat( buf, " Invis"         );
    if ( vector & AFF_DETECT_EVIL   ) strcat( buf, " DetEvil"   );
    if ( vector & AFF_DETECT_INVIS  ) strcat( buf, " DetInvis"  );
    if ( vector & AFF_DETECT_MAGIC  ) strcat( buf, " DetMagic"  );
    if ( vector & AFF_DETECT_HIDDEN ) strcat( buf, " DetHide" );
    if ( vector & AFF_DETECT_GOOD   ) strcat( buf, " DetGood"   );
    if ( vector & AFF_SANCTUARY     ) strcat( buf, " SANC"     );
    if ( vector & AFF_FAERIE_FIRE   ) strcat( buf, " FaeFire"   );
    if ( vector & AFF_INFRARED      ) strcat( buf, " Infra"      );
    if ( vector & AFF_CURSE         ) strcat( buf, " Curse"         );
    if ( vector & AFF_POISON        ) strcat( buf, " Poison"        );
    if ( vector & AFF_PROTECT_EVIL  ) strcat( buf, " ProEvil"     );
    if ( vector & AFF_PROTECT_GOOD  ) strcat( buf, " ProGood"     );
    if ( vector & AFF_SNEAK         ) strcat( buf, " Sneak"         );
    if ( vector & AFF_HIDE          ) strcat( buf, " Hide"          );
    if ( vector & AFF_SLEEP         ) strcat( buf, " Sleep"         );
    if ( vector & AFF_CHARM         ) strcat( buf, " Charm"         );
    if ( vector & AFF_FLYING        ) strcat( buf, " Fly"        );
    if ( vector & AFF_PASS_DOOR     ) strcat( buf, " PDoor"     );
    if ( vector & AFF_HASTE	    ) strcat( buf, " Haste"	    );
    if ( vector & AFF_CALM	    ) strcat( buf, " Calm"	    );
    if ( vector & AFF_PLAGUE	    ) strcat( buf, " Plague" 	    );
    if ( vector & AFF_WEAKEN	    ) strcat( buf, " Weak" 	    );
    if ( vector & AFF_DARK_VISION   ) strcat( buf, " DVision"   );
    if ( vector & AFF_BERSERK	    ) strcat( buf, " BSerk"	    );
    if ( vector & AFF_SWIM          ) strcat( buf, " Swim"          );
    if ( vector & AFF_REGENERATION  ) strcat( buf, " Regen"          );
    if ( vector & AFF_SLOW          ) strcat( buf, " Slow"          );
    if ( vector & AFF_GOLDEN        ) strcat( buf, " GolAURA"   );
    return ( buf[0] != '\0' ) ? buf+1 : "NONE";
}


/*
 * See if a string is one of the names of an object.
 */

bool is_full_name( const char *str, char *namelist )
{
    char name[MAX_INPUT_LENGTH];

    for ( ; ; )
    {
        namelist = one_argument( namelist, name );
        if ( name[0] == '\0' )
            return FALSE;
        if ( !str_cmp( str, name ) )
            return TRUE;
    }
}


char *affect2_bit_name( int vector )
{
    static char buf[512];

    buf[0] = '\0';
    if ( vector & AFF2_DIVINE         ) strcat( buf, " Divine"         );
    return ( buf[0] != '\0' ) ? buf+1 : "NONE";
}


/*
 * Return ascii name of extra flags vector.
 */
char *extra_bit_name( int extra_flags )
{
    static char buf[512];

    buf[0] = '\0';
    if ( extra_flags & ITEM_GLOW         ) strcat( buf, " Glow"         );
    if ( extra_flags & ITEM_HUM          ) strcat( buf, " HUM"          );
    if ( extra_flags & ITEM_EVIL         ) strcat( buf, " Evil"         );
    if ( extra_flags & ITEM_INVIS        ) strcat( buf, " Invis"        );
    if ( extra_flags & ITEM_MAGIC        ) strcat( buf, " Magic"        );
    if ( extra_flags & ITEM_NODROP       ) strcat( buf, " NoDrop"       );
    if ( extra_flags & ITEM_BLESS        ) strcat( buf, " Bless"        );
    if ( extra_flags & ITEM_ANTI_GOOD    ) strcat( buf, " AGood"        );
    if ( extra_flags & ITEM_ANTI_EVIL    ) strcat( buf, " AEvil"    	);
    if ( extra_flags & ITEM_ANTI_NEUTRAL ) strcat( buf, " ANeut"	);
    if ( extra_flags & ITEM_NOREMOVE     ) strcat( buf, " NoRem"     	);
    if ( extra_flags & ITEM_INVENTORY    ) strcat( buf, " Inven"    	);
    if ( extra_flags & ITEM_NOPURGE	 ) strcat( buf, " NoPurge"	);
    if ( extra_flags & ITEM_ROT_DEATH	 ) strcat( buf, " RotDeath"	);
    if ( extra_flags & ITEM_VIS_DEATH	 ) strcat( buf, " VisDeath"	);
    if ( extra_flags & ITEM_FRAGILE	 ) strcat( buf, " Frag"		);
    if ( extra_flags & ITEM_NONMETAL	 ) strcat( buf, " NONMetal"	);
    if ( extra_flags & ITEM_NOLOCATE	 ) strcat( buf, " NoLocate"	);
    if ( extra_flags & ITEM_MELT_DROP    ) strcat( buf, " MeltDrop"	);
    if ( extra_flags & ITEM_HAD_TIMER	 ) strcat( buf, " HadTimer"	);
    if ( extra_flags & ITEM_SELL_EXTRACT ) strcat( buf, " SellExt"	);
    if ( extra_flags & ITEM_FIREPROOF	 ) strcat( buf, " FireProof"	);
    if ( extra_flags & ITEM_NOUNCURSE	 ) strcat( buf, " NoUncurse"	);
    if ( extra_flags & ITEM_CLAN_EQ	 ) strcat( buf, " ClanEQ"	);
    if ( extra_flags & ITEM_SPECIAL_KEY	 ) strcat( buf, " SpecKEY"	);
    if ( extra_flags & ITEM_FORTIFIED	 ) strcat( buf, " FORTIFIED"	);
    if ( extra_flags & ITEM_LEGEND	 ) strcat( buf, " Legend"	);
    // Stones of Wisdom Stuff
    if ( extra_flags & ITEM_DICE	 ) strcat( buf, " Dice"	);
    return ( buf[0] != '\0' ) ? buf+1 : "NONE";
}

char *area_bit_name( int area_flags )
{
    static char buf[512];

    buf[0] = '\0';
    if ( area_flags & AREA_CHANGED	 ) strcat( buf, " CHANGED"	);
    if ( area_flags & AREA_ADDED	 ) strcat( buf, " ADDED"	);
    if ( area_flags & AREA_LOADING       ) strcat( buf, " LOADING"	);
    if ( area_flags & AREA_ENVY 	 ) strcat( buf, " ENVY"		);
    if ( area_flags & AREA_PROTO	 ) strcat( buf, " PROTO"	);
    if ( area_flags & AREA_PLAYER	 ) strcat( buf, " PLAYER"	);
    if ( area_flags & AREA_GODS		 ) strcat( buf, " GODS"		);
    if ( area_flags & AREA_NOTRANS	 ) strcat( buf, " NOTRANS"	);
    if ( area_flags & AREA_NOIMM	 ) strcat( buf, " NOIMM"	);
    if ( area_flags & AREA_IMP		 ) strcat( buf, " IMP"		);
    return ( buf[0] != '\0' ) ? buf+1 : "NONE";
}



/* return ascii name of an act vector */
char *act_bit_name( int act_flags )
{
    static char buf[512];

    buf[0] = '\0';

    if (IS_SET(act_flags,ACT_IS_NPC))
    { 
 	strcat(buf," NPC");
    	if (act_flags & ACT_SENTINEL 	) strcat(buf, " Sentinel");
    	if (act_flags & ACT_SCAVENGER	) strcat(buf, " Scavenger");
	if (act_flags & ACT_BANKER	) strcat(buf, " Bank");
	if (act_flags & ACT_FORGER      ) strcat(buf, " Forge");
	if (act_flags & ACT_AGGRESSIVE	) strcat(buf, " Aggr");
	if (act_flags & ACT_STAY_AREA	) strcat(buf, " Stay");
	if (act_flags & ACT_WIMPY	) strcat(buf, " Wimp");
	if (act_flags & ACT_PET		) strcat(buf, " Pet");
	if (act_flags & ACT_TRAIN	) strcat(buf, " Train");
	if (act_flags & ACT_PRACTICE	) strcat(buf, " Prac");
	if (act_flags & ACT_PALADIN	) strcat(buf, " Paladin");
	if (act_flags & ACT_ASSASSIN	) strcat(buf, " Assassin");
	if (act_flags & ACT_ANTIPALADIN	) strcat(buf, " Anti-Pal");
	if (act_flags & ACT_UNDEAD	) strcat(buf, " Undead");
	if (act_flags & ACT_PSIONIC	) strcat(buf, " Psion");
	if (act_flags & ACT_CLERIC	) strcat(buf, " Cleric");
	if (act_flags & ACT_MAGE	) strcat(buf, " Mage");
	if (act_flags & ACT_THIEF	) strcat(buf, " Thief");
	if (act_flags & ACT_WARRIOR	) strcat(buf, " Warrior");
	if (act_flags & ACT_NOALIGN	) strcat(buf, " NoAlign");
	if (act_flags & ACT_NOPURGE	) strcat(buf, " NoPurge");
	if (act_flags & ACT_QUESTMASTER ) strcat(buf, " QMaster");
	if (act_flags & ACT_NOBINGO	) strcat(buf, " NoBingo");
	if (act_flags & ACT_IS_HEALER	) strcat(buf, " Healer");
	if (act_flags & ACT_GAIN	) strcat(buf, " Gain");
	if (act_flags & ACT_RANGER ) 	  strcat(buf," Ranger");
	if (act_flags & ACT_NOSPECATTK )  strcat(buf," NoSpecAttk");
    }
    return ( buf[0] != '\0' ) ? buf+1 : "NONE";
}


char *pact_bit_name( int plr_flags )
{
    static char buf[512];

    buf[0] = '\0';

    {
	strcat(buf," PC");
	if (plr_flags & PLR_QUESTOR	) strcat(buf, " Quest");
	if (plr_flags & PLR_AUTOASSIST	) strcat(buf, " AAssist");
	if (plr_flags & PLR_AUTOEXIT	) strcat(buf, " AExit");
	if (plr_flags & PLR_AUTOLOOT	) strcat(buf, " ALoot");
	if (plr_flags & PLR_AUTOSAC	) strcat(buf, " ASac");
	if (plr_flags & PLR_AUTOGOLD	) strcat(buf, " AGold");
	if (plr_flags & PLR_AUTOSPLIT	) strcat(buf, " ASplit");
	if (plr_flags & PLR_TARGET	) strcat(buf, " Target");
	if (plr_flags & PLR_DRAGONPIT	) strcat(buf, " DragonPIT");
	if (plr_flags & PLR_CANLOOT	) strcat(buf, " Loot");
	if (plr_flags & PLR_NOSUMMON	) strcat(buf, " NoSumm");
	if (plr_flags & PLR_NOFOLLOW	) strcat(buf, " NoFol");
	if (plr_flags & PLR_PERMIT	) strcat(buf, " Permit");
	if (plr_flags & PLR_JAIL	) strcat(buf, " Jail");
	if (plr_flags & PLR_LOG		) strcat(buf, " Log");
	if (plr_flags & PLR_DENY	) strcat(buf, " Deny");
	if (plr_flags & PLR_FREEZE	) strcat(buf, " Froze");
	if (plr_flags & PLR_THIEF	) strcat(buf, " Thief");
	if (plr_flags & PLR_REMORT	) strcat(buf, " Remort");
	if (plr_flags & PLR_PKILLER	) strcat(buf, " PKiller");
	if (plr_flags & PLR_CLAN_LEADER	) strcat(buf, " CLeader");
    }
    return ( buf[0] != '\0' ) ? buf+1 : "NONE";
}


char *comm_bit_name(int comm_flags)
{
    static char buf[512];

    buf[0] = '\0';

    if (comm_flags & COMM_QUIET		) strcat(buf, " Quiet");
    if (comm_flags & COMM_DEAF		) strcat(buf, " Deaf");
    if (comm_flags & COMM_NOWIZ		) strcat(buf, " NoWiz");
    if (comm_flags & COMM_NOAUCTION	) strcat(buf, " NoAuct");
    if (comm_flags & COMM_DPITKILL	) strcat(buf, " NODPit");
    if (comm_flags & COMM_NOPNOTE 	) strcat(buf, " NoPNote");
    if (comm_flags & COMM_COMBAT	) strcat(buf, " Combat");
    if (comm_flags & COMM_HELPER	) strcat(buf, " Helper");
    if (comm_flags & COMM_NOEMOTE	) strcat(buf, " NoEmote");
    if (comm_flags & COMM_NOTELL	) strcat(buf, " NoTell");
    if (comm_flags & COMM_NOCHANNELS	) strcat(buf, " NoChan");
    if (comm_flags & COMM_SNOOP_PROOF	) strcat(buf, " SnoopPr");
    if (comm_flags & COMM_AFK		) strcat(buf, " AFK");
    if (comm_flags & COMM_ICON		) strcat(buf, " ICONed");
    if (comm_flags & COMM_NOTITLE 	) strcat(buf, " NoTitle");

    return ( buf[0] != '\0' ) ? buf+1 : "NONE";
}

char *imm_bit_name(int imm_flags)
{
    static char buf[512];

    buf[0] = '\0';

    if (imm_flags & IMM_SUMMON		) strcat(buf, " Summ");
    if (imm_flags & IMM_CHARM		) strcat(buf, " Charm");
    if (imm_flags & IMM_MAGIC		) strcat(buf, " Magic");
    if (imm_flags & IMM_WEAPON		) strcat(buf, " Weap");
    if (imm_flags & IMM_BASH		) strcat(buf, " Bash");
    if (imm_flags & IMM_PIERCE		) strcat(buf, " Pierce");
    if (imm_flags & IMM_SLASH		) strcat(buf, " Slash");
    if (imm_flags & IMM_FIRE		) strcat(buf, " Fire");
    if (imm_flags & IMM_COLD		) strcat(buf, " Cold");
    if (imm_flags & IMM_LIGHTNING	) strcat(buf, " Lghtng");
    if (imm_flags & IMM_ACID		) strcat(buf, " Acid");
    if (imm_flags & IMM_POISON		) strcat(buf, " Poison");
    if (imm_flags & IMM_NEGATIVE	) strcat(buf, " Nega");
    if (imm_flags & IMM_HOLY		) strcat(buf, " Holy");
    if (imm_flags & IMM_ENERGY		) strcat(buf, " Energy");
    if (imm_flags & IMM_MENTAL		) strcat(buf, " Mental");
    if (imm_flags & IMM_DISEASE		) strcat(buf, " Disease");
    if (imm_flags & IMM_DROWNING	) strcat(buf, " Drown");
    if (imm_flags & IMM_LIGHT2		) strcat(buf, " Light2");
    if (imm_flags & IMM_SOUND		) strcat(buf, " Sound");
    if (imm_flags & IMM_ELEMENTAL	) strcat(buf, " Elemental");
    if (imm_flags & IMM_WOOD		) strcat(buf, " Wood");
    if (imm_flags & IMM_SILVER		) strcat(buf, " Silver");
    if (imm_flags & IMM_IRON		) strcat(buf, " Iron");
    if (imm_flags & IMM_MAIM		) strcat(buf, " Maim");
    if (imm_flags & IMM_CRUSH		) strcat(buf, " Crush");
    if (imm_flags & IMM_CLEAVE		) strcat(buf, " Cleave");
    if (imm_flags & IMM_PUNCH		) strcat(buf, " Punch");
    if (imm_flags & IMM_BITE		) strcat(buf, " Bite");
    if (imm_flags & IMM_CLAW		) strcat(buf, " Claw");

    return ( buf[0] != '\0' ) ? buf+1 : "NONE";
}

char *res_bit_name(int res_flags)
{
    static char buf[512];

    buf[0] = '\0';

    if (res_flags & RES_SUMMON		) strcat(buf, " Summ");
    if (res_flags & RES_CHARM		) strcat(buf, " Charm");
    if (res_flags & RES_MAGIC		) strcat(buf, " Magic");
    if (res_flags & RES_WEAPON		) strcat(buf, " Weap");
    if (res_flags & RES_BASH		) strcat(buf, " Bash");
    if (res_flags & RES_PIERCE		) strcat(buf, " Pierce");
    if (res_flags & RES_SLASH		) strcat(buf, " Slash");
    if (res_flags & RES_FIRE		) strcat(buf, " Fire");
    if (res_flags & RES_COLD		) strcat(buf, " Cold");
    if (res_flags & RES_LIGHTNING	) strcat(buf, " Lghtng");
    if (res_flags & RES_ACID		) strcat(buf, " Acid");
    if (res_flags & RES_POISON		) strcat(buf, " Poison");
    if (res_flags & RES_NEGATIVE	) strcat(buf, " Nega");
    if (res_flags & RES_HOLY		) strcat(buf, " Holy");
    if (res_flags & RES_ENERGY		) strcat(buf, " Energy");
    if (res_flags & RES_MENTAL		) strcat(buf, " Mental");
    if (res_flags & RES_DISEASE		) strcat(buf, " Disease");
    if (res_flags & RES_DROWNING	) strcat(buf, " Drown");
    if (res_flags & RES_LIGHT2		) strcat(buf, " Light2");
    if (res_flags & RES_SOUND		) strcat(buf, " Sound");
    if (res_flags & RES_ELEMENTAL	) strcat(buf, " Elemental");
    if (res_flags & RES_WOOD		) strcat(buf, " Wood");
    if (res_flags & RES_SILVER		) strcat(buf, " Silver");
    if (res_flags & RES_IRON		) strcat(buf, " Iron");
    if (res_flags & RES_MAIM		) strcat(buf, " Maim");
    if (res_flags & RES_CRUSH		) strcat(buf, " Crush");
    if (res_flags & RES_CLEAVE		) strcat(buf, " Cleave");
    if (res_flags & RES_PUNCH		) strcat(buf, " Punch");
    if (res_flags & RES_BITE		) strcat(buf, " Bite");
    if (res_flags & RES_CLAW		) strcat(buf, " Claw");

    return ( buf[0] != '\0' ) ? buf+1 : "NONE";
}

char *vuln_bit_name(int vuln_flags)
{
    static char buf[512];

    buf[0] = '\0';

    if (vuln_flags & VULN_SUMMON	) strcat(buf, " Summ");
    if (vuln_flags & VULN_CHARM		) strcat(buf, " Charm");
    if (vuln_flags & VULN_MAGIC		) strcat(buf, " Magic");
    if (vuln_flags & VULN_WEAPON	) strcat(buf, " Weap");
    if (vuln_flags & VULN_BASH		) strcat(buf, " Bash");
    if (vuln_flags & VULN_PIERCE	) strcat(buf, " Pierce");
    if (vuln_flags & VULN_SLASH		) strcat(buf, " Slash");
    if (vuln_flags & VULN_FIRE		) strcat(buf, " Fire");
    if (vuln_flags & VULN_COLD		) strcat(buf, " Cold");
    if (vuln_flags & VULN_LIGHTNING	) strcat(buf, " Lghtng");
    if (vuln_flags & VULN_ACID		) strcat(buf, " Acid");
    if (vuln_flags & VULN_POISON	) strcat(buf, " Poison");
    if (vuln_flags & VULN_NEGATIVE	) strcat(buf, " Nega");
    if (vuln_flags & VULN_HOLY		) strcat(buf, " Holy");
    if (vuln_flags & VULN_ENERGY	) strcat(buf, " Energy");
    if (vuln_flags & VULN_MENTAL	) strcat(buf, " Mental");
    if (vuln_flags & VULN_DISEASE	) strcat(buf, " Disease");
    if (vuln_flags & VULN_DROWNING	) strcat(buf, " Drown");
    if (vuln_flags & VULN_LIGHT2	) strcat(buf, " Light2");
    if (vuln_flags & VULN_SOUND		) strcat(buf, " Sound");
    if (vuln_flags & VULN_ELEMENTAL	) strcat(buf, " Elemental");
    if (vuln_flags & VULN_WOOD		) strcat(buf, " Wood");
    if (vuln_flags & VULN_SILVER	) strcat(buf, " Silver");
    if (vuln_flags & VULN_IRON		) strcat(buf, " Iron");
    if (vuln_flags & VULN_MAIM		) strcat(buf, " Maim");
    if (vuln_flags & VULN_CRUSH		) strcat(buf, " Crush");
    if (vuln_flags & VULN_CLEAVE	) strcat(buf, " Cleave");
    if (vuln_flags & VULN_PUNCH		) strcat(buf, " Punch");
    if (vuln_flags & VULN_BITE		) strcat(buf, " Bite");
    if (vuln_flags & VULN_CLAW		) strcat(buf, " Claw");

    return ( buf[0] != '\0' ) ? buf+1 : "NONE";
}

char *wear_bit_name(int wear_flags)
{
    static char buf[512];

    buf [0] = '\0';
    if (wear_flags & ITEM_TAKE		) strcat(buf, " take");
    if (wear_flags & ITEM_WEAR_FINGER	) strcat(buf, " finger");
    if (wear_flags & ITEM_WEAR_NECK	) strcat(buf, " neck");
    if (wear_flags & ITEM_WEAR_BODY	) strcat(buf, " torso");
    if (wear_flags & ITEM_WEAR_HEAD	) strcat(buf, " head");
    if (wear_flags & ITEM_WEAR_LEGS	) strcat(buf, " legs");
    if (wear_flags & ITEM_WEAR_FEET	) strcat(buf, " feet");
    if (wear_flags & ITEM_WEAR_HANDS	) strcat(buf, " hands");
    if (wear_flags & ITEM_WEAR_ARMS	) strcat(buf, " arms");
    if (wear_flags & ITEM_WEAR_SHIELD	) strcat(buf, " shield");
    if (wear_flags & ITEM_WEAR_ABOUT	) strcat(buf, " body");
    if (wear_flags & ITEM_WEAR_WAIST	) strcat(buf, " waist");
    if (wear_flags & ITEM_WEAR_WRIST	) strcat(buf, " wrist");
    if (wear_flags & ITEM_WIELD		) strcat(buf, " wield");
    if (wear_flags & ITEM_HOLD		) strcat(buf, " hold");
    if (wear_flags & ITEM_NO_SAC	) strcat(buf, " nosac");
    if (wear_flags & ITEM_WEAR_FLOAT	) strcat(buf, " float");
    if (wear_flags & ITEM_WEAR_BACK	) strcat(buf, " back");
    if (wear_flags & ITEM_WEAR_LIGHT	) strcat(buf, " light");
	
    return ( buf[0] != '\0' ) ? buf+1 : "NONE";
}

char *form_bit_name(int form_flags)
{
    static char buf[512];

    buf[0] = '\0';
    if (form_flags & FORM_POISON	) strcat(buf, " poison");
    else if (form_flags & FORM_EDIBLE	) strcat(buf, " edible");
    if (form_flags & FORM_MAGICAL	) strcat(buf, " magical");
    if (form_flags & FORM_INSTANT_DECAY	) strcat(buf, " instant_rot");
    if (form_flags & FORM_OTHER		) strcat(buf, " other");
    if (form_flags & FORM_ANIMAL	) strcat(buf, " animal");
    if (form_flags & FORM_SENTIENT	) strcat(buf, " sentient");
    if (form_flags & FORM_UNDEAD	) strcat(buf, " undead");
    if (form_flags & FORM_CONSTRUCT	) strcat(buf, " construct");
    if (form_flags & FORM_MIST		) strcat(buf, " mist");
    if (form_flags & FORM_INTANGIBLE	) strcat(buf, " intangible");
    if (form_flags & FORM_BIPED		) strcat(buf, " biped");
    if (form_flags & FORM_CENTAUR	) strcat(buf, " centaur");
    if (form_flags & FORM_INSECT	) strcat(buf, " insect");
    if (form_flags & FORM_SPIDER	) strcat(buf, " spider");
    if (form_flags & FORM_CRUSTACEAN	) strcat(buf, " crustacean");
    if (form_flags & FORM_WORM		) strcat(buf, " worm");
    if (form_flags & FORM_BLOB		) strcat(buf, " blob");
    if (form_flags & FORM_MAMMAL	) strcat(buf, " mammal");
    if (form_flags & FORM_BIRD		) strcat(buf, " bird");
    if (form_flags & FORM_REPTILE	) strcat(buf, " reptile");
    if (form_flags & FORM_SNAKE		) strcat(buf, " snake");
    if (form_flags & FORM_DRAGON	) strcat(buf, " dragon");
    if (form_flags & FORM_AMPHIBIAN	) strcat(buf, " amphibian");
    if (form_flags & FORM_FISH		) strcat(buf, " fish");
    if (form_flags & FORM_COLD_BLOOD 	) strcat(buf, " cold_blooded");

    return ( buf[0] != '\0' ) ? buf+1 : "NONE";
}

char *part_bit_name(int part_flags)
{
    static char buf[512];

    buf[0] = '\0';
    if (part_flags & PART_HEAD		) strcat(buf, " head");
    if (part_flags & PART_ARMS		) strcat(buf, " arms");
    if (part_flags & PART_LEGS		) strcat(buf, " legs");
    if (part_flags & PART_HEART		) strcat(buf, " heart");
    if (part_flags & PART_BRAINS	) strcat(buf, " brains");
    if (part_flags & PART_GUTS		) strcat(buf, " guts");
    if (part_flags & PART_HANDS		) strcat(buf, " hands");
    if (part_flags & PART_FEET		) strcat(buf, " feet");
    if (part_flags & PART_FINGERS	) strcat(buf, " fingers");
    if (part_flags & PART_EAR		) strcat(buf, " ears");
    if (part_flags & PART_EYE		) strcat(buf, " eyes");
    if (part_flags & PART_LONG_TONGUE	) strcat(buf, " long_tongue");
    if (part_flags & PART_EYESTALKS	) strcat(buf, " eyestalks");
    if (part_flags & PART_TENTACLES	) strcat(buf, " tentacles");
    if (part_flags & PART_FINS		) strcat(buf, " fins");
    if (part_flags & PART_WINGS		) strcat(buf, " wings");
    if (part_flags & PART_TAIL		) strcat(buf, " tail");
    if (part_flags & PART_CLAWS		) strcat(buf, " claws");
    if (part_flags & PART_FANGS		) strcat(buf, " fangs");
    if (part_flags & PART_HORNS		) strcat(buf, " horns");
    if (part_flags & PART_SCALES	) strcat(buf, " scales");

    return ( buf[0] != '\0' ) ? buf+1 : "NONE";
}

char *weapon_bit_name(int weapon_flags)
{
    static char buf[512];

    buf[0] = '\0';
    if (weapon_flags & WEAPON_FLAMING	) strcat(buf, " flaming");
    if (weapon_flags & WEAPON_FROST	) strcat(buf, " frost");
    if (weapon_flags & WEAPON_VAMPIRIC	) strcat(buf, " vampiric");
    if (weapon_flags & WEAPON_SHARP	) strcat(buf, " sharp");
    if (weapon_flags & WEAPON_VORPAL	) strcat(buf, " vorpal");
    if (weapon_flags & WEAPON_TWO_HANDS ) strcat(buf, " two-handed");
    if (weapon_flags & WEAPON_SHOCKING 	) strcat(buf, " shocking");
    if (weapon_flags & WEAPON_POISON	) strcat(buf, " poison");

    return ( buf[0] != '\0' ) ? buf+1 : "NONE";
}

char *cont_bit_name( int cont_flags)
{
    static char buf[512];

    buf[0] = '\0';

    if (cont_flags & CONT_CLOSEABLE	) strcat(buf, " closable");
    if (cont_flags & CONT_PICKPROOF	) strcat(buf, " pickproof");
    if (cont_flags & CONT_CLOSED	) strcat(buf, " closed");
    if (cont_flags & CONT_LOCKED	) strcat(buf, " locked");

    return (buf[0] != '\0' ) ? buf+1 : "NONE";
}


char *off_bit_name(int off_flags)
{
    static char buf[512];

    buf[0] = '\0';

    if (off_flags & OFF_AREA_ATTACK	) strcat(buf, " AreaAttk");
    if (off_flags & OFF_BACKSTAB	) strcat(buf, " BStab");
    if (off_flags & OFF_BASH		) strcat(buf, " Bash");
    if (off_flags & OFF_BERSERK		) strcat(buf, " BSerk");
    if (off_flags & OFF_DISARM		) strcat(buf, " Disarm");
    if (off_flags & OFF_DODGE		) strcat(buf, " Dodge");
    if (off_flags & OFF_FADE		) strcat(buf, " Fade");
    if (off_flags & OFF_FAST		) strcat(buf, " Fast");
    if (off_flags & OFF_KICK		) strcat(buf, " Kick");
    if (off_flags & OFF_KICK_DIRT	) strcat(buf, " KDirt");
    if (off_flags & OFF_PARRY		) strcat(buf, " Parry");
    if (off_flags & OFF_RESCUE		) strcat(buf, " Rescue");
    if (off_flags & OFF_TAIL		) strcat(buf, " Tail");
    if (off_flags & OFF_TRIP		) strcat(buf, " Trip");
    if (off_flags & OFF_CRUSH		) strcat(buf, " Crush");
    if (off_flags & ASSIST_ALL		) strcat(buf, " AsstAll");
    if (off_flags & ASSIST_ALIGN	) strcat(buf, " AsstAlign");
    if (off_flags & ASSIST_RACE		) strcat(buf, " AsstRace");
    if (off_flags & ASSIST_PLAYERS	) strcat(buf, " AsstPC");
    if (off_flags & ASSIST_GUARD	) strcat(buf, " AsstGuard");
    if (off_flags & ASSIST_VNUM		) strcat(buf, " AsstVnum");

    return ( buf[0] != '\0' ) ? buf+1 : "NONE";
}

char *strip_color( char *argument )
{
    char *str;
    static char new_str [ MAX_STRING_LENGTH ];
    int i = 0;
    new_str[0] = '\0';
    for ( str = argument; *str; str++ )
      {
      if ( new_str[ i-1 ] == '{'
      && is_colcode( *str ) )
        {
        i--;
        continue;
        }
      if ( new_str[ i-1 ] == '{'
      && *str == '{' )
        continue;
      new_str[ i ] = *str;
      i++;
      }
    if ( new_str[ i ] != '\0' )
        new_str[ i ] = '\0';
    return new_str;
}

bool is_colcode( char c )
{
  char *colors = "BbCcDdGgMmRrWwYyx";
  int iCtr;
  for ( iCtr = 0; colors[iCtr] != '\0'; iCtr++ )
    if ( colors[iCtr] == c )
      return TRUE;
  return FALSE;
}


int strlen_wo_col( char *argument )
{
  char *str;
  bool found = FALSE;
  int colfound = 0;
  int ampfound = 0;
  int len;
  for ( str = argument; *str != '\0'; str++ )
    {
    if ( found && is_colcode( *str ) )
        {
        colfound++;
        found = FALSE;
        }
    if ( found && *str == '{' )
        ampfound++;
    if ( *str == '{' )
        found = found ? FALSE : TRUE;
    else
        found = FALSE;
    }
  len = strlen( argument );
  len = len - ampfound - ( colfound * 2 );
  return len;
}

int     same_obj(OBJ_DATA *obj_match, OBJ_DATA *obj)
{
    OBJ_DATA *      obj2;
    int             count;
    
    count=0;
    
    for(obj2=obj; obj2!=NULL; obj2 = obj2->next_content)
    {
        if(obj2->item_type == ITEM_CONTAINER)
            count+=same_obj(obj_match, obj2->contains);

        if(obj2->pIndexData->vnum == obj_match->pIndexData->vnum)
            count++;
    }
    
    return count;
}

int roll_stat( CHAR_DATA *ch, int stat )
{
    int temp,low,high;

    high = (pc_race_table[ch->race].max_stats[stat] - 5);
    low =  (pc_race_table[ch->race].stats[stat] - 3);
    temp = (number_range(low,high));

    if (class_table[ch->class].attr_prime == stat)
	{
        if (ch->race == race_lookup("human"))
           temp += 4;
        else
           temp += 1;
	}

    return UMIN(temp,high);
}

void	make_stats(CHAR_DATA *ch)
{
	int	lower, higher, difference, i, target;

	lower=higher=0;

	for(i=0;i<MAX_STATS;i++)
	{
		ch->perm_stat[i]=(roll_stat(ch, i) +2);
		lower+=pc_race_table[ch->race].stats[i];
		higher+=ch->perm_stat[i];
	}
	
	difference=higher-lower;
	difference*=-1;
	difference-=5;
	if(difference>0)
	{
		target=number_range(0, MAX_STATS);
		for( ;ch->perm_stat[target]>3 && difference >0;
			ch->perm_stat[target]-=1, difference-=1);
	}
}




void do_fileident( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char word[20];
    char start[20];
    char end[20];
    char file[20];
    
    int svnum;
    int evnum;
    int vnum;
    FILE *fp;
    AFFECT_DATA *paf;

    int nMatch=0;
    extern int top_obj_index;
    OBJ_INDEX_DATA *pObjIndex;

   file[0] = '\0';

   argument = one_argument( argument , word   );
   argument = one_argument( argument , start  );
   argument = one_argument( argument , end    );
   argument = one_argument( argument , file   );

   svnum = atoi(start);
   evnum = atoi(end);

   if(file[0] == '\0')
     {
  send_to_char("\n\r{GSyntax{w: {WFILEIDENT OBJ {c<{WSTARTING VNUM{c> <{WENDING VNUM{c> <{WFILE NAME{c>{x\n\r",ch); 
     return;
     }

   sprintf(buf,"/home/dragon/kotrd/files/dbdump/%s.obj",file);
   if ( ( fp = fopen( buf, "w" ) ) == NULL )
    {
        perror( buf );
        return;
    }

    if( !str_cmp(word,"obj") || !str_cmp(word,"object") )
    { 

     for ( vnum = svnum; (nMatch < top_obj_index) && (vnum < evnum) ; vnum++ )
      {
          if ( ( pObjIndex = get_obj_index( vnum ) ) != NULL )
          {
           OBJ_DATA *obj=NULL;

           nMatch++;
           fprintf( fp,"  Name,%s\n", strip_color(pObjIndex->name) );

          fprintf( fp, "A_Name,%s\n",strip_color(pObjIndex->area->name));

          fprintf( fp, "A_VNum,%d\nO_Vnum,%d\n",pObjIndex->area->vnum,pObjIndex->vnum);

          fprintf( fp, " LeveL,%d\n",pObjIndex->level);

          fprintf( fp, "  Type,%s\n", item_name(pObjIndex->item_type));

           fprintf( fp,"Materl,%s\n",
           strip_color((material_name(pObjIndex->material_type))) );

          fprintf( fp, " Extra,%s\n",extra_bit_name( pObjIndex->extra_flags ) );

          fprintf( fp, "Values, v0 %d / v1 %d / v2 %d / v3 %d / v4 %d\n",
              pObjIndex->value[0], pObjIndex->value[1], pObjIndex->value[2],
              pObjIndex->value[3], pObjIndex->value[4] );

          fprintf( fp, "  Wear,%s\n",wear_bit_name(pObjIndex->wear_flags));

          fprintf( fp, "  Cost,%d\n",pObjIndex->cost);

          fprintf( fp, "Weight,%d\n",pObjIndex->weight);

             switch ( pObjIndex->item_type )
              {
               case ITEM_SCROLL:
               case ITEM_POTION:
               case ITEM_PILL:

               fprintf( fp, "SPELLs,");


               if(pObjIndex->value[1] >= 0 && pObjIndex->value[1] < MAX_SKILL )
                 {
               fprintf( fp, "LvL %d  S_Name: 1 %s\n", pObjIndex->value[0],
skill_table[pObjIndex->value[1]].name);
                 }
	        else
                 {
               fprintf( fp, "LvL %d  S_Name:\n", pObjIndex->value[0]);
                 }

               if(pObjIndex->value[2] >= 0 && pObjIndex->value[2] < MAX_SKILL )
               {
                fprintf(fp ,",                        2 %s\n", skill_table[pObjIndex->value[2]].name);
               }
               if(pObjIndex->value[3] >= 0 && pObjIndex->value[3] < MAX_SKILL )
               {
                fprintf(fp ,",                        3 %s\n", skill_table[pObjIndex->value[3]].name);
               }
               if(pObjIndex->value[4] >= 0 && pObjIndex->value[4] < MAX_SKILL )
               {
                fprintf(fp ,",                        4 %s\n", skill_table[pObjIndex->value[4]].name);
               }
             else
               fprintf(fp ,"\n");

             break;

              case ITEM_WAND:
              case ITEM_STAFF:
              fprintf( fp, " Chrgs,%d(%d)  LvL %d  ",
               pObjIndex->value[1], pObjIndex->value[2], pObjIndex->value[0] );

             if ( pObjIndex->value[3] >= 0 && pObjIndex->value[3] < MAX_SKILL )
             {
               fprintf(fp ,"Spell: %s\n", skill_table[pObjIndex->value[3]].name);
             }

               break;

           case ITEM_DRINK_CON:
                fprintf(fp,"Liquid,%s\nLiQClR,%s\n",
                     liq_table[pObjIndex->value[2]].liq_name,
                     liq_table[pObjIndex->value[2]].liq_color);
                break;

           case ITEM_WEAPON:
               fprintf(fp," WType,");
               switch (pObjIndex->value[0])
               {
                   case(WEAPON_SWORD):
                       fprintf(fp,"SWORD\n");
                       break;
                   case(WEAPON_DAGGER):
                       fprintf(fp,"DAGGER\n");
                       break;
                   case(WEAPON_SPEAR):
                       fprintf(fp,"SPEAR_STAFF\n");
                       break;
                   case(WEAPON_MACE):
                       fprintf(fp,"MACE_CLUB\n");
                       break;
                   case(WEAPON_AXE):
                       fprintf(fp,"AXE\n");
                       break;
                   case(WEAPON_FLAIL):
                       fprintf(fp,"FLAIL\n");
                       break;
                   case(WEAPON_WHIP):
                       fprintf(fp,"WHIP\n");
                       break;
                   case(WEAPON_POLEARM):
                       fprintf(fp,"POLEARM\n");
                       break;
                   case(WEAPON_EXOTIC):
                       fprintf(fp,"EXOTIC\n");
                       break;
                   default:
                       fprintf(fp,"*UNKNOWN*\n");
                       break;
                }

                if (pObjIndex->new_format)
                    fprintf(fp," DDice,%dD%d (%d)\n",
                      pObjIndex->value[1],pObjIndex->value[2],
                      (1 + pObjIndex->value[2]) * pObjIndex->value[1] / 2);
                else
                   fprintf( fp, " DDice,%d to %d (%d)\n",
                       pObjIndex->value[1], pObjIndex->value[2],
                       ( pObjIndex->value[1] + pObjIndex->value[2] ) / 2 );

                fprintf(fp,"D_NOUN,%s\n",
                    (   pObjIndex->value[3] > 0 
                     && pObjIndex->value[3] < MAX_DAMAGE_MESSAGE) ?
                        attack_table[pObjIndex->value[3]].noun : "undefined");
  
                if (pObjIndex->value[4])  /* weapon flags */
                {
                    fprintf(fp,"WFlags,%s\n",
                        weapon_bit_name(pObjIndex->value[4]));
                 }


              break;

        case ITEM_ARMOR:
            fprintf( fp,
            "    AC, P %d / B %d / S %d / E %d\n",
                pObjIndex->value[0], pObjIndex->value[1], 
                pObjIndex->value[2], pObjIndex->value[3] );

            break;

        case ITEM_CONTAINER:
            fprintf(fp," Capac,%d#\nMaXWgt,%d#\nCONFlg,%s\n",
                pObjIndex->value[0], pObjIndex->value[3], 
                cont_bit_name(pObjIndex->value[1]));

            if (pObjIndex->value[4] != 100)
            {
                fprintf(fp,"  WgtX,%d%%\n",
                    pObjIndex->value[4]);
            }
        break;

       
     default :  
               break;


             }


          fprintf( fp, "Resets,%d\nFormat,%s\n",pObjIndex->reset_num,
pObjIndex->new_format ? "new" : "old");


             for ( paf = pObjIndex->affected; paf != NULL; paf = paf->next )
             {
               fprintf( fp, " AFFTs,%s x %d - LvL %d\n",
                 affect_loc_name( paf->location ), paf->modifier,paf->level );

             if (paf->bitvector)
              {
                  switch(paf->where)
                  {
                      case TO_AFFECTS:
                         fprintf(fp,"+AFFt,%s\n",
                             affect_bit_name(paf->bitvector));
                        break;
                      case TO_AFFECTS2:
                         fprintf(fp,"+AFFt2,%s\n",
                             affect2_bit_name(paf->bitvector));
                        break;
                      case TO_WEAPON:
                         fprintf(fp,"+WFlag,%s\n",
                            weapon_bit_name(paf->bitvector));
                        break;
                      case TO_OBJECT:
                         fprintf(fp,"+OFlag,%s\n",
                           extra_bit_name(paf->bitvector));
                        break;
                      case TO_IMMUNE:
                          fprintf(fp,"+IMMun,%s\n",
                            imm_bit_name(paf->bitvector));
                        break;
                      case TO_RESIST:
                          fprintf(fp,"+RESit,%s\n\r",
                             imm_bit_name(paf->bitvector));
                         break;
                      case TO_VULN:
                          fprintf(fp,"+VULn,%s\n\r",
                            imm_bit_name(paf->bitvector));
                         break;
                      default:
                          fprintf(fp,"UNKn,%d: %d\n\r",
                            paf->where,paf->bitvector);
                         break;
                    }
                }
             }

          fprintf( fp, "S_DSEC,%s\n",
           strip_color(pObjIndex->short_descr));

           fprintf( fp, "L_DESC,%s\n",strip_color(pObjIndex->description));

             if ( ( obj = get_obj_world( ch, pObjIndex->name ) ) == NULL )
              {
               fprintf(fp, "  LOAD,NO\n" );
              }
             else
             {
              fprintf( fp,
              " Where,N_Rm %d / N_OBJ %s / Carried By %s / WEAR_Loc %d\n",
              obj->in_room    == NULL    ?        0 : obj->in_room->vnum,
              obj->in_obj     == NULL    ? "NONE" : obj->in_obj->short_descr,
              obj->carried_by == NULL    ? "NONE" :
              obj->carried_by->name , obj->wear_loc );
              }


             fprintf(fp,"\n---------------------------------------\n\n");
        }


      }


    }
    else
    {
      send_to_char("syntax : fileident [mob/obj/room] [begin vnum] [end vnum] [file name]",ch);
    }
    fclose(fp);
    return;
}


CHAR_DATA *get_char_area( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *ach;
    int number,count;
    
    if ( (ach = get_char_room( ch, argument )) != NULL )
        return ach;
        
    number = number_argument( argument, arg );
    count = 0;
    for ( ach = char_list; ach != NULL; ach = ach->next )
    {
        if ((!ach->in_room || (ach->in_room->area != ch->in_room->area)) ||  !can_see( ch, ach ) || !is_name( arg, ach->name ))
            continue;
        if (++count == number)
            return ach;
    }
     
    return ach;
}

bool is_owner( CHAR_DATA *ch )
{
  CHAR_DATA *rch = ch;
  if ( !rch )
    return FALSE;
  if ( IS_NPC( rch )
  &&   rch->desc
  &&   rch->desc->original )
    rch = rch->desc->original;

  if ( !str_cmp( rch->name, "Yavi" )
  ||   !str_cmp( rch->name, "Venus" )
  ||   !str_cmp( rch->name, "Sembiance" )
  ||   !str_cmp( rch->name, "Argon") 
  ||   !str_cmp( rch->name, "Lybre"))  
  return TRUE;
  return FALSE;
} 


void check_spirit( CHAR_DATA *ch, CHAR_DATA *victim )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *spirit;

    if(ch->level < 20)  return;
    
    if ( number_range(0,25) != 0 || !IS_NPC(victim))
        return;

    spirit = create_mobile( victim->pIndexData );

    SET_BIT ( spirit->form,FORM_INSTANT_DECAY|FORM_UNDEAD|FORM_INTANGIBLE );

    spirit->parts = 0;

    SET_BIT ( spirit->act, ACT_AGGRESSIVE );

    SET_BIT ( spirit->affected_by, AFF_PASS_DOOR );

    sprintf(buf,"{DThe Spirit of {W%s {Dis haunting this spot!{D{x\n\r",
    capitalize(strip_color(victim->short_descr)));

    spirit->long_descr = str_dup(buf);

    sprintf(buf,"{Dthe spirit of {W%s{D",capitalize(strip_color(victim->short_descr)));

    spirit->short_descr = str_dup(buf);

    sprintf(buf,"spirit %s",victim->name);

    spirit->name = str_dup(buf);

    char_to_room( spirit, ch->in_room );

    act("\n\r{rYou cower in fear as {R$N {rappears before you!{x\n\r",ch,NULL,spirit,TO_CHAR);

    act("\n\r{R$N {rsuddenly appears and attacks {W$n{r!{x\n\r",ch,NULL,spirit,TO_ROOM);

    multi_hit( spirit, ch, TYPE_UNDEFINED );

    return;
}

char * makedummy(char * string, CHAR_DATA *ch)
{
        int     i;
char    c;
        bool    firstChar=TRUE;
        bool    secondChar=FALSE;

        for(i=0,c=string[i];i<strlen(string);i++,c=string[i])
        {
                if((c>='A' && c<='Z')||(c>='a' && c<='z'))
                {
                        if(firstChar)
                        {
                                string[i]='d';
                                firstChar=FALSE;
                                secondChar=TRUE;
                        }
                        else if(secondChar)
                        {
                                string[i]='u';
                                secondChar=FALSE;
                        }
                        else
                                string[i]='h';
                }
                else
                {
                        firstChar=TRUE;
                        secondChar=FALSE;
                }
        }

        return string;
}

char ** GetObjExtraDescKeywords(OBJ_DATA * obj)
{
    char **             keywords=0;
    EXTRA_DESCR_DATA *  ed;
    
    if(obj->extra_descr || (obj->pIndexData && obj->pIndexData->extra_descr))
    {
        for(ed=obj->extra_descr;ed;ed=ed->next)
        {
            keywords = array_append(keywords, ed->keyword);
        }

        for(ed=obj->pIndexData->extra_descr;ed!=NULL;ed=ed->next)
        {
            keywords = array_append(keywords, ed->keyword);
        }
    }
    
    return keywords;
}

char * GetObjExtraDescText(EXTRA_DESCR_DATA * ed)
{
    if(ed)
        return ed->description;
    
    return 0;
}

EXTRA_DESCR_DATA *  GetObjExtraDesc(OBJ_DATA * obj, char * name)
{
    EXTRA_DESCR_DATA *  ed;
    
    if(obj->extra_descr || (obj->pIndexData && obj->pIndexData->extra_descr))
    {
        for(ed=obj->extra_descr;ed;ed=ed->next)
        {
            if(!strcmp(ed->keyword, name))
                return ed;
        }

        for(ed=obj->pIndexData->extra_descr;ed!=NULL;ed=ed->next)
        {
            if(!strcmp(ed->keyword, name))
                return ed;
        }
    }
    
    return 0;
}

void    AddObjExtraDesc(OBJ_DATA * obj, char * keyword, char * description)
{
    EXTRA_DESCR_DATA *  ed;
    
    ed = new_extra_descr();
    ed->keyword = str_dup(keyword);
    ed->description = str_dup(description);
    ed->next = obj->extra_descr;
    obj->extra_descr = ed;
}

void DeleteObjExtraDesc(OBJ_DATA *obj, char * keyword)
{
    EXTRA_DESCR_DATA *  ed=0;
    EXTRA_DESCR_DATA *  ped=0;
    
    for(ed=obj->extra_descr;ed;ed=ed->next )
    {
        if(is_name(keyword, ed->keyword))
            break;
        ped = ed;
    }
    
    if(!ed)
        return;


    if(!ped)
        obj->extra_descr = ed->next;
    else
        ped->next = ed->next;

    free_extra_descr(ed);
}


void extract_objs_in_list(OBJ_DATA * contents, char ** vnums)
{
    OBJ_DATA *  obj=0;
    OBJ_DATA *  obj_next=0;
    char **     ar=0;
    
    for(ar=vnums;ar && *ar;ar++)
    {
        for(obj=contents;obj;obj=obj_next)
        {
            obj_next = obj->next_content;
            if(obj->pIndexData && obj->pIndexData->vnum==atoi(*ar))
                extract_obj(obj);
        }    
    }
}

