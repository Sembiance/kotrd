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

// Explorer/Killer Percentages
char *          gTopKillers=0;
char *          gTopExplorers=0;
char *          gTopTreasureHunters=0;
char *          gTopKiller=0;
char *          gTopExplorer=0;
char *          gTopTreasureHunter=0;
unsigned long   gTotalMobs;
unsigned long   gTotalRooms;
unsigned long   gTotalObjects;

bool valid_explorer_killer(CHAR_DATA *ch)
{
    if(!ch || !ch->pcdata)
        return FALSE;
    
    if(ch->level==IMPLEMENTOR)
        return TRUE;
    
    if(ch->level==LEVEL_HERO && IS_REMORT(ch))
        return TRUE;
        
    if(IS_HARDCORE(ch) && ch->level>=50)
        return TRUE;
    
    if(IS_LEGEND(ch))
        return TRUE;
    
    return FALSE;
}

char ** GetCommaDelimitedArray(char * dynamicFilePath)
{
    int             dynamicOpen;
    char *          dynamicBuffer=0;
    FILE *          dynamicFP=0;
    unsigned long   dynamicSize=0;
    struct stat	    dynamicStat;
    char **         toReturn=0;

    if(access(dynamicFilePath, 0)!=0)
        return 0;

	if(!(dynamicOpen=open(dynamicFilePath, 0)))
	   return 0;
	   
    fstat(dynamicOpen, &dynamicStat);
	close(dynamicOpen);
	
    dynamicSize = (dynamicStat.st_size<0 ? 0 : (unsigned long)dynamicStat.st_size);
    if(dynamicSize<1)
        return 0;
        
    dynamicBuffer = (char *)malloc(dynamicSize+2);
    memset(dynamicBuffer, 0, dynamicSize+2);
    dynamicFP = fopen(dynamicFilePath, "r");
    if(!dynamicFP)
    {
        dynamicBuffer = strfree(dynamicBuffer);
        return 0;
    }
    
    fgets(dynamicBuffer, dynamicSize+1, dynamicFP);
    fclose(dynamicFP);
    
    toReturn = strchrexplode(dynamicBuffer, ',');

    dynamicBuffer = strfree(dynamicBuffer);

    return toReturn;
}

