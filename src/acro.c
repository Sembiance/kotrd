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

#include "Utility.h"
#include "StringUtility.h"
#include "ArrayUtility.h"

//////////////////////////////////////////////////////////////////////////
// Definitions
#define ACRO_DELAY_GAME_START				30		// Number of seconds between 'Game Announcement' and game start
#define	ACRO_DELAY_ROUND_START				4		// Number of seconds between 'Round Announcement' and round start

#define ACRO_MINIMUM_ROUND_TIME_LIMIT		60		// Minimum number of seconds to submit answers

#define ACRO_ADDITION_TIME_PER_LETTER		15		// Number of additional seconds to submit answer per letter over the ACRO_ADDITION_TIME_LETTER_COUNT
#define ACRO_ADDITION_TIME_LETTER_COUNT		4		// This many letters or higher, and players get ACRO_ADDITION_TIME_PER_LETTER * number of letters over
													// So a 3 or 4 letter acro would get 60 seconds to submit answers
													// A 5 letter acronym would get 75 seconds to submit answers
													// A 6 letter acronym would get 90 seconds to submit answers, etc.

#define ACRO_ROUND_ALMOST_OVER_WARNING		10		// How many seconds before round end to warn to get answers in

#define ACRO_DELAY_BETWEEN_ANSWERS			3		// Number of seconds between showing answers
#define ACRO_DELAY_BETWEEN_RESULTS			2		// Number of seconds between showing results

#define ACRO_DELAY_BETWEEN_ROUNDS			30		// Number of seconds between rounds
#define ACRO_DELAY_BETWEEN_GAMES			30		// Number of seconds between games

#define ACRO_DELAY_GIVE_PRIZE				10		// Delay from announcement of winner until prize is awarded

#define ACRO_VOTE_ROUND_TIME_LIMIT			45		// Number of seconds to allow for voting

#define ACRO_MINIMUM_ACRO_LETTERS			3		// Minimum number of letters in an acro
#define ACRO_MAXIMUM_ACRO_LETTERS			7		// Maximum number of letters in an acro

#define	ACRO_BONUS_SPEED_POINTS				2		// Number of bonus points to give for getting in the first answer
#define ACRO_MAX_SCORE_FOR_SPEED_BONUS		25		// If player has this many points or more, they won't get a speed bonus

#define ACRO_DEFAULT_GAME_OVER_SCORE		30		// How many points a player needs to win

#define ACRO_STAGE_INACTIVE					0		// Acro is inactive
#define ACRO_STAGE_GAME_ANNOUNCE			1		// Announce the game
#define ACRO_STAGE_ACRO_ROUND_ANNOUNCE		2		// Announce the acro round
#define	ACRO_STAGE_ACRO_ROUND_START			3		// Start the acro round
#define ACRO_STAGE_ACRO_ROUND_END_WARN		4		// Warn that the acro round is about to end
#define ACRO_STAGE_ANNOUNCE_ANSWERS_START	5		// Start the answer announcment
#define ACRO_STAGE_ANNOUNCE_ANSWERS			6		// Announce our answers
#define ACRO_STAGE_VOTE_ROUND_START			7		// Start the vote round
#define ACRO_STAGE_ANNOUNCE_RESULTS_START	8		// Start the results announcement
#define ACRO_STAGE_ANNOUNCE_RESULTS			9		// Announce our results
#define ACRO_STAGE_SHOW_SCORES				10		// Show our scores
#define ACRO_STAGE_NEXT_ROUND_ANNONCE		11		// Announce the next round
#define ACRO_STAGE_GIVE_PRIZE				12		// Give out prizes!

#define ACRO_PRIZE_DEFAULT_XP1						500
#define ACRO_PRIZE_DEFAULT_XP2						1000
#define ACRO_PRIZE_DEFAULT_ITEM1					23592
#define ACRO_PRIZE_DEFAULT_ITEM2					3018
#define ACRO_PRIZE_DEFAULT_ITEM3					34

//////////////////////////////////////////////////////////////////////////
// Globals
ACRO_DATA  *	gAcro;


//////////////////////////////////////////////////////////////////////////
// clear_new_round_acro - Clears gAcro based on starting a new round
void	clear_new_round_acro(void)
{
	DESCRIPTOR_DATA *	d;
	CHAR_DATA *			original;
	unsigned int		i;

	gAcro->acro_length = 0;
	gAcro->acro[0] = '\0';

	gAcro->acro_answers = array_free(gAcro->acro_answers);
	gAcro->acro_answer_names = array_free(gAcro->acro_answer_names);
	gAcro->acro_results = array_free(gAcro->acro_results);

	gAcro->num_answers = 0;
	gAcro->current_acro_answer = 0;
	gAcro->current_acro_result = 0;

	gAcro->acro_counter = 0;

	for(i=0;i<500;i++)
		gAcro->acro_votes[i] = 0;

	// Clear status of all players except score
	for(d=descriptor_list;d!=NULL;d=d->next)
	{
		if(d->connected!=CON_PLAYING || !d->character || !d->character->in_room)
			continue;

		original = d->original ? d->original : d->character; /* if switched */
		
		if(!original->pcdata)
			continue;

		original->pcdata->acro_answer[0] = '\0';
		original->pcdata->acro_voted_for = 0;
	}
}


