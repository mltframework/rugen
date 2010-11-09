/*
 * dv1394app.c -- GTK+ 2 dv1394d client demo
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include "interface.h"
#include "support.h"
#include "dv1394app.h"
#include "page.h"
#include "gtkenhancedscale.h"
#include <mvcp/mvcp_remote.h>
#include <melted/melted_local.h>


/** Window close event.
*/

static gboolean on_main_window_delete_event( GtkWidget *widget, GdkEvent *event, gpointer user_data )
{
	gtk_exit( 0 );
	return TRUE;
}

static gboolean instance_connect( dv1394app this, char *server, char *port )
{
	if ( this->parser == NULL )
	{
		if ( strstr( server, ".conf" ) == NULL )
			this->parser = mvcp_parser_init_remote( server, atoi( port ) );
		else
			this->parser = melted_parser_init_local( );

		this->command = mvcp_init( this->parser );

		if ( strstr( server, ".conf" ) != NULL )
			mvcp_run( this->command, server );

		if ( mvcp_connect( this->command ) == mvcp_ok )
		{
			struct timespec t = { 1, 0 };
			nanosleep( &t, NULL );
			//gdk_threads_leave( );
			dv1394app_connect( this );
			//gdk_threads_enter( );
		}
		else
		{
			mvcp_close( this->command );
			mvcp_parser_close( this->parser );
			this->parser = NULL;
//			beep( );
		}
	}

	return this->parser != NULL;
}

/** Connection window - Connect button pressed.
*/

static gboolean on_connect_pressed( GtkWidget *button, gpointer user_data )
{
	dv1394app this = user_data;
	GtkWidget *widget;
	char *server;
	char *port;

	if ( this->parser == NULL )
	{
		widget = lookup_widget( this->connect, "entry_server" );
		server = ( char * )gtk_entry_get_text( GTK_ENTRY( widget ) );
		widget = lookup_widget( this->connect, "entry_port" );
		port = ( char * )gtk_entry_get_text( GTK_ENTRY( widget ) );
		instance_connect( this, server, port );
	}
	gtk_widget_destroy( this->connect );

	return FALSE;
}

/** Connection window - Cancel button pressed.
*/

static gboolean on_cancel_pressed( GtkWidget *button, gpointer user_data )
{
	dv1394app this = user_data;
	gtk_widget_destroy( this->connect );
	return FALSE;
}

/** Main window - connect menu item selected.
*/

void on_item_connect_activate( GtkMenuItem *menuitem, gpointer user_data )
{
	dv1394app this = user_data;
	GtkWidget *widget;
	
	this->connect = create_window_connection( );
	
	/* Connection set up handling */
	widget = lookup_widget( this->connect, "button_connect" );
	gtk_signal_connect( GTK_OBJECT( widget ), "clicked", GTK_SIGNAL_FUNC( on_connect_pressed ), this );
	widget = lookup_widget( this->connect, "button_cancel" );
	gtk_signal_connect( GTK_OBJECT( widget ), "clicked", GTK_SIGNAL_FUNC( on_cancel_pressed ), this );
	
	gtk_widget_show( this->connect );
}

/** Main window - disconnect menu item selected.
*/

void on_item_disconnect_activate( GtkMenuItem *menuitem, gpointer user_data )
{
	dv1394app this = user_data;
	dv1394app_disconnect( this );
}

/** Main window - quit menu item selected.
*/

void on_item_quit_activate( GtkMenuItem *menuitem, gpointer user_data )
{
	gtk_main_quit( );
}

static gboolean on_page_switch_pressed( GtkWidget *button, gpointer user_data )
{
	dv1394app this = user_data;
	int index = 0;
	GtkWidget *notebook = lookup_widget( button, "notebook1" );
	
	for ( index = 0; index < this->page_count; index ++ )
	{
		if ( this->page_buttons[ index ] == button )
			break;
	}
	
	gtk_notebook_set_current_page( GTK_NOTEBOOK( notebook ), index );
	
	return TRUE;
}

