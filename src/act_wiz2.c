#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "merc.h"
#include "interp.h"
#include "recycle.h"
#include "tables.h"
#include "lookup.h"
#include "olc.h"
#include "db.h"
#include "magic.h"

/*
 * Local functions.
 */
ROOM_INDEX_DATA *	find_location	args( ( CHAR_DATA *ch, char *arg ) );

void raw_kill( CHAR_DATA *victim, CHAR_DATA *killer );

void do_copyove  (CHAR_DATA *ch, char * argument)
{
    send_to_char( "\n\r{rTo do a {WCOPYOVER{r you {RMUST{r spell it out.{x\n\r", ch );
    return;
}

/*  Copyover - Original idea: Fusion of MUD++
 *  Adapted to Diku by Erwin S. Andreasen, <erwin@andreasen.org>
 *  http://www.andreasen.org
 *  Changed into a ROM patch after seeing the 100th request for it :)
 */
void do_copyover (CHAR_DATA *ch, char * argument)
{
	FILE *fp;
	DESCRIPTOR_DATA *d, *d_next;
	char buf [100], buf2[100];
	extern int port,control; /* db.c */

    do_asave(ch,"world");
    save_clans();
	
	fp = fopen (COPYOVER_FILE, "w");
	
	if (!fp)
	{
		send_to_char ("Copyover file not writeable, aborted.\n\r",ch);
		logf2 ("Could not write to copyover file: %s", COPYOVER_FILE);
		perror ("do_copyover:fopen");
		return;
	}
	
	/* Consider changing all saved areas here, if you use OLC */	
		
	/* For each playing descriptor, save its state */
	for (d = descriptor_list; d ; d = d_next)
	{
		CHAR_DATA * och = CH (d);
		d_next = d->next; /* We delete from the list , so need to save this */
		
		if (!d->character || d->connected > CON_PLAYING) /* drop those logging on */
		{
			send_to_desc("\n\rSorry, we are rebooting. Come back in a few minutes.\n\r", d);
			close_socket (d); /* throw'em out */
		}
		else
		{
			fprintf (fp, "%d %s %s\n", d->descriptor, och->name, d->host);	
			save_char_obj (och);
		}
	}
	
	fprintf (fp, "-1\n");
	fclose (fp);
	
	/* Close reserve and other always-open files and release other resources */
	
	fclose (fpReserve);
	
	/* exec - descriptors are inherited */
	
	sprintf (buf, "%d", port);
	sprintf (buf2, "%d", control);
	execl (EXE_FILE, "rom", buf, "copyover", buf2, (char *) NULL);

	/* Failed - sucessful exec will not return */
	
	perror ("do_copyover: execl");
	send_to_char ("{RCopyover FAILED!{x\n\r",ch);
	
	/* Here you might want to reopen fpReserve */
	fpReserve = fopen (NULL_FILE, "r");
}

/* Recover from a copyover - load players */
void copyover_recover ()
{
	DESCRIPTOR_DATA *d;
	FILE *fp;
	char name [100];
	char host[MSL];
	int desc;
	bool fOld;
	
	logf2 ("Copyover recovery initiated");
	
	fp = fopen (COPYOVER_FILE, "r");
	
	if (!fp) /* there are some descriptors open which will hang forever then ? */
	{
		perror ("copyover_recover:fopen");
		logf2 ("Copyover file not found. Exitting.\n\r");
		exit (1);
	}

	unlink (COPYOVER_FILE); /* In case something crashes - doesn't prevent reading	*/
	
	for (;;)
	{
		fscanf (fp, "%d %s %s\n", &desc, name, host);
		if (desc == -1)
			break;

		d = new_descriptor();
		d->descriptor = desc;
		d->host = str_dup (host);
		d->next = descriptor_list;
		descriptor_list = d;

		d->connected = CON_COPYOVER_RECOVER; /* -15, so close_socket frees the char */
		/* Write something, and check if it goes error-free */		
		if (!write_to_descriptor (d, "\n\rRestoring from copyover...\n\r",0))
		{
			close_socket(d); /* nope */
			continue;
		}
				
	
		/* Now, find the pfile */
		
		fOld = load_char_obj (d, name);


		
		if (!fOld) /* Player file not found?! */
		{
			write_to_descriptor (d, "\n\rSomehow, your character was lost in the copyover. Sorry.\n\r", 0);
			close_socket (d);			
		}
		else /* ok! */
		{
            crn_players++;
            crm_players = UMAX( crn_players, crm_players );
            
	        if( IS_SET( d->character->pact, PLR_COLOUR ) )
	        {
	           d->character->desc->fcolour=TRUE;
	           SET_BIT( d->character->pact, PLR_COLOUR );
            }

			write_to_descriptor (d, "\n\r{WCopyover recovery complete.{x\n\r",0);
	
			/* Just In Case */
			if (!d->character->in_room)
				d->character->in_room = get_room_index (ROOM_VNUM_TEMPLE);


			/* Insert in the char_list */
			d->character->next = char_list;
			char_list = d->character;
			char_to_room (d->character, d->character->in_room);
			do_look (d->character, "auto");
			act ("$n materializes!", d->character, NULL, NULL, TO_ROOM);
			d->connected = CON_PLAYING;
			if (d->character->pet != NULL)
			{
			    char_to_room(d->character->pet,d->character->in_room);
			    act("$n materializes!.",d->character->pet,NULL,NULL,TO_ROOM);
			}
		}
		
	}
   fclose (fp);
}

void do_spellup (CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int level;
    int sn;

    argument = one_argument(argument, arg);

    if (arg[0] == '\0')
    {
        send_to_char( "\n\r{GSyntax: {WSPELLUP {c<{WPLAYER{c>{x\n\r", ch);
        return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
    	send_to_char( "\n\r{RThey are {WNOT{R within the MUD.{x\n\r", ch );
    	return;
    }

    if ( IS_NPC(victim) )
    {
        send_to_char( "\n\r{RNot on NPCs{x\n\r", ch);
        return;
    }

    //level = UMAX(10, victim->level * 15);
    //level = UMIN(200, level);
    level = 500;

    sn = find_spell(ch,"golden aura");
    spell_golden(sn, level, ch, (void *) victim, TARGET_CHAR);
    
    sn = find_spell(ch,"haste");
    spell_haste(sn, level, ch, (void *) victim, TARGET_CHAR);
    
    sn = find_spell(ch,"bless");
    spell_bless(sn, level, ch, (void *) victim, TARGET_CHAR);

    sn = find_spell(ch,"combat mind");
    spell_combat_mind(sn, level, ch, (void *) victim, TARGET_CHAR);
    
    sn = find_spell(ch,"stone skin");
    spell_stone_skin(sn, level, ch, (void *) victim, TARGET_CHAR);
    
    sn = find_spell(ch,"fly");
    spell_fly(sn, level, ch, (void *) victim, TARGET_CHAR);
    
    sn = find_spell(ch,"giant strength");
    spell_giant_strength(sn, level, ch, (void *) victim, TARGET_CHAR);
    
    sn = find_spell(ch,"protection evil");
    spell_protection_evil(sn, level, ch, (void *) victim, TARGET_CHAR);
    
    sn = find_spell(ch,"inertial barrier");
    spell_inertial_barrier(sn, level, ch, (void *) victim, TARGET_CHAR);

    return;
}
    

void do_ftick (CHAR_DATA *ch, char *argument )
{
    update_handler( TRUE );
    send_to_char( "You Have Forced Time To Fly By....TICK\n\r", ch );
    return;
}

void do_arealinks(CHAR_DATA *ch, char *argument)
{
    FILE *fp;
    BUFFER *buffer;
    AREA_DATA *parea;
    EXIT_DATA *pexit;
    ROOM_INDEX_DATA *to_room;
    ROOM_INDEX_DATA *from_room;
    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    int vnum = 0;
    int iHash, door;
    bool found = FALSE;

    /* To provide a convenient way to translate door numbers to words */
    static char * const dir_name[] = {"north","east","south","west","up","down"};

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    /* First, the 'all' option */
    if (!str_cmp(arg1,"all"))
    {
	/*
	 * If a filename was provided, try to open it for writing
	 * If that fails, just spit output to the screen.
	 */
	if (arg2[0] != '\0')
	{
	    fclose(fpReserve);
	    if( (fp = fopen(arg2, "w")) == NULL)
	    {
		send_to_char("Error opening file, printing to screen.\n\r",ch);
		fclose(fp);
		fpReserve = fopen(NULL_FILE, "r");
		fp = NULL;
	    }
	}
	else
	    fp = NULL;

	/* Open a buffer if it's to be output to the screen */
    buffer = new_buf();

	/* Loop through all the areas */
	for (parea = area_first; parea != NULL; parea = parea->next)
	{
	    /* First things, add area name  and vnums to the buffer */
	    sprintf(buf, "*** %s (%d to %d) ***\n\r",
			 parea->name, parea->min_vnum, parea->max_vnum);
	    fp ? fprintf(fp, buf) : add_buf(buffer, buf);

	    /* Now let's start looping through all the rooms. */
	    found = FALSE;
	    for(iHash = 0; iHash < MAX_KEY_HASH; iHash++)
	    {
		for( from_room = room_index_hash[iHash];
		     from_room != NULL;
		     from_room = from_room->next )
		{
		    /*
		     * If the room isn't in the current area,
		     * then skip it, not interested.
		     */
		    if ( from_room->vnum < parea->min_vnum
		    ||   from_room->vnum > parea->max_vnum )
			continue;

		    /* Aha, room is in the area, lets check all directions */
		    for (door = 0; door < 6; door++)
		    {
			/* Does an exit exist in this direction? */
			if( (pexit = from_room->exit[door]) != NULL )
			{
			    to_room = pexit->u1.to_room;

			    /*
			     * If the exit links to a different area
			     * then add it to the buffer/file
			     */
			    if( to_room != NULL
			    &&  (to_room->vnum < parea->min_vnum
			    ||   to_room->vnum > parea->max_vnum) )
			    {
				found = TRUE;
				sprintf(buf, "    (%d) links %s to %s (%d)\n\r",
				    from_room->vnum, dir_name[door],
				    to_room->area->name, to_room->vnum);

				/* Add to either buffer or file */
				if(fp == NULL)
				    add_buf(buffer, buf);
				else
				    fprintf(fp, buf);
			    }
			}
		    }
		}
	    }

	    /* Informative message for areas with no external links */
	    if (!found)
		add_buf(buffer, "    No links to other areas found.\n\r");
	}

	/* Send the buffer to the player */
	if (!fp)
	{
	    page_to_char(buf_string(buffer), ch);
	    free_buf(buffer);
	}
	/* Or just clean up file stuff */
	else
	{
	    fclose(fp);
	    fpReserve = fopen(NULL_FILE, "r");
	}

	return;
    }

    /* No argument, let's grab the char's current area */
    if(arg1[0] == '\0')
    {
	parea = ch->in_room ? ch->in_room->area : NULL;

	/* In case something wierd is going on, bail */
	if (parea == NULL)
	{
	    send_to_char("You aren't in an area right now, funky.\n\r",ch);
	    return;
	}
    }
    /* Room vnum provided, so lets go find the area it belongs to */
    else if(is_number(arg1))
    {
	vnum = atoi(arg1);

	/* Hah! No funny vnums! I saw you trying to break it... */
	if (vnum <= 0 || vnum > 65536)
	{
	    send_to_char("The vnum must be between 1 and 65536.\n\r",ch);
	    return;
	}

	/* Search the areas for the appropriate vnum range */
	for (parea = area_first; parea != NULL; parea = parea->next)
	{
	    if(vnum >= parea->min_vnum && vnum <= parea->max_vnum)
		break;
	}

	/* Whoops, vnum not contained in any area */
	if (parea == NULL)
	{
	    send_to_char("There is no area containing that vnum.\n\r",ch);
	    return;
	}
    }
    /* Non-number argument, must be trying for an area name */
    else
    {
	/* Loop the areas, compare the name to argument */
	for(parea = area_first; parea != NULL; parea = parea->next)
	{
	    if(!str_prefix(arg1, parea->name))
		break;
	}

	/* Sorry chum, you picked a goofy name */
	if (parea == NULL)
	{
	    send_to_char("There is no such area.\n\r",ch);
	    return;
	}
    }

    /* Just like in all, trying to fix up the file if provided */
    if (arg2[0] != '\0')
    {
	fclose(fpReserve);
	if( (fp = fopen(arg2, "w")) == NULL)
	{
	    send_to_char("Error opening file, printing to screen.\n\r",ch);
	    fclose(fp);
	    fpReserve = fopen(NULL_FILE, "r");
	    fp = NULL;
	}
    }
    else
	fp = NULL;

    /* And we loop the rooms */
    for(iHash = 0; iHash < MAX_KEY_HASH; iHash++)
    {
	for( from_room = room_index_hash[iHash];
	     from_room != NULL;
	     from_room = from_room->next )
	{
	    /* Gotta make sure the room belongs to the desired area */
	    if ( from_room->vnum < parea->min_vnum
	    ||   from_room->vnum > parea->max_vnum )
		continue;

	    /* Room's good, let's check all the directions for exits */
	    for (door = 0; door < 6; door++)
	    {
		if( (pexit = from_room->exit[door]) != NULL )
		{
		    to_room = pexit->u1.to_room;

		    /* Found an exit, does it lead to a different area? */
		    if( to_room != NULL
		    &&  (to_room->vnum < parea->min_vnum
		    ||   to_room->vnum > parea->max_vnum) )
		    {
			found = TRUE;
			sprintf(buf, "%s (%d) links %s to %s (%d)\n\r",
				    parea->name, from_room->vnum, dir_name[door],
				    to_room->area->name, to_room->vnum);

			/* File or buffer output? */
			if(fp == NULL)
			    send_to_char(buf, ch);
			else
			    fprintf(fp, buf);
		    }
		}
	    }
	}
    }

    /* Informative message telling you it's not externally linked */
    if(!found)
    {
	send_to_char("No links to other areas found.\n\r",ch);
	/* Let's just delete the file if no links found */
	if (fp)
	    unlink(arg2);
	return;
    }

    /* Close up and clean up file stuff */
    if(fp)
    {
	fclose(fp);
	fpReserve = fopen(NULL_FILE, "r");
    }

}

void do_otype(CHAR_DATA *ch, char *argument)
{
    int type;
    int type2;
    int vnum=1;
    char buf[MAX_STRING_LENGTH];
    char buffer[12 * MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];     
    OBJ_INDEX_DATA *obj;
    bool found;
	
	argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    found = FALSE;
    buffer [0] = '\0';

    if (arg1[0] == '\0')     
        {
        send_to_char("Type 'Help Otype' for usage\n\r",ch);
        return;
        }

    type2 = 0;

    if ( (!str_cmp(arg1,"armor") || !str_cmp(arg1,"weapon"))
        && arg2[0] == '\0')
    {        
        send_to_char("Type 'Help Otype' for proper usage.\n\r",ch);
        return;
    }
    else if (!str_cmp(arg1,"armor"))
    {
        type = flag_value(type_flags,arg1);
        if ((type2 = flag_value(wear_flags,arg2)) == NO_FLAG)
        {

            send_to_char("No such armor type.\n\r",ch);
            return;      

        }
    }
    else if (!str_cmp(arg1,"weapon"))
   {
        type = flag_value(type_flags,arg1);
        if ((type2 = flag_value(weapon_class,arg2)) == NO_FLAG)
        {
            send_to_char("No such weapon type.\n\r",ch);
            return;
        }            

   }
   else
   {

        if((type = flag_value(type_flags,arg1)) == NO_FLAG)
            {
                send_to_char("Unknown Type.\n\r", ch);
                return;
            }
   }        

   for(;vnum <= top_vnum_obj; vnum++)
   {
        if((obj=get_obj_index(vnum)) != NULL)
        {
            if((obj->item_type == type && type2 == 0
                && str_cmp(arg1,"weapon") && str_cmp(arg1,"armor"))
            || (obj->item_type == type && obj->value[0] == type2
                && str_cmp(arg1,"armor"))
            || (obj->item_type == type && IS_SET(obj->wear_flags,type2)
                && str_cmp(arg1,"weapon")))    
            {
               sprintf(buf, "Area [%2d] - %5d - %s\n\r", obj->area->vnum, vnum, obj->short_descr);   
               found = TRUE;
               strcat(buffer,buf);
            }
        }
   }
   if (!found)
        send_to_char("No objects of that type exist\n\r",ch);
   else            
        if (ch->lines)
            page_to_char(buffer,ch);
        else
            send_to_char(buffer,ch);
}

void do_olevel(CHAR_DATA *ch, char *argument)
{
    extern int top_obj_index;    
    OBJ_INDEX_DATA *pObjIndex;
    char buf[MAX_INPUT_LENGTH];
    char level[MAX_INPUT_LENGTH];
    char type[MAX_INPUT_LENGTH];
    char class[MAX_INPUT_LENGTH];
    int vnum;
    int nMatch;
    bool found;
	
	found	= FALSE;
    nMatch	= 0;

    argument = one_argument(argument, level);
	
    if (level[0] == '\0')
    {
        send_to_char("Syntax: olevel <level>\n\r",ch);
        send_to_char("        olevel <level> <type>\n\r",ch);
        send_to_char("        olevel <level> weapon <class>\n\r", ch);
        return;
    }
 
    argument = one_argument(argument, type);

    // ex. olevel 5
    if (type[0] == '\0')  
    {
        for ( vnum = 0; nMatch < top_obj_index; vnum++ )
        {
        	if ( ( pObjIndex = get_obj_index( vnum ) ) != NULL )
        	{
        	    nMatch++;
    
        	    if ( atoi(level) == pObjIndex->level )
        	    {
            		found = TRUE;
            		sprintf( buf, "[%5d] %s\n\r",
            		    pObjIndex->vnum, pObjIndex->short_descr );
            		send_to_char( buf, ch );
        	    }
        	}
        }
    }
    // ex. olevel 5 light
    else if (str_cmp(type,"weapon") && str_cmp(type,"armor") )   
    {
            for ( vnum = 0; nMatch < top_obj_index; vnum++ )
            {
            	if ( ( pObjIndex = get_obj_index( vnum ) ) != NULL )
            	{
            	    nMatch++;
        
            	    if ( ( atoi(level) == pObjIndex->level) && 
                           is_name( type, flag_string( type_flags, pObjIndex->item_type )) )
            	    {
                		found = TRUE;
                		sprintf( buf, "[%5d] %s\n\r",
                		    pObjIndex->vnum, pObjIndex->short_descr );
                		send_to_char( buf, ch );
            	    }
            	}
            }
    }
    else if (!str_cmp(type,"weapon"))
    {
        argument = one_argument(argument, class);

        if (class[0] == '\0')
        {
            for ( vnum = 0; nMatch < top_obj_index; vnum++ )
            {
            	if ( ( pObjIndex = get_obj_index( vnum ) ) != NULL )
            	{
            	    nMatch++;
        
            	    if ( ( atoi(level) == pObjIndex->level) &&
                           is_name( type, flag_string( type_flags, pObjIndex->item_type )) )
            	    {
                		found = TRUE;
                		sprintf( buf, "[%5d] %s\n\r",
                		    pObjIndex->vnum, pObjIndex->short_descr );
                		send_to_char( buf, ch );
            	    }
            	}
            }
        }
        else
        {
            for ( vnum = 0; nMatch < top_obj_index; vnum++ )
            {
            	if ( ( pObjIndex = get_obj_index( vnum ) ) != NULL )
            	{
            	    nMatch++;
        
            	    if ( ( atoi(level) == pObjIndex->level) &&
                           is_name( type, flag_string( type_flags, pObjIndex->item_type )) && 
                           is_name( class, flag_string( weapon_class, pObjIndex->value[0] )) )
            	    {
                		found = TRUE;
                		sprintf( buf, "[%5d] %s\n\r",
                		    pObjIndex->vnum, pObjIndex->short_descr );
                		send_to_char( buf, ch );
            	    }
            	}
            }
        }
    }
    else if (!str_cmp(type,"armor"))
    {
        if (argument[0] == '\0')
        {
            for ( vnum = 0; nMatch < top_obj_index; vnum++ )
            {
            	if ( ( pObjIndex = get_obj_index( vnum ) ) != NULL )
            	{
            	    nMatch++;
        
            	    if ( ( atoi(level) == pObjIndex->level) &&
                           is_name( type, flag_string( type_flags, pObjIndex->item_type )) )
            	    {
                		found = TRUE;
                		sprintf( buf, "[%5d] %s\n\r",
                		    pObjIndex->vnum, pObjIndex->short_descr );
                		send_to_char( buf, ch );
            	    }
            	}
            }
        }
        else
        {
            for ( vnum = 0; nMatch < top_obj_index; vnum++ )
            {
            	if ( ( pObjIndex = get_obj_index( vnum ) ) != NULL )
            	{
            	    nMatch++;
        
            	    if ( ( atoi(level) == pObjIndex->level) &&
                           is_name( type, flag_string( type_flags, pObjIndex->item_type )) && 
                           is_name( argument, flag_string( wear_flags, pObjIndex->wear_flags )) )
            	    {
                		found = TRUE;
                		sprintf( buf, "[%5d] %s\n\r",
                		    pObjIndex->vnum, pObjIndex->short_descr );
                		send_to_char( buf, ch );
            	    }
            	}
            }
        }
    }
    else
    {
        send_to_char("Syntax: olevel <level>\n\r",ch);
        send_to_char("        olevel <level> <type>\n\r",ch);
        send_to_char("        olevel <level> weapon <class>\n\r", ch);
        return;
    }

    if ( !found )
	send_to_char( "No objects found.\n\r", ch );

    return;
}

