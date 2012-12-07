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
  * You can check the runtime type information of the returned objects by
  * using Python's built-in type() function.
  *
  * \note While it is not possible to add objects/arrays to the data store, you
  *       can modify the contents of existing ones.
  */
  class PyStoreArray {
  public:
    /** constructor.
    * @param name Name of the entry to be accessed
    * @param durability 0: event, 1: persistent
    */
    explicit PyStoreArray(const std::string& name, int durability = 0);

    ~PyStoreArray() { }

    /** Does this PyStoreArray contain a valid datastore array?
     *
     * Accessing the array's data is UNSAFE if this returns false.
     */
    operator bool() const { return m_storeArray; }

    /** returns object at index i, or null pointer if out of range */
    TObject* operator [](int i) const {return (*m_storeArray)[i];}

    /** returns number of entries for current event. */
    int getEntries() const { return m_storeArray->GetEntriesFast(); }

    /** Allow iteration using for in Python.
     *
     * That is, this should work:
     *
     * \code
        simhits = Belle2.PyStoreArray('PXDSimHits')
        for hit in simhits:
            print "Edep: ", str(hit.getEnergyDep())
       \endcode
     **/
    TIter __iter__() const {
      //will create empty iterator if NULL pointer
      return TIter(m_storeArray);
    }

  private:
    TClonesArray* m_storeArray; /**< Pointer to array */
  };
}
#endif
