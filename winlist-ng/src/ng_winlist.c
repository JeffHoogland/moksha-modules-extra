#include "e_mod_main.h"


static void _ngw_winlist_activate               (void);
static void _ngw_winlist_deactivate             (void);
static Eina_Bool  _ngw_winlist_cb_event_border_add    (void *data, int type,  void *event);
static Eina_Bool  _ngw_winlist_cb_event_border_remove (void *data, int type,  void *event);
static Eina_Bool  _ngw_winlist_cb_key_down            (void *data, int type, void *event);
static Eina_Bool  _ngw_winlist_cb_key_up              (void *data, int type, void *event);
static Eina_Bool  _ngw_winlist_cb_mouse_wheel         (void *data, int type, void *event);


static int  _ngw_winlist_warp_timer             (void *data);
static int  _ngw_winlist_animator               (void *data);
static void _ngw_winlist_to_desk                (void);

static void _ngw_winlist_create_list            (int initial);
static void _ngw_winlist_show_desk_toggle       (void);
static void _ngw_winlist_show_iconified_toggle  (void);
static Eina_List *_ngw_winlist_create_border_list(int initial);

static void _ngw_winlist_show_desk_toggle       (void);
static void _ngw_winlist_show_selected_type     (void);
static void _ngw_winlist_show_iconified_toggle  (void);
static void _ngw_winlist_show_all_toggle        (void);

static E_Border *_ngw_winlist_border_above_get  (E_Border *bd);
static int _ngw_winlist_check_border            (E_Border *bd);
static Eina_Bool _ngw_winlist_window_show_timer_cb    (void *data);
static Eina_Bool _ngw_winlist_fade_out_timer          (void *data);
static void _ngw_winlist_move_mouse_to_boder    (E_Border *bd);



static Ng           *winlist = NULL;
static Ngw_Item     *win_selected = NULL;
static int           win_sel_nr;


static E_Border     *last_border = NULL;
static int           hold_count = 0;
static int           hold_mod = 0;
static Eina_List    *handlers = NULL;
static Ecore_X_Window input_window = 0;
static Ecore_X_Window root_win = 0;

static int           warp_to = 0;
static double        warp_to_x = 0;
static double        warp_to_y = 0;
static double        warp_x = 0;
static double        warp_y = 0;

static Ecore_Timer  *warp_timer = NULL;
static Ecore_Timer  *animator = NULL;

static int           items_cnt = 0;

static int           winlist_active = 0; // TODO change to 'state enum'

static int           keep_toggles = 0;
static int           show_all      = 0;
static int           show_all_desk = 0;
static int           show_iconified = 0;
static E_Border     *show_type = NULL;

static int           bring_to_desk = 0;

static double        fade_out_time = 0;
static Ecore_Timer  *fade_out_timer = NULL;
static Ecore_Timer  *show_timer = NULL;
static Ecore_Timer  *activate_timer = NULL;


/*urgh*/
E_Border            *deleted_border = NULL;


static int          fade_step;


static int round_counter;


int
ngw_winlist_init(void)
{
  winlist = ngw_new(ngw_config->winlist_cfg); 
  return 1;
}

int
ngw_winlist_shutdown(void)
{
  ngw_winlist_hide();
  ngw_free(winlist);
  return 1;
}

