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

#include "hd-status-area-box.h"

/* UI Style guide */
#define ITEM_HEIGHT 16
#define ITEM_WIDTH 16
#define SPACING 8
#define MAX_VISIBLE_CHILDREN 8

struct _HDStatusAreaBoxPrivate
{
  GList *children;
};

typedef struct _HDStatusAreaBoxChild HDStatusAreaBoxChild;
struct _HDStatusAreaBoxChild
{
  GtkWidget *widget;
  guint      priority;
};

G_DEFINE_TYPE (HDStatusAreaBox, hd_status_area_box, GTK_TYPE_CONTAINER);

static gint
hd_status_area_box_cmp_priority (gconstpointer a,
                                 gconstpointer b)
{
  if (((HDStatusAreaBoxChild *)a)->priority >
      ((HDStatusAreaBoxChild *)b)->priority)
    return 1;

  return -1;
}

static void
hd_status_area_box_add (GtkContainer *container,
                        GtkWidget    *child)
{
  HDStatusAreaBoxPrivate *priv;
  HDStatusAreaBoxChild *info;

  g_return_if_fail (HD_IS_STATUS_AREA_BOX (container));
  g_return_if_fail (GTK_IS_WIDGET (child));
  g_return_if_fail (gtk_widget_get_parent (child) == NULL);

  priv = HD_STATUS_AREA_BOX (container)->priv;

  info = g_slice_new0 (HDStatusAreaBoxChild);
  info->widget = child;
  info->priority = G_MAXUINT;

  priv->children = g_list_insert_sorted (priv->children,
                                         info,
                                         hd_status_area_box_cmp_priority);

  gtk_widget_set_parent (child, GTK_WIDGET (container));  
}

static void
hd_status_area_box_remove (GtkContainer *container,
                           GtkWidget    *child)
{
  HDStatusAreaBoxPrivate *priv;
  GList *c;

  g_return_if_fail (HD_IS_STATUS_AREA_BOX (container));
  g_return_if_fail (GTK_IS_WIDGET (child));
  g_return_if_fail (gtk_widget_get_parent (child) == (GtkWidget *)container);

  priv = HD_STATUS_AREA_BOX (container)->priv;

  /* search for child in children and remove it */
  for (c = priv->children; c; c = c->next)
    {
      HDStatusAreaBoxChild *info = c->data;

      if (info->widget == child)
        {
          gboolean visible;

          visible = GTK_WIDGET_VISIBLE (child);

          gtk_widget_unparent (child);

          priv->children = g_list_delete_link (priv->children, c);
          g_slice_free (HDStatusAreaBoxChild, info);

          /* resize container if child was visible */
          if (visible)
            gtk_widget_queue_resize (GTK_WIDGET (container));

          break;
        }
    }
}

static void
hd_status_area_box_forall (GtkContainer *container,
                           gboolean      include_internals,
                           GtkCallback   callback,
                           gpointer      data)
{
  HDStatusAreaBoxPrivate *priv;
  GList *c;

  g_return_if_fail (HD_IS_STATUS_AREA_BOX (container));

  priv = HD_STATUS_AREA_BOX (container)->priv;

  for (c = priv->children; c; )
    {
      HDStatusAreaBoxChild *info = c->data;

      /* callback could destroy c */
      c = c->next;

      (* callback) (info->widget, data);
    }
}

static GType
hd_status_area_box_child_type (GtkContainer *container)
{
  return GTK_TYPE_WIDGET;
}

static void
hd_status_area_box_get_child_property (GtkContainer *container,
                                       GtkWidget    *child,
                                       guint         propid,
                                       GValue       *value,
                                       GParamSpec   *pspec)
{
}

static void
hd_status_area_box_set_child_property (GtkContainer *container,
                                       GtkWidget    *child,
                                       guint         propid,
                                       const GValue *value,
                                       GParamSpec   *pspec)
{
}