void init_area_object_lists(void)
{
    OBJ_INDEX_DATA *    obj;
    OBJ_DATA *          mobObj;
    OBJ_DATA *          roomObj;
    AREA_DATA *         pArea;
    ROOM_INDEX_DATA *   room;
    CHAR_DATA *         mobChar;
    bool                notFindable;
    char                buf[1024];
    bool                invalidRoom;
    int                 iWear, vnum, roomVnum, i, z;

    gTotalObjects=0;

    for(pArea=area_first;pArea;pArea=pArea->next)
    {
        for(vnum=pArea->min_vnum;vnum<=pArea->max_vnum;vnum++)
        {
            obj = get_obj_index(vnum);
            
            if(!obj ||
               !CAN_WEAR(obj, ITEM_TAKE) ||
               IS_OBJ_STAT(obj, ITEM_CLAN_EQ) ||
               obj->item_type == ITEM_MONEY ||
               vnum==OBJ_VNUM_SCHOOL_SWORD || vnum==OBJ_VNUM_SCHOOL_DAGGER || vnum==OBJ_VNUM_SCHOOL_SPEAR ||
               vnum==OBJ_VNUM_SCHOOL_MACE || vnum==OBJ_VNUM_SCHOOL_AXE || vnum==OBJ_VNUM_SCHOOL_FLAIL ||
               vnum==OBJ_VNUM_SCHOOL_WHIP || vnum==OBJ_VNUM_SCHOOL_POLEARM || vnum==OBJ_VNUM_SCHOOL_STAFF ||
               vnum==OBJ_VNUM_SCHOOL_VEST || vnum==OBJ_VNUM_SCHOOL_SHIELD || vnum==OBJ_VNUM_SCHOOL_BANNER)
                continue;
            
            notFindable=TRUE;
            
            // Look in all mobs on mud for a mob carrying the object
            for(mobChar=char_list;mobChar;mobChar=mobChar->next)
            {
			    if(!IS_NPC(mobChar) ||
                   mobChar->in_room->clanowner>0 || 
                   (mobChar->in_room->exit[0]==NULL && mobChar->in_room->exit[1]==NULL && mobChar->in_room->exit[2]==NULL && mobChar->in_room->exit[3]==NULL && mobChar->in_room->exit[4]==NULL && 
                    mobChar->in_room->exit[5]==NULL && mobChar->in_room->exit[6]==NULL && mobChar->in_room->exit[7]==NULL && mobChar->in_room->exit[8]==NULL && mobChar->in_room->exit[9]==NULL) ||
                   IS_SET(mobChar->in_room->room_flags, ROOM_MUD_SCHOOL) ||
                   IS_SET(mobChar->in_room->room_flags, ROOM_DEATHTRAP) ||
                   IS_SET(mobChar->in_room->room_flags, ROOM_IMP_ONLY) ||
                   IS_SET(mobChar->in_room->room_flags, ROOM_GODS_ONLY) ||
                   IS_SET(mobChar->in_room->room_flags, ROOM_NEWBIES_ONLY) ||
                   IS_SET(mobChar->in_room->room_flags, ROOM_DRAGONPIT) ||
                   IS_SET(mobChar->in_room->area->area_flags, AREA_PROTO) ||
                   IS_SET(mobChar->in_room->room_flags, ROOM_SAFE) ||
                   mobChar->in_room->vnum<100 ||
                   mobChar->pIndexData->pShop ||
                   mobChar->in_room->vnum==ROOM_VNUM_LIMBO || mobChar->in_room->vnum==ROOM_VNUM_IMM || mobChar->in_room->vnum==ROOM_VNUM_SCHOOL || 
                   mobChar->in_room->vnum==ROOM_VNUM_JAIL || mobChar->in_room->vnum==ROOM_VNUM_DRAGONPIT || mobChar->in_room->vnum==ROOM_VNUM_DRAGONPIT_RETURN || 
                   mobChar->in_room->vnum==ROOM_VNUM_BANK_THIEF || mobChar->in_room->vnum==ROOM_VNUM_LIMBO || mobChar->in_room->vnum==ROOM_VNUM_LIMBO || 
                   mobChar->in_room->vnum==ROOM_VNUM_LIMBO || mobChar->in_room->vnum==ROOM_VNUM_LIMBO || mobChar->in_room->vnum==ROOM_VNUM_LIMBO)
			        continue;

                for(invalidRoom=FALSE,i=0;i<MAX_CLASS && !invalidRoom;i++)
                {
                    for(z=0;z<MAX_GUILD;z++)
                    {
                        if(class_table[i].guild[z]==mobChar->in_room->vnum)
                        {
                            invalidRoom = TRUE;
                            break;
                        }
                    }
                }
                
                if(invalidRoom)
                    continue;
			        
			    for(iWear=0;iWear<MAX_WEAR;iWear++)
			    {
                    if((mobObj=get_eq_char(mobChar, iWear))!=NULL && mobObj->pIndexData && mobObj->pIndexData->vnum==vnum)
                    {
                        notFindable = false;
                        break;
                    }
			    }
			    
			    if(notFindable==false)
			        break;
			        
			    for(mobObj=mobChar->carrying;mobObj!=NULL;mobObj=mobObj->next_content)
			    {
			        if(mobObj->wear_loc==WEAR_INVENTORY && mobObj->pIndexData && mobObj->pIndexData->vnum==vnum)
			        {
			            notFindable = false;
			            break;
			        }
			    }

			    if(notFindable==false)
			        break;
			}
			
			// Look in all rooms for the object just in the room
			for(roomVnum=1;roomVnum<32767;roomVnum++)
			{
                room = get_room_index(vnum);
                
                if(!room ||
                   room->clanowner>0 || 
                   (room->exit[0]==NULL && room->exit[1]==NULL && room->exit[2]==NULL && room->exit[3]==NULL && room->exit[4]==NULL && 
                    room->exit[5]==NULL && room->exit[6]==NULL && room->exit[7]==NULL && room->exit[8]==NULL && room->exit[9]==NULL) ||
                   IS_SET(room->room_flags, ROOM_MUD_SCHOOL) ||
                   IS_SET(room->room_flags, ROOM_DEATHTRAP) ||
                   IS_SET(room->room_flags, ROOM_IMP_ONLY) ||
                   IS_SET(room->room_flags, ROOM_GODS_ONLY) ||
                   IS_SET(room->room_flags, ROOM_NEWBIES_ONLY) ||
                   IS_SET(room->room_flags, ROOM_DRAGONPIT) ||
                   IS_SET(room->area->area_flags, AREA_PROTO) ||
                   vnum<100 || 
                   vnum==ROOM_VNUM_LIMBO || vnum==ROOM_VNUM_IMM || vnum==ROOM_VNUM_SCHOOL || 
                   vnum==ROOM_VNUM_JAIL || vnum==ROOM_VNUM_DRAGONPIT || vnum==ROOM_VNUM_DRAGONPIT_RETURN || 
                   vnum==ROOM_VNUM_BANK_THIEF || vnum==ROOM_VNUM_LIMBO || vnum==ROOM_VNUM_LIMBO || 
                   vnum==ROOM_VNUM_LIMBO || vnum==ROOM_VNUM_LIMBO || vnum==ROOM_VNUM_LIMBO)
                    continue;
                
                for(roomObj=room->contents;roomObj!=NULL;roomObj=roomObj->next_content)
                {
                    if(roomObj->pIndexData && roomObj->pIndexData->vnum==vnum)
                    {
			            notFindable = false;
			            break;
                    }
                }
                
			    if(notFindable==false)
			        break;                
			}
			
			if(notFindable==TRUE)
			    continue;
            
            sprintf(buf, "%d", vnum);
            pArea->objectVnums = array_append(pArea->objectVnums, buf);
            gTotalObjects++;
        }
    }
}

