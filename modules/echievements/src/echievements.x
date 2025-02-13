/* TO ADD A NEW ECHIEVEMENT:
 *
 * 1) Append new Echievement_Id before ECH(LAST) in e_mod_main.h
 * 2) Add Echievement_Hide_States
 * 3) Add Echievement_Goals
 * 4) Add Echievement_Strings
 * 5) Add Echievement_Descs
 * 6) Add Echievement_Callbacks
 * 7) Add related ECH_INIT and ECH_EH functions in echievements.c
 */

static Etrophy_Trophy_Visibility Echievement_Hide_States[] =
{
   [ECH(SHELF_POSITIONS)] = ETROPHY_TROPHY_STATE_VISIBLE,
   [ECH(NOTHING_ELSE_MATTERS)] = ETROPHY_TROPHY_STATE_VISIBLE,
   [ECH(FEAR_OF_THE_DARK)] = ETROPHY_TROPHY_STATE_VISIBLE,
   [ECH(CAVE_DWELLER)] = ETROPHY_TROPHY_STATE_VISIBLE,
   [ECH(WINDOW_ENTHUSIAST)] = ETROPHY_TROPHY_STATE_VISIBLE,
   [ECH(WINDOW_LOVER)] = ETROPHY_TROPHY_STATE_VISIBLE,
   [ECH(WINDOW_STALKER)] = ETROPHY_TROPHY_STATE_HIDDEN,
   [ECH(WINDOW_MOVER)] = ETROPHY_TROPHY_STATE_VISIBLE,
   [ECH(WINDOW_OCD)] = ETROPHY_TROPHY_STATE_VISIBLE,
   [ECH(MOUSE_RUNNER)] = ETROPHY_TROPHY_STATE_VISIBLE,
   [ECH(MOUSE_MARATHONER)] = ETROPHY_TROPHY_STATE_VISIBLE,
   [ECH(MOUSE_HERO)] = ETROPHY_TROPHY_STATE_HIDDEN,
   [ECH(MIGHTY_MOUSE)] = ETROPHY_TROPHY_STATE_HIDDEN,
   [ECH(WINDOW_HAULER)] = ETROPHY_TROPHY_STATE_VISIBLE,
   [ECH(WINDOW_SLINGER)] = ETROPHY_TROPHY_STATE_VISIBLE,
   [ECH(WINDOW_SHERPA)] = ETROPHY_TROPHY_STATE_VISIBLE,
   [ECH(PHYSICIST)] = ETROPHY_TROPHY_STATE_HIDDEN,
   [ECH(QUICKDRAW)] = ETROPHY_TROPHY_STATE_HIDDEN,
   [ECH(OPAQUE)] = ETROPHY_TROPHY_STATE_HIDDEN,
   [ECH(KEYBOARD_USER)] = ETROPHY_TROPHY_STATE_VISIBLE,
   [ECH(KEYBOARD_TAPPER)] = ETROPHY_TROPHY_STATE_VISIBLE,
   [ECH(KEYBOARD_ABUSER)] = ETROPHY_TROPHY_STATE_VISIBLE,
   [ECH(KEYBOARD_NINJA)] = ETROPHY_TROPHY_STATE_HIDDEN,
   [ECH(CLICKER)] = ETROPHY_TROPHY_STATE_VISIBLE,
   [ECH(SUPER_CLICKER)] = ETROPHY_TROPHY_STATE_VISIBLE,
   [ECH(SUPER_DUPER_CLICKER)] = ETROPHY_TROPHY_STATE_VISIBLE,
   [ECH(CLICK_MANIAC)] = ETROPHY_TROPHY_STATE_HIDDEN,
   [ECH(SLEEPER)] = ETROPHY_TROPHY_STATE_HIDDEN,
   [ECH(HALTER)] = ETROPHY_TROPHY_STATE_HIDDEN,
   [ECH(EDGY)] = ETROPHY_TROPHY_STATE_VISIBLE,
   [ECH(EDGIER)] = ETROPHY_TROPHY_STATE_HIDDEN,
   [ECH(EDGIEST)] = ETROPHY_TROPHY_STATE_HIDDEN,
   [ECH(EDGAR)] = ETROPHY_TROPHY_STATE_HIDDEN,
   [ECH(WHEELY)] = ETROPHY_TROPHY_STATE_VISIBLE,
   [ECH(ROLLIN)] = ETROPHY_TROPHY_STATE_HIDDEN,
   [ECH(ROLY_POLY)] = ETROPHY_TROPHY_STATE_HIDDEN,
   [ECH(SIGNALLER)] = ETROPHY_TROPHY_STATE_VISIBLE,
   [ECH(SIGNAL_CONTROLLER)] = ETROPHY_TROPHY_STATE_HIDDEN,
   [ECH(SIGNAL_MASTER)] = ETROPHY_TROPHY_STATE_HIDDEN,
   [ECH(SIGURD)] = ETROPHY_TROPHY_STATE_HIDDEN,
   [ECH(NOT_SO_INCOGNITO)] = ETROPHY_TROPHY_STATE_HIDDEN,
   [ECH(DUALIST)] = ETROPHY_TROPHY_STATE_HIDDEN,
   [ECH(THRICE)] = ETROPHY_TROPHY_STATE_HIDDEN,
   [ECH(GOING_HD)] = ETROPHY_TROPHY_STATE_HIDDEN,
   [ECH(REAL_ESTATE_MOGUL)] = ETROPHY_TROPHY_STATE_HIDDEN,
   [ECH(MAXIMUM_DEFINITION)] = ETROPHY_TROPHY_STATE_HIDDEN,
   [ECH(TERMINOLOGIST)] = ETROPHY_TROPHY_STATE_HIDDEN,
   [ECH(LIFE_ON_THE_EDGE)] = ETROPHY_TROPHY_STATE_HIDDEN,
   [ECH(BILINGUAL)] = ETROPHY_TROPHY_STATE_HIDDEN,
   [ECH(POLYGLOT)] = ETROPHY_TROPHY_STATE_HIDDEN,
   [ECH(GADGETEER)] = ETROPHY_TROPHY_STATE_VISIBLE,
   [ECH(INSPECTOR_GADGET)] = ETROPHY_TROPHY_STATE_HIDDEN,
   [ECH(GADGET_HACKWRENCH)] = ETROPHY_TROPHY_STATE_HIDDEN,
   [ECH(SECURITY_CONSCIOUS)] = ETROPHY_TROPHY_STATE_VISIBLE,
   [ECH(SECURITY_NUT)] = ETROPHY_TROPHY_STATE_HIDDEN,
   [ECH(CHIEF_OF_SECURITY)] = ETROPHY_TROPHY_STATE_HIDDEN,
   [ECH(TILED)] = ETROPHY_TROPHY_STATE_HIDDEN,
   [ECH(PERSISTENT)] = ETROPHY_TROPHY_STATE_HIDDEN,
   [ECH(NEVER_GONNA_GIVE_YOU_UP)] = ETROPHY_TROPHY_STATE_HIDDEN,
};

