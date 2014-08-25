#pragma once

#include <string>

class TObject;

namespace Belle2 {
  /** Wraps a stored array/object, stored under unique (name, durability) key. See DataStore::m_storeEntryMap. */
  struct StoreEntry {
    StoreEntry() : isArray(false), dontWriteOut(false), object(0), ptr(0), name("") {};
    /** Reset stored object to defaults, or nullptr. Only useful for input modules (before GetEntry()). */
    void resetForGetEntry();
    /** Reset stored object to defaults, set ptr to new object. More or less equivalent to delete object; object = new X;, but optimized. */
    void recreate();
    bool        isArray;     /**< Flag that indicates whether the object is a TClonesArray **/
    bool        dontWriteOut; /**< Flag that indicates whether the object should be written to the output by default **/
    TObject*    object;      /**< The pointer to the actual object. Associated memory may exceed object durability, and is kept until the object is replaced.  **/
    TObject*    ptr;         /**< The pointer to the returned object, either equal to 'object' or 0, depending on wether the object was created in the current event **/
    std::string name;        /**< Name of the entry. Equal to the key in the map. **/
  };
}
