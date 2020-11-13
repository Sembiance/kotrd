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
//#include "dpit.h" 

int min_level;
int max_level;
int indp;
int dptype;
int dptimeleft;
int dptimer;
int cost_level;
int jackpot;
bool isdp;
int remvalue;
int dpran;

/* [DragonPIT] The DragonLord: ! */

extern int atoi args((const char *string));

void do_function args((CHAR_DATA *ch, DO_FUN *do_fun, char *argument));

DECLARE_DO_FUN(do_look);
DECLARE_DO_FUN(do_dptalk);
DECLARE_DO_FUN(do_restore);

bool isdp;
int dpran;

void do_startdp(CHAR_DATA *ch, char *argument)
  {   
   char buf[MAX_STRING_LENGTH];   
   char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];   
   char arg3[MAX_INPUT_LENGTH];   
   char arg4[MAX_INPUT_LENGTH];   
   DESCRIPTOR_DATA *d;

   argument = one_argument(argument, arg1);   
   argument = one_argument(argument, arg2);   
   argument = one_argument(argument, arg3);   
   argument = one_argument(argument, arg4);   

  if (arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' || arg4[0] == '\0' )   
    {     
     send_to_char("\n\r{GSyntax{w: {WSTARTDP {c<{WTYPE NUMBER{c> <{WMIN LEVEL{c> <{WMAX LEVEL{c> <{WLEVEL COST{c>{x\n\r",ch);
     send_to_char("\n\r{cFields{w: {CTYPE {c= {D( {W1 {D) {Cfor FREE FOR ALL or {D( {W2 {D) {Cfor TEAM DRAGONPIT{x\n\r",ch);
     send_to_char("        {CMIN{c/{CMAX LEVEL {c= {CThe LEVEL RANGE allowed to enter{x\n\r",ch);
     send_to_char("        {CLEVEL COST {c= {CGPs per Level of PC they pay to join{x\n\r",ch);
     send_to_char("\n\r{D( {ci.e.{w: {WSTARTDP 1 20 100 50 {cWould start a Free for All\n\r",ch);
     send_to_char("{c     DragonPIT for levels 20 to 100 with a 50GP per PC lvl cost to join.{D){x\n\r",ch);
     return;   
    }    

/*
   if (atoi(arg1) < 1 || atoi(arg1) > 2)   
     {
      send_to_char("\n\r{cThe {WTYPE NUMBER {chas to be a 1 {D({CFREE for ALL{D) or a 2 {D({CTEAM DRAGONPIT{D).{x\n\r", ch);     
      return;   
     }
*/

   if (atoi(arg1) < 1 || atoi(arg1) > 1)   
     {     
      send_to_char("\n\r{cThe {WTYPE NUMBER {chas to be a 1 {D({CFREE for ALL{D) {cfor now.\n\r"
                   "{D({CTEAM DRAGONPIT{D){c will be added later..{x\n\r", ch);     
      return;   
     }    

   if (dpran == 50)
     {
      send_to_char("\n\r{RThe MAX number of DRAGONPITs has been ran, please try again later.{x\n\r", ch);     
      return;   
     }
     

   if (!IS_IMP(ch))
     {
      if (atoi(arg2) <= 0 || atoi(arg2) > 100)   
        {     
         send_to_char("\n\r{cLevel must be between {C1 {cand {C100{c.{x\n\r", ch);
         return;  
        }    

      if (atoi(arg3) <= 0 || atoi(arg3) > 100)   
        {     
         send_to_char("\n\r{cLevel must be between {C1 {cand {C100{c.{x\n\r", ch);
         return;   
        }
     }
   else    
     {
      if (atoi(arg2) <= 0 || atoi(arg2) > 500)   
        {     
         send_to_char("\n\r{cLevel must be between {C1 {cand {C500 {D({WIMP ONLY MAX LEVEL{D){c.{x\n\r", ch);
         return;  
        }    

      if (atoi(arg3) <= 0 || atoi(arg3) > 500)   
        {     
         send_to_char("\n\r{cLevel must be between {C1 {cand {C500 {D({WIMP ONLY MAX LEVEL{D){c.{x\n\r", ch);
         return;   
        }
     }


   if (atoi(arg3) < atoi(arg2))   
     {     
      send_to_char("\n\r{cMax level must be {CGREATER{c than the min level.{c\n\r",ch);
      return;   
     }    

  if(!IS_IMP(ch))
    {
     if (atoi(arg4) <= 0 || atoi(arg4) > 50)   
       {     
        send_to_char("\n\r{CCOST per LEVEL must be between {C1 {cand {C50 {Ygold{c.{x\n\r", ch);
        return;  
       }
     }
   else
     {
         if (atoi(arg4) < 0 || atoi(arg4) > 500)   
           {      
            send_to_char("\n\r{CCOST per LEVEL must be between {C0 {cand {C500 {Ygold {D({WIMP ONLY MAX AMOUNT{D){c.{x\n\r", ch);
            return;  
           }
     }
    

   if (isdp == TRUE)   
     {     
      send_to_char("\n\r{RThere already is a DragonPIT  running, let that one finish first!{x\n\r", ch);     
      return;   
     }    

   isdp = TRUE;   
   dpran++;
   dptype = atoi(arg1);   
   min_level = atoi(arg2);   
   max_level = atoi(arg3);   
   cost_level = atoi(arg4);

   dptimer = -1;
   dptimeleft = 2;
   jackpot = 0;   

  for (d = descriptor_list; d != NULL; d = d->next)   
     {     
      if (d->connected == CON_PLAYING)
        {
         if (IS_SET(d->character->pact, PLR_DRAGONPIT)
         && (!IS_NPC(d->character)))         
           REMOVE_BIT(d->character->pact, PLR_DRAGONPIT);     

if(!IS_SET(d->character->deaf, CHANNEL_DPTALK))
  {
/*
sprintf(buf, "\n\r\n\r       {RThe DragonLord announces a new DragonPIT{w:\n\r
          {rThis is a {W%s {rwith a Level Range of {W%d {rto {W%d{r.\r
          {rThe Entry Fee is {W%d{Y gp{r per Contestants Lvl.\r
          {rThe Jackpot for this DragonPIT is up to {W%d {Ygp{r!\r
          {W%d {r%s %s fighting in the DragonPIT, so far.\r
          {W%d {rtick{D({Ws{D)[r left to join the DragonPIT.\r
          {rType '{WDragonPIT STATS{r' at anytime to see the DragonPIT Statistics!{x\n\r\n\r",
dptype == 1 ? "FREE-FOR-ALL" : "TEAM DRAGONPIT",min_level,max_level, 
cost_level,
(jackpot-(jackpot/10)),
indp == -1 ? 0 : indp, indp == 1 ? "person" : "people", indp == 1 ? "is" : "are",
dptimeleft+1);
*/
sprintf(buf, "\n\r\n\r       {RThe DragonLord announces a new DragonPIT{w:\n\r"
          "{rThis is a {W%s {rwith a Level Range of {W%d {rto {W%d{r.\r"
          "{rThe Entry Fee is {W%d{Y gp{r per Contestants Lvl.\r"
          "{rThe Total Jackpot so far is {W%d {Ygp{r!\r"
          "{W%d {r%s %s already Joined and %s waiting for more Contestants!\r"
          "{W%d {r%s\r"
          "{rTo see this info again type {c'{WDragonPIT STATS{c'{r at anytime!{x\n\r\n\r",
dptype == 1 ? "FREE-FOR-ALL" : "TEAM BATTLE",min_level,max_level, 
cost_level,
(jackpot-(jackpot/10)),
indp == -1 ? 0 : indp, indp == 1 ? "person" : "people", indp == 1 ? "has" : "have",
indp == 1 ? "is" : "are",
dptimeleft+1,dptimeleft > 1 ? "ticks left to Join. {c<{Wtype {c'{WDRAGONPIT{c' {Wto JOIN{c>{x" :
dptimeleft <= 0 ? "tick remaining, it is about to begin! {c<{Wtype {c'{WDRAGONPIT{c' {Wto JOIN{c>{x" :
"tick left to Join. {c<{Wtype {c'{WDRAGONPIT{c' {Wto JOIN{c>{x");

sprintf(buf, "\n\r{D[{RDragonPIT{D] {RThe DragonLord{w: {rI am about to start a DragonPIT!!\rAfter it is announced type '{WDRAGONPIT{r' to JOIN.{x\n\r");

send_to_char(buf, d->character);
  }   
      }
     }
return;
}


