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

char    noPicture[]="NOPICTURE";

char    owl1[]=" {g  ,{y_{g,   ";
char    owl2[]=" {y ({wO{g,{wO{y)  ";
char    owl3[]=" {y (   )  ";
char    owl4[]=" {y  {g\"{y-{g\"   ";
char    owl5[]="         ";

char    bear1[]="{c   ___   ";
char    bear2[]="{c (~{Y.{c_{Y.{c~) ";
char    bear3[]="{c _( {rY {c)_ ";
char    bear4[]="{c(:_{y~{B*{y~{c_:)";
char    bear5[]="{c (_)-(_) ";

char    heart1[]="{r __  __  ";
char    heart2[]="{r/  \\/  \\ ";
char    heart3[]="{r\\      / ";
char    heart4[]="{r \\    /  ";
char    heart5[]="{r  \\../   ";

char    dragon1[]="  {R^  ^   ";
char    dragon2[]="{r( {R:  : {r) ";
char    dragon3[]="  {r ||    ";
char    dragon4[]=" {r ({Roo{r)   ";
char    dragon5[]="         ";

char    cow1[]="{w(___)    ";
char    cow2[]="{w({Wo o{w)___/";
char    cow3[]=" {M@@ {W`{w   \\";
char    cow4[]=" {w \\ ___{W,{w/";
char    cow5[]="  {w//  // ";

char *  slotString(int lineNum, int picture);

