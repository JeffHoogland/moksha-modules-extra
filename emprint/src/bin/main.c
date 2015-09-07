#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <Eina.h>
#include <Evas.h>
#include <Ecore.h>
#include <Ecore_Input.h>
#include <Ecore_Evas.h>
#include <Ecore_File.h>
#include <Ecore_X.h>
#include <Ecore_X_Cursor.h>
#include <getopt.h>
#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif
#include "config.h"
#include "emprint.h"

/* Function Prototypes */
static void _em_parse_cmdln(Options *o, int argc, char *argv[]);
static void _em_print_help(void);
static void _em_print_version(void);
static void _em_free_options(void);
static void _em_get_filename(void);
static void _em_do_shot(void);
static void _em_do_screen(void);
static void _em_do_window(void);
static void _em_do_region(void);
static void _em_do_thumb(const char *filename);
static void _em_take_shot(int x, int y, int w, int h);
static Eina_Bool _em_cb_key_down(void *data __UNUSED__, int type __UNUSED__, void *event);
static Eina_Bool _em_cb_mouse_move(void *data __UNUSED__, int type __UNUSED__, void *event);
static Eina_Bool _em_cb_mouse_up(void *data __UNUSED__, int type __UNUSED__, void *event);
static Eina_Bool _em_cb_mouse_down(void *data __UNUSED__, int type __UNUSED__, void *event);
static Eina_Bool _em_cb_timer(void *data __UNUSED__);
static void _em_band_show(void);
static void _em_band_move(int x, int y);
static void _em_band_hide(void);
static void _em_band_resize(int w, int h);
static void _em_grab_region_end(void);
static void _em_do_app(void);
static int _em_get_filecount(const char *filename);

/* Global Variables */
static Options *opts = NULL;
static Ecore_X_Window input_window = 0;
static Ecore_Event_Handler *mouse_move_hdl = 0;
static Ecore_Event_Handler *mouse_up_hdl = 0;
static Ecore_Event_Handler *mouse_down_hdl = 0;
static Ecore_Event_Handler *key_hdl = 0;
static Ecore_Timer *timer = NULL;
static Band *band = NULL;
static int gx = -1, gy = -1;

/* Main Program Function */
int 
main(int argc, char **argv) 
{
   /* initialize eina */
   if (!eina_init()) exit(EXIT_FAILURE);

   /* allocate a structure to hold our options */
   opts = calloc(1, sizeof(Options));
   if (!opts)
     {
	printf("Cannot allocate memory for Options structure.\n");
        eina_shutdown();
	exit(EXIT_FAILURE);
     }

   /* parse our command line */
   _em_parse_cmdln(opts, argc, argv);

   /* initialize ecore */
   if (!ecore_init()) 
     {
	_em_free_options();
        eina_shutdown();
	exit(EXIT_FAILURE);
     }

   /* initialize ecore_evas */
   if (!ecore_evas_init()) 
     {
	_em_free_options();
	ecore_shutdown();
        eina_shutdown();
	exit(EXIT_FAILURE);
     }

   /* initialize ecore_x */
   if (!ecore_x_init(NULL)) 
     {
	_em_free_options();
	ecore_evas_shutdown();
	ecore_shutdown();
        eina_shutdown();
	exit(EXIT_FAILURE);
     }

   /* set app arguments */
   ecore_app_args_set(argc, (const char **)argv);

   /* parse the filename for '%', or create one if no filename is given */
   _em_get_filename();

   /* start screenshot process */
   _em_do_shot();

   /* begin the ecore main loop which will keep our app running */
   ecore_main_loop_begin();

   /* free our option structure */
   _em_free_options();

   /* shutdown ecore_x */
   ecore_x_shutdown();

   /* shutdown ecore_evas */
   ecore_evas_shutdown();

   /* shutdown ecore */
   ecore_shutdown();

   /* shutdown eina */
   eina_shutdown();

   return EXIT_SUCCESS;
}

