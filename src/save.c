#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <malloc.h>
#include "merc.h"
#include "recycle.h"
#include "tables.h"
#include "lookup.h"
 
#if !defined(macintosh)
extern  int     _filbuf         args( (FILE *) );
#endif

// Explorer/Killer Percentages
char ** GetCommaDelimitedArray(char * dynamicFilePath);
void SaveCommaDelimitedArray(char * dynamicFilePath, char ** values);

int rename(const char *oldfname, const char *newfname);

char *print_flags(int flag)
{
    int count, pos = 0;
    static char buf[52];


    for (count = 0; count < 32;  count++)
    {
        if (IS_SET(flag,1<<count))
        {
            if (count < 26)
                buf[pos] = 'A' + count;
            else
                buf[pos] = 'a' + (count - 26);
            pos++;
        }
    }

    if (pos == 0)
    {
        buf[pos] = '0';
        pos++;
    }

    buf[pos] = '\0';

    return buf;
}


/*
 * Array of containers read for proper re-nesting of objects.
 */
#define MAX_NEST	100
static	OBJ_DATA *	rgObjNest	[MAX_NEST];



/*
 * Local functions.
 */
void	fwrite_char	args( ( CHAR_DATA *ch,  FILE *fp ) );
void	fwrite_obj	args( ( CHAR_DATA *ch,  OBJ_DATA  *obj,
			    FILE *fp, int iNest ) );
void	fwrite_pet	args( ( CHAR_DATA *pet, FILE *fp) );
void	fread_char	args( ( CHAR_DATA *ch,  FILE *fp ) );
void    fread_pet	args( ( CHAR_DATA *ch,  FILE *fp ) );
void	fread_obj	args( ( CHAR_DATA *ch,  FILE *fp ) );



/*
 * Save a character and inventory.
 * Would be cool to save NPC's too for quest purposes,
 *   some of the infrastructure is provided.
 */
void save_char_obj( CHAR_DATA *ch )
{
    char strsave[MAX_INPUT_LENGTH];
    FILE *fp;
    // Explorer/Killer Percentages
    char    dynamicFilePath[4096];

    if ( !ch || IS_NPC(ch) )
	return;

    if ( ch->desc != NULL && ch->desc->original != NULL )
	ch = ch->desc->original;

#if defined(unix)
    /* create god log */
    if (IS_IMMORTAL(ch) || ch->level >= LEVEL_IMMORTAL)
    {
	fclose(fpReserve);
	sprintf(strsave, "%s%s",GOD_DIR, capitalize(ch->name));
	if ((fp = fopen(strsave,"w")) == NULL)
	{
	    bug("Save_char_obj: fopen",0);
	    perror(strsave);
 	}

	fprintf(fp,"Lev %2d  %s%s\n",
	    ch->level,ch->name, ch->pcdata->title);
	fclose( fp );
	fpReserve = fopen( NULL_FILE, "r" );
    }
#endif

	// Explorer/Killer Percentages
    sprintf(dynamicFilePath, "%s%s.killer", PLAYER_DIR, capitalize(ch->name));
    SaveCommaDelimitedArray(dynamicFilePath, ch->pcdata->killed);
    sprintf(dynamicFilePath, "%s%s.explorer", PLAYER_DIR, capitalize(ch->name));
    SaveCommaDelimitedArray(dynamicFilePath, ch->pcdata->explored);
    sprintf(dynamicFilePath, "%s%s.treasurehunter", PLAYER_DIR, capitalize(ch->name));
    SaveCommaDelimitedArray(dynamicFilePath, ch->pcdata->objectsFound);

    // Granted
    sprintf(dynamicFilePath, "%s%s.granted", PLAYER_DIR, capitalize(ch->name));
    SaveCommaDelimitedArray(dynamicFilePath, ch->pcdata->granted);
    

    fclose( fpReserve );
    sprintf( strsave, "%s%s", PLAYER_DIR, capitalize( ch->name ) );
    if ( ( fp = fopen( TEMP_FILE, "w" ) ) == NULL )
    {
	bug( "Save_char_obj: fopen", 0 );
	perror( strsave );
    }
    else
    {
	fwrite_char( ch, fp );
	if ( ch->carrying != NULL )
	    fwrite_obj( ch, ch->carrying, fp, 0 );
	/* save the pets */
	if (ch->pet != NULL && ch->pet->in_room == ch->in_room)
	    fwrite_pet(ch->pet,fp);
	fprintf( fp, "#END\n" );
    }
    fclose( fp );
    if (ch->pcdata->member)
      save_pkinfo (ch->pcdata->member);
    rename(TEMP_FILE,strsave);
    fpReserve = fopen( NULL_FILE, "r" );
    return;
}



/*
 * Write the char.
 */
