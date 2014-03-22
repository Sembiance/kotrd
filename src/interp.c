#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> /* unlink() */
#include <string.h>
#include <time.h>
#include "merc.h"
#include "interp.h"

char    last_command[MAX_STRING_LENGTH];
bool 	check_disabled (const struct cmd_type *command);
bool	check_social	args( ( CHAR_DATA *ch, char *command,
			    char *argument ) );

/*
 * Command logging types.
 */
#define LOG_NORMAL	0
#define LOG_ALWAYS	1
#define LOG_NEVER	2

DISABLED_DATA *disabled_first;

/*
 * Log-all switch.
 */
bool				fLogAll		= FALSE;



/*
 * Command table.
 */
const	struct	cmd_type	cmd_table	[] =
{
    // Movement Commands
    { "north",		   do_north,	    POS_STANDING,    0,  LOG_NEVER, 0 },
    { "east",		   do_east,	        POS_STANDING,	 0,  LOG_NEVER, 0 },
    { "south",		   do_south,	    POS_STANDING,	 0,  LOG_NEVER, 0 },
    { "west",		   do_west,	        POS_STANDING,	 0,  LOG_NEVER, 0 },
    { "up",		       do_up,		    POS_STANDING,	 0,  LOG_NEVER, 0 },
    { "down",		   do_down,	        POS_STANDING,	 0,  LOG_NEVER, 0 },
    { "northeast",	   do_northeast,	POS_STANDING,    0,  LOG_NEVER, 0 },
    { "southeast",	   do_southeast,	POS_STANDING,	 0,  LOG_NEVER, 0 },
    { "southwest",	   do_southwest,	POS_STANDING,	 0,  LOG_NEVER, 0 },
    { "northwest",	   do_northwest,	POS_STANDING,	 0,  LOG_NEVER, 0 },
    { "ne",		       do_northeast,	POS_STANDING,    0,  LOG_NEVER, 0 },
    { "se",		       do_southeast,	POS_STANDING,	 0,  LOG_NEVER, 0 },
    { "sw",		       do_southwest,	POS_STANDING,	 0,  LOG_NEVER, 0 },
    { "nw",		       do_northwest,	POS_STANDING,	 0,  LOG_NEVER, 0 },
        
    // Shortcutted commands
    { "'",             do_say,          POS_RESTING,     0,  LOG_NORMAL, 0 },
    { ",",             do_emote,        POS_RESTING,     0,  LOG_NORMAL, 0 },
    { ".",             do_gossip,       POS_SLEEPING,    0,  LOG_NORMAL, 0 },
    { ";",             do_gtell,        POS_RESTING,     0,  LOG_NORMAL, 0 },
    { "at",            do_at,           POS_DEAD,        GG, LOG_NORMAL, 1 },
    { "buy",           do_buy,          POS_RESTING,     0,  LOG_NORMAL, 1 },
    { "cast",          do_cast,         POS_FIGHTING,    0,  LOG_NORMAL, 1 },
    { "chip",		   do_chip,	        POS_STANDING, 	 0,  LOG_NORMAL, 1 },
    { "consider",      do_consider,     POS_RESTING,     0,  LOG_NORMAL, 1 },
    { "configure",     do_pcinfo,       POS_DEAD,        0,  LOG_NORMAL, 1 }, 
    { "group",         do_group,        POS_SLEEPING,    0,  LOG_NORMAL, 1 },
    { "groups",        do_groups,       POS_SLEEPING,    0,  LOG_NORMAL, 1 },
    { "inventory",     do_inventory,    POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "kill",          do_kill, 	    POS_FIGHTING, 	 0,  LOG_NORMAL, 0 },
    { "look",          do_look,         POS_RESTING,     0,  LOG_NORMAL, 1 },
    { "remove",        do_remove,       POS_RESTING,     0,  LOG_NORMAL, 1 },
    { "score",         do_score,        POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "scoreswap",     do_scoreswap,    POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "scoreold",         do_scoreold,        POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "tell",          do_tell,         POS_RESTING,     0,  LOG_NORMAL, 1 },
    { "wizlist",       do_wizlist,      POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "remort",        do_remort,       POS_DEAD,	     0,  LOG_NORMAL, 0 },
    { "remortlist",    do_remortlist,   POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "wiznet",        do_wiznet,       POS_DEAD,        0,  LOG_NORMAL, 0 },
    
    // Mortal commands
	{ "accept",		   do_stones_accept,POS_SLEEPING,	 0,	 LOG_NORMAL, 0 },    // Stones of Wisdom Stuff
	{ "acro",		   do_acro,		    POS_SLEEPING,	 0,	 LOG_NORMAL, 0 },    // Acrophobia Stuff
    { "affects",       do_affects,      POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "afk",           do_afk,          POS_SLEEPING,    0,  LOG_NORMAL, 1 },
    { "alia",          do_alia,         POS_DEAD,        0,  LOG_NORMAL, 0 },
    { "alias",         do_alias,        POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "alertme",       do_alertme,      POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "aquire",        do_aquire,       POS_STANDING,    0,  LOG_NORMAL, 0 },
    { "areas",         do_areas,        POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "dragonpit",	   do_dragonpit,    POS_STANDING,	 0,  LOG_NORMAL, 1 },
    { "dptalk",		   do_dptalk,	    POS_STANDING,	 0,  LOG_NORMAL, 1 },
    { "assassinate",   do_assassinate,	POS_FIGHTING,    0,  LOG_NORMAL, 0 },  
    { "auction", 	   do_auction, 	    POS_SLEEPING, 	 0,  LOG_NORMAL, 1 },
    { "autoassist",    do_autoassist,   POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "autoexit",      do_autoexit,     POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "autogold",      do_autogold,     POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "autoloot",      do_autoloot,     POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "autosac",       do_autosac,      POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "autosplit",     do_autosplit,    POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "bacid",         do_bacid,        POS_FIGHTING,    0,  LOG_NORMAL, 0 },
    { "backstab",      do_backstab,     POS_FIGHTING,    0,  LOG_NORMAL, 0 },
    { "ban",           do_ban,          POS_DEAD,        ML, LOG_NORMAL, 1 }, 
    { "bankshow",	   do_bankshow,	    POS_RESTING,	 0,  LOG_NORMAL, 1 }, 
    { "bash",          do_bash,         POS_FIGHTING,    0,  LOG_NORMAL, 0 },
    { "beep",		   do_beep,	        POS_SLEEPING,	 0,  LOG_NORMAL, 1 },
    { "berserk",       do_berserk,      POS_FIGHTING,    0,  LOG_NORMAL, 0 },
    { "bfire",         do_bfire,        POS_FIGHTING,    0,  LOG_NORMAL, 0 },
    { "bfrost",        do_bfrost,       POS_FIGHTING,    0,  LOG_NORMAL, 0 },
    { "bgas",          do_bgas,         POS_FIGHTING,    0,  LOG_NORMAL, 0 },
	{ "bid",		   do_stones_bid,	POS_SLEEPING,	 0,	 LOG_NORMAL, 0 },    // Stones of Wisdom Stuff
    { "blightning",    do_blightning,   POS_FIGHTING,    0,  LOG_NORMAL, 0 },
    { "brandish",      do_brandish,     POS_RESTING,     0,  LOG_NORMAL, 1 },
    { "bounty",        do_bounty,       POS_STANDING,    25, LOG_NORMAL, 1 },
    { "brew",		   do_brew,	        POS_RESTING,     0,  LOG_NORMAL, 0 },
    { "brief",         do_brief,        POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "bs",            do_backstab,     POS_FIGHTING,    0,  LOG_NORMAL, 0 },
    { "classtat",      do_classtat,     POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "chameleon",     do_chameleon_power, POS_STANDING, 0,  LOG_NORMAL, 0 },
	{ "challenge",	   do_stones_challenge, POS_SLEEPING,0,	 LOG_NORMAL, 0 },    // Stones of Wisdom Stuff
    { "channels",      do_channels,     POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "chant",		   do_chant,	    POS_SITTING,     0,  LOG_NORMAL, 0 },
    { "chat",          do_gossip,       POS_SLEEPING,    0,  LOG_NORMAL, 1 },
    { "choose",        do_choose,       POS_DEAD,        0,  LOG_ALWAYS, 1 },
    { "circle",		   do_circle,       POS_FIGHTING,    0,  LOG_NORMAL, 0 }, 
    { "clans",		   do_clans,	    POS_SLEEPING,	 0,  LOG_NORMAL, 1 },
    { "close",         do_close,        POS_RESTING,     0,  LOG_NORMAL, 1 },
    { "colour",        do_colour,       POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "combine",       do_combine,      POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "commands",      do_commands,     POS_DEAD,        0,  LOG_NORMAL, 0 },
    { "compact",       do_compact,      POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "compare",       do_compare,      POS_RESTING,     0,  LOG_NORMAL, 1 },
    { "consent",	   do_consent,	    POS_RESTING,	 1,  LOG_NORMAL, 1 },
    { "combat",        do_combat,       POS_SLEEPING,    0,  LOG_NORMAL, 1 },
    { "crecall",	   do_crecall,      POS_STANDING,    0,  LOG_NORMAL, 0 },
    { "critical",      do_critical_strike, POS_FIGHTING, 0,  LOG_NORMAL, 0 }, 
    { "credits",       do_credits,      POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "ct",		       do_clantalk,	    POS_SLEEPING,	 0,  LOG_NORMAL, 0 },  
    { "cwar",		   do_cwar,	        POS_RESTING,	 0,  LOG_NORMAL, 0 },
    { "deaf",          do_deaf,         POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "delet",         do_delet,        POS_DEAD,        0,  LOG_ALWAYS, 0 },
    { "delete",        do_delete,       POS_STANDING,    0,  LOG_ALWAYS, 1 },
    { "deposit",	   do_deposit,	    POS_STANDING,	 0,  LOG_NORMAL, 0 }, 
    { "account",	   do_account,	    POS_STANDING,	 0,  LOG_NORMAL, 1 }, 
    { "share",	 	   do_share,	    POS_STANDING,	 0,  LOG_NORMAL, 0 }, 
    { "change",	 	   do_change,	    POS_STANDING,	 0,  LOG_NORMAL, 0 }, 
    { "demonic",	   do_demonic,   	POS_RESTING,  	 0,  LOG_NORMAL, 1 },
    { "description",   do_description,  POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "dirt",          do_dirt,         POS_FIGHTING,    0,  LOG_NORMAL, 0 },
    { "disarm",        do_disarm,       POS_FIGHTING,    0,  LOG_NORMAL, 0 },
    { "dip",           do_dip,          POS_STANDING,    0,  LOG_NORMAL, 1 },
    { "donate",        do_donate,       POS_RESTING,     0,  LOG_NORMAL, 1 },
    { "draconian",	   do_draconian,    POS_RESTING,  	 0,  LOG_NORMAL, 1 },
    { "drink",         do_drink,        POS_RESTING,     0,  LOG_NORMAL, 0 },
    { "drop",          do_drop,         POS_RESTING,     0,  LOG_NORMAL, 1 },
    { "drow",		   do_drow,   	    POS_RESTING,  	 0,  LOG_NORMAL, 1 },
    { "dual",          do_second,       POS_RESTING,     0,  LOG_NORMAL, 0 },
    { "dwarven",	   do_dwarven,   	POS_RESTING,  	 0,  LOG_NORMAL, 1 },
    { "eat",           do_eat,          POS_RESTING,     0,  LOG_NORMAL, 0 },
    { "elven",		   do_elven,   	    POS_RESTING,  	 0,  LOG_NORMAL, 1 },
    { "emote",         do_emote,        POS_RESTING,     0,  LOG_NORMAL, 1 },
    { "enter",         do_enter,        POS_STANDING,    0,  LOG_NORMAL, 1 },
    { "envenom",       do_envenom,      POS_RESTING,     0,  LOG_NORMAL, 0 },
    { "equipment",     do_equipment,    POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "exits",         do_exits,        POS_RESTING,     0,  LOG_NORMAL, 1 },
    { "fill",          do_fill,         POS_RESTING,     0,  LOG_NORMAL, 1 },
    { "flee",          do_flee,         POS_FIGHTING,    0,  LOG_NORMAL, 1 },
    { "follow",        do_follow,       POS_RESTING,     0,  LOG_NORMAL, 1 },
    { "forestsign",	   do_forestsign,   POS_RESTING,  	 0,  LOG_NORMAL, 1 },
	{ "forfeit",	   do_stones_forfeit, POS_SLEEPING,	 0,	 LOG_NORMAL, 0 },    // Stones of Wisdom Stuff
    { "forge",         do_forge,        POS_STANDING,    0,  LOG_ALWAYS, 0 },
    { "gain",          do_gain,         POS_STANDING,    0,  LOG_NORMAL, 1 },
    { "get",           do_get,          POS_RESTING,     0,  LOG_NORMAL, 0 },
    { "grats",         do_grats,        POS_SLEEPING,    0,  LOG_NORMAL, 1 },
    { "join",          do_join,         POS_RESTING,     0,  LOG_NORMAL, 1 },
    { "give",          do_give,         POS_RESTING,     0,  LOG_NORMAL, 1 },
    { "godspeak",	   do_godspeak,   	POS_RESTING,  	 0,  LOG_NORMAL, 1 },
    { "gtell",         do_gtell,        POS_RESTING,     0,  LOG_NORMAL, 1 },
    { "guild",         do_guild,      	POS_DEAD,        20, LOG_ALWAYS, 0 },
    { "handtalk",	   do_handtalk,   	POS_RESTING,  	 0,  LOG_NORMAL, 1 },
    { "heal",          do_heal,         POS_RESTING,     0,  LOG_NORMAL, 1 },
    { "heighten",      do_heighten_senses, POS_STANDING, 0,  LOG_NORMAL, 0 },
    { "help",          do_help,         POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "hero",          do_hero,         POS_SLEEPING,    0,  LOG_NORMAL, 0 }, 
    { "hide",          do_hide,         POS_RESTING,     0,  LOG_NORMAL, 0 },
    { "high",	       do_high_elven,   POS_RESTING,  	 0,  LOG_NORMAL, 1 },
    { "hit",		   do_kill,	        POS_FIGHTING,	 0,  LOG_NORMAL, 0 },
    { "hold",          do_wear,         POS_RESTING,     0,  LOG_NORMAL, 1 },
    { "board",		   do_board,	    POS_SLEEPING,	 0,  LOG_NORMAL, 1 },
    { "identify",      do_identify,     POS_STANDING,    0,  LOG_NORMAL, 1 },
    { "ignore",        do_ignore,       POS_DEAD,        0,  LOG_ALWAYS, 1 },
    { "info",          do_groups,       POS_SLEEPING,    0,  LOG_NORMAL, 1 },
	{ "invite",		   do_stones_invite,POS_SLEEPING,	 0,	 LOG_NORMAL, 0 },    // Stones of Wisdom Stuff
    { "italian",	   do_italian,	    POS_RESTING,  	 0,  LOG_NORMAL, 0 },
    { "kender",		   do_kender,   	POS_RESTING,  	 0,  LOG_NORMAL, 1 },
    { "kick",          do_kick,         POS_FIGHTING,    0,  LOG_NORMAL, 0 },
    { "list",          do_list,         POS_RESTING,     0,  LOG_NORMAL, 1 },
    { "lock",          do_lock,         POS_RESTING,     0,  LOG_NORMAL, 1 },
    { "junk",          do_junk,         POS_RESTING,     0,  LOG_NORMAL, 1 },
    { "legend",        do_legend,       POS_STANDING,    0,  LOG_ALWAYS, 1 },   // LEGEND SYSTEM
    { "lottery",       do_lottery,      POS_RESTING,     0,  LOG_NORMAL, 1 },
    { "magespeak",	   do_magespeak,  	POS_RESTING,  	 0,  LOG_NORMAL, 1 },
    { "map",           do_map,		    POS_STANDING,	 0,  LOG_NORMAL, 1 },
    { "motd",          do_motd,         POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "multiburst",	   do_multiburst,	POS_FIGHTING,    0,  LOG_NORMAL, 0 },
    { "nephilim",      do_nephilim,     POS_RESTING,     0,  LOG_NORMAL, 1 },
    { "nofollow",      do_nofollow,     POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "norestore",     do_norestore,    POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "noloot",        do_noloot,       POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "nosummon",      do_nosummon,     POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "note",		   do_note,	        POS_DEAD,	     0,  LOG_NORMAL, 1 },
    { "ntalk",         do_ntalk,        POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "open",          do_open,         POS_RESTING,     0,  LOG_NORMAL, 1 },
    { "order",         do_order,        POS_RESTING,     0,  LOG_NORMAL, 1 },
    { "outfit",        do_outfit,       POS_RESTING,     0,  LOG_NORMAL, 1 },
    { "password",      do_password,     POS_DEAD,        0,  LOG_NEVER,  1 },
    { "pcinfo",        do_pcinfo,       POS_DEAD,        0,  LOG_NORMAL, 1 },
	{ "percentages",   do_percentages,	POS_SLEEPING,	 0,	 LOG_NORMAL, 0 },    // Explorer/Killer Percentages
	{ "percenthint",   do_percenthint,	POS_SLEEPING,	 0,	 LOG_NORMAL, 0 },    // Explorer/Killer Percentages
    { "pick",          do_pick,         POS_RESTING,     0,  LOG_NORMAL, 0 },
    { "pour",          do_pour,         POS_RESTING,     0,  LOG_NORMAL, 1 },
    { "practice",      do_practice,     POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "prompt",        do_prompt,       POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "put",           do_put,          POS_RESTING,     0,  LOG_NORMAL, 1 },
    { "push",          do_push,         POS_STANDING,    0,  LOG_NORMAL, 1 },
    { "quaff",         do_quaff,        POS_STANDING,    0,  LOG_NORMAL, 1 },
    { "quest",         do_quest,        POS_RESTING,     0,  LOG_NORMAL, 1 },
    { "quicken",	   do_quicken,      POS_STANDING,    0,  LOG_NORMAL, 0 },
    { "quiet",         do_quiet,        POS_SLEEPING,    0,  LOG_NORMAL, 1 },
    { "qui",           do_qui,          POS_DEAD,        0,  LOG_NORMAL, 0 },
    { "quit",          do_quit,         POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "racestat",      do_racestat,     POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "recall",        do_recall,       POS_FIGHTING,    0,  LOG_NORMAL, 1 },
    { "read",          do_read,         POS_RESTING,     0,  LOG_NORMAL, 1 },
    { "recite",        do_recite,       POS_RESTING,     0,  LOG_NORMAL, 1 },
    { "replay",        do_replay,       POS_RESTING,     0,  LOG_NORMAL, 0 },
    { "reply",         do_reply,        POS_RESTING,     0,  LOG_NORMAL, 1 },
    { "report",        do_report,       POS_RESTING,     0,  LOG_NORMAL, 1 },
    { "reptilian",	   do_reptilian,    POS_RESTING,  	 0,  LOG_NORMAL, 1 },
    { "rescue",        do_rescue,       POS_FIGHTING,    0,  LOG_NORMAL, 0 },
    { "rest",          do_rest,         POS_SLEEPING,    0,  LOG_NORMAL, 1 },
    { "rp",            do_rp,           POS_RESTING,     0,  LOG_NORMAL, 1 },
    { "rules",         do_rules,        POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "sacrifice",     do_sacrifice,    POS_RESTING,     0,  LOG_NORMAL, 1 },
    { "save",          do_save,         POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "say",           do_say,          POS_RESTING,     0,  LOG_NORMAL, 1 },
    { "scan",		   do_scan,	        POS_DEAD,	     0,  LOG_NORMAL, 1 },
    { "scribe",		   do_scribe,	    POS_RESTING,     0,  LOG_NORMAL, 0 },
    { "scroll",        do_scroll,       POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "sell",          do_sell,         POS_RESTING,     0,  LOG_NORMAL, 1 },
    { "shadow",		   do_shadow_form,	POS_STANDING,	 0,  LOG_NORMAL, 0 },
    { "show",          do_show,         POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "sit",           do_sit,          POS_SLEEPING,    0,  LOG_NORMAL, 1 },
    { "skills",        do_skills,       POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "sleep",         do_sleep,        POS_SLEEPING,    0,  LOG_NORMAL, 1 },
    { "slot",		   do_slot,	        POS_STANDING,	 0,  LOG_NORMAL, 1 },
    { "sneak",         do_sneak,        POS_STANDING,    0,  LOG_NORMAL, 0 },
    { "socials",       do_socials,      POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "spells",        do_spells,       POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "split",         do_split,        POS_RESTING,     0,  LOG_NORMAL, 1 },
    { "spousetalk",	   do_spousetalk,	POS_SLEEPING,	 1,  LOG_NEVER,  1 },
    { "stand",         do_stand,        POS_SLEEPING,    0,  LOG_NORMAL, 1 },
    { "steal",         do_steal,        POS_STANDING,    0,  LOG_NORMAL, 0 },
	{ "stones",		   do_stones,		POS_SLEEPING,	 0,	 LOG_NORMAL, 0 },    // Stones of Wisdom Stuff
    { "story",         do_story,        POS_DEAD,        0,  LOG_NORMAL, 0 },
	{ "suicide",	   do_suicide,		POS_SLEEPING,	 0,	 LOG_NORMAL, 0 },
    { "surrender",     do_surrender,    POS_FIGHTING,    0,  LOG_NORMAL, 1 },
    { "take",          do_get,          POS_RESTING,     0,  LOG_NORMAL, 1 },
    { "thieves",       do_thieves_cant, POS_RESTING,  	 0,  LOG_NORMAL, 1 },
    { "time",          do_time,         POS_DEAD,        0,  LOG_NORMAL, 0 },
    { "title",         do_title,        POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "top",           do_top_percentages, POS_SLEEPING, 0,  LOG_NORMAL, 0 },    // Explorer/Killer Percentages
    { "track",		   do_track,	    POS_STANDING,	 0,  LOG_NORMAL, 0 },
    { "train",         do_train,        POS_STANDING,    0,  LOG_NORMAL, 1 },
    { "trip",          do_trip,         POS_FIGHTING,    0,  LOG_NORMAL, 0 },
    { "unalias",       do_unalias,      POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "undead",		   do_undead,   	POS_RESTING,  	 0,  LOG_NORMAL, 1 },
    { "unlock",        do_unlock,       POS_RESTING,     0,  LOG_NORMAL, 1 },
    { "vent",		   do_vent,	        POS_SLEEPING,	 0,  LOG_NORMAL, 1 },
    { "visible",       do_visible,      POS_SLEEPING,    0,  LOG_NORMAL, 1 },
    { "war",	       do_war_chant,	POS_RESTING,  	 0,  LOG_NORMAL, 1 },
    { "wake",          do_wake,         POS_SLEEPING,    0,  LOG_NORMAL, 1 },
    { "wear",          do_wear,         POS_RESTING,     0,  LOG_NORMAL, 1 },
    { "where",         do_where,        POS_RESTING,     0,  LOG_NORMAL, 1 },
    { "whirlwind",     do_whirlwind,    POS_FIGHTING,	 0,  LOG_NORMAL, 0 },
    { "who",           do_who,          POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "wield",         do_wear,         POS_RESTING,     0,  LOG_NORMAL, 1 },
    { "wimpy",         do_wimpy,        POS_DEAD,        0,  LOG_NORMAL, 1 },
    { "withdraw",  	   do_withdraw, 	POS_STANDING,	 0,  LOG_NORMAL, 0 }, 
    { "worth",         do_account,      POS_SLEEPING,    0,  LOG_NORMAL, 1 },
    { "yell",          do_yell,         POS_RESTING,     0,  LOG_NORMAL, 1 },
    { "zap",           do_zap,          POS_RESTING,     0,  LOG_NORMAL, 1 },

    // Implementor Commands (lvl 500)
    { "@",             do_ximm,         POS_DEAD,        ML, LOG_NORMAL, 0 },
    { "addlag",        do_addlag,       POS_DEAD,        ML, LOG_NORMAL, 1 },
    { "advance",       do_advance,      POS_DEAD,        ML, LOG_ALWAYS, 1 },
    { "aedit",		   do_aedit,	    POS_DEAD,    	 ML, LOG_NORMAL, 1 },
    { "allow",         do_allow,        POS_DEAD,        ML, LOG_NORMAL, 1 },
    { "arealinks",     do_arealinks,    POS_DEAD,        ML, LOG_NORMAL, 1 },
    { "bolt",          do_bolt,         POS_DEAD,        ML, LOG_NORMAL, 0 },
    { "bonus",         do_bonus,        POS_DEAD,        ML, LOG_ALWAYS, 1 },
    { "cedit",         do_cedit,        POS_DEAD,        ML, LOG_NORMAL, 1 },
    { "copyove",       do_copyove,      POS_DEAD,        ML, LOG_NORMAL, 0 },
    { "copyover",      do_copyover,     POS_DEAD,        ML, LOG_ALWAYS, 1 }, 
    { "ctimp",         do_ctimp,        POS_DEAD,        ML, LOG_ALWAYS, 1 },
    { "discon",        do_discon,       POS_DEAD,        ML, LOG_NORMAL, 1 },
    { "doat",          do_doat,         POS_DEAD,        ML, LOG_NORMAL, 1 },
    { "dump",          do_dump,         POS_DEAD,        ML, LOG_NORMAL, 0 },
    { "emboo",         do_emboo,        POS_DEAD,        ML, LOG_NORMAL, 0 },
    { "emboot",        do_emboot,       POS_DEAD,        ML, LOG_NORMAL, 0 },
    { "fileident",     do_fileident,    POS_DEAD,        ML, LOG_NORMAL, 0 },
    { "fixage",        do_fixage,       POS_DEAD,        ML, LOG_NORMAL, 1 },
    { "flag",          do_flag,         POS_DEAD,        ML, LOG_ALWAYS, 1 },
    { "flame",         do_flame,        POS_DEAD,        ML, LOG_NORMAL, 0 },
    { "flottery",      do_flottery,     POS_DEAD,        ML, LOG_NORMAL, 1 },
    { "ftick",         do_ftick,        POS_DEAD,        ML, LOG_NORMAL, 1 },
    { "grant",         do_grant,        POS_DEAD,        ML, LOG_NORMAL, 1 },    // Grant system
    { "log",           do_log,          POS_DEAD,        ML, LOG_ALWAYS, 1 },
    { "memory",        do_memory,       POS_DEAD,        ML, LOG_NORMAL, 1 },
    { "mlevel",        do_mlevel,       POS_DEAD,        ML, LOG_NORMAL, 1 },
    { "mpdump",		   do_mpdump,	    POS_DEAD,	     ML, LOG_NORMAL, 0 },
    { "mpedit",		   do_mpedit,	    POS_DEAD,    	 ML, LOG_NORMAL, 0 },
    { "mpstat",		   do_mpstat,	    POS_DEAD,	     ML, LOG_NORMAL, 0 },
    { "newlock",       do_newlock,      POS_DEAD,        ML, LOG_NORMAL, 1 },
    { "object",        do_object,       POS_DEAD,        ML, LOG_NORMAL, 0 },
    { "olevel",        do_olevel,       POS_DEAD,        ML, LOG_NORMAL, 1 },
    { "otype",         do_otype,        POS_DEAD,        ML, LOG_NORMAL, 1 },
    { "owner",  	   do_owner, 	    POS_DEAD, 	     ML, LOG_NEVER,  1 },
    { "permban",       do_permban,      POS_DEAD,        ML, LOG_NORMAL, 1 },
    { "pload",         do_pload,        POS_DEAD,        ML, LOG_ALWAYS, 1 },
    { "plzap",         do_plzap,        POS_DEAD,        ML, LOG_ALWAYS, 1 },
    { "prefi",         do_prefi,        POS_DEAD,        ML, LOG_NORMAL, 0 },
    { "psycho",        do_psycho,       POS_DEAD,        ML, LOG_NORMAL, 1 },
    { "punload",       do_punload,      POS_DEAD,        ML, LOG_NORMAL, 1 },
    { "reboo",         do_reboo,        POS_DEAD,        ML, LOG_NORMAL, 0 },
    { "reboot",        do_reboot,       POS_DEAD,        ML, LOG_NORMAL, 1 },
    { "revoke",        do_revoke,       POS_DEAD,        ML, LOG_NORMAL, 1 },    // Grant system
    { "rset",          do_rset,         POS_DEAD,        ML, LOG_NORMAL, 1 },
    { "rstat",         do_rstat,        POS_DEAD,        ML, LOG_NORMAL, 1 },
    { "stat",          do_stat,         POS_DEAD,        ML, LOG_NORMAL, 1 },
    { "shutdow",       do_shutdow,      POS_DEAD,        ML, LOG_NORMAL, 0 },
    { "shutdown",      do_shutdown,     POS_DEAD,        ML, LOG_NORMAL, 1 },
    { "sla",           do_sla,          POS_DEAD,        ML, LOG_NORMAL, 0 },
    { "sset",          do_sset,         POS_DEAD,        ML, LOG_NORMAL, 1 },
    { "sockets",       do_sockets,      POS_DEAD,        ML, LOG_NORMAL, 1 },
    { "spellup",       do_spellup,      POS_DEAD,        ML, LOG_NORMAL, 1 },
    { "updatetop",     do_updatetop,    POS_DEAD,        ML, LOG_NORMAL, 1 },    // Explorer/Killer Percentages
    { "vlist",         do_vlist,        POS_DEAD,        ML, LOG_NORMAL, 1 },
    { "wizlock",       do_wizlock,      POS_DEAD,        ML, LOG_NORMAL, 1 },
    
    // Assistant Implementor Commands (lvl 499)
    { "imp",           do_imp,          POS_DEAD,        AI, LOG_NORMAL, 0 },
    { "deny",          do_deny,         POS_DEAD,        AI, LOG_ALWAYS, 1 },
    { "disable",       do_disable,      POS_DEAD,        AI, LOG_ALWAYS, 1 },
    { "gecho",         do_gecho,       	POS_DEAD,        AI, LOG_NORMAL, 1 },

    // Greater Commands (lvl 108)
    { "clone",         do_clone,        POS_DEAD,        GG, LOG_NORMAL, 1 },
    { "invis",         do_invis,        POS_DEAD,        GG, LOG_NORMAL, 0 },
    { "wpeace",        do_wpeace,       POS_DEAD,        GG, LOG_ALWAYS, 1 },
    { "fv",            do_fvlist,       POS_DEAD,        GG, LOG_NORMAL, 0 },
    { "vl",            do_vnumlist,     POS_DEAD,        GG, LOG_NORMAL, 0 },

    // Lesser God Commands (lvl 107)
    // DemiGod Commands (lvl 106)
    // Veteran Immortal Commands (lvl 105)
    
    // Newbie Immortal Commands (lvl 104)
    { "prefix",        do_prefix,       POS_DEAD,        NI, LOG_NORMAL, 1 },
    { "poofin",		   do_bamfin,	    POS_DEAD,	     NI, LOG_NORMAL, 1 },
    { "poofout",	   do_bamfout,	    POS_DEAD,	     NI, LOG_NORMAL, 1 },

    // Builder Commands  (lvl 103)
    { "builder",	   do_builder,	    POS_DEAD,	     BL, LOG_NORMAL, 1 },
    { "holylight",	   do_holylight,	POS_DEAD,	     BL, LOG_NORMAL, 1 },

    // Visiting Immortal Commands (lvl 102)

    // All other immortal commands (lvl 101)
    { "alist",		   do_alist,	    POS_DEAD,    	 IM, LOG_NORMAL, 0 },
    { "asave",         do_asave,	    POS_DEAD,   	 IM, LOG_NORMAL, 0 },
    { "dbag",          do_dbag,       	POS_DEAD,        IM, LOG_ALWAYS, 0 },
    { "divorce",	   do_divorce,	    POS_DEAD,	     IM, LOG_NORMAL, 0 },
    { "dpitkill",	   do_dpitkill,	    POS_DEAD,	     IM, LOG_ALWAYS, 0 },
    { "dummy",         do_dummy,        POS_DEAD,        IM, LOG_NORMAL, 0 },
	{ "fine",		   do_fine,	        POS_DEAD,		 ML, LOG_NORMAL, 0 },
    { "force",		   do_force,	    POS_DEAD,	     IM, LOG_NORMAL, 0 },
    { "freeze",		   do_freeze,	    POS_DEAD,	     IM, LOG_ALWAYS, 0 },
    { "fvlist",	       do_fvlist,	    POS_DEAD,	     IM, LOG_NORMAL, 0 },
    { "goto",          do_goto,       	POS_DEAD,        IM, LOG_NORMAL, 0 },
    { "grab",          do_grab,       	POS_DEAD,        IM, LOG_NORMAL, 0 },
    { "hedit",		   do_hedit,	    POS_DEAD,    	 IM, LOG_NORMAL, 0 },
    { "icon",		   do_icon,	        POS_DEAD,        IM, LOG_ALWAYS, 0 }, 
    { "incognito",     do_incog,  	    POS_DEAD,        IM, LOG_NORMAL, 0 },
    { "immtalk",	   do_immtalk,	    POS_DEAD,	     IM, LOG_NORMAL, 1 },
    { "immhelp",       do_immhelp,      POS_SLEEPING,    IM, LOG_NORMAL, 0 },
    { "imotd",         do_imotd,        POS_DEAD,        IM, LOG_NORMAL, 1 },
    { "jail",          do_jail,       	POS_DEAD,        IM, LOG_ALWAYS, 0 },
    { "links",		   do_links,	    POS_DEAD,    	 IM, LOG_NORMAL, 0 },
    { "marry",	  	   do_marry,	    POS_DEAD,	     IM, LOG_ALWAYS, 0 },
    { "medit",		   do_medit,	    POS_DEAD,    	 IM, LOG_NORMAL, 0 },
    { "mload",         do_mload,	    POS_DEAD,	     IM, LOG_NORMAL, 0 },
    { "msearch",       do_msearch,	    POS_DEAD,	     IM, LOG_NORMAL, 0 },
    { "mset",          do_mset,       	POS_DEAD,        IM, LOG_ALWAYS, 0 },
    { "mstat",         do_mstat,	    POS_DEAD,	     IM, LOG_NORMAL, 0 },
    { "mwhere",        do_mwhere,     	POS_DEAD,        IM, LOG_NORMAL, 0 },
    { "nochan",		   do_nochan,	    POS_DEAD,	     IM, LOG_ALWAYS, 0 },
    { "noemote",	   do_noemote,	    POS_DEAD,	     IM, LOG_NORMAL, 0 },
    { "nopnote",	   do_nopnote,	    POS_DEAD,	     IM, LOG_NORMAL, 0 },
    { "notell",        do_notell,     	POS_DEAD,        IM, LOG_NORMAL, 0 },
    { "notitle",	   do_notitle,	    POS_DEAD,	     IM, LOG_NORMAL, 0 },
    { "oedit",		   do_oedit,	    POS_DEAD,    	 IM, LOG_NORMAL, 0 },
    { "oload",         do_oload,	    POS_DEAD,	     IM, LOG_NORMAL, 0 },
    { "osearch",       do_osearch,	    POS_DEAD,	     IM, LOG_NORMAL, 0 },
    { "oset",          do_oset,       	POS_DEAD,        IM, LOG_NORMAL, 0 },
    { "ostat",         do_ostat,	    POS_DEAD,	     IM, LOG_NORMAL, 0 },
    { "overview",      do_overview,     POS_DEAD,        IM, LOG_NORMAL, 0 },
    { "owhere",        do_owhere,     	POS_DEAD,        IM, LOG_NORMAL, 0 },
    { "pardon",        do_pardon,     	POS_DEAD,        IM, LOG_ALWAYS, 0 },
    { "peace",         do_peace,      	POS_DEAD,        IM, LOG_NORMAL, 0 },
    { "pecho",         do_pecho,      	POS_DEAD,        IM, LOG_NORMAL, 0 },
    { "pnote",         do_pnote,      	POS_DEAD,        IM, LOG_NORMAL, 0 },
    { "purge",         do_purge,        POS_DEAD,        IM, LOG_NORMAL, 0 },
    { "recho",         do_recho,      	POS_DEAD,        IM, LOG_NORMAL, 0 },
    { "redit",		   do_redit,	    POS_DEAD,    	 IM, LOG_NORMAL, 0 },
    { "rename",        do_rename,     	POS_DEAD,        IM, LOG_ALWAYS, 0 },
    { "resets",		   do_resets,	    POS_DEAD,    	 IM, LOG_NORMAL, 0 },
    { "restore",       do_restore,    	POS_DEAD,        IM, LOG_NORMAL, 0 },
    { "return",        do_return,     	POS_DEAD,        IM, LOG_NORMAL, 0 },
    { "rsearch",       do_rsearch,      POS_DEAD,        IM, LOG_NORMAL, 0 },
    { "scatter",       do_scatter,      POS_DEAD,	     IM, LOG_NORMAL, 0 },
    { "shatter",       do_shatter,      POS_DEAD, 	     IM, LOG_NORMAL, 0 },
    { "skilltable",    do_skilltable,   POS_DEAD,        IM, LOG_NORMAL, 0 },
    { "slay",		   do_slay,	        POS_DEAD,    	 IM, LOG_NORMAL, 0 },
    { "slookup",       do_slookup,    	POS_DEAD,        IM, LOG_NEVER,  1 },
    { "snoop",         do_snoop,      	POS_DEAD,        IM, LOG_ALWAYS, 0 },
    { "sstat",         do_sstat,        POS_DEAD,        IM, LOG_NORMAL, 0 },
    { "startdp",	   do_startdp,	    POS_DEAD,        IM, LOG_ALWAYS, 0 },
    { "string",        do_string,     	POS_DEAD,        IM, LOG_ALWAYS, 0 },
    { "switch",        do_switch,     	POS_DEAD,        IM, LOG_NORMAL, 0 },
    { "transfer",      do_transfer,   	POS_DEAD,        IM, LOG_NORMAL, 0 },
    { "violate",       do_violate,    	POS_DEAD,        IM, LOG_ALWAYS, 0 },
    { "vnumlist",      do_vnumlist,   	POS_DEAD,        IM, LOG_NORMAL, 0 },
    { "wizinvis",      do_invis,   	    POS_DEAD,        IM, LOG_NORMAL, 0 },
    { "wizhelp",       do_wizhelp,      POS_DEAD,        IM, LOG_NORMAL, 1 },
    { "wizindex",      do_wizindex,     POS_DEAD,        IM, LOG_NORMAL, 1 },
    { ":",		       do_immtalk,	    POS_DEAD,	     IM, LOG_NORMAL, 0 },
     
    { "",	           0,		        POS_DEAD,	     0,  LOG_NORMAL, 0 }
};




