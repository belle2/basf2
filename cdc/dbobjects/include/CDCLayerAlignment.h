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
  /// CDC layers alignment constants
  class CDCLayerAlignment: public DBObjCalibrationConstMapBase {

  public:
    /// Layer shift in global X
    static const baseType layerX = 1;
    /// Layer shift in global Y
    static const baseType layerY = 2;
    /// Layer rotation in global X-Y plane (gamma)
    static const baseType layerPhi = 6;

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

