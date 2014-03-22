#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <time.h>
#include "merc.h"
#include "tables.h"



/* for position */
const struct position_type position_table[] =
{
    {	"dead",			"dead"	},
    {	"mortally wounded",	"mort"	},
    {	"incapacitated",	"incap"	},
    {	"stunned",		"stun"	},
    {	"sleeping",		"sleep"	},
    {	"resting",		"rest"	},
    {   "sitting",		"sit"   },
    {	"fighting",		"fight"	},
    {	"standing",		"stand"	},
    {	NULL,			NULL	}
};

/* for sex */
const struct sex_type sex_table[] =
{
   {	"none"		},
   {	"male"		},
   {	"female"	},
   {	"either"	},
   {	NULL		}
};

/* for sizes */
const struct size_type size_table[] =
{ 
    {	"tiny"		},
    {	"small" 	},
    {	"medium"	},
    {	"large"		},
    {	"huge", 	},
    {	"giant" 	},
    {	NULL		}
};

/* various flag tables */
const struct flag_type act_flags[] =
{
    {	"NPC",			A,	FALSE	},
    {	"Sentinel",		B,	TRUE	},
    {	"Scavenger",		C,	TRUE	},
    {	"Bank",			D,	TRUE	},
    {   "Forge",		E,	TRUE	},
    {	"Aggr",			F,	TRUE	},
    {	"Stay",			G,	TRUE	},
    {	"Wimp",			H,	TRUE	},
    {	"Pet",			I,	TRUE	},
    {	"Train",		J,	TRUE	},
    {	"Prac",			K,	TRUE	},
    {	"Paladin",		L,	TRUE	},
    {	"Assassin",		M,	TRUE	},
    {	"Anti-Pal",		N,	TRUE	},
    {	"Undead",		O,	TRUE	},
    {	"Psion",		P,	TRUE	},
    {	"Cleric",		Q,	TRUE	},
    {	"Mage",			R,	TRUE	},
    {	"Thief",		S,	TRUE	},
    {	"Warrior",		T,	TRUE	},
    {	"NoAlign",		U,	TRUE	},
    {	"NoPurge",		V,	TRUE	},
    {	"QMaster",		X,	TRUE	},
    {	"NoBingo",		Z,	TRUE	},
    {	"Healer",		aa,	TRUE	},
    {	"Gain",			bb,	TRUE	},
    {	"NoSpecAttk",		cc,	TRUE	},
    {	"Changer",		dd,	TRUE	},
    {	"Ranger",		ee,	TRUE	},
    {	NULL,			0,	FALSE	}
};

const struct flag_type plr_flags[] =
{
    {	"PC",			A,	FALSE	},
    {	"Quest",		B,	TRUE	},
    {	"AAssist",		C,	FALSE	},
    {	"AExit",		D,	FALSE	},
    {	"ALoot",		E,	FALSE	},
    {	"ASac",			F,	FALSE	},
    {	"AGold",		G,	FALSE	},
    {	"ASplit",		H,	FALSE	},
    {	"Target",		J,	FALSE	},
    {	"DragonPIT",		K,	FALSE	},
    {	"Loot",			P,	FALSE	},
    {	"NoSumm",		Q,	FALSE	},
    {	"NoFol",		R,	FALSE	},
    {	"Permit",		U,	TRUE	},
    {	"Jail",			V,	FALSE	},
    {	"Log",			W,	FALSE	},
    {	"Deny",			X,	FALSE	},
    {	"Froze",		Y,	FALSE	},
    {	"Thief",		Z,	FALSE	},
    {	"Killer",		aa,	FALSE	},
    {	"Remort",		bb,	FALSE	},
    {	"PKiller",		cc,	FALSE	},
    {	"CLeader",		ee,	FALSE	},
    {	NULL,			0,	0	}
};

const struct flag_type affect_flags[] =
{
    {	"Blind",		A,	TRUE	},
    {	"Invis",		B,	TRUE	},
    {	"DetEvil",		C,	TRUE	},
    {	"DetInvis",		D,	TRUE	},
    {	"DetMagic",		E,	TRUE	},
    {	"DetHidden",		F,	TRUE	},
    {	"DetGood",		G,	TRUE	},
    {	"SANC",			H,	TRUE	},
    {	"FaeFire",		I,	TRUE	},
    {	"Infra",		J,	TRUE	},
    {	"Curse",		K,	TRUE	},
    {	"Poison",		M,	TRUE	},
    {	"ProEvil",		N,	TRUE	},
    {	"ProGood",		O,	TRUE	},
    {	"Sneak",		P,	TRUE	},
    {	"Hide",			Q,	TRUE	},
    {	"Sleep",		R,	TRUE	},
    {	"Charm",		S,	TRUE	},
    {	"Fly",			T,	TRUE	},
    {	"PDoor",		U,	TRUE	},
    {	"Haste",		V,	TRUE	},
    {	"Calm",			W,	TRUE	},
    {	"Plague",		X,	TRUE	},
    {	"Weak",			Y,	TRUE	},
    {	"DVision",		Z,	TRUE	},
    {	"BSerk",		aa,	TRUE	},
    {	"Swim",			bb,	TRUE	},
    {	"Regen",		cc,	TRUE	},
    {	"Slow",			dd,	TRUE	},
    {   "GolAURA", 	         ee,     TRUE    }, 
    {	NULL,			0,	0	}
};

const struct flag_type affect2_flags[] =
{
    {	"Divine",		A,	TRUE	},
    {	NULL,			0,	0	}
};

