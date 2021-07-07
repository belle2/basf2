/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <calibration/DBObjCalibrationConstMapBase.h>
#include <cdc/dataobjects/WireID.h>

namespace Belle2 {

  /// CDC alignment constants
  // N.B. The original was CDCCalibration.h by Tadeas; copied here and removed
  // calibration-related part. This is tentative for code development; to be
  // moved to an appropriate directory later.
  class CDCAlignment: public DBObjCalibrationConstMapBase {

  public:
    // Alignment of wires ------------------------------------------------
    /// Wire X position w.r.t. nominal on backward endplate
    static const baseType wireBwdX = 0;
    /// Wire Y position w.r.t. nominal on backward endplate
    static const baseType wireBwdY = 1;
    /// Wire Z position w.r.t. nominal on backward endplate
    static const baseType wireBwdZ = 2;
    /// Wire X position w.r.t. nominal on forward endplate
    static const baseType wireFwdX = 4;
    /// Wire Y position w.r.t. nominal on forward endplate
    static const baseType wireFwdY = 5;
    /// Wire Z position w.r.t. nominal on forward endplate
    static const baseType wireFwdZ = 6;

    // Tension and additional wire shape parametrization ---------------
    /// Wire tension w.r.t. nominal (=50. ?)
    static const baseType wireTension = 21;

    // Alignment of layers ------------------------------------------------
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
    CDCAlignment() : DBObjCalibrationConstMapBase() {}

    /// Getter to X shift of bwd wire
    double getBwdWireShiftX(int ICLayer,  int ILayer) const
    {
      return get(WireID(ICLayer, ILayer), wireBwdX);
    }
    /// Getter to Y shift of bwd wire
    double getBwdWireShiftY(int ICLayer,  int ILayer) const
    {
      return get(WireID(ICLayer, ILayer), wireBwdY);
    }
    /// Getter to Z shift of bwd wire
    double getBwdWireShiftZ(int ICLayer,  int ILayer) const
    {
      return get(WireID(ICLayer, ILayer), wireBwdZ);
    }

    /// Getter to X shift of fwd wire
    double getFwdWireShiftX(int ICLayer,  int ILayer) const
    {
      return get(WireID(ICLayer, ILayer), wireFwdX);
    }
    /// Getter to Y shift of fwd wire
    double getFwdWireShiftY(int ICLayer,  int ILayer) const
    {
      return get(WireID(ICLayer, ILayer), wireFwdY);
    }
    /// Getter to Z shift of fwd wire
    double getFwdWireShiftZ(int ICLayer,  int ILayer) const
    {
      return get(WireID(ICLayer, ILayer), wireFwdZ);
    }

    /// Getter to wire tension
    double getWireTension(int ICLayer,  int ILayer) const
    {
      return get(WireID(ICLayer, ILayer), wireTension);
    }

    /// Destructor
    ~CDCAlignment() {}

    // ------------- Interface to global Millepede calibration ----------------
    /// Get global unique id
    static unsigned short getGlobalUniqueID() {return 27;}
    /// Get global parameter
    double getGlobalParam(unsigned short element, unsigned short param) const
    {
      return get(element, param);
    }
    /// Set global parameter
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

    ClassDef(CDCAlignment, 2); /**< Storage for CDC alignment constants (mainly now for Millepede) */

  };

} // end namespace Belle2

