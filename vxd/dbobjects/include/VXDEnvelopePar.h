/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Schwenker                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <vxd/dbobjects/VXDRotationSolidPar.h>

namespace Belle2 {

  class GearDir;

  /**
  * The Class for VXD Envelope parameters
  */
  class VXDEnvelopePar: public VXDRotationSolidPar {
  public:
    //! Default constructor
    VXDEnvelopePar() {}
    //! Constructor using Gearbox
    explicit VXDEnvelopePar(const GearDir& content) : VXDRotationSolidPar(content) { }
  private:
    ClassDef(VXDEnvelopePar, 5);  /**< ClassDef, must be the last term before the closing {}*/
  };

} // end of namespace Belle2

