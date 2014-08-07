/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Pulvermacher                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PYSTOREOBJ_H
#define PYSTOREOBJ_H

#include <string>

class TClass;
class TObject;

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
    from ROOT import Belle2
    evtmetadata = Belle2.PyStoreObj('EventMetaData')
    if not evtmetadata:
      B2ERROR("No EventMetaData found")
    else:
      event = evtmetadata.obj().getEvent()
    \endcode
  *
  * <h1>Creating objects</h1>
  * You can also create new objects in your Python basf2 module, using
  * registerInDataStore() and create(). Since you
  * cannot specify the type directly, as with template arguments to StoreObjPtr,
  * the class name is assumed to be identical to the 'name' argument given to the
  * constructor, and to reside in the Belle2 namespace.
  * Consequently, you can only create objects with their default names.
  *
  * See display/examples/displaydata.py for a concrete example.
  *
  * \sa PyStoreArray
  */
  class PyStoreObj {
  public:
    /** constructor.
    * @param name Name of the entry to be accessed
    * @param durability 0: event, 1: persistent
    */
    explicit PyStoreObj(const std::string& name, int durability = 0);

    ~PyStoreObj() { }

    /** Print list of available objects for given durability. */
    static void list(int durability = 0);

    /** Create a default object in the data store.
     *
     *  The object created will be of the type given by the 'name' argument of the ctor
     *  (assuming it's in the Belle2 namespace).
     *
     *  @param replace   Should an existing object be replaced?
     *  @return          True if the creation succeeded.
     **/
    bool create(bool replace = false);

    /** Register the object in the data store.
     *  This must be called in the initialization phase.
     *
     *  @param storeFlags ORed combination of DataStore::EStoreFlag flags. (default: c_WriteOut)
     *  @return            True if the registration succeeded.
     */
    bool registerInDataStore(int storeFlags = 0);

    /** Does this PyStoreObj contain a valid datastore object?
     *
     * Accessing the object's data is UNSAFE if this returns false.
     */
    operator bool() const { return m_storeObjPtr && *m_storeObjPtr; }

    /** Returns the attached data store object, or NULL if no valid object exists. */
    TObject* obj() const { return (m_storeObjPtr ? (*m_storeObjPtr) : NULL); }

  private:
    /** Return TClass for given name; or NULL if not found. */
    static TClass* getClass(const std::string& name);

    TObject** m_storeObjPtr; /**< Pointer to pointer to object */
    std::string m_name; /**< Name of data store entry. */
    int m_durability; /**< Durability of data store entry. */
  };
}
#endif
