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

#include <TCollection.h> //for TIter

class TClonesArray;
class TClass;
class TObject;

namespace Belle2 {
  /** Class to access a DB Array from Python. In contrast to the C++
   * DBArray<T> we don't have templates but python will handle the typing
   * dynamically.
   *
   * \code{.py}
     from ROOT import Belle2
     bklmmapping = Belle2.PyDBArray('BKLMElectronicMapping')
     # Alternative: simhits = Belle2.PyDBArray(Belle2.BKLMElectronicMapping.Class())
     for mapping in bklmmapping:
         print(mapping.getCopperId())
     \endcode
   *
   * \warning Be aware that PyDBArrays are only usable during event flow, that
   * is in the initialize() or event() calls of a module.
   */
  class PyDBArray: private DBAccessorBase {
  public:
    /** Construct the array from the name of the payload */
    explicit PyDBArray(const std::string& name);
    /** Construct the array from the type of the payload, payload name will be
     * the class name */
    explicit PyDBArray(const TClass* objClass);
    /** Construct the array from the name of the payload and make sure the
     * class if compatible with objClass */
    explicit PyDBArray(const std::string& name, const TClass* objClass);

    using DBAccessorBase::isValid;
    using DBAccessorBase::operator bool;
    using DBAccessorBase::hasChanged;

    /** Element access */
    const TObject* operator [](int i) const;
    /** Get the number of entries */
    int getEntries() const;
    /** Get the number of entries in a pythonic way to enable len(array) */
    int __len__() const { return getEntries(); }
    /** Make the array iterable in python */
    TIter __iter__() const;
  private:
    /** Pointer to the actual array */
    TClonesArray** m_array;
  };
}
