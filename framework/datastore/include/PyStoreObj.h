#ifndef PYSTOREOBJ_H
#define PYSTOREOBJ_H

#include <framework/datastore/StoreObjPtr.h>

#include <TObject.h>

#include <string>

namespace Belle2 {
  /** a (simplified) python wrapper for StoreObjPtr.
  *
  * Compared to StoreObjPtr, PyStoreObj returns only TObjects (since
  * it doesn't use templates). Thanks to Python, you can still access all
  * public functions and data members of the actual type.
  *
  * You can check the runtime type information of the returned objects by
  * using Python's built-in type() function.
  *
  * <h1>Usage example</h1>
  * Inside a Python module's event() function, you can access data store
  * objects like this:
  * \code{.py}
    evtmetadata = Belle2.PyStoreObj('EventMetaData')
    if not evtmetadata:
      B2ERROR("No EventMetaData found")
    else:
      event = evtmetadata.obj().getEvent()
    \endcode
  *
  * \note While it is not possible to add objects/arrays to the data store, you
  *       can modify the contents of existing ones.
  */
  class PyStoreObj : public TObject {
  public:
    /** constructor.
    * @param name Name of the entry to be accessed
    * @param durability 0: event, 1: persistent
    */
    explicit PyStoreObj(const std::string& name, int durability = 0);

    ~PyStoreObj() { }

    /** Does this PyStoreObj contain a valid datastore object?
     *
     * Accessing the object's data is UNSAFE if this returns false.
     */
    operator bool() const { return m_storeObjPtr && *m_storeObjPtr; }

    /** Returns the attached data store object. */
    TObject* obj() const { return *m_storeObjPtr; }

  private:
    TObject** m_storeObjPtr; /**< Pointer to pointer to object */

    ClassDef(PyStoreObj, 0)
  };
}
#endif
