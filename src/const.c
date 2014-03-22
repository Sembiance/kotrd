#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <time.h>
#include "merc.h"
#include "magic.h"
#include "interp.h"


/* material type list */
const struct material_type		material_table	[]	=
{ 
 /* {   "FLAG"          "NAME"        "DESCRIPTION"}  */
{MATERIAL_NONE, "SELECT_A_MATERIAL","Place Holder For Material Slot"},
{METORICHALCUM, "ORICHALCUM",  	"METAL-Orichalcum: Extremely Magical, Almost Legendary."},  
{METNIOBLAER,   "LAER_NIOB",  	"METAL-Niobium: Laerkai are the Masters of this Metal."},  
{METNIOBDRAK,   "DRAK_NIOB",  	"METAL-Niobium: Drakyri surpassed only by the Laerkai."},
{METNIOBIUM,    "NIOBIUM",    	"METAL-Niobium: If used, this is what MOST EQ should be."},
{METADAMDELF,   "DELF_ADAM",  	"METAL-Adamantium: Dark Elves perfected this Metals use."},
{METADAMANT,    "ADAMANTIUM",   "METAL-Adamantium: If not going on a DELF, use this one."},
{METMITHDWAR,   "DWRF_MITH",  	"METAL-Mithril: Dwarven WeaponSmiths Prize this Metal."},
{METMITHRIL,    "MITHRIL",    	"METAL-Mithril: As Per NIOB & ADAM Unless Dwarven Area."},
{METSTLELF,     "ELF_STEEL",   	"METAL-Steel: Elven ArmorSmiths make Elven Chain from this."},
{METSTLHIGH,    "HIGH_STEEL",  	"METAL-Steel: High Quality Steel should be a bit Uncommon."},
{METSTEEL,      "STEEL",      	"METAL-Steel: Normal run-of-the-mill Steel."},
{METIRON,	"IRON",       	"METAL-Iron: The most common Metal used for EQ"},
{METBRONZE,	"BRONZE",     	"METAL-Bronze: Not as strong as Steel but better for Casters."},
{METBRASS,	"BRASS",      	"METAL-Brass: Less MINUS to stealth/casting & still metal"},
{METCOPPER,	"COPPER",     	"METAL-Copper: Low TECH races still make EQ from this."},
{METPLATINUM,   "PLATINUM",   	"METAL-Platinum: Worth more than Gold, but MUCH Rarer."},
{METGOLD,	"GOLD",       	"METAL-Gold: The Currency Standard in KotRD."},
{METSILVER,	"SILVER",     	"METAL-Silver: Also used as Currency in KotRD."},
{METGENERIC,	"MET_GENERIC",	"METAL-Other: Generic Metal, tell Arioch to get more Metals."},
{GEMSOLSTN,	"SOULSTONE",  	"GEM-SoulStone: Highly Magical-Precious *Extremely Rare"},
{GEMBLKDIA,     "BLACK_DIAMOND","GEM-Black Diamond: High Magic & Value-Precious *Very Rare"},
{GEMBLKOPAL,    "BLACK_OPAL",   "GEM-Black Opal: Mysterious Qualities-Precious *Very Rare"},
{GEMBLKSAPP,    "BLACK_SAPPHIRE","GEM-Black Sapphire: EXTREMELY Valuable-Precious *Very Rare"},
{GEMSTRRUBY,    "STAR_RUBY",  	"GEM-Star Ruby: Precious *Rare"},
{GEMSTRSAPP,    "STAR_SAPPHIRE","GEM-Star Sapphire: Precious *Rare"},
{GEMFIREOPAL,   "FIRE_OPAL",  	"GEM-Fire Opal: Precious *Rare"	},
{GEMSTRSTN,     "STARSTONE",  	"GEM-StarStone: Has some PSI Energy-Precious *Very Uncommon"},
{GEMDIAMOND,	"DIAMOND",    	"GEM-Diamond: Precious *Very Uncommon"},
{GEMRUBY,	"RUBY",       	"GEM-Ruby: Precious *Uncommon"},
{GEMEMERALD,	"EMERALD",    	"GEM-Emerald: Precious *Uncommon"},
{GEMSAPPHIRE,   "SAPPHIRE",   	"GEM-Sapphire: Precious *Uncommon"},
{GEMOPAL,	"OPAL",       	"GEM-Opal: Precious *Uncommon"}, 
{GEMAQUAMAR,	"AQUAMARINE", 	"GEM-Aquamarine: Precious *Uncommon"},
{GEMAMETHYST,   "AMETHYST",   	"GEM-Amethyst: Semi-Precious"},
{GEMJADE,	"JADE",       	"GEM-Jade: Semi-Precious"},
{GEMPEARL,	"PEARL",      	"GEM-Pearl: Semi-Precious"},
{GEMAMBER,	"AMBER",      	"GEM-Amber: Semi-Precious"},
{GEMJET,	"JET",        	"GEM-Jet: Semi-Precious"},
{GEMTOPAZ,	"TOPAZ",      	"GEM-Topaz: Semi-Precious"},
{GEMJACINTH,	"JACINTH",    	"GEM-Jacinth: Semi-Precious"},
{GEMMOONSTN,	"MOONSTONE",  	"GEM-MoonStone: Common"},
{GEMONYX,	"ONYX",       	"GEM-Onyx: Common"},
{GEMJASPER,	"JASPER",     	"GEM-Jasper: Common"},
{GEMZIRCON,	"ZIRCON",     	"GEM-Zircon: Common"},
{GEMSTRROSE,    "STAR_ROSE",  	"GEM-Star Rose: Common"},
{GEMGENERIC,	"GEM_GENERIC",	"GEM-Other: Tell Arioch if you need more GEM-types."},
{ELEDIVINE,	"DIVINE_ELEM", 	"ELEMENTAL-Divine Power: The Power of the GODs."},
{ELESTELLAR,	"STELLAR_ELEM", "ELEMENTAL-Stellar: The Power of the STARs."},
{ELELIGHT,	"LIGHT_ELEM",  	"ELEMENTAL-Light: The Power of GOOD."},    
{ELEDARK,	"DARK_ELEM",   	"ELEMENTAL-Darkness: Made of PUREST Evil."},
{ELEFIRE,	"FIRE_ELEM",   	"ELEMENTAL-Fire: Made with the ESSENCE of Fire."},
{ELEWATER,	"WATER_ELEM",  	"ELEMENTAL-Water: Drawn from the Power of Water."},
{ELEEARTH,	"EARTH_ELEM",  	"ELEMENTAL-Earth: Made with the STRENGTH of the Earth."},
{ELEAIR,	"AIR_ELEM",   	"ELEMENTAL-Air: Blessed by the Elementals of Air."},
{ELELIGHTING,   "LIGHTNING_ELEM","ELEMENTAL-Lightning: The Harnessed Power of Electricity."},
{CLOSPIDSILK,   "SPIDER_SILK",	"CLOTH-Spider Silk: The Most Valuable Fabric Known."},
{CLOELFSILK,    "ELVEN_SILK", 	"CLOTH-Elven Silk: Second only to Spider Silk."},
{CLOSILK,	"SILK",       	"CLOTH-Silk: High Nobility Prize This Fabric."},
{CLOHIGH,	"HIGH_CLOTH", 	"CLOTH-High Quality: A Merchant's Clothing."},
{CLONORMAL,	"CLOTH", "CLOTH-Normal: Standard Fabric used for Clothes."},
{CLOCANVAS,	"CANVAS",     	"CLOTH-Canvas: Used for making tents & such."},
{LEATHIGH,	"HIGH_LEATHER", "LEATHER-High Quality: Expensive & Rare, but worth it."},
{LEATNORMAL,	"LEATHER", "LEATHER-Normal: The Standard Leather Used."},
{RCKQUARTZ,     "ROCK_QUARTZ",     	"ROCK-Quartz: Just a Pretty Rock."},
{RCKGENERIC,	"ROCK_GENERIC", "ROCK-Generic: Just a Rock, Tell Arioch for more."},
{RCKMARBLE,	"ROCK_MARBLE", "ROCK-Marble: A Common Type of Marble."},
{MAGENERGY,	"MAGIC_ENERGY", "MAGICAL-Energy: Made of PURE Magic  *Very Rare"},
{MATGLASS,	"GLASS",      	"MISC-Glass: Used for Vials & Beakers."},
{MATWOOD,       "WOOD",       	"MISC-Wood: Used for Houses, Arrows, etc..."},
{MATBONE,       "BONE",       	"MISC-Bone: This is Used for ANY type of Bone."},
{MATPARCH,      "PARCHMENT",    "MISC-Parchment: Used for Scrolls & Letters."},
{MATPOTTERY,    "POTTERY",    	"MISC-Pottery: Plates, Mugs, Other EatingWare.."},  
{MATORGANIC,    "ORGANIC",    	"MISC-Organic: Made from & is still, slightly ALIVE."},  
{MATUNKNOWN,	"UNKNOWN",    	"UNKNOWN: Need more Material-types, Tell Argon."},
{   0, NULL,		NULL		}
};

/* item type list */
const struct item_type		item_table	[]	=
{
    {	ITEM_LIGHT_SOURCE,	"light_source"	},
    {	ITEM_SCROLL,	"scroll"	},
    {	ITEM_WAND,	"wand"		},
    {   ITEM_STAFF,	"staff"		},
    {   ITEM_WEAPON,	"weapon"	},
    {   ITEM_TREASURE,	"treasure"	},
    {   ITEM_ARMOR,	"armor"		},
    {	ITEM_POTION,	"potion"	},
    {	ITEM_CLOTHING,	"clothing"	},
    {   ITEM_FURNITURE,	"furniture"	},
    {	ITEM_TRASH,	"trash"		},
    {	ITEM_CONTAINER,	"container"	},
    {	ITEM_DRINK_CON, "drink"		},
    {	ITEM_KEY,	"key"		},
    {	ITEM_FOOD,	"food"		},
    {	ITEM_MONEY,	"money"		},
    {	ITEM_BOAT,	"boat"		},
    {	ITEM_CORPSE_NPC,"npc_corpse"	},
    {	ITEM_CORPSE_PC,	"pc_corpse"	},
    {   ITEM_FOUNTAIN,	"fountain"	},
    {	ITEM_PILL,	"pill"		},
    {	ITEM_PROTECT,	"protect"	},
    {	ITEM_MAP,	"map"		},
    {	ITEM_PORTAL,	"portal"	},
    {	ITEM_WARP_STONE,"warp_stone"	},
    {	ITEM_ROOM_KEY,	"room_key"	},
    {	ITEM_GEM,	"gem"		},
    {	ITEM_JEWELRY,	"jewelry"	},
    {   ITEM_SEGMENT,	"segment"	},
    {   0,		NULL		}
};


/* weapon selection table */
const	struct	weapon_type	weapon_table	[]	=
{
   { "none",	OBJ_VNUM_SCHOOL_DAGGER,	WEAPON_NONE,	&gsn_dagger	}, 
   { "sword",	OBJ_VNUM_SCHOOL_SWORD,	WEAPON_SWORD,	&gsn_sword	},
   { "dagger",	OBJ_VNUM_SCHOOL_DAGGER,WEAPON_DAGGER,	&gsn_dagger	},
   { "spear",	OBJ_VNUM_SCHOOL_STAFF,	WEAPON_SPEAR,	&gsn_spear	},
   { "mace",	OBJ_VNUM_SCHOOL_MACE,	WEAPON_MACE,	&gsn_mace 	},
   { "axe",	OBJ_VNUM_SCHOOL_AXE,	WEAPON_AXE,	&gsn_axe	},
   { "flail",	OBJ_VNUM_SCHOOL_FLAIL,	WEAPON_FLAIL,	&gsn_flail	},
   { "whip",	OBJ_VNUM_SCHOOL_WHIP,	WEAPON_WHIP,	&gsn_whip	},
   { "polearm",	OBJ_VNUM_SCHOOL_POLEARM,WEAPON_POLEARM, &gsn_polearm	},
   { "exotic",	OBJ_VNUM_SCHOOL_WHIP,   WEAPON_EXOTIC,  &gsn_exotic	},
   { NULL,	0,				0,	NULL		}

};

const  struct slay_type                slay_table      [MAX_SLAY_TYPES]        =
{
/* owner        title           char_msg        vict_msg        room_msg */
  {
   "Argon",
   "1 {c- {RDemon{x",
   "\n\r{RYou gesture, and a slavering demon appears.  With a horrible grin, the foul\n\rcreature turns on {W$N{R, who screams in panic before being eaten alive.{x",
   "\n\r{W$n {Rgestures, and a slavering demon appears.  The foul creature turns on you\n\r with a horrible grin. You scream in panic before being eaten alive.{x",
   "\n\r{W$n {Rgestures, and a slavering demon appears.  With a horrible grin, the foul\n\rcreature turns on {W$N{R, who screams in panic before being eaten alive.{x"
  },

  {
   "Argon",
   "2 {c- {GSkin{x",
   "\n\r{GYou rip the flesh from {W$N {Gand send his soul to the fiery depths of hell.{x",
   "\n\r{GYour flesh has been torn from your bones and your bodyless soul now watches\n\ryour bones incenerate in the fires of hell.{x",
   "\n\r{W$n {Grips the flesh off of {W$N{G, releasing his soul into the fiery depths of hell.{x" 
  },

  {
   "Argon",
   "3 {c- {D9mm{x",
   "\n\r{DYou pull out your 9mm Barrette loaded with Armor Piercing-Explosive Rounds,\n\rand bust a cap in {W$N{D's ass.{x",
   "\n\r{W$n {Dpulls out $s 9mm Baretta loaded with Teflon Coated, Explosive Rounds,\n\rand busts a cap in your ass.{x",
   "\n\r{W$n {Dpulls out $s 9mm Baretta loaded with Teflon Coated, Explosive Rounds,\n\rand busts a cap in {W$N{D's ass.{x"
  },

  {
   "Argon",
   "4 {c- {MHeart{x",
   "\n\r{MYou rip through {W$N{M's chest and pull out $S still beating heart and show it to $M.{x",
   "\n\r{MYou feel a sharp pain as {W$n {Mrips into your chest and pulls out your beating\n\rheart, showing it to you!{x",
   "\n\r{MSpecks of blood hit your face as {M$n {Wrips through {W$N{M's chest pulling out $S\n\rstill beating heart while laughing!{x"
  },

  {
   "Argon",
   "5 {c- {rFireBall{x",
   "\n\r{rYour fireball turns {W$N {rinto a smoldering pile of chargrilled, bite size pieces!{x",
   "\n\r{W$n {rreleases a searing fireball in your direction.{x",
   "\n\r{W$n {rpoints at {W$N{r, who explodes!  All that remains is a smoldering pile of\n\rchargrilled, bite size pieces!{x"
  },

  {
   "Argon",
   "6 {c- {BGlance{x",
   "\n\r{BWith a glance, you freeze {W$N {Band then shatter the frozen corpse into tiny shards!{x",
   "\n\r{BWith a glance, {W$n {Bfreezes you and then shatters your frozen body into tiny shards!{x",
   "\n\r{BWith a glance, {W$n {Bfreezes {W$N {Band then shatters the frozen body into tiny shards!{x"
  },

  {
   "Argon",
   "7 {c- {YThroat{x",
   "\n\r{YLeaping upon {W$N {Ywith bared fangs, you tear open $S throat and toss the\n\rcorpse to the ground...{x",
   "\n\r{YIn a heartbeat, {W$n {Yrips $s fangs through your throat!  Your blood sprays\n\rand pours to the ground as your life ends...{x",
   "\n\r{YLeaping suddenly, {W$n {Ysinks $s fangs into {W$N{Y's throat.  As blood sprays\n\rand gushes to the ground, {W$n {Ytosses {W$N{Y's dying body away.{x"
  },
 
  {
    "",
    "A {c- {WSoul{x",
    "\n\r{GYou tear the soul from {W$N {Gand leave their corpse to rot!{x",
    "\n\r{W$n {Grips the soul from you, leaving your corpse to rot!{x",
    "\n\r{W$n {Gtears the soul from {W$N{G, leaving the corpse to rot!{x"
  }
};

 
/* wiznet table and prototype for future flag setting */
const   struct wiznet_type      wiznet_table    []              =
{
   {    "on",           WIZ_ON,         IM },
   {    "prefix",	WIZ_PREFIX,	IM },
   {    "ticks",        WIZ_TICKS,      GG },
   {    "logins",       WIZ_LOGINS,     LG },
   {    "sites",        WIZ_SITES,      AI },
   {    "links",        WIZ_LINKS,      GG },
   {	"newbies",	WIZ_NEWBIE,	NI },
   {	"levels",	WIZ_LEVELS,	GG },
   {	"secure",	WIZ_SECURE,	AI },
   {	"penalties",	WIZ_PENALTIES,	AI },
   {	"deaths",	WIZ_DEATHS,	ML },
   {    "bugs",         WIZ_BUGS,       ML },
   {	NULL,		0,		0  }
};

char gWarriorScoreLegend[] =
"\n\r{W                                    $name {cClan{w: {C$clan {x\n\r"
"{W      _,.  {c_____________________________________________________________________ (@){x\n\r"
"{W    ,` -.){c/ {cHours{w: {C$hours {cLevel {w: {C$level {c| {cSTR {w: {W$str  {c| {cTrains    {w: {C$trains {c\\| |{x\n\r"
"{W   ( {W_{W/{D-{W\\{D-._  {cAge{w: {C$age {cSex   {w: {C$sex {c| {cINT {w: {W$int  {c| {cPractices {w: {C$practices  {c|{w-{c|{x\n\r"
"{W  /,{D|`--._,-^|             {cAlign {w: {C$align {c| {cWIS {w: {W$wis  {c| {cExperience{w: {C$experience  {c| |{x\n\r"
"{W  \\_{D| |`-.,/||           {y,;     {Y$legend    {c| {cDEX {w: {W$dex  {c| {RExp to Lvl{w: {C$exptolevel  {c| |{x\n\r"
"{D    |  `-. .'|          {y/{w,{y;      {YLegend     {c| {cCON {w: {W$con  {c|                       | |{x\n\r"
"{D    |     || |         {y/{w.{y/{c-------------------------------------------------------|{w-{c|{x\n"
"{D     {D`r-._||/   __    {y/{w.{y/  {cHitPt {w: {W$hitpt{C($maxhit) {c|   {GPKills{w: {CAbove your lvl{w: {G$pkillsabove  {c| |{x\n\r"
"{D __,-<_     )`-/  `. {y/{w.{y/   {cMana  {w: {W$mana{C($maxmana) {c|           {cBelow your lvl{w: {G$pkillsbelow  {c| |{x\n\r"
"{D'  \\   `---'   \\   |{y/{w.{y/    {cMove  {w: {W$move{C($maxmove) {c|   {RPDeths{w: {CAbove your lvl{w: {R$pdeathsabove  {c| |{x\n\r"
"{D    |           |._{y/{w.{y/     {cWimp  {w: {C$wimp {c       |           {cBelow your lvl{w: {R$pdeathsbelow  {c| |{x\n\r"
"{D    /   {rRot{RR{rD   {D/ {y/{w.{y/{c------------------------------------------------------------|{w-{c|{x\n\r"
"{D\\_/' \\         | {y/ /       {cHITroll{w: {C$hitroll {c|        {cItems {w: {C$items {cof {C$maxitems  {c| |{x\n\r"
"{D |    |  __/\\-. {y/ /        {cDAMroll{w: {C$damroll {c|        {cWeight{w: {C$weight {cof {C$maxweight  {c| |{x\n\r"
"{D |    ,`'    {w. {y/ /{w.{c--------------------------------------------------------------|{w-{c|{x\n\r"
"{D  \\,.->._    {y\\-{Dx{y-/   {cTotal mobs killed    {w: {W$percentkilledcur/$percentkilledmax {C$percentkilledpercent%  {c|    {cPierce {w: {C$pierce {c| |{x\n\r"
"{D  (  /   `-._{y//      {cTotal rooms explored {w: {W$percentexploredcur/$percentexploredmax {C$percentexploredpercent%  {c|    {cBash   {w: {C$bash {c| |{x\n\r"
"{D   `Y-.____(__;      {cTotal objects found  {w: {W$percentfoundcur/$percentfoundmax {C$percentfoundpercent%  {c|    {cSlash  {w: {C$slash {c| |{x\n\r"
"{D    |     {D(_){c-----------------------------------------------|    {cMagic  {w: {C$magic {c| |{x\n\r"
"{D    \\_____{y()         {cYour BeltPouch{w:    {cDeposited{C/{cBank{w:     {c+--------------------|{w-{c|{x\n\r"
"{c       \\             {YGold   {w: {Y$gold Gold   {w: {Y$bankgold {cQuest Pnts{w: {C$questpoints  {c| |{x\n\r"
"{c        \\            {WSilver {w: {W$silver Silver {w: {W$banksilver {cQuest Time{w: {C$questtime  {c| |{x\n\r"
"{c         \\_____________________________________________________________________  | |{x\n\r"
"{c                                                                               `\\|{w-{c|{x\n\r"
"{B                     $affects {c(@){x\n\r"
"{c                                                                                  '{x\n\r";