/*
 * The main entry point for executing commands.
 * Can be recursively called from 'at', 'order', 'force'.
 */
void interpret( CHAR_DATA *ch, char *argument )
{
    char command[MAX_INPUT_LENGTH];
    char logline[MAX_INPUT_LENGTH];
    char  buf[MAX_STRING_LENGTH];
    int cmd;
    int level;
    bool found;

    /*
     * Strip leading spaces.
     */
    while ( isspace(*argument) )
	argument++;
    if ( argument[0] == '\0' )
	return;

    /*
     * Implement freeze command.
     */
    if ( !IS_NPC(ch) && IS_SET(ch->pact, PLR_FREEZE) )
    {
	send_to_char( "You're totally frozen!\n\r", ch );
	return;
    }

    /*
     * Grab the command word.
     * Special parsing so ' can be a command,
     *   also no spaces needed after punctuation.
     */

    strcpy( logline, argument );
    strcpy( buf, argument);
    sprintf(last_command,"%s in room[%d]: %s.",ch->name,ch->in_room->vnum,buf);

    if ( !isalpha(argument[0]) && !isdigit(argument[0]) )
    {
	command[0] = argument[0];
	command[1] = '\0';
	argument++;
	while ( isspace(*argument) )
	    argument++;
    }
    else
    {
	argument = one_argument( argument, command );
    }

    /*
     * Look for command in command table.
     */
    found = FALSE;

     level = ch->level;

    for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
    {
	if ( command[0] == cmd_table[cmd].name[0]
	&&   !str_prefix( command, cmd_table[cmd].name ))
	{
	   if(cmd_table[cmd].level <= level || granted(ch, cmd_table[cmd].name))    // Grant System
	   {
	      found = TRUE;
	      break;
	   }
	}
    }

if ( !IS_NPC(ch) && IS_SET(ch->pact, PLR_JAIL) )
         {
                  if ( str_cmp (cmd_table[cmd].name, "say") &&
                                 str_cmp (cmd_table[cmd].name, "'")   &&
                                 str_cmp (cmd_table[cmd].name, "clan"))
                  {
                    send_to_char( "\n\r{RYou can't do that in jail!{x\n\r", ch );
                   return;
                  }
         }


    /*
     * Log and snoop.
     */
    if ( cmd_table[cmd].log == LOG_NEVER )
	strcpy( logline, "" );


     if ((!IS_NPC(ch) 
     && IS_SET(ch->pact, PLR_LOG) )
     || cmd_table[cmd].log == LOG_ALWAYS )
       {
        sprintf( log_buf, "Log %s: %s", ch->name, logline );
        wiznet(log_buf,ch,NULL,WIZ_SECURE,0,ch->level);
        log_string( log_buf );
       }

    if ( ch->desc != NULL && ch->desc->snoop_by != NULL )
    {
	write_to_buffer( ch->desc->snoop_by, "\n\r",  2 );
	write_to_buffer( ch->desc->snoop_by, "% ",    2 );
	write_to_buffer( ch->desc->snoop_by, logline, 0 );
	write_to_buffer( ch->desc->snoop_by, "\n\r\n\r",  2 );
    }

    if ( !found )
       {
	if (!check_social( ch, command, argument ) )
	send_to_char( "\n\r{cHuh?\n\r", ch );
	return;
        }
     else /* a normal valid command.. check if it is disabled */
     	if (check_disabled (&cmd_table[cmd]))
     	{
     		send_to_char ("\n\r{RThis command has been temporarily disabled!{x\n\r",ch);
     		return;
     	}

    /*
     * Character not in position for command?
     */
    if ( ch->position < cmd_table[cmd].position )
    {
	switch( ch->position )
	{
	case POS_DEAD:
	    send_to_char( "Lie still; you are DEAD.\n\r", ch );
	    break;

	case POS_MORTAL:
	case POS_INCAP:
	    send_to_char( "You are hurt far too bad for that.\n\r", ch );
	    break;

	case POS_STUNNED:
	    send_to_char( "You are too stunned to do that.\n\r", ch );
	    break;

	case POS_SLEEPING:
	    send_to_char( "In your dreams, or what?\n\r", ch );
	    break;

	case POS_RESTING:
	    send_to_char( "Nah... You feel too relaxed...\n\r", ch);
	    break;

	case POS_SITTING:
	    send_to_char( "Better stand up first.\n\r",ch);
	    break;

	case POS_FIGHTING:
	    send_to_char( "No way!  You are still fighting!\n\r", ch);
	    break;

	}
	return;
    }

    /*
     * Dispatch the command.
     */
    (*cmd_table[cmd].do_fun) ( ch, argument );

    tail_chain( );
    return;
}