void init_area_room_lists(void)
{
    AREA_DATA *         pArea;
    int                 vnum;
    char                buf[1024];
    ROOM_INDEX_DATA *   room;
    bool                invalidRoom;
    int                 i, z;
    
    gTotalRooms=0;

    for(pArea=area_first;pArea;pArea=pArea->next)
    {
        for(vnum=pArea->min_vnum;vnum<=pArea->max_vnum;vnum++)
        {
            room = get_room_index(vnum);
            
            if(!room ||
               room->clanowner>0 || 
               (room->exit[0]==NULL && room->exit[1]==NULL && room->exit[2]==NULL && room->exit[3]==NULL && room->exit[4]==NULL && 
                room->exit[5]==NULL && room->exit[6]==NULL && room->exit[7]==NULL && room->exit[8]==NULL && room->exit[9]==NULL) ||
               IS_SET(room->room_flags, ROOM_MUD_SCHOOL) ||
               IS_SET(room->room_flags, ROOM_DEATHTRAP) ||
               IS_SET(room->room_flags, ROOM_IMP_ONLY) ||
               IS_SET(room->room_flags, ROOM_GODS_ONLY) ||
               IS_SET(room->room_flags, ROOM_NEWBIES_ONLY) ||
               IS_SET(room->room_flags, ROOM_DRAGONPIT) ||
               IS_SET(room->area->area_flags, AREA_PROTO) ||
               vnum<100 || 
               vnum==ROOM_VNUM_LIMBO || vnum==ROOM_VNUM_IMM || vnum==ROOM_VNUM_SCHOOL || 
               vnum==ROOM_VNUM_JAIL || vnum==ROOM_VNUM_DRAGONPIT || vnum==ROOM_VNUM_DRAGONPIT_RETURN || 
               vnum==ROOM_VNUM_BANK_THIEF || vnum==ROOM_VNUM_LIMBO || vnum==ROOM_VNUM_LIMBO || 
               vnum==ROOM_VNUM_LIMBO || vnum==ROOM_VNUM_LIMBO || vnum==ROOM_VNUM_LIMBO)
                continue;
            
            for(invalidRoom=FALSE,i=0;i<MAX_CLASS && !invalidRoom;i++)
            {
                for(z=0;z<MAX_GUILD;z++)
                {
                    if(class_table[i].guild[z]==vnum)
                    {
                        invalidRoom = TRUE;
                        break;
                    }
                }
            }
            
            if(invalidRoom)
                continue;
                
            sprintf(buf, "%d", vnum);
            pArea->roomVnums = array_append(pArea->roomVnums, buf);
            gTotalRooms++;
        }
    }
}