static unsigned int Echievement_Goals[] =
{
                                      /* LAST - first possible shelf orient */
   [ECH(SHELF_POSITIONS)] = E_GADCON_ORIENT_LAST - E_GADCON_ORIENT_LEFT,
   [ECH(NOTHING_ELSE_MATTERS)] = 1,
   [ECH(FEAR_OF_THE_DARK)] = 1,
   [ECH(CAVE_DWELLER)] = 1,
   [ECH(WINDOW_ENTHUSIAST)] = 100,
   [ECH(WINDOW_LOVER)] = 1000,
   [ECH(WINDOW_STALKER)] = 10000,
   [ECH(WINDOW_MOVER)] = 50,
   [ECH(WINDOW_OCD)] = 200,
   [ECH(MOUSE_RUNNER)] = 250000,
   [ECH(MOUSE_MARATHONER)] = 1000000,
   [ECH(MOUSE_HERO)] = 10000000,
   [ECH(MIGHTY_MOUSE)] = 100000000,
   [ECH(WINDOW_HAULER)] = 250000,
   [ECH(WINDOW_SLINGER)] = 1000000,
   [ECH(WINDOW_SHERPA)] = 10000000,
   [ECH(PHYSICIST)] = 1,
   [ECH(QUICKDRAW)] = 1,
   [ECH(OPAQUE)] = 1,
   [ECH(KEYBOARD_USER)] = 5,
   [ECH(KEYBOARD_TAPPER)] = 10,
   [ECH(KEYBOARD_ABUSER)] = 20,
   [ECH(KEYBOARD_NINJA)] = 30,
   [ECH(CLICKER)] = 5,
   [ECH(SUPER_CLICKER)] = 10,
   [ECH(SUPER_DUPER_CLICKER)] = 20,
   [ECH(CLICK_MANIAC)] = 30,
   [ECH(SLEEPER)] = 2,
   [ECH(HALTER)] = 5,
   [ECH(EDGY)] = 2,
   [ECH(EDGIER)] = 4,
   [ECH(EDGIEST)] = 6,
   [ECH(EDGAR)] = 8,
   [ECH(WHEELY)] = 3,
   [ECH(ROLLIN)] = 6,
   [ECH(ROLY_POLY)] = 10,
   [ECH(SIGNALLER)] = 5,
   [ECH(SIGNAL_CONTROLLER)] = 10,
   [ECH(SIGNAL_MASTER)] = 20,
   [ECH(SIGURD)] = 30,
   [ECH(NOT_SO_INCOGNITO)] = 1,
   [ECH(DUALIST)] = 2,
   [ECH(THRICE)] = 3,
   [ECH(GOING_HD)] = 1920 * 1080,
   [ECH(REAL_ESTATE_MOGUL)] = 1920 * 1080 + 1280 * 1024,
   [ECH(MAXIMUM_DEFINITION)] = 2,
   [ECH(TERMINOLOGIST)] = 1,
   [ECH(LIFE_ON_THE_EDGE)] = 1,
   [ECH(BILINGUAL)] = 2,
   [ECH(POLYGLOT)] = 4,
   [ECH(GADGETEER)] = 5,
   [ECH(INSPECTOR_GADGET)] = 10,
   [ECH(GADGET_HACKWRENCH)] = 15,
   [ECH(SECURITY_CONSCIOUS)] = 5,
   [ECH(SECURITY_NUT)] = 10,
   [ECH(CHIEF_OF_SECURITY)] = 20,
   [ECH(TILED)] = 1,
   [ECH(PERSISTENT)] = 24,
   [ECH(NEVER_GONNA_GIVE_YOU_UP)] = 24 * 7,
};

