#include "e_mod_main.h"

static void     _ngw_item_del_icon(Ngw_Item *ic);
static void     _ngw_item_border_set_icon(Ngw_Item *it);


Evas_Object *
_ng_border_icon_add(E_Border *bd, Evas *evas)
{
  Evas_Object *o;

  E_OBJECT_CHECK_RETURN(bd, NULL);
  E_OBJECT_TYPE_CHECK_RETURN(bd, E_BORDER_TYPE, NULL);

  o = NULL;
  if (bd->internal)
    {
      o = edje_object_add(evas);
      if (!bd->internal_icon)
	e_util_edje_icon_set(o, "enlightenment/e");
      else
	{
	  if (!bd->internal_icon_key)
	    {
	      char *ext;
	      ext = strrchr(bd->internal_icon, '.');
	      if ((ext) && ((!strcmp(ext, ".edj"))))
		{
		  if (!edje_object_file_set(o, bd->internal_icon, "icon"))
		    e_util_edje_icon_set(o, "enlightenment/e");
		}
	      else if (ext)
		{
		  evas_object_del(o);
		  o = e_icon_add(evas);
		  e_icon_file_set(o, bd->internal_icon);
		}
	      else
		{
		  if (!e_util_edje_icon_set(o, bd->internal_icon))
		    e_util_edje_icon_set(o, "enlightenment/e");
		}
	    }
	  else
	    {
	      edje_object_file_set(o, bd->internal_icon,
				   bd->internal_icon_key);
	    }
	}
      return o;
    }

  if ((bd->client.netwm.icons) &&
      ((e_config->use_app_icon) ||
       (bd->remember && (bd->remember->prop.icon_preference == E_ICON_PREF_NETWM))))
    {
      int i, size, tmp, found = 0;
      o = e_icon_add(evas);

      size = bd->client.netwm.icons[0].width;
	
      for (i = 1; i < bd->client.netwm.num_icons; i++)
	{
	  if ((tmp = bd->client.netwm.icons[i].width) > size)
	    {
	      size = tmp;
	      found = i;
	    }
	}

      e_icon_data_set(o, bd->client.netwm.icons[found].data,
		      bd->client.netwm.icons[found].width,
		      bd->client.netwm.icons[found].height);
      e_icon_alpha_set(o, 1);

      return o;
    }

  if (bd->desktop)
    {
      o = e_util_desktop_icon_add(bd->desktop, 256, evas);
      if (o) return o;
      o = e_util_desktop_icon_add(bd->desktop, 128, evas);
      if (o) return o;
      o = e_util_desktop_icon_add(bd->desktop, 48, evas);
      if (o) return o;
      o = e_util_desktop_icon_add(bd->desktop, 32, evas);
      if (o) return o;
    }

  if (bd->client.netwm.icons)
    {
      int i, size, tmp, found = 0;
      o = e_icon_add(evas);

      size = bd->client.netwm.icons[0].width;

      for (i = 1; i < bd->client.netwm.num_icons; i++)
	{
	  if ((tmp = bd->client.netwm.icons[i].width) > size)
	    {
	      size = tmp;
	      found = i;
	    }
	}

      e_icon_data_set(o, bd->client.netwm.icons[found].data,
		      bd->client.netwm.icons[found].width,
		      bd->client.netwm.icons[found].height);
      e_icon_alpha_set(o, 1);
      return o;
    }

  o = e_border_icon_add(bd, evas);
  if (o) return o;

  o = edje_object_add(evas);
  e_util_edje_icon_set(o, "enlightenment/unknown");

  return o;
}

Ngw_Item *
ngw_item_new(Ng *ng, E_Border *bd)
{
  Ngw_Item *it;

  it = E_NEW(Ngw_Item, 1);
  it->ng = ng;
  it->border = NULL;
  it->label = NULL;
  it->o_icon = NULL;
  it->scale = 1.0;
  it->desk = NULL;
  it->bd_above = NULL;
  it->was_iconified = 0;
  it->was_shaded = 0;
  it->set_state = 0;
  
  it->obj = edje_object_add(ng->win->evas);

  if (!e_theme_edje_object_set(it->obj, "base/theme/modules/ng", "e/modules/ng/icon"))
    edje_object_file_set(it->obj, ngw_config->theme_path, "e/modules/ng/icon");

  evas_object_show(it->obj);

  e_object_ref(E_OBJECT(bd));
  it->border = bd;

  _ngw_item_border_set_icon(it);
  ngw_box_item_show(ng->box, it);
  ngw_item_signal_emit(it, "inactive");
 
  return it;
}

static void
_ngw_item_border_set_icon(Ngw_Item *it)
{
  it->o_icon = _ng_border_icon_add(it->border, it->ng->win->evas);
  edje_object_part_swallow(it->obj, "e.swallow.content", it->o_icon);
  evas_object_pass_events_set(it->o_icon, 1);
  evas_object_show(it->o_icon);
}

void
ngw_item_remove(Ngw_Item *it)
{
  ngw_box_item_remove(it->ng->box, it);
}

void
ngw_item_free(Ngw_Item *it)
{
  it->ng->items = eina_list_remove(it->ng->items, it);

  _ngw_item_del_icon(it);
  evas_object_del(it->obj);
  if(it->label) free(it->label);

  e_object_unref(E_OBJECT(it->border));
  
  free(it);
  it = NULL;
}

static void
_ngw_item_del_icon(Ngw_Item *it)
{
  if (it->o_icon)
    {
      edje_object_part_unswallow(it->obj, it->o_icon);
      evas_object_del(it->o_icon);
      it->o_icon = NULL;
    }
}

void
ngw_item_signal_emit(Ngw_Item *it, char *sig)
{
   if (it->obj)  edje_object_signal_emit(it->obj, sig, "e");
}



