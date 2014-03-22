void do_function args((CHAR_DATA *ch, DO_FUN *do_fun, char *argument));

/* for command types */
#define ML 	MAX_LEVEL	/* IMPLEMENTOR */
#define AI	MAX_LEVEL - 1   /* ASSTIMP */
#define GG	MAX_LEVEL - 379	/* GGOD*/
#define LG	MAX_LEVEL - 384	/* LGOD */
#define DG 	MAX_LEVEL - 389	/* DGOD */
#define VI 	MAX_LEVEL - 395	/* VETIMM */
#define NI	MAX_LEVEL - 396 /* NEWIMM */
#define BL	MAX_LEVEL - 397 /* BUILDER */
#define VS	MAX_LEVEL - 398 /* VISIMM */
#define IM	LEVEL_IMMORTAL 	/* STARTING IMMORTAL LEVEL */
#define HE	LEVEL_HERO	/* HERO */

#define COM_INGORE	1


/*
 * Structure for a command in the command lookup table.
 */
struct	cmd_type
{
    char * const	name;
    DO_FUN *		do_fun;
    sh_int		    position;
    sh_int		    level;
    sh_int		    log;
    sh_int          show;
};

/* the command table itself */
extern	const	struct	cmd_type	cmd_table	[];

/*
 * Command functions.
 * Defined in act_*.c (mostly).
 */
