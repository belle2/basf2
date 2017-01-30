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

#include <vxd/dbobjects/VXDPolyConePar.h>
#include <TObject.h>
#include <string>
#include <vector>


namespace Belle2 {

  /**
  * The Class for VXD Heavy Metal Shield
  */

  class HeavyMetalShieldGeometryPar: public TObject {
  public:

    //! Default constructor
    HeavyMetalShieldGeometryPar() {}
    //! Get shields
    std::vector<VXDPolyConePar>& getShields(void) { return m_shields; }
    //! Get shields
    const std::vector<VXDPolyConePar>& getShields(void) const { return m_shields; }


  private:
    //! Shields
    std::vector<VXDPolyConePar> m_shields;

    ClassDef(HeavyMetalShieldGeometryPar, 5);  /**< ClassDef, must be the last term before the closing {}*/
  };

} // end of namespace Belle2