//////////////////////////////////////////////////////////////////////////
// clear_new_game_acro - Clears gAcro based on starting a new game
void	clear_new_game_acro(void)
{
	DESCRIPTOR_DATA *	d;
	CHAR_DATA *			original;

	gAcro->winner = 0;

 	// Clear status of all players
	for(d=descriptor_list;d!=NULL;d=d->next)
	{
		if(d->connected!=CON_PLAYING || !d->character || !d->character->in_room)
			continue;

		original = d->original ? d->original : d->character; /* if switched */
		
		if(!original->pcdata)
			continue;

		original->pcdata->acro_score = 0;
		original->pcdata->acro_has_participated = 0;
	}

	clear_new_round_acro();
}


//////////////////////////////////////////////////////////////////////////
// clear_all_acro - Clears all of gAcro
void	clear_all_acro(void)
{
	gAcro->active = 0;
	gAcro->stage = ACRO_STAGE_INACTIVE;
	gAcro->pulse = 0;

	clear_new_game_acro();
}

//////////////////////////////////////////////////////////////////////////
// init_acro - Initiliazes global memory on boot
void	init_acro(void)
{
	gAcro = (ACRO_DATA *)malloc(sizeof(ACRO_DATA));

	gAcro->acro_answers = 0;
	gAcro->acro_answer_names = 0;
	gAcro->acro_results = 0;

	gAcro->xpPrize1 = ACRO_PRIZE_DEFAULT_XP1;
	gAcro->xpPrize2 = ACRO_PRIZE_DEFAULT_XP2;

	gAcro->itemPrize1 = ACRO_PRIZE_DEFAULT_ITEM1;
	gAcro->itemPrize2 = ACRO_PRIZE_DEFAULT_ITEM2;
	gAcro->itemPrize3 = ACRO_PRIZE_DEFAULT_ITEM3;

	gAcro->gameOverScore = ACRO_DEFAULT_GAME_OVER_SCORE;

	clear_all_acro();
}


//////////////////////////////////////////////////////////////////////////
// talk_acro - Handles the channel talking for the Acrophobia channel
void talk_acro(char *argument, CHAR_DATA * ch)
{
    DESCRIPTOR_DATA *	d;
    char				buf[MAX_STRING_LENGTH];
    CHAR_DATA *			original;

	if(argument[0]=='\0')
    {
        if(IS_SET(ch->deaf, CHANNEL_ACRO))
		{
			REMOVE_BIT(ch->comm, CHANNEL_ACRO);
			REMOVE_BIT(ch->deaf, CHANNEL_ACRO);
			sprintf(buf, "\n\rAcrophobia Channel is now {GON{x.\n\r");
			send_to_char(buf, ch);
		}
		else
		{
			SET_BIT(ch->deaf, CHANNEL_ACRO);
			sprintf(buf, "\n\rAcrophobia Channel is now {MOFF{x.\n\r");
			send_to_char(buf, ch);
		}

		return;
	}

	sprintf(buf,"\n\r{G[{YAcrophobia{G]{x: {M%s{x\n\r", argument);

	for(d=descriptor_list;d;d=d->next)
	{
		original = d->original ? d->original : d->character; /* if switched */
		if((d->connected==CON_PLAYING) && !IS_SET(original->deaf, CHANNEL_ACRO))
			act_new(buf, original, NULL, NULL, TO_CHAR, POS_DEAD);
	}
}