const struct flag_type off_flags[] =
{
    {	"AreaAttk",		A,	TRUE	},
    {	"BStab",		B,	TRUE	},
    {	"Bash",			C,	TRUE	},
    {	"BSerk",		D,	TRUE	},
    {	"Disarm",		E,	TRUE	},
    {	"Dodge",		F,	TRUE	},
    {	"Fade",			G,	TRUE	},
    {	"Fast",			H,	TRUE	},
    {	"Kick",			I,	TRUE	},
    {	"KDirt",		J,	TRUE	},
    {	"Parry",		K,	TRUE	},
    {	"Rescue",		L,	TRUE	},
    {	"Tail",			M,	TRUE	},
    {	"Trip",			N,	TRUE	},
    {	"Crush",		O,	TRUE	},
    {	"AsstAll",		P,	TRUE	},
    {	"AsstAlign",		Q,	TRUE	},
    {	"AsstRace",		R,	TRUE	},
    {	"AsstPC",		S,	TRUE	},
    {	"AsstGuard",		T,	TRUE	},
    {	"AsstVnum",		U,	TRUE	},
    {	NULL,			0,	0	}
};

const struct flag_type imm_flags[] =
{
    {	"Summ",			A,	TRUE	},
    {	"Charm",		B,	TRUE	},
    {	"Magic",		C,	TRUE	},
    {	"Weap",			D,	TRUE	},
    {	"Bash",			E,	TRUE	},
    {	"Pierce",		F,	TRUE	},
    {	"Slash",		G,	TRUE	},
    {	"Fire",			H,	TRUE	},
    {	"Cold",			I,	TRUE	},
    {	"Lghtng",		J,	TRUE	},
    {	"Acid",			K,	TRUE	},
    {	"Poison",		L,	TRUE	},
    {	"Nega",			M,	TRUE	},
    {	"Holy",			N,	TRUE	},
    {	"Energy",		O,	TRUE	},
    {	"Mental",		P,	TRUE	},
    {	"Disease",		Q,	TRUE	},
    {	"Drown",		R,	TRUE	},
    {	"Light2",		S,	TRUE	},
    {	"Sound",		T,	TRUE	},
    {	"Elemental",		U,	TRUE	},
    {	"Claw", 		V,	TRUE	},
    {	"Wood",			X,	TRUE	},
    {	"Silver",		Y,	TRUE	},
    {	"Iron",			Z,	TRUE	},
    {	"Maim",			aa,	TRUE	},
    {	"Crush",		bb,	TRUE	},
    {	"Cleave",		cc,	TRUE	},
    {	"Punch",		dd,	TRUE	},
    {	"Bite",	  	 	ee,	TRUE	},
    {	NULL,			0,	0	}
};

const struct flag_type res_flags[] =
{
    {	"Summ",			A,	TRUE	},
    {	"Charm",		B,	TRUE	},
    {	"Magic",		C,	TRUE	},
    {	"Weap",			D,	TRUE	},
    {	"Bash",			E,	TRUE	},
    {	"Pierce",		F,	TRUE	},
    {	"Slash",		G,	TRUE	},
    {	"Fire",			H,	TRUE	},
    {	"Cold",			I,	TRUE	},
    {	"Lghtng",		J,	TRUE	},
    {	"Acid",			K,	TRUE	},
    {	"Poison",		L,	TRUE	},
    {	"Nega",			M,	TRUE	},
    {	"Holy",			N,	TRUE	},
    {	"Energy",		O,	TRUE	},
    {	"Mental",		P,	TRUE	},
    {	"Disease",		Q,	TRUE	},
    {	"Drown",		R,	TRUE	},
    {	"Light2",		S,	TRUE	},
    {	"Sound",		T,	TRUE	},
    {	"Elemental",		U,	TRUE	},
    {	"Claw", 		V,	TRUE	},
    {	"Wood",			X,	TRUE	},
    {	"Silver",		Y,	TRUE	},
    {	"Iron",			Z,	TRUE	},
    {	"Maim",			aa,	TRUE	},
    {	"Crush",		bb,	TRUE	},
    {	"Cleave",		cc,	TRUE	},
    {	"Punch",		dd,	TRUE	},
    {	"Bite",	  	 	ee,	TRUE	},
    {	NULL,			0,	0	}
};

const struct flag_type vuln_flags[] =
{
    {	"Summ",			A,	TRUE	},
    {	"Charm",		B,	TRUE	},
    {	"Magic",		C,	TRUE	},
    {	"Weap",			D,	TRUE	},
    {	"Bash",			E,	TRUE	},
    {	"Pierce",		F,	TRUE	},
    {	"Slash",		G,	TRUE	},
    {	"Fire",			H,	TRUE	},
    {	"Cold",			I,	TRUE	},
    {	"Lghtng",		J,	TRUE	},
    {	"Acid",			K,	TRUE	},
    {	"Poison",		L,	TRUE	},
    {	"Nega",			M,	TRUE	},
    {	"Holy",			N,	TRUE	},
    {	"Energy",		O,	TRUE	},
    {	"Mental",		P,	TRUE	},
    {	"Disease",		Q,	TRUE	},
    {	"Drown",		R,	TRUE	},
    {	"Light2",		S,	TRUE	},
    {	"Sound",		T,	TRUE	},
    {	"Elemental",		U,	TRUE	},
    {	"Claw", 		V,	TRUE	},
    {	"Wood",			X,	TRUE	},
    {	"Silver",		Y,	TRUE	},
    {	"Iron",			Z,	TRUE	},
    {	"Maim",			aa,	TRUE	},
    {	"Crush",		bb,	TRUE	},
    {	"Cleave",		cc,	TRUE	},
    {	"Punch",		dd,	TRUE	},
    {	"Bite",	  	 	ee,	TRUE	},
    {	NULL,			0,	0	}
};


