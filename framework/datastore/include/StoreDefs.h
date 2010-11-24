/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/** typedefs and enums for the DataStore.
 *
 *  @file The typedefs are specifications for templateised maps.
 */

#ifndef STOREDEF_H
#define STOREDEF_H

#include <TObject.h>
#include <TClonesArray.h>

#include <map>
#include <string>


namespace Belle2 {

  typedef std::map<std::string, TObject*> StoreObjMap;         /**< Map for TObjects. */
  typedef std::map<std::string, TClonesArray*> StoreArrayMap;  /**< Map for TClonesArrays. */
  typedef StoreObjMap::iterator StoreObjIter;                  /**< Iterator for TObjectMap. */
  typedef StoreArrayMap::iterator StoreArrayIter;              /**< Iterator for TClonesArraysMap.*/

  /** Durability types.
   *
   *  These types are used to identify the map used in the DataStore.
   *  @author <a href="mailto:martin.heck@kit.edu?subject=EDurability">Martin Heck</a>
   */
  enum EDurability {
    c_Event,     /**< Object is deleted after event. */
    c_Run,       /**< Object is deleted after run. */
    c_Persistent /**< Object is persistent. */
  };

  /** Number of Durability Types.
   *
   *  Probably useless, but in principle additional maps are easily created this way.
   */
  enum ENDurabilityTypes {
    c_NDurabilityTypes = 3 /**< Total number of durability types. */
  };
}

#endif //EDURABILITY_H


