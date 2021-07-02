/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <string>

class TObject;
class TClass;
class TClonesArray;

namespace Belle2 {
  /** Wraps a stored array/object, stored under unique (name, durability) key. See DataStore::m_storeEntryMap. */
  struct StoreEntry {
    StoreEntry() : isArray(false), dontWriteOut(false), objClass(nullptr), object(nullptr), ptr(nullptr), name() {};

    /** useful constructor, creates 'object', but leaves 'ptr' NULL. */
    StoreEntry(bool isArray, TClass* cl, std::string  name, bool dontWriteOut);

    /** invalidate entry for next event. (ptr will be null afterwards, memory may be reused.) */
    void invalidate();
    /** Reset stored object to defaults, or nullptr. Only useful for input modules (before GetEntry()). */
    void resetForGetEntry();
    /** Recreate object if null. Used to recover from temporary invalid states after reading empty branches. */
    void recoverFromNullObject();
    /** Reset stored object to defaults, set ptr to new object. More or less equivalent to delete object; object = new X;, but optimized. */
    void recreate();
    /** Return ptr cast to TClonesArray. If this is not an array, return null. */
    TClonesArray* getPtrAsArray() const;

    bool isArray;     /**< Flag that indicates whether the object is a TClonesArray **/
    bool dontWriteOut; /**< Flag that indicates whether the object should be written to the output by default **/
    TClass* objClass; /**< class of object. If isArray==true, class of array objects */

    /** The pointer to the actual object.
     *
     * Associated memory may exceed object durability, and is kept until the object is replaced.
     * In normal use, this should never be null. Temporary exceptions are allowed for input modules.
     */
    TObject* object;

    /** The pointer to the returned object, either equal to 'object' or null, depending on wether the object was created in the current event **/
    TObject* ptr;

    std::string name; /**< Name of the entry. Equal to the key in the map. **/
  };
}
