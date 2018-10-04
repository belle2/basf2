/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012-2017  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/database/DBAccessorBase.h>

class TClonesArray;
class TClass;
class TObject;

namespace Belle2 {
  /** Class to access a DB Array from Python. In contrast to the C++
   * DBArray we don't have templates but python will handle the typing
   * dynamically.
   *
   * \code{.py}
     from ROOT import Belle2
     bklmmapping = Belle2.PyDBArray('BKLMElectronicMapping')
     # Alternative: simhits = Belle2.PyDBArray(Belle2.BKLMElectronicMapping.Class())
     # Or with name and type: beamparams = Belle2.PyDBArray("BKLMElectronicMapping, Belle2.BKLMElectronicMapping.Class())
     for mapping in bklmmapping:
         print(mapping.getCopperId())
     \endcode
   *
   * \warning Be aware that `PyDBArray` objects are only usable during event flow, that
   * is in the `initialize()` or `event()` calls of a module as there is no
   * Conditions data available otherwise.
   *
   * \see the object version PyDBObj as well as the interface classes to the
   * DataStore, PyStoreArray and PyStoreObj
   */
  class PyDBArray: private DBAccessorBase {
  public:
    /** Construct the array from the name of the payload */
    explicit PyDBArray(const std::string& name, bool required = true);
    /** Construct the array from the type of the payload, payload name will be
     * the class name */
    explicit PyDBArray(const TClass* objClass, bool required = true);
    /** Construct the array from the name of the payload and make sure the
     * class if compatible with objClass */
    explicit PyDBArray(const std::string& name, const TClass* objClass, bool required = true);

    using DBAccessorBase::isValid;
    using DBAccessorBase::operator bool;
    using DBAccessorBase::hasChanged;

    /** Get the number of entries */
    int getEntries() const;
    /** Get the number of entries in a pythonic way to enable len(array) */
    int __len__() const { return getEntries(); }

    /** Element access.
     * Sadly the const in the return value is lost in python so this is a
     * protected method and we add a pure python method to wrap the object in
     * something to guarantee constness in framework/scripts/basf2.py */
    const TObject* _get(int i) const;
  };
}
