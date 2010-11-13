/*
 * page_clips.c -- Clips Page Handling
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

typedef struct 
{
	struct page_t parent;
	dv1394app app;
	GtkWidget *widget;
	mvcp dv;
	char *path;
	int unit;
	int generation;
	int clip;
	
	// TODO: This comes out later
	int mode;
	GtkWidget *modes[ 4 ];
}
*page_clips, page_clips_t;

static GtkWidget *this_page_get_widget( page_clips this );

static void list_clips( page_clips this, char *path )
{
	GtkWidget *treeview = lookup_widget( this_page_get_widget( this ), "list_clips" );
	GtkWidget *dirview = lookup_widget( this_page_get_widget( this ), "list_dir" );
	GtkWidget *label = lookup_widget( this_page_get_widget( this ), "label_directory" );
	if ( path != NULL )
	{
		mvcp_dir dir = mvcp_dir_init( this->dv, path );
		GtkListStore *dir_store = NULL;
		GtkListStore *clip_store = NULL;
		GtkTreeIter iter;
		int index;
		mvcp_dir_entry_t entry;

		free( this->path );
		this->path = strdup( path );
		
		gtk_label_set_text( GTK_LABEL( label ), this->path );
		
		if (  gtk_tree_view_get_model( GTK_TREE_VIEW( treeview ) ) == NULL )
		{
			GtkCellRenderer *renderer;
			GtkTreeViewColumn *column;

			clip_store = gtk_list_store_new( 2, G_TYPE_STRING, G_TYPE_STRING );
			gtk_tree_view_set_model( GTK_TREE_VIEW( treeview ), GTK_TREE_MODEL( clip_store ) );
			
			renderer = gtk_cell_renderer_text_new( );
			column = gtk_tree_view_column_new_with_attributes ( "Description", renderer, "text", 0, NULL);
			gtk_tree_view_column_set_sort_column_id( column, 0 );
			gtk_tree_view_append_column( GTK_TREE_VIEW( treeview ), column );
			
			dir_store = gtk_list_store_new( 2, G_TYPE_STRING, G_TYPE_STRING );
			gtk_tree_view_set_model( GTK_TREE_VIEW( dirview ), GTK_TREE_MODEL( dir_store ) );
			
			renderer = gtk_cell_renderer_text_new( );
			column = gtk_tree_view_column_new_with_attributes ( "Description", renderer, "text", 0, NULL);
			gtk_tree_view_column_set_sort_column_id( column, 0 );
			gtk_tree_view_append_column( GTK_TREE_VIEW( dirview ), column );			
		}
		else
		{
			clip_store = GTK_LIST_STORE( gtk_tree_view_get_model( GTK_TREE_VIEW( treeview ) ) );
			dir_store = GTK_LIST_STORE( gtk_tree_view_get_model( GTK_TREE_VIEW( dirview ) ) );
			gtk_list_store_clear( clip_store );
			gtk_list_store_clear( dir_store );
		}
		
		if ( strcmp( path, "/" ) )
		{
			gtk_list_store_append( clip_store, &iter );
			gtk_list_store_set( clip_store, &iter, 0, "..", -1 );
			gtk_list_store_append( dir_store, &iter );
			gtk_list_store_set( dir_store, &iter, 0, "..", -1 );
		}
		
		for ( index = 0; index < mvcp_dir_count( dir ); index ++ )
		{
			mvcp_dir_get( dir, index, &entry );
			if ( strchr( entry.name, '/' ) )
			{
				gtk_list_store_append( dir_store, &iter );
				gtk_list_store_set( dir_store, &iter, 0, entry.name, -1 );
			}
			else
			{
				gtk_list_store_append( clip_store, &iter );
				gtk_list_store_set( clip_store, &iter, 0, entry.name, -1 );
			}
		}
                                             
		mvcp_dir_close( dir );
	}
	else
	{
		gtk_label_set_text( GTK_LABEL( label ), "Disconnected" );
		if ( gtk_tree_view_get_model( GTK_TREE_VIEW( treeview ) ) )
		{
			GtkListStore *list_store = GTK_LIST_STORE( gtk_tree_view_get_model( GTK_TREE_VIEW( treeview ) ) );
			gtk_list_store_clear( list_store );
			list_store = GTK_LIST_STORE( gtk_tree_view_get_model( GTK_TREE_VIEW( dirview ) ) );
			gtk_list_store_clear( list_store );
			treeview = lookup_widget( this_page_get_widget( this ), "treeview1" );
			gtk_list_store_clear( list_store );
		}
	}
}

static void list_queue( page_clips this, int clip )
{
	GtkWidget *treeview = lookup_widget( this_page_get_widget( this ), "treeview1" );
	mvcp_list list = mvcp_list_init( this->dv, dv1394app_get_selected_unit( this->app ) );
	GtkListStore *list_store = NULL;
	GtkTreeIter iter;
	GtkTreePath *path;
	int index;
	mvcp_list_entry_t entry;

	if ( gtk_tree_view_get_model( GTK_TREE_VIEW( treeview ) ) == NULL )
	{
		GtkCellRenderer *renderer;
		GtkTreeViewColumn *column;

		list_store = gtk_list_store_new( 6, G_TYPE_BOOLEAN, G_TYPE_INT, G_TYPE_INT, G_TYPE_INT, G_TYPE_STRING, G_TYPE_INT );
		gtk_tree_view_set_model( GTK_TREE_VIEW( treeview ), GTK_TREE_MODEL( list_store ) );
		
		renderer = gtk_cell_renderer_toggle_new( );
		column = gtk_tree_view_column_new_with_attributes ( "", renderer, "active", 0, NULL);
		gtk_tree_view_append_column( GTK_TREE_VIEW( treeview ), column );	
		
		renderer = gtk_cell_renderer_text_new( );
		column = gtk_tree_view_column_new_with_attributes ( "In", renderer, "text", 1, NULL);
		gtk_tree_view_append_column( GTK_TREE_VIEW( treeview ), column );
		
		renderer = gtk_cell_renderer_text_new( );
		column = gtk_tree_view_column_new_with_attributes ( "Out", renderer, "text", 2, NULL);
		gtk_tree_view_append_column( GTK_TREE_VIEW( treeview ), column );
		
		renderer = gtk_cell_renderer_text_new( );
		column = gtk_tree_view_column_new_with_attributes ( "Length", renderer, "text", 3, NULL);
		gtk_tree_view_append_column( GTK_TREE_VIEW( treeview ), column );
		
		renderer = gtk_cell_renderer_text_new( );
		column = gtk_tree_view_column_new_with_attributes ( "Clip", renderer, "text", 4, NULL);
		gtk_tree_view_append_column( GTK_TREE_VIEW( treeview ), column );
	}
	else
	{
		list_store = GTK_LIST_STORE( gtk_tree_view_get_model( GTK_TREE_VIEW( treeview ) ) );
		gtk_list_store_clear( list_store );
	}
	
	this->generation = list->generation;
	
	for ( index = 0; index < mvcp_list_count( list ); index ++ )
	{
		mvcp_list_get( list, index, &entry );
		gtk_list_store_append( list_store, &iter );
		gtk_list_store_set( list_store, &iter, 0, index == clip, 1, ( int )entry.in, 2, ( int )entry.out, 3, ( int )entry.size, 4, entry.full, 5, entry.clip, -1 );
	}

	this->clip = clip;
	if ( clip < mvcp_list_count( list ) )
	{
		path = gtk_tree_path_new_from_indices( this->clip, -1 );
		gtk_tree_view_scroll_to_cell( GTK_TREE_VIEW( treeview ), path, NULL, TRUE, 0.5, 0 );
		gtk_tree_path_free( path );
	}
	
	mvcp_list_close( list );
}

static void list_active( page_clips this, int clip )
{
	GtkWidget *treeview = lookup_widget( this_page_get_widget( this ), "treeview1" );
	GtkListStore *list_store = GTK_LIST_STORE( gtk_tree_view_get_model( GTK_TREE_VIEW( treeview ) ) );
	GtkTreePath *path = gtk_tree_path_new_from_indices( this->clip, -1 );
	GtkTreeIter iter;

	gtk_tree_model_get_iter( GTK_TREE_MODEL (list_store), &iter, path );
	gtk_tree_path_free( path );
	gtk_list_store_set( list_store, &iter, 0, FALSE, -1 );
	
	this->clip = clip;
	path = gtk_tree_path_new_from_indices( this->clip, -1 );
	gtk_tree_view_scroll_to_cell( GTK_TREE_VIEW( treeview ), path, NULL, TRUE, 0.5, 0 );
	gtk_tree_model_get_iter( GTK_TREE_MODEL (list_store), &iter, path );
	gtk_tree_path_free( path );
	gtk_list_store_set( list_store, &iter, 0, TRUE, -1 );
}

static gboolean on_ok( GtkWidget *dummy, gpointer data )
{
	page_clips this = data;
	GtkWidget *widget = lookup_widget( this_page_get_widget( this ), "list_clips" );
	GtkTreeSelection *select = gtk_tree_view_get_selection( GTK_TREE_VIEW( widget ) );
	GtkTreeModel *model;
	GtkTreeIter iter;
	gchar *text;
	
	if ( gtk_tree_selection_get_selected( select, &model, &iter ) )
	{
		gtk_tree_model_get( model, &iter, 0, &text,	-1 );
		
		if ( !strcmp( text, ".." ) )
		{
			char *temp = strdup( this->path );
			temp[ strlen( temp ) - 1 ] = '\0';
			*( strrchr( temp, '/' ) + 1 ) = '\0';
			list_clips( this, temp );
			free( temp );
		}
		else
		{
			char *temp = malloc( strlen( this->path ) + strlen( text ) + 1 );
			strcpy( temp, this->path );
			strcat( temp, text );
			switch( this->mode )
			{
				case 0:
					mvcp_unit_load_back( this->dv, dv1394app_get_selected_unit( this->app ), temp );
					mvcp_unit_play( this->dv, dv1394app_get_selected_unit( this->app ) );
					break;
				case 1: 
					mvcp_unit_load( this->dv, dv1394app_get_selected_unit( this->app ), temp );
					break;
				case 2: 
					mvcp_unit_append( this->dv, dv1394app_get_selected_unit( this->app ), temp, -1, -1 );
					break;
				case 3:
				{
					GtkWidget *widget = lookup_widget( this_page_get_widget( this ), "treeview1" );
					GtkTreeSelection *select = gtk_tree_view_get_selection( GTK_TREE_VIEW( widget ) );
					GtkTreeModel *model;
					GtkTreeIter iter;
					int clip;
					
					if ( gtk_tree_selection_get_selected( select, &model, &iter ) )
					{
						gtk_tree_model_get( model, &iter, 5, &clip,	-1 );
						mvcp_unit_clip_insert( this->dv, dv1394app_get_selected_unit( this->app ), mvcp_absolute, clip, temp, -1, -1 );
					}
					break;
				}
			}
			
			free( temp );
		}
			
		g_free( text );
	}
	
	return TRUE;
}

static gboolean on_dir( GtkWidget *dummy, gpointer data )
{
	page_clips this = data;
	GtkWidget *widget = lookup_widget( this_page_get_widget( this ), "list_dir" );
	GtkTreeSelection *select = gtk_tree_view_get_selection( GTK_TREE_VIEW( widget ) );
	GtkTreeModel *model;
	GtkTreeIter iter;
	gchar *text;
	
	if ( gtk_tree_selection_get_selected( select, &model, &iter ) )
	{
		gtk_tree_model_get( model, &iter, 0, &text,	-1 );
		
		if ( !strcmp( text, ".." ) )
		{
			char *temp = strdup( this->path );
			temp[ strlen( temp ) - 1 ] = '\0';
			*( strrchr( temp, '/' ) + 1 ) = '\0';
			list_clips( this, temp );
			free( temp );
		}
		else if ( text[ strlen( text ) - 1 ] == '/' )
		{
			char *temp = malloc( strlen( this->path ) + strlen( text ) + 1 );
			strcpy( temp, this->path );
			strcat( temp, text );
			list_clips( this, temp );
			free( temp );			
		}
			
		g_free( text );
	}
	
	return TRUE;
}

static gboolean on_queue_item( GtkWidget *dummy, gpointer data )
{
	page_clips this = data;
	GtkWidget *widget = lookup_widget( this_page_get_widget( this ), "treeview1" );
	GtkTreeSelection *select = gtk_tree_view_get_selection( GTK_TREE_VIEW( widget ) );
	GtkTreeModel *model;
	GtkTreeIter iter;
	int clip;
	
	if ( gtk_tree_selection_get_selected( select, &model, &iter ) )
	{
		gtk_tree_model_get( model, &iter, 5, &clip,	-1 );
		mvcp_unit_clip_goto( this->dv, dv1394app_get_selected_unit( this->app ), mvcp_absolute, clip, 0 );
		mvcp_unit_play( this->dv, dv1394app_get_selected_unit( this->app ) );
	}
	
	return TRUE;
}

static gboolean on_clip_selected( GtkWidget *widget, GdkEventButton *event, gpointer data )
{
	if ( event->button==1 && event->type==GDK_2BUTTON_PRESS )
		return on_ok( widget, data );
	return FALSE;
}

static gboolean on_clip_key_press( GtkWidget *widget, GdkEventKey *event, gpointer data )
{
	if ( event->keyval == GDK_Return )
		return on_ok( widget, data );
	return FALSE;		
}

static gboolean on_dir_selected( GtkWidget *widget, GdkEventButton *event, gpointer data )
{
	if ( event->button==1 && event->type==GDK_2BUTTON_PRESS )
		return on_dir( widget, data );
	return FALSE;
}

static gboolean on_dir_key_press( GtkWidget *widget, GdkEventKey *event, gpointer data )
{
	if ( event->keyval == GDK_Return )
		return on_dir( widget, data );
	return FALSE;		
}

static gboolean on_queue_selected( GtkWidget *widget, GdkEventButton *event, gpointer data )
{
	if ( event->button==1 && event->type==GDK_2BUTTON_PRESS )
		return on_queue_item( widget, data );
	return FALSE;
}

static gboolean on_queue_key_press( GtkWidget *widget, GdkEventKey *event, gpointer data )
{
	if ( event->keyval == GDK_Return )
		return on_queue_item( widget, data );
	return FALSE;		
}

static gboolean on_home( GtkWidget *button, gpointer data )
{
	page_clips this = data;
	list_clips( this, "/" );
	return TRUE;
}

static gboolean on_refresh( GtkWidget *button, gpointer data )
{
	page_clips this = data;
	char *temp = strdup( this->path );
	list_clips( this, temp );
	free( temp );
	
	return TRUE;
}

static gboolean on_up( GtkWidget *dummy, gpointer data )
{
	page_clips this = data;
	GtkWidget *widget = lookup_widget( this_page_get_widget( this ), "treeview1" );
	GtkTreeSelection *select = gtk_tree_view_get_selection( GTK_TREE_VIEW( widget ) );
	GtkTreeModel *model;
	GtkTreeIter iter;
	int clip;

	if ( gtk_tree_selection_get_selected( select, &model, &iter ) )
	{
		gtk_tree_model_get( model, &iter, 5, &clip,	-1 );
		mvcp_unit_clip_move( this->dv, dv1394app_get_selected_unit( this->app ), mvcp_absolute, clip, mvcp_absolute, clip - 1 < 0 ? 0 : clip - 1 );
	}
	return TRUE;
}

static gboolean on_down( GtkWidget *dummy, gpointer data )
{
	page_clips this = data;
	GtkWidget *widget = lookup_widget( this_page_get_widget( this ), "treeview1" );
	GtkTreeSelection *select = gtk_tree_view_get_selection( GTK_TREE_VIEW( widget ) );
	GtkTreeModel *model;
	GtkTreeIter iter;
	int clip;

	if ( gtk_tree_selection_get_selected( select, &model, &iter ) )
	{
		gtk_tree_model_get( model, &iter, 5, &clip,	-1 );
		mvcp_unit_clip_move( this->dv, dv1394app_get_selected_unit( this->app ), mvcp_absolute, clip, mvcp_absolute, clip + 1 );
	}
	return TRUE;
}

static gboolean on_remove( GtkWidget *dummy, gpointer data )
{
	page_clips this = data;
	GtkWidget *widget = lookup_widget( this_page_get_widget( this ), "treeview1" );
	GtkTreeSelection *select = gtk_tree_view_get_selection( GTK_TREE_VIEW( widget ) );
	GtkTreeModel *model;
	GtkTreeIter iter;
	int clip;

	if ( gtk_tree_selection_get_selected( select, &model, &iter ) )
	{
		gtk_tree_model_get( model, &iter, 5, &clip,	-1 );
		mvcp_unit_clip_remove( this->dv, dv1394app_get_selected_unit( this->app ), mvcp_absolute, clip );
	}
	return TRUE;
}

static gboolean on_clean( GtkWidget *dummy, gpointer data )
{
	page_clips this = data;
	mvcp_unit_clean( this->dv, dv1394app_get_selected_unit( this->app ) );
	return TRUE;
}

void on_mode_change( GtkMenuItem *menuitem, gpointer data )
{
	page_clips this = data;
	int index = 0;
	
	for ( index = 0; index < 4; index ++ )
		if ( GTK_WIDGET( menuitem ) == this->modes[ index ] )
			break;
	
	this->mode = index;
}

static GtkWidget *this_page_get_widget( page_clips this )
{
	if ( this->widget == NULL )
		this->widget = create_page_clips( );
	return this->widget;
}

static void this_page_get_toolbar_info( page this, GtkIconSize size, GtkWidget **icon, char **label )
{
	*icon = gtk_image_new_from_stock( "gtk-justify-fill",  size );
	*label = _("_Playlist");
}

static void this_page_on_connect( page_clips this )
{
	this->dv = mvcp_init( dv1394app_get_parser( this->app ) );
	list_clips( this, "/" );
}

static void this_page_on_unit_change( page_clips this, int unit )
{
	if ( unit != this->unit )
		this->unit = unit;
}

static void this_page_on_disconnect( page_clips this )
{
	list_clips( this, NULL );
	mvcp_close( this->dv );
}

static void this_page_show_status( page_clips this, mvcp_status status )
{
	if ( status->status != unit_disconnected )
	{
		if ( this->generation != status->generation )
			list_queue( this, status->clip_index );
		else if ( this->clip != status->clip_index )
			list_active( this, status->clip_index );
	}		
}

static void this_page_close( page_clips this )
{
	if ( this != NULL )
		free( this );
}

page page_clips_init( dv1394app app )
{
	page_clips this = calloc( 1, sizeof( page_clips_t ) );
	GtkWidget *widget;
	int index = 0;

	this->parent.get_widget = ( GtkWidget *(*)( page ) )this_page_get_widget;
	this->parent.get_toolbar_info = this_page_get_toolbar_info;
	this->parent.on_connect = ( void (*)( page ) )this_page_on_connect;
	this->parent.on_unit_change = ( void (*)( page, int ) )this_page_on_unit_change;
	this->parent.on_disconnect = ( void (*)( page ) )this_page_on_disconnect;
	this->parent.show_status = ( void (*)( page, mvcp_status ) )this_page_show_status;
	this->parent.close = ( void (*)( page ) )this_page_close;
	this->app = app;
	this->generation = -1;

	widget = lookup_widget( this_page_get_widget( this ), "list_clips" );
	g_signal_connect( G_OBJECT( widget ), "button-press-event", G_CALLBACK( on_clip_selected ), this );
	g_signal_connect( G_OBJECT( widget ), "key-press-event", G_CALLBACK( on_clip_key_press ), this );
	widget = lookup_widget( this_page_get_widget( this ), "list_dir" );
	g_signal_connect( G_OBJECT( widget ), "button-press-event", G_CALLBACK( on_dir_selected ), this );
	g_signal_connect( G_OBJECT( widget ), "key-press-event", G_CALLBACK( on_dir_key_press ), this );
	widget = lookup_widget( this_page_get_widget( this ), "button_clips_refresh" );
	gtk_signal_connect( GTK_OBJECT( widget ), "clicked", GTK_SIGNAL_FUNC( on_refresh ), this );
	widget = lookup_widget( this_page_get_widget( this ), "button_clips_home" );
	gtk_signal_connect( GTK_OBJECT( widget ), "clicked", GTK_SIGNAL_FUNC( on_home ), this );
	widget = lookup_widget( this_page_get_widget( this ), "button_up" );
	gtk_signal_connect( GTK_OBJECT( widget ), "clicked", GTK_SIGNAL_FUNC( on_up ), this );
	widget = lookup_widget( this_page_get_widget( this ), "button_down" );
	gtk_signal_connect( GTK_OBJECT( widget ), "clicked", GTK_SIGNAL_FUNC( on_down ), this );
	widget = lookup_widget( this_page_get_widget( this ), "button_remove" );
	gtk_signal_connect( GTK_OBJECT( widget ), "clicked", GTK_SIGNAL_FUNC( on_remove ), this );
	widget = lookup_widget( this_page_get_widget( this ), "button_clean" );
	gtk_signal_connect( GTK_OBJECT( widget ), "clicked", GTK_SIGNAL_FUNC( on_clean ), this );

	widget = lookup_widget( this_page_get_widget( this ), "table4" );
	gtk_widget_show( widget );

	widget = lookup_widget( this_page_get_widget( this ), "treeview1" );
	g_signal_connect( G_OBJECT( widget ), "button-press-event", G_CALLBACK( on_queue_selected ), this );
	g_signal_connect( G_OBJECT( widget ), "key-press-event", G_CALLBACK( on_queue_key_press ), this );

	for ( index = 0; index < 4; index ++ )
	{
		char item[ 256 ];
		sprintf( item, "mode_%d", index );
		widget = lookup_widget( this_page_get_widget( this ), item );
		gtk_signal_connect( GTK_OBJECT( widget ), "activate", GTK_SIGNAL_FUNC( on_mode_change ), this );
		this->modes[ index ] = widget;
	}
	
	return ( page )this;
}
