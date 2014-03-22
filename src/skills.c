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
#include "recycle.h"
#include "dpit.h"
#include "tables.h"

#include "Utility.h"
#include "StringUtility.h"
#include "ArrayUtility.h"

DECLARE_DO_FUN (do_chant);
DECLARE_DO_FUN (do_crecall);


/* Local Functions */
void  crecall  args((CHAR_DATA *ch ));
int     find_door       args( ( CHAR_DATA *ch, char *arg ) );
bool    has_key         args( ( CHAR_DATA *ch, int key ) );
void    disarm          args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void    set_fighting    args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void    raw_kill        args( ( CHAR_DATA *victim, CHAR_DATA *killer ) );
void    group_gain      args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void    one_hit         args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt, bool secondary ));
void    update_pkinfo   args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );

int remvalue;

int  gain_skills_array_sort(const void *a, const void *b)
{
    char bufA[MAX_STRING_LENGTH];
    char bufB[MAX_STRING_LENGTH];
    
    strcpy(bufA, *(char **)a);
    strchrrep(bufA, ',', '\0');
    strcpy(bufB, *(char **)b);
    strchrrep(bufB, ',', '\0');
    
    if(atoi(bufA)<atoi(bufB))
        return -1;
    else if(atoi(bufA)==atoi(bufB))
        return 0;

    return 1;
}

void do_gain(CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char ** skills=0;
   char ** spells=0;
   char ** cols=0;
   char ** ar=0;
   BUFFER *    buffer;

    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *trainer;
    int gn = 0, sn = 0;

    if (IS_NPC(ch))
	return;

    /* find a trainer */
    for ( trainer = ch->in_room->people; 
	  trainer != NULL; 
	  trainer = trainer->next_in_room)
	if (IS_NPC(trainer) && IS_SET(trainer->act,ACT_GAIN))
	    break;

    if (trainer == NULL || !can_see(ch,trainer))
    {
	send_to_char("\n\r{GYou can't do that here.{x\n\r",ch);
	return;
    }


    argument=one_argument(argument,arg);
    argument=one_argument(argument, arg2);

    if (arg[0] == '\0')
    {
send_to_char("\n\r {GSyntax{w: {WGAIN {c<{WOPTION{c>{x\n\r",ch);
send_to_char("\n\r{cOptions{w: {c<{WSKILL NAME{c> - {CYou GAIN the Choosen SKILL{c.{x\n\r",ch);
send_to_char("         {c<{WGROUP NAME{c> - {CYou GAIN the Choosen SKILL or SPELL GROUP{c.{x\n\r",ch);
send_to_char("                 {WLIST {c- {CList ALL Groups & Skills Available to you{c.{x\n\r",ch);
send_to_char("               {WPOINTS {c- {CThis LOWERs your CP cost by 1.  This costs 2 TRAINs{c.{x\n\r",ch);
send_to_char("            {G**{WCONVERT {c- {CTurns 1 TRAIN into 10 PRACs or Turns 10 PRACs into 1 TRAIN{x\n\r",ch);
send_to_char("                        {Cor Turns 2,000 QPOINTs into 1 TRAIN{c.{x\n\r",ch);
send_to_char("\n\r{G** {gi.e. {WGAIN CONVERT TRAIN{g, {WGAIN CONVERT PRAC{g, {WGAIN CONVERT QPOINTS{x\n\r",ch);
if(IS_LEGEND(ch))
{
    send_to_char("\n\r{GLegends use{w:  {WGAIN SPELLLIST {c- {CLists the SPELLS you can gain{x\n\r",ch);

    send_to_char("{GLegends purchase skills with quest points, gold and trains.{x\n\r", ch);
    send_to_char("{GYou do that with{w: {WGAIN BUY {c<{WSKILL NAME{c>{x\n\r", ch);
}

	return;
    }

    if (!str_prefix(arg,"list"))
    {
    	int col;
    	
    	col = 0;
    
        if(!IS_LEGEND(ch))
        {
        	sprintf(buf, "\n\r{D [{R%-16s{D][{RLv{D][{RTr{D] [{R%-16s{D][{RLv{D][{RTr{D]{x\n\r", "Group Name", "Group Name");
        	send_to_char(buf,ch);
    
        	for (gn = 0; gn < MAX_GROUP; gn++)
        	{
        	    if (group_table[gn].name == NULL)
        		break;
        
        	    if (!ch->pcdata->group_known[gn]
        	    &&  (group_table[gn].rating[ch->class] > 0 || IS_LEGEND(ch)))
        	    {
        
           		sprintf(buf,"{D [{W%-16s{D][  ][{W%2d{D]", group_table[gn].name, group_table[gn].rating[ch->class]);
        		send_to_char(buf,ch);
        		if (++col % 2 == 0)
        		    send_to_char("\n\r",ch);
        	    }
        	}
        	if (col % 2 != 0)
        	    send_to_char("\n\r",ch);
        	
        	send_to_char("\n\r",ch);		
        
        	col = 0;
        }
 
        for (sn = 0; sn < MAX_SKILL; sn++)
        {
            if (skill_table[sn].name == NULL)
                break;
            if(skill_table[sn].legendLevel==-1)
                continue;
 
            if (!ch->pcdata->learned[sn]
            &&  (skill_table[sn].rating[ch->class] > 0 || IS_LEGEND(ch))
	    &&  skill_table[sn].spell_fun == spell_null )
            {
                if(IS_LEGEND(ch))
                    sprintf(buf, "%s,%d,%d,%d,%d", skill_table[sn].name, skill_table[sn].legendLevel, skill_table[sn].legendTrainCost, skill_table[sn].legendQPCost, skill_table[sn].legendGoldCost);
                else
                    sprintf(buf, "%s,%d,%d", skill_table[sn].name, skill_table[sn].skill_level[ch->class], skill_table[sn].rating[ch->class]);
                
                skills = array_append(skills, buf);            
            }
        }
        
        array_sort(skills);
        //qsort(skills, array_len(skills), sizeof(char *), gain_skills_array_sort);
        buffer = new_buf();
           
        if(IS_LEGEND(ch))
            sprintf(buf, "\n\r{D[{R%-20s{D][{RLv{D][{RTr{D][{G  QP  {D][{Y  Gold  {D]{x\n\r", "Skill Name" );
        else
            sprintf(buf, "\n\r{D [{R%-16s{D][{RLv{D][{RTr{D] [{R%-16s{D][{RLv{D][{RTr{D]{x\n\r", "Skill Name", "Skill Name" );
        add_buf(buffer, buf);
           
        for(ar=skills;ar && *ar;ar++)
        {
            cols = strchrexplode(*ar, ',');

            if(IS_LEGEND(ch))
               sprintf(buf,"{D[{W%-20s{D][{W%2d{D][{W%2d{D][{G%6d{D][{Y%8d{D]{x\n\r",
                       cols[0], atoi(cols[1]), atoi(cols[2]), atoi(cols[3]), atoi(cols[4]));
            else
               sprintf(buf,"{D [{W%-16s{D][{W%2d{D][{W%2d{D]{x",
                       cols[0], atoi(cols[1]), atoi(cols[2]));
             add_buf(buffer, buf);
             
             if (++col % 2 == 0 && !IS_LEGEND(ch))
                 add_buf(buffer, "\n\r");     
            
            cols = array_free(cols);

        }
        skills = array_free(skills);        
    
        sprintf(buf,"\n\r\n\r{RTrains{w: {C%d    {RPractices{w: {C%d\n\r{x",ch->train,ch->practice);
        add_buf(buffer, buf);
                
        page_to_char(buf_string(buffer), ch);
        free_buf(buffer);
/*
        if (col % 2 != 0)
            send_to_char("\n\r",ch);
*/
	   return;
    }

    if (!str_prefix(arg,"spelllist") && IS_LEGEND(ch))
    {
        for (sn = 0; sn < MAX_SKILL; sn++)
        {
            if (skill_table[sn].name == NULL)
                break;
            if(skill_table[sn].legendLevel==-1)
                continue;
 
            if (!ch->pcdata->learned[sn]
            &&  (skill_table[sn].rating[ch->class] > 0 || IS_LEGEND(ch))
	    &&  (skill_table[sn].spell_fun == spell_null || IS_LEGEND(ch)))
            {
                if(IS_LEGEND(ch))
                    sprintf(buf, "%s,%d,%d,%d,%d", skill_table[sn].name, skill_table[sn].legendLevel, skill_table[sn].legendTrainCost, skill_table[sn].legendQPCost, skill_table[sn].legendGoldCost);
                else
                    sprintf(buf, "%s,%d,%d", skill_table[sn].name, skill_table[sn].skill_level[ch->class], skill_table[sn].rating[ch->class]);
                
                if(skill_table[sn].spell_fun!=spell_null)
                    spells = array_append(spells, buf);                
            }
        }
        
        array_sort(spells);
        buffer = new_buf();

        sprintf(buf, "\n\r{D[{R%-20s{D][{RLv{D][{RTr{D][{G  QP  {D][{Y  Gold  {D]{x\n\r", "Spell Name" );
        add_buf(buffer, buf);
        
        for(ar=spells;ar && *ar;ar++)
        {
            cols = strchrexplode(*ar, ',');

           sprintf(buf,"{D[{W%-20s{D][{W%2d{D][{W%2d{D][{G%6d{D][{Y%8d{D]{x\n\r",
                   cols[0], atoi(cols[1]), atoi(cols[2]), atoi(cols[3]), atoi(cols[4]));
             add_buf(buffer, buf);
                         
            cols = array_free(cols);

        }
        spells = array_free(spells);    

        sprintf(buf,"\n\r\n\r{RTrains{w: {C%d    {RPractices{w: {C%d\n\r{x",ch->train,ch->practice);
        add_buf(buffer, buf);
                
        page_to_char(buf_string(buffer), ch);
        free_buf(buffer);
        return;
    }


    if (!str_prefix(arg,"convert"))
    {
      if (!str_prefix(arg2,"practices") )
        {
        if (ch->practice < 10)
          {
          act("\n\r{W$N {Ytells you 'You are not yet ready.'{x\n\r",
               ch,NULL,trainer,TO_CHAR);
          return;
          }
        act("{W$N {chelps you apply your practice to training.{x",
              ch,NULL,trainer,TO_CHAR);
        ch->practice -= 10;
        ch->train +=1 ;
        return;
        }

      if (!str_prefix(arg2,"qpoints") )
        {
        if (ch->questpoints < 750)
          {
          act("\n\r{W$N {Ytells you 'You MUST have atleast 750 QPoints to do this.'{x\n\r",
               ch,NULL,trainer,TO_CHAR);
          return;
          }
        act("\n\r{W$N {ccashes in your {CQPOINTs{c for {C1 TRAIN{c.{x\n\r",
              ch,NULL,trainer,TO_CHAR);
        ch->questpoints -= 750;
        ch->train +=1 ;
        return;
        }


      if (!str_prefix(arg2,"trains") )
        {
        if ( ch->train == 0 )
          {
          act("\n\r{W$N {Ytells you 'You are not yet ready.'{x\n\r",
              ch,NULL,trainer,TO_CHAR);
          return;
          }
        act("\n\r{W$N {chelps you apply training to practice.\n\r{x", ch,NULL, trainer, TO_CHAR );
        ch->practice += 10;
        ch->train -= 1;
        return;
        }
      act("\n\r{W$N {Yasks you 'What do you wish to convert? practices or trains?{x\n\r",
                ch, NULL, trainer, TO_CHAR );
      return;
    }

    if (!str_prefix(arg,"points"))
    {
	if (ch->train < 2)
	{
	    act("\n\r{W$N {Ytells you 'You are not yet ready.'{x\n\r",
		ch,NULL,trainer,TO_CHAR);
	    return;
	}

	if (ch->pcdata->points <= 40)
	{
	    act("\n\r{W$N {Ytells you 'There would be no point in that.'{x\n\r",
		ch,NULL,trainer,TO_CHAR);
	    return;
	}

	act("\n\r{W$N {ctrains you, and you feel more at ease with your skills.{x\n\r",
	    ch,NULL,trainer,TO_CHAR);

	ch->train -= 2;
	ch->pcdata->points -= 1;
	ch->exp = exp_per_level(ch,ch->pcdata->points) * ch->level;
	return;
    }
    
    if(IS_LEGEND(ch) && !str_prefix(arg, "buy"))
    {
        sn = skill_lookup(arg2);
        if(sn==-1)
        {
            act("\n\r{W$N {Ytells you 'I do not understand...'\n\r{x",ch,NULL,trainer,TO_CHAR);
            return;
        }

        if (ch->pcdata->learned[sn])
        {
            act("\n\r{W$N {Ytells you 'You already know that skill!'{x\n\r", ch,NULL,trainer,TO_CHAR);
            return;
        }

        if(skill_table[sn].legendLevel==-1)
        {
            act("\n\r{W$N {Ytells you 'You may not learn that skill.'{x\n\r", ch,NULL,trainer,TO_CHAR);
            return;
        }
        
        if(ch->questpoints<skill_table[sn].legendQPCost)
        {
            act("\n\r{W$N {Ytells you 'You do not have enough quest points to buy that skill.'\n\r{x", ch,NULL,trainer,TO_CHAR);
            return;
        }
        
        if(ch->gold<skill_table[sn].legendGoldCost)
        {
            act("\n\r{W$N {Ytells you 'You do not have enough gold to buy that skill.'\n\r{x", ch,NULL,trainer,TO_CHAR);
            return;
        }
        
        if(ch->train<skill_table[sn].legendTrainCost)
        {
            act("\n\r{W$N {Ytells you 'You do not have enough trains to buy that skill.'\n\r{x", ch,NULL,trainer,TO_CHAR);
            return;
        }
        
        ch->questpoints-=skill_table[sn].legendQPCost;
        ch->gold-=skill_table[sn].legendGoldCost;
        ch->train-=skill_table[sn].legendTrainCost;
 
        /* add the skill */
	    ch->pcdata->learned[sn] = 75;
        act("\n\r{W$N {ctrains you in the art of {C$t{x\n\r",
            ch,skill_table[sn].name,trainer,TO_CHAR);
        return;
    }
    
    if(!IS_LEGEND(ch))
    {
        /* else add a group/skill */
    
        gn = group_lookup(arg);
        if (gn > 0)
        {
        	if (ch->pcdata->group_known[gn])
        	{
        	    act("\n\r{W$N {Ytells you 'You already know that group!{x\n\r'",
        		ch,NULL,trainer,TO_CHAR);
        	    return;
        	}
        
        	if (group_table[gn].rating[ch->class] <= 0)
        	{
                act("\n\r{W$N {Ytells you 'That group is beyond your powers.'{x\n\r",
        		ch,NULL,trainer,TO_CHAR);
        	    return;
        	}
        
        	if (ch->train < group_table[gn].rating[ch->class])
        	{
                act("\n\r{W$N {Ytells you 'You are not yet ready for that group.'\n\r{x",
        		ch,NULL,trainer,TO_CHAR);
        	    return;
        	}
        
        	/* add the group */
        	gn_add(ch,gn);
        	act("\n\r{W$N {ctrains you in the art of {C$t{x\n\r",
        	    ch,group_table[gn].name,trainer,TO_CHAR);
        	ch->train -= group_table[gn].rating[ch->class];
        	return;
        }
    
        sn = skill_lookup(arg);
        if (sn > -1)
        {
        	if (skill_table[sn].spell_fun != spell_null)
        	{
        	    act("\n\r{W$N {Ytells you 'You must learn the full group.'{x\n\r",
        		ch,NULL,trainer,TO_CHAR);
        	    return;
        	}
    	    
    
            if (ch->pcdata->learned[sn])
            {
                act("\n\r{W$N {Ytells you 'You already know that skill!'{x\n\r",
                    ch,NULL,trainer,TO_CHAR);
                return;
            }
     
            if (skill_table[sn].rating[ch->class] <= 0)
            {
                act("\n\r{W$N {Ytells you 'That skill is beyond your powers.'\n\r{x",
                    ch,NULL,trainer,TO_CHAR);
                return;
            }
     
            if (ch->train < skill_table[sn].rating[ch->class])
            {
                act("\n\r{W$N {Ytells you 'You are not yet ready for that skill.'\n\r{x",
                    ch,NULL,trainer,TO_CHAR);
                return;
            }
     
            /* add the skill */
    	    ch->pcdata->learned[sn] = 1;
            act("\n\r{W$N {ctrains you in the art of {C$t{x\n\r",
                ch,skill_table[sn].name,trainer,TO_CHAR);
            ch->train -= skill_table[sn].rating[ch->class];
            return;
        }
    }

    act("\n\r{W$N {Ytells you 'I do not understand...'\n\r{x",ch,NULL,trainer,TO_CHAR);
}
    



/* RT spells and skills show the players spells (or skills) */