static gboolean on_transport_pressed( GtkWidget *button, gpointer data )
{
	int index = 0;
	dv1394app this = ( dv1394app )data;
	mvcp dv = dv1394app_get_command( this );
	int unit = dv1394app_get_selected_unit( this );
	
	for ( index = 0; index < 11; index ++ )
		if ( this->buttons[ index ] == button )
			break;
	
	switch( index )
	{
		case 0:
			mvcp_unit_clip_goto( dv, unit, mvcp_absolute, 0, 0 );
			break;
		
		case 1:
			mvcp_unit_goto( dv, unit, 0 );
			break;
		
		case 2:
			mvcp_unit_rewind( dv, unit );
			break;
		
		case 3:
			mvcp_unit_step( dv, unit, -1 );
			break;
		
		case 4:
			mvcp_unit_pause( dv, unit );
			break;
		
		case 5:
			mvcp_unit_play( dv, unit );
			break;
		
		case 6:
			mvcp_unit_stop( dv, unit );
			break;
		
		case 7:
			mvcp_unit_step( dv, unit, 1 );
			break;
		
		case 8:
			mvcp_unit_fast_forward( dv, unit );
			break;
		
		case 9:
			mvcp_unit_clip_goto( dv, unit, mvcp_relative, 1, 0 );
			break;
		
		case 10:
			mvcp_unit_clip_goto( dv, unit, mvcp_absolute, 9999, -1 );
			break;
		
		default:
			break;
	}
	
	return TRUE;
}

static void dv1394app_register_page( dv1394app this, page item )
{
	GtkWidget *toolbar = lookup_widget( this->window, "toolbar2" );
	GtkIconSize size = gtk_toolbar_get_icon_size( GTK_TOOLBAR( toolbar ) );
	GtkWidget *widget = lookup_widget( this->window, "notebook1" );
	GtkWidget *bin = gtk_frame_new( NULL );
	char *label = NULL;
	
	this->pages[ this->page_count ] = item;
	gtk_widget_reparent( GTK_BIN( page_get_widget( item ) )->child, bin );
	gtk_container_add(GTK_CONTAINER( widget ), bin );
	gtk_frame_set_label_align( GTK_FRAME( bin ), 0, 0 );
	gtk_frame_set_shadow_type( GTK_FRAME( bin ), GTK_SHADOW_NONE );
	gtk_widget_show( bin );
	
	page_get_toolbar_info( item, size, &widget, &label );
	this->page_buttons[ this->page_count ] = gtk_toolbar_append_element( GTK_TOOLBAR ( toolbar ), GTK_TOOLBAR_CHILD_BUTTON, NULL, label, NULL, NULL, widget, NULL, NULL);
	gtk_label_set_use_underline( GTK_LABEL(((GtkToolbarChild*)(g_list_last( GTK_TOOLBAR( toolbar )->children)->data))->label), TRUE);
	gtk_widget_show( widget );
	gtk_signal_connect( GTK_OBJECT( this->page_buttons[ this->page_count ] ), "clicked", GTK_SIGNAL_FUNC( on_page_switch_pressed ), this );
	gtk_signal_connect( GTK_OBJECT( this->page_buttons[ this->page_count ] ), "activate", GTK_SIGNAL_FUNC( on_page_switch_pressed ), this );
	
	this->page_count ++;
}

static GtkAdjustment *trim_adj[3];
#define TRIM_ADJ_POS 0
#define TRIM_ADJ_IN 1
#define TRIM_ADJ_OUT 2

void dv1394app_show_status( dv1394app this, mvcp_status status )
{
	int index = 0;
	for ( index = 0; index < this->page_count; index ++ )
		page_show_status( this->pages[ index ], status );
	
	if ( status->seek_flag != this->seek_flag )
	{
		gtk_widget_set_sensitive( lookup_widget( dv1394app_get_widget( this ), "transport_2" ), status->seek_flag );
		gtk_widget_set_sensitive( lookup_widget( dv1394app_get_widget( this ), "transport_3" ), status->seek_flag );
		gtk_widget_set_sensitive( lookup_widget( dv1394app_get_widget( this ), "transport_7" ), status->seek_flag );
		gtk_widget_set_sensitive( lookup_widget( dv1394app_get_widget( this ), "transport_8" ), status->seek_flag );
		this->seek_flag = status->seek_flag;
	}
	
	if ( !this->trim_in_use )
	{
		int upper = status->length > 0 ? status->length - 1 : 0;
		trim_adj[TRIM_ADJ_IN]->upper = upper;
		trim_adj[TRIM_ADJ_IN]->value = status->in;
		trim_adj[TRIM_ADJ_OUT]->upper = upper;
		trim_adj[TRIM_ADJ_OUT]->value = status->out;
		trim_adj[TRIM_ADJ_POS]->upper = upper;
		trim_adj[TRIM_ADJ_POS]->value = status->position;
		gtk_signal_emit_by_name( GTK_OBJECT(trim_adj[TRIM_ADJ_POS]), "value_changed" );
	}
}