//////////////////////////////////////////////////////////////////////////
// acro_help - Sends the Acrophobia instructions to the player
void	acro_help(CHAR_DATA * ch)
{
	char	buf[MAX_STRING_LENGTH];

	sprintf(buf, 
			"{MAcrophobia is a fun game where players fill in the blanks to an acronym\n\r" \
			"in a chance to win prizes!\n\r" \
			"\n\r" \
			"Each round a random acronym is generated between {G%d{M and {G%d{M letters.\n\r" \
			"Your job is to come up with words that start with each of the letters given.\n\r" \
			"\n\r" \
			"For example, if the acronym {WTMITA{M is generated, you might come up with:\n\r" \
			"	{WT{Ghis {WM{Gud {WI{Gs {WT{Gotally {WA{Gwesome!{M\n\r" \
			"\n\r" \
			"After answer submission is over, the answers will be displayed to the\n\r" \
			"players for a voting round. Who submitted the answer will not be shown.\n\r" \
			"\n\r" \
			"Each player will then vote for the answer they like the best.\n\r" \
			"A player cannot vote for their own answer.\n\r" \
			"\n\r" \
			"Each player will receive {G1{M point per vote.\n\r" \
			"\n\r" \
			"The player with the most votes will receive bonus points.\n\r" \
			"These bonus points will be equal to the number of letters in the acronym.\n\r" \
			"If this player did not vote theirself, their bonus points are lost.\n\r" \
			"\n\r" \
			"Game continues until one person reaches {G%d points{M or higher.\n\r" \
			"In the case of a tie, a tie-breaker round will be played.\n\r" \
			"\n\r" \
			"The winner will receive a {Grandom prize{R!{M!{R!{M!{R!{M!{R!{M!{R!{M!{R!{M!{M\n\r" \
			"\n\r" \
			"To submit an answer type: {Wacro <your acronym answer here>{M\n\r" \
			"To vote for another answer type: {Wacro <number>{M\n\r" \
			"\n\r" \
			"{GAdditional Commands:\n\r" \
			"\t{Wacro scores\t\t{MWill show the current scores\n\r" \
			"\t{Wacro answers\t\t{MWill show the answers up for vote\n\r" \
			"\t{Wacro acro\t\t{MWill show the current acronym to answer for\n\r" \
			"\t{Wacro help\t\t{MShows what you are reading right now\n\r\n\r",
			ACRO_MINIMUM_ACRO_LETTERS, ACRO_MAXIMUM_ACRO_LETTERS, gAcro->gameOverScore);
	send_to_char(buf, ch);

	if(IS_IMMORTAL(ch))
	{
		sprintf(buf,
				"{YIMM ONLY COMMANDS:\n\r" \
				"\t{Wacro start\t\t\t{MStarts Acrophobia!\n\r" \
				"\t{Wacro stop\t\t\t{MWill stop Acrophobia\n\r" \
				"\t{Wacro winningscore <value>\t{MSets how high someone has to get to win\n\r" \
				"\t{Wacro prizexp1 <value>\t\t{MSets the first XP prize to <value>\n\r" \
				"\t{Wacro prizexp2 <value>\t\t{MSets the second XP prize to <value>\n\r" \
				"\t{Wacro prizeitem1 <vnum>\t\t{MSets the first item prize to <vnum>\n\r" \
				"\t{Wacro prizeitem2 <vnum>\t\t{MSets the second item prize to <vnum>\n\r" \
				"\t{Wacro prizeitem3 <vnum>\t\t{MSets the third item prize to <vnum>\n\r\n\r");
		send_to_char(buf, ch);
	}
}


