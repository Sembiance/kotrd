struct flag_type
{
    char *name;
    int bit;
    bool settable;
};

struct command_type
{
    char *      com;
};

struct position_type
{
    char *name;
    char *short_name;
};

struct sex_type
{
    char *name;
};

struct size_type
{
    char *name;
};

struct  bit_type
{
    const   struct  flag_type * table;
    char *              help;
};

struct quest_type
{
 char *  obj_name;
 int     vnum;
 int     qp_cost;
 int     level;
};

/* game tables */
extern	const	struct	position_type	position_table[];
extern	const	struct	sex_type	sex_table[];
extern	const	struct	size_type	size_table[];
extern  const   struct  quest_type      quest_table[];
extern	const	struct	command_type	builder_table[];
extern  const   struct  command_type    police_table[];
extern  const   struct  command_type    advanced_table[];
extern  const   struct  command_type    basic_table[];
extern  const   struct  command_type    medium_table[];
extern  const   struct  command_type    extra_table[];

/* flag tables */
extern	const	struct	flag_type	act_flags[];
extern	const	struct	flag_type	plr_flags[];
extern	const	struct	flag_type	affect_flags[];
extern	const	struct	flag_type	affect2_flags[];
extern	const	struct	flag_type	off_flags[];
extern	const	struct	flag_type	form_flags[];
extern	const	struct	flag_type	part_flags[];
extern	const	struct	flag_type	comm_flags[];
extern	const	struct	flag_type	extra_flags[];
extern	const	struct	flag_type	wear_flags[];
extern	const	struct	flag_type	weapon_flags[];
extern	const	struct	flag_type	container_flags[];
extern	const	struct	flag_type	portal_flags[];
extern	const	struct	flag_type	room_flags[];
extern	const	struct	flag_type	exit_flags[];
extern 	const	struct  flag_type	mprog_flags[];
extern	const	struct	flag_type	area_flags[];
extern	const	struct	flag_type	sector_flags[];
extern	const	struct	flag_type	door_resets[];
extern	const	struct	flag_type	wear_loc_strings[];
extern	const	struct	flag_type	wear_loc_flags[];
extern	const	struct	flag_type	imm_flags[];
extern	const	struct	flag_type	res_flags[];
extern	const	struct	flag_type	vuln_flags[];
extern	const	struct	flag_type	material_flags[];
extern	const	struct	flag_type	type_flags[];
extern	const	struct	flag_type	apply_flags[];
extern	const	struct	flag_type	sex_flags[];
extern	const	struct	flag_type	furniture_flags[];
extern	const	struct	flag_type	weapon_class[];
extern	const	struct	flag_type	apply_types[];
extern	const	struct	flag_type	weapon_type2[];
extern	const	struct	flag_type	apply_types[];
extern	const	struct	flag_type	size_flags[];
extern	const	struct	flag_type	position_flags[];
extern	const	struct	flag_type	ac_type[];
extern	const	struct	flag_type	racechan_flags[];
extern	const	struct	bit_type	bitvector_type[];
