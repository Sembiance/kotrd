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

char *const distance[4]=
{
"right here.", "nearby to the %s.", "not far %s.", "off in the distance %s."
};

void scan_list           args((ROOM_INDEX_DATA *scan_room, CHAR_DATA *ch,
                               sh_int depth, sh_int door));
void scan_char           args((CHAR_DATA *victim, CHAR_DATA *ch,
                               sh_int depth, sh_int door));
void do_scan(CHAR_DATA *ch, char *argument)
{
   extern char *const dir_name[];
   char arg1[MAX_INPUT_LENGTH], buf[MAX_INPUT_LENGTH];
   ROOM_INDEX_DATA *scan_room;
   EXIT_DATA *pExit;
   sh_int door, depth;

   argument = one_argument(argument, arg1);

   if (arg1[0] == '\0')
   {
      act("$n looks all around.", ch, NULL, NULL, TO_ROOM);
      send_to_char("Looking around you see:\n\r", ch);
                scan_list(ch->in_room, ch, 0, -1);

      for (door=0;door<10;door++)
      {
         if ((pExit = ch ->in_room->exit[door]) != NULL)
            scan_list(pExit->u1.to_room, ch, 1, door);
      }
      return;
   }
   else if (!str_cmp(arg1, "n") || !str_cmp(arg1, "north")) door = 0;
   else if (!str_cmp(arg1, "e") || !str_cmp(arg1, "east"))  door = 1;
   else if (!str_cmp(arg1, "s") || !str_cmp(arg1, "south")) door = 2;
   else if (!str_cmp(arg1, "w") || !str_cmp(arg1, "west"))  door = 3;
   else if (!str_cmp(arg1, "u") || !str_cmp(arg1, "up" ))   door = 4;
   else if (!str_cmp(arg1, "d") || !str_cmp(arg1, "down"))  door = 5;
   else if (!str_cmp(arg1, "ne") || !str_cmp(arg1, "northeast")) door = 6;
   else if (!str_cmp(arg1, "se") || !str_cmp(arg1, "southeast")) door = 7;
   else if (!str_cmp(arg1, "sw") || !str_cmp(arg1, "southwest")) door = 8;
   else if (!str_cmp(arg1, "nw") || !str_cmp(arg1, "northwest")) door = 9;
   else { send_to_char("Which way do you want to scan?\n\r", ch); return; }


  if (IS_SET(ch->comm, COMM_AFK))
    {   
     send_to_char("AFK mode removed due to PLAYER INPUT. Type 'replay' to see tells.\n\r",ch);
      REMOVE_BIT(ch->comm,COMM_AFK);
    }

   act("You peer intently $T.", ch, NULL, dir_name[door], TO_CHAR);
   act("$n peers intently $T.", ch, NULL, dir_name[door], TO_ROOM);
   sprintf(buf, "Looking %s you see:\n\r", dir_name[door]);
                                                                                  
   scan_room = ch->in_room;

   for (depth = 1; depth < 4; depth++)
   {
      if ((pExit = scan_room->exit[door]) != NULL)
      {
         scan_room = pExit->u1.to_room;
         scan_list(pExit->u1.to_room, ch, depth, door);
      }
   }
   return;
}

void scan_list(ROOM_INDEX_DATA *scan_room, CHAR_DATA *ch, sh_int depth,
               sh_int door)
{
   CHAR_DATA *rch;

   if (scan_room == NULL) return;
   for (rch=scan_room->people; rch != NULL; rch=rch->next_in_room)
   {
      if (rch == ch) continue;
      if (!IS_NPC(rch) && rch->invis_level > ch->level) continue;
      if (can_see(ch, rch) && !IS_AFFECTED(rch, AFF_HIDE)) scan_char(rch, ch, depth, door);
   }
   return;
}

void scan_char(CHAR_DATA *victim, CHAR_DATA *ch, sh_int depth, sh_int door)
{
   extern char *const dir_name[];
   extern char *const distance[];
   char buf[MAX_INPUT_LENGTH], buf2[MAX_INPUT_LENGTH];

   buf[0] = '\0';

   strcat(buf, PERS(victim, ch));
   strcat(buf, ", ");
   sprintf(buf2, distance[depth], dir_name[door]);
   strcat(buf, buf2);
   strcat(buf, "\n\r");
 
   send_to_char(buf, ch);
   return;
}