int
ngw_winlist_show(void)
{
  if(winlist_active) return 0;

   evas_object_color_set(winlist->box->clip, 255, 255, 255, 255);

   winlist->box->icon_size = winlist->cfg->size;
 
   if(fade_out_timer)
    {
      ngw_remove_items(winlist); 
     
      ecore_timer_del(fade_out_timer);       
      fade_out_timer = NULL;
    }
   
  winlist->zone = e_util_zone_current_get(e_manager_current_get());
   
  input_window = ecore_x_window_input_new(winlist->zone->container->win, 0, 0, 1, 1);
  ecore_x_window_show(input_window);
  
  if(!e_grabinput_get(input_window, 0, input_window))
    {
      ecore_x_window_free(input_window);
      input_window = 0;
      return 0;
    }
  
  round_counter = 0;

  E_Zone *zone = winlist->zone;
  
  if(!keep_toggles)
    {    
      show_all = 1;
      show_all_desk = 2;
      show_iconified = 2;
    }
  
  _ngw_winlist_create_list(1);

  
    if(!items_cnt && keep_toggles) //TODO config
    {
      show_all = 1;
      show_iconified = 1;
      show_all_desk = 2;
  
      //ngw_remove_items(winlist); 
      _ngw_winlist_create_list(1);
    }
    /*
  if(items_cnt < 1) //TODO config
    {
      show_iconified = 1;
  
      ngw_remove_items(winlist); 
      _ngw_winlist_create_list(1);
    }
  */
  if(!winlist->items)
    {
      ecore_x_window_free(input_window);
      input_window = 0;

      show_iconified = 0;
      show_all_desk = 0;
      
      return 0;
    }

   int size = 240;
   
   winlist->win->w = zone->w;
   winlist->win->h = size;
   winlist->win->x = zone->x;
   winlist->win->y = (zone->h - size) / 2 + zone->y;
   
   winlist->box->win_w = zone->w;
   winlist->box->win_h = size;
   
  e_popup_move_resize(winlist->win->popup, winlist->win->x, winlist->win->y, winlist->win->w, winlist->win->h);
  
  _ngw_winlist_activate();

  // FIXME last_border needed?
  last_border = e_border_focused_get();

  handlers = eina_list_append
    (handlers, ecore_event_handler_add
     (E_EVENT_BORDER_ADD, _ngw_winlist_cb_event_border_add, NULL));
  handlers = eina_list_append
    (handlers, ecore_event_handler_add
     (E_EVENT_BORDER_REMOVE, _ngw_winlist_cb_event_border_remove, NULL));
  handlers = eina_list_append
    (handlers, ecore_event_handler_add
     (ECORE_EVENT_KEY_DOWN, _ngw_winlist_cb_key_down, NULL));
  handlers = eina_list_append
    (handlers, ecore_event_handler_add
     (ECORE_EVENT_KEY_UP, _ngw_winlist_cb_key_up, NULL));
  handlers = eina_list_append
     (handlers, ecore_event_handler_add
      (ECORE_EVENT_MOUSE_WHEEL, _ngw_winlist_cb_mouse_wheel, NULL));

  show_timer = ecore_timer_add(0.1, _ngw_winlist_window_show_timer_cb, NULL);

  winlist_active = 1;
 

  return 1;
}

void
ngw_winlist_hide(void)
{
  E_Border *bd = NULL;
  
  if(!winlist_active) return;
  
  while(handlers)
    {
      ecore_event_handler_del(handlers->data);
      handlers = eina_list_remove_list(handlers, handlers);
    }

  ecore_x_window_free(input_window);
  e_grabinput_release(input_window, input_window);
  input_window = 0;

  if(activate_timer) ecore_timer_del(activate_timer);
  activate_timer = NULL;
  

  if(warp_timer)
    {
      ecore_timer_del(warp_timer);
      warp_timer = NULL;
    }

  if(animator)
    {
      ecore_timer_del(animator);
      animator = NULL;
    }
  
  if(win_selected)
    bd = win_selected->border;

  if(bd)
    {
      e_desk_show(bd->desk);     
      e_border_raise(bd);
 
      if(bd->iconic)
	{
	  if(!bd->lock_user_iconify)
	    e_border_uniconify(bd);
	}
      if(bd->shaded)
	{
	  if(!bd->lock_user_shade)
	    e_border_unshade(bd, bd->shade.dir);
	}
      
      if(!bd->lock_focus_out)
	{
	  e_border_focus_set(bd, 1, 1);
	  e_border_focus_latest_set(bd);
	}
      
      if(e_config->focus_policy != E_FOCUS_CLICK) // TODO make this configurable
        _ngw_winlist_move_mouse_to_boder(bd);
    }

  if(show_timer)
    {
      ecore_timer_del(show_timer);
      show_timer = NULL;

      ngw_remove_items(winlist); 

      ngw_win_hide(winlist->win); // FIXME ???
    }
  else if(ngw_config->use_composite && ngw_config->winlist_cfg->hide_animation)
    {    
      fade_out_time = ecore_time_get();
      fade_out_timer = ecore_timer_add(0.02, _ngw_winlist_fade_out_timer, NULL);
      edje_object_signal_emit(winlist->box->o_bg, "label_hide", "e");
    }
  else // TODO: option:  no hide animation
    {
      ngw_remove_items(winlist); 
      ngw_win_hide(winlist->win);
    }
  
  win_selected = NULL;
  winlist_active = 0;

  if(!keep_toggles)
    {    
      show_all = 0;  
      show_all_desk = 0;
    }
  
  show_iconified = 0;
  bring_to_desk = 0;
  show_type = NULL;

  hold_count = 0;
  hold_mod = 0;
}