void init_area_mob_lists(void)
{
    AREA_DATA *         pArea;
    int                 vnum;
    MOB_INDEX_DATA *    mob;
    char                buf[1024];
    CHAR_DATA *         mobChar;
    bool                notKillable;
    bool                invalidRoom;
    int                 i, z;
    
    gTotalMobs=0;
    
    for(pArea=area_first;pArea;pArea=pArea->next)
    {
        for(vnum=pArea->min_vnum;vnum<=pArea->max_vnum;vnum++)
        {
            mob = get_mob_index(vnum);
            
            if(!mob ||
               mob->pShop!=NULL ||
               IS_SET(mob->area->area_flags, AREA_PROTO) ||
               IS_SET(mob->act, ACT_TRAIN) ||
               IS_SET(mob->act, ACT_PRACTICE) ||
               IS_SET(mob->act, ACT_IS_HEALER) ||
               IS_SET(mob->act, ACT_PET) ||
               IS_SET(mob->act, ACT_BANKER) ||
               IS_SET(mob->act, ACT_FORGER) ||
               IS_SET(mob->act, ACT_QUESTMASTER) ||
               IS_SET(mob->act, ACT_GAIN) ||
               IS_SET(mob->affected_by, AFF_CHARM) ||
               mob->spec_fun==spec_lookup("spec_draglord") ||
               mob->spec_fun==spec_lookup("spec_questmaster") ||
               vnum==MOB_VNUM_CAT || vnum==MOB_VNUM_FIDO || vnum==MOB_VNUM_COW || vnum==MOB_VNUM_WOLF ||
               vnum==MOB_VNUM_BEAR || vnum==MOB_VNUM_RABBIT || vnum==MOB_VNUM_SNAIL || vnum==MOB_VNUM_BOAR ||
               vnum==MOB_VNUM_SLIME || vnum==MOB_VNUM_ZOMBIE || vnum==MOB_VNUM_COMPANION)
                continue;
            
            for(notKillable=TRUE,mobChar=char_list;mobChar;mobChar=mobChar->next)
            {
                if(!IS_NPC(mobChar) || !mobChar->pIndexData || mobChar->pIndexData->vnum!=vnum)
			        continue;
			    
			    if(mobChar->in_room->clanowner>0 || 
                   (mobChar->in_room->exit[0]==NULL && mobChar->in_room->exit[1]==NULL && mobChar->in_room->exit[2]==NULL && mobChar->in_room->exit[3]==NULL && mobChar->in_room->exit[4]==NULL && 
                    mobChar->in_room->exit[5]==NULL && mobChar->in_room->exit[6]==NULL && mobChar->in_room->exit[7]==NULL && mobChar->in_room->exit[8]==NULL && mobChar->in_room->exit[9]==NULL) ||
                   IS_SET(mobChar->in_room->room_flags, ROOM_MUD_SCHOOL) ||
                   IS_SET(mobChar->in_room->room_flags, ROOM_DEATHTRAP) ||
                   IS_SET(mobChar->in_room->room_flags, ROOM_IMP_ONLY) ||
                   IS_SET(mobChar->in_room->room_flags, ROOM_GODS_ONLY) ||
                   IS_SET(mobChar->in_room->room_flags, ROOM_NEWBIES_ONLY) ||
                   IS_SET(mobChar->in_room->room_flags, ROOM_DRAGONPIT) ||
                   IS_SET(mobChar->in_room->area->area_flags, AREA_PROTO) ||
                   IS_SET(mobChar->in_room->room_flags, ROOM_SAFE) ||
                   mobChar->in_room->vnum<100 ||
                   mobChar->in_room->vnum==ROOM_VNUM_LIMBO || mobChar->in_room->vnum==ROOM_VNUM_IMM || mobChar->in_room->vnum==ROOM_VNUM_SCHOOL || 
                   mobChar->in_room->vnum==ROOM_VNUM_JAIL || mobChar->in_room->vnum==ROOM_VNUM_DRAGONPIT || mobChar->in_room->vnum==ROOM_VNUM_DRAGONPIT_RETURN || 
                   mobChar->in_room->vnum==ROOM_VNUM_BANK_THIEF || mobChar->in_room->vnum==ROOM_VNUM_LIMBO || mobChar->in_room->vnum==ROOM_VNUM_LIMBO || 
                   mobChar->in_room->vnum==ROOM_VNUM_LIMBO || mobChar->in_room->vnum==ROOM_VNUM_LIMBO || mobChar->in_room->vnum==ROOM_VNUM_LIMBO)
                    notKillable = TRUE;
			    else
			    {
                    for(invalidRoom=FALSE,i=0;i<MAX_CLASS && !invalidRoom;i++)
                    {
                        for(z=0;z<MAX_GUILD;z++)
                        {
                            if(class_table[i].guild[z]==mobChar->in_room->vnum)
                            {
                                invalidRoom = TRUE;
                                break;
                            }
                        }
                    }
                    
                    if(invalidRoom)
                        notKillable = TRUE;
			        else
    			        notKillable = FALSE;
    		    }
			}
			
			if(notKillable==TRUE)
			    continue;
            
            sprintf(buf, "%d", vnum);
            pArea->mobVnums = array_append(pArea->mobVnums, buf);
            gTotalMobs++;
        }
    }
}

