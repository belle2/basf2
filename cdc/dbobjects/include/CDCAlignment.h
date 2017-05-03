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

    /// Constructor
    CDCAlignment() : DBObjCalibrationConstMapBase() {}

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

    /// Destructor
    ~CDCAlignment() {}

    // ------------- Interface to global Millepede calibration ----------------
    //TODO: enable the wire alignment
    /// Get global unique id
    static unsigned short getGlobalUniqueID() {return 27;}
    /// Get global parameter FIXME does nothing because CDC is not ready
    double getGlobalParam(unsigned short, unsigned short)
    {
      //return get(element, param);
      return 0.;
    }
    /// Set global parameter FIXME does nothing because CDC is not ready
    void setGlobalParam(double, unsigned short, unsigned short)
    {
      //set(element, param, value);
    }
    /// list stored global parameters TODO FIXME CDC not ready
    std::vector<std::pair<unsigned short, unsigned short>> listGlobalParams()
    {
      return {};
    }
    // ------------------------------------------------------------------------

  private:

    ClassDef(CDCAlignment, 1); /**< Storage for CDC alignment constants (mainly now for Millepede) */

  };

} // end namespace Belle2