int
ngw_winlist_active_get(void)
{
   if (winlist) return 1;
   return 0;
}

/* TODO REMOVE THESE TWO */
void
ngw_winlist_next(void)
{
  if(!winlist) return;
  if(items_cnt <= 1) return;
   
  _ngw_winlist_deactivate();

  win_sel_nr = (win_sel_nr + 1) % items_cnt;
  win_selected = (Ngw_Item*) eina_list_nth(winlist->items, win_sel_nr);
  ngw_box_position_to_item(winlist->box, win_selected);
      
  _ngw_winlist_activate();
}

void
ngw_winlist_prev(void)
{
  if(!winlist) return;
  if(items_cnt == 1) return;

  _ngw_winlist_deactivate();

  win_sel_nr -= 1;
  if(win_sel_nr < 0) win_sel_nr = items_cnt - 1;
    
  win_selected = (Ngw_Item*) eina_list_nth(winlist->items, win_sel_nr);
  ngw_box_position_to_item(winlist->box, win_selected);
  
  _ngw_winlist_activate();
}

void
ngw_winlist_goto(int dir)
{
  if(!winlist) return;
  if(items_cnt == 1) return;

  _ngw_winlist_deactivate();

  /* FIXME works just for positive nums */
  win_sel_nr = (win_sel_nr + dir) % items_cnt;

  win_selected = (Ngw_Item*) eina_list_nth(winlist->items, win_sel_nr);
  ngw_box_position_to_item(winlist->box, win_selected);
  
  _ngw_winlist_activate();
}

void
ngw_winlist_modifiers_set(int mod)
{
  // if (!winlist) return;
   hold_mod = mod;
   hold_count = 0;
   if (hold_mod & ECORE_EVENT_MODIFIER_SHIFT) hold_count++;
   if (hold_mod & ECORE_EVENT_MODIFIER_CTRL) hold_count++;
   if (hold_mod & ECORE_EVENT_MODIFIER_ALT) hold_count++;
   if (hold_mod & ECORE_EVENT_MODIFIER_WIN) hold_count++;
}

static void
_ngw_winlist_show_all_toggle(void)
{
  //if(show_all) return;

  show_all = 1;

  show_all_desk = 2;
  show_iconified = 2;
  show_type = NULL;

  _ngw_winlist_deactivate();
  
  if(eina_list_count(_ngw_winlist_create_border_list(0)) == 0)
    {
      ngw_winlist_hide();
      return;
    }
   
  ngw_remove_items(winlist); 
  
  _ngw_winlist_create_list(0);
}

static void
_ngw_winlist_show_desk_toggle(void)
{
  show_all_desk = (show_all_desk + 1) % 3;
  
  show_iconified = 0;
  show_all = 0;
  show_type = NULL;

  _ngw_winlist_deactivate();
      
  if(eina_list_count(_ngw_winlist_create_border_list(0)) == 0)
    {
      if(!show_all_desk) 
        show_all_desk = 2;

      if(eina_list_count(_ngw_winlist_create_border_list(0)) == 0)
        ngw_winlist_hide();
      
      return;
    }
   
  ngw_remove_items(winlist); 
    
  _ngw_winlist_create_list(0);
}

static void
_ngw_winlist_show_iconified_toggle(void)
{
  if(show_iconified && !show_all) return;
 
  show_iconified = 1; //show_iconified ? 0 : 1;

  show_all_desk = 0;
  show_all = 0;
  show_type = NULL;

  _ngw_winlist_deactivate();

  if(eina_list_count(_ngw_winlist_create_border_list(0)) == 0)
    {
      ngw_winlist_hide();
      return;
    }

  ngw_remove_items(winlist); 
  
  _ngw_winlist_create_list(0);
}