void fwrite_char( CHAR_DATA *ch, FILE *fp )
{
    CLAN_DATA *pClan;
    AFFECT_DATA *paf;
    int sn, gn, pos, i;

    fprintf( fp, "#%s\n", IS_NPC(ch) ? "MOB" : "PLAYER"	);

    fprintf( fp, "Name %s~\n",	ch->name		);
    fprintf( fp, "Id   %ld\n", ch->id			);
    fprintf( fp, "LogO %ld\n",	current_time		);
    fprintf( fp, "Vers %d\n",   5			);
    if (ch->short_descr[0] != '\0')
      	fprintf( fp, "ShD  %s~\n",	ch->short_descr	);
    if( ch->long_descr[0] != '\0')
	fprintf( fp, "LnD  %s~\n",	ch->long_descr	);
    if (ch->description[0] != '\0')
    	fprintf( fp, "Desc %s~\n",	ch->description	);
    if (ch->prompt != NULL || !str_cmp(ch->prompt,"<%hhp %mm %vmv> "))
        fprintf( fp, "Prom %s~\n",      ch->prompt  	);
    fprintf( fp, "Race %s~\n", pc_race_table[ch->race].name );
    if (ch->clan && (pClan=get_clan_index(ch->clan)))
      fprintf (fp, "Clan %s~\n",pClan->name);

    if (ch->ctimp_clan && (pClan=get_clan_index(ch->ctimp_clan)))
      fprintf( fp, "CTImp %s~\n",pClan->name );

    fprintf( fp, "Sex  %d\n",	ch->sex			);
    fprintf( fp, "Cla  %d\n",	ch->class		);

if ( !IS_NPC( ch ) && ch->pcdata->remorting )
      fprintf( fp, "Rem  %d\n", ch->pcdata->remorting   );

if ( !IS_NPC( ch ))      
{
      fprintf( fp, "AlertMe  %d\n", ch->pcdata->alertMe   );
      fprintf( fp, "Hardcore  %d\n", ch->pcdata->hardcore   );
        fprintf(fp, "CasterScore   %d\n", ch->pcdata->casterScore);
      fprintf( fp, "PrevHCLevel  %d\n", ch->pcdata->previousHCLevel);
    // LEGEND SYSTEM
        fprintf(fp, "Legend %d\n", ch->pcdata->legend );
}

    fprintf( fp, "OCla %d\n",ch->pcdata->oldcl );
    fprintf( fp, "Levl %d\n",	ch->level		);

if (!IS_NPC(ch))
   {
    if(ch->oldlvl < ch->level)
      {
       ch->oldlvl = ch->level;
      }
   }
    
    fprintf( fp, "OldLvl %d\n", ch->oldlvl              );

    fprintf( fp, "Sec  %d\n",    ch->pcdata->security	);	/* OLC */
  if(ch->pcdata->spouse > 0)
    fprintf( fp, "Spou %s~\n", ch->pcdata->spouse);
    fprintf( fp, "Plyd %d\n",
	ch->played + (int) (current_time - ch->logon)	);

    fprintf( fp, "Scro %d\n", 	ch->lines		);

   if (ch->bank > 0)
      fprintf( fp, "Bank %ld\n", ch->bank);


    fprintf( fp, "Room %d\n",
        (  ch->in_room == get_room_index( ROOM_VNUM_LIMBO )
        && ch->was_in_room != NULL )
            ? ch->was_in_room->vnum
            : ch->in_room == NULL ? 3001 : ch->in_room->vnum );

    fprintf( fp, "HMV  %d %d %d %d %d %d\n",
	ch->hit, ch->max_hit, ch->mana, ch->max_mana, ch->move, ch->max_move );

    if (ch->gold > 0)
      fprintf( fp, "Gold %ld\n",	ch->gold		);
    else
      fprintf( fp, "Gold %d\n", 0			); 

    if (ch->silver > 0)
	fprintf( fp, "Silv %ld\n",ch->silver		);
    else
	fprintf( fp, "Silv %d\n",0			);

   if (ch->pcdata->balance >= 1)
       fprintf (fp, "Balance  %d\n",0);

    if (ch->pcdata->gold_bank > 0)
      fprintf( fp, "Gold_bank %ld\n",ch->pcdata->gold_bank);
    else
      fprintf( fp, "Gold_bank %d\n", 0);

    if (ch->pcdata->silver_bank > 0)
	fprintf( fp, "Silv_bank %ld\n",ch->pcdata->silver_bank);
    else
	fprintf( fp, "Silv_bank %d\n",0);

    if (ch->pcdata->shares > 0)
	fprintf( fp, "Shares %d\n",ch->pcdata->shares);
    else
	fprintf( fp, "Shares %d\n",0);

    if (ch->pcdata->share_level > 0)
	fprintf( fp, "Share_level %d\n",ch->pcdata->share_level);
    else

	fprintf( fp, "Shares_level %d\n",0);
    if (ch->pcdata->shares_bought > 0)
	fprintf( fp, "Shares_bought %d\n",ch->pcdata->shares_bought);
    else
	fprintf( fp, "Shares_bought %d\n",0);

    if (ch->pcdata->shares / 4 != 0)
      {
       ch->pcdata->duration = ch->pcdata->shares / 4;
	 fprintf( fp, "Duration %d\n",ch->pcdata->duration);
      }
    else
	fprintf( fp, "Duration %d\n",0);


    fprintf( fp, "Bounty %d\n",	ch->pcdata->bounty  );
    fprintf( fp, "House %d\n",  ch->pcdata->house   );
    fprintf( fp, "Exp  %d\n",	ch->exp			);
    if (ch->questpoints != 0)
        fprintf( fp, "QuestPnts %d\n",  ch->questpoints );
    if (ch->nextquest != 0)
        fprintf( fp, "QuestNext %d\n",  ch->nextquest   );
    else if (ch->countdown != 0)
        fprintf( fp, "QuestNext %d\n",  10              );
    if (ch->pact != 0)
	fprintf( fp, "PAct  %s\n",   print_flags(ch->pact));

    if (ch->act != 0)
/*	fprintf( fp, "Act  %s\n",   print_flags(ch->act)); */
	fprintf( fp, "Act  %d\n",0); 

    if (ch->affected_by != 0)
	fprintf( fp, "AfBy %s\n",   print_flags(ch->affected_by));

    if (ch->affected2_by != 0)
	fprintf( fp, "AfBy2 %s\n",   print_flags(ch->affected2_by)); 

    fprintf( fp, "WName %s~\n",ch->who_name );

    fprintf( fp, "Comm %s\n",       print_flags(ch->comm));

    fprintf( fp, "RaceChan %s\n",       print_flags(ch->racechan));

    fprintf( fp, "Deaf %s\n",       print_flags(ch->deaf));

    if (ch->wiznet)
    	fprintf( fp, "Wizn %s\n",   print_flags(ch->wiznet));
    if (ch->invis_level)
	fprintf( fp, "Invi %d\n", 	ch->invis_level	);
    if (ch->incog_level)
	fprintf(fp,"Inco %d\n",ch->incog_level);
    fprintf( fp, "Pos  %d\n",	
	ch->position == POS_FIGHTING ? POS_STANDING : ch->position );
    if (ch->practice != 0)
    	fprintf( fp, "Prac %d\n",	ch->practice	);
    if (ch->train != 0)
	fprintf( fp, "Trai %d\n",	ch->train	);
    if (ch->saving_throw != 0)
	fprintf( fp, "Save  %d\n",	ch->saving_throw);
    fprintf( fp, "Alig  %d\n",	ch->alignment		);
    if (ch->hitroll != 0)
	fprintf( fp, "Hit   %d\n",	ch->hitroll	);
    if (ch->damroll != 0)
	fprintf( fp, "Dam   %d\n",	ch->damroll	);
    fprintf( fp, "ACs %d %d %d %d\n",	
	ch->armor[0],ch->armor[1],ch->armor[2],ch->armor[3]);
    if (ch->wimpy !=0 )
	fprintf( fp, "Wimp  %d\n",	ch->wimpy	);
    fprintf( fp, "Attr %d %d %d %d %d\n",
	ch->perm_stat[STAT_STR],
	ch->perm_stat[STAT_INT],
	ch->perm_stat[STAT_WIS],
	ch->perm_stat[STAT_DEX],
	ch->perm_stat[STAT_CON] );

    fprintf (fp, "AMod %d %d %d %d %d\n",
	ch->mod_stat[STAT_STR],
	ch->mod_stat[STAT_INT],
	ch->mod_stat[STAT_WIS],
	ch->mod_stat[STAT_DEX],
	ch->mod_stat[STAT_CON] );

    if ( IS_NPC(ch) )
    {
	fprintf( fp, "Vnum %d\n",	ch->pIndexData->vnum	);
    }
    else
    {
	fprintf( fp, "Pass %s~\n",	ch->pcdata->pwd		);
	if (ch->pcdata->bamfin[0] != '\0')
	    fprintf( fp, "Bin  %s~\n",	ch->pcdata->bamfin);
	if (ch->pcdata->bamfout[0] != '\0')
		fprintf( fp, "Bout %s~\n",	ch->pcdata->bamfout);
	fprintf( fp, "Titl %s~\n",	ch->pcdata->title	);

        if (IS_IMMORTAL(ch))
        {
          /*    if ( ch->pcdata->job[0] != '\0' )  */
                        fprintf( fp, "Job %s~\n",       ch->pcdata->job );
        }

    	fprintf( fp, "Pnts %d\n",   	ch->pcdata->points      );
	fprintf( fp, "TSex %d\n",	ch->pcdata->true_sex	);
	fprintf( fp, "PSize %d\n",	ch->perm_size	);
	fprintf( fp, "LLev %d\n",	ch->pcdata->last_level	);
	fprintf( fp, "HMVP %d %d %d\n", ch->pcdata->perm_hit, 
						   ch->pcdata->perm_mana,
						   ch->pcdata->perm_move);
	fprintf( fp, "Cnd  %d %d %d %d\n",
	    ch->pcdata->condition[0],
	    ch->pcdata->condition[1],
	    ch->pcdata->condition[2],
	    ch->pcdata->condition[3] );

  for (pos = 0; pos < MAX_IGNORE; pos++)  
  {
    if (ch->pcdata->ignore[pos] == NULL)
    break;

    fprintf(fp, "Ignore %s~\n", ch->pcdata->ignore[pos]);
  }
	
/* write alias */
        for (pos = 0; pos < MAX_ALIAS; pos++)
	{
	    if (ch->pcdata->alias[pos] == NULL
	    ||  ch->pcdata->alias_sub[pos] == NULL)
		break;

	    fprintf(fp,"Alias %s %s~\n",ch->pcdata->alias[pos],
		    ch->pcdata->alias_sub[pos]);
	}
	/* Save note board status */
	/* Save number of boards in case that number changes */
	fprintf (fp, "Boards       %d ", MAX_BOARD);
	for (i = 0; i < MAX_BOARD; i++)
	    fprintf (fp, "%s %ld ", boards[i].short_name, ch->pcdata->last_note[i]);
	fprintf (fp, "\n");



	for ( sn = 0; sn < MAX_SKILL; sn++ )
	{
	    if ( skill_table[sn].name != NULL && ch->pcdata->learned[sn] > 0 )
	    {
		fprintf( fp, "Sk %d '%s'\n",
		    ch->pcdata->learned[sn], skill_table[sn].name );
	    }
	}

	for ( gn = 0; gn < MAX_GROUP; gn++ )
        {
            if ( group_table[gn].name != NULL && ch->pcdata->group_known[gn])
            {
                fprintf( fp, "Gr '%s'\n",group_table[gn].name);
            }
        }
    }

    for ( paf = ch->affected; paf != NULL; paf = paf->next )
    {
	if (paf->type < 0 || paf->type>= MAX_SKILL)
	    continue;
	
	fprintf( fp, "Affc '%s' %3d %3d %3d %3d %3d %10d\n",
	    skill_table[paf->type].name,
	    paf->where,
	    paf->level,
	    paf->duration,
	    paf->modifier,
	    paf->location,
	    paf->bitvector
	    );
    }

    fprintf( fp, "End\n\n" );
    return;
}