char gWarriorScore[] =
"\n\r{W                                    $name {cClan{w: {C$clan {x\n\r"
"{W      _,.  {c_____________________________________________________________________ (@){x\n\r"
"{W    ,` -.){c/ {cHours{w: {C$hours {cLevel {w: {C$level {c| {cSTR {w: {W$str  {c| {cTrains    {w: {C$trains {c\\| |{x\n\r"
"{W   ( {W_{W/{D-{W\\{D-._  {cAge{w: {C$age {cRace  {w: {C$race {c| {cINT {w: {W$int  {c| {cPractices {w: {C$practices  {c|{w-{c|{x\n\r"
"{W  /,{D|`--._,-^|             {cSex   {w: {C$sex {c| {cWIS {w: {W$wis  {c| {cExperience{w: {C$experience  {c| |{x\n\r"
"{W  \\_{D| |`-.,/||           {y,;{cClass {w: {C$class {c| {cDEX {w: {W$dex  {c| {RExp to Lvl{w: {C$exptolevel  {c| |{x\n\r"
"{D    |  `-. .'|          {y/{w,{y;{cAlign {w: {C$align {c| {cCON {w: {W$con  {c|                       | |{x\n\r"
"{D    |     || |         {y/{w.{y/{c-------------------------------------------------------|{w-{c|{x\n"
"{D     {D`r-._||/   __    {y/{w.{y/  {cHitPt {w: {W$hitpt{C($maxhit) {c|   {GPKills{w: {CAbove your lvl{w: {G$pkillsabove  {c| |{x\n\r"
"{D __,-<_     )`-/  `. {y/{w.{y/   {cMana  {w: {W$mana{C($maxmana) {c|           {cBelow your lvl{w: {G$pkillsbelow  {c| |{x\n\r"
"{D'  \\   `---'   \\   |{y/{w.{y/    {cMove  {w: {W$move{C($maxmove) {c|   {RPDeths{w: {CAbove your lvl{w: {R$pdeathsabove  {c| |{x\n\r"
"{D    |           |._{y/{w.{y/     {cWimp  {w: {C$wimp {c       |           {cBelow your lvl{w: {R$pdeathsbelow  {c| |{x\n\r"
"{D    /   {rRot{RR{rD   {D/ {y/{w.{y/{c------------------------------------------------------------|{w-{c|{x\n\r"
"{D\\_/' \\         | {y/ /       {cHITroll{w: {C$hitroll {c|        {cItems {w: {C$items {cof {C$maxitems  {c| |{x\n\r"
"{D |    |  __/\\-. {y/ /        {cDAMroll{w: {C$damroll {c|        {cWeight{w: {C$weight {cof {C$maxweight  {c| |{x\n\r"
"{D |    ,`'    {w. {y/ /{w.{c--------------------------------------------------------------|{w-{c|{x\n\r"
"{D  \\,.->._    {y\\-{Dx{y-/   {cTotal mobs killed    {w: {W$percentkilledcur/$percentkilledmax {C$percentkilledpercent%  {c|    {cPierce {w: {C$pierce {c| |{x\n\r"
"{D  (  /   `-._{y//      {cTotal rooms explored {w: {W$percentexploredcur/$percentexploredmax {C$percentexploredpercent%  {c|    {cBash   {w: {C$bash {c| |{x\n\r"
"{D   `Y-.____(__;      {cTotal objects found  {w: {W$percentfoundcur/$percentfoundmax {C$percentfoundpercent%  {c|    {cSlash  {w: {C$slash {c| |{x\n\r"
"{D    |     {D(_){c-----------------------------------------------|    {cMagic  {w: {C$magic {c| |{x\n\r"
"{D    \\_____{y()         {cYour BeltPouch{w:    {cDeposited{C/{cBank{w:     {c+--------------------|{w-{c|{x\n\r"
"{c       \\             {YGold   {w: {Y$gold Gold   {w: {Y$bankgold {cQuest Pnts{w: {C$questpoints  {c| |{x\n\r"
"{c        \\            {WSilver {w: {W$silver Silver {w: {W$banksilver {cQuest Time{w: {C$questtime  {c| |{x\n\r"
"{c         \\_____________________________________________________________________  | |{x\n\r"
"{c                                                                               `\\|{w-{c|{x\n\r"
"{B                     $affects {c(@){x\n\r"
"{c                                                                                  '{x\n\r";

const  struct score_type  warrior_score_table [] =
{
	{ "$name", 13, TRUE },
	{ "$clan", 25, TRUE },
	{ "$hours", 7, TRUE },
	{ "$level", 8, TRUE },
	{ "$trains", 8, FALSE },
	{ "$age", 7, TRUE },
	{ "$race", 8, TRUE },
	{ "$sex", 8, TRUE },
	{ "$class", 8, TRUE },
	{ "$align", 8, TRUE },
	{ "$str", 3, FALSE },
	{ "$int", 3, FALSE },
	{ "$wis", 3, FALSE },
	{ "$dex", 3, FALSE },
	{ "$con", 3, FALSE },
	{ "$practices", 8, FALSE },
	{ "$experience", 8, FALSE },
	{ "$exptolevel", 8, FALSE },
	{ "$hitpt", 5, FALSE },
	{ "$mana", 5, FALSE },
	{ "$move", 5, FALSE },
	{ "$wimp", 5, FALSE },
	{ "$maxhit", 5, FALSE },
	{ "$maxmana", 5, FALSE },
	{ "$maxmove", 5, FALSE },
	{ "$pkillsabove", 3, FALSE },
	{ "$pkillsbelow", 3, FALSE },
	{ "$pdeathsabove", 3, FALSE },
	{ "$pdeathsbelow", 3, FALSE },
	{ "$hitroll", 11, TRUE },
	{ "$damroll", 11, TRUE },
	{ "$items", 5, FALSE },
	{ "$maxitems", 5, FALSE },
	{ "$weight", 5, FALSE },
	{ "$maxweight", 5, FALSE },
	{ "$pierce", 6, TRUE },
	{ "$bash", 6, TRUE },
	{ "$slash", 6, TRUE },
	{ "$magic", 6, TRUE },
	{ "$questpoints", 5, FALSE },
	{ "$questtime", 5, FALSE },
	{ "$gold", 9, TRUE },
	{ "$silver", 9, TRUE },
	{ "$bankgold", 12, TRUE },
	{ "$banksilver", 12, TRUE },
	{ "$percentkilledcur", 4, FALSE },
	{ "$percentkilledmax", 4, FALSE },
	{ "$percentkilledpercent", 3, FALSE },
	{ "$percentexploredcur", 4, FALSE },
	{ "$percentexploredmax", 4, FALSE },
	{ "$percentexploredpercent", 3, FALSE },
	{ "$percentfoundcur", 4, FALSE },
	{ "$percentfoundmax", 4, FALSE },
	{ "$percentfoundpercent", 3, FALSE },
	{ "$affects", 59, TRUE },
	{ "$legend", 8, TRUE },
    { 0, 0, FALSE }
};

char gCasterScoreLegend[] =
"\n\r{W                                   $name {cClan{w: {C$clan {c.{x\n\r"
"{w          _____{c________________________________________________________________ (@){x\n\r"
"{w        .'{B* {W*{w./ {cHRs{w:{C$hours {cLevel {w: {C$level {c| STR {w: {W$str  {c| Trains    {w: {C$trains {c\\| |{x\n\r"
"{w     __/_{W*{w_{B*{w(_  {cAge{w: {C$age {cSex   {w: {C$sex {c| INT {w: {W$int  {c| Practices {w: {C$practices  {c|{w-{c|{x\n\r"
"{w    / _______ \\           {cAlign {w: {C$align {c| WIS {w: {W$wis  {c| Experience{w: {C$experience  {c| |{x\n\r"
"{r   _{w\\{r_{w){y/___\\({r_{w/{r_               {Y$legend    {c| DEX {w: {W$dex  {c| {RExp to Lvl{w: {C$exptolevel  {c| |{x\n\r"
"{r  / _{W(({y\\- -/{W)){r_ \\               {YLegend     {c| CON {w: {W$con  {c|                       | |{x\n\r"
"{r  \\ \\{W())({y-{W)((){r/ /{c---------------------------------------------------------------|{w-{c|{x\n\r"
"{r   ' \\{W(((())){r/ '          {cHitPt {w: {W$hitpt{C($maxhit) {c|   {GPKills{w: {CAbove your lvl{w: {G$pkillsabove  {c| |{x\n\r"
"{r  / ' \\{W)).)){r/ ' \\         {cMana  {w: {W$mana{C($maxmana) {c|           {cBelow your lvl{w: {G$pkillsbelow  {c| |{x\n\r"
"{r / _ \\ - | - /_  \\        {cMove  {w: {W$move{C($maxmove) {c|   {RPDeths{w: {CAbove your lvl{w: {R$pdeathsabove  {c| |{x\n\r"
"{r(   ( {W.;''';. {r.'  )       {cWimp  {w: {W$wimp {c       |           {cBelow your lvl{w: {R$pdeathsbelow  {c| |{x\n\r"
"{W_{r\\\"{W__ /    )\\ {W__{r\"/{W_ {c------------------------------------------------------------|{w-{c|{x\n\r"
"{r  \\/  {W\\   ' {W/  {r\\/         {cHITroll{w: {C$hitroll {c|        Items {w: {C$items {cof {C$maxitems  {c| |{x\n\r"
"{r   .'  {W'...' {r' )          {cDAMroll{w: {C$damroll {c|        Weight{c: {C$weight {cof {C$maxweight  {c| |{x\n\r"
"{r    / /  |  \\ \\{c-----------------------------------------------------------------|{w-{c|{x\n\r"
"{r   / .   .   . \\    {cTotal mobs killed    {w: {W$percentkilledcur/$percentkilledmax {C$percentkilledpercent%  {c|    Pierce {w: {C$pierce {c| |{x\n\r"
"{r  /   .     .   \\   {cTotal rooms explored {w: {W$percentexploredcur/$percentexploredmax {C$percentexploredpercent%  {c|    Bash   {w: {C$bash {c| |{x\n\r"
"{r /   /   |   \\   \\  {cTotal objects found  {w: {W$percentfoundcur/$percentfoundmax {C$percentfoundpercent%  {c|    Slash  {w: {C$slash {c| |{x\n\r"
"{r'   /    {Db    {r'.  '.{c---------------------------------------|    {cMagic  {w: {C$magic {c| |{x\n\r"
"{r   /     {DBb     {r'-. {cYour BeltPouch{w:    {cDeposited/Bank{w:     {c+--------------------|{w-{c|{x\n\r"
"{r  |      {DBBb       {r'-. {YGold   {w: {Y$gold Gold   {w: {Y$bankgold {cQuest Pnts{w: {C$questpoints {c| |{x\n\r"
"{r__\\____{D.dBBBb{r.________){WSilver {w: {W$silver Silver {w: {W$banksilver {cQuest Time{w: {C$questtime {c| |{x\n\r"
"{c             \\________________________________________________________________  | |{x\n\r"
"{c                                                                              `\\|{w-{c|{x\n\r"
"{B                     $affects {c(@){x\n\r"
"{c                                                                                 '{x\n\r";


char gCasterScore[] =
"\n\r{W                                   $name {cClan{w: {C$clan {c.{x\n\r"
"{w          _____{c________________________________________________________________ (@){x\n\r"
"{w        .'{B* {W*{w./ {cHRs{w:{C$hours {cLevel {w: {C$level {c| STR {w: {W$str  {c| Trains    {w: {C$trains {c\\| |{x\n\r"
"{w     __/_{W*{w_{B*{w(_  {cAge{w: {C$age {cRace  {w: {C$race {c| INT {w: {W$int  {c| Practices {w: {C$practices  {c|{w-{c|{x\n\r"
"{w    / _______ \\           {cSex   {w: {C$sex {c| WIS {w: {W$wis  {c| Experience{w: {C$experience  {c| |{x\n\r"
"{r   _{w\\{r_{w){y/___\\({r_{w/{r_          {cClass {w: {C$class {c| DEX {w: {W$dex  {c| {RExp to Lvl{w: {C$exptolevel  {c| |{x\n\r"
"{r  / _{W(({y\\- -/{W)){r_ \\         {cAlign {w: {C$align {c| CON {w: {W$con  {c|                       | |{x\n\r"
"{r  \\ \\{W())({y-{W)((){r/ /{c---------------------------------------------------------------|{w-{c|{x\n\r"
"{r   ' \\{W(((())){r/ '          {cHitPt {w: {W$hitpt{C($maxhit) {c|   {GPKills{w: {CAbove your lvl{w: {G$pkillsabove  {c| |{x\n\r"
"{r  / ' \\{W)).)){r/ ' \\         {cMana  {w: {W$mana{C($maxmana) {c|           {cBelow your lvl{w: {G$pkillsbelow  {c| |{x\n\r"
"{r / _ \\ - | - /_  \\        {cMove  {w: {W$move{C($maxmove) {c|   {RPDeths{w: {CAbove your lvl{w: {R$pdeathsabove  {c| |{x\n\r"
"{r(   ( {W.;''';. {r.'  )       {cWimp  {w: {W$wimp {c       |           {cBelow your lvl{w: {R$pdeathsbelow  {c| |{x\n\r"
"{W_{r\\\"{W__ /    )\\ {W__{r\"/{W_ {c------------------------------------------------------------|{w-{c|{x\n\r"
"{r  \\/  {W\\   ' {W/  {r\\/         {cHITroll{w: {C$hitroll {c|        Items {w: {C$items {cof {C$maxitems  {c| |{x\n\r"
"{r   .'  {W'...' {r' )          {cDAMroll{w: {C$damroll {c|        Weight{c: {C$weight {cof {C$maxweight  {c| |{x\n\r"
"{r    / /  |  \\ \\{c-----------------------------------------------------------------|{w-{c|{x\n\r"
"{r   / .   .   . \\    {cTotal mobs killed    {w: {W$percentkilledcur/$percentkilledmax {C$percentkilledpercent%  {c|    Pierce {w: {C$pierce {c| |{x\n\r"
"{r  /   .     .   \\   {cTotal rooms explored {w: {W$percentexploredcur/$percentexploredmax {C$percentexploredpercent%  {c|    Bash   {w: {C$bash {c| |{x\n\r"
"{r /   /   |   \\   \\  {cTotal objects found  {w: {W$percentfoundcur/$percentfoundmax {C$percentfoundpercent%  {c|    Slash  {w: {C$slash {c| |{x\n\r"
"{r'   /    {Db    {r'.  '.{c---------------------------------------|    {cMagic  {w: {C$magic {c| |{x\n\r"
"{r   /     {DBb     {r'-. {cYour BeltPouch{w:    {cDeposited/Bank{w:     {c+--------------------|{w-{c|{x\n\r"
"{r  |      {DBBb       {r'-. {YGold   {w: {Y$gold Gold   {w: {Y$bankgold {cQuest Pnts{w: {C$questpoints {c| |{x\n\r"
"{r__\\____{D.dBBBb{r.________){WSilver {w: {W$silver Silver {w: {W$banksilver {cQuest Time{w: {C$questtime {c| |{x\n\r"
"{c             \\________________________________________________________________  | |{x\n\r"
"{c                                                                              `\\|{w-{c|{x\n\r"
"{B                     $affects {c(@){x\n\r"
"{c                                                                                 '{x\n\r";

const  struct score_type  caster_score_table [] =
{
	{ "$name", 13, TRUE },
	{ "$clan", 25, TRUE },
	{ "$hours", 5, TRUE },
	{ "$level", 8, TRUE },
	{ "$trains", 8, FALSE },
	{ "$age", 4, TRUE },
	{ "$race", 8, TRUE },
	{ "$sex", 8, TRUE },
	{ "$class", 8, TRUE },
	{ "$align", 8, TRUE },
	{ "$str", 3, FALSE },
	{ "$int", 3, FALSE },
	{ "$wis", 3, FALSE },
	{ "$dex", 3, FALSE },
	{ "$con", 3, FALSE },
	{ "$practices", 8, FALSE },
	{ "$experience", 8, FALSE },
	{ "$exptolevel", 8, FALSE },
	{ "$hitpt", 5, FALSE },
	{ "$mana", 5, FALSE },
	{ "$move", 5, FALSE },
	{ "$wimp", 5, FALSE },
	{ "$maxhit", 5, FALSE },
	{ "$maxmana", 5, FALSE },
	{ "$maxmove", 5, FALSE },
	{ "$pkillsabove", 3, FALSE },
	{ "$pkillsbelow", 3, FALSE },
	{ "$pdeathsabove", 3, FALSE },
	{ "$pdeathsbelow", 3, FALSE },
	{ "$hitroll", 11, TRUE },
	{ "$damroll", 11, TRUE },
	{ "$items", 5, FALSE },
	{ "$maxitems", 5, FALSE },
	{ "$weight", 5, FALSE },
	{ "$maxweight", 5, FALSE },
	{ "$pierce", 6, TRUE },
	{ "$bash", 6, TRUE },
	{ "$slash", 6, TRUE },
	{ "$magic", 6, TRUE },
	{ "$questpoints", 5, FALSE },
	{ "$questtime", 5, FALSE },
	{ "$gold", 9, TRUE },
	{ "$silver", 9, TRUE },
	{ "$bankgold", 10, TRUE },
	{ "$banksilver", 10, TRUE },
	{ "$percentkilledcur", 4, FALSE },
	{ "$percentkilledmax", 4, FALSE },
	{ "$percentkilledpercent", 3, FALSE },
	{ "$percentexploredcur", 4, FALSE },
	{ "$percentexploredmax", 4, FALSE },
	{ "$percentexploredpercent", 3, FALSE },
	{ "$percentfoundcur", 4, FALSE },
	{ "$percentfoundmax", 4, FALSE },
	{ "$percentfoundpercent", 3, FALSE },
	{ "$affects", 58, TRUE },
	{ "$legend", 8, TRUE },
    { 0, 0, FALSE }
};

/* attack table  -- not very organized :( */
const 	struct attack_type	attack_table	[MAX_DAMAGE_MESSAGE]	=
{ 
 /* {   "NAME"          "NOUN"        "DAMAGE CLASS"    }  */

    { 	"none",		"HIT",		-1		},  /*  0 */
    {	"slice",	"SLICE", 	DAM_SLASH	},	
    {   "stab",		"STAB",		DAM_PIERCE	},
    {	"slash",	"SLASH",	DAM_SLASH	},
    {	"whip",		"WHIP",		DAM_SLASH	},
    {   "claw",		"CLAW",		DAM_CLAW	},  /*  5 */
    {	"blast",	"BLAST",	DAM_ENERGY	},
    {   "pound",	"POUND",	DAM_PUNCH	},
    {	"crush",	"CRUSH",	DAM_CRUSH	},
    {   "grep",		"GREP",		DAM_OTHER	},
    {	"bite",		"BITE",		DAM_BITE	},  /* 10 */
    {   "pierce",	"PIERCE",	DAM_PIERCE	},
    {   "suction",	"SUCTION",	DAM_CRUSH	},
    {	"beating",	"BEATING",	DAM_MAIM	},
    {   "digestion",	"DIGESTION",	DAM_ACID	},
    {	"charge",	"CHARGE",	DAM_BASH	},  /* 15 */
    { 	"slap",		"SLAP",		DAM_PUNCH	},
    {	"punch",	"PUNCH",	DAM_PUNCH	},
    {	"wrath",	"WRATH",	DAM_HARM	},
    {	"magic",	"MAGIC",	DAM_MAGIC	},
    {   "divine",	"DIVINE POWER",	DAM_HOLY	},  /* 20 */
    {	"cleave",	"CLEAVE",	DAM_CLEAVE	},
    {	"scratch",	"SCRATCH",	DAM_CLAW	},
    {   "peck",		"PECK",		DAM_PIERCE	},
    {   "peckb",	"PECK-BASH",	DAM_BASH	},
    {   "chop",		"CHOP",		DAM_CLEAVE	},  /* 25 */
    {   "sting",	"STING",	DAM_PIERCE	},
    {   "smash",	"SMASH",	DAM_CRUSH	},
    {   "shbite",	"SHOCKING BITE",DAM_LIGHTNING	},
    {	"flbite",	"FLAMING BITE", DAM_FIRE	},
    {	"frbite",	"FREEZING BITE",DAM_COLD	},  /* 30 */
    {	"acbite",	"ACIDIC BITE", 	DAM_ACID	},
    {	"chomp",	"CHOMP",	DAM_CRUSH	},
    {  	"drain",	"LIFE DRAIN",	DAM_NEGATIVE	},
    {   "thrust",	"THRUST",	DAM_PIERCE	},
    {   "slime",	"SLIME",	DAM_DROWNING	},
    {	"shock",	"SHOCK",	DAM_LIGHTNING	},
    {   "thwack",	"THWACK",	DAM_PUNCH	},
    {   "flame",	"FLAME",	DAM_FIRE	},
    {   "chill",	"CHILL",	DAM_COLD	},
    {   "maim", 	"MAIM" ,	DAM_MAIM	},
    {   NULL,		NULL,		0		}
};

