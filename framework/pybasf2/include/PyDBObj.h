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

class TClass;
class TObject;

namespace Belle2 {
  /** Class to access a DBObjPtr from Python. In contrast to the C++
   * DBObjPtr we don't have templates but python will handle the typing
   * dynamically.
   *
   * \code{.py}
     from ROOT import Belle2
     beamparams = Belle2.PyDBObj('BeamParameters')
     # alternative: beamparams = Belle2.PyDBObj(Belle2.BeamParameters.Class())
     # Or with name and type: beamparams = Belle2.PyDBObj("BeamParameters", Belle2.BeamParameters.Class())
     print(beamparams.getMass(), beamparams.getEnergy())
     # alternative: beamparams.obj().getMass()
     \endcode
   *
   * Most of the time you can just use the `PyDBObj` instance like an constant
   * instance of the class it represents, i.e. call all the members. The only
   * exceptions are if the class has members which are also present in
   * `PyDBObj` (for example isValid() or hasChanged()). In this case you need
   * to use the obj() member to obtain a reference to the real object first as
   * shown in the example.
   *
   * \warning Be aware that `PyDBObj` objects are only usable during event
   * flow, that is in the `initialize()` or `event()` calls of a module as
   * there is no Conditions data available otherwise.
   *
   * \see the array version PyDBArray as well as the interface classes to the
   * DataStore, PyStoreArray and PyStoreObj
   */
  class PyDBObj: private DBAccessorBase {
  public:
    /** Construct the object from the name of the payload */
    explicit PyDBObj(const std::string& name, bool required = true);
    /** Construct the object from the type of the payload, payload name will be
     * the class name */
    explicit PyDBObj(const TClass* objClass, bool required = true);
    /** Construct the object from the name of the payload and make sure the
     * class if compatible with objClass */
    explicit PyDBObj(const std::string& name, const TClass* objClass, bool required = true);

    using DBAccessorBase::isValid;
    using DBAccessorBase::operator bool;
    using DBAccessorBase::hasChanged;

    /** Return the object.
     * Sadly the const in the return value is lost in python so this is a
     * protected method and we add a pure python method to wrap the object in
     * something to guarantee constness in framework/scripts/basf2.py
     */
    const TObject* _obj() const { return isValid() ? getObject() : nullptr; }
  };
}
