/*
 * page.h -- Page handling
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

#ifndef _DV1394_PAGE_
#define _DV1394_PAGE_

#include "dv1394app.h"
#include <gtk/gtk.h>

/** Page structure.
*/

struct page_t
{
	GtkWidget *( *get_widget )( page );
	void ( *get_toolbar_info )( page, GtkIconSize, GtkWidget **, char ** );
	void ( *on_connect )( page );
	void ( *on_disconnect )( page );
	void ( *on_unit_change )( page, int );
	void ( *show_status )( page, mvcp_status );
	void ( *close )( page );
};

/* page api */
extern GtkWidget *page_get_widget( page );
extern void page_get_toolbar_info( page, GtkIconSize, GtkWidget **, char ** );
extern void page_on_connect( page );
extern void page_on_disconnect( page );
extern void page_on_unit_change( page, int );
extern void page_show_status( page, mvcp_status );
extern void page_close( page );

/* page factories */
extern page page_clips_init( dv1394app );
extern page page_command_init( dv1394app );
extern page page_status_init( dv1394app );
extern page page_units_init( dv1394app );

#endif