int CustomPercentageSort(const void *a, const void *b)
{
    char    numOne[1024];
    char    numTwo[1024];
    
    sprintf(numOne, *(char **)a);
    sprintf(numTwo, *(char **)b);
    
    strchrrep(numOne, ',', '\0');
    strchrrep(numTwo, ',', '\0');

    return (atol(numOne)<atol(numTwo));
}


char * CreateTopList(char * title, char * suffix, unsigned long gTotalCount)
{
    char *          topList;
    char **         playerFileListing=0;
    char **         playerListing=0;
    char **         ar=0;
    char **         arTwo=0;
    char            buf[MAX_STRING_LENGTH];
    char            playerName[MAX_STRING_LENGTH];
    char            playerCount[1024];
    char **         vnumArray=0;
    unsigned long   totalCountInArea=0,  countInThisArea=0, totalCount=0, i=0;
    AREA_DATA *     pArea;

    playerFileListing = GetDirListing(PLAYER_DIR);
    for(totalCount=0,ar=playerFileListing;ar && *ar;totalCount=0,ar++)
    {
        if(!strendswith(*ar, suffix))
            continue;
            
        sprintf(buf, "%s%s", PLAYER_DIR, *ar);
        vnumArray = GetCommaDelimitedArray(buf);
        
        if(array_len(vnumArray)<1)
        {
            vnumArray = array_free(vnumArray);
            continue;
        }
        
        for(totalCountInArea=0,pArea=area_first;pArea;totalCountInArea=0,pArea=pArea->next)
        {   
            if(strendswith(suffix, "killer"))
            {
                for(arTwo=pArea->mobVnums,totalCountInArea=array_len(pArea->mobVnums),countInThisArea=0;arTwo && *arTwo;arTwo++)
                {
                    if(array_find(vnumArray, *arTwo)!=-1)
                        countInThisArea++;
                }
            }
            else if(strendswith(suffix, "explorer"))
            {
                for(arTwo=pArea->roomVnums,totalCountInArea=array_len(pArea->roomVnums),countInThisArea=0;arTwo && *arTwo;arTwo++)
                {
                    if(array_find(vnumArray, *arTwo)!=-1)
                        countInThisArea++;
                }
            }
            else if(strendswith(suffix, "treasurehunter"))
            {
                for(arTwo=pArea->objectVnums,totalCountInArea=array_len(pArea->objectVnums),countInThisArea=0;arTwo && *arTwo;arTwo++)
                {
                    if(array_find(vnumArray, *arTwo)!=-1)
                        countInThisArea++;
                }
            }
            
            if(totalCountInArea==0)
                continue;
            
            totalCount+=countInThisArea;
         }
         
         vnumArray = array_free(vnumArray);
            
         sprintf(playerName, "%s", *ar);
         strchrrep(playerName, '.', '\0');
         sprintf(buf, "%ld,%s", ((totalCount*100)/gTotalCount), playerName);
        
         playerListing = array_append(playerListing, buf);
    }
    
    playerFileListing = array_free(playerFileListing);
    
    qsort(playerListing, array_len(playerListing), sizeof(char *), CustomPercentageSort);

    topList = strdup("{R/=========================\\\n\r");
    topList = strappend(topList, title);
    topList = strappend(topList, "{R+-------------------------+\n\r");
    
    for(ar=playerListing,i=1;ar && *ar && i<=10;ar++,i++)
    {
        if(i==1)
        {
            if(strendswith(suffix, "killer"))
            {
                gTopKiller = strfree(gTopKiller);
                gTopKiller = strdup(*ar);
            }
            else if(strendswith(suffix, "explorer"))
            {
                gTopExplorer = strfree(gTopExplorer);
                gTopExplorer = strdup(*ar);
            }
            else if(strendswith(suffix, "treasurehunter"))
            {
                gTopTreasureHunter = strfree(gTopTreasureHunter);
                gTopTreasureHunter = strdup(*ar);
            }
        }
        
        sprintf(playerCount, "%s", *ar);
        strchrrep(playerCount, ',', '\0');
        sprintf(buf, "{R| {C%2ld. {W%-12s   {%c%3s%% {R|\n\r", i, strchr(*ar, ',')+1, (atol(playerCount)==100 ? 'G' : 'W'), playerCount);
        topList = strappend(topList, buf);
    }
    
    for(;i<=10;i++)
    {
        sprintf(buf, "{R| {C%2ld. {W                    {R|\n\r", i);
        topList = strappend(topList, buf);
    }
    
    playerListing = array_free(playerListing);
    
    topList = strappend(topList, "{R\\-------------------------/{x\n\r");

    return topList;    
}