#define EP(X) ETROPHY_TROPHY_POINTS_##X

static unsigned int Echievement_Points[] =
{
                                      /* LAST - first possible shelf orient */
   [ECH(SHELF_POSITIONS)] = EP(FEW),
   [ECH(NOTHING_ELSE_MATTERS)] = EP(FEW),
   [ECH(FEAR_OF_THE_DARK)] = EP(FEW),
   [ECH(CAVE_DWELLER)] = EP(FEW),
   [ECH(WINDOW_ENTHUSIAST)] = EP(VERY_FEW),
   [ECH(WINDOW_LOVER)] = EP(FEW),
   [ECH(WINDOW_STALKER)] = EP(MANY),
   [ECH(WINDOW_MOVER)] = EP(VERY_FEW),
   [ECH(WINDOW_OCD)] = EP(FEW),
   [ECH(MOUSE_RUNNER)] = EP(VERY_FEW),
   [ECH(MOUSE_MARATHONER)] = EP(FEW),
   [ECH(MOUSE_HERO)] = EP(MANY),
   [ECH(MIGHTY_MOUSE)] = EP(LOTS),
   [ECH(WINDOW_HAULER)] = EP(FEW),
   [ECH(WINDOW_SLINGER)] = EP(MANY),
   [ECH(WINDOW_SHERPA)] = EP(LOTS),
   [ECH(PHYSICIST)] = EP(VERY_FEW),
   [ECH(QUICKDRAW)] = EP(VERY_FEW),
   [ECH(OPAQUE)] = EP(FEW),
   [ECH(KEYBOARD_USER)] = EP(VERY_FEW),
   [ECH(KEYBOARD_TAPPER)] = EP(FEW),
   [ECH(KEYBOARD_ABUSER)] = EP(MANY),
   [ECH(KEYBOARD_NINJA)] = EP(LOTS),
   [ECH(CLICKER)] = EP(VERY_FEW),
   [ECH(SUPER_CLICKER)] = EP(FEW),
   [ECH(SUPER_DUPER_CLICKER)] = EP(MANY),
   [ECH(CLICK_MANIAC)] = EP(LOTS),
   [ECH(SLEEPER)] = EP(FEW),
   [ECH(HALTER)] = EP(MANY),
   [ECH(EDGY)] = EP(VERY_FEW),
   [ECH(EDGIER)] = EP(FEW),
   [ECH(EDGIEST)] = EP(MANY),
   [ECH(EDGAR)] = EP(LOTS),
   [ECH(WHEELY)] = EP(FEW),
   [ECH(ROLLIN)] = EP(MANY),
   [ECH(ROLY_POLY)] = EP(LOTS),
   [ECH(SIGNALLER)] = EP(VERY_FEW),
   [ECH(SIGNAL_CONTROLLER)] = EP(FEW),
   [ECH(SIGNAL_MASTER)] = EP(MANY),
   [ECH(SIGURD)] = EP(LOTS),
   [ECH(NOT_SO_INCOGNITO)] = EP(MANY),
   [ECH(DUALIST)] = EP(FEW),
   [ECH(THRICE)] = EP(LOTS),
   [ECH(GOING_HD)] = EP(VERY_FEW),
   [ECH(REAL_ESTATE_MOGUL)] = EP(FEW),
   [ECH(MAXIMUM_DEFINITION)] = EP(MANY),
   [ECH(TERMINOLOGIST)] = EP(FEW),
   [ECH(LIFE_ON_THE_EDGE)] = EP(VERY_FEW),
   [ECH(BILINGUAL)] = EP(FEW),
   [ECH(POLYGLOT)] = EP(MANY),
   [ECH(GADGETEER)] = EP(FEW),
   [ECH(INSPECTOR_GADGET)] = EP(MANY),
   [ECH(GADGET_HACKWRENCH)] = EP(LOTS),
   [ECH(SECURITY_CONSCIOUS)] = EP(FEW),
   [ECH(SECURITY_NUT)] = EP(MANY),
   [ECH(CHIEF_OF_SECURITY)] = EP(LOTS),
   [ECH(TILED)] = EP(VERY_FEW),
   [ECH(PERSISTENT)] = EP(FEW),
   [ECH(NEVER_GONNA_GIVE_YOU_UP)] = EP(LOTS),
};
#undef EP