void    do_chip(CHAR_DATA * ch, char * argument)
{
    char                arg1[MAX_INPUT_LENGTH];
    char                arg2[MAX_INPUT_LENGTH];
    char                arg3[MAX_INPUT_LENGTH];
    char                buf[MAX_STRING_LENGTH];
    OBJ_INDEX_DATA *    chipData;
    OBJ_DATA *          chipObject;
    OBJ_DATA *          sellObject;
    OBJ_DATA *          nextObject;
    int                 chipKind, number, chipValue, i, totalGold;
    
    totalGold=0;
    chipKind=SLOT_NONE;
    chipValue=0;
    number=1;
    
    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);
    argument = one_argument(argument, arg3);

    if(ch->in_room != get_room_index(SLOT_CHIP_OFFICE))
    {
        send_to_char("{gYou can not perform chip trading here.\n\r", ch);
        return;
    }

    if(arg1[0]=='\0')
    {
        send_to_char("{gUsage: chip buy <{Wwhite{g,{Rred{g,{Bblue{g,{Ggreen{g,{Yyellow{g> <number>\n\r", ch);
        send_to_char("{g       chip sell <{Wwhite{g,{Rred{g,{Bblue{g,{Ggreen{g,{Yyellow{g> <number>\n\r", ch);
        send_to_char("{g       chip list{x\n\r", ch);
        return;
    }
    
    if(strcmp(arg1, "list") && strcmp(arg1, "buy") && strcmp(arg1, "sell"))
    {
        send_to_char("{gUsage: chip buy <{Wwhite{g,{Rred{g,{Bblue{g,{Ggreen{g,{Yyellow{g> <number>\n\r", ch);
        send_to_char("{g       chip sell <{Wwhite{g,{Rred{g,{Bblue{g,{Ggreen{g,{Yyellow{g> <number>\n\r", ch);
        send_to_char("{g       chip list{x\n\r", ch);
        return;
    }
    
    if(arg2[0]=='\0' && strcmp(arg1, "list"))
    {
        send_to_char("{gWhat chip color is that?{x\n\r", ch);
        return;
    }
    
    if(!strcmp(arg2, "white"))
        chipKind=SLOT_WHITE;
    else if(!strcmp(arg2, "red"))
        chipKind=SLOT_RED;
    else if(!strcmp(arg2, "blue"))
        chipKind=SLOT_BLUE;
    else if(!strcmp(arg2, "green"))
        chipKind=SLOT_GREEN;
    else if(!strcmp(arg2, "yellow"))
        chipKind=SLOT_YELLOW;
    else if(strcmp(arg1, "list"))
    {
        sprintf(buf, "{g%s is not a valid chip color.{x\n\r", arg2);
        send_to_char(buf, ch);
        return;
    }

    if(arg3[0] != '\0')
    {
        number = atoi(arg3);
        if(number<=0 || number>50)
        {
            send_to_char("{gInvalid number of chips. Range is {G1 {gto{G 50{g.{x\n\r", ch);
            return;
        }
    }

    if(!strcmp(arg1, "list"))
    {
        send_to_char("{Go{g------------------------------------{Go\n\r", ch);
        send_to_char("{g|        {MChip Trade in Value         {g|\n\r", ch);
        send_to_char("{g|                                    {g|\n\r", ch);
        send_to_char("{g|  {Wwhite casino chip          {Y1 gold {g|\n\r", ch);
        send_to_char("{g|   {Bblue casino chip         {Y 5 gold {g|\n\r", ch);
        send_to_char("{g|    {Rred casino chip         {Y10 gold {g|\n\r", ch);
        send_to_char("{g|  {Ggreen casino chip        {Y100 gold {g|\n\r", ch);
        send_to_char("{g| {Yyellow casino chip        500 gold {g|\n\r", ch);
        send_to_char("{Go{g------------------------------------{Go{x\n\r\n\r", ch);
    
        return;
    }

    chipData=get_obj_index(chipKind);
    if(!chipData)
    {
        send_to_char("{gFatal error in Slot Machine Code. #07142128{x\n\r", ch);
        return;
    }
        
    chipValue=chipData->cost;   // although cost is in silver, its gonna be gold
    
    if(!strcmp(arg1, "buy"))
    {
        if((chipValue*number)>((ch->silver/100)+ch->gold))
        {
            sprintf(buf, "{gThe total price for that would be {Y%d gold{g. You do not have that much.{x\n\r", chipValue*number);
            send_to_char(buf, ch);
            return;
        }
    
        chipObject=create_object(chipData, chipData->level);
        
        if(chipObject->level > ch->level)
        {
            send_to_char("{gYou are not high enough level to buy that chip.{x\n\r", ch);
            return;
        }
        
        if(ch->carry_number +  number*get_obj_number(chipObject) > can_carry_n(ch))
        {
            send_to_char("{gYou can't carry that many items.{x\n\r", ch );
            return;
        }

        if(ch->carry_weight + number*get_obj_weight(chipObject) > can_carry_w(ch))
        {
            send_to_char("{gYou can't carry that much weight.{x\n\r", ch );
            return;
        }

        if(same_obj(chipObject, ch->carrying)>=75)
        {
            send_to_char("{gYou are carrying too many of those chips already. Max: {G75{g.{x\n\r", ch);
            return;
        }
        
        for(i=0;i<number;i++)
        {
            chipObject = create_object(chipData, chipData->level);
            obj_to_char(chipObject, ch);
        }
        
        deduct_cost(ch, ((chipValue*number)*100));
        
        sprintf(buf, "{g$n {gbuys $p{g[%d].", number);
        act(buf, ch, chipObject, NULL, TO_ROOM);

        sprintf(buf, "{gYou buy $p{g[%d].", number);
        act(buf, ch, chipObject, NULL, TO_CHAR);        
        
        return;
    }
    
    if(!strcmp(arg1, "sell"))
    {
        chipObject=create_object(chipData, chipData->level);
        
        if(same_obj(chipObject, ch->carrying)<number)
        {
            send_to_char("{gYou are not carrying that many chips.{x\n\r", ch);
            return;
        }
        
        for(sellObject=ch->carrying,i=0;sellObject!=NULL && i<number;sellObject=nextObject)
        {
            nextObject=sellObject->next;
            if(sellObject->pIndexData->vnum == chipObject->pIndexData->vnum)
            {
                extract_obj(sellObject);
                totalGold+=chipValue;
                i++;
                nextObject=ch->carrying;
            }
        }
        
        ch->gold+=totalGold;
        
        sprintf(buf, "{g$n {gsells $p{g[%d] for {Y%d gold{g.{x", i, totalGold);
        act(buf, ch, chipObject, NULL, TO_ROOM);
        sprintf(buf, "{gYou sell $p{g[%d] for {Y%d gold{g.{x", i, totalGold);
        act(buf, ch, chipObject, NULL, TO_CHAR);
        
        return;
    }
    
    send_to_char("{gFatal error in Slot Machine Code. #0088822{x\n\r", ch);
    
    return;
}

