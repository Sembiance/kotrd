CC      = gcc
PROF    = -O -g3
NOCRYPT =
C_FLAGS =  -Wall -Wno-char-subscripts -Wcast-align -Wstrict-prototypes $(PROF) $(NOCRYPT)
L_FLAGS =  $(PROF)

O_FILES =   	act_clan.o \
		act_comm.o \
		act_enter.o \
		act_info.o \
		act_move.o \
		act_obj.o \
		act_wiz.o \
		act_wiz2.o \
		acro.o \
		alias.o \
		automap.o \
		ban.o \
		bank.o \
		bit.o \
		board.o \
		dip.o \
		casino.o \
		comm.o \
		const.o \
		db.o \
		db2.o \
		draglord.o \
		dpit.o \
		drunk.o \
		effects.o \
		fight.o \
		flags.o \
		forge.o \
		grant.o \
		handler.o \
		healer.o \
		ignore.o \
		interp.o \
		legend.o \
		lookup.o \
        lottery.o \
		magic.o \
		magic2.o \
		marry.o \
		mem.o \
		mob_cmds.o \
		mob_prog.o \
		olc.o \
		olc_act.o \
		olc_mpcode.o \
		olc_save.o \
		percentages.o \
		quest.o \
		recycle.o \
		save.o \
		scan.o \
		skills.o \
		special.o \
		string.o \
		StonesOfWisdom.o \
		tables.o \
		track.o \
		update.o \
		Utility.o \
		StringUtility.o \
		ArrayUtility.o

rom: $(O_FILES)
	rm -f rom
	$(CC) $(L_FLAGS) -o rom $(O_FILES) -lm -lcrypt

.c.o: merc.h
	$(CC) -c $(C_FLAGS) $<

clean:
	rm -f *.o
