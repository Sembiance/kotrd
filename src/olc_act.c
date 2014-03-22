#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "tables.h"
#include "olc.h"
#include "recycle.h"
#include "lookup.h"
#include "recycle.h"
#include "interp.h"

#include "Utility.h"
#include "StringUtility.h"
#include "ArrayUtility.h"

char * mprog_type_to_name ( int type );

/* Return TRUE if area changed, FALSE if not. */
#define REDIT( fun )		bool fun( CHAR_DATA *ch, char *argument )
#define OEDIT( fun )		bool fun( CHAR_DATA *ch, char *argument )
#define MEDIT( fun )		bool fun( CHAR_DATA *ch, char *argument )
#define AEDIT( fun )		bool fun( CHAR_DATA *ch, char *argument )
#define HEDIT( fun )        bool fun( CHAR_DATA *ch, char *argument )

struct olc_help_type
{
    char *command;
    const void *structure;
    char *desc;
};



bool show_version( CHAR_DATA *ch, char *argument )
{
    send_to_char( VERSION, ch );
    send_to_char( "\n\r", ch );
    send_to_char( AUTHOR, ch );
    send_to_char( "\n\r", ch );
    send_to_char( DATE, ch );
    send_to_char( "\n\r", ch );
    send_to_char( CREDITS, ch );
    send_to_char( "\n\r", ch );

    return FALSE;
}    

/*
 * This table contains help commands and a brief description of each.
 * ------------------------------------------------------------------
 */
const struct olc_help_type help_table[] =
{
    {	"area",		area_flags,	 "Area attributes."		 },
    {	"room",		room_flags,	 "Room attributes."		 },
    {	"sector",	sector_flags,	 "Sector types, terrain."	 },
    {	"exit",		exit_flags,	 "Exit types."			 },
    {	"type",		type_flags,	 "Types of objects."		 },
    {	"extra",	extra_flags,	 "Object attributes."		 },
    {	"wear",		wear_flags,	 "Where to wear object."	 },
    {	"spec",		spec_table,	 "Available special programs." 	 },
    {	"sex",		sex_flags,	 "Sexes."			 },
    {	"act",		act_flags,	 "Mobile attributes."		 },
    {	"affect",	affect_flags,	 "Mobile affects."		 },
    {	"affect2",	affect2_flags,	 "Mobile affects2."		 },
    {	"wear_loc",	wear_loc_flags,	 "Where mobile wears object."	 },
    {	"spells",	skill_table,	 "Names of current spells." 	 },
    {	"container",	container_flags, "Container status."		 },

/* ROM specific bits: */

    {	"armor",	ac_type,	 "Ac for different attacks."	 },
    {   "apply",	apply_flags,	 "Apply flags"			 },
    {	"form",		form_flags,	 "Mobile body form."	         },
    {	"part",		part_flags,	 "Mobile body parts."		 },
    {	"imm",		imm_flags,	 "Mobile immunity."		 },
    {	"res",		res_flags,	 "Mobile resistance."	         },
    {	"vuln",		vuln_flags,	 "Mobile vulnerability."	 },
    {	"off",		off_flags,	 "Mobile offensive behaviour."	 },
    {	"size",		size_flags,	 "Mobile size."			 },
    {   "position",     position_flags,  "Mobile positions."             },
    {   "wclass",       weapon_class,    "Weapon class."                 }, 
    {   "wtype",        weapon_type2,    "Special weapon type."          },
    {	"portal",	portal_flags,	 "Portal types."		 },
    {	"furniture",	furniture_flags, "Furniture types."		 },
    {   "liquid",	liq_table,	 "Liquid types."		 },
    {	"apptype",	apply_types,	 "Apply types."			 },
    {	"material",	material_table,	 "Material Types for Objects."	 },
    {	"weapon",	attack_table,	 "Weapon types."		 },
    {	"mprog",	mprog_flags,	 "MobProgram flags."		 },
    {	NULL,		NULL,		 NULL				 }
};

void unlink_reset( ROOM_INDEX_DATA *pRoom, RESET_DATA *pReset )
{
	RESET_DATA *prev, *wReset;

	prev = pRoom->reset_first;
	for( wReset = pRoom->reset_first; wReset; wReset = wReset->next )
	{
		if( wReset == pReset )
		{
			if( pRoom->reset_first == pReset )
			{
				pRoom->reset_first = pReset->next;
				if( !pRoom->reset_first )
					pRoom->reset_last = NULL;
			}
			else if( pRoom->reset_last == pReset )
			{
				pRoom->reset_last = prev;
				prev->next = NULL;
			}
			else
				prev->next = prev->next->next;

			if( pRoom->area->reset_first == pReset )
				pRoom->area->reset_first = pReset->next;

			if( !pRoom->area->reset_first )
				pRoom->area->reset_last = NULL;
		}

		prev = wReset;
	}
}


void unlink_obj_index( OBJ_INDEX_DATA *pObj )
{
	int iHash;
	OBJ_INDEX_DATA *iObj, *sObj;

	iHash = pObj->vnum % MAX_KEY_HASH;

	sObj = obj_index_hash[iHash];

	if( sObj->next == NULL ) /* only entry */
		obj_index_hash[iHash] = NULL;
	else if( sObj == pObj ) /* first entry */
		obj_index_hash[iHash] = pObj->next;
	else /* everything else */
	{
		for( iObj = sObj; iObj != NULL; iObj = iObj->next )
		{
			if( iObj == pObj )
			{
				sObj->next = pObj->next;
				break;
			}
			sObj = iObj;
		}
	}
}


void unlink_room_index( ROOM_INDEX_DATA *pRoom )
{
	int iHash;
	ROOM_INDEX_DATA *iRoom, *sRoom;

	iHash = pRoom->vnum % MAX_KEY_HASH;

	sRoom = room_index_hash[iHash];

	if( sRoom->next == NULL ) /* only entry */
		room_index_hash[iHash] = NULL;
	else if( sRoom == pRoom ) /* first entry */
		room_index_hash[iHash] = pRoom->next;
	else /* everything else */
	{
		for( iRoom = sRoom; iRoom != NULL; iRoom = iRoom->next )
		{
			if( iRoom == pRoom )
			{
				sRoom->next = pRoom->next;
				break;
			}
			sRoom = iRoom;
		}
	}
}


void unlink_mob_index( MOB_INDEX_DATA *pMob )
{
	int iHash;
	MOB_INDEX_DATA *iMob, *sMob;

	iHash = pMob->vnum % MAX_KEY_HASH;

	sMob = mob_index_hash[iHash];

	if( sMob->next == NULL ) /* only entry */
		mob_index_hash[iHash] = NULL;
	else if( sMob == pMob ) /* first entry */
		mob_index_hash[iHash] = pMob->next;
	else /* everything else */
	{
		for( iMob = sMob; iMob != NULL; iMob = iMob->next )
		{
			if( iMob == pMob )
			{
				sMob->next = pMob->next;
				break;
			}
			sMob = iMob;
		}
	}
}



/*****************************************************************************
 Name:		show_flag_cmds
 Purpose:	Displays settable flags and stats.
 Called by:	show_help(olc_act.c).
 ****************************************************************************/
void show_flag_cmds( CHAR_DATA *ch, const struct flag_type *flag_table )
{
    char buf  [ MAX_STRING_LENGTH ];
    char buf1 [ MAX_STRING_LENGTH ];
    int  flag;
    int  col;
 
    buf1[0] = '\0';
    col = 0;

    send_to_char("\n\r{GAvailable FLAGs{w:{x\n\r\n\r",ch);

    for (flag = 0; flag_table[flag].name != NULL; flag++)
    {
	 if ( flag_table[flag].settable )
	   {
	    sprintf( buf, "{c[{C%-40.40s{c]", flag_table[flag].name );
	    strcat( buf1, buf );
	    if ( ++col % 2 == 0 )
		strcat( buf1, "{x\n\r" );
   	   }
    }
           if ( col % 2 != 0 )
   	   strcat( buf1, "{x\n\r" );

    send_to_char( buf1, ch );
    return;
}


/*****************************************************************************
 Name:		show_skill_cmds
 Purpose:	Displays all skill functions.
 		Does remove those damn immortal commands from the list.
 		Could be improved by:
 		(1) Adding a check for a particular class.
 		(2) Adding a check for a level range.
 Called by:	show_help(olc_act.c).
 ****************************************************************************/
void show_skill_cmds( CHAR_DATA *ch, int tar )
{
    char buf  [ MAX_STRING_LENGTH ];
    char buf1 [ MAX_STRING_LENGTH*2 ];
    int  sn;
    int  col;
 
    buf1[0] = '\0';
    col = 0;
    for (sn = 0; sn < MAX_SKILL; sn++)
    {
	if ( !skill_table[sn].name )
	    break;

	if ( !str_cmp( skill_table[sn].name, "reserved" )
	  || skill_table[sn].spell_fun == spell_null )
	    continue;

	if ( tar == -1 || skill_table[sn].target == tar )
	{
	    sprintf( buf, "%-19.18s", skill_table[sn].name );
	    strcat( buf1, buf );
	    if ( ++col % 4 == 0 )
		strcat( buf1, "\n\r" );
	}
    }
 
    if ( col % 4 != 0 )
	strcat( buf1, "\n\r" );

    send_to_char( buf1, ch );
    return;
}



/*****************************************************************************
 Name:		show_spec_cmds
 Purpose:	Displays settable special functions.
 Called by:	show_help(olc_act.c).
 ****************************************************************************/
void show_spec_cmds( CHAR_DATA *ch )
{
    char buf  [ MAX_STRING_LENGTH ];
    char buf1 [ MAX_STRING_LENGTH ];
    int  spec;
    int  col;
 
    buf1[0] = '\0';
    col = 0;
    send_to_char( "Preceed special functions with 'spec_'\n\r\n\r", ch );
    for (spec = 0; spec_table[spec].function != NULL; spec++)
    {
	sprintf( buf, "%-19.18s", &spec_table[spec].name[5] );
	strcat( buf1, buf );
	if ( ++col % 4 == 0 )
	    strcat( buf1, "\n\r" );
    }
 
    if ( col % 4 != 0 )
	strcat( buf1, "\n\r" );

    send_to_char( buf1, ch );
    return;
}



/*****************************************************************************
 Name:		show_help
 Purpose:	Displays help for many tables used in OLC.
 Called by:	olc interpreters.
 ****************************************************************************/
bool show_help( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    char spell[MAX_INPUT_LENGTH];
    int cnt;

    argument = one_argument( argument, arg );
    one_argument( argument, spell );

    /*
     * Display syntax.
     */
    if ( arg[0] == '\0' )
    {
	send_to_char( "Syntax:  ? [command]\n\r\n\r", ch );
	send_to_char( "[command]  [description]\n\r", ch );
	for (cnt = 0; help_table[cnt].command != NULL; cnt++)
	{
	    sprintf( buf, "%-10.10s -%s\n\r",
	        capitalize( help_table[cnt].command ),
		help_table[cnt].desc );
	    send_to_char( buf, ch );
	}
	return FALSE;
    }

    /*
     * Find the command, show changeable data.
     * ---------------------------------------
     */
    for (cnt = 0; help_table[cnt].command != NULL; cnt++)
    {
        if (  arg[0] == help_table[cnt].command[0]
          && !str_prefix( arg, help_table[cnt].command ) )
	{
	    if ( help_table[cnt].structure == spec_table )
	    {
		show_spec_cmds( ch );
		return FALSE;
	    }
	    else
	    if ( help_table[cnt].structure == liq_table )
	    {
	        show_liqlist( ch );
	        return FALSE;
	    }
	    else
	    if ( help_table[cnt].structure == attack_table )
	    {
	        show_damlist( ch );
	        return FALSE;
	    }
	    else
	    if ( help_table[cnt].structure == material_table )
	    {
	        show_mattype( ch );
	        return FALSE;
	    }
	    else
	    if ( help_table[cnt].structure == skill_table )
	    {

		if ( spell[0] == '\0' )
		{
		    send_to_char( "Syntax:  ? spells "
		        "[ignore/attack/defend/self/object/all]\n\r", ch );
		    return FALSE;
		}

		if ( !str_prefix( spell, "all" ) )
		    show_skill_cmds( ch, -1 );
		else if ( !str_prefix( spell, "ignore" ) )
		    show_skill_cmds( ch, TAR_IGNORE );
		else if ( !str_prefix( spell, "attack" ) )
		    show_skill_cmds( ch, TAR_CHAR_OFFENSIVE );
		else if ( !str_prefix( spell, "defend" ) )
		    show_skill_cmds( ch, TAR_CHAR_DEFENSIVE );
		else if ( !str_prefix( spell, "self" ) )
		    show_skill_cmds( ch, TAR_CHAR_SELF );
		else if ( !str_prefix( spell, "object" ) )
		    show_skill_cmds( ch, TAR_OBJ_INV );
		else
		    send_to_char( "Syntax:  ? spell "
		        "[ignore/attack/defend/self/object/all]\n\r", ch );
		    
		return FALSE;
	    }
	    else
	    {
		show_flag_cmds( ch, help_table[cnt].structure );
		return FALSE;
	    }
	}
    }

    show_help( ch, "" );
    return FALSE;
}


REDIT( redit_rlist )
{
    ROOM_INDEX_DATA	*pRoomIndex;
    AREA_DATA		*pArea;
    char		buf  [ MAX_STRING_LENGTH   ];
    BUFFER		*buf1;
    char		arg  [ MAX_INPUT_LENGTH    ];
    bool found;
    int vnum;
    int  col = 0;

    one_argument( argument, arg );

	send_to_char("\n\r",ch);

    pArea = ch->in_room->area;
    buf1=new_buf();
/*    buf1[0] = '\0'; */
    found   = FALSE;

    for ( vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++ )
    {
	if ( ( pRoomIndex = get_room_index( vnum ) ) )
	{
		found = TRUE;
		sprintf( buf, "[%5d] %-17.16s",
		    vnum, strip_color(capitalize( pRoomIndex->name )) );
		add_buf( buf1, buf );
		if ( ++col % 3 == 0 )
		    add_buf( buf1, "\n\r" );
	}
    }

    if ( !found )
    {
	send_to_char( "Room(s) not found in this area.\n\r", ch);
	return FALSE;
    }

    if ( col % 3 != 0 )
	add_buf( buf1, "\n\r" );

    page_to_char( buf_string(buf1), ch );
    free_buf(buf1);
    return FALSE;
}

REDIT( redit_mlist )
{
    MOB_INDEX_DATA	*pMobIndex;
    AREA_DATA		*pArea;
    char		buf  [ MAX_STRING_LENGTH   ];
    BUFFER		*buf1;
    char		arg  [ MAX_INPUT_LENGTH    ];
    bool fAll, found;
    int vnum;
    int  col = 0;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Syntax:  mlist <all/name>\n\r", ch );
	return FALSE;
    }

    buf1=new_buf();
    pArea = ch->in_room->area;
/*    buf1[0] = '\0'; */
    fAll    = !str_cmp( arg, "all" );
    found   = FALSE;

    for ( vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++ )
    {
	if ( ( pMobIndex = get_mob_index( vnum ) ) != NULL )
	{
	    if ( fAll || is_name( arg, pMobIndex->player_name ) )
	    {
		found = TRUE;
		sprintf( buf, "[%5d] %-17.16s",
		    pMobIndex->vnum, strip_color(capitalize( pMobIndex->short_descr) ) );
		add_buf( buf1, buf );
		if ( ++col % 3 == 0 )
		    add_buf( buf1, "\n\r" );
	    }
	}
    }

    if ( !found )
    {
	send_to_char( "Mobile(s) not found in this area.\n\r", ch);
	return FALSE;
    }

    if ( col % 3 != 0 )
	add_buf( buf1, "\n\r" );

    page_to_char( buf_string(buf1), ch );
    free_buf(buf1);
    return FALSE;
}



REDIT( redit_olist )
{
    OBJ_INDEX_DATA	*pObjIndex;
    AREA_DATA		*pArea;
    char		buf  [ MAX_STRING_LENGTH   ];
    BUFFER		*buf1;
    char		arg  [ MAX_INPUT_LENGTH    ];
    bool fAll, found;
    int vnum;
    int  col = 0;

	send_to_char("\n\r",ch);

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Syntax:  olist <all/name/item_type>\n\r", ch );
	return FALSE;
    }

    pArea = ch->in_room->area;
    buf1=new_buf();
/*    buf1[0] = '\0'; */
    fAll    = !str_cmp( arg, "all" );
    found   = FALSE;

    for ( vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++ )
    {
	if ( ( pObjIndex = get_obj_index( vnum ) ) )
	{
	    if ( fAll || is_name( arg, pObjIndex->name )
	    || flag_value( type_flags, arg ) == pObjIndex->item_type )
	    {
		found = TRUE;
		sprintf( buf, "[%5d] %-17.16s",
		    pObjIndex->vnum, strip_color(capitalize( pObjIndex->short_descr))
);
		add_buf( buf1, buf );
		if ( ++col % 3 == 0 )
		    add_buf( buf1, "\n\r" );
	    }
	}
    }

    if ( !found )
    {
	send_to_char( "Object(s) not found in this area.\n\r", ch);
	return FALSE;
    }

    if ( col % 3 != 0 )
	add_buf( buf1, "\n\r" );

    page_to_char( buf_string(buf1), ch );
    free_buf(buf1);
    return FALSE;
}



REDIT( redit_mshow )
{
    MOB_INDEX_DATA *pMob;
    int value;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax:  mshow <vnum>\n\r", ch );
	return FALSE;
    }

    if ( !is_number( argument ) )
    {
       send_to_char( "REdit: Ingresa un numero.\n\r", ch);
       return FALSE;
    }

    if ( is_number( argument ) )
    {
	value = atoi( argument );
	if ( !( pMob = get_mob_index( value ) ))
	{
	    send_to_char( "REdit:  That mobile does not exist.\n\r", ch );
	    return FALSE;
	}

	ch->desc->pEdit = (void *)pMob;
    }
 
    medit_show( ch, argument );
    ch->desc->pEdit = (void *)ch->in_room;
    return FALSE; 
}



REDIT( redit_oshow )
{
    OBJ_INDEX_DATA *pObj;
    int value;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax:  oshow <vnum>\n\r", ch );
	return FALSE;
    }

    if ( !is_number( argument ) )
    {
       send_to_char( "REdit: Ingresa un numero.\n\r", ch);
       return FALSE;
    }

    if ( is_number( argument ) )
    {
	value = atoi( argument );
	if ( !( pObj = get_obj_index( value ) ))
	{
	    send_to_char( "REdit:  That object does not exist.\n\r", ch );
	    return FALSE;
	}

	ch->desc->pEdit = (void *)pObj;
    }
 
    oedit_show( ch, argument );
    ch->desc->pEdit = (void *)ch->in_room;
    return FALSE; 
}



/*****************************************************************************
 Name:		check_range( lower vnum, upper vnum )
 Purpose:	Ensures the range spans only one area.
 Called by:	aedit_vnum(olc_act.c).
 ****************************************************************************/
bool check_range( int lower, int upper )
{
    AREA_DATA *pArea;
    int cnt = 0;

    for ( pArea = area_first; pArea; pArea = pArea->next )
    {
	/*
	 * lower < area < upper
	 */
        if ( ( lower <= pArea->min_vnum && pArea->min_vnum <= upper )
	||   ( lower <= pArea->max_vnum && pArea->max_vnum <= upper ) )
	    ++cnt;

	if ( cnt > 1 )
	    return FALSE;
    }
    return TRUE;
}



AREA_DATA *get_vnum_area( int vnum )
{
    AREA_DATA *pArea;

    for ( pArea = area_first; pArea; pArea = pArea->next )
    {
        if ( vnum >= pArea->min_vnum
          && vnum <= pArea->max_vnum )
            return pArea;
    }

    return 0;
}



/*
 * Area Editor Functions.
 */
AEDIT( aedit_show )
{
    AREA_DATA *pArea;
    char buf  [MAX_STRING_LENGTH];

    EDIT_AREA(ch, pArea);

    sprintf( buf, "\n\r    {cVNUM {C& {cNAME{w: {D[{C%3d{D] {W%s\n\r", pArea->vnum, pArea->name );
    send_to_char( buf, ch );

    sprintf( buf, "   {cTEMP LVL RNG{w: {D[{C%s{D]\n\r", pArea->credits );
    send_to_char( buf, ch );

    sprintf( buf, "    {cLEVEL RANGE{w: {D[{C%d{c-{C%d{D]\n\r", pArea->min_level, pArea->max_level );
    send_to_char( buf, ch );

    sprintf( buf, "{cROOM VNUM RANGE{w: {D[{C%d{c-{C%d{D]\n\r", pArea->min_vnum, pArea->max_vnum );
    send_to_char( buf, ch );

    sprintf( buf, "       {cSECURITY{w: {D[{C%d{D]\n\r", pArea->security );
    send_to_char( buf, ch );

    sprintf( buf, "       {cBUILDERs{w: {D[{C%s{D]\n\r", pArea->builders );
    send_to_char( buf, ch );

/*
#if 0
    sprintf( buf, "Recall:   [%5d] %s\n\r", pArea->recall,
	get_room_index( pArea->recall )
	? get_room_index( pArea->recall )->name : "none" );
    send_to_char( buf, ch );
#endif
*/

    sprintf( buf, "{cMORTALs in Area{w: {D[{C%d{D]\n\r", pArea->nplayer );
    send_to_char( buf, ch );

    sprintf( buf, "            {cAGE{w: {D[{C%d{D]\n\r",	pArea->age );
    send_to_char( buf, ch );

    sprintf( buf, "      {cFILE NAME{w: {D[{C%s{D]\n\r", pArea->file_name );
    send_to_char( buf, ch );

    if (pArea->area_flags)
    {
    sprintf(buf,  "          {cFLAGs{w: {D[{C%s{D]\n\r",area_bit_name(pArea->area_flags));
    send_to_char(buf,ch);  
    }
    else
    {
    send_to_char( "          {cFLAGs{w: {D[{RNONE{D]\n\r",ch);
    }

    return FALSE;
}



AEDIT( aedit_reset )
{
    AREA_DATA *pArea;

    EDIT_AREA(ch, pArea);

    reset_area( pArea );
    send_to_char( "Area reset.\n\r", ch );

    return FALSE;
}



AEDIT( aedit_create )
{
    AREA_DATA *pArea;

    pArea               =   new_area();
    area_last->next     =   pArea;
    area_last		=   pArea;	/* Thanks, Walker. */
    ch->desc->pEdit     =   (void *)pArea;

    SET_BIT( pArea->area_flags, AREA_ADDED );
    send_to_char( "Area Created.\n\r", ch );
    return FALSE;
}



AEDIT( aedit_name )
{
    AREA_DATA *pArea;

    EDIT_AREA(ch, pArea);

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax:   name [$name]\n\r", ch );
	return FALSE;
    }

    free_string( pArea->name );
    pArea->name = str_dup( argument );

    send_to_char( "Name set.\n\r", ch );
    return TRUE;
}

AEDIT( aedit_credits )
{
    AREA_DATA *pArea;

    EDIT_AREA(ch, pArea);

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax:   credits [$credits]\n\r", ch );
	return FALSE;
    }

    free_string( pArea->credits );
    pArea->credits = str_dup( argument );

    send_to_char( "Credits set.\n\r", ch );
    return TRUE;
}


