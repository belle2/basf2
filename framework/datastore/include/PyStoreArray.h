#ifndef PYSTOREARRAY_H
#define PYSTOREARRAY_H

#include <framework/datastore/StoreArray.h>

#include <string>

namespace Belle2 {
  /** a (simplified) python wrapper for StoreArray.
  *
  *  to convert the returned TObjects into useful types, use something like
  *   myobject = Belle2.MyObject(tobject)
  *
  *  to check runtime type information, use type(tobject)
  */
  class PyStoreArray : public TObject {
  public:
    /** constructor.
    * @param name Name of the branch to be read/saved
    * @param durability 0: event, 1: run, 2: persistent
    */
    PyStoreArray(const std::string& name = "", int durability = 0):
        TObject(),
        m_storearray(name, DataStore::EDurability(durability)) { }

    ~PyStoreArray() { }

    /** returns object at index i, or null pointer if out of range */
    TObject* operator [](int i) const {return m_storearray[i];}

    /** returns number of entries for current event. */
    int getEntries() const { return m_storearray.getEntries(); }

    /** returns the branch name */
    std::string getName() const { return m_storearray.getName(); }

    /** Return  durability with which the object is saved in the DataStore.
    * @return 0: event, 1: run, 2: persistent
    */
    int getDurability() const { return m_storearray.getDurability(); }

  private:
    StoreArray<TObject> m_storearray; /**< wrapped object */

    ClassDef(PyStoreArray, 0)
  };
}
#endif
