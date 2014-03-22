#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"
#include "recycle.h"

DECLARE_DO_FUN( do_help		);

void do_account (CHAR_DATA *ch, char *argument)
{
   long gold = 0, silver = 0, shares = 0;
   int diff, bonus = 0;
   char buf[MAX_STRING_LENGTH];
    
    if(!ch->pcdata)
     {
      send_to_char("Only players need money!\n\r", ch);
      return;
     }
        
   gold = ch->pcdata->gold_bank;
   silver = ch->pcdata->silver_bank;
   shares = ch->pcdata->shares;
   diff = (SHARE_MAX - ch->pcdata->shares_bought);
   if( ch->pcdata->shares / 4 != 0)
      bonus = shares / 4;
   ch->pcdata->duration = bonus;

   if( (IS_NPC(ch) 
   || IS_SET(ch->act,ACT_PET)) )
     {
      send_to_char("Only players need money!\n\r", ch);
      return;
     }

/*
   if ((IS_CLASS(ch, CLASS_THIEF))
   || (IS_CLASS(ch, CLASS_ASSASSIN)))
*/
   if (ch->class == 3 || ch->class == 4)
   {
      sprintf( buf,"\n\r{DThe Guild's '{wTome of Profits{D' reveals{w:\n\r"
		   "{wYour BeltPouch Contains{D:    {wHidden within the Guild is{D:\n\r"
                   "    {YGold{w: {Y%-10ld     {W&       {YGold{w: {Y%-10ld\n\r"
		   "  {WSilver{w: {W%-10ld           {WSilver{w: {W%-10ld{x\n\r",
ch->gold,gold,ch->silver,silver); 
   }
   else
   {
      sprintf( buf,"\n\r{cThe Midgaard Account Records Show{w:\n\r"
		   "{CYour BeltPouch Contains{w:    {CDeposited in your Bank is{w:\n\r"
                   "    {YGold{w: {Y%-10ld     {W&      {YGold{w: {Y%-10ld\n\r"
		   "  {WSilver{w: {W%-10ld          {WSilver{w: {W%-10ld{x\n\r",
ch->gold,gold,ch->silver,silver); 
   }
   send_to_char(buf, ch);

   if ( ch->level >= 500 
   && (ch->class != 3 || ch->class != 4)) 
/*   && (!IS_CLASS(ch, CLASS_THIEF))) */
   {
      sprintf( buf,"\n\r{cSHAREs{w: {W%-3ld {Cwith a {c+{W%d {CHours of Spell Duration{c.{x\n\r",
shares,bonus); 
      send_to_char(buf, ch);
      sprintf( buf,"{cSHAREs Still Available at Level {C%d{w: {W%d{c.{x\n\r",ch->level, diff);
      send_to_char(buf, ch);
   }
   return;
}


void do_deposit (CHAR_DATA *ch, char *argument)
{
   long amount = 0; 
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];

   /* No NPC's, No pets, No imms,
    * No chainmail, No service! 
    */

 if (!IS_IMP(ch))
   {
    if( (IS_NPC(ch) || IS_SET(ch->act,ACT_PET)) 
        || (IS_IMMORTAL(ch)) )
      {
       send_to_char("\n\r{RONLY Players need money!{x\n\r", ch);
       return;
      }
   }

   if(ch->in_room != get_room_index(ROOM_VNUM_BANK)
   && (ch->class != 3) 
   && (ch->class != 4) )
/*
   && (!IS_CLASS(ch, CLASS_THIEF)) 
   && (!IS_CLASS(ch, CLASS_ASSASSIN)) )
*/
   {
      send_to_char("\n\r{MYou must be in the MIDGAARD BANK to make deposits!{x\n\r",ch);
      return;
   }
   else 
     if(ch->in_room != get_room_index(ROOM_VNUM_BANK_THIEF)
     && ((ch->class == 3) 
     ||  (ch->class == 4)) )

