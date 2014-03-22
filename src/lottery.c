#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "merc.h"
#include "tables.h"
#include "lookup.h"

#define MAX_LOTTERY         10000

DECLARE_DO_FUN(do_gecho);
void talk_channel( CHAR_DATA *ch, char *argument, int channel, const char *verb );

void do_flottery(CHAR_DATA *ch, char *argument )
{
    if (!IS_IMP(ch))
        return;

    send_to_char("{WDo you feel lucky?{x\n\r", ch);

    lottery_update(TRUE);

    return;
}

void do_lottery(CHAR_DATA *ch, char *argument )
{
    char buf[MAX_INPUT_LENGTH];

    if (IS_NPC(ch))
        return;

    if (IS_IMP(ch))
    {
        sprintf(buf, "{WThe {RL{YO{GT{CT{BE{mR{MY {RJ{YA{GC{CK{BP{mO{MT {Wis valued at {Y%d.{x\n\r",
            lottery);
    }
    else
    {
        sprintf(buf, "{WThe {RL{YO{GT{CT{BE{mR{MY {RJ{YA{GC{CK{BP{mO{MT {Wis estimated to be {Y%d.{x\n\r",
            lottery / 1000 * 1000);
    }

    send_to_char( buf, ch );

    return;
}

void save_misc(void)
{
    FILE *fp;
    
    if ( ( fp = fopen( MISC_FILE, "w" ) ) == NULL )
    {
        perror( MISC_FILE );
        return;
    }
    
    fprintf(fp, "Lottery %d\n", lottery);
    
    fclose(fp);
}

void load_misc( void )
{
    FILE *fp;
    char *temp;
    
    if ((fp = fopen(MISC_FILE,"r")) == NULL)
    {
    	bug("Couldn't open lottery file, reset to 0.",0);
        lottery = 0;
      	fclose(fp);
       	return;
    }
    
    temp = str_dup(fread_word(fp));
    lottery = fread_number(fp);
    
    fclose(fp);
}

void lottery_update( bool forced )
{
    CHAR_DATA   *ch=0;
    DESCRIPTOR_DATA *d;
    char buf[MAX_INPUT_LENGTH];
    int player;
    int i = 1;

    if (crm_players == 0)
        return;

    if (lottery < MAX_LOTTERY && !forced)
        return;

    if (number_percent() < 75 && !forced)
        return;

    player = number_range(1, crm_players);

    for ( d = descriptor_list; d; d = d->next )
    {
        if (i >= crm_players)
            return;

        if (i != player)
        {
            i++;
            continue;
        }

        if (d->connected != CON_PLAYING)
            return;

        ch   = ( d->original != NULL ) ? d->original : d->character;

        if (IS_IMMORTAL(ch))
            return;

        if (ch->level < 10)
            return;

        break;
    }
    
    if(!ch)
        return;

    send_to_char("{RC{YO{GN{CG{BR{mA{MT{WU{ML{mA{BT{CI{GO{YN{RS{W!! You've won the {RL{YO{GT{CT{BE{mR{MY{W!!{x\n\r", ch);
    sprintf( buf, "{WThe {YJACKPOT {Wwas worth {Y%d {Wgold.{x\n\r", lottery);
    send_to_char( buf, ch );

    ch->gold += lottery;
    lottery = 0;

    sprintf(buf, "{W[{RG{YR{GA{CT{MS{W] {xMota{W: {G%s {Whas won the {RL{YO{GT{CT{BE{mR{MY{W!{x\n\r", ch->name);    
    //talk_channel( ch, buf, CHANNEL_GRAT, "grats" );
    do_gecho (ch, buf);    
 
    return;
}
            
            

    
