#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>

#include "Utility.h"
#include "StringUtility.h"

#include "merc.h"
#include "interp.h"
#include "recycle.h"
#include "tables.h"

DECLARE_DO_FUN ( do_dbag );
DECLARE_DO_FUN ( do_outfit );
DECLARE_DO_FUN ( do_racestat );
DECLARE_DO_FUN ( do_classtat );
DECLARE_DO_FUN ( do_gecho );

void init_signals     args((void));
void do_auto_shutdown  args( (void) );

extern bool is_ignoring(CHAR_DATA *ch, CHAR_DATA *victim);
void make_stats(CHAR_DATA *ch);

// LEGEND SYSTEM
void legend_weapon_picked(CHAR_DATA * ch, int weaponChosen);
void legend_logging_in_first_time(CHAR_DATA * ch);
bool legend_has_race_chat(CHAR_DATA * legend, CHAR_DATA * target);
void legend_garble_text(char * original, char * destination);


/*
 * Malloc debugging stuff.
 */
#if defined(sun)
#undef MALLOC_DEBUG
#endif

#if defined(MALLOC_DEBUG)
#include <malloc.h>
extern  int malloc_debug    args( ( int  ) );
extern  int malloc_verify   args( ( void ) );
#endif

#if defined(unix)
#include <signal.h>
#endif

#if defined(apollo)
#undef __attribute
#endif



/*
 * Socket and TCP/IP stuff.
 */
#if defined(macintosh) || defined(MSDOS)
const   char    echo_off_str    [] = { '\0' };
const   char    echo_on_str [] = { '\0' };
const   char    go_ahead_str    [] = { '\0' };
#endif

#if defined(unix)
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "telnet.h"
const   char    echo_off_str    [] = { IAC, WILL, TELOPT_ECHO, '\0' };
const   char    echo_on_str [] = { IAC, WONT, TELOPT_ECHO, '\0' };
const   char    go_ahead_str    [] = { IAC, GA, '\0' };
#endif



/*
 * OS-dependent declarations.
 */
#if defined(_AIX)
#include <sys/select.h>
int accept      args( ( int s, struct sockaddr *addr, int *addrlen ) );
int bind        args( ( int s, struct sockaddr *name, int namelen ) );
void    bzero       args( ( char *b, int length ) );
int getpeername args( ( int s, struct sockaddr *name, int *namelen ) );
int getsockname args( ( int s, struct sockaddr *name, int *namelen ) );
int gettimeofday    args( ( struct timeval *tp, struct timezone *tzp ) );
int listen      args( ( int s, int backlog ) );
int setsockopt  args( ( int s, int level, int optname, void *optval,
                int optlen ) );
int socket      args( ( int domain, int type, int protocol ) );
#endif

#if defined(apollo)
#include <unistd.h>
void    bzero       args( ( char *b, int length ) );
#endif

#if defined(__hpux)
int accept      args( ( int s, void *addr, int *addrlen ) );
int bind        args( ( int s, const void *addr, int addrlen ) );
void    bzero       args( ( char *b, int length ) );
int getpeername args( ( int s, void *addr, int *addrlen ) );
int getsockname args( ( int s, void *name, int *addrlen ) );
int gettimeofday    args( ( struct timeval *tp, struct timezone *tzp ) );
int listen      args( ( int s, int backlog ) );
int setsockopt  args( ( int s, int level, int optname,
                const void *optval, int optlen ) );
int socket      args( ( int domain, int type, int protocol ) );
#endif

#if defined(interactive)
#include <net/errno.h>
#include <sys/fnctl.h>
#endif

#if defined(linux)
/* 
    Linux shouldn't need these. If you have a problem compiling, try
    uncommenting these functions.
*/
/*
int accept      args( ( int s, struct sockaddr *addr, int *addrlen ) );
int bind        args( ( int s, struct sockaddr *name, int namelen ) );
int getpeername args( ( int s, struct sockaddr *name, int *namelen ) );
int getsockname args( ( int s, struct sockaddr *name, int *namelen ) );
int listen      args( ( int s, int backlog ) );
*/

int close       args( ( int fd ) );
//int gettimeofday    args( ( struct timeval *tp, struct timezone *tzp ) );
int read        args( ( int fd, char *buf, int nbyte ) );
int select      args( ( int width, fd_set *readfds, fd_set *writefds,
                fd_set *exceptfds, struct timeval *timeout ) );
int socket      args( ( int domain, int type, int protocol ) );
int write       args( ( int fd, char *buf, int nbyte ) );
#endif

#if defined(macintosh)
#include <conrole.h>
#include <fcntl.h>
#include <unix.h>
struct  timeval
{
    time_t  tv_sec;
    time_t  tv_usec;
};
#if !defined(isascii)
#define isascii(c)      ( (c) < 0200 )
#endif
static  long            theKeys [4];

int gettimeofday        args( ( struct timeval *tp, void *tzp ) );
#endif

#if defined(MIPS_OS)
extern  int     errno;
#endif

#if defined(MSDOS)
int gettimeofday    args( ( struct timeval *tp, void *tzp ) );
int kbhit       args( ( void ) );
#endif

#if defined(NeXT)
int close       args( ( int fd ) );
int fcntl       args( ( int fd, int cmd, int arg ) );
#if !defined(htons)
u_short htons       args( ( u_short hostshort ) );
#endif
#if !defined(ntohl)
u_long  ntohl       args( ( u_long hostlong ) );
#endif
int read        args( ( int fd, char *buf, int nbyte ) );
int select      args( ( int width, fd_set *readfds, fd_set *writefds,
                fd_set *exceptfds, struct timeval *timeout ) );
int write       args( ( int fd, char *buf, int nbyte ) );
#endif

#if defined(sequent)
int accept      args( ( int s, struct sockaddr *addr, int *addrlen ) );
int bind        args( ( int s, struct sockaddr *name, int namelen ) );
int close       args( ( int fd ) );
int fcntl       args( ( int fd, int cmd, int arg ) );
int getpeername args( ( int s, struct sockaddr *name, int *namelen ) );
int getsockname args( ( int s, struct sockaddr *name, int *namelen ) );
int gettimeofday    args( ( struct timeval *tp, struct timezone *tzp ) );
#if !defined(htons)
u_short htons       args( ( u_short hostshort ) );
#endif
int listen      args( ( int s, int backlog ) );
#if !defined(ntohl)
u_long  ntohl       args( ( u_long hostlong ) );
#endif
int read        args( ( int fd, char *buf, int nbyte ) );
int select      args( ( int width, fd_set *readfds, fd_set *writefds,
                fd_set *exceptfds, struct timeval *timeout ) );
int setsockopt  args( ( int s, int level, int optname, caddr_t optval,
                int optlen ) );
int socket      args( ( int domain, int type, int protocol ) );
int write       args( ( int fd, char *buf, int nbyte ) );
#endif