AEDIT( aedit_file )
{
    AREA_DATA *pArea;
    char file[MAX_STRING_LENGTH];
    int i, length;

    EDIT_AREA(ch, pArea);

    one_argument( argument, file );	/* Forces Lowercase */

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax:  filename [$file]\n\r", ch );
	return FALSE;
    }

    /*
     * Simple Syntax Check.
     */
    length = strlen( argument );
    if ( length > 8 )
    {
	send_to_char( "No more than eight characters allowed.\n\r", ch );
	return FALSE;
    }
    
    /*
     * Allow only letters and numbers.
     */
    for ( i = 0; i < length; i++ )
    {
	if ( !isalnum( file[i] ) )
	{
	    send_to_char( "Only letters and numbers are valid.\n\r", ch );
	    return FALSE;
	}
    }    

    free_string( pArea->file_name );
    strcat( file, ".are" );
    pArea->file_name = str_dup( file );

    send_to_char( "Filename set.\n\r", ch );
    return TRUE;
}



AEDIT( aedit_age )
{
    AREA_DATA *pArea;
    char age[MAX_STRING_LENGTH];

    EDIT_AREA(ch, pArea);

    one_argument( argument, age );

    if ( !is_number( age ) || age[0] == '\0' )
    {
	send_to_char( "Syntax:  age [#xage]\n\r", ch );
	return FALSE;
    }

    pArea->age = atoi( age );

    send_to_char( "Age set.\n\r", ch );
    return TRUE;
}


#if 0 /* ROM OLC */
AEDIT( aedit_recall )
{
    AREA_DATA *pArea;
    char room[MAX_STRING_LENGTH];
    int  value;

    EDIT_AREA(ch, pArea);

    one_argument( argument, room );

    if ( !is_number( argument ) || argument[0] == '\0' )
    {
	send_to_char( "Syntax:  recall [#xrvnum]\n\r", ch );
	return FALSE;
    }

    value = atoi( room );

    if ( !get_room_index( value ) )
    {
	send_to_char( "AEdit:  Room vnum does not exist.\n\r", ch );
	return FALSE;
    }

    pArea->recall = value;

    send_to_char( "Recall set.\n\r", ch );
    return TRUE;
}
#endif /* ROM OLC */


