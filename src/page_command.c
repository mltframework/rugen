/*
 * page_command.c -- Command Page Handling
 * Copyright (C) 2002-2003 Charles Yates <charles.yates@pandora.be>
 * Copyright (C) 2010 Dan Dennedy <dan@dennedy.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>

#include "interface.h"
#include "support.h"
#include "dv1394app.h"
#include "page.h"

typedef struct page_command_t
{
	struct page_t parent;
	dv1394app app;
	GtkWidget *widget;
}
*page_command;

static GtkWidget *this_page_get_widget( page_command );

/** Main window - command tab - command to be executed.
*/

static gboolean on_command_pressed( GtkWidget *button, gpointer user_data )
{
	page_command this = user_data;
	GtkWidget *widget;
	GtkTextBuffer *buffer;
	char *command;
	PangoFontDescription *font_desc;
	
	if ( dv1394app_get_parser( this->app ) != NULL )
	{
		int index = 0;
		mvcp_response response = NULL;
		GtkTextIter iter;

		widget = lookup_widget( this_page_get_widget( this ), "entry_command" );
		command = (char *)gtk_entry_get_text( GTK_ENTRY( widget ) );

		widget = lookup_widget( this_page_get_widget( this ), "textview_command" );
		
		font_desc = pango_font_description_from_string( "Courier 9" );
		gtk_widget_modify_font( widget, font_desc );
		pango_font_description_free( font_desc );
		buffer = gtk_text_view_get_buffer( GTK_TEXT_VIEW( widget ) );
		gtk_text_buffer_get_end_iter( buffer, &iter );
		gtk_text_buffer_place_cursor( buffer, &iter );
		gtk_text_buffer_insert_at_cursor( buffer, "> ", -1 );
		gtk_text_buffer_insert_at_cursor( buffer, command, -1 );
		gtk_text_buffer_insert_at_cursor( buffer, "\n", -1 );
		mvcp_execute( dv1394app_get_command( this->app ), 1024, "%s", command );
	    response = mvcp_get_last_response( dv1394app_get_command( this->app ) );
	    for ( index = 0; index < mvcp_response_count( response ); index ++ )
		{
			if ( index != mvcp_response_count( response ) - 1 || 
				 strcmp( mvcp_response_get_line( response, index ), "" ) )
			{
				gtk_text_buffer_insert_at_cursor( buffer, mvcp_response_get_line( response, index ), -1 );
				gtk_text_buffer_insert_at_cursor( buffer, "\n", -1 );
			}
		}
		gtk_text_buffer_insert_at_cursor( buffer, "\n", -1 );
		gtk_text_view_scroll_mark_onscreen( GTK_TEXT_VIEW( widget ), gtk_text_buffer_get_insert( buffer ) );
		widget = lookup_widget( this_page_get_widget( this ), "entry_command" );
		gtk_entry_set_text( GTK_ENTRY( widget ), "" );
		gtk_widget_grab_focus( widget );
	}
	else
	{
//		beep( );
	}

	return FALSE;
}

/** Main window - command tab - clear button pressed.
*/

static gboolean on_command_cleared( GtkWidget *button, gpointer user_data )
{
	page_command this = user_data;
	GtkWidget *widget = lookup_widget( this_page_get_widget( this ), "textview_command" );
	GtkTextBuffer *buffer = gtk_text_view_get_buffer( GTK_TEXT_VIEW( widget ) );
	gtk_text_buffer_set_text( buffer, "", -1 );
	return FALSE;
}

static GtkWidget *this_page_get_widget( page_command this )
{
	if ( this->widget == NULL )
		this->widget = create_page_shell( );
	return this->widget;
}

static void this_page_get_toolbar_info( page this, GtkIconSize size, GtkWidget **icon, char **label )
{
	*icon = gtk_image_new_from_stock( "gtk-dialog-info",  size );
	*label = "_Shell";
}

static void this_on_connect( page_command this )
{
}

static void this_on_disconnect( page_command this )
{
}

static void this_close( page_command this )
{
	if ( this != NULL )
		free( this );
}

page page_command_init( dv1394app app )
{
	GtkWidget *widget;
	page_command this = calloc( 1, sizeof( struct page_command_t ) );
	
	this->parent.get_widget = ( GtkWidget *(*)( page ) )this_page_get_widget;
	this->parent.get_toolbar_info = this_page_get_toolbar_info;
	this->parent.on_connect = ( void (*)( page ) )this_on_connect;
	this->parent.on_disconnect = ( void (*)( page ) )this_on_disconnect;
	this->parent.close = ( void (*)( page ) )this_close;
	this->app = app;
		
	/* Command execution handling */
	widget = lookup_widget( this_page_get_widget( this ), "entry_command" );
	gtk_signal_connect( GTK_OBJECT( widget ), "activate", GTK_SIGNAL_FUNC( on_command_pressed ), this );
	widget = lookup_widget( this_page_get_widget( this ), "button_command" );
	gtk_signal_connect( GTK_OBJECT( widget ), "clicked", GTK_SIGNAL_FUNC( on_command_pressed ), this );
	widget = lookup_widget( this_page_get_widget( this ), "button_command_clear" );
	gtk_signal_connect( GTK_OBJECT( widget ), "clicked", GTK_SIGNAL_FUNC( on_command_cleared ), this );

	return ( page )this;
}