/*
     && ((IS_CLASS(ch, CLASS_THIEF)) 
     ||  (IS_CLASS(ch, CLASS_ASSASSIN))) )
*/
       {
         send_to_char("\n\r{RYou MUST be at your Guild's Treasury to make deposits!{x\n\r", ch);
         return;
       }
    else
     {
      argument = one_argument( argument, arg1 );
      argument = one_argument( argument, arg2 );
      if (arg1[0] == '\0' || arg2[0] == '\0')
      {

     if ((ch->class == 3) 
     ||  (ch->class == 4)) 
/*
         if((IS_CLASS(ch, CLASS_THIEF))
         || (IS_CLASS(ch, CLASS_ASSASSIN)))
*/
           {
            send_to_char("\n\r{CHow much do you wish to stash at your Guild?{x\n\r", ch );

            send_to_char("\n\r{GSyntax{w: {WDEPOSIT {c<{WAMOUNT{c> {WGOLD\n\r",ch);
            send_to_char("        {WDEPOSIT {c<{WAMOUNT{c> {WSILVER{x\n\r",ch);
           }
         else
           {
            send_to_char("\n\r{CHow much do you wish to DEPOSIT into your Savings Account?{x\n\r", ch );

            send_to_char("\n\r{GSyntax{w: {WDEPOSIT {c<{WAMOUNT{c> {WGOLD\n\r",ch);
            send_to_char("        {WDEPOSIT {c<{WAMOUNT{c> {WSILVER{x\n\r",ch);
           }
         return;
      }

      if(ch->in_room == get_room_index(ROOM_VNUM_BANK))
      {
       if ((ch->class == 3) 
       ||  (ch->class == 4)) 
/*
         if((IS_CLASS(ch, CLASS_THIEF))
         || (IS_CLASS(ch, CLASS_ASSASSIN)))
*/
         {
            act("\n\r{MA CutThroat lurks in the shadows here!{x\n\r", ch,NULL,NULL, TO_ROOM);
            send_to_char(
"\n\r{YThe Banker tells you 'You aren't allowed in here, you'd better go'!{x\n\r",ch); 
            return;
         }
         else
         {
            if( is_number( arg1 ) )
            {
	       amount = atoi(arg1);

	       if ( amount <= 0 )
               {
	          send_to_char( 
"\n\r{YThe Banker tells you 'To DEPOSIT you must GIVE money!'{x\n\r", ch );
	          return;
	       }
               if(!str_cmp( arg2, "gold")) 
               {
                  if (ch->gold < amount)
                  {
	             send_to_char(
"\n\r{YThe Banker tells you 'You don't have that much GOLD!'{x\n\r",ch);
	             return;
                  }
                  else 
                  {
                     ch->pcdata->gold_bank += amount;
                     ch->gold -= amount;
                     act("\n\r{W$n {cDEPOSITs {YGOLD {cinto $s Savings Account.{x\n\r", 
ch,NULL,NULL,TO_ROOM);
       sprintf( buf, 
"\n\r{YThe Banker tells you 'You have deposited %ld Gold.'{x\n\r"
"   {cTotals{w: {CBeltPouch{w: {W%-10ld  {CAccount{w: {W%-10ld{x\n\r",
                              amount, ch->gold, ch->pcdata->gold_bank);
                     send_to_char( buf, ch);
                     return;
                  }
               }  
               if(!str_cmp( arg2, "silver")) 
               {
                  if (ch->silver < amount)
                    {
	             send_to_char(
"\n\r{YThe Banker tells you 'You don't have that much {WSILVER{Y!'{x\n\r",ch);
	             return;
                    }
                  else 
                  {
                     ch->pcdata->silver_bank += amount;
                     ch->silver -= amount;
                     act("\n\r{W$n {cDEPOSITs {WSILVER {cinto $s Savings Account.{x\n\r", 
ch,NULL,NULL,TO_ROOM);
       sprintf( buf, 
"\n\r{YThe Banker tells you 'You have deposited {W%ld Silver{Y.'\n\r"
"   {cTotals{w: {CBeltPouch{w: {W%-10ld  {CAccount{w: {W%-10ld{x\n\r",
                              amount, ch->silver, ch->pcdata->silver_bank);
                     send_to_char( buf, ch);
                     return;
                  }
               }  
            }
         }
      }
      else 
      if(ch->in_room == get_room_index(ROOM_VNUM_BANK_THIEF))
      {
       if ((ch->class != 3) 
       &&  (ch->class != 4)) 
/*
         if((!IS_CLASS(ch, CLASS_THIEF))
         && (!IS_CLASS(ch, CLASS_ASSASSIN)))
*/
         {
            act("\n\r{RA Do-Gooder is here to kill the GuildMaster.{x\n\r", ch,NULL,NULL, TO_ROOM);
            send_to_char("\n\r{GYou aren't allowed in here, you'd better go!{x\n\r",ch); 
            return;
         }
         else
         {
            if( is_number( arg1 ) )
            {
	       amount = atoi(arg1);

	       if ( amount <= 0 )
               {
	             send_to_char(
"\n\r{YThe Guild Accountant tells you 'To DEPOSIT you must GIVE money!'{x\n\r", ch );
	          return;
	       }
               if(!str_cmp( arg2, "gold")) 
               {
                  if (ch->gold < amount)
                  {
	             send_to_char(
"\n\r{YThe Guild Accountant tells you 'You don't have that much GOLD!'{x\n\r",ch);
	             return;
                  }
                  else 
                  {
                     ch->pcdata->gold_bank += amount;
                     ch->gold -= amount;
                     act(
"\n\r{W$n {cstashes {YGOLD {cin a secret compartment.{x\n\r", ch,NULL,NULL,TO_ROOM);
       sprintf( buf, 
"\n\r{YThe Guild Accountant tells you 'You have deposited %ld Gold.'{x\n\r"
"   {cTotals{w: {CBeltPouch{w: {W%-10ld  {CAccount{w: {W%-10ld{x\n\r",
                              amount, ch->gold, ch->pcdata->gold_bank);
                     send_to_char( buf, ch);
                     return;
                  }
               }  

               if(!str_cmp( arg2, "silver")) 
               {
                  if (ch->silver < amount)
                  {
	             send_to_char(
"\n\r{YThe Guild Accountant tells you 'You don't have that much {WSILVER{Y!'{x\n\r",ch);
	             return;
                  }
                  else 
                  {
                     ch->pcdata->silver_bank += amount;
                     ch->silver -= amount;
                     act(
"\n\r{W$n {cstashes {WSILVER {cin a secret compartment.{x\n\r", ch,NULL,NULL,TO_ROOM);
       sprintf( buf, 
"\n\r{YThe Banker tells you 'You have deposited {W%ld Silver{Y.'\n\r"
"   {cTotals{w: {CBeltPouch{w: {W%-10ld  {CAccount{w: {W%-10ld{x\n\r",
                              amount, ch->silver, ch->pcdata->silver_bank);
                     send_to_char( buf, ch);
                     return;
                  }
               }  
            }
            else
            {
               send_to_char("\n\r{RThe TYPE of currency MUST be stated AFTER the AMOUNT.{x\n\r",ch);
            }
         }
      }
      else
      {
         bug( "Do_deposit: Bank doesn't exist.", 0 );
         send_to_char( "\n\r{RBank IS missing... Please NOTE Arioch ASAP!{x\n\r", ch );
         return;
      } 
   }
   return;
}