AEDIT( aedit_security )
{
    AREA_DATA *pArea;
    char sec[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int  value;

    EDIT_AREA(ch, pArea);

    one_argument( argument, sec );

    if ( !is_number( sec ) || sec[0] == '\0' )
    {
	send_to_char( "Syntax:  security [#xlevel]\n\r", ch );
	return FALSE;
    }

    value = atoi( sec );

    if ( value > ch->pcdata->security || value < 0 )
    {
	if ( ch->pcdata->security != 0 )
	{
	    sprintf( buf, "Security is 0-%d.\n\r", ch->pcdata->security );
	    send_to_char( buf, ch );
	}
	else
	    send_to_char( "Security is 0 only.\n\r", ch );
	return FALSE;
    }

    pArea->security = value;

    send_to_char( "Security set.\n\r", ch );
    return TRUE;
}



AEDIT( aedit_builder )
{
    AREA_DATA *pArea;
    char name[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];

    EDIT_AREA(ch, pArea);

    one_argument( argument, name );

    if ( name[0] == '\0' )
    {
	send_to_char( "Syntax:  builder [$name]  -toggles builder\n\r", ch );
	send_to_char( "Syntax:  builder All      -allows everyone\n\r", ch );
	return FALSE;
    }

    name[0] = UPPER( name[0] );

    if ( strstr( pArea->builders, name ) != '\0' )
    {
	pArea->builders = string_replace( pArea->builders, name, "\0" );
	pArea->builders = string_unpad( pArea->builders );

	if ( pArea->builders[0] == '\0' )
	{
	    free_string( pArea->builders );
	    pArea->builders = str_dup( "None" );
	}
	send_to_char( "Builder removed.\n\r", ch );
	return TRUE;
    }
    else
    {
	buf[0] = '\0';
	if ( strstr( pArea->builders, "None" ) != '\0' )
	{
	    pArea->builders = string_replace( pArea->builders, "None", "\0" );
	    pArea->builders = string_unpad( pArea->builders );
	}

	if (pArea->builders[0] != '\0' )
	{
	    strcat( buf, pArea->builders );
	    strcat( buf, " " );
	}
	strcat( buf, name );
	free_string( pArea->builders );
	pArea->builders = string_proper( str_dup( buf ) );

	send_to_char( "Builder added.\n\r", ch );
	send_to_char( pArea->builders,ch);
	return TRUE;
    }

    return FALSE;
}



AEDIT( aedit_vnum )
{
    AREA_DATA *pArea;
    char lower[MAX_STRING_LENGTH];
    char upper[MAX_STRING_LENGTH];
    int  ilower;
    int  iupper;

    EDIT_AREA(ch, pArea);

    argument = one_argument( argument, lower );
    one_argument( argument, upper );

    if ( !is_number( lower ) || lower[0] == '\0'
    || !is_number( upper ) || upper[0] == '\0' )
    {
	send_to_char( "Syntax:  vnum [#xlower] [#xupper]\n\r", ch );
	return FALSE;
    }

    if ( ( ilower = atoi( lower ) ) > ( iupper = atoi( upper ) ) )
    {
	send_to_char( "AEdit:  Upper must be larger then lower.\n\r", ch );
	return FALSE;
    }
    
    if ( !check_range( atoi( lower ), atoi( upper ) ) )
    {
	send_to_char( "AEdit:  Range must include only this area.\n\r", ch );
	return FALSE;
    }

    if ( get_vnum_area( ilower )
    && get_vnum_area( ilower ) != pArea )
    {
	send_to_char( "AEdit:  Lower vnum already assigned.\n\r", ch );
	return FALSE;
    }

    pArea->min_vnum = ilower;
    send_to_char( "Lower vnum set.\n\r", ch );

    if ( get_vnum_area( iupper )
    && get_vnum_area( iupper ) != pArea )
    {
	send_to_char( "AEdit:  Upper vnum already assigned.\n\r", ch );
	return TRUE;	/* The lower value has been set. */
    }

    pArea->max_vnum = iupper;
    send_to_char( "Upper vnum set.\n\r", ch );

    return TRUE;
}



AEDIT( aedit_lvnum )
{
    AREA_DATA *pArea;
    char lower[MAX_STRING_LENGTH];
    int  ilower;
    int  iupper;

    EDIT_AREA(ch, pArea);

    one_argument( argument, lower );

    if ( !is_number( lower ) || lower[0] == '\0' )
    {
	send_to_char( "Syntax:  min_vnum [#xlower]\n\r", ch );
	return FALSE;
    }

    if ( ( ilower = atoi( lower ) ) > ( iupper = pArea->max_vnum ) )
    {
	send_to_char( "AEdit:  Value must be less than the max_vnum.\n\r", ch );
	return FALSE;
    }
    
    if ( !check_range( ilower, iupper ) )
    {
	send_to_char( "AEdit:  Range must include only this area.\n\r", ch );
	return FALSE;
    }

    if ( get_vnum_area( ilower )
    && get_vnum_area( ilower ) != pArea )
    {
	send_to_char( "AEdit:  Lower vnum already assigned.\n\r", ch );
	return FALSE;
    }

    pArea->min_vnum = ilower;
    send_to_char( "Lower vnum set.\n\r", ch );
    return TRUE;
}



AEDIT( aedit_uvnum )
{
    AREA_DATA *pArea;
    char upper[MAX_STRING_LENGTH];
    int  ilower;
    int  iupper;

    EDIT_AREA(ch, pArea);

    one_argument( argument, upper );

    if ( !is_number( upper ) || upper[0] == '\0' )
    {
	send_to_char( "Syntax:  max_vnum [#xupper]\n\r", ch );
	return FALSE;
    }

    if ( ( ilower = pArea->min_vnum ) > ( iupper = atoi( upper ) ) )
    {
	send_to_char( "AEdit:  Upper must be larger then lower.\n\r", ch );
	return FALSE;
    }
    
    if ( !check_range( ilower, iupper ) )
    {
	send_to_char( "AEdit:  Range must include only this area.\n\r", ch );
	return FALSE;
    }

    if ( get_vnum_area( iupper )
    && get_vnum_area( iupper ) != pArea )
    {
	send_to_char( "AEdit:  Upper vnum already assigned.\n\r", ch );
	return FALSE;
    }

    pArea->max_vnum = iupper;
    send_to_char( "Upper vnum set.\n\r", ch );

    return TRUE;
}


/* wizzle */

AEDIT( aedit_level )
{
    AREA_DATA *pArea;
    char llevel[MAX_STRING_LENGTH];
    char ulevel[MAX_STRING_LENGTH];
    int  illevel;
    int  iulevel;

    EDIT_AREA(ch, pArea);

    argument = one_argument( argument, llevel );
    one_argument( argument, ulevel );

    if ( !is_number( llevel ) || llevel[0] == '\0'
    || !is_number( ulevel ) || ulevel[0] == '\0' )
    {
	send_to_char( "\n\r{GSyntax{w: {WLEVEL {c<{WMINIMUM LEVEL{c> <{WUPPER LEVEL{c>{x\n\r", ch );
	return FALSE;
    }

    if ( ( illevel = atoi( llevel ) ) > ( iulevel = atoi( ulevel ) ) )
    {
	send_to_char( "\n\r{RUPPER LEVEL must be {WGREATER {Rthan the MINIMUM LEVEL!{x\n\r", ch );
	return FALSE;
    }
    
    pArea->min_level = illevel;
    send_to_char( "\n\r{CMinimum Level Set{x\n\r", ch );

    pArea->max_level = iulevel;
    send_to_char( "\n\r{CUpper Level Set{x\n\r", ch );

    return TRUE;
}


AEDIT( aedit_llevel )
{
    AREA_DATA *pArea;
    char llevel[MAX_STRING_LENGTH];
    int  illevel;
    int  iulevel;

    EDIT_AREA(ch, pArea);

    one_argument( argument, llevel );


    if ( !is_number( llevel ) || llevel[0] == '\0' )
    {
	send_to_char( "\n\r{GSyntax{w:  {WLEVEL {c<{WMINIMUM LEVEL{c> <{WUPPER LEVEL{c>\n\r{x", ch );
	return FALSE;
    }

    if ( ( illevel = atoi( llevel ) ) > ( iulevel = pArea->max_level ) )
    {
	send_to_char( "\n\r{RMINIMUM LEVEL must be {WLESS {Rthan the UPPER LEVEL!\n\r{x", ch );
	return FALSE;
    }

    pArea->min_level = illevel;
    send_to_char( "\n\r{CMinimum Level Set{x\n\r", ch );
    return TRUE;
}

AEDIT( aedit_ulevel )
{
    AREA_DATA *pArea;
    char ulevel[MAX_STRING_LENGTH];
    int  illevel;
    int  iulevel;

    EDIT_AREA(ch, pArea);

    one_argument( argument, ulevel );

    if ( !is_number( ulevel ) || ulevel[0] == '\0' )
    {
	send_to_char( "\n\r{GSyntax{w:  {WLEVEL {c<{WMINIMUM LEVEL{c> <{WUPPER LEVEL{c>{x\n\r", ch );
	return FALSE;
    }

    if ( ( illevel = pArea->min_level ) > ( iulevel = atoi( ulevel ) ) )
    {
	send_to_char( "\n\r{RUPPER LEVEL must be {WGREATER {Rthan the MINIMUM LEVEL!\n\r{x", ch );
	return FALSE;
    }

    pArea->max_level = iulevel;
    send_to_char( "\n\r{CUpper Level Set{x\n\r", ch );

    return TRUE;
}


REDIT( redit_show )
{
    ROOM_INDEX_DATA	*pRoom;
    CLAN_DATA		*pClan;
    char		buf  [MAX_STRING_LENGTH];
    char		buf1 [2*MAX_STRING_LENGTH];
    OBJ_DATA		*obj;
    CHAR_DATA		*rch;
    int			door;
    bool		fcnt;
    
    EDIT_ROOM(ch, pRoom);

    buf1[0] = '\0';
    
    sprintf( buf, "\n\r{cRoom DESC{w:\n\r    %s\n\r", pRoom->description );
    strcat( buf1, buf );

    sprintf(buf,"{RArea Name{w: {r[ {W%s{r ]  {RArea VNUM{w: {r[ {W%d{r ]\n\r",
pRoom->area->name, pRoom->area->vnum );
    strcat( buf1, buf );

    sprintf( buf, "     {cName{w: {c[ {C%s{c ]\n\r",pRoom->name);
    strcat( buf1, buf );

    sprintf( buf, "     {cVNUM{w: {c[ {C%d{c ]\n\r",
	    pRoom->vnum);
    strcat( buf1, buf );

sprintf(buf,"{cClanOwner{w: {c[ {D({W%d{D) {C%s{c ]{x\n\r", 
    ( pRoom->clanowner > 0 && (pClan=get_clan_index(pRoom->clanowner))) ? pClan->vnum : 0,
    ( pRoom->clanowner > 0 && (pClan=get_clan_index(pRoom->clanowner))) ? pClan->name : "NONE{x");
     strcat(buf1, buf);

    sprintf( buf, "\n\r{cRecovery Rates{w:\n\r");
    strcat( buf1, buf );

    if ( pRoom->heal_rate != 100 || pRoom->mana_rate != 100 )
    {
     sprintf( buf, 
"    {c-{CHP {D({WHEAL{D){w: {c[ {W%d{c ]\n\r         {c-{CMana{w: {c[ {W%d{c ]\n\r", 
pRoom->heal_rate , pRoom->mana_rate );
     strcat( buf1, buf );
    }
   else
    {
     sprintf( buf,
"    {c-{CHP {D({WHEAL{D){w: {c[ {W%d{c ]\n\r         {c-{CMana{w: {c[ {W%d{c ]\n\r", 
100,100);
     strcat( buf1, buf );
    }

    sprintf( buf, "\n\r {cSector Type{w: {c[ {C%s{c ]\n\r       {cFlags{w: {c[ {C%s{c ]\n\r",
flag_string( sector_flags, pRoom->sector_type ), flag_string( room_flags, pRoom->room_flags ) );
    strcat( buf1, buf );

    strcat( buf1, "{cNPCs in Room{w: {c[ {C" );
    fcnt = FALSE;
    
    for ( rch = pRoom->people; rch; rch = rch->next_in_room )
    {
     if (IS_NPC(rch))
       {
	one_argument( rch->name, buf );
	strcat( buf1, buf );
	strcat( buf1, " " );
	fcnt = TRUE;
       }
    }

    if ( fcnt )
    {
	int end;

	end = strlen(buf1) - 1;
	buf1[end] = ' ';
	strcat( buf1, "{c]{x\n\r" );
    }
    else
	strcat( buf1, "{CNONE{c ]\n\r" );

    strcat( buf1, "{cOBJs in Room{w: {c[ {C" );
    fcnt = FALSE;
    for ( obj = pRoom->contents; obj; obj = obj->next_content )
    {
	one_argument( obj->name, buf );
	strcat( buf1, buf );
	strcat( buf1, " " );
	fcnt = TRUE;
    }

    if ( fcnt )
    {
	int end;

	end = strlen(buf1) - 1;
	buf1[end] = ' ';
	strcat( buf1, "{c]{x\n\r" );
    }
    else
	strcat( buf1, "{CNONE{c ]\n\r" );

    if ( pRoom->extra_descr )
    {
	EXTRA_DESCR_DATA *ed;

	strcat( buf1, "{cExtra DESC Keywords{w:  {c[ {C" );
	for ( ed = pRoom->extra_descr; ed; ed = ed->next )
	{
	    strcat( buf1, ed->keyword );
	    if ( ed->next )
		strcat( buf1, " " );
	}
	strcat( buf1, "{c ]\n\r" );
    }
  else
    {
	sprintf( buf, "{cExtra DESC KeyWords{w: {c[ {C%s{c ]\n\r","NONE");
	strcat( buf1, buf );
    }

    sprintf( buf, "{cEXITs From Room{w:\n\r");
    strcat( buf1, buf );
    
    for ( door = 0; door < MAX_DIR; door++ )
    {
	EXIT_DATA *pexit;

	if ( ( pexit = pRoom->exit[door] ) )
	{
	    char word[MAX_INPUT_LENGTH];
	    char reset_state[MAX_STRING_LENGTH];
	    char *state;
	    int i, length;

	    sprintf( buf, "   {c-{C%-5s {cto [{W%5d{c] Key{w: {c[{W%5d{c] ",
		capitalize(dir_name[door]),
		pexit->u1.to_room ? pexit->u1.to_room->vnum : 0,      /* ROM OLC */
		pexit->key );
	    strcat( buf1, buf );

	    /*
	     * Format up the exit info.
	     * Capitalize all flags that are not part of the reset info.
	     */
	    strcpy( reset_state, flag_string( exit_flags, pexit->rs_flags ) );
	    state = flag_string( exit_flags, pexit->exit_info );
	    strcat( buf1, "{cExit flags{w: {c[ {C" );
	    for (; ;)
	    {
		state = one_argument( state, word );

		if ( word[0] == '\0' )
		{
		    int end;

		    end = strlen(buf1) - 1;
		    buf1[end] = ' ';
		    strcat( buf1, "{c]{x\n\r" );
		    break;
		}

		if ( str_infix( word, reset_state ) )
		{
		    length = strlen(word);
		    for (i = 0; i < length; i++)
			word[i] = UPPER(word[i]);
		}
		strcat( buf1, word );
		strcat( buf1, " " );
	    }

	    if ( pexit->keyword && pexit->keyword[0] != '\0' )
	    {
		sprintf( buf, "{cKeywords{w: {c[ {C%s{c ]\n\r", pexit->keyword );
		strcat( buf1, buf );
	    }
	    if ( pexit->description && pexit->description[0] != '\0' )
	    {
		sprintf( buf, "{C%s{x", pexit->description );
		strcat( buf1, buf );
	    }
	}
    }

    send_to_char("{x",ch);
    send_to_char( buf1, ch );
    return FALSE;
}

/*
 * redit_copy function thanks to Zanthras of Mystical Realities MUD.
 */
REDIT( redit_copy )
{
    ROOM_INDEX_DATA	*pRoom;
    ROOM_INDEX_DATA	*pRoom2; /* Room to copy */
    int vnum;

    if ( argument[0] == '\0' )
    {
      send_to_char("Syntax: copy <vnum> \n\r",ch);
      return FALSE;
    }

    if ( !is_number(argument) )
    {
      send_to_char("REdit: You must enter a number (vnum).\n\r",ch);
      return FALSE;
    }
    else /* argument is a number */
    {
      vnum = atoi(argument);
      if( !( pRoom2 = get_room_index(vnum) ) )
      {
	send_to_char("REdit: That room does not exist.\n\r",ch);
	return FALSE;
      }
    }

    EDIT_ROOM(ch, pRoom);

    free_string( pRoom->description );
    pRoom->description = str_dup( pRoom2->description );
    
    free_string( pRoom->name );
    pRoom->name = str_dup( pRoom2->name );

    /* sector flags */
    pRoom->sector_type = pRoom2->sector_type;

    /* room flags */
    pRoom->room_flags = pRoom2->room_flags;

    pRoom->heal_rate = pRoom2->heal_rate;
    pRoom->mana_rate = pRoom2->mana_rate;

    pRoom->clanowner = pRoom2->clanowner;

    free_string( pRoom->owner );
    pRoom->owner = str_dup( pRoom2->owner );

    pRoom->extra_descr = pRoom2->extra_descr;

    send_to_char( "Room info copied.", ch );
    return TRUE;
}


/* Local function. */
bool change_exit( CHAR_DATA *ch, char *argument, int door )
{
    ROOM_INDEX_DATA *pRoom;
    char command[MAX_INPUT_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    int  value;

    EDIT_ROOM(ch, pRoom);

    /*
     * Set the exit flags, needs full argument.
     * ----------------------------------------
     */
    if ( ( value = flag_value( exit_flags, argument ) ) != NO_FLAG )
    {
	ROOM_INDEX_DATA *pToRoom;
	sh_int rev;                                    /* ROM OLC */

	if ( !pRoom->exit[door] )
	   {
	   	send_to_char("Salida no existe.\n\r",ch);
	   	return FALSE;
	   }
	 /*   pRoom->exit[door] = new_exit(); */

	/*
	 * This room.
	 */
	TOGGLE_BIT(pRoom->exit[door]->rs_flags,  value);
	/* Don't toggle exit_info because it can be changed by players. */
	pRoom->exit[door]->exit_info = pRoom->exit[door]->rs_flags;

	/*
	 * Connected room.
	 */
	pToRoom = pRoom->exit[door]->u1.to_room;     /* ROM OLC */
	rev = rev_dir[door];

	if (pToRoom->exit[rev] != NULL)
	{
	   TOGGLE_BIT(pToRoom->exit[rev]->rs_flags,  value);
	   TOGGLE_BIT(pToRoom->exit[rev]->exit_info, value);
	}

	send_to_char( "Exit flag toggled.\n\r", ch );
	return TRUE;
    }

    /*
     * Now parse the arguments.
     */
    argument = one_argument( argument, command );
    one_argument( argument, arg );

    if ( command[0] == '\0' && argument[0] == '\0' )	/* Move command. */
    {
	move_char( ch, door, TRUE );                    /* ROM OLC */
	return FALSE;
    }

    if ( command[0] == '?' )
    {
	do_help( ch, "EXIT" );
	return FALSE;
    }

    if ( !str_cmp( command, "delete" ) )
    {
	ROOM_INDEX_DATA *pToRoom;
	sh_int rev;                                     /* ROM OLC */
	
	if ( !pRoom->exit[door] )
	{
	    send_to_char( "REdit:  Cannot delete a null exit.\n\r", ch );
	    return FALSE;
	}

	/*
	 * Remove ToRoom Exit.
	 */
	rev = rev_dir[door];
	pToRoom = pRoom->exit[door]->u1.to_room;       /* ROM OLC */
	
	if ( pToRoom->exit[rev] )
	{
	    free_exit( pToRoom->exit[rev] );
	    pToRoom->exit[rev] = NULL;
	}

	/*
	 * Remove this exit.
	 */
	free_exit( pRoom->exit[door] );
	pRoom->exit[door] = NULL;


        SET_BIT( pToRoom->area->area_flags,AREA_CHANGED );
        send_to_char( "Exit unlinked.\n\r", ch );
	return TRUE;
    }

    if ( !str_cmp( command, "link" ) )
    {
	EXIT_DATA *pExit;

	if ( arg[0] == '\0' || !is_number( arg ) )
	{
	    send_to_char( "Syntax:  [direction] link [vnum]\n\r", ch );
	    return FALSE;
	}

	value = atoi( arg );

	if ( !get_room_index( value ) )
	{
	    send_to_char( "REdit:  Cannot link to non-existant room.\n\r", ch );
	    return FALSE;
	}

  if(!IS_IMP(ch))
   {
    if((IS_SET(get_room_index( value )->area->area_flags, AREA_NOIMM))                        
    ||  (IS_SET(get_room_index( value )->area->area_flags, AREA_IMP)))                        
      {
          send_to_char(
          "\n\r{GIf you need something changed in this {WAREA{G, please note the IMPs!{x\n\r",ch);
          return FALSE;
      }
   }


	if ( get_room_index( value )->exit[rev_dir[door]] )
	{
	    send_to_char( "REdit:  Remote side's exit already exists.\n\r", ch );
	    return FALSE;
	}

	if ( !pRoom->exit[door] )
	{
	    pRoom->exit[door] = new_exit();
	}

	pRoom->exit[door]->u1.to_room = get_room_index( value );   /* ROM OLC */
	pRoom->exit[door]->orig_door = door;
       
	
/*	pRoom->exit[door]->vnum = value;                Can't set vnum in ROM */

	pRoom                   = get_room_index( value );
	door                    = rev_dir[door];
	pExit                   = new_exit();
	pExit->u1.to_room       = ch->in_room;
/*	pExit->vnum             = ch->in_room->vnum;    Can't set vnum in ROM */
	pExit->orig_door	= door;
	pRoom->exit[door]       = pExit;

	send_to_char( "Two-way link established.\n\r", ch );
        SET_BIT( get_room_index( value )->area->area_flags,AREA_CHANGED );
	return TRUE;
    }
        
    if ( !str_cmp( command, "dig" ) )
    {
	char buf[MAX_STRING_LENGTH];
	
	if ( arg[0] == '\0' || !is_number( arg ) )
	{
	    send_to_char( "Syntax: [direction] dig <vnum>\n\r", ch );
	    return FALSE;
	}
	
	redit_create( ch, arg );
	sprintf( buf, "link %s", arg );
	change_exit( ch, buf, door);
	return TRUE;
    }

    if ( !str_cmp( command, "room" ) )
    {
	if ( arg[0] == '\0' || !is_number( arg ) )
	{
	    send_to_char( "Syntax:  [direction] room [vnum]\n\r", ch );
	    return FALSE;
	}

	if ( !pRoom->exit[door] )
	{
	    pRoom->exit[door] = new_exit();
	}

	value = atoi( arg );

	if ( !get_room_index( value ) )
	{
	    send_to_char( "REdit:  Cannot link to non-existant room.\n\r", ch );
	    return FALSE;
	}

	pRoom->exit[door]->u1.to_room = get_room_index( value );    /* ROM OLC */
	pRoom->exit[door]->orig_door = door;
/*	pRoom->exit[door]->vnum = value;                 Can't set vnum in ROM */

	send_to_char( "One-way link established.\n\r", ch );
	return TRUE;
    }

    if ( !str_cmp( command, "key" ) )
    {
	if ( arg[0] == '\0' || !is_number( arg ) )
	{
	    send_to_char( "Syntax:  [direction] key [vnum]\n\r", ch );
	    return FALSE;
	}

	if ( !pRoom->exit[door] )
	   {
	   	send_to_char("Salida no existe.\n\r",ch);
	   	return FALSE;
	   }

/*	if ( !pRoom->exit[door] )
	{
	    pRoom->exit[door] = new_exit();
	} */

	value = atoi( arg );

	if ( !get_obj_index( value ) )
	{
	    send_to_char( "REdit:  Item doesn't exist.\n\r", ch );
	    return FALSE;
	}

	if ( get_obj_index( atoi( argument ) )->item_type != ITEM_KEY )
	{
	    send_to_char( "REdit:  Key doesn't exist.\n\r", ch );
	    return FALSE;
	}

	pRoom->exit[door]->key = value;

	send_to_char( "Exit key set.\n\r", ch );
	return TRUE;
    }

    if ( !str_cmp( command, "name" ) )
    {
	if ( arg[0] == '\0' )
	{
	    send_to_char( "Syntax:  [direction] name [string]\n\r", ch );
	    send_to_char( "         [direction] name none\n\r", ch );
	    return FALSE;
	}

	if ( !pRoom->exit[door] )
	   {
	   	send_to_char("Salida no existe.\n\r",ch);
	   	return FALSE;
	   }

/*	if ( !pRoom->exit[door] )
	{
	    pRoom->exit[door] = new_exit();
	} */

	free_string( pRoom->exit[door]->keyword );
	if (str_cmp(arg,"none"))
		pRoom->exit[door]->keyword = str_dup( arg );
	else
		pRoom->exit[door]->keyword = str_dup( "" );

	send_to_char( "Exit name set.\n\r", ch );
	return TRUE;
    }

    if ( !str_prefix( command, "description" ) )
    {
	if ( arg[0] == '\0' )
	{
	   if ( !pRoom->exit[door] )
	   {
	   	send_to_char("Salida no existe.\n\r",ch);
	   	return FALSE;
	   }

/*	    if ( !pRoom->exit[door] )
	    {
	        pRoom->exit[door] = new_exit();
	    } */

	    string_append( ch, &pRoom->exit[door]->description );
	    return TRUE;
	}

	send_to_char( "Syntax:  [direction] desc\n\r", ch );
	return FALSE;
    }

    return FALSE;
}



REDIT( redit_north )
{
    if ( change_exit( ch, argument, DIR_NORTH ) )
	return TRUE;

    return FALSE;
}



REDIT( redit_south )
{
    if ( change_exit( ch, argument, DIR_SOUTH ) )
	return TRUE;

    return FALSE;
}



REDIT( redit_east )
{
    if ( change_exit( ch, argument, DIR_EAST ) )
	return TRUE;

    return FALSE;
}



REDIT( redit_west )
{
    if ( change_exit( ch, argument, DIR_WEST ) )
	return TRUE;

    return FALSE;
}



REDIT( redit_up )
{
    if ( change_exit( ch, argument, DIR_UP ) )
	return TRUE;

    return FALSE;
}



REDIT( redit_down )
{
    if ( change_exit( ch, argument, DIR_DOWN ) )
	return TRUE;

    return FALSE;
}

REDIT( redit_northeast )
{
    if ( change_exit( ch, argument, DIR_NORTHEAST ) )
	return TRUE;

    return FALSE;
}

REDIT( redit_southeast )
{
    if ( change_exit( ch, argument, DIR_SOUTHEAST ) )
	return TRUE;

    return FALSE;
}

REDIT( redit_southwest )
{
    if ( change_exit( ch, argument, DIR_SOUTHWEST ) )
	return TRUE;

    return FALSE;
}

REDIT( redit_northwest )
{
    if ( change_exit( ch, argument, DIR_NORTHWEST ) )
	return TRUE;

    return FALSE;
}



REDIT( redit_ed )
{
    ROOM_INDEX_DATA *pRoom;
    EXTRA_DESCR_DATA *ed;
    char command[MAX_INPUT_LENGTH];
    char keyword[MAX_INPUT_LENGTH];

    EDIT_ROOM(ch, pRoom);

    argument = one_argument( argument, command );
    one_argument( argument, keyword );

    if ( command[0] == '\0' || keyword[0] == '\0' )
    {
	send_to_char( "Syntax:  ed add [keyword]\n\r", ch );
	send_to_char( "         ed edit [keyword]\n\r", ch );
	send_to_char( "         ed delete [keyword]\n\r", ch );
	send_to_char( "         ed format [keyword]\n\r", ch );
	return FALSE;
    }

    if ( !str_cmp( command, "add" ) )
    {
	if ( keyword[0] == '\0' )
	{
	    send_to_char( "Syntax:  ed add [keyword]\n\r", ch );
	    return FALSE;
	}

	ed			=   new_extra_descr();
	ed->keyword		=   str_dup( keyword );
	ed->description		=   str_dup( "" );
	ed->next		=   pRoom->extra_descr;
	pRoom->extra_descr	=   ed;

	string_append( ch, &ed->description );

	return TRUE;
    }


    if ( !str_cmp( command, "edit" ) )
    {
	if ( keyword[0] == '\0' )
	{
	    send_to_char( "Syntax:  ed edit [keyword]\n\r", ch );
	    return FALSE;
	}

	for ( ed = pRoom->extra_descr; ed; ed = ed->next )
	{
	    if ( is_name( keyword, ed->keyword ) )
		break;
	}

	if ( !ed )
	{
	    send_to_char( "REdit:  Extra description keyword not found.\n\r", ch );
	    return FALSE;
	}

	string_append( ch, &ed->description );

	return TRUE;
    }


    if ( !str_cmp( command, "delete" ) )
    {
	EXTRA_DESCR_DATA *ped = NULL;

	if ( keyword[0] == '\0' )
	{
	    send_to_char( "Syntax:  ed delete [keyword]\n\r", ch );
	    return FALSE;
	}

	for ( ed = pRoom->extra_descr; ed; ed = ed->next )
	{
	    if ( is_name( keyword, ed->keyword ) )
		break;
	    ped = ed;
	}

	if ( !ed )
	{
	    send_to_char( "REdit:  Extra description keyword not found.\n\r", ch );
	    return FALSE;
	}

	if ( !ped )
	    pRoom->extra_descr = ed->next;
	else
	    ped->next = ed->next;

	free_extra_descr( ed );

	send_to_char( "Extra description deleted.\n\r", ch );
	return TRUE;
    }


    if ( !str_cmp( command, "format" ) )
    {
	if ( keyword[0] == '\0' )
	{
	    send_to_char( "Syntax:  ed format [keyword]\n\r", ch );
	    return FALSE;
	}

	for ( ed = pRoom->extra_descr; ed; ed = ed->next )
	{
	    if ( is_name( keyword, ed->keyword ) )
		break;
	}

	if ( !ed )
	{
	    send_to_char( "REdit:  Extra description keyword not found.\n\r", ch );
	    return FALSE;
	}

	ed->description = format_string( ed->description );

	send_to_char( "Extra description formatted.\n\r", ch );
	return TRUE;
    }

    redit_ed( ch, "" );
    return FALSE;
}



REDIT( redit_create )
{
    AREA_DATA *pArea;
    ROOM_INDEX_DATA *pRoom;
    int value;
    int iHash;
    
    EDIT_ROOM(ch, pRoom);

    value = atoi( argument );

    if ( argument[0] == '\0' || value <= 0 )
    {
	send_to_char( "Syntax:  create [vnum > 0]\n\r", ch );
	return FALSE;
    }

    pArea = get_vnum_area( value );
    if ( !pArea )
    {
	send_to_char( "REdit:  That vnum is not assigned an area.\n\r", ch );
	return FALSE;
    }


  if(!IS_IMP(ch))
   {
    if((IS_SET(pArea->area_flags, AREA_NOIMM))
    ||  (IS_SET(pArea->area_flags, AREA_IMP)))
      {
          send_to_char(
          "\n\r{GIf you need something changed in this {WAREA{G, please note the IMPs!{x\n\r",ch);
          return FALSE;
      }
   }

    if ( get_room_index( value ) )
    {
	send_to_char( "REdit:  Room vnum already exists.\n\r", ch );
	return FALSE;
    }

    pRoom			= new_room_index();
    pRoom->area			= pArea;
    pRoom->vnum			= value;

    if ( value > top_vnum_room )
        top_vnum_room = value;

    iHash			= value % MAX_KEY_HASH;
    pRoom->next			= room_index_hash[iHash];
    room_index_hash[iHash]	= pRoom;
    ch->desc->pEdit		= (void *)pRoom;

    send_to_char( "Room created.\n\r", ch );
    return TRUE;
}



REDIT( redit_name )
{
    ROOM_INDEX_DATA *pRoom;

    EDIT_ROOM(ch, pRoom);

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax:  name [name]\n\r", ch );
	return FALSE;
    }

    free_string( pRoom->name );
    pRoom->name = str_dup( argument );

    send_to_char( "Name set.\n\r", ch );
    return TRUE;
}



REDIT( redit_desc )
{
    ROOM_INDEX_DATA *pRoom;

    EDIT_ROOM(ch, pRoom);

    if ( argument[0] == '\0' )
    {
	string_append( ch, &pRoom->description );
	return TRUE;
    }

    send_to_char( "Syntax:  desc\n\r", ch );
    return FALSE;
}

REDIT( redit_heal )
{
    ROOM_INDEX_DATA *pRoom;
    
    EDIT_ROOM(ch, pRoom);
    
    if (is_number(argument))
       {
          pRoom->heal_rate = atoi ( argument );
          send_to_char ( "Heal rate set.\n\r", ch);
          return TRUE;
       }

    send_to_char ( "Syntax : heal <#xnumber>\n\r", ch);
    return FALSE;
}       

REDIT( redit_mana )
{
    ROOM_INDEX_DATA *pRoom;
    
    EDIT_ROOM(ch, pRoom);
    
    if (is_number(argument))
       {
          pRoom->mana_rate = atoi ( argument );
          send_to_char ( "Mana rate set.\n\r", ch);
          return TRUE;
       }

    send_to_char ( "Syntax : mana <#xnumber>\n\r", ch);
    return FALSE;
}       

REDIT( redit_format )
{
    ROOM_INDEX_DATA *pRoom;

    EDIT_ROOM(ch, pRoom);

    pRoom->description = format_string( pRoom->description );

    send_to_char( "String formatted.\n\r", ch );
    return TRUE;
}



REDIT( redit_mreset )
{
  ROOM_INDEX_DATA	*pRoom;
  MOB_INDEX_DATA	*pMobIndex;
  CHAR_DATA		*newmob;
  char		arg [ MAX_INPUT_LENGTH ];
  char		arg2 [ MAX_INPUT_LENGTH ];

  RESET_DATA		*pReset;
  char		output [ MAX_STRING_LENGTH ];

  EDIT_ROOM(ch, pRoom);

  argument = one_argument( argument, arg );
  argument = one_argument( argument, arg2 );

  if ( arg[0] == '\0' || !is_number( arg ) )
    {
    send_to_char ( "Syntax:  mreset <vnum> <max #x> <mix #x>\n\r", ch );
    return FALSE;
    }

  if ( !( pMobIndex = get_mob_index( atoi( arg ) ) ) )
    {
    send_to_char( "REdit: No mobile has that vnum.\n\r", ch );
    return FALSE;
    }

  if ( pMobIndex->area != pRoom->area )
    {
    send_to_char( "REdit: No such mobile in this area.\n\r", ch );
    return FALSE;
    }

  /*
   * Create the mobile reset.
   */
  pReset              = new_reset_data();
  pReset->command	= 'M';
  pReset->arg1	= pMobIndex->vnum;
  pReset->arg2	= is_number( arg2 ) ? atoi( arg2 ) : MAX_MOB;
  pReset->arg3	= pRoom->vnum;
  pReset->arg4	= is_number( argument ) ? atoi (argument) : 1;
  add_reset( pRoom, pReset, 0/* Last slot*/ );

  /*
   * Create the mobile.
   */
  newmob = create_mobile( pMobIndex );
  char_to_room( newmob, pRoom );

  sprintf( output, "%s (%d) has been loaded and added to resets.\n\r"
	"There will be a maximum of %d loaded to this room.\n\r",
	capitalize( pMobIndex->short_descr ),
	pMobIndex->vnum,
	pReset->arg2 );
    send_to_char( output, ch );
    act( "$n has created $N!", ch, NULL, newmob, TO_ROOM );
    return TRUE;
}



struct wear_type
{
    int	wear_loc;
    int	wear_bit;
};



const struct wear_type wear_table[] =
{
    {	WEAR_INVENTORY,	ITEM_TAKE		},
    {	WEAR_LIGHT,	ITEM_WEAR_LIGHT		},
    {	WEAR_FINGER_L,	ITEM_WEAR_FINGER	},
    {	WEAR_FINGER_R,	ITEM_WEAR_FINGER	},
    {	WEAR_NECK_1,	ITEM_WEAR_NECK		},
    {	WEAR_NECK_2,	ITEM_WEAR_NECK		},
    {	WEAR_BODY,	ITEM_WEAR_BODY		},
    {	WEAR_HEAD,	ITEM_WEAR_HEAD		},
    {	WEAR_LEGS,	ITEM_WEAR_LEGS		},
    {	WEAR_FEET,	ITEM_WEAR_FEET		},
    {	WEAR_HANDS,	ITEM_WEAR_HANDS		},
    {	WEAR_ARMS,	ITEM_WEAR_ARMS		},
    {	WEAR_SHIELD,	ITEM_WEAR_SHIELD	},
    {	WEAR_ABOUT,	ITEM_WEAR_ABOUT		},
    {	WEAR_WAIST,	ITEM_WEAR_WAIST		},
    {	WEAR_WRIST_L,	ITEM_WEAR_WRIST		},
    {	WEAR_WRIST_R,	ITEM_WEAR_WRIST		},
    {	WEAR_WIELD,	ITEM_WIELD		},
    {	WEAR_HOLD,	ITEM_HOLD		},
    {	WEAR_SECONDARY,	ITEM_WIELD		},
    {	WEAR_BACK,	ITEM_WEAR_BACK		},
    {	NO_FLAG,	NO_FLAG			}
};



/*****************************************************************************
 Name:		wear_loc
 Purpose:	Returns the location of the bit that matches the count.
 		1 = first match, 2 = second match etc.
 Called by:	oedit_reset(olc_act.c).
 ****************************************************************************/
int wear_loc(int bits, int count)
{
    int flag;
 
    for (flag = 0; wear_table[flag].wear_bit != NO_FLAG; flag++)
    {

        if ( IS_SET(bits, wear_table[flag].wear_bit) && --count < 1)
            return wear_table[flag].wear_loc;
    }
 
    return NO_FLAG;
}



/*****************************************************************************
 Name:		wear_bit
 Purpose:	Converts a wear_loc into a bit.
 Called by:	redit_oreset(olc_act.c).
 ****************************************************************************/
int wear_bit(int loc)
{
    int flag;
 
    for (flag = 0; wear_table[flag].wear_loc != NO_FLAG; flag++)
    {
        if ( loc == wear_table[flag].wear_loc )
            return wear_table[flag].wear_bit;
    }
 
    return -1;
}



REDIT( redit_oreset )
{
    ROOM_INDEX_DATA	*pRoom;
    OBJ_INDEX_DATA	*pObjIndex;
    OBJ_DATA		*newobj;
    OBJ_DATA		*to_obj;
    CHAR_DATA		*to_mob;
    char		arg1 [ MAX_INPUT_LENGTH ];
    char		arg2 [ MAX_INPUT_LENGTH ];
    int			olevel = 0;

    RESET_DATA		*pReset;
    char		output [ MAX_STRING_LENGTH ];

    EDIT_ROOM(ch, pRoom);

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || !is_number( arg1 ) )
    {
	send_to_char ( "Syntax:  oreset <vnum> <args>\n\r", ch );
	send_to_char ( "        -no_args               = into room\n\r", ch );
	send_to_char ( "        -<obj_name>            = into obj\n\r", ch );
	send_to_char ( "        -<mob_name> <wear_loc> = into mob\n\r", ch );
	return FALSE;
    }

    if ( !( pObjIndex = get_obj_index( atoi( arg1 ) ) ) )
    {
	send_to_char( "REdit: No object has that vnum.\n\r", ch );
	return FALSE;
    }

    if ( pObjIndex->area != pRoom->area )
    {
	send_to_char( "REdit: No such object in this area.\n\r", ch );
	return FALSE;
    }

    /*
     * Load into room.
     */
    if ( arg2[0] == '\0' )
    {
	pReset		= new_reset_data();
	pReset->command	= 'O';
	pReset->arg1	= pObjIndex->vnum;
	pReset->arg2	= 0;
	pReset->arg3	= pRoom->vnum;
	pReset->arg4	= 0;
	add_reset( pRoom, pReset, 0/* Last slot*/ );

	newobj = create_object( pObjIndex, number_fuzzy( olevel ) );
	obj_to_room( newobj, pRoom );

	sprintf( output, "%s (%d) has been loaded and added to resets.\n\r",
	    capitalize( pObjIndex->short_descr ),
	    pObjIndex->vnum );
	send_to_char( output, ch );
    }
    else
    /*
     * Load into object's inventory.
     */
    if ( argument[0] == '\0'
    && ( ( to_obj = get_obj_list( ch, arg2, pRoom->contents ) ) != NULL ) )
    {
	pReset		= new_reset_data();
	pReset->command	= 'P';
	pReset->arg1	= pObjIndex->vnum;
	pReset->arg2	= 0;
	pReset->arg3	= to_obj->pIndexData->vnum;
	pReset->arg4	= 1;
	add_reset( pRoom, pReset, 0/* Last slot*/ );

	newobj = create_object( pObjIndex, number_fuzzy( olevel ) );
	newobj->cost = 0;
	obj_to_obj( newobj, to_obj );

	sprintf( output, "%s (%d) has been loaded into "
	    "%s (%d) and added to resets.\n\r",
	    capitalize( newobj->short_descr ),
	    newobj->pIndexData->vnum,
	    to_obj->short_descr,
	    to_obj->pIndexData->vnum );
	send_to_char( output, ch );
    }
    else
    /*
     * Load into mobile's inventory.
     */
    if ( ( to_mob = get_char_room( ch, arg2 ) ) != NULL )
    {
	int	wear_loc;

	/*
	 * Make sure the location on mobile is valid.
	 */

	if ( (wear_loc = flag_value( wear_loc_flags, argument )) == NO_FLAG )
	{
          if (str_cmp(argument, "light"))
            {         
	    send_to_char( "REdit: Invalid wear_loc.  '? wear_loc'\n\r", ch );
	    return FALSE;
  	    }
           else
            {
            wear_loc = WEAR_LIGHT;
            } 
       }

	/*
	 * Disallow loading a sword(WEAR_WIELD) into WEAR_HEAD.
	 */
	if ( !IS_SET( pObjIndex->wear_flags, wear_bit(wear_loc) ) )
	{
	    sprintf( output,
	        "%s (%d) has wear flags: [%s]\n\r",
	        capitalize( pObjIndex->short_descr ),
	        pObjIndex->vnum,
		flag_string( wear_flags, pObjIndex->wear_flags ) );
	    send_to_char( output, ch );
	    return FALSE;
	}

	/*
	 * Can't load into same position.
	 */
	if ( get_eq_char( to_mob, wear_loc) )
	{
	    send_to_char( "REdit:  Object already equipped.\n\r", ch );
	    return FALSE;
	}

	pReset		= new_reset_data();
	pReset->arg1	= pObjIndex->vnum;
	pReset->arg2	= wear_loc;
	if ( pReset->arg2 == WEAR_INVENTORY )
	    pReset->command = 'G';
	else
	    pReset->command = 'E';
	pReset->arg3	= wear_loc;

	add_reset( pRoom, pReset, 0/* Last slot*/ );

	olevel  = URANGE( 0, to_mob->level - 2, LEVEL_HERO );
        newobj = create_object( pObjIndex, number_fuzzy( olevel ) );

	if ( to_mob->pIndexData->pShop )	/* Shop-keeper? */
	{
	    switch ( pObjIndex->item_type )
	    {
	    default:		olevel = 0;				break;
	    case ITEM_PILL:	olevel = number_range(  0, 10 );	break;
	    case ITEM_POTION:	olevel = number_range(  0, 10 );	break;
	    case ITEM_SCROLL:	olevel = number_range(  5, 15 );	break;
	    case ITEM_WAND:	olevel = number_range( 10, 20 );	break;
	    case ITEM_STAFF:	olevel = number_range( 15, 25 );	break;
	    case ITEM_ARMOR:	olevel = number_range(  5, 15 );	break;
	    case ITEM_WEAPON:	if ( pReset->command == 'G' )
	    			    olevel = number_range( 5, 15 );
				else
				    olevel = number_fuzzy( olevel );
		break;
	    }

	    newobj = create_object( pObjIndex, olevel );
	    if ( pReset->arg2 == WEAR_INVENTORY )
		SET_BIT( newobj->extra_flags, ITEM_INVENTORY );
	}
	else
	    newobj = create_object( pObjIndex, number_fuzzy( olevel ) );

	obj_to_char( newobj, to_mob );
	if ( pReset->command == 'E' )
	    equip_char( to_mob, newobj, pReset->arg3 );

	sprintf( output, "%s (%d) has been loaded "
	    "%s of %s (%d) and added to resets.\n\r",
	    capitalize( pObjIndex->short_descr ),
	    pObjIndex->vnum,
	    flag_string( wear_loc_strings, pReset->arg3 ),
	    to_mob->short_descr,
	    to_mob->pIndexData->vnum );
	send_to_char( output, ch );
    }
    else	/* Display Syntax */
    {
	send_to_char( "REdit:  That mobile isn't here.\n\r", ch );
	return FALSE;
    }

    act( "$n has created $p!", ch, newobj, NULL, TO_ROOM );
    return TRUE;
}



