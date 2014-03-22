#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "merc.h"

void do_marry( CHAR_DATA *ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH],arg2[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  CHAR_DATA *victim2;
    
  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
      {
      send_to_char( "Syntax: marry <char1> <char2>\n\r",ch);
      return;
      }
    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
      {
      send_to_char( "The first person mentioned isn't playing.\n\r", ch );
      return;
      }
    
    if ( ( victim2 = get_char_world( ch, arg2 ) ) == NULL )
      {
      send_to_char( "The second person mentioned isn't playing.\n\r", ch);
      return;
      }
    
    if ( IS_NPC(victim) || IS_NPC(victim2))
      {
      send_to_char("I don't think they want to be Married to the Mob.\n\r", ch);
      return;
      }        
    
    if (!IS_SET(victim->pact, PLR_CONSENT) || !IS_SET(victim2->pact, PLR_CONSENT))
    {
     send_to_char( "They do not give consent.\n\r", ch);
     return;
    }
    
    if (victim->pcdata->spouse > 0 || victim2->pcdata->spouse > 0)
    {
       send_to_char( "They are already married! \n\r", ch);
       return;
    }
   

    if (victim->level < 12 || victim2->level < 12)
      {
       send_to_char( "They are not of the proper level to marry.\n\r", ch);
       return;
      }
    
    send_to_char( "You pronounce them man and wife!\n\r", ch);
    send_to_char( "You say the big 'I do.'\n\r", victim);
    send_to_char( "You say the big 'I do.'\n\r", victim2);
    victim->pcdata->spouse  = str_dup( victim2->name );
    victim2->pcdata->spouse = str_dup( victim->name  );
    return;
}

void do_divorce( CHAR_DATA *ch, char *argument)
{

    char arg1[MAX_INPUT_LENGTH],arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *victim2;
    
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
        send_to_char( "Syntax: divorce <char1> <char2>\n\r",ch);
        return;
    }
    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
        send_to_char( "The first person mentioned isn't playing.\n\r", ch );
        return;
    }
    
    if ( ( victim2 = get_char_world( ch, arg2 ) ) == NULL )
    {
	send_to_char( "The second person mentioned isn't playing.\n\r", ch);
	return;
    }
    
    if ( IS_NPC(victim) || IS_NPC(victim2))
    {
     send_to_char("I don't think they're Married to the Mob...\n\r", ch);
     return;
    }
            
    if (!IS_SET(victim->pact, PLR_CONSENT) || !IS_SET(victim2->pact, PLR_CONSENT))
    {
     send_to_char( "They do not give consent.\n\r", ch);
     return;
    }
    
    if (victim->pcdata->spouse != victim2->name)
    {
     send_to_char( "They aren't even married!!\n\r",ch);
     return;
    }
       
    send_to_char( "You hand them their papers.\n\r", ch);
    send_to_char( "Your divorce is final.\n\r", victim);
    send_to_char( "Your divorce is final.\n\r", victim2);
    free_string( victim->pcdata->spouse );
    free_string( victim2->pcdata->spouse);
    return;
}

void do_consent( CHAR_DATA *ch )
{
   if (IS_NPC(ch))
    return;
    
   if ( IS_SET(ch->pact, PLR_CONSENT) )
   {
    send_to_char( "You no longer give consent.\n\r", ch);
    REMOVE_BIT(ch->pact, PLR_CONSENT);
    return;
   }
                           
   send_to_char( "You now give consent to Married!\n\r", ch);
   SET_BIT(ch->pact, PLR_CONSENT);
   return;
}

void do_spousetalk( CHAR_DATA *ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *victim;
 /* bool WiffleOrRage = ( !str_cmp( ch->name, "mindcry" ) || !str_cmp( ch->name, "venus" ) );*/

  if ( IS_NPC( ch ) )
    return;

	if(ch->pcdata->spouse == '\0')
  	{
		send_to_char("\n\r{RYou aren't married.{x\n\r",ch);
		return;
  	}

  if ( argument[0] == '\0')
    {
  	send_to_char( "\n\r{WTell your spouse what?{x\n\r", ch );
  	return;
    }

	if ( ( victim = get_char_world( ch, ch->pcdata->spouse ) ) == NULL )
    {
  	send_to_char( "\n\r{CYour spouse is not here.{x\n\r", ch );
  	return;
    }
	sprintf(buf, "\n\r{C[{WSpo{wuse{Rta{rlk{C]{w '{r%s{w'{x\n\r", argument);        send_to_char(buf, ch);     
send_to_char(buf, victim);
	return;
}

      /* sprintf(buf, "\n\r{RYou tell your %s '%s'\n\r{x", WiffleOrRage ? "{Wsoulmate{R" : "spouse", argument);
        send_to_char(buf,ch);
        sprintf(buf, "\n\r{RYour %s tells you '%s'{x\n\r",WiffleOrRage ? "{Wsoulmate{R" : "spouse", argument);
        send_to_char(buf, victim);
        return;
}*/



