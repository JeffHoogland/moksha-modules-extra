#include "e_mod_main.h"


void
ngi_item_init_defaults(Ngi_Item *it)
{
   Ng *ng = it->box->ng;
   int ok = 0;
   
   it->obj = edje_object_add(ng->evas);

   switch(ng->cfg->orient)
     {
      case E_GADCON_ORIENT_BOTTOM:
	 ok = ngi_object_theme_set(it->obj, "e/modules/engage/icon_bottom");
   	 break;
      case E_GADCON_ORIENT_TOP:
	 ok = ngi_object_theme_set(it->obj, "e/modules/engage/icon_top");
   	 break;
      case E_GADCON_ORIENT_LEFT:
	 ok = ngi_object_theme_set(it->obj, "e/modules/engage/icon_left");
   	 break;
      case E_GADCON_ORIENT_RIGHT:
	 ok = ngi_object_theme_set(it->obj, "e/modules/engage/icon_right");
   	 break;
     }

   /* TODO remove fallback */
   if (!ok)
     {
   	if (!e_theme_edje_object_set(it->obj, "base/theme/modules/engage", "e/modules/engage/icon"))
   	  edje_object_file_set(it->obj, ngi_config->theme_path, "e/modules/engage/icon");
     }

   it->over = edje_object_add(ng->evas);

   switch(ng->cfg->orient)
     {
      case E_GADCON_ORIENT_BOTTOM:
	 ok = ngi_object_theme_set(it->over, "e/modules/engage/icon_over_bottom");
   	 break;
      case E_GADCON_ORIENT_TOP:
	 ok = ngi_object_theme_set(it->over, "e/modules/engage/icon_over_top");
   	 break;
      case E_GADCON_ORIENT_LEFT:
	 ok = ngi_object_theme_set(it->over, "e/modules/engage/icon_over_left");
   	 break;
      case E_GADCON_ORIENT_RIGHT:
	 ok = ngi_object_theme_set(it->over, "e/modules/engage/icon_over_right");
   	 break;
     }
   if (!ok)
     {
	if (!e_theme_edje_object_set(it->over, "base/theme/modules/engage", "e/modules/engage/icon_overlay"))
	  edje_object_file_set(it->over, ngi_config->theme_path, "e/modules/engage/icon_overlay");
     }


   evas_object_layer_set(it->over, 9999);
   evas_object_show(it->over);
   evas_object_show(it->obj);
}

void
ngi_item_show(Ngi_Item *it, int instant)
{
   Ng *ng = it->box->ng;

   evas_object_show(it->obj);
   evas_object_show(it->over);
   
   ngi_item_signal_emit(it, "e,state,item,show");

   if (eina_list_data_find(ng->items_remove, it))
     ng->items_remove = eina_list_remove(ng->items_remove, it);

   if (instant)
     {
        it->scale = 1.0;
	ngi_animate(ng);
	return;
     }
   
   it->start_time = ecore_time_get();
   it->scale = 0.0;
   ng->items_show = eina_list_append(ng->items_show, it);
   ngi_animate(ng);
}

void
ngi_item_remove(Ngi_Item *it)
{
   if (!it) return;
     
   Ng *ng = it->box->ng;
   double now = ecore_time_get();
   
   edje_object_signal_emit(it->obj, "e,state,item,hide", "e");

   if (now - it->start_time < ng->opt.fade_duration)
     it->start_time = now - (ng->opt.fade_duration - (now - it->start_time));
   else
     it->start_time = now;
   
   it->delete_me = 1;
   
   if (eina_list_data_find(ng->items_show, it))
     ng->items_show = eina_list_remove(ng->items_show, it);

   if (!eina_list_data_find(ng->items_remove, it))
     ng->items_remove = eina_list_append(ng->items_remove, it);

   if (it == ng->item_active)
      ng->item_active = NULL;

   if (it == ng->item_drag)
      ng->item_drag = NULL;

   ngi_animate(ng);
}

void
ngi_item_free(Ngi_Item *it)
{
   Ng *ng = it->box->ng;

   if (it == ng->item_active)
     ng->item_active = NULL;

   if (it == ng->item_drag)
     ng->item_drag = NULL;

   if (eina_list_data_find(ng->items_remove, it))
     ng->items_remove = eina_list_remove(ng->items_remove, it);

   if (eina_list_data_find(ng->items_show, it))
     ng->items_show = eina_list_remove(ng->items_show, it);

   it->box->items = eina_list_remove(it->box->items, it);

   if (it->obj)
     evas_object_del(it->obj);

   if (it->over)
     evas_object_del(it->over);

   if (it->label)
     eina_stringshare_del(it->label);

   if (it->cb_free) it->cb_free(it);
}

void
ngi_item_label_set(Ngi_Item *it, const char *label)
{
   if (it->label)
     eina_stringshare_del(it->label); 

   if (label)
     it->label = eina_stringshare_add(label);
   else
     it->label = NULL;
}

void
ngi_item_signal_emit(Ngi_Item *it, char *sig)
{
   if (it->obj)
     edje_object_signal_emit(it->obj, sig, "e");
   if (it->over)
     edje_object_signal_emit(it->over, sig, "e");
}

void
ngi_item_mouse_in(Ngi_Item *it)
{
   if (!it || it->delete_me) return;

   ngi_item_signal_emit(it,"e,state,mouse,in");

   if (it->box->ng->cfg->mouse_over_anim)
     ngi_item_signal_emit(it,"e,state,animate,in");

   if (it->cb_mouse_in)
     it->cb_mouse_in(it);
}

void
ngi_item_mouse_out(Ngi_Item *it)
{
   if (!it || it->delete_me) return;

   ngi_item_signal_emit(it,"e,state,mouse,out");

   if (it->box->ng->cfg->mouse_over_anim)
     ngi_item_signal_emit(it,"e,state,animate,out");

   if (it->cb_mouse_out)
     it->cb_mouse_out(it);
}

void
ngi_item_mouse_down(Ngi_Item *it, Ecore_Event_Mouse_Button *ev)
{
   if (!it || it->delete_me) return;

   ngi_item_signal_emit(it,"e,state,mouse,down");

   if (it->cb_mouse_down)
     it->cb_mouse_down(it, ev);
}

void
ngi_item_mouse_up(Ngi_Item *it, Ecore_Event_Mouse_Button *ev)
{
   if (!it || it->delete_me) return;

   ngi_item_signal_emit(it,"e,state,mouse,up");

   if (it->cb_mouse_up)
     it->cb_mouse_up(it, ev);
}