static gboolean trim_pressed( GtkWidget *button, GdkEventButton *event, gpointer user_data )
{
	dv1394app this = (dv1394app)user_data;
	mvcp_unit_pause( dv1394app_get_command( this ), this->selected_unit );
	this->trim_in = -1;
	this->trim_out = -1;
	this->trim_in_use = 1;
	return FALSE;
}

static gboolean trim_released( GtkWidget *button, GdkEventButton *event, gpointer user_data )
{
	dv1394app this = (dv1394app)user_data;
	this->trim_in_use = 0;
	if ( this->trim_in != -1 )
		mvcp_unit_set_in( dv1394app_get_command( this ), this->selected_unit, this->trim_in );
	if ( this->trim_out != -1 )
		mvcp_unit_set_out( dv1394app_get_command( this ), this->selected_unit, this->trim_out );
	return TRUE;
}

static gboolean on_trim_value_changed_event( GtkWidget *button, gpointer user_data )
{
	dv1394app this = (dv1394app)user_data;
	if ( this->trim_in_use )
	{
		char *value;
		g_object_get( G_OBJECT( button ), "user_data", &value, NULL );
		
		if ( !strcmp( value, "position" ) )
		{
			mvcp_unit_goto( dv1394app_get_command( this ), this->selected_unit, trim_adj[TRIM_ADJ_POS]->value );
		}
		else if ( !strcmp( value, "in" ) )
		{
			this->trim_in = trim_adj[TRIM_ADJ_IN]->value;
			mvcp_unit_goto( dv1394app_get_command( this ), this->selected_unit, trim_adj[TRIM_ADJ_IN]->value );
		}
		else if ( !strcmp( value, "out" ) )
		{
			this->trim_out = trim_adj[TRIM_ADJ_OUT]->value;
			mvcp_unit_goto( dv1394app_get_command( this ), this->selected_unit, trim_adj[TRIM_ADJ_OUT]->value );
		}
		
		gtk_widget_queue_draw (lookup_widget(this->window, "vbox_trim") );
		return TRUE;
	}
	return FALSE;
}

/** Initialiser for application state.
*/

