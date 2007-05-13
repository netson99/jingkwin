/*****************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 1999, 2000 Matthias Ettrich <ettrich@kde.org>
Copyright (C) 2003 Lubos Lunak <l.lunak@kde.org>

You can Freely distribute this program under the GNU General Public
License. See the file "COPYING" for the exact licensing terms.
******************************************************************/

#include "main.h"

//#define QT_CLEAN_NAMESPACE
#include <ksharedconfig.h>

#include <kglobal.h>
#include <klocale.h>
#include <stdlib.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <QX11Info>
#include <stdio.h>
#include <fixx11h.h>
#include <QtDBus/QtDBus>
#include <kglobal.h>

#include "atoms.h"
#include "options.h"
#include "sm.h"
#include "utils.h"
#include "effects.h"

#define INT8 _X11INT8
#define INT32 _X11INT32
#include <X11/Xproto.h>
#undef INT8
#undef INT32

namespace KWin
{

Options* options;

Atoms* atoms;

int screen_number = -1;

static bool initting = false;

static
int x11ErrorHandler(Display *d, XErrorEvent *e)
    {
    char msg[80], req[80], number[80];
    bool ignore_badwindow = true; //maybe temporary

    if (initting &&
        (
         e->request_code == X_ChangeWindowAttributes
         || e->request_code == X_GrabKey
         )
        && (e->error_code == BadAccess))
        {
        fputs(i18n("kwin: it looks like there's already a window manager running. kwin not started.\n").toLocal8Bit(), stderr);
        exit(1);
        }

    if (ignore_badwindow && (e->error_code == BadWindow || e->error_code == BadColor))
        return 0;

    XGetErrorText(d, e->error_code, msg, sizeof(msg));
    sprintf(number, "%d", e->request_code);
    XGetErrorDatabaseText(d, "XRequest", number, "<unknown>", req, sizeof(req));

    fprintf(stderr, "kwin: %s(0x%lx): %s\n", req, e->resourceid, msg);

    if (initting)
        {
        fputs(i18n("kwin: failure during initialization; aborting").toLocal8Bit(), stderr);
        exit(1);
        }
    return 0;
    }

Application::Application( )
: KApplication( ), owner( screen_number )
    {
    KCmdLineArgs* args = KCmdLineArgs::parsedArgs();
    KSharedConfig::Ptr config = KGlobal::config();
    if (!config->isImmutable() && args->isSet("lock"))
        {
#warning this shouldn not be necessary
        //config->setReadOnly(true);
        config->reparseConfiguration();
        }

    if (screen_number == -1)
        screen_number = DefaultScreen(display());

    if( !owner.claim( args->isSet( "replace" ), true ))
        {
        fputs(i18n("kwin: unable to claim manager selection, another wm running? (try using --replace)\n").toLocal8Bit(), stderr);
        ::exit(1);
        }
    connect( &owner, SIGNAL( lostOwnership()), SLOT( lostSelection()));

    // if there was already kwin running, it saved its configuration after loosing the selection -> reread
    config->reparseConfiguration();

    initting = true; // startup....

    // install X11 error handler
    XSetErrorHandler( x11ErrorHandler );

    // check  whether another windowmanager is running
    XSelectInput(display(), rootWindow(), SubstructureRedirectMask  );
    syncX(); // trigger error now

    options = new Options;
    atoms = new Atoms;

    initting = false; // TODO

    // create workspace.
    (void) new Workspace( isSessionRestored() );

    syncX(); // trigger possible errors, there's still a chance to abort

    initting = false; // startup done, we are up and running now.

    XEvent e;
    e.xclient.type = ClientMessage;
    e.xclient.message_type = XInternAtom( display(), "_KDE_SPLASH_PROGRESS", False );
    e.xclient.display = display();
    e.xclient.window = rootWindow();
    e.xclient.format = 8;
    strcpy( e.xclient.data.b, "wm" );
    XSendEvent( display(), rootWindow(), False, SubstructureNotifyMask, &e );
    }

Application::~Application()
    {
    delete Workspace::self();
    if( owner.ownerWindow() != None ) // if there was no --replace (no new WM)
        XSetInputFocus( display(), PointerRoot, RevertToPointerRoot, xTime() );
    delete options;
    delete effects;
    delete atoms;
    }

void Application::lostSelection()
    {
    sendPostedEvents();
    delete Workspace::self();
    // remove windowmanager privileges
    XSelectInput(display(), rootWindow(), PropertyChangeMask );
    quit();
    }

bool Application::x11EventFilter( XEvent *e )
    {
    if ( Workspace::self()->workspaceEvent( e ) )
             return true;
    return KApplication::x11EventFilter( e );
    }

bool Application::notify( QObject* o, QEvent* e )
    {
    if( Workspace::self()->workspaceEvent( e ))
        return true;
    return KApplication::notify( o, e );
    }

static void sighandler(int)
    {
    QApplication::exit();
    }


} // namespace

