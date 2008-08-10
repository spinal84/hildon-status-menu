/*
 * This file is part of hildon-status-menu
 * 
 * Copyright (C) 2008 Nokia Corporation.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gdk/gdkx.h>

#include <X11/X.h>
#include <X11/Xatom.h>

#include <string.h>

#include "hd-status-menu.h"
#include "hd-status-area-box.h"

#include "hd-status-area.h"

/* UI Style guide */
#define STATUS_AREA_HEIGHT 56
#define MINIMUM_STATUS_AREA_WIDTH 112

#define STATUS_AREA_ICON_HEIGHT 16
#define SPECIAL_ICON_WIDTH 48

/* Configuration file keys */
#define HD_STATUS_AREA_KEYFILE_GROUP          "X-Status-Area"
#define HD_STATUS_AREA_KEYFILE_SIGNAL_PLUGIN  "X-Signal-Plugin"
#define HD_STATUS_AREA_KEYFILE_BATTERY_PLUGIN "X-Battery-Plugin"

#define HD_STATUS_AREA_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), HD_TYPE_STATUS_AREA, HDStatusAreaPrivate));

static GQuark      quark_hd_status_area_image  = 0;
static const gchar hd_status_area_image[] = "hd_status_area_image";

enum
{
  PROP_0,
  PROP_PLUGIN_MANAGER
};

struct _HDStatusAreaPrivate
{
  HDPluginManager *plugin_manager;

  GtkWidget *status_menu;

  GtkWidget *icon_box;

  GtkWidget *signal_image, *battery_image;

  gchar *signal_plugin, *battery_plugin;
};

G_DEFINE_TYPE (HDStatusArea, hd_status_area, GTK_TYPE_WINDOW);

static gboolean
button_release_event_cb (GtkWidget      *widget,
                         GdkEventButton *event,
                         HDStatusArea   *status_area)
{
  HDStatusAreaPrivate *priv = status_area->priv;

  gtk_widget_show (priv->status_menu);

  return TRUE;
}

static void
hd_status_area_init (HDStatusArea *status_area)
{
  HDStatusAreaPrivate *priv = HD_STATUS_AREA_GET_PRIVATE (status_area);

  /* UI Style guide */
  GtkWidget *ebox, *alignment, *main_hbox, *left_vbox;

  GtkWidget *left_top_row, *left_hsep, *left_bottom_row;

  GtkWidget *clock_box, *clock_label, *clock_ampm_image;
  GtkWidget *alarm_image;

  /* Set priv member */
  status_area->priv = priv;

  /* Create Status area UI */
  ebox = gtk_event_box_new ();
  gtk_widget_add_events (GTK_WIDGET (ebox), GDK_BUTTON_RELEASE_MASK);
  g_signal_connect (G_OBJECT (ebox), "button-release-event",
                    G_CALLBACK (button_release_event_cb), status_area);
  gtk_widget_show (ebox);

  alignment = gtk_alignment_new (0.5, 0.5, 0.0, 0.0);
  gtk_alignment_set_padding (GTK_ALIGNMENT (alignment), 4, 10, 10, 10);
  gtk_widget_show (alignment);

  main_hbox = gtk_hbox_new (FALSE, 8);
  gtk_widget_show (main_hbox);

  left_vbox = gtk_vbox_new (FALSE, 0);  
  gtk_widget_show (left_vbox);

  left_top_row = gtk_hbox_new (TRUE, 0);
  gtk_widget_set_size_request (left_top_row, SPECIAL_ICON_WIDTH * 2, STATUS_AREA_ICON_HEIGHT);
  gtk_widget_show (left_top_row);

  left_hsep = gtk_hseparator_new ();
  gtk_widget_set_size_request (left_hsep, -1, 8);
  gtk_widget_show (left_hsep);

  left_bottom_row = gtk_hbox_new (FALSE, 8);
  gtk_widget_show (left_bottom_row);

  priv->signal_image = gtk_image_new ();
  gtk_widget_set_size_request (priv->signal_image, SPECIAL_ICON_WIDTH, STATUS_AREA_ICON_HEIGHT);
  gtk_widget_show (priv->signal_image);

  priv->battery_image = gtk_image_new ();
  gtk_widget_set_size_request (priv->battery_image, SPECIAL_ICON_WIDTH, STATUS_AREA_ICON_HEIGHT);
  gtk_widget_show (priv->battery_image);

  clock_box = gtk_hbox_new (FALSE, 4);
  gtk_widget_show (clock_box);

  clock_label = gtk_label_new (NULL);
  gtk_label_set_markup (GTK_LABEL (clock_label), "<span font_desc=\"12\">15:48</span>");
  gtk_widget_set_size_request (clock_label, -1, 16);
  gtk_widget_show (clock_label);

  clock_ampm_image = gtk_image_new ();
/*  gtk_image_set_from_file (GTK_IMAGE (clock_ampm_image), "ampm.png");*/
  gtk_widget_show (clock_ampm_image);

  alarm_image = gtk_image_new ();
/*  gtk_image_set_from_file (GTK_IMAGE (alarm_image), "alarm.png");*/
  gtk_widget_show (alarm_image);

  priv->icon_box = hd_status_area_box_new ();
  gtk_widget_show (priv->icon_box);

  /* Pack widgets */
  gtk_container_add (GTK_CONTAINER (status_area), ebox);
  gtk_container_add (GTK_CONTAINER (ebox), alignment);
  gtk_container_add (GTK_CONTAINER (alignment), main_hbox);
  gtk_box_pack_start (GTK_BOX (main_hbox), left_vbox, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (main_hbox), priv->icon_box, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (left_vbox), left_top_row, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (left_vbox), left_hsep, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (left_vbox), left_bottom_row, FALSE, FALSE, 0);

  gtk_box_pack_start (GTK_BOX (left_top_row), priv->signal_image, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (left_top_row), priv->battery_image, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (left_bottom_row), clock_box, FALSE, FALSE, 0);
  gtk_box_pack_end (GTK_BOX (left_bottom_row), alarm_image, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (clock_box), clock_label, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (clock_box), clock_ampm_image, FALSE, FALSE, 0);
}