//////////////////////////////////////////////////////////////////////////
// acro_update - Is called every tick to perform acrophobia updates
void	acro_update(void)
{
	char				buf[MAX_STRING_LENGTH];
	char *				dynamicString=0;
	unsigned int		i=0, highestVotes=0, whichPrize=0, highestScore=0;
	char				randomLetter;
    DESCRIPTOR_DATA *	d;
    CHAR_DATA *			original=0;
	CHAR_DATA *			winner=0;
	char **				ar=0;
	long				loc=-1;
	bool				skipLetter=0;
	char				goodLetters[]="SOIMPRUHLTD";
	char				okayLetters[]="NCBAFEGWKJV";
	//char				badLetters[]="QYZX";
	sh_int              randomPrizeVnum;
    OBJ_INDEX_DATA *	prizeObjectIndex=0;
    OBJ_DATA *			prizeObject=0;

	if(!gAcro->active)
		return;

	if(gAcro->pulse)
	{
		gAcro->pulse--;
		return;
	}

	switch(gAcro->stage)
	{
		case(ACRO_STAGE_INACTIVE):					// Acro is inactive
			clear_all_acro();
			break;
		case(ACRO_STAGE_GAME_ANNOUNCE):				// Announce the game
			clear_new_game_acro();

			if(ACRO_DELAY_GAME_START)
			{
				sprintf(buf, "A new game of {YAcrophobia{M is beginning in {G%d{M seconds!", ACRO_DELAY_GAME_START);
				talk_acro(buf, 0);
				talk_acro("You can type {W'acro help'{M for instructions on how to play.", 0);
			}

			gAcro->pulse = ACRO_DELAY_GAME_START*PULSE_PER_SECOND;
			gAcro->stage = ACRO_STAGE_ACRO_ROUND_ANNOUNCE;
			break;
		case(ACRO_STAGE_ACRO_ROUND_ANNOUNCE):		// Announce the acro round
			clear_new_round_acro();

			talk_acro("A new round is starting!", 0);
			talk_acro("Submit your answers by typing: {Wacro <your answer here>{M", 0);

			gAcro->pulse = ACRO_DELAY_ROUND_START*PULSE_PER_SECOND;
			gAcro->stage = ACRO_STAGE_ACRO_ROUND_START;
			break;
		case(ACRO_STAGE_ACRO_ROUND_START):			// Start the acro round
			// Random length of acro
			gAcro->acro_length = number_range(ACRO_MINIMUM_ACRO_LETTERS, ACRO_MAXIMUM_ACRO_LETTERS);

			if(gAcro->acro_length!=ACRO_MINIMUM_ACRO_LETTERS && gAcro->acro_length!=ACRO_MINIMUM_ACRO_LETTERS+1)
				gAcro->acro_length = number_range(ACRO_MINIMUM_ACRO_LETTERS, ACRO_MAXIMUM_ACRO_LETTERS);
			if(gAcro->acro_length!=ACRO_MINIMUM_ACRO_LETTERS && gAcro->acro_length!=ACRO_MINIMUM_ACRO_LETTERS+1 && gAcro->acro_length!=ACRO_MINIMUM_ACRO_LETTERS+2)
				gAcro->acro_length = number_range(ACRO_MINIMUM_ACRO_LETTERS, ACRO_MAXIMUM_ACRO_LETTERS);

			// Make the acro
			for(skipLetter=0,i=0;i<gAcro->acro_length;skipLetter=0,i++)
			{
				randomLetter = (char)number_range(65, 90);

				if(!strchr(goodLetters, randomLetter))
				{
					// We don't have a good letter, so let's see if we should skip
					if(strchr(okayLetters, randomLetter))
					{
						if(number_percent()<=30)	// 30 percent chance of picking a new letter
							skipLetter=1;
					}
					else	// We must have a bad letter, ick!
					{
						if(number_percent()<=95)	// 95 percent chance of picking a new letter
							skipLetter=1;
					}
				}

				if(skipLetter)
				{
					i--;
					continue;
				}

				gAcro->acro[i] = randomLetter;
			}
			gAcro->acro[i] = '\0';

			// Announce the acro
			sprintf(buf, "The acronym for this round is: {W%s{x", gAcro->acro);
			talk_acro(buf, 0);

			gAcro->pulse = ACRO_MINIMUM_ROUND_TIME_LIMIT;

			if(gAcro->acro_length>ACRO_ADDITION_TIME_LETTER_COUNT)
				gAcro->pulse+=(gAcro->acro_length-ACRO_ADDITION_TIME_LETTER_COUNT)*ACRO_ADDITION_TIME_PER_LETTER;

			if(gAcro->pulse>ACRO_ROUND_ALMOST_OVER_WARNING)
				gAcro->pulse-=ACRO_ROUND_ALMOST_OVER_WARNING;

			gAcro->pulse*=PULSE_PER_SECOND;

			gAcro->stage = ACRO_STAGE_ACRO_ROUND_END_WARN;
			break;
		case(ACRO_STAGE_ACRO_ROUND_END_WARN):		// Warn the acro round is about to end
			if(ACRO_ROUND_ALMOST_OVER_WARNING)
			{
				sprintf(buf, "{G%d{M seconds left - Get Those Acros In!", ACRO_ROUND_ALMOST_OVER_WARNING);
				talk_acro(buf, 0);
			}

			gAcro->pulse = ACRO_ROUND_ALMOST_OVER_WARNING*PULSE_PER_SECOND;
			gAcro->stage = ACRO_STAGE_ANNOUNCE_ANSWERS_START;
			break;
		case(ACRO_STAGE_ANNOUNCE_ANSWERS_START):		// Start the answer anouncement
			// Form our list of names with answers
			for(d=descriptor_list;d;d=d->next)
			{
				if(d->connected!=CON_PLAYING || !d->character || !d->character->in_room)
					continue;

				original = d->original ? d->original : d->character; /* if switched */
				
				if(!original->pcdata || original->pcdata->acro_answer[0]=='\0')
					continue;

				gAcro->acro_answer_names = array_append(gAcro->acro_answer_names, original->name);
				gAcro->acro_answers = array_append(gAcro->acro_answers, "Invalid Answer");
			}

			// Randomize our names
			array_sort_randomly(gAcro->acro_answer_names);

			// Form our list of answers filling in the correct spots
			for(d=descriptor_list;d;d=d->next)
			{
				if(d->connected!=CON_PLAYING || !d->character || !d->character->in_room)
					continue;

				original = d->original ? d->original : d->character; /* if switched */
				
				if(!original->pcdata || original->pcdata->acro_answer[0]=='\0')
					continue;

				loc = array_find(gAcro->acro_answer_names, original->name);
				if(loc>=0)
				{
					gAcro->acro_answers[loc] = strfree(gAcro->acro_answers[loc]);
					gAcro->acro_answers[loc] = strdup(original->pcdata->acro_answer);

					gAcro->acro_votes[loc] = 0;
				}
			}

			gAcro->num_answers = array_len(gAcro->acro_answers);

			if(gAcro->num_answers<=1)
			{
				talk_acro("Not enough answers received to vote on.", 0);
				gAcro->pulse = ACRO_DELAY_BETWEEN_ANSWERS*PULSE_PER_SECOND;
				gAcro->stage = ACRO_STAGE_SHOW_SCORES;
			}
			else
			{
				gAcro->current_acro_answer = 0;

				talk_acro("Times up! Voting round!", 0);

				gAcro->pulse = ACRO_DELAY_BETWEEN_ANSWERS*PULSE_PER_SECOND;
				gAcro->stage = ACRO_STAGE_ANNOUNCE_ANSWERS;
			}
			break;
		case(ACRO_STAGE_ANNOUNCE_ANSWERS):				// Announce our answers
			sprintf(buf, "Answer [{G%d{M]: {W%s{x", gAcro->current_acro_answer+1, gAcro->acro_answers[gAcro->current_acro_answer]);
			talk_acro(buf, 0);

			gAcro->current_acro_answer++;

			if(gAcro->current_acro_answer==gAcro->num_answers)
				gAcro->stage = ACRO_STAGE_VOTE_ROUND_START;
			else
				gAcro->pulse = ACRO_DELAY_BETWEEN_ANSWERS*PULSE_PER_SECOND;
			break;
		case(ACRO_STAGE_VOTE_ROUND_START):
			talk_acro("Use {Wacro <votenumber>{M to cast your vote for the person with the best expanded acronym.", 0);
			sprintf(buf, "You have {G%d{M seconds to cast your votes.", ACRO_VOTE_ROUND_TIME_LIMIT);
			talk_acro(buf, 0);
			
			gAcro->pulse = ACRO_VOTE_ROUND_TIME_LIMIT*PULSE_PER_SECOND;
			gAcro->stage = ACRO_STAGE_ANNOUNCE_RESULTS_START;
			break;
		case(ACRO_STAGE_ANNOUNCE_RESULTS_START):		// Start the results announcement
		// Determine how many votes is the top vote
			for(highestVotes=0,i=0;i<array_len(gAcro->acro_answers);i++)
			{
				if(gAcro->acro_votes[(i+1)]>highestVotes)
					highestVotes = gAcro->acro_votes[(i+1)];
			}

			// Form our list of results
			for(i=0,ar=gAcro->acro_answers;*ar;i++,ar++)
			{
				for(original=char_list;original && strcmp(gAcro->acro_answer_names[i], original->name);original=original->next)
				;

				sprintf(buf, "{Y%s's{M answer of {W\"%s\"{M received {G%d{M votes!", gAcro->acro_answer_names[i], *ar, gAcro->acro_votes[i+1]);
				gAcro->acro_results = array_append(gAcro->acro_results, buf);

				if(original && original->pcdata)
				{
					original->pcdata->acro_score+=gAcro->acro_votes[i+1];

					//sprintf(buf, "{Y%s{M received {G%d{M speed answer points!", gAcro->acro_answer_names[i], ACRO_BONUS_SPEED_POINTS);
					//gAcro->acro_results = array_append(gAcro->acro_results, buf);

					//original->pcdata->acro_score+=ACRO_BONUS_SPEED_POINTS;
				}
			}

			// Add bonus points to the people with the most votes
			dynamicString = 0;

			for(i=0,ar=gAcro->acro_answers;*ar;i++,ar++)
			{
				for(original=char_list;original && strcmp(gAcro->acro_answer_names[i], original->name);original=original->next)
				;

				if(highestVotes && gAcro->acro_votes[i+1]==highestVotes && original && original->pcdata && original->pcdata->acro_voted_for)
				{
					if(!dynamicString)
					{
						sprintf(buf, "The following recieve {G%d{M bonus points:{Y ", gAcro->acro_length);
						dynamicString = strdup(buf);
					}

					sprintf(buf, "%s  ", gAcro->acro_answer_names[i]);
					dynamicString = strappend(dynamicString, buf);

					original->pcdata->acro_score+=gAcro->acro_length;
				}
			}
			if(dynamicString)
			{
				gAcro->acro_results = array_append(gAcro->acro_results, dynamicString);
				dynamicString = strfree(dynamicString);
			}

			gAcro->current_acro_result = gAcro->acro_results;

			talk_acro("The votes were as follows:", 0);

			gAcro->pulse = ACRO_DELAY_BETWEEN_RESULTS*PULSE_PER_SECOND;
			gAcro->stage = ACRO_STAGE_ANNOUNCE_RESULTS;
			break;
		case(ACRO_STAGE_ANNOUNCE_RESULTS):				// Announce our results
			talk_acro(*gAcro->current_acro_result, 0);

			gAcro->current_acro_result++;

			gAcro->pulse = ACRO_DELAY_BETWEEN_RESULTS*PULSE_PER_SECOND;

			if(!(*gAcro->current_acro_result))
				gAcro->stage = ACRO_STAGE_SHOW_SCORES;
			break;
		case(ACRO_STAGE_SHOW_SCORES):					// Show our scores
			talk_acro("The current scores are:", 0);

			for(dynamicString=0,i=0,d=descriptor_list;d;d=d->next)
			{
				if(d->connected!=CON_PLAYING || !d->character || !d->character->in_room)
					continue;

				original = d->original ? d->original : d->character; /* if switched */
				
				if(!original->pcdata || !original->pcdata->acro_has_participated)
					continue;

				sprintf(buf, "{Y%s{M:{G%d{x    ", original->name, original->pcdata->acro_score);
				dynamicString = strappend(dynamicString, buf);

				if(++i==4)
				{
					if(dynamicString)
					{
						talk_acro(dynamicString, 0);
						dynamicString = strfree(dynamicString);
					}
					i=0;
				}

				gAcro->acro_answer_names = array_append(gAcro->acro_answer_names, original->name);
				gAcro->acro_answers = array_append(gAcro->acro_answers, "Invalid Answer");
			}

			if(dynamicString)
			{
				talk_acro(dynamicString, 0);
				dynamicString = strfree(dynamicString);
			}

			// Time for game to end?
			for(highestScore=0,i=0,d=descriptor_list;d;d=d->next)
			{
				if(d->connected!=CON_PLAYING || !d->character || !d->character->in_room)
					continue;

				original = d->original ? d->original : d->character; /* if switched */
				
				if(!original->pcdata || original->pcdata->acro_score<gAcro->gameOverScore)
					continue;

				if(original->pcdata->acro_score>highestScore)
				{
					highestScore = original->pcdata->acro_score;
					winner = original;
					i=1;
				}
				else if(original->pcdata->acro_score==highestScore)
					i++;
			}

			if(i>1)
			{
				sprintf(buf, "We have a %d-way tie! Going to a tiebreak round!", i);
				talk_acro(buf, 0);

				gAcro->pulse = PULSE_PER_SECOND;
				gAcro->stage = ACRO_STAGE_NEXT_ROUND_ANNONCE;
			}
			else if(i==1)
			{
				gAcro->winner = winner;
				sprintf(buf, "{Y*{R*{Y*{R*{Y*{R*{Y*{R*{Y*{R*{W %s{G is the winner! {Y*{R*{Y*{R*{Y*{R*{Y*{R*{Y*{R*{x", winner->name);
				talk_acro(buf, 0);

				gAcro->pulse = ACRO_DELAY_GIVE_PRIZE*PULSE_PER_SECOND;
				gAcro->stage = ACRO_STAGE_GIVE_PRIZE;
			}
			else
			{
				gAcro->pulse = PULSE_PER_SECOND;
				gAcro->stage = ACRO_STAGE_NEXT_ROUND_ANNONCE;
			}
			break;
		case(ACRO_STAGE_NEXT_ROUND_ANNONCE):
			sprintf(buf, "Starting next round in {G%d{M seconds.", ACRO_DELAY_BETWEEN_ROUNDS);
			talk_acro(buf, 0);

			gAcro->pulse = ACRO_DELAY_BETWEEN_ROUNDS*PULSE_PER_SECOND;
			gAcro->stage = ACRO_STAGE_ACRO_ROUND_ANNOUNCE;
			break;
		case(ACRO_STAGE_GIVE_PRIZE):
			// Come up with prize
			sprintf(buf, "For some amazing acronym skills, {W%s{M wins: ", gAcro->winner->name);
			dynamicString = strdup(buf);

			whichPrize = number_range(0, 6);
			
			if(gAcro->winner->level==LEVEL_HERO && (whichPrize==2 || whichPrize==3))
			    whichPrize = 0;

			sprintf(buf, "{WA bottle of bird poop!{x");
			if(whichPrize==0 || whichPrize==1) 	// Random item!
			{
			    do
			    {
                    for(randomPrizeVnum=number_range(1, 32766),prizeObjectIndex=get_obj_index(randomPrizeVnum);!prizeObjectIndex;randomPrizeVnum=number_range(1, 32766),prizeObjectIndex=get_obj_index(randomPrizeVnum))
                    ;
                    
                    prizeObject=create_object(prizeObjectIndex, UMIN(gAcro->winner->level, prizeObjectIndex->level));
                    prizeObject->level = UMIN(gAcro->winner->level, prizeObjectIndex->level);
                }while(!prizeObject);
                
                obj_to_char(prizeObject, gAcro->winner);
                sprintf(buf, "{x%s{x", prizeObject->short_descr);
			}
			else if(whichPrize==2)	// XP 1
			{
				gain_exp(gAcro->winner, gAcro->xpPrize1);
				sprintf(buf, "{G%lu experience!!!{x", gAcro->xpPrize1);
			}
			else if(whichPrize==3)	// XP 2
			{
				gain_exp(gAcro->winner, gAcro->xpPrize2);
				sprintf(buf, "{G%lu experience!!!{x", gAcro->xpPrize2);
			}
			else if(whichPrize==4)	// Item 1
			{
				if((prizeObjectIndex=get_obj_index(gAcro->itemPrize1)) && (prizeObject=create_object(prizeObjectIndex, prizeObjectIndex->level)))
				{
					obj_to_char(prizeObject, gAcro->winner);
					sprintf(buf, "{x%s{x", prizeObject->short_descr);
				}
			}
			else if(whichPrize==5)	// Item 2
			{
				if((prizeObjectIndex=get_obj_index(gAcro->itemPrize2)) && (prizeObject=create_object(prizeObjectIndex, prizeObjectIndex->level)))
				{
					obj_to_char(prizeObject, gAcro->winner);
					sprintf(buf, "{x%s{x", prizeObject->short_descr);
				}
			}
			else if(whichPrize==6)	// Item 3
			{
				if((prizeObjectIndex=get_obj_index(gAcro->itemPrize3)) && (prizeObject=create_object(prizeObjectIndex, prizeObjectIndex->level)))
				{
					obj_to_char(prizeObject, gAcro->winner);
					sprintf(buf, "{x%s{x", prizeObject->short_descr);
				}
			}
			dynamicString = strappend(dynamicString, buf);
			talk_acro(dynamicString, 0);

			gAcro->pulse = ACRO_DELAY_BETWEEN_GAMES*PULSE_PER_SECOND;
			gAcro->stage = ACRO_STAGE_GAME_ANNOUNCE;
			break;
		default:
			clear_all_acro();
			break;
	}
}