dv1394app dv1394app_init( GtkWidget *window, char *instance )
{
	dv1394app this = calloc( 1, sizeof( dv1394app_t ) );

	if ( this != NULL )
	{
		GtkWidget *widget;
		
		this->window = window;
		
		/* Window destroy event */
		gtk_signal_connect( GTK_OBJECT( this->window ), "destroy", GTK_SIGNAL_FUNC( on_main_window_delete_event ), this );

		/* Menu item signal handling */
 		widget = lookup_widget( this->window, "item_connect" );
		gtk_signal_connect( GTK_OBJECT( widget ), "activate", GTK_SIGNAL_FUNC( on_item_connect_activate ), this );
 		widget = lookup_widget( this->window, "button_connect" );
		gtk_signal_connect( GTK_OBJECT( widget ), "clicked", GTK_SIGNAL_FUNC( on_item_connect_activate ), this );
 		widget = lookup_widget( this->window, "item_disconnect" );
		gtk_signal_connect( GTK_OBJECT( widget ), "activate", GTK_SIGNAL_FUNC( on_item_disconnect_activate ), this );
 		widget = lookup_widget( this->window, "button_disconnect" );
		gtk_signal_connect( GTK_OBJECT( widget ), "clicked", GTK_SIGNAL_FUNC( on_item_disconnect_activate ), this );
 		widget = lookup_widget( this->window, "item_quit" );
		gtk_signal_connect( GTK_OBJECT( widget ), "activate", GTK_SIGNAL_FUNC( on_item_quit_activate ), this );
 		widget = lookup_widget( this->window, "button_quit" );
		gtk_signal_connect( GTK_OBJECT( widget ), "clicked", GTK_SIGNAL_FUNC( on_item_quit_activate ), this );

		/* Initialise the pages. */
		dv1394app_register_page( this, page_status_init( this ) );
		dv1394app_register_page( this, page_clips_init( this ) );
		dv1394app_register_page( this, page_command_init( this ) );

		/* Remove the empty page */
		widget = lookup_widget( this->window, "notebook1" );
		gtk_notebook_remove_page( GTK_NOTEBOOK( widget ), 0 );
		
		guint keys[ ] = { GDK_0, GDK_1, GDK_2, GDK_3, GDK_4, GDK_5, GDK_6, GDK_7, GDK_8, GDK_9, GDK_A };
		int index;
		GtkAccelGroup *accel_group = gtk_accel_group_new( );
		
		for ( index = 0; index < 11; index ++ )
		{
			char name[ 256 ];
			sprintf( name, "transport_%d", index );
			this->buttons[ index ] = lookup_widget( dv1394app_get_widget( this ), name );
			gtk_signal_connect( GTK_OBJECT( this->buttons[ index ] ), "clicked", GTK_SIGNAL_FUNC( on_transport_pressed ), this );
			gtk_widget_add_accelerator( this->buttons[ index ], "clicked", accel_group, keys[ index ], GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE );
		}
		
		gtk_window_add_accel_group( GTK_WINDOW( dv1394app_get_widget( this ) ), accel_group);
		
		trim_adj[TRIM_ADJ_POS] = GTK_ADJUSTMENT( gtk_adjustment_new( 0, 0, 1000, 1, 10, 0 ) );
		g_object_set( G_OBJECT( trim_adj[TRIM_ADJ_POS] ), "user_data", "position", NULL );
		trim_adj[TRIM_ADJ_IN] = GTK_ADJUSTMENT(	gtk_adjustment_new( 0, 0, 1000, 1, 10, 0 ) );
		g_object_set( G_OBJECT( trim_adj[TRIM_ADJ_IN] ), "user_data", "in", NULL );
		trim_adj[TRIM_ADJ_OUT] = GTK_ADJUSTMENT( gtk_adjustment_new( 0, 0, 1000, 1, 10, 0 ) );
		g_object_set( G_OBJECT( trim_adj[TRIM_ADJ_OUT] ), "user_data", "out", NULL );
		
		int i;
		for (i = 0; i < 3; i++)
			gtk_signal_connect (GTK_OBJECT (trim_adj[i]), "value_changed", GTK_SIGNAL_FUNC (on_trim_value_changed_event), this );
		
		GtkWidget *trim = gtk_enhanced_scale_new( (GtkObject**) trim_adj, 3);
		if ( trim != NULL )
		{
			gtk_widget_set_name (trim, "trim");
			GTK_WIDGET_UNSET_FLAGS( GTK_WIDGET( trim ), GTK_CAN_FOCUS );
			gtk_widget_ref(trim);
			gtk_object_set_data_full (GTK_OBJECT( this->window ), "trim", trim,	(GtkDestroyNotify) gtk_widget_unref);
			GtkWidget *vbox_trim = lookup_widget(this->window, "vbox_trim");
			gtk_widget_show(trim);
			gtk_box_pack_start(GTK_BOX (vbox_trim), trim, TRUE, TRUE, 0);
			gtk_signal_connect( GTK_OBJECT( trim ), "button_press_event", GTK_SIGNAL_FUNC (trim_pressed), this );
			gtk_signal_connect( GTK_OBJECT( trim ), "button_release_event", GTK_SIGNAL_FUNC (trim_released), this );
		}
		this->seek_flag = 1;
	}

	if ( instance != NULL )
	{
		char *server = strdup( instance );
		char *port = strchr( server, ':' );
		if ( port != NULL )
			*port ++ = '\0';
		else
			port = "5250";
		instance_connect( this, server, port );
		free( server );
	}


	return this;
}

/** Get the app window widget.
*/

GtkWidget *dv1394app_get_widget( dv1394app this )
{
	return this->window;
}

/** Get the applications parser.
*/

mvcp_parser dv1394app_get_parser( dv1394app this )
{
	return this->parser;
}

/** Return the command parser.
*/

mvcp dv1394app_get_command( dv1394app this )
{
	return this->command;
}

/** Issue a connect to all pages.
*/

void dv1394app_connect( dv1394app this )
{
	int index = 0;
	for ( index = 0; index < this->page_count; index ++ )
		page_on_connect( this->pages[ index ] );
}

/** Inform all pages that the unit has changed.
*/

void dv1394app_on_unit_change( dv1394app this, int unit )
{
	int index = 0;
	this->selected_unit = unit;
	for ( index = 0; index < this->page_count; index ++ )
		page_on_unit_change( this->pages[ index ], unit );
}

/** Return the selected unit.
*/

int dv1394app_get_selected_unit( dv1394app this )
{
	return this->selected_unit;
}

/** Issue a disconnect to all pages.
*/

void dv1394app_disconnect( dv1394app this )
{
	int index = 0;
	if ( this->parser != NULL )
	{
		for ( index = 0; index < this->page_count; index ++ )
			page_on_disconnect( this->pages[ index ] );
		mvcp_close( this->command );
		this->command = NULL;
		mvcp_parser_close( this->parser );
		this->parser = NULL;
	}
}

/** Close application.
*/

void dv1394app_close( dv1394app this )
{
	dv1394app_disconnect( this );
	while ( this->page_count > 0 )
		page_close( this->pages[ -- this->page_count ] );
	free( this );
}