/* race table */
const 	struct	race_type	race_table	[]		=
{

/*
    {
	name,		pc_race?,
	act bits,	aff_by bits,	off bits,
	imm,		res,		vuln,
	form,		parts,		remort_race?,
	aff2_by
    },
*/

    { "unique",		FALSE, 0, 0, 0, 0, 0, 0, 0, 0,FALSE,0},

    {
	"Human",		TRUE,
	0,		0, 		0,
	0, 		0,		0,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K, FALSE,
	0
    },
    {
	"Elf",			TRUE,
        0,        AFF_INFRARED,   0,
        0,        RES_MENTAL|RES_DISEASE,      VULN_IRON,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K, FALSE,
	0
    },
    {
	"Succubus",		TRUE,
        0,              AFF_FLYING|AFF_DETECT_INVIS|AFF_DETECT_HIDDEN, 0,
        0,              RES_FIRE|RES_POISON,   VULN_HOLY,
	A|B|H|M,	A|B|C|D|E|F|G|H|I|J|K|P|Q|U|V, FALSE,
	0
    },
    {
	"Inccubus",             TRUE,
        0,              AFF_HASTE|AFF_FLYING,  0,
        0,              RES_POISON|RES_DISEASE,  VULN_HOLY,
	A|B|H|M,          A|B|C|D|E|F|G|H|I|J|K|P|Q|U|V, FALSE,
	0
    },
    {
	"Cambion",             TRUE,
        0,              AFF_DETECT_GOOD|AFF_DETECT_INVIS|AFF_FLYING,  0,
        IMM_POISON|IMM_MENTAL,	 RES_FIRE, 	VULN_HOLY,
	A|B|C|H|M|cc,          A|B|C|D|E|F|G|H|I|J|K|P|Q|U|V|W|X, FALSE,
	0
    },
    {
	"Dwarf",                TRUE,
        0,              AFF_DETECT_HIDDEN,   0,
        0,              RES_POISON|RES_ENERGY, VULN_MENTAL,
	A|H|M|V,        A|B|C|D|E|F|G|H|I|J|K, FALSE,
	0
    },
    {
	"Giant",		TRUE,
        0,              AFF_INFRARED,              0,
        0,              RES_SLASH, 	VULN_MENTAL,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K, FALSE,
	0
    },
    {
       "Draconian",               TRUE,
        0, AFF_INFRARED|AFF_FLYING,    0,
        0, RES_ACID|RES_FIRE|RES_LIGHTNING, 0,
	A|B|C|H|M|Z|cc, A|B|C|D|E|F|G|H|I|J|K|P|Q|U|V|X, FALSE,
	0
    },
    {
	"Seraph",           TRUE,
         0,  AFF_FLYING|AFF_DETECT_HIDDEN,        0,
         IMM_HOLY|IMM_LIGHTNING,	 RES_DISEASE,	VULN_NEGATIVE,
          A|H|M|V,  A|B|C|D|E|F|G|H|I|J|K, FALSE,
	0
    },
    
    {
       "Kender",  TRUE,
        0, AFF_DETECT_HIDDEN|AFF_HASTE, 0,
        0, RES_DISEASE|RES_POISON, VULN_MENTAL,
           A|H|M|V,        A|B|C|D|E|F|G|H|I|J|K, FALSE,
	0
          
    },  
    
    {
       "Troll",  TRUE,
        0,    AFF_INFRARED, 0,
        0, RES_PIERCE|RES_COLD, VULN_FIRE,
            A|B|H|M, A|B|C|D|E|F|G|H|I|J|K|U|Y, FALSE,
	0
    },

    {
       "DarkElf",  TRUE,
        0,     AFF_DETECT_HIDDEN, 0,
        IMM_POISON|IMM_NEGATIVE, RES_DISEASE,  VULN_LIGHT2|VULN_HOLY,
            A|H|M|V, A|B|C|D|E|F|G|H|I|J|K, FALSE,
	0
    },
    
    {
       "Lizardman",  TRUE,
        0, AFF_INFRARED|AFF_SWIM, 0,
        IMM_POISON,RES_MENTAL,  0,
            A|H|M|X|aa|cc, A|B|C|D|E|F|G|H|I|J|K|Q|U|X, FALSE,
	0
    },

    {
       "Golem",  TRUE,
        0,  AFF_REGENERATION, 0,
        IMM_POISON|IMM_DISEASE|IMM_DROWNING,
        RES_PIERCE,  VULN_LIGHTNING|VULN_ENERGY,
            C|H|J|M, A|B|C|G|H|I|J|K, FALSE,
	0
    },

    {
       "Lamia",  TRUE,
        0, AFF_DETECT_INVIS, 0,
        0, RES_COLD|RES_LIGHTNING,  VULN_FIRE,
            A|B|C|G|H|Y|cc, A|B|D|E|F|G|H|I|J|K|L|Q|U|V, FALSE,
	0
    },
    
    {
       "Titan",  TRUE,
        0,   AFF_REGENERATION, 0,
        IMM_POISON|IMM_DISEASE, RES_FIRE,
        VULN_COLD,	
            A|C|H|M|V, A|B|C|D|E|F|G|H|I|J|K, FALSE,
	0
    },

    {
       "Minotaur",  TRUE,
        0, AFF_BERSERK, 0,
        IMM_BASH, RES_POISON|RES_DISEASE,
        VULN_MAGIC,	
            A|B|C|G|H|M, A|B|C|D|E|F|G|H|I|J|K|W|Y, FALSE,
	0
    },

    {
       "Sidhe Elf", TRUE, 
        0,
        AFF_DETECT_INVIS|AFF_HASTE|AFF_DETECT_HIDDEN,  0,
        IMM_NEGATIVE|IMM_MENTAL|RES_COLD,RES_POISON|RES_DISEASE,
        0,
           A|C|H|M|V,  A|B|C|D|E|F|G|H|I|J|K,TRUE,
	0
    },

   {
          "Nephilim",             FALSE,
   0, AFF_DETECT_INVIS|AFF_DETECT_HIDDEN|AFF_HASTE, 0,
   IMM_BASH|IMM_ENERGY|IMM_POISON|IMM_MENTAL|IMM_HOLY|IMM_NEGATIVE,
   RES_FIRE|RES_LIGHTNING|RES_MAGIC|RES_LIGHTNING|RES_ACID|RES_PIERCE,
	0,	  
            C|H|M|cc,        A|B|C|D|E|F|G|H|I|J|K, FALSE,
	0
    },

    {   
        "Myrddraal",               TRUE,  
0,AFF_FLYING|AFF_DETECT_INVIS|AFF_DETECT_HIDDEN|AFF_PASS_DOOR|AFF_REGENERATION|AFF_HASTE,
          0,
          IMM_BASH|IMM_HOLY|IMM_POISON|IMM_DISEASE|IMM_DROWNING,
          RES_MENTAL|RES_ACID,
          VULN_SILVER|VULN_IRON,	
          B|C|I|L|M, A|B|C|G|H|I|U|V|W, TRUE,
	0
    },

    {
	"bat",			FALSE,
	0,		AFF_FLYING|AFF_DARK_VISION,	OFF_DODGE|OFF_FAST,
	0,		0,		VULN_LIGHT2,	
	A|G|V,		A|C|D|E|F|H|J|K|P, FALSE,
	0
    },

    {
	"bear",			FALSE,
	0,		0,		OFF_CRUSH|OFF_DISARM|OFF_BERSERK,
	0,		RES_BASH|RES_COLD,	0,	
	A|G|V,		A|B|C|D|E|F|H|J|K|U|V, FALSE,
	0
    },

    {
	"cat",			FALSE,
	0,		AFF_DARK_VISION,	OFF_FAST|OFF_DODGE,
	0,		0,		0,	
	A|G|V,		A|C|D|E|F|H|J|K|Q|U|V, FALSE,
	0
    },

    {
	"centipede",		FALSE,
	0,		AFF_DARK_VISION,	0,
	0,		RES_PIERCE|RES_COLD,0,	
 	A|B|G|O,		A|C|K	, FALSE,
	0
    },

    {
	"dog",			FALSE,
	0,		0,		OFF_FAST,
	0,		0,		0,	
	A|G|V,		A|C|D|E|F|H|J|K|U|V, FALSE,
	0
    },

    {
	"doll",			FALSE,
	0,		0,		0,
	IMM_COLD|IMM_POISON|IMM_HOLY|IMM_NEGATIVE|IMM_MENTAL|IMM_DISEASE
	|IMM_DROWNING,	RES_BASH|RES_LIGHT2,	0,
	E|J|M|cc,	A|B|C|G|H|K, FALSE,
	0
    },

    { 	"dragon", 		FALSE, 
	0, 			AFF_INFRARED|AFF_FLYING,	0,
	0,			RES_FIRE|RES_BASH|RES_CHARM, 
	0,
	A|H|Z,		A|C|D|E|F|G|H|I|J|K|P|Q|U|V|X, FALSE,
	0
    },

    {
	"fido",			FALSE,
	0,		0,		OFF_DODGE|ASSIST_RACE,
	0,		0,		0,
	A|B|G|V,	A|C|D|E|F|H|J|K|Q|V, FALSE,
	0
    },		
   
    {
	"fox",			FALSE,
	0,		AFF_DARK_VISION,	OFF_FAST|OFF_DODGE,
	0,		0,		0,	
	A|G|V,		A|C|D|E|F|H|J|K|Q|V, FALSE,
	0
    },

    {
	"goblin",		FALSE,
	0,		AFF_INFRARED,	0,
	0,		RES_DISEASE,	0,	
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K, FALSE,
	0
    },

    {
	"hobgoblin",		FALSE,
	0,		AFF_INFRARED,	0,
	0,		RES_DISEASE|RES_POISON,	0,
	A|H|M|V,        A|B|C|D|E|F|G|H|I|J|K|Y, FALSE,
	0
    },

    {
	"kobold",		FALSE,
	0,		AFF_INFRARED,	0,
	0,		RES_POISON,	0,
	A|B|H|M|V,	A|B|C|D|E|F|G|H|I|J|K|Q, FALSE,
	0
    },

    {
	"Laerkai",		FALSE,
	ACT_STAY_AREA|ACT_NOPURGE,
	AFF_DETECT_INVIS|AFF_DETECT_HIDDEN|AFF_PROTECT_GOOD|AFF_HASTE,
	OFF_DODGE|OFF_FADE|ASSIST_RACE,
	IMM_POISON|IMM_CHARM|IMM_NEGATIVE,
	RES_SOUND|RES_ELEMENTAL|RES_PIERCE|RES_SLASH,
	VULN_LIGHT2|VULN_HOLY,	
	B|C|H|M|V,	A|B|C|E|F|G|H|I|J|K, FALSE,
	0
    },

    {
	"lizard",		FALSE,
	0,		0,		0,
	0,		RES_POISON,	0,
	A|G|X|cc,	A|C|D|E|F|H|K|Q|V, FALSE,
	0
    },

    {
	"modron",		FALSE,
	0,		AFF_INFRARED,		ASSIST_RACE|ASSIST_ALIGN,
	IMM_CHARM|IMM_DISEASE|IMM_MENTAL|IMM_HOLY|IMM_NEGATIVE,
			RES_FIRE|RES_COLD|RES_ACID,	0,
	H,		A|B|C|G|H|J|K, FALSE,
	0
    },

    {
	"orc",			FALSE,
	0,		AFF_INFRARED,	0,
	0,		RES_DISEASE,	0,	
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K, FALSE,
	0
    },

    {
	"pig",			FALSE,
	0,		0,		0,
	0,		0,		0,
	A|G|V,	 	A|C|D|E|F|H|J|K, FALSE,
	0
    },	

    {
	"rabbit",		FALSE,
	0,		0,		OFF_DODGE|OFF_FAST,
	0,		0,		0,
	A|G|V,		A|C|D|E|F|H|J|K, FALSE,
	0
    },
    
    {
	"school monster",	FALSE,
	ACT_NOALIGN,		0,		0,
	IMM_CHARM|IMM_SUMMON,	0,		0,
	A|M|V,		A|B|C|D|E|F|H|J|K|Q|U, FALSE,
	0
    },	

    {
	"snake",		FALSE,
	0,		0,		0,
	0,		RES_POISON,	0,
	A|G|X|Y|cc,	A|D|E|F|K|L|Q|V|X, FALSE,
	0
    },
 
    {
	"song bird",		FALSE,
	0,		AFF_FLYING,		OFF_FAST|OFF_DODGE,
	0,		0,		0,
	A|G|W,		A|C|D|E|F|H|K|P, FALSE,
	0
    },

    {
	"troll",		FALSE,
	0,		AFF_REGENERATION|AFF_INFRARED|AFF_DETECT_HIDDEN,
	OFF_BERSERK,
 	0,	RES_CHARM|RES_BASH,	VULN_FIRE,
	A|B|H|M|V,		A|B|C|D|E|F|G|H|I|J|K|U|V, FALSE,
	0
    },

    {
	"water fowl",		FALSE,
	0,		AFF_SWIM|AFF_FLYING,	0,
	0,		RES_DROWNING,		0,
	A|G|W,		A|C|D|E|F|H|K|P, FALSE,
	0
    },		
  
    {
	"wolf",			FALSE,
	0,		AFF_DARK_VISION,	OFF_FAST|OFF_DODGE,
	0,		0,		0,
	A|G|V,		A|C|D|E|F|J|K|Q|V, FALSE,
	0
    },

    {
	"wyvern",		FALSE,
	0,		AFF_FLYING|AFF_DETECT_INVIS|AFF_DETECT_HIDDEN,
	OFF_BASH|OFF_FAST|OFF_DODGE,
	IMM_POISON,		0,		0,
	A|B|G|Z,		A|C|D|E|F|H|J|K|Q|V|X, FALSE,
	0
    },

    {
	"unique",		FALSE,
	0,		0,		0,
	0,		0,		0,		
	FALSE,		0
    },


    {
	NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    }
};

const	struct	pc_race_type	pc_race_table	[]	=
{
    { "null race", "", 0, 
    { 100, 100, 100, 100 },
      { "" }, 
    { 13, 13, 13, 13, 13 }, { 18, 18, 18, 18, 18 }, 0, "", "", "" },
 
/*
    {
	"race name", 	short name, 	points,	{ class multipliers },
	{ bonus skills },
	{ base stats },		{ max stats },		size,
	"classes availble", "aligns availble", "female" or "male" or "Both"
    },
*/
      {
	"Human", "Hum", 0,
          { 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100 },
	  { "" },
	  { 13, 13, 13, 13, 13 },{ 18, 18, 18, 18, 18 },	SIZE_MEDIUM,
	"All", "All", "All"
      },

      {
              "Elf", "Elf", 8,
                 { 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100 },
         { "hide", "meditation" },
        { 12, 14, 14, 15, 11 }, { 17, 19, 19, 20, 16 }, SIZE_SMALL,
	"All", "All", "All"
      },

      {
        "Succubus", "Suc",          15,
        { 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100 },
        { "charm person", "energy drain" },
        { 14, 12, 12, 14, 14 }, { 20, 17, 17, 20, 19 }, SIZE_MEDIUM,
	"Mag, Thi, War, Apl, Asn", "Evil", "F"
      },

      {
        "Inccubus", "Inc",       16,     
        { 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100 },
        { "charm person", "flamestrike" },
	  { 14, 10, 12, 15, 14 }, { 20, 15, 17, 20, 19 }, SIZE_MEDIUM,
	"Mag, Thi, War, Apl, Asn", "Evil", "M"
      },

      {
        "Cambion", "Cam",     17,
        { 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100 },
        { "demonfire", "curse" },
        { 16, 8, 11, 14, 13 }, { 21, 13, 17, 20, 19 }, SIZE_LARGE,
	"Mag, Cle, Thi, War, Asn, Enc", "Evil", "All"
      },

      {
        "Dwarf", "Dwf",          8,
        { 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100 },
        { "berserk", "dodge" },
        { 14, 11, 13, 11, 16 }, { 19, 16, 18, 16, 21 }, SIZE_MEDIUM,
	"Cle, Thi, War, Pal, Apl, Asn, Enc", "All", "All"
      },

      { 
        "Giant", "Gia",       4,
        { 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100 },
        { "bash", "enhanced damage" },
        { 16, 10, 10, 12, 17 }, { 20, 15, 15, 17, 22 }, SIZE_HUGE,
	"War, Pal, Apl", "All", "All"
	},

	{
        "Draconian", "Dra",          14,     
        { 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100 },
        { "fast healing", "bash" },
        { 17, 12, 12, 10, 14 }, { 22, 17, 17, 15, 19 }, SIZE_LARGE,
	"Mag, Cle, Psi, Thi, Asn, War, Apl", "Evil", "All"
	},

      {
        "Seraph", "Sra",             15,
        { 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100 },
        { "enhanced damage", "fast healing" },
        { 16, 14, 16, 11, 13 }, {19, 19, 20, 15, 20 }, SIZE_MEDIUM,
	"Mag, Cle, War, Pal", "Good", "All"
      },
         
	{
        "Kender", "Ken",   10,
        { 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100 },
        { "steal", "peek" },
        { 10, 13, 13, 18, 11 }, { 15, 18, 18, 23, 16 }, SIZE_SMALL,
	"Thi, War, Asn, Ran", "All", "All"
                
      },
         
      { 
        "Troll", "Tro",         7,
        { 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100 },
        { "second attack", "enhanced damage" },
        { 15, 9, 10, 13, 18 }, { 20, 14, 15, 18, 23 }, SIZE_HUGE,
	"War, Asn", "Evil, Neutral", "All"
	},

      {
        "Darkelf", "DEl",       9,
        { 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100 },
        { "sneak", "detect hidden" },
        { 12, 15, 14, 14, 10 }, { 17, 20, 19, 19, 15 }, SIZE_MEDIUM,
	"Mag, Cle, Asn, War, Apl, Enc", "Evil", "All"
      },

      {
        "Lizardman", "Liz",          9,
        { 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100 },
        { "fast healing", "trip" },
        { 14, 12, 12, 15, 15 }, { 19, 17, 17, 20, 20 }, SIZE_MEDIUM,
	"Cle, Thi, War, Asn, Ran", "Evil, Neutral", "M,F"
      },

      {
        "Golem", "Gol",             14,
        { 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100 },
        { "armor", "stone skin"},
        { 22, 10, 10, 8, 18 }, { 27, 15, 15, 13, 21 }, SIZE_HUGE,
	"War, Pal, Apl, Asn", "All", "All"
      },

      {
        "Lamia", "Lam",            9,
        { 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100 },
        { "acid blast", "stone skin" },
        { 13, 13, 13, 16, 16 }, { 18, 18, 18, 21, 21 }, SIZE_LARGE,
	"Mag, Cle, War, Asn", "Evil, Neutral", "F"
      },
  
      {
        "Titan", "Tit",        10,
        { 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100 },
        { "bash", "hand to hand" },
        { 16, 15, 15, 11, 13 }, { 21, 20, 20, 16, 18 }, SIZE_GIANT,
	"Mag, Cle, War, Pal, Apl, Enc", "All", "All"
      },

      {
        "Minotaur", "Min",       9,
        { 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100 },
        { "frenzy", "giant strength" },
        { 17, 9, 9, 15, 16 }, { 22, 14, 14, 20, 21 }, SIZE_HUGE,
	"War", "Evil, Neutral", "M"
      },

      {                
        "Sidhe Elf",   "Sid",    26,
        { 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100 },
        {  "word of recall", "metamorph", "spellcraft" },
        { 12, 16, 16, 16, 12 }, { 18, 27, 27, 22, 18 }, SIZE_MEDIUM,
         "Mag, Cle, Pal, Psi, Ran, Enc", "Good, Neutral", "All"
      },

      {
         "Nephilim",  "Nep",  0,
        { 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100 },
        { "warrior default", "fourth attack" },
        { 18, 18, 18, 18, 18 }, { 26, 26, 26, 26, 26 }, SIZE_HUGE,
          "War","Neutral", "All"
      },

      {
        "Myrddraal", "Myr", 35,     
        { 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100 },
        { "portal","gas breath","curse","assassinate","backstab","bash"},
        { 20, 10, 10, 20, 18 }, { 27, 17, 17, 27, 25 }, SIZE_LARGE,
	"Thi, Asn, War, Apl", "Evil", "N"
      },


	{ NULL, "", 0, 
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{},{0,0,0,0,0},{0,0,0,0,0}, 0, "", "", ""},    
};


/*
 * Class table.
 */

const	struct	class_type	class_table	[MAX_CLASS]	=
{
/*
	{
	 "Nullclass", "NCl",  STAT_CON,  OBJ_VNUM_DAGGER,
	 { 3055, 9618 },  75,  20, 6,  6,  8, TRUE,
	 "", "", TRUE, "", "", ""
	},
*/
	{
	 "Mage", "Mag",  STAT_INT,  OBJ_VNUM_SCHOOL_DAGGER,
	 { 3018,5699,9648,27 },  75,  20, 6,  6,  8, TRUE,
	 "mage basics", "mage default", FALSE, "All", "All",
	 "Poor fighter, great off. spells."
	},

	{
	 "Cleric", "Cle",  STAT_WIS,  OBJ_VNUM_SCHOOL_MACE,
	 { 3003,9603,18111,28 },  75,  20, 2, 7 , 10, TRUE,
	 "cleric basics", "cleric default", FALSE, "All", "All",
	 "Great healing+enchant spells."
	},

      {
       "Psionic", "Psi",  STAT_INT,  OBJ_VNUM_SCHOOL_DAGGER,
       { 3213,31426,31490,27 },  75,  20, 4,  7, 10, TRUE,
       "psionic basics", "psionic default", TRUE, "All", "All",
	"Good offensive spells+fighter."
      },

	{
	 "Thief", "Thi",  STAT_DEX,  OBJ_VNUM_SCHOOL_DAGGER,
	 { 3029,5633,9646,29 },  75,  20,  -4,  8, 12, FALSE,
	 "thief basics", "thief default", FALSE, "Evil, Neutral", "All",
	"Sneaky fighters. Poor spells."
	},
	
      {
       "Assassin", "Asn",  STAT_DEX,  OBJ_VNUM_SCHOOL_DAGGER,
       { 3212,16414,18113,29 },  75,  20,  -6,  9, 14, FALSE,
       "assassin basics", "assassin default", FALSE, "Evil", "All",
	"Brutal fighters. Hit and run."
      },

	{
	 "Warrior", "War",  STAT_STR,  OBJ_VNUM_SCHOOL_SWORD,
	 { 3022,5613,31476,29 },  75,  20,  -12,  14, 18, FALSE,
	 "warrior basics", "warrior default", FALSE, "All", "All",
	"Highly offensive warrior fighter."
	},

      {
       "Paladin", "Pal",  STAT_STR,  OBJ_VNUM_SCHOOL_SWORD,
       { 3208,18111,31487,29 },  75,  20,   -8,  12, 16, FALSE,
       "paladin basics", "paladin default", FALSE, "Good", "All",
	"Honest+holy, good fighter+healer."
      },

      {
       "AntiPaladin", "Apl",  STAT_STR,  OBJ_VNUM_SCHOOL_SWORD,
       { 3209,9634,18113,29 },  75,  20,   -8,  12, 16, FALSE,
       "antipaladin basics", "antipaladin default", FALSE, "Evil", "All",
	"Evil, good fighter."
      },

      {
       "Ranger", "Ran",  STAT_INT,  OBJ_VNUM_SCHOOL_SWORD,
	 { 3207,31435,31458,29 },  75,  20,  -10,  11, 15, FALSE,
       "ranger basics", "ranger default", TRUE, "Good, Neutral", "All",
	"Woodsman, second best fighter."
      },

      {
       "Enchanter", "Enc",  STAT_INT,  OBJ_VNUM_SCHOOL_DAGGER,
       {3018,5699,9648,27 },  75,  20, 10,  4,  10, TRUE,
       "enchanter basics", "enchanter default", TRUE, "All", "All", ""
      }
};