static void
_em_parse_cmdln(Options *o, int argc, char *argv[])
{
   int tw, th, ts;
   char c;
   struct option longopts[]  = 
     {
	  {"beep", no_argument, &(o->beep), 1},
	  {"delay", required_argument, 0, 'd'},
	  {"app", required_argument, 0, 'a'},
	  {"thumb", required_argument, 0, 't'},
	  {"thumb-geom", required_argument, 0, 'g'},
	  {"quality", required_argument, 0, 'q'},
	  {"region", no_argument, &(o->region), 1},
	  {"window", no_argument, &(o->window), 1},
	  {"help", no_argument, 0, 'h'},
	  {"version", no_argument, 0, 'v'},
	  {NULL, 0, NULL, 0}
     };

   /* parse the options provided by user */
   while ((c = getopt_long_only(argc, argv, "d:a:t:g:q:hv", longopts, NULL)) != -1)
     {
	switch (c) 
          {
           case 0: /* Flags were set.... do nothing. */
             break;
           case 'd':
             o->delay = atoi(optarg);
             break;
           case 'a':
             o->app = eina_stringshare_add(optarg);
             break;
           case 't':
             o->use_thumb = 1;
             o->thumb.filename = eina_stringshare_add(optarg);
             break;
           case 'g':
             o->use_thumb = 1;
             if (strstr(optarg, "x"))
               {
                  sscanf(optarg, "%ix%i", &tw, &th);
                  o->thumb.width = tw;
                  o->thumb.height = th;
               }
             else
               {
                  ts = atoi(optarg);
                  if (ts < 1) ts = 1;
                  else if (ts > 100) ts = 100;
                  o->thumb.size = ts;
               }
             break;
           case 'q':
             o->quality = atoi(optarg);
             break;
           case 'v': /* Print version and bail */
             _em_print_version();
             break;
           case '?': /* ErrMsg is printed, then Fallthrough */
           case 'h': /* Fallthrough */
           default:
             _em_print_help();
             break;
          }
     }

   /* The filename, if it exists, is expected to be the last command line arg */
   if (optind < argc) o->filename = eina_stringshare_add(argv[optind]);
}

static void 
_em_print_help(void) 
{
   printf("Usage: emprint [OPTIONS]... FILE \n"
	  " Unless otherwise noted, all options may be used in\n"
	  " their shorthand form (e.g. --thumb == -t) using the first\n"
	  " letter in the option name.\n"
	  "\tFILE\t\t\tWhere FILE is the target for\n"
	  "\t\t\t\tthe screenshot. If no FILE is specified,\n"
	  "\t\t\t\ta date-stamped file will be saved in the\n"
	  "\t\t\t\tcurrent directory.\n"
	  "\t--help\t\t\tDisplay this help\n"
	  "\t--beep\t\t\tBeep before taking screenshot\n"
	  "\t--delay NUM\t\tWait NUM seconds before taking screenshot\n"
	  "\t--app APP\t\tLaunch APP after taking screenshot.  A '%%s'\n"
	  "\t\t\t\tincluded in the app command line will be\n"
	  "\t\t\t\treplaced with the filename.\n"
	  "\t--thumb THUMB\t\tGenerate a thumbnail as THUMB\n"
	  "\t--thumb-geom NUM\tGeometry to use for thumbnail\n"
	  "\t\t\t\tNUM can be a percentage of the original size OR\n"
	  "\t\t\t\tthe actual geometry to use for the thumbnail,\n"
	  "\t\t\t\te.g. 100x100.\n"
	  "\t\t\t\t(Shorthand: -g)\n"
	  "\t--quality NUM\t\tImage quality of screenshot (in percentage)\n"
	  "\t--region\t\tSelect a specific screen region\n"
	  "\t--window\t\tSelect a specific window to grab\n"
	  "\t--version\t\tPrint the version.\n"
	  );

   _em_free_options();
   exit(EXIT_SUCCESS);
}

static void 
_em_print_version(void) 
{
   printf("Emprint version "VERSION"\n");
   _em_free_options();
   exit(EXIT_SUCCESS);
}

static void 
_em_free_options(void) 
{
   /* cleanup our options structure */
   if (!opts) return;
   if (opts->app) eina_stringshare_del(opts->app);
   if (opts->filename) eina_stringshare_del(opts->filename);
   if (opts->thumb.filename) eina_stringshare_del(opts->thumb.filename);
   free(opts);
   opts = NULL;
}