static void 
_ngw_winlist_show_selected_type(void)
{
  if(show_type || !win_selected) 
    {
      return;
    }

  if(!show_type) // FIXME set this back, if the border gets removed
    {
      show_type = win_selected->border;
    }
  _ngw_winlist_deactivate();
   
  ngw_remove_items(winlist); 
  
  _ngw_winlist_create_list(0);
}

static E_Border *
_ngw_winlist_border_above_get(E_Border *bd)
{
  Eina_List *l;
  E_Border *above;
  
  for(l = eina_list_data_find_list(e_border_focus_stack_get(), bd); l; l = l->prev)
    {
      if(!l->prev) break;
      l = l->prev;
      
      above = (E_Border*)l->data;
        
      if(above->desk == bd->desk)
	{
  	  return above;
	}
    }
  return NULL;
}

#define CHECK(expr)   { if(expr) return 1; else return 0; }
#define CHECK_0(expr) { if(expr) return 0; }

static int
_ngw_winlist_check_border(E_Border *bd)
{
  E_Desk *desk;

  CHECK_0 (deleted_border && bd == deleted_border);
  
  CHECK_0 ((!bd->client.icccm.accepts_focus) &&
           (!bd->client.icccm.take_focus));
  
  CHECK_0 (bd->client.netwm.state.skip_taskbar);
  
  CHECK_0 (bd->user_skip_winlist);
 
  CHECK_0 (bd->zone != winlist->zone);
  
  if(show_type) 
    {
      CHECK(bd->client.icccm.class &&
            !strcmp(bd->client.icccm.class, show_type->client.icccm.class));          
    }
  
  CHECK_0(!show_iconified && bd->iconic);
  
  if(show_iconified && !show_all)
    {
      CHECK(bd->iconic);
    }
  
  desk = e_desk_current_get(winlist->zone);

  if(show_all_desk == 1)
    {
      CHECK(bd->desk != desk);
    }
 else if(bd->desk != desk && !bd->sticky)
    {
      CHECK(show_all_desk == 2);    
    }  

  return 1;
}

static Eina_List *
_ngw_winlist_create_border_list(int initial)
{
  Eina_List *l, *borders = NULL;
  E_Border *bd;
  int cnt = 0;
  
  for (l = e_border_focus_stack_get(); l; l = l->next)
    {     
      bd = (E_Border*)l->data;
 
      if (_ngw_winlist_check_border(bd))
	{
          borders = eina_list_append(borders, bd);

          if(cnt++ > 10 && initial) // TODO config option
            {
              return borders;
            }
	}
    }
  return borders;
}

static void
_ngw_winlist_create_list(int initial)
{
  Eina_List *l, *borders = NULL;
  E_Border *bd;
  E_Border *bd_last = NULL;  
  Ngw_Item *it;
  int cnt = 0;
      
  borders = _ngw_winlist_create_border_list(initial);

  items_cnt = eina_list_count(borders);

  for(l = eina_list_last(borders); l && cnt < (items_cnt / 2 - (initial ? 1 : 0)); l = l->prev, cnt++)
    {
      bd = (E_Border*)l->data;
      it = ngw_item_new(winlist, bd);

      winlist->items = eina_list_prepend(winlist->items, it);
      bd_last = bd;
    }

  for(l = borders; l && cnt < items_cnt; l = l->next, cnt++)
    {
      bd = (E_Border*)l->data;
      it = ngw_item_new(winlist, bd);
      
      winlist->items = eina_list_append(winlist->items, it);
      if(!bd_last) bd_last = bd;
    }

  while(borders)
    borders = eina_list_remove_list(borders,borders);

  if(items_cnt == 0) 
    {
      return;
    }

  ngw_box_reposition(winlist->box);

  win_sel_nr = items_cnt/2;
  win_selected = (Ngw_Item*)eina_list_nth(winlist->items, win_sel_nr);

  ngw_box_position_to_item(winlist->box, win_selected);
  _ngw_winlist_activate();
}

static Eina_Bool 
_ngw_winlist_window_show_timer_cb(void *data)
{
  show_timer = NULL;

  if(winlist_active)
    ngw_win_show(winlist->win);

  fade_step = 5; // TODO: Config
  
  return EINA_FALSE;
}