/* sighhh the amount of time it actually took me to build the list below...
 * update periodically from http://www.ovguide.com/adult-tubesites-visits-alltime.html
 * to maintain hilarity
 */
static const char *const Echievement_NOT_SO_INCOGNITO_Strings[] =
{
   "Porn Tube, You Porn, Free Porn Movies", //youjizz
   "Free Porn Videos - XVIDEOS.COM", //xvideos
   "Free Porn Sex Videos - Redtube", //redtube
   "Free sex videos, Sex tube", //tube8
   "xHamster's Free Porn Videos", //xhamster
   "Video - Free Porn Videos", //yuvutu <--I guess they just gave up at naming?
   "Free porno tube site!", //koostube
   "Alot Porn Tube", //alotporn <--nice grammar, jackasses
   "Free Porn Videos, Porn Tube, Sex Videos", //mofosex
   "Free Porn Tube Movies & XXX Sex Pics", //keezmovies
   "Free Porn Videos, Porn Tube, Free Porn", //tnaflix
   "Porn Videos, Sex, XXX, Free Porn Tube", //youporn
   "Free Porn, Sex Videos 4 You", //spankwire
   "Free Porn Videos, Free Sex, XXX", //empflix
   "Free Porn Videos & Sex Movies", //pornhub
   NULL
};

