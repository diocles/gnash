// RemoveObjectTag.h: RemoveObject* tag for Gnash.
// 
//   Copyright (C) 2007, 2008 Free Software Foundation, Inc.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA


#ifndef GNASH_SWF_REMOVEOBJECTTAG_H
#define GNASH_SWF_REMOVEOBJECTTAG_H

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "DisplayListTag.h" // for inheritance
#include "swf.h" // for tag_type definition

// Forward declarations
namespace gnash {
	class SWFStream;
	class sprite_instance;
	class swf_event;
	class movie_definition;
	class DisplayList;
}

namespace gnash {
namespace SWF {

/// SWF Tag RemoveObject (5) or RemoveObject2 (28)
//
/// The RemoveObject tag removes the character instance at the specified depth.
///
/// TODO: make this and PlaceObject2Tag subclasses of DisplayListTag (subclass of ControlTag)
///
class RemoveObjectTag : public DisplayListTag
{
public:

	RemoveObjectTag()
		:
		DisplayListTag(-1),
		m_id(-1)
	{}

	/// Read SWF::REMOVEOBJECT or SWF::REMOVEOBJECT2 
	void read(SWFStream* in, tag_type tag);

	/// Remove object at specified depth from sprite_instance DisplayList.
	void execute(sprite_instance* m, DisplayList& dlist) const;

	static void loader(SWFStream* in, tag_type tag, movie_definition* m);

private:

	int m_id;

};

} // namespace gnash::SWF
} // namespace gnash


#endif // GNASH_SWF_REMOVEOBJECTTAG_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End: