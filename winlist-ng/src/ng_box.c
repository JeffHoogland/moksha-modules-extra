#include "e_mod_main.h"

#define ITEM_SPACING 8

static void  _ngw_box_zoom_function     (Ngw_Box *box, double d, double *zoom, double *disp);
static void  _ngw_box_redraw            (Ngw_Box *box);



Ngw_Box *
ngw_box_new(Ng *ng)
{
  Ngw_Box *box;

  box = E_NEW(Ngw_Box, 1);
  if (!box) return NULL;

  box->ng = ng;
  box->win_w = ng->win->w;
  box->win_h = ng->win->h;
  box->o_bg = NULL;
    box->zoom = 1.0;

  box->w = 0;
  box->h = 0;
  box->pos = 0;
  box->icon_size = ng->cfg->size;

  box->clip = evas_object_rectangle_add(ng->win->evas);
  evas_object_move(box->clip, -100004, -100004);
  evas_object_resize(box->clip, 200008, 200008);
  evas_object_color_set(box->clip, 255, 255, 255, 255);
  evas_object_show(box->clip);

  box->o_bg = edje_object_add(ng->win->evas);
  box->o_over = NULL; 
  
  if(ngw_config->use_composite)
    {
      box->o_over =  edje_object_add(ng->win->evas);
      
      
      if (!e_theme_edje_object_set(box->o_bg, "base/theme/modules/ng", "e/modules/ng/bg_composite"))
        edje_object_file_set(box->o_bg, ngw_config->theme_path, "e/modules/ng/bg_composite");
      if (!e_theme_edje_object_set(box->o_over, "base/theme/modules/ng", "e/modules/ng/over"))
        edje_object_file_set(box->o_over, ngw_config->theme_path, "e/modules/ng/over");

    evas_object_layer_set(box->o_over, 100);
  evas_object_clip_set(box->o_over, box->clip);

  evas_object_show(box->o_over);
     
    }
  else
    {  
      if (!e_theme_edje_object_set(box->o_bg, "base/theme/modules/ng", "e/modules/ng/bg"))
        edje_object_file_set(box->o_bg, ngw_config->theme_path, "e/modules/ng/bg");
    }
  
  evas_object_layer_set(box->o_bg, -1);

  evas_object_clip_set(box->o_bg, box->clip);

  evas_object_show(box->o_bg);

  box->zoom = ng->cfg->zoomfactor;

  return box;
}

void 
ngw_box_free(Ngw_Box *box)
{
  if (box->o_bg) evas_object_del(box->o_bg);
  if (box->clip) evas_object_del(box->clip);

  free(box);
}

void 
ngw_box_item_show(Ngw_Box *box, Ngw_Item *it)
{
  it->scale = 1.0;
  evas_object_clip_set(it->obj, box->clip);
}

void 
ngw_box_item_remove(Ngw_Box *box, Ngw_Item *it)
{
  evas_object_clip_unset(it->obj);
  ngw_item_free(it);

  _ngw_box_redraw(box);
}

void
ngw_box_position_to_item(Ngw_Box *box, Ngw_Item  *item)
{
   if (!box) return;
   if (!item) return;
   
   box->pos = item->x; // - (box->icon_size + ITEM_SPACING)/2;
  _ngw_box_redraw(box);
}

void 
ngw_box_reposition(Ngw_Box *box)
{
  double x;
  Eina_List *l;
  Ngw_Item *it;
 
  int size = box->icon_size + ITEM_SPACING;
  
  box->w = eina_list_count(box->ng->items) * (box->icon_size + ITEM_SPACING);
  box->h = box->win_h;

  x = 0.5 * (box->win_w - box->w);

  for (l = box->ng->items; l; l = l->next)
    {
      it = (Ngw_Item*) l->data;

      x += 0.5 *  (it->scale * size);
      it->x = (int) x;
      x += 0.5 *  (it->scale * size);
   }
}

static void 
_ngw_box_zoom_function(Ngw_Box *box, double d, double *zoom, double *disp)
{
  double  range, f, x, ff, sqrt_ffxx, sqrt_ff_1;

  Config_Item *cfg = box->ng->cfg;

  range = cfg->zoom_range;
  f = 2.5;
  x = d / range;

  ff = f * f;
  sqrt_ffxx = sqrt(ff - x * x);
  sqrt_ff_1 = sqrt(ff - 1.0);

  if (d > -range && d < range)
    {
      *zoom = (box->zoom - 1.0) * (cfg->zoomfactor - 1.0) *
	((sqrt_ff_1 - sqrt_ffxx) / (sqrt_ff_1 - f)) + 1.0;

      *disp = (box->icon_size + ITEM_SPACING)
           * ((box->zoom - 1.0) * (cfg->zoomfactor - 1.0)
	   * (range * (x * (2 * sqrt_ff_1 - sqrt_ffxx) -
	     ff * atan(x / sqrt_ffxx)) / (2.0 * (sqrt_ff_1 - f))) + d);
    }
  else
    {
      *zoom = 1.0;

      *disp = (box->icon_size + ITEM_SPACING) *
	((box->zoom - 1.0) * (cfg->zoomfactor - 1.0) *
	 (range * (sqrt_ff_1 - ff * atan(1.0 / sqrt_ff_1)) /
	  (2.0 * (sqrt_ff_1 - f))) + range + fabs(d) - range);
      if (d < 0.0)
	*disp = -(*disp);
    }
}

static void _ngw_box_redraw(Ngw_Box *box)
{
  double zoom, disp, distance, pos;
  int size, end1, end2, size_spacing;
  Ngw_Item *it;

  Config_Item *cfg = box->ng->cfg;
  Eina_List *l = box->ng->items;
 
  size_spacing = box->icon_size + ITEM_SPACING;
 
  l = box->ng->items;

  if(l)
    {
      box->zoom = cfg->zoomfactor;
      int width = eina_list_count(l) * size_spacing;
      int start = (int) (0.5 * (box->win_w - width));

      distance = (double)(start - box->win_w / 2) / size_spacing;
      _ngw_box_zoom_function(box, distance, &zoom, &disp);

      end1 = (int)((box->win_w / 2 + disp) - (size_spacing / 2));
      end2 = box->win_w - end1;

      evas_object_move(box->o_bg, end1 - 15,(int)(box->win_h - box->icon_size * (cfg->zoomfactor + 0.5)) / 2 - 15);

      evas_object_resize(box->o_bg, end2 - end1 + 30,
                         (int) (box->icon_size * (cfg->zoomfactor + 0.5) + 40)); // FIXME * item_zoomed
      if(box->o_over)
        {
          
          evas_object_move(box->o_over, end1 - 15,(int)(box->win_h - box->icon_size * (cfg->zoomfactor + 0.5)) / 2 - 15);

          evas_object_resize(box->o_over, end2 - end1 + 30,
                             (int) (box->icon_size * (cfg->zoomfactor + 0.5) + 40)); // FIXME * item_zoomed
        }
      


    }
  for (; l; l = l->next)
    {
      it = (Ngw_Item*) l->data;
      distance = (double)(it->x - box->pos) / size_spacing; 
  
      box->zoom = cfg->zoomfactor;
      _ngw_box_zoom_function(box, distance, &zoom, &disp);

      size = (int)(it->scale * zoom * box->icon_size);
            
      pos = (box->pos + disp) - (size / 2);

      evas_object_move(it->obj,  (Evas_Coord) pos, (Evas_Coord)(box->win_h - size) / 2);

      evas_object_resize(it->obj, size, size);
    }
}