static GObject *
hd_status_area_constructor (GType                  type,
                            guint                  n_construct_properties,
                            GObjectConstructParam *construct_properties)
{
  GObject *object;
  HDStatusAreaPrivate *priv;

  object = G_OBJECT_CLASS (hd_status_area_parent_class)->constructor (type,
                                                                      n_construct_properties,
                                                                      construct_properties);

  /* Create Status Menu */
  priv = HD_STATUS_AREA (object)->priv;
  priv->status_menu = hd_status_menu_new (priv->plugin_manager);

  return object;
}

static void
hd_status_area_dispose (GObject *object)
{
  HDStatusAreaPrivate *priv = HD_STATUS_AREA (object)->priv;

  if (priv->plugin_manager)
    {
      g_object_unref (priv->plugin_manager);
      priv->plugin_manager = NULL;
    }

  G_OBJECT_CLASS (hd_status_area_parent_class)->dispose (object);
}

static void
hd_status_area_finalize (GObject *object)
{
  HDStatusAreaPrivate *priv = HD_STATUS_AREA (object)->priv;

  g_free (priv->signal_plugin);
  g_free (priv->battery_plugin);

  G_OBJECT_CLASS (hd_status_area_parent_class)->finalize (object);
}

static void
status_area_icon_changed (HDStatusPluginItem *plugin)
{
  GtkWidget *image;
  GdkPixbuf *pixbuf;

  /* Get the image connected with the plugin */
  image = g_object_get_qdata (G_OBJECT (plugin),
                              quark_hd_status_area_image);

  /* Update icon */
  g_object_get (G_OBJECT (plugin),
                "status-area-icon", &pixbuf,
                NULL);
  gtk_image_set_from_pixbuf (GTK_IMAGE (image), pixbuf);

  /* Hide image if icon is not set */
  if (pixbuf)
    {
      g_object_unref (pixbuf);

      gtk_widget_show (image);
    }
  else
    gtk_widget_hide (image);
}

static void
hd_status_area_plugin_added_cb (HDPluginManager *plugin_manager,
                                GObject         *plugin,
                                HDStatusArea    *status_area)
{
  HDStatusAreaPrivate *priv = status_area->priv;
  GtkWidget *image;

  /* Plugin must be a HDStatusMenuItem */
  if (!HD_IS_STATUS_PLUGIN_ITEM (plugin))
    return;

  g_object_ref (plugin);

  /* Check if plugin is the special battery or signal item */
  if (priv->signal_plugin != NULL &&
      strcmp (priv->signal_plugin,
              hd_status_plugin_item_get_dl_filename (HD_STATUS_PLUGIN_ITEM (plugin))) == 0)
    {
      image = priv->signal_image;
      g_object_set_qdata (plugin, quark_hd_status_area_image, image);
    }
  else if (priv->battery_plugin != NULL &&
           strcmp (priv->battery_plugin,
                   hd_status_plugin_item_get_dl_filename (HD_STATUS_PLUGIN_ITEM (plugin))) == 0)
    {
      image = priv->battery_image;
      g_object_set_qdata (plugin, quark_hd_status_area_image, image);
    }
  else
    {
      /* Create GtkImage to display the icon */
      image = gtk_image_new ();
      g_object_set_qdata_full (plugin, quark_hd_status_area_image,
                               image, (GDestroyNotify) gtk_widget_destroy);
      gtk_container_add (GTK_CONTAINER (priv->icon_box), image);
    }

  g_signal_connect (plugin, "notify::status-area-icon",
                    G_CALLBACK (status_area_icon_changed), NULL);
  status_area_icon_changed (HD_STATUS_PLUGIN_ITEM (plugin));
}