/* write a pet */
void fwrite_pet( CHAR_DATA *pet, FILE *fp)
{
    AFFECT_DATA *paf;
    
    fprintf(fp,"#PET\n");
    
    fprintf(fp,"Vnum %d\n",pet->pIndexData->vnum);
    
    fprintf(fp,"Name %s~\n", pet->name);
    fprintf(fp,"LogO %ld\n", current_time);
    if (pet->short_descr != pet->pIndexData->short_descr)
    	fprintf(fp,"ShD  %s~\n", pet->short_descr);
    if (pet->long_descr != pet->pIndexData->long_descr)
    	fprintf(fp,"LnD  %s~\n", pet->long_descr);
    if (pet->description != pet->pIndexData->description)
    	fprintf(fp,"Desc %s~\n", pet->description);
    if (pet->race != pet->pIndexData->race)
    	fprintf(fp,"Race %s~\n", race_table[pet->race].name);
    fprintf(fp,"Sex  %d\n", pet->sex);
    if (pet->level != pet->pIndexData->level)
    	fprintf(fp,"Levl %d\n", pet->level);
    fprintf(fp, "HMV  %d %d %d %d %d %d\n",
    	pet->hit, pet->max_hit, pet->mana, pet->max_mana, pet->move, pet->max_move);
    if (pet->gold > 0)
    	fprintf(fp,"Gold %ld\n",pet->gold);
    if (pet->silver > 0)
	fprintf(fp,"Silv %ld\n",pet->silver);
    if (pet->exp > 0)
    	fprintf(fp, "Exp  %d\n", pet->exp);
    if (pet->act != pet->pIndexData->act)
    	fprintf(fp, "Act  %s\n", print_flags(pet->act));
    if (pet->affected_by != pet->pIndexData->affected_by)
    	fprintf(fp, "AfBy %s\n", print_flags(pet->affected_by));
    if (pet->affected2_by != pet->pIndexData->affected2_by)
    	fprintf(fp, "AfBy2 %s\n", print_flags(pet->affected2_by)); 
/*    	fprintf(fp, "AfBy2 %d\n",0); */

    if (pet->comm != 0)
    	fprintf(fp, "Comm %s\n", print_flags(pet->comm));
    fprintf(fp,"Pos  %d\n", pet->position = POS_FIGHTING ? POS_STANDING : pet->position);
    if (pet->saving_throw != 0)
    	fprintf(fp, "Save %d\n", pet->saving_throw);
    if (pet->alignment != pet->pIndexData->alignment)
    	fprintf(fp, "Alig %d\n", pet->alignment);
    if (pet->hitroll != pet->pIndexData->hitroll)
    	fprintf(fp, "Hit  %d\n", pet->hitroll);
    if (pet->damroll != pet->pIndexData->damage[DICE_BONUS])
    	fprintf(fp, "Dam  %d\n", pet->damroll);
    fprintf(fp, "ACs  %d %d %d %d\n",
    	pet->armor[0],pet->armor[1],pet->armor[2],pet->armor[3]);
    fprintf(fp, "Attr %d %d %d %d %d\n",
    	pet->perm_stat[STAT_STR], pet->perm_stat[STAT_INT],
    	pet->perm_stat[STAT_WIS], pet->perm_stat[STAT_DEX],
    	pet->perm_stat[STAT_CON]);
    fprintf(fp, "AMod %d %d %d %d %d\n",
    	pet->mod_stat[STAT_STR], pet->mod_stat[STAT_INT],
    	pet->mod_stat[STAT_WIS], pet->mod_stat[STAT_DEX],
    	pet->mod_stat[STAT_CON]);
    
    for ( paf = pet->affected; paf != NULL; paf = paf->next )
    {
    	if (paf->type < 0 || paf->type >= MAX_SKILL)
    	    continue;
    	    
    	fprintf(fp, "Affc '%s' %3d %3d %3d %3d %3d %10d\n",
    	    skill_table[paf->type].name,
    	    paf->where, paf->level, paf->duration, paf->modifier,paf->location,
    	    paf->bitvector);
    }
    
    fprintf(fp,"End\n");
    return;
}
    
/*
 * Write an object and its contents.
 */
void fwrite_obj( CHAR_DATA *ch, OBJ_DATA *obj, FILE *fp, int iNest )
{
    EXTRA_DESCR_DATA *ed;
    AFFECT_DATA *paf;

    /*
     * Slick recursion to write lists backwards,
     *   so loading them will load in forwards order.
     */
    if ( obj->next_content != NULL )
	fwrite_obj( ch, obj->next_content, fp, iNest );


    /*
     * No saving stuff from proto areas.
     */

   if(!IS_IMP(ch))
     {
    if ( IS_SET( obj->pIndexData->area->area_flags, AREA_PROTO ))
      {
      obj_from_char( obj );
      extract_obj( obj );
      return;
      }
     }

    /*
     * Castrate storage characters.
     */
    if(IS_HARDCORE(ch) && ch->pcdata->previousHCLevel>ch->level)
    {
        if ( (ch->pcdata->previousHCLevel < obj->level - 2 
        && obj->item_type != ITEM_CONTAINER)
        || (obj->item_type == ITEM_KEY 
        && !IS_OBJ_STAT(obj, ITEM_SPECIAL_KEY)) )
        	return;
    }
    else
    {
        if ( (ch->level < obj->level - 2 
        && obj->item_type != ITEM_CONTAINER)
        || (obj->item_type == ITEM_KEY 
        && !IS_OBJ_STAT(obj, ITEM_SPECIAL_KEY)) )
        	return;
    }

    fprintf( fp, "#O\n" );
    fprintf( fp, "Vnum %d\n",   obj->pIndexData->vnum        );

    if (!obj->pIndexData->new_format)
	fprintf( fp, "Oldstyle\n");

    if (obj->enchanted)
	fprintf( fp,"Enchanted\n");
        fprintf( fp, "Nest %d\n",iNest);

    /* these data are only used if they do not match the defaults */

    if ( obj->name != obj->pIndexData->name)
    	fprintf( fp, "Name %s~\n",	obj->name		     );
    if ( obj->short_descr != obj->pIndexData->short_descr)
        fprintf( fp, "ShD  %s~\n",	obj->short_descr	     );
    if ( obj->description != obj->pIndexData->description)
        fprintf( fp, "Desc %s~\n",	obj->description	     );
    if ( obj->extra_flags != obj->pIndexData->extra_flags)
        fprintf( fp, "ExtF %d\n",	obj->extra_flags	     );
    if ( obj->wear_flags != obj->pIndexData->wear_flags)
        fprintf( fp, "WeaF %d\n",	obj->wear_flags		     );
    if ( obj->material_type != obj->pIndexData->material_type)
        fprintf( fp, "MatR %d\n",	obj->material_type	     );
    if ( obj->item_type != obj->pIndexData->item_type)
        fprintf( fp, "Ityp %d\n",	obj->item_type		     );
    if ( obj->weight != obj->pIndexData->weight)
        fprintf( fp, "Wt   %d\n",	obj->weight		     );
    if ( obj->condition != obj->pIndexData->condition)
	fprintf( fp, "Cond %d\n",	obj->condition		     );

    /* variable data */

    fprintf( fp, "Wear %d\n",   obj->wear_loc                );
    if (obj->level != obj->pIndexData->level)
        fprintf( fp, "Lev  %d\n",	obj->level		     );
    if (obj->timer != 0)
        fprintf( fp, "Time %d\n",	obj->timer	     );
    fprintf( fp, "Cost %d\n",	obj->cost		     );
    if (obj->value[0] != obj->pIndexData->value[0]
    ||  obj->value[1] != obj->pIndexData->value[1]
    ||  obj->value[2] != obj->pIndexData->value[2]
    ||  obj->value[3] != obj->pIndexData->value[3]
    ||  obj->value[4] != obj->pIndexData->value[4]) 
    	fprintf( fp, "Val  %d %d %d %d %d\n",
	    obj->value[0], obj->value[1], obj->value[2], obj->value[3],
	    obj->value[4]	     );

    switch ( obj->item_type )
    {
    case ITEM_POTION:
    case ITEM_SCROLL:
    case ITEM_PILL:
	if ( obj->value[1] > 0 )
	{
	    fprintf( fp, "Spell 1 '%s'\n", 
		skill_table[obj->value[1]].name );
	}

	if ( obj->value[2] > 0 )
	{
	    fprintf( fp, "Spell 2 '%s'\n", 
		skill_table[obj->value[2]].name );
	}

	if ( obj->value[3] > 0 )
	{
	    fprintf( fp, "Spell 3 '%s'\n", 
		skill_table[obj->value[3]].name );
	}

	break;

    case ITEM_STAFF:
    case ITEM_WAND:
	if ( obj->value[3] > 0 )
	{
	    fprintf( fp, "Spell 3 '%s'\n", 
		skill_table[obj->value[3]].name );
	}

	break;
    }

    for ( paf = obj->affected; paf != NULL; paf = paf->next )
    {
	if (paf->type < 0 || paf->type >= MAX_SKILL)
	    continue;
        fprintf( fp, "Affc '%s' %3d %3d %3d %3d %3d %10d\n",
            skill_table[paf->type].name,
            paf->where,
            paf->level,
            paf->duration,
            paf->modifier,
            paf->location,
            paf->bitvector
            );
    }

    for ( ed = obj->extra_descr; ed != NULL; ed = ed->next )
    {
	fprintf( fp, "ExDe %s~ %s~\n",
	    ed->keyword, ed->description );
    }

    fprintf( fp, "End\n\n" );

    if ( obj->contains != NULL )
	fwrite_obj( ch, obj->contains, fp, iNest + 1 );

    return;
}