void do_vlist (CHAR_DATA *ch, char *argument)
{
  int i;
  OBJ_INDEX_DATA *pObjIndex;
  MOB_INDEX_DATA *pMobIndex;
  ROOM_INDEX_DATA *pRoomIndex;
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];

  argument = one_argument(argument,arg);
 
  if (arg[0] == '\0')
    {
      send_to_char("Syntax:\n\r",ch);
      send_to_char("  vlist obj\n\r",ch);
      send_to_char("  vlist mob\n\r",ch);
      send_to_char("  vlist room\n\r",ch);
      return;
    }
  if (!str_cmp(arg,"obj"))
  {
      sprintf(buf,"{C%s{W vnum listing for area {C%s{x\n\r",arg,
		     ch->in_room->area->name);
	  send_to_char( buf, ch );
      sprintf(buf, "{Y=============================================================================={C\n\r");
	  send_to_char( buf, ch);
      for (i = ch->in_room->area->min_vnum; i <= ch->in_room->area->max_vnum; i++)
      {
          if ( (pObjIndex = get_obj_index(i)) != NULL) 
          {
            sprintf( buf, "[%5d] [%3d] %s\n\r",
                pObjIndex->vnum, pObjIndex->level, pObjIndex->short_descr );
            send_to_char( buf, ch );
          }
      }

      return;
  }

  if (!str_cmp(arg,"mob"))
    { 
      sprintf(buf,"{C%s {Wvnum listing for area {C%s{x\n\r",arg,
		     ch->in_room->area->name);
	  send_to_char( buf, ch );
      sprintf(buf,"{Y=============================================================================={C\n\r");
      send_to_char( buf, ch );
      for (i = ch->in_room->area->min_vnum; i <= ch->in_room->area->max_vnum; i++) 
      {
	      if ((pMobIndex = get_mob_index(i)) != NULL) 
          {
              sprintf( buf, "[%5d] [%3d] %s\n\r",
                  pMobIndex->vnum, pMobIndex->level, pMobIndex->short_descr );
              send_to_char( buf, ch );

	      }
      }

      return;
    }
  if (!str_cmp(arg,"room"))
    { 
      sprintf(buf,"{C%s {Wvnum listing for area {C%s{x\n\r",arg,
		     ch->in_room->area->name);
      send_to_char( buf, ch );
      sprintf(buf,"{Y=============================================================================={C\n\r");
      send_to_char( buf, ch );
      for (i = ch->in_room->area->min_vnum; i <= ch->in_room->area->max_vnum; i++)
      {
	      if ((pRoomIndex = get_room_index(i)) != NULL) 
          {
              sprintf( buf, "[%5d] %s\n\r",
                  pRoomIndex->vnum, pRoomIndex->name );
              send_to_char( buf, ch );
	      }
      }
      return;
    }
  send_to_char("WHAT??? \n\r",ch);
  send_to_char("Syntax:\n\r",ch);
  send_to_char("   vlist obj\n\r",ch);
  send_to_char("   vlist mob\n\r",ch);
  send_to_char("   vlist room\n\r",ch);
}


void do_fvlist (CHAR_DATA *ch, char *argument)
{
  char buf[MAX_STRING_LENGTH]; 
  char arg[MAX_INPUT_LENGTH];
  char *string;
  int i,col;
  col = 0;

  string = one_argument(argument,arg); 

  if (arg[0] == '\0')
    {
     if (IS_IMP(ch))
       {
      send_to_char("\n\r{GSyntax{w: {WFVLIST {c<{WOBJ {cor {WMOB {cor {WROOM{c>{x\n\r",ch);
      send_to_char("        {cor {WFV {c<{WOBJ {cor {WMOB {cor {WROOM{c>{x\n\r",ch);
      return;
       }
     else
       {
      send_to_char("\n\r{GSyntax{w: {WFVLIST {c<{WOBJ {cor {WMOB {cor {WROOM{c>{x\n\r",ch);
      return;
       }
    }

  if (!str_cmp(arg,"obj"))
    {
      sprintf(buf,"\n\r{cAvailable {COBJECT VNUMs{c for {C%s {cArea [{W%d{c].{x\n\r",ch->in_room->area->name,ch->in_room->area->vnum);
      send_to_char(buf,ch);

send_to_char("{c==============================================================================={x\n\r",ch);
      for (i = ch->in_room->area->min_vnum; i <= ch->in_room->area->max_vnum; i++) 
      {
	if (get_obj_index(i) == NULL) 
        {
          sprintf( buf,"{c[{W%-5d{c]  ",i);
          send_to_char(buf,ch);

          if ( ++col % 9 == 0 )
          send_to_char( "{x\n\r", ch );
	}
      }
      send_to_char("{x\n\r",ch);
      return;
    }

  if (!str_cmp(arg,"mob"))
    { 
      sprintf(buf,"\n\r{rAvailable {RMOBILE VNUMs{r for {R%s {rArea [{W%d{r].{x\n\r",ch->in_room->area->name,ch->in_room->area->vnum);
      send_to_char(buf,ch);

send_to_char("{r==============================================================================={x\n\r",ch);
      for (i = ch->in_room->area->min_vnum; i <= ch->in_room->area->max_vnum; i++) 
         {
	  if (get_mob_index(i) == NULL) 
            {
          sprintf( buf,"{r[{W%-5d{r]  ",i);
          send_to_char(buf,ch);

          if ( ++col % 9 == 0 )
          send_to_char( "{x\n\r", ch );
  	    }
         }
      send_to_char("{x\n\r",ch);
      return;
    }

  if (!str_cmp(arg,"room"))
    {
sprintf(buf,"\n\r{gAvailable {GROOM VNUMs{g for {G%s {gArea [{W%d{g].{x\n\r",ch->in_room->area->name,ch->in_room->area->vnum);
      send_to_char(buf,ch);

send_to_char("{g==============================================================================={x\n\r",ch);
      for (i = ch->in_room->area->min_vnum; i <= ch->in_room->area->max_vnum; i++) 
         {
	if (get_room_index(i) == NULL) 
          {
          sprintf( buf,"{g[{W%-5d{g]  ",i);
          send_to_char(buf,ch);

          if ( ++col % 9 == 0 )
          send_to_char( "{x\n\r", ch );
	}
      }
      send_to_char("{x\n\r",ch);
      return;
    }

  send_to_char("\n\r{cH{Cu{CH!{x\n\r",ch);

     if (IS_IMP(ch))
       { 
      send_to_char("\n\r{GSyntax{w: {WFVLIST {c<{WOBJ {cor {WMOB {cor {WROOM{c>{x\n\r",ch);
      send_to_char("        {cor {WFV {c<{WOBJ {cor {WMOB {cor {WROOM{c>{x\n\r",ch);   
      return;
       } 
     else
       {
      send_to_char("\n\r{GSyntax{w: {WFVLIST {c<{WOBJ {cor {WMOB {cor {WROOM{c>{x\n\r",ch);
      return;
       }

  return;
}

void do_ctimp( CHAR_DATA *ch, char *argument )
{
  char arg1[MAX_INPUT_LENGTH];
  CLAN_DATA *pClan;
  int iClan;
 
  argument = one_argument( argument, arg1 );


  if ( arg1[0] == '\0')
    {
     send_to_char( "\n\r{GSyntax{w: {WCTIMP {c<{WCLAN NAME{c>{x\n\r",ch);
     return;
    }

  if ((iClan = clan_lookup(arg1)) == 0
  || !(pClan = get_clan_index( iClan )) )
    {
        send_to_char("\n\r{RNO {Wsuch clan exists.{x\n\r",ch);
        return;
    }

  ch->ctimp_clan = iClan;

  return;

}


void do_vnumlist( CHAR_DATA *ch, char *argument )
{
  ROOM_INDEX_DATA *room;
  ROOM_INDEX_DATA *in_room;
  MOB_INDEX_DATA *mob;
  OBJ_INDEX_DATA *obj;
  char buf[MAX_STRING_LENGTH]; 
  char arg[MAX_STRING_LENGTH];
  char arg1[MAX_STRING_LENGTH];
  char arg2[MAX_STRING_LENGTH];
  int uvnum;
  int lvnum;
  int MR = 60000;
  int type = -1;
  int counter = 0;
  
  argument = one_argument( argument, arg );
  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );
  uvnum = ( is_number( arg2 ) ) ? atoi( arg2 ) : 0;
  lvnum = ( is_number( arg1 ) ) ? atoi( arg1 ) : 0;  

  if ( !str_cmp( arg, "o" ) )
    type = 2;
  if ( !str_cmp( arg, "m" ) )
    type = 1;
  if ( !str_cmp( arg, "r" ) )
    type = 0;

  if ( ( ( uvnum == 0 ) && ( lvnum == 0 ) ) || ( arg[0] == '\0' ) 
   || ( type == -1 ) )
  {
send_to_char( "\n\r{GSyntax{x: {WVNUMLIST {c<{WTYPE{c> <{WLOW VNUM{c> <{WUPPER VNUM{c>{x\n\r{x", ch );
send_to_char( "\n\r {cTypes{x: {CM {c({WMOB{c) {CO {c({WOBJECT{c) {CR {c({WROOM{c){x\n\r{x", ch );
    return;
  }

  if ( uvnum > MR || uvnum < 1 || lvnum > MR || lvnum < 1 || lvnum > uvnum )
  {
    send_to_char( "\n\r{RThat is {WNOT {Ra VALID VNUM range{!{x\n\r{x", ch );
    return;
  }

    if ( type == 0 )
    {
      send_to_char("\n\r     {D[{CVNum {D]  {D[{CRoom Name           {D]{x\n\r",ch);
    }
   else
    if ( type == 2 )
      {
       send_to_char("\n\r{D[{CVNum {D]{D[{CLvL{D]{D[{CTrue Name{c({Cs{c)        {D]{D[{CShort Description                                 {D]{x\n\r",ch);
      }
   else
    if ( type == 1 )
      {
       send_to_char("\n\r{D[{CVNum {D]{D[{CLvL{D]{D[{CTrue Name{c({Cs{c)        {D]{D[{CShort Description                                 {D]{x\n\r",ch);
      }


  in_room = ch->in_room;  
  if ( type == 0 )
  {
    char_from_room( ch );
  }
  for ( MR = lvnum; MR <= uvnum; MR++ )
  {
    if ( type == 0 )
    {
      if ( ( room = get_room_index( MR ) ) )
      {
        sprintf( buf, "\n\r{D[{W%d{D] {R%-5d  {w%-20s\n\r",counter, room->vnum,room->name);
        send_to_char( buf, ch );
        char_to_room( ch, room );
        do_resets( ch, "" );
        counter++;

        if (counter == 21)
          break;
        else
          char_from_room( ch );

      }
    }
    if ( type == 2 )
      {
       if ( ( obj = get_obj_index( MR ) ) )
         {
          sprintf( buf, "{D[{R%5d{D]{D[{W%3d{D]{D[{w%-20.20s{D]{D[{w%-50.50s{D]{x\n\r",
  	  obj->vnum, obj->level, obj->name, strip_color(obj->short_descr));
          send_to_char( buf, ch );
          counter++;

          if (counter == 100)
          break;
         }
      }

    if ( type == 1 )
    {
      if ( ( mob = get_mob_index( MR ) ) )
      {
        sprintf( buf, "{D[{R%5d{D]{D[{W%3d{D]{D[{w%-20.20s{D]{D[{w%-50.50s{D] \n\r", 
mob->vnum,mob->level,mob->player_name,strip_color(mob->short_descr));
        send_to_char( buf, ch );
        counter++;

          if (counter == 100)
          break;
      }
    }
  }

    if ( type == 2 )
      {
       sprintf(buf,"\n\r{DTotal OBJECTs Listed{w: {D[{C%d{D]\n\r{c({WMAX # That can be listed at one time is {C100{c){x\n\r",counter);
          send_to_char( buf, ch );
      }
   else
    if ( type == 1 )
      {
       sprintf(buf,"\n\r{DTotal MOBILEs Listed{w: {D[{C%d{D]\n\r{c({WMAX # That can be listed at one time is {C100{c){x\n\r",counter);
       send_to_char( buf, ch );
      }

  if ( type == 0 )
    char_to_room( ch, in_room );
  return;
}


void do_icon( CHAR_DATA *ch, char *argument )
{  
    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    int level;
    int iLevel;

    argument = one_argument( argument, arg1 );
 
    if ( arg1[0] == '\0' || !is_number( arg1 ) )
      {
       send_to_char( "\n\r{GSyntax{x: {c'{WICON {c<{WLEVEL{c>'{x\n\r", ch );
       return;
      }

    if ( IS_NPC(ch) )
      {
       send_to_char( "\n\r{RNOT as a NPC's!{x\n\r", ch );
       return;
      }


    if ( ( level = atoi( arg1 ) ) < 1 || level > MAX_LEVEL )
      {
sprintf(buf, "\n\r{rLevel {RMUST {rbe from {W1 {rto {W100{R OR {rbe your {WIMM {rlevel {D({W%d{D){r!{x\n\r",
       ch->oldlvl);
       send_to_char( buf, ch );
       return;
      }

    if ((level >= 101)
    && (level != ch->oldlvl))
      {
sprintf(buf, "\n\r{rLevel {RMUST {rbe from {W1 {rto {W100{R OR {rbe your {WIMM {rlevel {D({W%d{D){r!{x\n\r",
       ch->oldlvl);
       send_to_char( buf, ch );
       return;
      }


    if ( level <= 100)
      {
       int temp_prac;
       int temp_train;

       temp_prac = ch->practice;
       temp_train = ch->train;

       do_remove(ch, "all"); 
       SET_BIT(ch->comm,COMM_ICON);

       ch->level    = 1;
       ch->exp      = exp_per_level(ch,ch->pcdata->points);
       ch->max_hit  = 20;
       ch->max_mana = 100;
       ch->max_move = 100;
       ch->practice = 0;
       ch->hit      = ch->max_hit;
       ch->mana     = ch->max_mana;
       ch->move     = ch->max_move;
       advance_level( ch,TRUE,FALSE );
       ch->practice = temp_prac;
       ch->train    = temp_train;

       for ( iLevel = ch->level ; iLevel < level; iLevel++ )
          {
           ch->level += 1;
           advance_level( ch,TRUE,FALSE);
          }
    
    ch->exp = exp_per_level(ch,ch->pcdata->points)* UMAX( 1, ch->level );
    act( "\n\r{GYou are now in your {gICON {Gform!\n\r", ch, NULL,NULL, TO_CHAR);
    act( "{GYour current ICON's statistics are{g:\n\r", ch, NULL,NULL, TO_CHAR);
sprintf(buf,"{gCLASS{x: {G%s\n\r{gLEVEL{x: {G%d\n\r {gRACE{x: {G%s\n\r{gHPnts{x: {G%d\n\r {gMove{x: {G%d\n\r {gMana{x: {G%d{x\n\r",
class_table[ch->class].name,ch->level,race_table[ch->race].name,ch->max_hit,ch->max_move,ch->max_mana);
    send_to_char(buf,ch);

    act("\n\r{r*** {RREMEMBER {r- {WYou can now die in this form, becareful with any EQ you might have! {r***{x",ch, NULL,NULL, TO_CHAR);
    do_restore (ch, "self");
       } 
    else
      {
       if (level == ch->oldlvl)
         {
          ch->level = ch->oldlvl;
          ch->exp           = exp_per_level(ch,ch->pcdata->points);
          ch->max_hit       = ch->oldlvl * 160;
          ch->max_mana      = ch->oldlvl * 160;
          ch->max_move 	    = ch->oldlvl * 160;
          ch->train         = 0;
          ch->practice      = 0;
          ch->questpoints   = 0;
          ch->hit           = ch->max_hit;
          ch->mana          = ch->max_mana;
          ch->move          = ch->max_move;
          do_restore (ch, "self");
          REMOVE_BIT(ch->comm,COMM_ICON);
          act( "\n\r{GYou have returned to your TRUE IMMORTAL {Gform!\n\r", ch, NULL,NULL, TO_CHAR);
         }
      }
    save_char_obj(ch);
    return;
}


void do_dpitkill(CHAR_DATA *ch, char *argument)
{
  char arg[MAX_STRING_LENGTH];
  CHAR_DATA *victim;

 one_argument( argument, arg );

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "\n\r{RThey aren't here.{x\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "\n\r{RNot on NPC's.{x\n\r", ch );
	return;
    }


  if (IS_SET(victim->comm, COMM_DPITKILL))
  {
    REMOVE_BIT(victim->comm, COMM_DPITKILL);
    REMOVE_BIT(victim->deaf, CHANNEL_DPTALK );  
    send_to_char("\n\r{GDragonPIT Privileges RESTORED.{x\n\r", ch);
  }
  else
  {  
    SET_BIT(victim->comm, COMM_DPITKILL);
    SET_BIT(victim->deaf, CHANNEL_DPTALK );  
    send_to_char("\n\r{GDragonPIT Privileges REVOKED!!{x\n\r", ch);
  }
}  


