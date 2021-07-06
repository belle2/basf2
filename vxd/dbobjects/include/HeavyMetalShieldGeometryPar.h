/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <vxd/dbobjects/VXDPolyConePar.h>
#include <TObject.h>
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