/*
 * Load a char and inventory into a new ch structure.
 */
bool load_char_obj( DESCRIPTOR_DATA *d, char *name )
{
    char strsave[MAX_INPUT_LENGTH];
    char buf[100];
    CHAR_DATA *ch;
    FILE *fp;
    bool found;
    int stat;
    // Explorer/Killer Percentages
    char    dynamicFilePath[4096];

    ch = new_char();
    ch->pcdata = new_pcdata();

    d->character			= ch;
    ch->desc				= d;
    ch->name				= str_dup( name );
    ch->id				= get_pc_id();
    ch->race				= race_lookup("human");
    ch->pcdata->oldcl                   = -1;
    ch->pcdata->bounty			= 0; 
    ch->gold                    = 100;
    ch->pact				= PLR_NOSUMMON | PLR_PNOTE;
    ch->comm				= COMM_COMBINE 
					| COMM_PROMPT;
	ch->racechan = 0;
    ch->prompt 				= str_dup( "{c<%hhp %mm %vmv>{x " );
    ch->pcdata->confirm_delete		= FALSE;
    ch->pcdata->board                   = &boards[DEFAULT_BOARD];
    ch->pcdata->confirm_choose		= FALSE;
    ch->pcdata->pwd			= str_dup( "" );
    ch->pcdata->bamfin			= str_dup( "" );
    ch->pcdata->bamfout			= str_dup( "" );
    ch->pcdata->title			= str_dup( "" );
    ch->pcdata->remorting               = FALSE;
    ch->pcdata->previousHCLevel = 0;
    ch->pcdata->casterScore = FALSE;
    ch->pcdata->hardcore               = FALSE;
    ch->pcdata->alertMe               = FALSE;
    ch->pcdata->legending = FALSE; // LEGEND SYSTEM
    ch->pcdata->legend      = -1;    // LEGEND SYSTEM
    ch->pcdata->gbuffered               = FALSE;
    for (stat =0; stat < MAX_STATS; stat++)
	ch->perm_stat[stat]		= 13;
    ch->pcdata->condition[COND_THIRST]	= 48; 
    ch->pcdata->condition[COND_FULL]	= 48;
    ch->pcdata->condition[COND_HUNGER]	= 48;
    ch->pk_timer	= 0;
    ch->pcdata->security		= 0;	/* OLC */
    ch->pcdata->house           = 0;

	//////////////////////////////////////////////////////////////////////////
	// Acrophobia Stuff
	ch->pcdata->acro_answer[0] = '\0';
	ch->pcdata->acro_voted_for = 0;
	ch->pcdata->acro_score = 0;
	ch->pcdata->acro_has_participated = 0;

	// Explorer/Killer Percentages
    sprintf(dynamicFilePath, "%s%s.killer", PLAYER_DIR, capitalize(name));
    ch->pcdata->killed = GetCommaDelimitedArray(dynamicFilePath);
    sprintf(dynamicFilePath, "%s%s.explorer", PLAYER_DIR, capitalize(name));
    ch->pcdata->explored = GetCommaDelimitedArray(dynamicFilePath);
    sprintf(dynamicFilePath, "%s%s.treasurehunter", PLAYER_DIR, capitalize(name));
    ch->pcdata->objectsFound = GetCommaDelimitedArray(dynamicFilePath);
    
    // Grant system
    sprintf(dynamicFilePath, "%s%s.granted", PLAYER_DIR, capitalize(name));
    ch->pcdata->granted = GetCommaDelimitedArray(dynamicFilePath);

    found = FALSE;
    fclose( fpReserve );
    
    #if defined(unix)
    /* decompress if .gz file exists */
    sprintf( strsave, "%s%s%s", PLAYER_DIR, capitalize(name),".gz");
    if ( ( fp = fopen( strsave, "r" ) ) != NULL )
    {
	fclose(fp);
	sprintf(buf,"gzip -dfq %s",strsave);
	system(buf);
    }
    #endif

    sprintf( strsave, "%s%s", PLAYER_DIR, capitalize( name ) );
    if ( ( fp = fopen( strsave, "r" ) ) != NULL )
    {
	int iNest;

	for ( iNest = 0; iNest < MAX_NEST; iNest++ )
	    rgObjNest[iNest] = NULL;

	found = TRUE;
	for ( ; ; )
	{
	    char letter;
	    char *word;

	    letter = fread_letter( fp );
	    if ( letter == '*' )
	    {
		fread_to_eol( fp );
		continue;
	    }

	    if ( letter != '#' )
	    {
		bug( "Load_char_obj: # not found.", 0 );
		break;
	    }

	    word = fread_word( fp );
	    if      ( !str_cmp( word, "PLAYER" ) ) fread_char ( ch, fp );
	    else if ( !str_cmp( word, "OBJECT" ) ) fread_obj  ( ch, fp );
	    else if ( !str_cmp( word, "O"      ) ) fread_obj  ( ch, fp );
	    else if ( !str_cmp( word, "PET"    ) ) fread_pet  ( ch, fp );
	    else if ( !str_cmp( word, "END"    ) ) break;
	    else
	    {
		bug( "Load_char_obj: bad section.", 0 );
		break;
	    }
	}
	fclose( fp );
    }

    fpReserve = fopen( NULL_FILE, "r" );


    /* initialize race */
    if (found)
    {
	int i;

	if (ch->race == 0)
	    ch->race = race_lookup("human");

    if (ch->perm_size != pc_race_table[ch->race].size)
      ch->size = ch->perm_size;
    else
      ch->size = pc_race_table[ch->race].size;

	ch->dam_type = 17; /*punch */

	for (i = 0; i < 5; i++)
	{
	    if (pc_race_table[ch->race].skills[i] == NULL)
		break;
	    group_add(ch,pc_race_table[ch->race].skills[i],FALSE);
	}
	ch->affected_by = ch->affected_by|race_table[ch->race].aff;
	ch->imm_flags	= ch->imm_flags | race_table[ch->race].imm;
	ch->res_flags	= ch->res_flags | race_table[ch->race].res;
	ch->vuln_flags	= ch->vuln_flags | race_table[ch->race].vuln;
	ch->form	= race_table[ch->race].form;
	ch->parts	= race_table[ch->race].parts;
/*	ch->affected2_by = ch->affected2_by|race_table[ch->race].aff2; */
    }

	
    /* RT initialize skills */

    if (found && ch->version < 2)  /* need to add the new skills */
    {
	group_add(ch,"rom basics",FALSE);
	group_add(ch,class_table[ch->class].base_group,FALSE);
	group_add(ch,class_table[ch->class].default_group,TRUE);
	ch->pcdata->learned[gsn_recall] = 50;
    }
 
    /* fix levels */
    if (found && ch->version < 3 && (ch->level > 35))
    {
	switch (ch->level)
	{
	    case(40) : ch->level = 60;	break;  /* imp -> imp */
	    case(39) : ch->level = 58; 	break;	/* god -> supreme */
	    case(38) : ch->level = 56;  break;	/* deity -> god */
	    case(37) : ch->level = 53;  break;	/* angel -> demigod */
            case(36) : ch->level = 51;  break;	/* hero -> hero */
	}
    }

    /* ream gold */
    if (found && ch->version < 4)
    {
	ch->gold   /= 100;
    }
    return found;
}



/*
 * Read in a char.
 */

#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value ) if ( !str_cmp( word, literal ) ) {	field  = value;	fMatch = TRUE; break; 	}

/* provided to free strings */
#if defined(KEYS)
#undef KEYS
#endif

#define KEYS( literal, field, value ) if ( !str_cmp( word, literal ) ) { free_string(field); field  = value; fMatch = TRUE; break; }