void do_dragonpit(CHAR_DATA *ch, char *argument) 
    {   
    char arg[MAX_INPUT_LENGTH];
     char buf[MAX_STRING_LENGTH];   
     ROOM_INDEX_DATA *location;    
     DESCRIPTOR_DATA *d;

     argument = one_argument( argument, arg );


     if (IS_NPC(ch))
       {     
        send_to_char("\n\r{RThis is for PLAYERS ONLY!{x\n\r", ch);     
        return;   
       }    

     if ( !str_cmp(arg, "stats"))
       {
        send_to_char( "\n\r{rThe DragonPIT's Current Status{w:\n\r{x", ch );

        sprintf( buf, "\n\r{rIn Progress{w: {R%s{x\n\r", 
(isdp == TRUE ? "YES" : "NO"));
        send_to_char( buf, ch );


     if (dptimeleft != -1)
       {
        sprintf( buf, "\n\r{rTime to Join{w: {R%d {rtick%s{x\n\r",
        dptimeleft == -1 ? 0 : dptimeleft+1, dptimeleft > 1 ? "s!" : "!" );
        send_to_char( buf, ch );
       }
     else
       {
        send_to_char("\n\r{rTime to Join{w: {RNONE {r- {RCLOSED{x\n\r",ch);
       }

sprintf( buf, "\n\r{rLevel Range{w: {R%d {rto {R%d{x\n\r",min_level,max_level);
        send_to_char( buf, ch );

        sprintf( buf, "\n\r{YGP {rper Entrants Level{w: {R%d {Ygp{x\n\r",cost_level);
        send_to_char( buf, ch );

        sprintf( buf, "\n\r{rCurrent Jackpot{w: {R%d {Ygp{x\n\r",(jackpot-(jackpot/10)));
        send_to_char( buf, ch );

        if (dptimer > 0)
          {
           if (dptimer > 1)
             {
              sprintf( buf, "\n\r{rTime Remaining in Current PIT{w: {R%d {rticks{x\n\r",dptimer);
              send_to_char( buf, ch );
             }
           else
             {
              send_to_char("\n\r{rTime Remaining in Current PIT{w: {WFINAL TICK{x\n\r",ch);
             }
          }
        else
          {
        send_to_char( "\n\r{rDragonPIT Combat has not started yet.{x\n\r", ch );
          }


        sprintf( buf, "\n\r{rNumber of Contestants{w: {R%d {rDragonWarrior%s{x\n\r",
        indp == -1 ? 0 : indp, indp > 1 ? "s!" : "!");
        send_to_char( buf, ch );

        return;
       }


  if (ch->level >= 500)
    {
     if (!str_cmp(arg, "stop"))
       {
      for (d = descriptor_list; d != NULL; d = d->next)   
         {     
             if (d->connected == CON_PLAYING)
           {
          if ((IS_SET(d->character->in_room->room_flags, ROOM_DRAGONPIT))
             && (IS_SET(d->character->pact, PLR_DRAGONPIT))
             && (!IS_NPC(d->character)))
                  {

             if(!IS_SET(d->character->deaf, CHANNEL_DPTALK))
               {
                   sprintf(buf, "\n\r{D[{RDragonPIT{D] {RThe DragonLord{w: {GThis DragonPIT has been CANCELLED by a Higher Power!!\r{GSorry, but I am just a slave of the Immortals...{x\n\r");
                   send_to_char(buf, d->character);
               }
         if (IS_SET(d->character->pact, PLR_DRAGONPIT)
         && (!IS_NPC(d->character)))         
            {
                   REMOVE_BIT(d->character->pact, PLR_DRAGONPIT);
            }
  	           d->character->gold += (d->character->level * cost_level);
                   char_from_room(d->character);
                   char_to_room(d->character,(get_room_index(ROOM_VNUM_DRAGONPIT_RETURN)));
                   do_look(d->character, "auto");
                  }
           }
         }

          isdp = FALSE;  
	  dpran--;
          dptimeleft = -1;
          dptimer = -1;
          indp = -1;   
          min_level = 0;
          max_level = 0; 
          dptype = 0;
          cost_level = 0;
          jackpot = 0;

       sprintf(buf,"\n\r{GYou have STOPPED this DragonPIT!{x\n\r");
       send_to_char(buf,ch);
       return;
      }
   }


     if (arg[0] == '\0' 
     || str_cmp(arg, "stats")
     || str_cmp(arg, "stop"))
       {
        int cost;
   
        cost = (ch->level * cost_level);

     if (isdp != TRUE)    
       {     
        send_to_char("\n\r{RThere is no DragonPIT currently running!{x\n\r", ch);     
        return;   
       }    

     if (ch->level < min_level || ch->level > max_level)   
       {     
        send_to_char("\n\r{CSorry, you are outside the set level range for this DragonPIT!{x\n\r", ch);     
        return;   
       }    

     if (cost > ch->gold)
       {    
        if (cost > ch->pcdata->gold_bank)
          {
           send_to_char("\n\r{CSorry, you do not have enough GOLD to enter this DragonPIT...{x\n\r", ch);     
           return;   
          }
       }
    

     if (IS_SET(ch->comm,COMM_DPITKILL))
       {     
        send_to_char("\n\r{CYour DragonPIT privileges were revoked by an Immortal.{x\n\r",ch);     
        send_to_char("\n\r{CIf you think this is an error let an IMP know.{x\n\r",ch);     
        send_to_char("\n\r{CBut if you are just trying to scam your DragonPIT privs back...{x\n\r",ch);
        send_to_char("\n\r{CYou will lose them permenantly.{x\n\r",ch);     
        return;   
       }    

     if (dptimeleft == -1)
       {     
        send_to_char("\n\r{CThis DragonPIT is already closed, try to get in the next one.{x\n\r",ch);     
        return;   
       }    
    

     if (IS_SET(ch->pact, PLR_DRAGONPIT)
     && (!IS_NPC(ch)))   
       {     
        send_to_char("\n\r{CYou are already flagged for this DragonPIT.{x\n\r",ch);     
        return;   
       }    

    if ( IS_SET( ch->deaf, CHANNEL_DPTALK ) )
      {
       REMOVE_BIT( ch->deaf, CHANNEL_DPTALK );
       sprintf( buf, "\n\r{wDRAGONPIT (DPTALK) Channel is now {gON{w.\n\r{x");
       send_to_char( buf, ch );
      }

     if (dptype == 1)   
       {     
        if ((location = get_room_index(ROOM_VNUM_DRAGONPIT)) == NULL)     
          {       
           send_to_char("\n\r{CDragonPIT is not yet completed, sorry.{x\n\r", ch);       
           return;     
          }     
         else     
          {       

   for (d = descriptor_list; d != NULL; d = d->next)
     {   
      if (d->connected == CON_PLAYING)
        {
             if(!IS_SET(d->character->deaf, CHANNEL_DPTALK))
               {
sprintf(buf, "\n\r{D[{RDragonPIT{D] {RThe DragonLord{w: {W%s {rjoins the DragonPIT!{x\n\r",ch->name);
send_to_char(buf,d->character);
               }
        }
     }

	   do_restore(ch,"self");   
           stop_follower(ch);
           act("\n\r{W$n {cgoes to get $s {Rass {cwhipped in the DragonPIT!{x", ch, NULL, NULL,TO_ROOM);

     if (cost > ch->gold)
       {
        if (cost > ch->pcdata->gold_bank)
          {
           return;
          }
        else
          ch->pcdata->gold_bank -= cost;
       }
     else
       { 
        ch->gold -= cost; 
       }

    jackpot += cost;

    while ( ch->affected )
        affect_remove( ch, ch->affected );
    ch->affected_by = race_table[ch->race].aff;
    ch->affected2_by = race_table[ch->race].aff2;

           char_from_room(ch);       
           char_to_room(ch, location);       
act("\n\r{W$n {carrives to get $s {Rass {cwhipped!{x", ch, NULL, NULL, TO_ROOM);
           do_look(ch, "auto");       
           if (indp <= -1)
           indp = 0;
           indp++;       
           SET_BIT(ch->pact, PLR_DRAGONPIT);       
           if (indp >= 2)
           dptimer = 0;

       return;     
          }    
   }   
 }   
}