static void 
_em_get_filename(void) 
{
   char buf[256];
   struct tm *loctime;
   time_t t;

   /* get the current time in local format */
   t = time(NULL);
   loctime = localtime(&t);

   /* if no filename given, create one based on time */
   if (!opts->filename)
     strftime(buf, sizeof(buf), "%Y-%m-%d-%H%M%S.png", loctime);
   else 
     {
	if (ecore_file_is_dir(opts->filename)) 
	  {
	     strftime(buf, sizeof(buf), "%Y-%m-%d-%H%M%S.png", loctime);
	     snprintf(buf, sizeof(buf), "%s/%s", opts->filename, strdup(buf));
	  }
	else 
	  {
             char *filename = NULL;
             const char *file = NULL, *dir = NULL;
             char *ext = NULL, *p = NULL;

             filename = strdup(opts->filename);

             dir = ecore_file_dir_get(filename);
             file = ecore_file_file_get(filename);

             if ((dir) && (file))
               {
                  if (!strcmp(dir, file)) dir = getenv("PWD");
               }
             else if (!dir) 
               dir = getenv("PWD");

             /* strip the extension for searches */
             ext = strrchr(file, '.');
             if (!ext) ext = strdup(".png");

	     /* filename was given, check for '%' signs to 
	      * format based on strftime or file count */
	     if ((strstr(opts->filename, "%")) && 
                 (!strstr(opts->filename, "%N")))
	       strftime(buf, sizeof(buf), opts->filename, loctime);
             else if (strstr(opts->filename, "%N")) 
               {
                  int c = 0;

                  /* strip any %N out of filename */
                  p = strrchr(file, '%');
                  if (p) *p = 0;

                  c = _em_get_filecount(filename);

                  if (c > 0) 
                    snprintf(buf, sizeof(buf), "%s/%s%i%s", dir, file, c, ext);
                  else
                    snprintf(buf, sizeof(buf), "%s/%s%s", dir, file, ext);
               }
             else 
               {
                  if (!strrchr(file, '.'))
                    snprintf(buf, sizeof(buf), "%s/%s.png", dir, file);
                  else
                    snprintf(buf, sizeof(buf), "%s/%s", dir, file);
               }
	  }
     }

   /* set the new filename */
   eina_stringshare_replace(&opts->filename, buf);
}

static void 
_em_do_shot(void) 
{
   /* if user specified a delay, do it */
   if (opts->delay > 0) 
     {
	/* create a timer for the delay */
	if (timer) ecore_timer_del(timer);
	timer = ecore_timer_add(1.0, _em_cb_timer, NULL);
     }
   else 
     {
	/* are we taking a region, window or whole screen shot? */
	if (opts->region)
	  _em_do_region();
	else if (opts->window)
	  _em_do_window();
	else
	  _em_do_screen();
     }
}

static void 
_em_do_screen(void) 
{
   int w, h;

   /* get the size of the root window */
   ecore_x_window_size_get(ecore_x_window_root_first_get(), &w, &h);

   /* actually take the shot */
   _em_take_shot(0, 0, w, h);

   /* quit the main ecore loop */
   ecore_main_loop_quit();
}

static void 
_em_do_window(void) 
{
   Ecore_X_Window root;
   Ecore_X_Cursor cursor = 0;
   int x, y, w, h;

   /* get the root window */
   root = ecore_x_window_root_first_get();

   /* get the size of the current root window */
   ecore_x_window_geometry_get(root, &x, &y, &w, &h);

   /* check if our input window already exists & delete it */
   if (input_window) ecore_x_window_free(input_window);

   /* create a new input window to recieve click event */
   input_window = ecore_x_window_input_new(root, x, y, w, h);

   /* show the input window */
   ecore_x_window_show(input_window);

   /* grab keystrokes */
   ecore_x_keyboard_grab(input_window);

   /* set the mouse pointer */
   if ((cursor = ecore_x_cursor_shape_get(ECORE_X_CURSOR_CROSS))) 
     ecore_x_window_cursor_set(input_window, cursor);

   /* setup handler to recieve key event */
   key_hdl = ecore_event_handler_add(ECORE_EVENT_KEY_DOWN, 
                                     _em_cb_key_down, NULL);

   /* setup handler to recieve click event */
   mouse_up_hdl = ecore_event_handler_add(ECORE_EVENT_MOUSE_BUTTON_UP, 
					  _em_cb_mouse_up, NULL);
}

