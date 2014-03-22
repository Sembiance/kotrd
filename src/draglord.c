/* Added by Arioch */

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"

DECLARE_DO_FUN( do_say  );


CHAR_DATA * find_draglord args( ( CHAR_DATA *ch ) );
CHAR_DATA *find_draglord ( CHAR_DATA *ch )


{
    CHAR_DATA * draglord;

    for ( draglord = ch->in_room->people; draglord != NULL; draglord = draglord->next_in_room )
    {
        if (!IS_NPC(draglord))
	    continue;

        if (draglord->spec_fun == spec_lookup( "spec_draglord" ))
            return draglord;
    }

   if (draglord == NULL || draglord->spec_fun != spec_lookup( "spec_draglord" ))
   {
	send_to_char("\n\r{RYou can't do that here.\n\r{x",ch);
	return NULL;
   }

   if ( draglord->fighting != NULL)
   {
	send_to_char("\n\r{RWait until the fighting stops.{x\n\r",ch);
	return NULL;
   }

   return NULL;
}


void do_aquire(CHAR_DATA *ch, char *argument, void *vo)
{
    CHAR_DATA *draglord;
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    char arg[MAX_STRING_LENGTH];
    char arg1[MAX_STRING_LENGTH];
    int sn = 0;  

    argument = one_argument(argument, arg );
    argument = one_argument(argument, arg1);

    draglord = find_draglord (ch);

    if (!draglord)
        return;

    if ( ch->race != RACE_DRACONIAN )
       {
        send_to_char( "\n\r{RYou're no draconian.{x\n\r", ch );
        return;
       }

    if (arg[0] == '\0')
    {
        send_to_char("\n\r{CType 'AQUIRE LIST' to get a list of options.{x\n\r", ch);
	return;
    }

    if (!strcmp( arg, "list"))
    {
        send_to_char("\n\r{WThe differnt breath weapons you may choose from are:{x\n\r",ch);
send_to_char("{C********************************************{x",ch);
        send_to_char("\n\r  {WType      {Y: {WSyntax to use.{x\n\r",ch);
        send_to_char("  {RFire     {Y : {rbfire{x  \n\r",ch);
        send_to_char("  {MLightning {Y: {mblightning{x  \n\r",ch);
        send_to_char("  {dAcid      {Y: {Dbacid{x  \n\r",ch);
        send_to_char("  {GGas       {Y: {gbgas{x  \n\r",ch);
        send_to_char("  {BFrost     {Y: {bbfrost{x  \n\r",ch);
send_to_char("{C********************************************{x",ch);
        send_to_char("\n\r{GSyntax{Y:  {WAQUIRE <BREATH TYPE> <YOUR CHARCTER NAME>.{x\n\r",ch);
        send_to_char("         {Gi.e.{Y: {WAQUIRE FIRE ARIOCH.{x\n\r",ch);
        return;
    }
   
    if (!strcmp( arg, "fire"))
    {
	if (arg1[0] == '\0')
	{
          send_to_char( "\n\r{CYou must type in your CHARACTERs name.{x\n\r", ch );
          return;
	}

        if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
        {
          send_to_char( "{C\n\rYou must type in your CHARACTERs name.{x\n\r", ch );
          return;
        }

        if ( ch == victim )
         {
         ch = victim;
          sn = skill_lookup("bfire");
          if (sn > -1)

          {
           if ((ch->pcdata->learned[sn])
           || (ch->pcdata->learned[gsn_bacid])
           || (ch->pcdata->learned[gsn_blightning])
           || (ch->pcdata->learned[gsn_bfrost])
           || (ch->pcdata->learned[gsn_bgas]))

           {
            act("\n\r{Y$N tells you 'You already have a breath weapon!'{x\n\r",
            ch,NULL,draglord,TO_CHAR);
            return;
           }

           ch->pcdata->learned[sn] = 100;
           act("\n\r{C$N trains you in the ways of the dragon!{x\n\r",
           ch,skill_table[sn].name,draglord,TO_CHAR);
           ch->train -= skill_table[sn].rating[ch->class];
           return;
          }
         }
    }

    else
    if (!strcmp( arg, "lightning"))
    {
       if (arg1[0] == '\0')
        {  
          send_to_char( "\n\r{CYou must type in your CHARACTERs name.{x\n\r", ch );
          return;
        }
           
        if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
        {
          send_to_char( "{C\n\rYou must type in your CHARACTERs name.{x\n\r", ch );
          return;
        }
        if ( ch == victim )
         {
          ch = victim;
          sn = skill_lookup("blightning");
          if (sn > -1)
          {
           if ((ch->pcdata->learned[sn])
           || (ch->pcdata->learned[gsn_bacid])
           || (ch->pcdata->learned[gsn_bfire])
           || (ch->pcdata->learned[gsn_bfrost])
           || (ch->pcdata->learned[gsn_bgas]))
           {
            act("\n\r{Y$N tells you 'You already have a breath weapon!'{x\n\r",
            ch,NULL,draglord,TO_CHAR);
            return;
           }
           ch->pcdata->learned[sn] = 100;
           act("\n\r{C$N trains you in the ways of the dragon!{x\n\r",
           ch,skill_table[sn].name,draglord,TO_CHAR);
           ch->train -= skill_table[sn].rating[ch->class];
           return;
          }
         }
    }

    else
    if (!strcmp( arg, "acid"))
    {
       if (arg1[0] == '\0')
        {  
          send_to_char( "\n\r{CYou must type in your CHARACTERs name.{x\n\r", ch );
          return;
        }
           
        if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
        {
          send_to_char( "{C\n\rYou must type in your CHARACTERs name.{x\n\r", ch );
          return;
        }
        if ( ch == victim )
         {
          ch = victim;
          sn = skill_lookup("bacid");
          if (sn > -1)
          {
           if ((ch->pcdata->learned[sn])
           || (ch->pcdata->learned[gsn_bfire])
           || (ch->pcdata->learned[gsn_blightning])
           || (ch->pcdata->learned[gsn_bfrost])
           || (ch->pcdata->learned[gsn_bgas]))
           {
            act("\n\r{Y$N tells you 'You already have a breath weapon!'{x\n\r",
            ch,NULL,draglord,TO_CHAR);
            return;
           }
           ch->pcdata->learned[sn] = 100;
           act("\n\r{C$N trains you in the ways of the dragon!{x\n\r",
           ch,skill_table[sn].name,draglord,TO_CHAR);
           ch->train -= skill_table[sn].rating[ch->class];
           return;
          }
         }
    }

    else
    if (!strcmp( arg, "gas"))
    {
       if (arg1[0] == '\0')
        {  
          send_to_char( "\n\r{CYou must type in your CHARACTERs name.{x\n\r", ch );
          return;
        }
           
        if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
        {
          send_to_char( "{C\n\rYou must type in your CHARACTERs name.{x\n\r", ch );
          return;
        }
        if ( ch == victim )
         {
          ch = victim;
          sn = skill_lookup("bgas");
          if (sn > -1)
          {
           if ((ch->pcdata->learned[sn])
           || (ch->pcdata->learned[gsn_bacid])
           || (ch->pcdata->learned[gsn_blightning])
           || (ch->pcdata->learned[gsn_bfrost])
           || (ch->pcdata->learned[gsn_bfire]))
           {
            act("\n\r{Y$N tells you 'You already have a breath weapon!'{x\n\r",
            ch,NULL,draglord,TO_CHAR);
            return;
           }
           ch->pcdata->learned[sn] = 100;
           act("\n\r{C$N trains you in the ways of the dragon!{x\n\r",
           ch,skill_table[sn].name,draglord,TO_CHAR);
           ch->train -= skill_table[sn].rating[ch->class];
           return;
          }
         }
    }

    else
    if (!strcmp( arg, "frost"))
    {
       if (arg1[0] == '\0')
        {  
          send_to_char( "\n\r{CYou must type in your CHARACTERs name.{x\n\r", ch );
          return;
        }
           
        if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
        {
          send_to_char( "{C\n\rYou must type in your CHARACTERs name.{x\n\r", ch );
          return;
        }
        if ( ch == victim )
         {
          ch = victim;
          sn = skill_lookup("bfrost");
          if (sn > -1)
          {
           if ((ch->pcdata->learned[sn])
           || (ch->pcdata->learned[gsn_bacid])
           || (ch->pcdata->learned[gsn_blightning])
           || (ch->pcdata->learned[gsn_bfire])
           || (ch->pcdata->learned[gsn_bgas]))
           {
            act("\n\r{Y$N tells you 'You already have a breath weapon!'{x\n\r",
            ch,NULL,draglord,TO_CHAR);
            return;
           }
           ch->pcdata->learned[sn] = 100;
           act("\n\r{C$N trains you in the ways of the dragon!{x\n\r",
           ch,skill_table[sn].name,draglord,TO_CHAR);
           ch->train -= skill_table[sn].rating[ch->class];
           return;
          }
         }
    }
}
