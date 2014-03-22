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
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include "merc.h"
#include "interp.h"
#include "recycle.h"

#include "Utility.h"
#include "StringUtility.h"
#include "ArrayUtility.h"


//////////////////////////////////////////////////////////////////////////
// Definitions
#define STONES_DICE_IN_GAME             5       // How many dice each player starts with  !!! CAN'T CHANGE THIS YET - Need to modify struct and printing of dice !!!
#define STONES_STORAGE_ROOM_VNUM        20000   // What VNUM to store wagered items in during game

// These states are in the PAST tense!
#define STONES_STATE_IDLE				0	// No match, no invites sent
#define STONES_STATE_INVITING			1	// Invite has been sent
#define STONES_STATE_ACCEPTING          2   // Invite has been accepted
#define STONES_STATE_MATCH_STARTING		3	// Match is starting (2 sec delay)
#define STONES_STATE_STAKES_SHOWN		4	// Stakes have been shown (3 sec delay)
#define STONES_STATE_ROLL_GO_FIRST		5	// Roll to see who goes first (2 sec delay)
#define STONES_STATE_BEGINNING_ROUND	6	// Beginning the round (2 sec delay)
#define STONES_STATE_WHO_GOES_FIRST		7	// Show who goes first (5 sec delay)
#define STONES_STATE_ROLL_DICE			8	// Roll the dice (2 sec delay)
#define STONES_STATE_PLACE_BID			9	// Ask for bid (50 second timeout if bidding)
#define STONES_STATE_PLACE_BID_WARNING	10  // Warn the user they have to bid (10 seconds)
#define STONES_STATE_CHALLENGE_MADE		11	// Challange made (3 sec delay)
#define STONES_STATE_REVEAL_DICE		12	// Reveal the dice (4 sec delay)
#define STONES_STATE_CHALLENGE_OUTCOME	13	// Show challenge outcome (2 sec delay)
#define STONES_STATE_LOSE_DIE			14	// Someone loses a die (3 sec delay)
#define STONES_STATE_MATCH_OUTCOME		15	// Show who lost all their dice (3 sec delay)

#define STONES_DELAY_MATCH_STARTING     3
#define STONES_DELAY_STAKES_SHOWN       4
#define STONES_DELAY_ROLL_GO_FIRST      3
#define STONES_DELAY_BEGINNING_ROUND    3
#define STONES_DELAY_WHO_GOES_FIRST     6
#define STONES_DELAY_ROLL_DICE          3
#define STONES_DELAY_PLACE_BID          50
#define STONES_DELAY_PLACE_BID_WARNING  10
#define STONES_DELAY_CHALLENGE_MADE     4
#define STONES_DELAY_REVEAL_DICE        5
#define STONES_DELAY_CHALLENGE_OUTCOME  3
#define STONES_DELAY_LOSE_DIE           4
#define STONES_DELAY_MATCH_OUTCOME      4

#define STONES_DELAY_BID_MADE           1
#define STONES_DELAY_REROLL_TIE         4
#define STONES_DELAY_BETWEEN_ROUNDS     10

#define DIE_SPACER_WIDTH                3
#define MAX_DICE_LINE_LENGTH            100

typedef char        diestring[40];    

const diestring     dieStrings[] = { "{D/-----\\{x", "{D|     |{x", "{D|  {Wo  {D|{x", "{D|     |{x", "{D\\-----/{x",
                                     "{D/-----\\{x", "{D|{Wo    {D|{x", "{D|     |{x", "{D|    {Wo{D|{x", "{D\\-----/{x",
                                     "{D/-----\\{x", "{D|{Wo    {D|{x", "{D|  {Wo  {D|{x", "{D|    {Wo{D|{x", "{D\\-----/{x",
                                     "{D/-----\\{x", "{D|{Wo   o{D|{x", "{D|     |{x", "{D|{Wo   o{D|{x", "{D\\-----/{x",
                                     "{D/-----\\{x", "{D|{Wo   o{D|{x", "{D|  {Wo  {D|{x", "{D|{Wo   o{D|{x", "{D\\-----/{x",
                                     "{D/-----\\{x", "{D|{Wo   o{D|{x", "{D|{Wo   o{D|{x", "{D|{Wo   o{D|{x", "{D\\-----/{x" };

const diestring     dieStringsHighlighted[] = { "{R/-----\\{x", "{R|     |{x", "{R|  o  |{x", "{R|     |{x", "{R\\-----/{x",
                                                "{R/-----\\{x", "{R|o    |{x", "{R|     |{x", "{R|    o|{x", "{R\\-----/{x",
                                                "{R/-----\\{x", "{R|o    |{x", "{R|  o  |{x", "{R|    o|{x", "{R\\-----/{x",
                                                "{R/-----\\{x", "{R|o   o|{x", "{R|     |{x", "{R|o   o|{x", "{R\\-----/{x",
                                                "{R/-----\\{x", "{R|o   o|{x", "{R|  o  |{x", "{R|o   o|{x", "{R\\-----/{x",
                                                "{R/-----\\{x", "{R|o   o|{x", "{R|o   o|{x", "{R|o   o|{x", "{R\\-----/{x" };

//////////////////////////////////////////////////////////////////////////
// stones_clear - Clears a stones_data structure
void    stones_clear(STONES_DATA * stones)
{
    stones->state = STONES_STATE_IDLE;
    stones->invited = 0;
    stones->playing = 0;
    stones->objectWager = 0;
    stones->moneyWager = 0;
    stones->questPointWager = 0;
    stones->diceLeft = 0;
    stones->dice[0] = 0;
    stones->dice[1] = 0;
    stones->dice[2] = 0;
    stones->dice[3] = 0;
    stones->dice[4] = 0;
    stones->myTurn = FALSE;
    stones->bidDiceCount = 0;
    stones->bidDiceType = 0;
    stones->bidDiceTypeFriendly[0] = '\0';
    stones->issuedChallenge = FALSE;
    stones->challengeWorked = FALSE;
    stones->pulse = 0;
    stones->movementWarning = FALSE;
    stones->diceBeingUsed = 0;
}


//////////////////////////////////////////////////////////////////////////
// stones_has_dice - Gets dice capable of being used to play, or 0 if none found
OBJ_DATA * stones_has_dice(OBJ_DATA * list)
{
    OBJ_DATA *  obj;
    
    for(obj=list;obj!=NULL;obj=obj->next_content)
    {
        if(IS_OBJ_STAT(obj, ITEM_DICE))
            return obj;
    }
    
    return 0;
}


//////////////////////////////////////////////////////////////////////////
// stones_count_dice - Returns the number of dice type between the players
char stones_count_dice(CHAR_DATA * ch, CHAR_DATA * playing, char type)
{
    char   i=0, total=0;
    
    for(i=0;i<ch->pcdata->stones->diceLeft;i++)
    {
        if(ch->pcdata->stones->dice[i]==type)
            total++;
    }
    
    for(i=0;i<playing->pcdata->stones->diceLeft;i++)
    {
        if(playing->pcdata->stones->dice[i]==type)
            total++;
    }
    
    return total;
}


//////////////////////////////////////////////////////////////////////////
// stones_print_dice
char * stones_print_dice(char dice[5], char highlightType)
{
    char             z, i;
    char *           allLines=0;
    char             lines[5][MAX_DICE_LINE_LENGTH];
        
    for(i=0;i<5;i++)
    {
        memset(lines[i], ' ', MAX_DICE_LINE_LENGTH);
        lines[i][MAX_DICE_LINE_LENGTH-1] = '\0';
    }
        
    for(z=0;z<5 && dice[z];z++)
    {
        for(i=0;i<5;i++)
        {
            memcpy(&(lines[i][strrchr(&(lines[i][0]), 'x') ? ((strrchr(&(lines[i][0]), 'x')-(&(lines[i][0])))+DIE_SPACER_WIDTH) : 0]),
                   dice[z]==highlightType ? dieStringsHighlighted[(5*(dice[z]-1))+i] : dieStrings[(5*(dice[z]-1))+i],
                   dice[z]==highlightType ? strlen(dieStringsHighlighted[(5*(dice[z]-1))+i]) : strlen(dieStrings[(5*(dice[z]-1))+i]));
        }
    }
    
    for(i=0;i<5;i++)
    {
        *(strrchr(lines[i], 'x')+1) = '\0';
        allLines = strappend(allLines, lines[i]);
        allLines = strappend(allLines, "\n\r");
    }
    
    return allLines;
}