void do_plzap( CHAR_DATA *ch, char *argument)
{
   char strsave[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;
   char arg[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];

   one_argument( argument, arg );
   
   if ( arg[0] == '\0' )
     {
      send_to_char( "\n\r{GSyntax{w: {WPLZAP {c<{WCHAR NAME{c>{x\n\r", ch );
      return;
     }

   if (( victim = get_char_world(ch, arg) ) == NULL )
     {
      send_to_char( "\n\r{RThey are not within the Realms!{x\n\r", ch );
      return;
     }

  if (IS_NPC(victim))
    {
     send_to_char( "\n\r{RYou cannot {WPLZAP {RMOBs!{x\n\r", ch);
     return;
    }

    sprintf( strsave, "%s%s", PLAYER_DIR, capitalize( victim->name ) );
    stop_fighting(victim,TRUE);
/*
sprintf( buf,"\n\r{W%s {Dreaches up from the Abyss and removes you from this existence!!!{x\n\r",ch->name);
*/
    sprintf( buf,"\n\r{RYou are outta here PERMENANTLY!!!{x\n\r");
    send_to_char(buf,victim);
    sprintf( buf,"\n\r{cYou erase {C%s {ccompletely from this existence !!!!{x\n\r",victim->name);
    send_to_char(buf,ch);

do_quit(victim,"");
unlink(strsave);
return;
}


void do_pardon( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "\n\r{GSyntax{w: {c'{WPARDON {c<{WCHAR NAME{c> {WFLAG.{x\n\r", ch );
	send_to_char( "\n\r{cFLAGS{w:  {CTHIEF{c.{x\n\r", ch );

         if (IS_IMP(ch))
           {
            if(!is_owner(ch))
            {
	send_to_char( "\n\r{R  IMP ONLY{w: {WPKILLER QUESTOR REMORT TARGET MARRIED HELPER BOUNTY{x\n\r", ch );
            }
          else          
            {
	send_to_char( "\n\r{ROWNER ONLY{w: {WPKILLER QUESTOR REMORT TARGET DPIT MARRIED HELPER BOUNTY{x\n\r", ch);
            }
          }
        return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "\n\r{RThey aren't here.{x\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "\n\r{RNot on NPC's.{x\n\r", ch );
	return;
    }


    if ( !str_cmp( arg2, "thief" ) )
    {
	if ( IS_SET(victim->pact, PLR_THIEF) )
	{
	    REMOVE_BIT( victim->pact, PLR_THIEF );
	    send_to_char( "\n\r{cTHIEF flag removed.{x\n\r", ch );
	    send_to_char( "\n\r{cYou are no longer a THIEF.{x\n\r", victim );
            return;
          } 
        else
          {
           send_to_char("\n\r{RVictim is not flagged with this!{x\n\r",ch);
           return;
          }
    }


  if (IS_IMP(ch))
    {


    if ( !str_cmp( arg2, "pkiller" ) )
    {
      if (IS_IMP(ch))
        {   
  	  if ( IS_SET(victim->pact, PLR_PKILLER) )
	   {
	    REMOVE_BIT( victim->pact, PLR_PKILLER );
	    send_to_char( "\n\r{cPKILLER flag removed.{x\n\r", ch );
	    send_to_char( "\n\r{cYou are no longer a PKILLER.{x\n\r", victim);
            return;
	   }
          else
           {
	    send_to_char( "\n\r{RThat player is not a {WPKiller{R.{x\n\r", ch );
            return;
           }          
         }
       }

    if ( !str_cmp( arg2, "questor" ) )
    {
      if (IS_IMP(ch))
        {   
  	  if ( IS_SET(victim->pact, PLR_QUESTOR) )
	   {
	    REMOVE_BIT( victim->pact, PLR_QUESTOR );
	    send_to_char( "\n\r{cQUESTOR flag removed.{x\n\r", ch );
	    send_to_char( "\n\r{cYour QUEST flag has been removed.{x\n\r", victim);
            victim->nextquest = 10;
            return;
	   }
          else
           {
	    send_to_char( "\n\r{RThat player is not a {WQUESTOR{R.{x\n\r", ch );
            return;
           }          
         }
      return;
    }

    if ( !str_cmp( arg2, "remort" ) )
    {
	if ( IS_SET(victim->pact, PLR_REMORT) )
	{
	    REMOVE_BIT( victim->pact, PLR_REMORT );
	    send_to_char( "\n\r{cREMORT flag removed.{x\n\r", ch );
	    send_to_char( "\n\r{cYou are no longer flagged a REMORT!{x\n\r", victim );
            return;
          } 
        else
          {
           send_to_char("\n\r{RVictim is not flagged with this!{x\n\r",ch);
           return;
          }
    }

    if ( !str_cmp( arg2, "target" ) )
    {
	if ( IS_SET(victim->pact, PLR_TARGET) )
	{
	    REMOVE_BIT( victim->pact, PLR_TARGET );
	    send_to_char( "\n\r{cTARGET flag removed.{x\n\r", ch );
            return;
          } 
        else
          {
           send_to_char("\n\r{RVictim is not flagged with this!{x\n\r",ch);
           return;
          }
    }

    if ( !str_cmp( arg2, "married" ) )
    {
     if ( !IS_NPC(victim) && victim->pcdata->spouse > 0 )
       {
        victim->pcdata->spouse = 0;        
	send_to_char( "\n\r{cVictim is {CNO {clonger {WMARRIED{c.{x\n\r", ch );
        do_save (victim, "");
            return;
          } 
        else
          {
           send_to_char("\n\r{RVictim is not {WMARRIED{R!{x\n\r",ch);
           return;
          }
    }

    if ( !str_cmp( arg2, "helper" ) )
    {
      if (IS_IMP(ch))
        {   
  	  if ( IS_SET(victim->comm, COMM_HELPER) )
	   {
	    REMOVE_BIT( victim->comm, COMM_HELPER );
	    send_to_char( "\n\r{cHELPER flag removed.{x\n\r", ch );
	    send_to_char( "\n\r{cYou are no longer flagged as an IMP HELPER.{x\n\r",victim);
            return;
	   }
          else
           {
	    send_to_char( "\n\r{RThat player is not an IMP {WHELPER{R.{x\n\r", ch );
            return;
           }          
         }
      return;
    }


    if ( !str_cmp( arg2, "bounty" ) )
    {
     if ( !IS_NPC(victim) && victim->pcdata->bounty > 0 )
       {
        victim->pcdata->bounty = 0;        
        do_save (victim, "");
	sprintf(buf,"\n\r{CBOUNTY {cremoved from {W%s{c's Fragile Little Head!{x\n\r",capitalize(victim->name) );
         send_to_char(buf,ch);
            return;
          } 
        else
          {
           sprintf(buf,"\n\r{RThere is no {rBOUNTY {Ron {W%s{R's Fragile Little Head!{x\n\r",capitalize(victim->name));
           send_to_char(buf,ch);
           return;
          }
    }
}

            if(is_owner(ch))
            {

            if ( !str_cmp( arg2, "dpit" ) )
               {
 	if ( IS_SET(victim->pact, PLR_DRAGONPIT) )
  	  {
	    REMOVE_BIT( victim->pact, PLR_DRAGONPIT );
	    send_to_char( "\n\r{cIN DRAGONPIT flag removed.{x\n\r", ch );
   	    return;
	  }
        else
          {
           send_to_char("\n\r{RVictim is not flagged with this!{x\n\r",ch);
   	   return;
          }
                }



            }
   

	send_to_char( "\n\r{GSyntax{w: {c'{WPARDON {c<{WCHAR NAME{c> {WFLAG.{x\n\r", ch );
	send_to_char( "\n\r{cFLAGS{w:  {CTHIEF{c.{x\n\r", ch );

         if (IS_IMP(ch))
           {
            if(!is_owner(ch))
            {
	send_to_char( "\n\r{RIMP ONLY{w: {WPKILLER QUESTOR REMORT TARGET MARRIED HELPER BOUNTY{x\n\r", ch );
            }
          else          
            {
	send_to_char( "\n\r{ROWNER ONLY{w: {WPKILLER QUESTOR REMORT TARGET DPIT MARRIED HELPER BOUNTY{x\n\r", ch);
            }
          }
        return;
}

void do_emboo( CHAR_DATA *ch, char *argument )
{
    send_to_char( "\n\r{rTo do an {WEMERGENCY REBOOT{r you {RMUST{r type {WEMBOOT{r.{x\n\r", ch );
    return;
}

void do_emboot( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    extern bool merc_down;
    DESCRIPTOR_DATA *d,*d_next;

    	sprintf( buf, "{rAn {REMERGENCY {WREBOOT{r has been done.{x\n\r"
                      "{GYou will be able to log back on in a couple of minutes... {C;){x\n\r\n\r");
    	do_function(ch, &do_gecho, buf );

    for ( d = descriptor_list; d != NULL; d = d_next )
    {
	d_next = d->next;
        close_socket(d);
    }

    merc_down = TRUE;
    return;
}


void do_nopnote( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "\n\r{GNOPNOTE whom?{x\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "\n\r{RThey aren't here.{x\n\r", ch );
	return;
    }


    if (!IS_IMP(ch))
      {
    if ( victim->level >= ch->level )
    {
	send_to_char( "\n\r{RYou failed.{x\n\r", ch );
	return;
    }
      }

    if ( IS_SET(victim->comm, COMM_NOPNOTE) )
    {
	REMOVE_BIT(victim->comm, COMM_NOPNOTE);
        SET_BIT(victim->pact, PLR_PNOTE);
	send_to_char( "\n\r{gYour {WNOTE PRIVILEGEs{g have been {GRETURNED{g!{x\n\r",victim );
	sprintf(buf, "\n\r{cYou have {CRETURNED{W %s{c's {CNOTE PRIVILEGEs{x.{x\n\r",victim->name );
        send_to_char(buf, ch);
	sprintf(buf,"$N restores NOTE PRIVs to %s.",victim->name);
    wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,ch->level);
    }
    else
    {
	SET_BIT(victim->comm, COMM_NOPNOTE);
	REMOVE_BIT(victim->pact, PLR_PNOTE);
	send_to_char( "\n\r{rYour {WNOTE PRIVILEGEs {rhave been {RREVOKED{r!{x\n\r",victim );
	sprintf(buf, "\n\r{gYou have {GREVOKED {W%s{r's {WNOTE PRIVILEGEs{r!{x\n\r",victim->name );
        send_to_char(buf, ch);
	sprintf(buf,"$N revokes %s's NOTE PRIVs.",victim->name);
    wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,ch->level);
    }

    return;
}


void do_noemote( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Noemote whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }


    if (victim->level >= ch->level )
    {
	send_to_char( "\n\r{RYou failed{r!{x\n\r", ch );
	return;
    }

    if ( IS_SET(victim->comm, COMM_NOEMOTE) )
    {
	REMOVE_BIT(victim->comm, COMM_NOEMOTE);
	send_to_char( "You can emote again.\n\r", victim );
	send_to_char( "NOEMOTE removed.\n\r", ch );
	sprintf(buf,"$N restores EMOTEs to %s.",victim->name);
    wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,ch->level);
    }
    else
    {
	SET_BIT(victim->comm, COMM_NOEMOTE);
	send_to_char( "You can't emote!\n\r", victim );
	send_to_char( "NOEMOTE set.\n\r", ch );
	sprintf(buf,"$N revokes %s's EMOTEs.",victim->name);
    wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,ch->level);
    }
    return;
}


void do_notitle( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
         {
        send_to_char( "\n\r{WRemove the {GTITLE{W command from who?{x\n\r", ch );
        return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
         {
        send_to_char( "\n\r{WThey aren't here.{x\n\r", ch );
        return;
    }

    if (victim->level > ch->level )
    {
        send_to_char( "\n\r{RYou failed.{x\n\r", ch );
                  return;
    }

    if ( IS_SET(victim->comm, COMM_NOTITLE) )
    {
        REMOVE_BIT(victim->comm, COMM_NOTITLE);
        send_to_char( "\n\r{GYou can use the {WTITLE{G command once more.{x\n\r",victim );
        send_to_char( "\n\r{WThe {BTITLE{W command has been restored.{x\n\r", ch );
        sprintf(buf,"\n\r{G$N {Wreturns the {RTITLE{W command to {G%s{x\n\r",victim->name);
    wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,ch->level);
    }
    else
    {
        SET_BIT(victim->comm, COMM_NOTITLE);
        set_title(victim, "says 'I lost TITLE because I am IMMATURE!'");
        send_to_char( "\n\r{RThe {WTITLE{R command has been taken from you!{x\n\r",victim);
        send_to_char( "\n\r{cTheir {CTITLE{c commandhas been removed!{x\n\r", ch );
        sprintf(buf,"\n\r{B$N {Whas taken {c%s's {WTITLE command{x\n\r",victim->name);
    wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,ch->level);
         }

    return;
}


void do_dummy( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

	 one_argument( argument, arg );

    if ( arg[0] == '\0' )
	 {
        send_to_char( "\n\r{WTurn who into a {Rdummy{W?{x\n\r", ch );
        return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
	 {
        send_to_char( "\n\r{WThey aren't here.{x\n\r", ch );
        return;

    }

    if (victim->level >= ch->level )
    {
        send_to_char( "\n\r{RYou failed.{x\n\r", ch );
		  return;
    }

    if ( IS_SET(victim->comm, COMM_DUMMY) )
    {
        REMOVE_BIT(victim->comm, COMM_DUMMY);
	send_to_char( "\n\r{GDuhh... your no longer a dummy.{x\n\r",victim );
        send_to_char( "\n\r{WThey are no longer a dummy now..{x\n\r", ch );
	sprintf(buf,"\n\r{G$N {Wremoves dummy from {G%s{x\n\r",victim->name);
    wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,ch->level);
    }
    else
    {
	SET_BIT(victim->comm, COMM_DUMMY);
        set_title(victim, "says 'Duh, I sure am a dummy! DUHHHH! DUHHHH!'");
        send_to_char( "\n\r{RYou are SUCH a big {RDUMMY{x\n\r",victim);
        send_to_char( "\n\r{cThey are now a dummy!{x\n\r", ch );
	sprintf(buf,"\n\r{B$N {Whas turned {c%s {Winto a dummy!{x\n\r",victim->name);
    wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,ch->level);
	 }
 
    return;
     
} 


void do_notell( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Notell whom?", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim->level >= ch->level )
    {
	send_to_char( "\n\r{RYou failed{r!{x\n\r", ch );
	return;
    }

    if ( IS_SET(victim->comm, COMM_NOTELL) )
    {
	REMOVE_BIT(victim->comm, COMM_NOTELL);
	send_to_char( "You can tell again.\n\r", victim );
	send_to_char( "NOTELL removed.\n\r", ch );
	sprintf(buf,"$N restores tells to %s.",victim->name);
    wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,ch->level);
    }
    else
    {
	SET_BIT(victim->comm, COMM_NOTELL);
	send_to_char( "You can't tell!\n\r", victim );
	send_to_char( "NOTELL set.\n\r", ch );
	sprintf(buf,"$N revokes %s's tells.",victim->name);
    wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,ch->level);
    }

    return;
}


void do_sset( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int value;
    int sn;
    bool fAll;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );

/*
    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
	send_to_char( "Syntax:\n\r",ch);
	send_to_char( "  set skill <name> <spell or skill> <value>\n\r", ch);
	send_to_char( "  set skill <name> all <value>\n\r",ch);  
	send_to_char("   (use the name of the skill, not the number)\n\r",ch);
	return;
    }
*/

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    fAll = !str_cmp( arg2, "all" );
    sn   = 0;
    if ( !fAll && ( sn = skill_lookup( arg2 ) ) < 0 )
    {
	send_to_char( "No such skill or spell.\n\r", ch );
	return;
    }

    /*
     * Snarf the value.
     */
    if ( !is_number( arg3 ) )
    {
	send_to_char( "Value must be numeric.\n\r", ch );
	return;
    }

    value = atoi( arg3 );
    if ( value < 0 || value > 100 )
    {
	send_to_char( "Value range is 0 to 100.\n\r", ch );
	return;
    }

    if ( fAll )
    {
	for ( sn = 0; sn < MAX_SKILL; sn++ )
	{
	    if ( skill_table[sn].name != NULL )
		victim->pcdata->learned[sn]	= value;
	}
    }
    else
    {
	victim->pcdata->learned[sn] = value;
    }
    return;
}


void do_disable (CHAR_DATA *ch, char *argument)
{
	int i;
	DISABLED_DATA *p,*q;
	char buf[100];
	
	if (IS_NPC(ch))
	{
		send_to_char ("\n\rRETURN first.\n\r",ch);
		return;
	}
	
	if (!argument[0]) /* Nothing specified. Show disabled commands. */
	{
		if (!disabled_first) /* Any disabled at all ? */
		{
			send_to_char ("\n\r{GThere are no commands disabled.{x\n\r",ch);
			return;
		}

		send_to_char ("\n\r{cDisabled Commands{w:{x\n\r"
		              "{CCommand       Level   Disabled by{x\n\r",ch);
		                
		for (p = disabled_first; p; p = p->next)
		{
		sprintf (buf, "{W%-12s %5d    {R%-12s{x\n\r",p->command->name,p->level,p->disabled_by);
		send_to_char (buf,ch);
		}
		return;
	}
	
	/* command given */

	/* First check if it is one of the disabled commands */
	for (p = disabled_first; p ; p = p->next)
		if (!str_cmp(argument, p->command->name))
			break;
			
	if (p) /* this command is disabled */
	{
	/* Optional: The level of the imm to enable the command must equal or exceed level
	   of the one that disabled it */
	
		if (ch->level < p->level)
		{
			send_to_char ("This command was disabled by a higher power.\n\r",ch);
			return;
		}
		
		/* Remove */
		
		if (disabled_first == p) /* node to be removed == head ? */
			disabled_first = p->next;
		else /* Find the node before this one */
		{
			for (q = disabled_first; q->next != p; q = q->next); /* empty for */
			q->next = p->next;
		}
		
		free_string (p->disabled_by); /* free name of disabler */
		free_mem (p,sizeof(DISABLED_DATA)); /* free node */
		save_disabled(); /* save to disk */
		send_to_char ("Command enabled.\n\r",ch);
	}
	else /* not a disabled command, check if that command exists */
	{
		/* IQ test */
		if (!str_cmp(argument,"disable"))
		{
			send_to_char ("You cannot disable the disable command.\n\r",ch);
			return;
		}

		/* Search for the command */
		for (i = 0; cmd_table[i].name[0] != '\0'; i++)
			if (!str_cmp(cmd_table[i].name, argument))
				break;

		/* Found? */				
		if (cmd_table[i].name[0] == '\0')
		{
			send_to_char ("No such command.\n\r",ch);
			return;
		}

		/* Can the imm use this command at all ? */				
		if (cmd_table[i].level > ch->level)
		{
			send_to_char ("You dot have access to that command; you cannot disable it.\n\r",ch);
			return;
		}
		
		/* Disable the command */
		
		p = alloc_mem (sizeof(DISABLED_DATA));

		p->command = &cmd_table[i];
		p->disabled_by = str_dup (ch->name); /* save name of disabler */
		p->level = ch->level; 
		p->next = disabled_first;
		disabled_first = p; /* add before the current first element */
		
		send_to_char ("Command disabled.\n\r",ch);
		save_disabled(); /* save to disk */
	}
}

/* Check if that command is disabled 
   Note that we check for equivalence of the do_fun pointers; this means
   that disabling 'chat' will also disable the '.' command
*/   
bool check_disabled (const struct cmd_type *command)
{
	DISABLED_DATA *p;
	
	for (p = disabled_first; p ; p = p->next)
		if (p->command->do_fun == command->do_fun)
			return TRUE;

	return FALSE;
}