/* function to keep argument safe in all commands -- no static strings */
void do_function (CHAR_DATA *ch, DO_FUN *do_fun, char *argument)
{
    char *command_string;
    
    /* copy the string */
    command_string = str_dup(argument);
    
    /* dispatch the command */
    (*do_fun) (ch, command_string);
    
    /* free the string */
    free_string(command_string);
}
    
bool check_social( CHAR_DATA *ch, char *command, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *rch;
    int cmd;
    bool found;

    found  = FALSE;

    for ( cmd = 0; social_table[cmd].name[0] != '\0'; cmd++ )
    {
	if ( command[0] == social_table[cmd].name[0]
	&&   !str_prefix( command, social_table[cmd].name ) )
	{
	    found = TRUE;
	    break;
	}
    }

    if ( !found )
	return FALSE;

    if ( !IS_NPC(ch) && IS_SET(ch->comm, COMM_NOEMOTE) )
    {
	send_to_char( "\n\r{GYou are anti-social!{x\n\r", ch );
	return TRUE;
    }

    switch ( ch->position )
    {
    case POS_DEAD:
	send_to_char( "\n\r{RLie still; you are DEAD.{x\n\r", ch );
	return TRUE;

    case POS_INCAP:
    case POS_MORTAL:
	send_to_char( "\n\r{CYou are hurt far too bad for that.{x\n\r", ch );
	return TRUE;

    case POS_STUNNED:
	send_to_char( "\n\r{rYou are too stunned to do that.{x\n\r", ch );
	return TRUE;

    case POS_SLEEPING:
	/*
	 * I just know this is the path to a 12" 'if' statement.  :(
	 * But two players asked for it already!  -- Furey
	 */
	if ( !str_cmp( social_table[cmd].name, "snore" ) )
	    break;
	send_to_char( "\n\r{WIn your dreams, or what?{x\n\r", ch );
	return TRUE;

    }

    one_argument( argument, arg );
    victim = NULL;
       for (rch=ch->in_room->people;rch;rch=rch->next_in_room)
          {
           send_to_char("\n\r",rch);
          }

     

    if ( arg[0] == '\0' )
    {	
	act( social_table[cmd].others_no_arg, ch, NULL, victim, TO_ROOM    );
	act( social_table[cmd].char_no_arg,   ch, NULL, victim, TO_CHAR    );
    }
    else if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "\n\r{RThey aren't here.{x\n\r", ch );
    }
    else if ( victim == ch )
    {
	act( social_table[cmd].others_auto,   ch, NULL, victim, TO_ROOM    );
	act( social_table[cmd].char_auto,     ch, NULL, victim, TO_CHAR    );
    }
    else
    {
	act( social_table[cmd].others_found,  ch, NULL, victim, TO_NOTVICT );
	act( social_table[cmd].char_found,    ch, NULL, victim, TO_CHAR    );
	act( social_table[cmd].vict_found,    ch, NULL, victim, TO_VICT    );

	if ( !IS_NPC(ch) && IS_NPC(victim)
	&&   !IS_AFFECTED(victim, AFF_CHARM)
	&&   IS_AWAKE(victim) 
	&&   victim->desc == NULL)
	{
	    switch ( number_bits( 4 ) )
	    {
	    case 0:

	    case 1: case 2: case 3: case 4:
	    case 5: case 6: case 7: case 8:
		act( social_table[cmd].others_found,victim, NULL, ch, TO_NOTVICT );
		act( social_table[cmd].char_found,victim, NULL, ch, TO_CHAR    );
		act( social_table[cmd].vict_found,victim, NULL, ch, TO_VICT    );
		break;

	    case 9: case 10: case 11: case 12:
		act( "\n\r{C$n slaps $N.{x\n\r",  victim, NULL, ch, TO_NOTVICT );
		act( "\n\r{MYou slap $N.{x\n\r",  victim, NULL, ch, TO_CHAR    );
		act( "\n\r{R$n slaps you.{x\n\r", victim, NULL, ch, TO_VICT    );
		break;
	    }
	}
    }

    return TRUE;
}