static const char version[] = "3.0";
static const char description[] = I18N_NOOP( "KDE window manager" );

static KCmdLineOptions args[] =
    {
        { "lock", I18N_NOOP("Disable configuration options"), 0 },
        { "replace", I18N_NOOP("Replace already-running ICCCM2.0-compliant window manager"), 0 },
        KCmdLineLastOption
    };

extern "C"
KDE_EXPORT int kdemain( int argc, char * argv[] )
    {
    bool restored = false;
    for (int arg = 1; arg < argc; arg++)
        {
        if (! qstrcmp(argv[arg], "-session"))
            {
            restored = true;
            break;
            }
        }

    if (! restored)
        {
        // we only do the multihead fork if we are not restored by the session
	// manager, since the session manager will register multiple kwins,
        // one for each screen...
        QByteArray multiHead = getenv("KDE_MULTIHEAD");
        if (multiHead.toLower() == "true")
            {

            Display* dpy = XOpenDisplay( NULL );
            if ( !dpy )
                {
                fprintf(stderr, "%s: FATAL ERROR while trying to open display %s\n",
                        argv[0], XDisplayName(NULL ) );
                exit (1);
                }

            int number_of_screens = ScreenCount( dpy );
            KWin::screen_number = DefaultScreen( dpy );
            int pos; // temporarily needed to reconstruct DISPLAY var if multi-head
            QByteArray display_name = XDisplayString( dpy );
            XCloseDisplay( dpy );
            dpy = 0;

            if ((pos = display_name.lastIndexOf('.')) != -1 )
                display_name.remove(pos,10); // 10 is enough to be sure we removed ".s"

            QString envir;
            if (number_of_screens != 1)
                {
                for (int i = 0; i < number_of_screens; i++ )
                    {
		    // if execution doesn't pass by here, then kwin
		    // acts exactly as previously
                    if ( i != KWin::screen_number && fork() == 0 )
                        {
                        KWin::screen_number = i;
			// break here because we are the child process, we don't
			// want to fork() anymore
                        break;
                        }
                    }
		// in the next statement, display_name shouldn't contain a screen
		//   number. If it had it, it was removed at the "pos" check
                envir.sprintf("DISPLAY=%s.%d", display_name.data(), KWin::screen_number);

                if (putenv( strdup(envir.toAscii())) )
                    {
                    fprintf(stderr,
                            "%s: WARNING: unable to set DISPLAY environment variable\n",
                            argv[0]);
                    perror("putenv()");
                    }
                }
            }
        }

    KAboutData aboutData( "kwin", I18N_NOOP("KWin"),
                          version, description, KAboutData::License_GPL,
                          I18N_NOOP("(c) 1999-2005, The KDE Developers"));
    aboutData.addAuthor("Matthias Ettrich",0, "ettrich@kde.org");
    aboutData.addAuthor("Cristian Tibirna",0, "tibirna@kde.org");
    aboutData.addAuthor("Daniel M. Duley",0, "mosfet@kde.org");
    aboutData.addAuthor("Luboš Luňák", I18N_NOOP( "Maintainer" ), "l.lunak@kde.org");

    KCmdLineArgs::init(argc, argv, &aboutData);
    KCmdLineArgs::addCmdLineOptions( args );

    if (signal(SIGTERM, KWin::sighandler) == SIG_IGN)
        signal(SIGTERM, SIG_IGN);
    if (signal(SIGINT, KWin::sighandler) == SIG_IGN)
        signal(SIGINT, SIG_IGN);
    if (signal(SIGHUP, KWin::sighandler) == SIG_IGN)
        signal(SIGHUP, SIG_IGN);
#ifdef __GNUC__
#warning D-BUS TODO
//    KApplication::disableAutoDcopRegistration();
#endif
    KWin::Application a;
    KWin::SessionManager weAreIndeed;
    KWin::SessionSaveDoneHelper helper;

    fcntl(XConnectionNumber(KWin::display()), F_SETFD, 1);

    QString appname;
    if (KWin::screen_number == 0)
        appname = "org.kde.kwin";
    else
        appname.sprintf("org.kde.kwin-screen-%d", KWin::screen_number);

    QDBusConnection::sessionBus().interface()->registerService( appname, QDBusConnectionInterface::DontQueueService );

    return a.exec();
    }

#include "main.moc"