//////////////////////////////////////////////////////////////////////////
// stones_show_wagers - Shows ch what each player has at stake
void    stones_show_wagers(CHAR_DATA * ch, CHAR_DATA * playing)
{
    char        buf[MAX_STRING_LENGTH];
    
    if(ch->pcdata->stones->objectWager)
        act("{BYou wagered{W: {x$p{B.{x", ch, ch->pcdata->stones->objectWager, NULL, TO_CHAR);
    else if(ch->pcdata->stones->moneyWager)
    {
        sprintf(buf, "{BYou wagered{W: {Y%ld gold{B.{x", ch->pcdata->stones->moneyWager);
        act(buf, ch, NULL, NULL, TO_CHAR);
    }
    else if(ch->pcdata->stones->questPointWager)
    {
        sprintf(buf, "{BYou wagered{W: {G%ld quest points{B.{x", ch->pcdata->stones->questPointWager);
        act(buf, ch, NULL, NULL, TO_CHAR);
    }
    else
        act("{BYou haven't wagered anything.{x", ch, NULL, NULL, TO_CHAR);
    
    if(playing)
    {
        if(playing->pcdata->stones->objectWager)
            act("{C$N{B wagered{W: {x$p{B.{x", ch, playing->pcdata->stones->objectWager, playing, TO_CHAR);
        else if(playing->pcdata->stones->moneyWager)
        {
            sprintf(buf, "{C$N{B wagered{W: {Y%ld gold{B.{x", playing->pcdata->stones->moneyWager);
            act(buf, ch, NULL, playing, TO_CHAR);
        }
        else if(playing->pcdata->stones->questPointWager)
        {
            sprintf(buf, "{C$N{B wagered{W: {G%ld quest points{B.{x", playing->pcdata->stones->questPointWager);
            act(buf, ch, NULL, playing, TO_CHAR);
        }
        else
            act("{C$N{B has not wagered anything.{x", ch, NULL, playing, TO_CHAR);
    }
}


//////////////////////////////////////////////////////////////////////////
// stones_winner - ch wins the game
void    stones_winner(CHAR_DATA *ch)
{
    char            buf[MAX_STRING_LENGTH];
    CHAR_DATA *     playing;
    
    playing = ch->pcdata->stones->playing;
    
    act("{B{WYou are the winner!!!{x", ch, NULL, playing, TO_CHAR);
    act("{C$n{B {Wis the winner!!!{x", ch, NULL, playing, TO_VICT);
    
    // Give them back what they wagered
    if(ch->pcdata->stones->objectWager)
        obj_to_char(ch->pcdata->stones->objectWager, ch);
    else if(ch->pcdata->stones->moneyWager)
        ch->gold+=ch->pcdata->stones->moneyWager;
    else if(ch->pcdata->stones->questPointWager)
        ch->questpoints+=ch->pcdata->stones->questPointWager;
    
    // Now give them what they have won!
    if(playing->pcdata->stones->objectWager)
    {
        act("{BYou win{W: {x$p{B!!!{x", ch, playing->pcdata->stones->objectWager, playing, TO_CHAR);
        act("{C$n{B wins{W: {x$p{B!!!{x", ch, playing->pcdata->stones->objectWager, playing, TO_VICT);

        obj_to_char(playing->pcdata->stones->objectWager, ch);
    }
    else if(playing->pcdata->stones->moneyWager)
    {
        sprintf(buf, "{BYou win{W: {Y%ld gold{B!!!{x", playing->pcdata->stones->moneyWager);
        act(buf, ch, NULL, playing, TO_CHAR);
        
        sprintf(buf, "{C$n{B wins{W: {Y%ld gold{B!!!{x", playing->pcdata->stones->moneyWager);
        act(buf, ch, NULL, playing, TO_VICT);
        
        ch->gold+=playing->pcdata->stones->moneyWager;
    }
    else if(playing->pcdata->stones->questPointWager)
    {
        sprintf(buf, "{BYou win{W: {G%ld quest points{B!!!{x", playing->pcdata->stones->questPointWager);
        act(buf, ch, NULL, playing, TO_CHAR);
        
        sprintf(buf, "{C$n{B wins{W: {G%ld quest points{B!!!{x", playing->pcdata->stones->questPointWager);
        act(buf, ch, NULL, playing, TO_VICT);
        
        ch->questpoints+=playing->pcdata->stones->questPointWager;
    }
    else
    {
        act("{C$N{B did not wager anything, therefore your only prize is bragging rights.{x", ch, NULL, playing, TO_CHAR);
        act("{BYou did not wager anything, therefore {C$n{B does not receive any prizes for winning.{x", ch, NULL, playing, TO_VICT);
    }
    
    act("\n\r{W*********************************************************{x", ch, NULL, playing, TO_ROOM);
    act("{C$n {Bjust beat {C$N{B in a match of Stones of Wisdom!{x", ch, NULL, playing, TO_ROOM);
    act("{W*********************************************************{x\n\r", ch, NULL, playing, TO_ROOM);

    stones_clear(ch->pcdata->stones);
    stones_clear(playing->pcdata->stones);
}


//////////////////////////////////////////////////////////////////////////
// stones_player_quit - Called whenever a player quits the mud
void    stones_player_quit(CHAR_DATA * ch)
{
    DESCRIPTOR_DATA *	d;
    CHAR_DATA *			original;
	
	if(!ch || !ch->pcdata || !ch->pcdata->stones)
	    return;
	
	// If they are in a game, they forfeit
	if(ch->pcdata->stones->playing)
	    stones_winner(ch->pcdata->stones->playing);
	else
	{
	    // Not in a game, so refund any bets
        if(ch->pcdata->stones->objectWager)
            obj_to_char(ch->pcdata->stones->objectWager, ch);
        else if(ch->pcdata->stones->moneyWager)
            ch->gold+=ch->pcdata->stones->moneyWager;
        else if(ch->pcdata->stones->questPointWager)
            ch->questpoints+=ch->pcdata->stones->questPointWager;
	}

	for(d=descriptor_list;d;d=d->next)
	{
		original = d->original ? d->original : d->character; /* if switched */
		if(!original || !original->pcdata || !original->pcdata->stones)
		    continue;
		
		// This player has invited the quitting player to a game, clear his stones
		if(original->pcdata->stones->invited==ch)
		{
		    // Refund the inviter his money
            if(original->pcdata->stones->objectWager)
                obj_to_char(original->pcdata->stones->objectWager, original);
            else if(original->pcdata->stones->moneyWager)
                original->gold+=original->pcdata->stones->moneyWager;
            else if(original->pcdata->stones->questPointWager)
                original->questpoints+=original->pcdata->stones->questPointWager;
            
            ch->questpoints+=ch->pcdata->stones->questPointWager;
		    act("{BYour invitation to {C$N{B to play a match of Stones has been withdrawn.{x", original, NULL, ch, TO_CHAR);
		    stones_clear(original->pcdata->stones);
		    
		    act("{C$N{B's invitation to you to play a match of Stones has been withdrawn.{x", ch, NULL, original, TO_CHAR);
		}
	}
	
	stones_clear(ch->pcdata->stones);
}