/*
 * Return true if an argument is completely numeric.
 */
bool is_number ( char *arg )
{
 
    if ( *arg == '\0' )
        return FALSE;
 
    if ( *arg == '+' || *arg == '-' )
        arg++;
 
    for ( ; *arg != '\0'; arg++ )
    {
        if ( !isdigit( *arg ) )
            return FALSE;
    }
 
    return TRUE;
}



/*
 * Given a string like 14.foo, return 14 and 'foo'
 */
int number_argument( char *argument, char *arg )
{
    char *pdot;
    int number;
    
    for ( pdot = argument; *pdot != '\0'; pdot++ )
    {
	if ( *pdot == '.' )
	{
	    *pdot = '\0';
	    number = atoi( argument );
	    *pdot = '.';
	    strcpy( arg, pdot+1 );
	    return number;
	}
    }

    strcpy( arg, argument );
    return 1;
}

/* 
 * Given a string like 14*foo, return 14 and 'foo'
*/
int mult_argument(char *argument, char *arg)
{
    char *pdot;
    int number;

    for ( pdot = argument; *pdot != '\0'; pdot++ )
    {
        if ( *pdot == '*' )
        {
            *pdot = '\0';
            number = atoi( argument );
            *pdot = '*';
            strcpy( arg, pdot+1 );
            return number;
        }
    }
 
    strcpy( arg, argument );
    return 1;
}



/*
 * Pick off one argument from a string and return the rest.
 */
char *one_argument( char *argument, char *arg_first )
{
    char cEnd;

    while ( isspace(*argument) )
	argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"' )
	cEnd = *argument++;

    while ( *argument != '\0' )
    {
	if ( *argument == cEnd )
	{
	    argument++;
	    break;
	}
	*arg_first = LOWER(*argument);
	arg_first++;
	argument++;
    }
    *arg_first = '\0';

    while ( isspace(*argument) )
	argument++;

    return argument;
}

/*
 * Contributed by Alander.
 */



void do_commands( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    int cmd;
         int col;
 
    col = 0;

    send_to_char("\n\r",ch);

    for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
    {
        if ( cmd_table[cmd].level <  LEVEL_HERO
        && ( cmd_table[cmd].level <= ch->level) 
        &&   cmd_table[cmd].show)
        {
            sprintf( buf, "{m[{W%-11.11s{m]", cmd_table[cmd].name );
            send_to_char( buf, ch );
            if ( ++col % 6 == 0 )
                send_to_char( "{x\n\r", ch );
        }

    }
        
     if ( col % 6 != 0 )
      send_to_char( "{x\n\r", ch );
}