void do_spells(CHAR_DATA *ch, char *argument)
{
    int sn, level, mana, col = 0;
    bool was_printed[ MAX_SKILL ];
    bool found = FALSE;
    bool pSpell = 0;
    char buf[MAX_STRING_LENGTH];
 
    if (IS_NPC(ch))
      return;

    for ( sn = 0; sn < MAX_SKILL; sn++ )
      was_printed[sn] = FALSE;
    for ( level = 1; level < LEVEL_IMMORTAL; level++ )
    {
    col = 0;
    pSpell = TRUE;
    for (sn = 0; sn < MAX_SKILL; sn++)
    	{
        if (skill_table[sn].name == NULL )
	    break;


	if ( ch->pcdata->oldcl == -1 ) 
          {
          if ( ch->pcdata->learned[sn] <= 0
          || skill_table[sn].skill_level[ch->class] != level
          || skill_table[sn].spell_fun == spell_null )
            continue;
          }
        else
          {
          if ( ch->pcdata->learned[sn] <= 0
          ||   skill_table[sn].spell_fun == spell_null
          || ( skill_table[sn].skill_level[ch->class] != level
          &&   skill_table[sn].skill_level[ch->pcdata->oldcl] != level ) )
            continue;
         }

	if ( was_printed[sn] )
	  continue;
	was_printed[sn] = TRUE;
	if ( !found )
	  {
	  send_to_char( "\n\r\n\r{cLevel  Spells\n\r\n\r{x", ch );
	  found = TRUE;
	  }
	if ( pSpell )
	  {
	  sprintf( buf, "{r[{W%3d{r]", level );
	  send_to_char( buf, ch );
	  pSpell = FALSE;
	  }
	if ( (++col-1) % 2 == 0 && col != 1 )
	  send_to_char( "     ", ch );
	if (ch->level < level)
	  sprintf(buf,"{r[{W%-16.16s{r][{C n/a {r]",skill_table[sn].name);
	else
	  {
	  mana = MANA_COST( ch, sn );

	  sprintf(buf,"{r[{W%-16.16s{r][{W%3d {RMN{r][{W%3d{R%%{r]",skill_table[sn].name, mana,
ch->pcdata->learned[sn]);
	  }
	send_to_char( buf, ch );
	if ( col % 2 == 0 )
	  send_to_char( "{x\n\r", ch );
	}
    if ( col % 2 != 0 )
      send_to_char( "{x\n\r", ch );
    }
    if (!found)
    {
      	send_to_char("\n\r\n\r{BNo spells found!!\n\r\n\r{x",ch);
      	return;
    }
    return;
}


void do_skills(CHAR_DATA *ch, char *argument)
{
    int sn;
    BUFFER * buffer;
    char buf[MAX_STRING_LENGTH];
    char ** skills=0;
    bool trained=FALSE;
    char ** ar=0;
    char ** cols=0;
    int i;
    
    if (IS_NPC(ch))
      return;

    for (sn = 0; sn < MAX_SKILL; sn++)
    {
        if (skill_table[sn].name == NULL)
            break;
        if(skill_table[sn].legendLevel==-1)
            continue;

        if (ch->pcdata->learned[sn]
        &&  (skill_table[sn].rating[ch->class] > 0 || IS_LEGEND(ch))
    &&  skill_table[sn].spell_fun == spell_null )
        {
            if(IS_LEGEND(ch))
                sprintf(buf, "%s", skill_table[sn].name);
            else
                sprintf(buf, "%d,%s", skill_table[sn].skill_level[ch->class], skill_table[sn].name);
            
            skills = array_append(skills, buf);            
        }
    }

    array_sort(skills);
    //qsort(skills, array_len(skills), sizeof(char *), gain_skills_array_sort);
    buffer = new_buf();
       
    if(IS_LEGEND(ch))
        sprintf(buf, "\n\r{D[{R%-20s{D][{R %%  {D]{x\n\r", "Skill Name" );
    else
        sprintf(buf, "\n\r{D [{R%-16s{D][{RLv{D][{R %%  {D]{x\n\r", "Skill Name" );
    add_buf(buffer, buf);
       
    for(ar=skills;ar && *ar;ar++)
    {
        cols = strchrexplode(*ar, ',');

        if(IS_LEGEND(ch))
           sprintf(buf,"{D[{W%-20s{D][{W%3d%%{D]{x\n\r",
                   cols[0], ch->pcdata->learned[skill_lookup(cols[0])]);
        else
           sprintf(buf,"{D [{W%-16s{D][{W%2d{D][{W%2d{D]{x\n\r",
                   cols[1], atoi(cols[0]), ch->pcdata->learned[skill_lookup(cols[1])]);
         add_buf(buffer, buf);
        
        cols = array_free(cols);

    }
    skills = array_free(skills);        

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
        add_buf(buffer, buf);
    }

            
    page_to_char(buf_string(buffer), ch);
    free_buf(buffer);
    
    return;
}