static void
hd_status_area_plugin_removed_cb (HDPluginManager *plugin_manager,
                                  GObject         *plugin,
                                  HDStatusArea    *status_area)
{
//  HDStatusAreaPrivate *priv = status_area->priv;

  /* Plugin must be a HDStatusMenuItem */
  if (!HD_IS_STATUS_PLUGIN_ITEM (plugin))
    return;

  g_signal_handlers_disconnect_by_func (plugin,
                                        status_area_icon_changed,
                                        NULL);
  g_object_set_qdata (plugin, quark_hd_status_area_image, NULL);
  g_object_unref (plugin);
}

static void
hd_status_area_configuration_loaded_cb (HDPluginManager *plugin_manager,
                                        GKeyFile        *keyfile,
                                        HDStatusArea    *status_area)
{
  HDStatusAreaPrivate *priv = status_area->priv;

  g_free (priv->signal_plugin);
  g_free (priv->battery_plugin);

  priv->signal_plugin = g_key_file_get_string (keyfile,
                                               HD_STATUS_AREA_KEYFILE_GROUP,
                                               HD_STATUS_AREA_KEYFILE_SIGNAL_PLUGIN,
                                               NULL);
  priv->battery_plugin = g_key_file_get_string (keyfile,
                                                HD_STATUS_AREA_KEYFILE_GROUP,
                                                HD_STATUS_AREA_KEYFILE_BATTERY_PLUGIN,
                                                NULL);
}

static void
hd_status_area_set_property (GObject      *object,
                             guint         prop_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
  HDStatusAreaPrivate *priv = HD_STATUS_AREA (object)->priv;

  switch (prop_id)
    {
    case PROP_PLUGIN_MANAGER:
      /* The property is CONSTRUCT_ONLY so there is no value yet */
      priv->plugin_manager = g_value_dup_object (value);
      if (priv->plugin_manager != NULL)
        {
          g_signal_connect_object (G_OBJECT (priv->plugin_manager), "plugin-added",
                                   G_CALLBACK (hd_status_area_plugin_added_cb), object, 0);
          g_signal_connect_object (G_OBJECT (priv->plugin_manager), "plugin-removed",
                                   G_CALLBACK (hd_status_area_plugin_removed_cb), object, 0);
          g_signal_connect_object (G_OBJECT (priv->plugin_manager), "configuration-loaded",
                                   G_CALLBACK (hd_status_area_configuration_loaded_cb), object, 0);
        }
      else
        g_warning ("plugin-manager should not be NULL");
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
hd_status_area_realize (GtkWidget *widget)
{
  GdkDisplay *display;
  Atom atom;
  const gchar *notification_type = "_HILDON_WM_WINDOW_TYPE_APP_MENU";

  GTK_WIDGET_CLASS (hd_status_area_parent_class)->realize (widget);

  /* Use only a border as decoration */
  gdk_window_set_decorations (widget->window, 0);

  /* Set the _NET_WM_WINDOW_TYPE property (copied from HildonAppMenu) */
  display = gdk_drawable_get_display (widget->window);
  atom = gdk_x11_get_xatom_by_name_for_display (display, "_NET_WM_WINDOW_TYPE");
  XChangeProperty (GDK_WINDOW_XDISPLAY (widget->window), GDK_WINDOW_XID (widget->window),
                   atom, XA_STRING, 8, PropModeReplace, (guchar *) notification_type,
                   strlen (notification_type));
}

static void
hd_status_area_class_init (HDStatusAreaClass *klass)
{
  GObjectClass* object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass* widget_class = GTK_WIDGET_CLASS (klass);

  quark_hd_status_area_image = g_quark_from_static_string (hd_status_area_image);

  object_class->constructor = hd_status_area_constructor;
  object_class->dispose = hd_status_area_dispose;
  object_class->finalize = hd_status_area_finalize;
  object_class->set_property = hd_status_area_set_property;

  widget_class->realize = hd_status_area_realize;

  g_object_class_install_property (object_class,
                                   PROP_PLUGIN_MANAGER,
                                   g_param_spec_object ("plugin-manager",
                                                        "Plugin Manager",
                                                        "The plugin manager which should be used",
                                                        HD_TYPE_PLUGIN_MANAGER,
                                                        G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));

  g_type_class_add_private (klass, sizeof (HDStatusAreaPrivate));
}

GtkWidget *
hd_status_area_new (HDPluginManager *plugin_manager)
{
  GtkWidget *status_area;

  status_area = g_object_new (HD_TYPE_STATUS_AREA,
                              "type", GTK_WINDOW_TOPLEVEL,
                              "plugin-manager", plugin_manager,
                              NULL);

  return status_area;
}
