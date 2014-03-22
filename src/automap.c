#include <string.h>
#include <stdlib.h>
#include <stdio.h> 
#include <time.h> 
#include "merc.h"
#include "interp.h"

#define MAX_MAP 72
#define MAX_MAP_DIR 4

char *map[MAX_MAP][MAX_MAP];
int offsets[4][2] ={ {-1, 0},{ 0, 1},{ 1, 0},{ 0,-1} };

void MapArea 
(ROOM_INDEX_DATA *room, CHAR_DATA *ch, int x, int y, int min, int max)
{
ROOM_INDEX_DATA *prospect_room;
EXIT_DATA *pexit;
int door;

/* marks the room as visited */
switch (room->sector_type) 
{
case SECT_INSIDE:	map[x][y]="{WI";		break;
case SECT_CITY:		map[x][y]="{cC";		break;
case SECT_FIELD:	map[x][y]="{G\"";		break;
case SECT_FOREST:	map[x][y]="{g@";		break;
case SECT_HILLS:	map[x][y]="{G^";		break;
case SECT_MOUNTAIN:	map[x][y]="{y^";		break;
case SECT_WATER_SWIM:	map[x][y]="{B-";		break;
case SECT_WATER_NOSWIM:	map[x][y]="{b~";		break;
case SECT_UNUSED:	map[x][y]="{DX";		break;
case SECT_AIR:		map[x][y]="{C#";		break;
case SECT_DESERT:	map[x][y]="{Y+";		break;
case SECT_UNDERGROUND:	map[x][y]="{wU";		break;
case SECT_ROAD:		map[x][y]="{RR";		break;
default: 		map[x][y]="{m|";
}

    for ( door = 0; door < MAX_MAP_DIR; door++ ) 
    {
	if (
             (pexit = room->exit[door]) != NULL
	     &&   pexit->u1.to_room != NULL 
	     &&   can_see_room(ch,pexit->u1.to_room)  /* optional */
	     &&   !IS_SET(pexit->exit_info, EX_CLOSED)
           )
        { /* if exit there */

	prospect_room = pexit->u1.to_room;

        if ( prospect_room->exit[rev_dir[door]] &&
	 prospect_room->exit[rev_dir[door]]->u1.to_room!=room)
		{ /* if not two way */
		if ((prospect_room->sector_type==SECT_CITY)
		||  (prospect_room->sector_type==SECT_INSIDE))
			map[x][y]="{W@";			
		else
			map[x][y]="{D?";
		return;
		} /* end two way */

        if ((x<=min)||(y<=min)||(x>=max)||(y>=max)) return;
        if (map[x+offsets[door][0]][y+offsets[door][1]]==NULL) {
                MapArea (pexit->u1.to_room,ch,
                    x+offsets[door][0], y+offsets[door][1],min,max);
        }

	} /* end if exit there */
    }
return;
}

void ShowMap( CHAR_DATA *ch, int min, int max)
{
int x,y;

    for (x = min; x < max; ++x) 
    {
         for (y = min; y < max; ++y)
         {
	   if (map[x][y]==NULL) send_to_char(" ",ch);		
	   else 		
           send_to_char(map[x][y],ch); 	
         }
      send_to_char("\n\r",ch); 
    }   
/*
send_to_char("\n\r{cC{w:CITY{c/{WI{w:INSIDE{c/{G\"{w:FIELD{c/{g@{w:FOREST{c/{G^{w:HILLS{c/{y^{w:MOUNTAINS{c/{RR{w:ROAD\n\r{x",ch);
send_to_char("{B-{w:WATER(swim){c/{b~{w:WATER(noswim){c/{C#{w:AIR{c/{Y+{w:DESERT{c/{wU{W:{wUNDERGROUND{x\n\r",ch);
*/
send_to_char("\n\r{cC {wCity  {WI {wInside  {G\" {wField  {g@ {wForest  {G^ {wHill  {y^ {wMountain  {RR {wRoad\n\r{x",ch);
send_to_char("{B- {wWater(swim)  {b~ {wWater(noswim)  {C# {wAir  {Y+ {wDesert  {wU {WUnderground{x\n\r",ch);

return;
}

void do_map( CHAR_DATA *ch, char *argument )
{
int size,center,x,y,min,max;
char arg1[10];

   one_argument( argument, arg1 );
   size = atoi (arg1);

size=URANGE(7,size,72);
center=MAX_MAP/2;

min = MAX_MAP/2-size/2;
max = MAX_MAP/2+size/2;

for (x = 0; x < MAX_MAP; ++x)
        for (y = 0; y < MAX_MAP; ++y)
                  map[x][y]=NULL;

/* starts the mapping with the center room */
MapArea(ch->in_room, ch, center, center, min, max); 

/* marks the center, where ch is */
map[center][center]="{R*";	
ShowMap (ch, min, max); 

return;
}