void do_remskill(CHAR_DATA *ch, char *argument)
{
    int sn, level, col = 0;
    bool was_printed[ MAX_SKILL ];
    bool found = FALSE;
    bool pSpell;
    char arg1[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim; 

    argument = one_argument(argument,arg1);



    if ( arg1[0] == '\0' )
    {
        send_to_char( "\n\r{GSyntax{w: {WREMSKILL {c<{WCHAR NAME{c>{x{x\n\r", ch );
        return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
        send_to_char( "\n\r{RThey aren't here.{x\n\r", ch );
        return;
    }

    if ((IS_IMP(victim))
    && (!IS_IMP(ch)))
    {
        send_to_char( "\n\r{RNo descriptor data found.{x\n\r", ch );
        return;
    }

    if (IS_NPC(ch))
      return;

    if (IS_NPC(victim))
      return;


    for ( sn = 0; sn < MAX_SKILL; sn++ )
      was_printed[sn] = FALSE;
    for ( level = 1; level < LEVEL_IMMORTAL; level++ )
    {
    col = 0;
    pSpell = TRUE;
    for (sn = 0; sn < MAX_SKILL; sn++)
    	{
        if (skill_table[sn].name == NULL )
	    break;


if ( victim->pcdata->oldcl == -1 )
          {
          if ( victim->pcdata->learned[sn] <= 0
          || skill_table[sn].skill_level[victim->class] != level
          || skill_table[sn].spell_fun != spell_null )
            continue;
          } 
        else
          {
          if ( victim->pcdata->learned[sn] <= 0
          ||   skill_table[sn].spell_fun != spell_null
          || ( skill_table[sn].skill_level[victim->class] != level
          &&   skill_table[sn].skill_level[victim->pcdata->oldcl] != level ) )
            continue;
          }

	if ( was_printed[sn] )
	  continue;
	was_printed[sn] = TRUE;
	if ( !found )
	  {
    sprintf(buf,"\n\r{GRemort SkillStat Target{w: {W%s   {GOLD Class{w: {W%s{x\n\r",
victim->name,capitalize(class_table[victim->pcdata->oldcl].name));
    send_to_char(buf,ch);
	  found = TRUE;
	  }
	if ( pSpell )
	  {
	  sprintf( buf, "{b[{W%3d{b]", level );
	  send_to_char( buf, ch );
	  pSpell = FALSE;
	  }
	if ( ( ++col % 4 == 0 && (col-1) % 3 != 0 && col < 7 )
	|| (col > 3 && (col-1) % 3 == 0) )
	  send_to_char( "     ", ch );
	if (victim->level < level)
	  sprintf(buf,"{c[{C%15s{c] {y[{W n/a{y]", skill_table[sn].name);
	else
	  {
	  sprintf(buf,"{D[{C%15s{D]{c[{W%3d{R%%{c]",skill_table[sn].name,
		  ch->pcdata->learned[sn] );
	  }
	send_to_char( buf, ch );
	if ( col % 3 == 0 )
	  send_to_char( "{x\n\r", ch );
	}
    if ( col % 3 != 0 )
      send_to_char( "{x\n\r", ch );
    }
    if (!found)
    {
      	send_to_char("\n\r\n\r{RNo skills found!\n\r\n\r{x",ch);
      	return;
    }
    return;
}


/* shows skills, groups and costs (only if not bought) */

void list_group_costs(CHAR_DATA *ch)
{
    char buf[ MAX_STRING_LENGTH ];
    int gn,sn,col;

    if ( IS_NPC( ch ) )
	   return;

    col = 0;

    sprintf(buf,

"{D[{R%-16s{D][{RLv{D][{RCp{D][{R%-16s{D][{RLv{D][{RCp{D][{R%-16s{D][{RLv{D][{RCp{D]{x\n\r",
		 "Group Name", "Group Name", "Group Name" );
    send_to_char( buf, ch );
    buf[0] = '\0';
    for (gn = 0; gn < MAX_GROUP; gn++)
    {
	if (group_table[gn].name == NULL)
	    break;
	
	  if (!ch->pcdata->group_known[gn]
	  &&  group_table[gn].rating[ch->class] > 0)
	    {
            sprintf( buf + strlen( buf ), 
"{D[{W%-16s{D][  ][{W%2d{D]{x%s",group_table[gn].name, group_table[gn].rating[ch->class],
		     ((col+1)%3==0) ? "\n\r" : "" );
            col++;
 	    }

    }
    if (col % 3 != 0)
      strcat( buf, "\n\r" );
    send_to_char( buf, ch );
    send_to_char( "\n\r", ch );

    col = 0;

    sprintf(buf,

"{D[{R%-16s{D][{RLv{D][{RCp{D][{R%-16s{D][{RLv{D][{RCp{D][{R%-16s{D][{RLv{D][{RCp{D]{x\n\r",
		 "Skill Name", "Skill Name", "Skill Name" );
    send_to_char( buf, ch );
    buf[0] = '\0';
    for (sn = 0; sn < MAX_SKILL; sn++)
    {
        if (skill_table[sn].name == NULL)
            break;

	  if ( ch->pcdata->learned[sn] == 0
	  &&  skill_table[sn].spell_fun == spell_null
	  &&  skill_table[sn].rating[ch->class] > 0)
	    {
            sprintf( buf + strlen( buf ),"{D[{W%-16s{D][{W%2d{D][{W%2d{D]{x%s",
		     skill_table[sn].name,
         skill_table[sn].skill_level[ch->class],
                     skill_table[sn].rating[ch->class],
                     ((col+1)%3==0) ? "\n\r" : "" );
            col++;
 	    }

    }
/*
    if (col % 3 != 0)
      strcat( buf, "\n\r" );
    send_to_char( buf, ch );
    send_to_char( "\n\r", ch );
    return;
*/

    if (col % 3 != 0)
      strcat( buf, "\n\r" );
    send_to_char( buf, ch );
    send_to_char( "\n\r", ch );
    sprintf(buf,"{RCreation points{D: {W%d{x\n\r",ch->pcdata->points);
    send_to_char(buf,ch);
    sprintf(buf,"{RExperience per level{D: {W%d{x\n\r",
	    exp_per_level(ch,ch->gen_data->points_chosen));
    send_to_char(buf,ch);
    return;
}


void list_group_chosen( CHAR_DATA *ch )
{
    char buf[ MAX_STRING_LENGTH ];
    int gn,sn,col;

    if ( IS_NPC( ch ) )
      return;

    col = 0;
    sprintf( buf,

"{D[{R%-16s{D][{RLv{D][{RCp{D][{R%-16s{D][{RLv{D][{RCp{D][{R%-16s{D][{RLv{D][{RCp{D]{x\n\r",
		 "Group Name", "Group Name", "Group Name" );
    send_to_char( buf, ch );
    buf[0] = '\0';
    for (gn = 0; gn < MAX_GROUP; gn++)
    {
        if (group_table[gn].name == NULL)
            break;

        if (ch->gen_data->group_chosen[gn]
	&&  group_table[gn].rating[ch->class] > 0)
        {
            sprintf( buf + strlen( buf ), "{D[{W%-16s{D][{W%-6d{D]{x%s",
		     group_table[gn].name, group_table[gn].rating[ch->class],
                     ((col+1)%3==0) ? "\n\r" : "" );
            col++;
        }
    }
    if (col % 3 != 0)
      strcat( buf, "\n\r" );
    send_to_char( buf, ch );
    send_to_char( "\n\r", ch );

    col = 0;

    sprintf(buf,
"{D[{R%-16s{D][{RLv{D][{RCp{D][{R%-16s{D][{RLv{D][{RCp{D][{R%-16s{D][{RLv{D][{RCp{D]{x\n\r",
		 "Skill Name", "Skill Name", "Skill Name" );
    send_to_char( buf, ch );
    buf[0] = '\0';
    for (sn = 0; sn < MAX_SKILL; sn++)
    {
        if (skill_table[sn].name == NULL)
            break;

        if (ch->gen_data->skill_chosen[sn]
	&&  skill_table[sn].rating[ch->class] > 0)
        {
            sprintf( buf + strlen( buf ),"{D[{W%-16s{D][{W%2d{D][{W%2d{D]{x%s",
		     skill_table[sn].name,
         skill_table[sn].skill_level[ch->class],
                     skill_table[sn].rating[ch->class],
                     ((col+1)%3==0) ? "\n\r" : "" );
            col++;
        }
    }
    if (col % 3 != 0)
      strcat( buf, "\n\r" );
    send_to_char( buf, ch );
    send_to_char( "\n\r", ch );
    sprintf(buf,"{RCreation points{D: {W%d{x\n\r",ch->gen_data->points_chosen);
    send_to_char(buf,ch);
    sprintf(buf,"{RExperience per level{D: {W%d{x\n\r",exp_per_level(ch,ch->gen_data->points_chosen));
    send_to_char(buf,ch);
    return;
}


int exp_per_level(CHAR_DATA *ch, int points)
{
    int expl,inc;

    if (IS_NPC(ch))
	return 1000; 

    expl = 1000;
    inc = 500;

if (points < 40)
    {
     expl = 1000 * (pc_race_table[ch->race].class_mult[ch->class] ?
                    pc_race_table[ch->race].class_mult[ch->class]/100 : 1);
     if ( ch->pcdata->oldcl != -1 )
       expl *= 2;
     return expl;
    }


    /* processing */
    points -= 40;

    while (points > 9)
    {
	expl += inc;
        points -= 10;
        if (points > 9)
	{
	    expl += inc;
	    inc *= 2;
	    points -= 10;
	}
    }

    expl += points * inc / 10;  
    expl *= pc_race_table[ch->race].class_mult[ch->class];
    expl /= 100;
    if ( ch->pcdata->oldcl != -1 )
      expl *= 2;
    return expl;
}

/* this procedure handles the input parsing for the skill generator */
bool parse_gen_groups(CHAR_DATA *ch,char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    char buf[100];
    int gn,sn,i;
 
    if (argument[0] == '\0')
	return FALSE;

    argument = one_argument(argument,arg);

    if (!str_prefix(arg,"help"))
    {
	if (argument[0] == '\0')
	{
	    do_function(ch, &do_help, "group help");

	    return TRUE;
	}

        do_function(ch, &do_help, argument);
	return TRUE;
    }

    if (!str_prefix(arg,"add"))
    {
	if (argument[0] == '\0')
	{
	    send_to_char("\n\r{RYou must provide a skill name.{x\n\r",ch);
	    return TRUE;
	}

	gn = group_lookup(argument);
	if (gn != -1)
	{
	    if (ch->gen_data->group_chosen[gn]
	    ||  ch->pcdata->group_known[gn])
	    {
		send_to_char("{RYou already know that group!{x\n\r",ch);
		return TRUE;
	    }

	    if (group_table[gn].rating[ch->class] < 1)
	    {
	  	send_to_char("{GThat group is not available.{x\n\r",ch);
	 	return TRUE;
	    }

	    /* Close security hole */
	    if (ch->gen_data->points_chosen + group_table[gn].rating[ch->class]
		> 150)
	    {
		send_to_char("\n\r{WYou cannot take more than {R150{W creation points.{x\n\r", ch);
		return TRUE;
	    }

	    sprintf(buf,"{W%s {Rgroup added\n\r\n\r{x",group_table[gn].name);
	    send_to_char(buf,ch);
	    ch->gen_data->group_chosen[gn] = TRUE;
	    ch->gen_data->points_chosen += group_table[gn].rating[ch->class];
	    gn_add(ch,gn);
	    ch->pcdata->points += group_table[gn].rating[ch->class];
	    return TRUE;
	}

	sn = skill_lookup(argument);
	if (sn != -1)
	{
	    if (ch->gen_data->skill_chosen[sn]
	    ||  ch->pcdata->learned[sn] > 0)
	    {
		send_to_char("\n\r{WYou already know that skill!{x\n\r",ch);
		return TRUE;
	    }

	    if (skill_table[sn].rating[ch->class] < 1
	    ||  skill_table[sn].spell_fun != spell_null)
	    {
		send_to_char("\n\r{WThat skill is not available.{x\n\r",ch);
		return TRUE;
	    }

	    /* Close security hole */
	    if (ch->gen_data->points_chosen + skill_table[sn].rating[ch->class]
		> 150)
	    {
		send_to_char(
		    "\n\r{WYou cannot take more than {R150 {Wcreation points.{x\n\r",ch);
		return TRUE;
	    }
	    sprintf(buf, "{W%s {Rskill added{x\n\r\n\r",skill_table[sn].name);
	    send_to_char(buf,ch);
	    ch->gen_data->skill_chosen[sn] = TRUE;
	    ch->gen_data->points_chosen += skill_table[sn].rating[ch->class];
	    ch->pcdata->learned[sn] = 1;
	    ch->pcdata->points += skill_table[sn].rating[ch->class];
	    return TRUE;
	}

	send_to_char("\n\r{RNo skills or groups by that name...{x\n\r",ch);
	return TRUE;
    }

    if (!strcmp(arg,"drop"))
    {

    if (ch->pcdata->oldcl != -1)
      {
        send_to_char("\n\r{RREMORTS are unable to {WDROP{R groups temporarily, we hope to have this{x\n\r",ch);
        send_to_char("{Rfixed soon.  If you have made a mistake, you will get one more chance to{x\n\r",ch);
        send_to_char("{Rredo this.  Just complete this process as fast as possible and tell the{x\n\r",ch);
        send_to_char("{RIMP you would like to have a second chance, remember you only get one.{x\n\r\n\r\n\r",ch);
        return TRUE;
      }


	if (argument[0] == '\0')
  	{
	    send_to_char("\n\r{RYou must provide a skill to drop.{x\n\r",ch);
	    return TRUE;
	}

	gn = group_lookup(argument);
	if (gn != -1 && ch->gen_data->group_chosen[gn])
	{
	    send_to_char("\n\r{GGroup dropped.{x\n\r",ch);
	    ch->gen_data->group_chosen[gn] = FALSE;
	    ch->gen_data->points_chosen -= group_table[gn].rating[ch->class];
	    gn_remove(ch,gn);
	    for (i = 0; i < MAX_GROUP; i++)
	    {
		if (ch->gen_data->group_chosen[gn])
		    gn_add(ch,gn);
	    }
	    ch->pcdata->points -= group_table[gn].rating[ch->class];
	    return TRUE;
	}

	sn = skill_lookup(argument);
	if (sn != -1 && ch->gen_data->skill_chosen[sn])
	{
	    send_to_char("\n\r{GSkill dropped.{x\n\r\n\r",ch);
	    ch->gen_data->skill_chosen[sn] = FALSE;
	    ch->gen_data->points_chosen -= skill_table[sn].rating[ch->class];
	    ch->pcdata->learned[sn] = 0;
	    ch->pcdata->points -= skill_table[sn].rating[ch->class];
	    return TRUE;
	}

	send_to_char("{RYou haven't bought any such skill or group.{x\n\r",ch);
	return TRUE;
    }

    if (!str_prefix(arg,"premise"))
    {
	do_function(ch, &do_help, "premise");
	return TRUE;
    }

    if (!str_prefix(arg,"list"))
    {
	list_group_costs(ch);
	return TRUE;
    }

    if (!str_prefix(arg,"learned"))
    {
	list_group_chosen(ch);
	return TRUE;
    }

    if (!str_prefix(arg,"info"))
    {
	do_function(ch, &do_groups, argument);
	return TRUE;
    }

    return FALSE;
}
	    
	


        

/* shows all groups, or the sub-members of a group */
void do_groups(CHAR_DATA *ch, char *argument)
{
  char buf[ MAX_STRING_LENGTH ];
  char LvStr[ 3 ];
  char CpStr[ 3 ];
  int gn,sn,col,spsn,grgn;

  if ( IS_NPC( ch ) )
    return;

  col = 0;

  if (argument[0] == '\0' || !str_cmp(argument,"all"))
    { /* show all groups */
    send_to_char( "\n\r{cAll groups{w:\n\r", ch );
    sprintf(buf,
    "{D[{R%-16s{D][{RLv{D][{RCp{D][{R%-16s{D][{RLv{D][{RCp{D][{R%-16s{D][{RLv{D][{RCp{D]\n\r",
		 "Group Name", "Group Name", "Group Name" );
    send_to_char( buf, ch );
    buf[0] = '\0';
    for (gn = 0; gn < MAX_GROUP; gn++)
      {
      if (group_table[gn].name == NULL)
	break;
      if (group_table[gn].rating[ch->class] > 0 )
	{
	sprintf(buf+strlen(buf),"{D[{W%-16s{D][  ][{W%2d{D]%s",
  			group_table[gn].name, group_table[gn].rating[ch->class],
		(col+1)%3==0 ? "\n\r" : "" );
	col++;
	}
      }
    if (col % 3 != 0)
      strcat( buf, "\n\r{x" );
    send_to_char( buf, ch );
    if ( argument[0] == '\0' )
      {
      sprintf(buf,"\n\r{cCreation {Rpoints{w: {W%d{w\n\r",ch->pcdata->points);
      send_to_char( buf, ch );
      }
    return;
    }

   /* show the sub-members of a group */
  gn = group_lookup(argument);
  if (gn == -1)
    {
    send_to_char("\n\r{cNo group of that name exist{w.{x\n\r",ch);
    return;
    }
  sprintf(buf,"\n\r{cAll spells in group {C%s{w:{x\n\r", group_table[gn].name
);
  send_to_char( buf, ch );
  sprintf(buf,
    "{D[{R%-16s{D][{RLv{D][{RCp{D][{R%-16s{D][{RLv{D][{RCp{D][{R%-16s{D][{RLv{D][{RCp{D]\n\r",
		 "Spell Name", "Spell Name", "Spell Name" );
    send_to_char( buf, ch );
  buf[0] = '\0';
  for (sn = 0; sn < MAX_IN_GROUP; sn++)
    {
    if (group_table[gn].spells[sn] == NULL)
      break;
    spsn = skill_lookup( group_table[gn].spells[sn] );
    grgn = group_lookup( group_table[gn].spells[sn] );
    strcpy( LvStr, "  " );
    strcpy( CpStr, "  " );
    if ( spsn == -1 )  /* it's a group */
      {
      if ( group_table[gn].rating[ch->class] > 0 )
	sprintf( CpStr, "%2d", group_table[grgn].rating[ch->class] );
      else
	sprintf( CpStr, "NA" );
      }
    else /* skill or spell */
      {
      if ( group_table[gn].rating[ch->class] > 0 )
        sprintf( LvStr, "%2d", skill_table[spsn].skill_level[ch->class] );
      else
    	sprintf( LvStr, "NA" );
      }

    sprintf( buf + strlen( buf ), "{D[{W%-16s{D][{W%2s{D][{W%2s{D]%s{x",
        group_table[gn].spells[sn], LvStr, CpStr,
        ((col+1)%3==0) ? "\n\r" : "" );
    col++;
    }
  if (col % 3 != 0)
    strcat( buf, "\n\r{x" );
  send_to_char( buf, ch );
  return;
}



/* checks for skill improvement */
void check_improve( CHAR_DATA *ch, int sn, bool success, int multiplier )
{
    int chance;
    char buf[100];

    if (IS_NPC(ch))
	return;

    if(skill_table[sn].rating[ch->class]!=0)
    {
        if (ch->level < skill_table[sn].skill_level[ch->class]
            ||  skill_table[sn].rating[ch->class] == 0
            ||  ch->pcdata->learned[sn] == 0
            ||  ch->pcdata->learned[sn] == 100)
    	   return;  /* skill is not known */ 
    
        /* check to see if the character has a chance to learn */
        chance = 10 * int_app[get_curr_stat(ch,STAT_INT)].learn;
        chance /= (		multiplier
    		*	skill_table[sn].rating[ch->class] 
    		*	4);
        chance += ch->level;
    
        if (number_range(1,1000) > chance)
    	   return;
    
        /* now that the character has a CHANCE to learn, see if they really have */	
    
        if (success)
        {
        	chance = URANGE(5,100 - ch->pcdata->learned[sn], 95);
        	if (number_percent() < chance)
        	{
        	    sprintf(buf,"You have become better at %s!\n\r",
        		    skill_table[sn].name);
        	    send_to_char(buf,ch);
        	    ch->pcdata->learned[sn]++;
        	    gain_exp(ch,2 * skill_table[sn].rating[ch->class]);
        	}
        }
    
        else
        {
        	chance = URANGE(5,ch->pcdata->learned[sn]/2,30);
        	if (number_percent() < chance)
        	{
        	    sprintf(buf,
        		"{cYou learn from your mistakes, and your {C%s {cskill improves.{x\n\r",
        		skill_table[sn].name);
        	    send_to_char(buf,ch);
        	    ch->pcdata->learned[sn] += number_range(1,3);
        	    ch->pcdata->learned[sn] = UMIN(ch->pcdata->learned[sn],100);
        	    gain_exp(ch,2 * skill_table[sn].rating[ch->class]);
        	}
        }    
    }
    else if(ch->pcdata->oldcl != -1 && skill_table[sn].rating[ch->pcdata->oldcl]!=0)
    {
        if (ch->level < skill_table[sn].skill_level[ch->pcdata->oldcl]
            ||  skill_table[sn].rating[ch->pcdata->oldcl] == 0
            ||  ch->pcdata->learned[sn] == 0
            ||  ch->pcdata->learned[sn] == 100)
    	   return;  /* skill is not known */ 
    
        /* check to see if the character has a chance to learn */
        chance = 10 * int_app[get_curr_stat(ch,STAT_INT)].learn;
        chance /= (		multiplier
    		*	skill_table[sn].rating[ch->pcdata->oldcl] 
    		*	4);
        chance += ch->level;
    
        if (number_range(1,1000) > chance)
    	   return;
    
        /* now that the character has a CHANCE to learn, see if they really have */	
    
        if (success)
        {
        	chance = URANGE(5,100 - ch->pcdata->learned[sn], 95);
        	if (number_percent() < chance)
        	{
        	    sprintf(buf,"You have become better at %s!\n\r",
        		    skill_table[sn].name);
        	    send_to_char(buf,ch);
        	    ch->pcdata->learned[sn]++;
        	    gain_exp(ch,2 * skill_table[sn].rating[ch->pcdata->oldcl]);
        	}
        }
    
        else
        {
        	chance = URANGE(5,ch->pcdata->learned[sn]/2,30);
        	if (number_percent() < chance)
        	{
        	    sprintf(buf,
        		"{cYou learn from your mistakes, and your {C%s {cskill improves.{x\n\r",
        		skill_table[sn].name);
        	    send_to_char(buf,ch);
        	    ch->pcdata->learned[sn] += number_range(1,3);
        	    ch->pcdata->learned[sn] = UMIN(ch->pcdata->learned[sn],100);
        	    gain_exp(ch,2 * skill_table[sn].rating[ch->pcdata->oldcl]);
        	}
        }    
    }
}

/* returns a group index number given the name */
int group_lookup( const char *name )
{
    int gn;
 
    for ( gn = 0; gn < MAX_GROUP; gn++ )
    {
        if ( group_table[gn].name == NULL )
            break;
        if ( LOWER(name[0]) == LOWER(group_table[gn].name[0])
        &&   !str_prefix( name, group_table[gn].name ) )
            return gn;
    }
 
    return -1;
}

/* recursively adds a group given its number -- uses group_add */
void gn_add( CHAR_DATA *ch, int gn)
{
    int i;
    
    ch->pcdata->group_known[gn] = TRUE;
    for ( i = 0; i < MAX_IN_GROUP; i++)
    {
        if (group_table[gn].spells[i] == NULL)
            break;
        group_add(ch,group_table[gn].spells[i],FALSE);
    }
}

/* recusively removes a group given its number -- uses group_remove */
void gn_remove( CHAR_DATA *ch, int gn)
{
    int i;

    ch->pcdata->group_known[gn] = FALSE;

    for ( i = 0; i < MAX_IN_GROUP; i ++)
    {
	if (group_table[gn].spells[i] == NULL)
	    break;
	group_remove(ch,group_table[gn].spells[i]);
    }
}
	
/* use for processing a skill or group for addition  */
void group_add( CHAR_DATA *ch, const char *name, bool deduct)
{
    int sn,gn;

    if (IS_NPC(ch)) /* NPCs do not have skills */
	return;

    sn = skill_lookup(name);

    if (sn != -1)
    {
	if (ch->pcdata->learned[sn] == 0) /* i.e. not known */
	{
	    ch->pcdata->learned[sn] = 1;
	    if (deduct)
	   	ch->pcdata->points += skill_table[sn].rating[ch->class]; 
	}
	return;
    }
	
    /* now check groups */

    gn = group_lookup(name);

    if (gn != -1)
    {
	if (ch->pcdata->group_known[gn] == FALSE)  
	{
	    ch->pcdata->group_known[gn] = TRUE;
	    if (deduct)
		ch->pcdata->points += group_table[gn].rating[ch->class];
	}
	gn_add(ch,gn); /* make sure all skills in the group are known */
    }
}

/* used for processing a skill or group for deletion -- no points back! */

void group_remove(CHAR_DATA *ch, const char *name)
{
    int sn, gn;
    
     sn = skill_lookup(name);

    if (sn != -1)
    {
	ch->pcdata->learned[sn] = 0;
	return;
    }
 
    /* now check groups */
 
    gn = group_lookup(name);
 
    if (gn != -1 && ch->pcdata->group_known[gn] == TRUE)
    {
	ch->pcdata->group_known[gn] = FALSE;
	gn_remove(ch,gn);  /* be sure to call gn_add on all remaining groups */
    }
}

bool can_use_skpell( CHAR_DATA *ch, int sn )
{
  if ( IS_NPC( ch ) || IS_LEGEND(ch) )
    return TRUE;
  if ( ch->level >= skill_table[sn].skill_level[ch->class] || ( ch->pcdata->oldcl != -1 && ch->level >=skill_table[sn].skill_level[ch->pcdata->oldcl]) )
    return TRUE;
   return FALSE;
}

void do_pick( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *gch;
    OBJ_DATA *obj;
    int door;

    one_argument( argument, arg );

    if ( !IS_NPC(ch) 
    &&   !can_use_skpell( ch, gsn_pick_lock ) )      
      {
        send_to_char( "\n\r{GPicking locks?  Anyone can pick a lock... right?{x\n\r",ch);
        return;    
      }


    if ( arg[0] == '\0' )
    {
	send_to_char( "{WPick what?{x\n\r", ch );
	return;
    }

    WAIT_STATE( ch, skill_table[gsn_pick_lock].beats );

    /* look for guards */
    for ( gch = ch->in_room->people; gch; gch = gch->next_in_room )
    {
	if ( IS_NPC(gch) && IS_AWAKE(gch) && ch->level + 5 < gch->level )
	{
	    act( "$N {cis standing too close to the lock.{x",
		ch, NULL, gch, TO_CHAR );
	    return;
	}
    }

    if ( !IS_NPC(ch) && number_percent( ) > get_skill(ch,gsn_pick_lock))
    {
	send_to_char( "{RYou failed.{x\n\r", ch);
	check_improve(ch,gsn_pick_lock,FALSE,2);
	return;
    }

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
	/* portal stuff */
	if (obj->item_type == ITEM_PORTAL)
	{
	    if (!IS_SET(obj->value[1],EX_ISDOOR))
	    {	
		send_to_char("{RYou can't do that.{x\n\r",ch);
		return;
	    }

	    if (!IS_SET(obj->value[1],EX_CLOSED))
	    {
		send_to_char("{RIt's not closed.{x\n\r",ch);
		return;
	    }

	    if (obj->value[4] < 0)
	    {
		send_to_char("{RIt can't be unlocked.{x\n\r",ch);
		return;
	    }

	    if (IS_SET(obj->value[1],EX_PICKPROOF))
	    {
		send_to_char("{RYou failed.{x\n\r",ch);
		return;
	    }

	    REMOVE_BIT(obj->value[1],EX_LOCKED);
	    act("{cYou pick the lock on {C$p{c.{x",ch,obj,NULL,TO_CHAR);
	    act("{W$n {cpicks the lock on {C$p{c.{x",ch,obj,NULL,TO_ROOM);
	    check_improve(ch,gsn_pick_lock,TRUE,2);
	    return;
	}

	    


	
	/* 'pick object' */
	if ( obj->item_type != ITEM_CONTAINER )
	    { send_to_char( "{RThat's not a container.{x\n\r", ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_CLOSED) )
	    { send_to_char( "{RIt's not closed.{x\n\r",        ch ); return; }
	if ( obj->value[2] < 0 )
	    { send_to_char( "{RIt can't be unlocked.{x\n\r",   ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_LOCKED) )
	    { send_to_char( "{WIt's already unlocked.{x\n\r",  ch ); return; }
	if ( IS_SET(obj->value[1], CONT_PICKPROOF) )
	    { send_to_char( "{RYou failed.{x\n\r",             ch ); return; }

	REMOVE_BIT(obj->value[1], CONT_LOCKED);
        act("{cYou pick the lock on {C$p{c.{x",ch,obj,NULL,TO_CHAR);
        act("{W$n {cpicks the lock on {C$p{c.{x",ch,obj,NULL,TO_ROOM);
	check_improve(ch,gsn_pick_lock,TRUE,2);
	return;
    }

    if ( ( door = find_door( ch, arg ) ) >= 0 )
    {
	/* 'pick door' */
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;

	pexit = ch->in_room->exit[door];
	if ( !IS_SET(pexit->exit_info, EX_CLOSED) && !IS_IMMORTAL(ch))
	    { send_to_char( "{RIt's not closed.{x\n\r",        ch ); return; }
	if ( pexit->key < 0 && !IS_IMMORTAL(ch))
	    { send_to_char( "{RIt can't be picked.{x\n\r",     ch ); return; }
	if ( !IS_SET(pexit->exit_info, EX_LOCKED) )
	    { send_to_char( "{WIt's already unlocked.{x\n\r",  ch ); return; }
	if ( IS_SET(pexit->exit_info, EX_PICKPROOF) && !IS_IMMORTAL(ch))
	    { send_to_char( "{RYou failed.{x\n\r",             ch ); return; }

	REMOVE_BIT(pexit->exit_info, EX_LOCKED);
	send_to_char( "{W*Click*{x\n\r", ch );
	act( "{W$n {cpicks the {C$d{c.{x", ch, NULL, pexit->keyword, TO_ROOM );
	check_improve(ch,gsn_pick_lock,TRUE,2);

	/* pick the other side */
	if ( ( to_room   = pexit->u1.to_room            ) != NULL
	&&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != NULL
	&&   pexit_rev->u1.to_room == ch->in_room )
	{
	    REMOVE_BIT( pexit_rev->exit_info, EX_LOCKED );
	}
    }

    return;
}