/*
 * Object Editor Functions.
 */
void show_obj_values( CHAR_DATA *ch, OBJ_INDEX_DATA *obj )
{
    char buf[MAX_STRING_LENGTH];

    switch( obj->item_type )
    {
	default:	/* No values. */
	    break;
            
	case ITEM_LIGHT_SOURCE:
            if ( obj->value[2] == -1 || obj->value[2] == 999 ) /* ROM OLC */
		sprintf( buf, "{c[{C v2 {c] {CLight{w:  {WInfinite{D[{C-1{D]\n\r" );
            else
		sprintf( buf, "{c[{C v2 {c] {CLight{w:  {D[{W%d{D]\n\r", obj->value[2] );
	    send_to_char( buf, ch );
	    break;

	case ITEM_WAND:
	case ITEM_STAFF:
            sprintf( buf,
		"{c[{C v0 {c] {CLevel{w:          {D[{W%d{D]\n\r"
		"{c[{C v1 {c] {CCharges Total{w:  {D[{W%d{D]\n\r"
		"{c[{C v2 {c] {CCharges Left{w:   {D[{W%d{D]\n\r"
		"{c[{C v3 {c] {CSpell{w:          {W%s\n\r",
		obj->value[0],
		obj->value[1],
		obj->value[2],
		obj->value[3] != -1 ? capitalize(skill_table[obj->value[3]].name)
		                    : "NONE" );
	    send_to_char( buf, ch );
	    break;

	case ITEM_PORTAL:
	    sprintf( buf,
	        "{c[{C v0 {c] {CCharges{w:        {D[{W%d}D]\n\r"
	        "{c[{C v1 {c] {CExit Flags{w:     {W%s\n\r"
	        "{c[{C v2 {c] {CPortal Flags{w:   {W%s\n\r"
	        "{c[{C v3 {c] {CGoes to {c({GVNUM{c){w: {D[{W%d{D]\n\r"
		"{c[{C v4 {c] {CKey {c({GVNUM{c)    {w: {D[{W%d{D]\n\r",
	        obj->value[0],
	        capitalize(flag_string( exit_flags, obj->value[1])),
	        capitalize(flag_string( portal_flags , obj->value[2])),
	        obj->value[3], obj->value[4]);
	    send_to_char( buf, ch);
	    break;
  case ITEM_SEGMENT:
	sprintf( buf,
		"{c[{C v0 {c] {CCreated Object{w: {D[{W%d{D]\n\r"
		"{c[{C v1 {c] {CJoinable Object{w:{D[{W%d{D]\n\r"
                "{c[{C v2 {c] {CJoinable Object{w:{D[{W%d{D]\n\r"
                "{c[{C v3 {c] {CJoinable Object{w:{D[{W%d{D]\n\r"
                "{c[{C v4 {c] {CJoinable Object{w:{D[{W%d{D]\n\r",
		obj->value[0], obj->value[1], obj->value[2], obj->value[3], obj->value[4]);
		send_to_char(buf, ch);
		break;
	
    
	case ITEM_FURNITURE:          
	    sprintf( buf,
	        "{c[{C v0 {c] {CMax People{w:      {D[{W%d{D]\n\r"
	        "{c[{C v1 {c] {CMax Weight{w:      {D[{W%d{D]\n\r"
	        "{c[{C v2 {c] {CFurniture Flags{w: {W%s\n\r"
	        "{c[{C v3 {c] {CHeal Bonus{w:      {D[{W%d{D]\n\r"
	        "{c[{C v4 {c] {CMana Bonus{w:      {D[{W%d{D]\n\r",
	        obj->value[0],
	        obj->value[1],
	        capitalize(flag_string( furniture_flags, obj->value[2])),
	        obj->value[3],
	        obj->value[4] );
	    send_to_char( buf, ch );
	    break;

	case ITEM_SCROLL:
	case ITEM_POTION:
	case ITEM_PILL:
            sprintf( buf,
		"{c[{C v0 {c] {CLevel{w:  {D[{W%d{D]\n\r"
		"{c[{C v1 {c] {CSpell{w:  {W%s\n\r"
		"{c[{C v2 {c] {CSpell{w:  {W%s\n\r"
		"{c[{C v3 {c] {CSpell{w:  {W%s\n\r"
		"{c[{C v4 {c] {CSpell{w:  {W%s\n\r",
		obj->value[0],
		obj->value[1] != -1 ? capitalize(skill_table[obj->value[1]].name)
		                    : "NONE",
		obj->value[2] != -1 ? capitalize(skill_table[obj->value[2]].name)
                                    : "NONE",
		obj->value[3] != -1 ? capitalize(skill_table[obj->value[3]].name)
		                    : "NONE",
		obj->value[4] != -1 ? capitalize(skill_table[obj->value[4]].name)
		                    : "NONE" );
	    send_to_char( buf, ch );
	    break;

/* ARMOR for ROM */

        case ITEM_ARMOR:
	    sprintf( buf,
		"{c[{C v0 {c] {CAc Pierce{w:       {D[{W%d{D]\n\r"
		"{c[{C v1 {c] {CAc Bash{w:         {D[{W%d{D]\n\r"
		"{c[{C v2 {c] {CAc Slash{w:        {D[{W%d{D]\n\r"
		"{c[{C v3 {c] {CAc Exotic{w:       {D[{W%d{D]\n\r",
		obj->value[0],
		obj->value[1],
		obj->value[2],
		obj->value[3] );
	    send_to_char( buf, ch );
	    break;

/* WEAPON changed in ROM: */
/* I had to split the output here, I have no idea why, but it helped -- Hugin */
/* It somehow fixed a bug in showing scroll/pill/potions too ?!             */
/* It's pretty friggin' simple.  flag_string is static.                       */
	case ITEM_WEAPON:
            sprintf( buf, "{c[ {Cv0 {c] {CWeapon Class{w:   {W%s\n\r",
		     capitalize(flag_string( weapon_class, obj->value[0] ) ));
	    send_to_char( buf, ch );
	    sprintf( buf, "{c[{C v1 {c] {CNumber of Dice{w: {D[{W%d{D]\n\r", obj->value[1] );
	    send_to_char( buf, ch );
	    sprintf( buf, "{c[{C v2 {c] {CType of Dice{w:   {D[{W%d{D]\n\r", obj->value[2] );
	    send_to_char( buf, ch );
	    sprintf( buf, "{c[{C v3 {c] {CType{w:           {W%s\n\r",
		    capitalize(attack_table[obj->value[3]].name) );
	    send_to_char( buf, ch );
 	    sprintf( buf, "{c[{C v4 {c] {CSpecial Type{w:   {W%s\n\r",
		     capitalize(flag_string( weapon_type2,  obj->value[4]) ) );
	    send_to_char( buf, ch );
	    break;

	case ITEM_CONTAINER:
	    sprintf( buf,
		"{c[{C v0 {c] {CWeight{w:      {D[{W%d {ckg{D]\n\r"
		"{c[{C v1 {c] {CFlags{w:       {W%s\n\r"
		"{c[{C v2 {c] {CKey{w:         {W%s {D[{W%d{D]\n\r"
		"{c[{C v3 {c] {CCapacity{w:    {D[{W%d{D]\n\r"
		"{c[{C v4 {c] {CWeight Mult{w: {D[{W%d{D]\n\r",
		obj->value[0],
		capitalize(flag_string( container_flags, obj->value[1]) ),
                get_obj_index(obj->value[2])
                    ? capitalize(strip_color(get_obj_index(obj->value[2])->short_descr))
                    : "NONE",
                obj->value[2],
                obj->value[3],
                obj->value[4] );
	    send_to_char( buf, ch );
	    break;

	case ITEM_DRINK_CON:
	    sprintf( buf,
	        "{c[{C v0 {c] {CLiquid Total{w: {D[{W%d{D]\n\r"
	        "{c[{C v1 {c] {CLiquid Left{w:  {D[{W%d{D]\n\r"
	        "{c[{C v2 {c] {CLiquid{w:       {W%s\n\r"
	        "{c[{C v3 {c] {CPoisoned{w:     {W%s\n\r",
	        obj->value[0],
	        obj->value[1],
	        capitalize(liq_table[obj->value[2]].liq_name),
	        obj->value[3] != 0 ? "{RYES" : "{GNO" );
	    send_to_char( buf, ch );
	    break;

	case ITEM_FOUNTAIN:
	    sprintf( buf,
	        "{c[{C v0 {c] {CLiquid Total{w: {D[{W%d{D]\n\r"
	        "{c[{C v1 {c] {CLiquid Left{w:  {D[{W%d{D]\n\r"
	        "{c[{C v2 {c] {CLiquid{w:	    {W%s\n\r",
	        obj->value[0],
	        obj->value[1],
	        capitalize(liq_table[obj->value[2]].liq_name) );
	    send_to_char( buf,ch );
	    break;
	        
	case ITEM_FOOD:
	    sprintf( buf,
		"{c[{C v0 {c] {CFood hours{w: {D[{W%d{D]\n\r"
		"{c[{C v1 {c] {CFull hours{w: {D[{W%d{D]\n\r"
		"{c[{C v3 {c] {CPoisoned{w:   %s\n\r",
		obj->value[0],
		obj->value[1],
		obj->value[3] != 0 ? "{RYES" : "{GNO" );
	    send_to_char( buf, ch );
	    break;

	case ITEM_MONEY:
            sprintf( buf, "{c[{C v0 {c] {YGOLD{w:   {D[{W%d{D]\n\r", obj->value[0] );
	    send_to_char( buf, ch );
	    break;
    }

    send_to_char("{x",ch);
    return;
}



bool set_obj_values( CHAR_DATA *ch, OBJ_INDEX_DATA *pObj, int value_num, char *argument)
{
    switch( pObj->item_type )
    {
        default:
            break;
            
        case ITEM_LIGHT_SOURCE:
	    switch ( value_num )
	    {
	        default:
		    do_help( ch, "ITEM_LIGHT" );
	            return FALSE;
	        case 2:
	            send_to_char( "HOURS OF LIGHT SET.\n\r\n\r", ch );
	            pObj->value[2] = atoi( argument );
	            break;
	    }
            break;


        case ITEM_WAND:
        case ITEM_STAFF:
	    switch ( value_num )
	    {
	        default:
		    do_help( ch, "ITEM_STAFF_WAND" );
	            return FALSE;
	        case 0:
	            send_to_char( "SPELL LEVEL SET.\n\r\n\r", ch );
	            pObj->value[0] = atoi( argument );
	            break;
	        case 1:
	            send_to_char( "TOTAL NUMBER OF CHARGES SET.\n\r\n\r", ch );
	            pObj->value[1] = atoi( argument );
	            break;
	        case 2:
	            send_to_char( "CURRENT NUMBER OF CHARGES SET.\n\r\n\r", ch );
	            pObj->value[2] = atoi( argument );
	            break;
	        case 3:
	            send_to_char( "SPELL TYPE SET.\n\r", ch );
	            pObj->value[3] = skill_lookup( argument );
	            break;
	    }
            break;

        case ITEM_SCROLL:
        case ITEM_POTION:
        case ITEM_PILL:
	    switch ( value_num )
	    {
	        default:
		    do_help( ch, "ITEM_SCROLL_POTION_PILL" );
	            return FALSE;
	        case 0:
	            send_to_char( "SPELL LEVEL SET.\n\r\n\r", ch );
	            pObj->value[0] = atoi( argument );
	            break;
	        case 1:
	            send_to_char( "SPELL TYPE 1 SET.\n\r\n\r", ch );
	            pObj->value[1] = skill_lookup( argument );
	            break;
	        case 2:
	            send_to_char( "SPELL TYPE 2 SET.\n\r\n\r", ch );
	            pObj->value[2] = skill_lookup( argument );
	            break;
	        case 3:
	            send_to_char( "SPELL TYPE 3 SET.\n\r\n\r", ch );
	            pObj->value[3] = skill_lookup( argument );
	            break;
	        case 4:
	            send_to_char( "SPELL TYPE 4 SET.\n\r\n\r", ch );
	            pObj->value[4] = skill_lookup( argument );
	            break;
 	    }
	    break;

/* ARMOR for ROM: */

        case ITEM_ARMOR:
	    switch ( value_num )
	    {
	        default:
		    do_help( ch, "ITEM_ARMOR" );
		    return FALSE;
	        case 0:
		    send_to_char( "AC PIERCE SET.\n\r\n\r", ch );
		    pObj->value[0] = atoi( argument );
		    break;
	        case 1:
		    send_to_char( "AC BASH SET.\n\r\n\r", ch );
		    pObj->value[1] = atoi( argument );
		    break;
	        case 2:
		    send_to_char( "AC SLASH SET.\n\r\n\r", ch );
		    pObj->value[2] = atoi( argument );
		    break;
	        case 3:
		    send_to_char( "AC EXOTIC SET.\n\r\n\r", ch );
		    pObj->value[3] = atoi( argument );
		    break;
	    }
	    break;

/* WEAPONS changed in ROM */

        case ITEM_WEAPON:
	    switch ( value_num )
	    {
	        default:
		    do_help( ch, "ITEM_WEAPON" );
	            return FALSE;
	        case 0:
		    send_to_char( "WEAPON CLASS SET.\n\r\n\r", ch );
		    pObj->value[0] = flag_value( weapon_class, argument );
		    break;
	        case 1:
	            send_to_char( "NUMBER OF DICE SET.\n\r\n\r", ch );
	            pObj->value[1] = atoi( argument );
	            break;
	        case 2:
	            send_to_char( "TYPE OF DICE SET.\n\r\n\r", ch );
	            pObj->value[2] = atoi( argument );
	            break;
	        case 3:
	            send_to_char( "WEAPON TYPE SET.\n\r\n\r", ch );
	            pObj->value[3] = attack_lookup( argument );
	            break;
	        case 4:
                    send_to_char( "SPECIAL WEAPON TYPE TOGGLED.\n\r\n\r", ch );
		    pObj->value[4] ^= (flag_value( weapon_type2, argument ) != NO_FLAG
		    ? flag_value( weapon_type2, argument ) : 0 );
		    break;
	    }
            break;

	case ITEM_PORTAL:
	    switch ( value_num )
	    {
	        default:
	            do_help(ch, "ITEM_PORTAL" );
	            return FALSE;
	            
	    	case 0:
	    	    send_to_char( "CHARGES SET.\n\r\n\r", ch);
	    	    pObj->value[0] = atoi ( argument );
	    	    break;
	    	case 1:
	    	    send_to_char( "EXIT FLAGS SET.\n\r\n\r", ch);
	    	    pObj->value[1] = flag_value( exit_flags, argument );
	    	    break;
	    	case 2:
	    	    send_to_char( "PORTAL FLAGS SET.\n\r\n\r", ch);
	    	    pObj->value[2] = flag_value( portal_flags, argument );
	    	    break;
	    	case 3:
	    	    send_to_char( "EXIT VNUM SET.\n\r\n\r", ch);
	    	    pObj->value[3] = atoi ( argument );
	    	    break;
		case 4:
	    	    send_to_char( "PORTAL KEY VNUM SET.\n\r\n\r", ch );
	    	    pObj->value[4] = atoi( argument );
	    	    break;
	   }
	   break;
	case ITEM_SEGMENT:	
		switch(value_num)
	{
		default:
			do_help(ch, "ITEM_SEGMENT");
			return FALSE;
		case 0:
			send_to_char( "OBJECT TO BE CREATED SET.\n\r\n\r", ch);
			pObj->value[0] = atoi ( argument );
			break;
		case 1:
		case 2:
		case 3:
		case 4:
			send_to_char( "JOINABLE OBJECT SET.\n\r\n\r", ch);
			pObj->value[value_num] = atoi ( argument );
			break;
	}

	case ITEM_FURNITURE:
	    switch ( value_num )
	    {
	        default:
	            do_help( ch, "ITEM_FURNITURE" );
	            return FALSE;
	            
	        case 0:
	            send_to_char( "NUMBER OF PEOPLE SET.\n\r\n\r", ch);
	            pObj->value[0] = atoi ( argument );
	            break;
	        case 1:
	            send_to_char( "MAX WEIGHT SET.\n\r\n\r", ch);
	            pObj->value[1] = atoi ( argument );
	            break;
	        case 2:
	            send_to_char( "FURNITURE FLAGS TOGGLED.\n\r\n\r", ch);
	            pObj->value[2] ^= (flag_value( furniture_flags, argument ) != NO_FLAG
	            ? flag_value( furniture_flags, argument ) : 0);
	            break;
	        case 3:
	            send_to_char( "HEAL BONUS SET.\n\r\n\r", ch);
	            pObj->value[3] = atoi ( argument );
	            break;
	        case 4:
	            send_to_char( "MANA BONUS SET.\n\r\n\r", ch);
	            pObj->value[4] = atoi ( argument );
	            break;
	    }
	    break;
	   
        case ITEM_CONTAINER:
	    switch ( value_num )
	    {
		int value;
		
		default:
		    do_help( ch, "ITEM_CONTAINER" );
	            return FALSE;
		case 0:
	            send_to_char( "WEIGHT CAPACITY SET.\n\r\n\r", ch );
	            pObj->value[0] = atoi( argument );
	            break;
		case 1:
	            if ( ( value = flag_value( container_flags, argument ) )
	              != NO_FLAG )
	        	TOGGLE_BIT(pObj->value[1], value);
		    else
		    {
			do_help ( ch, "ITEM_CONTAINER" );
			return FALSE;
		    }
	            send_to_char( "CONTAINER TYPE SET.\n\r\n\r", ch );
	            break;
		case 2:
		    if ( atoi(argument) != 0 )
		    {
			if ( !get_obj_index( atoi( argument ) ) )
			{
			    send_to_char( "THERE IS NO SUCH ITEM.\n\r\n\r", ch );
			    return FALSE;
			}

			if ( get_obj_index( atoi( argument ) )->item_type != ITEM_KEY )
			{
			    send_to_char( "THAT ITEM IS NOT A KEY.\n\r\n\r", ch );
			    return FALSE;
			}
		    }
		    send_to_char( "CONTAINER KEY SET.\n\r\n\r", ch );
		    pObj->value[2] = atoi( argument );
		    break;
		case 3:
		    send_to_char( "CONTAINER MAX WEIGHT SET.\n\r", ch);
		    pObj->value[3] = atoi( argument );
		    break;
		case 4:
		    send_to_char( "WEIGHT MULTIPLIER SET.\n\r\n\r", ch );
		    pObj->value[4] = atoi ( argument );
		    break;
	    }
	    break;

	case ITEM_DRINK_CON:
	    switch ( value_num )
	    {
	        default:
		    do_help( ch, "ITEM_DRINK" );
/* OLC		    do_help( ch, "liquids" );    */
	            return FALSE;
	        case 0:
	            send_to_char( "MAXIMUM AMOUT OF LIQUID HOURS SET.\n\r\n\r", ch );
	            pObj->value[0] = atoi( argument );
	            break;
	        case 1:
	            send_to_char( "CURRENT AMOUNT OF LIQUID HOURS SET.\n\r\n\r", ch );
	            pObj->value[1] = atoi( argument );
	            break;
	        case 2:
	            send_to_char( "LIQUID TYPE SET.\n\r\n\r", ch );
	            pObj->value[2] = ( liq_lookup(argument) != -1 ?
	            		       liq_lookup(argument) : 0 );
	            break;
	        case 3:
	            send_to_char( "POISON VALUE TOGGLED.\n\r\n\r", ch );
	            pObj->value[3] = ( pObj->value[3] == 0 ) ? 1 : 0;
	            break;
	    }
            break;

	case ITEM_FOUNTAIN:
	    switch (value_num)
	    {
	    	default:
		    do_help( ch, "ITEM_FOUNTAIN" );
/* OLC		    do_help( ch, "liquids" );    */
	            return FALSE;
	        case 0:
	            send_to_char( "MAXIMUM AMOUT OF LIQUID HOURS SET.\n\r\n\r", ch );
	            pObj->value[0] = atoi( argument );
	            break;
	        case 1:
	            send_to_char( "CURRENT AMOUNT OF LIQUID HOURS SET.\n\r\n\r", ch );
	            pObj->value[1] = atoi( argument );
	            break;
	        case 2:
	            send_to_char( "LIQUID TYPE SET.\n\r\n\r", ch );
	            pObj->value[2] = ( liq_lookup( argument ) != -1 ?
	            		       liq_lookup( argument ) : 0 );
	            break;
            }
	break;
		    	
	case ITEM_FOOD:
	    switch ( value_num )
	    {
	        default:
		    do_help( ch, "ITEM_FOOD" );
	            return FALSE;
	        case 0:
	            send_to_char( "HOURS OF FOOD SET.\n\r\n\r", ch );
	            pObj->value[0] = atoi( argument );
	            break;
	        case 1:
	            send_to_char( "HOURS OF FULL SET.\n\r\n\r", ch );
	            pObj->value[1] = atoi( argument );
	            break;
	        case 3:
	            send_to_char( "POISON VALUE TOGGLED.\n\r\n\r", ch );
	            pObj->value[3] = ( pObj->value[3] == 0 ) ? 1 : 0;
	            break;
	    }
            break;

	case ITEM_MONEY:
	    switch ( value_num )
	    {
	        default:
		    do_help( ch, "ITEM_MONEY" );
	            return FALSE;
	        case 0:
	            send_to_char( "GOLD AMOUNT SET.\n\r\n\r", ch );
	            pObj->value[0] = atoi( argument );
	            break;
		case 1:
		    send_to_char( "SILVER AMOUNT SET.\n\r\n\r", ch );
		    pObj->value[1] = atoi( argument );
		    break;
	    }
            break;
    }

    show_obj_values( ch, pObj );

    return TRUE;
}



