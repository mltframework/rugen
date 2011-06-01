/*
 * dv1394app.h -- GTK+ 2 dv1394d client demo
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
 
#ifndef _DV1394_APP_
#define _DV1394_APP_

#include <gtk/gtk.h>
#include <mvcp/mvcp.h>

#define TRANSPORT_BUTTONS_COUNT 13

typedef struct page_t *page;
	
typedef struct
{
	GtkWidget *window;
	GtkWidget *connect;
	mvcp_parser parser;
	mvcp command;
	int page_count;
	GtkWidget *page_buttons[ 10 ];
	page pages[ 10 ];
	int selected_unit;
	
	// TODO: This comes out later
	GtkWidget *buttons[ TRANSPORT_BUTTONS_COUNT ];
	
	int trim_in_use;
	int seek_flag;
	
	int trim_in;
	int trim_out;
	int guard;
	int eof;
}
*dv1394app, dv1394app_t;

extern dv1394app dv1394app_init( GtkWidget *, char * );
extern GtkWidget *dv1394app_get_widget( dv1394app );
extern mvcp_parser dv1394app_get_parser( dv1394app );
extern mvcp dv1394app_get_command( dv1394app );
extern void dv1394app_connect( dv1394app );
extern void dv1394app_on_unit_change( dv1394app, int );
extern int dv1394app_get_selected_unit( dv1394app );
extern void dv1394app_show_status( dv1394app, mvcp_status );
extern void dv1394app_disconnect( dv1394app );
extern void dv1394app_close( dv1394app );

#endif