void do_sneak( CHAR_DATA *ch, char *argument )
{
    AFFECT_DATA af;
  ROOM_INDEX_DATA *location;
  
  location=NULL;
  

    if ( !IS_NPC(ch)
    &&   !can_use_skpell( ch, gsn_sneak ) )
      {
        send_to_char( "\n\r{COther than all your EQ banging around, you are moving pretty quietly.{x\n\r",ch);  
        return;
      }

  location = get_room_index(ROOM_VNUM_DRAGONPIT);
  
  if (ch->in_room == location)
    {
send_to_char( "\n\r{GWait until the DragonPIT begins so it will be fair...{x\n\r",ch); 
    return;
    }



    send_to_char( "\n\r{WYou attempt to move silently.{x\n\r", ch );

    if (IS_AFFECTED(ch,AFF_SNEAK))
        affect_strip( ch, gsn_sneak );

    if ( number_percent( ) < get_skill(ch,gsn_sneak))
    {
	check_improve(ch,gsn_sneak,TRUE,3);
	af.where     = TO_AFFECTS;
	af.type      = gsn_sneak;
	af.level     = ch->level; 
	af.duration  = ch->level;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = AFF_SNEAK;
	affect_to_char( ch, &af );
        send_to_char( "\n\r{WYou are moving silently.{x\n\r", ch );

    }
    else
        send_to_char( "\n\r{RYou failed to move silently!{x\n\r", ch );
	check_improve(ch,gsn_sneak,FALSE,3);

    return;
}



void do_hide( CHAR_DATA *ch, char *argument )
{
    AFFECT_DATA af;
  ROOM_INDEX_DATA *location;
  
  location=NULL;
  
    if ( !IS_NPC(ch)
    &&   !can_use_skpell( ch, gsn_hide ) )
      {
        send_to_char( "\n\r{WYou find a small rock to hide behind but something seems wrong...{x\n\r", ch);
        return;
      }

  location = get_room_index(ROOM_VNUM_DRAGONPIT);
  
  if (ch->in_room == location)
    {
send_to_char( "\n\r{GWait until the DragonPIT begins so it will be fair...{x\n\r",ch);
    return;        
    }

    send_to_char( "\n\r{cYou attempt to hide.{x\n\r", ch );

    if ( IS_AFFECTED(ch, AFF_HIDE) )
        affect_strip( ch, gsn_hide );


    if ( number_percent( ) < get_skill(ch,gsn_hide))
    {
	check_improve(ch,gsn_hide,TRUE,3);
	af.where     = TO_AFFECTS;
	af.type      = gsn_hide;
	af.level     = ch->level; 
	af.duration  = ch->level;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = AFF_HIDE;
	affect_to_char( ch, &af );
        send_to_char( "\n\r{WThe shadows are concealing you.{x\n\r", ch );
    }
    else
        send_to_char( "\n\r{RYou failed to HIDE in the shadows!{x\n\r", ch );
	check_improve(ch,gsn_hide,FALSE,3);

    return;
}  

void do_berserk( CHAR_DATA *ch, char *argument)
{
    int chance, hp_percent;
    int sn;
  ROOM_INDEX_DATA *location;
  
  location=NULL;
 
  location = get_room_index(ROOM_VNUM_DRAGONPIT);
  


  if (ch->level <= ASSTIMP)
    {
  if (ch->in_room == location)
    {
send_to_char( "\n\r{GWait until the DragonPIT begins so it will be fair...{x\n\r",ch);
    return;
    } 
     
    if (IS_AFFECTED(ch,AFF_BERSERK) || is_affected(ch,gsn_berserk)
    ||  is_affected(ch,skill_lookup("frenzy")))
    {
	send_to_char("You get a little madder.\n\r",ch);
	return;
    }

    if (IS_AFFECTED(ch,AFF_CALM))
    {
	send_to_char("You're feeling to mellow to berserk.\n\r",ch);
	return;
    }

    if (ch->mana < 50)
    {
	send_to_char("You can't get up enough energy.\n\r",ch);
	return;
    }
  }
 else
   chance -= 500;

if ((chance = get_skill(ch,gsn_berserk)) == 0
    ||  (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_BERSERK))
    ||  (!IS_NPC(ch)
    &&  !can_use_skpell( ch, gsn_berserk ) ))
    {
     send_to_char("\n\r{RYou turn red in the face, but nothing happens.{x\n\r",ch);
     return;
    }



    /* modifiers */

    /* fighting */
    if (ch->position == POS_FIGHTING)
	chance += 10;

    /* damage -- below 50% of hp helps, above hurts */
    hp_percent = 100 * (ch->hit/ch->max_hit);
    chance += 50 - hp_percent/2;


    if (number_percent() < chance)
    {
	AFFECT_DATA af;

	WAIT_STATE(ch,PULSE_VIOLENCE);
	ch->mana -= 50;
	ch->move /= 2;

	/* heal a little damage */
	ch->hit += ch->level * 2;
	ch->hit = UMIN(ch->hit,ch->max_hit);
        sn = gsn_berserk;

	send_to_char("Your pulse races as you are consumed by rage!\n\r",ch);
	act("$n gets a wild look in $s eyes.",ch,NULL,NULL,TO_ROOM);
	check_improve(ch,gsn_berserk,TRUE,1);

	af.where	= TO_AFFECTS;
	af.type		= sn;
	af.level	= ch->level;
	af.duration	= number_fuzzy(ch->level / 8);
	af.modifier	= UMAX(1,ch->level/5);
	af.bitvector 	= AFF_BERSERK;

	af.location	= APPLY_HITROLL;
	affect_to_char(ch,&af);

	af.location	= APPLY_DAMROLL;
	affect_to_char(ch,&af);

	af.modifier	= UMAX(10,10 * (ch->level/5));
	af.location	= APPLY_AC;
	affect_to_char(ch,&af);
    }

    else
    {
	WAIT_STATE(ch,3 * PULSE_VIOLENCE);
	ch->mana -= 25;
	ch->move /= 2;

	send_to_char("Your pulse speeds up, but nothing happens.\n\r",ch);
	check_improve(ch,gsn_berserk,FALSE,2);
    }

}

void do_bash( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int chance;

    one_argument(argument,arg);
 

    if ( (chance = get_skill(ch,gsn_bash)) == 0
    ||     (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_BASH))
    ||     (!IS_NPC(ch)
    &&      !can_use_skpell( ch, gsn_bash ) ))
         {
     send_to_char("\n\r{WBashing? What's that?{x\n\r",ch);
     return;
    }
 
    if (arg[0] == '\0')
    {
	victim = ch->fighting;
	if (victim == NULL)
	{
	    send_to_char("\n\r{RBut you aren't fighting anyone!{x\n\r",ch);
	    return;
	}
    }

    else if ((victim = get_char_room(ch,arg)) == NULL)
    {
	send_to_char("\n\r{RThey aren't here.{x\n\r",ch);
	return;
    }


  if (IS_NPC(victim))
    {
     if (IS_SET(victim->act, ACT_NOSPECATTK))
       {
        send_to_char(
"\n\r\n\r{GYour {WTARGET{G is wise to your tactics... Try using a different attack form!{x\n\r\n\r",ch);
        return;
       }
    }

   
      if (IS_SET(ch->act,ACT_PET)
      && (IS_NPC(ch)))
        {
     if (!IS_SET(ch->act, ACT_WARRIOR))
       {
        return;
       }
        }


    if (victim->position <= POS_SITTING)
    {
	act("\n\r{WYou'll have to let $M get back up first.{x",ch,NULL,victim,TO_CHAR);
	return;
    } 

    if(IS_SET(ch->in_room->room_flags, ROOM_SAFE))
    {
        send_to_char("\n\r{RNot in a SAFE room!{x\n\r",ch);
        return;
    }


    if (victim == ch)
    {
	send_to_char("\n\r{GYou try to bash your brains out, but fail.{x\n\r",ch);
	return;
    }

    if (is_safe(ch,victim))
	return;
     
    if ( IS_NPC(victim) && 
	victim->fighting != NULL && 
	!is_same_group(ch,victim->fighting))
    {
        send_to_char("\n\r{RKill stealing is not permitted.{x\n\r",ch);
        return;
    }

    if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim)
    {
	act("\n\r{CBut $N is your friend!{x\n\r",ch,NULL,victim,TO_CHAR);
	return;
    }

    if (IS_SET(victim->imm_flags,IMM_BASH))
    {
        send_to_char("\n\r{GYour victim can not be BASHED!!{x\n\r",ch);
        return;
    }



/* Check if players are CHOOSEN */
if (!IS_NPC(victim))
{
if (!IS_NPC(ch))
  {
   if (ch->level < MAX_LEVEL)
     {
      if (!IS_SET(victim->in_room->room_flags, ROOM_DRAGONPIT) && !IS_SET(victim->pact, PLR_DRAGONPIT))
        {
         if (!IS_SET(ch->pact, PLR_DRAGONPIT))
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
  }}

    /* modifiers */

    /* size  and weight */
    chance += ch->carry_weight / 250;
    chance -= victim->carry_weight / 200;

    if (ch->size < victim->size)
	chance += (ch->size - victim->size) * 15;
    else
	chance += (ch->size - victim->size) * 10; 


    /* stats */
    chance += get_curr_stat(ch,STAT_STR);
    chance -= (get_curr_stat(victim,STAT_DEX) * 4)/3;
    chance -= GET_AC(victim,AC_BASH) /25;
    /* speed */
    if (IS_SET(ch->off_flags,OFF_FAST))
        chance += 10;
    if (IS_SET(victim->off_flags,OFF_FAST))
        chance -= 30;

    /* level */
    chance += (ch->level - victim->level);

    if (!IS_NPC(victim) 
	&& chance < get_skill(victim,gsn_dodge) )
    {	/*
        act("$n tries to bash you, but you dodge it.",ch,NULL,victim,TO_VICT);
        act("$N dodges your bash, you fall flat on your face.",ch,NULL,victim,TO_CHAR);
        WAIT_STATE(ch,skill_table[gsn_bash].beats);
        return;*/
	chance -= 3 * (get_skill(victim,gsn_dodge) - chance);
    }

    /* now the attack */
    if (number_percent() < chance )
    {
    
     if (IS_SET(victim->res_flags,RES_BASH))
       {
        act("\n\r{R$n{R sends you sprawling with a powerful bash!{x\n\r",
		ch,NULL,victim,TO_VICT);
	act("{CYou slam into $N{C, and send $M{C flying!{x",ch,NULL,victim,TO_CHAR);
	act("\n\r{W$n{W sends $N{W sprawling with a powerful bash.{x\n\r",
		ch,NULL,victim,TO_NOTVICT);
	check_improve(ch,gsn_bash,TRUE,1);

	DAZE_STATE(victim, 1 * PULSE_VIOLENCE);
	WAIT_STATE(ch,skill_table[gsn_bash].beats);
	victim->position = POS_STUNNED;
	damage(ch,victim,number_range(2,2 + 2 * ch->size + chance/20),gsn_bash,
	    DAM_BASH,FALSE);
       }
     else
     if (IS_SET(victim->vuln_flags,VULN_BASH))
       {
        act("\n\r{R$n{R sends you sprawling with a powerful bash!{x\n\r",
		ch,NULL,victim,TO_VICT);
	act("{CYou slam into $N{C, and send $M{C flying!{x",ch,NULL,victim,TO_CHAR);
	act("\n\r{W$n{W sends $N{W sprawling with a powerful bash.{x\n\r",
		ch,NULL,victim,TO_NOTVICT);
	check_improve(ch,gsn_bash,TRUE,1);

	DAZE_STATE(victim, 4 * PULSE_VIOLENCE);
	WAIT_STATE(ch,skill_table[gsn_bash].beats);
	victim->position = POS_STUNNED;
	damage(ch,victim,number_range(2,2 + 2 * ch->size + chance/20),gsn_bash,
	    DAM_BASH,FALSE);
       }
     else
       {
        act("\n\r{R$n{R sends you sprawling with a powerful bash!{x\n\r",
		ch,NULL,victim,TO_VICT);
	act("{CYou slam into $N{C, and send $M{C flying!{x",ch,NULL,victim,TO_CHAR);
	act("\n\r{W$n{W sends $N{W sprawling with a powerful bash.{x\n\r",
		ch,NULL,victim,TO_NOTVICT);
	check_improve(ch,gsn_bash,TRUE,1);

	DAZE_STATE(victim, 2 * PULSE_VIOLENCE);
	WAIT_STATE(ch,skill_table[gsn_bash].beats);
	victim->position = POS_STUNNED;
	damage(ch,victim,number_range(2,2 + 2 * ch->size + chance/20),gsn_bash,
	    DAM_BASH,FALSE);
       }





    }
    else
    {
	damage(ch,victim,0,gsn_bash,DAM_BASH,FALSE);
	act("{RYou fall flat on your face!{x",
	    ch,NULL,victim,TO_CHAR);
	act("{W$n{W falls flat on $s face.{x",
	    ch,NULL,victim,TO_NOTVICT);
	act("\n\r{CYou evade $n{C's bash, causing $m{C to fall flat on $s{C face.{x\n\r",
	    ch,NULL,victim,TO_VICT);
	check_improve(ch,gsn_bash,FALSE,1);
	ch->position = POS_RESTING;
	DAZE_STATE(ch, 2 * PULSE_VIOLENCE);
	WAIT_STATE(ch,skill_table[gsn_bash].beats * 3/2); 
    }
	/* check_killer(ch,victim); */
}

void do_dirt( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int chance;

    one_argument(argument,arg);


    
    if ( (chance = get_skill(ch,gsn_dirt)) == 0
    ||   (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_KICK_DIRT))
    ||   (!IS_NPC(ch)
    &&    !can_use_skpell( ch, gsn_dirt ) ))
    {
     send_to_char("\n\r{GYou get your feet dirty.{x\n\r",ch);
     return;
    }

    if (arg[0] == '\0')
    {
	victim = ch->fighting;
	if (victim == NULL)
	{
	    send_to_char("But you aren't in combat!\n\r",ch);
	    return;
	}
    }

    else if ((victim = get_char_room(ch,arg)) == NULL)
    {
	send_to_char("They aren't here.\n\r",ch);
	return;
    }

    if(IS_SET(ch->in_room->room_flags, ROOM_SAFE))
    {
	send_to_char("\n\r{RNot in a SAFE room!{x\n\r",ch);
	return;
    }



    if (IS_AFFECTED(victim,AFF_BLIND))
    {
	act("$E's already been blinded.",ch,NULL,victim,TO_CHAR);
	return;
    }

    if (victim == ch)
    {
	send_to_char("Very funny.\n\r",ch);
	return;
    }

    if (is_safe(ch,victim))
	return;

    if (IS_NPC(victim) &&
	 victim->fighting != NULL && 
	!is_same_group(ch,victim->fighting))
    {
        send_to_char("Kill stealing is not permitted.\n\r",ch);
        return;
    }

    if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim)
    {
	act("But $N is such a good friend!",ch,NULL,victim,TO_CHAR);
	return;
    }

    /* modifiers */

    /* dexterity */
    chance += get_curr_stat(ch,STAT_DEX);
    chance -= 2 * get_curr_stat(victim,STAT_DEX);

    /* speed  */
    if (IS_SET(ch->off_flags,OFF_FAST))
	chance += 10;
    if (IS_SET(victim->off_flags,OFF_FAST))
	chance -= 25;

    /* level */
    chance += (ch->level - victim->level) * 2;

    /* sloppy hack to prevent false zeroes */
    if (chance % 5 == 0)
	chance += 1;

    /* terrain */

    switch(ch->in_room->sector_type)
    {
	case(SECT_INSIDE):		chance -= 20;	break;
	case(SECT_CITY):		chance -= 10;	break;
	case(SECT_FIELD):		chance +=  5;	break;
	case(SECT_FOREST):				break;
	case(SECT_HILLS):				break;
	case(SECT_MOUNTAIN):		chance -= 10;	break;
	case(SECT_WATER_SWIM):		chance  =  0;	break;
	case(SECT_WATER_NOSWIM):	chance  =  0;	break;
	case(SECT_AIR):			chance  =  0;  	break;
	case(SECT_DESERT):		chance += 10;   break;
    }

    if (chance == 0)
    {
	send_to_char("There isn't any dirt to kick.\n\r",ch);
	return;
    }

    /* now the attack */
    if (number_percent() < chance)
    {
	AFFECT_DATA af;
	act("$n is blinded by the dirt in $s eyes!",victim,NULL,NULL,TO_ROOM);
	act("$n kicks dirt in your eyes!",ch,NULL,victim,TO_VICT);
        damage(ch,victim,number_range(2,5),gsn_dirt,DAM_NONE,FALSE);
	send_to_char("You can't see a thing!\n\r",victim);
	check_improve(ch,gsn_dirt,TRUE,2);
	WAIT_STATE(ch,skill_table[gsn_dirt].beats);

	af.where	= TO_AFFECTS;
	af.type 	= gsn_dirt;
	af.level 	= ch->level;
	af.duration	= 0;
	af.location	= APPLY_HITROLL;
	af.modifier	= -4;
	af.bitvector 	= AFF_BLIND;

	affect_to_char(victim,&af);
    }
    else
    {
	damage(ch,victim,0,gsn_dirt,DAM_NONE,TRUE);
	check_improve(ch,gsn_dirt,FALSE,2);
	WAIT_STATE(ch,skill_table[gsn_dirt].beats);
    }
	/* check_killer(ch,victim); */
}