static void
_ngw_winlist_to_desk(void)
{
  Ngw_Item *it;
  E_Border *bd;
  
  if(!win_selected) return; 

  it = win_selected;
  bd = it->border;

  if(it->desk)
    {
      e_border_desk_set(bd, it->desk);
      it->desk = NULL;
    }
  else if(it->border->desk != e_desk_current_get(winlist->zone))
    {
      it->desk = bd->desk;
      e_border_desk_set(bd, e_desk_current_get(winlist->zone));
      e_border_raise(bd);
    }
}

static Eina_Bool
_ngw_winlist_fade_out_timer(void *data)
{
  int a;
  evas_object_color_get(winlist->box->clip, NULL, NULL, NULL, &a);
  
  a -= 255 / fade_step;
  fade_step--;
  
  if(a > 0)
    { 
      evas_object_color_set(winlist->box->clip, a, a, a, a);

      if(ngw_config->winlist_cfg->hide_animation == 2)
        winlist->box->icon_size -= 6; 
    }
  else
    {
      ngw_win_hide(winlist->win);
      evas_object_color_set(winlist->box->clip, 255, 255, 255, 255);
      
      ngw_remove_items(winlist); 
            
      fade_out_timer = NULL;
      return EINA_FALSE;
    }
 
  if(fade_out_timer &&  ecore_time_get() - fade_out_time > 0.08) 
    {
      ecore_timer_del(fade_out_timer);
      fade_out_timer = ecore_timer_add(0.001, _ngw_winlist_fade_out_timer, NULL);
    }

  fade_out_time = ecore_time_get();
  
  return EINA_TRUE;
}

static void 
_ngw_winlist_move_mouse_to_boder(E_Border *bd)
{
  int wx,wy;
          
  warp_to_x = bd->x + (bd->w / 2); 
  if (warp_to_x < (bd->zone->x + 1))
    warp_to_x = bd->zone->x + ((bd->x + bd->w - bd->zone->x) / 2);
  else if (warp_to_x >= (bd->zone->x + bd->zone->w - 1))
    warp_to_x = (bd->zone->x + bd->zone->w + bd->x) / 2; 
     
  warp_to_y = bd->y + (bd->h / 2);
  if (warp_to_y < (bd->zone->y + 1))
    warp_to_y = bd->zone->y + ((bd->y + bd->h - bd->zone->y) / 2);
  else if (warp_to_y >= (bd->zone->y + bd->zone->h - 1))
    warp_to_y = (bd->zone->y + bd->zone->h + bd->y) / 2; 
	  
  ecore_x_pointer_xy_get(winlist->zone->container->win, &wx, &wy);
  warp_x = wx;
  warp_y = wy;
	  
  root_win = winlist->zone->container->win;
	  
  warp_to = 1;

  ecore_x_pointer_warp(root_win, warp_to_x, warp_to_y);
}

static void
_ngw_winlist_set_label(Ng *ng, Ngw_Item *it)
{
  const char *title;

  if(it->label)
    {
      edje_object_part_text_set(ng->box->o_bg, "e.text.label", it->label);
    }
  
  title = e_border_name_get(it->border);

  if ((!title) || (!title[0]))
    title  = D_("No name!");

  int max_len = 36;
  
  if (strlen(title) > max_len)
    {
      char *abbv;
      const char *left, *right;

      abbv = (char*) calloc(E_CLIENTLIST_MAX_CAPTION_LEN + 4, sizeof(char)); // TODO use an own config var
      left = title;
      right = title + (strlen(title) - (max_len / 2));

      strncpy(abbv, left, max_len / 2);
      strncat(abbv, "...", 3);
      strncat(abbv, right, max_len / 2);

      it->label =  abbv;
      edje_object_part_text_set(ng->box->o_bg, "e.text.label", abbv);
    }
  else
    {
      edje_object_part_text_set(ng->box->o_bg, "e.text.label", title);
      it->label = strdup(title);
    }
}