void fread_char( CHAR_DATA *ch, FILE *fp )
{
    char buf[MAX_STRING_LENGTH];
    char *word;
    bool fMatch;
    int count = 0;
    int lastlogoff = current_time;
    int percent;

    sprintf(buf,"Loading %s.",ch->name);
    log_string(buf);

    for ( ; ; )
    {
	word   = feof( fp ) ? "End" : fread_word( fp );
	fMatch = FALSE;

	switch ( UPPER(word[0]) )
	{
	case '*':
	    fMatch = TRUE;
	    fread_to_eol( fp );
	    break;

	case 'A':
	    KEY( "Act",		ch->act,		fread_flag( fp ) ); 
	    KEY( "AffectedBy",	ch->affected_by,	fread_flag( fp ) );
	    KEY( "AfBy",	ch->affected_by,	fread_flag( fp ) );
	    KEY( "AffectedBy2",	ch->affected2_by,	fread_flag( fp ) );
	    KEY( "AfBy2",	ch->affected2_by,	fread_flag( fp ) );
	    KEY( "AlertMe",         ch->pcdata->alertMe,  fread_number( fp ) );
	    KEY( "Alignment",	ch->alignment,		fread_number( fp ) );
	    KEY( "Alig",	ch->alignment,		fread_number( fp ) );

	    if (!str_cmp( word, "Alia"))
	      {
        if (count >= MAX_ALIAS)
          {
          fread_to_eol(fp);
          fMatch = TRUE;
          break;
  		    }
        ch->pcdata->alias[count] 	= str_dup(fread_word(fp));
        ch->pcdata->alias_sub[count]	= str_dup(fread_word(fp));
        count++;
        fMatch = TRUE;
        break;
        }
      if (!str_cmp( word, "Alias"))
        {
        if (count >= MAX_ALIAS)
          {
          fread_to_eol(fp);
          fMatch = TRUE;
          break;
          }
        ch->pcdata->alias[count]        = str_dup(fread_word(fp));
        ch->pcdata->alias_sub[count]    = fread_string(fp);
        count++;
        fMatch = TRUE;
        break;
        }

	    if (!str_cmp( word, "AC") || !str_cmp(word,"Armor"))
	    {
		fread_to_eol(fp);
		fMatch = TRUE;
		break;
	    }

	    if (!str_cmp(word,"ACs"))
	    {
		int i;

		for (i = 0; i < 4; i++)
		    ch->armor[i] = fread_number(fp);
		fMatch = TRUE;
		break;
	    }

	    if (!str_cmp(word, "AffD"))
	    {
		AFFECT_DATA *paf;
		int sn;

		paf = new_affect();

		sn = skill_lookup(fread_word(fp));
		if (sn < 0)
		    bug("Fread_char: unknown skill.",0);
		else
		    paf->type = sn;

		paf->level	= fread_number( fp );
		paf->duration	= fread_number( fp );
		paf->modifier	= fread_number( fp );
		paf->location	= fread_number( fp );
		paf->bitvector	= fread_number( fp );
		paf->next	= ch->affected;
		ch->affected	= paf;
		fMatch = TRUE;
		break;
	    }

            if (!str_cmp(word, "Affc"))
            {
                AFFECT_DATA *paf;
                int sn;
 
                paf = new_affect();
 
                sn = skill_lookup(fread_word(fp));
                if (sn < 0)
                    bug("Fread_char: unknown skill.",0);
                else
                    paf->type = sn;
 
                paf->where  = fread_number(fp);
                paf->level      = fread_number( fp );
                paf->duration   = fread_number( fp );
                paf->modifier   = fread_number( fp );
                paf->location   = fread_number( fp );
                paf->bitvector  = fread_number( fp );
                paf->next       = ch->affected;
                ch->affected    = paf;
                fMatch = TRUE;
                break;
            }

	    if ( !str_cmp( word, "AttrMod"  ) || !str_cmp(word,"AMod"))
	    {
		int stat;
		for (stat = 0; stat < MAX_STATS; stat ++)
		   ch->mod_stat[stat] = fread_number(fp);
		fMatch = TRUE;
		break;
	    }

	    if ( !str_cmp( word, "AttrPerm" ) || !str_cmp(word,"Attr"))
	    {
		int stat;

		for (stat = 0; stat < MAX_STATS; stat++)
		    ch->perm_stat[stat] = fread_number(fp);
		fMatch = TRUE;
		break;
	    }
	    break;


	case 'B':
	    KEY( "Bank",	ch->bank,		fread_number( fp ) ); 
	    KEY( "Bamfin",	ch->pcdata->bamfin,	fread_string( fp ) );
	    KEY( "Bamfout",	ch->pcdata->bamfout,	fread_string( fp ) );
	    KEY( "Bin",		ch->pcdata->bamfin,	fread_string( fp ) );
	    KEY( "Bout",	ch->pcdata->bamfout,	fread_string( fp ) );
            KEY( "Bounty",	ch->pcdata->bounty,	fread_number( fp ) );
	    KEY( "Balance",	ch->pcdata->balance,	fread_number( fp )); 

	    /* Read in board status */
	    if (!str_cmp(word, "Boards" ))
	    {
		int i,num = fread_number (fp); /* number of boards saved */
                char *boardname;

                for (; num ; num-- ) /* for each of the board saved */
                {
		    boardname = fread_word (fp);
		    i = board_lookup (boardname); /* find board number */

		    if (i == BOARD_NOTFOUND) /* Does board still exist ? */
                    {
			sprintf (buf, "fread_char: %s had unknown board name: %s. Skipped.",
			    ch->name, boardname);
			log_string (buf);
			fread_number (fp); /* read last_note and skip info */
		    }
		    else /* Save it */
			ch->pcdata->last_note[i] = fread_number (fp);
		} /* for */

                fMatch = TRUE;
	      } /* Boards */
	    
            break;

	case 'C':
	    KEY( "CasterScore",         ch->pcdata->casterScore,  fread_number( fp ) );
	    KEY( "Class",	ch->class,		fread_number( fp ) );
	    KEY( "Cla",		ch->class,		fread_number( fp ) );
	    KEY( "Clan",	ch->clan,	clan_lookup(fread_string( fp )));
	    KEY( "CTimp",	ch->ctimp_clan,	clan_lookup(fread_string( fp )));

	    if ( !str_cmp( word, "Condition" ) || !str_cmp(word,"Cond"))
	    {
		ch->pcdata->condition[0] = fread_number( fp );
		ch->pcdata->condition[1] = fread_number( fp );
		ch->pcdata->condition[2] = fread_number( fp );
		fMatch = TRUE;
		break;
	    }
            if (!str_cmp(word,"Cnd"))
            {
                ch->pcdata->condition[0] = fread_number( fp );
                ch->pcdata->condition[1] = fread_number( fp );
                ch->pcdata->condition[2] = fread_number( fp );
		ch->pcdata->condition[3] = fread_number( fp );
                fMatch = TRUE;
                break;
            }
	    KEY("Comm",		ch->comm,		fread_flag( fp ) ); 
          
	    break;

	case 'D':
	    KEY( "Damroll",	ch->damroll,		fread_number( fp ) );
	    KEY( "Dam",		ch->damroll,		fread_number( fp ) );
            KEY( "Deaf",        ch->deaf,               fread_flag( fp ) );
	    KEY( "Description", ch->description, fread_string( fp ) );
	    KEY( "Desc",	ch->description,	fread_string( fp ) );
	    KEY( "Duration", ch->pcdata->duration,  fread_number(fp));
	    break;

	case 'E':
	    if ( !str_cmp( word, "End" ) )
	    {
    		/* adjust hp mana move up  -- here for speed's sake */
    		percent = (current_time - lastlogoff) * 25 / ( 2 * 60 * 60);

		percent = UMIN(percent,100);
 
    		if (percent > 0 && !IS_AFFECTED(ch,AFF_POISON)
    		&&  !IS_AFFECTED(ch,AFF_PLAGUE))
    		{
        	    ch->hit	+= (ch->max_hit - ch->hit) * percent / 100;
        	    ch->mana    += (ch->max_mana - ch->mana) * percent / 100;
        	    ch->move    += (ch->max_move - ch->move)* percent / 100;
    		}
		return;
	    }

	    KEY( "Exp",		ch->exp,		fread_number( fp ) );
	    break;

	case 'G':
	    KEY( "Gold",	ch->gold,		fread_number( fp ) );
	    KEY( "Gold_bank", ch->pcdata->gold_bank,  fread_number(fp));
	    
            if ( !str_cmp( word, "Group" )  || !str_cmp(word,"Gr"))
            {
                int gn;
                char *temp;
 
                temp = fread_word( fp ) ;
                gn = group_lookup(temp);
                /* gn    = group_lookup( fread_word( fp ) ); */
                if ( gn < 0 )
                {
                    fprintf(stderr,"%s",temp);
                    bug( "Fread_char: unknown group. ", 0 );
                }
                else
		    gn_add(ch,gn);
                fMatch = TRUE;
            }
	    break;

	case 'H':
	    KEY( "Hardcore",         ch->pcdata->hardcore,  fread_number( fp ) );
	    KEY( "Hitroll",	ch->hitroll,		fread_number( fp ) );
	    KEY( "Hit",		ch->hitroll,		fread_number( fp ) );
        KEY( "House",   ch->pcdata->house,  fread_number( fp ) );

	    if ( !str_cmp( word, "HpManaMove" ) || !str_cmp(word,"HMV"))
	    {
		ch->hit		= fread_number( fp );
		ch->max_hit	= fread_number( fp );
		ch->mana	= fread_number( fp );
		ch->max_mana	= fread_number( fp );
		ch->move	= fread_number( fp );
		ch->max_move	= fread_number( fp );
		fMatch = TRUE;
		break;
	    }

            if ( !str_cmp( word, "HpManaMovePerm" ) || !str_cmp(word,"HMVP"))
            {
                ch->pcdata->perm_hit	= fread_number( fp );
                ch->pcdata->perm_mana   = fread_number( fp );
                ch->pcdata->perm_move   = fread_number( fp );
                fMatch = TRUE;
                break;
            }
      
	    break;

	case 'I':
  if (!str_cmp( word, "Ignore"))
  {
    if (count >= MAX_IGNORE)
    {
      fread_to_eol(fp);
      fMatch = TRUE;
      break;
    }

    ch->pcdata->ignore[count]        =      fread_string(fp);
    count++;
    fMatch = TRUE;
    break;
  }

	    KEY( "Id",		ch->id,			fread_number( fp ) );
	    KEY( "InvisLevel",	ch->invis_level,	fread_number( fp ) );
	    KEY( "Inco",	ch->incog_level,	fread_number( fp ) );
	    KEY( "Invi",	ch->invis_level,	fread_number( fp ) );
	    break;

        case 'J':
                 if ( !str_cmp( word, "Job" ))
                 {
                        ch->pcdata->job = fread_string( fp );
                        sprintf( buf, "%s", ch->pcdata->job );
                        free_string( ch->pcdata->job );
                        ch->pcdata->job = str_dup( buf );
                        fMatch = TRUE;
                 }
                 break;

	case 'L':
	    KEY( "LastLevel",	ch->pcdata->last_level, fread_number( fp ) );
	    KEY( "LLev",	ch->pcdata->last_level, fread_number( fp ) );
        // LEGEND SYSTEM
	    KEY( "Legend",         ch->pcdata->legend,  fread_number( fp ) );
        
	    KEY( "Level",	ch->level,		fread_number( fp ) );
	    KEY( "Lev",		ch->level,		fread_number( fp ) );
	    KEY( "Levl",	ch->level,		fread_number( fp ) );
	    KEY( "LogO",	lastlogoff,		fread_number( fp ) );
	    KEY( "LongDescr",	ch->long_descr,		fread_string( fp ) );
	    KEY( "LnD",		ch->long_descr,		fread_string( fp ) );
	    break;

	case 'N':
	    KEYS( "Name",	ch->name,		fread_string( fp ) );
	    break;
    
        case 'O':
	    KEY( "OCla",   ch->pcdata->oldcl,     fread_number( fp ) );
	    KEY( "OldLvl",	ch->oldlvl,		fread_number( fp ) );
            break;

	case 'P':
	    KEY( "PAct",	ch->pact,		fread_flag( fp ) );
	    KEY( "Password",	ch->pcdata->pwd,	fread_string( fp ) );
	    KEY( "Pass",	ch->pcdata->pwd,	fread_string( fp ) );
	    KEY( "Played",	ch->played,		fread_number( fp ) );
	    KEY( "Plyd",	ch->played,		fread_number( fp ) );
	    KEY( "Points",	ch->pcdata->points,	fread_number( fp ) );
	    KEY( "PermSize",	ch->perm_size,		fread_number( fp ) );
	    KEY( "PSize",	ch->perm_size,		fread_number( fp ) );
	    KEY( "Pnts",	ch->pcdata->points,	fread_number( fp ) );
	    KEY( "Position",	ch->position,		fread_number( fp ) );
	    KEY( "Pos",		ch->position,		fread_number( fp ) );
	    KEY( "Practice",	ch->practice,		fread_number( fp ) );
	    KEY( "Prac",	ch->practice,		fread_number( fp ) );
	    KEY( "PrevHCLevel", ch->pcdata->previousHCLevel, fread_number(fp));
        KEYS( "Prompt",      ch->prompt,             fread_string( fp ) );
 	    KEY( "Prom",	ch->prompt,		fread_string( fp ) );
	    break;

        case 'Q':
            KEY( "QuestPnts",   ch->questpoints,        fread_number( fp ) );
            KEY( "QuestNext",   ch->nextquest,          fread_number( fp ) );
            break;

	case 'R':
	    KEY( "Race",        ch->race,	race_lookup(fread_string( fp )) );
	    KEY("RaceChan",		ch->racechan,		fread_flag( fp ) ); 

	    KEY( "Rem",         ch->pcdata->remorting,  fread_number( fp ) );
	    if ( !str_cmp( word, "Room" ) )
	    {
		ch->in_room = get_room_index( fread_number( fp ) );
		if ( ch->in_room == NULL )
		    ch->in_room = get_room_index( ROOM_VNUM_LIMBO );
		fMatch = TRUE;
		break;
	    }

	    break;

	case 'S':
	    KEY( "SavingThrow",	ch->saving_throw,	fread_number( fp ) );
	    KEY( "Save",	ch->saving_throw,	fread_number( fp ) );
	    KEY( "Scro",	ch->lines,		fread_number( fp ) );
	    KEY( "Sex",		ch->sex,		fread_number( fp ) );
	    KEY( "ShortDescr",	ch->short_descr,	fread_string( fp ) );
	    KEY( "ShD",		ch->short_descr,	fread_string( fp ) );
	    KEY( "Shares",	ch->pcdata->shares,	fread_number(fp));
 	    KEY( "Shares_bought",	ch->pcdata->shares_bought,	fread_number(fp));
	    KEY( "Share_level",	ch->pcdata->share_level,	fread_number(fp));
	    KEY( "Sec",         ch->pcdata->security,	fread_number( fp ) );	/* OLC */
            KEY( "Silv",        ch->silver,             fread_number( fp ) );
	    KEY( "Silv_bank", ch->pcdata->silver_bank,  fread_number(fp));
	    KEY( "Spou",        ch->pcdata->spouse,     fread_string( fp ) );

	    if ( !str_cmp( word, "Skill" ) || !str_cmp(word,"Sk"))
	    {
		int sn;
		int value;
		char *temp;

		value = fread_number( fp );
		temp = fread_word( fp ) ;
		sn = skill_lookup(temp);
		/* sn    = skill_lookup( fread_word( fp ) ); */
		if ( sn < 0 )
		{
		    fprintf(stderr,"%s",temp);
		    bug( "Fread_char: unknown skill. ", 0 );
		}
		else
		    ch->pcdata->learned[sn] = value;
		fMatch = TRUE;
	    }

	    break;

	case 'T':
            KEY( "TrueSex",     ch->pcdata->true_sex,  	fread_number( fp ) );
	    KEY( "TSex",	ch->pcdata->true_sex,   fread_number( fp ) );
	    KEY( "Trai",	ch->train,		fread_number( fp ) );

	    if ( !str_cmp( word, "Title" )  || !str_cmp( word, "Titl"))
	    {
		ch->pcdata->title = fread_string( fp );
    		if (ch->pcdata->title[0] != '.' && ch->pcdata->title[0] != ',' 
		&&  ch->pcdata->title[0] != '!' && ch->pcdata->title[0] != '?')
		{
		    sprintf( buf, " %s", ch->pcdata->title );
		    free_string( ch->pcdata->title );
		    ch->pcdata->title = str_dup( buf );
		}
		fMatch = TRUE;
		break;
	    }

	    break;

	case 'V':
	    KEY( "Version",     ch->version,		fread_number ( fp ) );
	    KEY( "Vers",	ch->version,		fread_number ( fp ) );
	    if ( !str_cmp( word, "Vnum" ) )
	    {
		ch->pIndexData = get_mob_index( fread_number( fp ) );
		fMatch = TRUE;
		break;
	    }
	    break;

	case 'W':
	    KEY( "WName",	ch->who_name,		fread_string( fp ) );
	    KEY( "Wimpy",	ch->wimpy,		fread_number( fp ) );
	    KEY( "Wimp",	ch->wimpy,		fread_number( fp ) );
	    KEY( "Wizn",	ch->wiznet,		fread_flag( fp ) );
	    break;
	}

	if ( !fMatch )
	{
	    bug( "Fread_char: No FMatch",0);
	    fread_to_eol( fp );
	}
    }
}