/* Load disabled commands */
void load_disabled()
{
	FILE *fp;
	DISABLED_DATA *p;
	char *name;
	int i;
	
	disabled_first = NULL;
	
	fp = fopen (DISABLED_FILE, "r");
	
	if (!fp) /* No disabled file.. no disabled commands : */
		return;
		
	name = fread_word (fp);
	
	while (str_cmp(name, END_MARKER)) /* as long as name is NOT END_MARKER :) */
	{
		/* Find the command in the table */
		for (i = 0; cmd_table[i].name[0] ; i++)
			if (!str_cmp(cmd_table[i].name, name))
				break;
				
		if (!cmd_table[i].name[0]) /* command does not exist? */
		{
			bug ("Skipping uknown command in " DISABLED_FILE " file.",0);
			fread_number(fp); /* level */
			fread_word(fp); /* disabled_by */
		}
		else /* add new disabled command */
		{
			p = alloc_mem(sizeof(DISABLED_DATA));
			p->command = &cmd_table[i];
			p->level = fread_number(fp);
			p->disabled_by = str_dup(fread_word(fp)); 
			p->next = disabled_first;
			
			disabled_first = p;

		}
		
		name = fread_word(fp);
	}

	fclose (fp);		
}

/* Save disabled commands */
void save_disabled()
{
	FILE *fp;
	DISABLED_DATA *p;
	
	if (!disabled_first) /* delete file if no commands are disabled */
	{
		unlink (DISABLED_FILE);
		return;
	}
	
	fp = fopen (DISABLED_FILE, "w");
	
	if (!fp)
	{
		bug ("Could not open " DISABLED_FILE " for writing",0);
		return;
	}
	
	for (p = disabled_first; p ; p = p->next)
		fprintf (fp, "%s %d %s\n", p->command->name, p->level, p->disabled_by);
		
	fprintf (fp, "%s\n",END_MARKER);
		
	fclose (fp);
}


void do_slay( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char type[MAX_INPUT_LENGTH];
    char who[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int i=0;
    bool found=FALSE;

    argument = one_argument(argument, who);
    argument = one_argument(argument, type);


    if ( !str_prefix(who, "list") || who == NULL )
      {
       send_to_char("\n\r{GSyntax{w: {WSLAY {c[{WTARGET NAME{c] <{WSLAY TYPE{c>{x\n\r", ch);
       send_to_char("{cSLAY Types Available to You{w:\n\r\n\r", ch); 

       for ( i=0; i < MAX_SLAY_TYPES-1; i++ ) 

        if ( (slay_table[i].owner == NULL)
        || (!str_prefix(slay_table[i].owner,ch->name)  
        && slay_table[i].title[0] != '\0') )
        {
          if(slay_table[i].title != NULL)
            {
           sprintf(buf, "%s\n\r", slay_table[i].title);
           send_to_char(buf, ch);
            }
          else
            {
           send_to_char("{CAVAILABLE SLOT{x\n\r",ch);
            }
        }

       send_to_char(
          "\n\r{cTyping the normal '{WSLAY {c<{WTARGET NAME{c>' also works.\n\r",ch);
       return;
     }


   if(!IS_IMMORTAL(ch))
    {
     send_to_char( "\n\r{RYou wish Fan Boy!{x\n\r", ch );
     return;
    }
  
  
     if(IS_IMP(ch))
      {
       if ( ( victim = get_char_world( ch, who ) ) == NULL )
         {
          send_to_char( "\n\r{RThey aren't here!{x\n\r", ch );
          return;
         }
      }
     else
     {
       if ( ( victim = get_char_room( ch, who ) ) == NULL )
         {
          send_to_char( "\n\r{RThey aren't here!{x\n\r", ch );
          return;
         }
   }
 
     if ( ch == victim )
     {
		 send_to_char("\n\r{WYou want to SLAY yourself?!{x\n\r", ch);
		 return;
     } 
     
     if ( !IS_NPC(victim) && victim->level > ch->level )
     {
        send_to_char( "\n\r{RYou failed!{x\n\r", ch );
        return;
     } 

     if ( type[0] == '\0' )
       {
          raw_kill(victim,ch);
          return;
       }

     for (i=0; i < MAX_SLAY_TYPES; i++)
     {
       if (
            !str_prefix(type, slay_table[i].title) &&
             ( slay_table[i].owner ==  NULL        ||
              !str_prefix(slay_table[i].owner, ch->name) )
          )
          {
             found=TRUE;
             sprintf(buf, "%s\n\r", slay_table[i].char_msg);
             act(buf, ch, NULL, victim, TO_CHAR );
             sprintf(buf, "%s\n\r", slay_table[i].vict_msg);
             act(buf, ch, NULL, victim, TO_VICT );
             sprintf(buf, "%s\n\r", slay_table[i].room_msg);
             act(buf, ch, NULL, victim, TO_NOTVICT );
             raw_kill(victim,ch);
             return;
          }
     }


    if (!found)
      send_to_char("\n\r{GSLAY Type not defined. Type '{WSLAY LIST{G' for Types available to you...{x\n\r", ch);
    return;
}


void  do_overview(CHAR_DATA *ch, char *argument)
{
    DESCRIPTOR_DATA *   d;
    CHAR_DATA *         wch;
    BUFFER *            output;
    char                buf[MAX_STRING_LENGTH];

    output=new_buf();

    add_buf(output,"\n\r{b[{c   Player {b- {cLvl     Class  Pkl Cln Pno Fgt Qst Lgd {D({WHitPts{c/{CMaxHPs{D)  {cRvnum  Bounty  {b]{x\n\r");
        
        for(d=descriptor_list;d;d=d->next)
        {
            if(d->character && can_see(ch, d->character))
            {
                wch = d->original ? d->original : d->character;
    
                 
    if (!IS_SET(wch->pact, PLR_REMORT))
      {   
       CLAN_DATA *pClan = get_clan_index(wch->clan);
    sprintf(buf, 
    "{D[{W%9s {b- {W%3d  %s  %-3s   %-3s  {C%-2d %-3s %-3s %-3s %-3s {D({W%6d{c/{C%6d{D)  {M%5d  {R%-7d {D]{x\n\r",
    wch->name,wch->level,
    IS_HARDCORE(wch) ? "{RHC{W" : "  ",
    class_table[wch->class].who_name,
    !IS_SET(wch->pact, PLR_PKILLER) ? " {RN ": " {GY ",
    (wch->clan > 0) ? pClan->vnum : 0,
    !IS_SET(wch->pact, PLR_PNOTE) ? " {RN ": " {GY ",
    wch->fighting ? " {GY " : " {RN ",
    !IS_SET(wch->pact, PLR_QUESTOR) ? " {RN ": " {GY ",
    IS_LEGEND(wch) ? " {GY " : " {RN ",
    wch->hit,wch->max_hit,wch->in_room ? wch->in_room->vnum : 0,wch->pcdata->bounty);
                add_buf(output, buf);
       }
      else
       {
       CLAN_DATA *pClan = get_clan_index(wch->clan);
    
    sprintf(buf, 
    "{D[{W%9s {b- {W%3d %s %-3s{c/{W%-3s %-3s  {C%-2d %-3s %-3s %-3s %-3s {D({W%6d{c/{C%6d{D)  {M%5d  {R%-7d {D]{x\n\r",
    wch->name,wch->level,
    IS_HARDCORE(wch) ? "{RHC{W" : "  ",
    class_table[wch->class].who_name,class_table[wch->pcdata->oldcl].who_name,
    !IS_SET(wch->pact, PLR_PKILLER) ? " {RN ": " {GY ",
    (wch->clan > 0) ? pClan->vnum : 0,
    !IS_SET(wch->pact, PLR_PNOTE) ? " {RN ": " {GY ",
    wch->fighting ? " {GY " : " {RN ",
    !IS_SET(wch->pact, PLR_QUESTOR) ? " {RN ": " {GY ",
    IS_LEGEND(wch) ? " {GY " : " {RN ",
    wch->hit,wch->max_hit,wch->in_room ? wch->in_room->vnum : 0,wch->pcdata->bounty);
                add_buf(output, buf);
       }
            }   
        }
    
    page_to_char(buf_string(output), ch);
    free_buf(output);


    if(IS_IMP(ch))
      {
    char eqbuf[MAX_STRING_LENGTH];
    CHAR_DATA *eqvic;
    BUFFER *equip;
    DESCRIPTOR_DATA *ech;
         
        equip = new_buf();

   send_to_char(
"\n\r{b[{C      NAME {b- {CITM  WGHT  HIT  DAM  SAVE  PIERC   BASH  SLASH  EXOTI  SNOOPED        {b]{x\n\r",ch);
    
        for (ech = descriptor_list; ech != NULL; ech = ech->next)
           {
            eqvic = ech->character;

                if((eqvic == NULL)
                || (IS_NPC(eqvic))
                || (eqvic->desc->connected != CON_PLAYING) )
                  continue;

    	    sprintf(eqbuf,
"{D[ {W%9.9s {D- {W%3d  %4d  %3d  %3d  %4d  %5d  %5d  %5d  %5d  {R%-14.14s {D]{x\n\r",
    	    	eqvic->name, eqvic->carry_number, eqvic->carry_weight, 
    	    	eqvic->hitroll, eqvic->damroll, eqvic->saving_throw,
    	    	eqvic->armor[AC_PIERCE], eqvic->armor[AC_BASH],
    	    	eqvic->armor[AC_SLASH],eqvic->armor[AC_EXOTIC],
                ( (eqvic->desc->snoop_by != NULL) ? eqvic->desc->snoop_by->character->name : "NOT SNOOPED" ) );
    	    add_buf(equip, eqbuf);
       	   }

        page_to_char(buf_string(equip),ch);
        free_buf(equip);
        return;
   } 
 
}



void do_sstat(CHAR_DATA *ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    bool found = FALSE;
    int gn, sn, level, mana, col = 0;
    bool was_printed[ MAX_SKILL ];
    bool pSpell = 0;
    CHAR_DATA *victim; 

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if ( arg1[0] == '\0' || arg2[0] == '\0')
    {
        send_to_char( "\n\r{GSyntax{w: {WSSTAT {c<{WCHAR NAME{c> <{WTYPE{c>{x\n\r", ch );
        send_to_char( " {cTypes{w: {CSPELLS SKILLS GROUPS{x\n\r", ch );
        return;
    }

  if (!str_cmp(arg2,"groups"))
    {
    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
        send_to_char( "\n\r{RThey aren't here.{x\n\r", ch );
        return;
    }

    if (IS_NPC(victim))
    {
        send_to_char( "\n\r{RUse this for GROUPs on players.{x\n\r", ch );
        return;
    }


    sprintf(buf,"\n\r{GGroupStat Target{w: {W%s{x\n\r",victim->name);
    send_to_char(buf,ch);

    send_to_char("\n\r{GGroups{w:{x\n\r",ch);

    col = 0;

    for (gn = 0; gn < MAX_GROUP; gn++)
    {
        if (group_table[gn].name == NULL)
        break;
        if (victim->pcdata->group_known[gn])
        {
            sprintf(buf,"{b[{C%-20s{b]",group_table[gn].name);
            send_to_char(buf,ch);
            if (++col % 3 == 0)
                send_to_char("\n\r",ch);
        }
    }
    if ( col % 3 != 0 )
    {
        send_to_char( "{x\n\r", ch );
    }
    return;
   }

  if (!str_cmp(arg2,"skills"))
    {
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

if ( victim->pcdata->oldcl == -1 )
  {
    sprintf(buf,"\n\r{GSkillStat Target{w: {W%s{x\n\r\n\r",victim->name);
    send_to_char(buf,ch);
  }
else
  {
    sprintf(buf,"\n\r{GSkillStat Target{w: {W%s   {GClass ReMorted From{w: {W%s{x\n\r\n\r",
victim->name,capitalize(class_table[victim->pcdata->oldcl].name));
    send_to_char(buf,ch);
  }
	  found = TRUE;
	  }


	if ( pSpell )
	  {
	  sprintf( buf, "{g[{W%3d{g]", level );
	  send_to_char( buf, ch );
	  pSpell = FALSE;
	  }

	if ( ( ++col % 4 == 0 && (col-1) % 3 != 0 && col < 7 )
	|| (col > 3 && (col-1) % 3 == 0) )
	  send_to_char( "     ", ch );
	if (victim->level < level)
	  sprintf(buf,"{c[{C%15s{c] {c[{R n/a{c]", skill_table[sn].name);
	else
	  {
	  sprintf(buf,"{g[{W%15s{g]{g[{W%3d{c%%{g]",skill_table[sn].name,
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
   }

  if (!str_cmp(arg2,"spells"))
    {
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
          || skill_table[sn].spell_fun == spell_null )
            continue;  
          }
        else
          {
          if ( victim->pcdata->learned[sn] <= 0
          ||   skill_table[sn].spell_fun == spell_null
          || ( skill_table[sn].skill_level[victim->class] != level
          &&   skill_table[sn].skill_level[victim->pcdata->oldcl] != level ) )
            continue;
         }

	if ( was_printed[sn] )
	  continue;
	was_printed[sn] = TRUE;
	if ( !found )
	  {

        if ( victim->pcdata->oldcl == -1 )
          {
    sprintf(buf,"\n\r{rSpellStat Target{w: {W%s{x\n\r\n\r",victim->name);
    send_to_char(buf,ch);
          }
        else
          {
    sprintf(buf,"\n\r{rSpellStat Target{w: {W%s   {rClass ReMorted From{w: {W%s{x\n\r\n\r",
victim->name,capitalize(class_table[victim->pcdata->oldcl].name));
    send_to_char(buf,ch);
          }

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
	  sprintf(buf,"{r[{W%-16.16s{r] [{R n/a {r]",skill_table[sn].name);
	else
	  {
	  mana = MANA_COST( ch, sn );

	  sprintf(buf,"{r[{W%-16.16s{r][{W%3d {RMN{r][{W%3d{R%%{r]",skill_table[sn].name, mana,
victim->pcdata->learned[sn] );
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
  }


 return;
}

void do_wpeace(CHAR_DATA *ch, char *argument )
{
     CHAR_DATA *rch;
  char buf[MAX_STRING_LENGTH]; 

     rch = get_char_world( ch, argument );


     for ( rch = char_list; rch; rch = rch->next )
      {
	if ( ch->desc == NULL || ch->desc->connected != CON_PLAYING )
	   continue;

	if ( rch->fighting )
	  {
           sprintf(buf,"\n\r{W%s {Ghas declared WorldPeace!{x\n\r",ch->name);
           do_gecho(ch,buf);
	   stop_fighting( rch, TRUE );
          }
      }

    send_to_char( "\n\r{RYou have declared World Peace.{x\n\r", ch );
    return;
}


void do_jail( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA       *victim;
    ROOM_INDEX_DATA *location;
    char             arg [ MAX_INPUT_LENGTH ];
    char buf [MAX_STRING_LENGTH];

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "\n\r{cJail whom?{x\n\r", ch );
        return;
    }

    if ( !( victim = get_char_world( ch, arg ) ) )
    {
        send_to_char( "\n\r{cThey aren't here.{x\n\r", ch );
        return;
    }

    if(IS_NPC(victim))
     {
      send_to_char( "\n\r{RMOBs have enough problems without you JAILing them!{x\n\r", ch );
      return;
     }

    if ( victim->level >= ch->level )
    {
        send_to_char( "\n\r{RYou failed.{x\n\r", ch );
        return;
    }

    if ( IS_SET( victim->pact, PLR_JAIL ) )
    {
        REMOVE_BIT( victim->pact, PLR_JAIL );
  
           if (victim->played <= 30)
            {
             victim->played = 0;
            }
           else
            {
             victim->played -= (victim->played / 2);
            }

        location = get_room_index( ROOM_VNUM_ALTAR );
        act( "\n\r{R$n disappears in a mushroom cloud.\n\r{x",
              victim, NULL, NULL, TO_ROOM );
        char_from_room( victim );
        char_to_room( victim, location );
        act( "\n\r{c$n arrives from a puff of smoke.{x\n\r",
              victim, NULL, NULL, TO_ROOM );
        do_look( victim, "auto" );

        send_to_char( "\n\r{WYour victim is free now.{x\n\r",ch);
        send_to_char( "\n\r{WYou can play again.{x\n\r", victim );
    }
    else
    {
        SET_BIT(    victim->pact, PLR_JAIL );

         if (victim->exp <= 4999)
           {
            victim->exp = 0;
           }
          else
           {
            victim->exp -= 5000;
           }

        location = get_room_index( ROOM_VNUM_JAIL );
        if ( victim->fighting )
           stop_fighting( victim, TRUE );
        act( "\n\r{R$n disappears in a mushroom cloud.{x\n\r",
              victim, NULL, NULL, TO_ROOM );
        char_from_room( victim );
        char_to_room( victim, location );
        act( "\n\r{c$n arrives from a puff of smoke.{x\n\r", 
              victim, NULL, NULL, TO_ROOM );
        do_look( victim, "auto" );
 
        send_to_char( "\n\r{CYour victim goes to jail!{x\n\r", ch );
        send_to_char( "\n\r{RYou can't do ANYthing!{x\n\r", victim );
sprintf(buf, "\n\r{GThe jail keeper shouts '{W%s has came to visit me in jail!{G'{x\n\r",victim->name  );
do_gecho( ch, buf );
    }

    save_char_obj( victim );
    return;
}

void do_stat ( CHAR_DATA *ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   char *string;
   OBJ_DATA *obj;
   ROOM_INDEX_DATA *location;
   CHAR_DATA *victim;
   
   string = one_argument(argument, arg);
   if ( arg[0] == '\0')
   {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  stat <name>\n\r",ch);
	send_to_char("  stat obj <name>\n\r",ch);
	send_to_char("  stat mob <name>\n\r",ch);
 	send_to_char("  stat room <number>\n\r",ch);
	return;
   }

   if (!str_cmp(arg,"room"))
   {
	do_function(ch, &do_rstat, string);
	return;
   }
  
   if (!str_cmp(arg,"obj"))
   {
	do_function(ch, &do_ostat, string);
	return;
   }

   if(!str_cmp(arg,"char")  || !str_cmp(arg,"mob"))
   {
	do_function(ch, &do_mstat, string);
	return;
   }
   
   /* do it the old way */


   
  obj = get_obj_world(ch,argument);
  if (obj != NULL)
  {
   	do_function(ch, &do_ostat, argument);
   	return;
  }

  victim = get_char_world(ch,argument);
  if (victim != NULL)
  {
    do_function(ch, &do_mstat, argument);
    return;
  }

  location = find_location(ch,argument);
  if (location != NULL)
  {
    do_function(ch, &do_rstat, argument);
    return;
  }

  send_to_char("Nothing by that name found anywhere.\n\r",ch);
}

