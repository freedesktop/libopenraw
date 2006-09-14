/*
 * libopenraw - debug.h
 *
 * Copyright (C) 2006 Hubert Figuiere
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


#ifndef _OPENRAW_DEBUG_H_
#define _OPENRAW_DEBUG_H_



namespace Debug {
	
	typedef enum _debug_level {
		ERROR = 0,
		WARNING,
		NOTICE,
		DEBUG1,
		DEBUG2
	} debug_level;

	extern int debugLevel;
	
	/** a basic Trace class for debug */
	class Trace 
	{
	public:
		
		Trace(debug_level level)
			: m_level(level)
			{
			}
		Trace & operator<<(int i);
		Trace & operator<<(const char * s);
		Trace & operator<<(void *);
	private:
		int m_level;
	};
	
}

#endif