void    do_slot(CHAR_DATA * ch, char * argument)
{
    OBJ_DATA *              slotMachine;
    OBJ_DATA *              chip;
    OBJ_DATA *              obj;
    OBJ_DATA *              returnChip;
    OBJ_DATA *              returnChip2;
    OBJ_DATA *              returnChip3;
    EXTRA_DESCR_DATA *      ed;
    OBJ_INDEX_DATA *        returnChipData;
    bool                    foundSlot, foundChip, doneOnce, cantCarry;
    int                     slotClass, bar1, bar2, bar3, jackpot, left, i;
    char                    buf[MAX_STRING_LENGTH];
    char                    jackpotString[MAX_STRING_LENGTH];
    
    foundSlot=foundChip=doneOnce=cantCarry=FALSE;
    slotClass=SLOT_NONE;
    slotMachine=chip=NULL;
    jackpot=left=i=0;
    
    for(obj=ch->in_room->contents; obj!=NULL && !foundSlot; obj=obj->next_content)
    {
        if(obj->extra_descr != NULL || obj->pIndexData->extra_descr != NULL)
        {
            for(ed=obj->extra_descr; ed!=NULL && !foundSlot; ed=ed->next)
            {
                if(!str_cmp("whiteslot", ed->keyword))
                {
                    slotClass=SLOT_WHITE;
                    foundSlot=TRUE;
                    slotMachine=obj;
                }
                if(!str_cmp("blueslot", ed->keyword))
                {
                    slotClass=SLOT_BLUE;
                    foundSlot=TRUE;
                    slotMachine=obj;
                }
                if(!str_cmp("redslot", ed->keyword))
                {
                    slotClass=SLOT_RED;
                    foundSlot=TRUE;
                    slotMachine=obj;
                }
                if(!str_cmp("greenslot", ed->keyword))
                {
                    slotClass=SLOT_GREEN;
                    foundSlot=TRUE;
                    slotMachine=obj;
                }
                if(!str_cmp("yellowslot", ed->keyword))
                {
                    slotClass=SLOT_YELLOW;
                    foundSlot=TRUE;
                    slotMachine=obj;
                }
            }
            
            for(ed=obj->pIndexData->extra_descr; ed!=NULL && !foundSlot; ed=ed->next)
            {
                if(!str_cmp("whiteslot", ed->keyword))
                {
                    slotClass=SLOT_WHITE;
                    foundSlot=TRUE;
                    slotMachine=obj;
                }
                if(!str_cmp("blueslot", ed->keyword))
                {
                    slotClass=SLOT_BLUE;
                    foundSlot=TRUE;
                    slotMachine=obj;
                }
                if(!str_cmp("redslot", ed->keyword))
                {
                    slotClass=SLOT_RED;
                    foundSlot=TRUE;
                    slotMachine=obj;
                }
                if(!str_cmp("greenslot", ed->keyword))
                {
                    slotClass=SLOT_GREEN;
                    foundSlot=TRUE;
                    slotMachine=obj;
                }
                if(!str_cmp("yellowslot", ed->keyword))
                {
                    slotClass=SLOT_YELLOW;
                    foundSlot=TRUE;
                    slotMachine=obj;
                }
            }
        }
    }

    if(!foundSlot || slotClass==SLOT_NONE || slotMachine==NULL)
    {
        send_to_char("{gThere is no slot machine in this room.{x\n\r", ch);
        return;
    }
    
    if(slotMachine->pIndexData->weight>1000)
    {
        slotMachine->pIndexData->weight=slotMachine->pIndexData->cost;
        SET_BIT(slotMachine->pIndexData->area->area_flags, AREA_CHANGED);
    }
    
    jackpot=slotMachine->pIndexData->weight;
    left=jackpot;

    // we have a slot machine now, check to make sure that the player has the right chips
    for(obj=ch->carrying; obj!=NULL && !foundChip; obj=obj->next_content)
    {
        switch(slotClass)
        {
            case(SLOT_WHITE):
                if(obj->pIndexData->vnum==SLOT_WHITE)
                {
                    foundChip=TRUE;
                    chip=obj;
                    sprintf(jackpotString, "{g|{M_{g|---  {MJackpot: {W%4d white chips{g  -----/\n\r", jackpot);
                }
                break;
            case(SLOT_BLUE):
                if(obj->pIndexData->vnum==SLOT_BLUE)
                {
                    foundChip=TRUE;
                    chip=obj;
                    sprintf(jackpotString, "{g|{M_{g|---  {MJackpot: {B%4d  blue chips{g  -----/\n\r", jackpot);
                }
                break;
            case(SLOT_RED):
                if(obj->pIndexData->vnum==SLOT_RED)
                {
                    foundChip=TRUE;
                    chip=obj;
                    sprintf(jackpotString, "{g|{M_{g|---  {MJackpot: {R%4d  red  chips{g  -----/\n\r", jackpot);
                }
                break;
            case(SLOT_GREEN):
                if(obj->pIndexData->vnum==SLOT_GREEN)
                {
                    foundChip=TRUE;
                    chip=obj;
                    sprintf(jackpotString, "{g|{M_{g|---  {MJackpot: {G%4d green chips{g  -----/\n\r", jackpot);
                }
                break;
            case(SLOT_YELLOW):
                if(obj->pIndexData->vnum==SLOT_YELLOW)
                {
                    foundChip=TRUE;
                    chip=obj;
                    sprintf(jackpotString, "{g|{M_{g|---  {MJackpot: {Y%4d yellow chips{g -----/\n\r", jackpot);
                }
                break;
        }
    }
    
    if(!foundChip)
    {
        send_to_char("{gYou do not have the correct color chip.{x\n\r", ch);
        return;
    }

    if(!chip || !slotMachine)
    {
        send_to_char("{gFatal error in Slot Machine code. #0014427{x\n\r", ch);
        return;
    }

    //chip is the chip, slotMachine is our machine, extract the chip and gamble away!
    chip->owner=NULL;
    act("{g$n{g inserts $p {ginto $P{g and pulls the arm.{x", ch, chip, slotMachine, TO_ROOM);
    act("{gYou insert $p {ginto $P {gand pull the arm.{x", ch, chip, slotMachine, TO_CHAR);
    
    extract_obj(chip);
    
    bar1 = number_range(1, 5);
    bar2 = number_range(1, 5);
    bar3 = number_range(1, 5);

    /*magicWin = number_range(1, 12);
    if(magicWin==3)
    {
	bar2=bar1;
	bar3=bar1;
    }*/
    
    send_to_char("\n\r                 {R \\   /\n\r", ch);
    send_to_char("           {R )     {R)\\_/(     (\n\r", ch);
    send_to_char("           {r/{R|{r\\   {R(/\\|/\\)  {r /{R|{r\\\n\r", ch);
    send_to_char("          {r/ {R| {r\\   {R\\`|'/  {r / {R| {r\\       {g({Y@{g)\n\r", ch);
    send_to_char("{g/--------{r/{g--{R|{g--{Ro{g---{R\\|/{g---{Ro{g--{R|{g--{r\\{g------|{M_{g|\n\r", ch);
    send_to_char("{g|{M_{g|           {w'{W^{w`   {RV   {w'{W^{w`           {g|{M_{g|\n\r", ch);
    
    sprintf(buf, "{g|{M_{g| %s {G| %s {G| %s {g|{M_{g|\n\r", slotString(1, bar1), slotString(1, bar2), slotString(1, bar3));
    send_to_char(buf, ch);
    sprintf(buf, "{g|{M_{g| %s {G| %s {G| %s {g|{M_{g|\n\r", slotString(2, bar1), slotString(2, bar2), slotString(2, bar3));
    send_to_char(buf, ch);
    sprintf(buf, "{g|{M_{g| %s {G| %s {G| %s {g|{M_{g|\n\r", slotString(3, bar1), slotString(3, bar2), slotString(3, bar3));
    send_to_char(buf, ch);
    sprintf(buf, "{g|{M_{g| %s {G| %s {G| %s {g|{M_{g|\n\r", slotString(4, bar1), slotString(4, bar2), slotString(4, bar3));
    send_to_char(buf, ch);
    sprintf(buf, "{g|{M_{g| %s {G| %s {G| %s {g|{M_{g|\n\r", slotString(5, bar1), slotString(5, bar2), slotString(5, bar3));
    send_to_char(buf, ch);

    send_to_char("{g|{M_{g|           {G|           |  {g         |{M_{g|\n\r", ch);
    send_to_char(jackpotString, ch);
    send_to_char("{g({Y@{g) {r l    /\\ /    {R \\\\{r      \\ /\\    l\n\r", ch);
    send_to_char("    {r l  /   V    {R   ))    {r  V   \\  l\n\r", ch);
    send_to_char("    {r l/           {R //   {r          \\l\n\r", ch);
    send_to_char("                  {R V{x\n\r\n\r", ch);
    
    returnChipData=get_obj_index(slotClass);
    if(!returnChipData)
    {
        send_to_char("{gFatal error in Slot Machine Code. #0014492{x\n\r", ch);
        return;
    }
    
    if((bar1==bar2) && (bar2==bar3))    // we have a match!
    {
        if(bar1==1) //owl, return chip
        {
            returnChip=create_object(returnChipData, returnChipData->level);

	        slotMachine->pIndexData->weight--;
    	    SET_BIT(slotMachine->pIndexData->area->area_flags, AREA_CHANGED);

            if(same_obj(returnChip, ch->carrying) >= 75)
            {
                send_to_char("\n\r{gYou are unable to carry the chip you won. You have too many.{x\n\r", ch);
                obj_to_room(returnChip, ch->in_room);
                act("{g$P {gcomes up with {W3 Owls{g! A chip is spit out onto the floor because $n{g can not carry it.{x", ch, NULL, slotMachine, TO_ROOM);
            }
            else
            {
                obj_to_char(returnChip, ch);
                act("{g$P{g matches{W 3 Owls{g! A Chip is received by $n{g.{x", ch, NULL, slotMachine, TO_ROOM);
                act("{gYou matched {W3 Owls{g! You receive $p {gfrom $P{g!{x", ch, returnChip, slotMachine, TO_CHAR);
            }
        }
        
        if(bar1==2) //bear, return chip
        {
            returnChip=create_object(returnChipData, returnChipData->level);

    	    slotMachine->pIndexData->weight--;
	        SET_BIT(slotMachine->pIndexData->area->area_flags, AREA_CHANGED);

            if(same_obj(returnChip, ch->carrying) >= 75)
            {
                send_to_char("\n\r{gYou are unable to carry the chip you won. You have too many.{x\n\r", ch);
                obj_to_room(returnChip, ch->in_room);
                act("{g$P comes up with {B3 Bears{g! A chip is spit out onto the floor because $n {gcan not carry it.{x", ch, NULL, slotMachine, TO_ROOM);
            }
            else
            {
                obj_to_char(returnChip, ch);
                act("{g$P{g matches {B3 Bears{g! A Chip is received by $n{g.{x", ch, NULL, slotMachine, TO_ROOM);
                act("{gYou matched {B3 Bears{g! You receive $p {gfrom $P{g!{x", ch, returnChip, slotMachine, TO_CHAR);
            }
        }
    
        if(bar1==3) // heart, return 2x chips
        {
            returnChip=create_object(returnChipData, returnChipData->level);
            returnChip2=create_object(returnChipData, returnChipData->level);

	        slotMachine->pIndexData->weight-=2;
    	    SET_BIT(slotMachine->pIndexData->area->area_flags, AREA_CHANGED);

            if(same_obj(returnChip, ch->carrying) >= 75)
            {
                send_to_char("\n\r{gYou are unable to carry the chips you won. You have too many.{x\n\r", ch);
                obj_to_room(returnChip, ch->in_room);
                obj_to_room(returnChip2, ch->in_room);
                act("{g$P comes up with {R3 hearts{g! Two chips are spit out onto the floor because $n{g can not carry them.{x", ch, NULL, slotMachine, TO_ROOM);
            }
            else
            {
                obj_to_char(returnChip, ch);
                obj_to_char(returnChip2, ch);
                act("{g$P matches{R 3 Hearts{g! Two Chips are received by $n{g.{x", ch, NULL, slotMachine, TO_ROOM);
                act("{gYou matched {R3 Hearts{g! You receive 2 $p{g from $P{g!{x", ch, returnChip, slotMachine, TO_CHAR);
            }
        }

        if(bar1==5) // cow, return 3x chips
        {
            returnChip=create_object(returnChipData, returnChipData->level);
            returnChip2=create_object(returnChipData, returnChipData->level);
            returnChip3=create_object(returnChipData, returnChipData->level);

	        slotMachine->pIndexData->weight-=3;
    	    SET_BIT(slotMachine->pIndexData->area->area_flags, AREA_CHANGED);

            if(same_obj(returnChip, ch->carrying) >= 75)
            {
                send_to_char("\n\r{gYou are unable to carry the chips you won. You have too many.{x\n\r", ch);
                obj_to_room(returnChip, ch->in_room);
                obj_to_room(returnChip2, ch->in_room);
                obj_to_room(returnChip3, ch->in_room);
                act("{g$P{g comes up with{G 3 Cows{g! Three chips are spit out onto the floor because $n{g can not carry them.{x", ch, NULL, slotMachine, TO_ROOM);
            }
            else
            {
                obj_to_char(returnChip, ch);
                obj_to_char(returnChip2, ch);
                obj_to_char(returnChip3, ch);
                act("{g$P {gmatches {G3 Cows{g! Three Chips are received by $n{g.{x", ch, NULL, slotMachine, TO_ROOM);
                act("{gYou matched {G3 Cows{g! You receive 3 $p{g from $P{g!{x", ch, returnChip, slotMachine, TO_CHAR);
            }
        }

        if(bar1==4) // DRAGONS!, return JACKPOT
        {
            for(i=0;i<jackpot;i++)
            {
                returnChip=create_object(returnChipData, returnChipData->level);
                
                if(same_obj(returnChip, ch->carrying) >= 75)
                {
                    if(!cantCarry)
                    {
                        send_to_char("\n\r{gYou are unable to carry the chips you won. You have too many!{x\n\r", ch);
                        sprintf(buf, "{g$P{g comes up with {Y3 DRAGONS{g!!! %4d chips are spit out onto the floor because $n can not carry them.{x", left);
                        act(buf, ch, NULL, slotMachine, TO_ROOM);
                        
                        cantCarry=TRUE;
                    }

                    left--;
                    obj_to_room(returnChip, ch->in_room);
                }
                else
                {
                    if(!doneOnce)
                    {
                        sprintf(buf, "{g$P{g matches {Y3 DRAGONS{g!!! %4d Chips are received by $n{g.{x", left);
                        act(buf, ch, NULL, slotMachine, TO_ROOM);
                    
                        sprintf(buf, "{gYou matched{Y 3 DRAGONS{g!!! You receive %4d $p {gfrom $P{g!{x", left);
                        act(buf, ch, returnChip, slotMachine, TO_CHAR);
                        
                        doneOnce=TRUE;
                    }
                    
                    left--;
                    obj_to_char(returnChip, ch);
                }
            }
            slotMachine->pIndexData->weight=slotMachine->pIndexData->cost;
            SET_BIT(slotMachine->pIndexData->area->area_flags, AREA_CHANGED);
        }
    }
    else
    {
        act("{g$P{g returns to $n {gnothing!{x", ch, NULL, slotMachine, TO_ROOM);
        act("{gYou have come up with nothing!{x", ch, NULL, slotMachine, TO_CHAR);
        
        slotMachine->pIndexData->weight++;
        SET_BIT(slotMachine->pIndexData->area->area_flags, AREA_CHANGED);
    }
    
    return;
}