const struct flag_type form_flags[] =
{
    {	"edible",		FORM_EDIBLE,		TRUE	},
    {	"poison",		FORM_POISON,		TRUE	},
    {	"magical",		FORM_MAGICAL,		TRUE	},
    {	"instant_rot",		FORM_INSTANT_DECAY,	TRUE	},
    {	"other",		FORM_OTHER,		TRUE	},
    {	"animal",		FORM_ANIMAL,		TRUE	},
    {	"sentient",		FORM_SENTIENT,		TRUE	},
    {	"undead",		FORM_UNDEAD,		TRUE	},
    {	"construct",		FORM_CONSTRUCT,		TRUE	},
    {	"mist",			FORM_MIST,		TRUE	},
    {	"intangible",		FORM_INTANGIBLE,	TRUE	},
    {	"biped",		FORM_BIPED,		TRUE	},
    {	"centaur",		FORM_CENTAUR,		TRUE	},
    {	"insect",		FORM_INSECT,		TRUE	},
    {	"spider",		FORM_SPIDER,		TRUE	},
    {	"crustacean",		FORM_CRUSTACEAN,	TRUE	},
    {	"worm",			FORM_WORM,		TRUE	},
    {	"blob",			FORM_BLOB,		TRUE	},
    {	"mammal",		FORM_MAMMAL,		TRUE	},
    {	"bird",			FORM_BIRD,		TRUE	},
    {	"reptile",		FORM_REPTILE,		TRUE	},
    {	"snake",		FORM_SNAKE,		TRUE	},
    {	"dragon",		FORM_DRAGON,		TRUE	},
    {	"amphibian",		FORM_AMPHIBIAN,		TRUE	},
    {	"fish",			FORM_FISH ,		TRUE	},
    {	"cold_blood",		FORM_COLD_BLOOD,	TRUE	},
    {	NULL,			0,			0	}
};

const struct flag_type part_flags[] =
{
    {	"head",			PART_HEAD,		TRUE	},
    {	"arms",			PART_ARMS,		TRUE	},
    {	"legs",			PART_LEGS,		TRUE	},
    {	"heart",		PART_HEART,		TRUE	},
    {	"brains",		PART_BRAINS,		TRUE	},
    {	"guts",			PART_GUTS,		TRUE	},
    {	"hands",		PART_HANDS,		TRUE	},
    {	"feet",			PART_FEET,		TRUE	},
    {	"fingers",		PART_FINGERS,		TRUE	},
    {	"ear",			PART_EAR,		TRUE	},
    {	"eye",			PART_EYE,		TRUE	},
    {	"long_tongue",		PART_LONG_TONGUE,	TRUE	},
    {	"eyestalks",		PART_EYESTALKS,		TRUE	},
    {	"tentacles",		PART_TENTACLES,		TRUE	},
    {	"fins",			PART_FINS,		TRUE	},
    {	"wings",		PART_WINGS,		TRUE	},
    {	"tail",			PART_TAIL,		TRUE	},
    {	"claws",		PART_CLAWS,		TRUE	},
    {	"fangs",		PART_FANGS,		TRUE	},
    {	"horns",		PART_HORNS,		TRUE	},
    {	"scales",		PART_SCALES,		TRUE	},
    {	"tusks",		PART_TUSKS,		TRUE	},
    {	NULL,			0,			0	}
};

const struct flag_type comm_flags[] =
{
    {	"Quiet",		COMM_QUIET,		TRUE	},
    {   "Deaf",			COMM_DEAF,		TRUE	},
    {   "NoWiz",		COMM_NOWIZ,		TRUE	},
    {   "NoAuct",		COMM_NOAUCTION,		TRUE	},
    {   "NODPit",		COMM_DPITKILL,		TRUE	},
    {   "NoPNote",		COMM_NOPNOTE,		TRUE	},
    {   "Combat",		COMM_COMBAT,		TRUE	},
    {   "Helper",		COMM_HELPER,		TRUE	},
    {   "NoEmote",		COMM_NOEMOTE,		TRUE	},
    {   "NoTell",		COMM_NOTELL,		TRUE	},
    {   "NoChan",		COMM_NOCHANNELS,	TRUE	},
    {   "SnoopPr",		COMM_SNOOP_PROOF,	TRUE	},
    {   "AFK",			COMM_AFK,		TRUE	},
    {   "NoTitle",		COMM_NOTITLE,		TRUE	},
    {	NULL,			0,			0	}
};

const struct flag_type mprog_flags[] =
{
    {	"act",			TRIG_ACT,		TRUE	},
    {	"bribe",		TRIG_BRIBE,		TRUE 	},
    {	"death",		TRIG_DEATH,		TRUE    },
    {	"entry",		TRIG_ENTRY,		TRUE	},
    {	"fight",		TRIG_FIGHT,		TRUE	},
    {	"give",			TRIG_GIVE,		TRUE	},
    {	"greet",		TRIG_GREET,		TRUE    },
    {	"grall",		TRIG_GRALL,		TRUE	},
    {	"kill",			TRIG_KILL,		TRUE	},
    {	"hpcnt",		TRIG_HPCNT,		TRUE    },
    {	"random",		TRIG_RANDOM,		TRUE	},
    {	"speech",		TRIG_SPEECH,		TRUE	},
    {	"exit",			TRIG_EXIT,		TRUE    },
    {	"exall",		TRIG_EXALL,		TRUE    },
    {	"delay",		TRIG_DELAY,		TRUE    },
    {	"surr",			TRIG_SURR,		TRUE    },
    {	NULL,			0,			TRUE	}
};

const struct flag_type area_flags[] =
{
    {	"CHANGED",		AREA_CHANGED,		TRUE	},
    {	"ADDED",		AREA_ADDED,		TRUE	},
    {	"loading",		AREA_LOADING,		FALSE	},
    {	"envy",			AREA_ENVY,		FALSE	},
    {   "PROTO",		AREA_PROTO, 		TRUE 	},
    {   "PLAYER",    		AREA_PLAYER, 		TRUE 	},
    {   "GODS",    		AREA_GODS, 		TRUE 	},
    {   "NOTRANS",    		AREA_NOTRANS, 		TRUE 	},
    {   "NOIMM",	    	AREA_NOIMM, 		TRUE 	},
    {   "IMP",		    	AREA_IMP, 		FALSE 	},
/*
    {   "no_quest",     AREA_NO_QUEST, TRUE },
*/  
  {	NULL,			0, 0 }
};



