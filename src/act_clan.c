#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "merc.h"
#include "tables.h"
#include "lookup.h"

/*
 *Local functions
 */
MEMBER_DATA *new_member       args( ( void ) );
void         free_member      args( ( MEMBER_DATA *pMember ) );
void         remove_member    args( ( CLAN_DATA *pClan, MEMBER_DATA *pMemOld ) );

CLAN_DATA *clan_free;
MEMBER_DATA *member_free;


void do_guild( CHAR_DATA *ch, char *argument )
{
  char arg1[MAX_INPUT_LENGTH],arg2[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *victim;
  CLAN_DATA *pClan;
  MEMBER_DATA *pMember;
  int iClan;
  int sn = 0;
 
  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );

   if (!IS_IMP(ch) && !IS_SET(ch->pact, PLR_CLAN_LEADER))   
     {
	  send_to_char( "\n\r{cHuh?\n\r", ch );
      return;
     }

  if ( arg1[0] == '\0' || arg2[0] == '\0' )
	  {
	send_to_char( "\n\r{GSyntax{w: {WGUILD {c<{WCHAR NAME{c> <{WCLAN NAME{c>{x\n\r",ch);
		return;
          }

      if ((victim = get_char_world(ch, arg1)) == NULL)
        {
         send_to_char( "\n\r{RThat player is not currently online.\n\r{x",ch);
         return;
        }            

      pClan = get_clan_index(victim->clan);
       

      if (pClan->members >= MAX_IN_CLAN )
        {
         send_to_char( "\n\r{RThis clan has reached its {Wlimit{R on members.\n\r{x",ch);
         return;
        }   
 
      if (victim->level < 20  
      || victim->level > 40)
        {          
         if (!IS_IMP(ch))
          {
         send_to_char("\n\r{WThe target must be from 20th to 40th level to join a clan.{x\n\r",ch);
         return;
          }
         }
     
      if (!IS_SET (victim->pact, PLR_PKILLER))
        {
         if (!IS_IMP(ch))
          {
         send_to_char("\n\r{WThe target must use the {GCHOOSE{W command before they can be clanned.{x\n\r", ch);
         return;
          }
       }

  if (!str_prefix(arg2,"none")) 
    {
     if (IS_IMP(ch))
       {
	  if ( victim->clan == 0 )
	    {
	    send_to_char( "\n\r{WThey are already clanless{c!{x\n\r", ch );
	    return;
	    }
  	send_to_char("\n\r{WThey are now clanless.{x\n\r",ch);
  	send_to_char("\n\r{WYou are now a member of {RNO {Wclan{c!{x\n\r",victim);
      REMOVE_BIT(victim->pact, PLR_CLAN_LEADER);
	  pClan = get_clan_index( victim->clan );
	  pClan->members--;
  	victim->clan = 0;
  	remove_member( pClan, victim->pcdata->member );
  	victim->pcdata->member = NULL;
	  save_clans( );
	  save_char_obj( victim );
  	return;
      }
    else
      {
       send_to_char( "\n\r{WThat is not a {RVALID{W Clan Choice.{x\n\r", ch );
       return;
      }
   }

    if ( victim->clan != 0)
    {
        if (ch->clan == victim->clan)
        {
            send_to_char("\n\r{WThey are already in your clan.{x\n\r", ch);
        }
        else
        {
            send_to_char("\n\r{RYOU MAY NOT REMOVE THEM FROM THEIR CLAN (do guild none first){x\n\r", ch);
        }

        return;
    }


  if ((iClan = clan_lookup(arg2)) == 0
  || !(pClan = get_clan_index( iClan )) )
    {
  	send_to_char("\n\r{RNO {Wsuch clan exists.{x\n\r",ch);
  	return;
    }

  if ( victim->clan != 0 )
    {
    sprintf( buf, "%s none", victim->name );
    do_guild( ch, buf );
    }

  if (!IS_IMP(ch) && ch->clan != iClan )
    {
        send_to_char("\n\r{RYou may only guild members into your clan.\n\r{x", ch);
        return;
    }

  if ( pClan->loners )
    {
    sprintf(buf,"\n\r{WThey are now {G%s{W.{x\n\r", capitalize( pClan->name ) );
  	send_to_char(buf,ch);
  	sprintf(buf,"\n\r{WYou are now {G%s{W.{x\n\r", capitalize( pClan->name ) );
  	send_to_char(buf,victim);
    }
  else
  {
  	sprintf(buf,"\n\r{cThey are now a member of {W%s{c.{x\n\r", capitalize( pClan->name ) );
  	send_to_char(buf,ch);
  	sprintf(buf,"\n\r{cYou are now a member of {W%s{c.{x\n\r", capitalize( pClan->name ) );
  }

  SET_BIT(victim->comm,COMM_CLANSHOW);
  sn = skill_lookup("clan recall");
  victim->pcdata->learned[sn] = 50;
  victim->clan = iClan;
  pClan->members++;
  if ( !( pMember = fread_pkinfo( victim->name )) )
    {
    pMember = new_member( );
    pMember->name = str_dup( victim->name );
    }
  sort_members( pClan, pMember );
  victim->clan = pClan->vnum;
  if(victim->pcdata == NULL)
{
	send_to_char("\n\r{RHuh{c?!! {WPCdata is {RNULL{W. {WWrite a note to {RMyserie{c!!{x\n\r", ch);
	return;
}
  victim->pcdata->member = pMember;
  save_clans( );
  save_char_obj( victim );
  return;
}

