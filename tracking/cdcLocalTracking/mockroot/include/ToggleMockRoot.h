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

//This compiler macro toggles whether the whole local tracking code is compiled with or without ROOT support

//This means that all dataholder objects are not inheriting from TObject and no dictionaries are created if mocking is on.
//Consequently there can be no StoreArrays and StoreObjPtr of tracking objects


#define USE_ROOT_IN_CDCLOCALTRACKING

//The mocking objects and bases are defined in the MockRoot.h


#endif //TOGGLEMOCKROOT_H_  