static void 
_em_do_region(void) 
{
   Ecore_X_Window root;
   Ecore_X_Cursor cursor = 0;
   int x, y, w, h;

   /* get the root window */
   root = ecore_x_window_root_first_get();

   /* get the size of the current root window */
   ecore_x_window_geometry_get(root, &x, &y, &w, &h);

   /* check if our input window already exists & delete it */
   if (input_window) ecore_x_window_free(input_window);

   /* create a new input window to recieve click event */
   input_window = ecore_x_window_input_new(root, x, y, w, h);

   /* show the input window */
   ecore_x_window_show(input_window);

   /* grab keystrokes */
   ecore_x_keyboard_grab(input_window);

   /* setup handler to recieve key event */
   key_hdl = ecore_event_handler_add(ECORE_EVENT_KEY_DOWN, 
                                     _em_cb_key_down, NULL);

   /* setup handlers to recieve mouse events */
   mouse_move_hdl = ecore_event_handler_add(ECORE_EVENT_MOUSE_MOVE, 
					    _em_cb_mouse_move, NULL);
   mouse_up_hdl = ecore_event_handler_add(ECORE_EVENT_MOUSE_BUTTON_UP,
					  _em_cb_mouse_up, NULL);
   mouse_down_hdl = ecore_event_handler_add(ECORE_EVENT_MOUSE_BUTTON_DOWN,
					    _em_cb_mouse_down, NULL);

   /* set the mouse pointer */
   if ((cursor = ecore_x_cursor_shape_get(ECORE_X_CURSOR_CROSS))) 
     ecore_x_window_cursor_set(input_window, cursor);

   /* create the rubber band */
   _em_band_show();
}

static void 
_em_do_thumb(const char *filename) 
{
   Ecore_Evas *ee, *sub_ee;
   Evas *sub_evas;
   Evas_Object *tmp, *img;
   char *ext = NULL;
   char buf[PATH_MAX];
   int w, h, tw, th;

   if (!filename) return;

   ee = ecore_evas_buffer_new(1, 1);

   tmp = ecore_evas_object_image_new(ee);
   sub_ee = evas_object_data_get(tmp, "Ecore_Evas");
   sub_evas = ecore_evas_get(sub_ee);

   img = evas_object_image_add(sub_evas);
   evas_object_image_file_set(img, filename, NULL);

   /* get the image width & height */
   evas_object_image_size_get(img, &w, &h);

   /* calculate thumbnail size */
   if ((opts->thumb.width > 0) && (opts->thumb.height > 0)) 
     {
	tw = opts->thumb.width;
	th = opts->thumb.height;
     }
   else if (opts->thumb.size > 0) 
     {
	tw = w * opts->thumb.size / 100;
	th = h * opts->thumb.size / 100;
     }
   else 
     {
	/* by default, create a 50% thumbnail */
	tw = w * 50 / 100;
	th = h * 50 / 100;
     }

   evas_object_image_file_set(img, NULL, NULL);
   evas_object_image_load_size_set(img, tw, th);
   evas_object_image_file_set(img, filename, NULL);
   evas_object_show(img);
   evas_object_move(img, 0, 0);
   evas_object_resize(img, tw, th);
   evas_object_image_fill_set(img, 0, 0, tw, th);

   evas_object_image_size_set(tmp, tw, th);
   ecore_evas_resize(sub_ee, tw, th);

   evas_damage_rectangle_add(sub_evas, 0, 0, tw, th);
   evas_render(sub_evas);

   /* check for user-supplied thumbnail filename */
   if (!opts->thumb.filename) 
     {
        /* no thumbname filename supplied, create one */
        ext = ecore_file_strip_ext(opts->filename);
        snprintf(buf, sizeof(buf), "%s-thumb", ext);

        /* check for extension */
        ext = strrchr(opts->filename, '.');
        if (ext) 
          snprintf(buf, sizeof(buf), "%s%s", strdup(buf), ext);
        else
          snprintf(buf, sizeof(buf), "%s.png", strdup(buf));
        opts->thumb.filename = eina_stringshare_add(buf);
     }

   /* actually save the thumbnail */
   snprintf(buf, sizeof(buf), "quality=%d compress=9", opts->quality);
   if (!(evas_object_image_save(tmp, opts->thumb.filename, NULL, buf))) 
     printf("Error saving thumbnail: %s\n", opts->thumb.filename);

   if (tmp) evas_object_del(tmp);
   if (img) evas_object_del(img);
   if (sub_ee) ecore_evas_free(sub_ee);
   if (ee) ecore_evas_free(ee);
}