const struct flag_type sex_flags[] =
{
    {	"male",			SEX_MALE,		TRUE	},
    {	"female",		SEX_FEMALE,		TRUE	},
    {	"neutral",		SEX_NEUTRAL,		TRUE	},
    {   "random",               3,                      TRUE    },   /* ROM */
    {	"none",			SEX_NEUTRAL,		TRUE	},
    {	NULL,			0,			0	}
};



const struct flag_type exit_flags[] =
{
    {   "door",			EX_ISDOOR,		TRUE    },
    {	"closed",		EX_CLOSED,		TRUE	},
    {	"locked",		EX_LOCKED,		TRUE	},
    {	"pickproof",		EX_PICKPROOF,		TRUE	},
    {   "nopass",		EX_NOPASS,		TRUE	},
    {   "easy",			EX_EASY,		TRUE	},
    {   "hard",			EX_HARD,		TRUE	},
    {	"infuriating",		EX_INFURIATING,		TRUE	},
    {	"noclose",		EX_NOCLOSE,		TRUE	},
    {	"nolock",		EX_NOLOCK,		TRUE	},
    {	NULL,			0,			0	}
};



const struct flag_type door_resets[] =
{
    {	"open and unlocked",	0,		TRUE	},
    {	"closed and unlocked",	1,		TRUE	},
    {	"closed and locked",	2,		TRUE	},
    {	NULL,			0,		0	}
};



const struct flag_type room_flags[] =
{
    {	"dark",			ROOM_DARK,		TRUE	},
    {	"no_mob",		ROOM_NO_MOB,		TRUE	},
    {	"indoors",		ROOM_INDOORS,		TRUE	},
    {	"private",		ROOM_PRIVATE,		TRUE    },
    {	"deathtrap",		ROOM_DEATHTRAP,		TRUE    },
    {	"see_invis",		ROOM_SEE_INVIS,		TRUE    },
    {   "legends_only",           ROOM_LEGENDS_ONLY,  TRUE },
    {	"safe",			ROOM_SAFE,		TRUE	},
    {	"solitary",		ROOM_SOLITARY,		TRUE	},
    {	"pet_shop",		ROOM_PET_SHOP,		TRUE	},
    {	"no_recall",		ROOM_NO_RECALL,		TRUE	},
    {	"imp_only",		ROOM_IMP_ONLY,		TRUE    },
    {	"gods_only",	        ROOM_GODS_ONLY,		TRUE    },
    {	"heroes_only",		ROOM_HEROES_ONLY,	TRUE	},
    {	"newbies_only",		ROOM_NEWBIES_ONLY,	TRUE	},
    {	"law",			ROOM_LAW,		TRUE	},
    {   "nowhere",		ROOM_NOWHERE,		TRUE	},
    {   "noloot",		ROOM_NOLOOT,		TRUE	},
    {   "DragonPIT",            ROOM_DRAGONPIT,         TRUE    },
    {   "mud_school",           ROOM_MUD_SCHOOL,        TRUE    },
	{	"gauntlet",		ROOM_GAUNTLET,	TRUE	},
    {	NULL,			0,			0	}
};


const struct flag_type racechan_flags[] =
{
    { "elven", RACECHAN_ELVEN, TRUE },
    { "demonic", RACECHAN_DEMONIC, TRUE },
    { "dwarven", RACECHAN_DWARVEN, TRUE },
    { "draconian", RACECHAN_DRACONIAN, TRUE },
    { "reptilian", RACECHAN_REPTILIAN, TRUE },
    { "kender", RACECHAN_KENDER, TRUE },
    { "drow", RACECHAN_DROW, TRUE },
    { "magespeak", RACECHAN_MAGESPEAK, TRUE },
    { "highelven", RACECHAN_HIGH_ELVEN, TRUE },
    { "undead", RACECHAN_UNDEAD, TRUE },
    { "thieves", RACECHAN_THIEVES_CANT, TRUE },
    { "handtalk", RACECHAN_HANDTALK, TRUE },
    { "forestsign", RACECHAN_FORESTSIGN, TRUE },
    { "warchant", RACECHAN_WAR_CHANT, TRUE },
    {	NULL,		0,			0	}
};


const struct flag_type sector_flags[] =
{
    {	"inside",	SECT_INSIDE,		TRUE	},
    {	"city",		SECT_CITY,		TRUE	},
    {	"field",	SECT_FIELD,		TRUE	},
    {	"forest",	SECT_FOREST,		TRUE	},
    {	"hills",	SECT_HILLS,		TRUE	},
    {	"mountain",	SECT_MOUNTAIN,		TRUE	},
    {	"swim",		SECT_WATER_SWIM,	TRUE	},
    {	"noswim",	SECT_WATER_NOSWIM,	TRUE	},
    {   "unused",	SECT_UNUSED,		TRUE	},
    {	"air",		SECT_AIR,		TRUE	},
    {	"desert",	SECT_DESERT,		TRUE	},
    {	"underground",	SECT_UNDERGROUND,	TRUE	},
    {	"road",	 	SECT_ROAD,		TRUE	},
    {	NULL,		0,			0	}
};


