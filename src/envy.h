struct aff_type {
	long	envy;
	long	rom;
};

const	struct	aff_type	aff_change	[]	=
{
	{	1,		AFF_BLIND		},
	{	2,		AFF_INVISIBLE		},
	{	4,		AFF_DETECT_EVIL		},
	{	8,		AFF_DETECT_INVIS	},
	{	16,		AFF_DETECT_MAGIC	},
	{	32,		AFF_DETECT_HIDDEN	},
	{	64,		0			},
	{	128,		AFF_SANCTUARY		},
	{	256,		AFF_FAERIE_FIRE		},
	{	512,		AFF_INFRARED		},
	{	1024,		AFF_CURSE		},
	{	2048,		0			},
	{	4096,		AFF_POISON		},
	{	8192,		0			},
	{	16384,		0			},
	{	32768,		AFF_SNEAK		},
	{	65536,		AFF_HIDE		},
	{	131072,		AFF_SLEEP		},
	{	262144,		AFF_CHARM		},
	{	524288,		AFF_FLYING		},
	{	1048576,	AFF_PASS_DOOR		},
	{	0,		0			}
};