void do_ostat( CHAR_DATA *ch, char *argument )
{ 
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    AFFECT_DATA *paf;
    OBJ_DATA *obj;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "\n\r{GSyntax{w: {WOSTAT {c<{WOBJ NAME{c>{x\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_world( ch, argument ) ) == NULL )
    {
	send_to_char( "\n\r{RNothing like that in hell, earth, or heaven.{x\n\r", ch );
	return;
    }

  if(!IS_IMP(ch))
    {
     if(IS_SET(obj->pIndexData->area->area_flags, AREA_IMP))
       {
        send_to_char(
        "\n\r{RThat OBJ is NOT from your AREA{r!{x\n\r",ch);
        return;
       }
        
     if(ch->level < ASSTIMP)
       {
        if(IS_SET(obj->pIndexData->area->area_flags, AREA_NOIMM))
          {
           if(!IS_SET(ch->comm, COMM_HELPER))
             {
              send_to_char(
              "\n\r{MThat OBJ is NOT from your AREA{m!{x\n\r",ch);
              return;
             }
          }  
           
        if(!IS_BUILDER(ch,obj->pIndexData->area))
          {
           if(!IS_SET(ch->comm, COMM_HELPER))
             {
              send_to_char(
              "\n\r{GThat OBJ is NOT from your AREA{g!{x\n\r",ch);
              return;
             }
          }
       }
    }

  if((IS_SET(obj->extra_flags, ITEM_CLAN_EQ))
  || (obj->enchanted))
   {
    send_to_char("\n\r",ch);

  if(IS_SET(obj->extra_flags, ITEM_CLAN_EQ))
    {
    sprintf( buf, "{g[{GCLAN Equipment{g] {x");
    send_to_char( buf, ch );
    }

  if(obj->enchanted)
   {
    sprintf( buf, "{B[{CItem Enchanted{B]{x");
    send_to_char( buf, ch );
   }
  }
 
    sprintf( buf, "\n\r{cName{D({Ws{D){w: {W%s{x\n\r",obj->name);
    send_to_char( buf, ch );

    sprintf( buf, "  {cLevel{w: {W%-3d       {cVnum{w: {W%-5d           {cType{w: {W%s{x\n\r",
obj->level,obj->pIndexData->vnum,item_name(obj->item_type));
    send_to_char( buf, ch );


    sprintf( buf, "  {cTimer{w: {W%-4d    {cWeight{w: {W%4d{c/{W%4d{c/{W%4d  {cCost{w: {W%d{x\n\r",
obj->timer,obj->weight, get_obj_weight( obj ),get_true_weight(obj), obj->cost);
    send_to_char( buf, ch );

sprintf( buf, " {cNumber{w: {W%2d{c/{W%2d  {cIn Room{w: {W%-5d     {cCarried by{w: {W%s{x\n\r",
1,get_obj_number(obj),
obj->in_room    == NULL    ?        0 : obj->in_room->vnum,
obj->carried_by == NULL    ? "{D({WNONE{D){x" : can_see(ch,obj->carried_by) ? obj->carried_by->name
  : "{D({WSOMEONE{D){x" );
send_to_char( buf, ch );

    sprintf( buf,
	"{c In Object{w: {W%s{x\n\r",
	obj->in_obj     == NULL    ? "{D({WNONE{D){x" :
strip_color(obj->in_obj->short_descr));
    send_to_char( buf, ch );

    sprintf( buf, 
        "{c  Material{w: {W%s{x\n\r", flag_string( material_flags,obj->material_type ));
    send_to_char( buf, ch );
 
    sprintf( buf, "    {cValues{w: {W%3d {c- {W%3d {c- {W%3d {c- {W%3d {c- {W%3d{x\n\r",
obj->value[0], obj->value[1], obj->value[2], obj->value[3],obj->value[4]);
    send_to_char( buf, ch );

    sprintf( buf, "{c Wear Bits{w: {W%s    {cWear Location{w: {W%d\n\r{cExtra Bits{w: {W%s{x\n\r\n\r",
wear_bit_name(obj->wear_flags),obj->wear_loc,extra_bit_name(obj->extra_flags ) );
    send_to_char( buf, ch );
    
    /* now give out vital statistics as per identify */
    
    switch ( obj->item_type )
    {
    	case ITEM_SCROLL: 
    	case ITEM_POTION:
    	case ITEM_PILL:
	    sprintf( buf, "{cLevel {W%d {cSpells of{w:{x", obj->value[0] );
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
	    	send_to_char( " {c'{W", ch );
	    	send_to_char( skill_table[obj->value[3]].name, ch );
	    	send_to_char( "{c'", ch );
	    }

	    if (obj->value[4] >= 0 && obj->value[4] < MAX_SKILL)
	    {
	    	send_to_char( " {c'{W", ch );
		send_to_char(skill_table[obj->value[4]].name,ch);
	    	send_to_char( "{c'", ch );
	    }

	    send_to_char( "{x\n\r\n\r", ch );
	break;

    	case ITEM_WAND: 
    	case ITEM_STAFF: 
	    sprintf( buf, "{cHas {W%d{D({W%d{D) {cCharges of Level {W%d{x",
	    	obj->value[1], obj->value[2], obj->value[0] );
	    send_to_char( buf, ch );
      
	    if ( obj->value[3] >= 0 && obj->value[3] < MAX_SKILL )
	    {
	    	send_to_char( " {c'{W", ch );
	    	send_to_char( skill_table[obj->value[3]].name, ch );
	    	send_to_char( "{c'", ch );
	    }

	    send_to_char( "{x\n\r\n\r", ch );
	break;

	case ITEM_DRINK_CON:
	    sprintf(buf,"{cIt Holds {W%s{w-{cColored {W%s{c.{x\n\r\n\r",
liq_table[obj->value[2]].liq_color,
liq_table[obj->value[2]].liq_name);
	    send_to_char(buf,ch);
	    break;
		
      
    	case ITEM_WEAPON:
 	    send_to_char("{cWeapon Type is{w: ",ch);
	    switch (obj->value[0])
	    {
	    	case(WEAPON_SWORD): 
		    send_to_char("{WSWORD{x\n\r",ch);
		    break;	
	    	case(WEAPON_DAGGER): 
		    send_to_char("{WDAGGER{x\n\r",ch);
		    break;
	    	case(WEAPON_SPEAR):
		    send_to_char("{WSPEAR {c/ {WSTAFF{x\n\r",ch);
		    break;
	    	case(WEAPON_MACE): 
		    send_to_char("{WMACE {c/ {WCLUB{x\n\r",ch);	
		    break;
	   	case(WEAPON_AXE): 
		    send_to_char("{WAXE{x\n\r",ch);	
		    break;
	    	case(WEAPON_FLAIL): 
		    send_to_char("{WFLAIL{x\n\r",ch);
		    break;
	    	case(WEAPON_WHIP): 
		    send_to_char("{WWHIP{x\n\r",ch);
		    break;
	    	case(WEAPON_POLEARM): 
		    send_to_char("{WPOLEARM{x\n\r",ch);
		    break;
	  	case(WEAPON_EXOTIC): 
		    send_to_char("{WEXOTIC{x\n\r",ch);
		    break;
  	        default: 
		    send_to_char("{R!!UNKNOWN{w-{RNeeds fixing!!{x\n\r",ch);
		    break;
 	    }
	    sprintf(buf,"{cDamage Noun is{w: {W%s{x\n\r",
		(obj->value[3] > 0 && obj->value[3] < MAX_DAMAGE_MESSAGE) ?
attack_table[obj->value[3]].noun : "{R!!UNDEFINED!!{x");
	    send_to_char(buf,ch);
	    
	    if (obj->value[4])  /* weapon flags */
	    {
	        sprintf(buf,"{cWeapons Flags{w: {W%s{x\n\r",weapon_bit_name(obj->value[4]));
	        send_to_char(buf,ch);
            }

	    if (obj->pIndexData->new_format)
	    	sprintf(buf,"{cDamage is{w: {W%d{cd{W%d {D({cAVERAGE{w: {W%d{D){x\n\r\n\r",
		    obj->value[1],obj->value[2],
		    (1 + obj->value[2]) * obj->value[1] / 2);
	    else
	    	sprintf( buf, "{cDamage is {W%d {cto {W%d {D({cAVERAGE{w: {W%d{D){x\n\r\n\r",
	    	    obj->value[1], obj->value[2],
	    	    ( obj->value[1] + obj->value[2] ) / 2 );
	    send_to_char( buf, ch );
	break;

    	case ITEM_ARMOR:
	    sprintf( buf, 
	    "{cArmor Class is{w: {W%d {CPierce  {W%d {CBash  {W%d {CSlash  {W%d {CMagic{x\n\r\n\r",
	        obj->value[0], obj->value[1], obj->value[2], obj->value[3] );
	    send_to_char( buf, ch );
	break;

        case ITEM_CONTAINER:
            sprintf(buf,"{cCapacity{w: {W%d{c#  Maximum Weight{w: {W%d{c#  Flags{w: {W%s{x\n\r",
                obj->value[0], obj->value[3], cont_bit_name(obj->value[1]));
            send_to_char(buf,ch);
            if (obj->value[4] != 100)
            {
                sprintf(buf,"{cWeight Multiplier{w: {W%d{c%%{x\n\r\n\r",
		    obj->value[4]);
                send_to_char(buf,ch);
            }
        break;
    }



    sprintf( buf, "{cS DESC{w: {W%s",
	strip_color(obj->short_descr));
    send_to_char( buf, ch );

    sprintf( buf, "\n\r{cL DESC{w: {W%s{x\n\r",strip_color(obj->description) );
    send_to_char( buf, ch );

    if ( obj->extra_descr != NULL || obj->pIndexData->extra_descr != NULL )
    {
	EXTRA_DESCR_DATA *ed;

	send_to_char( "{cExtra Description Keywords{w: {c'{x", ch );

	for ( ed = obj->extra_descr; ed != NULL; ed = ed->next )
	{
	    send_to_char( ed->keyword, ch );
	    if ( ed->next != NULL )
	    	send_to_char( " ", ch );
	}

	for ( ed = obj->pIndexData->extra_descr; ed != NULL; ed = ed->next )
	{
	    send_to_char( ed->keyword, ch );
	    if ( ed->next != NULL )
		send_to_char( " ", ch );
	}

	send_to_char( "{c'{x\n\r", ch );
    }

    for ( paf = obj->affected; paf != NULL; paf = paf->next )
    {
	sprintf( buf, "{cAffects{w: {W%-9s {cby {W%-4d {w- {cLevel{w: {W%-4d{x",
affect_loc_name( paf->location), paf->modifier,paf->level );
	send_to_char(buf,ch);
	if ( paf->duration > -1)
	    sprintf(buf,"{c, {W%-4d {cHours{x\n\r",paf->duration);
	else
	    sprintf(buf,"{x\n\r");
	send_to_char( buf, ch );
	if (paf->bitvector)
	{
	    switch(paf->where)
	    {
		case TO_AFFECTS:
		    sprintf(buf,"{cAdds{w: {W%s {cAffect{x\n\r",
			affect_bit_name(paf->bitvector));
		    break;
		case TO_AFFECTS2:
		    sprintf(buf,"{cAdds{w: {W%s {cAffect2{x\n\r",
			affect2_bit_name(paf->bitvector));
		    break;
                case TO_WEAPON:
                    sprintf(buf,"{cAdds{w: {W%s {cWeapon Flags{x\n\r",
                        weapon_bit_name(paf->bitvector));
		    break;
		case TO_OBJECT:
		    sprintf(buf,"{cAdds{w: {W%s {cObject Flags{x\n\r",
			extra_bit_name(paf->bitvector));
		    break;
		case TO_IMMUNE:
		    sprintf(buf,"{cAdds Immunity to{w: {W%s{x\n\r",
			imm_bit_name(paf->bitvector));
		    break;
		case TO_RESIST:
		    sprintf(buf,"{cAdds Resistance to{w: {W%s{x\n\r",
			imm_bit_name(paf->bitvector));
		    break;
		case TO_VULN:
		    sprintf(buf,"{cAdds Vulnerability to{w: {W%s{x\n\r",
			imm_bit_name(paf->bitvector));
		    break;
		default:
		    sprintf(buf,"{R!!UNKNOWN BIT{w-{W%d{w: {W%d{R!!{x\n\r",
			paf->where,paf->bitvector);
		    break;
	    }
	    send_to_char(buf,ch);
	}
    }

    if (!obj->enchanted)
    for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
    {
	sprintf( buf, "{cAffects{w: {W%-9s {cby {W%-4d {w- {cLevel{w: {W%-4d{x\n\r",
affect_loc_name( paf->location), paf->modifier,paf->level );
	send_to_char( buf, ch );
        if (paf->bitvector)
        {
            switch(paf->where)
            {
		case TO_AFFECTS:
		    sprintf(buf,"{cAdds{w: {W%s {cAffect{x\n\r",
			affect_bit_name(paf->bitvector));
		    break;
		case TO_AFFECTS2:
		    sprintf(buf,"{cAdds{w: {W%s {cAffect2{x\n\r",
			affect2_bit_name(paf->bitvector));
		    break;
		case TO_OBJECT:
		    sprintf(buf,"{cAdds{w: {W%s {cObject Flag{x\n\r",
			extra_bit_name(paf->bitvector));
		    break;
		case TO_IMMUNE:
		    sprintf(buf,"{cAdds Immunity to{w: {W%s{x\n\r",
			imm_bit_name(paf->bitvector));
		    break;
		case TO_RESIST:
		    sprintf(buf,"{cAdds Resistance to{w: {W%s{x\n\r",
			imm_bit_name(paf->bitvector));
		    break;
		case TO_VULN:
		    sprintf(buf,"{cAdds Vulnerability to{w: {W%s{x\n\r",
			imm_bit_name(paf->bitvector));
		    break;
		default:
		    sprintf(buf,"{R!!UNKNOWN BIT{w-{W%d{w: {W%d{R!!{x\n\r",
			paf->where,paf->bitvector);
		    break;
            }
            send_to_char(buf,ch);
        }
    }
    return;
}


void do_mstat( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    AFFECT_DATA *paf;
    CHAR_DATA *victim;

    argument = one_argument( argument, arg1 );
    	       one_argument( argument, arg2 );

    if ( arg1[0] == '\0' 
    &&   arg2[0] == '\0' )
    {
	send_to_char( "\n\r    {GSyntax{w: {WMSTAT {c<{WTARGET NAME{c>{x\n\r", ch );
	send_to_char( "{c More Info{w: {WMSTAT {c<{WTARGET NAME{c> {WMORE{x\n\r",ch);
	return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "\n\r{RThey aren't here!{x\n\r", ch );
	return;
    }



    if ((IS_IMP(victim))
    && (!IS_IMP(ch)))
    {
	send_to_char( "\n\r{RNo descriptor data found.{x\n\r", ch );
	return;
    }