const struct flag_type material_flags[] =
{
{"SELECT_A_MATERIAL",	MATERIAL_NONE,	TRUE},
{"ORICHALCUM",  METORICHALCUM,  TRUE},
{"LAER_NIOB",  	METNIOBLAER,   	TRUE},
{"DRAK_NIOB",  	METNIOBDRAK,   	TRUE},
{"NIOBIUM",    	METNIOBIUM,  	TRUE},
{"DELF_ADAM",  	METADAMDELF,   	TRUE},
{"ADAMANTIUM",  METADAMANT,     TRUE},
{"DWRF_MITH",  	METMITHDWAR,  	TRUE},
{"MITHRIL",    	METMITHRIL,   	TRUE},
{"ELF_STEEL",   	METSTLELF,    	TRUE},
{"HIGH_STEEL",  	METSTLHIGH,   	TRUE},
{"STEEL",      	METSTEEL,	TRUE},
{"IRON",       	METIRON,	TRUE},
{"BRONZE",     	METBRONZE,	TRUE},
{"BRASS",      	METBRASS,	TRUE},
{"COPPER",     	METCOPPER,	TRUE},
{"PLATINUM",   	METPLATINUM,	TRUE},
{"GOLD",       	METGOLD,	TRUE},
{"SILVER",     	METSILVER,	TRUE},
{"MET_GENERIC",	METGENERIC,	TRUE},
{"SOULSTONE",  	GEMSOLSTN,	TRUE},
{"BLACK_DIAMOND",	GEMBLKDIA,	TRUE},
{"BLACK_OPAL",   	GEMBLKOPAL,	TRUE},
{"BLACK_SAPPHIRE",	GEMBLKSAPP,	TRUE},
{"STAR_RUBY",  	GEMSTRRUBY,	TRUE},
{"STAR_SAPPHIRE",	GEMSTRSAPP,	TRUE},
{"FIRE_OPAL",  	GEMFIREOPAL,	TRUE},
{"STARSTONE",  	GEMSTRSTN,	TRUE},
{"DIAMOND",    	GEMDIAMOND,	TRUE},
{"RUBY",       	GEMRUBY,	TRUE},
{"EMERALD",    	GEMEMERALD,	TRUE},
{"SAPPHIRE",   	GEMSAPPHIRE,	TRUE},
{"OPAL",       	GEMOPAL,	TRUE},
{"AQUAMARINE", 	GEMAQUAMAR,	TRUE},
{"AMETHYST",   	GEMAMETHYST,	TRUE},
{"JADE",       	GEMJADE,	TRUE},
{"PEARL",      	GEMPEARL,	TRUE},
{"AMBER",      	GEMAMBER,	TRUE},
{"JET",        	GEMJET,	TRUE},
{"TOPAZ",      	GEMTOPAZ,	TRUE},
{"JACINTH",    	GEMJACINTH,	TRUE},
{"MOONSTONE",  	GEMMOONSTN,	TRUE},
{"ONYX",       	GEMONYX,	TRUE},
{"JASPER",     	GEMJASPER,	TRUE},
{"ZIRCON",     	GEMZIRCON,	TRUE},
{"STAR_ROSE",  	GEMSTRROSE,	TRUE},
{"GEM_GENERIC",	GEMGENERIC,	TRUE},
{"DIVINE_ELEM", 	ELEDIVINE,	TRUE},
{"STELLAR_ELEM", ELESTELLAR,	TRUE},
{"LIGHT_ELEM",  	ELELIGHT,	TRUE},
{"DARK_ELEM",   	ELEDARK,	TRUE},
{"FIRE_ELEM",   	ELEFIRE,	TRUE},
{"WATER_ELEM",  	ELEWATER,	TRUE},
{"EARTH_ELEM",  	ELEEARTH,	TRUE},
{"AIR_ELEM",   	ELEAIR,	TRUE},
{"LIGHTNING_ELEM",	ELELIGHTING,	TRUE},
{"SPIDER_SILK",	CLOSPIDSILK,	TRUE},
{"ELVEN_SILK", 	CLOELFSILK,	TRUE},
{"SILK",       	CLOSILK,	TRUE},
{"HIGH_CLOTH", 	CLOHIGH,	TRUE},
{"CLOTH", 	CLONORMAL,	TRUE},
{"CANVAS",     	CLOCANVAS,	TRUE},
{"HIGH_LEATHER", 	LEATHIGH,	TRUE},
{"LEATHER", 	LEATNORMAL,	TRUE},
{"ROCK_QUARTZ",     	RCKQUARTZ,	TRUE},
{"ROCK_GENERIC", 	RCKGENERIC,	TRUE},
{"ROCK_MARBLE", 	RCKMARBLE,	TRUE},
{"MAGIC_ENERGY", 	MAGENERGY,	TRUE},
{"GLASS",      	MATGLASS,	TRUE},
{"WOOD",       	MATWOOD,	TRUE},
{"BONE",       	MATBONE,	TRUE},
{"PARCHMENT",   MATPARCH,	TRUE},
{"POTTERY",    	MATPOTTERY,	TRUE},
{"ORGANIC",    	MATORGANIC,	TRUE},
{"UNKNOWN",    	MATUNKNOWN,	TRUE},
    {	NULL,		0,			0	}
};


