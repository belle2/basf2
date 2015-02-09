/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Pulvermacher                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PYSTOREARRAY_H
#define PYSTOREARRAY_H

#include "TClonesArray.h"

#include <string>

namespace Belle2 {
  /** a (simplified) python wrapper for StoreArray.
  *
  * Compared to StoreArray, PyStoreArray returns only TObject pointers (since
  * it doesn't use templates). Thanks to Python, you can still access all
  * public functions and data members of the actual type.
  *
  * Relations can also be accessed using functions like RelationsObject::getRelationsTo()
  * as long as the data objects are derived from RelationsObject. A full example
  * for accessing simhits and some related objects can be found in framework/examples/cdcplotmodule.py
  *
  * Example:
  *
  * \code{.py}
     from ROOT import Belle2
     simhits = Belle2.PyStoreArray('PXDSimHits')
     for hit in simhits:
         part = hit.getRelatedFrom('MCParticles')
         print "Edep: ", str(hit.getEnergyDep())
         print "Particle: ", str(part.getPDG())
    \endcode
  *
  * You can check the runtime type information of the returned objects by
  * using Python's built-in type() function.
  *
  * \sa PyStoreObj
  */
  class PyStoreArray {
  public:
    /** constructor.
    * @param name Name of the entry to be accessed
    * @param durability 0: event, 1: persistent
    */
    explicit PyStoreArray(const std::string& name, int durability = 0);

    ~PyStoreArray() { }

    /** Return list of available arrays for given durability. */
    static std::vector<std::string> list(int durability = 0);

    /** Print list of available arrays for given durability. */
    static void printList(int durability = 0);

    /** Does this PyStoreArray contain a valid datastore array?  */
    operator bool() const { return m_storeArray; }

    /** returns object at index i, or null pointer if out of range (+error) */
    TObject* operator [](int i) const;

    /** returns number of entries for current event. */
    int getEntries() const { return m_storeArray ? (m_storeArray->GetEntriesFast()) : 0;}

    /** Support for len(). */
    int __len__() const { return getEntries(); }

    /** Allow iteration using for in Python.
     **/
    TIter __iter__() const {
      //will create empty iterator if NULL pointer
      return TIter(m_storeArray);
    }

    /** Construct a new object of the array's type at the end of the array.
     *
     * @returns the created object, to be modified by the user
     */
    TObject* appendNew();

    /** Raw access to the underlying TClonesArray.
     *
     *  \warning TClonesArray is dangerously easy to misuse. Whatever you do will probably
     *           be slow, leak memory, and murder your pets.
     *           In most cases, you'll want to use functions like operator[],
     *           getEntries() or appendNew() instead.
     */
    TClonesArray* getPtr() { return m_storeArray; }

  private:
    TClonesArray* m_storeArray; /**< Pointer to array */
  };
}
#endif