if (!IS_NPC(victim))
{
 if(arg2[0] == '\0')
 {
  if (IS_IMP(ch))
    {
if(victim->desc->snoop_by != NULL)
  {
/*
sprintf( buf, "\n\r{cMSTAT Target{w: {W%s  {RSecurity{w: {W%d  {GSNOOPed By{w: {W%-10.10s\n\r",
victim->name,victim->pcdata->security,capitalize(victim->desc->snoop_by->character->name));
send_to_char( buf, ch );
*/

sprintf( buf, "\n\r{cMSTAT Target{w: {W%s  {RSecurity{w: {W%d\n\r",
victim->name,victim->pcdata->security);
send_to_char( buf, ch );
  }
else
  {
sprintf( buf, "\n\r{cMSTAT Target{w: {W%s  {RSecurity{w: {W%d\n\r",
victim->name,victim->pcdata->security);
send_to_char( buf, ch );
  }
    }
  else
    {
sprintf( buf, "\n\r{mMSTAT Target{w: {W%s  ",
victim->name);
send_to_char( buf, ch );
    }

if(victim->level == IMPLEMENTOR) 
  { 
   sprintf (buf,"{BImmortal Title{w: {WIMPLEMENTOR{x\n\r" ); 
   send_to_char( buf, ch ); 
  }
else 
if (victim->level == ASSTIMP)
  { 
   sprintf (buf,"{BImmortal Title{w: {WASSISTANT IMPLEMENTOR{x\n\r" );          
   send_to_char( buf, ch );
  } 
else 
if (victim->level >= GGOD && victim->level <= (MAX_LEVEL - 370))
  { 
   sprintf (buf,"{BImmortal Title{w: {WGREATER GOD{x\n\r" );          
   send_to_char( buf, ch );
  } 
else 
if (victim->level >= LGOD && victim->level < GGOD)
  { 
   sprintf (buf,"{BImmortal Title{w: {WLESSER GOD{x\n\r" );          
   send_to_char( buf, ch );
  } 
else 
if (victim->level >= DGOD && victim->level < LGOD)
  { 
   sprintf (buf,"{BImmortal Title{w: {WDEMI-GOD{x\n\r" );          
   send_to_char( buf, ch );
  } 
else 
if (victim->level >= VETIMM && victim->level < DGOD)
  { 
   sprintf (buf,"{BImmortal Title{w: {WVETERAN IMMORTAL{x\n\r" );          
   send_to_char( buf, ch );
  } 
else 
if (victim->level == NEWIMM)
  { 
   sprintf (buf,"{BImmortal Title{w: {WNEWBIE IMMORTAL{x\n\r" );          
   send_to_char( buf, ch );
  } 
else 
if (victim->level == BUILDER)
  { 
   sprintf (buf,"{BImmortal Title{w: {WBUILDER{x\n\r" );          
   send_to_char( buf, ch );
  } 
else 
if (victim->level == IMMORTAL || victim->level == VISIMM)
  { 
   sprintf (buf,"{BImmortal Title{w: {WVISITING IMMORTAL{x\n\r" );          
   send_to_char( buf, ch );
  } 
else
if(IS_LEGEND(victim))
{
   sprintf (buf,"{GMortal Status{w: {Y%s Legend{x\n\r", legend_table[victim->pcdata->legend].name );          
   send_to_char( buf, ch );
}
else
if(IS_SET(victim->pact, PLR_REMORT)) 
  { 
   sprintf (buf,"{GMortal Status{w: {WREMORT-HERO{x\n\r" );          
   send_to_char( buf, ch );
  } 
else 
if (victim->level == 100)
  { 
   sprintf (buf,"{GMortal Status{w: {WHERO{x\n\r" );          
   send_to_char( buf, ch );
  } 
else 
if (victim->level >= 1 && victim->level <= 99 )
  { 
   sprintf (buf,"{GMortal Status{w: {WNORMAL PC{x\n\r" );          
   send_to_char( buf, ch );
  } 
  
  if(IS_HARDCORE(victim))
    send_to_char("{RHARDCORE PLAYER{x\n\r", ch);

sprintf( buf,"\n\r{cLevel{w: {C%-12d {cSTR{w: {W%2d{c({C%2d{c)          {cAge{w: {C%-8d{x\n\r",
victim->level,get_curr_stat(victim,STAT_STR),victim->perm_stat[STAT_STR],get_age(victim));
send_to_char( buf, ch );

sprintf( buf," {cRace{w: {C%-12s {cINT{w: {W%2d{c({C%2d{c)        {cHours{w: {C%-8d{x\n\r",
race_table[victim->race].name,get_curr_stat(victim,STAT_INT),victim->perm_stat[STAT_INT], 
((victim->played + (int) (current_time - victim->logon) ) /3600));

send_to_char( buf, ch );

sprintf( buf,"{cClass{w: {C%-12s {cWIS{w: {W%2d{c({C%2d{c)     {cLast Lvl{w: {C%d{x\n\r", 
IS_NPC(victim) ? "mobile" :
class_table[victim->class].name,get_curr_stat(victim,STAT_WIS),
victim->perm_stat[STAT_WIS],victim->pcdata->last_level);
send_to_char( buf, ch );

sprintf( buf,"{cAlign{w: {C%-12d {cDEX{w: {W%2d{c({C%2d{c)       {cTrains{w: {C%-3d{x\n\r",
victim->alignment,get_curr_stat(victim,STAT_DEX),victim->perm_stat[STAT_DEX],
victim->train);
send_to_char( buf, ch );

sprintf( buf,"  {cSex{w: {C%-12s {cCON{w: {W%2d{c({C%2d{c)        {cPracs{w: {C%-5d{x\n\r",
victim->sex == 0 ? "SEXLESS" : victim->sex == 1 ? "MALE" : "FEMALE",
get_curr_stat(victim,STAT_CON),
victim->perm_stat[STAT_CON],victim->practice);
send_to_char( buf, ch );

if (victim->clan >0)
  {
    CLAN_DATA *pClan = get_clan_index(victim->clan);
 
    sprintf( buf, " {cClan{w: {C%-10s{x",capitalize(pClan->name));
    send_to_char(buf,ch);
  }
 else
  {
    send_to_char(" {cClan{w: {D({WNONE{D)    {x",ch);
  }

if (victim->pcdata->spouse > 0)
  {
    sprintf( buf,"{cSpouse{w: {G%-10.10s {x",capitalize(victim->pcdata->spouse));
    send_to_char( buf, ch );
  }
 else
  {
    send_to_char("{cSpouse{w: {D({WNONE{D)     {x",ch);
  }

if (IS_SET(victim->pact, PLR_REMORT))
  {
    sprintf( buf,"{cRemorted{w: {C%-11s{x",
capitalize(class_table[victim->pcdata->oldcl].name));
    send_to_char( buf, ch );
  }
 else
  {
    send_to_char("  {cRemort{w: {D( {WNO {D){x",ch);
  }

send_to_char("\n\r\n\r",ch);

sprintf(buf, "{cHitPt{w: {W%-5d{c({C%5d{c)  {cExperience{w: {C%-8d {cThirst{w: {C%d{x\n\r",
victim->hit,victim->max_hit,victim->exp,
victim->pcdata->condition[COND_THIRST]);
send_to_char( buf, ch );

sprintf(buf," {cMana{w: {W%-5d{c({C%5d{c)  {RExp to Lvl{w: {C%-8d {cHunger{w: {C%d{x\n\r",
victim->mana, victim->max_mana,
((victim->level + 1) * exp_per_level(victim,victim->pcdata->points) - victim->exp),
victim->pcdata->condition[COND_HUNGER]);
send_to_char( buf, ch );





sprintf(buf," {cMove{w: {W%-5d{c({C%5d{c)  {cQuest Pnts{w: {C%-8d  {cDrunk{w: {C%d{x\n\r",
victim->move,victim->max_move,victim->questpoints,
victim->pcdata->condition[COND_DRUNK]);
send_to_char( buf, ch );

sprintf(buf," {cIdle{w: {C%-5d         {cQuest Time{w: {C%-9d  {cFull{w: {C%d{x\n\r",
victim->timer,victim->nextquest,
victim->pcdata->condition[COND_FULL]);
send_to_char( buf, ch );

sprintf(buf,"\n\r{cPierce{w: {C%-6d    {CHIT{croll{w: {C%-5d      {cRoom{w: {C%d{x\n\r",
GET_AC(victim,AC_PIERCE),GET_HITROLL( victim ),
victim->in_room == NULL ? 0 : victim->in_room->vnum );
send_to_char( buf, ch );

sprintf(buf,"  {cBash{w: {C%-6d    {CDAM{croll{w: {C%-5d  {cPosition{w: {C%s{x\n\r",
GET_AC(victim,AC_BASH),GET_DAMROLL( victim ),
position_table[victim->position].name);
send_to_char( buf, ch );

sprintf(buf," {cSlash{w: {C%-6d      {cSaves{w: {C%-5d  {cFighting{w: {R%s{x\n\r",
GET_AC(victim,AC_SLASH),
victim->saving_throw,
victim->fighting ? victim->fighting->name : "{D({WNONE{D){x");
send_to_char( buf, ch );

sprintf(buf," {cMagic{w: {C%-6d       {cWimp{w: {C%-6d  {cPKTimer{w: {C%d{x\n\r\n\r",
GET_AC(victim,AC_EXOTIC),victim->wimpy,
victim->pk_timer);
send_to_char( buf, ch );

if (!IS_IMMORTAL(victim))
  {
     sprintf( buf,
	"{cGold{w: {Y%ld  {cSilver{w: {W%ld  {cBank Balance{w: {C%d{x\n\r",
victim->gold,victim->silver,victim->pcdata->balance );
	 send_to_char( buf, ch );

sprintf( buf,"{cInventory{w: {C%2.2d {cof {C%2.2d  {cCarrying{w: {C%ld {cof {C%d {c#{x\n\r", 
victim->carry_number, can_carry_n(victim),
get_carry_weight(victim), can_carry_w(victim));
         send_to_char( buf, ch );
  }

    sprintf( buf, "{cMaster{w: {C%s  {cLeader{w: {C%s  {cPet{w: {C%s{x\n\r",
	victim->master      ? victim->master->name   : "{D({WNONE{D)",
	victim->leader      ? victim->leader->name   : "{D({WNONE{D)",
	victim->pet 	    ? victim->pet->name	     : "{D({WNONE{D)");
    send_to_char( buf, ch );

    if ( victim->pcdata->member )
      {
            send_to_char ( "\n\r", ch);

      sprintf( buf, 
"{cPKills{w: {CAbove their lvl{w: {G%d  {c/ {CBelow their lvl{w: {G%d{x\n\r",
                    victim->pcdata->member->pks_up, victim->pcdata->member->pks_dwn);
      send_to_char( buf, ch );
sprintf( buf, 
"{cPDeths{w: {CAbove their lvl{w: {R%d  {c/ {CBelow their lvl{w: {R%d{x\n\r",
                    victim->pcdata->member->pkd_up, victim->pcdata->member->pkd_dwn);
      send_to_char( buf, ch );
      }

send_to_char("\n\r",ch);



    if (victim->pact)
    {
        sprintf(buf, "   {cACT{w: {C%s{x\n\r",pact_bit_name(victim->pact));
        send_to_char(buf,ch);
    }
    else
    {
	send_to_char("   {cACT{w: {D({WNONE{D){x\n\r",ch);
    }


    if (victim->comm)
    {
        sprintf(buf,"  {cCOMM{w: {C%s{x\n\r",comm_bit_name(victim->comm));
        send_to_char(buf,ch);
    }
    else
    {
	send_to_char("  {cCOMM{w: {D({WNONE{D){x\n\r",ch);
    }
    

    if (victim->imm_flags)
    {
	sprintf(buf, "{cIMMUNE{w: {C%s{x\n\r",imm_bit_name(victim->imm_flags));
	send_to_char(buf,ch);
    }
    else
    {
	send_to_char("{cIMMUNE{w: {D({WNONE{D){x\n\r",ch);
    }
 
    if (victim->res_flags)
    {
	sprintf(buf, "{cRESIST{w: {C%s{x\n\r", imm_bit_name(victim->res_flags));
	send_to_char(buf,ch);
    }
   else
    {
	send_to_char("{cRESIST{w: {D({WNONE{D){x\n\r", ch);
    }

    if (victim->vuln_flags)
    {
	sprintf(buf, "  {cVULN{w: {C%s{x\n\r", imm_bit_name(victim->vuln_flags));
	send_to_char(buf,ch);
    }
   else
    {
	send_to_char("  {cVULN{w: {D({WNONE{D){x\n\r",ch);
    }

    if (victim->affected_by)
    {
	sprintf(buf, "  {cAFFT{w: {C%s{x\n\r", 
	    affect_bit_name(victim->affected_by));
	send_to_char(buf,ch);
    }
   else
    {
	send_to_char("  {cAFFT{w: {D({WNONE{D){x\n\r",ch);
    }

    if (victim->affected2_by)
    {
	sprintf(buf, " {cAFFT2{w: {C%s{x\n\r\n\r", 
	    affect2_bit_name(victim->affected2_by));
	send_to_char(buf,ch);
    }
  else
    {
	send_to_char(" {cAFFT2{w: {D({WNONE{D){x\n\r\n\r",ch);
    }
}
else
   if (!str_cmp(arg2,"more"))
     {
sprintf( buf, "\n\r{mMSTAT Target{w: {W%s   {WPlayer Info Page 2{x\n\r",victim->name);
send_to_char( buf, ch );

sprintf( buf, "\n\r{cCreation Points{w: {C%-3d    {cBank Balance{w: {Y%d{x\n\r",
victim->pcdata->points,victim->pcdata->balance);
send_to_char( buf, ch );

sprintf( buf, "       {cTrue Sex{w: {C%-6s         {cSize{w: {C%s{x",
victim->pcdata->true_sex == 0 ? "SEXLESS":victim->pcdata->true_sex == 1 ? "MALE":"FEMALE",
capitalize(size_table[victim->size].name));
send_to_char( buf, ch );

if (IS_IMMORTAL(victim))
  {
   send_to_char("{x\n\r",ch);

   if (victim->invis_level)
     {
     sprintf( buf, "   {cWizInvis Lvl{w: {C%-3d ",victim->invis_level);
     send_to_char( buf, ch );
     }

   if (victim->incog_level)
     {
     sprintf( buf, "      {cIncog Lvl{w: {C%-3d",victim->incog_level);
     send_to_char( buf, ch );
     }

   send_to_char("{x\n\r",ch);
  }
else
   send_to_char("{x\n\r",ch);

if (IS_SET(victim->pact, PLR_QUESTOR))
  {
   sprintf( buf, "\n\r{cTime Remaining on Current Quest{w: {C%d{x\n\r",victim->countdown);
   send_to_char( buf, ch );
  }

sprintf( buf, "\n\r{cPrompt{w: {C%s{x\n\r",victim->prompt);
send_to_char( buf, ch );
if (IS_SET(victim->comm, COMM_SNOOP_PROOF))
  send_to_char("{cTarget is SNOOP PROOF{x\n\r",ch);

sprintf( buf, "\n\r{cForm{w: {C%s{x\n\r",form_bit_name(victim->form));
send_to_char( buf, ch );

sprintf( buf, "{cParts{w: {C%s{x\n\r",part_bit_name(victim->parts));
send_to_char( buf, ch );

send_to_char("\n\r{c/----------------------------------------\\{x\n\r",ch);
send_to_char("{W*         {CPENALTIES{c\\{CPUNISHMENTS          {W*{x\n\r\n\r",ch);

if (IS_SET(victim->comm, COMM_NOCHANNELS))
  {send_to_char("  {GNOCHANNEL Set{x ",ch);}
if(IS_SET(victim->pact,PLR_FREEZE))
  {send_to_char("  {GFREEZE Set{x ",ch);}
if (IS_SET(victim->comm, COMM_NOTELL))
  {send_to_char("  {GNOTELL Set{x ",ch);}

send_to_char("{x\n\r",ch);

if (IS_SET(victim->comm, COMM_NOTITLE))
  {send_to_char("  {GNOTITLE Set{x ",ch);}
if (IS_SET(victim->comm, COMM_NOEMOTE))
  {send_to_char("  {GNOEMOTE Set{x ",ch);}
if (IS_SET(victim->comm, COMM_NOPNOTE))
  {send_to_char("  {GNOPNOTE Set{x ",ch);}

send_to_char("{x\n\r",ch);

if (IS_SET(victim->pact, PLR_JAIL))
  {send_to_char("{G  IN JAIL{x ",ch);}
if (IS_SET(victim->comm, COMM_DPITKILL))
  {send_to_char("{G  DPITKILL Set{x ",ch);}

send_to_char("{x\n\r",ch);

send_to_char("\n\r{W*                                        {W*{x",ch);
send_to_char("\n\r{c\\----------------------------------------/{x\n\r",ch);
return;
   }
 else
  if ( arg2[0] != '\0'
  || (str_cmp(arg2,"more")))
  {
   send_to_char( "\n\r    {GSyntax{w: {WMSTAT {c<{WTARGET NAME{c>{x\n\r", ch );
   send_to_char( "{c More Info{w: {WMSTAT {c<{WTARGET NAME{c> {WMORE{x\n\r",ch);
   return;
  }

}
else  /* MOB'S MSTAT */
 {
sprintf( buf, "\n\r{mMSTAT Target{w: {W%s{x\n\r",
victim->name);
send_to_char( buf, ch );

sprintf( buf, "    {RMOB VNUM{w: {r({W%5d{r){x\n\r",
victim->pIndexData->vnum);
send_to_char( buf, ch );

sprintf( buf,"{cLevel{w: {C%-12d {cSTR{w: {W%2d{c({C%2d{c)  {cHitPt{w: {W%-5d{c({C%5d{c){x\n\r",
victim->level,get_curr_stat(victim,STAT_STR),victim->perm_stat[STAT_STR],
victim->hit,victim->max_hit);
send_to_char( buf, ch );

sprintf( buf," {cRace{w: {C%-12s {cINT{w: {W%2d{c({C%2d{c)   {cMana{w: {W%-5d{c({C%5d{c) {x\n\r",
race_table[victim->race].name,get_curr_stat(victim,STAT_INT),victim->perm_stat[STAT_INT],
victim->mana, victim->max_mana);
send_to_char( buf, ch );

sprintf( buf,"{cClass{w: {C%-12s {cWIS{w: {W%2d{c({C%2d{c)   {cMove{w: {W%-5d{c({C%5d{c){x\n\r", 
IS_NPC(victim) ? "mobile" :
class_table[victim->class].name,get_curr_stat(victim,STAT_WIS),
victim->perm_stat[STAT_WIS],victim->move,victim->max_move);
send_to_char( buf, ch );

sprintf( buf,"{cAlign{w: {C%-12d {cDEX{w: {W%2d{c({C%2d{c)  {cPName{w: {w%s{x\n\r",
victim->alignment,get_curr_stat(victim,STAT_DEX),victim->perm_stat[STAT_DEX],
victim->pIndexData->player_name);
send_to_char( buf, ch );

sprintf( buf,"  {cSex{w: {C%-12s {cCON{w: {W%2d{c({C%2d{c)        {x\n\r",
victim->sex == 0 ? "SEXLESS" : victim->sex == 1 ? "MALE" : "FEMALE",
get_curr_stat(victim,STAT_CON),
victim->perm_stat[STAT_CON]);
send_to_char( buf, ch );

sprintf(buf,"\n\r{cPierce{w: {C%-6d    {CHIT{croll{w: {C%-5d      {cRoom{w: {C%d{x\n\r",
GET_AC(victim,AC_PIERCE),GET_HITROLL( victim ),
victim->in_room == NULL ? 0 : victim->in_room->vnum );
send_to_char( buf, ch );

sprintf(buf,"  {cBash{w: {C%-6d    {CDAM{croll{w: {C%-5d  {cPosition{w: {C%s{x\n\r",
GET_AC(victim,AC_BASH),GET_DAMROLL( victim ),
position_table[victim->position].name);
send_to_char( buf, ch );

sprintf(buf," {cSlash{w: {C%-6d      {cSaves{w: {C%-5d  {cFighting{w: {R%s{x\n\r",
GET_AC(victim,AC_SLASH),
victim->saving_throw,
victim->fighting ? victim->fighting->name : "{D({WNONE{D){x");
send_to_char( buf, ch );

sprintf(buf," {cMagic{w: {C%-6d       {cGold{w: {Y%ld    {cSilver{w: {W%ld{x\n\r\n\r",
GET_AC(victim,AC_EXOTIC),victim->gold,victim->silver);
send_to_char( buf, ch );

    sprintf( buf, "{cMaster{w: {C%s  {cLeader{w: {C%s   ",
	victim->master      ? victim->master->name   : "{D({WNONE{D)",
	victim->leader      ? victim->leader->name   : "{D({WNONE{D)");
    send_to_char( buf, ch );

    if ( victim->spec_fun != 0 )
    {
	sprintf(buf,"{cSPEC PROC{w: {C%s{x\n\r",spec_name(victim->spec_fun));
	send_to_char(buf,ch);
    }
   else
    {
	send_to_char("{cSPEC PROC{w: {D({WNONE{D){x\n\r",ch);
    }

    if (victim->comm)
    {
        sprintf(buf, "\n\r   {cACT{w: {C%s{x\n\r",act_bit_name(victim->act));
        send_to_char(buf,ch);
    }
   else
    {
	send_to_char("   {cACT{w: {D({WNONE{D){x\n\r",ch);
    }

    if (victim->imm_flags)
    {
	sprintf(buf, "{cIMMUNE{w: {C%s{x\n\r",imm_bit_name(victim->imm_flags));
	send_to_char(buf,ch);
    }
    else
    {
	send_to_char("{cIMMUNE{w: {D({WNONE{D){x\n\r",ch);
    }
 
    if (victim->res_flags)
    {
	sprintf(buf, "{cRESIST{w: {C%s{x\n\r", imm_bit_name(victim->res_flags));
	send_to_char(buf,ch);
    }
   else
    {
	send_to_char("{cRESIST{w: {D({WNONE{D){x\n\r", ch);
    }

    if (victim->vuln_flags)
    {
	sprintf(buf, "  {cVULN{w: {C%s{x\n\r", imm_bit_name(victim->vuln_flags));
	send_to_char(buf,ch);
    }
   else
    {
	send_to_char("  {cVULN{w: {D({WNONE{D){x\n\r",ch);
    }

    if (victim->off_flags)
    {
    	sprintf(buf, "   {cOFF{w: {C%s{x\n\r",off_bit_name(victim->off_flags));
	send_to_char(buf,ch);
    }
   else
    {
	send_to_char("   {cOFF{w: {D({WNONE{D){x\n\r",ch);
    }

    if (victim->affected_by)
    {
	sprintf(buf, "  {cAFFT{w: {C%s{x\n\r", 
	    affect_bit_name(victim->affected_by));
	send_to_char(buf,ch);
    }
   else
    {
	send_to_char("  {cAFFT{w: {D({WNONE{D){x\n\r",ch);
    }

    if (victim->affected2_by)
    {
	sprintf(buf, " {cAFFT2{w: {C%s{x\n\r\n\r", 
	    affect2_bit_name(victim->affected2_by));
	send_to_char(buf,ch);
    }
   else
    {
	send_to_char(" {cAFFT2{w: {D({WNONE{D){x\n\r\n\r",ch);
    }

sprintf( buf, "  {cForm{w: {C%s{x\n\r",form_bit_name(victim->form));
send_to_char( buf, ch );

sprintf( buf, " {cParts{w: {C%s{x\n\r",part_bit_name(victim->parts));
send_to_char( buf, ch );

    sprintf( buf, "\n\r{cS DESC{w: {W%s{x\n\r",victim->short_descr);
    send_to_char( buf, ch );

    sprintf(buf, "{cL DESC{w: {C%s{x\n\r",
    victim->long_descr[0] != '\0' ? victim->long_descr : "{D({WNONE{D){x\n\r" );
    send_to_char( buf, ch );
 }

/*
    for ( paf = victim->affected; paf != NULL; paf = paf->next )
    {
	sprintf( buf,
"{cSpell{w: {c'{W%s{c' Modifies{w: {W%s {cby {W%d {cfor {W%d {chours w/bits {W%s {w- {cLevel{w: {W%d{x\n\r",
	    skill_table[(int) paf->type].name,
	    affect_loc_name( paf->location ),
	    paf->modifier,
	    paf->duration,
	    affect_bit_name( paf->bitvector ),
	    paf->level
	    );
	send_to_char( buf, ch );
   }
*/
    for ( paf = victim->affected; paf != NULL; paf = paf->next )
    {
        if(paf->type<0)
            continue;
	sprintf( buf,
"{cSpell{w: {c'{W%s{c' Modifies{w: {W%s {cby {W%d {cfor {W%d {chours w/bits {W%s {w- {cLevel{w: {W%d{x\n\r",
	    skill_table[(int) paf->type].name,
	    affect_loc_name( paf->location ),
	    paf->modifier,
	    paf->duration,
	(paf->where == TO_AFFECTS2) ? affect2_bit_name(paf->bitvector) : 
((paf->where == TO_AFFECTS) ? affect_bit_name( paf->bitvector ) : "{D({WNONE{D)"),
	    paf->level
	    );
	send_to_char( buf, ch );
    }