/* load a pet from the forgotten reaches */
void fread_pet( CHAR_DATA *ch, FILE *fp )
{
    char *word;
    CHAR_DATA *pet;
    bool fMatch;
    int lastlogoff = current_time;
    int percent;

    /* first entry had BETTER be the vnum or we barf */
    word = feof(fp) ? "END" : fread_word(fp);
    if (!str_cmp(word,"Vnum"))
    {
    	int vnum;
    	
    	vnum = fread_number(fp);
    	if (get_mob_index(vnum) == NULL)
	{
    	    bug("Fread_pet: bad vnum %d.",vnum);
	    pet = create_mobile(get_mob_index(MOB_VNUM_FIDO));
	}
    	else
    	    pet = create_mobile(get_mob_index(vnum));
    }
    else
    {
        bug("Fread_pet: no vnum in file.",0);
        pet = create_mobile(get_mob_index(MOB_VNUM_FIDO));
    }
    
    for ( ; ; )
    {
    	word 	= feof(fp) ? "END" : fread_word(fp);
    	fMatch = FALSE;
    	
    	switch (UPPER(word[0]))
    	{
    	case '*':
    	    fMatch = TRUE;
    	    fread_to_eol(fp);
    	    break;
    		
    	case 'A':
    	    KEY( "Act",		pet->act,		fread_flag(fp));
    	    KEY( "AfBy",	pet->affected_by,	fread_flag(fp));
    	    KEY( "AfBy2",	pet->affected2_by,	fread_flag(fp));
    	    KEY( "Alig",	pet->alignment,		fread_number(fp));
    	    
    	    if (!str_cmp(word,"ACs"))
    	    {
    	    	int i;
    	    	
    	    	for (i = 0; i < 4; i++)
    	    	    pet->armor[i] = fread_number(fp);
    	    	fMatch = TRUE;
    	    	break;
    	    }
    	    
    	    if (!str_cmp(word,"AffD"))
    	    {
    	    	AFFECT_DATA *paf;
    	    	int sn;
    	    	
    	    	paf = new_affect();
    	    	
    	    	sn = skill_lookup(fread_word(fp));
    	     	if (sn < 0)
    	     	    bug("Fread_char: unknown skill.",0);
    	     	else
    	     	   paf->type = sn;
    	     	   
    	     	paf->level	= fread_number(fp);
    	     	paf->duration	= fread_number(fp);
    	     	paf->modifier	= fread_number(fp);
    	     	paf->location	= fread_number(fp);
    	     	paf->bitvector	= fread_number(fp);
    	     	paf->next	= pet->affected;
    	     	pet->affected	= paf;
    	     	fMatch		= TRUE;
    	     	break;
    	    }

            if (!str_cmp(word,"Affc"))
            {
                AFFECT_DATA *paf;
                int sn;
 
                paf = new_affect();
 
                sn = skill_lookup(fread_word(fp));
                if (sn < 0)
                    bug("Fread_char: unknown skill.",0);
                else
                   paf->type = sn;
 
		paf->where	= fread_number(fp);
                paf->level      = fread_number(fp);
                paf->duration   = fread_number(fp);
                paf->modifier   = fread_number(fp);
                paf->location   = fread_number(fp);
                paf->bitvector  = fread_number(fp);
                paf->next       = pet->affected;
                pet->affected   = paf;
                fMatch          = TRUE;
                break;
            }
    	     
    	    if (!str_cmp(word,"AMod"))
    	    {
    	     	int stat;
    	     	
    	     	for (stat = 0; stat < MAX_STATS; stat++)
    	     	    pet->mod_stat[stat] = fread_number(fp);
    	     	fMatch = TRUE;
    	     	break;
    	    }
    	     
    	    if (!str_cmp(word,"Attr"))
    	    {
    	         int stat;
    	         
    	         for (stat = 0; stat < MAX_STATS; stat++)
    	             pet->perm_stat[stat] = fread_number(fp);
    	         fMatch = TRUE;
    	         break;
    	    }
    	    break;
    	     
    	 case 'C':
             KEY( "Clan",	pet->clan,	clan_lookup(fread_string(fp)));
    	     KEY( "Comm",	pet->comm,		fread_flag(fp));
    	     break;
    	     
    	 case 'D':
    	     KEY( "Dam",	pet->damroll,		fread_number(fp));
    	     KEY( "Desc",	pet->description,	fread_string(fp));
    	     break;
    	     
    	 case 'E':
    	     if (!str_cmp(word,"End"))
	     {
		pet->leader = ch;
		pet->master = ch;
		ch->pet = pet;
    		/* adjust hp mana move up  -- here for speed's sake */
    		percent = (current_time - lastlogoff) * 25 / ( 2 * 60 * 60);
 
    		if (percent > 0 && !IS_AFFECTED(ch,AFF_POISON)
    		&&  !IS_AFFECTED(ch,AFF_PLAGUE))
    		{
		    percent = UMIN(percent,100);
    		    pet->hit	+= (pet->max_hit - pet->hit) * percent / 100;
        	    pet->mana   += (pet->max_mana - pet->mana) * percent / 100;
        	    pet->move   += (pet->max_move - pet->move)* percent / 100;
    		}
    	     	return;
	     }
    	     KEY( "Exp",	pet->exp,		fread_number(fp));
    	     break;
    	     
    	 case 'G':
    	     KEY( "Gold",	pet->gold,		fread_number(fp));
    	     break;
    	     
    	 case 'H':
    	     KEY( "Hit",	pet->hitroll,		fread_number(fp));
    	     
    	     if (!str_cmp(word,"HMV"))
    	     {
    	     	pet->hit	= fread_number(fp);
    	     	pet->max_hit	= fread_number(fp);
    	     	pet->mana	= fread_number(fp);
    	     	pet->max_mana	= fread_number(fp);
    	     	pet->move	= fread_number(fp);
    	     	pet->max_move	= fread_number(fp);
    	     	fMatch = TRUE;
    	     	break;
    	     }
    	     break;
    	     
     	case 'L':
    	     KEY( "Levl",	pet->level,		fread_number(fp));
    	     KEY( "LnD",	pet->long_descr,	fread_string(fp));
	     KEY( "LogO",	lastlogoff,		fread_number(fp));
    	     break;
    	     
    	case 'N':
    	     KEY( "Name",	pet->name,		fread_string(fp));
    	     break;
    	     
    	case 'P':
    	     KEY( "Pos",	pet->position,		fread_number(fp));
    	     break;
    	     
	case 'R':
    	    KEY( "Race",	pet->race, race_lookup(fread_string(fp)));
    	    break;
 	    
    	case 'S' :
    	    KEY( "Save",	pet->saving_throw,	fread_number(fp));
    	    KEY( "Sex",		pet->sex,		fread_number(fp));
    	    KEY( "ShD",		pet->short_descr,	fread_string(fp));
            KEY( "Silv",        pet->silver,            fread_number( fp ) );
    	    break;
    	    
    	if ( !fMatch )
    	{
    	    bug("Fread_pet: no match.",0);
    	    fread_to_eol(fp);
    	}
    	
    	}
    }
}