/*
 * Attribute bonus tables.
 */
const	struct	str_app_type	str_app		[41]		=
{
    { -5, -4,   0,  0 },  /* 0  */
    { -5, -4,   30,  1 },  /* 1  */
    { -3, -2,   30,  2 },
    { -3, -1,  100,  3 },  /* 3  */
    { -2, -1,  250,  4 },
    { -2, -1,  550,  5 },  /* 5  */
    { -1,  0,  800,  6 },
    { -1,  0,  900,  7 },
    {  0,  0, 1000,  8 },
    {  0,  0, 1000,  9 },
    {  0,  0, 1150, 10 }, /* 10  */
    {  0,  0, 1150, 11 },
    {  0,  0, 1300, 12 },
    {  0,  0, 1300, 13 }, /* 13  */
    {  0,  1, 1400, 14 },
    {  1,  1, 1500, 15 }, /* 15  */
    {  1,  2, 1650, 16 },
    {  2,  3, 1800, 22 },
    {  2,  3, 2000, 25 }, /* 18  */
    {  3,  4, 2250, 30 },
    {  3,  5, 2500, 35 }, /* 20  */
    {  4,  6, 3000, 40 },
    {  4,  6, 3500, 45 },
    {  5,  7, 4000, 50 },
    {  5,  8, 4500, 55 },
    {  6,  9, 5000, 60 }, /* 25   */
    {  6, 10, 5500, 65 },
    {  7, 11, 6000, 70 },
    {  8, 12, 6500, 75 },
    {  9, 13, 7000, 80 },
    { 10, 15, 7500, 85 }, /* 30  */
    { 11, 16, 8000, 90 },
    { 12, 17, 8500, 95 },
    { 13, 18, 9000, 100 },
    { 14, 19, 9500, 105 },
    { 15, 20, 10000, 110 },  /* 35   */
    { 18, 23, 11000, 120 },
    { 21, 26, 12000, 130 },
    { 24, 29, 13000, 140 },
    { 27, 32, 14000, 150 },
    { 30, 35, 15000, 160 }  /* 40   */



};



const	struct	int_app_type	int_app		[41]		=
{
    {  3 },	/*  0 */
    {  5 },	/*  1 */
    {  7 },
    {  8 },	/*  3 */
    {  9 },
    { 10 },	/*  5 */
    { 11 },
    { 12 },
    { 13 },
    { 15 },
    { 17 },	/* 10 */
    { 19 },
    { 22 },
    { 25 },
    { 28 },
    { 31 },	/* 15 */
    { 34 },
    { 37 },
    { 40 },	/* 18 */
    { 44 },
    { 49 },	/* 20 */
    { 55 },
    { 60 },
    { 70 },
    { 80 },
    { 85 },	/* 25 */
    { 90 },
    { 90 },
    { 90 },
    { 90 },
    { 95 },	/* 30 */
    { 95 },
    { 95 },
    { 95 },	
    { 95 },
    { 100 },	/* 35 */
    { 100 },
    { 100 },
    { 100 },
    { 100 },
    { 100 }	/* 40 */

};



const	struct	wis_app_type	wis_app		[41]		=
{
    { 0 },	/*  0 */
    { 0 },	/*  1 */
    { 0 },
    { 0 },	/*  3 */
    { 0 },
    { 0 },	/*  5 */
    { 0 },
    { 0 },
    { 0 },
    { 1 },      /*  9 */
    { 1 },	
    { 1 },
    { 1 },
    { 1 },
    { 1 },      /* 14 */
    { 2 },	/* 15 */
    { 2 },
    { 2 },
    { 2 },	
    { 2 },      /* 19 */
    { 3 },	/* 20 */
    { 3 },
    { 3 },
    { 3 },      /* 23 */
    { 4 },      /* 24 */
    { 4 },	/* 25 */
    { 4 },
    { 5 },
    { 5 },
    { 5 },      
    { 6 },	/* 30 */
    { 6 },
    { 6 },
    { 7 },	
    { 7 },     
    { 7 },	/* 35 */
    { 8 },
    { 8 },
    { 9 },     
    { 9 },      
    { 10 }	/* 40 */
};


const	struct	dex_app_type	dex_app		[41]		=
{
    {   60 },   /* 0 */
    {   50 },   /* 1 */
    {   50 },
    {   40 },
    {   30 },
    {   20 },   /* 5 */
    {   10 },
    {    0 },
    {    0 },
    {    0 },
    {    0 },   /* 10 */
    {    0 },
    {    0 },
    {    0 },
    {    0 },
    { - 10 },   /* 15 */
    { - 15 },
    { - 20 },
    { - 30 },
    { - 40 },
    { - 50 },   /* 20 */
    { - 60 },
    { - 75 },
    { - 90 },
    { -105 },
    { -120 },    /* 25 */
    { -135 },
    { -150 },
    { -165 },
    { -180 },
    { -200 },   /* 30 */
    { -220 },
    { -240 },
    { -260 },
    { -280 },
    { -300 },   /* 35 */
    { -325 },
    { -350 },
    { -375 },
    { -400 },
    { -450 }    /* 40 */
};


const	struct	con_app_type	con_app		[41]		=
{
    { -4, 20 },   /*  0 */
    { -3, 25 },   /*  1 */
    { -2, 30 },
    { -2, 35 },	  /*  3 */
    { -1, 40 },
    { -1, 45 },   /*  5 */
    { -1, 50 },
    {  0, 55 },
    {  0, 60 },
    {  0, 65 },
    {  0, 70 },   /* 10 */
    {  0, 75 },
    {  0, 80 },
    {  0, 85 },
    {  0, 88 },
    {  1, 90 },   /* 15 */
    {  2, 95 },
    {  2, 97 },
    {  3, 99 },   /* 18 */
    {  3, 99 },
    {  4, 99 },   /* 20 */
    {  4, 99 },
    {  5, 99 },
    {  6, 99 },
    {  7, 99 },
    {  8, 99 },    /* 25 */
    {  9, 99 },
    { 10, 99 },
    { 12, 99 },
    { 14, 99 },
    { 16, 99 },   /* 30 */
    { 18, 99 },
    { 20, 99 },
    { 22, 99 },   
    { 24, 99 },
    { 26, 100 },   /* 35 */
    { 28, 100 },
    { 30, 100 },
    { 32, 100 },
    { 34, 100 },
    { 40, 100 }    /* 40 */

};



/*
 * Liquid properties.
 * Used in world.obj.
 */
const	struct	liq_type	liq_table	[]	=
{
/*    name			color	proof, full, thirst, food, ssize */
    { "water",			"clear",	{   0, 1, 10, 0, 16 }	},
    { "beer",			"amber",	{  12, 1,  8, 1, 12 }	},
    { "red wine",			"burgundy",	{  30, 1,  8, 1,  5 }	},
    { "ale",			"brown",	{  15, 1,  8, 1, 12 }	},
    { "dark ale",			"dark",		{  16, 1,  8, 1, 12 }	},

    { "whisky",			"golden",	{ 120, 1,  5, 0,  2 }	},
    { "lemonade",			"pink",		{   0, 1,  9, 2, 12 }	},
    { "firebreather",		"boiling",	{ 190, 0,  4, 0,  2 }	},
    { "local specialty",	"clear",	{ 151, 1,  3, 0,  2 }	},
    { "slime mold juice",	"green",	{   0, 2, -8, 1,  2 }	},

    { "milk",			"white",	{   0, 2,  9, 3, 12 }	},
    { "tea",			"tan",		{   0, 1,  8, 0,  6 }	},
    { "coffee",			"black",	{   0, 1,  8, 0,  6 }	},
    { "blood",			"red",		{   0, 2, -1, 2,  6 }	},
    { "salt water",		"clear",	{   0, 1, -2, 0,  1 }	},

    { "coke",			"brown",	{   0, 2,  9, 2, 12 }	}, 
    { "root beer",		"brown",	{   0, 2,  9, 2, 12 }   },
    { "elvish wine",		"green",	{  35, 2,  8, 1,  5 }   },
    { "white wine",		"golden",	{  28, 1,  8, 1,  5 }   },
    { "champagne",		"golden",	{  32, 1,  8, 1,  5 }   },

    { "mead",			"honey-colored",{  34, 2,  8, 2, 12 }   },
    { "rose wine",		"pink",		{  26, 1,  8, 1,  5 }	},
    { "benedictine wine",	"burgundy",	{  40, 1,  8, 1,  5 }   },
    { "vodka",			"clear",	{ 130, 1,  5, 0,  2 }   },
    { "cranberry juice",	"red",		{   0, 1,  9, 2, 12 }	},

    { "orange juice",		"orange",	{   0, 2,  9, 3, 12 }   }, 
    { "absinthe",			"green",	{ 200, 1,  4, 0,  2 }	},
    { "brandy",			"golden",	{  80, 1,  5, 0,  4 }	},
    { "aquavit",			"clear",	{ 140, 1,  5, 0,  2 }	},
    { "schnapps",			"clear",	{  90, 1,  5, 0,  2 }   },

    { "icewine",			"purple",	{  50, 2,  6, 1,  5 }	},
    { "amontillado",		"burgundy",	{  35, 2,  8, 1,  5 }	},
    { "sherry",			"red",		{  38, 2,  7, 1,  5 }   },	
    { "framboise",		"red",		{  50, 1,  7, 1,  5 }   },
    { "rum",			"amber",	{ 151, 1,  4, 0,  2 }	},

    { "cordial",			"clear",	{ 100, 1,  5, 0,  2 }   },
    { NULL,				NULL,		{   0, 0,  0, 0,  0 }	}
};



/*
 * The skill and spell table.
 * Slot numbers must never be changed as they appear in #OBJECTS sections.
 */
#define SLOT(n)	n