void do_updatetop(CHAR_DATA *ch, char *argument )
{
    gTopKillers = strfree(gTopKillers);
    gTopKillers = CreateTopList("{R|       {YTOP KILLERS       {R|\n\r", ".killer", gTotalMobs);
    
    gTopExplorers = strfree(gTopExplorers);
    gTopExplorers = CreateTopList("{R|      {YTOP EXPLORERS      {R|\n\r", ".explorer", gTotalRooms);
    
    gTopTreasureHunters = strfree(gTopTreasureHunters);
    gTopTreasureHunters = CreateTopList("{R|  {YTOP TREASURE HUNTERS{R   |\n\r", ".treasurehunter", gTotalObjects);
}

void do_top_percentages( CHAR_DATA *ch, char *argument )
{
    char **         killerLines;
    char **         explorerLines;
    char **         treasureHunterLines;
    unsigned long   listLength, i;
    char            buf[1024];
    
    killerLines = strstrexplode(gTopKillers, "\n\r");
    explorerLines = strstrexplode(gTopExplorers, "\n\r");
    treasureHunterLines = strstrexplode(gTopTreasureHunters, "\n\r");
    
    if(array_len(killerLines)!=array_len(explorerLines) || array_len(killerLines)!=array_len(treasureHunterLines))
    {
        send_to_char("Please inform Sembiance of unequal array lengths in top percentages\n\r", ch);
        return;
    }
    
    send_to_char("\n\r\n\r", ch);

    listLength = array_len(killerLines);
    for(i=0;i<listLength;i++)
    {
        sprintf(buf, "%s    %s    %s\n\r", explorerLines[i], killerLines[i], treasureHunterLines[i]);
        send_to_char(buf, ch);
    }
}

void do_score_section(CHAR_DATA * ch)
{
    char            buf[MAX_STRING_LENGTH];
    AREA_DATA *     pArea;
    char **         ar=0;          
    unsigned long   totalMobsInArea=0, killedInThisArea=0, totalMobsKilled=0;
    unsigned long   totalRoomsInArea=0, exploredInThisArea=0, totalRoomsExplored=0;
    unsigned long   totalObjectsInArea=0, foundInThisArea=0, totalObjectsFound=0;

    if(!ch || !ch->pcdata)
        return;

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
    
    sprintf(buf, "\n\r{C   Total mobs killed: {W%4ld/%4ld  %3ld%%{x\n\r", totalMobsKilled, gTotalMobs, ((totalMobsKilled*100)/gTotalMobs));
    send_to_char(buf, ch);
    
    sprintf(buf, "{CTotal rooms explored: {W%4ld/%4ld  %3ld%%{x\n\r", totalRoomsExplored, gTotalRooms, ((totalRoomsExplored*100)/gTotalRooms));
    send_to_char(buf, ch);
    
    sprintf(buf, "{C Total objects found: {W%4ld/%4ld  %3ld%%{x\n\r\n\r", totalObjectsFound, gTotalObjects, ((totalObjectsFound*100)/gTotalObjects));
    send_to_char(buf, ch);
}