void sort_members( CLAN_DATA *pClan, MEMBER_DATA *pMemNew )
{
  pMemNew->next      = pClan->member_list;
  pClan->member_list = pMemNew;
  return;
}

void remove_member( CLAN_DATA *pClan, MEMBER_DATA *pMemOld )
{
  MEMBER_DATA *pMember;
  MEMBER_DATA *pMemPrv = NULL;
  for ( pMember = pClan->member_list; pMember; pMemPrv = pMember, pMember = pMember->next )
    {
    if ( pMember == pMemOld )
      {
      if ( !pMemPrv )
      	pClan->member_list = pMember->next;
      else
      	pMemPrv->next = pMember->next;
      free_member( pMember );
      break;
      }
    }
  return;
}

bool is_active_member( CHAR_DATA *ch, CLAN_DATA *pClan )
{
  MEMBER_DATA *pMember;
  for ( pMember = pClan->member_list; pMember; pMember = pMember->next )
    if ( !str_cmp( ch->name, pMember->name ) )
    	return TRUE;
  return FALSE;
}

void do_clans( CHAR_DATA *ch, char *argument )
{
  CLAN_DATA    *pClan;
  char          buf[MAX_STRING_LENGTH];
  char          arg1[MAX_INPUT_LENGTH];
  int           num;

  send_to_char("\n\r",ch);

  if ( clan_first == NULL )
    return;

  if ( argument[0] == '\0' )
    {
  if (IS_IMMORTAL(ch))
     {
    sprintf( buf,
      "{c[{C%2s{c] {D[{W%-20s{D] [{W%2s{D] [   {W%6s{D    ] [   {W%6s{D   ]\n\r"
      " {W%2s{D   {W%20s{D   {W%2s{D  [{W%-5s{D   {W%5s{D] [{W%-5s{D   {W%5s{D]{x\n\r",
      "Nm", "Clan Name", " #", "PKills", "PDeaths",
      "","","","Below", "Above", "Below", "Above" );
      send_to_char( buf, ch );
    for ( pClan = clan_first->next; pClan; pClan = pClan->next )
    	{
    	sprintf( buf,
        "{c[{C%2d{c] {D[{W%-20s{D] [{W%2d{D] [ {r%3d {D] [ {R%3d {D] [ {R%3d {D] [ {r%3d {D]\n\r",
	     pClan->vnum,
	     capitalize( pClan->name ),
	     pClan->members,
	     pClan->pks_dwn,
	     pClan->pks_up,
	     pClan->pkd_dwn,
	     pClan->pkd_up );
    	send_to_char(buf, ch );
        }
      }
    else
      {
    sprintf( buf,
      "[{W%-20s{D] [{W%2s{D] [   {W%6s{D    ] [   {W%6s{D   ]\n\r"
      "{W%20s{D   {W%2s{D   [{W%-5s{D   {W%5s{D] [{W%-5s{D   {W%5s{D]\n\r",
      "Clan Name", " #", "PKills", "PDeaths",
      "","","Below", "Above", "Below", "Above" );
      send_to_char( buf, ch );
    for ( pClan = clan_first->next; pClan; pClan = pClan->next )
       {
    	sprintf( buf,
        "[{W%-20s{D] [{W%2d{D] [ {r%3d {D] [ {R%3d {D] [ {R%3d {D] [ {r%3d {D]\n\r",
	     capitalize( pClan->name ),
	     pClan->members,
	     pClan->pks_dwn,
	     pClan->pks_up,
	     pClan->pkd_dwn,
	     pClan->pkd_up );
    	send_to_char(buf, ch );
       }




      }
    return;
    }


  one_argument( argument, arg1 );
  num = is_number( arg1 ) ? atoi( arg1 ) : -1;
  if ( num == -1 )
    num = clan_lookup( arg1 );
  if ( num == -1 || !(pClan = get_clan_index(num)))
    {
    do_clans( ch, "" );
    return;
    }

  sprintf( buf, "{WInformation on {D- {R%s{D -{x\n\r\n\r", pClan->name );
  send_to_char( buf, ch );
  sprintf( buf, "{cDescription{w:\n\r{W%s\n\r", pClan->description );
  send_to_char( buf, ch );

  sprintf( buf, "  {cMortal Leader{w: {W%s\n\r", pClan->mort_leader );
  send_to_char( buf, ch );
  sprintf( buf, "     {cLieutenant{w: {W%s\n\r{x", pClan->lieutenant );
  send_to_char( buf, ch );


  if (ch->level < ASSTIMP)
    {
     if (num != 8)
       {
       if (ch->clan != num)
         {
          send_to_char("\n\r{WYou may only see the members of YOUR clan.{x\n\r",ch);
    	  return;
         }
       }
    }

  if ( pClan->member_list )
    {
    MEMBER_DATA *pMember1;
    MEMBER_DATA *pMember2 = NULL;
    char buf[ MAX_STRING_LENGTH ];
    int iMaxNum;
    int iHlfNum;
    int iCtr;
    for ( iMaxNum = 1, pMember1 = pClan->member_list;
          pMember1;
          pMember1 = pMember1->next, iMaxNum++ );
    iHlfNum = ( iMaxNum / 2 ) + ( iMaxNum % 2 != 0 );
    for ( iCtr = 1, pMember1 = pClan->member_list;
          iCtr < iHlfNum;
          pMember1 = pMember1->next, iCtr++ );
    pMember2 = pMember1->next;
    send_to_char( "\n\r{cActive members{w:\n\r", ch );
    send_to_char( "{D[{WMember Name  PKills  PDeaths{D] [{WMember Name  PKills  PDeaths{D]\n\r"
                  "             {D[{WLow Hgh Low Hgh{D]              {D[{WLow Hgh Low Hgh{D]\n\r{x", ch );
    for ( pMember1 = pClan->member_list, iCtr = 0; iCtr < iHlfNum; iCtr++ )
    	{
    	sprintf( buf, "{D[{C%-12s {R%3d {r%3d {R%3d {r%3d{D] ",
    	  pMember1->name, pMember1->pks_dwn, pMember1->pks_up, pMember1->pkd_dwn, pMember1->pkd_up );
    	pMember1 = pMember1->next;
    	if ( pMember2 )
    	  {
      	sprintf( buf + strlen( buf ), "{D[{C%-12s {R%3d {r%3d {R%3d {r%3d{D] ",
    	  pMember2->name, pMember2->pks_dwn, pMember2->pks_up, pMember2->pkd_dwn, pMember2->pkd_up );
	      pMember2 = pMember2->next;
        }
	    strcat( buf, "\n\r{x" );
    	send_to_char( buf, ch );
    	}
    }

  return;
}