OEDIT( oedit_show )
{
    OBJ_INDEX_DATA *pObj;
    char buf[MAX_STRING_LENGTH];
    AFFECT_DATA *paf;
    int cnt;

    EDIT_OBJ(ch, pObj);

    sprintf( buf, "\n\r{cName{w:        {D[{W%s{D]\n\r{cArea{w:        {D[{W%5d{D] {C%s\n\r",
	pObj->name,
	!pObj->area ? -1        : pObj->area->vnum,
	!pObj->area ? "No Area" : pObj->area->name );
    send_to_char( buf, ch );


    sprintf( buf, "{cVnum{w:        {D[{W%5d{D]\n\r{cType{w:        {D[{W%s{D]\n\r",
	pObj->vnum,
	flag_string( type_flags, pObj->item_type ) );
    send_to_char( buf, ch );

    sprintf( buf, "{cLevel{w:       {D[{W%5d{D]\n\r", pObj->level );
    send_to_char( buf, ch );

    sprintf( buf, "{cWear flags{w:  {D[{W%s{D]\n\r",
	flag_string( wear_flags, pObj->wear_flags ) );
    send_to_char( buf, ch );

    sprintf( buf, "{cExtra flags{w: {D[{W%s{D]\n\r",
	flag_string( extra_flags, pObj->extra_flags ) );
    send_to_char( buf, ch );

    sprintf( buf, "{cMaterial{w:    {D[{W%s{D]\n\r",               
	flag_string( material_flags, pObj->material_type ) ); 

    send_to_char( buf, ch );

    sprintf( buf, "{cCondition{w:   {D[{W%5d{D]\n\r",
	pObj->condition );
    send_to_char( buf, ch );

    sprintf( buf, "{cWeight{w:      {D[{W%5d{D]\n\r{cCost{w:        {D[{W%5d{D]\n\r",
	pObj->weight, pObj->cost );
    send_to_char( buf, ch );

    if ( pObj->extra_descr )
    {
	EXTRA_DESCR_DATA *ed;

	send_to_char( "{cExtra DESC{w: ", ch );

	for ( ed = pObj->extra_descr; ed; ed = ed->next )
	{
	    send_to_char( "{D[{W", ch );
	    send_to_char( ed->keyword, ch );
	    send_to_char( "{D]", ch );
	}

	send_to_char( "\n\r", ch );
    }

    sprintf( buf, "{cS_DESC{w:  {W%s\n\r", capitalize(strip_color(pObj->short_descr)));
    send_to_char( buf, ch );

    sprintf( buf, "{cL_DESC{w:  {W%s\n\r", capitalize(strip_color(pObj->description)));
    send_to_char( buf, ch );

    for ( cnt = 0, paf = pObj->affected; paf; paf = paf->next )
    {
	if ( cnt == 0 )
	{
	    send_to_char( "\n\r{CNumber Modifier Affects\n\r", ch );
	    send_to_char( "{c------ -------- -------\n\r", ch );
	}
	sprintf( buf, "{D[{W%4d{D] {C%-8d {c%s\n\r", cnt,
	    paf->modifier,
	    flag_string( apply_flags, paf->location ) );
	send_to_char( buf, ch );
	cnt++;
    }

    send_to_char("{x\n\r",ch);
    show_obj_values( ch, pObj );

    return FALSE;
}

/*
 * oedit_copy function thanks to Zanthras of Mystical Realities MUD.
 */
OEDIT( oedit_copy )
{
    OBJ_INDEX_DATA *pObj;
    OBJ_INDEX_DATA *pObj2; /* The object to copy */
    int vnum, i;

    if ( argument[0] == '\0' )
    {
      send_to_char("Syntax: copy <vnum> \n\r",ch);
      return FALSE;
    }

    if ( !is_number(argument) )
    {
      send_to_char("OEdit: You must enter a number (vnum).\n\r",ch);
      return FALSE;
    }
    else /* argument is a number */
    {
      vnum = atoi(argument);
      if( !( pObj2 = get_obj_index(vnum) ) )
      {
	send_to_char("OEdit: That object does not exist.\n\r",ch);
	return FALSE;
      }
    }

    EDIT_OBJ(ch, pObj);

    free_string( pObj->name );
    pObj->name = str_dup( pObj2->name );

    pObj->material_type = pObj2->material_type;

    pObj->item_type = pObj2->item_type;

    pObj->level = pObj2->level;

    pObj->wear_flags  = pObj2->wear_flags;
    pObj->extra_flags = pObj2->extra_flags;

    
    pObj->condition = pObj2->condition;

    pObj->weight = pObj2->weight;
    pObj->cost   = pObj2->cost;

    pObj->extra_descr = pObj2->extra_descr;

    free_string( pObj->short_descr );
    pObj->short_descr = str_dup( pObj2->short_descr );

    free_string( pObj->description );
    pObj->description = str_dup( pObj2->description );

    pObj->affected = pObj2->affected;

    for (i = 0; i < 5; i++)
    {
      pObj->value[i] = pObj2->value[i];
    }

    send_to_char( "Object info copied.", ch );
    return TRUE;
}

/*
 * medit_copy function thanks to Zanthras of Mystical Realities MUD.
 * Thanks to Ivan for what there is of the incomplete mobprog part.
 * Hopefully it will be finished in a later release of this snippet.
 */
MEDIT( medit_copy )
{
    MOB_INDEX_DATA *pMob;
    MOB_INDEX_DATA *pMob2; /* The mob to copy */
    int vnum;

    /* MPROG_LIST *list; */ /* Used for the mob prog for loop */

    if ( argument[0] == '\0' )
    {
      send_to_char("Syntax: copy <vnum> \n\r",ch);
      return FALSE;
    }

    if ( !is_number(argument) )
    {
      send_to_char("MEdit: You must enter a number (vnum).\n\r",ch);
      return FALSE;
    }
    else /* argument is a number */
    {
      vnum = atoi(argument);
      if( !( pMob2 = get_mob_index(vnum) ) )
      {
	send_to_char("MEdit: That mob does not exist.\n\r",ch);
	return FALSE;
      }
    }

    EDIT_MOB(ch, pMob);

    free_string( pMob->player_name );
    pMob->player_name = str_dup( pMob2->player_name );

    pMob->new_format = pMob2->new_format;
    pMob->act = pMob2->act;
    pMob->sex = pMob2->sex;
 
    pMob->race = pMob2->race;

    pMob->level = pMob2->level;
    
    pMob->alignment = pMob2->alignment;
    
    pMob->hitroll = pMob2->hitroll;
    
    pMob->dam_type = pMob2->dam_type;

    pMob->group = pMob2->group;

    pMob->hit[DICE_NUMBER] = pMob2->hit[DICE_NUMBER];
    pMob->hit[DICE_TYPE]   = pMob2->hit[DICE_TYPE];
    pMob->hit[DICE_BONUS]  = pMob2->hit[DICE_BONUS];

    pMob->damage[DICE_NUMBER] = pMob2->damage[DICE_NUMBER];
    pMob->damage[DICE_TYPE]   = pMob2->damage[DICE_TYPE];
    pMob->damage[DICE_BONUS]  = pMob2->damage[DICE_BONUS];
    
    pMob->mana[DICE_NUMBER] = pMob2->mana[DICE_NUMBER];
    pMob->mana[DICE_TYPE]   = pMob2->mana[DICE_TYPE];
    pMob->mana[DICE_BONUS]  = pMob2->mana[DICE_BONUS];

    pMob->affected_by = pMob2->affected_by;
    pMob->affected2_by = pMob2->affected2_by;
    
    pMob->ac[AC_PIERCE] = pMob2->ac[AC_PIERCE];
    pMob->ac[AC_BASH]   = pMob2->ac[AC_BASH];
    pMob->ac[AC_SLASH]  = pMob2->ac[AC_SLASH];
    pMob->ac[AC_EXOTIC] = pMob2->ac[AC_EXOTIC];
    

    pMob->form  = pMob2->form;
    pMob->parts = pMob2->parts;

    pMob->imm_flags  = pMob2->imm_flags;
    pMob->res_flags  = pMob2->res_flags;
    pMob->vuln_flags = pMob2->vuln_flags;
    pMob->off_flags  = pMob2->off_flags;

    pMob->size     = pMob2->size;

    free_string( pMob->material );
    pMob->material = str_dup( pMob2->material ); 

    pMob->start_pos   = pMob2->start_pos;
    pMob->default_pos = pMob2->default_pos;

    pMob->wealth = pMob2->wealth;

    pMob->spec_fun = pMob2->spec_fun;

    free_string( pMob->short_descr );
    pMob->short_descr = str_dup( pMob2->short_descr );

    free_string( pMob->long_descr );
    pMob->long_descr = str_dup( pMob2->long_descr   );

    free_string( pMob->description );
    pMob->description = str_dup( pMob2->description );

    /* Hopefully get the shop data to copy later
     * This is the fields here if you get it copying send me
     * a copy and I'll add it to the snippet with credit to
     * you of course :) same with the mobprogs for loop :)
     */

/*
    if ( pMob->pShop )
    {
	SHOP_DATA *pShop, *pShop2;

	pShop =  pMob->pShop;
	pShop2 = pMob2->pShop;
 
	pShop->profit_buy = pShop2->profit_buy;
	pShop->profit_sell = pShop2->profit_sell;
	
	pShop->open_hour = pShop2->open_hour;
	pShop->close_hour = pShop2->close_hour;
	
	pShop->buy_type[iTrade] = pShop2->buy_type[iTrade];
    }
*/
/*  for loop thanks to Ivan, still needs work though

    for (list = pMob->mprogs; list; list = list->next )
    {
      MPROG_LIST *newp = new_mprog();
      newp->trig_type = list->trig_type;

      free_string( newp->trig_phrase );
      newp->trig_phrase = str_dup( list->trig_phrase );

      newp->vnum = list->vnum;

      free_string( newp->code )
      newp->code = str_dup( list->code );

      pMob->mprogs = newp;
    }
*/

    send_to_char( "Mob info copied.", ch );
    return FALSE;
}


/*
 * Need to issue warning if flag isn't valid. -- does so now -- Hugin.
 */
OEDIT( oedit_addaffect )
{
    int value;
    OBJ_INDEX_DATA *pObj;
    AFFECT_DATA *pAf;
    char loc[MAX_STRING_LENGTH];
    char mod[MAX_STRING_LENGTH];

    EDIT_OBJ(ch, pObj);

    argument = one_argument( argument, loc );
    one_argument( argument, mod );

    if ( loc[0] == '\0' || mod[0] == '\0' || !is_number( mod ) )
    {
	send_to_char( "Syntax:  addaffect [location] [#xmod]\n\r", ch );
	return FALSE;
    }

    if ( ( value = flag_value( apply_flags, loc ) ) == NO_FLAG ) /* Hugin */
    {
        send_to_char( "Valid affects are:\n\r", ch );
	show_help( ch, "apply" );
	return FALSE;
    }

    pAf             =   new_affect();
    pAf->location   =   value;
    pAf->modifier   =   atoi( mod );
    pAf->where	    =   TO_OBJECT;
    pAf->type       =   -1;
    pAf->duration   =   -1;
    pAf->bitvector  =   0;
    pAf->level      =	pObj->level;
    pAf->next       =   pObj->affected;
    pObj->affected  =   pAf;

    send_to_char( "Affect added.\n\r", ch);
    return TRUE;
}

OEDIT( oedit_addapply )
{
    int value,bv,typ;
    OBJ_INDEX_DATA *pObj;
    AFFECT_DATA *pAf;
    char loc[MAX_STRING_LENGTH];
    char mod[MAX_STRING_LENGTH];
    char type[MAX_STRING_LENGTH];
    char bvector[MAX_STRING_LENGTH];

    EDIT_OBJ(ch, pObj);

    argument = one_argument( argument, type );
    argument = one_argument( argument, loc );
    argument = one_argument( argument, mod );
    one_argument( argument, bvector );

    if ( type[0] == '\0' || ( typ = flag_value( apply_types, type ) ) == NO_FLAG )
    {
    	send_to_char( "Invalid apply type. Valid apply types are:\n\r", ch);
    	show_help( ch, "apptype" );
    	return FALSE;
    }

    if ( loc[0] == '\0' || ( value = flag_value( apply_flags, loc ) ) == NO_FLAG )
    {
        send_to_char( "Valid applys are:\n\r", ch );
	show_help( ch, "apply" );
	return FALSE;
    }

    if ( bvector[0] == '\0' || ( bv = flag_value( bitvector_type[typ].table, bvector ) ) == NO_FLAG )
    {
    	send_to_char( "Invalid bitvector type.\n\r", ch );
	send_to_char( "Valid bitvector types are:\n\r", ch );
	show_help( ch, bitvector_type[typ].help );
    	return FALSE;
    }

    if ( mod[0] == '\0' || !is_number( mod ) )
    {
	send_to_char( "Syntax:  addapply [type] [location] [#xmod] [bitvector]\n\r", ch );
	return FALSE;
    }

    pAf             =   new_affect();
    pAf->location   =   value;
    pAf->modifier   =   atoi( mod );
    pAf->where	    =   apply_types[typ].bit;
    pAf->type	    =	-1;
    pAf->duration   =   -1;
    pAf->bitvector  =   bv;
    pAf->level      =	pObj->level;
    pAf->next       =   pObj->affected;
    pObj->affected  =   pAf;

    send_to_char( "Apply added.\n\r", ch);
    return TRUE;
}

/*
 * My thanks to Hans Hvidsten Birkeland and Noam Krendel(Walker)
 * for really teaching me how to manipulate pointers.
 */
OEDIT( oedit_delaffect )
{
    OBJ_INDEX_DATA *pObj;
    AFFECT_DATA *pAf;
    AFFECT_DATA *pAf_next;
    char affect[MAX_STRING_LENGTH];
    int  value;
    int  cnt = 0;

    EDIT_OBJ(ch, pObj);

    one_argument( argument, affect );

    if ( !is_number( affect ) || affect[0] == '\0' )
    {
	send_to_char( "Syntax:  delaffect [#xaffect]\n\r", ch );
	return FALSE;
    }

    value = atoi( affect );

    if ( value < 0 )
    {
	send_to_char( "Only non-negative affect-numbers allowed.\n\r", ch );
	return FALSE;
    }

    if ( !( pAf = pObj->affected ) )
    {
	send_to_char( "OEdit:  Non-existant affect.\n\r", ch );
	return FALSE;
    }

    if( value == 0 )	/* First case: Remove first affect */
    {
	pAf = pObj->affected;
	pObj->affected = pAf->next;
	free_affect( pAf );
    }
    else		/* Affect to remove is not the first */
    {
	while ( ( pAf_next = pAf->next ) && ( ++cnt < value ) )
	     pAf = pAf_next;

	if( pAf_next )		/* See if it's the next affect */
	{
	    pAf->next = pAf_next->next;
	    free_affect( pAf_next );
	}
	else                                 /* Doesn't exist */
	{
	     send_to_char( "No such affect.\n\r", ch );
	     return FALSE;
	}
    }

    send_to_char( "Affect removed.\n\r", ch);
    return TRUE;
}



OEDIT( oedit_name )
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ(ch, pObj);

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax:  name [string]\n\r", ch );
	return FALSE;
    }

    if(!str_cmp(argument, "all"))
    {
	send_to_char("You may not name an object 'all'\n\r", ch);
	return FALSE;
    }

    free_string( pObj->name );
    pObj->name = str_dup( argument );

    send_to_char( "Name set.\n\r", ch);
    return TRUE;
}



OEDIT( oedit_short )
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ(ch, pObj);

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax:  short [string]\n\r", ch );
	return FALSE;
    }

    free_string( pObj->short_descr );
    pObj->short_descr = str_dup( argument );
    pObj->short_descr[0] = LOWER( pObj->short_descr[0] );

    send_to_char( "Short description set.\n\r", ch);
    return TRUE;
}



OEDIT( oedit_long )
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ(ch, pObj);

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax:  long [string]\n\r", ch );
	return FALSE;
    }
        
    free_string( pObj->description );
    pObj->description = str_dup( argument );
    pObj->description[0] = UPPER( pObj->description[0] );

    send_to_char( "Long description set.\n\r", ch);
    return TRUE;
}



bool set_value( CHAR_DATA *ch, OBJ_INDEX_DATA *pObj, char *argument, int value )
{
    if ( argument[0] == '\0' )
    {
	set_obj_values( ch, pObj, -1, "" );     /* '\0' changed to "" -- Hugin */
	return FALSE;
    }

    if ( set_obj_values( ch, pObj, value, argument ) )
	return TRUE;

    return FALSE;
}



/*****************************************************************************
 Name:		oedit_values
 Purpose:	Finds the object and sets its value.
 Called by:	The four valueX functions below. (now five -- Hugin )
 ****************************************************************************/
bool oedit_values( CHAR_DATA *ch, char *argument, int value )
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ(ch, pObj);

    if ( set_value( ch, pObj, argument, value ) )
        return TRUE;

    return FALSE;
}


OEDIT( oedit_value0 )
{
    if ( oedit_values( ch, argument, 0 ) )
        return TRUE;

    return FALSE;
}



OEDIT( oedit_value1 )
{
    if ( oedit_values( ch, argument, 1 ) )
        return TRUE;

    return FALSE;
}



OEDIT( oedit_value2 )
{
    if ( oedit_values( ch, argument, 2 ) )
        return TRUE;

    return FALSE;
}



OEDIT( oedit_value3 )
{
    if ( oedit_values( ch, argument, 3 ) )
        return TRUE;

    return FALSE;
}



OEDIT( oedit_value4 )
{
    if ( oedit_values( ch, argument, 4 ) )
        return TRUE;

    return FALSE;
}



OEDIT( oedit_weight )
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ(ch, pObj);

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
	send_to_char( "Syntax:  weight [number]\n\r", ch );
	return FALSE;
    }

    pObj->weight = atoi( argument );

    send_to_char( "Weight set.\n\r", ch);
    return TRUE;
}

OEDIT( oedit_cost )
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ(ch, pObj);

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
	send_to_char( "Syntax:  cost [number]\n\r", ch );
	return FALSE;
    }

    pObj->cost = atoi( argument );

    send_to_char( "Cost set.\n\r", ch);
    return TRUE;
}

OEDIT( oedit_autoweapon )
{
	OBJ_INDEX_DATA *pObj;
	AFFECT_DATA *pAf;
	int dice, size, bonus;
	double avg;

	EDIT_OBJ(ch, pObj);
	if (pObj->item_type != ITEM_WEAPON)
	{
		 send_to_char( " {rAutoweapn only works on weapons...{x\n\r", ch);
	return FALSE;
	}
	if (pObj->level < 1)
	{
		send_to_char( " {cAutoweapon requires a level to be set on the weapon first.{x\n\r", ch);
		return FALSE;
	}
   bonus = UMAX(0, pObj->level/10 - 1);
/* adjust this next line to change the avg dmg your weapons will get! */
	avg = (pObj->level * .76);
	dice = (pObj->level/10 + 1);
	size = dice/2;
/* loop through dice sizes until we find that the Next dice size's avg
will be too high... ie, find the "best fit" */
	for (size=dice/2 ; dice * (size +2)/2 < avg ; size++ )
	{ }

	dice = UMAX(1, dice);
	size = UMAX(2, size);

	switch (pObj->value[0]) {
	default:
	case WEAPON_EXOTIC:
	case WEAPON_SWORD:
		break;
	case WEAPON_DAGGER:
		dice = UMAX(1, dice - 1);
		size = UMAX(2, size - 1);
   	        break;
	case WEAPON_SPEAR:
	case WEAPON_POLEARM:
		size++;
		break;
	case WEAPON_MACE:
	case WEAPON_AXE:
		size = UMAX(2,size - 1);
		break;
	case WEAPON_FLAIL:
	case WEAPON_WHIP:
		dice = UMAX(1, dice - 1);
		break;
	}
	dice = UMAX(1, dice);
	size = UMAX(2, size);
	
	
	pObj->cost = 25 * (size * (dice + 1)) + 20 * bonus + 20 * pObj->level;
	pObj->weight = pObj->level + 1;
	pObj->value[1] = dice;
pObj->value[2] = size;
if (bonus > 0) {
pAf             =   new_affect();
    pAf->location   =   APPLY_DAMROLL;
    pAf->modifier   =   bonus;
    pAf->where	    =   TO_OBJECT;
    pAf->type       =   -1;
    pAf->duration   =   -1;
    pAf->bitvector  =   0;
    pAf->level      =	pObj->level;
    pAf->next       =   pObj->affected;
    pObj->affected  =   pAf;

pAf             =   new_affect();
    pAf->location   =   APPLY_HITROLL;
    pAf->modifier   =   bonus;
    pAf->where	    =   TO_OBJECT;
    pAf->type       =   -1;
    pAf->duration   =   -1;
    pAf->bitvector  =   0;
    pAf->level      =	pObj->level;
    pAf->next       =   pObj->affected;
    pObj->affected  =   pAf;
}
send_to_char(" {cExperimental values set on weapon...{x\n\r", ch);
return TRUE;
}

OEDIT( oedit_autoarmor )
{
   OBJ_INDEX_DATA *pObj;
	int size;

	EDIT_OBJ(ch, pObj);
	if (pObj->item_type != ITEM_ARMOR)
	{
		 send_to_char( " {rAutoArmor only works on Armor ...{x\n\r", ch);
	return FALSE;
	}
	if (pObj->level < 1)
	{
		send_to_char( " {cAutoArmor requires a level to be set on the armor first.{x\n\r", ch);
		return FALSE;
	}
	size = UMAX(1, pObj->level/2.8 + 1);
	pObj->weight = pObj->level + 1;
pObj->cost = pObj->level^2 * 2;	
pObj->value[0] = size;
	pObj->value[1] = size;
	pObj->value[2] = size;
		pObj->value[3] = (size - 1);
		send_to_char( " {cAutoArmor has set experimental values for AC.{x\n\r", ch);
		return TRUE;
}


OEDIT( oedit_create )
{
    OBJ_INDEX_DATA *pObj;
    AREA_DATA *pArea;
    int  value;
    int  iHash;

    value = atoi( argument );
    if ( argument[0] == '\0' || value == 0 )
    {
	send_to_char( "Syntax:  oedit create [vnum]\n\r", ch );
	return FALSE;
    }

    pArea = get_vnum_area( value );
    if ( !pArea )
    {
	send_to_char( "OEdit:  That vnum is not assigned an area.\n\r", ch );
	return FALSE;
    }


  if(!IS_IMP(ch))
   {
    if((IS_SET(pArea->area_flags, AREA_NOIMM))
    ||  (IS_SET(pArea->area_flags, AREA_IMP)))
      {
          send_to_char(
          "\n\r{GIf you need something changed in this {WAREA{G, please note the IMPs!{x\n\r",ch);
          return FALSE;
      }
   }


    if ( get_obj_index( value ) )
    {
	send_to_char( "OEdit:  Object vnum already exists.\n\r", ch );
	return FALSE;
    }
        
    pObj			= new_obj_index();
    pObj->vnum			= value;
    pObj->area			= pArea;
        
    if ( value > top_vnum_obj )
	top_vnum_obj = value;

    iHash			= value % MAX_KEY_HASH;
    pObj->next			= obj_index_hash[iHash];
    obj_index_hash[iHash]	= pObj;
    ch->desc->pEdit		= (void *)pObj;

    send_to_char( "Object Created.\n\r", ch );
    return TRUE;
}



OEDIT( oedit_ed )
{
    OBJ_INDEX_DATA *pObj;
    EXTRA_DESCR_DATA *ed;
    char command[MAX_INPUT_LENGTH];
    char keyword[MAX_INPUT_LENGTH];

    EDIT_OBJ(ch, pObj);

    argument = one_argument( argument, command );
    one_argument( argument, keyword );

    if ( command[0] == '\0' )
    {
	send_to_char( "Syntax:  ed add [keyword]\n\r", ch );
	send_to_char( "         ed delete [keyword]\n\r", ch );
	send_to_char( "         ed edit [keyword]\n\r", ch );
	send_to_char( "         ed format [keyword]\n\r", ch );
	return FALSE;
    }

    if ( !str_cmp( command, "add" ) )
    {
	if ( keyword[0] == '\0' )
	{
	    send_to_char( "Syntax:  ed add [keyword]\n\r", ch );
	    return FALSE;
	}

	ed                  =   new_extra_descr();
	ed->keyword         =   str_dup( keyword );
	ed->next            =   pObj->extra_descr;
	pObj->extra_descr   =   ed;

	string_append( ch, &ed->description );

	return TRUE;
    }

    if ( !str_cmp( command, "edit" ) )
    {
	if ( keyword[0] == '\0' )
	{
	    send_to_char( "Syntax:  ed edit [keyword]\n\r", ch );
	    return FALSE;
	}

	for ( ed = pObj->extra_descr; ed; ed = ed->next )
	{
	    if ( is_name( keyword, ed->keyword ) )
		break;
	}

	if ( !ed )
	{
	    send_to_char( "OEdit:  Extra description keyword not found.\n\r", ch );
	    return FALSE;
	}

	string_append( ch, &ed->description );

	return TRUE;
    }

    if ( !str_cmp( command, "delete" ) )
    {
	EXTRA_DESCR_DATA *ped = NULL;

	if ( keyword[0] == '\0' )
	{
	    send_to_char( "Syntax:  ed delete [keyword]\n\r", ch );
	    return FALSE;
	}

	for ( ed = pObj->extra_descr; ed; ed = ed->next )
	{
	    if ( is_name( keyword, ed->keyword ) )
		break;
	    ped = ed;
	}

	if ( !ed )
	{
	    send_to_char( "OEdit:  Extra description keyword not found.\n\r", ch );
	    return FALSE;
	}

	if ( !ped )
	    pObj->extra_descr = ed->next;
	else
	    ped->next = ed->next;

	free_extra_descr( ed );

	send_to_char( "Extra description deleted.\n\r", ch );
	return TRUE;
    }


    if ( !str_cmp( command, "format" ) )
    {
	EXTRA_DESCR_DATA *ped = NULL;

	if ( keyword[0] == '\0' )
	{
	    send_to_char( "Syntax:  ed format [keyword]\n\r", ch );
	    return FALSE;
	}

	for ( ed = pObj->extra_descr; ed; ed = ed->next )
	{
	    if ( is_name( keyword, ed->keyword ) )
		break;
	    ped = ed;
	}

	if ( !ed )
	{
                send_to_char( "OEdit:  Extra description keyword not found.\n\r", ch );
                return FALSE;
	}

	ed->description = format_string( ed->description );

	send_to_char( "Extra description formatted.\n\r", ch );
	return TRUE;
    }

    oedit_ed( ch, "" );
    return FALSE;
}