void do_percenthint( CHAR_DATA *ch, char *argument )
{
  //char arg[MAX_INPUT_LENGTH];
    AREA_DATA *     pArea=0;
    char            buf[MAX_STRING_LENGTH];
    unsigned long   totalMobsInArea=0, killedInThisArea=0;
    unsigned long   totalRoomsInArea=0, exploredInThisArea=0;
    unsigned long   totalObjectsInArea=0, foundInThisArea=0;
    MOB_INDEX_DATA * mob=0;
    ROOM_INDEX_DATA * room=0;
    OBJ_INDEX_DATA * object=0;
    char **         ar=0;
    
    if(!ch || !ch->pcdata)
        return;
    
    for(pArea=area_first;pArea;pArea=pArea->next)
    {
        // mobs to kill
        for(ar=pArea->mobVnums,totalMobsInArea=array_len(pArea->mobVnums),killedInThisArea=0;ar && *ar;ar++)
        {
            if(array_find(ch->pcdata->killed, *ar)!=-1)
                killedInThisArea++;
            else
            {
                mob = get_mob_index(atoi(*ar));
                sprintf(buf, "{CIn Area {x%s{C you still need to kill: {x%s{x {D(%d){x\n\r", pArea->name, mob ? mob->short_descr : "unknown", mob ? mob->vnum : 0);
            }
        }
        
        if((totalMobsInArea-killedInThisArea)<=2 && ((totalMobsInArea-killedInThisArea)>0))
            send_to_char(buf, ch);
            
        // rooms to explore
        for(ar=pArea->roomVnums,totalRoomsInArea=array_len(pArea->roomVnums),exploredInThisArea=0;ar && *ar;ar++)
        {
            if(array_find(ch->pcdata->explored, *ar)!=-1)
                exploredInThisArea++;
            else
            {
                room = get_room_index(atoi(*ar));
                sprintf(buf, "{CIn Area {x%s{C you still need to explore room: {x%s{x {D(%d){x\n\r", pArea->name, room ? room->name : "unknown", room ? room->vnum : 0);
            }
        }
        
        if((totalRoomsInArea-exploredInThisArea)<=2 && ((totalRoomsInArea-exploredInThisArea)>0))
            send_to_char(buf, ch);            
            
        // objects to find
        for(ar=pArea->objectVnums,totalObjectsInArea=array_len(pArea->objectVnums),foundInThisArea=0;ar && *ar;ar++)
        {
            if(array_find(ch->pcdata->objectsFound, *ar)!=-1)
                foundInThisArea++;
            else
            {
                object = get_obj_index(atoi(*ar));
                sprintf(buf, "{CIn Area {x%s{C you still need to find object: {x%s{x {D(%d){x\n\r", pArea->name, object ? object->short_descr : "unknown", object ? object->vnum : 0);
            }
        }
        
        if((totalObjectsInArea-foundInThisArea)<=2 && ((totalObjectsInArea-foundInThisArea)>0))
            send_to_char(buf, ch);            
    }
}

void do_alertme( CHAR_DATA *ch, char *argument )
{
    if(IS_NPC(ch))
        return;
    
    if(ch->pcdata->alertMe)
    {
        ch->pcdata->alertMe = FALSE;
        send_to_char("\n\r{GYou will no longer be alerted when you find a new room, object or kill something new.{x\n\r", ch);
    }
    else
    {
        ch->pcdata->alertMe = TRUE;
        send_to_char("\n\r{GYou will now be alerted when you find a new room, object or kill something new.{x\n\r", ch);
    }
}


