// 
// Copyright (C) 2001,2002,2003,2004 Jorge Daza Garcia-Blanes
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
// USA
// 
/*
 * $Id$
 */

#include <gtk/gtk.h>

#include "drqm_common.h"

#ifdef __CYGWIN
#include <malloc.h>
void cygwin_conv_to_posix_path(const char *path, char *posix_path);
void cygwin_conv_to_win32_path(const char *path, char *win32_path);
#endif

GtkWidget *ConfirmDialog (char *text, GList *callbacks)
{
  GtkWidget *dialog;
  GtkWidget *label;
  GtkWidget *button;
	GList *cb2;
	gpointer data;

  /* Dialog */
  dialog = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW(dialog),"You Sure?");
  gtk_signal_connect_object(GTK_OBJECT(dialog),"destroy",
			    GTK_SIGNAL_FUNC(gtk_widget_destroy),
			    (gpointer)dialog);

  /* Label */
  label = gtk_label_new (text);
  gtk_misc_set_padding (GTK_MISC(label), 10, 10);
  gtk_box_pack_start (GTK_BOX(GTK_DIALOG(dialog)->vbox),label,TRUE,TRUE,5);
 
  /* Buttons */
  button = gtk_button_new_with_label ("Yes");
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area),button, TRUE, TRUE, 5);
  for (;callbacks;callbacks = cb2->next) {
		cb2 = callbacks->next;
		data = cb2->data;
    g_signal_connect(G_OBJECT(button),"clicked",G_CALLBACK(callbacks->data),data);
  }
  g_signal_connect_swapped(GTK_OBJECT(button),"clicked",G_CALLBACK(gtk_widget_destroy),
			    (GtkObject*)dialog);

  button = gtk_button_new_with_label ("No");
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area),button, TRUE, TRUE, 5);
  gtk_signal_connect_object(GTK_OBJECT(button),"clicked",GTK_SIGNAL_FUNC(gtk_widget_destroy),
			    (GtkObject*)dialog);
  GTK_WIDGET_SET_FLAGS(button,GTK_CAN_DEFAULT);
  gtk_widget_grab_default(button);

  gtk_widget_show_all (dialog);

  return dialog;
}

GtkTooltips *TooltipsNew (void)
{
  GtkTooltips *tooltips;

  tooltips = gtk_tooltips_new ();
  gtk_tooltips_set_delay (tooltips,TOOLTIPS_DELAY);

  return tooltips;
}

#ifdef __CYGWIN

char *conv_to_posix_path(char *win32_path)
{
  char *posix_path;

  if ((posix_path = malloc(MAXCMDLEN)) == NULL)
	return (NULL);
  cygwin_conv_to_posix_path(win32_path, posix_path);
  return (posix_path);
}


char *conv_to_win32_path(char *posix_path)
{
  char *win32_path;

  if ((win32_path = malloc(MAXCMDLEN)) == NULL)
	return(NULL);
  cygwin_conv_to_win32_path(posix_path, win32_path);
  return (win32_path);

}
#endif