extern	OBJ_DATA	*obj_free;

void fread_obj( CHAR_DATA *ch, FILE *fp )
{
    OBJ_DATA *obj;
    char *word;
    int iNest;
    bool fMatch;
    bool fNest;
    bool fVnum;
    bool first;
    bool new_format;  /* to prevent errors */
    bool make_new;    /* update object */
    
    fVnum = FALSE;
    obj = NULL;
    first = TRUE;  /* used to counter fp offset */
    new_format = FALSE;
    make_new = FALSE;

    word   = feof( fp ) ? "End" : fread_word( fp );
    if (!str_cmp(word,"Vnum" ))
    {
        int vnum;
	first = FALSE;  /* fp will be in right place */
 
        vnum = fread_number( fp );
        if (  get_obj_index( vnum )  == NULL )
	{
            bug( "Fread_obj: bad vnum %d.", vnum );
	     vnum=9;
	}
	    obj = create_object(get_obj_index(vnum),-1);
	    new_format = TRUE;
	    
    }

    if (obj == NULL)  /* either not found or old style */
    {
    	obj = new_obj();
    	obj->name		= str_dup( "" );
    	obj->short_descr	= str_dup( "" );
    	obj->description	= str_dup( "" );
    }

    fNest		= FALSE;
    fVnum		= TRUE;
    iNest		= 0;

    for ( ; ; )
    {
	if (first)
	    first = FALSE;
	else
	    word   = feof( fp ) ? "End" : fread_word( fp );
	fMatch = FALSE;

	switch ( UPPER(word[0]) )
	{
	case '*':
	    fMatch = TRUE;
	    fread_to_eol( fp );
	    break;

	case 'A':
	    if (!str_cmp(word,"AffD"))
	    {
		AFFECT_DATA *paf;
		int sn;

		paf = new_affect();

		sn = skill_lookup(fread_word(fp));
		if (sn < 0)
		    bug("Fread_obj: unknown skill.",0);
		else
		    paf->type = sn;

		paf->level	= fread_number( fp );
		paf->duration	= fread_number( fp );
		paf->modifier	= fread_number( fp );
		paf->location	= fread_number( fp );
		paf->bitvector	= fread_number( fp );
		paf->next	= obj->affected;
		obj->affected	= paf;
		fMatch		= TRUE;
		break;
	    }
            if (!str_cmp(word,"Affc"))
            {
                AFFECT_DATA *paf;
                int sn;
 
                paf = new_affect();
 
                sn = skill_lookup(fread_word(fp));
                if (sn < 0)
                    bug("Fread_obj: unknown skill.",0);
                else
                    paf->type = sn;
 
		paf->where	= fread_number( fp );
                paf->level      = fread_number( fp );
                paf->duration   = fread_number( fp );
                paf->modifier   = fread_number( fp );
                paf->location   = fread_number( fp );
                paf->bitvector  = fread_number( fp );
                paf->next       = obj->affected;
                obj->affected   = paf;
                fMatch          = TRUE;
                break;
            }
	    break;

	case 'C':
	    KEY( "Cond",	obj->condition,		fread_number( fp ) );
	    KEY( "Cost",	obj->cost,		fread_number( fp ) );
	    break;

	case 'D':
	    KEY( "Description",	obj->description,	fread_string( fp ) );
	    KEY( "Desc",	obj->description,	fread_string( fp ) );
	    break;

	case 'E':

	    if ( !str_cmp( word, "Enchanted"))
	    {
		obj->enchanted = TRUE;
	 	fMatch 	= TRUE;
		break;
	    }

	    KEY( "ExtraFlags",	obj->extra_flags,	fread_number( fp ) );
	    KEY( "ExtF",	obj->extra_flags,	fread_number( fp ) );

	    if ( !str_cmp( word, "ExtraDescr" ) || !str_cmp(word,"ExDe"))
	    {
		EXTRA_DESCR_DATA *ed;

		ed = new_extra_descr();

		ed->keyword		= fread_string( fp );
		ed->description		= fread_string( fp );
		ed->next		= obj->extra_descr;
		obj->extra_descr	= ed;
		fMatch = TRUE;
	    }

	    if ( !str_cmp( word, "End" ) )
	    {
		if ( !fNest || ( fVnum && obj->pIndexData == NULL ) )
		{
		    bug( "Fread_obj: incomplete object.", 0 );
		    free_obj(obj);
		    return;
		}
		else
	        {
		    if ( !fVnum )
		    {
			free_obj( obj );
			obj = create_object( get_obj_index( OBJ_VNUM_DUMMY ), 0 );
		    }

		    if (!new_format)
		    {
		    	obj->next	= object_list;
		    	object_list	= obj;
		    	obj->pIndexData->count++;
		    }

		    if (!obj->pIndexData->new_format 
		    && obj->item_type == ITEM_ARMOR
		    &&  obj->value[1] == 0)
		    {
			obj->value[1] = obj->value[0];
			obj->value[2] = obj->value[0];
		    }
		    if (make_new)
		    {
			int wear;
			
			wear = obj->wear_loc;
			extract_obj(obj);

			obj = create_object(obj->pIndexData,0);
			obj->wear_loc = wear;
		    }
		    if ( iNest == 0 || rgObjNest[iNest] == NULL )
			obj_to_char( obj, ch );
		    else
			obj_to_obj( obj, rgObjNest[iNest-1] );
		    return;
		}
	    }
	    break;

	case 'I':
	    KEY( "ItemType",	obj->item_type,		fread_number( fp ) );
	    KEY( "Ityp",	obj->item_type,		fread_number( fp ) );
	    break;

	case 'L':
	    KEY( "Level",	obj->level,		fread_number( fp ) );
	    KEY( "Lev",		obj->level,		fread_number( fp ) );
	    break;

        case 'M':
            KEY( "Material",	obj->material_type,		fread_number( fp ) );
            KEY( "MatR",	obj->material_type,		fread_number( fp ) );
	    break;

	case 'N':
	    KEY( "Name",	obj->name,		fread_string( fp ) );

	    if ( !str_cmp( word, "Nest" ) )
	    {
		iNest = fread_number( fp );
		if ( iNest < 0 || iNest >= MAX_NEST )
		{
		    bug( "Fread_obj: bad nest %d.", iNest );
		}
		else
		{
		    rgObjNest[iNest] = obj;
		    fNest = TRUE;
		}
		fMatch = TRUE;
	    }
	    break;

   	case 'O':
	    if ( !str_cmp( word,"Oldstyle" ) )
	    {
		if (obj->pIndexData != NULL && obj->pIndexData->new_format)
		    make_new = TRUE;
		fMatch = TRUE;
	    }
	    break;
		    

	case 'S':
	    KEY( "ShortDescr",	obj->short_descr,	fread_string( fp ) );
	    KEY( "ShD",		obj->short_descr,	fread_string( fp ) );

	    if ( !str_cmp( word, "Spell" ) )
	    {
		int iValue;
		int sn;

		iValue = fread_number( fp );
		sn     = skill_lookup( fread_word( fp ) );
		if ( iValue < 0 || iValue > 4 )
		{
		    bug( "Fread_obj: bad iValue %d.", iValue );
		}
		else if ( sn < 0 )
		{
		    bug( "Fread_obj: unknown skill.", 0 );
		}
		else
		{
		    obj->value[iValue] = sn;
		}
		fMatch = TRUE;
		break;
	    }

	    break;

	case 'T':
	    KEY( "Timer",	obj->timer,		fread_number( fp ) );
	    KEY( "Time",	obj->timer,		fread_number( fp ) );
	    break;

	case 'V':
	    if ( !str_cmp( word, "Values" ) || !str_cmp(word,"Vals"))
	    {
		obj->value[0]	= fread_number( fp );
		obj->value[1]	= fread_number( fp );
		obj->value[2]	= fread_number( fp );
		obj->value[3]	= fread_number( fp );
		if (obj->item_type == ITEM_WEAPON && obj->value[0] == 0)
		   obj->value[0] = obj->pIndexData->value[0];
		fMatch		= TRUE;
		break;
	    }

	    if ( !str_cmp( word, "Val" ) )
	    {
		obj->value[0] 	= fread_number( fp );
	 	obj->value[1]	= fread_number( fp );
	 	obj->value[2] 	= fread_number( fp );
		obj->value[3]	= fread_number( fp );
		obj->value[4]	= fread_number( fp );
		fMatch = TRUE;
		break;
	    }

	    if ( !str_cmp( word, "Vnum" ) )
	    {
		int vnum;

		vnum = fread_number( fp );
		if ( ( obj->pIndexData = get_obj_index( vnum ) ) == NULL )
	{
		    bug( "Fread_obj: bad vnum %d.", vnum );
	vnum=9;
	}
		    fVnum = TRUE;
		fMatch = TRUE;
		break;
	    }
	    break;

	case 'W':
	    KEY( "WearFlags",	obj->wear_flags,	fread_number( fp ) );
	    KEY( "WeaF",	obj->wear_flags,	fread_number( fp ) );
	    KEY( "WearLoc",	obj->wear_loc,		fread_number( fp ) );
	    KEY( "Wear",	obj->wear_loc,		fread_number( fp ) );
	    KEY( "Weight",	obj->weight,		fread_number( fp ) );
	    KEY( "Wt",		obj->weight,		fread_number( fp ) );
	    break;

	}

	if ( !fMatch )
	{
	    bug( "Fread_obj: no match.", 0 );
	    fread_to_eol( fp );
	}
    }
}