void do_percentages( CHAR_DATA *ch, char *argument )
{
    char            buf[MAX_STRING_LENGTH];
    BUFFER *        buffer;
    AREA_DATA *     pArea;
    char **         ar=0;          
    unsigned long   totalMobsInArea=0, killedInThisArea=0, totalMobsKilled=0;
    unsigned long   totalRoomsInArea=0, exploredInThisArea=0, totalRoomsExplored=0;
    unsigned long   totalObjectsInArea=0, foundInThisArea=0, totalObjectsFound=0;
    bool            killedAll, exploredAll, foundAll;
    char            killedBuf[MAX_STRING_LENGTH];
    char            exploredBuf[MAX_STRING_LENGTH];
    char            foundBuf[MAX_STRING_LENGTH];

    if(!ch || !ch->pcdata)
        return;

    buffer = new_buf();

    sprintf(buf, "\n\r{c[{C%-25s{c] [{C%-14s{c] [{C%-14s{c] [{C%-14s{c]{x\n\r", "Area Name", "Killed","Explored","Found");
    add_buf(buffer,buf);

    for(pArea=area_first,killedAll=0,exploredAll=0,foundAll=0;pArea;pArea=pArea->next,killedAll=0,exploredAll=0,foundAll=0)
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
        
        killedAll = (totalMobsInArea==killedInThisArea);
        exploredAll = (totalRoomsInArea==exploredInThisArea);
        foundAll = (totalObjectsInArea==foundInThisArea);
        
        if(killedAll)
            sprintf(killedBuf, "{c[{GCOMPLETE  100%%{c]");
        else
        {
            sprintf(killedBuf, "{c[{%c%4ld/%4ld %3ld%%{c]", ((killedInThisArea==0) ? 'd' : 'W'),
                    killedInThisArea, totalMobsInArea, ((totalMobsInArea<1) ? 100 : ((killedInThisArea*100)/totalMobsInArea)));
        }
        
        if(exploredAll)
            sprintf(exploredBuf, "{c[{GCOMPLETE  100%%{c]");
        else
        {
            sprintf(exploredBuf, "{c[{%c%4ld/%4ld %3ld%%{c]", ((exploredInThisArea==0) ? 'd' : 'W'),
                    exploredInThisArea, totalRoomsInArea, ((totalRoomsInArea<1) ? 100 : ((exploredInThisArea*100)/totalRoomsInArea)));
        }
        
        if(foundAll)
            sprintf(foundBuf, "{c[{GCOMPLETE  100%%{c]");
        else
        {
            sprintf(foundBuf, "{c[{%c%4ld/%4ld %3ld%%{c]", ((foundInThisArea==0) ? 'd' : 'W'),
                    foundInThisArea, totalObjectsInArea, ((totalObjectsInArea<1) ? 100 : ((foundInThisArea*100)/totalObjectsInArea)));
        }
                
        sprintf(buf, "{c[{%c%-25s{c] %s %s %s{x\n\r", ((killedAll&&exploredAll&&foundAll) ? 'G' : ((killedInThisArea==0 && exploredInThisArea==0 && foundInThisArea==0) ? 'd' : 'W')), pArea->name, killedBuf, exploredBuf, foundBuf);
                
        add_buf(buffer, buf);
    }
    
    sprintf(buf, "\n\r{C   Total mobs killed: {W%4ld/%4ld  %3ld%%{x\n\r", totalMobsKilled, gTotalMobs, ((totalMobsKilled*100)/gTotalMobs));
    add_buf(buffer, buf);
    
    sprintf(buf, "{CTotal rooms explored: {W%4ld/%4ld  %3ld%%{x\n\r", totalRoomsExplored, gTotalRooms, ((totalRoomsExplored*100)/gTotalRooms));
    add_buf(buffer, buf);
    
    sprintf(buf, "{C Total objects found: {W%4ld/%4ld  %3ld%%{x\n\r", totalObjectsFound, gTotalObjects, ((totalObjectsFound*100)/gTotalObjects));
    add_buf(buffer, buf);
    
    page_to_char(buf_string(buffer), ch);
    free_buf(buffer);
}

void SaveCommaDelimitedArray(char * dynamicFilePath, char ** values)
{
    char *  dynamicFused=0;
    FILE *  dynamicFP;
    
    if(!dynamicFilePath || !values || !array_len(values))
        return;
        
    unlink(dynamicFilePath);
    dynamicFused = array_fuse(values, ",");
    dynamicFP = fopen(dynamicFilePath, "w");
    fprintf(dynamicFP, "%s", dynamicFused);
    fclose(dynamicFP);
    dynamicFused = strfree(dynamicFused);
}


