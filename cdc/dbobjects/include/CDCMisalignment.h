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

  /// CDC misalignment constants
  // N.B. The original was CDCCalibration.h by Tadeas; copied here and removed
  // calibration-related part. This is tentative for code development; to be
  // moved to an appropriate directory later.
  //TODO: define using a template tech. (?) since this and CDCAlignment are the same.
  class CDCMisalignment: public DBObjCalibrationConstMapBase {

  public:
    // Define some not used wire/cell ids for other purposes ------------
    /// Last bit for wire (actually over 100 unused - but useful for us)
    static const baseType WireNone = 511;
    /// Last bit for continous layer (unused)
    static const baseType ICLayerNone = 63;
    /// layer = (ICLayer, 400)
    static const baseType Wire4Layers = 400;
    /// SuperLayer = (SuperLayer, 0, 405)
    static const baseType Wire4SuperLayers = 405;
    /// Outer conical end part = (63, 405)
    static const baseType Wire4EndPartOuter = 410;
    /// Middle conical end part = (63, 411)
    static const baseType Wire4EndPartMiddle = 411;
    /// Inner conical end part = (63, 412)
    static const baseType Wire4EndPartInner = 412;

    // Misalignment of wires ------------------------------------------------
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

    // Layer misalignment -------------------------------------------------
    /// Layer X position w.r.t. geometry
    static const baseType layerX = 0;
    /// Layer Y position w.r.t. geometry
    static const baseType layerY = 1;

    // Tension and additional wire shape parametrization ---------------
    /// Wire tension w.r.t. nominal (=50. ?)
    static const baseType wireTension = 21;

    // Endplates -------------------------------------------------------
    /// Backward endplate rotation w.r.t. nominal (geometry)
    static const baseType endPlateBwdPhi = 46;
    /// Forward enplate rotation taking endPlateBwdPhi as new nominal
    static const baseType endPlateFwdDphi = 47;

    /// Constructor
    CDCMisalignment() : DBObjCalibrationConstMapBase() {}

    /// Getter to X shift of bwd wire
    double getBwdWireShiftX(int ICLayer,  int ILayer)
    {
      return get(WireID(ICLayer, ILayer), wireBwdX);
    }
    /// Getter to Y shift of bwd wire
    double getBwdWireShiftY(int ICLayer,  int ILayer)
    {
      return get(WireID(ICLayer, ILayer), wireBwdY);
    }
    /// Getter to Z shift of bwd wire
    double getBwdWireShiftZ(int ICLayer,  int ILayer)
    {
      return get(WireID(ICLayer, ILayer), wireBwdZ);
    }

    /// Getter to X shift of fwd wire
    double getFwdWireShiftX(int ICLayer,  int ILayer)
    {
      return get(WireID(ICLayer, ILayer), wireFwdX);
    }
    /// Getter to Y shift of fwd wire
    double getFwdWireShiftY(int ICLayer,  int ILayer)
    {
      return get(WireID(ICLayer, ILayer), wireFwdY);
    }
    /// Getter to Z shift of fwd wire
    double getFwdWireShiftZ(int ICLayer,  int ILayer)
    {
      return get(WireID(ICLayer, ILayer), wireFwdZ);
    }

    /// Getter to wire tension
    double getWireTension(int ICLayer,  int ILayer)
    {
      return get(WireID(ICLayer, ILayer), wireTension);
    }

    /// Direct getter to X shift of layer
    double getLayerShiftX(int ICLayer) {return get(WireID(ICLayer, Wire4Layers), layerX);}
    /// Direct getter to Y shift of layer
    double getLayerShiftY(int ICLayer) {return get(WireID(ICLayer, Wire4Layers), layerY);}
    /// Destructor
    ~CDCMisalignment() {}

  private:

    ClassDef(CDCMisalignment, 1); /**< Storage for CDC misalignment constants (mainly now for Millepede) */

  };

} // end namespace Belle2