static void
_ngw_winlist_bring_to_front(void)
{
  Ngw_Item *it = win_selected;
  int ok;
  E_Border *bd;

  ok = 0;
  bd = it->border;

  it->bd_above = _ngw_winlist_border_above_get(bd);
   
  if(bd->iconic) /* this is a bit senseless: */ 
    {
      e_border_uniconify(bd);
      it->was_iconified = 1;
      ok = 1;
    }
  else if((bd->desk == e_desk_current_get(winlist->zone)) || bd->sticky)
    {
      ok = 1;
    }
  else if(bring_to_desk) 
    {
      _ngw_winlist_to_desk();
      ok = 1;
    }
  
  if((bd->shaded || (bd->changes.shaded && (bd->shade.val != bd->shaded) && bd->shade.val)) &&
      bd->desk == e_desk_current_get(winlist->zone))
    {
      e_border_unshade(it->border, it->border->shade.dir);
      it->was_shaded = 1;
      ok = 1;
    }
    
  if(ok)
    {
      _ngw_winlist_move_mouse_to_boder(bd);
	
      if (!it->border->lock_user_stacking) 
        e_border_raise(it->border);
      if (!it->border->lock_focus_out)
        e_border_focus_set(it->border, 1, 1);
    }
}

static Eina_Bool
_ngw_winlist_bring_to_front_cb(void *data)
{
  _ngw_winlist_bring_to_front();
  activate_timer = NULL;
  
  return EINA_FALSE;
}

static void
_ngw_winlist_activate(void)
{
  Ngw_Item *it;
  
  if (!win_selected) return;
  it = win_selected;

  if(activate_timer) ecore_timer_del(activate_timer);
  activate_timer = ecore_timer_add(0.3, _ngw_winlist_bring_to_front_cb, NULL);
  
  _ngw_winlist_set_label(winlist, it);
  edje_object_signal_emit(winlist->box->o_bg, "label_show", "e");
  ngw_item_signal_emit(it, "active");
}

static void
_ngw_winlist_deactivate(void)
{
  Ngw_Item *it;
  
  if (!win_selected) return;

  it = win_selected;
  win_selected = NULL;

  ngw_item_signal_emit(it, "inactive");

  if(activate_timer) ecore_timer_del(activate_timer);
  activate_timer = NULL;
  
  if(it->set_state)
    {
      it->was_shaded = 0;
      it->was_iconified = 0;
      return;  
    }
 
  if(it->was_shaded)
    {
      if (!it->border->lock_user_shade)
        e_border_shade(it->border, it->border->shade.dir);
    }

  if(it->was_iconified)
    {
      if (!it->border->lock_user_iconify)
        e_border_iconify(it->border);
    }

  it->was_shaded = 0;
  it->was_iconified = 0;
  //if(!it->desk && it->bd_above)
  if(it->bd_above)  /* FIXME stack above issues*/
    e_border_stack_below(it->border, it->bd_above);
 
  if(it->desk && it->desk != e_desk_current_get(winlist->zone))
    {
      e_border_desk_set(it->border, it->desk);
      it->desk = NULL;      
    }
 
  if (!it->border->lock_focus_in)
    e_border_focus_set(it->border, 0, 0);

}

static Eina_Bool
_ngw_winlist_cb_event_border_add(void *data, int type,  void *event)
{
  E_Event_Border_Add *ev;

  ev = (E_Event_Border_Add*) event;

  if(!_ngw_winlist_check_border(ev->border))
    return EINA_TRUE;
   
  _ngw_winlist_deactivate();

  ngw_remove_items(winlist); 
  
  _ngw_winlist_create_list(0);
   
  if(!winlist->items)
    {
      ngw_winlist_hide();
      return EINA_TRUE;
    }
  
  _ngw_winlist_activate();

  return EINA_TRUE;
}

static Eina_Bool
_ngw_winlist_cb_event_border_remove(void *data, int type,  void *event)
{
  E_Event_Border_Remove *ev;

  ev = (E_Event_Border_Remove*) event;
  Eina_List *l;
  Ngw_Item *tmp;
  
  if(!winlist_active) return EINA_TRUE;

  for(l = winlist->items; l; l = l->next)
    {
      tmp = (Ngw_Item*) l->data;
      if(tmp->border == ev->border)
        {
          win_selected = NULL;
   
          ngw_remove_items(winlist); 

          deleted_border = ev->border;
          _ngw_winlist_create_list(0);
          deleted_border = NULL;

          if(!winlist->items)
            {
              ngw_winlist_hide();
              return EINA_TRUE;
            }
  
          _ngw_winlist_activate();
        }      
    }
  return EINA_TRUE;
}