const struct flag_type type_flags[] =
{
    {	"light_source",		ITEM_LIGHT_SOURCE,	TRUE	},
    {	"scroll",		ITEM_SCROLL,		TRUE	},
    {	"wand",			ITEM_WAND,		TRUE	},
    {	"staff",		ITEM_STAFF,		TRUE	},
    {	"weapon",		ITEM_WEAPON,		TRUE	},
    {	"treasure",		ITEM_TREASURE,		TRUE	},
    {	"armor",		ITEM_ARMOR,		TRUE	},
    {	"potion",		ITEM_POTION,		TRUE	},
    {	"furniture",		ITEM_FURNITURE,		TRUE	},
    {	"trash",		ITEM_TRASH,		TRUE	},
    {	"container",		ITEM_CONTAINER,		TRUE	},
    {	"drinkcontainer",	ITEM_DRINK_CON,		TRUE	},
    {	"key",			ITEM_KEY,		TRUE	},
    {	"food",			ITEM_FOOD,		TRUE	},
    {	"money",		ITEM_MONEY,		TRUE	},
    {	"boat",			ITEM_BOAT,		TRUE	},
    {	"npccorpse",		ITEM_CORPSE_NPC,	TRUE	},
    {	"pc corpse",		ITEM_CORPSE_PC,		FALSE	},
    {	"fountain",		ITEM_FOUNTAIN,		TRUE	},
    {	"pill",			ITEM_PILL,		TRUE	},
    {	"protect",		ITEM_PROTECT,		TRUE	},
    {	"map",			ITEM_MAP,		TRUE	},
    {   "portal",		ITEM_PORTAL,		TRUE	},
    {   "warpstone",		ITEM_WARP_STONE,	TRUE	},
    {	"roomkey",		ITEM_ROOM_KEY,		TRUE	},
    { 	"gem",			ITEM_GEM,		TRUE	},
    {	"jewelry",		ITEM_JEWELRY,		TRUE	},
    {   "segment",		ITEM_SEGMENT,		TRUE	},
    {	NULL,			0,			0	}
};


const struct flag_type extra_flags[] =
{
    {	"Glow",			ITEM_GLOW,		TRUE	},
    {	"HUM",			ITEM_HUM,		TRUE	},
    {	"Evil",			ITEM_EVIL,		TRUE	},
    {	"Invis",		ITEM_INVIS,		TRUE	},
    {	"Magic",		ITEM_MAGIC,		TRUE	},
    {	"NoDrop",		ITEM_NODROP,		TRUE	},
    {	"Bless",		ITEM_BLESS,		TRUE	},
    {	"AGood",		ITEM_ANTI_GOOD,		TRUE	},
    {	"AEvil",		ITEM_ANTI_EVIL,		TRUE	},
    {	"ANeut",		ITEM_ANTI_NEUTRAL,	TRUE	},
    {	"NoRem",		ITEM_NOREMOVE,		TRUE	},
    {	"Inven",		ITEM_INVENTORY,		TRUE	},
    {	"NoPurge",		ITEM_NOPURGE,		TRUE	},
    {	"RotDeath",		ITEM_ROT_DEATH,		TRUE	},
    {	"VisDeath",		ITEM_VIS_DEATH,		TRUE	},
    {   "Frag",			ITEM_FRAGILE,		TRUE	},
    {   "NONMetal",		ITEM_NONMETAL,		TRUE	},
    {	"NoLocate",		ITEM_NOLOCATE,		TRUE	},
    {	"MeltDrop",		ITEM_MELT_DROP,		TRUE	},
    {	"HadTimer",		ITEM_HAD_TIMER,		TRUE	},
    {	"SellExt",		ITEM_SELL_EXTRACT,	TRUE	},
    {	"FireProof",		ITEM_FIREPROOF,		TRUE	},
    {	"NoUncurse",		ITEM_NOUNCURSE,		TRUE	},
    {	"ClanEQ",		ITEM_CLAN_EQ,		TRUE	},
    {	"SpecKEY",		ITEM_SPECIAL_KEY,	TRUE	},
    {	"FORTIFIED",		ITEM_FORTIFIED,		TRUE	},
    {   "Legend",       ITEM_LEGEND,       TRUE },
    // Stones of Wisdom Stuff
    {   "Dice",         ITEM_DICE,      TRUE  },
    {	NULL,			0,			0	}
};



const struct flag_type wear_flags[] =
{
    {	"take",			ITEM_TAKE,		TRUE	},
    {	"light",		ITEM_WEAR_LIGHT,	TRUE	},
    {	"finger",		ITEM_WEAR_FINGER,	TRUE	},
    {	"neck",			ITEM_WEAR_NECK,		TRUE	},
    {	"body",			ITEM_WEAR_BODY,		TRUE	},
    {	"head",			ITEM_WEAR_HEAD,		TRUE	},
    {	"legs",			ITEM_WEAR_LEGS,		TRUE	},
    {	"feet",			ITEM_WEAR_FEET,		TRUE	},
    {	"hands",		ITEM_WEAR_HANDS,	TRUE	},
    {	"arms",			ITEM_WEAR_ARMS,		TRUE	},
    {	"shield",		ITEM_WEAR_SHIELD,	TRUE	},
    {	"about",		ITEM_WEAR_ABOUT,	TRUE	},
    {	"waist",		ITEM_WEAR_WAIST,	TRUE	},
    {	"wrist",		ITEM_WEAR_WRIST,	TRUE	},
    {	"wield",		ITEM_WIELD,		TRUE	},
    {	"hold",			ITEM_HOLD,		TRUE	},
    {   "nosac",		ITEM_NO_SAC,		TRUE	},
    {	"wearfloat",		ITEM_WEAR_FLOAT,	TRUE	},
    {	"back",			ITEM_WEAR_BACK,		TRUE	},
    {	NULL,			0,			0	}
};

/*
 * Used when adding an affect to tell where it goes.
 * See addaffect and delaffect in act_olc.c
 */
const struct flag_type apply_flags[] =
{
    {	"NONE",			APPLY_NONE,		TRUE	},
    {	"STR",			APPLY_STR,		TRUE	},
    {	"DEX",			APPLY_DEX,		TRUE	},
    {	"INT",			APPLY_INT,		TRUE	},
    {	"WIS",			APPLY_WIS,		TRUE	},
    {	"CON",			APPLY_CON,		TRUE	},
    {	"SEX",			APPLY_SEX,		TRUE	},
    {	"Mana",			APPLY_MANA,		TRUE	},
    {	"HP",			APPLY_HIT,		TRUE	},
    {	"Move",			APPLY_MOVE,		TRUE	},
    {	"Armor",		APPLY_AC,		TRUE	},
    {	"HITRoll",		APPLY_HITROLL,		TRUE	},
    {	"DAMRoll",		APPLY_DAMROLL,		TRUE	},
    {	"Saves",		APPLY_SAVES,		TRUE	},
    {	NULL,			0,			0	}
};



