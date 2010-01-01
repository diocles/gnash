// EventPhase_as.hx:  ActionScript 3 "EventPhase" class, for Gnash.
//
// Generated by gen-as3.sh on: 20090515 by "rob". Remove this
// after any hand editing loosing changes.
//
//   Copyright (C) 2009, 2010 Free Software Foundation, Inc.
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
//

// This test case must be processed by CPP before compiling to include the
//  DejaGnu.hx header file for the testing framework support.

#if flash9
import flash.events.EventPhase;
import flash.display.MovieClip;
#end
import flash.Lib;
import Type;
import Std;

// import our testing API
import DejaGnu;

// Class must be named with the _as suffix, as that's the same name as the file.
class EventPhase_as {
    static function main() {
        #if !flash9
			DejaGnu.note("this class did not exist in as2");
		#end
		
		#if flash9
		if (Std.string(flash.events.EventPhase.AT_TARGET) == "2") {
			DejaGnu.pass("EventPhase::AT_TARGET has correct constant");
		} else {
			DejaGnu.fail("EventPhase::AT_TARGET doesn't return the correct constant");
		}

		if (Std.string(flash.events.EventPhase.BUBBLING_PHASE) == "3") {
			DejaGnu.pass("EventPhase::BUBBLING_PHASE has correct constant");
		} else {
			DejaGnu.fail("EventPhase::BUBBLING_PHASE doesn't return the correct constant");
		}
		
		if (Std.string(flash.events.EventPhase.CAPTURING_PHASE) == "1") {
			DejaGnu.pass("EventPhase::CAPTURING_PHASE has correct constant");
		} else {
			DejaGnu.fail("EventPhase::CAPTURING_PHASE doesn't return the correct constant");
		}

		#end
        // Call this after finishing all tests. It prints out the totals.
        DejaGnu.done();
    }
}

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

