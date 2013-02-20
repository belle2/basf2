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
  class PyStoreObj {
  public:
    /** constructor.
    * @param name Name of the entry to be accessed
    * @param durability 0: event, 1: persistent
    */
    explicit PyStoreObj(const std::string& name, int durability = 0);

    ~PyStoreObj() { }

    /** Create a default object in the data store.
     *
     *  The object created will be of the type given by the 'name' argument of the ctor
     *  (assuming it's in the Belle2 namespace).
     *
     *  @param replace   Should an existing object be replaced?
     *  @return          True if the creation succeeded.
     **/
    bool create(bool replace = false);

    /** Register the object in the data store and include it in the output by default.
     *  This must be called in the initialization phase.
     *
     *  @param errorIfExisting  Flag whether an error will be reported if the object was already registered.
     *  @return            True if the registration succeeded.
     */
    bool registerAsPersistent(bool errorIfExisting = false);

    /** Register the object in the data store and do not include it in the output by default.
     *  This must be called in the initialization phase.
     *
     *  @param errorIfExisting  Flag whether an error will be reported if the object was already registered.
     *  @return            True if the registration succeeded.
     */
    bool registerAsTransient(bool errorIfExisting = false);

    /** Does this PyStoreObj contain a valid datastore object?
     *
     * Accessing the object's data is UNSAFE if this returns false.
     */
    operator bool() const { return m_storeObjPtr && *m_storeObjPtr; }

    /** Returns the attached data store object. */
    TObject* obj() const { return *m_storeObjPtr; }

  private:
    /** Return TClass for given name; or NULL if not found. */
    TClass* getClass(const std::string& name);

    TObject** m_storeObjPtr; /**< Pointer to pointer to object */
    std::string m_name; /**< Name of data store entry. */
    int m_durability; /**< Durability of data store entry. */
  };
}
#endif