static const char *const Echievement_Strings[] =
{
   [ECH(SHELF_POSITIONS)] = N_("Shelves Everywhere"),
   [ECH(NOTHING_ELSE_MATTERS)] = N_("Nothing Else Matters"),
   [ECH(FEAR_OF_THE_DARK)] = N_("Fear Of The Dark"),
   [ECH(CAVE_DWELLER)] = N_("Cave Dweller"),
   [ECH(WINDOW_ENTHUSIAST)] = N_("Window Enthusiast"),
   [ECH(WINDOW_LOVER)] = N_("Window Lover"),
   [ECH(WINDOW_STALKER)] = N_("Window Stalker"),
   [ECH(WINDOW_MOVER)] = N_("Window Mover"),
   [ECH(WINDOW_OCD)] = N_("Window OCD"),
   [ECH(MOUSE_RUNNER)] = N_("Mouse Runner"),
   [ECH(MOUSE_MARATHONER)] = N_("Mouse Marathoner"),
   [ECH(MOUSE_HERO)] = N_("Mouse Hero"),
   [ECH(MIGHTY_MOUSE)] = N_("Mighty Mouse"),
   [ECH(WINDOW_HAULER)] = N_("Window Hauler"),
   [ECH(WINDOW_SLINGER)] = N_("Window Slinger"),
   [ECH(WINDOW_SHERPA)] = N_("Window Sherpa"),
   [ECH(PHYSICIST)] = N_("Physicist"),
   [ECH(QUICKDRAW)] = N_("Quickdraw"),
   [ECH(OPAQUE)] = N_("Opaque"),
   [ECH(KEYBOARD_USER)] = N_("Keyboard User"),
   [ECH(KEYBOARD_TAPPER)] = N_("Keyboard Tapper"),
   [ECH(KEYBOARD_ABUSER)] = N_("Keyboard Abuser"),
   [ECH(KEYBOARD_NINJA)] = N_("Keyboard Ninja"),
   [ECH(CLICKER)] = N_("Clicker"),
   [ECH(SUPER_CLICKER)] = N_("Super Clicker"),
   [ECH(SUPER_DUPER_CLICKER)] = N_("Super Duper Clicker"),
   [ECH(CLICK_MANIAC)] = N_("Click Maniac"),
   [ECH(SLEEPER)] = N_("Sleeper"),
   [ECH(HALTER)] = N_("Halter"),
   [ECH(EDGY)] = N_("Edgy"),
   [ECH(EDGIER)] = N_("Edgier"),
   [ECH(EDGIEST)] = N_("Edgiest"),
   [ECH(EDGAR)] = N_("Edgar"),
   [ECH(WHEELY)] = N_("Wheely"),
   [ECH(ROLLIN)] = N_("Rollin'"),
   [ECH(ROLY_POLY)] = N_("Roly Poly"),
   [ECH(SIGNALLER)] = N_("Signaller"),
   [ECH(SIGNAL_CONTROLLER)] = N_("Signal Controller"),
   [ECH(SIGNAL_MASTER)] = N_("Signal Master"),
   [ECH(SIGURD)] = N_("Sigurd"),
   [ECH(NOT_SO_INCOGNITO)] = N_("Not So Incognito"),
   [ECH(DUALIST)] = N_("Dualist"),
   [ECH(THRICE)] = N_("Thrice"),
   [ECH(GOING_HD)] = N_("Going HD"),
   [ECH(REAL_ESTATE_MOGUL)] = N_("Real Estate Mogul"),
   [ECH(MAXIMUM_DEFINITION)] = N_("Maximum Definition"),
   [ECH(TERMINOLOGIST)] = N_("Terminologist"),
   [ECH(LIFE_ON_THE_EDGE)] = N_("Life On The Edge"),
   [ECH(BILINGUAL)] = N_("Bilingual"),
   [ECH(POLYGLOT)] = N_("Polyglot"),
   [ECH(GADGETEER)] = N_("Gadgeteer"),
   [ECH(INSPECTOR_GADGET)] = N_("Inspector Gadget"),
   [ECH(GADGET_HACKWRENCH)] = N_("Gadget Hackwrench"),
   [ECH(SECURITY_CONSCIOUS)] = N_("Security Conscious"),
   [ECH(SECURITY_NUT)] = N_("Security Nut"),
   [ECH(CHIEF_OF_SECURITY)] = N_("Chief Of Security"),
   [ECH(TILED)] = N_("Tiled"),
   [ECH(PERSISTENT)] = N_("Persistent"),
   [ECH(NEVER_GONNA_GIVE_YOU_UP)] = N_("Never Gonna Give You Up"),
};