/* ROM object functions : */

OEDIT( oedit_extra )      /* Moved out of oedit() due to naming conflicts -- Hugin */
{
    OBJ_INDEX_DATA *pObj;
    int value;

    if ( argument[0] != '\0' )
    {
	EDIT_OBJ(ch, pObj);
      
	if ( ( value = flag_value(extra_flags, argument)) != NO_FLAG )
	{
	    TOGGLE_BIT(pObj->extra_flags, value);

	    send_to_char( "Extra flag toggled.\n\r", ch);
	    return TRUE;
	}
    }

    send_to_char( "Syntax:  extra [flag]\n\r"
		  "Type '? extra' for a list of flags.\n\r", ch );
    return FALSE;
}


OEDIT( oedit_wear )      /* Moved out of oedit() due to naming conflicts -- Hugin */
{
    OBJ_INDEX_DATA *pObj;
    int value;

     if ( argument[0] != '\0' )
    {
	EDIT_OBJ(ch, pObj);

	if ( ( value = flag_value( wear_flags, argument ) ) != NO_FLAG )
	{
	    TOGGLE_BIT(pObj->wear_flags, value);

	    send_to_char( "Wear flag toggled.\n\r", ch);
	    return TRUE;
	}
    }

    send_to_char( "Syntax:  wear [flag]\n\r"
		  "Type '? wear' for a list of flags.\n\r", ch );
    return FALSE;
}


OEDIT( oedit_type )      /* Moved out of oedit() due to naming conflicts -- Hugin */
{
    OBJ_INDEX_DATA *pObj;
    int value;

    if ( argument[0] != '\0' )
    {
	EDIT_OBJ(ch, pObj);

	if ( ( value = flag_value( type_flags, argument ) ) != NO_FLAG )
	{
	    pObj->item_type = value;

	    send_to_char( "Type set.\n\r", ch);

	    /*
	     * Clear the values.
	     */
	    pObj->value[0] = 0;
	    pObj->value[1] = 0;
	    pObj->value[2] = 0;
	    pObj->value[3] = 0;
	    pObj->value[4] = 0;     /* ROM */

	    return TRUE;
	}
    }

    send_to_char( "Syntax:  type [flag]\n\r"
		  "Type '? type' for a list of flags.\n\r", ch );
    return FALSE;
}

OEDIT( oedit_material )
{
    OBJ_INDEX_DATA *pObj;
    int value;

    if ( argument[0] != '\0' )
    {
	EDIT_OBJ(ch, pObj);
	if ( ( value = flag_value( material_flags, argument ) ) != NO_FLAG )
	{
	 pObj->material_type = value;
	 send_to_char( "\n\r{CMATERIAL SET\n\r", ch);
         return TRUE;
        }
     }
     send_to_char( "\n\r{GSYNTAX{w:  {WMATERIAL {c<{WMATERIAL NUMBER{c>{x\n\r"
		   "{cType '{W? MATERIAL{c' to list available {CMATERIALs{x\n\r",ch);
     return FALSE;
}


OEDIT( oedit_level )
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ(ch, pObj);

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
	send_to_char( "Syntax:  level [number]\n\r", ch );
	return FALSE;
    }

    pObj->level = atoi( argument );

    send_to_char( "Level set.\n\r", ch);
    return TRUE;
}



OEDIT( oedit_condition )
{
    OBJ_INDEX_DATA *pObj;
    int value;

    if ( argument[0] != '\0'
    && ( value = atoi (argument ) ) >= 0
    && ( value <= 100 ) )
    {
	EDIT_OBJ( ch, pObj );

	pObj->condition = value;
	send_to_char( "Condition set.\n\r", ch );

	return TRUE;
    }

    send_to_char( "Syntax:  condition [number]\n\r"
		  "Where number can range from 0 (ruined) to 100 (perfect).\n\r",
		  ch );
    return FALSE;
}





/*
 * Mobile Editor Functions.
 */
MEDIT( medit_show )
{ 
    MOB_INDEX_DATA *pMob;
    char buf[MAX_STRING_LENGTH];
    MPROG_LIST *list;

    EDIT_MOB(ch, pMob);

    sprintf( buf, "\n\r{cFull DESC{w:\n\r{W%s{x", pMob->description );
    send_to_char( buf, ch );

    sprintf( buf, "\n\r     {cName{w: {c[{C%s{c]\n\r     Area{w: {c[{C%5d{c]{W %s\n\r",
	pMob->player_name,
	!pMob->area ? -1        : pMob->area->vnum,
	!pMob->area ? "No Area" : pMob->area->name );
    send_to_char( buf, ch );

    sprintf( buf, "{c     Vnum{w: {c[{C%5d{c]   Sex{w: {c[{C%s{c]  Race{w: {c[{C%s{c]{x\n\r",
	pMob->vnum,
	pMob->sex == SEX_MALE    ? " MALE  " :
	pMob->sex == SEX_FEMALE  ? "FEMALE " : 
	pMob->sex == 3           ? "RANDOM " : "NEUTRAL",
	race_table[pMob->race].name );
    send_to_char( buf, ch );

    sprintf( buf,
"    {cLevel{w: {c[{C%4d{c]  Align{w: {c[{C%4d{c]  HITRoll{w: {c[{C%3d{c]  DamType{w: {c[{C%s{c]{x\n\r",
	pMob->level,	pMob->alignment,
	pMob->hitroll,	attack_table[pMob->dam_type].name );
    send_to_char( buf, ch );

    sprintf( buf, "{c  HITDice{w: {c[{C%3d{cd{C%-3d{c+{C%5d{c] ",
	     pMob->hit[DICE_NUMBER],
	     pMob->hit[DICE_TYPE],
	     pMob->hit[DICE_BONUS] );
    send_to_char( buf, ch );

    sprintf( buf, " DAMDice{w: {c[{C%3d{cd{C%-3d{c+{C%5d{c] ",
	     pMob->damage[DICE_NUMBER],
	     pMob->damage[DICE_TYPE],
	     pMob->damage[DICE_BONUS] );
    send_to_char( buf, ch );

    sprintf( buf, " MANADice{w: {c[{C%3d{cd{C%-3d{c+{C%5d{c]{x\n\r",
	     pMob->mana[DICE_NUMBER],
	     pMob->mana[DICE_TYPE],
	     pMob->mana[DICE_BONUS] );
    send_to_char( buf, ch );

    sprintf( buf, "{c     Size{w: {c[{C%s{c]  Wealth{w: {c[{C%7ld{c]  ",
	flag_string( size_flags, pMob->size), pMob->wealth );
    send_to_char( buf, ch );

    if ( pMob->spec_fun )
      {
	sprintf( buf, "{cSPEC PROC{w: {c[{W%s{c]\n\r",  spec_name( pMob->spec_fun ));
	send_to_char( buf, ch );
      }
    else
	send_to_char("\n\r", ch );

    sprintf( buf, "{c       AC{w: {c[{CPierce{w: {W%d  {CBash{w: {W%d  {CSlash{w: {W%d  {CMagic{w: {W%d{c]\n\r\n\r",
	pMob->ac[AC_PIERCE], pMob->ac[AC_BASH],
	pMob->ac[AC_SLASH], pMob->ac[AC_EXOTIC] );
    send_to_char( buf, ch );

    sprintf( buf, "{c      IMM{w: {c[{C%s{c]\n\r",
	flag_string( imm_flags, pMob->imm_flags ) );
    send_to_char( buf, ch );

    sprintf( buf, "{c      RES{w: {c[{C%s{c]\n\r",
	flag_string( res_flags, pMob->res_flags ) );
    send_to_char( buf, ch );

    sprintf( buf, "{c     VULN{w: {c[{C%s{c]\n\r",
	flag_string( vuln_flags, pMob->vuln_flags ) );
    send_to_char( buf, ch );

    sprintf( buf, "{c      OFF{w: {c[{C%s{c]\n\r\n\r",
	flag_string( off_flags,  pMob->off_flags ) );
    send_to_char( buf, ch );

    sprintf( buf, "{c      Act{w: {c[{C%s{c]{x\n\r",
	flag_string( act_flags, pMob->act ) );
    send_to_char( buf, ch );

    sprintf( buf, "{c      AFF{w: {c[{C%s{c]\n\r",
	flag_string( affect_flags, pMob->affected_by ) );
    send_to_char( buf, ch );

    sprintf( buf, "{c     AFF2{w: {c[{C%s{c]\n\r\n\r",
	flag_string( affect2_flags, pMob->affected2_by ) );
    send_to_char( buf, ch );

    sprintf( buf, "{c     Form{w: {c[{C%s{c]\n\r",
	flag_string( form_flags, pMob->form ) );
    send_to_char( buf, ch );

    sprintf( buf, "{c    Parts{w: {c[{C%s{c]\n\r\n\r",
	flag_string( part_flags, pMob->parts ) );
    send_to_char( buf, ch );

    sprintf( buf, "{cStart POS{w: {c[{C%s{c]  DEF POS{w: {c[{C%s{c]\n\r",
	flag_string( position_flags, pMob->start_pos),
        flag_string( position_flags, pMob->default_pos ) );
    send_to_char( buf, ch );


    if ( pMob->group )
    {
	sprintf( buf, "{c    Group{w: {c[{C%5d{c]\n\r", pMob->group );
	send_to_char( buf, ch );
    }

    sprintf( buf, "{c   S DESC{w: {C%s\n\r{c   L DESC{w: {C%s\n\r",
	pMob->short_descr,
	pMob->long_descr );
    send_to_char( buf, ch );

    if ( pMob->pShop )
    {
	SHOP_DATA *pShop;
	int iTrade;

	pShop = pMob->pShop;

	sprintf( buf,
	  "{cShop Data for {D[{W%5d{D]{w:\n\r"
	  "  {CMarkup for Purchaser{w: {W%d{c%%\n\r"
	  "  {CMarkdown for Seller{w:  {W%d{c%%\n\r",
	    pShop->keeper, pShop->profit_buy, pShop->profit_sell );
	send_to_char( buf, ch );
	sprintf( buf, "  {cHours{w: {C%d {cto {C%d{x\n\r",
	    pShop->open_hour, pShop->close_hour );
	send_to_char( buf, ch );

	for ( iTrade = 0; iTrade < MAX_TRADE; iTrade++ )
	{
	    if ( pShop->buy_type[iTrade] != 0 )
	    {
		if ( iTrade == 0 ) {
		    send_to_char( "  {cNumber Trades Type\n\r", ch );
		    send_to_char( "  {C------ -----------\n\r", ch );
		}
		sprintf( buf, "  {c[{W%4d{c] {C%s{x\n\r", iTrade,
		    flag_string( type_flags, pShop->buy_type[iTrade] ) );
		send_to_char( buf, ch );
	    }
	}
    }

    if ( pMob->mprogs )
    {
	int cnt;

	sprintf(buf, "\n\r{cMOBProgs for {D[{W%5d{D]{w:\n\r", pMob->vnum);
	send_to_char( buf, ch );

	for (cnt=0, list=pMob->mprogs; list; list=list->next)
	{
		if (cnt ==0)
		{
			send_to_char ( " {cNumber Vnum Trigger Phrase\n\r", ch );
			send_to_char ( " {C------ ---- ------- ------\n\r", ch );
		}

		sprintf(buf, "{c[{W%5d{c] {C%4d %7s %s\n\r", cnt,
			list->vnum,mprog_type_to_name(list->trig_type),
			list->trig_phrase);
		send_to_char( buf, ch );
		cnt++;
	}
    }

    return FALSE;
}



MEDIT( medit_create )
{
    MOB_INDEX_DATA *pMob;
    AREA_DATA *pArea;
    int  value;
    int  iHash;

    value = atoi( argument );
    if ( argument[0] == '\0' || value == 0 )
    {
	send_to_char( "Syntax:  medit create [vnum]\n\r", ch );
	return FALSE;
    }

    pArea = get_vnum_area( value );

    if ( !pArea )
    {
	send_to_char( "MEdit:  That vnum is not assigned an area.\n\r", ch );
	return FALSE;
    }

  if(!IS_IMP(ch))
   {
    if((IS_SET(pArea->area_flags, AREA_NOIMM))
    ||  (IS_SET(pArea->area_flags, AREA_IMP)))
      {
          send_to_char(
          "\n\r{GIf you need something changed in this {WAREA{G, please note the IMPs!{x\n\r",ch);
          return FALSE;
      }
   }

    if ( get_mob_index( value ) )
    {
	send_to_char( "MEdit:  Mobile vnum already exists.\n\r", ch );
	return FALSE;
    }

    pMob			= new_mob_index();
    pMob->vnum			= value;
    pMob->area			= pArea;
        
    if ( value > top_vnum_mob )
	top_vnum_mob = value;        

    pMob->act			= ACT_IS_NPC;
    iHash			= value % MAX_KEY_HASH;
    pMob->next			= mob_index_hash[iHash];
    mob_index_hash[iHash]	= pMob;
    ch->desc->pEdit		= (void *)pMob;

    send_to_char( "Mobile Created.\n\r", ch );
    return TRUE;
}



MEDIT( medit_spec )
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch, pMob);

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax:  spec [special function]\n\r", ch );
	return FALSE;
    }


    if ( !str_cmp( argument, "none" ) )
    {
        pMob->spec_fun = NULL;

        send_to_char( "Spec removed.\n\r", ch);
        return TRUE;
    }

    if ( spec_lookup( argument ) )
    {
	pMob->spec_fun = spec_lookup( argument );
	send_to_char( "Spec set.\n\r", ch);
	return TRUE;
    }

    send_to_char( "MEdit: No such special function.\n\r", ch );
    return FALSE;
}

MEDIT( medit_damtype )
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch, pMob);

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax:  damtype [damage message]\n\r", ch );
	send_to_char( "Para ver una lista de tipos de mensajes, pon '? weapon'.\n\r", ch );
	return FALSE;
    }

    pMob->dam_type = attack_lookup(argument);
    send_to_char( "Damage type set.\n\r", ch);
    return TRUE;
}


MEDIT( medit_align )
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch, pMob);

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
	send_to_char( "Syntax:  alignment [number]\n\r", ch );
	return FALSE;
    }

    pMob->alignment = atoi( argument );

    send_to_char( "Alignment set.\n\r", ch);
    return TRUE;
}



MEDIT( medit_level )
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch, pMob);

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
	send_to_char( "Syntax:  level [number]\n\r", ch );
	return FALSE;
    }

    pMob->level = atoi( argument );

    send_to_char( "Level set.\n\r", ch);
    return TRUE;
}



MEDIT( medit_desc )
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch, pMob);

    if ( argument[0] == '\0' )
    {
	string_append( ch, &pMob->description );
	return TRUE;
    }

    send_to_char( "Syntax:  desc    - line edit\n\r", ch );
    return FALSE;
}




MEDIT( medit_long )
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch, pMob);

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax:  long [string]\n\r", ch );
	return FALSE;
    }

    free_string( pMob->long_descr );
    strcat( argument, "\n\r" );
    pMob->long_descr = str_dup( argument );
    pMob->long_descr[0] = UPPER( pMob->long_descr[0]  );

    send_to_char( "Long description set.\n\r", ch);
    return TRUE;
}



MEDIT( medit_short )
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch, pMob);

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax:  short [string]\n\r", ch );
	return FALSE;
    }

    free_string( pMob->short_descr );
    pMob->short_descr = str_dup( argument );

    send_to_char( "Short description set.\n\r", ch);
    return TRUE;
}



MEDIT( medit_name )
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch, pMob);

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax:  name [string]\n\r", ch );
	return FALSE;
    }

    free_string( pMob->player_name );
    pMob->player_name = str_dup( argument );

    send_to_char( "Name set.\n\r", ch);
    return TRUE;
}

MEDIT( medit_shop )
{
    MOB_INDEX_DATA *pMob;
    char command[MAX_INPUT_LENGTH];
    char arg1[MAX_INPUT_LENGTH];

    argument = one_argument( argument, command );
    argument = one_argument( argument, arg1 );

    EDIT_MOB(ch, pMob);

    if ( command[0] == '\0' )
    {
	send_to_char( "Syntax:  shop hours [#xopening] [#xclosing]\n\r", ch );
	send_to_char( "         shop profit [#xbuying%] [#xselling%]\n\r", ch );
	send_to_char( "         shop type [#x0-4] [item type]\n\r", ch );
	send_to_char( "         shop assign\n\r", ch );
	send_to_char( "         shop remove\n\r", ch );
	return FALSE;
    }


    if ( !str_cmp( command, "hours" ) )
    {
	if ( arg1[0] == '\0' || !is_number( arg1 )
	|| argument[0] == '\0' || !is_number( argument ) )
	{
	    send_to_char( "Syntax:  shop hours [#xopening] [#xclosing]\n\r", ch );
	    return FALSE;
	}

	if ( !pMob->pShop )
	{
	    send_to_char( "MEdit:  Debes crear un shop primero (shop assign).\n\r", ch );
	    return FALSE;
	}

	pMob->pShop->open_hour = atoi( arg1 );
	pMob->pShop->close_hour = atoi( argument );

	send_to_char( "Shop hours set.\n\r", ch);
	return TRUE;
    }


    if ( !str_cmp( command, "profit" ) )
    {
	if ( arg1[0] == '\0' || !is_number( arg1 )
	|| argument[0] == '\0' || !is_number( argument ) )
	{
	    send_to_char( "Syntax:  shop profit [#xbuying%] [#xselling%]\n\r", ch );
	    return FALSE;
	}

	if ( !pMob->pShop )
	{
	    send_to_char( "MEdit:  Debes crear un shop primero (shop assign).\n\r", ch );
	    return FALSE;
	}

	pMob->pShop->profit_buy     = atoi( arg1 );
	pMob->pShop->profit_sell    = atoi( argument );

	send_to_char( "Shop profit set.\n\r", ch);
	return TRUE;
    }


    if ( !str_cmp( command, "type" ) )
    {
	char buf[MAX_INPUT_LENGTH];
	int value;

	if ( arg1[0] == '\0' || !is_number( arg1 )
	|| argument[0] == '\0' )
	{
	    send_to_char( "Syntax:  shop type [#x0-4] [item type]\n\r", ch );
	    return FALSE;
	}

	if ( atoi( arg1 ) >= MAX_TRADE )
	{
	    sprintf( buf, "MEdit:  May sell %d items max.\n\r", MAX_TRADE );
	    send_to_char( buf, ch );
	    return FALSE;
	}

	if ( !pMob->pShop )
	{
	    send_to_char( "MEdit:  Debes crear un shop primero (shop assign).\n\r", ch );
	    return FALSE;
	}

	if ( ( value = flag_value( type_flags, argument ) ) == NO_FLAG )
	{
	    send_to_char( "MEdit:  That type of item is not known.\n\r", ch );
	    return FALSE;
	}

	pMob->pShop->buy_type[atoi( arg1 )] = value;

	send_to_char( "Shop type set.\n\r", ch);
	return TRUE;
    }

    /* shop assign && shop delete by Phoenix */

    if ( !str_prefix(command, "assign") )
    {
    	if ( pMob->pShop )
    	{
        	send_to_char("Mob already has a shop assigned to it.\n\r", ch);
        	return FALSE;
	}

	pMob->pShop		= new_shop();
	if ( !shop_first )
        	shop_first	= pMob->pShop;
	if ( shop_last )
		shop_last->next	= pMob->pShop;
	shop_last		= pMob->pShop;

	pMob->pShop->keeper	= pMob->vnum;

	send_to_char("New shop assigned to mobile.\n\r", ch);
	return TRUE;
    }

    if ( !str_prefix(command, "remove") )
    {
	SHOP_DATA *pShop;

	pShop		= pMob->pShop;
	pMob->pShop	= NULL;

	if ( pShop == shop_first )
	{
		if ( !pShop->next )
		{
			shop_first = NULL;
			shop_last = NULL;
		}
		else
			shop_first = pShop->next;
	}
	else
	{
		SHOP_DATA *ipShop;

		for ( ipShop = shop_first; ipShop; ipShop = ipShop->next )
		{
			if ( ipShop->next == pShop )
			{
				if ( !pShop->next )
				{
					shop_last = ipShop;
					shop_last->next = NULL;
				}
				else
					ipShop->next = pShop->next;
			}
		}
	}

	free_shop(pShop);

	send_to_char("Mobile is no longer a shopkeeper.\n\r", ch);
	return TRUE;
    }

    medit_shop( ch, "" );
    return FALSE;
}


/* ROM medit functions: */


MEDIT( medit_sex )          /* Moved out of medit() due to naming conflicts -- Hugin */
{
    MOB_INDEX_DATA *pMob;
    int value;

    if ( argument[0] != '\0' )
    {
	EDIT_MOB( ch, pMob );

	if ( ( value = flag_value( sex_flags, argument ) ) != NO_FLAG )
	{
	    pMob->sex = value;

	    send_to_char( "Sex set.\n\r", ch);
	    return TRUE;
	}
    }

    send_to_char( "Syntax: sex [sex]\n\r"
		  "Type '? sex' for a list of flags.\n\r", ch );
    return FALSE;
}


MEDIT( medit_act )          /* Moved out of medit() due to naming conflicts -- Hugin */
{
    MOB_INDEX_DATA *pMob;
    int value;

    if ( argument[0] != '\0' )
    {
	EDIT_MOB( ch, pMob );

	if ( ( value = flag_value( act_flags, argument ) ) != NO_FLAG )
	{
	    pMob->act ^= value;
	    SET_BIT( pMob->act, ACT_IS_NPC );

	    send_to_char( "Act flag toggled.\n\r", ch);
	    return TRUE;
	}
    }

    send_to_char( "Syntax: act [flag]\n\r"
		  "Type '? act' for a list of flags.\n\r", ch );
    return FALSE;
}