static Eina_Bool
_ngw_winlist_cb_key_down(void *data, int type, void *event)
{
  Ecore_Event_Key *ev;
   
  ev = (Ecore_Event_Key*) event;
  if (ev->window != input_window) return EINA_TRUE;

  if(!strcmp(ev->key, "s"))
    {
      bring_to_desk = (bring_to_desk ? 0 : 1);  
      _ngw_winlist_to_desk();
    }
  else if (!strcmp(ev->key, "a"))
    _ngw_winlist_show_all_toggle();
  else if (!strcmp(ev->key, "d"))
    _ngw_winlist_show_desk_toggle();
  else if (!strcmp(ev->key, "D"))
    {
      keep_toggles = keep_toggles ?  0 : 1;
      _ngw_winlist_show_desk_toggle();
    }
  else if (!strcmp(ev->key, "i"))
    _ngw_winlist_show_iconified_toggle();
  else if (!strcmp(ev->key, "y"))
    _ngw_winlist_show_iconified_toggle();
  else if (!strcmp(ev->key, "j"))
    ngw_winlist_prev();
  else if (!strcmp(ev->key, "k"))
    ngw_winlist_next();
  else if (!strcmp(ev->key, "c"))
     _ngw_winlist_show_selected_type();
  else if (!strcmp(ev->key, "w"))
     _ngw_winlist_show_selected_type();
  else if (!strcmp(ev->key, "q"))
    {
      if(win_selected)
        win_selected->set_state = 1;
    }
  else if (!strcmp(ev->key, "Escape"))
    {
      _ngw_winlist_deactivate();
      win_selected = NULL;
      ngw_winlist_hide();
    }
  else if (!strcmp(ev->key, "1"))
     ngw_winlist_goto(1);
  else if (!strcmp(ev->key, "2"))
     ngw_winlist_goto(2);
  else if (!strcmp(ev->key, "3"))
     ngw_winlist_goto(3);
  else if (!strcmp(ev->key, "4"))
     ngw_winlist_goto(4);
  else if (!strcmp(ev->key, "5"))
     ngw_winlist_goto(5);
  else
    {
      E_Action *act;
      Eina_List *l;
      E_Config_Binding_Key *bind;
      E_Binding_Modifier mod;

      for (l = e_config->key_bindings; l; l = l->next)
        {
          bind = l->data;

          if (bind->action && 
              strcmp(bind->action, "ngw_winlist_prev") && 
              strcmp(bind->action, "ngw_winlist_next")) continue;

          mod = 0;

          if (ev->modifiers & ECORE_EVENT_MODIFIER_SHIFT) mod |= E_BINDING_MODIFIER_SHIFT;
          if (ev->modifiers & ECORE_EVENT_MODIFIER_CTRL) mod |= E_BINDING_MODIFIER_CTRL;
          if (ev->modifiers & ECORE_EVENT_MODIFIER_ALT) mod |= E_BINDING_MODIFIER_ALT;
          if (ev->modifiers & ECORE_EVENT_MODIFIER_WIN) mod |= E_BINDING_MODIFIER_WIN;

          if (bind->key && (!strcmp(bind->key, ev->keyname)) && ((bind->modifiers == mod))) 
            {	
              act = e_action_find(bind->action);
		  
              if(!act) continue;

              if (act->func.go_key)
                act->func.go_key(E_OBJECT(winlist->zone), bind->params, ev); 
              else if (act->func.go)
                act->func.go(E_OBJECT(winlist->zone), bind->params); 

            }
        }
    }
  return EINA_TRUE;
}

