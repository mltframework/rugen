/*
 * main.c -- GTK+ 2 dv1394d client demo
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

#include <gtk/gtk.h>
#include <unistd.h>
#include <string.h>

#include <stdio.h>
#include "interface.h"
#include "support.h"
#include "dv1394app.h"

int main( int argc, char *argv[] )
{
	char path[ 512 ];
	dv1394app app = NULL;
	GtkWidget *gdv1394d;

#ifdef ENABLE_NLS
	bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);
#endif
	
	g_thread_init(NULL);
	gdk_threads_init( );
	gtk_set_locale( );
	gtk_init( &argc, &argv );

	// Linux hack to determine path of the executable
	readlink( "/proc/self/exe", path, 512 );
	if ( strstr( path, "/bin/rugen" ) )
	{
		( *strstr( path, "/bin/rugen" ) ) = '\0';
		strcat( path, "/share/rugen/pixmaps" );
  		add_pixmap_directory( path );
	}
	else
	{
  		add_pixmap_directory( PACKAGE_DATA_DIR "/" PACKAGE "/pixmaps" );
	}

	gdv1394d = create_gdv1394d ();
	gtk_widget_show (gdv1394d);
	
	gdk_threads_enter();
	app = dv1394app_init( gdv1394d, argv[ 1 ] );
	gtk_main ();
	gdk_threads_leave();
	
	dv1394app_close( app );

	return 0;
}