/* This includes Solaris Sys V as well */
#if defined(sun)
int accept      args( ( int s, struct sockaddr *addr, int *addrlen ) );
int bind        args( ( int s, struct sockaddr *name, int namelen ) );
void    bzero       args( ( char *b, int length ) );
int close       args( ( int fd ) (;
int getpeername args( ( int s, struct sockaddr *name, int *namelen ) );
int getsockname args( ( int s, struct sockaddr *name, int *namelen ) );
int listen      args( ( int s, int backlog ) );
int read        args( ( int fd, char *buf, int nbyte ) );
int select      args( ( int width, fd_set *readfds, fd_set *writefds,
                fd_set *exceptfds, struct timeval *timeout ) );

#if !defined(__SVR4)
int gettimeofday    args( ( struct timeval *tp, struct timezone *tzp ) );

#if defined(SYSV)
int setsockopt      args( ( int s, int level, int optname,
                const char *optval, int optlen ) );
#else
int setsockopt  args( ( int s, int level, int optname, void *optval,
                int optlen ) );
#endif
#endif
int socket      args( ( int domain, int type, int protocol ) );
int write       args( ( int fd, char *buf, int nbyte ) );
#endif

#if defined(ultrix)
int accept      args( ( int s, struct sockaddr *addr, int *addrlen ) );
int bind        args( ( int s, struct sockaddr *name, int namelen ) );
void    bzero       args( ( char *b, int length ) );
int close       args( ( int fd ) );
int getpeername args( ( int s, struct sockaddr *name, int *namelen ) );
int getsockname args( ( int s, struct sockaddr *name, int *namelen ) );
int gettimeofday    args( ( struct timeval *tp, struct timezone *tzp ) );
int listen      args( ( int s, int backlog ) );
int read        args( ( int fd, char *buf, int nbyte ) );
int select      args( ( int width, fd_set *readfds, fd_set *writefds,
                fd_set *exceptfds, struct timeval *timeout ) );
int setsockopt  args( ( int s, int level, int optname, void *optval,
                int optlen ) );
int socket      args( ( int domain, int type, int protocol ) );
int write       args( ( int fd, char *buf, int nbyte ) );
#endif



/*
 * Global variables.
 */
DESCRIPTOR_DATA *   descriptor_list;    /* All open descriptors     */
DESCRIPTOR_DATA *   d_next;     /* Next descriptor in loop  */
FILE *          fpReserve;      /* Reserved file handle     */
bool            god;        /* All new chars are gods!  */
bool            merc_down;      /* Shutdown         */
bool            wizlock;        /* Game is wizlocked        */
bool            newlock;        /* Game is newlocked        */
char            str_boot_time[MAX_INPUT_LENGTH];
time_t          current_time;   /* time of this pulse */    
bool            MOBtrigger = TRUE;  /* act() switch                 */
int             port,control;       /* Copyover stuff */


/*
 * OS-dependent local functions.
 */

#if defined(unix)
void    game_loop_unix      args( ( int control ) );
int     init_socket     args( ( int port ) );
void    init_descriptor     args( ( int control ) );
bool    read_from_descriptor    args( ( DESCRIPTOR_DATA *d ) );
#endif




/*
 * Other local functions (OS-independent).
 */
bool    check_parse_name    args( ( char *name ) );
bool    check_reconnect     args( ( DESCRIPTOR_DATA *d, char *name,
                    bool fConn ) );
bool    check_playing       args( ( DESCRIPTOR_DATA *d, char *name ) );
int main	args( (int argc, char **argv ) );
void    nanny           args( ( DESCRIPTOR_DATA *d, char *argument ) );
bool    process_output      args( ( DESCRIPTOR_DATA *d, bool fPrompt, char * where ) );
void    read_from_buffer    args( ( DESCRIPTOR_DATA *d ) );
void    stop_idling     args( ( CHAR_DATA *ch ) );
void    bust_a_prompt           args( ( CHAR_DATA *ch ) );
bool	legal_class		args( (int race, char * class_name) );
bool	legal_align		args( (char * align, char * type) );
bool	legal_sex		args( (char * sex, char * type) );

int main( int argc, char **argv )
{
    struct timeval now_time;
    bool fCopyOver = FALSE;


    /*
     * Memory debugging if needed.
     */
#if defined(MALLOC_DEBUG)
    malloc_debug( 2 );
#endif

    /*
     * Init time.
     */
    gettimeofday( &now_time, NULL );
    current_time    = (time_t) now_time.tv_sec;
    strcpy( str_boot_time, ctime( &current_time ) );

    /*
     * Macintosh console initialization.
     */
#if defined(macintosh)
    console_options.nrows = 31;
    cshow( stdout );
    csetmode( C_RAW, stdin );
    cecho2file( "log file", 1, stderr );
#endif

    /*
     * Reserve one channel for our use.
     */
    if ( ( fpReserve = fopen( NULL_FILE, "r" ) ) == NULL )
    {
    perror( NULL_FILE );
    exit( 1 );
    }

    /*
     * Get the port number.
     */
    port = 4500;
    if ( argc > 1 )
    {
    if ( !is_number( argv[1] ) )
    {
        fprintf( stderr, "Usage: %s [port #]\n", argv[0] );
        exit( 1 );
    }
    else if ( ( port = atoi( argv[1] ) ) <= 1024 )
    {
        fprintf( stderr, "Port number must be above 1024.\n" );
        exit( 1 );
    }

	/* Are we recovering from a copyover? */
 	if (argv[2] && argv[2][0])
 	{
 		fCopyOver = TRUE;
 		control = atoi(argv[3]);
 	}
 	else
 		fCopyOver = FALSE;
    }

    /*
     * Run the game.
     */

#if defined(unix)
	if (!fCopyOver)
	    control = init_socket( port );
init_signals(); /* For the use of the signal handler. -Ferric */
    boot_db( );
    sprintf( log_buf, "ROM is ready to rock on port %d.", port );
    log_string( log_buf );
    if (fCopyOver)
    	copyover_recover();
    game_loop_unix( control );
    close (control);
#endif

    /*
     * That's all, folks.
     */
    log_string( "Normal termination of game." );
    exit( 0 );
    return 0;
}

    



#if defined(unix)
int init_socket( int port )
{
    static struct sockaddr_in sa_zero;
    struct sockaddr_in sa;
    int x = 1;
    int fd;

    if ( ( fd = socket( AF_INET, SOCK_STREAM, 0 ) ) < 0 )
    {
    perror( "Init_socket: socket" );
    exit( 1 );
    }

    if ( setsockopt( fd, SOL_SOCKET, SO_REUSEADDR,
    (char *) &x, sizeof(x) ) < 0 )
    {
    perror( "Init_socket: SO_REUSEADDR" );
    close(fd);
    exit( 1 );
    }

#if defined(SO_DONTLINGER) && !defined(SYSV)
    {
    struct  linger  ld;

    ld.l_onoff  = 1;
    ld.l_linger = 1000;

    if ( setsockopt( fd, SOL_SOCKET, SO_DONTLINGER,
    (char *) &ld, sizeof(ld) ) < 0 )    {
        perror( "Init_socket: SO_DONTLINGER" );
        close(fd);
        exit( 1 );
    }
    }
#endif

    sa          = sa_zero;
    sa.sin_family   = AF_INET;
    sa.sin_port     = htons( port );

    if ( bind( fd, (struct sockaddr *) &sa, sizeof(sa) ) < 0 )
    {
    perror("Init socket: bind" );
    close(fd);
    exit(1);
    }


    if ( listen( fd, 3 ) < 0 )
    {
    perror("Init socket: listen");
    close(fd);
    exit(1);
    }

    return fd;
}
#endif


#if defined(unix)
void game_loop_unix( int control )
{
    static struct timeval null_time;
    struct timeval last_time;

    signal( SIGPIPE, SIG_IGN );
    gettimeofday( &last_time, NULL );
    current_time = (time_t) last_time.tv_sec;

    /* Main loop */
    while ( !merc_down )
    {
    fd_set in_set;
    fd_set out_set;
    fd_set exc_set;
    DESCRIPTOR_DATA *d;
    int maxdesc;

#if defined(MALLOC_DEBUG)
    if ( malloc_verify( ) != 1 )
        abort( );
#endif

    /*
     * Poll all active descriptors.
     */
    FD_ZERO( &in_set  );
    FD_ZERO( &out_set );
    FD_ZERO( &exc_set );
    FD_SET( control, &in_set );
    maxdesc = control;
    for ( d = descriptor_list; d; d = d->next )
    {
        maxdesc = UMAX( maxdesc, d->descriptor );
        FD_SET( d->descriptor, &in_set  );
        FD_SET( d->descriptor, &out_set );
        FD_SET( d->descriptor, &exc_set );
    }

    if ( select( maxdesc+1, &in_set, &out_set, &exc_set, &null_time ) < 0 )
    {
        perror( "Game_loop: select: poll" );
        exit( 1 );
    }

    /*
     * New connection?
     */
    if ( FD_ISSET( control, &in_set ) )
        init_descriptor( control );

    /*
     * Kick out the freaky folks.
     */
    for ( d = descriptor_list; d != NULL; d = d_next )
    {
        d_next = d->next;   
        if ( FD_ISSET( d->descriptor, &exc_set ) )
        {
        FD_CLR( d->descriptor, &in_set  );
        FD_CLR( d->descriptor, &out_set );
        if ( d->character && d->connected == CON_PLAYING)
            save_char_obj( d->character );
        d->outtop   = 0;
        close_socket( d );
        }
    }

    /*
     * Process input.
     */
    for ( d = descriptor_list; d != NULL; d = d_next )
    {
        d_next  = d->next;
        d->fcommand = FALSE;

        if ( FD_ISSET( d->descriptor, &in_set ) )
        {
        if ( d->character != NULL )
            d->character->timer = 0;
        if ( !read_from_descriptor( d ) )
        {
            FD_CLR( d->descriptor, &out_set );
            if ( d->character != NULL && d->connected == CON_PLAYING)
            save_char_obj( d->character );
            d->outtop   = 0;
            close_socket( d );
            continue;
        }
        }


        if (d->character != NULL && d->character->daze > 0)
        --d->character->daze;

	if ( d->character != NULL && d->character->race_wait > 0 )
        --d->character->race_wait;

        if ( d->character != NULL && d->character->wait > 0 )
        {
        --d->character->wait;
        continue;
        }

        read_from_buffer( d );
        if ( d->incomm[0] != '\0' )
        {
        d->fcommand = TRUE;
        stop_idling( d->character );

    /* OLC */
    if ( d->showstr_point )
        show_string( d, d->incomm );
    else
    if ( d->pString )
        string_add( d->character, d->incomm );
    else
        switch ( d->connected )
        {
            case CON_PLAYING:
            if ( !run_olc_editor( d ) )
                    substitute_alias( d, d->incomm );
            break;
            default:
            nanny( d, d->incomm );
            break;
        }

        d->incomm[0]    = '\0';
        }
    }



    /*
     * Autonomous game motion.
     */
    update_handler( FALSE );



    /*
     * Output.
     */
    for ( d = descriptor_list; d != NULL; d = d_next )
    {
        d_next = d->next;

        if ( ( d->fcommand || d->outtop > 0 )
        &&   FD_ISSET(d->descriptor, &out_set) )
        {
        if ( !process_output( d, TRUE, "game_loop_unix (Output.)" ) )
        {
            if ( d->character != NULL && d->connected == CON_PLAYING)
            save_char_obj( d->character );
            d->outtop   = 0;
            close_socket( d );
        }
        }
    }



    /*
     * Synchronize to a clock.
     * Sleep( last_time + 1/PULSE_PER_SECOND - now ).
     * Careful here of signed versus unsigned arithmetic.
     */
    {
        struct timeval now_time;
        long secDelta;
        long usecDelta;

        gettimeofday( &now_time, NULL );
        usecDelta   = ((int) last_time.tv_usec) - ((int) now_time.tv_usec)
            + 1000000 / PULSE_PER_SECOND;
        secDelta    = ((int) last_time.tv_sec ) - ((int) now_time.tv_sec );
        while ( usecDelta < 0 )
        {
        usecDelta += 1000000;
        secDelta  -= 1;
        }

        while ( usecDelta >= 1000000 )
        {
        usecDelta -= 1000000;
        secDelta  += 1;
        }

        if ( secDelta > 0 || ( secDelta == 0 && usecDelta > 0 ) )
        {
        struct timeval stall_time;

        stall_time.tv_usec = usecDelta;
        stall_time.tv_sec  = secDelta;
        if ( select( 0, NULL, NULL, NULL, &stall_time ) < 0 )
        {
            perror( "Game_loop: select: stall" );
            exit( 1 );
        }
        }
    }

    gettimeofday( &last_time, NULL );
    current_time = (time_t) last_time.tv_sec;
    }

    return;
}
#endif



#if defined(unix)
void init_descriptor( int control )
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *dnew;
    struct sockaddr_in sock;
    struct hostent *from;
    int desc;
    int size;

    size = sizeof(sock);
    getsockname( control, (struct sockaddr *) &sock, &size );
    if ( ( desc = accept( control, (struct sockaddr *) &sock, &size) ) < 0 )
    {
    perror( "New_descriptor: accept" );
    return;
    }

#if !defined(FNDELAY)
#define FNDELAY O_NDELAY
#endif

    if ( fcntl( desc, F_SETFL, FNDELAY ) == -1 )
    {
    perror( "New_descriptor: fcntl: FNDELAY" );
    return;
    }

    /*
     * Cons a new descriptor.
     */
    dnew = new_descriptor();
    dnew->descriptor    = desc;


    size = sizeof(sock);
    if ( getpeername( desc, (struct sockaddr *) &sock, &size ) < 0 )
    {
    perror( "New_descriptor: getpeername" );
    dnew->host = str_dup( "(unknown)" );
    }
    else
    {
    /*
     * Would be nice to use inet_ntoa here but it takes a struct arg,
     * which ain't very compatible between gcc and system libraries.
     */
    int addr;

    addr = ntohl( sock.sin_addr.s_addr );
    sprintf( buf, "%d.%d.%d.%d",
        ( addr >> 24 ) & 0xFF, ( addr >> 16 ) & 0xFF,
        ( addr >>  8 ) & 0xFF, ( addr       ) & 0xFF
        );

    sprintf( log_buf, "Sock.sinaddr:  %s", buf );
    log_string( log_buf );
    from = gethostbyaddr( (char *) &sock.sin_addr,
        sizeof(sock.sin_addr), AF_INET );
    dnew->host = str_dup( from ? from->h_name : buf );
}
    
    /*
     * Swiftest: I added the following to ban sites.  I don't
     * endorse banning of sites, but Copper has few descriptors now
     * and some people from certain sites keep abusing access by
     * using automated 'autodialers' and leaving connections hanging.
     *
     * Furey: added suffix check by request of Nickel of HiddenWorlds.
     */
    if ( check_ban(dnew->host,BAN_ALL))
    {
    write_to_descriptor( dnew,
        "Your site has been banned from this mud.\n\r", 0 );
    close( desc );
    free_descriptor(dnew);
    return;
    }
    /*
     * Init descriptor data.
     */
    dnew->next          = descriptor_list;
    descriptor_list     = dnew;

write_to_buffer( dnew, "Do you want ANSI? (Y/N): ", 0 );

    return;
}
#endif



void close_socket( DESCRIPTOR_DATA *dclose )
{
    CHAR_DATA *ch;

    if ( dclose->outtop > 0 )
    process_output( dclose, FALSE, "close_socket" );

    if ( dclose->snoop_by != NULL )
    {
    write_to_buffer( dclose->snoop_by,
        "Your victim has left the game.\n\r", 0 );
    }

    {
    DESCRIPTOR_DATA *d;

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
        if ( d->snoop_by == dclose )
        d->snoop_by = NULL;
    }
    }

    if ( ( ch = dclose->character ) != NULL )
    {
    sprintf( log_buf, "Closing link to %s.", ch->name );
    log_string( log_buf );

                if ( ch->pet && ch->pet->in_room == NULL )
                {
                        char_to_room( ch->pet, get_room_index(ROOM_VNUM_LIMBO) );
                        extract_char( ch->pet, TRUE );
                }
                
	/* If ch is writing note or playing, just lose link otherwise clear char */
	if ((dclose->connected == CON_PLAYING && !merc_down)
	    ||((dclose->connected >= CON_NOTE_TO)
	    && (dclose->connected <= CON_NOTE_FINISH)))
    {
        act( "$n has lost $s link.", ch, NULL, NULL, TO_ROOM );
      wiznet("Net death has claimed $N.",ch,NULL,WIZ_LINKS,0,ch->level); 
        ch->desc = NULL;

    }
    else
    {
        free_char(dclose->original ? dclose->original : 
        dclose->character );
    }
    }

    if ( d_next == dclose )
    d_next = d_next->next;   

    if ( dclose == descriptor_list )
    {
    descriptor_list = descriptor_list->next;
    }
    else
    {
    DESCRIPTOR_DATA *d;

    for ( d = descriptor_list; d && d->next != dclose; d = d->next )
        ;
    if ( d != NULL )
        d->next = dclose->next;
    else
        bug( "Close_socket: dclose not found.", 0 );
    }

    close( dclose->descriptor );
    free_descriptor(dclose);
#if defined(MSDOS) || defined(macintosh)
    exit(1);
#endif
    return;
}





bool read_from_descriptor( DESCRIPTOR_DATA *d )
{
    int iStart;

    /* Hold horses if pending command already. */
    if ( d->incomm[0] != '\0' )
    return TRUE;

    /* Check for overflow. */
    iStart = strlen(d->inbuf);
    if ( iStart >= sizeof(d->inbuf) - 10 )
    {
    sprintf( log_buf, "%s input overflow!", d->host );
    log_string( log_buf );
    write_to_descriptor( d,
        "\n\r*** PUT A LID ON IT!!! ***\n\r", 0 );
    return FALSE;
    }

    /* Snarf input. */
#if defined(macintosh)
    for ( ; ; )
    {
    int c;
    c = getc( stdin );
    if ( c == '\0' || c == EOF )
        break;
    putc( c, stdout );
    if ( c == '\r' )
        putc( '\n', stdout );
    d->inbuf[iStart++] = c;
    if ( iStart > sizeof(d->inbuf) - 10 )
        break;
    }
#endif

#if defined(MSDOS) || defined(unix)
    for ( ; ; )
    {
    int nRead;

    nRead = read( d->descriptor, d->inbuf + iStart,
        sizeof(d->inbuf) - 10 - iStart );
    if ( nRead > 0 )
    {
        iStart += nRead;
        if ( d->inbuf[iStart-1] == '\n' || d->inbuf[iStart-1] == '\r' )
        break;
    }
    else if ( nRead == 0 )
    {
        log_string( "EOF encountered on read." );
        return FALSE;
    }
    else if ( errno == EWOULDBLOCK )
        break;
    else
    {
        perror( "Read_from_descriptor" );
        return FALSE;
    }
    }
#endif

    d->inbuf[iStart] = '\0';
    return TRUE;
}


void sig_handler(int sig)
{
switch(sig)
  {
  case SIGBUS:
    bug("Sig handler SIGBUS.",0);
    do_auto_shutdown();
    break;
  case SIGTERM:
    bug("Sig handler SIGTERM.",0);
    do_auto_shutdown();
    break;
  case SIGABRT:
    bug("Sig handler SIGABRT",0);  
    do_auto_shutdown();
    break;
  case SIGSEGV:
    bug("Sig handler SIGSEGV",0);
    do_auto_shutdown();
    break;
  }
}


void init_signals()
{
  //signal(SIGBUS,sig_handler);
  //signal(SIGTERM,sig_handler);
  //signal(SIGABRT,sig_handler);
  //signal(SIGSEGV,sig_handler);
}



/*
 * Transfer one line from input buffer to input line.
 */