char *  slotString(int lineNum, int picture)
{
    switch(picture)
    {
        case(1):
            if(lineNum==1)
                return owl1;
            if(lineNum==2)
                return owl2;
            if(lineNum==3)
                return owl3;
            if(lineNum==4)
                return owl4;
            if(lineNum==5)
                return owl5;
            break;
        case(2):
            if(lineNum==1)
                return bear1;
            if(lineNum==2)
                return bear2;
            if(lineNum==3)
                return bear3;
            if(lineNum==4)
                return bear4;
            if(lineNum==5)
                return bear5;
            break;
        case(3):
            if(lineNum==1)
                return heart1;
            if(lineNum==2)
                return heart2;
            if(lineNum==3)
                return heart3;
            if(lineNum==4)
                return heart4;
            if(lineNum==5)
                return heart5;
            break;
        case(4):
            if(lineNum==1)
                return dragon1;
            if(lineNum==2)
                return dragon2;
            if(lineNum==3)
                return dragon3;
            if(lineNum==4)
                return dragon4;
            if(lineNum==5)
                return dragon5;
            break;
        case(5):
            
            if(lineNum==1)
                return cow1;
            if(lineNum==2)
                return cow2;
            if(lineNum==3)
                return cow3;
            if(lineNum==4)
                return cow4;
            if(lineNum==5)
                return cow5;
            break;
    }

    return noPicture;
}

