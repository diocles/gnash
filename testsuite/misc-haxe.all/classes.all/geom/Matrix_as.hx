// Matrix_as.hx:  ActionScript 3 "Matrix" class, for Gnash.
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
import flash.display.MovieClip;
#end
#if flash8
import flash.MovieClip;
#end
#if !(flash6 || flash7)
import flash.geom.Matrix;
#end
import flash.Lib;
import Type;
import Std;

// import our testing API
import DejaGnu;

// Class must be named with the _as suffix, as that's the same name as the file.
class Matrix_as {
    static function main() {
#if !(flash6 || flash7)
        var x1:Matrix = new Matrix();

        // Make sure we actually get a valid class        
        if (Std.is(x1, Matrix)) {
            DejaGnu.pass("Matrix class exists");
        } else {
            DejaGnu.fail("Matrix class doesn't exist");
        }
// Tests to see if all the properties exist. All these do is test for
// existance of a property, and don't test the functionality at all. This
// is primarily useful only to test completeness of the API implementation.
	//FIXME: gnash uses incorrect data type Int
	if (Type.typeof(x1.a) == ValueType.TFloat) {
	    DejaGnu.xpass("Matrix.a property exists");
	} else {
	    DejaGnu.xfail("Matrix.a property should be float, returns type "+Type.typeof(x1.a));
	}
	//FIXME: gnash uses incorrect data type Int
	if (Type.typeof(x1.b) == ValueType.TFloat) {
	    DejaGnu.xpass("Matrix.b property exists");
	} else {
	    DejaGnu.xfail("Matrix.b property should be float, returns type "+Type.typeof(x1.b));
	}
	//FIXME: gnash uses incorrect data type Int
	if (Type.typeof(x1.c) == ValueType.TFloat) {
	    DejaGnu.xpass("Matrix.c property exists");
	} else {
	    DejaGnu.xfail("Matrix.c property should be float, returns type "+Type.typeof(x1.c));
	}
	//FIXME: gnash uses incorrect data type Int
	if (Type.typeof(x1.d) == ValueType.TFloat) {
	    DejaGnu.xpass("Matrix.d property exists");
	} else {
	    DejaGnu.xfail("Matrix.d property should be float, returns type "+Type.typeof(x1.d));
	}
	//FIXME: gnash uses incorrect data type Int
	if (Type.typeof(x1.tx) == ValueType.TFloat) {
	    DejaGnu.xpass("Matrix.tx property exists");
	} else {
	    DejaGnu.xfail("Matrix.tx property should be float, returns type "+Type.typeof(x1.tx));
	}
	//FIXME: gnash uses incorrect data type Int
	if (Type.typeof(x1.ty) == ValueType.TFloat) {
	    DejaGnu.xpass("Matrix.ty property exists");
	} else {
	    DejaGnu.xfail("Matrix.ty property should be float, returns type "+Type.typeof(x1.ty));
	}

// Tests to see if all the methods exist. All these do is test for
// existance of a method, and don't test the functionality at all. This
// is primarily useful only to test completeness of the API implementation.
// 	if (x1.clone == Matrix) {
// 	    DejaGnu.pass("Matrix::clone() method exists");
// 	} else {
// 	    DejaGnu.fail("Matrix::clone() method doesn't exist");
// 	}
	if (Type.typeof(x1.concat) == ValueType.TFunction) {
	    DejaGnu.pass("Matrix::concat() method exists");
	} else {
	    DejaGnu.fail("Matrix::concat() method doesn't exist");
	}
	if (Type.typeof(x1.createBox) == ValueType.TFunction) {
	    DejaGnu.pass("Matrix::createBox() method exists");
	} else {
	    DejaGnu.fail("Matrix::createBox() method doesn't exist");
	}
	if (Type.typeof(x1.createGradientBox) == ValueType.TFunction) {
	    DejaGnu.pass("Matrix::createGradientBox() method exists");
	} else {
	    DejaGnu.fail("Matrix::createGradientBox() method doesn't exist");
	}
 	if (Type.typeof(x1.deltaTransformPoint) == ValueType.TFunction) {
 	    DejaGnu.pass("Matrix::deltaTransformPoint() method exists");
 	} else {
 	    DejaGnu.fail("Matrix::deltaTransformPoint() method doesn't exist");
 	}
	if (Type.typeof(x1.identity) == ValueType.TFunction) {
	    DejaGnu.pass("Matrix::identity() method exists");
	} else {
	    DejaGnu.fail("Matrix::identity() method doesn't exist");
	}
	if (Type.typeof(x1.invert) == ValueType.TFunction) {
	    DejaGnu.pass("Matrix::invert() method exists");
	} else {
	    DejaGnu.fail("Matrix::invert() method doesn't exist");
	}
	if (Type.typeof(x1.rotate) == ValueType.TFunction) {
	    DejaGnu.pass("Matrix::rotate() method exists");
	} else {
	    DejaGnu.fail("Matrix::rotate() method doesn't exist");
	}
	if (Type.typeof(x1.scale) == ValueType.TFunction) {
	    DejaGnu.pass("Matrix::scale() method exists");
	} else {
	    DejaGnu.fail("Matrix::scale() method doesn't exist");
	}
	if (Type.typeof(x1.toString) == ValueType.TFunction) {
	    DejaGnu.pass("Matrix::toString() method exists");
	} else {
	    DejaGnu.fail("Matrix::toString() method doesn't exist");
	}
 	if (Type.typeof(x1.transformPoint) == ValueType.TFunction) {
 	    DejaGnu.pass("Matrix::transformPoint() method exists");
 	} else {
 	    DejaGnu.fail("Matrix::transformPoint() method doesn't exist");
 	}
	if (Type.typeof(x1.translate) == ValueType.TFunction) {
	    DejaGnu.pass("Matrix::translate() method exists");
	} else {
	    DejaGnu.fail("Matrix::translate() method doesn't exist");
	}

        // Call this after finishing all tests. It prints out the totals.
        DejaGnu.done();
#else
    DejaGnu.note("This class (Matrix) is only available in flash8 and flash9");
#end
    }
}

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