/*
 * What is seen.
 */
const struct flag_type wear_loc_strings[] =
{
    {	"inventory",	WEAR_INVENTORY,	TRUE	},
    {	"light",	WEAR_LIGHT,	TRUE	},
    {	"left finger",	WEAR_FINGER_L,	TRUE	},
    {	"right finger",	WEAR_FINGER_R,	TRUE	},
    {	"neck (1)",	WEAR_NECK_1,	TRUE	},
    {	"neck (2)",	WEAR_NECK_2,	TRUE	},
    {	"body",		WEAR_BODY,	TRUE	},
    {	"head",		WEAR_HEAD,	TRUE	},
    {	"legs",		WEAR_LEGS,	TRUE	},
    {	"feet",		WEAR_FEET,	TRUE	},
    {	"hands",	WEAR_HANDS,	TRUE	},
    {	"arms",		WEAR_ARMS,	TRUE	},
    {	"shield",	WEAR_SHIELD,	TRUE	},
    {	"about",	WEAR_ABOUT,	TRUE	},
    {	"waist",	WEAR_WAIST,	TRUE	},
    {	"left wrist",	WEAR_WRIST_L,	TRUE	},
    {	"right wrist",	WEAR_WRIST_R,	TRUE	},
    {	"wielded",	WEAR_WIELD,	TRUE	},
    {	"holding",	WEAR_HOLD,	TRUE	},
    {	"floating",	WEAR_FLOAT,	TRUE	},
    {	"dual weapon",	WEAR_SECONDARY,	TRUE	},
    {	"on the back",	WEAR_BACK,	TRUE	},
    {	NULL,			0	      , 0	}
};


const struct flag_type wear_loc_flags[] =
{
    {	"inventory",	WEAR_INVENTORY,	TRUE	},
    {	"light",	WEAR_LIGHT,	TRUE	},
    {	"lfinger",	WEAR_FINGER_L,	TRUE	},
    {	"rfinger",	WEAR_FINGER_R,	TRUE	},
    {	"neck1",	WEAR_NECK_1,	TRUE	},
    {	"neck2",	WEAR_NECK_2,	TRUE	},
    {	"body",		WEAR_BODY,	TRUE	},
    {	"head",		WEAR_HEAD,	TRUE	},
    {	"legs",		WEAR_LEGS,	TRUE	},
    {	"feet",		WEAR_FEET,	TRUE	},
    {	"hands",	WEAR_HANDS,	TRUE	},
    {	"arms",		WEAR_ARMS,	TRUE	},
    {	"shield",	WEAR_SHIELD,	TRUE	},
    {	"about",	WEAR_ABOUT,	TRUE	},
    {	"waist",	WEAR_WAIST,	TRUE	},
    {	"lwrist",	WEAR_WRIST_L,	TRUE	},
    {	"rwrist",	WEAR_WRIST_R,	TRUE	},
    {	"wielded",	WEAR_WIELD,	TRUE	},
    {	"hold",		WEAR_HOLD,	TRUE	},
    {	"floating",	WEAR_FLOAT,	TRUE	},
    {	"dualed",	WEAR_SECONDARY,	TRUE	},
    {	"back",		WEAR_BACK,	TRUE	},
    {	NULL,		0,		0	}
};

const struct flag_type container_flags[] =
{
    {	"closeable",		1,		TRUE	},
    {	"pickproof",		2,		TRUE	},
    {	"closed",		4,		TRUE	},
    {	"locked",		8,		TRUE	},
    {	"puton",		16,		TRUE	},
    {	NULL,			0,		0	}
};

/*****************************************************************************
                      ROM - specific tables:
 ****************************************************************************/




const struct flag_type ac_type[] =
{
    {   "pierce",        AC_PIERCE,            TRUE    },
    {   "bash",          AC_BASH,              TRUE    },
    {   "slash",         AC_SLASH,             TRUE    },
    {   "exotic",        AC_EXOTIC,            TRUE    },
    {   NULL,              0,                    0       }
};


const struct flag_type size_flags[] =
{
    {   "tiny",          SIZE_TINY,            TRUE    },
    {   "small",         SIZE_SMALL,           TRUE    },
    {   "medium",        SIZE_MEDIUM,          TRUE    },
    {   "large",         SIZE_LARGE,           TRUE    },
    {   "huge",          SIZE_HUGE,            TRUE    },
    {   "giant",         SIZE_GIANT,           TRUE    },
    {   NULL,              0,                    0       },
};

/*
const struct flag_type weapon_class[] =
{
    {   "none",         0,	TRUE    },
    {   "sword",        1,	TRUE    },
    {   "dagger",       2,	TRUE    },
    {   "spear",        3,	TRUE    },
    {   "mace",   	4,	TRUE    },
    {   "axe",	        5,	TRUE    },
    {   "flail",        6,	TRUE    },
    {   "whip",	        7,	TRUE    },
    {   "polearm",      8,	TRUE    },
    {   "exotic",       9,	TRUE    },
    {   NULL,		0,			0       } 
};
*/

