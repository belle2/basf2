/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Tadeas Bilka (original)                                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <calibration/DBObjCalibrationConstMapBase.h>

#include <fstream>
#include <map>
#include <TObject.h>

#include <cdc/dataobjects/WireID.h>
#include <framework/logging/Logger.h>
#include <framework/utilities/FileSystem.h>

namespace Belle2 {

  class FileSystem;
  /// CDC alignment constants
  // N.B. The original was CDCCalibration.h by Tadeas; copied here and removed
  // calibration-related part. This is tentative for code development; to be
  // moved to an appropriate directory later.
  class CDCAlignment: public DBObjCalibrationConstMapBase {

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

    // Alignment of wires ------------------------------------------------
    /// Wire X position w.r.t. nominal on backward endplate
    static const baseType wireBwdX = 0;
    /// Wire Y position w.r.t. nominal on backward endplate
    static const baseType wireBwdY = 1;
    /// Wire Z position w.r.t. nominal on backward endplate
    static const baseType wireBwdZ = 2;
    /// Wire X position on forward enplate taking wireBwdX as new nominal
    static const baseType wireFwdDx = 4;
    /// Wire Y position on forward enplate taking wireBwdY as new nominal
    static const baseType wireFwdDy = 5;
    /// Wire Z position on forward enplate taking wireBwdZ as new nominal
    static const baseType wireFwdDz = 6;

    // Layer alignment -------------------------------------------------
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
    CDCAlignment() : DBObjCalibrationConstMapBase() {}

    /// Direct getter to X shift of layer
    double getLayerShiftX(int ICLayer) {return get(WireID(ICLayer, Wire4Layers), layerX);}
    /// Direct getter to Y shift of layer
    double getLayerShiftY(int ICLayer) {return get(WireID(ICLayer, Wire4Layers), layerY);}
    /// Destructor
    ~CDCAlignment() {}

  private:

    ClassDef(CDCAlignment, 1); /**< Storage for CDC alignment constants (mainly now for Millepede) */

  };

} // end namespace Belle2