void do_withdraw (CHAR_DATA *ch, char *argument)
{
   long amount = 0; 
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];

   /* No NPC's, No pets, No imms,
    * No chainmail, No service! 
    */

 if(!IS_IMP(ch))
   {
    if( (IS_NPC(ch) || IS_SET(ch->act,ACT_PET)) 
    || (IS_IMMORTAL(ch)) )
      {
       send_to_char("\n\r{RONLY Mortals Players need money!{x\n\r", ch);
       return;
      }
   }

   if(ch->in_room != get_room_index(ROOM_VNUM_BANK)
   && (ch->class != 3) 
   && (ch->class != 4)) 
/*
   && (!IS_CLASS(ch, CLASS_THIEF))
   && (!IS_CLASS(ch, CLASS_ASSASSIN)) )
*/
   {   
      send_to_char("\n\r{MYou must be in the MIDGAARD BANK to make deposits!{x\n\r",ch);
      return;
   }
   else 
     if(ch->in_room != get_room_index(ROOM_VNUM_BANK_THIEF)
     &&((ch->class == 3) 
     || (ch->class == 4)))
/*
     && ((IS_CLASS(ch, CLASS_THIEF))
     ||  (IS_CLASS(ch, CLASS_ASSASSIN))) )
*/
       {
         send_to_char("\n\r{RYou MUST be at your Guild's Treasury to make deposits!{x\n\r", ch);
         return;
       }
   else  
   {
      argument = one_argument( argument, arg1 );
      argument = one_argument( argument, arg2 );
      if (arg1[0] == '\0'|| arg2[0] == '\0')
      {
       if((ch->class == 3) 
       || (ch->class == 4))
         {
          send_to_char(
"\n\r{YThe Guild Accountant asks you 'How much you needin' to withdraw?'{x\n\r", ch );

          send_to_char("\n\r{GSyntax{w: {WWITHDRAW {c<{WAMOUNT{c> {WGOLD {cor {WSILVER{x\n\r",ch);
         }
         else
         {
          send_to_char(
"\n\r{YThe Banker asks you 'How much do you wish to withdraw from your savings?'{x\n\r", ch );

          send_to_char("\n\r{GSyntax{w: {WWITHDRAW {c<{WAMOUNT{c> {WGOLD {cor {WSILVER{x\n\r",ch);
         }
         return;
      }

      if(ch->in_room == get_room_index(ROOM_VNUM_BANK))
      {
       if((ch->class == 3) 
       || (ch->class == 4))
         {
            act("\n\r{RA CutThroat lurks in the shadows here.{x\n\r", ch,NULL,NULL, TO_ROOM);
            send_to_char("\n\r{GYou aren't allowed in here, you'd better go!{x\n\r",ch); 
            return;
         }
         else
         {
            if( is_number( arg1 ) )
            {
	       amount = atoi(arg1);

	       if ( amount <= 0 )
             {
	          send_to_char( 
"\n\r{YThe Banker tells you 'To make a WITHDRAWAL, you must specify an AMOUNT of money!'{x\n\r", ch );
	          return;

	          return;
	       }
               if(!str_cmp( arg2, "gold")) 
               {
                  if (ch->pcdata->gold_bank < amount)
                  {
	             send_to_char(
"\n\r{YThe Banker tells you 'You don't have that much GOLD in your Savings Account!'{x\n\r",ch);
	             return;
                  }
                  else 
                  {
                     ch->pcdata->gold_bank -= amount;
                     ch->gold += amount;

                     act("\n\r{W$n {cWITHDRAWs {YGOLD {cfrom $s Savings Account.{x\n\r", 
ch,NULL,NULL,TO_ROOM);
       sprintf( buf, 
"\n\r{YThe Banker tells you 'You have withdrawn %ld Gold{Y.'\n\r"
"   {cTotals{w: {CBeltPouch{w: {W%-10ld  {CAccount{w: {W%-10ld{x\n\r",
                              amount, ch->gold, ch->pcdata->gold_bank);
                     send_to_char( buf, ch);
                     return;
                  }
               }  
               if(!str_cmp( arg2, "silver")) 
               {
                  if (ch->pcdata->silver_bank < amount)
                  {
	             send_to_char(
"\n\r{YThe Banker tells you 'You don't have that much {WSILVER{Y in your Savings Account!'{x\n\r",ch);
	             return;
                  }
                  else 
                  {
                     ch->pcdata->silver_bank -= amount;
                     ch->silver += amount;
                     act("\n\r{W$n {cWITHDRAWs {WSILVER {cfrom $s Savings Account.{x\n\r", 
ch,NULL,NULL,TO_ROOM);
       sprintf( buf, 
"\n\r{YThe Banker tells you 'You have withdrawn {W%ld Silver{Y.'\n\r"
"   {cTotals{w: {CBeltPouch{w: {W%-10ld  {CAccount{w: {W%-10ld{x\n\r",
                              amount, ch->silver, ch->pcdata->silver_bank);
                     send_to_char( buf, ch);
                     return;
                  }
               }  
            }
            else
            {
               send_to_char("\n\r{RThe TYPE of currency MUST be stated AFTER the AMOUNT.{x\n\r",ch);
            }
         }
      }
      else if(ch->in_room == get_room_index(ROOM_VNUM_BANK_THIEF))
      {
       if((ch->class != 3) 
       && (ch->class != 4))
         {
            act("\n\r{CA Do-Gooder is Here to kill the Guildmaster!{x\n\r", ch,NULL,NULL, TO_ROOM);
            send_to_char("\n\r{RYou aren't allowed in Here, you'd better go!{x\n\r",ch); 
            return;
         }
         else
         {
            if( is_number( arg1 ) )
            {
	       amount = atoi(arg1);

	       if ( amount <= 0 )
               {
	             send_to_char(
"\n\r{YThe Guild Accountant tells you 'To make a WITHDRAWAL, you must specify an AMOUNT of money!'{x\n\r",ch );
	          return;
	       }
               if(!str_cmp( arg2, "gold")) 
               {
                  if (ch->pcdata->gold_bank < amount)
                  {
	             send_to_char(
"\n\r{YThe Guild Accountant tells you 'You don't have that much GOLD stashed away in the Guild!'{x\n\r",ch);
	             return;
                  }
                  else 
                  {
                     ch->pcdata->gold_bank -= amount;
                     ch->gold += amount;
                     act(
"\n\r{W$n {cgrabs some {YGOLD {cfrom a secret compartment.{x\n\r", ch,NULL,NULL,TO_ROOM);
       sprintf( buf, 
"\n\r{YThe Guild Accountant tells you 'You have withdrawn %ld Gold.'{x\n\r"
"   {cTotals{w: {CBeltPouch{w: {W%-10ld  {CAccount{w: {W%-10ld{x\n\r",
                              amount, ch->gold, ch->pcdata->gold_bank);
                     send_to_char( buf, ch);
                     return;
                  }
               }  
               if(!str_cmp( arg2, "silver")) 
               {
                  if (ch->pcdata->silver_bank < amount)
                  {
	             send_to_char(
"\n\r{YThe Guild Accountant tells you 'You don't have that much {WSILVER {Ystashed away in the Guild!'{x\n\r",ch);
	             return;
                  }
                  else 
                  {
                     ch->pcdata->silver_bank -= amount;
                     ch->silver += amount;
                     act(
"\n\r{W$n {cgrabs some {WSILVER {cfrom a secret compartment.{x\n\r", ch,NULL,NULL,TO_ROOM);
       sprintf( buf, 
"\n\r{YThe Guild Accountant tells you 'You have withdrawn {W%ld Silver.'{x\n\r"
"   {cTotals{w: {CBeltPouch{w: {W%-10ld  {CAccount{w: {W%-10ld{x\n\r",
                              amount, ch->silver, ch->pcdata->silver_bank);
                     send_to_char( buf, ch);
                     return;
                  }
               }  
            }
            else
            {
               send_to_char("\n\r{RThe TYPE of currency MUST be stated AFTER the AMOUNT.{x\n\r",ch);
            }
         }
      }
      else
      {
         bug( "Do_withdraw: Bank doesn't exist.", 0 );
         send_to_char( "\n\r{RBank is MISSING.  Please NOTE Arioch ASAP!{x\n\r", ch );
         return;
      } 
   }
   return;
}