//////////////////////////////////////////////////////////////////////////
// stones_status - Shows status of the match, invintations
void    stones_status(CHAR_DATA *ch)
{
    DESCRIPTOR_DATA *	d;
    CHAR_DATA *			victim;
    char *              diceDisplay;
    char                buf[MAX_STRING_LENGTH];
    CHAR_DATA *         playing;
        
    // START DEBUG STATUS TEXT
    /*send_to_char("\n\r********** DEBUG TEXT - TEMPORARY **********\n\r", ch);
    sprintf(buf, "stones->state: %d\n\r", ch->pcdata->stones->state);
    send_to_char(buf, ch);
    sprintf(buf, "stones->invited: %s\n\r", ch->pcdata->stones->invited ? ch->pcdata->stones->invited->name : "Invalid");
    send_to_char(buf, ch);
    sprintf(buf, "stones->playing: %s\n\r", ch->pcdata->stones->playing ? ch->pcdata->stones->playing->name : "Invalid");
    send_to_char(buf, ch);
    sprintf(buf, "stones->objectWager: %s\n\r", ch->pcdata->stones->objectWager ? ch->pcdata->stones->objectWager->name : "Invalid");
    send_to_char(buf, ch);
    sprintf(buf, "stones->moneyWager: %ld\n\r", ch->pcdata->stones->moneyWager);
    send_to_char(buf, ch);
    sprintf(buf, "stones->questPointWager: %ld\n\r", ch->pcdata->stones->questPointWager);
    send_to_char(buf, ch);
    sprintf(buf, "stones->diceLeft: %d\n\r", ch->pcdata->stones->diceLeft);
    send_to_char(buf, ch);
    sprintf(buf, "stones->dice: %d %d %d %d %d\n\r", ch->pcdata->stones->dice[0], ch->pcdata->stones->dice[1], ch->pcdata->stones->dice[2], ch->pcdata->stones->dice[3], ch->pcdata->stones->dice[4]);
    send_to_char(buf, ch);
    sprintf(buf, "stones->myTurn: %s\n\r", ch->pcdata->stones->myTurn ? "True" : "False");
    send_to_char(buf, ch);
    sprintf(buf, "stones->bidDiceCount: %d\n\r", ch->pcdata->stones->bidDiceCount);
    send_to_char(buf, ch);
    sprintf(buf, "stones->bidDiceType: %d\n\r", ch->pcdata->stones->bidDiceType);
    send_to_char(buf, ch);
    sprintf(buf, "stones->bidDiceTypeFriendly: %s\n\r", ch->pcdata->stones->bidDiceTypeFriendly);
    send_to_char(buf, ch);
    sprintf(buf, "stones->issuedChallenge: %s\n\r", ch->pcdata->stones->issuedChallenge ? "True" : "False");
    send_to_char(buf, ch);
    sprintf(buf, "stones->challengeWorked: %s\n\r", ch->pcdata->stones->challengeWorked ? "True" : "False");
    send_to_char(buf, ch);
    sprintf(buf, "stones->pulse: %d\n\r", ch->pcdata->stones->pulse);
    send_to_char(buf, ch);
    sprintf(buf, "stones->movementWarning: %s\n\r", ch->pcdata->stones->movementWarning ? "True" : "False");
    send_to_char(buf, ch);
    sprintf(buf, "stones->diceBeingUsed: %s\n\r", ch->pcdata->stones->diceBeingUsed ? ch->pcdata->stones->diceBeingUsed->name : "Invalid");
    send_to_char(buf, ch);
    send_to_char("************** END DEBUG TEXT **************\n\r\n\r", ch);*/
    // END DEBUG STATUS TEXT
        
    if(!ch->pcdata->stones->playing)
    {
        // Show invitation status
        if(ch->pcdata->stones->invited)
        {
            act("{BYou have invited {C$N{B to a match of Stones.{x", ch, NULL, ch->pcdata->stones->invited, TO_CHAR);
            stones_show_wagers(ch, NULL);
            
            send_to_char("\n\r", ch);
        }
        
    	for(d=descriptor_list;d;d=d->next)
    	{
    		victim = d->original ? d->original : d->character; /* if switched */
    		if(!victim || !victim->pcdata || !victim->pcdata->stones)
    		    continue;
    		
    		if(victim->pcdata->stones->invited==ch)
    		{
                act("{C$N{B has invited you to a match of Stones.{x", ch, NULL, victim, TO_CHAR);
                if(victim->pcdata->stones->objectWager)
                    act("{C$N{B has wagered{W: {x$p{B.{x", ch, victim->pcdata->stones->objectWager, victim, TO_CHAR);
                else if(victim->pcdata->stones->moneyWager)
                {
                    sprintf(buf, "{C$N{B has wagered{W: {Y%ld gold{B.{x", victim->pcdata->stones->moneyWager);
                    act(buf, ch, NULL, victim, TO_CHAR);
                }
                else if(victim->pcdata->stones->questPointWager)
                {
                    sprintf(buf, "{C$N{B has wagered{W: {G%ld quest points{B.{x", victim->pcdata->stones->questPointWager);
                    act(buf, ch, NULL, victim, TO_CHAR);
                }
                else
                    act("{C$N{B has not wagered anything.{x", ch, NULL, victim, TO_CHAR);
                send_to_char("\n\r", ch);
    		}
    	}
    }
    else
    {
        playing = ch->pcdata->stones->playing;
        
        // Show current game status
        act("{BYou are playing a match with {C$N{B.{x", ch, NULL, ch->pcdata->stones->playing, TO_CHAR);
        stones_show_wagers(ch, playing);
        
        send_to_char("\n\r", ch);
        
        sprintf(buf, "{BYou have {W%d{B dice left.{x", ch->pcdata->stones->diceLeft);
        act(buf, ch, NULL, NULL, TO_CHAR);
        
        sprintf(buf, "{C$N{B has {W%d{B dice left.{x", playing->pcdata->stones->diceLeft);
        act(buf, ch, NULL, playing, TO_CHAR);
        
        switch(ch->pcdata->stones->state)
        {
            case(STONES_STATE_MATCH_STARTING):
            case(STONES_STATE_STAKES_SHOWN):
            case(STONES_STATE_ROLL_GO_FIRST):
                act("{BYou are waiting to roll and see who goes first.{x", ch, NULL, playing, TO_CHAR);
                break;
            case(STONES_STATE_BEGINNING_ROUND):
            case(STONES_STATE_WHO_GOES_FIRST):
                act("{BYou are waiting for the current round to start.{x", ch, NULL, playing, TO_CHAR);
                break;
            case(STONES_STATE_ROLL_DICE):
                if(ch->pcdata->stones->myTurn)
                    act("{BYou are waiting to take your turn.{x", ch, NULL, playing, TO_CHAR);
                else
                    act("{BYou are waiting for {C$N{B to take $S turn.{x", ch, NULL, playing, TO_CHAR);
                break;
            case(STONES_STATE_PLACE_BID):
            case(STONES_STATE_PLACE_BID_WARNING):
                if(ch->pcdata->stones->myTurn)
                {
                    if(playing->pcdata->stones->bidDiceCount)
                    {
                        sprintf(buf, "{BIt is currently your turn. {C$N{B bid {W%d %s{B. Either bid higher, or {Rchallenge{C $N{B's bid!{x", playing->pcdata->stones->bidDiceCount, playing->pcdata->stones->bidDiceTypeFriendly);
                        act(buf, ch, NULL, playing, TO_CHAR);
                    }
                    else
                        act("{BIt is currently your turn. It's the opening move, place your bid!{x", ch, NULL, playing, TO_CHAR);
                }
                else
                {
                    if(ch->pcdata->stones->bidDiceCount)
                    {
                        sprintf(buf, "{BIt is currently {C$N{B's turn. Your last bid was {W%d %s{B. {C$N{B will either bid higher, or {Rchallenge{B your bid!{x", ch->pcdata->stones->bidDiceCount, ch->pcdata->stones->bidDiceTypeFriendly);
                        act(buf, ch, NULL, playing, TO_CHAR);
                    }
                    else
                        act("{BIt is currently {C$N{B's turn.{x", ch, NULL, playing, TO_CHAR);
                }
                break;
            case(STONES_STATE_CHALLENGE_MADE):
            case(STONES_STATE_REVEAL_DICE):
                act("{BYou are waiting to see the outcome of the {Rchallenge{B!{x", ch, NULL, playing, TO_CHAR);
                break;
            case(STONES_STATE_CHALLENGE_OUTCOME):
                act("{BYou are waiting for the loser of the {Rchallenge{B to lose a die!{x", ch, NULL, playing, TO_CHAR);
                break;
            case(STONES_STATE_LOSE_DIE):
            case(STONES_STATE_MATCH_OUTCOME):
                if(ch->pcdata->stones->diceLeft==0 || playing->pcdata->stones->diceLeft==0)
                    act("{BThe match is over. You are waiting to see the outcome of the match.{x", ch, NULL, playing, TO_CHAR);
                else
                    act("{BYou are waiting for the next round to start.{x", ch, NULL, playing, TO_CHAR);
                break;
            default:
                act("{B*** CODE ERROR *** UNKNOWN GAME STATE!{x", ch, NULL, playing, TO_CHAR);
                break;
        }
        
        if(ch->pcdata->stones->state>=STONES_STATE_ROLL_DICE && ch->pcdata->stones->state<=STONES_STATE_LOSE_DIE)
        {
            diceDisplay = stones_print_dice(ch->pcdata->stones->dice, 0);
            
            act("\n\r{BYour current dice{W:{x\n\r", ch, NULL, playing, TO_CHAR);
            send_to_char(diceDisplay, ch);
            
            diceDisplay = strfree(diceDisplay);
        }
        
        if(ch->pcdata->stones->state>=STONES_STATE_REVEAL_DICE && ch->pcdata->stones->state<=STONES_STATE_LOSE_DIE)
        {
            diceDisplay = stones_print_dice(playing->pcdata->stones->dice, 0);
            
            act("\n\r{C$N{B's revealed dice{W:{x\n\r", ch, NULL, playing, TO_CHAR);
            send_to_char(diceDisplay, ch);
            
            diceDisplay = strfree(diceDisplay);
        }
    }
}