void dp_update(void) 
{   
    char buf[MAX_STRING_LENGTH];   
    DESCRIPTOR_DATA *d;   
    ROOM_INDEX_DATA *random;   


if (isdp == TRUE)   
 {
 for (d = descriptor_list; d != NULL; d = d->next)
  {
   if (dptimeleft >= 0)   
     {     
      dptimeleft--;

      if (dptimeleft >=0)
        {

         for (d = descriptor_list; d != NULL; d = d->next)
            {  
             if (d->connected == CON_PLAYING)
               {
                if(!IS_SET(d->character->deaf, CHANNEL_DPTALK))
                  {
sprintf(buf, "\n\r\n\r          {RThe following DragonPIT is about to begin{w:\n\r"
          "{rThis is a {W%s {rwith a Level Range of {W%d {rto {W%d{r.\r"
          "{rThe Entry Fee is {W%d{Y gp{r per Contestants Lvl.\r"
          "{rThe Total Jackpot so far is {W%d {Ygp{r!\r"
          "{W%d {r%s %s already Joined and %s waiting for more Contestants!\r"
          "{W%d {r%s\r"
          "{rTo see this info again type {c'{WDragonPIT STATS{c'{r at anytime!{x\n\r\n\r",
dptype == 1 ? "FREE-FOR-ALL" : "TEAM BATTLE",min_level,max_level, 
cost_level,
(jackpot-(jackpot/10)),
indp == -1 ? 0 : indp, indp == 1 ? "person" : "people", indp == 1 ? "has" : "have",
indp == 1 ? "is" : "are",
dptimeleft+1,dptimeleft > 1 ? "ticks left to Join. {c<{Wtype {c'{WDRAGONPIT{c' {Wto JOIN{c>{x" :
dptimeleft <= 0 ? "tick remaining, it is about to begin!  {c<{Wtype {c'{WDRAGONPIT{c' {Wto JOIN{c>{x" :
"tick left to Join. {c<{Wtype {c'{WDRAGONPIT{c' {Wto JOIN{c>{x");
send_to_char(buf,d->character);
                  }
              }
          }
   return;
    }
   else
    {
     for (d = descriptor_list; d != NULL; d = d->next)
        {  
         if (d->connected == CON_PLAYING)
           {
             if(!IS_SET(d->character->deaf, CHANNEL_DPTALK))
               {
sprintf(buf, "\n\r{D[{RDragonPIT{D] {RThe DragonLord{w: {rThe DragonPIT is now CLOSED!!\rIt will begin within 1 tick!!{x\n\r");
send_to_char(buf,d->character);
               }
           }
        }
   return;
    }
  }



       if (((indp == 1)
       || (indp == 0))
       && (dptimer == -1))
         {       
          isdp = FALSE;       
          dptimeleft = -1;       
          dptimer = -1;       
          indp = -1;
          min_level = 0;       
          max_level = 0;       
          dptype = 0;
          jackpot = 0;

   for (d = descriptor_list; d != NULL; d = d->next)
     {
       if (d->connected == CON_PLAYING) 
         {

          if (IS_SET(d->character->in_room->room_flags, ROOM_DRAGONPIT)
          && (!IS_NPC(d->character)))
            {
         if (IS_SET(d->character->pact, PLR_DRAGONPIT)
         && (!IS_NPC(d->character)))
           {
            REMOVE_BIT(d->character->pact, PLR_DRAGONPIT);     
          }
	     d->character->gold += (d->character->level * cost_level);
             char_from_room(d->character);
             char_to_room(d->character, (get_room_index(ROOM_VNUM_DRAGONPIT_RETURN)));
             do_look(d->character, "auto");
            }
         }
      }


   for (d = descriptor_list; d != NULL; d = d->next)
     {
       if (d->connected == CON_PLAYING) 
         {
if(!IS_SET(d->character->deaf, CHANNEL_DPTALK))
           {          
sprintf(buf, "\n\r{D[{RDragonPIT{D] {RThe DragonLord{w: {CNOT ENOUGH {rpeople for DragonPIT.  DragonPIT Cancelled.{x\n\r");
send_to_char(buf,d->character);
           }
         }
     }

  cost_level = 0;    
  return;
      }     





  if ((indp >= 2)
  && (dptimer <= 0)
  && (isdp == TRUE))
    {       
   for (d = descriptor_list; d != NULL; d = d->next)
     {

          if (d->connected == CON_PLAYING)
           {
   if(!IS_SET(d->character->deaf, CHANNEL_DPTALK))
                      {
sprintf(buf,"\n\r{c({RDragonPIT{c) {RThe DragonLord{w: {rLet the DragonPIT Commence!!\r "
                           "{W%d {rSuicidal Contestants have entered, Only 1 may leave!\r"
                            "{rThe Total Jackpot for this DragonPIT is {W%d {Ygp{r!{x\n\r",
indp,(jackpot-(jackpot/10)));
send_to_char(buf,d->character);


                      }

           dptimer = (1 * indp); 

              if (IS_SET(d->character->pact, PLR_DRAGONPIT)
              && (!IS_NPC(d->character)))
                 {
                  random = get_room_index((number_range(101, 145)));
                  char_from_room(d->character);
                  char_to_room(d->character, random);
       	          while ( d->character->affected )
        	    affect_remove( d->character, d->character->affected );
                  d->character->affected_by = race_table[d->character->race].aff;
                  d->character->affected2_by = race_table[d->character->race].aff2;
                  do_look(d->character, "auto");


                 }
         }
     }
return;
   }


       if ((indp == -1)
       && (dptimeleft == -1)
       && (dptimer == -1)
       && (isdp == TRUE))
         {       
          isdp = FALSE;       
          jackpot = 0;
          cost_level = 0;
          return;
         }     



if (dptimer >= 1) 
  {
   dptimer--;

   for (d = descriptor_list; d != NULL; d = d->next)
     {
     if (d->connected == CON_PLAYING)
       {

        if(!IS_SET(d->character->deaf, CHANNEL_DPTALK))
               {
if (dptimer != 0)
  {
sprintf(buf, "\n\r{D[{RDragonPIT{D] {RThe DragonLord{w: {W%d {rtick%s remaining in this DragonPIT{x.\r{W%d {r%s %s still battling in this DragonPIT.{x\n\r",
dptimer,dptimer == 1 ? "" : "s",
indp, indp == 1 ? "person" : "people", indp == 1 ? "is" : "are");
send_to_char(buf,d->character);
  }
else
  {
sprintf(buf, "\n\r{D[{RDragonPIT{D] {RThe DragonLord{w: {rThis is the FINAL TICK remaining for this DragonPIT{x!\r{W%d {r%s %s still battling in this DragonPIT.{x\n\r",
indp, indp == 1 ? "person" : "people", indp == 1 ? "is" : "are");
send_to_char(buf,d->character);
  }
        }
       }
     }
return;
}




      }
    }
return;
 }