static const char *const Echievement_Descs[] =
{
   [ECH(SHELF_POSITIONS)] = N_("Have a shelf in every position"),
   [ECH(NOTHING_ELSE_MATTERS)] = N_("Use E17 with no shelves"),
   [ECH(FEAR_OF_THE_DARK)] = N_("Set backlight level to 100%"),
   [ECH(CAVE_DWELLER)] = N_("Set backlight level to 0%"),
   [ECH(WINDOW_ENTHUSIAST)] = N_("Open 100 windows"),
   [ECH(WINDOW_LOVER)] = N_("Open 1000 windows"),
   [ECH(WINDOW_STALKER)] = N_("Open 10000 windows"),
   [ECH(WINDOW_MOVER)] = N_("Move 50 windows"),
   [ECH(WINDOW_OCD)] = N_("Move 250 windows"),
   [ECH(MOUSE_RUNNER)] = N_("Mouse over 250,000 pixels"),
   [ECH(MOUSE_MARATHONER)] = N_("Mouse over 1,000,000 pixels"),
   [ECH(MOUSE_HERO)] = N_("Mouse over 10,000,000 pixels"),
   [ECH(MIGHTY_MOUSE)] = N_("Mouse over 100,000,000 pixels"),
   [ECH(WINDOW_HAULER)] = N_("Move windows 250,000 pixels"),
   [ECH(WINDOW_SLINGER)] = N_("Move windows 1,000,000 pixels"),
   [ECH(WINDOW_SHERPA)] = N_("Move windows 10,000,000 pixels"),
   [ECH(PHYSICIST)] = N_("Load the Physics module"),
   [ECH(QUICKDRAW)] = N_("Load the Quickaccess module"),
   [ECH(OPAQUE)] = N_("Unload the Composite module"),
   [ECH(KEYBOARD_USER)] = N_("Change 5 key bindings"),
   [ECH(KEYBOARD_TAPPER)] = N_("Change 10 key bindings"),
   [ECH(KEYBOARD_ABUSER)] = N_("Change 20 key bindings"),
   [ECH(KEYBOARD_NINJA)] = N_("Change 30 key bindings"),
   [ECH(CLICKER)] = N_("Change 5 mouse bindings"),
   [ECH(SUPER_CLICKER)] = N_("Change 10 mouse bindings"),
   [ECH(SUPER_DUPER_CLICKER)] = N_("Change 20 mouse bindings"),
   [ECH(CLICK_MANIAC)] = N_("Change 30 mouse bindings"),
   [ECH(SLEEPER)] = N_("Change 2 ACPI bindings"),
   [ECH(HALTER)] = N_("Change 5 ACPI bindings"),
   [ECH(EDGY)] = N_("Change 2 Edge bindings"),
   [ECH(EDGIER)] = N_("Change 4 Edge bindings"),
   [ECH(EDGIEST)] = N_("Change 6 Edge bindings"),
   [ECH(EDGAR)] = N_("Change 8 Edge bindings"),
   [ECH(WHEELY)] = N_("Change 3 Mouse Wheel bindings"),
   [ECH(ROLLIN)] = N_("Change 6 Mouse Wheel bindings"),
   [ECH(ROLY_POLY)] = N_("Change 10 Mouse Wheel bindings"),
   [ECH(SIGNALLER)] = N_("Change 5 Edje Signal Bindings"),
   [ECH(SIGNAL_CONTROLLER)] = N_("Change 10 Edje Signal Bindings"),
   [ECH(SIGNAL_MASTER)] = N_("Change 20 Edje Signal Bindings"),
   [ECH(SIGURD)] = N_("Change 30 Edje Signal Bindings"),
   [ECH(NOT_SO_INCOGNITO)] = N_("Watch porn while using E17"),
   [ECH(DUALIST)] = N_("Connect two monitors simultaneously"),
   [ECH(THRICE)] = N_("Connect three monitors simultaneously"),
   [ECH(GOING_HD)] = N_("Have 1080p on a single monitor"),
   [ECH(REAL_ESTATE_MOGUL)] = N_("At least 3,384,320 pixels"),
   [ECH(MAXIMUM_DEFINITION)] = N_("Dual 1080p monitors"),
   [ECH(TERMINOLOGIST)] = N_("Open a Terminology window"),
   [ECH(LIFE_ON_THE_EDGE)] = N_("Load a module which taints E17"),
   [ECH(BILINGUAL)] = N_("Have two languages available"),
   [ECH(POLYGLOT)] = N_("Have four languages available"),
   [ECH(GADGETEER)] = N_("Have 5 gadgets on the desktop"),
   [ECH(INSPECTOR_GADGET)] = N_("Have 10 gadgets on the desktop"),
   [ECH(GADGET_HACKWRENCH)] = N_("Have 15 gadgets on the desktop"),
   [ECH(SECURITY_CONSCIOUS)] = N_("Desklock 5 times in an hour"),
   [ECH(SECURITY_NUT)] = N_("Desklock 10 times in an hour"),
   [ECH(CHIEF_OF_SECURITY)] = N_("Desklock 20 times in an hour"),
   [ECH(TILED)] = N_("Load the Tiling module"),
   [ECH(PERSISTENT)] = N_("Keep a window open for 24 hours"),
   [ECH(NEVER_GONNA_GIVE_YOU_UP)] = N_("Keep a window open for 168 hours"),
};