void do_change (CHAR_DATA *ch, char *argument)
{
   int amount = 0,change = 0, roll, fence = 0;
   float profit;
   char buf [MAX_STRING_LENGTH];
   char arg [MAX_INPUT_LENGTH];


   if ((ch->class == 3) || (ch->class == 4))
     {

      if( (IS_NPC(ch) || IS_SET(ch->act,ACT_PET)) 
           || (IS_IMMORTAL(ch)) )
      {
        if(!IS_IMP(ch))
        {
         send_to_char("\n\r{ROnly players need to change currency!{x\n\r", ch);
         return;
        }
      }


      if(ch->in_room != get_room_index(ROOM_VNUM_BANK_THIEF))
        {
         send_to_char("\n\r{RYou must be in the Guild Treasury to exchange your loot.{x\n\r",ch);
         return;
        }
      else
        {
          argument = one_argument( argument, arg );
  
         if ( is_number( arg ) )
           {
 	      amount = atoi(arg);

	    if ( amount <= 0 )
            {
               send_to_char("\n\r{cHow much {WSILVER {cdo you wish to change?{x\n\r",ch);
               return;
            }
            else
            {
               if(ch->silver < amount )
               {
                  sprintf(buf, "\n\r{cYou can only change {W%ld SILVER{c.{x\n\r",ch->silver);
                  send_to_char( buf, ch);
                  return;
               }
               else
               {
	            /* haggle */
	            roll = number_percent();
	            if (roll <= get_skill(ch,gsn_haggle))
	            {
                       profit = roll/100;
            sprintf(buf,"\n\r{cYou haggle the fence's percentage down to {C%d{c%%.{x",(int)profit*100);
	               send_to_char(buf,ch);
	               check_improve(ch,gsn_haggle,TRUE,4);
	            }
                    else
                       profit = 0.1; /* 10% exchange rate at the fence */
                       
                  if( ch->level <= 10)  
                     change = amount/100;
                  else
                  {
                    if( profit > 0.1) /* no tax rate above 10% */
                        profit = 0.1;

                     change = amount/100 - (int) ((float)amount/100*profit);
                     fence = (int)(amount/100 * profit);
                  }
                  ch->gold += change;
                  ch->silver -= amount;
                  sprintf( buf, "\n\r{cYou have changed {W%d SILVER{c.{x\n\r{cYou have "
                                "recieved {Y%d GOLD{c.{x\n\r{CFence Profit{w: {Y%d GOLD{x\n\r",
                                amount, change, fence);
                  send_to_char( buf, ch);
                  return;
               }
            }
         }
         else
         {
            send_to_char("\n\r{GSyntax{w:  ",ch);
            send_to_char("{WCHANGE {c<{WSILVER AMOUNT{c>  {Ci{c.{Ce{c. {CCHANGE 1200{x\n\r",ch);
            send_to_char("\n\r{cProfitting occurs from Currency Exchange!{x\n\r",ch);
            send_to_char("{cProfit Max{w: {W10{c%% {Cof Currency Changed.  ",ch);
            send_to_char("{D({cHonour among Thieves and all that!{D){x\n\r",ch);
            return;
         }
      }
   }



   if ((ch->class != 3) && (ch->class != 4))
     {

      if( (IS_NPC(ch) || IS_SET(ch->act,ACT_PET)) 
           || (IS_IMMORTAL(ch)) )
      {
        if(!IS_IMP(ch))
        {
         send_to_char("\n\r{ROnly players need to change currency!{x\n\r", ch);
         return;
        }
      }


      if(ch->in_room != get_room_index(ROOM_VNUM_BANK))
        {
         send_to_char("\n\r{RYou must be in the Bank to change currency.{x\n\r",ch);
         return;
        }
       else
        {
         argument = one_argument( argument, arg );

         if ( is_number( arg ) )
           {
 	    amount = atoi(arg);

	    if ( amount <= 0 )
              {
               send_to_char("\n\r{CHow much {WSILVER {Cdo you wish to change?{x\n\r",ch);
               return;
              }
            else
              {
               if(ch->silver < amount )
                 {
                  sprintf(buf, "\n\r{CYou can only change {W%ld SILVER{C.{x\n\r",ch->silver);                   send_to_char( buf, ch);
                  return;
                 }
               else
                 {
                  if( ch->level <= 10)  
                     change = amount/100;
                  else
                    {
                     profit = 0.1;

                     change = amount/100 - (int) ((float)amount/100*profit);
                     fence = (int)(amount/100 * profit);
                     }

                  ch->gold += change;
                  ch->silver -= amount;

                  sprintf( buf, "\n\r{cYou have changed {W%d SILVER{c.{x\n\r{cYou have "
                                "recieved {Y%d GOLD{c.{x\n\r{CExchange Fee{w: {Y%d GOLD{x\n\r",
                                amount, change, fence);
                  send_to_char( buf, ch);
                  return;
                 }
             }
           }
         else
           {
     send_to_char("\n\r{GSyntax{w:  {WCHANGE {c<{WSILVER AMOUNT{c> {Ci{c.{Ce{c. {CCHANGE 1200{x\n\r",ch);
     send_to_char("         {cSo {W1200 SILVER {cchanges into {Y120 GOLD{c.{x\n\r",ch);
            return;
           }
       }
    }

/*

       if(ch->in_room == get_room_index(ROOM_VNUM_BANK)
       && (IS_CLASS(ch, CLASS_THIEF)))
      {
         act("\n\r{cA Thief lurks in the shadows of the Midgaard Bank.{x\n\r", ch,NULL,NULL, TO_ROOM);
         send_to_char("\n\r{CYou aren't allowed in here, you'd better go!{x\n\r",ch);
         send_to_char("{CYou must go to your Guild's Fence to change currency.{x\n\r",ch); 
         return;
      }
      else 
       if(ch->in_room == get_room_index(ROOM_VNUM_BANK_THIEF)
       && (!IS_CLASS(ch, CLASS_THIEF)))
      {
         act("\n\r{CA Noble is here waiting to be robbed.{x\n\r", ch,NULL,NULL, TO_ROOM);
         send_to_char("\n\r{CYou aren't allowed in Here, you'd better go!{x\n\r",ch); 
         return;
      }
*/

  return;
}