return;
}


void do_mset( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    char buf[100];
    CHAR_DATA *victim;
    ROOM_INDEX_DATA *location;
    int value;

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    strcpy( arg3, argument );

    if ( arg1[0] == '\0' 
    || arg2[0] == '\0'
    || (arg3[0] == '\0' && (str_cmp( arg2,"protect"))) )
    {
        if (IS_IMP(ch))
          {
	send_to_char("\n\r{RIMP {GSyntax{w: {c'{WMSET {c<{WCHAR NAME{c> <{WFIELD{c> <{WVALUE{c>'{x\n\r",ch);
	send_to_char("            {c'{WSET CHAR {c<{WCHAR NAME{c> <{WFIELD{c> <{WVALUE{c>'{x\n\r",ch);
	send_to_char("            {c'{WSET MOB {c<{WCHAR NAME{c> <{WFIELD{c> <{WVALUE{c>'{x\n\r",ch);
          }
        else
          {
	send_to_char("\n\r{GSyntax{w: {c'{WMSET {c<{WCHAR NAME{c> <{WFIELD{c> <{WVALUE{c>'{x\n\r",ch);
          }
 
	send_to_char( "\n\r{cFIELDs{w: {CSTR INT WIS DEX CON SEX HP MANA MOVE ALIGN HOUSE{x\n\r",ch);
	send_to_char( "        {CDRUNK FULL GOLD SILVER GOLDBANK SILVBANK{x\n\r", ch );

        if(IS_IMP(ch))
          {
	send_to_char("\n\r{RIMP {cFIELDs{w: {WSEC QTIME CLANLEAD QP THIRST HUNGER CLASS{x\n\r",ch);
	send_to_char("            {WLEVEL JOB PRAC TRAIN PROTECT WNAME{D({CPC ONLY{D){x\n\r", ch);
          }

        if (is_owner(ch))
          {
  	   send_to_char("\n\r{rOWNER{w: {WCOMM SIZE OLDCL OLDLVL RACE GROUP{x\n\r",ch);
          }
  
        if(IS_IMP(ch))
          {
	send_to_char("\n\r{cEXTRA INFO{w: {cWNAME {w- {CTo SET {WWNAME back to normal, use this {GSYNTAX{w:{x\n\r",ch);
	send_to_char("                    {c'{WMSET {c<{WCHAR NAME{c> {WWNAME NORMAL{c'{x\n\r",ch);
          }
	return;
    }

    if (!IS_IMP(ch))
      {
    	if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
    	{
	    send_to_char( "\n\r{RThey are {WNOT{R in this room.{x\n\r", ch );
	    return;
    	}
      }
    else
      { 
    	if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    	{
	    send_to_char( "\n\r{RThey are {WNOT{R within the MUD.{x\n\r", ch );
	    return;
    	}
      }

    /* clear zones for mobs */
    victim->zone = NULL;

    /*
     * Snarf the value (which need not be numeric).
     */
    value = is_number( arg3 ) ? atoi( arg3 ) : -1;

    /*
     * Set something.
     */

 if (IS_IMP(ch))
   {
    if ( !str_cmp( arg2, "wname" ) )
      {
       if ( IS_NPC(victim) )
         {
        send_to_char( "\n\r{RNot on NPCs!{x\n\r", ch );
        return;
         }

     if (!IS_IMP(victim) )
       {
        send_to_char( "\n\r{ROnly {WIMPLEMENTORS {Rmay have their {WWHO NAME {Rchanged!{x\n\r", ch );
        return;
       }

     if (!str_cmp(arg3,"normal"))
       {
        strcpy( buf, arg3 );
        free_string( victim->who_name );
        victim->who_name = victim->name;
       }
     else
       {
        strcpy( buf, arg3 );
        free_string( victim->who_name );
        victim->who_name = str_dup( buf );
       }
     return;
    }

  if ( !str_cmp( arg2, "protect" ) )
    {	
     if (IS_SET(victim->comm,COMM_SNOOP_PROOF))
       {
        act_new("\n\r{C$N {cis {WNO {clonger snoop-proof.{x",ch,NULL,victim,TO_CHAR,POS_DEAD);
        REMOVE_BIT(victim->comm,COMM_SNOOP_PROOF);
       }
     else 
       {
      act_new("\n\r{C$N {cis now snoop-proof.{x",ch,NULL,victim,TO_CHAR,POS_DEAD);
      SET_BIT(victim->comm,COMM_SNOOP_PROOF);
       }
      
     return;
    }


  if ( !str_cmp( arg2, "qp" ) )
    {	
        if ( IS_NPC(ch) )
        {
                send_to_char( "\n\r{WNot while you are an {RNPC{W.{x\n\r", ch );
                return;
        }

        if ( IS_NPC( victim ) )
        {
            send_to_char( "\n\r{WNot on {RNPC{W'{Rs{W.{x\n\r", ch );
            return;
        }

        victim->questpoints = value;
        return;
    }

    if ( !str_cmp( arg2, "sec" ) )	
    {
	if ( IS_NPC(ch) )
	{
		send_to_char( "\n\r{WNot while you are an {RNPC{W.{x\n\r", ch );
		return;
	}

        if ( IS_NPC( victim ) )
        {
            send_to_char( "\n\r{WNot on {RNPC{W'{Rs{W.{x\n\r", ch );
            return;
        }

	if ( value > ch->pcdata->security || value < 0 )
	{
	    if ( ch->pcdata->security != 0 )
	    {
		sprintf( buf, "\n\r{WValid security is{w: {c<{C0{C-{C%d{c>{W.{x\n\r",
		    ch->pcdata->security );
		send_to_char( buf, ch );
	    }
	    else
	    {
		send_to_char( "\n\r{WValid security is{w: {c< {C0{c >{W only.{x\n\r", ch );
	    }
	    return;
	}
	victim->pcdata->security = value;
	return;
     }

    if ( !str_cmp( arg2, "qtime" ) )	
      {
	if ( IS_NPC(ch) )
	{
		send_to_char( "\n\r{WNot while you are an {RNPC{W.{x\n\r", ch );
		return;
	}

        if ( IS_NPC( victim ) )
        {
            send_to_char( "\n\r{WNot on {RNPC{W'{Rs{W.{x\n\r", ch );
            return;
        }

	victim->nextquest = value;
	return;
      }

    if ( !str_cmp( arg2, "clanlead" ) )	
    {
	if ( IS_NPC(ch) )
	{
		send_to_char( "\n\r{RNPC{W's can't do this.{x\n\r", ch );
		return;
	}

        if ( IS_NPC( victim ) )
        {
            send_to_char( "\n\r{WNot on {RNPC{W'{Rs{W.{x\n\r", ch );
            return;
        }

        if (IS_SET(victim->pact, PLR_CLAN_LEADER))
          {
           REMOVE_BIT(victim->pact, PLR_CLAN_LEADER);
           sprintf(buf,"\n\r{W%s {cis no longer a {CClan Leader{c.{x\n\r",victim->name);
           send_to_char(buf,ch);
           send_to_char("\n\r{WYou are {Rno{W longer a {cClan Leader{W.{x\n\r",victim);
           return;
          }
         else
          {
           SET_BIT(victim->pact, PLR_CLAN_LEADER);
           sprintf(buf,"\n\r{W%s {cis now a {CClan Leader{c.{x\n\r",victim->name);
           send_to_char(buf,ch);
           send_to_char("\n\r{WYou are {Rnow {Wa {cClan Leader{W.{x\n\r",victim);
           return;
          }
    return;
    }

    if ( !str_prefix( arg2, "level" ) )
    {
	if ( !IS_NPC(victim) )
	{
	    send_to_char( "\n\r{MNot on PC's.{x\n\r", ch );
	    return;
	}

	if ( value < 0 || value > MAX_LEVEL )
	{
	    sprintf(buf, "{cLevel range is {W0 {cto {W%d{c.{x\n\r", MAX_LEVEL);
	    send_to_char(buf, ch);
	    return;
	}
	victim->level = value;
	return;
    }

    if ( !str_prefix( arg2, "house" ) )
    {
	if ( IS_NPC(victim) )
	{
	    send_to_char( "\n\r{MNot on NPC's.{x\n\r", ch );
	    return;
	}

    if ( value == -1 )
    {
        send_to_char( "\n\r{RPlease enter a numeric room vnum.\n\r", ch);
        return;
    }

    if ( value == 0 )
    {
        victim->pcdata->house = 0;
        return;
    }

    if ( ( location=get_room_index(value) ) == NULL )
    {
	send_to_char( "\n\r{RNo such location!{x\n\r", ch );
	return;
    }

	victim->pcdata->house = value;
	return;
    }

    if ( !str_prefix( arg2, "class" ) )
    {
	int class;

	if (IS_NPC(victim))
	{
	    send_to_char("\n\r{GMobiles have no class.{x\n\r",ch);
	    return;
	}

	class = class_lookup(arg3);
	if ( class == -1 )
	{
	    char buf[MAX_STRING_LENGTH];

        	strcpy( buf, "\n\r{cPossible classes are{w:{x\n\r{W " );
        	for ( class = 0; class < MAX_CLASS; class++ )
        	{
            	    if ( class > 0 )
                    	strcat( buf, " " );
            	    strcat( buf, class_table[class].name );
        	}
            strcat( buf, "{c.{x\n\r" );
	    send_to_char(buf,ch);
	    return;
	}
	victim->class = class;
	return;
    }

        if ( !str_cmp( arg2, "job" ) )
        {

                if ( IS_NPC(victim) )
                {
                        send_to_char( "\n\r{RNot on NPCs ya dope.{x\n\r", ch );
                        return;
                }

                if (victim->level<=LEVEL_HERO)
                {
send_to_char("\n\r{RIMMORTALS!  {WYou trying to crash the mud or something{R?{x\n\r",ch);
                        return;
                }
         strcpy( buf, arg3 );
         free_string( victim->pcdata->job );
         victim->pcdata->job = str_dup( buf );

         return;
         }

    if ( !str_cmp( arg2, "thirst" ) )
    {
	if ( IS_NPC(victim) )
	{
	    send_to_char( "\n\r{RNot on NPC's.{x\n\r", ch );
	    return;
	}

	if ( value < -1 || value > 100 )
	{
	    send_to_char( "\n\r{WThirst range is {r-1 {Wto {r100{W.{x\n\r", ch );
	    return;
	}

	victim->pcdata->condition[COND_THIRST] = value;
	return;
    }

    if ( !str_cmp( arg2, "hunger" ) )
    {
        if ( IS_NPC(victim) )
        {
	    send_to_char( "\n\r{RNot on NPC's.{x\n\r", ch );
            return;
        }
 
        if ( value < -1 || value > 100 )
        {
            send_to_char( "\n\r{WHunger range is {r-1 {Wto {r100{W.{x\n\r", ch );
            return;
        }
 
        victim->pcdata->condition[COND_HUNGER] = value;
        return;
    }

    if ( !str_cmp( arg2, "prac" ) )
    {
	if ( value < 0 || value > 5000 )
	{
	    send_to_char( "\n\r{WPractice range is {c0 {Wto {c5000 {Wsessions.{x\n\r",ch );
	    return;
	}
	victim->practice = value;
	return;
    }

    if ( !str_cmp( arg2, "train" ))
    {
 	if (value < 0 || value > 5000 )
	{
	    send_to_char("\n\r{WTraining session range is {c0 {Wto {c5000 {Wsessions.{x\n\r",ch);
	    return;
 	}
  
	victim->train = value;
	return;
    }

  if (is_owner(ch))
    {

    if ( !str_prefix( arg2, "oldlvl" ) )
    {
	if ( !IS_NPC(victim) )
	{
	    send_to_char( "\n\r{MNot on PC's.{x\n\r", ch );
	    return;
	}

	if ( value < 0 || value > MAX_LEVEL )
	{
	    sprintf(buf, "{cLevel range is {W0 {cto {W%d{c.{x\n\r", MAX_LEVEL);
	    send_to_char(buf, ch);
	    return;
	}
	victim->oldlvl = value;
	return;
    }

    if ( !str_prefix( arg2, "oldcl" ) )
    {
	int class;

	if (IS_NPC(victim))
	{
	    send_to_char("\n\r{GMobiles have no class.{x\n\r",ch);
	    return;
	}

	class = class_lookup(arg3);
	if ( class == -1 )
	{
	    char buf[MAX_STRING_LENGTH];

        	strcpy( buf, "\n\r{cPossible classes are{w:{x\n\r{W " );
        	for ( class = 0; class < MAX_CLASS; class++ )
        	{
            	    if ( class > 0 )
                    	strcat( buf, " " );
            	    strcat( buf, class_table[class].name );
        	}
            strcat( buf, "{c.{x\n\r" );
	    send_to_char(buf,ch);
	    return;
	}
	victim->pcdata->oldcl = class;
	return;
    }

    if (!str_cmp(arg2,"group"))
    {
	if (!IS_NPC(victim))
	{
	    send_to_char("\n\r{WOnly on {RNPC{Ws.{x\n\r",ch);
	    return;
	}
	victim->group = value;
	return;
    }

    if (!str_cmp( arg2, "race" ) )
    {
	int race;

	race = race_lookup(arg3);

	if ( race == 0)
	{
	    send_to_char("\n\r{RThat is not a valid race.{x\n\r",ch);
	    return;
	}

      if (!IS_IMP(ch))
        {
	 if (!IS_NPC(victim) && !race_table[race].pc_race)
	   {
	    send_to_char("\n\r{WThat is not a valid player race.{x\n\r",ch);
	    return;
	   }
        }

	victim->race = race;
	return;
    }

    if ( !str_cmp( arg2, "size" ) )
    {
	if ( value < 0 || value > 5)
	{
	    send_to_char("\n\r{RSIZE range is from {W0 {c({WTiny{c) {Rto {W5 {c({WGiant{c){R.{x\n\r",ch);
	    return;
	}

	victim->perm_size = value;
        victim->size = victim->perm_size;
	return;
    }

    if ( !str_cmp( arg2, "comm" ) )
    {
      if(!IS_SET(victim->comm, COMM_HELPER))
	{
         SET_BIT(victim->comm, COMM_HELPER);
	 send_to_char("\n\r{CYour {cTARGET {Cis now an {WIMP HELPER{C...{x\n\r",ch);
	 return;
	}
      else
	{
 send_to_char("\n\r{RThat {rTARGET {Ris already an {WIMP HELPER{R!  {WPARDON{R it from them first...{x\n\r",ch);
	 return;
	}
     return;
    }
 }
}

    if ( !str_cmp( arg2, "str" ) )
    {
	if ( value < 3 || value > get_max_train(victim,STAT_STR) )
	{
	    sprintf(buf,
		"\n\r{cStrength range is {W3{c to {W%d{x\n\r.",
		get_max_train(victim,STAT_STR));
	    send_to_char(buf,ch);
	    return;
	}

	victim->perm_stat[STAT_STR] = value;
	return;
    }

    if ( !str_cmp( arg2, "int" ) )
    {
        if ( value < 3 || value > get_max_train(victim,STAT_INT) )
        {
            sprintf(buf,
		"\n\r{cIntelligence range is {W3{c to {W%d{c.{x\n\r",
		get_max_train(victim,STAT_INT));
            send_to_char(buf,ch);
            return;
        }
 
        victim->perm_stat[STAT_INT] = value;
        return;
    }

    if ( !str_cmp( arg2, "wis" ) )
    {
	if ( value < 3 || value > get_max_train(victim,STAT_WIS) )
	{
	    sprintf(buf,
		"\n\r{cWisdom range is {W3 {cto {W%d{c.{x\n\r",
            get_max_train(victim,STAT_WIS));
	    send_to_char( buf, ch );
	    return;
	}

	victim->perm_stat[STAT_WIS] = value;
	return;
    }

    if ( !str_cmp( arg2, "dex" ) )
    {
	if ( value < 3 || value > get_max_train(victim,STAT_DEX) )
	{
	    sprintf(buf,
		"\n\r{cDexterity range is {W3 {cto {W%d{c.{x\n\r",
		get_max_train(victim,STAT_DEX));
	    send_to_char( buf, ch );
	    return;
	}

	victim->perm_stat[STAT_DEX] = value;
	return;
    }

    if ( !str_cmp( arg2, "con" ) )
    {
	if ( value < 3 || value > get_max_train(victim,STAT_CON) )
	{
	    sprintf(buf,
		"\n\r{cConstitution range is {W3 {cto {W%d{c.{x\n\r",
		get_max_train(victim,STAT_CON));
	    send_to_char( buf, ch );
	    return;
	}

	victim->perm_stat[STAT_CON] = value;
	return;
    }

    if ( !str_prefix( arg2, "sex" ) )
    {
	if ( value < 0 || value > 2 )
	{
	    send_to_char( "{cSex range is {W0 {cto {W2{c.{x\n\r", ch );
	    return;
	}
	victim->sex = value;
	if (!IS_NPC(victim))
	    victim->pcdata->true_sex = value;
	return;
    }


    if(!str_cmp(arg2,"goldbank"))
      {
       victim->pcdata->gold_bank=value;
       return;
      }

    if(!str_cmp(arg2,"Silvbank"))
      {
       victim->pcdata->silver_bank=value;
       return;
      }

    if ( !str_cmp( arg2, "gold" ) )
      {
       victim->gold = value;
       return;
      }

    if ( !str_cmp(arg2, "silver" ) )
    {
	victim->silver = value;
	return;
    }

    if ( !str_cmp( arg2, "hp" ) )
    {
     if (IS_IMP(ch))
      {
	if ( value < 1 || value > 200000 )
	{
	    send_to_char( "\n\r{cHP range is {W1 {cto {W200,000 {cpoints.{x\n\r",ch);
	    return;
	}
	victim->max_hit = value;
        if (!IS_NPC(victim))
            victim->pcdata->perm_hit = value;
	return;
      }
    else
      {
	if ( value < 1 || value > 30000 )
	{
	    send_to_char( "\n\r{cHP range is {W1 {cto {W30,000 {cpoints.{x\n\r",ch );
	    return;
	}
	victim->max_hit = value;
        if (!IS_NPC(victim))
            victim->pcdata->perm_hit = value;
	return;
      }
  return;
    }

    if ( !str_cmp( arg2, "mana" ) )
    {

     if (IS_IMP(ch))
       {
	if ( value < 1 || value > 200000 )
	  {
	    send_to_char( "\n\r{cMANA range is {W1 {cto {W200,000 {cpoints.{x\n\r",ch);
	    return;
	  }
	 victim->max_mana = value;
         if (!IS_NPC(victim))
            victim->pcdata->perm_mana = value;
	 return;
       }
      else
       {
	if ( value < 0 || value > 30000 )
	{
	    send_to_char( "\n\r{cMANA range is {W0 {cto {W30,000 {cpoints.{x\n\r",ch);
	    return;
	}
	victim->max_mana = value;
        if (!IS_NPC(victim))
            victim->pcdata->perm_mana = value;
	return;
       }
   return;
  }


    if ( !str_cmp( arg2, "move" ) )
    {

     if (IS_IMP(ch))
      {
	if ( value < 1 || value > 200000 )
	{
	    send_to_char( "\n\r{cMOVE range is {W1 {cto {W200,000 {cpoints.{x\n\r",ch );
	    return;
	}
	victim->max_move = value;
        if (!IS_NPC(victim))
            victim->pcdata->perm_move = value;
	return;
      }
    else
       {
	if ( value < 0 || value > 30000 )
	{
	    send_to_char( "\n\r{cMOVE range is {W0 {cto {W30,000{c points.{x\n\r", ch );
	    return;
	}
	victim->max_move = value;
        if (!IS_NPC(victim))
            victim->pcdata->perm_move = value;
	return;
       }
    return;
   }


    if ( !str_cmp( arg2, "align" ) )
    {
	if ( value < -1000 || value > 1000 )
	{
	    send_to_char( "\n\r{MAlignment range is {w-1000 {Mto {w1000{M.{x\n\r",ch );
	    return;
	}
	victim->alignment = value;
	return;
    }

    if ( !str_cmp( arg2, "drunk" ) )
    {
	if ( IS_NPC(victim) )
	{
	    send_to_char( "\n\r{RNot on NPC's.{x\n\r", ch );
	    return;
	}

	if ( value < -1 || value > 100 )
	{
	    send_to_char( "\n\r{WDrunk range is {r-1 {Wto {r100{W.{x\n\r", ch );
	    return;
	}

	victim->pcdata->condition[COND_DRUNK] = value;
	return;
    }

    if ( !str_cmp( arg2, "full" ) )
    {
	if ( IS_NPC(victim) )
	{
	    send_to_char( "\n\r{RNot on NPC's.{x\n\r", ch );
	    return;
	}

	if ( value < -1 || value > 100 )
	{
	    send_to_char( "\n\r{WFull range is {r-1 {Wto {r100{W.{x\n\r", ch );
	    return;
	}

	victim->pcdata->condition[COND_FULL] = value;
	return;
    }

    /*
     * Generate usage message.
     */
    do_function(ch, &do_mset, "" );
    return;
}