static void 
_em_take_shot(int x, int y, int w, int h) 
{
   Ecore_Evas *ee;
   Evas *evas;
   Evas_Object *im;
   Ecore_X_Window root;
   Ecore_X_Image *xim;
   Ecore_X_Window_Attributes att;
   unsigned int *pix;
   char buff[1024];

   /* if user wanted a beep, then beep there shall be */
   if (opts->beep) ecore_x_bell(0);

   memset(&att, 0, sizeof(Ecore_X_Window_Attributes));
   root = ecore_x_window_root_first_get();
   ecore_x_window_attributes_get(root, &att);

   xim = ecore_x_image_new(w, h, att.visual, att.depth);
   ecore_x_image_get(xim, root, x, y, 0, 0, w, h);
   pix = ecore_x_image_data_get(xim, NULL, NULL, NULL);

   ee = ecore_evas_buffer_new(w, h);
   evas = ecore_evas_get(ee);

   im = evas_object_image_filled_add(evas);
   evas_object_image_fill_set(im, x, y, w, h);
   evas_object_image_size_set(im, w, h);
   evas_object_image_data_set(im, pix);

   ecore_evas_buffer_pixels_get(ee);

   /* try to save the image to disk */
   snprintf(buff, sizeof(buff), "quality=%d compress=9", opts->quality);
   if (!(evas_object_image_save(im, opts->filename, NULL, buff)))
     {
	printf("Error taking screenshot: %s\n", opts->filename);
        if (im) evas_object_del(im);
        if (ee) ecore_evas_free(ee);
        if (xim) ecore_x_image_free(xim);
        return;
     }

   /* cleanup */
   if (im) evas_object_del(im);
   if (ee) ecore_evas_free(ee);
   if (xim) ecore_x_image_free(xim);

   /* if user wanted a thumbnail, do it */
   if (opts->use_thumb) _em_do_thumb(opts->filename);

   /* launch application if user wanted one */
   if (opts->app) _em_do_app();
}

static Eina_Bool 
_em_cb_key_down(void *data __UNUSED__, int type __UNUSED__, void *event) 
{
   Ecore_Event_Key *ev;

   ev = event;

   /* check for correct window */
   if (ev->window != input_window) return 1;

   /* check for correct key */
   if (!strcmp(ev->key, "Escape")) 
     {
        /* delete the event handlers */
        ecore_event_handler_del(key_hdl);
        if (mouse_move_hdl) ecore_event_handler_del(mouse_move_hdl);
        if (mouse_up_hdl) ecore_event_handler_del(mouse_up_hdl);
        if (mouse_down_hdl) ecore_event_handler_del(mouse_down_hdl);

        /* release key grab */
        ecore_x_keyboard_ungrab();

        /* delete the input window */
        ecore_x_window_free(input_window);
        input_window = 0;

        ecore_main_loop_quit();
        return EINA_FALSE;
     }
   return EINA_TRUE;
}