static void
hd_status_area_box_size_allocate (GtkWidget     *widget,
                                  GtkAllocation *allocation)
{
  HDStatusAreaBoxPrivate *priv;
  guint border_width;
  GtkAllocation child_allocation = {0, 0, 0, 0};
  guint visible_children = 0;
  GList *c;
  guint x[2] = {0, 0};

  priv = HD_STATUS_AREA_BOX (widget)->priv;

  border_width = gtk_container_get_border_width (GTK_CONTAINER (widget));

  /* chain up */
  GTK_WIDGET_CLASS (hd_status_area_box_parent_class)->size_allocate (widget,
                                                                     allocation);

  child_allocation.height = ITEM_HEIGHT;

  /* Place the first eight visible children */
  for (c = priv->children; c && visible_children < MAX_VISIBLE_CHILDREN; c = c->next)
    {
      HDStatusAreaBoxChild *info = c->data;
      GtkRequisition child_requisition;

      /* ignore hidden widgets */
      if (!GTK_WIDGET_VISIBLE (info->widget))
        continue;

      gtk_widget_size_request (info->widget, &child_requisition);

      child_allocation.x = allocation->x + border_width + x[visible_children % 2];
      child_allocation.y = allocation->y + border_width + (visible_children % 2 * (ITEM_HEIGHT + SPACING));

      child_allocation.width = child_requisition.width;
      x[visible_children % 2] += child_requisition.width;

      gtk_widget_size_allocate (info->widget, &child_allocation);
      gtk_widget_set_child_visible (info->widget, TRUE);


      visible_children++;
    }

  /* Hide the other children */
  for (; c; c = c->next)
    {
      HDStatusAreaBoxChild *info = c->data;

      gtk_widget_set_child_visible (info->widget, FALSE);
    }
}

static void
hd_status_area_box_size_request (GtkWidget      *widget,
                                 GtkRequisition *requisition)
{
  HDStatusAreaBoxPrivate *priv;
  guint border_width;
  GList *c;
  guint visible_children = 0;
  guint width[2] = {0, 0};

  priv = HD_STATUS_AREA_BOX (widget)->priv;

  border_width = gtk_container_get_border_width (GTK_CONTAINER (widget));

  /* calculate number of visible children */
  for (c = priv->children; c; c = c->next)
    {
      HDStatusAreaBoxChild *info = c->data;
      GtkRequisition child_requisition;

      if (!GTK_WIDGET_VISIBLE (info->widget))
        continue;

      /* there are some widgets which need a size request */
      gtk_widget_size_request (info->widget, &child_requisition);

      if (visible_children < MAX_VISIBLE_CHILDREN)
        width[visible_children % 2] += child_requisition.width;

      visible_children++;
    }

  /* width is maximum width of both rows */
  requisition->width =  MAX (width[0], width[1]) + 2 * border_width;
  /* height is two rows */
  requisition->height = 2 * ITEM_HEIGHT + SPACING + 2 * border_width;
}

static void
hd_status_area_box_class_init (HDStatusAreaBoxClass *klass)
{
  GtkContainerClass *container_class = GTK_CONTAINER_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  container_class->add = hd_status_area_box_add;
  container_class->remove = hd_status_area_box_remove;
  container_class->forall = hd_status_area_box_forall;
  container_class->child_type = hd_status_area_box_child_type;
  container_class->get_child_property = hd_status_area_box_get_child_property;
  container_class->set_child_property = hd_status_area_box_set_child_property;

  widget_class->size_allocate = hd_status_area_box_size_allocate;
  widget_class->size_request = hd_status_area_box_size_request;

  g_type_class_add_private (klass, sizeof (HDStatusAreaBoxPrivate));
}

static void
hd_status_area_box_init (HDStatusAreaBox *box)
{
  GTK_WIDGET_SET_FLAGS (box, GTK_NO_WINDOW);
  gtk_widget_set_redraw_on_allocate (GTK_WIDGET (box),
                                     FALSE);

  box->priv = G_TYPE_INSTANCE_GET_PRIVATE ((box), HD_TYPE_STATUS_AREA_BOX, HDStatusAreaBoxPrivate);

  box->priv->children = NULL;
}

GtkWidget *
hd_status_area_box_new (void)
{
  GtkWidget *box;

  box = g_object_new (HD_TYPE_STATUS_AREA_BOX,
                      NULL);

  return box;
}