void do_share (CHAR_DATA *ch, char *argument)
{
   long shares = 0, cost = 0;
   BUFFER *output;
   int level, row, money = 0, diff, i;
   int list_level, tax;
   int bonus = 0;
   char buf[MAX_STRING_LENGTH];
   char buf2[MAX_STRING_LENGTH];
   char arg[MAX_INPUT_LENGTH];

   shares = ch->pcdata->shares;

   if(ch->in_room != get_room_index(ROOM_VNUM_BANK)
      && (!IS_CLASS(ch, CLASS_THIEF)))
   {
      send_to_char("\n\r{RYou must be in the Bank to find out about shares.{x\n\r",ch);
      return;
   }
   else 
    if(IS_CLASS(ch, CLASS_THIEF))
   {
      send_to_char("\n\r{RDue to a poor economic base - Thieves cannot get shares.{x\n\r", ch);
      return;
   }
   else 
   {
      argument = one_argument( argument, arg );

  if(!IS_IMP(ch))
    {
      if( (IS_NPC(ch) || IS_SET(ch->act,ACT_PET)) 
           || (IS_IMMORTAL(ch)) )
      {
         send_to_char("Only players need shares!\n\r", ch);
         return;
      }
    }

      if(ch->in_room == get_room_index(ROOM_VNUM_BANK))
      {
         if (arg[0] == '\0' && ch->level >= 10)
         { 
            send_to_char("\n\r{GSyntax{w: {WSHARE {c<{W# {cto {WBUY{c>{x\n\r",ch);
            send_to_char("        {cFor the SHARE prices for your LEVEL type{w: {WSHARE PRICE{x\n\r",ch);
            send_to_char("\n\r{cSHARE Costs are all in {YGold{c.{x\n\r",ch);
            send_to_char("{cTo Buy SHAREs{w: {YGold {Cis taken from your Account{x\n\r",ch);
            send_to_char("{cSHARE Spell Bonus{w: {C+1 hour duration per 4 SHAREs{x\n\r",ch);
            send_to_char("{cONLY 5 SHAREs can be bought at EACH Level{x\n\r",ch);
            send_to_char("{cONLY 5 SHAREs can be bought at ONE time{x\n\r",ch);
            return;
         }
         else if(!str_cmp(arg, "price"))
         {
            output = new_buf();
            i = 0;
            send_to_char("                   GBS -> Share Listing <-\n\r",ch);
            sprintf(buf2,"Level:     Cost:      Level:     Cost:      Level:     Cost:      \n\r");
            send_to_char(buf2, ch);
            cost = 0;
            row = tax = 0;

            if(IS_HERO(ch) || ch->level+2 == LEVEL_HERO)
              list_level = ch->level;  /*if hero list ch->level*/
            else
              list_level = ch->level+2; /*2 level past ch->level*/

            /* list levels for mortals */ 
            for(level=10; level <= list_level; level++)  
            {
               if(++tax % 9 == 0)
                  cost += 120000; /* every 9 levels */
               else
                  cost += 20000;
               sprintf(buf,"%-6d     %-9ld  ",level,cost);
               if(++row % 3 == 0)
                  strcat( buf, "\n\r");
               add_buf( output, buf);
            }
            page_to_char(buf_string(output),ch);
            free_buf(output);
            send_to_char( "\n\r", ch);
            return;
         }
         else if(ch->level < 10)
         {
            send_to_char("\n\r{RSHARE buying is NOT allowed until level 10!{x\n\r",ch);
            return;
         }
         else
         {
            if( is_number( arg ) )
            {
	        shares = atoi(arg);
	        if ( shares < 0 )
                {
                     shares = abs(shares);
	             send_to_char( "\n\r{RBad market {r- {WSHAREs don't sell well!{x\n\r", ch );
                   if( ch->pcdata->shares < shares)
                   {
                      sprintf( buf, "\n\r{gYou only have {G%d {gSHAREs.{x\n\r",ch->pcdata->shares);
                      send_to_char( buf, ch);
                   }
                   else
                   {
                      ch->pcdata->shares -= shares;
                      act("$n sells $s some shares.", ch,NULL,NULL, TO_ROOM);

                      if(ch->pcdata->shares == 0)
                         send_to_char( "\n\r{CYou sell ALL your SHAREs in Midgaard Financial.{x\n\r",ch);
                      else
                         send_to_char( "\n\r{CYou sell SHAREs in Midgaard Financial.{x\n\r",ch);

                      money = 20000 / (int)  ( (float)shares * 0.10);
           sprintf( buf,"\n\r{CYou get {W%d {Ygold {Cfor {W%ld {C%s deposited in your Account.{x\n\r", 
                                money, shares, shares > 1 ? "shares" : "share");
                      send_to_char( buf, ch);
                      ch->pcdata->gold_bank +=money; /* put gold in the bank */
                      ch->pcdata->shares_bought -= shares;
                      if( ch->pcdata->shares / 4 != 0)
                          bonus = ch->pcdata->shares / 4;
                       ch->pcdata->duration = bonus;
                      return;
                   }   
	        }
	        else if ( shares > SHARE_MAX )
                {
	             sprintf(buf,"\n\r{RBad Market {r- {WDon't buy more than %d SHAREs!{x\n\r",SHARE_MAX);
	             send_to_char( buf, ch );
	             return;
	        }
	        else
                {
                   if(ch->pcdata->shares_bought > SHARE_MAX)
                   {
                      sprintf(buf,"\n\r{RYou have reached your {W%d SHARE {Rlimit per level.{x\n\r",SHARE_MAX);
                      send_to_char( buf, ch);
                      return;
                   }
                   else
                   {
                     if(ch->pcdata->share_level == ch->level
                        && ch->pcdata->shares_bought <= SHARE_MAX)
                     {
                       cost = 0;
                       tax = 0;
                       for(level=10;level<LEVEL_HERO;level++)
                       { 
                         if(++tax % 9 == 0)
                           cost += 120000; /* every 9 levels */
                         else
                            cost += 20000;
                         if(level == ch->level)
                            break;
                       }
                       diff = (SHARE_MAX - ch->pcdata->shares_bought - shares);
                       sprintf( buf,"\n\r{GYou can buy {W%d {Gmore {W%s {Gat your current level.{x\n\r", 
                                diff, shares > 1 ? "SHARE" : "SHAREs");
                       send_to_char( buf, ch); 
                       cost *= shares; /* actual cost to the player */
                       if(ch->pcdata->gold_bank < cost)
                       {
                          sprintf( buf,"GBS: You only have %ld gold in your Account.\n\r"
                                       "     For %ld %s cost %ld.\n\r",ch->pcdata->gold_bank, shares,
                                          shares > 1 ? "shares" : "share", cost);
                          send_to_char( buf, ch);
                          return;
                       }
                       else
                       {
                          if( shares == 1 )
                          {
                             act("$n buys one share in the Midgaard Bakery.", ch,NULL,NULL, TO_ROOM);
                             sprintf( buf,"GBS: You buy a share in the Midgaard Bakery.\n\r");
                          }
                          else if( shares > 1 && shares <= 3)
                          {
                             act("$n buys shares in the Midgaard Merchant's Guild.", ch,NULL,NULL,TO_ROOM);
                     sprintf( buf,"GBS: You buy %ld shares in the Midgaard Merchant's Guild.\n\r",shares);
                          }
                          else /* shares bought = 4 & 5 */
                          {
                           act("$n buys shares in the Midgaard Waterworks.", ch,NULL,NULL, TO_ROOM);
                           sprintf( buf,"GBS: You buy %ld shares in the Midgaard Waterworks.\n\r",shares);
	                    }
                          send_to_char( buf, ch );
                          ch->pcdata->gold_bank -= cost;
                          ch->pcdata->shares += shares;
                          ch->pcdata->shares_bought += shares;
                          do_account(ch, "");
                          if( ch->pcdata->shares / 4 != 0)
                              bonus = ch->pcdata->shares / 4;
                          ch->pcdata->duration = bonus;
 	                  return;
                       }
                     }
                     else
                     {
                        bug( "Do_shares: shares_bought && share_level error.", 0 );
                        bug( "Do_shares: bought > %d && share_level not ch->level.",SHARE_MAX );
                        send_to_char( "Stock market has crashed.\n\r", ch );
                        return;
                     }
                   }
	        }
            }
         } 
      }
      else
      {
         bug( "Do_shares: Bank doesn't exist.", 0 );
         send_to_char( "Bank doesn't exist.\n\r", ch );
         return;
      } 
   }
   return;
}