//////////////////////////////////////////////////////////////////////////
// stones_accept - Tries to accept a mutual invintation
void    stones_accept(CHAR_DATA *ch)
{    
    act("\n\r{BYou accept {C$N{B's invitation to play a match of Stones.{x", ch, NULL, ch->pcdata->stones->invited, TO_CHAR);
    ch->pcdata->stones->state = STONES_STATE_ACCEPTING;
    
    act("{C$n{B has accepted your invitation to play a match of Stones.{x", ch, NULL, ch->pcdata->stones->invited, TO_VICT);

    if(ch->pcdata->stones->invited->pcdata->stones->state==STONES_STATE_ACCEPTING)  // Other player has already accepted, let's begin!!
    {
        act("{BYou and {C$N{B are beginning a new match of Stones!{x", ch, NULL, ch->pcdata->stones->invited, TO_CHAR);
        ch->pcdata->stones->playing = ch->pcdata->stones->invited;
        ch->pcdata->stones->diceLeft = STONES_DICE_IN_GAME;
        ch->pcdata->stones->state = STONES_STATE_MATCH_STARTING;
        ch->pcdata->stones->pulse = STONES_DELAY_MATCH_STARTING*PULSE_PER_SECOND;
        
        act("{BYou and {C$n{B are beginning a new match of Stones!{x", ch, NULL, ch->pcdata->stones->invited, TO_VICT);
        ch->pcdata->stones->playing->pcdata->stones->playing = ch;
        ch->pcdata->stones->playing->pcdata->stones->diceLeft = STONES_DICE_IN_GAME;
        ch->pcdata->stones->playing->pcdata->stones->state = STONES_STATE_MATCH_STARTING;
        ch->pcdata->stones->playing->pcdata->stones->pulse = STONES_DELAY_MATCH_STARTING*PULSE_PER_SECOND;
    }
    else
    {
        act("{BAccept {C$n{B's invitation to a match of Stones when you are ready to play.{x", ch, NULL, ch->pcdata->stones->invited, TO_VICT);
    }
}


//////////////////////////////////////////////////////////////////////////
// stones_invite - A valid invite has happened, do the invite stuff
void stones_invite(CHAR_DATA * ch, CHAR_DATA * victim, OBJ_DATA * objectWager, long moneyWager, long questPointWager, OBJ_DATA * diceToUse)
{
    char        buf[MAX_STRING_LENGTH];

    // If I had already invited someone, and they have already accepted, remove the accept from their side    
    if(ch->pcdata->stones->invited && ch->pcdata->stones->invited->pcdata->stones->invited==ch && ch->pcdata->stones->invited->pcdata->stones->state==STONES_STATE_ACCEPTING)
    {
        ch->pcdata->stones->invited->pcdata->stones->state = STONES_STATE_INVITING;
        act("{C$n{B has changed their invite. Your acceptance of their invite has been cancelled.{x", ch, NULL, ch->pcdata->stones->invited, TO_VICT);
    }
    
    if(ch->pcdata->stones->objectWager)
        obj_to_char(ch->pcdata->stones->objectWager, ch);
    else if(ch->pcdata->stones->moneyWager)
        ch->gold+=ch->pcdata->stones->moneyWager;
    else if(ch->pcdata->stones->questPointWager)
        ch->questpoints+=ch->pcdata->stones->questPointWager;
    
    // Set up this players stones data
    stones_clear(ch->pcdata->stones);
    
    // Set up the new wagers
    if(objectWager!=NULL)
    {
        ch->pcdata->stones->objectWager = objectWager;
        obj_from_char(ch->pcdata->stones->objectWager);
    }
    else if(moneyWager>0)
    {
        ch->pcdata->stones->moneyWager = moneyWager;
        ch->gold-=ch->pcdata->stones->moneyWager;
    }
    else if(questPointWager>0)
    {
        ch->pcdata->stones->questPointWager = questPointWager;
        ch->questpoints-=ch->pcdata->stones->questPointWager;
    }
        
 	ch->pcdata->stones->diceBeingUsed = diceToUse;
    ch->pcdata->stones->invited = victim;
    ch->pcdata->stones->state = STONES_STATE_INVITING;
    
    // Inform players of the invitation
    if(victim->pcdata->stones->invited!=ch)     // One sided invitation so far
    {
        // Inform inviter
        act("\n\r{BYou have invited {C$N{B to a match of Stones!{x", ch, NULL, victim, TO_CHAR);
        stones_show_wagers(ch, NULL);
        
        // Inform invited
        act("{C$n{B has invited you to a match of Stones!{x\n\r", ch, NULL, victim, TO_VICT);
        if(ch->pcdata->stones->objectWager)
            act("{C$n{B has put up for wager{W: {x$p{B.{x", ch, ch->pcdata->stones->objectWager, victim, TO_VICT);
        else if(ch->pcdata->stones->moneyWager)
        {
            sprintf(buf, "{C$n{B has put up for wager {Y%ld gold{B.{x", ch->pcdata->stones->moneyWager);
            act(buf, ch, NULL, victim, TO_VICT);
        }
        else if(ch->pcdata->stones->questPointWager)
        {
            sprintf(buf, "{C$n{B has put up for wager {G%ld quest points{B.{x", ch->pcdata->stones->questPointWager);
            act(buf, ch, NULL, victim, TO_VICT);
        }
        else
            act("{C$n{B hasn't wagered anything, I guess $e want's a friendly match.{x", ch, NULL, victim, TO_VICT);
            
        act("\n\r{BYou have not invited {C$n{B yet, invite {C$n{B and place a wager.{x", ch, NULL, victim, TO_VICT);
    }
    else    // Both players have invited each other
    {
        // Inform inviter
        act("\n\r{BYou and {C$N{B have invited each other to a match of Stones.{x\n\r", ch, NULL, victim, TO_CHAR);
        stones_show_wagers(ch, ch->pcdata->stones->invited);
        act("\n\r{BAccept {C$N{B's invitation when you are ready to play!{x", ch, NULL, victim, TO_CHAR);
        
        // Inform invited
        act("{BYou and {C$n{B have invited each other to a match of Stones.{x\n\r", ch, NULL, victim, TO_VICT);
        stones_show_wagers(ch->pcdata->stones->invited, ch);
        act("\n\r{BAccept {C$n{B's invitation when you are ready to play!{x", ch, NULL, victim, TO_VICT);
    }
}


//////////////////////////////////////////////////////////////////////////
// stones_place_bid - A valid formed and timed bid has been made, check for other stuff, and then accept if okay
void    stones_place_bid(CHAR_DATA * ch, char num, char type)
{
    CHAR_DATA *     playing;
    char            minNumber, maxNumber, lastNumber, lastType;
	char            buf[MAX_STRING_LENGTH];
    
    playing = ch->pcdata->stones->playing;
    
    lastNumber = playing->pcdata->stones->bidDiceCount;
    lastType = playing->pcdata->stones->bidDiceType;
    minNumber = UMAX(1, lastNumber);
    maxNumber = (ch->pcdata->stones->diceLeft+playing->pcdata->stones->diceLeft);
    
    if(num>maxNumber || num<minNumber)
    {
        sprintf(buf, "{BNumber of dice must be between {W%d{B and {W%d{B.{x", minNumber, maxNumber);
        act(buf, ch, NULL, playing, TO_CHAR);
        return;
    }
    
    if(type<1 || type>6)
    {
        act("{BType of dice must be either {W1{B, {W2{B, {W3{B, {W4{B, {W5{B or {W6{B.{x", ch, NULL, playing, TO_CHAR);
        return;
    }
    
    if(num==minNumber && lastType>0 && type<=lastType)
    {
        sprintf(buf, "{BYou must bid higher than standing bid of {W%d %s{B or {Rchallenge{B the bid.{x", lastNumber, playing->pcdata->stones->bidDiceTypeFriendly);
        act(buf, ch, NULL, playing, TO_CHAR);
        return;
    }
    
    // We have a valid bid, record it
    ch->pcdata->stones->bidDiceCount = num;
    ch->pcdata->stones->bidDiceType = type;
    
    if(num==1)
        sprintf(ch->pcdata->stones->bidDiceTypeFriendly, "%s", type==1 ? "one" : (type==2 ? "two" : (type==3 ? "three" : (type==4 ? "four" : (type==5 ? "five" : (type==6 ? "six" : ""))))));
    else
        sprintf(ch->pcdata->stones->bidDiceTypeFriendly, "%s", type==1 ? "ones" : (type==2 ? "twos" : (type==3 ? "threes" : (type==4 ? "fours" : (type==5 ? "fives" : (type==6 ? "sixes" : ""))))));
    
    sprintf(buf, "{BYou bid {W%d %s{B.{x", ch->pcdata->stones->bidDiceCount, ch->pcdata->stones->bidDiceTypeFriendly);
    act(buf, ch, NULL, playing, TO_CHAR);
    
    sprintf(buf, "{C$n{B bids {W%d %s{B.{x", ch->pcdata->stones->bidDiceCount, ch->pcdata->stones->bidDiceTypeFriendly);
    act(buf, ch, NULL, playing, TO_VICT);
    
    ch->pcdata->stones->myTurn = FALSE;
    ch->pcdata->stones->state = STONES_STATE_ROLL_DICE;
    ch->pcdata->stones->pulse = STONES_DELAY_BID_MADE*PULSE_PER_SECOND;

    playing->pcdata->stones->myTurn = TRUE;
    playing->pcdata->stones->state = STONES_STATE_ROLL_DICE;
    playing->pcdata->stones->pulse = STONES_DELAY_BID_MADE*PULSE_PER_SECOND;
}


