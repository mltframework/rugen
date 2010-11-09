/*
 * page_units.c -- Units Page Handling
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

static void this_on_connect( page this )
{
}

static void this_on_disconnect( page this )
{
}

static void this_close( page this )
{
	if ( this != NULL )
		free( this );
}

page page_units_init( dv1394app app )
{
	page this = calloc( 1, sizeof( struct page_t ) );
	
	this->on_connect = this_on_connect;
	this->on_disconnect = this_on_disconnect;
	this->close = this_close;

	return this;
}