const struct flag_type weapon_class[] =
{
    {   "none",         WEAPON_NONE,	TRUE    },
    {   "sword",        WEAPON_SWORD,	TRUE    },
    {   "dagger",       WEAPON_DAGGER,	TRUE    },
    {   "spear",        WEAPON_SPEAR,	TRUE    },
    {   "mace",   	WEAPON_MACE,	TRUE    },
    {   "axe",	        WEAPON_AXE,	TRUE    },
    {   "flail",        WEAPON_FLAIL,	TRUE    },
    {   "whip",	        WEAPON_WHIP,	TRUE    },
    {   "polearm",      WEAPON_POLEARM,	TRUE    },
    {   "exotic",       WEAPON_EXOTIC,	TRUE    },
    {   NULL,		0,			0       } 
};

const struct flag_type weapon_type2[] =
{
    {   "flaming",       WEAPON_FLAMING,       TRUE    },
    {   "frost",         WEAPON_FROST,         TRUE    },
    {   "vampiric",      WEAPON_VAMPIRIC,      TRUE    },
    {   "sharp",         WEAPON_SHARP,         TRUE    },
    {   "vorpal",        WEAPON_VORPAL,        TRUE    },
    {   "twohands",     WEAPON_TWO_HANDS,     TRUE    },
    {	"shocking",	 WEAPON_SHOCKING,      TRUE    },
    {	"poison",	WEAPON_POISON,		TRUE	},
    {   NULL,              0,                    0       }
};



const struct flag_type position_flags[] =
{
    {   "dead",           POS_DEAD,            FALSE   },
    {   "mortal",         POS_MORTAL,          FALSE   },
    {   "incap",          POS_INCAP,           FALSE   },
    {   "stunned",        POS_STUNNED,         FALSE   },
    {   "sleeping",       POS_SLEEPING,        TRUE    },
    {   "resting",        POS_RESTING,         TRUE    },
    {   "sitting",        POS_SITTING,         TRUE    },
    {   "fighting",       POS_FIGHTING,        FALSE   },
    {   "standing",       POS_STANDING,        TRUE    },
    {   NULL,              0,                    0       }
};

const struct flag_type portal_flags[]=
{
    {   "normal_exit",	  GATE_NORMAL_EXIT,	TRUE	},
    {	"no_curse",	  GATE_NOCURSE,		TRUE	},
    {   "go_with",	  GATE_GOWITH,		TRUE	},
    {   "buggy",	  GATE_BUGGY,		TRUE	},
    {	"random",	  GATE_RANDOM,		TRUE	},
    {   NULL,		  0,			0	}
};

const struct flag_type furniture_flags[]=
{
    {   "stand_at",	  STAND_AT,		TRUE	},
    {	"stand_on",	  STAND_ON,		TRUE	},
    {	"stand_in",	  STAND_IN,		TRUE	},
    {	"sit_at",	  SIT_AT,		TRUE	},
    {	"sit_on",	  SIT_ON,		TRUE	},
    {	"sit_in",	  SIT_IN,		TRUE	},
    {	"rest_at",	  REST_AT,		TRUE	},
    {	"rest_on",	  REST_ON,		TRUE	},
    {	"rest_in",	  REST_IN,		TRUE	},
    {	"sleep_at",	  SLEEP_AT,		TRUE	},
    {	"sleep_on",	  SLEEP_ON,		TRUE	},
    {	"sleep_in",	  SLEEP_IN,		TRUE	},
    {	"put_at",	  PUT_AT,		TRUE	},
    {	"put_on",	  PUT_ON,		TRUE	},
    {	"put_in",	  PUT_IN,		TRUE	},
    {	"put_inside",	  PUT_INSIDE,		TRUE	},
    {	NULL,		  0,			0	}
};

const	struct	flag_type	apply_types	[]	=
{
	{	"affects",	TO_AFFECTS,	TRUE	},
	{	"affects2",	TO_AFFECTS2,	TRUE	},
	{	"object",	TO_OBJECT,	TRUE	},
	{	"immune",	TO_IMMUNE,	TRUE	},
	{	"resist",	TO_RESIST,	TRUE	},
	{	"vuln",		TO_VULN,	TRUE	},
	{	"weapon",	TO_WEAPON,	TRUE	},
	{	NULL,		0,		TRUE	}
};

const	struct	bit_type	bitvector_type	[]	=
{
	{	affect_flags,	"affect"	},
	{	affect2_flags,	"affect2"	},
	{	apply_flags,	"apply"		},
	{	imm_flags,	"imm"		},
	{	res_flags,	"res"		},
	{	vuln_flags,	"vuln"		},
	{	weapon_type2,	"weapon"	}
};

const struct quest_type	quest_table[] =
{
/*       {    name,	            vnum,    qps,    level  },*/
         {   "bottle oj",           25402,   300,       20  },
         {   "white",               25308,   600,       50  },
         {   "black",               25309,   600,       50  },
      	 {   "girth heroism",       25300,  1500,       51  },
         {   "evil",                25304,  1000,       51  },
         {   "ring heroism",        25301,  1000,       51  },
         {   "ring evil",           25305,  1000,       51  },
         {   "blade",               25302,  1500,       51  },
         {   "blade evil",          25306,  1500,       51  },
         {   "dagger",              25303,  1500,       51  },
         {   "dagger evil",         25307,  1500,       51  },
         {   "axe heroism",         25410,  1000,       51  },
         {   "axe evil",            25411,  1500,       51  },
         {   "flail heroism",       25412,  1000,       51  },
         {   "flail evil",          25413,  1500,       51  },
         {   "spear heroism",       25414,  1000,       51  },
         {   "spear evil",          25415,  1500,       51  },
         {   "whip heroism",        25416,  1000,       51  },
         {   "whip evil",           25417,  1500,       51  },
         {   "mace heroism",        25418,  1000,       51  },
         {   "mace evil",           25419,  1500,       51  },
         {   "fauchard heroism",    25420,  1000,       51  },
         {   "blanket power",       25421,  3000,       90  },
         {   "staff",               25310,  2000,       95  },
         {      NULL,                   0,      0,              0       }
};