//////////////////////////////////////////////////////////////////////////
// stones_challenge - A challenge has been made by the active player
void    stones_challenge(CHAR_DATA * ch)
{
    CHAR_DATA *     playing;

    playing = ch->pcdata->stones->playing;

    if(playing->pcdata->stones->bidDiceType<1)
    {
        act("{BIt is the opening move, there is no bid to challenge. You must bid.{x", ch, NULL, playing, TO_CHAR);
        return;
    }
    
    act("{BYou {RCHALLENGE {C$N{B's bid!{x", ch, NULL, playing, TO_CHAR);
    act("{C$n{B has {RCHALLENGED{B your bid!!{x", ch, NULL, playing, TO_VICT);
    
    ch->pcdata->stones->issuedChallenge = TRUE;
    ch->pcdata->stones->myTurn = FALSE;
    ch->pcdata->stones->state = STONES_STATE_CHALLENGE_MADE;
    ch->pcdata->stones->pulse = STONES_DELAY_CHALLENGE_MADE*PULSE_PER_SECOND;

    playing->pcdata->stones->issuedChallenge = FALSE;
    playing->pcdata->stones->myTurn = TRUE;
    playing->pcdata->stones->state = STONES_STATE_CHALLENGE_MADE;
    playing->pcdata->stones->pulse = STONES_DELAY_CHALLENGE_MADE*PULSE_PER_SECOND;
}


//////////////////////////////////////////////////////////////////////////
// stones_object_leaving - Called whenever an object on the mud is leaving a ROOM
void stones_object_leaving(OBJ_DATA * obj)
{
    DESCRIPTOR_DATA *	d;
    CHAR_DATA *			original;
	
	for(d=descriptor_list;d;d=d->next)
	{
		original = d->original ? d->original : d->character; /* if switched */
		if(!original || !original->pcdata || !original->pcdata->stones || !original->pcdata->stones->diceBeingUsed)
		    continue;
		    
		if(original->pcdata->stones->diceBeingUsed==obj)
		    stones_player_quit(original);
	}
}


//////////////////////////////////////////////////////////////////////////
// stones_try_moving - Called when a player on the mud tries moving. Return false to disallow movement
bool stones_try_moving(CHAR_DATA * ch)
{
    CHAR_DATA * playing=0;
    
    if(!ch || !ch->pcdata || !ch->pcdata->stones)
        return TRUE;
    
    if(!ch->pcdata->stones->invited)
    {
        stones_player_quit(ch);
        return TRUE;
    }
    
    if(ch->pcdata->stones->invited && !ch->pcdata->stones->playing)
    {
        // invited, but not playing yet, let them move but clear the stones
        act("\n\r{BYou have left the room, your stones invitation has been withdrawn.{x", ch, NULL, NULL, TO_CHAR);        
        stones_player_quit(ch);
        return TRUE;
    }
    
    playing = ch->pcdata->stones->playing;
    
    if(ch->pcdata->stones->movementWarning==FALSE)
    {
        // Have not been warned yet, warn them, set the flag and don't allow them to move
        act("\n\r{BYou have attempted to leave the room. If you try again you will forfeit the match of Stones you are playing.{x", ch, NULL, NULL, TO_CHAR);
        ch->pcdata->stones->movementWarning = TRUE;
        return FALSE;
    }
    
    act("\n\r{BYou have left the room and forfeit your Stones of Wisdom match.{x", ch, NULL, playing, TO_CHAR);
    act("{C$n{B has left the room and has forfeited your Stones of Wisdom match.{x", ch, NULL, playing, TO_VICT);
    
    stones_winner(playing);
    
    return TRUE;
}


//////////////////////////////////////////////////////////////////////////
// do_stones - The entry function for 'stones' command, just show current status right now
void    do_stones(CHAR_DATA * ch, char * argument)
{
    if(!ch->pcdata || !ch->pcdata->stones)
    {
		send_to_char("\n\r{BYou are not able to play a match of Stones right now.{x\n\r", ch);
 		return;
    }
    
    stones_status(ch);
}


//////////////////////////////////////////////////////////////////////////
// do_stones_invite - The entry function for 'invite' command
void    do_stones_invite(CHAR_DATA * ch, char * argument)
{
    char                arg1[MAX_INPUT_LENGTH];
	CHAR_DATA *			victim=0;
	OBJ_DATA *          diceToUse;
	OBJ_DATA *          obj=0;

    if(!ch->pcdata || !ch->pcdata->stones)
    {
		send_to_char("\n\r{BYou are not able to play a match of Stones right now.{x\n\r", ch);
 		return;
    }
    
	argument = one_argument(argument, arg1);
	
	if(!strcasecmp(arg1, "withdraw"))
	{
	    stones_player_quit(ch);
	    return;
	}

	if(arg1[0]=='\0' || argument[0]=='\0')
	{
		return;
	}
	
	// Verify player
	if(ch->pcdata->stones->playing)
	{
        send_to_char("\n\r{BYou are already playing a match of Stones right now.{x\n\r", ch);
	    return;
	}

	if((victim=get_char_room(ch, arg1))==NULL || IS_NPC(victim) || !victim->pcdata)
	{
        send_to_char("\n\r{BThere is no such player in the room right now.{x\n\r", ch);
	    return;
	}
	
	if(!victim->pcdata->stones)
	{
	    act("\n\r{C$N{B is not able to play a match right now.{x", ch, NULL, victim, TO_CHAR);
	    return;
	}
	
	if(victim->pcdata->stones->playing && victim->pcdata->stones->playing!=ch)
	{
	    act("\n\r{C$N{B is currently playing someone else.{x", ch, NULL, victim, TO_CHAR);
	    return;
	}
	
	if(ch==victim)
	{
	    act("\n\r{BYou can't play Stones of Wisdom with yourself.{x", ch, NULL, NULL, TO_CHAR);
        return;	
	}
	
	// Check for available dice
	if(!(diceToUse=stones_has_dice(ch->carrying)) && !(diceToUse=stones_has_dice(victim->carrying)) && !(diceToUse=stones_has_dice(ch->in_room->contents)))
	{
	    act("\n\r{BNeither you nor {C$N{B have any dice. Get some dice or find a room that has dice.{x", ch, NULL, victim, TO_CHAR);
	    return;
	}
	
	// Verify wager
	if(strendswith(argument, "qp") && argument[0]>='0' && argument[0]<='9')
	{
	    // Verify quest point wager
	    *strchr(argument, 'q') = '\0';
	    
	    if(ch->questpoints<atol(argument))
	    {
		    send_to_char("\n\r{BYou don't have that may quest points to wager.{x\n\r", ch);
		    return;
	    }
	    
	    stones_invite(ch, victim, NULL, 0, atol(argument), diceToUse);
	}
	else if(is_number(argument))
	{
	    // Verify gold amount
	    if(ch->gold<atol(argument))
	    {
		    send_to_char("\n\r{BYou do not have that much gold to wager.{x\n\r", ch);
		    return;
	    }
	    
	    stones_invite(ch, victim, NULL, atol(argument), 0, diceToUse);
	}
	else
	{
	    // Verify object wager
		if((obj=get_obj_carry(ch, argument, ch))==NULL)
		{
		    send_to_char("\n\r{BYou do not have that item to wager.{x\n\r", ch);
		    return;
		}
		
		if(!can_drop_obj(ch, obj))
		{
		    act("\n\r{BYou can't let go of {x$p{B to wager it.{x", ch, obj, victim, TO_CHAR);
		    return;
		}
		
		if(obj->wear_loc != WEAR_INVENTORY)
		{
		    act("\n\r{BYou must remove {x$p{B before trying to wager it.{x", ch, obj, victim, TO_CHAR);
		    return;
		}
    
        if((victim->carry_number+get_obj_number(obj))>can_carry_n(victim))
        {
    	    act("\n\r{C$N{B has $S hands full!{x", ch, NULL, victim, TO_CHAR);
    	    return;
        }
    
        if((get_carry_weight(victim)+get_obj_weight(obj))>can_carry_w(victim))
        {
    	    act("\n\r{C$N{B can't carry that much weight!{x", ch, NULL, victim, TO_CHAR);
    	    return;
        }
    
        if(!can_see_obj(victim, obj))
        {
    	    act( "\n\r{C$N{B can't see {x$p{B.{x", ch, obj, victim, TO_CHAR);
    	    return;
        }

        if(same_obj(obj, victim->carrying)>=30)
        {
	        act("\n\r{C$N{B has too many {x$p{B already.{x", ch, obj, victim, TO_CHAR);
            return;
        }   
             		
        if(IS_SET(obj->extra_flags, ITEM_CLAN_EQ) && (!IS_SET(victim->pact, PLR_PKILLER)))
        {
	        act("\n\r{C$N{B must PKill to use {x$p{B.{x", ch, obj, victim, TO_CHAR);
            return;
        }
        
        stones_invite(ch, victim, obj, 0, 0, diceToUse);
    }
}


