/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TOGGLEMOCKROOT_H_
#define TOGGLEMOCKROOT_H_


// This file is DEPRICATED
// The inheritance to ROOT can now be switch by defining CDCLOCALTRACKING_USE_ROOT

// Instead of doing defining it in this file you rather can now give a compiler flag
// -DCDCLOCALTRACKING_USE_ROOT. Forgeting to give the flag should result in the correct
// default behaviour, which is to build without ROOT support. It also avoids that one
// accidentally checks in a version with ROOT switch on.

// In the scons build system you can activate the additional compile time macro using
// scons --extra-ccflags='-DCDCLOCALTRACKING_USE_ROOT'

// The mocking objects and bases are defined in the MockRoot.h


/* originally was :

//This compiler macro toggles whether the whole local tracking code is compiled with or without ROOT support

//This means that all dataholder objects are not inheriting from TObject and no dictionaries are created if mocking is on.
//Consequently there can be no StoreArrays and StoreObjPtr of tracking objects
*/

#ifdef CDCLOCALTRACKING_USE_ROOT

#define USE_ROOT_IN_CDCLOCALTRACKING

#endif





#endif //TOGGLEMOCKROOT_H_  