void save_clans( void )
{
  FILE *fp;
  CLAN_DATA *pClan;

  if ( !(fp = fopen( CLAN_FILE, "w" )) )
    {
    bug( "Save_clans: fopen", 0 );
    perror( CLAN_FILE );
    }
  else
    {
  	for ( pClan = clan_first; pClan; pClan = pClan->next )
	    {
  	  fprintf( fp, "#%d\n",      pClan->vnum );
  	  fprintf( fp, "%d\n",       pClan->loners );
  	  fprintf( fp, "%s~ ",       pClan->name );
  	  fprintf( fp, "%s~\n",      pClan->who_name );
  	  fprintf( fp, "%s~ ",       pClan->mort_leader );
  	  fprintf( fp, "%s~\n",      pClan->lieutenant );
  	  fprintf( fp, "%s~\n",      pClan->description );
  	  fprintf( fp, "%d\n",       pClan->hall );
  	  fprintf( fp, "%d %d ",     pClan->pks_up, pClan->pks_dwn );
  	  fprintf( fp, "%d %d\n",    pClan->pkd_up, pClan->pkd_dwn );
  	  fprintf( fp, "%d\n",       pClan->members );
  	  if ( pClan->member_list )
  	    {
  	    MEMBER_DATA *pMember;
  	    for ( pMember = pClan->member_list; pMember; pMember = pMember->next )
  	      fprintf( fp, " %s", pMember->name );
  	    }
  	  fprintf( fp, "~\n" );
	    }
  	fprintf( fp, "#999\n\n" );
  	fclose( fp );
    }
  return;
}