//////////////////////////////////////////////////////////////////////////
// do_stones_accept - The entry function for 'accept' command
void    do_stones_accept(CHAR_DATA * ch, char * argument)
{
    if(!ch->pcdata || !ch->pcdata->stones)
    {
		send_to_char("\n\r{BYou are not able to play a match of Stones right now.{x\n\r", ch);
 		return;
    }
    
    if(!ch->pcdata->stones->invited)
    {
        send_to_char("\n\r{BYou have not invited anyone to play yet.{x\n\r", ch);
        return;
    }
    
    if(ch->pcdata->stones->invited->pcdata->stones->invited!=ch)
    {
        act("\n\r{C$N{B as not invited you to play yet.{x", ch, NULL, ch->pcdata->stones->invited, TO_CHAR);
        return;
    }
    
    if(ch->pcdata->stones->state==STONES_STATE_ACCEPTING)
    {
        act("\n\r{BYou have already accepted the invitation from {C$N{B.{x", ch, NULL, ch->pcdata->stones->invited, TO_CHAR);
        return;
    }

    stones_accept(ch);
}


//////////////////////////////////////////////////////////////////////////
// do_stones_bid - The entry function for 'bid' command
void    do_stones_bid(CHAR_DATA * ch, char * argument)
{
   char                arg1[MAX_INPUT_LENGTH];
   char                arg2[MAX_INPUT_LENGTH];

   if(!ch->pcdata || !ch->pcdata->stones)
   {
   	   send_to_char("\n\r{BYou are not able to play a match of Stones right now.{x\n\r", ch);
       return;
   }
    
   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);
   
   if(!ch->pcdata->stones->playing)
   {
       act("{BYou are not currently playing Stones of Wisdom, invite someone to a game!{x", ch, NULL, NULL, TO_CHAR);
       return;
   }
   
   if(ch->pcdata->stones->state!=STONES_STATE_PLACE_BID && ch->pcdata->stones->state!=STONES_STATE_PLACE_BID_WARNING)
   {
       act("{BRight now is not the time to place bids.{x", ch, NULL, NULL, TO_CHAR);
       return;
   }
   
   if(ch->pcdata->stones->myTurn==FALSE)
   {
       act("{BIt is not your turn.{x", ch, NULL, NULL, TO_CHAR);
       return;
   }
   
   // I'm playing, it's the state to place bids, and it's my turn
   if(arg1[0]=='\0' || arg2[0]=='\0')
   {
		send_to_char("\n\r{BIn order to place your bid, use the following command:{x\n\r", ch);
	    send_to_char("{B  {Cstones bid {W<number of dice> <type of die>{x\n\r\n\r", ch);
   }
   
   if(!is_number(arg1) || !is_number(arg2))
   {
       act("{BThe number and type of dice must both be numbers.{x", ch, NULL, NULL, TO_CHAR);
       return;
   }
   
   stones_place_bid(ch, atoi(arg1), atoi(arg2));
}


//////////////////////////////////////////////////////////////////////////
// do_stones_challenge - The entry function for 'challenge' command
void    do_stones_challenge(CHAR_DATA * ch, char * argument)
{
    if(!ch->pcdata || !ch->pcdata->stones)
    {
       send_to_char("\n\r{BYou are not able to play a match of Stones right now.{x\n\r", ch);
       return;
    }
    
    if(!ch->pcdata->stones->playing)
    {
       act("{BYou are not currently playing Stones of Wisdom, invite someone to a game!{x", ch, NULL, NULL, TO_CHAR);
       return;
    }
    
    if(ch->pcdata->stones->state!=STONES_STATE_PLACE_BID && ch->pcdata->stones->state!=STONES_STATE_PLACE_BID_WARNING)
    {
       act("{BRight now is not the time to {Rchallenge{B.{x", ch, NULL, NULL, TO_CHAR);
       return;
    }
    
    if(ch->pcdata->stones->myTurn==FALSE)
    {
       act("{BIt is not your turn.{x", ch, NULL, NULL, TO_CHAR);
       return;
    }
    
    stones_challenge(ch);
}


//////////////////////////////////////////////////////////////////////////
// do_stones_forfeit - The entry function for 'forfeit' command
void    do_stones_forfeit(CHAR_DATA * ch, char * argument)
{
    if(!ch->pcdata || !ch->pcdata->stones)
    {
       send_to_char("\n\r{BYou are not able to play a match of Stones right now.{x\n\r", ch);
       return;
    }
    
    if(!ch->pcdata->stones->playing)
    {
        send_to_char("{BYou are not currently playing Stones of Wisdom, invite someone to a game!{x\n\r", ch);
        return;
    }
    
    send_to_char("{BYou forfeit the match.{x\n\r", ch);
    stones_winner(ch->pcdata->stones->playing);
}