void do_trip( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int chance;

    one_argument(argument,arg);

    if ( (chance = get_skill(ch,gsn_trip)) == 0
    ||   (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_TRIP))
    ||   (!IS_NPC(ch)
       && !can_use_skpell( ch, gsn_trip ) ))
    {
     send_to_char("\n\r{CTripping?  What's that?{x\n\r",ch);
     return;
    }

    if (arg[0] == '\0')
    {
	victim = ch->fighting;
	if (victim == NULL)
	{
	    send_to_char("But you aren't fighting anyone!\n\r",ch);
	    return;
 	}
    }

    else if ((victim = get_char_room(ch,arg)) == NULL)
    {
	send_to_char("They aren't here.\n\r",ch);
	return;
    }

    if(IS_SET(ch->in_room->room_flags, ROOM_SAFE))
    {
        send_to_char("\n\r{RNot in a SAFE room!{x\n\r",ch);
        return;
    }

    if (is_safe(ch,victim))
	return;

    if (IS_NPC(victim) &&
	 victim->fighting != NULL && 
	!is_same_group(ch,victim->fighting))
    {
	send_to_char("Kill stealing is not permitted.\n\r",ch);
	return;
    }
    
    if (IS_AFFECTED(victim,AFF_FLYING))
    {
	act("$S feet aren't on the ground.",ch,NULL,victim,TO_CHAR);
	return;
    }

    if (victim->position < POS_FIGHTING)
    {
	act("$N is already down.",ch,NULL,victim,TO_CHAR);
	return;
    }

    if (victim == ch)
    {
	send_to_char("You fall flat on your face!\n\r",ch);
	WAIT_STATE(ch,2 * skill_table[gsn_trip].beats);
	act("$n trips over $s own feet!",ch,NULL,NULL,TO_ROOM);
	return;
    }

    if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim)
    {
	act("$N is your beloved master.",ch,NULL,victim,TO_CHAR);
	return;
    }

    /* modifiers */

    /* size */
    if (ch->size < victim->size)
        chance += (ch->size - victim->size) * 10;  /* bigger = harder to trip */

    /* dex */
    chance += get_curr_stat(ch,STAT_DEX);
    chance -= get_curr_stat(victim,STAT_DEX) * 3 / 2;

    /* speed */
    if (IS_SET(ch->off_flags,OFF_FAST))
	chance += 10;
    if (IS_SET(victim->off_flags,OFF_FAST))
	chance -= 20;

    /* level */
    chance += (ch->level - victim->level) * 2;


    /* now the attack */
    if (number_percent() < chance)
    {
	act("$n trips you and you go down!",ch,NULL,victim,TO_VICT);
	act("You trip $N and $N goes down!",ch,NULL,victim,TO_CHAR);
	act("$n trips $N, sending $M to the ground.",ch,NULL,victim,TO_NOTVICT);
	check_improve(ch,gsn_trip,TRUE,1);

	DAZE_STATE(victim,2 * PULSE_VIOLENCE);
        WAIT_STATE(ch,skill_table[gsn_trip].beats);
	victim->position = POS_RESTING;
	damage(ch,victim,number_range(2, 2 +  2 * victim->size),gsn_trip,
	    DAM_BASH,TRUE);
    }
    else
    {
	damage(ch,victim,0,gsn_trip,DAM_BASH,TRUE);
	WAIT_STATE(ch,skill_table[gsn_trip].beats*2/3);
	check_improve(ch,gsn_trip,FALSE,1);
    } 
	/* check_killer(ch,victim); */
} 


void do_disarm( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    int chance,hth,ch_weapon,vict_weapon,ch_vict_weapon;

    hth = 0;

  if ( !IS_NPC(ch)
  &&   !can_use_skpell( ch, gsn_disarm ) )
    {
    send_to_char( "\n\r{BDisarm!  You can barely wield your own weapon, much less try to disarm your opponent!{x\n\r", ch );
    return;
    } 

    if ((chance = get_skill(ch,gsn_disarm)) == 0)
    {
	send_to_char( "You don't know how to disarm opponents.\n\r", ch );
	return;
    }

    if (( get_eq_char( ch, WEAR_WIELD ) == NULL 
    &&  ( hth = get_skill(ch,gsn_hand_to_hand) == 0))
    ||  (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_DISARM)))
    {
	send_to_char( "\n\r{CYou must be wielding a weapon, or be proficient at hand-to-hand.{x\n\r", ch );
	return;
    }

    if ( ( victim = ch->fighting ) == NULL )
    {
	send_to_char( "You aren't fighting anyone.\n\r", ch );
	return;
    }

    if ( ( obj = get_eq_char( victim, WEAR_WIELD ) ) == NULL )
    {
	send_to_char( "Your opponent is not wielding a weapon.\n\r", ch );
	return;
    }

    if(IS_SET(ch->in_room->room_flags, ROOM_SAFE))
    {
        send_to_char("\n\r{RNot in a SAFE room!{x\n\r",ch);
        return;
    }

    /* find weapon skills */
    ch_weapon = get_weapon_skill(ch,get_weapon_sn(ch,FALSE));
    vict_weapon = get_weapon_skill(victim,get_weapon_sn(victim,FALSE));
    ch_vict_weapon = get_weapon_skill(ch,get_weapon_sn(victim,FALSE));

    /* modifiers */

    /* skill */
   
    if ( get_eq_char(ch,WEAR_WIELD) == NULL)
      {
       if (!IS_CLASS(ch, CLASS_WARRIOR))
         { 
	  chance = chance/4;
         }
       else 
         { 
	  chance = hth;
         }
      }
    else
      { chance = (chance * (ch_weapon/100)); }

    chance += (ch_vict_weapon/2 - vict_weapon) / 2; 

    /* dex vs. strength */
    chance += get_curr_stat(ch,STAT_DEX);
    chance -= 2 * get_curr_stat(victim,STAT_STR);


if (!IS_NPC(ch))
  {
   if (!IS_NPC(victim))
     {

       if (IS_CLASS(ch, CLASS_WARRIOR))
         {
          if (!IS_CLASS(victim, CLASS_WARRIOR))
           {
           if (!IS_CLASS(victim, CLASS_PALADIN)
           && !IS_CLASS(victim, CLASS_ANTI_PALADIN)
           && !IS_CLASS(victim, CLASS_RANGER))
             chance += (get_skill(ch,gsn_disarm) * 2);
          else
             chance += get_skill(ch,gsn_disarm);
           }
        else
          {
          if (ch->level > victim->level)
            chance += (ch->level - victim->level);
          else  
            chance -= ((victim->level - ch->level) * 1.5);
          }
       }
    else
    if (IS_CLASS(ch, CLASS_PALADIN))
      {
       if (!IS_CLASS(victim, CLASS_WARRIOR))
         {
          if (!IS_CLASS(victim, CLASS_PALADIN)
          && !IS_CLASS(victim, CLASS_ANTI_PALADIN)
          && !IS_CLASS(victim, CLASS_RANGER))
             chance += (get_skill(ch,gsn_disarm) * 1.5);
          else
            { 
             if (ch->level > victim->level)
               chance += (ch->level - victim->level);
             else  
               chance -= ((victim->level - ch->level) * 1.5);
            }
         }
       else
         { 
          if (ch->level > victim->level)
            chance += 0;
          else  
            chance -= ((victim->level - ch->level) * 2);
         }
       }
    else
    if (IS_CLASS(ch, CLASS_ANTI_PALADIN))
      {
       if (!IS_CLASS(victim, CLASS_WARRIOR))
         {
          if (!IS_CLASS(victim, CLASS_PALADIN)
          && !IS_CLASS(victim, CLASS_ANTI_PALADIN)
          && !IS_CLASS(victim, CLASS_RANGER))
             chance += (get_skill(ch,gsn_disarm) * 1.5);
          else
            { 
             if (ch->level > victim->level)
               chance += (ch->level - victim->level);
             else  
               chance -= ((victim->level - ch->level) * 1.5);
            }
         }
       else
         { 
          if (ch->level > victim->level)
            chance += 0;
          else  
            chance -= ((victim->level - ch->level) * 2);
         }
       }
    else
    if (IS_CLASS(ch, CLASS_RANGER))
      {
       if (!IS_CLASS(victim, CLASS_WARRIOR))
         {
          if (!IS_CLASS(victim, CLASS_PALADIN)
          && !IS_CLASS(victim, CLASS_ANTI_PALADIN)
          && !IS_CLASS(victim, CLASS_RANGER))
             chance += (get_skill(ch,gsn_disarm) * 1.5);
          else
            { 
             if (ch->level > victim->level)
               chance += (ch->level - victim->level);
             else  
               chance -= ((victim->level - ch->level) * 1.5);
            }
         }
       else
         { 
          if (ch->level > victim->level)
            chance += 0;
          else  
            chance -= ((victim->level - ch->level) * 2);
         }
       }
     else
       if (!IS_CLASS(victim, CLASS_WARRIOR))
         {
          if (!IS_CLASS(victim, CLASS_PALADIN)
          && !IS_CLASS(victim, CLASS_ANTI_PALADIN)
          && !IS_CLASS(victim, CLASS_RANGER))
             chance += 0;
           else
             chance -= get_skill(victim,gsn_disarm);
         }
       else
         chance -= (get_skill(victim,gsn_disarm) * 2);
      }
    else
     if (IS_NPC(victim))
    {
       if (IS_CLASS (ch, CLASS_WARRIOR))
      {
       if (!IS_SET(victim->act, ACT_WARRIOR))
         {
          if (!IS_SET(victim->act, ACT_PALADIN)
          || !IS_SET(victim->act, ACT_ANTIPALADIN)
          || !IS_SET(victim->act, ACT_RANGER))
             chance += (get_skill(ch,gsn_disarm) * 2);
          else
             chance += get_skill(ch,gsn_disarm);
         }
       else
         {
          if (ch->level > victim->level)
            chance += (ch->level - victim->level);
          else  
            chance -= ((victim->level - ch->level) * 1.5);
         }
      }
    }
  }
else
 if (IS_NPC(ch))
    {
       if (IS_SET(ch->act, ACT_WARRIOR))
      {
       if (!IS_CLASS(victim, CLASS_WARRIOR))
         {
          if (IS_CLASS(victim, CLASS_PALADIN)
          || IS_CLASS(victim, CLASS_ANTI_PALADIN)
          || IS_CLASS(victim, CLASS_RANGER))
             chance += get_skill(ch,gsn_disarm);
          else
             chance += (get_skill(ch,gsn_disarm) * 2);
         }
       else
         {
          if (ch->level > victim->level)
            chance += (ch->level - victim->level);
          else  
            chance -= ((victim->level - ch->level) * 1.5);
         }
      }
    }






    /* and now the attack */
    if (number_percent() < chance)
    {
    	WAIT_STATE( ch, skill_table[gsn_disarm].beats );
	disarm( ch, victim );
	check_improve(ch,gsn_disarm,TRUE,1);
    }
    else
    {
	WAIT_STATE(ch,skill_table[gsn_disarm].beats);
	act("You fail to disarm $N.",ch,NULL,victim,TO_CHAR);
	act("$n tries to disarm you, but fails.",ch,NULL,victim,TO_VICT);
	act("$n tries to disarm $N, but fails.",ch,NULL,victim,TO_NOTVICT);
	check_improve(ch,gsn_disarm,FALSE,1);
    }
    /* check_killer(ch,victim); */
    return;
} 



void do_backstab( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  OBJ_DATA *obj;
  OBJ_DATA  *weapon = NULL;
  int dam; //Added by Yavi 6/29/04 for backstab buffing
  
  one_argument( argument, arg );


  if ( !IS_NPC(ch)
  &&   !can_use_skpell( ch, gsn_backstab ) )    
    {
    send_to_char( "\n\r{WYou twirl around like a ballerina.{x\n\r", ch );
    return;
    }

  if (arg[0] == '\0')
    {
    send_to_char("Backstab whom?\n\r",ch);
    return;
    }

  if (ch->fighting != NULL)
    {
    send_to_char("You're facing the wrong end.\n\r",ch);
    return;
    }
  else if ((victim = get_char_room(ch,arg)) == NULL)
    {
    send_to_char("They aren't here.\n\r",ch);
    return;
    }

  if ( victim == ch )
    {
    send_to_char( "How can you sneak up on yourself?\n\r", ch );
    return;
    }

  if(IS_SET(ch->in_room->room_flags, ROOM_SAFE))
    {
    send_to_char("\n\r{RNot in a SAFE room!{x\n\r",ch);
    return;
    }

  if (IS_NPC(victim))
    {
     if (IS_SET(victim->act, ACT_NOSPECATTK))
       {
        send_to_char(
"\n\r\n\r{GYour {WTARGET{G is wise to your tactics... Try using a different attack form!{x\n\r\n\r",ch);
        return;
       }
    }

  if ( is_safe( ch, victim ) )
    return;

   if (IS_NPC(victim)
  &&  victim->fighting != NULL
  && 	!is_same_group(ch,victim->fighting))
    {
    send_to_char("Kill stealing is not permitted.\n\r",ch);
    return;
    }

  if ( !(obj = get_eq_char( ch, WEAR_WIELD )) )
    {
    send_to_char( "You need to wield a weapon to backstab.\n\r", ch );
    return;
    }

  weapon = get_eq_char( ch, WEAR_WIELD );
  if ( !weapon
  || ( weapon->value[3] != 11
  &&   weapon->value[3] != 34 
  &&   weapon->value[3] != 2 
  &&   weapon->value[3] != 1 ) )
    {
    send_to_char( "\n\r{WYou need a weapon that thrusts, pierces, stabs or slices to backstab with.{x\n\r",ch );
    return;
    }

  if ( victim->hit < (victim->max_hit * .65 )) 
    {
    act( "$N is hurt and suspicious ... you can't sneak up.", ch, NULL, victim, TO_CHAR );
    return;
    }

  /* Check if players are CHOOSEN */
if (!IS_NPC(victim))
{
if (!IS_NPC(ch))
  {
   if (ch->level < MAX_LEVEL)
     {
      if (!IS_SET(victim->in_room->room_flags, ROOM_DRAGONPIT) && !IS_SET(victim->pact, PLR_DRAGONPIT))
        {
         if (!IS_SET(ch->pact, PLR_DRAGONPIT))
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
  }}

  /* check_killer( ch, victim ); */
  WAIT_STATE( ch, skill_table[gsn_backstab].beats );

  if ( number_percent( ) < get_skill(ch,gsn_backstab)
  || ( get_skill(ch,gsn_backstab) >= 2 && !IS_AWAKE(victim) ) )
    {
    check_improve(ch,gsn_backstab,TRUE,1);
	multi_hit( ch, victim, gsn_backstab );
    }
  else
    {
    check_improve(ch,gsn_backstab,FALSE,1);
	dam = (ch->level * 10); //Yavi 6/29/04 Hack just to test, not final formula
    damage( ch, victim, dam, gsn_backstab,DAM_NONE,TRUE);
	}

  return;
} 

void do_rescue( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *fch;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Rescue whom?\n\r", ch );
	return;
    }

    if ( !IS_NPC(ch)
    &&   !can_use_skpell( ch, gsn_rescue ) )
      {
	send_to_char( "\n\r{yRescue! You may want to guard your own ass first!{x\n\r", ch );
	return;
      }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "What about fleeing instead?\n\r", ch );
	return;
    }

    if ( !IS_NPC(ch) && IS_NPC(victim) )
    {
	send_to_char( "Doesn't need your help!\n\r", ch );
	return;
    }

    if ( ch->fighting == victim )
    {
	send_to_char( "Too late.\n\r", ch );
	return;
    }

    if ( ( fch = victim->fighting ) == NULL )
    {
	send_to_char( "That person is not fighting right now.\n\r", ch );
	return;
    }

    if ( IS_NPC(fch) && !is_same_group(ch,victim))
    {
        send_to_char("Kill stealing is not permitted.\n\r",ch);
        return;
    }

    WAIT_STATE( ch, skill_table[gsn_rescue].beats );
    if ( number_percent( ) > get_skill(ch,gsn_rescue))
    {
	send_to_char( "You fail the rescue.\n\r", ch );
	check_improve(ch,gsn_rescue,FALSE,1);
	return;
    }

    act( "You rescue $N!",  ch, NULL, victim, TO_CHAR    );
    act( "$n rescues you!", ch, NULL, victim, TO_VICT    );
    act( "$n rescues $N!",  ch, NULL, victim, TO_NOTVICT );
    check_improve(ch,gsn_rescue,TRUE,1);

    stop_fighting( fch, TRUE );
    stop_fighting( victim, TRUE );

    /* check_killer( ch, fch ); */
    set_fighting( ch, fch );
    set_fighting( fch, ch );
    return;
}