MEDIT( medit_affect )      /* Moved out of medit() due to naming conflicts -- Hugin */
{
    MOB_INDEX_DATA *pMob;
    int value;

    if ( argument[0] != '\0' )
    {
	EDIT_MOB( ch, pMob );

	if ( ( value = flag_value( affect_flags, argument ) ) != NO_FLAG )
	{
	    pMob->affected_by ^= value;

	    send_to_char( "Affect flag toggled.\n\r", ch);
	    return TRUE;
	}
    }

    send_to_char( "Syntax: affect [flag]\n\r"
		  "Type '? affect' for a list of flags.\n\r", ch );
    return FALSE;
}

MEDIT( medit_affect2 )      /* Moved out of medit() due to naming conflicts -- Hugin */
{
    MOB_INDEX_DATA *pMob;
    int value;

    if ( argument[0] != '\0' )
    {
	EDIT_MOB( ch, pMob );

	if ( ( value = flag_value( affect2_flags, argument ) ) != NO_FLAG )
	{
	    pMob->affected2_by ^= value;

	    send_to_char( "Affect flag toggled.\n\r", ch);
	    return TRUE;
	}
    }

    send_to_char( "Syntax: affect2 [flag]\n\r"
		  "Type '? affect2' for a list of flags.\n\r", ch );
    return FALSE;
}

MEDIT( medit_ac )
{
    MOB_INDEX_DATA *pMob;
    char arg[MAX_INPUT_LENGTH];
    int pierce, bash, slash, exotic;

    do   /* So that I can use break and send the syntax in one place */
    {
	if ( argument[0] == '\0' )  break;

	EDIT_MOB(ch, pMob);
	argument = one_argument( argument, arg );

	if ( !is_number( arg ) )  break;
	pierce = atoi( arg );
	argument = one_argument( argument, arg );

	if ( arg[0] != '\0' )
	{
	    if ( !is_number( arg ) )  break;
	    bash = atoi( arg );
	    argument = one_argument( argument, arg );
	}
	else
	    bash = pMob->ac[AC_BASH];

	if ( arg[0] != '\0' )
	{
	    if ( !is_number( arg ) )  break;
	    slash = atoi( arg );
	    argument = one_argument( argument, arg );
	}
	else
	    slash = pMob->ac[AC_SLASH];

	if ( arg[0] != '\0' )
	{
	    if ( !is_number( arg ) )  break;
	    exotic = atoi( arg );
	}
	else
	    exotic = pMob->ac[AC_EXOTIC];

	pMob->ac[AC_PIERCE] = pierce;
	pMob->ac[AC_BASH]   = bash;
	pMob->ac[AC_SLASH]  = slash;
	pMob->ac[AC_EXOTIC] = exotic;
	
	send_to_char( "Ac set.\n\r", ch );
	return TRUE;
    } while ( FALSE );    /* Just do it once.. */

    send_to_char( "Syntax:  ac [ac-pierce [ac-bash [ac-slash [ac-exotic]]]]\n\r"
		  "help MOB_AC  gives a list of reasonable ac-values.\n\r", ch );
    return FALSE;
}

MEDIT( medit_form )
{
    MOB_INDEX_DATA *pMob;
    int value;

    if ( argument[0] != '\0' )
    {
	EDIT_MOB( ch, pMob );

	if ( ( value = flag_value( form_flags, argument ) ) != NO_FLAG )
	{
	    pMob->form ^= value;
	    send_to_char( "Form toggled.\n\r", ch );
	    return TRUE;
	}
    }

    send_to_char( "Syntax: form [flags]\n\r"
		  "Type '? form' for a list of flags.\n\r", ch );
    return FALSE;
}

MEDIT( medit_part )
{
    MOB_INDEX_DATA *pMob;
    int value;

    if ( argument[0] != '\0' )
    {
	EDIT_MOB( ch, pMob );

	if ( ( value = flag_value( part_flags, argument ) ) != NO_FLAG )
	{
	    pMob->parts ^= value;
	    send_to_char( "Parts toggled.\n\r", ch );
	    return TRUE;
	}
    }

    send_to_char( "Syntax: part [flags]\n\r"
		  "Type '? part' for a list of flags.\n\r", ch );
    return FALSE;
}

MEDIT( medit_imm )
{
    MOB_INDEX_DATA *pMob;
    int value;

    if ( argument[0] != '\0' )
    {
	EDIT_MOB( ch, pMob );

	if ( ( value = flag_value( imm_flags, argument ) ) != NO_FLAG )
	{
	    pMob->imm_flags ^= value;
	    send_to_char( "Immunity toggled.\n\r", ch );
	    return TRUE;
	}
    }

    send_to_char( "Syntax: imm [flags]\n\r"
		  "Type '? imm' for a list of flags.\n\r", ch );
    return FALSE;
}

MEDIT( medit_res )
{
    MOB_INDEX_DATA *pMob;
    int value;

    if ( argument[0] != '\0' )
    {
	EDIT_MOB( ch, pMob );

	if ( ( value = flag_value( res_flags, argument ) ) != NO_FLAG )
	{
	    pMob->res_flags ^= value;
	    send_to_char( "Resistance toggled.\n\r", ch );
	    return TRUE;
	}
    }

    send_to_char( "Syntax: res [flags]\n\r"
		  "Type '? res' for a list of flags.\n\r", ch );
    return FALSE;
}

MEDIT( medit_vuln )
{
    MOB_INDEX_DATA *pMob;
    int value;

    if ( argument[0] != '\0' )
    {
	EDIT_MOB( ch, pMob );

	if ( ( value = flag_value( vuln_flags, argument ) ) != NO_FLAG )
	{
	    pMob->vuln_flags ^= value;
	    send_to_char( "Vulnerability toggled.\n\r", ch );
	    return TRUE;
	}
    }

    send_to_char( "Syntax: vuln [flags]\n\r"
		  "Type '? vuln' for a list of flags.\n\r", ch );
    return FALSE;
}

MEDIT( medit_material )
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch, pMob);

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax:  material [string]\n\r", ch );
	return FALSE;
    }

    free_string( pMob->material );
    pMob->material = str_dup( argument );

    send_to_char( "Material set.\n\r", ch);
    return TRUE;
}

MEDIT( medit_off )
{
    MOB_INDEX_DATA *pMob;
    int value;

    if ( argument[0] != '\0' )
    {
	EDIT_MOB( ch, pMob );

	if ( ( value = flag_value( off_flags, argument ) ) != NO_FLAG )
	{
	    pMob->off_flags ^= value;
	    send_to_char( "Offensive behaviour toggled.\n\r", ch );
	    return TRUE;
	}
    }

    send_to_char( "Syntax: off [flags]\n\r"
		  "Type '? off' for a list of flags.\n\r", ch );
    return FALSE;
}

MEDIT( medit_size )
{
    MOB_INDEX_DATA *pMob;
    int value;

    if ( argument[0] != '\0' )
    {
	EDIT_MOB( ch, pMob );

	if ( ( value = flag_value( size_flags, argument ) ) != NO_FLAG )
	{
	    pMob->size = value;
	    send_to_char( "Size set.\n\r", ch );
	    return TRUE;
	}
    }

    send_to_char( "Syntax: size [size]\n\r"
		  "Type '? size' for a list of sizes.\n\r", ch );
    return FALSE;
}

MEDIT( medit_hitdice )
{
    static char syntax[] = "Syntax:  hitdice <number> d <type> + <bonus>\n\r";
    char *num, *type, *bonus, *cp;
    MOB_INDEX_DATA *pMob;

    EDIT_MOB( ch, pMob );

    if ( argument[0] == '\0' )
    {
	send_to_char( syntax, ch );
	return FALSE;
    }

    num = cp = argument;

    while ( isdigit( *cp ) ) ++cp;
    while ( *cp != '\0' && !isdigit( *cp ) )  *(cp++) = '\0';

    type = cp;

    while ( isdigit( *cp ) ) ++cp;
    while ( *cp != '\0' && !isdigit( *cp ) ) *(cp++) = '\0';

    bonus = cp;

    while ( isdigit( *cp ) ) ++cp;
    if ( *cp != '\0' ) *cp = '\0';

    if ( ( !is_number( num   ) || atoi( num   ) < 1 )
    ||   ( !is_number( type  ) || atoi( type  ) < 1 ) 
    ||   ( !is_number( bonus ) || atoi( bonus ) < 0 ) )
    {
	send_to_char( syntax, ch );
	return FALSE;
    }

    pMob->hit[DICE_NUMBER] = atoi( num   );
    pMob->hit[DICE_TYPE]   = atoi( type  );
    pMob->hit[DICE_BONUS]  = atoi( bonus );

    send_to_char( "Hitdice set.\n\r", ch );
    return TRUE;
}

MEDIT( medit_manadice )
{
    static char syntax[] = "Syntax:  manadice <number> d <type> + <bonus>\n\r";
    char *num, *type, *bonus, *cp;
    MOB_INDEX_DATA *pMob;

    EDIT_MOB( ch, pMob );

    if ( argument[0] == '\0' )
    {
	send_to_char( syntax, ch );
	return FALSE;
    }

    num = cp = argument;

    while ( isdigit( *cp ) ) ++cp;
    while ( *cp != '\0' && !isdigit( *cp ) )  *(cp++) = '\0';

    type = cp;

    while ( isdigit( *cp ) ) ++cp;
    while ( *cp != '\0' && !isdigit( *cp ) ) *(cp++) = '\0';

    bonus = cp;

    while ( isdigit( *cp ) ) ++cp;
    if ( *cp != '\0' ) *cp = '\0';

    if ( !( is_number( num ) && is_number( type ) && is_number( bonus ) ) )
    {
	send_to_char( syntax, ch );
	return FALSE;
    }

    if ( ( !is_number( num   ) || atoi( num   ) < 1 )
    ||   ( !is_number( type  ) || atoi( type  ) < 1 ) 
    ||   ( !is_number( bonus ) || atoi( bonus ) < 0 ) )
    {
	send_to_char( syntax, ch );
	return FALSE;
    }

    pMob->mana[DICE_NUMBER] = atoi( num   );
    pMob->mana[DICE_TYPE]   = atoi( type  );
    pMob->mana[DICE_BONUS]  = atoi( bonus );

    send_to_char( "Manadice set.\n\r", ch );
    return TRUE;
}

MEDIT( medit_damdice )
{
    static char syntax[] = "Syntax:  damdice <number> d <type> + <bonus>\n\r";
    char *num, *type, *bonus, *cp;
    MOB_INDEX_DATA *pMob;

    EDIT_MOB( ch, pMob );

    if ( argument[0] == '\0' )
    {
	send_to_char( syntax, ch );
	return FALSE;
    }

    num = cp = argument;

    while ( isdigit( *cp ) ) ++cp;
    while ( *cp != '\0' && !isdigit( *cp ) )  *(cp++) = '\0';

    type = cp;

    while ( isdigit( *cp ) ) ++cp;
    while ( *cp != '\0' && !isdigit( *cp ) ) *(cp++) = '\0';

    bonus = cp;

    while ( isdigit( *cp ) ) ++cp;
    if ( *cp != '\0' ) *cp = '\0';

    if ( !( is_number( num ) && is_number( type ) && is_number( bonus ) ) )
    {
	send_to_char( syntax, ch );
	return FALSE;
    }

    if ( ( !is_number( num   ) || atoi( num   ) < 1 )
    ||   ( !is_number( type  ) || atoi( type  ) < 1 ) 
    ||   ( !is_number( bonus ) || atoi( bonus ) < 0 ) )
    {
	send_to_char( syntax, ch );
	return FALSE;
    }

    pMob->damage[DICE_NUMBER] = atoi( num   );
    pMob->damage[DICE_TYPE]   = atoi( type  );
    pMob->damage[DICE_BONUS]  = atoi( bonus );

    send_to_char( "Damdice set.\n\r", ch );
    return TRUE;
}


MEDIT( medit_race )
{
    MOB_INDEX_DATA *pMob;
    int race;

    if ( argument[0] != '\0'
    && ( race = race_lookup( argument ) ) != 0 )
    {
	EDIT_MOB( ch, pMob );

	pMob->race = race;
	pMob->act	  |= race_table[race].act;
	pMob->affected_by |= race_table[race].aff;
	pMob->off_flags   |= race_table[race].off;
	pMob->imm_flags   |= race_table[race].imm;
	pMob->res_flags   |= race_table[race].res;
	pMob->vuln_flags  |= race_table[race].vuln;
	pMob->form        |= race_table[race].form;
	pMob->parts       |= race_table[race].parts;
       pMob->affected2_by |= race_table[race].aff2;

	send_to_char( "Race set.\n\r", ch );
	return TRUE;
    }

    if ( argument[0] == '?' )
    {
	char buf[MAX_STRING_LENGTH];

	send_to_char( "Available races are:", ch );

	for ( race = 0; race_table[race].name != NULL; race++ )
	{
	    if ( ( race % 3 ) == 0 )
		send_to_char( "\n\r", ch );
	    sprintf( buf, " %-15s", race_table[race].name );
	    send_to_char( buf, ch );
	}

	send_to_char( "\n\r", ch );
	return FALSE;
    }

    send_to_char( "Syntax:  race [race]\n\r"
		  "Type 'race ?' for a list of races.\n\r", ch );
    return FALSE;
}


MEDIT( medit_position )
{
    MOB_INDEX_DATA *pMob;
    char arg[MAX_INPUT_LENGTH];
    int value;

    argument = one_argument( argument, arg );

    switch ( arg[0] )
    {
    default:
	break;

    case 'S':
    case 's':
	if ( str_prefix( arg, "start" ) )
	    break;

	if ( ( value = flag_value( position_flags, argument ) ) == NO_FLAG )
	    break;

	EDIT_MOB( ch, pMob );

	pMob->start_pos = value;
	send_to_char( "Start position set.\n\r", ch );
	return TRUE;

    case 'D':
    case 'd':
	if ( str_prefix( arg, "default" ) )
	    break;

	if ( ( value = flag_value( position_flags, argument ) ) == NO_FLAG )
	    break;

	EDIT_MOB( ch, pMob );

	pMob->default_pos = value;
	send_to_char( "Default position set.\n\r", ch );
	return TRUE;
    }

    send_to_char( "Syntax:  position [start/default] [position]\n\r"
		  "Type '? position' for a list of positions.\n\r", ch );
    return FALSE;
}


MEDIT( medit_gold )
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch, pMob);

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
	send_to_char( "Syntax:  wealth [number]\n\r", ch );
	return FALSE;
    }

    pMob->wealth = atoi( argument );

    send_to_char( "Wealth set.\n\r", ch);
    return TRUE;
}

MEDIT( medit_hitroll )
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch, pMob);

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
	send_to_char( "Syntax:  hitroll [number]\n\r", ch );
	return FALSE;
    }

    pMob->hitroll = atoi( argument );

    send_to_char( "Hitroll set.\n\r", ch);
    return TRUE;
}

void show_liqlist(CHAR_DATA *ch)
{
    int liq;
    BUFFER *buffer;
    char buf[MAX_STRING_LENGTH];
    
    buffer = new_buf();
    
    for ( liq = 0; liq_table[liq].liq_name != NULL; liq++)
    {
	if ( (liq % 21) == 0 )
	    add_buf(buffer,"Name                 Color          Proof Full Thirst Food Ssize\n\r");

	sprintf(buf, "%-20s %-14s %5d %4d %6d %4d %5d\n\r",
		liq_table[liq].liq_name,liq_table[liq].liq_color,
		liq_table[liq].liq_affect[0],liq_table[liq].liq_affect[1],
		liq_table[liq].liq_affect[2],liq_table[liq].liq_affect[3],
		liq_table[liq].liq_affect[4] );
	add_buf(buffer,buf);
    }

    page_to_char(buf_string(buffer),ch);
    free_buf(buffer);

return;
}

void show_mattype(CHAR_DATA *ch)
{
    int type;
    BUFFER *buffer;
    char buf[MAX_STRING_LENGTH];
    
	send_to_char("\n\r",ch);

    buffer = new_buf();
    
    for ( type = 0; material_table[type].name != NULL; type++)
    {
	if ( (type % 40) == 0 )
	    add_buf(buffer,"\n\r{c FLAG NAME{w:       {cDESCRIPTION{w:{x\n\r");
	sprintf(buf, "{r[{R%-15.15s{r][{W%-60.60s{r]{x\n\r",
		material_table[type].name,material_table[type].desc );
	add_buf(buffer,buf);
    }

    page_to_char(buf_string(buffer),ch);
    free_buf(buffer);

return;
}


void show_damlist(CHAR_DATA *ch)
{
    int att;
    BUFFER *buffer;
    char buf[MAX_STRING_LENGTH];
    
	send_to_char("\n\r",ch);

    buffer = new_buf();
    
    for ( att = 0; attack_table[att].name != NULL; att++)
    {
	if ( (att % 21) == 0 )
	    add_buf(buffer,"Name                 Noun\n\r");

	sprintf(buf, "%-20s %-20s\n\r",
		attack_table[att].name,attack_table[att].noun );
	add_buf(buffer,buf);
    }

    page_to_char(buf_string(buffer),ch);
    free_buf(buffer);

return;
}

MEDIT( medit_group )
{
    MOB_INDEX_DATA *pMob;
    MOB_INDEX_DATA *pMTemp;
    char arg[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int temp;
    BUFFER *buffer;
    bool found = FALSE;
    
    EDIT_MOB(ch, pMob);
    
    if ( argument[0] == '\0' )
    {
    	send_to_char( "Syntax: group [number]\n\r", ch);
    	send_to_char( "        group show [number]\n\r", ch);
    	return FALSE;
    }
    
    if (is_number(argument))
    {
	pMob->group = atoi(argument);
    	send_to_char( "Group set.\n\r", ch );
	return TRUE;
    }
    
    argument = one_argument( argument, arg );
    
    if ( !strcmp( arg, "show" ) && is_number( argument ) )
    {
	if (atoi(argument) == 0)
	{
		send_to_char( "Are you crazy?\n\r", ch);
		return FALSE;
	}

	buffer = new_buf ();

    	for (temp = 0; temp < 65536; temp++)
    	{
    		pMTemp = get_mob_index(temp);
    		if ( pMTemp && ( pMTemp->group == atoi(argument) ) )
    		{
			found = TRUE;
    			sprintf( buf, "[%5d] %s\n\r", pMTemp->vnum, pMTemp->player_name );
			add_buf( buffer, buf );
    		}
    	}

	if (found)
		page_to_char( buf_string(buffer), ch );
	else
		send_to_char( "No mobs in that group.\n\r", ch );

	free_buf( buffer );
        return FALSE;
    }
    
    return FALSE;
}

/*
REDIT( redit_owner )
{
    ROOM_INDEX_DATA *pRoom;

    EDIT_ROOM(ch, pRoom);

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax:  owner [owner]\n\r", ch );
	send_to_char( "         owner none\n\r", ch );
	return FALSE;
    }

    free_string( pRoom->owner );
    if (!str_cmp(argument, "none"))
    	pRoom->owner = str_dup("");
    else
	pRoom->owner = str_dup( argument );

    send_to_char( "Owner set.\n\r", ch );
    return TRUE;
}
*/

REDIT( redit_clanowner )
{
    ROOM_INDEX_DATA *pRoom;

    EDIT_ROOM(ch, pRoom);

    if ( argument[0] == '\0' || (!is_number(argument)))
    {
	send_to_char( "\n\r{GSYNTAX{w:  {WCLANOWNER {c<{WCLAN VNUM{c>{x\n\r", ch );
	send_to_char( "         {cor {WCLANOWNER {c<{W0{c> {D({CSets Clan Ownership to {WNONE{D){x\n\r", ch );
	return FALSE;
    }

    if (is_number(argument)
    && (atoi (argument) > 0 && atoi (argument) < 22))
       {
        pRoom->clanowner = atoi (argument);
        send_to_char( "\n\r{CCLAN Ownership Set.{x\n\r", ch );
        return TRUE;
       }


    if (is_number(argument) && (atoi (argument) == 0))
        pRoom->clanowner = atoi (argument);
        send_to_char( "\n\r{CCLAN Ownership Removed.{x\n\r", ch );
        return TRUE;

}

MEDIT ( medit_addmprog )
{
  int value;
  MOB_INDEX_DATA *pMob;
  MPROG_LIST *list;
  MPROG_CODE *code;
  char trigger[MAX_STRING_LENGTH];
  char phrase[MAX_STRING_LENGTH];
  char num[MAX_STRING_LENGTH];

  EDIT_MOB(ch, pMob);
  argument=one_argument(argument, num);
  argument=one_argument(argument, trigger);
  argument=one_argument(argument, phrase);

  if (!is_number(num) || trigger[0] =='\0' || phrase[0] =='\0' )
  {
        send_to_char("Syntax:   addmprog [vnum] [trigger] [phrase]\n\r",ch);
        return FALSE;
  }

  if ( (value = flag_value (mprog_flags, trigger) ) == NO_FLAG)
  {
        send_to_char("Valid flags are:\n\r",ch);
        show_help( ch, "mprog");
        return FALSE;
  }

  if ( ( code =get_mprog_index (atoi(num) ) ) == NULL)
  {
        send_to_char("No such MOBProgram.\n\r",ch);
        return FALSE;
  }

  list                  = new_mprog();
  list->vnum            = atoi(num);
  list->trig_type       = value;
  list->trig_phrase     = str_dup(phrase);
  list->code            = code->code;
  SET_BIT(pMob->mprog_flags,value);
  list->next            = pMob->mprogs;
  pMob->mprogs          = list;

  send_to_char( "Mprog Added.\n\r",ch);
  return TRUE;
}

MEDIT ( medit_delmprog )
{
    MOB_INDEX_DATA *pMob;
    MPROG_LIST *list;
    MPROG_LIST *list_next;
    char mprog[MAX_STRING_LENGTH];
    int value;
    int cnt = 0;

    EDIT_MOB(ch, pMob);

    one_argument( argument, mprog );
    if (!is_number( mprog ) || mprog[0] == '\0' )
    {
       send_to_char("Syntax:  delmprog [#mprog]\n\r",ch);
       return FALSE;
    }

    value = atoi ( mprog );

    if ( value < 0 )
    {
        send_to_char("Only non-negative mprog-numbers allowed.\n\r",ch);
        return FALSE;
    }

    if ( !(list= pMob->mprogs) )
    {
        send_to_char("MEdit:  Non existant mprog.\n\r",ch);
        return FALSE;
    }

    if ( value == 0 )
    {
	REMOVE_BIT(pMob->mprog_flags, pMob->mprogs->trig_type);
        list = pMob->mprogs;
        pMob->mprogs = list->next;
        free_mprog( list );
    }
    else
    {
        while ( (list_next = list->next) && (++cnt < value ) )
                list = list_next;

        if ( list_next )
        {
		REMOVE_BIT(pMob->mprog_flags, list_next->trig_type);
                list->next = list_next->next;
                free_mprog(list_next);
        }
        else
        {
                send_to_char("No such mprog.\n\r",ch);
                return FALSE;
        }
    }

    send_to_char("Mprog removed.\n\r", ch);
    return TRUE;
}

REDIT( redit_room )
{
	ROOM_INDEX_DATA *room;
	int value;

	EDIT_ROOM(ch, room);

	if ( (value = flag_value( room_flags, argument )) == NO_FLAG )
	{
		send_to_char( "Sintaxis: room [flags]\n\r", ch );
		return FALSE;
	}

        TOGGLE_BIT(room->room_flags, value);
	send_to_char( "Room flags toggled.\n\r", ch );
        return TRUE;
}

REDIT( redit_sector )
{
	ROOM_INDEX_DATA *room;
	int value;

	EDIT_ROOM(ch, room);

	if ( (value = flag_value( sector_flags, argument )) == NO_FLAG )
	{
		send_to_char( "Sintaxis: sector [tipo]\n\r", ch );
		return FALSE;
	}

	room->sector_type = value;
	send_to_char( "Sector type set.\n\r", ch );

	return TRUE;
}

/*
help editor by blade */
HEDIT( hedit_show )
{
	char buf[500];

	HELP_DATA *pHelp;
	EDIT_HELP(ch, pHelp);
	
	if (!pHelp) 
          {
        send_to_char( "\n\r{rYou're not currently editing a {RHELP{r!{x\n\r", ch );
        return FALSE;
          }

sprintf(buf, "\n\r{cKeyWords{w: {C%s{x\n\r{cLevel{w: {C%d{x\n\r", pHelp->keyword, pHelp->level); 
send_to_char( buf, ch);

send_to_char("\n\r{cHELP DESCRIPTION{w:{C\n\r", ch); 
send_to_char( pHelp->text, ch);
send_to_char("{x\n\r", ch);
return TRUE;
          }

HEDIT( hedit_level )
{
    char buf[MAX_STRING_LENGTH];

    HELP_DATA *pHelp;

    EDIT_HELP(ch, pHelp);

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
	send_to_char( "\n\r{GSyntax{w: {WLEVEL {c<{WMIN LEVEL TO SEE HELP FILE{c>{x\n\r", ch );
	return FALSE;
    }

        if (!pHelp)
          {
        send_to_char( "\n\r{rYou're not currently editing a {RHELP{r!{x\n\r", ch );
        return FALSE;
          }

    pHelp->level = atoi( argument );

    sprintf( buf,"\n\r{gLEVEL {W%d {gset for this {GHELP{g file.{x\n\r", pHelp->level);
    send_to_char(buf,ch);
    return TRUE;
}
/* reset keyword */