//////////////////////////////////////////////////////////////////////////
// stones_update - Is called every tick to perform stones of wisdom updates
void	stones_update(void)
{
    DESCRIPTOR_DATA *	d;
    CHAR_DATA *			original;
    CHAR_DATA *         playing;
    char *              diceDisplay;
    char                buf[MAX_STRING_LENGTH];
    char                i, actualCount;
    char                friendlyDiceType[7];

    // Handle our updates	
	for(d=descriptor_list;d;d=d->next)
	{
		original = d->original ? d->original : d->character; /* if switched */
		if(!original || !original->pcdata || !original->pcdata->stones || !(playing=original->pcdata->stones->playing))
		    continue;
		
        if(original->pcdata->stones->state<STONES_STATE_MATCH_STARTING)
	        continue;
	        
	    if(original->pcdata->stones->pulse>0)
	    {
    	    original->pcdata->stones->pulse--;
    	    continue;
    	}
	    
	    switch(original->pcdata->stones->state)
	    {
	        case(STONES_STATE_MATCH_STARTING):     // Show our wagers
	           stones_show_wagers(original, playing);
	           
	           original->pcdata->stones->state = STONES_STATE_STAKES_SHOWN;
	           original->pcdata->stones->pulse = STONES_DELAY_STAKES_SHOWN*PULSE_PER_SECOND;
	           break;
	        case(STONES_STATE_STAKES_SHOWN):       // Inform of rolling (roll dice in background)
	           act("{BYou and {C$N{B each roll a die to see who goes first.{x", original, NULL, playing, TO_CHAR);
	           
	           original->pcdata->stones->dice[0] = number_range(1, 6);

	           original->pcdata->stones->state = STONES_STATE_ROLL_GO_FIRST;
	           original->pcdata->stones->pulse = STONES_DELAY_ROLL_GO_FIRST*PULSE_PER_SECOND;
	           break;
	        case(STONES_STATE_ROLL_GO_FIRST):      // Reveal roll results
	           diceDisplay = stones_print_dice(original->pcdata->stones->dice, 0);
	           
	           act("\n\r{BYou rolled{W:{x", original, NULL, playing, TO_CHAR);
	           send_to_char(diceDisplay, original);
	           sprintf(buf, "{BYou rolled a {W%d{B.{x", original->pcdata->stones->dice[0]);
	           act(buf, original, NULL, playing, TO_CHAR);
	           
	           act("\n\r{C$n{B rolled{W:{x", original, NULL, playing, TO_VICT);
	           send_to_char(diceDisplay, playing);
	           sprintf(buf, "{C$n{B rolled a {W%d{B.{x", original->pcdata->stones->dice[0]);
	           act(buf, original, NULL, playing, TO_VICT);
	           
	           diceDisplay = strfree(diceDisplay);
	           
	           original->pcdata->stones->state = STONES_STATE_BEGINNING_ROUND;
	           original->pcdata->stones->pulse = STONES_DELAY_BEGINNING_ROUND*PULSE_PER_SECOND;
	           break;
	        case(STONES_STATE_BEGINNING_ROUND):    // Reveal who goes first, might have to set if just rolled
	           if(original->pcdata->stones->diceLeft==STONES_DICE_IN_GAME && playing->pcdata->stones->diceLeft==STONES_DICE_IN_GAME)
	           {
	               // First round, we just rolled to see who goes first
	               if(original->pcdata->stones->dice[0]==playing->pcdata->stones->dice[0])
	               {
	                   act("{BYou and {C$N{B rolled the same thing! Rolling again!{x", original, NULL, playing, TO_CHAR);
	                   
        	           original->pcdata->stones->state = STONES_STATE_STAKES_SHOWN;
        	           original->pcdata->stones->pulse = STONES_DELAY_REROLL_TIE*PULSE_PER_SECOND;
        	           break;
	               }
	               else if(original->pcdata->stones->dice[0]>playing->pcdata->stones->dice[0])
	                   original->pcdata->stones->myTurn = TRUE;
                   else
	                   original->pcdata->stones->myTurn = FALSE;
	           }
	           
	           if(original->pcdata->stones->myTurn==TRUE)
	               act("{BYou go first!{x", original, NULL, playing, TO_CHAR);
	           else
	               act("{C$N{B goes first!{x", original, NULL, playing, TO_CHAR);
	            
	           // Beginning of a new round, clear the betting status
	           original->pcdata->stones->bidDiceCount = 0;
	           original->pcdata->stones->bidDiceType = 0;
	           original->pcdata->stones->bidDiceTypeFriendly[0] = '\0';
	           original->pcdata->stones->issuedChallenge = FALSE;
	           original->pcdata->stones->challengeWorked = FALSE;
	           
	           original->pcdata->stones->state = STONES_STATE_WHO_GOES_FIRST;
	           original->pcdata->stones->pulse = STONES_DELAY_WHO_GOES_FIRST*PULSE_PER_SECOND;
	           break;
	        case(STONES_STATE_WHO_GOES_FIRST):     // State that dice have been rolled, show player his dice
	           for(i=0;i<original->pcdata->stones->diceLeft;i++)
	           {
	               original->pcdata->stones->dice[i] = number_range(1, 6);
	           }
	           for(;i<STONES_DICE_IN_GAME;i++)
	           {
	               original->pcdata->stones->dice[i] = 0;
	           }
	           
	           sprintf(buf, "\n\r{C$n{B rolls {W%d{B %s.{x\n\r", original->pcdata->stones->diceLeft, original->pcdata->stones->diceLeft==1 ? "die" : "dice");
	           act(buf, original, NULL, playing, TO_VICT);
	           
               sprintf(buf, "{BYou roll {W%d {B%s, they are{W:{x", original->pcdata->stones->diceLeft, original->pcdata->stones->diceLeft==1 ? "die" : "dice");
	           act(buf, original, NULL, playing, TO_CHAR);
	           
	           diceDisplay = stones_print_dice(original->pcdata->stones->dice, 0);
               send_to_char(diceDisplay, original);

               diceDisplay = strfree(diceDisplay);

	           original->pcdata->stones->state = STONES_STATE_ROLL_DICE;
	           original->pcdata->stones->pulse = STONES_DELAY_ROLL_DICE*PULSE_PER_SECOND;
	           break;
	        case(STONES_STATE_ROLL_DICE):      // Dice have been rolled, inform player of who's turn it is to bid and how long they have
	           if(original->pcdata->stones->myTurn==FALSE)
	           {
	               // Not my turn, inform me that the other player has x seconds to bid or challenge
	               sprintf(buf, "{C$N{B has {W%d{B seconds to place a bid%s.{x", STONES_DELAY_PLACE_BID+STONES_DELAY_PLACE_BID_WARNING, 
                           original->pcdata->stones->bidDiceCount ? ", or {Rchallenge{B your last bid" : "");
	           }
	           else
	           {
	               // My turn! Inform me to take my turn
	               if(playing->pcdata->stones->bidDiceCount)
	                   sprintf(buf, "{BYou have {W%d{B seconds to either {Rchallenge {C$N{B's bid or place your own bid.{x", STONES_DELAY_PLACE_BID+STONES_DELAY_PLACE_BID_WARNING);
	               else
	                   sprintf(buf, "{BYou have the opening move! You have {W%d{B seconds to place your bid.{x", STONES_DELAY_PLACE_BID+STONES_DELAY_PLACE_BID_WARNING);
	           }
	            
	           act(buf, original, NULL, playing, TO_CHAR);
               original->pcdata->stones->state = STONES_STATE_PLACE_BID;
               original->pcdata->stones->pulse = STONES_DELAY_PLACE_BID*PULSE_PER_SECOND;
	           break;
	        case(STONES_STATE_PLACE_BID):      // Give the player one final warning that they have to place their bid!
	           if(original->pcdata->stones->myTurn==FALSE)
	           {
	               // Not my turn haha
	               if(original->pcdata->stones->bidDiceCount)
	                   sprintf(buf, "{C$N{B only has {W%d{B seconds left to place a bid or {Rchallenge{B your bid!{x", STONES_DELAY_PLACE_BID_WARNING);
	               else
	                   sprintf(buf, "{C$N{B only has {W%d{B seconds left to place a bid!{x", STONES_DELAY_PLACE_BID_WARNING);
	           }
	           else
	           {
	               // It is my turn!
	               if(original->pcdata->stones->bidDiceCount)
	                   sprintf(buf, "{BYou only have {W%d{B seconds left to place your bid or {Rchallenge {C$N{B's bid!{x", STONES_DELAY_PLACE_BID_WARNING);
	               else
	                   sprintf(buf, "{BYou only have {W%d{B seconds left to place your bid!{x", STONES_DELAY_PLACE_BID_WARNING);
	           }
	           
	           act(buf, original, NULL, playing, TO_CHAR);
               original->pcdata->stones->state = STONES_STATE_PLACE_BID_WARNING;
               original->pcdata->stones->pulse = STONES_DELAY_PLACE_BID_WARNING*PULSE_PER_SECOND;
	           break;
	        case(STONES_STATE_PLACE_BID_WARNING):      // Player did not make their bid in time, this code only executes for ONE of the players
	           if(original->pcdata->stones->myTurn==FALSE)
	           {
	               // I'm not the one who lost
	               act("{C$N{B has not placed a bid or challenged in the time allowed.{x", original, NULL, playing, TO_CHAR);
	               act("{C$N{B forfeits the match!{x", original, NULL, playing, TO_CHAR);
	               
	               act("{BYou have not placed your bid or challenged in the time allowed.{x", original, NULL, playing, TO_VICT);
	               act("{BYou forfeit the match!{x", original, NULL, playing, TO_VICT);
	               
	               stones_winner(original);
	           }
	           else
	           {
	               // This is the player who has lost
	               act("{C$n{B has not placed a bid or challenged in the time allowed.{x", original, NULL, playing, TO_VICT);
	               act("{C$n{B forfeits the match!{x", original, NULL, playing, TO_VICT);
	               
	               act("{BYou have not placed your bid or challenged in the time allowed.{x", original, NULL, playing, TO_CHAR);
	               act("{BYou forfeit the match!{x", original, NULL, playing, TO_CHAR);
	               
	               stones_winner(playing);
	           }
	           
	           // Both original and playing are no longer playing, thus don't do anything else with them!!!
	           break;
	        case(STONES_STATE_CHALLENGE_MADE):     // A challenge has been made by the active player, reveal dice
               act("{BYou reveal your dice{W:{x", original, NULL, playing, TO_CHAR);                 
	           diceDisplay = stones_print_dice(original->pcdata->stones->dice, original->pcdata->stones->issuedChallenge ? playing->pcdata->stones->bidDiceType : original->pcdata->stones->bidDiceType);
               send_to_char(diceDisplay, original);
               diceDisplay = strfree(diceDisplay);
               
               act("\n\r{C$N{B reveals $S dice{W:{x", original, NULL, playing, TO_CHAR);
	           diceDisplay = stones_print_dice(playing->pcdata->stones->dice, original->pcdata->stones->issuedChallenge ? playing->pcdata->stones->bidDiceType : original->pcdata->stones->bidDiceType);
               send_to_char(diceDisplay, original);
               diceDisplay = strfree(diceDisplay);
               
               original->pcdata->stones->state = STONES_STATE_REVEAL_DICE;
               original->pcdata->stones->pulse = STONES_DELAY_REVEAL_DICE*PULSE_PER_SECOND;               
	           break;
	        case(STONES_STATE_REVEAL_DICE):        // The dice have been revealed, show the outcome of the challenge!
	            if(original->pcdata->stones->issuedChallenge==FALSE)
	            {
	                // Not the challenger
                    sprintf(buf, "{BThe {Rchallenged{B bid was {W%d %s{B, bid by you!{x", original->pcdata->stones->bidDiceCount, original->pcdata->stones->bidDiceTypeFriendly);
                    actualCount = stones_count_dice(original, playing, original->pcdata->stones->bidDiceType);
                    if(actualCount==1)
                        sprintf(friendlyDiceType, "%s", original->pcdata->stones->bidDiceType==1 ? "one" : (original->pcdata->stones->bidDiceType==2 ? "two" : (original->pcdata->stones->bidDiceType==3 ? "three" : (original->pcdata->stones->bidDiceType==4 ? "four" : (original->pcdata->stones->bidDiceType==5 ? "five" : (original->pcdata->stones->bidDiceType==6 ? "six" : ""))))));
                    else
                        sprintf(friendlyDiceType, "%s", original->pcdata->stones->bidDiceType==1 ? "ones" : (original->pcdata->stones->bidDiceType==2 ? "twos" : (original->pcdata->stones->bidDiceType==3 ? "threes" : (original->pcdata->stones->bidDiceType==4 ? "fours" : (original->pcdata->stones->bidDiceType==5 ? "fives" : (original->pcdata->stones->bidDiceType==6 ? "sixes" : ""))))));
                }
                else
                {
                    // The challenger
                    sprintf(buf, "{BThe {Rchallenged{B bid was {W%d %s{B, bid by {C$N{B.{x", playing->pcdata->stones->bidDiceCount, playing->pcdata->stones->bidDiceTypeFriendly);
                    actualCount = stones_count_dice(original, playing, playing->pcdata->stones->bidDiceType);                
                    if(actualCount==1)
                        sprintf(friendlyDiceType, "%s", playing->pcdata->stones->bidDiceType==1 ? "one" : (playing->pcdata->stones->bidDiceType==2 ? "two" : (playing->pcdata->stones->bidDiceType==3 ? "three" : (playing->pcdata->stones->bidDiceType==4 ? "four" : (playing->pcdata->stones->bidDiceType==5 ? "five" : (playing->pcdata->stones->bidDiceType==6 ? "six" : ""))))));
                    else
                        sprintf(friendlyDiceType, "%s", playing->pcdata->stones->bidDiceType==1 ? "ones" : (playing->pcdata->stones->bidDiceType==2 ? "twos" : (playing->pcdata->stones->bidDiceType==3 ? "threes" : (playing->pcdata->stones->bidDiceType==4 ? "fours" : (playing->pcdata->stones->bidDiceType==5 ? "fives" : (playing->pcdata->stones->bidDiceType==6 ? "sixes" : ""))))));
                }
                
                act(buf, original, NULL, playing, TO_CHAR);
                    
                if(actualCount==0)
                    sprintf(buf, "{BThere are {Wno %s{B!{x", friendlyDiceType);

	            if(original->pcdata->stones->issuedChallenge==FALSE)
	            {
	                // Not the challenger
                    if(actualCount>=original->pcdata->stones->bidDiceCount)
                    {
                        original->pcdata->stones->challengeWorked = FALSE;
                        act("{C$N{B's {RCHALLENGE{B failed!{x", original, NULL, playing, TO_CHAR);
                        if(actualCount>0)
                            sprintf(buf, "{BThere %s {W%d %s{B!{x", actualCount>1 ? "are" : "is", actualCount, friendlyDiceType);
                    }
                    else
                    {
                        original->pcdata->stones->challengeWorked = TRUE;
                        act("{C$N{B's {RCHALLENGE{B was successful!{x", original, NULL, playing, TO_CHAR);
                        if(actualCount>0)
                            sprintf(buf, "{BThere %s only {W%d %s{B!{x", actualCount>1 ? "are" : "is", actualCount, friendlyDiceType);
                    }
               }
               else
               {
                    // The challenger
                    if(actualCount>=playing->pcdata->stones->bidDiceCount)
                    {
                        original->pcdata->stones->challengeWorked = FALSE;
                        act("{BYour {RCHALLENGE{B failed!{x", original, NULL, playing, TO_CHAR);
                        if(actualCount>0)
                            sprintf(buf, "{BThere %s {W%d %s{B!{x", actualCount>1 ? "are" : "is", actualCount, friendlyDiceType);
                    }
                    else
                    {
                        original->pcdata->stones->challengeWorked = TRUE;
                        act("{BYour {RCHALLENGE{B was successful!{x", original, NULL, playing, TO_CHAR);
                        if(actualCount>0)
                            sprintf(buf, "{BThere %s only {W%d %s{B!{x", actualCount>1 ? "are" : "is", actualCount, friendlyDiceType);
                    }
               }
                    
               act(buf, original, NULL, playing, TO_CHAR);
               
               original->pcdata->stones->state = STONES_STATE_CHALLENGE_OUTCOME;
               original->pcdata->stones->pulse = STONES_DELAY_CHALLENGE_OUTCOME*PULSE_PER_SECOND;               
	           break;
	        case(STONES_STATE_CHALLENGE_OUTCOME):      // Challenge outcome stated, someone loses a die now
	           if(original->pcdata->stones->issuedChallenge==FALSE)
	           {
	                // Not the challenger
	                if(original->pcdata->stones->challengeWorked==TRUE)
	                {
	                   act("{BYou have lost a die!!{x", original, NULL, playing, TO_CHAR);
	                   original->pcdata->stones->diceLeft--;
	                }
	                else
	                   act("{C$N{B has lost a die!!{x", original, NULL, playing, TO_CHAR);
               }
               else
               {
                    // The challenger
	                if(original->pcdata->stones->challengeWorked==TRUE)
	                   act("{C$N{B has lost a die!!{x", original, NULL, playing, TO_CHAR);
	                else
	                {
	                   act("{BYou have lost a die!!{x", original, NULL, playing, TO_CHAR);
	                   original->pcdata->stones->diceLeft--;
	                }
               }

               original->pcdata->stones->state = STONES_STATE_LOSE_DIE;
               original->pcdata->stones->pulse = STONES_DELAY_LOSE_DIE*PULSE_PER_SECOND;               
	           break;
	        case(STONES_STATE_LOSE_DIE):       // Someone has lost a die, see if we play another round or if the game is over
	           if(original->pcdata->stones->diceLeft==0 || playing->pcdata->stones->diceLeft==0)
	           {
	               // Match is over, someone lost all their dice
                   if(original->pcdata->stones->diceLeft==0)
    	           {
    	               act("{BYou have lost all your dice!{x", original, NULL, playing, TO_CHAR);
    	           }
                   else if(playing->pcdata->stones->diceLeft==0)
    	           {
    	               act("{C$N{B has lost all of $S dice!{x", original, NULL, playing, TO_CHAR);
    	           }
    	           
                   original->pcdata->stones->state = STONES_STATE_MATCH_OUTCOME;
                   original->pcdata->stones->pulse = STONES_DELAY_MATCH_OUTCOME*PULSE_PER_SECOND;               
    	       }
	           else
	           {
	               // Both players still have dice left, time for another round
	               sprintf(buf, "{BA new round will begin in {W%d{B seconds.{x", STONES_DELAY_BETWEEN_ROUNDS);
	               act(buf, original, NULL, playing, TO_CHAR);
	               
                   original->pcdata->stones->state = STONES_STATE_BEGINNING_ROUND;
                   original->pcdata->stones->pulse = STONES_DELAY_BETWEEN_ROUNDS*PULSE_PER_SECOND;               
	           }
	           break;
	        case(STONES_STATE_MATCH_OUTCOME):      // Someone has won! (and someone has lost hahaha)
	           if(original->pcdata->stones->diceLeft==0)
	               stones_winner(playing);
	           else if(playing->pcdata->stones->diceLeft==0)
	               stones_winner(original);
	               
	           // Both original and playing are no longer playing, thus don't do anything else with them!!!
	           break;
	    }
	}
}