void do_kick( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
	
    if ( !IS_NPC(ch)
    &&   !can_use_skpell( ch, gsn_kick ) )
    {
	send_to_char("\n\r{RYou better leave the martial arts to the skilled{x.\n\r",ch );
	return;
    }

    if (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_KICK))
	return;

    if ( ( victim = ch->fighting ) == NULL )
    {
	send_to_char( "You aren't fighting anyone.\n\r", ch );
	return;
    }

    if(IS_SET(ch->in_room->room_flags, ROOM_SAFE))
    {
        send_to_char("\n\r{RNot in a SAFE room!{x\n\r",ch);
        return;
    }
	  if (IS_NPC(victim) && victim->pIndexData->pShop != NULL)
	  {
		  send_to_char("{RYou can thank Quick for pointing out this bug.{x\n\r",ch);
		  send_to_char("{RDO NOT ATTACK SHOPKEEPERS!{x\n\r",ch);
		  return;
	  }
    
	WAIT_STATE( ch, skill_table[gsn_kick].beats );
    if ( get_skill(ch,gsn_kick) > number_percent())
    {
	damage(ch,victim,number_range( 1, ch->level ), gsn_kick,DAM_BASH,TRUE);
	check_improve(ch,gsn_kick,TRUE,1);
    }
    else
    {
	damage( ch, victim, 0, gsn_kick,DAM_BASH,TRUE);
	check_improve(ch,gsn_kick,FALSE,1);
    }
	/* check_killer(ch,victim); */
    return;
} 

void do_multiburst( CHAR_DATA *ch, char *argument )
{
  void say_spell( CHAR_DATA *ch, int sn );
  DECLARE_SPELL_FUN(  spell_null );
  CHAR_DATA *victim = ch->fighting;
  char arg[ MAX_INPUT_LENGTH ];
  bool legal[MAX_MULTI];
  int sn[MAX_MULTI];
  bool fail = TRUE;
  int iCtr;

  if ( !IS_NPC( ch ) 
  && (get_skill(ch,gsn_multiburst) == 0))
    {
    send_to_char( "\n\r{RYou're not enough of a mage to do multibursts.{x\n\r", ch );
    return;
    }

  if ( !ch->fighting )
    {
    send_to_char( "\n\r{WYou aren't fighting.{x\n\r", ch );
    return;
    }

    if(IS_SET(ch->in_room->room_flags, ROOM_SAFE))
    {
        send_to_char("\n\r{RNot in a SAFE room!{x\n\r",ch);
        return;
    }

  if ( !IS_NPC( ch ) 
  && get_skill( ch, gsn_multiburst ) < number_percent( ) )
    {
    send_to_char( "\n\r{WYou fail your multiburst.{x\n\r", ch );
    return;
    }

  if (IS_NPC(victim))
    {
     if (IS_SET(victim->act, ACT_NOSPECATTK))
       {
        send_to_char(
"\n\r\n\r{GYour {WTARGET{G is wise to your tactics... Try using a different attack form!{x\n\r\n\r",ch);
        return;
       }
    }

  for ( iCtr = 0; iCtr < MAX_MULTI; iCtr++ )
    { 
if (ch->in_room != get_room_index(ROOM_VNUM_DRAGONPIT_RETURN))
    {
    legal[iCtr] = FALSE;
    sn[iCtr] = -1;
    argument = one_argument( argument, arg );
    sn[iCtr] = skill_lookup( arg );
    if ( sn[iCtr] != -1
    && ( IS_NPC( ch ) || can_use_skpell( ch, sn[iCtr] ) )
    && ( IS_NPC( ch ) || get_skill( ch, sn[iCtr] ) > number_percent( ) )
    && (*skill_table[sn[iCtr]].spell_fun) != (*spell_null)
    && ch->position >= skill_table[sn[iCtr]].minimum_position
    && skill_table[sn[iCtr]].target != TAR_CHAR_SELF )
      legal[iCtr] = TRUE;
    }
  else
    {
     break;
    }
  }

  for ( iCtr = 0; iCtr < MAX_MULTI; iCtr++ )
    if ( legal[iCtr] )
      fail = FALSE;

  WAIT_STATE( ch, skill_table[gsn_multiburst].beats );
  if ( fail )
    { 
    send_to_char( "\n\r{WYour multiburst fails!{x\n\r", ch );
    return;
    }

  if ( ch->mana < 150
  && ch->level < LEVEL_IMMORTAL )
    {
    send_to_char( "\n\r{RYou don't have enough mana to use multiburst!{x\n\r", ch );
    return;
    }

 say_spell( ch, gsn_multiburst );
  send_to_char( "\n\r{GYou release a burst of energy!{x\n\r", ch );
  ch->mana -= 150;
  for ( iCtr = 0; iCtr < MAX_MULTI; iCtr++ )
    {
    if ( legal[iCtr] )
	{
      if ( skill_table[sn[iCtr]].target != TAR_CHAR_DEFENSIVE )
      	(*skill_table[sn[iCtr]].spell_fun) ( sn[iCtr], URANGE( 1, ch->level, LEVEL_HERO ), ch, victim, TAR_CHAR_OFFENSIVE );
      else
      	(*skill_table[sn[iCtr]].spell_fun) ( sn[iCtr], URANGE( 1, ch->level, LEVEL_HERO ), ch, ch, TAR_CHAR_DEFENSIVE );
	}
    if ( victim->position == POS_DEAD || ch->in_room != victim->in_room )
      break;
    }
  if ( ch->level < LEVEL_IMMORTAL 
  && !IS_NPC( ch ) )
  check_improve( ch, gsn_multiburst, TRUE, 1 );
  return;
} 

