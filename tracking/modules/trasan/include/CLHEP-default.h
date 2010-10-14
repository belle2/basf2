/* config/CLHEP-sparc-sun-solaris2.5.1-c++2952.h.  Generated automatically by configure.  */
// -*- C++ -*-
// CLASSDOC OFF
// $Id: CLHEP-default.h 10030 2007-03-09 07:51:44Z katayama $
// ---------------------------------------------------------------------------
// CLASSDOC ON
//
// This file is a part of the CLHEP - a Class Library for High Energy Physics.
//
// This file should define some platform dependent features necessary for
// the CLHEP class library. Go through it and change the definition of the
// macros to suit you platform.
//
#ifndef BELLE_CLHEP_COMPILER_H_
#define BELLE_CLHEP_COMPILER_H_

// Define if your FORTRAN compiler post-pends an underscore on all
// routine names. This is done automatically by the configure script.
//
#define FORTRAN_PPU 1


// Define if the definition of the "abs" template requires parameter
// without const. That was noticed for g++.
// This is done automatically by the configure script.
//
/* #undef HEP_ABS_NEEDS_PARAMETER_WITHOUT_CONST */

// Define if your C++ compiler requires the "sub" function (see the
// Matrix/ module) without const. Such a bug was noticed for some
// versions of DEC CXX, SGI CC and HP aCC.
// This is done automatically by the configure script.
//
/* #undef HEP_CC_NEED_SUB_WITHOUT_CONST */

#endif /* _CLHEP_COMPILER_H_ */
