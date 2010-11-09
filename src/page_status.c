/*
 * page_status.c -- Status Page Handling
 * Copyright (C) 2002-2003 Charles Yates <charles.yates@pandora.be>
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

#include "interface.h"
#include "support.h"
#include "dv1394app.h"
#include "util.h"
#include "page.h"

typedef struct 
{
	struct page_t parent;
	dv1394app app;
	GtkWidget *widget;
	valerie status;
	int terminated;
	pthread_t status_thread;
	guint context;
	int unit;
	int count;
}
*page_status, page_status_t;

static GtkWidget *this_page_get_widget( page );

/** Show the status associated to a unit.
*/

static int show_status( page_status this, valerie_status status )
{
	GtkWidget *widget = this_page_get_widget( ( page )this );
	char temp[ 1024 ] = "";
	char temp2[ 1024 ];
	char label_name[ 256 ];

	sprintf( temp, "%05d %05d %05d %s [", status->in, status->position, status->out, status->clip );
	
	switch( status->status )
	{
		case unit_offline:
			strcat( temp, "offline" );
			break;
		case unit_undefined:
			strcat( temp, "undefined" );
			break;
		case unit_not_loaded:
			strcat( temp, "unloaded" );
			break;
		case unit_stopped:
			strcat( temp, "stopped" );
			break;
		case unit_playing:
			strcat( temp, "playing" );
			break;
		case unit_paused:
			strcat( temp, "paused" );
			break;
		case unit_disconnected:
			strcat( temp, "disconnect" );
			break;
		default:
			strcat( temp, "unknown" );
			break;
	}

	strcat( temp, "]" );
	
	sprintf( label_name, "label_unit_%d", status->unit );

	gdk_threads_enter();
	widget = lookup_widget( widget, label_name );
	gtk_label_set_text( GTK_LABEL( widget ), temp );
	if ( status->unit == dv1394app_get_selected_unit( this->app ) )
	{
		sprintf( temp2, "U%d - %s", status->unit, temp );
		widget = lookup_widget( dv1394app_get_widget( this->app ), "statusbar" );
		gtk_statusbar_pop( GTK_STATUSBAR( widget ), this->context );
		gtk_statusbar_push( GTK_STATUSBAR( widget ), this->context, temp2 );
		dv1394app_show_status( this->app, status );
	}
	gdk_flush();
	gdk_threads_leave();
	
	return status->unit >= this->count;
}

static void show_units( page_status this, gboolean active )
{
	int index = 0;
	int index2 = 0;

	GtkWidget *widget;
	char temp[ 1024 ] = "";
	char button_name[ 256 ];
	
	valerie_units units = NULL;
	valerie_unit_entry_t unit;
	int unit_count = 0;
	
	valerie_nodes nodes = NULL;
	valerie_node_entry_t node;
	int node_count = 0;

	if ( active )
	{
		units = valerie_units_init( this->status );
		unit_count = valerie_units_count( units );
		nodes = valerie_nodes_init( this->status );
		node_count = valerie_nodes_count( nodes );
		this->count = unit_count;
	}
	
	gdk_threads_enter();
	
	for ( index = 0; index < MAX_UNITS; index ++ )
	{
		if ( index < unit_count )
		{
			valerie_units_get( units, index, &unit );
			
			for ( index2 = 0; index2 < node_count; index2 ++ )
			{
				valerie_nodes_get( nodes, index2, &node );
				if ( !strcmp( node.guid, unit.guid ) )
					break;
			}
			
			if ( index2 < node_count )
				sprintf( temp, "U%d - %s (%s)", unit.unit, node.name, unit.guid );
			else
				sprintf( temp, "U%d - %s", unit.unit, unit.guid );
			
			sprintf( button_name, "radiobutton_%d", index );
			widget = lookup_widget( this_page_get_widget( ( page )this ), button_name );
			gtk_widget_show( widget );
			gtk_button_set_label( GTK_BUTTON( widget ), temp );
			sprintf( button_name, "label_unit_%d", index );
			widget = lookup_widget( this_page_get_widget( ( page )this ), button_name );
			gtk_widget_show( widget );
		}
		else
		{
			sprintf( button_name, "radiobutton_%d", index );
			widget = lookup_widget( this_page_get_widget( ( page )this ), button_name );
			gtk_widget_hide( widget );
			sprintf( button_name, "label_unit_%d", index );
			widget = lookup_widget( this_page_get_widget( ( page )this ), button_name );
			gtk_widget_hide( widget );
		}
	}
	gdk_flush();
	gdk_threads_leave();			

	valerie_notifier notifier = valerie_get_notifier( this->status );
	valerie_status_t status;

	for ( index = 0; index < MAX_UNITS; index ++ )
	{
		status.unit = index;
		if ( !active )
			status.status = unit_disconnected;
		else
			valerie_notifier_get( notifier, &status, index );
		show_status( this, &status );		
	}
		
	if ( active )
	{
		valerie_nodes_close( nodes );
		valerie_units_close( units );
	}
}