DECLARE_DO_FUN( do_addlag	);
DECLARE_DO_FUN( do_auction	);
DECLARE_DO_FUN(	do_advance	);
DECLARE_DO_FUN( do_affects	);
DECLARE_DO_FUN( do_afk		);
DECLARE_DO_FUN( do_alia		);
DECLARE_DO_FUN( do_alias	);
DECLARE_DO_FUN(	do_allow	);
DECLARE_DO_FUN( do_rp		);
DECLARE_DO_FUN( do_grant		);    // Grant System
DECLARE_DO_FUN( do_revoke		);    // Grant System
DECLARE_DO_FUN( do_acro );  // Acrophobia Stuff
DECLARE_DO_FUN( do_percentages );   // Explorer/Killer Percentages
DECLARE_DO_FUN( do_percenthint );   // Explorer/Killer Percentages
DECLARE_DO_FUN( do_top_percentages );   // Explorer/Killer Percentages
DECLARE_DO_FUN( do_updatetop ); // Explorer/Killer Percentages
DECLARE_DO_FUN( do_stones );    // Stones of Wisdom Stuff
DECLARE_DO_FUN( do_stones_invite ); // Stones of Wisdom Stuff
DECLARE_DO_FUN( do_stones_accept ); // Stones of Wisdom Stuff
DECLARE_DO_FUN( do_stones_bid );    // Stones of Wisdom Stuff
DECLARE_DO_FUN( do_stones_challenge );  // Stones of Wisdom Stuff
DECLARE_DO_FUN( do_stones_forfeit );    // Stones of Wisdom Stuff
DECLARE_DO_FUN( do_suicide );
DECLARE_DO_FUN( do_account	);
DECLARE_DO_FUN( do_deposit	);
DECLARE_DO_FUN( do_withdraw 	);
DECLARE_DO_FUN( do_psycho	);
DECLARE_DO_FUN(	do_bankshow	);
DECLARE_DO_FUN( do_chip		);
DECLARE_DO_FUN(	do_change	);
DECLARE_DO_FUN(	do_share	);
DECLARE_DO_FUN( do_beep );
DECLARE_DO_FUN( do_outfit);
DECLARE_DO_FUN( do_imp		);
DECLARE_DO_FUN( do_owner	);
DECLARE_DO_FUN(	do_areas	);
DECLARE_DO_FUN( do_arealinks);
DECLARE_DO_FUN(	do_at		);
DECLARE_DO_FUN(	do_auction	);
DECLARE_DO_FUN( do_autoassist	);
DECLARE_DO_FUN( do_autoexit	);
DECLARE_DO_FUN( do_forge	);
DECLARE_DO_FUN( do_autogold	);
DECLARE_DO_FUN( do_autoloot	);
DECLARE_DO_FUN( do_alertme );
DECLARE_DO_FUN( do_autosac	);
DECLARE_DO_FUN( do_autosplit	);
DECLARE_DO_FUN( do_bonus );
DECLARE_DO_FUN( do_bounty );
DECLARE_DO_FUN( do_assassinate );
DECLARE_DO_FUN( do_circle );
DECLARE_DO_FUN( do_critical_strike );
DECLARE_DO_FUN( do_clantalk );
DECLARE_DO_FUN( do_copyove  );
DECLARE_DO_FUN( do_copyover );
DECLARE_DO_FUN( do_ctimp );
DECLARE_DO_FUN( do_cwar );
DECLARE_DO_FUN( do_vent );
DECLARE_DO_FUN(	do_backstab	);
DECLARE_DO_FUN(	do_bamfin	);
DECLARE_DO_FUN(	do_bamfout	);
DECLARE_DO_FUN(	do_ban		);
DECLARE_DO_FUN(	do_bolt 	);
DECLARE_DO_FUN(	do_flame	);
DECLARE_DO_FUN( do_bash		);
DECLARE_DO_FUN( do_berserk	);
DECLARE_DO_FUN( do_board	);
DECLARE_DO_FUN(	do_brandish	);
DECLARE_DO_FUN( do_brief	);
DECLARE_DO_FUN(	do_buy		);
DECLARE_DO_FUN(	do_cast		);
DECLARE_DO_FUN( do_channels	);
DECLARE_DO_FUN( do_choose	);
DECLARE_DO_FUN( do_clans	);
DECLARE_DO_FUN( do_clanwhoshow	);
DECLARE_DO_FUN( do_clone	);
DECLARE_DO_FUN(	do_close	);
DECLARE_DO_FUN( do_colour       );	/* Colour Command By Lope */
DECLARE_DO_FUN(	do_commands	);
DECLARE_DO_FUN( do_combine	);
DECLARE_DO_FUN( do_compact	);
DECLARE_DO_FUN(	do_compare	);
DECLARE_DO_FUN(	do_consider	);
DECLARE_DO_FUN( do_combat	);
DECLARE_DO_FUN(	do_credits	);
DECLARE_DO_FUN( do_dbag         );
DECLARE_DO_FUN( do_disable      );
DECLARE_DO_FUN( do_deaf		);
DECLARE_DO_FUN( do_delet	);
DECLARE_DO_FUN( do_delete	);
DECLARE_DO_FUN(	do_deny		);
DECLARE_DO_FUN(	do_description	);
DECLARE_DO_FUN( do_dirt		);
DECLARE_DO_FUN(	do_disarm	);
DECLARE_DO_FUN(	do_discon	);
DECLARE_DO_FUN(	do_down		);
DECLARE_DO_FUN(	do_drink	);
DECLARE_DO_FUN(	do_drop		);
DECLARE_DO_FUN( do_dump		);
DECLARE_DO_FUN(	do_emboo	);
DECLARE_DO_FUN(	do_emboot	);
DECLARE_DO_FUN(	do_fixage	);
DECLARE_DO_FUN(	do_fvlist	);
DECLARE_DO_FUN(	do_east		);
DECLARE_DO_FUN(	do_eat		);
DECLARE_DO_FUN(	do_emote	);
DECLARE_DO_FUN( do_enter	);
DECLARE_DO_FUN( do_envenom	);
DECLARE_DO_FUN(	do_equipment	);
DECLARE_DO_FUN(	do_exits	);
DECLARE_DO_FUN(	do_fill		);
DECLARE_DO_FUN( do_fine		);
DECLARE_DO_FUN( do_flag		);
DECLARE_DO_FUN(	do_flee		);
DECLARE_DO_FUN( do_flottery );
DECLARE_DO_FUN(	do_follow	);
DECLARE_DO_FUN( do_ftick    );
DECLARE_DO_FUN( do_grab		);
DECLARE_DO_FUN( do_gain		);
DECLARE_DO_FUN(	do_get		);
DECLARE_DO_FUN(	do_give		);
DECLARE_DO_FUN( do_gossip	);
DECLARE_DO_FUN(	do_goto		);
DECLARE_DO_FUN( do_grats        );
DECLARE_DO_FUN(	do_group	);
DECLARE_DO_FUN( do_groups	);
DECLARE_DO_FUN(	do_gtell	);
DECLARE_DO_FUN( do_hedit       );
DECLARE_DO_FUN( do_heal		);
DECLARE_DO_FUN(	do_help		);
DECLARE_DO_FUN( do_hero         );
DECLARE_DO_FUN(	do_hide		);
DECLARE_DO_FUN(	do_holylight	);
DECLARE_DO_FUN( do_identify );
DECLARE_DO_FUN(	do_immtalk	);
DECLARE_DO_FUN( do_immhelp	);
DECLARE_DO_FUN( do_incog	);
DECLARE_DO_FUN( do_ignore	);
DECLARE_DO_FUN( do_imotd	);
DECLARE_DO_FUN( do_ximm		);
DECLARE_DO_FUN( do_join );
DECLARE_DO_FUN( do_junk		);
DECLARE_DO_FUN(	do_inventory	);
DECLARE_DO_FUN( do_doat 	);
DECLARE_DO_FUN(	do_invis	);
DECLARE_DO_FUN(	do_kick		);
DECLARE_DO_FUN(	do_kill		);
DECLARE_DO_FUN(	do_legend		);
DECLARE_DO_FUN(	do_list		);
DECLARE_DO_FUN(	do_lock		);
DECLARE_DO_FUN(	do_log		);
DECLARE_DO_FUN(	do_look		);
DECLARE_DO_FUN( do_lottery  );
DECLARE_DO_FUN(	do_memory	);
DECLARE_DO_FUN(	do_mset		);
DECLARE_DO_FUN( do_mob		);
DECLARE_DO_FUN( do_motd		);
DECLARE_DO_FUN( do_newlock	);
DECLARE_DO_FUN( do_dummy );
DECLARE_DO_FUN( do_notitle	);
DECLARE_DO_FUN(	do_noemote	);
DECLARE_DO_FUN(	do_nopnote	);
DECLARE_DO_FUN( do_nofollow	);
DECLARE_DO_FUN( do_norestore	);
DECLARE_DO_FUN( do_noloot	);
DECLARE_DO_FUN(	do_north	);
DECLARE_DO_FUN( do_northeast 	); 
DECLARE_DO_FUN( do_southeast 	); 
DECLARE_DO_FUN( do_southwest 	); 
DECLARE_DO_FUN( do_northwest 	); 
DECLARE_DO_FUN( do_slot		);
DECLARE_DO_FUN( do_nosummon	);
DECLARE_DO_FUN(	do_notell	);
DECLARE_DO_FUN( do_object);
DECLARE_DO_FUN( do_olevel   );
DECLARE_DO_FUN(	do_open		);
DECLARE_DO_FUN(	do_order	);
DECLARE_DO_FUN(	do_oset		);
DECLARE_DO_FUN( do_otype    );
DECLARE_DO_FUN( do_pcinfo	);
DECLARE_DO_FUN( do_pload        );
DECLARE_DO_FUN( do_punload      );
DECLARE_DO_FUN(	do_password	);
DECLARE_DO_FUN(	do_peace	);
DECLARE_DO_FUN( do_push     );
DECLARE_DO_FUN( do_gecho	);
DECLARE_DO_FUN( do_pecho	);
DECLARE_DO_FUN( do_permban	);
DECLARE_DO_FUN(	do_pick		);
DECLARE_DO_FUN( do_recho	);
DECLARE_DO_FUN( do_rules	);
DECLARE_DO_FUN( do_dip );
DECLARE_DO_FUN( do_pour		);
DECLARE_DO_FUN(	do_practice	);
DECLARE_DO_FUN( do_prefi	);
DECLARE_DO_FUN( do_prefix	);
DECLARE_DO_FUN( do_prompt	);
DECLARE_DO_FUN(	do_purge	);
DECLARE_DO_FUN(	do_put		);
DECLARE_DO_FUN( do_quest	);
DECLARE_DO_FUN(	do_quaff	);
DECLARE_DO_FUN(	do_qui		);
DECLARE_DO_FUN( do_quiet	);
DECLARE_DO_FUN(	do_quit		);
DECLARE_DO_FUN( do_rename  	);
DECLARE_DO_FUN( do_read		);
DECLARE_DO_FUN(	do_reboo	);
DECLARE_DO_FUN(	do_reboot	);
DECLARE_DO_FUN(	do_recall	);
DECLARE_DO_FUN(	do_recho	);
DECLARE_DO_FUN(	do_recite	);
DECLARE_DO_FUN(	do_remove	);
DECLARE_DO_FUN( do_replay	);
DECLARE_DO_FUN(	do_reply	);
DECLARE_DO_FUN(	do_report	);
DECLARE_DO_FUN(	do_rescue	);
DECLARE_DO_FUN(	do_rest		);
DECLARE_DO_FUN(	do_restore	);
DECLARE_DO_FUN(	do_return	);
DECLARE_DO_FUN(	do_rset		);
DECLARE_DO_FUN(	do_rstat	);
DECLARE_DO_FUN(	do_sacrifice	);
DECLARE_DO_FUN(	do_donate	);
DECLARE_DO_FUN(	do_save		);
DECLARE_DO_FUN(	do_say		);
DECLARE_DO_FUN(	do_scan		);
DECLARE_DO_FUN(	do_score	);
DECLARE_DO_FUN( do_scoreswap );
DECLARE_DO_FUN(	do_scoreold	);
DECLARE_DO_FUN( do_scroll	);
DECLARE_DO_FUN(	do_sell		);
DECLARE_DO_FUN( do_second	);
DECLARE_DO_FUN( do_stat     );
DECLARE_DO_FUN( do_show		);
DECLARE_DO_FUN(	do_shutdow	);
DECLARE_DO_FUN(	do_shutdown	);
DECLARE_DO_FUN( do_sit		);
DECLARE_DO_FUN( do_skills	);
DECLARE_DO_FUN( do_skilltable );
DECLARE_DO_FUN(	do_sla		);
DECLARE_DO_FUN(	do_slay		);
DECLARE_DO_FUN(	do_sleep	);
DECLARE_DO_FUN(	do_slookup	);
DECLARE_DO_FUN(	do_sneak	);
DECLARE_DO_FUN( do_socials	);
DECLARE_DO_FUN(	do_south	);
DECLARE_DO_FUN( do_spells	);
DECLARE_DO_FUN( do_spellup  );
DECLARE_DO_FUN(	do_split	);
DECLARE_DO_FUN(	do_sset		);
DECLARE_DO_FUN(	do_stand	);
DECLARE_DO_FUN(	do_sstat	);
DECLARE_DO_FUN( do_spousetalk	);
DECLARE_DO_FUN( do_consent	);
DECLARE_DO_FUN(	do_steal	);
DECLARE_DO_FUN( do_story	);
DECLARE_DO_FUN( do_string	);
DECLARE_DO_FUN(	do_surrender	);
DECLARE_DO_FUN(	do_switch	);
DECLARE_DO_FUN(	do_tell		);
DECLARE_DO_FUN(	do_time		);
DECLARE_DO_FUN(	do_title	);
DECLARE_DO_FUN(	do_train	);
DECLARE_DO_FUN(	do_transfer	);
DECLARE_DO_FUN( do_trip		);
DECLARE_DO_FUN( do_unalias	);
DECLARE_DO_FUN(	do_unlock	);
DECLARE_DO_FUN(	do_up		);
DECLARE_DO_FUN(	do_visible	);
DECLARE_DO_FUN( do_violate	);
DECLARE_DO_FUN( do_vl		);
DECLARE_DO_FUN( do_vnumlist	);
DECLARE_DO_FUN( do_vlist    );
DECLARE_DO_FUN(	do_wake		);
DECLARE_DO_FUN(	do_wear		);
DECLARE_DO_FUN(	do_west		);
DECLARE_DO_FUN(	do_where	);
DECLARE_DO_FUN(	do_who		);
DECLARE_DO_FUN(	do_wimpy	);
DECLARE_DO_FUN(	do_wizhelp	);
DECLARE_DO_FUN(	do_wizlock	);
DECLARE_DO_FUN( do_wizlist	);
DECLARE_DO_FUN( do_wizindex	);
DECLARE_DO_FUN( do_remort	);
DECLARE_DO_FUN( do_remortlist	);
DECLARE_DO_FUN( do_wiznet	);
DECLARE_DO_FUN(	do_yell		);
DECLARE_DO_FUN(	do_zap		);
DECLARE_DO_FUN( do_ntalk            );
DECLARE_DO_FUN(	do_nephilim		);
DECLARE_DO_FUN( do_cedit	);
DECLARE_DO_FUN( do_mlevel);
DECLARE_DO_FUN( do_blightning  );
DECLARE_DO_FUN( do_bfire       );
DECLARE_DO_FUN( do_bacid       );
DECLARE_DO_FUN( do_bfrost      );
DECLARE_DO_FUN( do_bgas        );
DECLARE_DO_FUN( do_aquire      );
DECLARE_DO_FUN( do_pnote      );
DECLARE_DO_FUN( do_owhere     );
DECLARE_DO_FUN( do_mwhere     );
DECLARE_DO_FUN(	do_wpeace	);
DECLARE_DO_FUN( do_fileident	);
DECLARE_DO_FUN( do_dpitkill      );
DECLARE_DO_FUN( do_jail		);
DECLARE_DO_FUN( do_heighten_senses );
DECLARE_DO_FUN( do_chameleon_power );
DECLARE_DO_FUN( do_shadow_form     );
DECLARE_DO_FUN( do_crecall     );
DECLARE_DO_FUN( do_multiburst     );
DECLARE_DO_FUN( do_quicken     );
DECLARE_DO_FUN( do_scribe     );
DECLARE_DO_FUN( do_brew     );
DECLARE_DO_FUN( do_whirlwind     );
DECLARE_DO_FUN( do_map  );
DECLARE_DO_FUN( do_chant    );
DECLARE_DO_FUN( do_track    );
DECLARE_DO_FUN( do_racestat     );
DECLARE_DO_FUN( do_classtat     );
DECLARE_DO_FUN( do_dptalk       );
DECLARE_DO_FUN( do_dragonpit    );
DECLARE_DO_FUN( do_asave	);
DECLARE_DO_FUN( do_alist	);
DECLARE_DO_FUN( do_links	);
DECLARE_DO_FUN( do_resets	);
DECLARE_DO_FUN( do_redit	);
DECLARE_DO_FUN( do_aedit	);
DECLARE_DO_FUN( do_medit	);
DECLARE_DO_FUN( do_oedit	);
DECLARE_DO_FUN( do_mpedit	);
DECLARE_DO_FUN( do_guild 	);
DECLARE_DO_FUN( do_rsearch    	);
DECLARE_DO_FUN( do_osearch    	);
DECLARE_DO_FUN( do_msearch    	);
DECLARE_DO_FUN( do_ostat    	);
DECLARE_DO_FUN( do_mstat   	);
DECLARE_DO_FUN( do_mpstat	);
DECLARE_DO_FUN( do_mpdump	);
DECLARE_DO_FUN( do_marry	);
DECLARE_DO_FUN( do_divorce	);
DECLARE_DO_FUN( do_overview	);
DECLARE_DO_FUN( do_oload    	);
DECLARE_DO_FUN( do_mload   	);
DECLARE_DO_FUN(	do_force	);
DECLARE_DO_FUN(	do_freeze	);
DECLARE_DO_FUN( do_nochan	);
DECLARE_DO_FUN(	do_pardon	);
DECLARE_DO_FUN(	do_snoop	);
DECLARE_DO_FUN( do_scatter  	);
DECLARE_DO_FUN( do_shatter	);
DECLARE_DO_FUN( do_sockets	);
DECLARE_DO_FUN( do_icon		);
DECLARE_DO_FUN( do_startdp	);
DECLARE_DO_FUN( do_builder      );
DECLARE_DO_FUN( do_plzap	);
DECLARE_DO_FUN( do_godspeak	);
DECLARE_DO_FUN( do_elven	);
DECLARE_DO_FUN( do_demonic	);
DECLARE_DO_FUN( do_dwarven	);
DECLARE_DO_FUN( do_draconian	);
DECLARE_DO_FUN( do_reptilian	);
DECLARE_DO_FUN( do_kender	);
DECLARE_DO_FUN( do_drow		);
DECLARE_DO_FUN( do_magespeak	);
DECLARE_DO_FUN( do_high_elven	);
DECLARE_DO_FUN( do_undead	);
DECLARE_DO_FUN( do_thieves_cant	);
DECLARE_DO_FUN( do_handtalk	);
DECLARE_DO_FUN( do_forestsign	);
DECLARE_DO_FUN( do_italian	);
DECLARE_DO_FUN( do_war_chant	);