#define ECB(NAME) EINTERN void echievement_init_cb_##NAME(Echievement *ec)

ECB(SHELF_POSITIONS);
ECB(NOTHING_ELSE_MATTERS);
ECB(FEAR_OF_THE_DARK);
ECB(CAVE_DWELLER);
ECB(WINDOW_ENTHUSIAST);
ECB(WINDOW_MOVER);
ECB(MOUSE_RUNNER);
ECB(WINDOW_HAULER);
ECB(PHYSICIST);
ECB(QUICKDRAW);
ECB(OPAQUE);
ECB(KEYBOARD_USER);
ECB(CLICKER);
ECB(SLEEPER);
ECB(EDGY);
ECB(WHEELY);
ECB(SIGNALLER);
ECB(NOT_SO_INCOGNITO);
ECB(DUALIST);
ECB(GOING_HD);
ECB(REAL_ESTATE_MOGUL);
ECB(MAXIMUM_DEFINITION);
ECB(TERMINOLOGIST);
ECB(LIFE_ON_THE_EDGE);
ECB(BILINGUAL);
ECB(GADGETEER);
ECB(SECURITY_CONSCIOUS);
ECB(TILED);
ECB(PERSISTENT);

#undef ECB
#define ECB(NAME) [ECHIEVEMENT_ID_##NAME] = echievement_init_cb_##NAME
#define ECB_REUSE(NAME, REUSE) [ECHIEVEMENT_ID_##NAME] = echievement_init_cb_##REUSE