const	struct	skill_type	skill_table	[MAX_SKILL]	=
{

/****  SPELLS SECTION ****/

/***  An Explanation of what the spell/skill format means:
 ***  These definitions fit if you start at the name and read
 ***  left to right, one row at a time.

 The name of the spell or skill,

 Level needed by class to gain.  must have number of slots equal to MAX_CLASS in MERC.H
 i.e. if MAX_CLASS = 10 it would look like this: { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },

 How hard it is to learn the skill by class.  The higher the number the harder. Must
 follow the rules above for MAX_CLASS.  i.e. { 1, 2, 3, 4, 1, 2, 3, 4, 3, 2 },

 SPELL_FUN.  Place the name of the associated spell you want to point this one to. 
 (this is used for SPELLS ONLY) if not a spell put 'spell_null' here.

 This is the MINIMUM POSITION the caster/user must be in to use the skill/spell.

 This points to the associated gsn (global skill name).  If this was the skill
 Berserk, you type it in as such:   &gsn_berserk .  Very few spells also have an
 associated GSN, if the spell does not, put NULL here.  If it does, put in the
 appropriate GSN.

 This is the SLOT for #OBJECT loading.  (not sure what this means, researching)

 This is the MINIMUM amount of mana that will be used for this skill or spell.

 This is the amount of WAIT time a player has after using the skill or spell.

 This is the DAMAGE message that will be shown.
 
 This is the message someone sees when the(an) effect from the spell or skill
 wears off.

 This is the same as above but it is the wear off message for objects.
 
 Legend level, legend train cost, legend questpoint cost, legend gold cost

 ***
 *** Hope this helps.
 ***/

    {
	"reserved",	
        { 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101 },
	{ 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99},
	0,			TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT( 0),	 0,	 0,
	"",			"",		"",
	-1, -1, -1, -1
    },

/*  Mage Only Spells */

/* Combat Spells */

    {
      "magic missile",
      { 1, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101 },
      { 2,  4,  2,  6, 6, 8, 6, 6, 8, 8, 8, 8, 8, 8, 8 },
	spell_magic_missile,	TAR_CHAR_OFFENSIVE, POS_FIGHTING,
	NULL,			SLOT(32),	3,	8,
	"magic missile",	"!Magic Missile!",	"",
	1, 1, 600, 1000
    },

    {
	"chill touch",
      { 5, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101 },
      { 2,  4,  2,  6, 6, 8, 6, 6, 8, 8, 8, 8, 8, 8, 8 },
	spell_chill_touch,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT( 8),	8,	15,
	"chilling touch",	"You feel less cold.",	"",
	1, 1, 600, 100
    },

    {
	"burning hands",	
      {  10, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101 },  
      { 2,  4,  2,  6, 6, 8, 6, 6, 8, 8, 8, 8, 8, 8, 8 },
	spell_burning_hands,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT( 5),	12,	12,
	"burning hands",	"!Burning Hands!", 	"",
	1, 1, 600, 1000
    },  
    {
	"shocking grasp",
      {  16, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101 },
      { 2,  4,  2,  6, 6, 8, 6, 6, 8, 8, 8, 8, 8, 8, 8 },
	spell_shocking_grasp,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(53),	13,	15,
	"shocking grasp",	"!Shocking Grasp!",	"",
	1, 1, 600, 1000
    },

    {
	"colour spray",
      { 22, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101 },
      { 2,  4,  2,  6, 6, 8, 6, 6, 8, 8, 8, 8, 8, 8, 8 },
	spell_colour_spray,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(10),	15,	15,
	"colour spray",		"!Colour Spray!",	"",
	1, 1, 600, 1000
    },    

    {
	"fireball",
      { 26, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101 },
      { 2,  4,  2,  6, 6, 8, 6, 6, 8, 8, 8, 8, 8, 8, 8 },
	spell_fireball,		TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(26),	15,	15,
	"fireball",		"!Fireball!",		"",
	1, 2, 175, 1000
    },

    {
	"acid blast", 
      { 36,36, 101, 101,36,36, 101, 101, 101, 101, 101, 101, 101, 101, 101 }, 
     { 2,  2,  2,  6, 2, 2, 6, 6, 8, 8, 8, 8, 8, 8, 8 },
	spell_acid_blast,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(70),	18,	15,
	"acid blast",		"!AcidBlast!",  "",
	1, 2, 150, 1000
    },

    { /* New Spell */
	"spellfire", 
      { 42, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101 }, 
      { 3,  5,  3,  7, 7, 9, 7, 7, 9, 9, 9, 9, 9, 9, 9 },
	spell_spellfire,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(0),	20,	18,
	"SpellFire",		"!SpellFire!", "",
	1, 4, 100, 1000
    },

    { /* New Spell */
	"orb of chaos", 
      { 48, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101 }, 
      { 3,  5,  3,  7, 7, 9, 7, 7, 9, 9, 9, 9, 9, 9, 9 },
	spell_orb_chaos,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,SLOT(0),23,18,
	"Orb of Chaos",	"You are no longer consumed by Chaos!", "",
	1, 4, 175, 1000
    },

    {   
	"chain lightning", 
      { 52, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101 },
      { 2,  4,  2,  6, 6, 8, 6, 6, 8, 8, 8, 8, 8, 8, 8 },
	spell_chain_lightning,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(500),	25,	18,
	"chain lightning",		"!Chain Lightning!",	"",
	1, 4, 300, 1000
    }, 

    { /* New Spell */
	"wyldstryke", 
      { 57, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101 }, 
      { 3,  5,  3,  7, 7, 9, 7, 7, 9, 9, 9, 9, 9, 9, 9 },
	spell_wyldstryke,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,SLOT(0),25,18,
	"WyldStryke","You don't feel as mana dead!", "",
	1, 2, 200, 1000
    },

    { /* New Spell */
	"acid storm", 
      { 62, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101 }, 
      { 3,  5,  3,  7, 7, 9, 7, 7, 9, 9, 9, 9, 9, 9, 9 },
	spell_acid_storm,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,SLOT(0),30,18,
	"Acid Storm","!AcidStorm!", "",
	1, 2, 350, 1000
    },

    { /* New Spell */
	"wail of the banshee", 
      { 68, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101 }, 
      { 3,  5,  3,  7, 7, 9, 7, 7, 9, 9, 9, 9, 9, 9, 9 },
	spell_wail_banshee,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,SLOT(0),35,20,
	"Wail of the Banshee","The Banshee's curse has lifted!", "",
	1, 4, 150, 1000
    },

    { /* New Spell */
	"torment of cold", 
      { 73, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101 }, 
      { 3,  5,  3,  7, 7, 9, 7, 7, 9, 9, 9, 9, 9, 9, 9 },
	spell_torment_cold,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,SLOT(0),40,20,
	"Torment of Cold","You are no longer chiiled to the bone.", "",
	1, 4, 750, 1000
    },

    {  /* New Spell */
        "sonic scream", 
         { 80,102, 102, 102, 102, 102,102,60,102,70,80,102,102,102,102},
         { 4,  -1,  -1,  -1, -1, -1, -1, 4, -1, 4, 4, -1, -1, -1, -1},
        spell_sonic_scream,      TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
        NULL,                   SLOT(204),  75,    25,
        "sonic scream","Your eardrums aren't bleeding as much.","",
	1, 5, 1375, 10000
    }, 


/* End combat spells */

    {
	"ventriloquate",
      { 3, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101 },
      { 2,  4,  2,  6, 6, 8, 6, 6, 8, 8, 8, 8, 8, 8, 8 },
	spell_ventriloquate,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(41),	 5,	10,
	"",			"!Ventriloquate!",	"",
	1, 1, 600, 1000
    },

    {
	"recharge",
      { 15,101,101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101 },
      { 2,  4,  2,  6, 6, 8, 6, 6, 8, 8, 8, 8, 8, 8, 8 },
      spell_recharge,		TAR_OBJ_INV,		POS_STANDING,
	NULL,			SLOT(517),	30,	18,
	"",			"!Recharge!",		"",
	1, 1, 600, 1000
    },

    {
	"sleep",
      { 23, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101 },
      { 2,  4,  2,  6, 6, 8, 6, 6, 8, 8, 8, 8, 8, 8, 8 },
	spell_sleep,		TAR_CHAR_OFFENSIVE,	POS_STANDING,
	&gsn_sleep,		SLOT(38),	15,	8,
	"",			"You feel less tired.",	"",
	1, 1, 600, 1000
    },

    {
	"enchant armor",
      { 47, 101,101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101 },
      { 4,  8,  8,  8, 8, 8, 8, 8, 8, 3, 8, 8, 8, 8, 8 },
	spell_enchant_armor,	TAR_OBJ_INV,		POS_STANDING,
	NULL,			SLOT(510),	150,	36,
	"",			"!Enchant Armor!",	"",
	1, 3, 280, 1000
    },

    {
	"enchant weapon",
      { 57, 101,101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101 },
      { 4,  8,  8,  8, 8, 8, 8, 8, 8, 3, 8, 8, 8, 8, 8 },
	spell_enchant_weapon,	TAR_OBJ_INV,		POS_STANDING,
	NULL,			SLOT(24),	100,	24,
	"",			"!Enchant Weapon!",	"",
	1, 4, 650, 1000
    },

    {
	"charm person",
      { 33, 101, 101, 33, 33, 33, 101, 33, 101, 101, 101, 101, 101, 101, 101 },
      { 2,  4,  2,  2, 2, 2, 4, 2, 2, 8, 8, 8, 8, 8, 8 },
	spell_charm_person,	TAR_CHAR_OFFENSIVE,	POS_STANDING,
	&gsn_charm_person,	SLOT( 7),	 5,	12,
	"",			"You feel more self-confident.",	"",
	1, 3, 175, 1000
    },

    {
	"haste",
      { 35, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101},
      { 2,  4,  2,  6, 6, 8, 6, 6, 8, 8, 8, 8, 8, 8, 8 },
	spell_haste,		TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(502),	20,	6,
	"",			"You feel yourself slow down.",	"",
	1, 3, 225, 1000
    },

    {
	"pass door",
      { 42, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101 },
      { 2,  4,  2,  6, 6, 8, 6, 6, 8, 8, 8, 8, 8, 8, 8 },
	spell_pass_door,	TAR_CHAR_SELF, POS_STANDING,
	NULL,			SLOT(74),	20,	18,
	"",			"You feel solid again.",	"",
	1, 2, 200, 1000
    },

    {
        "portal",
        { 54, 101, 101, 50,50,50, 101,50, 101, 101, 101, 101, 101, 101, 101 },
        { 3,  8,  8,  3, 3, 3, 8, 3, 8, 8, 8, 8, 8, 8, 8}, 
        spell_portal,           TAR_IGNORE,             POS_STANDING,
        NULL,                   SLOT(519),       75,     12,
        "",                     "!Portal!",		"",
	1, 4, 150, 1000
    },

    {
      "nexus",
      { 68, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101 },
      { 4,  8,  8,  8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8 },
      spell_nexus,            TAR_IGNORE,             POS_STANDING,
      NULL,                   SLOT(520),       150,   36,
      "",                     "!Nexus!",		"",
	1, 4, 350, 1000
    },

    {
	"acid breath",
      { 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101 },
        { -1,-1,-1,-1,-1,-1,-1,-1, -1, -1, -1, -1, -1, -1, -1 }, 
	spell_acid_breath,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(200),	85,	12,
	"blast of acid",	"!Acid Breath!",	"",
	1, 2, 180, 1000
    },

    {
	"frost breath",
      { 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101 },
        { -1,-1,-1,-1,-1,-1,-1,-1, -1, -1, -1, -1, -1, -1, -1 }, 
	spell_frost_breath,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(202),	125,	15,
	"blast of frost",	"!Frost Breath!",	"",
	1, 2, 180, 1000
    },

    {
	"lightning bolt",
      { 30, 33, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101 },
      { 1,  1,  2,  2, 2, 2, 2, 2, 4, 4, 4, 4, 4, 4, 4 },
	spell_lightning_bolt,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(30),	15,	18,
	"lightning bolt",	"!Lightning Bolt!",	"",
	1, 3, 275, 1000
    },


    {
	"lightning breath",
      { 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101 },
        { -1,-1,-1,-1,-1,-1,-1,-1, -1, -1, -1, -1, -1, -1, -1 }, 
	spell_lightning_breath,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(204),	85,	12,
	"blast of lightning",	"!Lightning Breath!",	"",
	1, 2, 180, 1000
    },

    {
	"gas breath",
      { 101, 101, 101,40,40,40, 101,40, 101, 101, 101, 101,101,101, 101 },
        { -1,-1,-1,1,1,1,-1,1, -1, -1, -1, -1, -1, -1, -1 }, 
	spell_gas_breath,	TAR_CHAR_OFFENSIVE,		POS_FIGHTING,
	NULL,			SLOT(203),	100,	12,
	"blast of gas",		"!Gas Breath!",		"",
	1, 3, 150, 1000
    },

    { 
	"fire breath",
      { 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101 }, 
        { -1,-1,-1,-1,-1,-1,-1,-1, -1, -1, -1, -1, -1, -1, -1 }, 
	spell_fire_breath,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(201),	150,	18,
	"blast of flame",	"The smoke leaves your eyes.",	"",
	1, 2, 180, 1000
    },

/* ENDMAGE */



/* Cleric (and Paladin\Anti-Paladin) Only Spells */
    {
	"cure light",
      { 101,  1, 101, 101, 101, 101, 2, 101, 101, 101, 101, 101, 101, 101, 101 },
      { 4,  2,  4, 8, 8, 8, 2, 8, 8, 8, 8, 8, 8, 8, 8 },
	spell_cure_light,	TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(16),	10,	20,
	"",			"!Cure Light!",		"",
	1, 1, 600, 1000
    }, 


    {
	"change sex",
      { 101,63,101,101,101,101,101,101,101,101,101,101,101,101,101 },
      { 4,2,4,8,8,8,8,8,8,8,8,8,8,8,8 },
	spell_change_sex,	TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(82),	2,	10,
	"","Your body feels familiar again",		"",
	1, 1, 100, 1000
    }, 

  
    {
	"cause light",
      { 101,  2, 101, 101, 101, 101, 101, 2, 101, 101, 101, 101, 101, 101, 101 },
      { 4,  2,  4, 8, 8, 8, 8, 2, 8, 8, 8, 8, 8, 8, 8 },
	spell_cause_light,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(62),	7,	10,
	"spell",		"!Cause Light!",	"",
	1, 1, 600, 1000
    },

    {
	"cure blindness",
      { 101,  9, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101 },
      { 4,  2,  4, 8, 8, 8, 2, 2, 8, 8, 8, 8, 8, 8, 8 },
	spell_cure_blindness,	TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(14),	 5,	10,
	"",			"!Cure Blindness!",	"",
	1, 1, 600, 1000
    },

    {
	"bless",
      { 101,  1, 101, 101, 101, 101, 17, 101, 101, 101, 101, 101, 101, 101, 101 },
      { 4,  2,  4, 8, 8, 8, 2, 8, 8, 8, 8, 8, 8, 8, 8 },
	spell_bless,		TAR_OBJ_CHAR_DEF,	POS_STANDING,
	NULL,			SLOT( 3),	 4,	6,
	"",			"You feel less righteous.", 
	"$p's holy aura fades.",
	1, 2, 450, 1000
    },

    {
	"cause serious",
      { 101,  10, 101, 101, 101, 101, 101, 13, 101, 101, 101, 101, 101, 101, 101 },
      { 4,  2,  4, 8, 8, 8, 8, 2, 8, 8, 8, 8, 8, 8, 8 },
	spell_cause_serious,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(64),	12,	12,
	"spell",		"!Cause Serious!",	"",
	1, 2, 75, 1000
    },

    {
	"cure serious",
      { 101, 10, 101, 101, 101, 101, 13, 101, 13, 101, 101, 101, 101, 101, 101 },
      { 4,  2,  4, 8, 8, 8, 2, 8, 2, 8, 8, 8, 8, 8, 8 },
	spell_cure_serious,	TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(61),	15,	20,
	"",			"!Cure Serious!",	"",
	1, 2, 275, 2000
    },

    {
	"protection evil",
      { 101,  7, 101, 101, 101, 101, 10, 101, 10, 101, 101, 101, 101, 101, 101  },
      { 4,  2,  4, 8, 8, 8, 2, 8, 2, 8, 8, 8, 8, 8, 8 },
	spell_protection_evil,	TAR_CHAR_SELF,		POS_STANDING,
	NULL,			SLOT(34), 	5,	3,
	"",			"You feel less protected.",	"",
	1, 2, 300, 1000
    },

    {
      "protection good",
      { 101,  7, 101, 101, 101, 101, 101, 10, 101, 101, 101, 101, 101, 101, 101 },
      { 4,  2,  4, 8, 8, 8, 8, 2, 8, 8, 8, 8, 8, 8, 8 },
      spell_protection_good,  TAR_CHAR_SELF,          POS_STANDING,
      NULL,                   SLOT(514),       5,     3,
      "",                     "You feel less protected.",	"",
	1, 2, 800, 1000
    },

    {
	"earthquake",
      { 101,  19, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101 },
      { 4,  2,  4, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8 },
	spell_earthquake,	TAR_CHAR_OFFENSIVE,		POS_FIGHTING,
	NULL,			SLOT(23),	12,	12,
	"earthquake",		"!Earthquake!",		"",
	1, 2, 150, 1000
    },

    {
	"cause critical",
      { 101,  17, 101, 101, 101, 101, 101, 24, 101, 101, 101, 101, 101, 101, 101 },
      { 4,  2,  4, 8, 8, 8, 8, 2, 8, 8, 8, 8, 8, 8, 8 },
	spell_cause_critical,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(63),	15,	10,
	"spell",		"!Cause Critical!",	"",
	1, 1, 600, 1000
    },

    {
	"cure critical",
      { 101,  20, 101, 101, 101, 101, 24, 101, 101, 101, 101, 101, 101, 101, 101 },
      { 4,  2,  4, 8, 8, 8, 2, 8, 8, 8, 8, 8, 8, 8, 8 },
	spell_cure_critical,	TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(15),	20,	18,
	"",			"!Cure Critical!",	"",
	1, 3, 145, 1000
    },
  
    {
	"cure disease",
      { 101, 18, 101, 101, 101, 101, 33, 101, 101, 101, 101, 101, 101, 101, 101 },
      { 4,  2,  4, 8, 8, 8, 2, 8, 8, 8, 8, 8, 8, 8, 8 },
	spell_cure_disease,	TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(501),	15,	20,
	"",			"!Cure Disease!",	"",
	1, 1, 600, 1000
    },

    {
	"cure poison",
      { 101,  25, 101, 101, 101, 101, 101, 101, 29, 101, 101, 101, 101, 101, 101 },
      { 4,  2,  4, 8, 8, 8, 8, 8, 2, 8, 8, 8, 8, 8, 8 },
	spell_cure_poison,	TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(43),	 5,	8,
	"",			"!Cure Poison!",	"",
	1, 1, 600, 1000
    },

    {
	"dispel evil",
      { 101, 15, 101, 101, 101, 101, 20, 101, 23, 101, 101, 101, 101, 101, 101 },
      { 4,  2,  4, 8, 8, 8, 2, 8, 2, 8, 8, 8, 8, 8, 8 },
	spell_dispel_evil,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(22),	8,	10,
	"dispel evil",		"!Dispel Evil!",	"",
	1, 2, 75, 1000
    },

    {
      "dispel good",
      { 101, 15, 101, 101, 101, 101, 101, 20, 101, 101, 101, 101, 101, 101, 101 },
      { 4,  2,  4, 8, 8, 8, 8, 2, 8, 8, 8, 8, 8, 8, 8 },
        spell_dispel_good,      TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
        NULL,                   SLOT(512), 8,     10,
        "dispel good",          "!Dispel Good!",	"",
	1, 1, 600, 1000
    },

    {
	"heat metal",
      { 101, 22, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101 },
      { 4,  2,  4, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8 },
	spell_heat_metal,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(516), 	18,	12,
	"spell",		"!Heat Metal!",		"",
	1, 1, 600, 1000
    },

    {
	"remove curse",
      { 101, 28, 101, 101, 101, 101, 101, 101,33, 101, 101, 101, 101, 101, 101 },
      { 4,  2,  4, 8, 8, 8, 8, 8, 4, 8, 8, 8, 8, 8, 8 },
	spell_remove_curse,	TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(35),	 25,	24,
	"",			"!Remove Curse!",	"",
	1, 1, 600, 1000
    },

    {
	"flamestrike",
      { 27, 27, 101, 27, 27, 27, 101, 27, 101, 101, 101, 101, 101, 101, 101 },
      { 4,  2,  4, 4, 4, 4, 8, 4, 8, 8, 8, 8, 8, 8, 8 },
	spell_flamestrike,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(65),	20,	18,
	"flamestrike",		"!Flamestrike!",		"",
	1, 1, 600, 1000
    },

    {
	"heal",
      { 101, 40, 101, 101, 101, 101, 50, 101, 101, 101, 101, 101, 101, 101, 101 },
      { 4,  2,  4, 8, 8, 8, 4, 8, 8, 8, 8, 8, 8, 8, 8 },
	spell_heal,		TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(28),	40,	8,
	"",			"!Heal!",		"",
	1, 3, 150, 1000
    },

    {
	"harm",
      { 101, 42, 101, 101, 101, 101, 101, 38, 101, 101, 101, 101, 101, 101, 101 },
      { 4,  2,  4, 8, 8, 8, 8, 4, 8, 8, 8, 8, 8, 8, 8 },
	spell_harm,		TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(27),	18,	10,
	"harm spell",		"!Harm!",		"",
	1, 3, 100, 1000
    },

    {
      "frenzy",
      { 101, 29, 101, 101, 101,29, 101,101, 101, 101, 101, 101, 101, 101, 101 },
      { 4,  2,  4, 8, 8, 4, 8, 8, 8, 8, 8, 8, 8, 8, 8 },
      spell_frenzy,           TAR_CHAR_DEFENSIVE,     POS_STANDING,
      NULL,                   SLOT(504),      15,     24,
      "",                     "Your rage ebbs.",	"",
	1, 3, 800, 1500
    },

    {
	"demonfire",
      { 40, 40, 101, 40, 40,40, 101, 101, 101, 101, 101, 101, 101, 101, 101 },
      { 4,  2,  4, 4, 4, 4, 8, 8, 8, 8, 8, 8, 8, 8, 8 },
	spell_demonfire,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(505),	17,	25,
	"torments",		"!Demonfire!",		"",
	1, 3, 200, 1000
    },

    {
      "ray of truth",
      { 101, 58, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101 },
      { 4,  2,  4, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8 },
        spell_ray_of_truth,     TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
        NULL,                   SLOT(518),  10,     8,
        "ray of truth",         "!Ray of Truth!",	"",
	1, 3, 250, 1000
    },

    {
	"holy word",
      { 101, 62, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101 },
      { 4,  2,  4, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8 },
	spell_holy_word,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(506), 	100,	18,
	"holy word",		"!Holy Word!",		"",
	1, 3, 450, 1000
    },

    {
	"mass healing",
      { 101, 52, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101 },
      { 4,  2,  4, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8 },
	spell_mass_healing,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(508),	100,	20,
	"",			"!Mass Healing!",	"",
	1, 3, 275, 1000
    },

    {
      "golden aura",
      { 102, 65,102,102,102,102,102,102, 101, 101, 101, 101, 101, 101, 101},
      { 4,  2,  4, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8 },
      spell_golden,   TAR_CHAR_SELF,   POS_STANDING,
      NULL,           SLOT( 37 ),  100,  3,
      "",   "The golden aura around your fades away.", "",
	1, 5, 3750, 10000
    },

/* ENDCLERIC */


/* ENCHANTER ONLY SPELLS */

    {
	"fortify object",
      { 101, 101, 101, 101, 101, 101, 101, 101, 101, 90, 101, 101, 101, 101, 101 },
      { 8, 8, 8, 8, 8, 8, 8, 8, 8, 2, 8, 8, 8, 8, 8 },
	spell_fortify_object,	TAR_OBJ_INV,		POS_STANDING,
	NULL,			SLOT(523),	10,	12,
	"",			"",	"$p doesn't look as solid anymore.",
	1, 4, 1250, 1000
    },

    {
	"fireproof",
      { 101, 101, 101, 101, 101, 101, 101, 101, 101, 20, 101, 101, 101, 101, 101 },
      { 8, 8, 8, 8, 8, 8, 8, 8, 8, 2, 8, 8, 8, 8, 8 },
	spell_fireproof,	TAR_OBJ_INV,		POS_STANDING,
	NULL,			SLOT(523),	10,	12,
	"",	"",	"\n\r{R$p {rdoes not look {WIMPERVIOUS {rto fire any longer.{x\n\r",
	1, 1, 600, 1000
    },

/* END ENCHANTER */


/* MISC Spells */

    {
	"detect evil",
      { 14,  5, 101, 101, 101, 101, 5, 101, 5, 101, 101, 101, 101, 101, 101 },
      { 2,  2,  2,  8, 8, 8, 2, 8, 2, 8, 8, 8, 8, 8, 8 },
	spell_detect_evil,	TAR_CHAR_SELF,		POS_STANDING,
	NULL,			SLOT(18),	 3,	6,
	"",			"The red in your vision disappears.",	"",
	1, 2, 800, 1000
    },

    {
      "detect good",
      { 14,  5, 101, 101, 101, 101, 101, 5, 101, 101, 101, 101, 101, 101, 101 },
      { 2,  2,  2,  8, 8, 8, 8, 2, 8, 8, 8, 8, 8, 8, 8 },
      spell_detect_good,      TAR_CHAR_SELF,          POS_STANDING,
      NULL,                   SLOT(513), 3, 6,
      "",                     "The gold in your vision disappears.",	"",
	1, 1, 600, 1000
    },

    {
	"detect hidden",
      { 28, 26, 101, 28, 28, 28, 101,28, 19, 101, 101, 101, 101, 101, 101 },
      { 2,  2,  2,  4, 4, 4, 8, 4, 2, 8, 8, 8, 8, 8, 8 },
	spell_detect_hidden,	TAR_CHAR_SELF,		POS_STANDING,
	NULL,			SLOT(44),	 5,	10,
	"",			"You feel less aware of your surroundings.",	
	"",
	1, 2, 120, 1000
    },

  {
	"armor",
      { 7,  8, 101, 101, 8, 8, 8, 8, 101, 101, 101, 101, 101, 101, 101 },
      { 2,  2,  2,  8, 4, 4, 4, 4, 8, 8, 8, 8, 8, 8, 8 },
	spell_armor,		TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT( 1),	 3,	6,
	"",			"You feel less armored.",	"",
	1, 2, 800, 1000
    },

    {
	"blindness",
      {  22,  13, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101 },
      { 2,  2,  2,  8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8 },
	spell_blindness,	TAR_CHAR_OFFENSIVE, 	POS_FIGHTING,
	&gsn_blindness,		SLOT( 4),	 5,	8,
	"",			"You can see again.",	"",
	1, 2, 75, 1000
    },

    {
	"call lightning",
      { 36, 27, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101 },
      { 2,  2,  2,  8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8 },
	spell_call_lightning,	TAR_CHAR_OFFENSIVE,		POS_FIGHTING,
	NULL,			SLOT( 6),	12,	16,
	"lightning bolt",	"!Call Lightning!",	"",
	1, 3, 300, 1000
    },

    { 
      "calm",
      { 34, 20, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101 },
      { 2,  2,  2,  8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8 },
	spell_calm,		TAR_IGNORE,		POS_FIGHTING,
	NULL,			SLOT(509),	20,	12,
	"",			"You have lost your peace of mind.",	"",
	1, 1, 600, 1000
    },

    {
	"cancellation",
      { 20, 24, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101 },
      { 2,  2,  2,  8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8 },
	spell_cancellation,	TAR_CHAR_SELF,	POS_STANDING,
	NULL,			SLOT(507),	15,	18,
	"",			"!cancellation!",	"",
	1, 3, 175, 1000
    },

    {
	"continual light",
      {  11,  6, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101 },
      { 2,  2,  2,  8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8 },
	spell_continual_light,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(57),	 5,	6,
	"",			"!Continual Light!",	"",
	1, 1, 600, 1000
    },

    {
	"control weather",
      { 9, 20, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101 },
      { 2,  2,  2,  8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8 },
	spell_control_weather,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(11),	10,	18,
	"",			"!Control Weather!",	"",
	1, 0, 50, 1000
    },

    {
	"create food",
      { 8, 4, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101 },
      { 2,  2,  2,  8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8 },
	spell_create_food,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(12),	 3,	6,
	"",			"!Create Food!",	"",
	1, 1, 600, 1000
    },

    {
	"create rose",
      { 2, 4, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101 },
      { 2,  2,  2,  8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8 },
	spell_create_rose,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(511),	45, 	20,
	"",			"!Create Rose!",	"",
	1, 1, 600, 1000
    },  

    {
	"create spring",
      { 16, 10, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101 },
      { 2,  2,  2,  8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8 },
	spell_create_spring,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(80),	20,	6,
	"",			"!Create Spring!",	"",
	1, 1, 600, 1000
    },

    {
	"create water",
      { 6,  3, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101 },
      { 2,  2,  2,  8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8 },
	spell_create_water,	TAR_OBJ_INV,		POS_STANDING,
	NULL,			SLOT(13),	 2,	3,
	"",			"!Create Water!",	"",
	1, 1, 600, 1000
    },

    {
	"curse",
      { 41, 37, 101, 41, 10,41, 101,15, 101, 101, 101, 101, 101, 101, 101 },
      { 2,  2,  2,  4, 4, 4, 8, 4, 8, 8, 8, 8, 8, 8, 8 },
	spell_curse,		TAR_OBJ_CHAR_OFF,	POS_FIGHTING,
	&gsn_curse,		SLOT(17),	10,	10,
	"curse",		"The curse wears off.", 
	"$p is no longer impure.",
	1, 2, 190, 1000
    },

    {
	"detect invis",
      { 4,  14, 101, 101, 101, 101, 101, 101, 20, 101, 101, 101, 101, 101, 101 },
      { 2,  2,  2,  8, 8, 8, 8, 8, 3, 8, 8, 8, 8, 8, 8 },
	spell_detect_invis,	TAR_CHAR_SELF,		POS_STANDING,
	NULL,			SLOT(19),	 5,	6,
	"",			"You no longer see invisible objects.",
	"",
	1, 2, 300, 1500
    },

    {
	"detect magic",
      { 1,  8, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101 },
      { 2,  2,  2,  8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8 },
	spell_detect_magic,	TAR_CHAR_SELF,		POS_STANDING,
	NULL,			SLOT(20),	 2,	8,
	"",			"The detect magic wears off.",	"",
	1, 1, 600, 1000
    },

    {
	"detect poison",
      { 13,  17, 101, 101, 101, 101, 101, 101, 18, 101, 101, 101, 101, 101, 101 },
      { 2,  2,  2,  8, 8, 8, 8, 8, 3, 8, 8, 8, 8, 8, 8 },
	spell_detect_poison,	TAR_OBJ_INV,		POS_STANDING,
	NULL,			SLOT(21),	 3,	6,
	"",			"!Detect Poison!",	"",
	1, 1, 600, 1000
    },

    {
	"dispel magic",
      { 24, 54, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101 },
      { 2,  2,  2,  8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8 },
	spell_dispel_magic,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(59),	12,	10,
	"",			"!Dispel Magic!",	"",
	1, 2, 145, 1000
    },

    {
	"energy drain",
      { 44, 46, 101, 44, 44,44, 101, 44, 101, 101, 101, 101, 101, 101, 101 },
      { 2,  2,  2,  4, 4, 4, 8, 4, 8, 8, 8, 8, 8, 8, 8 },
	spell_energy_drain,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(25),	20,	18,
	"energy drain",		"!Energy Drain!",	"",
	1, 2, 175, 1000
    },

    {
	"faerie fire",
      {  4,  3, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101 },
      { 2,  2,  2,  8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8 },
	spell_faerie_fire,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(72),	 5,	10,
	"faerie fire",		"The pink aura around you fades away.",
	"",
	1, 1, 10, 1000
    },

    {
	"faerie fog",
      { 2, 2, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101 },
      { 2,  2,  2,  8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8 },
	spell_faerie_fog,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(73),	5,	10,
	"faerie fog",		"!Faerie Fog!",		"",
	1, 1, 600, 1000
    },

    {
	"farsight",
      { 17, 12, 101, 101, 101, 101, 101, 101, 1, 101, 101, 101, 101, 101, 101 },
      { 2,  2,  2,  8, 8, 8, 8, 8, 2, 8, 8, 8, 8, 8, 8 },
	spell_farsight,		TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(521),	12,	15,
	"farsight",		"!Farsight!",		"",
	1, 1, 600, 1000
    },	


    {
	"fly",
      { 30, 23, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101 },
      { 2,  2,  2,  8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8 },
	spell_fly,		TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(56),	5,	8,
	"",			"You slowly float to the ground.",	"",
	1, 2, 100, 1000
    },

    {
	"floating disc",
      {  6, 6, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101 },
      { 2,  2,  2,  8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8 },
	spell_floating_disc,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(522),	20,	10,
	"",			"!Floating disc!",	"",
	1, 1, 600, 1000
    },

    {
	"gate",
      { 30, 50, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101 },
      { 2,  2,  2,  8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8 },
	spell_gate,		TAR_IGNORE,		POS_FIGHTING,
	NULL,			SLOT(83),	50,	6,
	"",			"!Gate!",		"",
	1, 4, 1750, 3000
    },

    {
	"giant strength",
      { 25, 101, 101, 101, 101, 35, 101, 22, 101, 101, 101, 101, 101, 101, 101 },
      { 2,  2,  2,  8, 8, 7, 8, 2, 8, 8, 8, 8, 8, 8, 8 },
	spell_giant_strength,	TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(39),	10,	6,
	"",			"You feel weaker.",	"",
	1, 2, 600, 1000
    },

    {
	"identify",
      { 21, 31, 101, 25, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101 },
      { 2,  2,  2,  4, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8 },
	spell_identify,		TAR_OBJ_INV,		POS_STANDING,
	NULL,			SLOT(53),	6,	12,
	"",			"!Identify!",		"",
	1, 3, 800, 1000
    },

    {
	"infravision",
      {  7,  11, 101, 101, 101, 101, 101, 101, 2, 101, 101, 101, 101, 101, 101 },
      { 2,  2,  2,  8, 8, 8, 8, 8, 2, 8, 8, 8, 8, 8, 8 },
	spell_infravision,	TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(77),	 5,	10,
	"",			"You no longer see in the dark.",	"",
	1, 1, 600, 1000
    },

    {
	"invisibility",
      { 12, 101, 101, 101, 25, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101 },
      { 2,  2,  2,  8, 5, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8 },
	spell_invis,		TAR_CHAR_SELF,	POS_STANDING,
	&gsn_invisibility,		SLOT(29),	 5,	6,
	"",			"You are no longer invisible.",		
	"$p fades into view.",
	1, 2, 175, 1000
    },

    {
	"know alignment",
      {  18, 13, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101 },
      { 2,  2,  2,  8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8 },
	spell_know_alignment,	TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(58),	 10,	6,
	"",			"!Know Alignment!",	"",
	1, 1, 600, 1000
    },


    {
	"locate object",
      { 37, 21, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101 },
      { 2,  2,  2,  8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8 },
	spell_locate_object,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(31),	5,	8,
	"",			"!Locate Object!",	"",
	1, 3, 100, 1000
    },

    {
	"mass invis",
      { 39, 48, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101 },
      { 2,  2,  2,  8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8 },
	spell_mass_invis,	TAR_IGNORE,		POS_STANDING,
	&gsn_mass_invis,	SLOT(69),	10,	10,
	"",			"You are no longer invisible.",		"",
	1, 2, 190, 1000
    },

    {
	"plague",
      { 31, 22, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101 },
      { 2,  2,  2,  8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8 },
	spell_plague,		TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	&gsn_plague,		SLOT(503),	18,	12,
	"sickness",		"Your sores vanish.",	"",
	1, 1, 600, 1000
    },

    {
	"poison",
      { 20,  11, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101 },
      { 2,  2,  2,  8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8 },
	spell_poison,		TAR_OBJ_CHAR_OFF,	POS_FIGHTING,
	&gsn_poison,		SLOT(33),	10,	12,
	"poison",		"You feel less sick.",	
	"The poison on $p dries up.",
	1, 1, 600, 1000
    },

    {
	"refresh",
      { 19, 26, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101 },
      { 2,  2,  2,  8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8 },
	spell_refresh,		TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(81),	12,	10,
	"refresh",		"!Refresh!",		"",
	1, 1, 600, 1000
    },

    {
	"sanctuary", 
      { 58, 23, 101, 101, 101, 101, 60, 101, 101, 101, 101, 101, 101, 101, 101 }, 
      { 2,  2,  2,  8, 8, 8, 2, 8, 8, 8, 8, 8, 8, 8, 8 },
	spell_sanctuary,	TAR_CHAR_DEFENSIVE,	POS_STANDING,
	&gsn_sanctuary,		SLOT(36),	75,	20,
	"",			"The white aura around your body fades.",
	"",
	1, 5, 550, 2500
    },

    {
	"energy shield",
      { 29, 39, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101 },
      { 2,  2,  2,  8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8 },
	spell_shield,		TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(67),	12,	10,
	"",			"Your energy shield shimmers then fades away.",
	"",
	1, 3, 100, 1000
    },

    {
        "slow",
        { 32, 19, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101 },
      { 2,  2,  2,  8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8 },
        spell_slow,             TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
        NULL,                   SLOT(515),20, 12,
        "",                     "You feel yourself speed up.",	"",
	1, 1, 600, 1000
    },

    {
	"stone skin",
      { 19, 18, 101, 101, 19, 19, 19, 19, 101, 101, 101, 101, 101, 101, 101 },
      { 2,  2,  2,  8, 4, 4, 4, 4, 8, 8, 8, 8, 8, 8, 8 },
	spell_stone_skin,	TAR_CHAR_SELF,		POS_STANDING,
	NULL,			SLOT(66),	12,	10,
	"",			"Your skin feels soft again.",	"",
	1, 2, 275, 1000
    },

    {
	"summon",
      { 49, 44, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101 },
      { 2,  2,  2,  8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8 },
	spell_summon,		TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(40),	40,	10,
	"",			"!Summon!",		"",
	1, 4, 645, 1000
    },

    {
	"teleport",
      {  27, 32, 32, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101 },
      { 2,  2,  2,  8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8 },
	spell_teleport,		TAR_CHAR_SELF,		POS_FIGHTING,
	NULL,	 		SLOT( 2),	15,	6,
	"",			"!Teleport!",		"",
	1, 3, 165, 1000
    },

    {
	"weaken",
      {  26, 14, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101},
      { 2,  2,  2,  8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8 },
	spell_weaken,		TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(68),	13,	10,
	"spell",		"You feel stronger.",	"",
	1, 1, 600, 1000
    },

    {
	"word of recall",
      { 23, 24, 28, 101, 101, 35, 29, 101, 25, 101, 101, 101, 101, 101, 101 },
      { 2,  2,  2,  8, 8, 7, 2, 8, 2, 8, 8, 8, 8, 8, 8 },
	spell_word_of_recall,	TAR_CHAR_SELF,		POS_RESTING,
	NULL,			SLOT(42),	 25,	36,
	"",			"!Word of Recall!",	"",
	1, 3, 6800, 1000
    },

/* ENDMISC */


/* Psionic Only Spells */

     {
        "apport", 
        { 102,102, 21, 102, 102, 102,102,102,102,102,102,102,102,102,102},
        { 2,  2,  2,  4, 4, 6, 3, 3, 3, 2, 4, 4, 4, 4, 4},
	spell_apport,		TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(40),	25,	12,
	"",			"!Apport!",		"",
	1, 1, 600, 1000
    },

    {
        "combat mind",
        { 102,102, 22, 102, 102,102,102,102,102,102,102,102,102,102,102},
        { 2,  2,  2,  4, 4, 6, 3, 3, 3, 2, 4, 4, 4, 4, 4},
        spell_combat_mind,   TAR_CHAR_SELF, POS_STANDING,
        NULL,                SLOT(0),             25,      12,
        "",           "Your battle sense has faded.",      "",
	1, 4, 1000, 4500
    },

    {
        "adrenaline control",
        { 102,102,18, 102, 102, 102,102,102,102,102,102,102,102,102,102},
        { 2,  2,  2,  4, 4, 6, 3, 3, 3, 2, 4, 4, 4, 4, 4},
        spell_adrenaline_control,   TAR_CHAR_SELF,   POS_STANDING,
        NULL,              SLOT(0),                  10,        12,
        "", "The adrenaline rush wears off.", "",
	1, 2, 200, 1000
    },

    {
        "enhanced strength",
        { 102,102, 7, 102, 102,102,102,102,102,102,102,102,102,102,102},
        { 2,  2,  2,  4, 4, 6, 3, 3, 3, 2, 4, 4, 4, 4, 4},
        spell_enhanced_strength,     TAR_CHAR_SELF,         POS_STANDING,
        NULL,               SLOT(0),      8,                    12,
        "", "You no longer feel so HUGE.", "",
	1, 2, 800, 1000
    },

    {
        "aura sight",
        { 102,102, 2, 102, 102, 102,102,102,102,102,102,102,102,102,102},
        { 2,  2,  2,  4, 4, 6, 3, 3, 3, 2, 4, 4, 4, 4, 4},
        spell_aura_sight,     TAR_CHAR_DEFENSIVE,             POS_STANDING,
        NULL,SLOT(0),            2,                12,
        "", "!Aura Sight!", "",
	1, 1, 600, 1000
    },
    {
        "awe",
        { 102,102,60, 102, 102,102,102,102,102,102,102,102,102,102,102},
        { 2,  2,  2,  4, 4, 6, 3, 3, 3, 2, 4, 4, 4, 4, 4},
        spell_awe,            TAR_IGNORE,              POS_FIGHTING,
        NULL,                 SLOT(0),     200, 20,
        "", "!Awe!", "",
	1, 3, 450, 1000
    },
    {
        "thought shield",
        { 102,102, 25, 102,102, 102,102,102,102,102,102,102,102,102,102},
        { 2,  2,  2,  4, 4, 6, 3, 3, 3, 2, 4, 4, 4, 4, 4},
        spell_thought_shield,      TAR_CHAR_SELF,               POS_STANDING,
        NULL,                  SLOT(0),             17, 12,
        "", "You no longer feel so protected.", "",
	1, 1, 600, 1000
    },
    {
        "psychic healing",
        { 102,102, 10, 102, 102,102,102,102,102,102,102,102,102,102,102},
        { 2,  2,  2,  4, 4, 6, 3, 3, 3, 2, 4, 4, 4, 4, 4},
        spell_psychic_healing,      TAR_CHAR_DEFENSIVE,    POS_STANDING,
        NULL,                    SLOT(0),            15, 12,
        "", "!Psychic Healing!", "",
	1, 2, 200, 1000
    },

    {
        "mental barrier",
        { 102,102,70, 102, 102, 102,102,102,102,102,102,102,102,102,102},
        { 2,  2,  2,  4, 4, 6, 3, 3, 3, 2, 4, 4, 4, 4, 4},
        spell_mental_barrier,       TAR_CHAR_SELF,            POS_STANDING,
        NULL,                SLOT(0),                 35,              12,
        "", "Your mental barrier breaks down.", "",
	1, 3, 500, 1000
    },

    {
        "share strength",
        { 102,102,35, 102, 102, 102,102,102,102,102,102,102,102,102,102},
        { 2,  2,  2,  4, 4, 6, 3, 3, 3, 2, 4, 4, 4, 4, 4},
        spell_share_strength,       TAR_CHAR_DEFENSIVE,         POS_STANDING,
        NULL,                 SLOT(0),                  20, 12,
        "", "You no longer share strength with another.", "",
	1, 2, 125, 1000
    },

    {
        "levitation",
        { 102,102,14, 102, 102, 102,102,102,102,102,102,102,102,102,102},
        { 2,  2,  2,  4, 4, 6, 3, 3, 3, 2, 4, 4, 4, 4, 4},
        spell_levitation,        TAR_CHAR_SELF,            POS_STANDING,
        NULL,                  SLOT(0),    5,                  18,
        "", "You slowly float to the ground.", "",
	1, 1, 600, 1000
    },

    {
        "obfuscate",
        { 102,102,15, 102, 102, 102,102,102,102,102,102,102,102,102,102},
        { 2,  2,  2,  4, 4, 6, 3, 3, 3, 2, 4, 4, 4, 4, 4},
        spell_obfuscate,        TAR_CHAR_SELF,       POS_STANDING,
	&gsn_obfuscate,		SLOT(29),	 2,	12,
        "",        "You are no longer invisible.", "$p fades into view.",
	1, 1, 600, 1000
    },                             

    {
        "bio fortress",
        { 102,102,29, 102, 102, 102,102,102,102,102,102,102,102,102,102},
        { 2,  2,  2,  4, 4, 6, 3, 3, 3, 2, 4, 4, 4, 4, 4},
        spell_bio_fortress,      TAR_CHAR_SELF,            POS_STANDING,
        NULL,                   SLOT(0),         20,             12,
        "", "Your bio fortress has run its course.", "",
	1, 4, 800, 5000
    },

    {
        "cell adjustment",
        { 102,102,11, 102, 102,102,102,102,102,102,102,102,102,102,102},
        { 2,  2,  2,  4, 4, 6, 3, 3, 3, 2, 4, 4, 4, 4, 4},
        spell_cell_adjustment,   TAR_CHAR_SELF,              POS_STANDING,
        NULL,                   SLOT(0),          8,               12,
        "", "!Cell Adjustment!", "",
	1, 2, 175, 1000
    },

    {
        "displacement",
        { 102,102,55, 102, 102, 102,102,102,102,102,102,102,102,102,102},
        { 2,  2,  2,  4, 4, 6, 3, 3, 3, 2, 4, 4, 4, 4, 4},
        spell_displacement,        TAR_CHAR_SELF,             POS_STANDING,
        NULL,                    SLOT(0),     17,                12,
        "", "You are no longer displaced.", "",
	1, 3, 200, 1000
    },

    {
        "ectoplasmic form",        
        { 102,102,24, 102, 102, 102,102,102,102,102,102,102,102,102,102},
        { 2,  2,  2,  4, 4, 6, 3, 3, 3, 2, 4, 4, 4, 4, 4},
        spell_ectoplasmic_form,      TAR_CHAR_SELF,          POS_STANDING,
        NULL,               SLOT(0),           10,             12,
        "", "You feel solid again.", "",
	1, 2, 50, 1000
    },

    {
        "lend health",
        { 102,102,6, 102, 102, 102,102,102,102,102,102,102,102,102,102},
        { 2,  2,  2,  4, 4, 6, 3, 3, 3, 2, 4, 4, 4, 4, 4},
        spell_lend_health,  TAR_CHAR_DEFENSIVE, POS_STANDING,
        NULL,         SLOT(0),     10,     12,
        "", "!Lend Health!", "",
	1, 2, 800, 1000
    },

    {
        "complete healing",
        { 102,102, 85, 102, 102,102,102,102,102,102,102,102,102,102,102},
        { 2,  2,  2,  4, 4, 6, 3, 3, 3, 2, 4, 4, 4, 4, 4},
        spell_complete_healing,     TAR_CHAR_SELF,  POS_STANDING,
        NULL,                SLOT(0),      350,             12,
        "", "!Complete Healing!", "",
	1, 5, 3500, 10000
    },

    {
        "energy containment",
        { 102,102, 12, 102, 102,102,102,102,102,102,102,102,102,102,102},
        { 2,  2,  2,  4, 4, 6, 3, 3, 3, 2, 4, 4, 4, 4, 4},
        spell_energy_containment, TAR_CHAR_SELF, POS_STANDING,
        NULL, SLOT(0),20, 12,
        "", "You no longer absorb energy.","",
	1, 1, 600, 1000
    },
    {
        "enhance armor",
        { 102,102,48, 102, 102, 102,102,102,102,102,102,102,102,102,102},
        { 2,  2,  2,  4, 4, 6, 3, 3, 3, 2, 4, 4, 4, 4, 4},
        spell_enhance_armor, TAR_OBJ_INV, POS_STANDING,
        NULL, SLOT(0),75, 24,
        "", "!Enhance Armor!", "",
	1, 3, 110, 1000
    },
    {
        "flesh armor",
        { 102,102,45, 102, 102, 102,102,102,102,102,102,102,102,102,102},
        { 2,  2,  2,  4, 4, 6, 3, 3, 3, 2, 4, 4, 4, 4, 4},
        spell_flesh_armor, TAR_CHAR_SELF, POS_STANDING,
        NULL, SLOT(0),25, 12,
        "", "Your skin returns to normal.","",
	1, 2, 150, 1000
    },

    {
        "inertial barrier",
        { 102,102,3, 102, 102, 102,102,102,102,102,102,102,102,102,102},
        { 2,  2,  2,  4, 4, 6, 3, 3, 3, 2, 4, 4, 4, 4, 4},
        spell_inertial_barrier, TAR_CHAR_SELF, POS_STANDING,
        NULL, SLOT(0), 15, 24,
        "", "Your inertial barrier dissolves.", "",
	1, 2, 800, 1000
    },

    {
        "domination", 
        {102,102,20,102,102,102,102,102,102,102,102,102,102,102,102},
        { 2,  2,  2,  4, 4, 6, 3, 3, 3, 2, 4, 4, 4, 4, 4},
        spell_domination, TAR_CHAR_OFFENSIVE, POS_STANDING,
        &gsn_charm_person, SLOT(505),17, 12,
        "", "You regain control of your body.", "",
	1, 2, 450, 1000
    },


/* Psionic Powers  - Combat*/

    {
        "disintegrate",
        { 102,102, 42, 102, 102, 102,102,102,102,102,102,102,102,102,102},
        { 2,  2,  2,  4, 4, 6, 3, 3, 3, 2, 4, 4, 4, 4, 4},
        spell_disintegrate,     TAR_CHAR_OFFENSIVE,         POS_STANDING,
        NULL,                   SLOT(505),    75,     12,
        "disintegration",    "!Disintegrate!",       "",
	1, 2, 200, 1000
    },

    {
        "essence drain",  
        { 102,102,30, 102, 102, 102,102,102,102,102,102,102,102,102,102},
        { 2,  2,  2,  4, 4, 6, 3, 3, 3, 2, 4, 4, 4, 4, 4},
        spell_essence_drain, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
        NULL, SLOT(505),20, 12,
        "essence drain", "!Essence Drain!","",
	1, 2, 100, 1000
    },

    {
        "agitation", 
        { 102,102,13, 102, 102,102,102,102,102,102,102,102,102,102,102},
        { 2,  2,  2,  4, 4, 6, 3, 3, 3, 2, 4, 4, 4, 4, 4},
        spell_agitation, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
        NULL, SLOT(505),5, 12,
        "agitation", "!Agitation!", "",
	1, 1, 600, 1000
    },

    {
        "ballistic attack", 
        { 102,102,1, 102, 102, 102,102,102,102,102,102,102,102,102,102},
        { 2,  2,  2,  4, 4, 6, 3, 3, 3, 2, 4, 4, 4, 4, 4},
        spell_ballistic_attack, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
        NULL, SLOT(505),3, 12,
        "ballistic attack", "!Ballistic Attack!", "",
	1, 1, 600, 1000
    },

    {
        "control flames",  
        { 102,102,16, 102, 102, 102,102,102,102,102,102,102,102,102,102},
        { 2,  2,  2,  4, 4, 6, 3, 3, 3, 2, 4, 4, 4, 4, 4},
        spell_control_flames, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
        NULL, SLOT(505),8, 12,
        "tongue of flame", "!Control Flames!", "",
	1, 1, 600, 1000
    },

    {
        "death field",  
        { 102,102,50, 102, 102, 102,102,102,102,102,102,102,102,102,102},
        { 2,  2,  2,  4, 4, 6, 3, 3, 3, 2, 4, 4, 4, 4, 4},
        spell_death_field, TAR_IGNORE, POS_FIGHTING,
        NULL, SLOT(505),60, 18,
        "field of death", "!Death Field!", "",
	1, 2, 200, 1000
    },

    {
        "detonate", 
        { 102,102,29, 102, 102, 102,102,102,102,102,102,102,102,102,102},
        { 2,  2,  2,  4, 4, 6, 3, 3, 3, 2, 4, 4, 4, 4, 4},
        spell_detonate, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
        NULL, SLOT(505),13, 24,
        "detonation", "!Detonate!", "",
	1, 3, 50, 1000
    },

    {
        "ego whip",
        { 102,102,44, 102, 102, 102,102,102,102,102,102,102,102,102,102},
        { 2,  2,  2,  4, 4, 6, 3, 3, 3, 2, 4, 4, 4, 4, 4},
        spell_ego_whip, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
        NULL, SLOT(505),15, 12,
        "", "You feel more confident.", "",
	1, 2, 150, 1000
    },

    {
        "inflict pain",
        { 102,102,19, 102, 102, 102,102,102,102,102,102,102,102,102,102},
        { 2,  2,  2,  4, 4, 6, 3, 3, 3, 2, 4, 4, 4, 4, 4},
        spell_inflict_pain, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
        NULL, SLOT(505),12, 12,
        "mindpower", "!Inflict Pain!", "",
	1, 2, 35, 1000
    },

    {
        "mind thrust",
        { 102,102,5, 102, 102, 102,102,102,102,102,102,102,102,102,102},
        { 2,  2,  2,  4, 4, 6, 3, 3, 3, 2, 4, 4, 4, 4, 4},
        spell_mind_thrust, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
        NULL, SLOT(505),8, 12,
        "mind thrust", "!Mind Thrust!", "",
	1, 1, 600, 1000
    },
    
    {
        "project force",
        { 102,102,33, 102, 102, 102,102,102,102,102,102,102,102,102,102},
        { 2,  2,  2,  4, 4, 6, 3, 3, 3, 2, 4, 4, 4, 4, 4},
        spell_project_force, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
        NULL, SLOT(505),20, 12,
        "projected force", "!Project Force!", "",
	1, 2, 15, 1000
    },

    {
        "psionic blast",
        { 102,102,38, 102, 102, 102,102,102,102,102,102,102,102,102,102},
        { 2,  2,  2,  4, 4, 6, 3, 3, 3, 2, 4, 4, 4, 4, 4},
        spell_psionic_blast, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
        NULL, SLOT(505),23, 12,
        "psionic blast", "!Psionic Blast!", "",
	1, 2, 150, 1000
    },

    {
        "psychic crush",
        { 102,102, 23, 102, 102, 102,102,102,102,102,102,102,102,102,102},
        { 2,  2,  2,  4, 4, 6, 3, 3, 3, 2, 4, 4, 4, 4, 4},
        spell_psychic_crush, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
        NULL, SLOT(505),15, 18,
        "psychic crush", "!Psychic Crush!", "",
	1, 1, 600, 1000
    },

    {
        "psychic drain",
        { 102,102,9, 102, 102, 102,102,102,102,102,102,102,102,102,102},
        { 2,  2,  2,  4, 4, 6, 3, 3, 3, 2, 4, 4, 4, 4, 4},
        spell_psychic_drain, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
        NULL, SLOT(505),10, 12,
        "", "You no longer feel drained.", "",
	1, 2, 175, 1000
    },

    {
        "ultrablast", 
        { 102,102,80, 102, 102, 102,102,102,102,102,102,102,102,102,102},
        { 2,  2,  2,  4, 4, 6, 3, 3, 3, 2, 4, 4, 4, 4, 4},
        spell_ultrablast, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
        NULL, SLOT(505),100, 24,
        "ultrablast", "!Ultrablast!", "",
	1, 3, 200, 1000
    },

/* ENDPSIONIC */




/* Player Dragon Breaths */

    {
       "blightning",
        { 15, 15, 15, 10, 10, 5,101, 5, 101, 101, 101, 101, 101, 101, 101 },
        { -1,-1,-1,-1,-1,-1,-1,-1, -1, -1, -1, -1, -1, -1, -1 }, 
	spell_null,		TAR_CHAR_OFFENSIVE,		POS_FIGHTING,
        &gsn_blightning,           SLOT(0),       7,      24,
        "",                     "!blightning!",            "",
	1, 2, 300, 1000
    },

    {
       "bfire",
        { 25, 25, 25, 20, 20, 15, 101, 15, 101, 101, 101, 101, 101, 101, 101 },
        { -1,-1,-1,-1,-1,-1,-1,-1, -1, -1, -1, -1, -1, -1, -1 }, 
	spell_null,		TAR_CHAR_OFFENSIVE,		POS_FIGHTING,
        &gsn_bfire,           SLOT(0),       15,      24,
        "",                     "!bfire!",            "",
	1, 2, 450, 1000
    },

    {
       "bfrost",
        { 20, 20, 20, 15, 15, 10, 101, 10, 101, 101, 101, 101, 101, 101, 101 },
        { -1,-1,-1,-1,-1,-1,-1,-1, -1, -1, -1, -1, -1, -1, -1 }, 
	spell_null,		TAR_CHAR_OFFENSIVE,		POS_FIGHTING,
        &gsn_bfrost,           SLOT(0),       15,      24,
        "",                     "!bfrost!",            "",
	1, 2, 350, 2000
    },

    {
       "bacid",
        { 10, 10, 10, 5, 5, 1,101, 1, 101, 101, 101, 101, 101, 101, 101 },
        { -1,-1,-1,-1,-1,-1,-1,-1, -1, -1, -1, -1, -1, -1, -1 }, 
	spell_null,		TAR_CHAR_OFFENSIVE,		POS_FIGHTING,
        &gsn_bacid,           SLOT(0),       4,      24,
        "",                     "!bacid!",            "",
	1, 2, 800, 1000
    },

    {
       "bgas",
        { 17, 17, 17, 12, 12, 7,101, 7, 101, 101, 101, 101, 101, 101, 101 },
        { -1,-1,-1,-1,-1,-1,-1,-1, -1, -1, -1, -1, -1, -1, -1 }, 
	spell_null,		TAR_CHAR_OFFENSIVE,		POS_FIGHTING,
        &gsn_bgas,           SLOT(0),       10,      24,
        "",                     "!bgas!",            "",
	1, 2, 250, 1200
    },

/*
 * Spells for mega1.are from Glop/Erkenbrand.
 */
    {
        "general purpose",    
        {500,500,500,500,500,500,500,500,500,500,500,500,500,500,500},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        spell_general_purpose,  TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
        NULL,                   SLOT(401),      0,      12,
        "general purpose ammo", "!General Purpose Ammo!",	"",
	1, 2, 125, 1000
    },
 
    {
        "high explosive",       
        {500,500,500,500,500,500,500,500,500,500,500,500,500,500,500},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        spell_high_explosive,   TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
        NULL,                   SLOT(402),      0,      12,
        "high explosive ammo",  "!High Explosive Ammo!",	"",
	1, 4, 150, 1000
    },

    {
        "pdrain blade",
        {500,500,500,500,500,500,500,500,500,500,500,500,500,500,500},     
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        spell_pdrain_blade,	TAR_OBJ_INV,		POS_STANDING,
	NULL,                   SLOT(500),       1,     1,
        "",			"!Drain Blade!",	"$p looks less malevolant.",
	1, 15, 10000, 100000
    },

    {
        "pshocking blade",
        {500,500,500,500,500,500,500,500,500,500,500,500,500,500,500},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        spell_pshocking_blade,	TAR_OBJ_INV,		POS_STANDING,
	NULL,                   SLOT(501),       1,     1,
        "",			"!Shocking Blade!",	"$p looses its electrical charge.",
	1, 15, 10000, 100000
    },

    {
        "pflame blade",
        {500,500,500,500,500,500,500,500,500,500,500,500,500,500,500},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        spell_pflame_blade,	TAR_OBJ_INV,		POS_STANDING,
	NULL,                   SLOT(502),       1,     1,
        "",			"!Flame Blade!",	"$p looses its fiery glow.",
	1, 15, 10000, 100000
    },

    {
        "pfrost blade",
        {500,500,500,500,500,500,500,500,500,500,500,500,500,500,500},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        spell_pfrost_blade,	TAR_OBJ_INV,		POS_STANDING,
	NULL,                   SLOT(503),       1,     1,
        "",			"!Frost Blade!",	"$p warms back up.",
	1, 15, 10000, 100000
    },

    {
        "pvorpal blade",
        {500,500,500,500,500,500,500,500,500,500,500,500,500,500,500},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        spell_pvorpal_blade,	TAR_OBJ_INV,		POS_STANDING,
	NULL,                   SLOT(505),       1,     1,
        "",			"!Vorpal Blade!",	"$p becomes quite dull.",
	1, 15, 10000, 100000
    }, 
    {
        "psharp blade",
        {500,500,500,500,500,500,500,500,500,500,500,500,500,500,500},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        spell_psharp_blade,	TAR_OBJ_INV,		POS_STANDING,
	NULL,                   SLOT(0),       1,     1,
        "",			"!Sharp Blade!",	"$p becomes quite dull.",
	1, 15, 10000, 100000
    },
    {
        "ppoison blade",
        {500,500,500,500,500,500,500,500,500,500,500,500,500,500,500},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        spell_ppoison_blade,	TAR_OBJ_INV,		POS_STANDING,
	NULL,                   SLOT(505),       400,     12,
        "",			"!Poison Blade!",	"$p looses its venomous bite.",
	1, 15, 10000, 100000
    }, 




/**** SKILLS SECTION ****/

/* Weapon Skills */

    {
	"sword",
      { 101, 101, 101,  8, 6, 1, 1, 1, 1, 101, 101, 101, 101, 101, 101 },
      { 0, 0, 0, 4, 3, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0 },
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_sword,            	SLOT( 0),       0,      0,
        "",                     "!sword!",		"",
	1, 2, 800, 1000
    },

    {
        "dagger",
        {  1, 101, 1, 1, 1, 1, 26, 1, 1, 10, 101, 101, 101, 101, 101 },
        { 2, 0, 2, 2, 2, 2, 6, 2, 2, 2, 0, 0, 0, 0, 0 },
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_dagger,            SLOT( 0),       0,      0,
        "",                     "!Dagger!",		"",
	1, 2, 800, 1000
    },

    {
	"spear",
      {  1,  21,  101,  20, 15, 10, 3, 12, 5, 20, 101, 101, 101, 101, 101 },
      { 5, 6, 0, 3, 3, 2, 2, 2, 2, 5, 0, 0, 0, 0, 0 },
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_spear,            	SLOT( 0),       0,      0,
        "",                     "!Spear!",		"",
	1, 2, 50, 1000
    },

    {
	"mace",
      { 5,  1,  5,  4, 4, 1, 1, 1, 3,40, 101, 101, 101, 101, 101 },
      { 5, 2, 5, 3, 3, 2, 2, 2, 2, 5, 0, 0, 0, 0, 0 },
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_mace,            	SLOT( 0),       0,      0,
        "",                     "!Mace!",		"",
	1, 2, 800, 1000
    },
 
    {
	"axe", 
        { 101, 101, 101, 101, 12, 3, 6, 5, 10, 101, 101, 101, 101, 101, 101 }, 
        { 0, 0, 0, 0, 4, 2, 3, 3, 3, 0, 0, 0, 0, 0, 0 },
      spell_null,             TAR_IGNORE,             POS_FIGHTING,
      &gsn_axe,            	SLOT( 0),       0,      0,
      "",                     "!Axe!",		"",
	1, 2, 800, 1000
    },

    {
	"flail",
      { 101,  1, 101, 101, 101, 8, 10, 10, 101, 101, 101, 101, 101, 101, 101 },
      { 0, 3, 0, 0, 0, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0 },
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_flail,            	SLOT( 0),       0,      0,
        "",                     "!Flail!",		"",
	1, 1, 600, 1000
    },

    {
	"whip",
      { 101, 101,  23,101,101, 5, 101, 5, 101, 101, 101, 101, 101, 101, 101 },
      { 0, 0, 3, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0 },
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_whip,            	SLOT( 0),       0,      0,
        "",                     "!Whip!",	"",
	1, 2, 100, 1000
    },

    {
	"polearm",
      { 101, 14, 101,  101, 101, 12, 16, 16, 101, 101, 101, 101, 101, 101, 101 },
      { 0, 6, 0, 0, 0, 3, 3, 3, 0, 0, 0, 0, 0, 0, 0 },
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_polearm,           SLOT( 0),       0,      0,
        "",                     "!Polearm!",		"",
	1, 2, 100, 1000
    },

    {
	"exotic",
      { 101, 101, 101,  101, 101, 85, 95, 95, 95, 101, 101, 101, 101, 101, 101 },
      { 0, 0, 0, 0, 0, 8, 10, 10, 10, 0, 0, 0, 0, 0, 0 },
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_exotic,            SLOT( 0),       0,      0,
        "",                     "!Exotic!",		"",
	1, 2, 500, 1000
    },
    
/* ENDWEAPONS */

/* Combat Skills */


      {
        "assassinate",
        { 101, 101,  101,101,28, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101 },
        { 0, 0, 0, 0,7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        spell_null,             TAR_IGNORE,             POS_STANDING,
        &gsn_assassinate,          SLOT( 0),        0,     24,
        "assassinate",             "!Assassinate!",		"",
	1, 5, 1500, 5000
    },

      {
        "backstab",
        { 101, 101,  101, 15, 101, 101, 101, 27, 101, 101, 101, 101, 101, 101, 101 },
        { 0, 0, 0, 4, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0 },
        spell_null,             TAR_IGNORE,             POS_STANDING,
        &gsn_backstab,          SLOT( 0),        0,     24,
        "backstab",             "!Backstab!",		"",
	1, 5, 1500, 5000
    },

    { 
	"bash",
      { 101,101,101, 101, 101, 3, 30, 17, 101, 101, 101, 101, 101, 101, 101 },
      { 0, 0, 0, 0, 0, 3, 5, 4, 0, 0, 0, 0, 0, 0, 0 },
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_bash,            	SLOT( 0),       0,      30,
        "bash",                 "!Bash!",		"",
	1, 3, 1000, 1500
    },

    {
	"berserk",
      { 101,101,101, 101, 101, 15, 101, 40, 101, 101, 101, 101, 101, 101, 101 },
      { 0, 0, 0, 0, 0, 5, 0, 8, 0, 0, 0, 0, 0, 0, 0 },
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_berserk,        	SLOT( 0),       0,      24,
        "",                     "You feel your pulse slow down.",	"",
	1, 2, 125, 1000
    },

    {
       "circle",
       { 101, 101, 101,25, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101 },
       {0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        spell_null,             TAR_IGNORE,             POS_STANDING,
        &gsn_circle,            SLOT( 0 ),      0,      16,
        "swift circle attack",  "!Circle!", "",
	1, 5, 800, 2500
    },

    {
	"dirt kicking",
      {101, 101,101,  13, 19, 7, 101, 3, 10, 101, 101, 101, 101, 101, 101 },
      { 0, 0, 5, 4, 4, 2, 0, 4, 4, 0, 0, 0, 0, 0, 0 }, 
	spell_null,		TAR_IGNORE,		POS_FIGHTING,
	&gsn_dirt,		SLOT( 0),	0,	24,
	"kicked dirt",		"You rub the dirt out of your eyes.",	"",
	1, 2, 275, 1000
    },

    {
        "disarm",
        { 101, 101, 101, 50, 38, 15, 20, 19, 22, 101, 101, 101, 101, 101, 101 },
        { 0, 0, 0, 7, 6, 4, 4, 5, 4, 0, 0, 0, 0, 0, 0 },
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_disarm,            SLOT( 0),        0,     24,
        "",                     "!Disarm!",		"",
	1, 3, 150, 1000
    },
 
    {
        "dual wield", 
        { 101, 101, 101, 52, 30, 23, 26, 25, 24, 101, 101, 101, 101, 101, 101 },  
        { 0, 0, 0, 10, 8, 6, 7, 6, 7, 0, 0, 0, 0, 0, 0 },
        spell_null,             TAR_IGNORE,             POS_RESTING,
        &gsn_dualwield,         SLOT( 0),        0,     12,
        "",                     "!Dual Wield!",		"",
	1, 5, 1750, 6000
    },
 
    {
        "enhanced damage",
        { 101, 101, 101,101, 25, 6, 15, 8, 20, 101, 101, 101, 101, 101, 101 },
        { 0, 0, 0, 0, 6, 4, 7, 5, 6, 0, 0, 0, 0, 0, 0},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_enhanced_damage,   SLOT( 0),        0,     0,
        "",                     "!Enhanced Damage!",	"",
	1, 1, 375, 1200
    },

    {
	"hand to hand",
      { 50, 45, 30, 20, 13, 6, 10, 6, 7, 1, 101, 101, 101, 101, 101 },
      { 8, 5, 8, 6, 5, 4, 5, 4, 4, 2, 0, 0, 0, 0, 0 },
	spell_null,		TAR_IGNORE,		POS_FIGHTING,
	&gsn_hand_to_hand,	SLOT( 0),	0,	0,
	"",			"!Hand to Hand!",	"",
	1, 2, 250, 1000
    },

    {
        "kick",
        { 101, 101, 101,  6, 10, 1, 8, 6, 4, 101, 101, 101, 101, 101, 101 },
        { 0, 0, 0, 4, 3, 2, 3, 2, 2, 0, 0, 0, 0, 0, 0 },
        spell_null,             TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
        &gsn_kick,              SLOT( 0),        0,     12,
        "kick",                 "!Kick!",		"",
	1, 2, 800, 1000
    },

    {
        "dodge", 
        { 20, 20,  17, 16, 10, 4, 12, 12, 11, 20, 101, 101, 101, 101, 101 },  
        { 8, 8, 8, 4, 4, 6, 6, 6, 6, 8, 0, 0, 0, 0, 0 },
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_dodge,             SLOT( 0),        0,     0,
        "",                     "!Dodge!",		"",
	1, 2, 400, 1250
    },

    {
        "parry",
        { 101,101,101,101,25, 15, 22, 22, 21, 101, 101, 101, 101, 101, 101 },
        { 0, 0, 0, 0, 3, 2, 4, 4,3, 0, 0, 0, 0, 0, 0 },
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_parry,             SLOT( 0),        0,     0,
        "",                     "!Parry!",		"",
	1, 2, 350, 1000
    },

    {
	"shield block",
      { 101, 101, 101, 101, 23, 1, 1, 1,15, 101, 101, 101, 101, 101, 101 },
      { 0, 0, 0, 0, 5, 3, 3, 3, 4, 0, 0, 0, 0, 0, 0},
	spell_null,		TAR_IGNORE,		POS_FIGHTING,
	&gsn_shield_block,	SLOT(0),	0,	0,
	"",			"!Shield!",		"",
	1, 2, 800, 1000
    },

    {
        "rescue",
        { 101,101,101, 101, 101, 2, 2, 101, 9, 101, 101, 101, 101, 101, 101 },
        { 0, 0, 0, 0, 0, 4, 5, 0, 4, 0, 0, 0, 0, 0, 0},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_rescue,            SLOT( 0),        0,     12,
        "",                     "!Rescue!",		"",
	0, 0, 0, 0
    },

    {
	"trip", 
      { 101,101,  101, 3, 2, 15, 101, 9, 18, 101, 101, 101, 101, 101, 101 },
      { 0, 0, 0, 4, 4, 3, 0, 4, 4, 0, 0, 0, 0, 0, 0},
	spell_null,		TAR_IGNORE,		POS_FIGHTING,
	&gsn_trip,		SLOT( 0),	0,	24,
	"trip",			"!Trip!",		"",
	1, 2, 75, 1000
    },

    {
        "second attack", 
        { 35, 30, 27, 22, 22, 17, 20, 20, 20, 101, 101, 101, 101, 101, 101 }, 
        { 10, 9, 8, 6, 5, 3, 3, 3, 4, 0, 0, 0, 0, 0, 0 },
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_second_attack,     SLOT( 0),        0,     0,
        "",                     "!Second Attack!",	"",
	1, 3, 1500, 2500
    },

    {
        "third attack", 
        { 101, 101, 101, 101, 40, 32, 36, 34, 34, 101, 101, 101, 101, 101,101 }, 
        { 0, 0, 0, 0, 8, 4, 6, 5, 6, 0, 0, 0, 0, 0, 0},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_third_attack,      SLOT( 0),        0,     0,
        "",                     "!Third Attack!",	"",
	1, 6, 1200, 4000
    },

    {
        "fourth attack",
        { 102, 102, 102, 102, 102, 50, 102, 102, 101, 101, 101, 101, 101, 101, 101},
        { -1, -1, -1, -1, -1, 10, -1, -1, 0, 0, 0, 0, 0, 0, 0},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_fourth_attack,      SLOT( 0),        0,     0,
        "",                     "!Fourth Attack!",       "",
	1, 7, 3000, 5000
    },

/* ENDCOMABT */


/* Non-combat */

    {
	"envenom",
      { 101, 101, 101,25, 16, 101, 101, 17, 101, 8, 101, 101, 101, 101, 101 },
      { 0, 0, 0, 4, 3, 0, 0, 3, 0, 2, 0, 0, 0, 0, 0 },
	spell_null,		TAR_IGNORE,	  	POS_RESTING,
	&gsn_envenom,		SLOT(0),	0,	36,
	"",			"!Envenom!",		"",
	1, 2, 65, 1000
    },

    { 
	"fast healing",
      { 50, 101, 15, 35, 33, 26, 22, 22, 8, 101, 101, 101, 101, 101, 101 },
      { 8, 0, 5, 7, 5, 5, 4, 4, 3, 0, 0, 0, 0, 0, 0 },
	spell_null,		TAR_IGNORE,		POS_SLEEPING,
	&gsn_fast_healing,	SLOT( 0),	0,	0,
	"",			"!Fast Healing!",	"",
	1, 2, 450, 1000
    },

    {
	"haggle",
      { 10, 8,  10, 22, 37, 35, 40, 26, 101, 5, 101, 101, 101, 101, 101 }, 
      { 4, 4, 4, 3, 6, 4, 6, 5, 0, 2, 0, 0, 0, 0, 0 },
	spell_null,		TAR_IGNORE,		POS_RESTING,
	&gsn_haggle,		SLOT( 0),	0,	0,
	"",			"!Haggle!",		"",
	1, 2, 175, 1000
    },

    {
	"hide",
      { 101, 101,101, 1, 13,101, 101, 7, 1, 12, 101, 101, 101, 101, 101 },
      { 0, 0, 0, 3, 4, 0, 0, 5, 3, 4, 0, 0, 0, 0, 0},
	spell_null,		TAR_IGNORE,		POS_RESTING,
	&gsn_hide,		SLOT( 0),	 0,	12,
	"",			"!Hide!",		"",
	1, 2, 800, 1000
    },

    {
	"meditation",
      {  3,  2, 3, 101, 101, 101, 5, 15, 101, 3, 101, 101, 101, 101, 101 },
      { 3, 2, 3, 0, 0, 0, 4, 6, 0, 3, 0, 0, 0, 0, 0},
	spell_null,		TAR_IGNORE,		POS_SLEEPING,
	&gsn_meditation,	SLOT( 0),	0,	0,
	"",			"Meditation",		"",
	1, 1, 600, 1000
    },

    {
	"peek",
      {  101, 101,  101, 2,101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101 },
      { 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_peek,		SLOT( 0),	 0,	 0,
	"",			"!Peek!",		"",
	1, 2, 800, 1000
    },

    {
	"pick lock",
      { 101, 101,  101, 1, 27, 101, 101, 40, 101, 101, 101, 101, 101, 101, 101 }, 
      { 0, 0, 0, 4, 6, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0},
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_pick_lock,		SLOT( 0),	 0,	12,
	"",			"!Pick!",		"",
	1, 1, 600, 1000
    },

    {
	"sneak",
      { 101,101,  101, 12, 12, 101, 101, 38, 5, 101, 101, 101, 101, 101, 101 },
      { 0, 0, 0, 4, 2, 0, 0, 6, 6, 0, 0, 0, 0, 0, 0},
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_sneak,		SLOT( 0),	 0,	12,
	"",			"You no longer feel stealthy.",	"",
	1, 2, 350, 1575
    },

    {
	"steal",
      { 101, 101,  101, 22, 101, 101, 101,101, 101, 101, 101, 101, 101, 101, 101 },
      { 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_steal,		SLOT( 0),	 0,	24,
	"",			"!Steal!",		"",
	1, 1, 600, 1000
    },

    {
	"scrolls",
      {  1,  3,  5,101,101,101, 3, 5, 101, 1, 101, 101, 101, 101, 101 },
      { 2, 3, 3, 0, 0, 0, 4, 5, 0, 1, 0, 0, 0, 0, 0},
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_scrolls,		SLOT( 0),	0,	24,
	"",			"!Scrolls!",		"",
	1, 1, 600, 1000
    },

    {
	"staves",
      {  1,  3,  5,101,101,101,101,101,101, 1, 101, 101, 101, 101, 101 },
      { 2, 3, 4, 0, 0, 0, 0, 0,0, 1, 0, 0, 0, 0, 0 },
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_staves,		SLOT( 0),	0,	12,
	"",			"!Staves!",		"",
	1, 1, 600, 1000
    },
    
    {
	"wands",
      {  1,  3,  5,101,101,101,101,101,101, 1, 101, 101, 101, 101, 101 },
      { 2, 3, 4,0,0,0, 0, 0, 0, 1, 0, 0, 0, 0, 0 },
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_wands,		SLOT( 0),	0,	12,
	"",			"!Wands!",		"",
	1, 1, 600, 1000
    },

    {
	"recall",
      {  1,  1,  1,  1, 1, 1, 1, 1, 1, 1, 101, 101, 101, 101, 101 },
      { 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0 },
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_recall,		SLOT( 0),	0,	12,
	"",			"!Recall!",		"",
	1, 10, 1000, 3000
    },

/* New Skill/Spellss */ 
 /*  Skill/Spell Blank
    {
        "",
        { 101,101,101,101,101,101,101,101,101,101,101,101,101,101,101 },
        { -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
        spell_,                 TAR_,			POS_,
        &gsn_,			SLOT( 0),        0,     0,
        "",                     "!!",    ""
    },
*/

    {
        "spellcraft",
        { 15,25,101,101,101,101,101,101,101,1,101,101,101,101,101 },
        { 5,6,-1,-1,-1,-1,-1,-1,-1,2,-1,-1,-1,-1,-1},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_spellcraft,	SLOT( 0),        0,     0,
        "{MSpellCraft{x",     "{R!{WSPELLCRAFT{R!{x",    "",
	1, 2, 150, 1000
    },

    {
        "companion",              
        {101,101,101,101,101,101,101,101,27,101,101,101,101,101,101 },            
        { -1, -1,-1,-1,-1,-1,-1,-1,5,-1,-1,-1,-1,-1,-1},             
        spell_null,             TAR_CHAR_DEFENSIVE,             POS_SITTING,
        &gsn_companion,           SLOT( 0),       0,      0,
        "",             "!Companion!",   "",
	1, 3, 275, 1000
    },

    {
        "track",              
        {101,101,101,101,101,101,101,101,15,101,101,101,101,101,101 },            
        { -1, -1,-1,-1,-1,-1,-1,-1,4,-1,-1,-1,-1,-1,-1},             
        spell_null,             TAR_IGNORE,    POS_STANDING,
        &gsn_track,           SLOT( 0),       0,      0,
        "",             "!Track!",   "",
	1, 3, 150, 2500
    },


   { "whirlwind",		
      { 101,101,101, 101, 101, 101, 101, 101, 50, 101, 101, 101, 101, 101, 101 },
        { -1, -1,-1,  -1, -1, -1, -1, -1, 9, -1, -1, -1, -1, -1, -1},
        spell_null,		TAR_IGNORE,		POS_STANDING,
        &gsn_whirlwind,	SLOT(0),0, 20,   /*bob*/
        "{gWhi{GrlW{Wind{x","!Whirlwind!","",
	1, 7, 3000, 3500
    },

    { "heighten senses",
        { 102,102, 35,102,102,102,102,102,102,102,102,102,102,102,102},
        { -1,  -1, 7,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        spell_null,                   TAR_CHAR_SELF,         POS_STANDING,
        &gsn_heighten_senses,        SLOT(0),   30,                 12,
        "", "Your senses return to normal", "",
	1, 2, 120, 1000
    },

    {
        "chameleon power",
        { 102,102, 8, 102, 102,102,102,102,102,102,102,102,102,102,102},
        { -1,  -1, 3,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        spell_null,        TAR_CHAR_SELF,POS_STANDING,
        &gsn_chameleon_power,        SLOT(0),  5,12,
        "", "You no longer blend with your surroundings.", "",
	1, 2, 800, 1000
    },

   {
      "create zombie",
      { 102, 38,102,102,102,102,102,102,102,102,102,102,102,102,102},
      { -1, 2,-1,-1,-1,-1,-1,-1,-1,-1, -1, -1, -1, -1, -1},
      spell_create_zombie,   TAR_IGNORE,   POS_STANDING,
      NULL,           SLOT( 0 ),   100,  15,
      "",   "!Create Zombie!", "",
	1, 3, 175, 1000
    }, 


    {
      "metamorph",
      { 500,500,500,500,500,500,500,500,500,500,500,500,500,500,500},
      { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
      spell_metamorph,   TAR_CHAR_DEFENSIVE,   POS_STANDING,
      NULL,           SLOT( 0 ),   100,  15,
      "",   "!Metamorph!", "",
	1, 3, 200, 100
    }, 

    {
        "shadow form",
        { 102, 102, 20, 102, 102,102,102,102,102,102,102,102,102,102,102},
        {  -1,  -1,  5,  -1,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        spell_null,          TAR_CHAR_SELF,     POS_STANDING,
        &gsn_shadow_form,      SLOT(0),      8,       12,
        "", "You no longer move in the shadows.", "",
	1, 2, 75, 1000
    },

 {
      "multiburst",
      { 45,102,102,102,102,102,102,102,102,102,102,102,102,102,102},
      {  8,  -1,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
      spell_null,   TAR_IGNORE,   POS_FIGHTING,
      &gsn_multiburst,  SLOT( 0 ),  0,24,
      "",   "!multiburst!", "",
	1, 5, 1000, 1500
    }, 

    {
      "forsake",
      { 102, 102,102,102,102,102,102,40,102,102,102,102,102,102,102},
      { -1, -1,-1,-1,-1,-1,-1,3,-1,-1, -1, -1, -1, -1, -1},
      spell_forsake,   TAR_CHAR_OFFENSIVE,   POS_FIGHTING,
      NULL,           SLOT( 206 ),   20,  12,
      "",   "You are no longer forsaken.", "",
	1, 4, 1200, 1500
    },

    {
      "divine wrath",
      { 102,65,102,102,102,102,85,102,102,102,102,102,102,102,102},
      { -1, 1,-1,-1,-1,-1,2,-1,-1,-1, -1, -1, -1, -1, -1},
      spell_divine_wrath,   TAR_CHAR_OFFENSIVE,   POS_FIGHTING,
      NULL,           SLOT( 207 ), 75,  12,
      "",   "\n\rYou are no longer out of favor with any Gods!{x\n\r", "",
	1, 4, 750, 1000
    },

    {
        "immdis magic",
        { 104, 104, 104, 104, 104,104,104,104,101,101,101,101,101,101,101},
        { 1,  1,  1,  1, 1, 1, 1,1,1, 1, 1, 1, 1, 1, 1},
	spell_immdis_magic,	TAR_CHAR_DEFENSIVE, POS_STANDING,
	NULL,			SLOT(610),	 0,1,
	"",			"!Immortal Dispel Magic!",	"",
	1, 3, 200, 100
    },

    {
       "quicken", 
        { 102, 102,102,33, 102,102,102,102,102,102,102,102,102,102,102},
        { -1, -1,-1, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_quicken,		SLOT( 0 ),	0,	36,
	"",			"You feel yourself slow down.", "",
	1, 3, 175, 1000
    },


    {
       "brew",
         { 68, 102, 102, 102, 102,102,102,102,102,15,102,102,102,102,102},
         {8, -1, -1, -1, -1, -1, -1, -1, -1, 3, -1, -1, -1, -1, -1},
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_brew,	SLOT( 0),	0,	 24,
	"",			"!Brew!", "",
    1, 4, 300, 1000
    },
    {
	"scribe",
      { 102, 68, 102, 102, 102,102,102,102,102,15,102,102,102,102,102},
      {-1, 8, -1, -1, -1, -1, -1, -1, -1, 3, -1, -1, -1, -1, -1}, 
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_scribe,	SLOT( 0),	0,	 24,
	"",			"!Scribe!", "",
	1, 4, 300, 1000
    },   

/*
    {
      "vampiric blade",
{ 75,75,75,75,75,75,75,75,75,75,75,102,102,102,102},
        { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1, 1, -1, -1, -1, -1},
        spell_vampiric_blade,	TAR_OBJ_INV,		POS_STANDING,
	NULL,                   SLOT(0),   700,     12,
        "",			"!Vampiric Blade!",	"$p looks less malevolant."
    },
    {
        "shocking blade",
        { 65,65,65,65,65,65,65,65,65,65,65,102,102,102,102},
      { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1, 1, -1, -1, -1, -1},
        spell_shocking_blade,	TAR_OBJ_INV,		POS_STANDING,
	NULL,                   SLOT(0),      600,     12,
        "",			"!Shocking Blade!",	"$p looses its electrical charge."
    },
    {
        "flaming blade",
        { 65,65,65,65,65,65,65,65,65,65,65,102,102,102,102},
      { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1, 1, -1, -1, -1, -1},
        spell_flaming_blade,	TAR_OBJ_INV,		POS_STANDING,
	NULL,                   SLOT(0),     600,     12,
        "",			"!Flame Blade!",	"$p looses its fiery glow."
    },
    {
        "frost blade",
      { 55,55,55,55,55,55,55,55,55,55,55,102,102,102,102},
      { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1, 1, -1, -1, -1, -1},
        spell_frost_blade,	TAR_OBJ_INV,		POS_STANDING,
	NULL,                   SLOT(0),    550,     12,
        "",			"!Frost Blade!",	"$p warms back up."
    },
    {
        "vorpal blade",
      { 55,55,55,55,55,55,55,55,55,55,55,102,102,102,102},
      { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1, 1, -1, -1, -1, -1},
        spell_vorpal_blade,	TAR_OBJ_INV,		POS_STANDING,
	NULL,                   SLOT(505),       500,     12,
        "",			"!Vorpal Blade!",	"$p becomes quite dull."
    },
    {
        "poison blade",
        { 55,55,55,55,55,55,55,55,55,55,55,102,102,102,102},
      { 1,  1,  1,  1, 1, 1, 1, 1, 1, 1, 1, -1, -1, -1, -1},
        spell_poison_blade,	TAR_OBJ_INV,		POS_STANDING,
	NULL,                   SLOT(505),       400,     12,
        "",			"!Poison Blade!",	"$p looses its venomous bite."
    }, 


*/
    {
        "sharp blade",
        { 105, 105,105,105, 105,55,105,105,105,105,105,105,105,105,105},
        { -1, -1,-1,-1, -1,2, -1, -1,-1, -1, -1, -1, -1, -1, -1},
        spell_sharp_blade,	TAR_OBJ_INV,		POS_STANDING,
	NULL,                   SLOT(0),       300,     12,
        "",			"!Sharp Blade!",	"$p becomes quite dull.",
	1, 2, 200, 1000
    },

    {
        "critical strike",
        { 105, 105,105,105, 105,105,45,105,105,105,105,105,105,105,105},
        { -1, -1,-1,-1, -1,-1, 7, -1,-1, -1, -1, -1, -1, -1, -1},
        spell_null,	TAR_IGNORE,		POS_STANDING,
	&gsn_critical_strike,                   SLOT(0),       0,     16,
        "{CCRITICAL STRIKE{x",			"!Critical Strike!",	"",
	1, 5, 450, 2500
    },

    {
	"clan recall",
      {  20,  20,  20,  20, 20, 20, 20, 20, 20, 20, 101, 101, 101, 101,101 },
      { 0,0, 0,0,0,0,0,0, 0, 0, 0, 0, 0, 0, 0 },
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_crecall,		SLOT( 0),	0,	12,
	"",			"!Clan Recall!",		"",
	1, 3, 1000, 5000
    }

};
/*  Legend level, legend train cost, legend questpoint cost, legend gold cost */
const   struct  group_type      group_table     [MAX_GROUP]     =
{

    {
	"rom basics", { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ "recall" }
    },

    {
	"mage basics",	{ 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ "dagger" }
    },

    {
	"cleric basics",	{ -1, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ "mace" }
    },
    
    {
      "psionic basics", { -1, -1, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
      { "dagger" }
    },

    {
	"thief basics", { -1, -1, -1, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ "dagger" }
    },

    {
      "assassin basics", { -1, -1, -1, -1, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
      { "dagger" }
    },

    {
	"warrior basics",	{ -1, -1, -1, -1, -1, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ "sword" }
    },

    {
       "paladin basics", { -1, -1, -1, -1, -1, -1, 0, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ "sword" }
    },

    {
       "antipaladin basics", { -1, -1, -1, -1, -1, -1, -1, 0, -1, -1, -1, -1, -1, -1, -1},
        { "sword" }
    },

    {
       "ranger basics", { -1, -1, -1, -1, -1, -1, -1, -1, 0, -1, -1, -1, -1, -1,-1},
        { "sword" }
    },


    {
	"enchanter basics",	{ -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, -1, -1, -1, -1, -1 },
	{ "dagger", "spellcraft" }
    },


/* Class Default Groups */
    {
	"mage default",
      { 32, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
      { "beguiling", "offensive", "detection", "illusion", "full transport","weather"}
    },

    {
	"cleric default",
      { -1, 35, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ "attack", "creation", "benedictions", "healing", "maladictions", "full protective" }
    },
 
    {
      "psionic default",
      { -1, -1, 40, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
      {  "psi general", "psi attack", "psi enhance" }
    },

    {
	"thief default",
      { -1, -1, -1, 28, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ "backstab", "dodge", "second attack", "trip", "hide", "peek",
        "pick lock", "sneak" }
    },

    {
      "assassin default",
      { -1, -1, -1, -1, 30, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
      { "dodge", "second attack", "hide",  
        "envenom", "parry", "enhanced damage", "assassinate" }
    },

    {
	"warrior default",
      { -1, -1, -1, -1, -1, 25, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ "shield block", "bash", "disarm", "enhanced damage", 
	  "parry", "second attack", "third attack", "dual wield" }
    },

    {
      "paladin default",
      { -1, -1, -1, -1, -1, -1, 31, -1, -1, -1, -1, -1, -1, -1, -1 },
      { "parry", "second attack", "shield block", "rescue", 
        "dodge", "disarm", "third attack", "fast healing" }
    },                      
	
    {
      "antipaladin default",
      { -1, -1, -1, -1, -1, -1, -1, 34, -1, -1, -1, -1, -1, -1, -1 },
      { "parry", "second attack", "third attack", "enhanced damage",
        "backstab", "shield block", "hide", "dodge" }
    },                      

    {
      "ranger default",
      { -1, -1, -1, -1, -1, -1, -1, -1, 38, -1, -1, -1, -1, -1, -1 },
      { "hide", "sneak", "fast healing", "second attack", "track", 
        "companion", "dagger", "dodge", "disarm"}
    },                      

    {
	"weaponsmaster",
      { -1, -1, -1, -1,-1,19, 25, 22, -1, -1, -1, -1, -1, -1, -1 },
      { "sword", "dagger", "spear", "mace", "axe", "flail", "whip", "polearm", "exotic" }
    },

    {
	"enchanter default",
      { -1, -1, -1, -1, -1, -1, -1, -1, -1, 2, -1, -1, -1, -1, -1 },
      { "basic enchantment", "minor enchantment", "major enchantment"}
    },

/* END CLASS DEFAULT */

/* Spell Groups */

    {
	"attack",
      { -1, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ "demonfire", "dispel evil", "dispel good", "earthquake", 
	  "flamestrike", "heat metal", "ray of truth" }
    },

    {
	"beguiling",
      { 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ "calm", "charm person", "sleep" }
    },

    {
	"benedictions",
      { -1, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ "bless", "calm", "frenzy", "holy word", "remove curse",
          "mass invis", "infravision"}
    },

    {
	"creation",
      { 4, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ "continual light", "create food", "create spring", "create water",
	  "create rose", "floating disc", "create zombie" }
    },

    {
	"curative",
      { -1, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ "cure blindness", "cure disease", "cure poison" }
    }, 

    {
	"detection",
      { 8, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
 	{ "detect evil", "detect good", "detect hidden", "detect invis", 
	  "detect magic", "detect poison", "farsight", "identify", 
	  "know alignment", "locate object" }
    },

    {
	"enchantment",
      { 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ "enchant armor", "enchant weapon", "recharge" }
    },

    {
	"basic enchantment",
      { -1, -1, -1, -1, -1, -1, -1, -1, -1, 1, -1, -1, -1, -1, -1 },
	{ "recharge"}
    },

    {
	"minor enchantment",
      { -1, -1, -1, -1, -1, -1, -1, -1, -1, 1, -1, -1, -1, -1, -1 },
	{ "enchant armor", "enchant weapon","fireproof" }
    },

    {
	"major enchantment",
      { -1, -1, -1, -1, -1, -1, -1, -1, -1, 1, -1, -1, -1, -1, -1 },
	{ "fortify object" }
    },

    { 
	"enhancement",
      { 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ "giant strength", "haste", "infravision", "refresh" }
    },

    {
	"harmful",
      { -1, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ "cause critical", "cause light", "cause serious", "harm" }
    },

    {   
	"healing",
      { -1, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
 	{ "cure critical", "cure light", "cure serious", "heal", 
	  "mass healing", "refresh" }
    },

    {
	"illusion",
      { 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ "invis", "mass invis", "ventriloquate" }
    },
  
    {
	"maladictions",
      { 8, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ "blindness", "curse", "energy drain", "plague", "change sex", 
	  "poison", "slow", "weaken", "divine wrath" }
    },

    { 
	"full protective",
      { -1, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ "armor", "cancellation", "dispel magic",
	  "protection evil", "protection good", "sanctuary", "energy shield", 
	  "stone skin", "golden aura" }
    },

    { 
	"mage protective",
        { 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ "armor", "cancellation", "dispel magic", 
          "energy shield", "stone skin", "sanctuary" }
    },

    {
	"offensive",
      { 12, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ "magic missile", "chill touch", "burning hands", "shocking grasp",
          "colour spray", "fireball", "lightning bolt", "acid blast",
          "spellfire", "orb of chaos", "chain lightning", "wyldstryke",
          "acid storm", "wail of the banshee", "torment of cold", "sonic scream"  
	}
    },

    {
	"full transport",
      { 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ "fly", "gate", "nexus", "pass door", "portal", "summon", "teleport", 
	  "word of recall" }
    },

    {
	"cleric transport",
      { -1, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ "fly", "gate", "summon", "teleport", "word of recall" }
    },

    {
	"weather",
      { 5, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ "call lightning", "control weather", "faerie fire", "faerie fog",
	  "lightning bolt" }
    },

/* Psionic Groups */

    {
     "psi enhance", 
    { -1,  -1,  15,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    { "adrenaline control", "thought shield", "aura sight", "mental barrier", 
      "displacement", "share strength", "combat mind", "enhanced strength",
      "bio fortress", "energy containment", "enhance armor", "flesh armor",
      "inertial barrier"  }
    },

    {
     "psi attack",
        { -1,  -1,  15,  -1, -1, -1, -1, -1, -1, -1,  -1, -1, -1, -1, -1},
     { "disintegrate", "essence drain", "agitation", "ballistic attack",
       "control flame", "death field", "detonate", "ego whip", "inflict pain",
       "mind thrust", "project force", "psionic blast", "psychic crush",
       "psychic drain", "ultrablast" }
    },


    {
     "psi general",
     { -1,  -1,  15,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
     {  "levitation", "psychic healing", "awe", "obfuscate", "identify",
        "cell adjustment", "apport", "ectoplasmic form", "domination",
        "teleport",  "lend health", "complete healing" }
     },

/* END SPELL GROUPS */

/* Class Specific Spell Groups */	

    {
      "identify spell",
      { -1, -1, -1, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ "identify" }
    },
    
    {
      "invisibility spell",
      { -1, -1, -1, -1, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
      { "invisibility" }
    },

    {
      "warrior spells",
      { -1, -1, -1, -1, -1, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
      { "sharp blade", "giant strength"}
    },
    
    {
      "paladin spells",
      { -1, -1, -1, -1, -1, -1, 28, -1, -1, -1, -1, -1, -1, -1, -1 },
      {
       "cure light", "detect evil", "protection evil", "cure serious", "bless",
       "dispel evil", "cure critical", "word of recall", "cure disease", "heal",
       "sanctuary"
      }
    },
   
    {
      "antipaladin spells",
      { -1, -1, -1, -1, -1, -1, -1, 28, -1, -1, -1, -1, -1, -1, -1 },
      { "cause light", "detect good", "cause serious", "protection good",
        "cause critical", "dispel good", "giant strength","harm", "forsake" }
    },

    {
      "ranger spells",
      { -1, -1, -1, -1, -1, -1, -1, -1, 20, -1, -1, -1, -1, -1, -1 },
      { "farsight", "infravision", "detect evil", "protection evil", 
        "cure serious", "detect poison", "detect hidden", "detect invis", 
        "dispel evil", "cure poison", "remove curse"}
    },

};