void read_from_buffer( DESCRIPTOR_DATA *d )
{
    int i, j, k;

    /*
     * Hold horses if pending command already.
     */
    if ( d->incomm[0] != '\0' )
    return;

    /*
     * Look for at least one new line.
     */
    for ( i = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r'; i++ )
    {
    if ( d->inbuf[i] == '\0' )
        return;
    }

    /*
     * Canonical input processing.
     */
    for ( i = 0, k = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r'; i++ )
    {
    if ( k >= MAX_INPUT_LENGTH - 2 )
    {
        write_to_descriptor( d, "Line too long.\n\r", 0 );

        /* skip the rest of the line */
        for ( ; d->inbuf[i] != '\0'; i++ )
        {
        if ( d->inbuf[i] == '\n' || d->inbuf[i] == '\r' )
            break;
        }
        d->inbuf[i]   = '\n';
        d->inbuf[i+1] = '\0';
        break;
    }

    if ( d->inbuf[i] == '\b' && k > 0 )
        --k;
    else if ( isascii(d->inbuf[i]) && isprint(d->inbuf[i]) )
        d->incomm[k++] = d->inbuf[i];
    }

    /*
     * Finish off the line.
     */
    if ( k == 0 )
    d->incomm[k++] = ' ';
    d->incomm[k] = '\0';

    /*
     * Deal with bozos with #repeat 1000 ...
     */

    if ( k > 1 || d->incomm[0] == '!' )
    {
        if ( d->incomm[0] != '!' && strcmp( d->incomm, d->inlast ) )
    {
        d->repeat = 0;
    }
    else
    {
        if (++d->repeat >= 25 && d->character
        &&  d->connected == CON_PLAYING)
        {
        sprintf( log_buf, "%s input spamming!", d->host );
        log_string( log_buf );
         d->repeat = 0;
        }
    }
    }


    /*
     * Do '!' substitution.
     */
    if ( d->incomm[0] == '!' )
    strcpy( d->incomm, d->inlast );
    else
    strcpy( d->inlast, d->incomm );

    /*
     * Shift the input buffer.
     */
    while ( d->inbuf[i] == '\n' || d->inbuf[i] == '\r' )
    i++;
    for ( j = 0; ( d->inbuf[j] = d->inbuf[i+j] ) != '\0'; j++ )
    ;
    return;
}



/*
 * Low level output function.
 */
bool process_output( DESCRIPTOR_DATA *d, bool fPrompt, char * where )
{
/*
    char temp_str[150];
    int found = 0;
    char temp_buf[70000];
    FILE *fp;
    int i;
*/
    extern bool merc_down;

    /*
     * Bust a prompt.
     */
    if ( !merc_down )
	{
		if ( d->showstr_point )
			write_to_buffer( d, "[Hit Return to continue]\n\r", 0 );
		else if ( fPrompt && d->pString && d->connected == CON_PLAYING )
			write_to_buffer( d, "> ", 2 );
		else if ( fPrompt && d->connected == CON_PLAYING )
		{
			CHAR_DATA *ch;
		CHAR_DATA *victim;

		ch = d->character;

			/* battle prompt */
			if ((victim = ch->fighting) != NULL && can_see(ch,victim))
			{
				int percent;
				char wound[100];
			char buf[MAX_STRING_LENGTH];
	 
				if (victim->max_hit > 0)
					percent = victim->hit * 100 / victim->max_hit;
				else
					percent = -1;
	 
				if (percent >= 100)
					sprintf(wound,"is in excellent condition.");
				else if (percent >= 90)
					sprintf(wound,"has a few scratches.");
				else if (percent >= 75)
					sprintf(wound,"has some small wounds and bruises.");
				else if (percent >= 50)
					sprintf(wound,"has quite a few wounds.");
				else if (percent >= 30)
					sprintf(wound,"has some big nasty wounds and scratches.");
				else if (percent >= 15)
					sprintf(wound,"looks pretty hurt.");
				else if (percent >= 0)
					sprintf(wound,"is in awful condition.");
				else
					sprintf(wound,"is bleeding to death.");
	 
				sprintf(buf,"%s %s \n\r", 
					IS_NPC(victim) ? victim->short_descr : victim->name,wound);
			buf[0]  = UPPER( buf[0] );
				write_to_buffer( d, buf, 0);
			}


		ch = d->original ? d->original : d->character;
		if (!IS_SET(ch->comm, COMM_COMPACT) )
			write_to_buffer( d, "\n\r", 2 );


			if ( IS_SET(ch->comm, COMM_PROMPT) )
				bust_a_prompt( d->character );

		if (IS_SET(ch->comm,COMM_TELNET_GA))
			write_to_buffer(d,go_ahead_str,0);
		}
	}

    /*
     * Short-circuit if nothing to write.
     */
    if ( d->outtop == 0 )
    return TRUE;

    /*
     * Auto-Snoop-o-rama.
     */
/*
    if (d->character != NULL)
      {
        found = 0;
        for (i=0; i < MAX_NUM_AUTOSNOOP; i++)
          {
           if (!strcmp(as_string[i],d->character->name))
             {
              found = i;
             }
          }
   
        if (found)
          {
            sprintf(temp_str,"%s/%s.log",ASNOOP_DIR,as_string[found]);
            if ((fp = fopen(temp_str,"a")) != NULL)
              {
*
                fprintf(fp, "[ * INCOMING * ]");
                sprintf(temp_buf,strip_color("\n%s\n"),d->outbuf);
                fwrite(temp_buf, strlen_wo_col(temp_buf), 1, fp);
                fprintf(fp,"\n\r\n\r\n\r");
                fclose(fp);
*
                fprintf(fp, "[ * INCOMING * ]");
                sprintf(temp_buf,"\n%s\n",d->outbuf);
                fwrite(temp_buf, strlen_wo_col(temp_buf), 1, fp);
                fprintf(fp,"\n\r\n\r\n\r");
                fclose(fp);

              }   
            else   
              {
               perror("Autosnoop Failed to open File.");
              }
          }
      }
*/

    /*
     * Snoop-o-rama.
     */
    if ( d->snoop_by != NULL )
    {
    if (d->character != NULL)
        write_to_buffer( d->snoop_by, d->character->name,0);
    write_to_buffer( d->snoop_by, "> ", 2 );
    write_to_buffer( d->snoop_by, d->outbuf, d->outtop );
    }

    /*
     * OS-dependent output.
     */
    if ( !write_to_descriptor( d, d->outbuf, d->outtop ) )
    {
    d->outtop = 0;
    return FALSE;
    }
    else
    {
    d->outtop = 0;
    return TRUE;
    }
}

/*
 * Bust a prompt (player settable prompt)
 * coded by Morgenes for Aldara Mud
 */
void bust_a_prompt( CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    const char *str;
    const char *i;
    char *point;
    char doors[MAX_INPUT_LENGTH];
    EXIT_DATA *pexit;
    bool found;
    const char *dir_name[] = {"N","E","S","W","U","D","NE","SE","SW","NW"};
    int door;
    int percent;
 
    point = buf;
    str = ch->prompt;
    if( !str || str[0] == '\0')
    {
        sprintf( buf, "{c<{%dhp %dm %dmv>{x %s",
        ch->hit, ch->mana, ch->move, ch->prefix );
    send_to_char( buf, ch );
    return;
    }

   if (IS_SET(ch->comm,COMM_AFK))
   {
    send_to_char("{c<{WYou are {RAFK{c>  {x",ch);
    return;
   }

   while( *str != '\0' )
   {
      if( *str != '%' )
      {
         *point++ = *str++;
         continue;
      }
      ++str;
      switch( *str )
      {
         default :
            i = " "; break;
    case 'e':
        found = FALSE;
        doors[0] = '\0';
        for (door = 0; door < 10; door++)
        {
        if ((pexit = ch->in_room->exit[door]) != NULL
        &&  pexit ->u1.to_room != NULL
        &&  (can_see_room(ch,pexit->u1.to_room)
        ||   (IS_AFFECTED(ch,AFF_INFRARED) 
        &&    !IS_AFFECTED(ch,AFF_BLIND)))
        &&  !IS_SET(pexit->exit_info,EX_CLOSED))
        {
            found = TRUE;
            strcat(doors,dir_name[door]);
        }
        }
        if (!found)
        strcat(buf,"none");
        sprintf(buf2,"%s",doors);
        i = buf2; break;
     case 'c' :
        sprintf(buf2,"%s","\n\r");
        i = buf2; break;
         case 'h' :
/*     
       sprintf( buf2, "%d", ch->hit );
            i = buf2; break;
*/
         if(ch->hit >= (ch->max_hit * .65))  
            sprintf( buf2, "{G%d{x", ch->hit );
         else
           if((ch->hit >= (ch->max_hit * .25)) && (ch->hit < (ch->max_hit * .65)))
            sprintf( buf2, "{Y%d{x", ch->hit );
         else
            sprintf( buf2, "{R%d{x", ch->hit );
            i = buf2; break;
         case 'H' :
            sprintf( buf2, "%d", ch->max_hit );
            i = buf2; break;
         case 'm' :
/*
            sprintf( buf2, "%d", ch->mana );
            i = buf2; break;
*/
         if(ch->mana >= (ch->max_mana * .65))  
            sprintf( buf2, "{G%d{x", ch->mana );
         else
           if((ch->mana >= (ch->max_mana * .25)) && (ch->mana < (ch->max_mana * .65)))
            sprintf( buf2, "{Y%d{x", ch->mana );
         else
            sprintf( buf2, "{R%d{x", ch->mana );
            i = buf2; break;
         case 'M' :
            sprintf( buf2, "%d", ch->max_mana );
            i = buf2; break;
         case 'v' :
/*
            sprintf( buf2, "%d", ch->move );
            i = buf2; break;
*/

         if(ch->move >= (ch->max_move * .65))  
            sprintf( buf2, "{G%d{x", ch->move );
         else
           if((ch->move >= (ch->max_move * .25)) && (ch->move < (ch->max_move * .65)))
            sprintf( buf2, "{Y%d{x", ch->move );
         else
            sprintf( buf2, "{R%d{x", ch->move );
            i = buf2; break;
         case 'V' :
            sprintf( buf2, "%d", ch->max_move );
            i = buf2; break;
         case 'x' :
            sprintf( buf2, "%d", ch->exp );
            i = buf2; break;
     case 'X' :
        sprintf(buf2, "%d", IS_NPC(ch) ? 0 :
        (ch->level + 1) * exp_per_level(ch,ch->pcdata->points) - ch->exp);
        i = buf2; break;
         case 'g' :
            sprintf( buf2, "%ld", ch->gold);
            i = buf2; break;
     case 's' :
        sprintf( buf2, "%ld", ch->silver);
        i = buf2; break;
         case 'a' :
            if( ch->level > 9 )
               sprintf( buf2, "%d", ch->alignment );
            else
               sprintf( buf2, "%s", IS_GOOD(ch) ? "{Wgood{x" : IS_EVIL(ch) ?
                "{Devil{x" : "{dneutral{x" );
            i = buf2; break;
         case 'r' :
            if( ch->in_room != NULL )
               sprintf( buf2, "%s", 
        ((!IS_NPC(ch) && IS_SET(ch->pact,PLR_HOLYLIGHT)) ||
         (!IS_AFFECTED(ch,AFF_BLIND) && !room_is_dark( ch->in_room )))
        ? ch->in_room->name : "darkness");
            else
               sprintf( buf2, " " );
            i = buf2; break;
         case 'R' :
            if( IS_IMMORTAL( ch ) && ch->in_room != NULL )
               sprintf( buf2, "%d", ch->in_room->vnum );
            else
               sprintf( buf2, " " );
            i = buf2; break;
         case 'z' :
            if( IS_IMMORTAL( ch ) && ch->in_room != NULL )
               sprintf( buf2, "%s", ch->in_room->area->name );
            else
               sprintf( buf2, " " );
            i = buf2; break;
         case '%' :
            sprintf( buf2, "%%" );
            i = buf2; break;
         case 'o' :
            sprintf( buf2, "%s", olc_ed_name(ch) );
            i = buf2; break;
     case 'O' :
        sprintf( buf2, "%s", olc_ed_vnum(ch) );
        i = buf2; break;

 case 'K' :
           if ((victim = ch->fighting) != NULL && can_see(ch,victim))
             {
                if(victim->max_hit>0)
                  percent = ((victim->hit * 100) / victim->max_hit);
                else
                  percent = 0;
              if(percent >= 65)
                sprintf(buf2,"{cTarget{w: {c[{G%d{x%%{c]{x",percent);
              else 
		  if(percent >= 25 && percent < 65)
                 sprintf(buf2,"{cTarget{w: {c[{Y%d{x%%{c]{x",percent);
              else
                 sprintf(buf2,"{cTarget{w: {c[{R%d{x%%{c]{x",percent);
              i = buf2;break;
             }
           else
             sprintf( buf2," " );
             i = buf2; break;

   }
      ++str;
      while( (*point = *i) != '\0' )
         ++point, ++i;
   }
   *point   = '\0';
   write_to_buffer( ch->desc, buf, 0 );

   if (ch->prefix[0] != '\0')
        write_to_buffer(ch->desc,ch->prefix,0);
   return;
}

/*
 * Append onto an output buffer.
 */
void write_to_buffer( DESCRIPTOR_DATA *d, const char *txt, int length )
{
    /*
     * Find length in case caller didn't.
     */
    if ( length <= 0 )
        length = strlen(txt);

    /*
     * Initial \n\r if needed.
     */
    if ( d->outtop == 0 && !d->fcommand )
    {
    d->outbuf[0]    = '\n';
    d->outbuf[1]    = '\r';
    d->outtop   = 2;
    }

    /*
     * Expand the buffer as needed.
     */
    while ( d->outtop + length >= d->outsize )
    {
    char *outbuf;

        if (d->outsize >= 32000)
    {
        bug("Buffer overflow. Closing.\n\r",0);
        close_socket(d);
        return;
    }
    outbuf      = alloc_mem( 2 * d->outsize );
    strncpy( outbuf, d->outbuf, d->outtop );
    free_mem( d->outbuf, d->outsize );
    d->outbuf   = outbuf;
    d->outsize *= 2;
    }

    /*
     * Copy.
     */
/* bug fixed - WIZZLE 
    strncpy( d->outbuf + d->outtop, txt,length);
*/
    strcpy( d->outbuf + d->outtop, txt);
    d->outtop += length;
    return;
}

void send_to_desc( const char *txt, DESCRIPTOR_DATA *d )
{
    const	char 	*point;
    		char 	*point2;
    		char 	buf[ MAX_STRING_LENGTH*4 ];
		int	skip = 0;

    buf[0] = '\0';
    point2 = buf;
    if( txt && d )
	{
	    if( d->fcolour == TRUE )
	    {
		for( point = txt ; *point ; point++ )
	        {
		    if( *point == '{' )
		    {
			point++;
			skip = colour( *point, NULL, point2 );
			while( skip-- > 0 )
			    ++point2;
			continue;
		    }
		    *point2 = *point;
		    *++point2 = '\0';
		}
		*point2 = '\0';
        	write_to_buffer( d, buf, point2 - buf );
	    }
	    else
	    {
		for( point = txt ; *point ; point++ )
	        {
		    if( *point == '{' )
		    {
			point++;
			continue;
		    }
		    *point2 = *point;
		    *++point2 = '\0';
		}
		*point2 = '\0';
        	write_to_buffer( d, buf, point2 - buf );
	    }
	}
    return;
}

/*
 * Lowest level output function.
 * Write a block of text to the file descriptor.
 * If this gives errors on very long blocks (like 'ofind all'),
 *   try lowering the max block size.
 */
bool write_to_descriptor( DESCRIPTOR_DATA *d, char *txt, int length )
{
    int iStart;
    int nWrite;
    int nBlock;
    char                *pbuff;
    char                buffer[ MAX_STRING_LENGTH*4 ];
   pbuff        = buffer;
   colourconv( pbuff, txt, d);
   txt=pbuff;

    length = strlen(txt);

    for ( iStart = 0; iStart < length; iStart += nWrite )
    {
    nBlock = UMIN( length - iStart, 3072 );
    if ( ( nWrite = write( d->descriptor, txt + iStart, nBlock ) ) < 0 )
        { perror( "Write_to_descriptor" ); return FALSE; }
    } 

    return TRUE;
}



/*
 * Deal with sockets that haven't logged in yet.
 */
void nanny( DESCRIPTOR_DATA *d, char *argument )
{
    DESCRIPTOR_DATA *d_old, *d_next;
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *ch;
    char *pwdnew;
    char *p;
    int iClass,race,i,weapon;
    bool fOld;
    CHAR_DATA * original;
    HELP_DATA *pHelp;
    int maxGreetings=0, randomGreeting=0;

    /* Delete leading spaces UNLESS character is writing a note */
    if (d->connected != CON_NOTE_TEXT)
    {
        while ( isspace(*argument) )
            argument++;
    }

    ch = d->character;

    switch ( d->connected )
    {
        default:
            bug( "Nanny: bad d->connected %d.", d->connected );
            close_socket( d );
            return;
            
        case CON_GET_COLOUR:
            switch( argument[0] )
            {
                case '\0':
                    close_socket( d );
                    return;
                case 'y': case 'Y':
                    d->fcolour = TRUE;
                    write_to_buffer( d, "\n\r{RA{WN{BS{GI {Wset.\n\r{x", 0 );
                    break;
                case 'n': case 'N':
                    write_to_buffer( d, "\n\rANSI not set.{x\n\r\n\r", 0 );
                    d->fcolour = FALSE;
                    break;
                default:
                    if ( d->repeat >= 5 )
                        close_socket( d );
                    else
                        write_to_buffer( d, "\n\rDo you want ANSI? (YES or NO): ", 0 );
                    return;
            }
            d->connected = CON_GET_NAME;
            {
                for(maxGreetings=0,pHelp=help_first;pHelp;pHelp=pHelp->next)
                {
                    if(strstartswith(pHelp->keyword, "GREETING"))
                        maxGreetings++;
                }
                
                randomGreeting = number_range(1, maxGreetings);
                
                for(maxGreetings=0,pHelp=help_first;pHelp;pHelp=pHelp->next)
                {
                    if(strstartswith(pHelp->keyword, "GREETING"))
                    {
                        maxGreetings++;
                        if(maxGreetings==randomGreeting)
                        {
                            if ( pHelp->text[0] == '.' )
                                write_to_buffer( d, pHelp->text+1, 0 );
                            else
                                write_to_buffer( d, pHelp->text  , 0 );
                        
                            break;
                        }
                    }
                }
                
            }
            break;
            
        case CON_GET_NAME:
            if ( argument[0] == '\0' )
            {
                close_socket( d );
                return;
            }

            argument[0] = UPPER(argument[0]);
            if ( !check_parse_name( argument ) )
            {
                write_to_buffer( d, 
"\n\r{R*** {WThat is {RNOT {Wan allowable {CNAME {R***\n\r\n\r"
"{cChoose your {CCHARACTER's NAME {c{w:{W ", 0);
                return;
            }


            fOld = load_char_obj( d, argument ); 
            ch   = d->character;

            if(d->fcolour)
                SET_BIT(ch->pact, PLR_COLOUR);
            else
                REMOVE_BIT(ch->pact, PLR_COLOUR);

            if (IS_SET(ch->pact, PLR_DENY))
            {
                sprintf( log_buf, "Denying access to %s@%s.", argument, d->host );
                log_string( log_buf );
                write_to_buffer( d, "{cYou are{R denied {caccess.{x\n\r", 0 );
                close_socket( d );
                return;
            }

           /* if(strstr(d->host, ".aol.com"))
		{

			if(strcmp(argument, "Bim"))
		{
			sprintf(log_buf, "Player %s from %s tried to log on. Denied.", argument, d->host);
               log_string( log_buf );
                write_to_buffer( d, "{cYou are{R denied {caccess.{x\n\r", 0 );
                close_socket( d );
                return;

		}

		}*/

	  if(ch->level > MAX_LEVEL)
	{
	sprintf(log_buf, "%s had level greater than %d", ch->name, MAX_LEVEL);
	log_string(log_buf);	
	write_to_buffer(d, "\n\r{RTrying to cheat or something?  Your LEVEL is too HIGH{r!{x\n\r",0);
	close_socket(d);
	return;
	}
       
              if(ch->perm_stat[STAT_CON] <=1)
                {
                    write_to_buffer( d,"\n\r{RBecause you have died so many times, your soul has\n\r",0);
                    write_to_buffer( d,"{Rmoved on into the AfterWorld.  You may want to start exploring\n\r",0);
                    write_to_buffer( d,"{Rthe 'NEW CHARACTER' creation process!\n\r",0);
                    close_socket( d );
                    return;
                }


         if (check_ban(d->host,BAN_PERMIT) && !IS_SET(ch->pact,PLR_PERMIT))
            {
                write_to_buffer(d,"{cYour site has been {Rbanned{c from KOTRD.{x\n\r",0);
                close_socket(d);
                return;
            }

            if ( check_reconnect( d, argument, FALSE ) )
                fOld = TRUE;
            else
            {
                if ( wizlock && !IS_IMMORTAL(ch)) 
                {
                    write_to_buffer( d, "{cThe game is{R wizlocked{c.{x\n\r", 0 );
                    close_socket( d );
                    return;
                }
            }

            if ( fOld )
            {
                /* Old player */
                write_to_buffer( d, "{cPassword:{w ", 0 );
                write_to_buffer( d, echo_off_str, 0 );
                d->connected = CON_GET_OLD_PASSWORD;
                return;
            }
            else
            {
                /* New player */
                if (newlock)
                {
                    write_to_buffer( d, "{cThe game is {Rnewlocked{c.{x\n\r", 0 );
                    close_socket( d );
                    return;
                }

                if (check_ban(d->host,BAN_NEWBIES))
                {
                    write_to_buffer(d, "{cNew players are not allowed from your site.{x\n\r",0);
                    close_socket(d);
                    return;
                }
    
                sprintf( buf, "\n\r{W%s  {c-  Is that the {CNAME {cyou wish to use?  {c({GYES{c/{RNO{c)?{w:{W  ", argument );
                write_to_buffer( d, buf, 0 );
                d->connected = CON_CONFIRM_NEW_NAME;
                return;
            }
            break;

        case CON_GET_OLD_PASSWORD:
            #if defined(unix)
                write_to_buffer( d, "\n\r", 2 );
            #endif


            if ( strcmp( crypt( argument, ch->pcdata->pwd ), ch->pcdata->pwd ))
            {
                write_to_buffer( d, "{RWrong password.{x\n\r", 0 );
                close_socket( d );
                return;
            }

	    if ( ch->pcdata->remorting )  
	      {
               write_to_buffer( d, "\n\r{cResuming {WREMORTALIZE{c...{x\n\r\n\r", 0);

            write_to_buffer(d,
                "\n\r\n\r\n\r{c/---------------------------------------------------------\\\n\r", 0);
            write_to_buffer(d,
 "{c| {CType {c'{WRACESTAT {c<{WRACENAME{c>' {Cto see the {WRACE's {Cwrite-up.  {c|{x\n\r",0);
write_to_buffer(d,"{c|------------------------------------------------------------------------------\\\n\r",0);
            write_to_buffer(d,
"{c|{C Race Name {c|{C Points {c| {CAlignments    {c| {CClasses Availble to Race {c         | {CSex {c|\n\r", 0);


write_to_buffer(d,"{c|------------------------------------------------------------------------------|\n\r",
0);

            for(race=1;pc_race_table[race].name != NULL; race++)
            {
if (race_table[race].remort_race == TRUE)
   {
    sprintf(buf, 
"{c| {C%-9s {c| {C%-6d {c| {C%-13s {c| {C%-33s {c| {C%-3s {c|{x\n\r",
capitalize(pc_race_table[race].name), pc_race_table[race].points, pc_race_table[race].align,
              pc_race_table[race].class, pc_race_table[race].sex);
   }
else
if (race_table[race].name == race_table[19].name)   
   {
/* break; */   
continue;
   }
 else
   {
    sprintf(buf, 
"{c| {x%-9s {c| {x%-6d {c| {x%-13s {c| {x%-33s {c| {x%-3s {c|{x\n\r",
capitalize(pc_race_table[race].name), pc_race_table[race].points, pc_race_table[race].align,
              pc_race_table[race].class, pc_race_table[race].sex);
   }
                 
       write_to_buffer(d, buf, 0);
           
 }

write_to_buffer(d,"{c\\------------------------------------------------------------------------------/{x\n\r",
0);
            write_to_buffer(d, 
"\n\r{R*** {WThe {CHIGHLIGHTED RACEs {Wcan ONLY be choosen if you are {CREMORTTING {R***{x", 0);       
            write_to_buffer(d, "\n\r", 0);
            write_to_buffer(d, "\n\r{cChoose your {CRACE{w:{W ", 0);

               d->connected = CON_GET_NEW_RACE;
               return;
              }

            write_to_buffer( d, echo_on_str, 0 );
            if (check_playing(d,ch->name))
                return;
            if ( check_reconnect( d, ch->name, TRUE ) )
                return;
	write_to_buffer(d, "{x\n\r", 0);

            sprintf( log_buf, "%s@%s has connected.", ch->name, d->host );
            log_string( log_buf );
            wiznet(log_buf,NULL,NULL,WIZ_SITES,0,ch->level); 

     // Explorer/Killer Percentages
     do_top_percentages(ch, 0);

if (IS_IMP(ch))
  {
   
   write_to_buffer(d, "{WYou may log in as {c[{CW{c]{Cizi{W, {c[{CI{c]{Cncog{W, {c[{CB{c]{Coth{W, or {c[{CV{c]{Cisible{W.", 0);
   write_to_buffer(d, "{W\n\r\n\rOR\n\r\n\r", 0);
   write_to_buffer(d, "{WPress {c[{CRETURN{c]{W to continue...", 0); 
   d->connected = CON_VISIBILITY;
   }
else
    {
        if ( IS_IMMORTAL(ch) )
            {
                do_function(ch, &do_help, "imotd" );
                d->connected = CON_READ_IMOTD;
            }   
            else
            {
                do_function(ch, &do_help, "motd" );
                d->connected = CON_READ_MOTD;
            }  
     }
break;


    /* RT code for breaking link */
        case CON_BREAK_CONNECT:
            switch( *argument )
            {
                case 'y' : case 'Y':
                    for ( d_old = descriptor_list; d_old != NULL; d_old = d_next )
                    {
                        d_next = d_old->next;
                        if (d_old == d || d_old->character == NULL)
                            continue;

                        if (str_cmp(ch->name,d_old->original ? d_old->original->name :
                                             d_old->character->name))
                            continue;

                        close_socket(d_old);
                    }
                
                    if (check_reconnect(d,ch->name,TRUE))
                        return;
                    write_to_buffer(d,"{cReconnect attempt {Rfailed{c.\n\rName:{w ",0);
                    if ( d->character != NULL )
                    {
                        free_char( d->character );
                        d->character = NULL;
                    }
                    d->connected = CON_GET_NAME;
                    break;

                case 'n' : case 'N':
                    write_to_buffer(d,"{RName:{w ",0);
                    if ( d->character != NULL )
                    {
                        free_char( d->character );
                        d->character = NULL;
                    }
                    d->connected = CON_GET_NAME;
                    break;

                default:
                    write_to_buffer(d,"{cPlease type{G Y{c or{R N{c?{w ",0);
                    break;
                }
                break;

        case CON_CONFIRM_NEW_NAME:
            switch ( *argument )
            {
                case 'y': case 'Y':
                    sprintf( buf, 
"\n\r{R*** {WBEGINNING the {CNEW CHARACTER {WCREATION PROCESS {R***\n\r\n\r{cChoose a {CPASSWORD {cfor {W%s{w:{W %s",
                    ch->name, echo_off_str );
                    write_to_buffer( d, buf, 0 );
                    d->connected = CON_GET_NEW_PASSWORD;
                    break;

                case 'n': case 'N':
                    write_to_buffer( d,"{cChoose your {CCHARACTER's NAME {c{w:{W ", 0 );
                    free_char( d->character );
                    d->character = NULL;
                    d->connected = CON_GET_NAME;
                    break;

                default:
                    write_to_buffer( d, "{cType {GYES{c or{R NO{c {w: ", 0 );
                    break;
            }
            break;

        case CON_GET_NEW_PASSWORD:
            #if defined(unix)
                write_to_buffer( d, "\n\r", 2 );
            #endif

            if ( strlen(argument) < 5 )
            {
                write_to_buffer( d,
"\n\r{R*** {WYour {CPASSWORD {Wmust be at least {RFIVE {c({R5{c){W characters in length {R***{x\n\r\n\r{cReEnter {CPASSWORD{w:{W ",0 );
                return;
            }

            pwdnew = crypt( argument, ch->name );
            for ( p = pwdnew; *p != '\0'; p++ )
            {
                if ( *p == '~' )
                {
                    write_to_buffer( d,
                    "\n\r\n\r{R*** {WThat is {RNOT {Wan {RACCEPTABLE {CPASSWORD {R***\n\r\n\rReEnter {CPASSWORD{w:{W ",0 );
                    return;
                }
            }

            free_string( ch->pcdata->pwd );
            ch->pcdata->pwd = str_dup( pwdnew );
            write_to_buffer( d, "\n\r{cReEnter {CPASSWORD {cto confirm{w:{W ", 0 );
            d->connected = CON_CONFIRM_NEW_PASSWORD;
            break;

        case CON_CONFIRM_NEW_PASSWORD:
            #if defined(unix)
                write_to_buffer( d, "\n\r", 2 );
            #endif

            if ( strcmp( crypt( argument, ch->pcdata->pwd ), ch->pcdata->pwd ) )
            {
write_to_buffer( d, "\n\r\n\r{R*** {WThe {CPASSWORD {Wconfirmation has {RFAILED{W! {R***\n\r\n\r{cReEnter {CPASSWORD{w:{W ",0 );
                d->connected = CON_GET_NEW_PASSWORD;
                return;
            }
            write_to_buffer( d, echo_on_str, 0 );
            
            write_to_buffer(d, "\n\r\n\r{RHardcore{W mode means that when you die, you start back over at level 1!{x\n\r", 0);
            write_to_buffer(d, "{WYou don't lose any skills or items, but you have to level up all over again from level 1!{x\n\r\n\r", 0);
            write_to_buffer(d, "{WDO YOU WISH GO HARDCORE? {c({GYES{c/{RNO{c){W ", 0);
            d->connected = CON_GET_HARDCORE;
            break;
        
        case CON_GET_HARDCORE:
            switch (argument[0] )
   			{
   				case 'y': case 'Y':
     				write_to_buffer( d, "\n\r", 2 );
     				ch->pcdata->hardcore = TRUE;
					break;
				case 'n': case 'N':
     				write_to_buffer( d, "\n\r", 2 );
     				ch->pcdata->hardcore = FALSE;
     				break;
   				default: write_to_buffer( d, "\n\r{GYES {cor {RNO{c?{w:  ", 0 );
     				return;
			}            
        
            write_to_buffer(d, "\n\r\n\r\n\r{c/---------------------------------------------------------\\\n\r", 0);
            write_to_buffer(d, "{c| {CType {c'{WRACESTAT {c<{WRACENAME{c>' {Cto see the {WRACE's {Cwrite-up{c.  {c|{x\n\r",0);
            write_to_buffer(d,"{c|------------------------------------------------------------------------------\\\n\r",0);
            write_to_buffer(d, "{c|{C Race Name {c|{C Points {c| {CAlignments    {c| {CClasses Availble to Race {c         | {CSex {c|\n\r", 0);
            write_to_buffer(d,"{c|------------------------------------------------------------------------------|\n\r",0);

            for(race=1;pc_race_table[race].name != NULL; race++)
            {
                if (race_table[race].remort_race == TRUE)
               {
                sprintf(buf, 
            "{c| {C%-9s {c| {C%-6d {c| {C%-13s {c| {C%-33s {c| {C%-3s {c|{x\n\r",
            capitalize(pc_race_table[race].name), pc_race_table[race].points, pc_race_table[race].align,
                          pc_race_table[race].class, pc_race_table[race].sex);
               }
                else
                {
                    if (race_table[race].name == race_table[19].name)
                    {
                        /* break; */
                        continue;
                    }
                    else
                    {
                        sprintf(buf, 
                    "{c| {x%-9s {c| {x%-6d {c| {x%-13s {c| {x%-33s {c| {x%-3s {c|{x\n\r",
                    capitalize(pc_race_table[race].name), pc_race_table[race].points, pc_race_table[race].align,
                                  pc_race_table[race].class, pc_race_table[race].sex);
                    }
                }

                write_to_buffer(d, buf, 0);
            }

            write_to_buffer(d,"{c\\------------------------------------------------------------------------------/{x\n\r",0);
            write_to_buffer(d, "\n\r{R*** {WThe {CHIGHLIGHTED RACEs {Wcan ONLY be choosen if you are {CREMORTTING {R***{x", 0);
            write_to_buffer(d, "\n\r", 0);
            write_to_buffer(d, "\n\r{cChoose your {CRACE{w:{W ", 0);

            d->connected = CON_GET_NEW_RACE;
            break;

        case CON_GET_NEW_RACE:
            one_argument(argument,arg);

            if (!strcmp(arg,"racestat"))
            {
                argument = one_argument(argument,arg);
                if (argument[0] == '\0')
                    do_function(ch, &do_help, "race help");
                else
                    do_racestat(ch, argument);
                write_to_buffer(d, "{c\n\rChoose your {CRACE{w:{W ", 0);
                break;
            }

            race = race_lookup(argument);

            if ((race_table[race].remort_race == TRUE)
            && (!ch->pcdata->remorting))
              {
       write_to_buffer(d, 
"\n\r{R*** {WONLY those that are {CREMORTTING {Wmay select that {CRACE {R***{x\n\r\n\r", 0);
                write_to_buffer(d, "{c\n\rChoose your {CRACE{w:{W ", 0);
                break;
            }
     
            if (race == 0 || !race_table[race].pc_race)
            {
                write_to_buffer(d,"\n\r{R*** {WThat is {RNOT {Wa {RVALID {cRACE {R***{x\n\r\n\r",0);
                write_to_buffer(d,
                "\n\r\n\r\n\r{c/---------------------------------------------------------\\\n\r", 0);
                write_to_buffer(d,
 "{c| {CType {c'{WRACESTAT {c<{WRACENAME{c>' {Cto see the {WRACE's {Cwrite-up.  {c|{x\n\r",0);

write_to_buffer(d,"{c|------------------------------------------------------------------------------\\\n\r",0);
                write_to_buffer(d,
"{c|{C Race Name {c|{C Points {c| {CAlignments    {c| {CClasses Availble to Race {c         | {CSex {c|\n\r", 0);

write_to_buffer(d,"{c|------------------------------------------------------------------------------|\n\r",
0);

                for(race=1;pc_race_table[race].name != NULL; race++)
                {
if (race_table[race].remort_race == TRUE)
   {
    sprintf(buf,
"{c| {C%-9s {c| {C%-6d {c| {C%-13s {c| {C%-33s {c| {C%-3s {c|{x\n\r",
capitalize(pc_race_table[race].name), pc_race_table[race].points,
pc_race_table[race].align,
              pc_race_table[race].class, pc_race_table[race].sex);
   }
else
if (race_table[race].name == race_table[19].name)   
   {
/* break; */   
continue;
   }
 else
   {
    sprintf(buf,
"{c| {x%-9s {c| {x%-6d {c| {x%-13s {c| {x%-33s {c| {x%-3s {c|{x\n\r",
capitalize(pc_race_table[race].name), pc_race_table[race].points,
pc_race_table[race].align,
              pc_race_table[race].class, pc_race_table[race].sex);
   }

      write_to_buffer(d, buf, 0);
                }

write_to_buffer(d,"{c\\------------------------------------------------------------------------------/{x\n\r",
0);
            write_to_buffer(d, 
"\n\r{R*** {WThe {CHIGHLIGHTED RACEs {Wcan ONLY be choosen if you are {CREMORTTING {R***{x", 0);       
            write_to_buffer(d, "\n\r", 0);
            write_to_buffer(d, "\n\r{cChoose your {CRACE{w:{W ", 0);
                break;
            }

            ch->race = race;
            /* initialize stats */
            for (i = 0; i < MAX_STATS; i++)
            ch->perm_stat[i] = pc_race_table[race].stats[i];
            ch->affected_by = ch->affected_by|race_table[race].aff;
            ch->affected2_by = ch->affected2_by|race_table[race].aff2;
            ch->imm_flags   = ch->imm_flags|race_table[race].imm;
            ch->res_flags   = ch->res_flags|race_table[race].res;
            ch->vuln_flags  = ch->vuln_flags|race_table[race].vuln;
            ch->form    = race_table[race].form;
            ch->parts   = race_table[race].parts;

            /* add skills */
            for (i = 0; i < 5; i++)
            {
                if (pc_race_table[race].skills[i] == NULL)
                    break;
                group_add(ch,pc_race_table[race].skills[i],FALSE);
            }
            /* add cost */
            ch->pcdata->points = pc_race_table[race].points;
            ch->size = pc_race_table[race].size;

write_to_buffer(d,"\n\r\n\r\n\r\n\r{c/----------------------------------------------------------------\\\n\r",0);
write_to_buffer(d,
"{c| {CAvailable Classes {c- {Ctype {c'{WCLASSTAT {c<{WCLASSNAME{c>{c' {Cfor {CCLASS {cinfo.{c|{x\n\r",0);
write_to_buffer(d,"{c|---------------------------------------------------------------------------\\\n\r",0);
write_to_buffer(d,
"{c|{C Class Name{c  | {CLegal Alignments  {c| {CQuick Class Description           {c|{C Sex {c| \n\r",0);
write_to_buffer(d,"{c|---------------------------------------------------------------------------|\n\r",0);

for(iClass=0; iClass<MAX_CLASS; iClass++)
  {
   if(legal_class(ch->race, class_table[iClass].name))
     {
  if (class_table[iClass].remort_class == TRUE)
      {
      sprintf(buf, "{c| {C%-11s {c| {C%-17s {c| {C%-33s {c| {C%-3s {c|{x\n\r",
	capitalize(class_table[iClass].name), class_table[iClass].align,
	class_table[iClass].desc, class_table[iClass].sex);
      }
   else
      {
      sprintf(buf, "{c| {x%-11s {c| {x%-17s {c| {x%-33s {c| {x%-3s {c|{x\n\r",
	capitalize(class_table[iClass].name), class_table[iClass].align,
	class_table[iClass].desc, class_table[iClass].sex);
      }		

	write_to_buffer(d, buf, 0);
				}
			}
/* back1 */
write_to_buffer(d,"{c\\---------------------------------------------------------------------------/{x\n\r",0);

            write_to_buffer(d, 
"\n\r{R*** {WThe {CHIGHLIGHTED CLASSes {Wcan ONLY be choosen if you are {CREMORTTING {R***{x", 0);       
            write_to_buffer(d, "\n\r", 0);
            write_to_buffer(d, "\n\r{cChoose your {CCLASS{w:{W ", 0);
			
            d->connected = CON_GET_NEW_CLASS;
            break;
        

 case CON_GET_NEW_CLASS:

            one_argument(argument,arg);
            
           if (!strcmp(arg,"classtat"))
            {
                argument = one_argument(argument,arg);
                if (argument[0] == '\0')
                    do_classtat(ch, argument);
                else
                    do_classtat(ch, argument);
                write_to_buffer(d, "\n\r\n\r{cChoose your CLASS{w: ", 0);
                break;
            }

   if (!str_cmp( argument, "back") )			
     {
      write_to_buffer( d,"{\n\r\n\r\n\r\n\r{RPress {c<{WENTER {cor {WRETURN{c> {Rto continue{x",0 );
      d->connected = CON_GET_NEW_RACE;
      return;
     }
                 iClass = class_lookup(argument);

   if ((class_table[iClass].remort_class == TRUE)
   && (!ch->pcdata->remorting))
     {
            write_to_buffer(d, 
"\n\r{R*** {WThe {CHIGHLIGHTED CLASSes {Wcan ONLY be choosen if you are {CREMORTTING {R***{x", 0);       
            write_to_buffer(d, "\n\r", 0);
            write_to_buffer(d, "\n\r{cChoose your {CCLASS{w:{W ", 0);
      break;
     }

if ( ch->pcdata->oldcl != -1 && ch->pcdata->oldcl == iClass )
{
write_to_buffer( d, 
"\n\r{R*** {WChoosing the same {CCLASS {Wyou are {CREMORTTING {Wfrom is pointless {R***\n\r",0);
write_to_buffer(d, "\n\r{cChoose your {CCLASS{w:{W {x", 0);
return;
}
 
if(iClass == -1 || !legal_class(ch->race, class_table[iClass].name))
  {
write_to_buffer(d, "\n\r{R*** {WThat is {RNOT {Wa {RVALID {CCLASS {Wselection {R***\n\r", 0);
write_to_buffer(d,"\n\r\n\r\n\r\n\r{c/----------------------------------------------------------------\\\n\r",0);
write_to_buffer(d,"{c| {CAvailable Classes {c- {Ctype {c'{WCLASSTAT {c<{WCLASSNAME{c>' {Cfor {CCLASS {cinfo.{c|{x\n\r", 0);
write_to_buffer(d,"{c|---------------------------------------------------------------------------\\\n\r",0);
write_to_buffer(d,"{c|{C Class Name{c  |{C Legal Alignments  {c|{C Quick Class Description {c|{CSex {c|    \n\r", 0);
write_to_buffer(d,"{c|---------------------------------------------------------------------------|\n\r",0);

	for(iClass=0; iClass<MAX_CLASS; iClass++)
	  {
	   if(legal_class(ch->race, class_table[iClass].name))
	     {
  if (class_table[iClass].remort_class == TRUE)
      {
      sprintf(buf, "{c| {C%-11s {c| {C%-17s {c| {C%-33s {c| {C%-3s {c|{x\n\r",
	capitalize(class_table[iClass].name), class_table[iClass].align,
	class_table[iClass].desc, class_table[iClass].sex);
      }
   else
      {
      sprintf(buf, "{c| {x%-11s {c| {x%-17s {c| {x%-33s {c| {x%-3s {c|{x\n\r",
	capitalize(class_table[iClass].name), class_table[iClass].align,
	class_table[iClass].desc, class_table[iClass].sex);
      }		
	      write_to_buffer(d, buf, 0);
	     }
	  }
	
write_to_buffer(d,"{c\\---------------------------------------------------------------------------/{x\n\r",0);
            write_to_buffer(d, 
"\n\r{R*** {WThe {CHIGHLIGHTED CLASSes {Wcan ONLY be choosen if you are {CREMORTTING {R***{x", 0);       
            write_to_buffer(d, "\n\r", 0);
            write_to_buffer(d, "\n\r{cChoose your {CCLASS{w:{W ", 0);
				return;
			}


			ch->class = iClass;

			sprintf( log_buf, "%s@%s new player.", ch->name, d->host );
			log_string( log_buf );
			wiznet("Newbie alert!  $N sighted.",ch,NULL,WIZ_NEWBIE,0,0);
			wiznet(log_buf,NULL,NULL,WIZ_SITES,0,ch->level);

			write_to_buffer(d, "\n\r\n\r", 2);
                        write_to_buffer(d, "{cYou may be{w: ", 0);
			if(legal_align(class_table[ch->class].align, "Good") &&
			   legal_align(pc_race_table[ch->race].align, "Good"))
				write_to_buffer(d, "{WGOOD ", 0);
			if(legal_align(class_table[ch->class].align, "Neutral") &&
			   legal_align(pc_race_table[ch->race].align, "Neutral"))
				write_to_buffer(d, "{wNEUTRAL ", 0);
			if(legal_align(class_table[ch->class].align, "Evil") &&
			   legal_align(pc_race_table[ch->race].align, "Evil"))
				write_to_buffer(d, "{DEVIL ", 0);
			write_to_buffer(d, "\n\r\n\r", 0);
			write_to_buffer(d, "{cChoose your {CALIGNMENT{w:{W ",0);
			
    		d->connected = CON_GET_ALIGNMENT;
    		break;

		case CON_GET_ALIGNMENT:

			if(argument[0]=='g' || argument[0]=='G')
			{
				if(legal_align(class_table[ch->class].align, "Good") &&
				   legal_align(pc_race_table[ch->race].align, "Good"))
				  /* ch->alignment=750; */
				   ch->alignment=1000;
				else
				{
			write_to_buffer(d, "\n\r{R*** {WThat is {RNOT {Wa {RVALID{W ALIGNMENT {R***\n\r",
							0);
					write_to_buffer(d, "\n\r\n\r{cYou may be{w: ", 0);
					if(legal_align(class_table[ch->class].align, "Good") &&
			   		   legal_align(pc_race_table[ch->race].align, "Good"))
						write_to_buffer(d, "{WGOOD ", 0);
					if(legal_align(class_table[ch->class].align, "Neutral") &&
					   legal_align(pc_race_table[ch->race].align, "Neutral"))
						write_to_buffer(d, "wNEUTRAL ", 0);
					if(legal_align(class_table[ch->class].align, "Evil") &&
					   legal_align(pc_race_table[ch->race].align, "Evil"))
						write_to_buffer(d, "{DEVIL ", 0);
					write_to_buffer(d, "\n\r\n\r", 0);
					write_to_buffer(d, "{cChoose your {CALIGNMENT{w:{W ",0);
					return;
				}
			}
			else if(argument[0]=='n' || argument[0]=='N')
			{
				if(legal_align(class_table[ch->class].align, "Neutral") &&
				   legal_align(pc_race_table[ch->race].align, "Neutral"))
					ch->alignment=0;
				else
				{
                       write_to_buffer(d, "\n\r{R*** {WThat is {RNOT {Wa {RVALID{W ALIGNMENT {R***\n\r",
                                                        0);
                                        write_to_buffer(d, "\n\r\n\r{cYou may be{w: ", 0);
                                        if(legal_align(class_table[ch->class].align, "Good") &&
                                           legal_align(pc_race_table[ch->race].align, "Good")) 
                                                write_to_buffer(d, "{WGOOD ", 0);
                                        if(legal_align(class_table[ch->class].align, "Neutral") &&
                                           legal_align(pc_race_table[ch->race].align, "Neutral")) 
                                                write_to_buffer(d, "wNEUTRAL ", 0);
                                        if(legal_align(class_table[ch->class].align, "Evil") &&
                                           legal_align(pc_race_table[ch->race].align, "Evil")) 
                                                write_to_buffer(d, "{DEVIL ", 0);
                                        write_to_buffer(d, "\n\r\n\r", 0);
                                        write_to_buffer(d, "{cChoose your {CALIGNMENT{w:{W ",0);
                                        return;
				}
			}
			else if(argument[0]=='e' || argument[0]=='E')
			{
				if(legal_align(class_table[ch->class].align, "Evil") &&
				   legal_align(pc_race_table[ch->race].align, "Evil"))
				   /* ch->alignment=-750; */
				   ch->alignment=-1000;
				else
				{
                       write_to_buffer(d, "\n\r{R*** {WThat is {RNOT {Wa {RVALID{W ALIGNMENT {R***\n\r",
                                                        0);
                                        write_to_buffer(d, "\n\r\n\r{cYou may be{w: ", 0);
                                        if(legal_align(class_table[ch->class].align, "Good") &&
                                           legal_align(pc_race_table[ch->race].align, "Good")) 
                                                write_to_buffer(d, "{WGOOD ", 0);
                                        if(legal_align(class_table[ch->class].align, "Neutral") &&
                                           legal_align(pc_race_table[ch->race].align, "Neutral")) 
                                                write_to_buffer(d, "wNEUTRAL ", 0);
                                        if(legal_align(class_table[ch->class].align, "Evil") &&
                                           legal_align(pc_race_table[ch->race].align, "Evil")) 
                                                write_to_buffer(d, "{DEVIL ", 0);
                                        write_to_buffer(d, "\n\r\n\r", 0);
                                        write_to_buffer(d, "{cChoose your {CALIGNMENT{w:{W ",0);
                                        return;
				}
			}
			else
			{
                       write_to_buffer(d, "\n\r{R*** {WThat is {RNOT {Wa {RVALID{W ALIGNMENT {R***\n\r",
                                                        0);
                                        write_to_buffer(d, "\n\r\n\r{cYou may be{w: ", 0);
                                        if(legal_align(class_table[ch->class].align, "Good") &&
                                           legal_align(pc_race_table[ch->race].align, "Good")) 
                                                write_to_buffer(d, "{WGOOD ", 0);
                                        if(legal_align(class_table[ch->class].align, "Neutral") &&
                                           legal_align(pc_race_table[ch->race].align, "Neutral")) 
                                                write_to_buffer(d, "wNEUTRAL ", 0);
                                        if(legal_align(class_table[ch->class].align, "Evil") &&
                                           legal_align(pc_race_table[ch->race].align, "Evil")) 
                                                write_to_buffer(d, "{DEVIL ", 0);
                                        write_to_buffer(d, "\n\r\n\r", 0);
                                        write_to_buffer(d, "{cChoose your {CALIGNMENT{w:{W ",0);
                                        return;
  		        }
			write_to_buffer(d,"\n\r",0);
			write_to_buffer(d, "{cYour SEX may be{w: ", 0);
			if(legal_sex(pc_race_table[ch->race].sex, "F") &&
			   legal_sex(class_table[ch->class].sex, "F"))
				write_to_buffer(d, "{CFEMALE{c ", 0);
			if(legal_sex(pc_race_table[ch->race].sex, "M") &&
			   legal_sex(class_table[ch->class].sex, "M"))
				write_to_buffer(d, "{CMALE{c ", 0);
                        if (ch->race == race_lookup("Myrddraal"))
                          {
			if(legal_sex(pc_race_table[ch->race].sex, "N") &&
			   legal_sex(class_table[ch->class].sex, "N"))
				write_to_buffer(d, "{CNEUTRAL ", 0);
			write_to_buffer(d, "\n\r", 0);
			write_to_buffer(d, "\n\r{cChoose your {CSEX{w:{W ", 0);
			d->connected = CON_GET_NEW_SEX;
                          }
			else
                          {
			write_to_buffer(d, "\n\r", 0);
			write_to_buffer(d, "\n\r{cChoose your {CSEX{w:{W ", 0);
			d->connected = CON_GET_NEW_SEX;
                          }
    		break;

		case CON_GET_NEW_SEX:
			if(argument[0]=='m' || argument[0]=='M')
			{
				if(legal_sex(pc_race_table[ch->race].sex, "M") &&
				   legal_sex(class_table[ch->class].sex, "M"))
					ch->pcdata->true_sex = SEX_MALE;
				else
				{
			write_to_buffer(d, "\n\r{R*** {WNOT a {RVALID {CSEX {R***\n\r",0);
					write_to_buffer(d, "{cYour SEX may be{w: ", 0);
					if(legal_sex(pc_race_table[ch->race].sex, "F") &&
					   legal_sex(class_table[ch->class].sex, "F"))
						write_to_buffer(d, "{CFEMALE ", 0);
					if(legal_sex(pc_race_table[ch->race].sex, "M") &&
					   legal_sex(class_table[ch->class].sex, "M"))
						write_to_buffer(d, "{CMALE ", 0);

                                        if (ch->race == race_lookup("Myrddraal"))
		                          {
                   			if(legal_sex(pc_race_table[ch->race].sex,"N")
					&& legal_sex(class_table[ch->class].sex,"N"))
						write_to_buffer(d, "{CNEUTRAL ",0);
                                          }

					write_to_buffer(d, "\n\r", 0);
					write_to_buffer(d, "\n\r{cChoose your {CSEX{w:{W ", 0);
					return;
				}
			}
			else if(argument[0]=='f' || argument[0]=='F')
			{
				if(legal_sex(pc_race_table[ch->race].sex, "F") &&
				   legal_sex(class_table[ch->class].sex, "F"))
					ch->pcdata->true_sex = SEX_FEMALE;
				else
				{
                        write_to_buffer(d, "\n\r{R*** {wNOT a {RVALID {CSEX {R***\n\r",0);
                                        write_to_buffer(d, "{cYour SEX may be{w: ", 0);
                                        if(legal_sex(pc_race_table[ch->race].sex, "F") &&
                                           legal_sex(class_table[ch->class].sex, "F"))
                                                write_to_buffer(d, "{CFEMALE ", 0);
                                        if(legal_sex(pc_race_table[ch->race].sex, "M") &&
                                           legal_sex(class_table[ch->class].sex, "M"))
                                                write_to_buffer(d, "{CMALE ", 0);
                        
                                        if (ch->race == race_lookup("Myrddraal"))
                                          {
                                        if(legal_sex(pc_race_table[ch->race].sex,"N")
                                        && legal_sex(class_table[ch->class].sex,"N"))
                                                write_to_buffer(d, "{CNEUTRAL ",0);
                                          }
                                        
                                        write_to_buffer(d, "\n\r", 0);
                                        write_to_buffer(d, "\n\r{cChoose your {CSEX{w:{W ", 0);
                                        return;
				}
			}
			else 
                        if((argument[0]=='n' || argument[0]=='N')
			&& (ch->race == race_lookup("Myrddraal")))
			{
				if(legal_sex(pc_race_table[ch->race].sex, "N") &&
				   legal_sex(class_table[ch->class].sex, "N"))
					ch->pcdata->true_sex = SEX_NEUTRAL;
				else
				{
                        write_to_buffer(d, "\n\r{R*** {wNOT a {RVALID {CSEX {R***\n\r",0);
                                        write_to_buffer(d, "{cYour SEX may be{w: ", 0);
                                        if(legal_sex(pc_race_table[ch->race].sex, "F") &&
                                           legal_sex(class_table[ch->class].sex, "F"))
                                                write_to_buffer(d, "{CFEMALE ", 0);
                                        if(legal_sex(pc_race_table[ch->race].sex, "M") &&
                                           legal_sex(class_table[ch->class].sex, "M"))
                                                write_to_buffer(d, "{CMALE ", 0);

                                        if (ch->race == race_lookup("Myrddraal"))
                                          {
                                        if(legal_sex(pc_race_table[ch->race].sex,"N")
                                        && legal_sex(class_table[ch->class].sex,"N"))
                                                write_to_buffer(d, "{CNEUTRAL ",0);
                                          }
                                        write_to_buffer(d, "\n\r", 0);
                                        write_to_buffer(d, "\n\r{cChoose your {CSEX{w:{W ", 0);
					return;
				}
			}
			else
			{
                        write_to_buffer(d, "\n\r{R*** {wNOT a {RVALID {CSEX {R***\n\r",0);
                                        write_to_buffer(d, "{cYour SEX may be{w: ", 0);  
                                        if(legal_sex(pc_race_table[ch->race].sex, "F") &&
                                           legal_sex(class_table[ch->class].sex, "F"))   
                                                write_to_buffer(d, "{CFEMALE ", 0);      
                                        if(legal_sex(pc_race_table[ch->race].sex, "M") &&
                                           legal_sex(class_table[ch->class].sex, "M"))
                                                write_to_buffer(d, "{CMALE ", 0);
                                           
                                        if (ch->race == race_lookup("Myrddraal"))
                                          {
                                        if(legal_sex(pc_race_table[ch->race].sex,"N")
                                        && legal_sex(class_table[ch->class].sex,"N"))
                                                write_to_buffer(d, "{CNEUTRAL ",0);
                                          }
                                        write_to_buffer(d, "\n\r", 0);
                                        write_to_buffer(d, "\n\r{cChoose your {CSEX{w:{W ", 0);
                                        return;
			}
   write_to_buffer(d, "\n\r\n\r{R*** {WYou are now going to {CROLL {Wyour {CCHARACTER's ATTRIBUTES {R***{x\n\r",0);


		make_stats(ch);

sprintf(buf, "{CSTRENGTH{w: {W%d  {CINTELLIGENCE{w: {W%d  {CWISDOM{w: {W%d  {CDEXTERITY{w: {W%d  {CCONSTITUTION{w:"
             " {W%d \n\r{gKeep this ROLL? {c({GYES{c/{RNO{c){w:{W ",
        			ch->perm_stat[STAT_STR], ch->perm_stat[STAT_INT],
        			ch->perm_stat[STAT_WIS], ch->perm_stat[STAT_DEX],
        			ch->perm_stat[STAT_CON]);
                write_to_buffer(d, "\n\r\n\r", 0);
        	write_to_buffer(d, buf, 0);

        	d->connected = CON_ROLL_STATS;
        	break;

		case CON_ROLL_STATS:
			switch (argument[0] )
   			{
   				case 'y': case 'Y':
     				write_to_buffer( d, "\n\r", 2 );
					group_add(ch,"rom basics",FALSE);
					group_add(ch,class_table[ch->class].base_group,FALSE);
			if (ch->pcdata->learned[gsn_recall] < 50)
			  ch->pcdata->learned[gsn_recall] = 50;

write_to_buffer(d,
"\n\r\n\r\n\r{CCUSTOMIZING {cyour {CCHARACTER {callows you to select the SKILLS and/or SPELLS for your\n\r"
"{CCHARACTER {cfrom lists based on the {CCLASS {cand/or {CRACE {cthat you have selected.  By doing\n\r" 
"{cthis, your CHARACTER will start the game with the SKILLS/SPELLS you want, which means\n\r"
"{cit will start at the {CPOWER {clevel you want.  If you do not wish to do this or if you\n\r"
"{cwant to get more familiar with the {CMUD{c first, just answer {RNO {cto the question below and\n\r"
"{cthe {CMUD {cwill {CASSIGN {cyou the normal SKILL/SPELL defaults for your {CCLASS{c/{CRACE{c.\n\r"
"\n\r{R*** {WCUSTOMIZING a CHARACTER can take alot of time {R***{x\n\r",0);
write_to_buffer(d,"\n\r{cDo you wish to {CCUSTOMIZE {cthis character? {c({GYES{c/{RNO{c){w: ",0);
	d->connected = CON_DEFAULT_CHOICE;
					break;
				case 'n': case 'N':
				make_stats(ch);
sprintf(buf, "{CSTRENGTH{w: {W%d  {CINTELLIGENCE{w: {W%d  {CWISDOM{w: {W%d  {CDEXTERITY{w: {W%d  {CCONSTITUTION{w:"
             " {W%d \n\r{gKeep this ROLL? {c({GYES{c/{RNO{c){w:{W ",
                                ch->perm_stat[STAT_STR], ch->perm_stat[STAT_INT],
                                ch->perm_stat[STAT_WIS], ch->perm_stat[STAT_DEX],
                                ch->perm_stat[STAT_CON]);
                write_to_buffer(d, "\n\r\n\r", 0);
                write_to_buffer(d, buf, 0);
     				return;

   				default: write_to_buffer( d, "\n\r{GYES {cor {RNO{c?{w:  ", 0 );
     				return;
			}
			break;
/*
  !!!!!
  !!!!!
  !!!!!
  LEFT OFF HERE!!!!
  !!!!!
  !!!!!
  !!!!!
*/
// Afternote, Jan 3 2005, what the heck was this comment I made above? heh

case CON_DEFAULT_CHOICE:

    write_to_buffer(d,"\n\r",1);
        switch ( argument[0] )
        {

        case 'y': case 'Y': 
        ch->gen_data = new_gen_data();
        ch->gen_data->points_chosen = ch->pcdata->points;
        do_function(ch, &do_help, "group header");
        list_group_costs(ch);
        write_to_buffer(d,"\n\r{WYou already have the following skills{w:{x",0);
        do_function(ch, &do_skills, "");
        do_function(ch, &do_help, "menu choice");
        d->connected = CON_GEN_GROUPS;
        break;

        case 'n': case 'N': 
        group_add(ch,class_table[ch->class].default_group,TRUE);
            write_to_buffer( d, "\n\r", 2 );
        write_to_buffer(d,
"{cChoose a {CWEAPON {cfrom this list.\n\r"
"{cThe {CWEAPON {cyou choose is the one your {CCHARACTER {cwill begin the {CGAME {cwith\n\r"
"{cAND the corresponding {CWEAPON {cskill will start with a {W40% {CBASE CHANCE{c.{x\n\r\n\r",0);
        buf[0] = '\0';
        for ( i = 0; weapon_table[i].name != NULL; i++)
        if (ch->pcdata->learned[*weapon_table[i].gsn] > 0)
        {
            strcat(buf,weapon_table[i].name);
            strcat(buf," ");
        }
        strcat(buf,"\n\r\n\r{cYour choice? {w:{W ");
        write_to_buffer(d,buf,0);
            d->connected = CON_PICK_WEAPON;
            break;


        default:
            write_to_buffer( d, "{cPlease answer ({GY{c/{RN{c)?{w ", 0 );
            return;
        }
    break;

    case CON_PICK_WEAPON:

    write_to_buffer(d,"\n\r",2);
    weapon = weapon_lookup(argument);
    if (weapon == 0 || ch->pcdata->learned[*weapon_table[weapon].gsn] <= 0)
    {
        write_to_buffer(d,
"\n\r\n\r{R*** {WThat is {RNOT {Wa {RVALID {CWEAPON {Wselection {R***\n\r\n\r"
"{cChoose a {CWEAPON {cfrom this list.\n\r"
"{cThe {CWEAPON {cyou choose is the one your {CCHARACTER {cwill begin the {CGAME {cwith\n\r"
"{cAND the corresponding {CWEAPON {cskill will start with a {W40% {CBASE CHANCE{c.{x\n\r\n\r",0);
buf[0] = '\0';
            for ( i = 0; weapon_table[i].name != NULL; i++)
                if (ch->pcdata->learned[*weapon_table[i].gsn] > 0)
                {
                    strcat(buf,weapon_table[i].name);
            strcat(buf," ");
                }
            strcat(buf,"\n\r{cYour choice? {w:{W ");
            write_to_buffer(d,buf,0);
        return;
    }
    
    // LEGEND SYSTEM
    if(ch->pcdata->legend!=-1)
        legend_weapon_picked(ch, weapon);

    if (ch->pcdata->learned[*weapon_table[weapon].gsn] < 40)
      ch->pcdata->learned[*weapon_table[weapon].gsn] = 40;
    write_to_buffer(d,"\n\r",2);
    do_function(ch, &do_help, "motd");
    d->connected = CON_READ_MOTD;
    break;

    case CON_GEN_GROUPS:

    send_to_char("\n\r",ch);

        if (!str_cmp(argument,"done"))
        {

        if (ch->pcdata->points < 40 + pc_race_table[ch->race].points)
        {
        sprintf(buf,
            "\n\r{RYou must take at least {W%d {Rpoints of skills and groups{x",
            40 + pc_race_table[ch->race].points);
        send_to_char(buf, ch);
        break;
        }

        sprintf(buf,"{RCreation points{D: {W%d{x\n\r",ch->pcdata->points);
        send_to_char(buf,ch);
        sprintf(buf,"{RExperience per level{D: {W%d{x\n\r",
                exp_per_level(ch,ch->gen_data->points_chosen));
        if (ch->pcdata->points < 40 + pc_race_table[ch->race].points)
         ch->train = ((40 + pc_race_table[ch->race].points) - ch->pcdata->points + 1) / 2;
        free_gen_data(ch->gen_data);
        ch->gen_data = NULL;
        send_to_char(buf,ch);
            write_to_buffer( d, "\n\r", 2 );
        write_to_buffer(d,
"{cChoose a {CWEAPON {cfrom this list.\n\r"
"{cThe {CWEAPON {cyou choose is the one your {CCHARACTER {cwill begin the {CGAME {cwith\n\r"
"{cAND the corresponding {CWEAPON {cskill will start with a {W40% {CBASE CHANCE{c.{x\n\r\n\r",0);
            buf[0] = '\0';
            for ( i = 0; weapon_table[i].name != NULL; i++)
                if (ch->pcdata->learned[*weapon_table[i].gsn] > 0)
                {
                    strcat(buf,weapon_table[i].name);
            strcat(buf," ");
                }
            strcat(buf,"\n\r{cYour choice?{w ");
            write_to_buffer(d,buf,0);
            d->connected = CON_PICK_WEAPON;
            break;
        }

        if (!parse_gen_groups(ch,argument))
        send_to_char(
        "{RChoices are{D: {Wlist{R,{Wlearned{R,{Wpremise{R,{Wadd{R,{Wdrop{R,{Winfo{R,{Whelp{R and {Wdone{R.{x\n\r"
        ,ch);

        do_function(ch, &do_help, "menu choice");
        break;

    case CON_READ_IMOTD:
    write_to_buffer(d,"\n\r",2);
        do_function(ch, &do_help, "motd");
        d->connected = CON_READ_MOTD;
    break;

        /* states for new note system, (c)1995-96 erwin@andreasen.org */
        /* ch MUST be PC here; have nwrite check for PC status! */

    case CON_NOTE_TO:
        handle_con_note_to (d, argument);
        break;

    case CON_NOTE_SUBJECT:
        handle_con_note_subject (d, argument);
        break; /* subject */

    case CON_NOTE_EXPIRE:
        handle_con_note_expire (d, argument);
        break;

    case CON_NOTE_TEXT:
        handle_con_note_text (d, argument);
        break;

    case CON_NOTE_FINISH:
        handle_con_note_finish (d, argument);
        break;


    case CON_READ_MOTD:        
        if ( ch->pcdata == NULL || ch->pcdata->pwd[0] == '\0')
        {
            write_to_buffer( d, "Warning! Null password!\n\r",0 );
            write_to_buffer( d, "Please report old password with bug.\n\r",0);
            write_to_buffer( d,
                "Type 'password null <new password>' to fix.\n\r",0);
        }


if(!IS_IMP(ch))
  {

 write_to_buffer( d, "\n\rWelcome to ROM 2.4.  Please do not feed the mobiles.\n\r",0 );

  }

    if (ch->act != 0)
      {
       ch->pact = ch->act; 
       ch->act = 0;
      }

    ch->next    = char_list;
    char_list   = ch;
    d->connected    = CON_PLAYING;
    
    reset_char(ch);


    if(ch->pcdata->balance >= 1)
      {
       ch->gold += ch->pcdata->balance;
       ch->pcdata->balance = 0;
      }

    /*if ((ch->alignment != 1000)
    &&  (ch->alignment != 0)
    &&  (ch->alignment != -1000))
      {
       if (ch->alignment >= 350)
         {
          ch->alignment = 1000;
         }
       else
        if (ch->alignment <= -350)
          {
           ch->alignment = -1000;
          }
       else
        if (ch->alignment <= 349 && ch->alignment >= -349)
          {
           ch->alignment = 0;
          }
      }*/

    if (IS_SET(ch->pact, PLR_DRAGONPIT))
      { REMOVE_BIT(ch->pact, PLR_DRAGONPIT); }

if ( ch->pcdata->remorting )
    {
        ch->perm_stat[class_table[ch->class].attr_prime] += 3;
        SET_BIT( ch->deaf, CHANNEL_VENT );
        ch->pcdata->previousHCLevel = 0;
        ch->level     = 1;
        ch->oldlvl    = ch->level;
        ch->exp       = exp_per_level(ch,ch->pcdata->points);
        ch->max_hit   = 30;
        ch->max_mana  = 150;
        ch->max_move  = 150;
	ch->pcdata->condition[COND_THIRST]  = -1;
	ch->pcdata->condition[COND_HUNGER]  = -1;
        ch->hit       = ch->max_hit;
        ch->mana      = ch->max_mana;
        ch->move      = ch->max_move;
        ch->pcdata->perm_hit       = ch->max_hit;
        ch->pcdata->perm_mana      = ch->max_mana;
        ch->pcdata->perm_move      = ch->max_move;
        ch->train     = 6;
  	ch->practice  = 10;
	ch->nextquest = 15;
	set_title( ch,"says 'I have not used the TITLE command yet!'{x"  );

        char_to_room( ch, get_room_index( ROOM_VNUM_SCHOOL ) );
        send_to_char("\n\r",ch);
        send_to_char("\n\r",ch);
	do_outfit(ch, "");
        do_dbag (ch, "self");
    }
  else if (ch->pcdata->legending )
  {
        legend_logging_in_first_time(ch);
  }
  else if ( ch->level == 0 )
    {
        if (ch->pcdata->points < 40 + pc_race_table[ch->race].points)
          {
           ch->pcdata->points = (40 + pc_race_table[ch->race].points);
          }

        ch->perm_stat[class_table[ch->class].attr_prime] += 3;
        SET_BIT( ch->deaf, CHANNEL_VENT );
	SET_BIT( ch->deaf, CHANNEL_ACRO );
        ch->level    = 1;
        ch->oldlvl   = ch->level;
        ch->exp = exp_per_level(ch,ch->pcdata->points);
        ch->hit = ch->max_hit;
        ch->mana    = ch->max_mana;
        ch->move    = ch->max_move;
        ch->pcdata->oldcl = -1;
        ch->train    = 3;
        ch->practice = 4;
        if ( ch->race == RACE_GOLEM
        ||   ch->race == RACE_MYRDDRAAL )
          {
           ch->pcdata->condition[COND_THIRST]  = -1;
           ch->pcdata->condition[COND_HUNGER]  = -1;
          }
        set_title( ch,"says 'I have not used the TITLE command yet!'{x"  );

        char_to_room( ch, get_room_index( ROOM_VNUM_SCHOOL ) );
        send_to_char("\n\r",ch);
        do_function(ch, &do_help, "newbie info");
        send_to_char("\n\r",ch);
	do_outfit(ch, "");
        do_dbag (ch, "self");
        
        sprintf( buf, "%s%s", PLAYER_DIR, capitalize( ch->name ) );
        if(file_exists(buf))
        {
            send_to_char("\n\r{RI'm sorry but a player with that name already exists in the game world.{x\n\r", ch);
            do_quit(ch, "");
            return;
        }
        
        save_char_obj(ch);
    }
    else if ( ch->in_room != NULL )
    {
        char_to_room( ch, ch->in_room );
    }
    else if ( IS_IMMORTAL(ch) )
    {
        char_to_room( ch, get_room_index( ROOM_VNUM_CHAT ) );
    }
    else
    {
        char_to_room( ch, get_room_index( ROOM_VNUM_TEMPLE ) );
    }
   
  if (ch->clan > 0 )
    {
     CLAN_DATA *pClan = get_clan_index(ch->clan);
     MEMBER_DATA *pMember = NULL;
     if ( !is_active_member( ch, pClan))
       {
        if ( !(pMember = fread_pkinfo( ch->name)))
          {
           pMember = new_member ( );
           pMember->name = str_dup(ch->name);
          }
         sort_members(pClan, pMember );
       }
     else
       pMember = get_member_clan(ch, pClan );
     ch->pcdata->member = pMember;
    }
  
  if(ch->invis_level <= 100 && ch->incog_level <= 100)
  {
    sprintf(buf, "{G%s {Whas given up on reality, joining the ranks of the {rPhoenix.{x\n\r", ch->name);
	for(d=descriptor_list;d;d=d->next)
	{
		original = d->original ? d->original : d->character; /* if switched */
		if(d->connected==CON_PLAYING)
		  act_new(buf, original, NULL, NULL, TO_CHAR, POS_DEAD);
	}
  }

  if(ch->oldlvl <= 0)
    ch->oldlvl = ch->level;

  if (!IS_IMP(ch))
    {
     ch->who_name = ch->name;
    }

     act( "$n has entered the game.", ch, NULL, NULL, TO_ROOM );
    do_function(ch, &do_look, "auto" );
        if (ch->invis_level < LEVEL_HERO)
        act( "{c$n has entered the dragon!{x", ch, NULL, NULL, TO_ROOM );
      wiznet("$N has left real life behind.",ch,NULL,WIZ_LOGINS,WIZ_SITES,ch->level); 

    if (ch->pet != NULL)
    {
        char_to_room(ch->pet,ch->in_room);
        act("$n has entered the game.",ch->pet,NULL,NULL,TO_ROOM);
    }
        crn_players++;
        crm_players = UMAX( crn_players, crm_players );
        if ( crm_players > atm_players )
          {
          char temp[30];
          strcpy( temp, (char *)ctime( &current_time ) );
          *(temp+strlen(temp)-1) = '\0'; /* remove the dumb \n */
          atm_players = crm_players;
          free_string( date_atmp );
          date_atmp   = str_dup( temp );
          save_maxes( );
          }

	send_to_char("\n", ch);
    if(!ch->pcdata->board)
        ch->pcdata->board = &boards[DEFAULT_BOARD];
   	do_board(ch, 0);  /* Show board status */

	
    if ( ch->pcdata->remorting )
      {
       ch->pcdata->remorting = FALSE;
       SET_BIT(ch->pact, PLR_REMORT);
       save_char_obj( ch );
      }
    
    // LEGEND SYSTEM
    if ( ch->pcdata->legending )
      {
       ch->pcdata->legending = FALSE;
       save_char_obj( ch );
      }
    break;

    case CON_VISIBILITY:
        switch ( argument[0] )
        {

        case 'w': case 'W':
            ch->invis_level = ch->level;
            write_to_buffer(d, "{C\n\rYou slowly vanish into thin air.\n\r", 0);
            break;
        case 'i': case 'I':
            ch->incog_level = ch->level;
            write_to_buffer(d, "{D\n\rYou cloak your presence.\n\r", 0);
            break;
        case 'b': case 'B':
            ch->invis_level = ch->level;
            ch->incog_level = ch->level;
            write_to_buffer(d, "{C\n\rYou slowly vanish into thin air.\n\r", 0);
            write_to_buffer(d, "{D\n\rYou cloak your presence.\n\r", 0);
            break;
        case 'v': case 'V':
            ch->invis_level = 0;
            ch->incog_level = 0;
            write_to_buffer(d, "{W\n\rYou are completely visible now.\n\r", 0);
            break;
        default:
            break;
        }
    d->connected = CON_READ_MOTD;
    break;
        
    }

    return;
}

/*
 * Parse a name for acceptability.
 */
bool check_parse_name( char *name )
{
    /*
     * Reserved words.
     */
    if (is_exact_name(name,
    "all auto immortal self someone something the you loner none dragon"))
    {
    return FALSE;
    }

    if (str_cmp(capitalize(name),"Alander") && (!str_prefix("Alan",name)
    || !str_suffix("Alander",name)))
    return FALSE;

    /*
     * Length restrictions.
     */
     
    if ( strlen(name) <  2 )
    return FALSE;

#if defined(MSDOS)
    if ( strlen(name) >  8 )
    return FALSE;
#endif

#if defined(macintosh) || defined(unix)
    if ( strlen(name) > 12 )
    return FALSE;
#endif

    /*
     * Alphanumerics only.
     * Lock out IllIll twits.
     */
    {
    char *pc;
    bool fIll,adjcaps = FALSE,cleancaps = FALSE;
    int total_caps = 0;

    fIll = TRUE;
    for ( pc = name; *pc != '\0'; pc++ )
    {
        if ( !isalpha(*pc) )
        return FALSE;

        if ( isupper(*pc)) /* ugly anti-caps hack */
        {
        if (adjcaps)
            cleancaps = TRUE;
        total_caps++;
        adjcaps = TRUE;
        }
        else
        adjcaps = FALSE;

        if ( LOWER(*pc) != 'i' && LOWER(*pc) != 'l' )
        fIll = FALSE;
    }

    if ( fIll )
        return FALSE;

    if (cleancaps || (total_caps > (strlen(name)) / 2 && strlen(name) < 3))
        return FALSE;
    }

    /*
     * Prevent players from naming themselves after mobs.
     */
    {
    extern MOB_INDEX_DATA *mob_index_hash[MAX_KEY_HASH];
    MOB_INDEX_DATA *pMobIndex;
    int iHash;

    for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    {
        for ( pMobIndex  = mob_index_hash[iHash];
          pMobIndex != NULL;
          pMobIndex  = pMobIndex->next )
        {
        if ( is_name( name, pMobIndex->player_name ) )
            return FALSE;
        }
    }
    }

    return TRUE;
}



/*
 * Look for link-dead player to reconnect.
 */
bool check_reconnect( DESCRIPTOR_DATA *d, char *name, bool fConn )
{
    CHAR_DATA *ch;

    for ( ch = char_list; ch != NULL; ch = ch->next )
    {
    if ( !IS_NPC(ch)
    &&   (!fConn || ch->desc == NULL)
    &&   !str_cmp( d->character->name, ch->name ) )
    {
        if ( fConn == FALSE )
        {
        free_string( d->character->pcdata->pwd );
        d->character->pcdata->pwd = str_dup( ch->pcdata->pwd );
        }
        else
        {
        free_char( d->character );
        d->character = ch;
        ch->desc     = d;
        ch->timer    = 0;
        send_to_char(
            "Reconnecting. Type replay to see missed tells.\n\r", ch );
        act( "$n has reconnected.", ch, NULL, NULL, TO_ROOM );

       sprintf( log_buf, "%s@%s reconnected.", ch->name, d->host );
        log_string( log_buf );
       wiznet("$N groks the fullness of their link.",
            ch,NULL,WIZ_LINKS,0,ch->level); 
        d->connected = CON_PLAYING;
		/* Inform the character of a note in progress and the possbility
		 * of continuation!
		 */
		if (ch->pcdata->in_progress)
		    send_to_char ("You have a note in progress. Type NWRITE to continue it.\n\r", ch);

        }
        return TRUE;
    }
    }

    return FALSE;
}



/*
 * Check if already playing.
 */
bool check_playing( DESCRIPTOR_DATA *d, char *name )
{
    DESCRIPTOR_DATA *dold;

    for ( dold = descriptor_list; dold; dold = dold->next )
    {
    if ( dold != d
    &&   dold->character != NULL
    &&   dold->connected != CON_GET_NAME
    &&   dold->connected != CON_GET_OLD_PASSWORD
    &&   !str_cmp( name, dold->original
             ? dold->original->name : dold->character->name ) )
    {
        write_to_buffer( d, "That character is already playing.\n\r",0);
        write_to_buffer( d, "Do you wish to connect anyway (Y/N)?",0);
        d->connected = CON_BREAK_CONNECT;
        return TRUE;
    }
    }

    return FALSE;
}



void stop_idling( CHAR_DATA *ch )
{
    if ( ch == NULL
    ||   ch->desc == NULL
    ||   ch->desc->connected != CON_PLAYING
    ||   ch->was_in_room == NULL 
    ||   ch->in_room != get_room_index(ROOM_VNUM_LIMBO))
    return;

    ch->timer = 0;
    char_from_room( ch );
    char_to_room( ch, ch->was_in_room );
    ch->was_in_room = NULL;
    act( "$n has returned from the void.", ch, NULL, NULL, TO_ROOM );
    return;
}



/*
 * Write to one char.
 */
void send_to_char_bw( const char *txt, CHAR_DATA *ch )
{
    if ( txt != NULL && ch->desc != NULL )
        write_to_buffer( ch->desc, txt, strlen(txt) );
    return;
}

/*
 * Write to one char, new colour version, by Lope.
 */
void send_to_char( const char *txt, CHAR_DATA *ch )
{
    const   char    *point;
            char    *point2;
            char    buf[ MAX_STRING_LENGTH*4 ];
        int skip = 0;

    buf[0] = '\0';
    point2 = buf;
    if( txt && ch->desc )
    {
        if( IS_SET( ch->pact, PLR_COLOUR ) )
        {
        for( point = txt ; *point ; point++ )
            {
            if( *point == '{' )
            {
            point++;
                skip = colour( *point, ch, point2 );
            while( skip-- > 0 )
                ++point2;
            continue;
            }
            *point2 = *point;
            *++point2 = '\0';
        }           
        *point2 = '\0';
            write_to_buffer( ch->desc, buf, point2 - buf );
        }
        else
        {
        for( point = txt ; *point ; point++ )
            {
            if( *point == '{' )
            {
            point++;
            continue;
            }
            *point2 = *point;
            *++point2 = '\0';
        }
        *point2 = '\0';
            write_to_buffer( ch->desc, buf, point2 - buf );
        }
    }
    return;
}



/*
 * Send a page to one char.
 */
void page_to_char_bw( const char *txt, CHAR_DATA *ch )
{
    if ( txt == NULL || ch->desc == NULL)
    return;

    if (ch->lines == 0 )
    {
    send_to_char(txt,ch);
    return;
    }
    
#if defined(macintosh)
    send_to_char(txt,ch);
#else
    ch->desc->showstr_head = alloc_mem(strlen(txt) + 1);
    strcpy(ch->desc->showstr_head,txt);
    ch->desc->showstr_point = ch->desc->showstr_head;
    show_string(ch->desc,"");
#endif
}

/*
 * Page to one char, new colour version, by Lope.
 */
void page_to_char( const char *txt, CHAR_DATA *ch )
{
    const   char    *point;
            char    *point2;
            char    buf[ MAX_STRING_LENGTH * 4 ];
        int skip = 0;

    buf[0] = '\0';
    point2 = buf;
    if( txt && ch->desc )
    {
        if( IS_SET( ch->pact, PLR_COLOUR ) )
        {
        for( point = txt ; *point ; point++ )
            {
            if( *point == '{' )
            {
            point++;
                skip = colour( *point, ch, point2 );
            while( skip-- > 0 )
                ++point2;
            continue;
            }
            *point2 = *point;
            *++point2 = '\0';
        }           
        *point2 = '\0';
        ch->desc->showstr_head  = alloc_mem( strlen( buf ) + 1 );
        strcpy( ch->desc->showstr_head, buf );
        ch->desc->showstr_point = ch->desc->showstr_head;
        show_string( ch->desc, "" );
        }
        else
        {
        for( point = txt ; *point ; point++ )
            {
            if( *point == '{' )
            {
            point++;
            continue;
            }
            *point2 = *point;
            *++point2 = '\0';
        }
        *point2 = '\0';
        ch->desc->showstr_head  = alloc_mem( strlen( buf ) + 1 );
        strcpy( ch->desc->showstr_head, buf );
        ch->desc->showstr_point = ch->desc->showstr_head;
        show_string( ch->desc, "" );
        }
    }
    return;
}


/* string pager */
void show_string(struct descriptor_data *d, char *input)
{
    char buffer[4*MAX_STRING_LENGTH];
    char buf[MAX_INPUT_LENGTH];
    register char *scan, *chk;
    int lines = 0, toggle = 1;
    int show_lines;

    one_argument(input,buf);
    if (buf[0] != '\0')
    {
    if (d->showstr_head)
    {
        free_mem(d->showstr_head,strlen(d->showstr_head));
        d->showstr_head = 0;
    }
        d->showstr_point  = 0;
    return;
    }

    if (d->character)
    show_lines = d->character->lines;
    else
    show_lines = 0;

    for (scan = buffer; ; scan++, d->showstr_point++)
    {
    if (((*scan = *d->showstr_point) == '\n' || *scan == '\r')
        && (toggle = -toggle) < 0)
        lines++;

    else if (!*scan || (show_lines > 0 && lines >= show_lines))
    {
        *scan = '\0';
        write_to_buffer(d,buffer,strlen(buffer));
        for (chk = d->showstr_point; isspace(*chk); chk++);
        {
        if (!*chk)
        {
            if (d->showstr_head)
                {
                    free_mem(d->showstr_head,strlen(d->showstr_head));
                    d->showstr_head = 0;
                }
                d->showstr_point  = 0;
            }
        }
        return;
    }
    }
    return;
}
    

/* quick sex fixer */
void fix_sex(CHAR_DATA *ch)
{
    if (ch->sex < 0 || ch->sex > 2)
        ch->sex = IS_NPC(ch) ? 0 : ch->pcdata->true_sex;
}

void act_new( const char *format, CHAR_DATA *ch, const void *arg1, const void *arg2, int type, int min_pos)
{
    static char * const he_she  [] = { "it",  "he",  "she" };
    static char * const him_her [] = { "it",  "him", "her" };
    static char * const his_her [] = { "its", "his", "her" };
 
    char legendText[MAX_STRING_LENGTH];
     CHAR_DATA                 *to;
     CHAR_DATA                 *vch = ( CHAR_DATA * ) arg2;
     OBJ_DATA          *obj1 = ( OBJ_DATA  * ) arg1;
     OBJ_DATA          *obj2 = ( OBJ_DATA  * ) arg2;
     const     char    *str;
     char              *i = NULL;
     char              *point;
     char              buf[ MAX_STRING_LENGTH   ];
     char              fname[ MAX_INPUT_LENGTH  ];
     int               j=0;
 
    /*
     * Discard null and zero-length messages.
     */
    if ( !format|| format[0] == '\0' )
        return;

    /* discard null rooms and chars */
    if(!ch || !ch->in_room)
    return;

    to = ch->in_room->people;
    if ( type == TO_VICT )
    {
        if ( !vch)
        {
            bug( "Act: null vch with TO_VICT.", 0 );
            return;
        }

    if (!vch->in_room)
        return;

        to = vch->in_room->people;
    }
 
    for ( ; to ; to = to->next_in_room )
    {
        if ( (to->desc == NULL
        &&    (!IS_NPC(to) || !HAS_TRIGGER(to, TRIG_ACT)))
        ||   to->position < min_pos )
            continue;

  	if(is_ignoring(to, ch))
	    continue;
	    
/*
    if ( (!IS_NPC(to) &&
!to->desc)
    ||   ( IS_NPC(to) && !HAS_TRIGGER(to, TRIG_ACT) )
    ||    to->position < min_pos )
            continue;
 */
        if ( (type == TO_CHAR) && to != ch )
            continue;
        if ( type == TO_VICT && ( to != vch || to == ch ) )
            continue;
        if ( type == TO_ROOM && to == ch )
            continue;
        if ( type == TO_NOTVICT && (to == ch || to == vch) )
            continue;
 
        point   = buf;
        str     = format;
        while ( *str != '\0' )
        {
            if ( *str != '$' )
            {
                *point++ = *str++;
                continue;
            }
           ++str;

           i = " <@@@> ";
 
            if ( arg2 == NULL && *str >= 'A' && *str <= 'Z' )
            {
                bug( "Act: missing arg2 for code %d.", *str );
                i = " <@@@> ";
            }
            else
            {
                switch ( *str )
                {
                default:  bug( "Act: bad code %d.", *str );
                          i = " <@@@> ";                                break;
                /* Thx alex for 't' idea */
                case 't':
                     if((IS_LEGEND(to) && !legend_has_race_chat(to, ch)) ||
                        (IS_LEGEND(ch) && !legend_has_race_chat(ch, to)))
                     {
                        legend_garble_text((char *)arg1, legendText);
                        i = (char *)legendText;
                     }
                     else
                        i = (char *) arg1;
                    break;
                case 'T':
                     if((IS_LEGEND(to) && !legend_has_race_chat(to, ch)) ||
                        (IS_LEGEND(ch) && !legend_has_race_chat(ch, to)))
                     {
                        legend_garble_text((char *)arg2, legendText);
                        i = (char *)legendText;
                     }
                     else
                        i = (char *) arg2;
                    break;
                case 'n': i = PERS( ch,  to  );                         break;
                case 'N': i = PERS( vch, to  );                         break;
                case 'e': i = he_she  [URANGE(0, ch  ->sex, 2)];        break;
                case 'E': i = he_she  [URANGE(0, vch ->sex, 2)];        break;
                case 'm': i = him_her [URANGE(0, ch  ->sex, 2)];        break;
                case 'M': i = him_her [URANGE(0, vch ->sex, 2)];        break;
                case 's': i = his_her [URANGE(0, ch  ->sex, 2)];        break;
                case 'S': i = his_her [URANGE(0, vch ->sex, 2)];        break;
 
                case 'p':
                    i = can_see_obj( to, obj1 )
                            ? obj1->short_descr
                            : "something";
		    if(!i)
			i = "something";
                    break;
 
                case 'P':
                    i = can_see_obj( to, obj2 )
                            ? obj2->short_descr
                            : "something";
                    if(!i)
                        i = "something";
                    break;
 
                case 'd':
                    if ( arg2 == NULL || ((char *) arg2)[0] == '\0' )
                    {
                        i = "door";
                    }
                    else
                    {
                        one_argument( (char *) arg2, fname );
                        i = fname;
                    }
                    break;
                }
            }
 
            ++str;
            while ( ( *point = *i ) != '\0' )
                ++point, ++i;
        }
 
        *point++ = '\n';
        *point++ = '\r';
        *point   = '\0';
        
        j = 0;

        while ( (buf[j] < 'a' && buf[j] > 'Z') ||
                (buf[j] < 'A') || (buf[j] > 'z') )
        {
            if (buf[j] == '\0')
                break;

            if (buf[j] == '{')
                j++;

            j++;

        }

        buf[j]   = UPPER(buf[j]);


    if ( to->desc != NULL )
    {
        if (to->desc && (to->desc->connected == CON_PLAYING))
         write_to_buffer( to->desc, buf, 0 );
    }       /* write_to_buffer( to->desc, buf, point - buf );*/ /* add later */
    else
    if ( MOBtrigger )
        mp_act_trigger( buf, to, ch, arg1, arg2, TRIG_ACT );
    }
    return;
}



/*
 * Macintosh support functions.
 */
#if defined(macintosh)
int gettimeofday( struct timeval *tp, void *tzp )
{
    tp->tv_sec  = time( NULL );
    tp->tv_usec = 0;
}
#endif

void logf2 (char * fmt, ...)
{
 	char buf [2*MSL];
 	va_list args;
 	va_start (args, fmt);
 	vsprintf (buf, fmt, args);
 	va_end (args);
 	
 	log_string (buf);
}

int colour( char type, CHAR_DATA *ch, char *string )
{
    char    code[ 20 ];
    char    *p = '\0';
    if( ch && IS_NPC( ch ) )
        return( 0 );

    switch( type )
    {
    default:
        sprintf( code, CLEAR );
        break;
    case 'x':
        sprintf( code, CLEAR );
        break;
        
    case '!':
        sprintf( code, CODE_BOLD );
        break;
    case '^':
        sprintf( code, CODE_BLINK );
        break;
    case '&':
        sprintf( code, CODE_REVERSE );
        break;
    case '_':
        sprintf( code, CODE_UNDERLINE );
        break;
    case '$':
        sprintf( code, CODE_DIM );
        break;
    case '%':
        sprintf( code, CODE_HIDDEN );
        break;        
    case '0':
        sprintf( code, C_BACK_BLACK );
        break;        
    case '1':
        sprintf( code, C_BACK_RED );
        break;        
    case '2':
        sprintf( code, C_BACK_GREEN );
        break;        
    case '3':
        sprintf( code, C_BACK_BLUE );
        break;        
    case '4':
        sprintf( code, C_BACK_CYAN );
        break;        
    case '5':
        sprintf( code, C_BACK_MAGENTA );
        break;        
    case '6':
        sprintf( code, C_BACK_YELLOW );
        break;        
    case '7':
        sprintf( code, C_BACK_WHITE );
        break;                       
        
    case 'b':
        sprintf( code, C_BLUE );
        break;
    case 'c':
        sprintf( code, C_CYAN );
        break;
    case 'g':
        sprintf( code, C_GREEN );
        break;
    case 'm':
        sprintf( code, C_MAGENTA );
        break;
    case 'r':
        sprintf( code, C_RED );
        break;
    case 'w':
        sprintf( code, C_WHITE );
        break;
    case 'y':
        sprintf( code, C_YELLOW );
        break;
    case 'B':
        sprintf( code, C_B_BLUE );
        break;
    case 'C':
        sprintf( code, C_B_CYAN );
        break;
    case 'G':
        sprintf( code, C_B_GREEN );
        break;
    case 'M':
        sprintf( code, C_B_MAGENTA );
        break;
    case 'R':
        sprintf( code, C_B_RED );
        break;
    case 'W':
        sprintf( code, C_B_WHITE );
        break;
    case 'Y':
        sprintf( code, C_B_YELLOW );
        break;
    case 'D':
        sprintf( code, C_D_GREY );
        break;
    case '\\':
        sprintf( code, "\n\r" );
        break;

    case '*':
        sprintf( code, "%c", 007 );
        break;
    
    case '-':
        sprintf( code, "~");
        break;
/*
    case '/':
        sprintf( code, "%c", 012 );
        break;
*/
    case '{':
        sprintf( code, "%c", '{' );
        break;
    }

    p = code;
    while( *p != '\0' )
    {
    *string = *p++;
    *++string = '\0';
    }

    return( strlen( code ) );
}

void colourconv( char *buffer, const char *txt, DESCRIPTOR_DATA *d )
{
    const   char    *point;
        int skip = 0;

    if( d && txt )
    {
    if( d->fcolour==TRUE )
    {
        for( point = txt ; *point ; point++ )
        {
        if( *point == '{' )
        {
            point++;
                skip = colour( *point, d->character, buffer );
            while( skip-- > 0 )
            ++buffer;
            continue;
        }
        *buffer = *point;
        *++buffer = '\0';
        }           
        *buffer = '\0';
    }
    else
    {
        for( point = txt ; *point ; point++ )
        {
        if( *point == '{' )
        {
            point++;
            continue;
        }
        *buffer = *point;
        *++buffer = '\0';
        }
        *buffer = '\0';
    }
    }
    return;
}


void send_to_all_player( CHAR_DATA *ch, const char *txt )
{
    DESCRIPTOR_DATA *d;

    if ( !txt )
        return;

    for ( d = descriptor_list; d; d = d->next ) 
       {    
        if ( d->connected == CON_PLAYING )
          {
			act_new(txt,d->character,NULL,ch,TO_CHAR,POS_DEAD);
           return;
          }
       }
 return;
} 



bool	legal_class(int race, char * class_name)
{
	char	class[4];
	
	if(race<=0 || class_name[0]=='\0' || class_name[1]=='\0' || class_name[2]=='\0')
		return FALSE;
	
	if(strstr(pc_race_table[race].class, "All"))
		return TRUE;
	
	class[0]=class_name[0];
	class[1]=class_name[1];
	class[2]=class_name[2];
	class[3]='\0';
	
	if(!str_cmp(class, "Ass"))
		class[2]='n';

	if(!str_cmp(class, "Ant"))
	{
		class[1]='p';
		class[2]='l';
	}		
	
	if(!strstr(pc_race_table[race].class, capitalize(class)))
		return FALSE;

	return TRUE;
}

bool	legal_sex(char * sex, char * type)
{
	if(strstr(sex,  "All"))
		return TRUE;
	
	if(strstr(sex, type))
		return TRUE;

	return FALSE;
}

bool	legal_align(char * align, char * type)
{
	if(strstr(align,  "All"))
		return TRUE;
	
	if(strstr(align, type))
		return TRUE;

	return FALSE;
}

void fix_player_text(CHAR_DATA * ch, char * text)
{
    if(!IS_IMM(ch))
    {
        // filter out the special codes
        strstrstrip(text, "{!");
        strstrstrip(text, "{^");
        strstrstrip(text, "{&");
        strstrstrip(text, "{_");
        strstrstrip(text, "{$");
        strstrstrip(text, "{%");
        strstrstrip(text, "{1");
        strstrstrip(text, "{2");
        strstrstrip(text, "{3");
        strstrstrip(text, "{4");
        strstrstrip(text, "{5");
        strstrstrip(text, "{6");
        strstrstrip(text, "{7");
        strstrstrip(text, "{0");
        strstrstrip(text, "{\\");
    }
}