MEMBER_DATA *new_member( void )
{
    MEMBER_DATA *pMember;
    if ( !member_free )
      pMember = alloc_perm( sizeof( *pMember ) );
    else
      {
      pMember = member_free;
      member_free = member_free->next;
      }
    pMember->next = NULL;
    pMember->name = str_dup( "" );
    pMember->pks_up   = 0;
    pMember->pks_dwn  = 0;
    pMember->pkd_up   = 0;
    pMember->pkd_dwn  = 0;
    return pMember;
}
void free_member( MEMBER_DATA *pMember )
{
  free_string( pMember->name );
  pMember->next = member_free;
  member_free = pMember;
  return;
}
CLAN_DATA *new_clan_index( void )
{
  CLAN_DATA *pClan;

  if ( !clan_free )
    pClan		= alloc_perm( sizeof(*pClan) );
  else
    {
  	pClan     	=   clan_free;
  	clan_free 	=   clan_free->next;
    }

  pClan->next		=   NULL;
  pClan->member_list	=   NULL;
  pClan->who_name       =   &str_empty[0];
  pClan->name     	=   &str_empty[0];
  pClan->mort_leader    =   &str_empty[0];
  pClan->lieutenant     =   &str_empty[0];
  pClan->description	=   &str_empty[0];
  pClan->vnum	      	=   0;
  pClan->hall       	=   ROOM_VNUM_TEMPLE;
  pClan->members	=   0;
  pClan->pks_up         =   0;
  pClan->pks_dwn    	=   0;
  pClan->pkd_up         =   0;
  pClan->pkd_dwn        =   0;
  return pClan;
}

void free_clan_index( CLAN_DATA *pClan )
{
   pClan->next	= clan_free;
   clan_free	= pClan;
   free_string( pClan->name );
   free_string( pClan->who_name );
   free_string( pClan->mort_leader );
   free_string( pClan->lieutenant );
   free_string( pClan->description );
   pClan->member_list = NULL;
   return;
}


void clan_sort( CLAN_DATA *pClan )
{
  CLAN_DATA *fClan;

  if ( !clan_first )
  {
    clan_first = pClan;
    return;
  }
  for ( fClan = clan_first; fClan; fClan = fClan->next )
  {
    if ( pClan->vnum == fClan->vnum ||
       ( pClan->vnum > fClan->vnum &&
       (!fClan->next || pClan->vnum < fClan->next->vnum) ) )
    {
      pClan->next = fClan->next;
      fClan->next = pClan;
      return;
    }
  }
  pClan->next = clan_first;
  clan_first = pClan;
  return;
}
void save_pkinfo( MEMBER_DATA *pMember )
{
  FILE *fp;
  char buf[ MAX_STRING_LENGTH ];
  
  sprintf( buf, "%s%s.pk", PKINFO_DIR, capitalize( pMember->name ) );
  if ( !(fp = fopen( buf, "w" )) )
    {
    bug( "Save_PkInfo: Cannot open '%s'", (int)buf );
    return;
    }
  fprintf( fp, "%d %d %d %d\n", pMember->pks_up, pMember->pks_dwn,
                                pMember->pkd_up, pMember->pkd_dwn );
  fclose( fp );
  return;                                
}
MEMBER_DATA *fread_pkinfo( char *name )
{
  FILE *fp;
  MEMBER_DATA *pMemNew;
  char buf[ MAX_STRING_LENGTH ];
  
  sprintf( buf, "%s%s.pk", PKINFO_DIR, capitalize( name ) );
  if ( !( fp = fopen( buf, "r" )) )
    return NULL;
  pMemNew = new_member( );
  pMemNew->name     = str_dup( capitalize( name ) );
  pMemNew->pks_up   = fread_number( fp );
  pMemNew->pks_dwn  = fread_number( fp );
  pMemNew->pkd_up   = fread_number( fp );
  pMemNew->pkd_dwn  = fread_number( fp );
  return pMemNew;
}

MEMBER_DATA *get_member_clan( CHAR_DATA *ch, CLAN_DATA *pClan )
{
  MEMBER_DATA *pMember;
  for ( pMember = pClan->member_list; pMember; pMember = pMember->next )
    if ( !str_cmp( pMember->name, ch->name ) )
      break;
  return pMember;
}