void do_oset( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int value;

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    strcpy( arg3, argument );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
	send_to_char("\n\r{GSyntax{w: {WOSET {c<{WOBJ NAME{c> <{WFIELD{c> <{WVALUE{c>\n\r",ch);

	send_to_char("\n\r{cFIELDS{w: {CCOND LEVEL WEIGHT TIMER COST{x\n\r",ch );


        if (IS_IMP(ch))
          {
        send_to_char("\n\r\n\r{RIMP ONLY{x",ch);
        send_to_char("\n\r{GSyntax{w: {WSET OBJ {c<{WOBJ NAME{c> <{WFIELD{c> <{WVALUE{c>\n\r",ch);
send_to_char("\n\r{cFIELDS{w: {CVALUE0{c({Cv0{c) {CVALUE1{c({Cv1{c) {CVALUE2{c({Cv2{c){x\n\r",ch);
send_to_char("{c{r        {CVALUE3{c({Cv3{c) {CVALUE4{c({Cv4{c){x\n\r",ch);
        send_to_char("        {CEXTRA WEAR TYPE MATER{x\n\r",ch);
	   return;
          }
    }

    if(!IS_IMP(ch))
     {
      if ( ( obj = get_obj_here( ch, arg1 ) ) == NULL )
       {
	send_to_char( "\n\r{RNothing like that within this room.{x\n\r", ch );
	return;
       }
     }
    else
     {
      if ( ( obj = get_obj_world( ch, arg1 ) ) == NULL )
       {
	send_to_char( "\n\r{GNothing like that in heaven or earth.{x\n\r", ch );
	return;
       }
     }


    /*
     * Snarf the value (which need not be numeric).
     */
    value = atoi( arg3 );

    /*
     * Set something.
     */


        if (IS_IMP(ch))
          {
    if ( !str_cmp( arg2, "value0" ) || !str_cmp( arg2, "v0" ) )
    {
     if (!IS_IMP(ch))
       {
	obj->value[0] = UMIN(30,value);
       }
      else
       {
	obj->value[0] = value;
       }

     return;
    }

    if ( !str_cmp( arg2, "value1" ) || !str_cmp( arg2, "v1" ) )
    {
     if (!IS_IMP(ch))
       {  
        obj->value[1] = UMIN(30,value);
       }
      else
       {
        obj->value[1] = value;         
       }

	return;
    }

    if ( !str_cmp( arg2, "value2" ) || !str_cmp( arg2, "v2" ) )
    {
     if (!IS_IMP(ch))
       {  
        obj->value[2] = UMIN(30,value);
       }
      else
       {
        obj->value[2] = value;         
       }

	return;
    }

    if ( !str_cmp( arg2, "value3" ) || !str_cmp( arg2, "v3" ) )
    {
     if (!IS_IMP(ch))
       {  
        obj->value[3] = UMIN(30,value);
       }
      else
       {
        obj->value[3] = value;         
       }

	return;
    }

    if ( !str_cmp( arg2, "value4" ) || !str_cmp( arg2, "v4" ) )
    {
     if (!IS_IMP(ch))
       {  
        obj->value[4] = UMIN(30,value);
       }
      else
       {
        obj->value[4] = value;         
       }
	return;
    }

    if ( !str_prefix( arg2, "extra" ) )
    {
	obj->extra_flags = value;
	return;
    }

    if ( !str_prefix( arg2, "mater" ) )
    {
	obj->material_type = value;
	return;
    }

    if ( !str_prefix( arg2, "type" ) )
    {
	obj->item_type = value;
	return;
    }

    if ( !str_prefix( arg2, "wear" ) )
    {
	obj->wear_flags = value;
	return;
    }
   }


    if ( !str_prefix( arg2, "level" ) )
    {
     if (ch->level < value)
       {
        send_to_char( "\n\r{rThe {RLEVEL {rmay {RNOT {rbe above your current {RLEVEL{r!{x\n\r", ch );
        return;
       }
      else
       {
	obj->level = value;
        return;
       }
    }

    if ( !str_prefix( arg2, "cond" ) )
    {
	obj->condition = value;
	return;
    }
	
    if ( !str_prefix( arg2, "weight" ) )
    {
     if (!IS_IMP(ch))
       {
        if (value < 40)
          {
           send_to_char( "\n\r{rThe {RWEIGHT {rmay {RNOT {rbe set below {W40{r!{x\n\r", ch );
           return;
          }
        else
          {
	   obj->weight = value;
           return;
          }
       }
     
	obj->weight = value;
	return;
    }

    if ( !str_prefix( arg2, "cost" ) )
    {

     if (!IS_IMP(ch))
       {
        if (value > 10000)
          {
           send_to_char( "\n\r{rThe {RCOST {rmay {RNOT {rbe set above {W10000{r!{x\n\r", ch );
           return;
          }
        else
          {
	   obj->cost = value;
           return;
          }
       }
     
       obj->cost = value;
       return;
    }

    if ( !str_prefix( arg2, "timer" ) )
    {
	obj->timer = value;
	return;
    }

    /*
     * Generate usage message.
     */
    do_function(ch, &do_oset, "" );
    return;
}


void do_rset( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    int value;

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    strcpy( arg3, argument );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
	send_to_char( "Syntax:\n\r",ch);
	send_to_char( "  set room <location> <field> <value>\n\r",ch);
	send_to_char( "  Field being one of:\n\r",			ch );
	send_to_char( "    flags sector\n\r",				ch );
	return;
    }

    if ( ( location = find_location( ch, arg1 ) ) == NULL )
    {
	send_to_char( "No such location.\n\r", ch );
	return;
    }

    if (!is_room_owner(ch,location) && ch->in_room != location 
    &&  room_is_private(location) && (!IS_IMP(ch)) )
    {
        send_to_char("That room is private right now.\n\r",ch);
        return;
    }

    /*
     * Snarf the value.
     */
    if ( !is_number( arg3 ) )
    {
	send_to_char( "Value must be numeric.\n\r", ch );
	return;
    }
    value = atoi( arg3 );

    /*
     * Set something.
     */
    if ( !str_prefix( arg2, "flags" ) )
    {
	location->room_flags	= value;
	return;
    }

    if ( !str_prefix( arg2, "sector" ) )
    {
	location->sector_type	= value;
	return;
    }

    /*
     * Generate usage message.
     */
    do_function(ch, &do_rset, "");
    return;
}


void do_owhere(CHAR_DATA *ch, char *argument)
  {
    char                arg1[MAX_INPUT_LENGTH];
    char                buf[MAX_STRING_LENGTH];
    int                 counter = 0,max_found;
    OBJ_DATA *          obj;
    OBJ_DATA *          in_obj;
    bool                found;
    BUFFER *            buffer1;

    argument = one_argument( argument, arg1 );

    if(arg1[0] == '\0')
    {
        send_to_char("\n\r{GSyntax{w: {WOWHERE {c<{WOBJECT{c>{x\n\r",ch);
	return;
    }

    counter = 0;
    found = FALSE;
    max_found = 150;
    buffer1 = new_buf();

        for(obj=object_list; obj; obj=obj->next)
        {
            if(!IS_IMP(ch))
            {
                if(IS_SET(obj->pIndexData->area->area_flags, AREA_IMP))
                {
                    send_to_char("\n\r{RThe OBJ can NOT be located by this means{r!{x\n\r",ch); 
                    continue;
                }
                
                if(ch->level < ASSTIMP)
                {
                    if(IS_SET(obj->pIndexData->area->area_flags, AREA_NOIMM))
                      {
                       if(!IS_SET(ch->comm, COMM_HELPER))
                         {
                          send_to_char("\n\r{MThe OBJ can NOT be located by this means{m!{x\n\r",ch); 
                          continue;
                         }
                      }
                    
                    if(!IS_BUILDER(ch,obj->pIndexData->area))
                      {
                       if(!IS_SET(ch->comm, COMM_HELPER))
                         {
                          send_to_char(
                          "\n\r{GThe OBJ can NOT be located by this means{g!{x\n\r",ch); 
                          continue;
                         }
                      }
                }
            }

           if(!can_see_obj(ch,obj) 
           || !is_name(arg1, obj->name) 
           || (ch->level != (MAX_LEVEL)
           &&  (ch->level < obj->level
           ||   obj->pIndexData->vnum == 23433) ))
                continue;

            found = TRUE;
            counter++;

            for(in_obj=obj; in_obj->in_obj; in_obj=in_obj->in_obj)
            ;


            if (!in_obj->carried_by || !IS_IMP(in_obj->carried_by))
	        {
                if(in_obj->carried_by 
                && can_see(ch,in_obj->carried_by) 
                && in_obj->carried_by->in_room )
                {
                  if (IS_SET (obj->item_type, ITEM_CORPSE_PC))
                   {
                    sprintf(buf, "{c[{W%3d{c][{W%5d{c]{x %-23.23s{x   {ccarried {W%-20.20s  {c[{CRoom %d{c]{x\n\r",
                    		counter,obj->pIndexData->vnum,
                    strip_color(obj->short_descr),
                    PERS(in_obj->carried_by,ch),
                    in_obj->carried_by->in_room->vnum);
                    add_buf(buffer1, buf);
                   }
                  else
                   {
                    sprintf(buf, "{c[{W%3d{c][{W%5d{c]{x %-23.23s{x   {ccarried {W%-20.20s  {c[{CRoom %d{c]{x\n\r",
                            counter,obj->pIndexData->vnum,
                    strip_color(obj->pIndexData->short_descr),
                    PERS(in_obj->carried_by,ch),
                    in_obj->carried_by->in_room->vnum);
                    add_buf(buffer1, buf);
                   }
                }
                else if(in_obj->in_room && can_see_room(ch,in_obj->in_room))
                {
                  if (IS_SET (obj->item_type, ITEM_CORPSE_PC))
                   {
                    sprintf(buf, "{c[{W%3d{c][{W%5d{c]{x %-23.23s{x   {c is in",
                            counter,obj->pIndexData->vnum,strip_color(obj->short_descr));
                    add_buf(buffer1, buf);
    
                    sprintf(buf, "  {W%-20.20s  {c[{CRoom %d{c]{x\n\r",
                    strip_color(in_obj->in_room->name),in_obj->in_room->vnum);
                    add_buf(buffer1, buf);
                   }
                  else
                   {
                    sprintf(buf, "{c[{W%3d{c][{W%5d{c]{x %-23.23s{x   {c is in",
                            counter,obj->pIndexData->vnum,strip_color(obj->pIndexData->short_descr));
                    add_buf(buffer1, buf);
    
                    sprintf(buf, "  {W%-20.20s  {c[{CRoom %d{c]{x\n\r",
                    strip_color(in_obj->in_room->name), in_obj->in_room->vnum);
                    add_buf(buffer1, buf);
                   }
                }
                else
                {
                  if (IS_SET (obj->item_type, ITEM_CORPSE_PC))
                   {
                    sprintf(buf, "{c[{W%3d{c][{W%5d{c]{x %s{x {Wis somewhere{x\n\r",
                            counter,obj->pIndexData->vnum, 
                    strip_color(obj->short_descr));
                    add_buf(buffer1, buf);
                   }
                  else
                   {
                    sprintf(buf, "{c[{W%3d{c][{W%5d{c]{x %s{x {Wis somewhere{x\n\r",
                            counter,obj->pIndexData->vnum,
                    strip_color(obj->pIndexData->short_descr));
                    add_buf(buffer1, buf);
                   }
                }
            }
         if (counter >= max_found)
         break;
        }

        if(!found)
        {
            send_to_char("\n\r{RNo objects were found matching that name.{x\n\r", ch);
            free_buf(buffer1);
            return;
        }

        send_to_char("\n\r{c[{WNUM{c][{WVnum {c] {WItem Name                 {cStatus  {WWhere                 {c[{CRoom Vnum{c]{x",ch);
        send_to_char("\n\r{c---------------------------------------------------------------------------------{x\n\r",ch);
        add_buf(buffer1,"{c---------------------------------------------------------------------------------{x\n\r");


/*
    sprintf(buf, "{cNumber of Objects Found{w: {W%d{x\n\r", counter);
    add_buf(buffer1, buf);
*/  
  page_to_char(buf_string(buffer1), ch);
    free_buf(buffer1);
    
    return;
  }


void do_mwhere(CHAR_DATA *ch, char *argument)
  {
    char                arg1[MAX_INPUT_LENGTH];
    char                buf[MAX_STRING_LENGTH];
    int                 counter = 0,max_found;
    CHAR_DATA *         mob;
    bool                found;
    BUFFER *            buffer1;

    argument = one_argument( argument, arg1 );

    if(arg1[0] == '\0')
    {
        send_to_char("\n\r{GSyntax{w: {WMWHERE {c<{WMOB NAME{c>{x\n\r",ch);
	return;
    }

    counter = 0;
    max_found = 150;
    found = FALSE;
    buffer1 = new_buf();

        for(mob = char_list; mob; mob=mob->next)
        {
		if(!mob->pIndexData)
			continue;

  if(!IS_IMP(ch))
   {
     if(IS_SET(mob->pIndexData->area->area_flags, AREA_IMP))
       {
        send_to_char(
        "\n\r{RThe MOB can NOT be located{r!{x\n\r",ch); 
        continue;
       }
     
     if(ch->level < ASSTIMP)
       {
        if(IS_SET(mob->pIndexData->area->area_flags, AREA_NOIMM))
          {
           if(!IS_SET(ch->comm, COMM_HELPER))
             {
              send_to_char(
              "\n\r{MThe MOB can NOT be located{m!{x\n\r",ch); 
              continue;
             }
          }
        
        if(!IS_BUILDER(ch,mob->pIndexData->area))
          {
           if(!IS_SET(ch->comm, COMM_HELPER))
             {
              send_to_char(
              "\n\r{GThe MOB can NOT be located{g!{x\n\r",ch); 
              continue;
             }
          }
       }
    }


          if (!IS_IMP(mob))
            {
            if(mob->in_room && is_name(arg1, mob->name))
            {
                found = TRUE;
                counter++;
   sprintf(buf, "{c[{W%3d{c][{W%5d{c]{x %-25s {c[{WRoom %5d{c] %s{x\n\r",counter,
                        IS_NPC(mob) ? mob->pIndexData->vnum : 0,
   IS_NPC(mob) ? strip_color(mob->short_descr) : strip_color(mob->name),
                        mob->in_room->vnum, mob->in_room->name);
                add_buf(buffer1,buf);
                   
         if (counter >= max_found)
         break;
            }
          }
        else
         if (IS_IMP(ch))
            {
            if(mob->in_room && is_name(arg1, mob->name))
            {
                found = TRUE;
                counter++;
                sprintf(buf, "{c[{W%3d{c][{W%5d{c]{x %-25s {c[{WRoom %5d{c] %s{x\n\r",counter,
                        IS_NPC(mob) ? mob->pIndexData->vnum : 0,
   IS_NPC(mob) ? strip_color(mob->short_descr) : strip_color(mob->name),
                        mob->in_room->vnum, mob->in_room->name);
                add_buf(buffer1,buf);
                   
         if (counter >= max_found)
         break;
            }
          }
        else
        break;
        }


        if(!found)
        {
            send_to_char("\n\r{RNo mobiles were found matching that name.{x\n\r", ch);
            free_buf(buffer1);
            return;
        }


        send_to_char("\n\r{c[{WNUM{c][{WVnum {c] {WMobile Name               {c[{WRoom Vnum {c] {WRoom Name{x",ch);
        send_to_char("\n\r{c--------------------------------------------------------------------------------{x\n\r",ch);
        add_buf(buffer1,"{c--------------------------------------------------------------------------------{x\n\r");

    page_to_char(buf_string(buffer1), ch);
    free_buf(buffer1);

    return;
}

void do_fine( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char arg3[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	int amount;

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
	argument = one_argument( argument, arg3 );

	if ((arg1[0] == '\0') || (arg2[0] == '\0'))
	{
		send_to_char("{cSyntax: fine <{Wplayer{c> <{Wamount{c>{x\n\r",ch);
		return;
	}

	if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
    	send_to_char( "\n\r{RThey are {WNOT{R within the MUD.{x\n\r", ch );
    	return;
    }

	if (is_number(arg2))
	{
		amount = atoi(arg2);
	}
	else
	{
		send_to_char("{cAmount must be a number.{x\n\r",ch);
		return;
	}


	if (( amount <= 0) || (amount > 100000))
	{
		send_to_char("{cInvalid amount.{x\n\r",ch);
		return;
	}

	if (victim->gold >= amount)
	{
		victim->gold -= amount;
		send_to_char("{cFine assessed.{x\n\r",ch);
		
		sprintf(buf, "{RYou have been fined{W %i{R gold by {W%s{R!{x\n\r", amount, ch->name);
		send_to_char(buf,victim);
		return;
	}
	else if (victim->pcdata->gold_bank >= amount)
	{
		victim->pcdata->gold_bank -= amount;
		send_to_char("{cFine assessed.{x\n\r",ch);
		
		sprintf(buf, "{RYou have been fined{W %i{R gold by {W%s{R!{x\n\r", amount, ch->name);
		send_to_char(buf,victim);
		return;
	}
	else
	{
		victim->gold = 0;
		victim->silver = 0;
		victim->pcdata->gold_bank = 0;
		victim->pcdata->silver_bank = 0;

		send_to_char("{cFine assessed.  Character bankrupt.{x\n\r",ch);
		send_to_char("{RYou have been fined more than you're worth.  You are bankrupt!{x\n\r",victim);
		return;
	}

	send_to_char("There has been an unforseen error.\n\r",ch);
	return;
}
