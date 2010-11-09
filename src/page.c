/*
 * page.c -- Page handling
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

#include <page.h>
#include <gtk/gtk.h>

GtkWidget *page_get_widget( page this )
{
	return this->get_widget( this );
}

void page_get_toolbar_info( page this, GtkIconSize size, GtkWidget **icon, char **label )
{
	if ( this->get_toolbar_info )
	{
		this->get_toolbar_info( this, size, icon, label );
	}
	else
	{
		*icon = gtk_image_new_from_stock( "gtk-execute",  size );
		*label = "_Unknown";
	}
}

/** Called on connection being established to a server.
*/

void page_on_connect( page this )
{
	if ( this->on_connect )
		this->on_connect( this );
}

/** Called on a disconnection from a server.
*/

void page_on_disconnect( page this )
{
	if ( this->on_disconnect )
		this->on_disconnect( this );
}

/** Called when the user changes the selected unit.
*/

void page_on_unit_change( page this, int unit )
{
	if ( this->on_unit_change )
		this->on_unit_change( this, unit );
}

/** Called to propogate status information to any page that's interested.
*/

void page_show_status( page this, mvcp_status status )
{
	if ( this->show_status )
		this->show_status( this, status );
}

/** Called on close.
*/

void page_close( page this )
{
	if ( this->close )
		this->close( this );
}

