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
#include "recycle.h"
#include "tables.h"
#include "lookup.h"
#include "olc.h"


void	DelED(OBJ_DATA * obj, EXTRA_DESCR_DATA * ed);
EXTRA_DESCR_DATA *	AddED(OBJ_DATA * obj, char * keyword, char * data);
EXTRA_DESCR_DATA *	GetED(OBJ_DATA * obj, char * flag);


void do_dip(CHAR_DATA * ch, char * argument)
{
    char                arg1[MAX_INPUT_LENGTH];
    char                arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *			dipped;
    OBJ_DATA *			dipping;
    OBJ_DATA *			parchment;
	EXTRA_DESCR_DATA *	ed;
	EXTRA_DESCR_DATA *	dipEd;
	bool				fountain=FALSE;
    
    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

	if(arg1[0]=='\0')
	{
		send_to_char("Usage: dip <object to dip> <object to dip into>\n\r", ch);
		return;
	}
	
	// Lets make sure they have their object
	dipped = get_obj_carry(ch, arg1, ch);
	if(dipped==NULL)
	{
		send_to_char("You do not have the item you want to dip.\n\r", ch);
		return;
	}

	// If they don't type anything, check to see if there is a fountain in the room
	if(arg2[0]=='\0') 
	{
		for(dipping=ch->in_room->contents;dipping!=NULL;dipping=dipping->next_content)
        {
            if(dipping->item_type==ITEM_FOUNTAIN)
                break;
        }

        if(dipping==NULL)
        {
            send_to_char("Dip into what?\n\r", ch );
            return;
        }
        
        fountain=TRUE;
    }
    else
    {
    	dipping = get_obj_here(ch, arg2);
    	if(dipping==NULL)
    	{
    		send_to_char("You do not have the item you want to dip into.\n\r", ch);
            return;
        }
        
        if(dipping->item_type!=ITEM_POTION && dipping->item_type!=ITEM_DRINK_CON)
        {
        	send_to_char("You can only dip things into liquids! Like a potion!", ch);
        	return;
        }
    }
    
    // Ok now we have an item to dip and an item to dip into, lets see if its a special item
    ed = GetED(dipping, "*dip");
    if(ed==NULL)
    {
    	// That means the object being dipped into is not special, only thing this can do is
    	// erase scrolls and make them blank parchments, wet parchments for 2 ticks
    	if(dipped->item_type==ITEM_SCROLL)
    	{
			dipEd = GetED(dipped, "*fade");
			if(dipEd==NULL)
			{
    			act("You dip $p into $P. The writing on $p fades slightly.", ch, dipped, dipping, TO_CHAR);
    			act("$n dips $p into $P. The writing on $p fades slightly.", ch, dipped, dipping, TO_ROOM);
			
				dipEd = AddED(dipped, "*fade", "1");

    			if(fountain==FALSE)
    			{
    				// If its not a fountain, it uses the entire potion to erase the text
    				act("$p dries up!", ch, dipped, dipping, TO_CHAR);
    				act("$p dries up!", ch, dipped, dipping, TO_ROOM);
    			
    				extract_obj(dipping);
    			}
			}
			else
			{
    			act("You dip $p into $P. The writing completely fades away.", ch, dipped, dipping, TO_CHAR);
    			act("$n dips $p into $P. The writing completely fades away.", ch, dipped, dipping, TO_ROOM);
    			
    			if(fountain==FALSE)
    			{
    				// If its not a fountain, it uses the entire potion to erase the text
    				act("$P dries up!", ch, dipped, dipping, TO_CHAR);
    				act("$P dries up!", ch, dipped, dipping, TO_ROOM);
    			
    				extract_obj(dipping);
    			}
    			
    			extract_obj(dipped);
    			parchment = create_object(get_obj_index(26), ch->level);
    			obj_to_char(parchment, ch);
			}
    	}
    	else
    	{
    		act("You dip $p into $P. Nothing happens.", ch, dipped, dipping, TO_CHAR);
    		act("$n dips $p into $P. Nothing happens.", ch, dipped, dipping, TO_ROOM);
    	}
    }
    else
    {
    	// Add in flags for this stuff later
    	act("You dip $p into $P. Nothing happens.", ch, dipped, dipping, TO_CHAR);
    	act("$n dips $p into $P. Nothing happens.", ch, dipped, dipping, TO_ROOM);
    }
}

void	DelED(OBJ_DATA * obj, EXTRA_DESCR_DATA * ed)
{
	OBJ_INDEX_DATA *	pObj;
	EXTRA_DESCR_DATA *	eed;
	EXTRA_DESCR_DATA *	ped;
	
	ped=NULL;
	pObj = obj->pIndexData;
	
	for(eed=pObj->extra_descr;eed!=NULL;eed=eed->next)
    {
    	if(eed==ed)
    		break;
    	ped=eed;
    }
    
	if(!ped)
		pObj->extra_descr = ed->next;
	else
		ped->next = ed->next;
	
	free_extra_descr(ed);
}

EXTRA_DESCR_DATA *	AddED(OBJ_DATA * obj, char * keyword, char * data)
{
	OBJ_INDEX_DATA *	pObj;	
	EXTRA_DESCR_DATA *	ed;
	
	pObj = obj->pIndexData;
	
	ed = new_extra_descr();
	ed->keyword = str_dup(keyword);
	ed->next = pObj->extra_descr;
	pObj->extra_descr = ed;
	ed->description = str_dup(data);

	return ed;
}

EXTRA_DESCR_DATA *	GetED(OBJ_DATA * obj, char * flag)
{
	EXTRA_DESCR_DATA *		ed;
	
	for(ed=obj->extra_descr;ed!=NULL;ed=ed->next)
	{
		if(!str_cmp(flag, ed->keyword))
			return ed;
	}

	for(ed=obj->pIndexData->extra_descr;ed!=NULL;ed=ed->next)
	{
		if(!str_cmp(flag, ed->keyword))
			return ed;
	}
	
	return NULL;
}