static Eina_Bool 
_em_cb_mouse_move(void *data __UNUSED__, int type __UNUSED__, void *event) 
{
   Ecore_Event_Mouse_Move *ev;
   int x, y, w, h;

   ev = event;
   if ((gx < 0) && (gy < 0)) return EINA_TRUE;

   w = ev->root.x - gx;
   h = ev->root.y - gy;

   /* figure out how to move the band */
   if ((w < 0) && (h < 0))
     {
	ecore_x_pointer_xy_get(input_window, &x, &y);
	_em_band_move(x, y);
	_em_band_resize(-w, -h);
     }
   else if ((w < 0) &&  (h > 0))
     {
	ecore_x_pointer_xy_get(input_window, &x, &y);
	_em_band_move(x, gy);
	_em_band_resize(-w, h);
     }
   else if ((w > 0) && (h < 0))
     {
	ecore_x_pointer_xy_get(input_window, &x, &y);
	_em_band_move(gx, y);
	_em_band_resize(w, -h);
     }
   else
     _em_band_resize(w, h);
   return EINA_TRUE;
}

static Eina_Bool 
_em_cb_mouse_up(void *data __UNUSED__, int type __UNUSED__, void *event) 
{
   Ecore_Event_Mouse_Button *ev;
   Ecore_X_Window win, root;
   int x, y, w, h;

   if (opts->region) 
     {
	_em_grab_region_end();
	return EINA_TRUE;
     }
   ev = event;

   /* check for correct mouse button */
   if (ev->buttons != 1) return EINA_TRUE;

   /* check for correct window */
   if (ev->window != input_window) return EINA_TRUE;

   /* get last known pointer position */
   ecore_x_pointer_last_xy_get(&x, &y);

   /* delete the event handlers */
   if (key_hdl) ecore_event_handler_del(key_hdl);
   if (mouse_move_hdl) ecore_event_handler_del(mouse_move_hdl);
   if (mouse_up_hdl) ecore_event_handler_del(mouse_up_hdl);
   if (mouse_down_hdl) ecore_event_handler_del(mouse_down_hdl);

   /* release key grab */
   ecore_x_keyboard_ungrab();

   /* delete the input window */
   ecore_x_window_free(input_window);
   input_window = 0;

   /* get the window which was clicked */
   win = ecore_x_window_at_xy_get(x, y);

   /* E uses many parent windows...grab the correct one */
   root = ecore_x_window_root_first_get();
   while (win != root) 
     {
	if (ecore_x_window_parent_get(win) == root) break;
	win = ecore_x_window_parent_get(win);
     }

   /* get this window's dimensions */
   ecore_x_window_geometry_get(win, &x, &y, &w, &h);

   /* actually take the shot */
   _em_take_shot(x, y, w, h);

   /* quit the main ecore loop */
   ecore_main_loop_quit();

   return EINA_FALSE;
}

static Eina_Bool 
_em_cb_mouse_down(void *data __UNUSED__, int type __UNUSED__, void *event) 
{
   Ecore_Event_Mouse_Button *ev;

   ev = event;
   if (ev->window != input_window) return EINA_TRUE;
   if (ev->buttons != 1) return EINA_TRUE;

   /* get current mouse coordinates */
   ecore_x_pointer_xy_get(input_window, &gx, &gy);

   /* move the band to current coordinates */
   _em_band_move(gx, gy);
   return EINA_TRUE;
}

static Eina_Bool 
_em_cb_timer(void *data __UNUSED__) 
{
   static int count = 0;

   if (count == opts->delay) 
     {
	/* are we taking a region, window or whole screen shot? */
	if (opts->region)
	  _em_do_region();
	else if (opts->window)
	  _em_do_window();
	else
	  _em_do_screen();

        return EINA_FALSE;
     }

   /* tell the user we are counting down */
   printf("Taking shot in %d\n", (opts->delay - count));
   count++;

   return EINA_TRUE;
}