Echievement_Init_Cb Echievement_Callbacks[] =
{
   ECB(SHELF_POSITIONS),
   ECB(NOTHING_ELSE_MATTERS),
   ECB(FEAR_OF_THE_DARK),
   ECB(CAVE_DWELLER),
   ECB(WINDOW_ENTHUSIAST),
   ECB_REUSE(WINDOW_LOVER, WINDOW_ENTHUSIAST),
   ECB_REUSE(WINDOW_STALKER, WINDOW_ENTHUSIAST),
   ECB(WINDOW_MOVER),
   ECB_REUSE(WINDOW_OCD, WINDOW_MOVER),
   ECB(MOUSE_RUNNER),
   ECB_REUSE(MOUSE_MARATHONER, MOUSE_RUNNER),
   ECB_REUSE(MOUSE_HERO, MOUSE_RUNNER),
   ECB_REUSE(MIGHTY_MOUSE, MOUSE_RUNNER),
   ECB(WINDOW_HAULER),
   ECB_REUSE(WINDOW_SLINGER, WINDOW_HAULER),
   ECB_REUSE(WINDOW_SHERPA, WINDOW_HAULER),
   ECB(PHYSICIST),
   ECB(QUICKDRAW),
   ECB(OPAQUE),
   ECB(KEYBOARD_USER),
   ECB_REUSE(KEYBOARD_TAPPER, KEYBOARD_USER),
   ECB_REUSE(KEYBOARD_ABUSER, KEYBOARD_USER),
   ECB_REUSE(KEYBOARD_NINJA, KEYBOARD_USER),
   ECB(CLICKER),
   ECB_REUSE(SUPER_CLICKER, CLICKER),
   ECB_REUSE(SUPER_DUPER_CLICKER, CLICKER),
   ECB_REUSE(CLICK_MANIAC, CLICKER),
   ECB(SLEEPER),
   ECB_REUSE(HALTER, SLEEPER),
   ECB(EDGY),
   ECB_REUSE(EDGIER, EDGY),
   ECB_REUSE(EDGIEST, EDGY),
   ECB_REUSE(EDGAR, EDGY),
   ECB(WHEELY),
   ECB_REUSE(ROLLIN, WHEELY),
   ECB_REUSE(ROLY_POLY, WHEELY),
   ECB(SIGNALLER),
   ECB_REUSE(SIGNAL_CONTROLLER, SIGNALLER),
   ECB_REUSE(SIGNAL_MASTER, SIGNALLER),
   ECB_REUSE(SIGURD, SIGNALLER),
   ECB(NOT_SO_INCOGNITO),
   ECB(DUALIST),
   ECB_REUSE(THRICE, DUALIST),
   ECB(GOING_HD),
   ECB(REAL_ESTATE_MOGUL),
   ECB(MAXIMUM_DEFINITION),
   ECB(TERMINOLOGIST),
   ECB(LIFE_ON_THE_EDGE),
   ECB(BILINGUAL),
   ECB_REUSE(POLYGLOT, BILINGUAL),
   ECB(GADGETEER),
   ECB_REUSE(INSPECTOR_GADGET, GADGETEER),
   ECB_REUSE(GADGET_HACKWRENCH, GADGETEER),
   ECB(SECURITY_CONSCIOUS),
   ECB_REUSE(SECURITY_NUT, SECURITY_CONSCIOUS),
   ECB_REUSE(CHIEF_OF_SECURITY, SECURITY_CONSCIOUS),
   ECB(TILED),
   ECB(PERSISTENT),
   ECB_REUSE(NEVER_GONNA_GIVE_YOU_UP, PERSISTENT),
   NULL
};

#undef ECB
