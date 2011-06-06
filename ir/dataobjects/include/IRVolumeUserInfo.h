/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Hiroyuki Nakayama                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef IRVOLUMEUSERINFO_H_
#define IRVOLUMEUSERINFO_H_

#include <geometry/dataobjects/VolumeUserInfoBase.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

#include <TObject.h>

namespace Belle2 {

  /** Default step size.
   *  This is now the only place where Geant4 step length limit can be
   *  defined. Set to something positive to make step limit active.
   *  Will be fixed in future.
   */
  const double stepLengthInIR = -1.0 * Unit::um;
  //const double stepLengthInIR = 10.0 * Unit::mm;

  /**
    * IRVolumeUserInfo - Additional information for a IR sensitive volume.
    *
    */
  class IRVolumeUserInfo : public VolumeUserInfoBase {

  public:

    /** Empty constructor, required for ROOT IO.*/
    IRVolumeUserInfo(): VolumeUserInfoBase() {
      m_stepSize = stepLengthInIR;
      B2DEBUG(10, "stepLengthInIR is set to " << stepLengthInIR << "cm!")
    }

    /** Destructor */
    ~IRVolumeUserInfo() {
      /* Does nothing. */
    }

    /** Assignment operator.*/
    IRVolumeUserInfo& operator=(const IRVolumeUserInfo& other);

  private:

    ClassDef(IRVolumeUserInfo, 1)

  };

} // end namespace Belle2

#endif /* IRVOLUMEUSERINFO_H_ */