/** Status monitoring thread.
*/

static void *status_thread( void *arg )
{
	page_status this = arg;
	valerie_notifier notifier = valerie_get_notifier( this->status );
	valerie_status_t status;

	show_units( this, TRUE );
	
	while ( !this->terminated )
	{
		if ( valerie_notifier_wait( notifier, &status ) != -1 )
		{
			if ( status.status == unit_disconnected )
				break;
			if ( show_status( this, &status ) )
				show_units( this, TRUE );
		}
	}

	show_units( this, FALSE );
	
	if ( !this->terminated )
	{
		GtkWidget *widget = lookup_widget( dv1394app_get_widget( this->app ), "button_disconnect" );
		gdk_threads_enter();
		gtk_signal_emit_by_name( GTK_OBJECT( widget ), "clicked" );
		gdk_threads_leave();
	}
	
	return NULL;
}

void on_radiobutton_toggled( GtkToggleButton *togglebutton, gpointer user_data )
{
	page_status this = user_data;
	dv1394app app = this->app;
	int index = 0;
	GtkWidget *widget = this_page_get_widget( ( page )this );
	for ( index = 0; index < MAX_UNITS; index ++ )
	{
		char button_name[ 256 ];
		sprintf( button_name, "radiobutton_%d", index );
		if ( lookup_widget( widget, button_name ) == GTK_WIDGET( togglebutton ) )
			break;
	}
	if ( index < MAX_UNITS )
	{
		valerie_status_t status;
		valerie_notifier notifier = valerie_get_notifier( this->status );
		dv1394app_on_unit_change( app, index );
		valerie_notifier_get( notifier, &status, index );
		gdk_threads_leave( );
		show_status( this, &status );
		gdk_threads_enter( );
	}
}

static GtkWidget *this_page_get_widget( page super )
{
	page_status this = ( page_status )super;
	if ( this->widget == NULL )
		this->widget = create_page_status( );
	return this->widget;
}

static void this_page_get_toolbar_info( page super, GtkIconSize size, GtkWidget **icon, char **label )
{
	*icon = gtk_image_new_from_stock( "gtk-execute",  size );
	*label = "_Units";
}

static void this_page_on_connect( page super )
{
	page_status this = ( page_status )super;
	if ( this->terminated )
	{
		this->terminated = 0;
		this->status = valerie_init( dv1394app_get_parser( this->app ) );
		pthread_create( &this->status_thread, NULL, status_thread, this );
	}
}

static void this_page_on_disconnect( page super )
{
	page_status this = ( page_status )super;
	if ( !this->terminated )
	{
		GtkWidget *widget;
		this->terminated = 1;
		gdk_threads_leave();
		pthread_join( this->status_thread, NULL );
		gdk_threads_enter();
		valerie_close( this->status );
		widget = lookup_widget( dv1394app_get_widget( this->app ), "statusbar" );
		gtk_statusbar_push( GTK_STATUSBAR( widget ), this->context, "Disconnected." );
	}
}

static void this_page_close( page super )
{
	page_status this = ( page_status )super;
	if ( this != NULL )
		free( this );
}

page page_status_init( dv1394app app )
{
	page_status this = calloc( 1, sizeof( page_status_t ) );
	int index = 0;
	GtkWidget *widget;
	
	this->parent.get_widget = this_page_get_widget;
	this->parent.get_toolbar_info = this_page_get_toolbar_info;
	this->parent.on_connect = this_page_on_connect;
	this->parent.on_disconnect = this_page_on_disconnect;
	this->parent.close = this_page_close;
	this->app = app;
	this->terminated = 1;

	for ( index = 0; index < MAX_UNITS; index ++ )
	{
		char button_name[ 256 ];
		sprintf( button_name, "radiobutton_%d", index );
		widget = lookup_widget( this_page_get_widget( ( page )this ), button_name );
		gtk_signal_connect( GTK_OBJECT( widget ), "toggled", GTK_SIGNAL_FUNC( on_radiobutton_toggled ), this );
	}

	widget = lookup_widget( dv1394app_get_widget( this->app ), "statusbar" );
	this->context = gtk_statusbar_get_context_id( GTK_STATUSBAR( widget ), "info" );
	gtk_statusbar_push( GTK_STATUSBAR( widget ), this->context, "Disconnected." );

	return ( page )this;
}