//////////////////////////////////////////////////////////////////////////
// do_acro - The entry function for 'acro <arg>' commands
void    do_acro(CHAR_DATA * ch, char * argument)
{
	char				originalArgument[MAX_STRING_LENGTH];
	char                arg1[MAX_INPUT_LENGTH];
	char                arg2[MAX_INPUT_LENGTH];
	char				buf[MAX_STRING_LENGTH];
	char **				answers=0;
	char **				ar=0;
	unsigned int		i=0, vote_number, roughArgumentCount;
	bool				invalid_answer=0, invalid_vote=0;
	char *				dynamicString=0;
    DESCRIPTOR_DATA *	d;
    CHAR_DATA *			original=0;

	if(argument && argument[0]!='\0')
		strcpy(originalArgument, argument);

	roughArgumentCount = strchrcount(argument, ' ');

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

	// No NPCS
	if(IS_NPC(ch))
		return;

	// Switch on and off
	if(arg1[0]=='\0')
    {
		talk_acro(arg1, ch);
		return;
    }

	if(roughArgumentCount<=1)
	{
		// Start command, IMM only
		if(!strcmp(arg1, "start"))
		{
			if(!IS_IMMORTAL(ch))
			{
				acro_help(ch);
				return;
			}
			
			if(gAcro->active)
			{
				send_to_char("{MAcrophobia is already running.{x\n\r", ch);
				return;
			}

			gAcro->active = 1;
			gAcro->stage = ACRO_STAGE_GAME_ANNOUNCE;
			send_to_char("{MAcrophobia has been started.{x\n\r", ch);

			return;
		}

		// Help command
		if(!strcmp(arg1, "help") || !strcmp(arg1, "instructions"))
		{
			acro_help(ch);
			return;
		}

		// Score command
		if(!strcmp(arg1, "score") || !strcmp(arg1, "scores"))
		{
			send_to_char("{MThe current scores are:{x\n\r", ch);

			for(dynamicString=0,i=0,d=descriptor_list;d;d=d->next)
			{
				if(d->connected!=CON_PLAYING || !d->character || !d->character->in_room)
					continue;

				original = d->original ? d->original : d->character; /* if switched */
				
				if(!original->pcdata || !original->pcdata->acro_has_participated)
					continue;

				sprintf(buf, "{Y%s{M:{G%d{x    ", original->name, original->pcdata->acro_score);
				dynamicString = strappend(dynamicString, buf);

				if(++i==4)
				{
					if(dynamicString)
					{
						send_to_char(dynamicString, ch);
						dynamicString = strfree(dynamicString);
					}
					i=0;
				}
			}
			if(dynamicString)
			{
				send_to_char(dynamicString, ch);
				dynamicString = strfree(dynamicString);
			}
			
			return;
		}

		// Answers command
		if(!strcmp(arg1, "answer") || !strcmp(arg1, "answers"))
		{
			if(gAcro->stage!=ACRO_STAGE_ANNOUNCE_RESULTS_START)
			{
				send_to_char("{MWe are not voting right now, so therefore I cannot show you answers.{x\n\r", ch);
				return;
			}

			send_to_char("{MCurrent answers:{x\n\r", ch);
			for(i=1,ar=gAcro->acro_answers;*ar;i++,ar++)
			{
				sprintf(buf, "{MAnswer [{G%d{M]: {W%s{x\n\r", i, *ar);
				send_to_char(buf, ch);
			}

			return;
		}

		// acro command
		if(!strcmp(arg1, "acro") || !strcmp(arg1, "acronym"))
		{
			if(gAcro->stage!=ACRO_STAGE_ACRO_ROUND_END_WARN && gAcro->stage!=ACRO_STAGE_ANNOUNCE_ANSWERS_START)
			{
				send_to_char("{MThere is no available acronym examine at the moment.{x\n\r", ch);
				return;
			}

			sprintf(buf, "{MCurrent acronym: {W%s{x\n\r", gAcro->acro);
			send_to_char(buf, ch);

			return;
		}

		if(IS_IMMORTAL(ch) && !strcmp(arg1, "winningscore") && arg2[0]!='\0')
		{
			if(atoi(arg2)>0)
				gAcro->gameOverScore = atoi(arg2);
			
			sprintf(buf, "{MGame winning score set to {G%d {Mpoints.{x\n\r", gAcro->gameOverScore);
			send_to_char(buf, ch);

			return;
		}

		// Set xp amount
		if(IS_IMMORTAL(ch) && (!strcmp(arg1, "prizexp1") || !strcmp(arg1, "prizexp2") || !strcmp(arg1, "prizeitem1") || !strcmp(arg1, "prizeitem2") || !strcmp(arg1, "prizeitem3")) && arg2[0]!='\0')
		{
			if(!strcmp(arg1, "prizexp1"))
			{
				gAcro->xpPrize1 = strtoul(arg2, 0, 10);
				sprintf(buf, "{MXP prize 1 set to {G%lu {Mexperience.{x\n\r", gAcro->xpPrize1);
			}
			if(!strcmp(arg1, "prizexp2"))
			{
				gAcro->xpPrize2 = strtoul(arg2, 0, 10);
				sprintf(buf, "{MXP prize 2 set to {G%lu {Mexperience.{x\n\r", gAcro->xpPrize2);
			}
			if(!strcmp(arg1, "prizeitem1"))
			{
				gAcro->itemPrize1 = atoi(arg2);
				sprintf(buf, "{MItem prize 1 set to vnum {G%d{M.{x\n\r", gAcro->itemPrize1);
			}
			if(!strcmp(arg1, "prizeitem2"))
			{
				gAcro->itemPrize2 = atoi(arg2);
				sprintf(buf, "{MItem prize 2 set to vnum {G%d{M.{x\n\r", gAcro->itemPrize2);
			}
			if(!strcmp(arg1, "prizeitem3"))
			{
				gAcro->itemPrize3 = atoi(arg2);
				sprintf(buf, "{MItem prize 3 set to vnum {G%d{M.{x\n\r", gAcro->itemPrize3);
			}

			send_to_char(buf, ch);
			return;
		}
	}

	// If we're not running right now, leave
	if(!gAcro->active)
	{
		send_to_char("{MAcrophobia is not running right now. Talk to an immortal about starting a game!{x\n\r", ch);
		return;
	}

	if(roughArgumentCount==0)
	{
		// Stop command, IMM only
		if(!strcmp(arg1, "stop"))
		{
			if(!IS_IMMORTAL(ch))
			{
				acro_help(ch);
				return;
			}

			clear_all_acro();
			talk_acro("Acrophobia has been stopped.", 0);

			return;
		}
	}

	// Are they submitting an answer?
	if(gAcro->stage==ACRO_STAGE_ACRO_ROUND_END_WARN || gAcro->stage==ACRO_STAGE_ANNOUNCE_ANSWERS_START)
	{
		answers = strchrexplode(originalArgument, ' ');
		if(!answers || array_len(answers)!=gAcro->acro_length)
			invalid_answer = 1;
		else
		{
			for(i=0,ar=answers;*ar;i++,ar++)
			{
				if(UPPER((*ar)[0])!=gAcro->acro[i])
					invalid_answer = 1;
			}
			answers = array_free(answers);
		}

		for(d=descriptor_list;d;d=d->next)
		{
			if(d->connected!=CON_PLAYING || !d->character || !d->character->in_room)
				continue;

			original = d->original ? d->original : d->character; /* if switched */
			
			if(!original->pcdata || original->pcdata->acro_answer[0]=='\0')
				continue;

			if(!strcasecmp(originalArgument, original->pcdata->acro_answer))	// Same answer as someone else
			{
				send_to_char("{MThat answer has been submitted.{x\n\r", ch);
				return;
			}
		}

		if(invalid_answer)
		{
			sprintf(buf, "{MINVALID ANSWER for Acronym {W%s{x\n\r", gAcro->acro);
			send_to_char(buf, ch);
			return;
		}

		ch->pcdata->acro_has_participated = 1;

		if(ch->pcdata->acro_answer[0]!='\0')
			sprintf(buf, "{MYou've changed your answer to: {W\"%s\"{x\n\r", originalArgument);
		else
		{
			gAcro->acro_counter++;
			sprintf(buf, "Answer {G%d{M received!", gAcro->acro_counter);
			talk_acro(buf, 0);

			sprintf(buf, "{MYour answer of {W\"%s\"{M has been recorded.{x\n\r", originalArgument);
		}

		strcpy(ch->pcdata->acro_answer, originalArgument);
		send_to_char(buf, ch);

		return;
	}

	// Are they voting?
	if(gAcro->stage==ACRO_STAGE_ANNOUNCE_RESULTS_START || gAcro->stage==ACRO_STAGE_ANNOUNCE_ANSWERS)
	{
		vote_number = atoi(arg1);

		if(vote_number<1 || vote_number>gAcro->num_answers)
			invalid_vote = 1;

		if(invalid_vote)
		{
			sprintf(buf, "{MINVALID vote number. Valid vote numbers are {G1{M to {G%d{M{x\n\r", gAcro->num_answers);
			send_to_char(buf, ch);
			return;
		}

		
		if((vote_number-1)==array_find(gAcro->acro_answers, ch->pcdata->acro_answer))
		{
			send_to_char("{MYou cannot vote for yourself.\n\r", ch);
			return;
		}

		ch->pcdata->acro_has_participated = 1;

		if(ch->pcdata->acro_voted_for==0)
		{
			sprintf(buf, "{MYour vote for number {G%d{M has been recorded.{x\n\r", vote_number);
			gAcro->acro_votes[vote_number] = gAcro->acro_votes[vote_number]+1;
		}
		else if(ch->pcdata->acro_voted_for==vote_number)
			sprintf(buf, "{MYou have already voted for that number.{x\n\r");
		else
		{
			sprintf(buf, "{MYou have changed your vote, and you are now voting for number {G%d{M.{x\n\r", vote_number);
			gAcro->acro_votes[ch->pcdata->acro_voted_for] = gAcro->acro_votes[ch->pcdata->acro_voted_for]-1;
			gAcro->acro_votes[vote_number] = gAcro->acro_votes[vote_number]+1;
		}

		ch->pcdata->acro_voted_for = vote_number;
		send_to_char(buf, ch);

		return;
	}

	acro_help(ch);
    return;
}
