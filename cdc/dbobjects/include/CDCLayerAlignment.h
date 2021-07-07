/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <calibration/DBObjCalibrationConstMapBase.h>

namespace Belle2 {
  /// CDC layers alignment constants
  class CDCLayerAlignment: public DBObjCalibrationConstMapBase {

  public:
    //Numbering taken from Claus ... no need to change, good idea :-)
    //(1: X(bwd), 2: Y(bwd), 6: Phi(bwd), 11: X(fwd-bwd), 12: Y(fwd-bwd), 16: Phi(fwd-bwd)

    /// Layer shift in global X at backward endplate
    static const baseType layerX = 1;
    /// Layer shift in global Y at backward endplate
    static const baseType layerY = 2;
    /// Layer rotation in global X-Y plane (gamma) at backward endplate
    static const baseType layerPhi = 6;

    /// Layer shift in global X  dX = foward - backward endplate
    static const baseType layerDx = 11;
    /// Layer shift in global Y  dY = foward - backward endplate
    static const baseType layerDy = 12;
    /// Layer rotation in global X-Y plane (gamma)  dPhi = foward - backward endplate
    static const baseType layerDPhi = 16;

    /// Constructor
    CDCLayerAlignment() : DBObjCalibrationConstMapBase() {}

    /// Destructor
    ~CDCLayerAlignment() {}

    // ------------- Interface to global Millepede calibration ----------------
    //TODO: enable the wire alignment
    /// Get global unique id
    static unsigned short getGlobalUniqueID() {return 21;}
    /// Get global parameter FIXME does nothing because CDC is not ready
    double getGlobalParam(unsigned short element, unsigned short param)
    {
      return get(element, param);
    }
    /// Set global parameter FIXME does nothing because CDC is not ready
    void setGlobalParam(double value, unsigned short element, unsigned short param)
    {
      set(element, param, value);
    }
    /// list stored global parameters TODO FIXME CDC not ready
    std::vector<std::pair<unsigned short, unsigned short>> listGlobalParams()
    {
      return {};
    }
    // ------------------------------------------------------------------------

  private:
    ClassDef(CDCLayerAlignment, 1); /**< Storage for CDC alignment constants (mainly now for Millepede) */

  };

} // end namespace Belle2