static Eina_Bool
_ngw_winlist_cb_key_up(void *data, int type, void *event)
{
  Ecore_Event_Key *ev;

  ev = (Ecore_Event_Key*) event;
  if (!winlist_active) return EINA_TRUE;
  
  if (hold_mod)
    {
      if      ((hold_mod & ECORE_EVENT_MODIFIER_SHIFT) && (!strcmp(ev->key, "Shift_L"))) hold_count--;
      else if ((hold_mod & ECORE_EVENT_MODIFIER_SHIFT) && (!strcmp(ev->key, "Shift_R"))) hold_count--;
      else if ((hold_mod & ECORE_EVENT_MODIFIER_CTRL) && (!strcmp(ev->key, "Control_L"))) hold_count--;
      else if ((hold_mod & ECORE_EVENT_MODIFIER_CTRL) && (!strcmp(ev->key, "Control_R"))) hold_count--;
      else if ((hold_mod & ECORE_EVENT_MODIFIER_ALT) && (!strcmp(ev->key, "Alt_L"))) hold_count--;
      else if ((hold_mod & ECORE_EVENT_MODIFIER_ALT) && (!strcmp(ev->key, "Alt_R"))) hold_count--;
      else if ((hold_mod & ECORE_EVENT_MODIFIER_ALT) && (!strcmp(ev->key, "Meta_L"))) hold_count--;
      else if ((hold_mod & ECORE_EVENT_MODIFIER_ALT) && (!strcmp(ev->key, "Meta_R"))) hold_count--;
      else if ((hold_mod & ECORE_EVENT_MODIFIER_ALT) && (!strcmp(ev->key, "Super_L"))) hold_count--;
      else if ((hold_mod & ECORE_EVENT_MODIFIER_ALT) && (!strcmp(ev->key, "Super_R"))) hold_count--;
      else if ((hold_mod & ECORE_EVENT_MODIFIER_WIN) && (!strcmp(ev->key, "Super_L"))) hold_count--;
      else if ((hold_mod & ECORE_EVENT_MODIFIER_WIN) && (!strcmp(ev->key, "Super_R"))) hold_count--;
      else if ((hold_mod & ECORE_EVENT_MODIFIER_WIN) && (!strcmp(ev->key, "Mode_switch"))) hold_count--;
      else if ((hold_mod & ECORE_EVENT_MODIFIER_WIN) && (!strcmp(ev->key, "Meta_L"))) hold_count--;
      else if ((hold_mod & ECORE_EVENT_MODIFIER_WIN) && (!strcmp(ev->key, "Meta_R"))) hold_count--;

      if (hold_count <= 0)
	{
	  ngw_winlist_hide();
	  return EINA_TRUE;
	}
    }
  e_bindings_key_up_event_handle(E_BINDING_CONTEXT_WINLIST,
				 E_OBJECT(winlist->zone), ev);
  return EINA_TRUE;
}

static int
_ngw_winlist_warp_timer(void *data)
{
  if (warp_to)
    {
      double x, y;
      double spd;
	
      spd = e_config->winlist_warp_speed;
      x = warp_x;
      y = warp_y;
      warp_x = (x * (1.0 - spd)) + (warp_to_x * spd);
      warp_y = (y * (1.0 - spd)) + (warp_to_y * spd);
      return 1;
    }
  warp_timer = NULL;
  return 0;
}

static int
_ngw_winlist_animator (void *data)
{
  if (warp_to)
    {
      int dx, dy;
	
      dx = warp_x - warp_to_x;
      dy = warp_y - warp_to_y;
      dx = dx * dx;
      dy = dy * dy;
      if ((dx <= 1) && (dy <= 1))
        {
          warp_x = warp_to_x;
          warp_y = warp_to_y;
          warp_to = 0;
        }
      ecore_x_pointer_warp(root_win, warp_x, warp_y);
    }
  if (warp_to) return 1;
  animator = NULL;
  root_win = 0;
   
  return 0;
}

static Eina_Bool
_ngw_winlist_cb_mouse_wheel(void *data, int type, void *event)
{
   Ecore_Event_Mouse_Wheel *ev;
   
   ev = event;
   if (ev->window != input_window) return EINA_TRUE;
   //e_bindings_wheel_event_handle(E_BINDING_CONTEXT_ANY,
   //				 E_OBJECT(winlist->zone), ev);
   if (ev->z < 0) /* up */
     {
	int i;
	
	for (i = ev->z; i < 0; i++) ngw_winlist_prev();
     }
   else if (ev->z > 0) /* down */
     {
	int i;
	
	for (i = ev->z; i > 0; i--) ngw_winlist_next();
     }
   return EINA_TRUE;
}