HEDIT( hedit_keyword )
{
    HELP_DATA *pHelp;
	HELP_DATA *tHelp;
 
    EDIT_HELP(ch, pHelp);


    if ( argument[0] == '\0' || is_number( argument ) )
      {
       send_to_char("\n\r{GSyntax{w: {WKEYWORD {c<{WWORD{D({Ws{D){c>{x\n\r", ch);
       return FALSE;
      }


    if ( !is_number( argument ) )    
      {
	for (tHelp = help_first; tHelp != NULL; tHelp = tHelp->next )
       {
	if (is_name(argument, tHelp->keyword))
          { 
           send_to_char("\n\r{rThat {WKEYWORD {ralready exists.{x\n\r", ch); 
           return FALSE; 
          }
       }

     pHelp->keyword = str_dup(argument);
	return TRUE;
     }
       return FALSE;
  }

/* mod current help text */
HEDIT( hedit_text )
{
   HELP_DATA *pHelp;
	
   EDIT_HELP(ch, pHelp);

   if ( argument[0] == '\0')
     {
      if (!pHelp)
        {
         send_to_char( "\n\r{rYou're not currently editing a {RHELP{r!{x\n\r", ch );
         return FALSE;
        }

      string_append( ch, &pHelp->text );	
      return TRUE;
     }
   else 
     {
      send_to_char("\n\r{GSyntax{w: {WTEXT {c<{WWITH NO ARGUMENT{c>{x\n\r", ch);
      return FALSE;
     }
}

/* kill a help */
HEDIT( hedit_delete )
{		
		extern int top_help;
	extern HELP_DATA *help_last;	
	HELP_DATA *target;
	HELP_DATA *previous;
	
	previous = NULL;

	
if (argument[0] == '\0')
	{
	send_to_char("\n\r{GSynatx{w: {WHEDIT DELETE {c<{WKEYWORD{c>{x\n\r", ch);
		return FALSE;
	}
for(target = help_first; target != NULL; target = target->next)
{
 if (is_name(argument, target->keyword))
 {
	 if (target == help_first)
	 {
		 previous = target->next;
		 help_first = previous;
		 top_help--;
		 send_to_char("\n\r{gTarget HELP listing {GREMOVED{g.{x\n\r", ch);
		 return TRUE;
	 }
	 else if (target == help_last)
	 {
  previous->next = NULL;
  help_last = previous;
  top_help--;
  send_to_char("\n\r{gTarget HELP listing {GREMOVED{g.{x\n\r", ch);
  return TRUE;
	 }

	 else {
  previous->next = target->next; 
  top_help--;
  send_to_char("\n\r{gTarget HELP listing {GREMOVED{g.{x\n\r", ch);
  return TRUE;
	 }
 }
 previous = target; /* set previous to last target */
}
send_to_char("\n\r{rNo HELP listing with that {GKEYWORD{x\n\r", ch);
return FALSE;
}

/* mod an existing help - throw into text editor */
HEDIT( hedit_change )
{
HELP_DATA *tHelp;

if (argument[0] == '\0')
	{
	send_to_char("\n\r{GSynatx{w: {WHEDIT CHANGE {c<{WKEYWORD{c>{x\n\r", ch);
		return FALSE;
	}
for (tHelp = help_first; tHelp != NULL; tHelp = tHelp->next )
   {
    if (is_name(argument, tHelp->keyword))
      { 
       send_to_char("\n\r{GHELP listing found. {g*{GACCESSING LINE EDITOR{g*{x\n\r", ch);
       send_to_char("{x000000000{W1{x111111111{W2{x222222222{W3{x333333333{W4{x444444444{W5{x555555555{W6{x666666666{W7{x777777777{W8{x\n\r", ch);
       send_to_char("{x123456789{W0{x123456789{W0{x123456789{W0{x123456789{W0{x123456789{W0{x123456789{W0{x123456789{W0{x123456789{W0{x\n\r", ch);


	ch->desc->pEdit		= (void *)tHelp;
 send_to_char( tHelp->keyword, ch);
 send_to_char("{x\n\r", ch);

string_append( ch, &tHelp->text );
return TRUE;
	}
}
send_to_char("\n\r{gNo HELP listing with that {GKEYWORD{g.\n\r",ch);  
send_to_char("\n\r{GType {c<{WHEDIT INDEX{c> {Gto list current HELPs{x\n\r",ch);
send_to_char("{g** {GTarget KEYWORD must match HELP listing exactly {g**{x\n\r", ch);
return FALSE;
}

/* list all helps by keyword */
HEDIT( hedit_index)
{
    HELP_DATA *pHelp;
	BUFFER *output;
    char buf[MAX_STRING_LENGTH];
    char ** keywordsSorted=0;
    char ** ar=0;
                
	output = new_buf();

    send_to_char("\n\r",ch);

    for ( pHelp = help_first; pHelp != NULL; pHelp = pHelp->next )
    {
        keywordsSorted = array_append(keywordsSorted, pHelp->keyword);
    }
    
    for(ar=keywordsSorted;ar && *ar;strtolower(*ar),ar++)
    ;
    
    array_sort(keywordsSorted);
    
    add_buf(output, "{g[{YLEVEL{g] [{WKEYWORDS{g]{x\n\r");
    
    for(ar=keywordsSorted;ar && *ar;ar++)
    {
        for(pHelp=help_first;pHelp!=NULL && strcasecmp(pHelp->keyword, *ar);pHelp=pHelp->next)
        ;
        
        sprintf(buf, "{g[{Y%5d{g] [{W%s{g]{x\n\r", pHelp->level, pHelp->keyword);
        add_buf(output, buf);
    }

    page_to_char(buf_string(output),ch);
    free_buf(output);   
    send_to_char("\n\r", ch);
    return TRUE;
}


/* create a new help */
HEDIT( hedit_create )
{
	extern int top_help;
	extern HELP_DATA *help_last;
	HELP_DATA *pHelp;
	HELP_DATA *tHelp;	

if (argument[0] == '\0')
	{
	send_to_char("\n\r{GSynatx{w: {WHEDIT CREATE {c<{WKEYWORD{c>{x\n\r", ch);
		return FALSE;
	}

for (tHelp = help_first; tHelp != NULL; tHelp = tHelp->next )
{
	if (is_name(argument, tHelp->keyword))
{ 
send_to_char("\n\r{rThat HELP listing already existS.{x\n\r", ch);
			   return FALSE; 
}
}
/* make the help in memory and initialize */

pHelp = new_help();
pHelp->level = 0;
pHelp->keyword = str_dup(argument);
if ( help_first == NULL )
	    help_first = pHelp;
	if ( help_last  != NULL )
	    help_last->next = pHelp;

	help_last	= pHelp;
	pHelp->next	= NULL;
	top_help++;
    
pHelp->text = str_dup(" ");

 ch->desc->pEdit		= (void *)pHelp;
 send_to_char( pHelp->keyword, ch);
 send_to_char("{x\n\r", ch);

string_append( ch, &pHelp->text );


	return TRUE;
}

/*
 * Clan Editor Functions.
 */
bool cedit_show( CHAR_DATA *ch, char *argument )
{
    CLAN_DATA *pClan;
    char buf[MAX_STRING_LENGTH];

    EDIT_CLAN(ch, pClan);

    sprintf( buf, "\n\r       {cClan{w: [{R%3d{w]\n\r", pClan->vnum );
    send_to_char( buf, ch );
    sprintf( buf, "       {cName{w: [{W%s{w]\n\r", pClan->name );
    send_to_char( buf, ch );
    sprintf( buf, "    {cWhoName{w: [{W%s{w]\n\r", pClan->who_name );
    send_to_char( buf, ch );
    sprintf( buf, " {cMORTLeader{w: [{W%s{w]\n\r", pClan->mort_leader );
    send_to_char( buf, ch );
    sprintf( buf, " {cLieutenant{w: [{W%s{w]\n\r", pClan->lieutenant );
    send_to_char( buf, ch );
    sprintf( buf, "{cRecall Room{w: [{R%5d{w] {W%s{x\n\r", pClan->hall,
    			 get_room_index( pClan->hall ) ? get_room_index( pClan->hall )->name
             											: "none" );
    send_to_char( buf, ch );
    sprintf( buf, "{cDescription{w:\n\r{W%s\n\r", pClan->description );
    send_to_char( buf, ch );

    return FALSE;
}

bool cedit_create( CHAR_DATA *ch, char *argument )
{
  CLAN_DATA *pClan;
  int  value;

  value = atoi( argument );

  /* OLC 1.1b */
  if ( argument[0] == '\0' || value <= 0 || value >= MAX_CLAN )
    {
  	char output[MAX_STRING_LENGTH];

  	sprintf( output, "Syntax:  cedit create [1 < vnum < %d]\n\r", MAX_CLAN );
  	send_to_char(  output, ch );
   	return FALSE;
    }


  if ( get_clan_index( value ) )
    {
  	send_to_char(  "CEdit:  Clan vnum already exists.\n\r", ch );
  	return FALSE;
    }

  pClan			= new_clan_index();
  pClan->vnum			= value;
  clan_sort(pClan);
  ch->desc->pEdit		= (void *)pClan;
  send_to_char(  "Clan Created.\n\r", ch );
  return TRUE;
}

bool cedit_name( CHAR_DATA *ch, char *argument )
{
    CLAN_DATA *pClan;

    EDIT_CLAN(ch, pClan);

    if ( argument[0] == '\0' )
      {
    	send_to_char(  "Syntax:  name [string]\n\r", ch );
    	return FALSE;
      }

    free_string( pClan->name );
    pClan->name = str_dup( argument );

    send_to_char(  "Name set.\n\r", ch);
    return TRUE;
}

bool cedit_whoname( CHAR_DATA *ch, char *argument )
{
    CLAN_DATA *pClan;

    EDIT_CLAN(ch, pClan);

    if ( argument[0] == '\0' )
      {
    	send_to_char(  "Syntax:  name [string]\n\r", ch );
    	return FALSE;
      }

    free_string( pClan->who_name );
    pClan->who_name = str_dup( argument );

    send_to_char(  "Who name set.\n\r", ch);
    return TRUE;
}

bool cedit_mortleader( CHAR_DATA *ch, char *argument )
{
    CLAN_DATA *pClan;

    EDIT_CLAN(ch, pClan);

    if ( argument[0] == '\0' )
      {
    	send_to_char(  "Syntax:  mortleader [string]\n\r", ch );
    	return FALSE;
      }

    free_string( pClan->mort_leader );
    pClan->mort_leader = str_dup( argument );

    send_to_char(  "Mortal Clan Leader Set.\n\r", ch);
    return TRUE;
}

bool cedit_lieutenant( CHAR_DATA *ch, char *argument )
{
    CLAN_DATA *pClan;

    EDIT_CLAN(ch, pClan);

    if ( argument[0] == '\0' )
      {
    	send_to_char(  "Syntax:  lieutenant [string]\n\r", ch );
    	return FALSE;
      }

    free_string( pClan->lieutenant );
    pClan->lieutenant = str_dup( argument );

    send_to_char(  "Mortal Clan Lieutenant Set.\n\r", ch);
    return TRUE;
}

bool cedit_desc( CHAR_DATA *ch, char *argument )
{
    CLAN_DATA *pClan;

    EDIT_CLAN(ch, pClan);

    if ( argument[0] == '\0' )
      {
    	string_append( ch, &pClan->description );
    	return TRUE;
      }

    send_to_char(  "Syntax:  desc    - line edit\n\r", ch );
    return FALSE;
}

bool cedit_recall( CHAR_DATA *ch, char *argument )
{
    CLAN_DATA *pClan;
    char room[MAX_STRING_LENGTH];
    int  value;

    EDIT_CLAN(ch, pClan);

    one_argument( argument, room );

    if ( !is_number( argument ) || argument[0] == '\0' )
    {
  	send_to_char(  "Syntax:  recall [#rvnum]\n\r", ch );
  	return FALSE;
    }

    value = atoi( room );

    if ( !get_room_index( value ) )
    {
  	send_to_char(  "CEdit:  Room vnum does not exist.\n\r", ch );
  	return FALSE;
    }

    pClan->hall = value;

    send_to_char(  "Hall set.\n\r", ch );
    return TRUE;
}

OEDIT( oedit_delete )
{
	OBJ_DATA *obj, *obj_next;
	OBJ_INDEX_DATA *pObj;
	RESET_DATA *pReset, *wReset;
	ROOM_INDEX_DATA *pRoom;
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	int index, rcount, ocount, i, iHash;

	if ( argument[0] == '\0' )
	{
		send_to_char( "Syntax:  oedit delete [vnum]\n\r", ch );
		return FALSE;
	}

	one_argument( argument, arg );

	if( is_number( arg ) )
	{
		index = atoi( arg );
		pObj = get_obj_index( index );
	}
	else
	{
		send_to_char( "That is not a number.\n\r", ch );
		return FALSE;
	}

	if( !pObj )
	{
		send_to_char( "No such object.\n\r", ch );
		return FALSE;
	}

	SET_BIT( pObj->area->area_flags, AREA_CHANGED );

	if( top_vnum_obj == index )
		for( i = 1; i < index; i++ )
			if( get_obj_index( i ) )
				top_vnum_obj = i;


	top_obj_index--;

	/* remove objects */
	ocount = 0;
	for( obj = object_list; obj; obj = obj_next )
	{
		obj_next = obj->next;

		if( obj->pIndexData == pObj )
		{
			extract_obj( obj );
			ocount++;
		}
	}

	/* crush resets */
	rcount = 0;
	for( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
	{
		for( pRoom = room_index_hash[iHash]; pRoom; pRoom = pRoom->next )
		{
			for( pReset = pRoom->reset_first; pReset; pReset = wReset
)
			{
				wReset = pReset->next;
				switch( pReset->command )
				{
				case 'O':
				case 'E':
				case 'P':
				case 'G':
					if( ( pReset->arg1 == index ) ||
						( ( pReset->command == 'P' ) && (
					pReset->arg3 == index ) ) )
					{
						unlink_reset( pRoom, pReset );
						free_reset_data( pReset );

						rcount++;
				SET_BIT( pRoom->area->area_flags,AREA_CHANGED );

					}
				}
			}
		}
	}

	unlink_obj_index( pObj );

	pObj->area = NULL;
	pObj->vnum = 0;

	free_obj_index( pObj );

        edit_done(ch);

	sprintf( buf, "Removed object vnum {C%d{x and"
		" {C%d{x resets.\n\r", index,rcount );

	send_to_char( buf, ch );

	sprintf( buf, "{C%d{x occurences of the object"
		" were extracted from the mud.\n\r", ocount );

	send_to_char( buf, ch );

	return TRUE;
}


MEDIT( medit_delete )
{
	CHAR_DATA *wch, *wnext;
	MOB_INDEX_DATA *pMob;
	RESET_DATA *pReset, *wReset;
	ROOM_INDEX_DATA *pRoom;
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	int index, mcount, rcount, iHash, i;
	bool foundmob = FALSE;
	bool foundobj = FALSE;

	if( argument[0] == '\0' )
	{
		send_to_char( "Syntax:  medit delete [vnum]\n\r", ch );
		return FALSE;
	}

	one_argument( argument, arg );

	if( is_number( arg ) )
	{
		index = atoi( arg );
		pMob = get_mob_index( index );
	}
	else
	{
		send_to_char( "That is not a number.\n\r", ch );
		return FALSE;
	}

	if( !pMob )
	{
		send_to_char( "No such mobile.\n\r", ch );
		return FALSE;
	}

	SET_BIT( pMob->area->area_flags, AREA_CHANGED );

	if( top_vnum_mob == index )
		for( i = 1; i < index; i++ )
			if( get_mob_index( i ) )
				top_vnum_mob = i;

	top_mob_index--;

	/* Now crush all resets and take out mobs while were at it */
	rcount = 0;
	mcount = 0;
	
	for( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
	{
		for( pRoom = room_index_hash[iHash]; pRoom; pRoom = pRoom->next )
		{

			for( wch = pRoom->people; wch; wch = wnext )
			{
				wnext = wch->next_in_room;
				if( wch->pIndexData == pMob )
				{
					extract_char( wch, TRUE );
					mcount++;
				}
			}

			for( pReset = pRoom->reset_first; pReset; pReset = wReset
)
			{
				wReset = pReset->next;
				switch( pReset->command )
				{
				case 'M':
					if( pReset->arg1 == index )
					{
						foundmob = TRUE;

						unlink_reset( pRoom, pReset );
						free_reset_data( pReset );

						rcount++;
				SET_BIT( pRoom->area->area_flags,AREA_CHANGED );

					}
					else
						foundmob = FALSE;

					break;
				case 'E':
				case 'G':
					if( foundmob )
					{
						foundobj = TRUE;

						unlink_reset( pRoom, pReset );
						free_reset_data( pReset );

						rcount++;
						SET_BIT( pRoom->area->area_flags,
						AREA_CHANGED );

					}
					else
						foundobj = FALSE;

					break;
				case '0':
					foundobj = FALSE;
					break;
				case 'P':
					if( foundobj && foundmob )
					{
						unlink_reset( pRoom, pReset );
						free_reset_data( pReset );

						rcount++;
						SET_BIT( pRoom->area->area_flags,
						AREA_CHANGED );
					}
				}
			}
		}
	}

	unlink_mob_index( pMob );

	pMob->area = NULL;
	pMob->vnum = 0;

	free_mob_index( pMob );
        edit_done(ch);
	sprintf( buf, "Removed mobile vnum {C%d{x and"
		" {C%d{x resets.\n\r", index,rcount );

	send_to_char( buf, ch );

	sprintf( buf, "{C%d{x mobiles were extracted"
		" from the mud.\n\r",mcount );
	send_to_char( buf, ch );

	return TRUE;
}

REDIT( redit_delete )
{
	ROOM_INDEX_DATA *pRoom, *pRoom2;
	RESET_DATA *pReset;
	EXIT_DATA *ex;
	OBJ_DATA *Obj, *obj_next;
	CHAR_DATA *wch, *wnext;
	EXTRA_DESCR_DATA *pExtra;
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	int index, i, iHash, rcount, ecount, mcount, ocount, edcount;

	if ( argument[0] == '\0' )
	{
		send_to_char( "Syntax:  redit delete [vnum]\n\r", ch );
		return FALSE;
	}

	one_argument( argument, arg );

	if( is_number( arg ) )
	{
		index = atoi( arg );
		pRoom = get_room_index( index );
	}
	else
		{
		send_to_char( "That is not a number.\n\r", ch );
		return FALSE;
	}

	if( !pRoom )
	{
		send_to_char( "No such room.\n\r", ch );
		return FALSE;
	}

	/* Move the player out of the room. */
	if( ch->in_room->vnum == index )
	{

		send_to_char( "Moving you out of the room"
			" you are deleting.\n\r", ch);
		if( ch->fighting != NULL )
			stop_fighting( ch, TRUE );

		char_from_room( ch );
		char_to_room( ch, get_room_index( 3 ) ); /* limbo */
		ch->was_in_room = ch->in_room;
/*
		ch->from_room = ch->in_room;
*/
	}

	SET_BIT( pRoom->area->area_flags, AREA_CHANGED );

	/* Count resets. They are freed by free_room_index. */
	rcount = 0;

	for( pReset = pRoom->reset_first; pReset; pReset = pReset->next )
	{
		rcount++;
	}

	/* Now contents */
	ocount = 0;
	for( Obj = pRoom->contents; Obj; Obj = obj_next )
	{
		obj_next = Obj->next_content;

		extract_obj( Obj );
		ocount++;
	}

	/* Now PCs and Mobs */
	mcount = 0;
	for( wch = pRoom->people; wch; wch = wnext )
	{
		wnext = wch->next_in_room;
		if( IS_NPC( wch ) )
		{
			extract_char( wch, TRUE );
			mcount++;
		}
		else
			{
			send_to_char( "This room is being deleted. Moving" 
				" you somewhere safe.\n\r", ch );
			if( wch->fighting != NULL )
				stop_fighting( wch, TRUE );

			char_from_room( wch );

			/* Midgaard Temple */
			char_to_room( wch, get_room_index( 3054 ) ); 
			wch->was_in_room = wch->in_room;
/*
			wch->from_room = wch->in_room;
*/
		}
	}

	/* unlink all exits to the room. */
	ecount = 0;
	for( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
	{
		for( pRoom2 = room_index_hash[iHash]; pRoom2; pRoom2 =
pRoom2->next )
		{
			for( i = 0; i <= MAX_DIR; i++ )
			{
				if( !( ex = pRoom2->exit[i] ) )
					continue;

				if( pRoom2 == pRoom )
				{
					/* these are freed by free_room_index */
					ecount++;
					continue;
				}

				if( ex->u1.to_room == pRoom )
				{
					free_exit( pRoom2->exit[i] );
					pRoom2->exit[i] = NULL;
			SET_BIT( pRoom2->area->area_flags,AREA_CHANGED );
					ecount++;
				}
			}
		}
	}

	/* count extra descs. they are freed by free_room_index */
	edcount = 0;
	for ( pExtra = pRoom->extra_descr; pExtra; pExtra = pExtra->next )
	{
		edcount++;
	}

	if( top_vnum_room == index )
		for( i = 1; i < index; i++ )
			if( get_room_index( i ) )
				top_vnum_room = i;

	top_room--;

	unlink_room_index( pRoom );

	pRoom->area = NULL;
	pRoom->vnum = 0;

	free_room_index( pRoom );

	/* Na na na na! Hey Hey Hey, Good Bye! */

        edit_done(ch);
	sprintf( buf, "Removed room vnum {C%d{x, %d resets, %d extra "
		"descriptions and %d exits.\n\r", index, rcount, edcount, ecount
);
	send_to_char( buf, ch );
	sprintf( buf, "{C%d{x objects and {C%d{x mobiles were extracted "
		"from the room.\n\r", ocount, mcount );
	send_to_char( buf, ch );

	return TRUE;
}