void do_whirlwind( CHAR_DATA *ch, char *argument )
{
   CHAR_DATA *pChar;
   CHAR_DATA *pChar_next;
   OBJ_DATA *wield;
   bool found = FALSE;
   OBJ_DATA  *weapon = NULL;


    if ( !IS_NPC(ch)
    &&   !can_use_skpell( ch, gsn_whirlwind ) )
   {
      send_to_char( "\n\r{RYou don't know how to do that...{x\n\r", ch );
      return;
   }
 
   if ( ( wield = get_eq_char( ch, WEAR_WIELD ) ) == NULL )
   {
      send_to_char( "\n\r{GYou need to wield a weapon first...{x\n\r", ch );
      return;
   }

  weapon = get_eq_char( ch, WEAR_WIELD );
  if ( !weapon
  || ( weapon->value[3] != 1
  &&   weapon->value[3] != 3
  &&   weapon->value[3] != 5
  &&   weapon->value[3] != 22 ) )
    { 
    send_to_char( "\n\r{WYou need a weapon that slices, slashes, claws or scratches to whirlwind with.{x\n\r",ch );
    return;
    }

    if(IS_SET(ch->in_room->room_flags, ROOM_SAFE))
    {
        send_to_char("\n\r{RNot in a SAFE room!{x\n\r",ch);
        return;
    }
   
  if ( ch->move < (ch->level)/2 )
    {
    send_to_char( "You are tired, rest some then try  WHIRLWIND again.\n\r", ch );
    return;
    }
    


act( "\n\r{W$n holds their weapon firmly, and starts spinning round...{x\n\r", ch,wield, NULL,TO_ROOM );

act( "\n\r{WYou hold your weapon firmly, and start spinning round...{x\n\r",  ch,wield, NULL,TO_CHAR );
   

   pChar_next = NULL;   
   for ( pChar = ch->in_room->people; pChar; pChar = pChar_next )
   {
      pChar_next = pChar->next_in_room;

if (!is_same_group(ch,pChar))
{

if (!IS_IMMORTAL(pChar))
  {
if (!IS_NPC(pChar))
{
if (!IS_NPC(ch))
  {
   if (ch->level < MAX_LEVEL)
     {
      if (!IS_SET(pChar->in_room->room_flags, ROOM_DRAGONPIT) && !IS_SET(pChar->pact, PLR_DRAGONPIT))
        {
         if (!IS_SET(ch->pact, PLR_DRAGONPIT))
           {
            if (!IS_SET(ch->pact, PLR_PKILLER))
              {
               send_to_char("\n\r{WYou must type {RCHOOSE{W and follow those{x\n\r",ch);
               send_to_char("{Winstructions if you wish to PKILL.{x\n\r",ch);
               stop_fighting (ch, TRUE);
               return;
              }
       
            if (!IS_SET(pChar->pact, PLR_PKILLER))
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

  if (IS_NPC(pChar))
    {
     if (IS_SET(pChar->act, ACT_NOSPECATTK))
       {
        send_to_char(
"\n\r\n\r{GYour {WTARGET{G is wise to your tactics... Try using a different attack form!{x\n\r\n\r",ch);
        return;
       }
    }

    if ( pChar->hit < (pChar->max_hit * .20 )) 
    {
	act( "$N is hurt, and is aware of your tactics.",
	    ch, NULL, pChar, TO_CHAR );
	return;
    }


      if ( !IS_NPC( ch ) )
      {
       if ( number_percent( ) < get_skill(ch,gsn_whirlwind)
       && ( get_skill(ch,gsn_whirlwind) >= 2) )
         {
          found = TRUE;
          act( "\n\r{W$n turns towards YOU!{x\n\r", ch, NULL, pChar, TO_VICT    );  

/*          if ( is_same_group( pChar, ch ))
            {
           act("{C$n whirls past $N without doing any damage!{x",ch,NULL,pChar,TO_ROOM);
         
           act("\n\r{G$n's whirls past you without leaving a scratch...{x\n\r",ch,NULL,pChar,TO_VICT);
            }
          else
            {
  	  check_improve(ch,gsn_whirlwind,TRUE,1);
          multi_hit( ch, pChar, gsn_whirlwind );
          ch->move -= (ch->level/1.5);
            }
*/
  	  check_improve(ch,gsn_whirlwind,TRUE,1);
          multi_hit( ch, pChar, gsn_whirlwind );
          ch->move -= (ch->level/1.5);
         }
        else
         {
	  check_improve(ch,gsn_whirlwind,FALSE,1);
          ch->move -= (ch->level/1.5);
	  damage( ch, pChar, 0, gsn_whirlwind,DAM_NONE,TRUE);
         }
       }



   if ( !found )
   {
    act( "\n\r{W$n looks a little dizzy, and a little silly.{x\n\r", ch, NULL, NULL,TO_ROOM );
    act( "\n\r{WYou feel a little dizzy and a little silly.{x\n\r", ch, NULL, NULL,TO_CHAR );
   }
   
   WAIT_STATE( ch, skill_table[gsn_whirlwind].beats );

   if ( !found && number_percent() > 90 )
   {
    act( "\n\r{W$n loses $s balance and falls into a heap.{x\n\r",  ch, NULL, NULL,TO_ROOM );
    act( "\n\r{WYou lose your balance and fall into a heap.{x\n\r", ch, NULL, NULL,TO_CHAR );
    ch->position = POS_STUNNED;
   }
  }
 }


  }
   return;
}      




void do_circle( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *victim = ch->fighting;
  CHAR_DATA *fch    = NULL;
  OBJ_DATA  *weapon = NULL;

  if ( !IS_NPC(ch)
  &&   !can_use_skpell( ch, gsn_circle ) )
    {
    send_to_char( "\n\r{MYou twirl around like a ballerina.{x\n\r", ch );
    return;
    } 

    if(IS_SET(ch->in_room->room_flags, ROOM_SAFE))
    {
        send_to_char("\n\r{RNot in a SAFE room!{x\n\r",ch);
        return;
    }

  if ( argument[0] == '\0' )
    {
    if ( !victim )
      {
      send_to_char( "Circle attack whom?\n\r", ch );
      return;
      }
    }
  else
    {
    if ( !(victim = get_char_room( ch, argument )) )
      {
      send_to_char( "They aren't here.\n\r", ch );
      return;
      }
    if ( ch == victim )
      {
      send_to_char( "Circle attack yourself?  You must be double jointed.\n\r", ch );
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
      if (!IS_SET(victim->in_room->room_flags, ROOM_DRAGONPIT) && !IS_SET(victim->pact, PLR_DRAGONPIT))
        {
         if (!IS_SET(ch->pact, PLR_DRAGONPIT))
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

    if ( victim->hit < (victim->max_hit * .25 )) 
    {
	act( "$N is hurt and suspicious ... you can't sneak up.",
	    ch, NULL, victim, TO_CHAR );
	return;
    }

  if (IS_NPC(victim))
    {
     if (IS_SET(victim->act, ACT_NOSPECATTK))
       {
        send_to_char(
"\n\r\n\r{GYour {WTARGET{G is wise to your tactics... Try using a different attack form!{x\n\r\n\r",ch);
        return;
       }
    }


  weapon = get_eq_char( ch, WEAR_WIELD );
  if ( !weapon
  || ( weapon->value[3] != 11
  &&   weapon->value[3] != 34
  &&   weapon->value[3] != 2    
  &&   weapon->value[3] != 1 ) )
    {
    send_to_char( "\n\r{WYou need a weapon that thrusts, pierces, stabs or slices to circle with.{x\n\r",ch );
    return;
    }

  if ( ch->move < (ch->level)/2 )
    {
    send_to_char( "You are too tired to perform a circle attack.\n\r", ch );
    return;
    }
  if ( is_safe( ch, victim ) )
    return;
       
  if ( IS_NPC( victim )
  &&  victim->fighting
  &&  !is_same_group(ch,victim->fighting) )
    {
    send_to_char("Kill stealing is not permitted.\n\r",ch);
    return;
    } 


  /* check_killer( ch, victim ); */
  ch->move -= (ch->level/2);
  WAIT_STATE( ch, skill_table[gsn_circle].beats );
  if ( !IS_NPC( ch ) && number_percent( ) < get_skill(ch,gsn_circle) )
    {
    multi_hit( ch, victim, gsn_circle );
    check_improve(ch,gsn_circle,TRUE,1);
    }
  else
    {
    damage( ch, victim, 0, gsn_circle, DAM_NONE, TRUE );
    check_improve(ch,gsn_circle,FALSE,5);
    }
  fch = victim->fighting;
  if ( fch == ch )
    return;

  stop_fighting( ch, FALSE );
  stop_fighting( victim, FALSE );

  set_fighting( ch, victim );
  set_fighting( victim, ch );
  return;
}

void do_critical_strike( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *victim = ch->fighting;
  CHAR_DATA *fch    = NULL;
  OBJ_DATA  *weapon = NULL;

  if ( !IS_NPC(ch)
  &&   !can_use_skpell( ch, gsn_critical_strike ) )
    {
    send_to_char( "\n\r{MNow where was that vital organ?{x\n\r", ch );
    return;
    } 

    if(IS_SET(ch->in_room->room_flags, ROOM_SAFE))
    {
        send_to_char("\n\r{RNot in a SAFE room!{x\n\r",ch);
        return;
    }

  if ( argument[0] == '\0' )
    {
    if ( !victim )
      {
      send_to_char( "\n\r{CCritical Strike whom?{x\n\r", ch );
      return;
      }
    }
  else
    {
    if ( !(victim = get_char_room( ch, argument )) )
      {
      send_to_char( "\n\r{RThey aren't here.{x\n\r", ch );
      return;
      }
    }

    if ( ch == victim )
      {
      send_to_char( "\n\r{RWhy would you want to Critically Strike yourself?{x.\n\r",ch );
      return;
      }

/* Check if players are CHOOSEN */
if (!IS_NPC(victim))
{
if (!IS_NPC(ch))
  {
   if (ch->level < MAX_LEVEL)
     {
      if (!IS_SET(victim->in_room->room_flags, ROOM_DRAGONPIT) && !IS_SET(victim->pact, PLR_DRAGONPIT))
        {
         if (!IS_SET(ch->pact, PLR_DRAGONPIT))
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


  if (IS_NPC(victim))
    {
     if (IS_SET(victim->act, ACT_NOSPECATTK))
       {
        send_to_char(
"\n\r\n\r{GYour {WTARGET{G is wise to your tactics... Try using a different attack form!{x\n\r\n\r",ch);
        return;
       }
    }

  weapon = get_eq_char( ch, WEAR_WIELD );
  if ( !weapon )
    {
    send_to_char( "\n\r{WYou need to be wielding a weapon!{x\n\r",ch );
    return;
    }

  if (get_weapon_skill(ch, get_weapon_sn(ch,FALSE)) != 100)
    {
    send_to_char( "\n\r{GYou need to be a Master with the primary weapon you have wielded to try this!{x\n\r",ch );
    return;
    }
  
/*bob*/
  
if ( IS_NPC( victim )
  &&  victim->fighting
  &&  !is_same_group(ch,victim->fighting) )
    {
    send_to_char("\n\r{RKill stealing is not permitted.{x\n\r",ch);
    return;
    }

  WAIT_STATE( ch, skill_table[gsn_critical_strike].beats );




/*   
     if (!IS_NPC(ch))
       {
         if ((ch->level >= 1 && ch->level <= 25)
         &&  number_percent( ) < (get_skill(ch,gsn_critical_strike)/5 ))
          {
             multi_hit( ch, victim, gsn_critical_strike );
             check_improve(ch,gsn_critical_strike,TRUE,3);
          }
        else
         if ((ch->level > 25 && ch->level <= 45)
         &&  number_percent( ) < (get_skill(ch,gsn_critical_strike)/4 ))
           {
           multi_hit( ch, victim, gsn_critical_strike );
           check_improve(ch,gsn_critical_strike,TRUE,3);
           }
        else
         if ((ch->level > 45 && ch->level <= 65) 
         &&  number_percent( ) < (get_skill(ch,gsn_critical_strike)/3 ))
           {
           multi_hit( ch, victim, gsn_critical_strike );
           check_improve(ch,gsn_critical_strike,TRUE,3);
           }
        else
         if ((ch->level > 65 && ch->level <= 85)
         &&  number_percent( ) < (get_skill(ch,gsn_critical_strike)/ 2 ))
           {
           multi_hit( ch, victim, gsn_critical_strike );
           check_improve(ch,gsn_critical_strike,TRUE,3);
           }
        else
         if ((ch->level > 85 && ch->level <= 100)
         &&  number_percent( ) < (get_skill(ch,gsn_critical_strike)))
           {
           multi_hit( ch, victim, gsn_critical_strike );
           check_improve(ch,gsn_critical_strike,TRUE,3);
           }
       else
         {
          damage( ch, victim, 0, gsn_critical_strike, DAM_NONE, TRUE );
          check_improve(ch,gsn_critical_strike,FALSE,5);
         }
    }
*/

  if ( !IS_NPC( ch ) && number_percent( ) <= (get_skill(ch,gsn_critical_strike)/1.25))
    {
     one_hit( ch, victim, gsn_critical_strike, FALSE ); 
     check_improve(ch,gsn_critical_strike,TRUE,3);
    }
  else
    {
    damage( ch, victim, 0, gsn_critical_strike, DAM_NONE, TRUE );
    check_improve(ch,gsn_critical_strike,FALSE,5);
    }

  if ( ch->fighting != victim )
   return;

  fch = victim->fighting;
  if ( fch == ch )
    return;

  stop_fighting( ch, FALSE );
  stop_fighting( victim, FALSE );

  set_fighting( ch, victim );
  set_fighting( victim, ch );
  return;
}

void do_blightning( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *victim = ch->fighting;
  void *vo = (void *)victim;
  int  target = TARGET_CHAR;
  int sn = -1;
  int old_hp = 0;

  if ( ch->race != RACE_DRACONIAN )
    {
    send_to_char( "\n\r{RYou're no draconian.{x\n\r", ch );
    return;
    }

    if(IS_SET(ch->in_room->room_flags, ROOM_SAFE))
    {
        send_to_char("\n\r{RNot in a SAFE room!{x\n\r",ch);
        return;
    }

  if ( !victim )
    {
    send_to_char( "{xYou aren't even fighting.\n\r", ch );
    return;
    }

  if ( ch->race_wait > 0 )
    {
    send_to_char( "{xYou need to catch your breath first.\n\r", ch );
    return;
    }

  if (ch->level < (skill_table[gsn_blightning].skill_level[ch->class]))
    {
    send_to_char( "{\n\r{WYou are still to young a draconian to do this.{x\n\r", ch);
    return;
    }

  sn = skill_lookup("lightning breath");
  if ( sn == -1 || !"lightning breath")
    {
    send_to_char( "{xPlease send a note to Myserie, there was problem.\n\r", ch );
    send_to_char( "Put your class in the note.\n\r", ch );
    return;
    }
  ch->race_wait = 24;
  old_hp = victim->hit;
  if ( number_percent( ) > ch->pcdata->learned[gsn_blightning] )
    {
    send_to_char( "You choke and cough up nothing but smoke.\n\r", ch );
    check_improve(ch,gsn_blightning,FALSE,-3);
    return;
    }
  (*skill_table[sn].spell_fun) ( sn, ch->level, ch, vo,target);
  check_improve(ch,gsn_blightning,TRUE,-3);
  return;
}

void do_bfire( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *victim = ch->fighting;
  void *vo = (void *)victim;
  int  target = TARGET_CHAR;
  int sn = -1;
  int old_hp = 0;

  if ( ch->race != RACE_DRACONIAN )
    {
    send_to_char( "\n\r{RYou're no draconian.{x\n\r", ch );
    return;
    }

    if(IS_SET(ch->in_room->room_flags, ROOM_SAFE))
    {
        send_to_char("\n\r{RNot in a SAFE room!{x\n\r",ch);
        return;
    }


  if ( !victim )
    {
    send_to_char( "{xYou aren't even fighting.\n\r", ch );
    return;
    }
  if ( ch->race_wait > 0 )
    {
    send_to_char( "{xYou need to catch your breath first.\n\r", ch );
    return;
    }
  
  if (ch->level < (skill_table[gsn_bfire].skill_level[ch->class]))
    {
    send_to_char( "{\n\r{WYou are still to young a draconian to do this.{x\n\r", ch);
    return;
    }

    sn = skill_lookup("fire breath");
    if ( sn == -1 || !"fire breath")

    {
    send_to_char( "{xPlease send a note to Myserie, there was problem.\n\r", ch );
    send_to_char( "Put your class in the note.\n\r", ch );
    return;
    }
  ch->race_wait = 24;
  old_hp = victim->hit;
  if ( number_percent( ) > ch->pcdata->learned[gsn_bfire] )
    {
    send_to_char( "You choke and cough up nothing but smoke.\n\r", ch );
    check_improve(ch,gsn_bfire,FALSE,-3);
    return;
    }
  (*skill_table[sn].spell_fun) ( sn, ch->level, ch, vo,target);
  check_improve(ch,gsn_bfire,TRUE,-3);
  return;
}

void do_bacid( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *victim = ch->fighting;
  void *vo = (void *)victim;
  int  target = TARGET_CHAR;
  int sn = -1;
  int old_hp = 0;

  if ( ch->race != RACE_DRACONIAN )
    {
    send_to_char( "\n\r{RYou're no draconian.{x\n\r", ch );
    return;
    }

    if(IS_SET(ch->in_room->room_flags, ROOM_SAFE))
    {
        send_to_char("\n\r{RNot in a SAFE room!{x\n\r",ch);
        return;
    }

  if ( !victim )
    {
    send_to_char( "{xYou aren't even fighting.\n\r", ch );
    return;
    }
  if ( ch->race_wait > 0 )
    {
    send_to_char( "{xYou need to catch your breath first.\n\r", ch );
    return;
    }

  if (ch->level < (skill_table[gsn_bacid].skill_level[ch->class]))
    {
    send_to_char( "{\n\r{WYou are still to young a draconian to do this.{x\n\r", ch);
    return;
    }

  sn = skill_lookup("acid breath");
  if ( sn == -1 || !"acid breath")
    {
    send_to_char( "{xPlease send a note to Myserie, there was problem.\n\r", ch );
    send_to_char( "Put your class in the note.\n\r", ch );
    return;
    }
  ch->race_wait = 24;
  old_hp = victim->hit;
  if ( number_percent( ) > ch->pcdata->learned[gsn_bacid] )
    {
    send_to_char( "You choke and cough up nothing but smoke.\n\r", ch );
    check_improve(ch,gsn_bacid,FALSE,-3);
    return;
    }
  (*skill_table[sn].spell_fun) ( sn, ch->level, ch, vo,target);
  check_improve(ch,gsn_bacid,TRUE,-3);
  return;
}

void do_bfrost( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *victim = ch->fighting;
  void *vo = (void *)victim;
  int  target = TARGET_CHAR;
  int sn = -1;
  int old_hp = 0;

  if ( ch->race != RACE_DRACONIAN )
    {
    send_to_char( "\n\r{RYou're no draconian.{x\n\r", ch );
    return;
    }

    if(IS_SET(ch->in_room->room_flags, ROOM_SAFE))
    {
        send_to_char("\n\r{RNot in a SAFE room!{x\n\r",ch);
        return;
    }

  if ( !victim )
    {
    send_to_char( "{xYou aren't even fighting.\n\r", ch );
    return;
    }
  if ( ch->race_wait > 0 )
    {
    send_to_char( "{xYou need to catch your breath first.\n\r", ch );
    return;
    }

  if (ch->level < (skill_table[gsn_bfrost].skill_level[ch->class]))
    {
    send_to_char( "{\n\r{WYou are still to young a draconian to do this.{x\n\r", ch);
    return;
    }

  sn = skill_lookup("frost breath");
  if ( sn == -1 || !"frost breath")
    {
    send_to_char( "{xPlease send a note to Myserie, there was problem.\n\r", ch );
    send_to_char( "Put your class in the note.\n\r", ch );
    return;
    }
  ch->race_wait = 24;
  old_hp = victim->hit;
  if ( number_percent( ) > ch->pcdata->learned[gsn_bfrost] )
    {
    send_to_char( "You choke and cough up nothing but smoke.\n\r", ch );
    check_improve(ch,gsn_bfrost,FALSE,-3);
    return;
    }
  (*skill_table[sn].spell_fun) ( sn, ch->level, ch, vo,target);
  check_improve(ch,gsn_bfrost,TRUE,-3);
  return;
}

void do_bgas( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *victim = ch->fighting;
  void *vo = (void *)victim;
  int  target = TARGET_CHAR;
  int sn = -1;
  int old_hp = 0;

  if ( ch->race != RACE_DRACONIAN )
    {
    send_to_char( "\n\r{RYou're no draconian.{x\n\r", ch );
    return;
    }

    if(IS_SET(ch->in_room->room_flags, ROOM_SAFE))
    {
        send_to_char("\n\r{RNot in a SAFE room!{x\n\r",ch);
        return;
    }

  if ( !victim )
    {
    send_to_char( "{xYou aren't even fighting.\n\r", ch );
    return;
    }
  if ( ch->race_wait > 0 )
    {
    send_to_char( "{xYou need to catch your breath first.\n\r", ch );
    return;
    }

  if (ch->level < (skill_table[gsn_bgas].skill_level[ch->class]))
    {
    send_to_char( "{\n\r{WYou are still to young a draconian to do this.{x\n\r", ch);
    return;
    }

  sn = skill_lookup("gas breath");
  if ( sn == -1 || !"gas breath")
    {
    send_to_char( "{xPlease send a note to Myserie, there was problem.\n\r", ch );
    send_to_char( "Put your class in the note.\n\r", ch );
    return;
    }
  ch->race_wait = 24;
  old_hp = victim->hit;
  if ( number_percent( ) > ch->pcdata->learned[gsn_bgas] )
    {
    send_to_char( "You choke and cough up nothing but smoke.\n\r", ch );
    check_improve(ch,gsn_bgas,FALSE,-3);
    return;
    }
  (*skill_table[sn].spell_fun) ( sn, ch->level, ch, vo,target);
  check_improve(ch,gsn_bgas,TRUE,-3);
  return;
}

void do_quicken( CHAR_DATA *ch, char *argument )
{
  extern bool check_dispel( int dis_level, CHAR_DATA *victim, int sn);
  char arg[MAX_INPUT_LENGTH];
  AFFECT_DATA af;
  int diceroll;
  ROOM_INDEX_DATA *location;
  
  location=NULL;
 
  argument=one_argument(argument,arg);

    if ( IS_NPC( ch ) )
      return;
   
    if ( !IS_NPC(ch)
    &&   !can_use_skpell( ch, gsn_quicken) )          
      {
        send_to_char( "\n\r{CTry as you might, you just aren't faster than a speeding arrow...{x\n\r",ch);                                           
        return;   
      }

  location = get_room_index(ROOM_VNUM_DRAGONPIT);
  
  if (ch->in_room == location)
    {
send_to_char( "\n\r{GWait until the DragonPIT begins so it will be fair...{x\n\r",ch);
    return;
    }

  if (!str_cmp(arg, "remove"))
   {
    if (is_affected(ch, gsn_quicken))
      {
       affect_strip ( ch, gsn_quicken);
       REMOVE_BIT(ch->affected_by, AFF_HASTE);
       send_to_char("\n\r{WYou force yourself to slow down.{x\n\r",ch);
       WAIT_STATE( ch, skill_table[gsn_quicken].beats );
       return;
      }
     else
      {  
       send_to_char("\n\r{cYou are not affected by QUICKEN...{x\n\r",ch);
       return;
      }
    }
  else
    {
    if ((is_affected(ch, gsn_quicken))
    ||  IS_AFFECTED(ch,AFF_HASTE)
    ||  IS_SET(ch->off_flags,OFF_FAST))
      {
      send_to_char("\n\r{WYou can't move any faster!{x\n\r",ch);
      return;
      }


    if (IS_AFFECTED(ch,AFF_SLOW))
    {
	if (!check_dispel(ch->level,ch,skill_lookup("slow")))
	{
	    send_to_char("\n\r{WYou feel momentarily faster.{x\n\r", ch);
	    return;
	}
        act("\n\r{W$n is moving less slowly.{x\n\r", ch, NULL, NULL, TO_ROOM);
        return;
    }

    diceroll = number_percent( );

    if (diceroll < get_skill(ch,gsn_quicken))
      {
    af.where     = TO_AFFECTS;
    af.type      = gsn_quicken;
    af.level     = ch->level;
    af.duration  = ch->level/2;
    af.location  = APPLY_DEX;
    af.modifier  = 1 + (diceroll/15);
    af.bitvector = AFF_HASTE;
    affect_to_char( ch, &af );
    send_to_char( "\n\r{WYou feel yourself moving more quickly.{x\n\r", ch );
    act("\n\r{W$n is moving more quickly.{x", ch,NULL,NULL,TO_ROOM);
    check_improve(ch,gsn_quicken,TRUE,8);
    return;
      }
    else
      {
    send_to_char( "\n\r{WYou are having a problem becoming one with the stag...{x\n\r",ch);
    check_improve(ch,gsn_quicken,FALSE,10);
    return;
       }
  }
 return;
} 


void do_chant( CHAR_DATA *ch, char *argument)
{
        MOB_INDEX_DATA *pMobIndex;
        CHAR_DATA *companion;
        int i;
  ROOM_INDEX_DATA *location;
  
  location=NULL;
 
    if ( !IS_NPC(ch)
    &&   !can_use_skpell( ch, gsn_companion ) )
      {
       send_to_char("\n\r{GCLATU! VERATA! NIChrngahmsmsdm! That might have worked.{x\n\r",ch);
       return;
      }

  location = get_room_index(ROOM_VNUM_DRAGONPIT);
  
  if (ch->in_room == location)
    {
send_to_char( "\n\r{GWait until the DragonPIT begins so it will be fair...{x\n\r",ch);
    return;
    }
        if ( ch->pet != NULL )
        {
                send_to_char("One companion at a time is enough!\n\r",ch);
                return;
        }

        if (ch->fighting != NULL)
          {
                send_to_char("\n\r{RYou can't concentrate enough to CHANT!{x\n\r",ch);
                return;
          }

        if ( number_percent() > .8 * get_skill(ch,gsn_companion) )
        {

                act("You chant slowly, but nothing answers your call.",ch,NULL,NULL,TO_CHAR);
                act("$n chants slowly, but nature ignores the call.",ch,NULL,NULL,TO_ROOM);

                ch->mana -= 75;

                check_improve(ch,gsn_companion,FALSE,6);

                WAIT_STATE(ch,skill_table[gsn_companion].beats);

                return;
        }


	if (IS_SET(ch->in_room->room_flags, ROOM_DRAGONPIT))
          {
  send_to_char("\n\r{GNothing will answer your call while within a DragonPIT.{x\n\r",ch);
           return;
          }

        pMobIndex = get_mob_index( MOB_VNUM_COMPANION );

        companion = create_mobile(pMobIndex);

        for ( i=0; i < MAX_STATS; i++)
                companion->perm_stat[i]=24;

        companion->level = number_fuzzy( ch->level * 1.15 );
        companion->hit = companion->max_hit = number_fuzzy( companion->level * 40 + 500 );
        companion->mana = companion->max_mana = 0;
        companion->move = companion->max_move = number_fuzzy(companion->level * 10 + 150 );

companion->armor[0]=companion->armor[1]=companion->armor[2]=companion->armor[3]=number_fuzzy(companion->level
* -3 +180);
        companion->hitroll = companion->damroll = number_fuzzy(companion->level / 2);

        companion->damage[0]=(companion->level)/20;
        companion->damage[1]=(companion->level)/2;
        companion->damage[2]=(companion->level)/5;

        free_string(companion->name);
        free_string(companion->short_descr);
        free_string(companion->long_descr);
        free_string(companion->description);

        /* Choose a companion based on room sector */
        switch ( ch->in_room->sector_type )
        {
        case ( SECT_INSIDE ) :
           companion->name = str_dup("giant cockroach companion");
           companion->short_descr = str_dup("a giant cockroach");
           companion->long_descr = str_dup("A giant cockroach scuttles about here.\n\r");
           companion->description =
                 str_dup("This grisly cockroach is larger than any you have ever\n\r"
                         "seen, perhaps even big enough to tear you up and digest\n\r"
                         "you in his sick little way.\n\r");
           companion->dam_type = 22;          /* scratch */
           break;

        case ( SECT_CITY ) :
           companion->name = str_dup("vicious sewer rat companion");
           companion->short_descr = str_dup("a vicious sewer rat");
           companion->long_descr = str_dup("A vicious sewer rat peers around with beady eyes.\n\r");
           companion->description =
                 str_dup("This vile animal looks (and smells) like it has crawled\n\r"
                         "out of a sewer to where he stands now. Absolutely gross.\n\r");
           companion->dam_type = 10;          /* bite */
           break;

        case ( SECT_FIELD ) :
           companion->name = str_dup("crafty fox companion");
           companion->short_descr = str_dup("a crafty fox");
           companion->long_descr = str_dup("A crafty fox is sniffing about curiously.\n\r");
           companion->description =
                 str_dup("This cute little fox probably isn't so cute underneath.\n\r"
                         "He has a perky red tail and slanted eyes, indicating that\n\r"
                         "he is always on the alert for prey.\n\r");
           companion->dam_type = 7;          /* pound */
           break;

        case ( SECT_FOREST ) :
           companion->name = str_dup("black bear companion");
           companion->short_descr = str_dup("a black bear");
           companion->long_descr = str_dup("A black bear lumbers about restlessly here.\n\r");
           companion->description =
           str_dup("The bear before you is a testament to nature's raw power.\n\r"
                   "It has massive limbs and a gargantuan body, and its massive\n\r"
                   "claws invoke fear in the depth of your heart.\n\r");
           companion->dam_type = 15;          /* charge */
           break;

        case ( SECT_DESERT ) :
           companion->name = str_dup("giant cobra companion");
           companion->short_descr = str_dup("a giant cobra");
           companion->long_descr = str_dup("A giant cobra coils its body, anticipating your move.\n\r");
           companion->description =
                 str_dup("This scaled monstrosity reaches well over 12 feet in length,\n\r"
                         "and its muscular body ripples and shimmers in the light. It\n\r"
                         "bares its teeth at you and spits venomously.\n\r");
           companion->dam_type = 31;          /* acidic bite */
           break;

        case ( SECT_WATER_SWIM ) :
        case ( SECT_WATER_NOSWIM ) :
           companion->name = str_dup("sleek shark companion");
           companion->short_descr = str_dup("a sleek shark");
           companion->long_descr = str_dup("A sleek shark is here, searching for a quick meal.\n\r");
           companion->description =
                 str_dup("This shark swims about effortlessly, swingings its long tail\n\r"
                         "back and forth in smooth motions. It looks slender and quite\n\r"
                         "beautiful, but you know its bite is deadly.\n\r");
           companion->dam_type = 32;          /* chomp */
           break;

        case ( SECT_HILLS ) :
           companion->name = str_dup("hawk companion");
           companion->short_descr = str_dup("a hawk");
           companion->long_descr = str_dup("A dark-eyed hawk keeps a watchful eye over the room.\n\r");
           companion->description =
                 str_dup("This graceful bird is both glamorous and deadly. It has very long\n\r"
                         "wings, and short legs with deadly claws. Its sharp beak is stained\n\r"
                         "red from the flesh of recent prey.\n\r");
           companion->dam_type = 23;          /* peck */
           break;

        case ( SECT_MOUNTAIN ) :
           companion->name = str_dup("mountain lion companion");
           companion->short_descr = str_dup("a mountain lion");
           companion->long_descr = str_dup("A mountain lion paces slowly in circles, smelling the area.\n\r");
           companion->description =
                 str_dup("This deadly cat is the epitome of speed, grace, and power. It is made\n\r"
                         "made by nature to be a killing machine, and its sharp eyes and sharper\n\r"
                         "claws serve it well to this end.\n\r");
           companion->dam_type = 5;          /* claw */
           break;

        case ( SECT_AIR ) :
        default :
           companion->name = str_dup("chimera companion");
           companion->short_descr = str_dup("a chimera"); 
           companion->long_descr = str_dup("A black chimera is here, silently looking around.\n\r");
           companion->description =
                 str_dup("This dreadful creature looks like it could rip you limb from\n\r"
                         "limb in a heartbeat. Stand clear for your own good!\n\r");
           companion->dam_type = 29;          /* flaming bite */
           break;
        }
      
        act("You chant slowly, and $N answers your call!",ch,NULL,companion,TO_CHAR);
        act("$n chants slowly, and $N answers the call!",ch,NULL,companion,TO_ROOM);

        char_to_room( companion, ch->in_room );
        add_follower( companion,ch );
        companion->leader = ch;
        ch->pet = companion;
        ch->mana -= 150;

        SET_BIT(companion->act, ACT_PET);
        SET_BIT(companion->act, ACT_RANGER);
        SET_BIT(companion->affected_by, AFF_CHARM);
        SET_BIT(companion->affected_by, AFF_HASTE);

        check_improve(ch,gsn_companion,TRUE,6);
        WAIT_STATE(ch,skill_table[gsn_companion].beats);

        return;
}

void do_crecall( CHAR_DATA *ch, char *argument )
{
   ROOM_INDEX_DATA *location;
  
  location=NULL;
 
  location = get_room_index(ROOM_VNUM_DRAGONPIT);
  
 if (!IS_IMP(ch))
    {
    if (IS_IMMORTAL(ch))
      {
       send_to_char("\n\r{rIMMs can use {RGOTO{r so there is no reason to be using {RCLAN RECALL{r.{x\n\r",ch);
       return;
      }
    }

  if ((ch->in_room == location)
  || (IS_SET(ch->pact, PLR_DRAGONPIT)))
    {
send_to_char( "\n\r{GNot while you are within a DragonPIT.....{x\n\r",ch);
    return;
    }
 else
   {
 if ( ch->pk_timer > 0 )
    send_to_char( "\n\r{WYou must wait {R5 ticks{W before you can recall after a player fight.{x\n\r", ch );
  else if ( ch->fighting )
    send_to_char( "\n\r{WYou must use {RWORD OF RECALL{W during battle.{x\n\r", ch );    
  else
   crecall( ch );
    }

  return;
}


void crecall( CHAR_DATA *ch)
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    ROOM_INDEX_DATA *location;
    CLAN_DATA *pClan;
   
   location=NULL;

    if (IS_NPC(ch))
    {
	send_to_char("\n\rOnly players(PC) may use this.\n\r",ch);
	return;
    }

    act( "$n prays for transportation!", ch, 0, 0, TO_ROOM );

      if (ch->clan > 0)
        {
         pClan = get_clan_index(ch->clan);
      
         if (pClan->hall > 0)
           {
            location = get_room_index(pClan->hall);
           }  
         else
           {       
            location = get_room_index(ROOM_VNUM_TEMPLE);
           }
        }     
       else
                  {	
            send_to_char( "\n\r{WTry using {cRECALL{W instead.{x\n\r", ch);
	    return;
           }

    if (ch->pcdata->house != 0)
        location = get_room_index(ch->pcdata->house);

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


    if (( victim = ch->fighting ) != NULL )
    {
	int lose,skill;

	skill = get_skill(ch,gsn_crecall);

	if ( number_percent() < 80 * skill / 100 )
	{
	    check_improve(ch,gsn_crecall,FALSE,6);
	    WAIT_STATE( ch, 4 );
	    sprintf( buf, "You failed!.\n\r");
	    send_to_char( buf, ch );
	    return;
	}

	lose = (ch->desc != NULL) ? 25 : 50;
	gain_exp( ch, 0 - lose );
	check_improve(ch,gsn_crecall,TRUE,4);
	sprintf( buf, "You recall from combat!  You lose %d exps.\n\r", lose );
	send_to_char( buf, ch );
	stop_fighting( ch, TRUE );
	
    }

    ch->move /= 2;
    act( "$n disappears.", ch, NULL, NULL, TO_ROOM );
    char_from_room( ch );
    char_to_room( ch, location );
    act( "$n appears in the room.", ch, NULL, NULL, TO_ROOM );
    do_function(ch, &do_look, "auto" );

    return;
}


/* Psionic Skills */

void do_heighten_senses ( CHAR_DATA *ch, char *argument )
{
    AFFECT_DATA af;
   ROOM_INDEX_DATA *location;
  
  location=NULL;

    if ( !IS_NPC(ch)
    &&   !can_use_skpell( ch, gsn_heighten_senses) )
      {
        send_to_char( "\n\r{WYou are extremely aware of that rock in your boot!{x\n\r",ch);  
        return;
      }

  location = get_room_index(ROOM_VNUM_DRAGONPIT);
  
  if (ch->in_room == location)
    {
send_to_char( "\n\r{GWait until the DragonPIT begins so it will be fair...{x\n\r",ch);
    return;
    }

    if ( IS_AFFECTED( ch,AFF_HASTE) )
    {
     send_to_char( "\n\r{WYour senses and speed are at their peak.{x\n\r",ch);
     return;
    }

    send_to_char( "\n\r{WYou attempt to improve your senses.{x\n\r", ch );

    if ( number_percent( ) < get_skill(ch,gsn_heighten_senses))
    {
    af.where     = TO_AFFECTS;
    af.type      = gsn_heighten_senses;
    af.level     = ch->level;
    af.duration  = ch->level / 1.25;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_DETECT_INVIS;
    affect_to_char( ch, &af );

    af.bitvector = AFF_DETECT_HIDDEN;
    affect_to_char( ch, &af );

    af.bitvector = AFF_INFRARED;
    affect_to_char( ch, &af );

    af.location  = APPLY_DEX;
    af.modifier  = +5;
    af.bitvector = AFF_HASTE;
    affect_to_char( ch, &af );

    send_to_char( "\n\r{WYour mind's awareness expands drastically.{x\n\r", ch );
    check_improve(ch,gsn_heighten_senses,TRUE,3);
    return;    
    }
    else
    {
    send_to_char( "\n\r{WYou are having a problem conentrating.{x\n\r", ch);
    check_improve(ch,gsn_heighten_senses,FALSE,3);
    return;
    }
  return;
}


void do_chameleon_power ( CHAR_DATA *ch, char *argument )
{
   ROOM_INDEX_DATA *location;
  
  location=NULL;

    if ( IS_AFFECTED( ch, AFF_HIDE ) )
      {
    send_to_char( "\n\r{WYou are blending as best you can already.{x\n\r",ch);
    return;
      }

  location = get_room_index(ROOM_VNUM_DRAGONPIT);
  
  if (ch->in_room == location)
    {
send_to_char( "\n\r{GWait until the DragonPIT begins so it will be fair...{x\n\r",ch);
    return;
    }

    if ( !IS_NPC(ch)
    &&   !can_use_skpell( ch, gsn_chameleon_power) )          
      {
        send_to_char( "\n\r{CTry as you might, you just can't turn into a small lizard!{x\n\r",ch);                                           
        return;   
      }

    send_to_char( "\n\r{WYou attempt to blend in with your surroundings.{x", ch);

    if (number_percent( ) < get_skill(ch,gsn_chameleon_power))
      {
    SET_BIT( ch->affected_by, AFF_HIDE );
    check_improve(ch,gsn_chameleon_power,TRUE,3);
    send_to_char( "\n\r{WThe powers of your mind enshroud you, blending you perfectly.{x\n\r",ch);
    return;
      }
    else
      {
    send_to_char( "\n\r{WYour mind is on other things which causes{x\n\r",ch);
    send_to_char( "{WYou to lose your concentration.{x\n\r",ch);
    check_improve(ch,gsn_chameleon_power,FALSE,3);
    return;
      }
  return;
}

void do_shadow_form ( CHAR_DATA *ch, char *argument )
{
    AFFECT_DATA af;
   ROOM_INDEX_DATA *location;
  
  location=NULL;

    if ( IS_AFFECTED( ch, AFF_SNEAK ) )
      {
    send_to_char( "\n\r{WYou are already one with the Shadows.{x\n\r",ch);
    return;
      }

  location = get_room_index(ROOM_VNUM_DRAGONPIT);
  
  if (ch->in_room == location)
    {
send_to_char( "\n\r{GWait until the DragonPIT begins so it will be fair...{x\n\r",ch);
    return;
    }

    if ( !IS_NPC(ch)
    &&   !can_use_skpell( ch, gsn_shadow_form) )          
      {
        send_to_char( "\n\r{RMaybe you should try saying 'Form of... An Ice Slide', or something, it always worked for those twins..{x\n\r",ch);                                           
        return;   
      }

    send_to_char( "\n\r{WBy the power of your mind you attempt to{x\n\r", ch );
    send_to_char( "{Wshift your form into that of Pure Shadow!{x\n\r",ch);

    if (number_percent( ) < get_skill(ch,gsn_shadow_form))
    {

       af.where     = TO_AFFECTS;
       af.type      = gsn_shadow_form;
       af.level     = ch->level;
       af.duration  = ch->level;
       af.location  = APPLY_NONE;
       af.modifier  = 0;
       af.bitvector = AFF_SNEAK;
       affect_to_char( ch, &af );

    check_improve(ch,gsn_shadow_form,TRUE,3);
    send_to_char( "\n\r{WYou open your mind up to the shadows and{x\n\r",ch);
    send_to_char( "{Wthey welcome you.{x\n\r",ch);
    return;
    }
    else
    {
    send_to_char( "\n\r{WYour mind is on other things which causes{x\n\r",ch);
    send_to_char( "{WYou to lose your concentration.{x\n\r",ch);
    check_improve(ch,gsn_shadow_form,FALSE,3);
    return;
     }
  return;
} 


void do_remort( CHAR_DATA *ch, char *argument )
{
  char buf[MAX_STRING_LENGTH];
  extern void affect_remove( CHAR_DATA *ch, AFFECT_DATA *paf );
  extern void write_race_buffer( DESCRIPTOR_DATA *d );
  DESCRIPTOR_DATA *d;
  AFFECT_DATA *paf;
  AFFECT_DATA *paf_next;
  OBJ_DATA *obj;
  OBJ_DATA *obj_next;
  CHAR_DATA *vch;
  CHAR_DATA *pch;
  int race;
  
  if ( IS_NPC( ch )
  ||   ch->level != 100
  ||   ch->pcdata->oldcl != -1 )
    {
    send_to_char( "\n\r{RYou cannot, now go away!{x\n\r", ch );
    return;
    }

  if ( ch->in_room->vnum != remvalue )
    {
    send_to_char( "\n\r{RYou can't do that here!{x\n\r", ch );
    return;
    }

  if ( ch->questpoints < 4000 )
    {
    send_to_char( "\n\r{RYou do not have enough quest points.{x\n\r", ch );
    return;
    }
  send_to_char( "\n\r{WYou step into the portal..{x.\n\r", ch );
  ch->questpoints -= 4000;
  ch->affected_by = 0;
  ch->affected2_by = 0;
  ch->imm_flags = 0;
  ch->res_flags = 0;
  ch->vuln_flags= 0;
  ch->level	= 0;


  remvalue = 0;

  for (obj = ch->in_room->contents; obj != NULL; obj = obj_next)
     {
      obj_next = obj->next;

      if (obj->pIndexData->vnum == OBJ_VNUM_REMORT_PORTAL)
        extract_obj(obj);
     }

  send_to_char( "\n\r{GAll your equipment disintegrates...{x\n\r", ch );

  for ( obj = ch->carrying; obj; obj = obj_next )
    {
    obj_next = obj->next_content;
    extract_obj( obj );
    }
  send_to_char( "\n\r{GYou feel all magic seeping form you...{x\n\r", ch );
  for ( paf = ch->affected; paf; paf = paf_next )
    {
    paf_next = paf->next;
    affect_remove( ch, paf );
    }

  send_to_char( "\n\r{GYou begin do de-evolve, your knowldge the only thing intact...{x\n\r", ch );
  d = ch->desc;

            write_to_buffer(d,
                "\n\r{c/---------------------------------------------------------\\\n\r", 0);
            write_to_buffer(d,
 "{c| {x{RType '{WHELP {c<{WRACENAME{c>{R' to see {Wrace {Rstats.               {c|{x\n\r",0);
write_to_buffer(d,"{c|------------------------------------------------------------------------------\\\n\r",0);
            write_to_buffer(d,
"{c|{x Race Name {c|{x Points {c| {xAlignments    {c| {xClasses Availble to Race {c         | {xSex {c|\n\r", 0);


write_to_buffer(d,"{c|------------------------------------------------------------------------------|\n\r",
0);

            for(race=1;pc_race_table[race].name != NULL; race++)
            {
    sprintf(buf, 
"{c| {x%-9s {c| {x%-6d {c| {x%-13s {c| {x%-33s {c| {x%-3s {c|{x\n\r",
capitalize(pc_race_table[race].name), pc_race_table[race].points, pc_race_table[race].align,
              pc_race_table[race].class, pc_race_table[race].sex);
                write_to_buffer(d, buf, 0);
            }

write_to_buffer(d,"{c\\------------------------------------------------------------------------------/{x\n\r",
0);

            write_to_buffer(d, "\n\r", 0);
            write_to_buffer(d, "{cYour race selection:{w ", 0);



  ch->played *= 0.4;
  ch->pcdata->remorting = TRUE;
  ch->pcdata->oldcl = ch->class;

  act_new( "\n\r{W$n steps into the portal and fades from view.{x", ch,NULL,NULL,TO_ROOM, POS_RESTING );
  extract_char( ch, FALSE );
  save_char_obj( ch );
  char_from_room( ch );
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

  d->connected = CON_GET_NEW_RACE;
  return;
}
