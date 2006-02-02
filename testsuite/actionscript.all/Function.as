// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License

// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//

// Test case for Function ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf

var tmp = new Function;

// test the Function constuctor
if (tmp) {
	trace("PASSED: Function::Function() constructor");
} else {
	trace("FAILED: Function::Function()");		
}

// test the Function::apply method
if (tmp.apply) {
	trace("PASSED: Function::apply() exists");
} else {
	trace("FAILED: Function::apply() doesn't exist");
}
// test the Function::call method
if (tmp.call) {
	trace("PASSED: Function::call() exists");
} else {
	trace("FAILED: Function::call() doesn't exist");
}
