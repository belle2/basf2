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
  /** Class to access a DBObjPtr from Python. In contrast to the C++
   * DBObjPtr<T> we don't have templates but python will handle the typing
   * dynamically.
   *
   * \code{.py}
     from ROOT import Belle2
     beamparams = Belle2.PyDBObj('BeamParameters')
     # Alternative: simhits = Belle2.PyDBObj(Belle2.BeamParameters.Class())
     print(beamparams.obj().getMass(), beamparams.obj().getEnergy())
     \endcode
   *
   * \warning Be aware that PyDBObj are only usable during event flow, that
   * is in the initialize() or event() calls of a module.
   */
  class PyDBObj: private DBAccessorBase {
  public:
    /** Construct the object from the name of the payload */
    explicit PyDBObj(const std::string& name);
    /** Construct the object from the type of the payload, payload name will be
     * the class name */
    explicit PyDBObj(const TClass* objClass);
    /** Construct the object from the name of the payload and make sure the
     * class if compatible with objClass */
    explicit PyDBObj(const std::string& name, const TClass* objClass);

    using DBAccessorBase::isValid;
    using DBAccessorBase::operator bool;
    using DBAccessorBase::hasChanged;

    /** Return the object */
    const TObject* obj() { return isValid() ? *m_object : nullptr; }
  private:
    /** Pointer to the actual object */
    TObject** m_object;
  };
}