static void 
_em_band_show(void) 
{
   /* if the band already exists return */
   if (band) return;

   /* allocate memory for the band */
   band = calloc(1, sizeof(Band));

   /* create the ecore_evas for the band */
   band->ee = ecore_evas_software_x11_new(NULL, 0, 0, 0, 0, 0);

   /* get the band's canvas */
   band->evas = ecore_evas_get(band->ee);

   /* check if we have composite, if not then used shaped */
   if ((ecore_x_composite_query()) && (ecore_x_screen_is_composited(0)))
     ecore_evas_alpha_set(band->ee, 1);
   else
     ecore_evas_shaped_set(band->ee, 1);

   /* create the object which will display the band */
   band->edj = evas_object_image_add(band->evas);
   evas_object_image_file_set(band->edj, 
                              PACKAGE_DATA_DIR"/images/rect.png", NULL);
   evas_object_image_border_set(band->edj, 3, 3, 3, 3);
   evas_object_move(band->edj, 0, 0);
   evas_object_resize(band->edj, 50, 50);
   evas_object_image_fill_set(band->edj, 0, 0, 50, 50);

   /* tell the window manager to ignore this window */
   ecore_evas_override_set(band->ee, 1);

   /* make this window borderless */
   ecore_evas_borderless_set(band->ee, 1);

   /* show the band */
   ecore_evas_show(band->ee);
   evas_object_show(band->edj);
}

static void 
_em_band_move(int x, int y) 
{
   if (!band) return;
   ecore_evas_move(band->ee, x, y);
}

static void 
_em_band_hide(void) 
{
   if (!band) return;
   ecore_evas_hide(band->ee);
}

static void 
_em_band_resize(int w, int h) 
{
   if (!band) return;
   ecore_evas_resize(band->ee, w, h);
   if ((w >= 6) && (h >= 6)) 
     {
	evas_object_resize(band->edj, w, h);
	evas_object_image_fill_set(band->edj, 0, 0, w, h);
     }
   _em_band_show();
}

static void 
_em_grab_region_end(void) 
{
   int x, y, w, h;

   /* delete the event handlers */
   ecore_event_handler_del(key_hdl);
   ecore_event_handler_del(mouse_move_hdl);
   ecore_event_handler_del(mouse_up_hdl);
   ecore_event_handler_del(mouse_down_hdl);

   /* delete the input window */
   ecore_x_window_free(input_window);

   /* get the size of the band */
   ecore_evas_geometry_get(band->ee, &x, &y, &w, &h);

   /* hide the band */
   _em_band_hide();

   /* grab shot */
   _em_take_shot(x, y, w, h);

   /* quit the app */
   ecore_main_loop_quit();
}

static void 
_em_do_app(void) 
{
   Ecore_Exe *exe;
   char buf[PATH_MAX];

   /*
    * If there is a %s in our "app", lets replace it with the filename
    * This will allow a user to specify an app similar to:
    *
    *   /usr/local/bin/myapp arg1 %s arg2 arg3
    *
    * and have %s replaced with the filename emprint is using.
    */
   if (strstr(opts->app, "%s"))
     snprintf(buf, sizeof(buf), opts->app, opts->filename);
   else
     /* otherwise, assemble exe string including app & filename */
     snprintf(buf, sizeof(buf), "%s %s", opts->app, opts->filename);

   /* run the app */
   exe = ecore_exe_run(buf, NULL);
   if (exe) ecore_exe_free(exe);
}

static int 
_em_get_filecount(const char *filename) 
{
   Eina_List *fl = NULL;
   char *dir = NULL, *ext = NULL;
   char *p = NULL, *f = NULL;
   const char *file = NULL;
   int c =  0;

   /* grab dir and file */
   dir = ecore_file_dir_get(filename);
   file = ecore_file_file_get(filename);
   if ((dir) && (file))
     {
        if (!strcmp(dir, file)) dir = getenv("PWD");
     }
   else if (!dir) 
     dir = getenv("PWD");

   /* strip the extension for searches */
   ext = strrchr(file, '.');
   if (!ext) ext = strdup(".png");

   /* strip any %N out of filename */
   p = strrchr(file, '%');
   if (p) *p = 0;

   fl = ecore_file_ls(dir);
   EINA_LIST_FREE(fl, f) 
     {
        if (!strstr(f, ext)) continue;
        if (strncmp(f, file, strlen(file)) >= 0) 
          {
             if (strstr(f, "thumb")) continue;
             if (strstr(f, ext)) c++;
          }

        free(f);
     }

   return c;
}
