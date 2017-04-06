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
      //set(value, element, param);
    }
    /// list stored global parameters TODO FIXME CDC not ready
    std::vector<std::pair<unsigned short, unsigned short>> listGlobalParams()
    {
      return {};
    }

    //TODO this the temporary place to be able to change wire positions according to layer alignment - not yet done
    void readFromResult(std::vector<std::tuple<unsigned short, unsigned short, unsigned short, double>>& result)
    {
      double dxLayer[56] = {
        0.,    0.,  0.,   0.,   0.,   0.,  0., 0.,
        0.,    0.,  0.,   0.,   0.,   0.,
        0.,    0.,  0.,   0.,   0.,   0.,
        0.,    0.,  0.,   0.,   0.,   0.,
        0.,    0.,  0.,   0.,   0.,   0.,
        0.,    0.,  0.,   0.,   0.,   0.,
        0.,    0.,  0.,   0.,   0.,   0.,
        0.,    0.,  0.,   0.,   0.,   0.,
        0.,    0.,  0.,   0.,   0.,   0
      };
      double dyLayer[56] = {
        0.,    0.,  0.,   0.,   0.,   0.,  0., 0.,
        0.,    0.,  0.,   0.,   0.,   0.,
        0.,    0.,  0.,   0.,   0.,   0.,
        0.,    0.,  0.,   0.,   0.,   0.,
        0.,    0.,  0.,   0.,   0.,   0.,
        0.,    0.,  0.,   0.,   0.,   0.,
        0.,    0.,  0.,   0.,   0.,   0.,
        0.,    0.,  0.,   0.,   0.,   0.,
        0.,    0.,  0.,   0.,   0.,   0.
      };
      double dPhiLayer[56] = {
        0.,    0.,  0.,   0.,   0.,   0.,  0., 0.,
        0.,    0.,  0.,   0.,   0.,   0.,
        0.,    0.,  0.,   0.,   0.,   0.,
        0.,    0.,  0.,   0.,   0.,   0.,
        0.,    0.,  0.,   0.,   0.,   0.,
        0.,    0.,  0.,   0.,   0.,   0.,
        0.,    0.,  0.,   0.,   0.,   0.,
        0.,    0.,  0.,   0.,   0.,   0.,
        0.,    0.,  0.,   0.,   0.,   0.
      };

      for (auto entry : result) {
        unsigned short uid = std::get<0>(entry);
        if (uid != getGlobalUniqueID())
          continue;

        unsigned short layer = std::get<1>(entry);
        unsigned short param = std::get<2>(entry);
        double correction = std::get<3>(entry);

        if (param == 1)
          dxLayer[layer] = correction;
        if (param == 2)
          dyLayer[layer] = correction;
        if (param == 6)
          dPhiLayer[layer] = correction;

      }

      double R[56] = {
        168.0, 178.0, 188.0, 198.0, 208.0, 218.0, 228.0, 238.0,
        257.0, 275.2, 293.4, 311.6, 329.8, 348.0,
        365.2, 383.4, 401.6, 419.8, 438.0, 455.7,
        476.9, 494.6, 512.8, 531.0, 549.2, 566.9,
        584.1, 601.8, 620.0, 638.2, 656.4, 674.1,
        695.3, 713.0, 731.2, 749.4, 767.6, 785.3,
        802.5, 820.2, 838.4, 856.6, 874.8, 892.5,
        913.7, 931.4, 949.6, 967.8, 986.0, 1003.7,
        1020.9, 1038.6, 1056.8, 1075.0, 1093.2, 1111.4
      };

      int nWires[56] = {
        160, 160, 160, 160, 160, 160, 160, 160,
        160, 160, 160, 160, 160, 160,
        192, 192, 192, 192, 192, 192,
        224, 224, 224, 224, 224, 224,
        256, 256, 256, 256, 256, 256,
        288, 288, 288, 288, 288, 288,
        320, 320, 320, 320, 320, 320,
        352, 352, 352, 352, 352, 352,
        384, 384, 384, 384, 384, 384
      };

      int shiftHold[56] = {
        0, 0, 0, 0, 0, 0, 0, 0,
        18, 18, 18, 18, 18, 18,
        0, 0, 0, 0, 0, 0,
        -18, -18, -18, -18, -18, -18,
        0, 0, 0, 0, 0, 0,
        19, 19, 19, 19, 19, 19,
        0, 0, 0, 0, 0, 0,
        -20, -20, -20, -20, -20, -20,
        0, 0, 0, 0, 0, 0
      };
      double offset[56] = {
        0.5, 0., 0.5, 0., 0.5, 0.,  0.5, 0.,
        0., 0.5, 0., 0.5, 0.,  0.5,
        0., 0.5, 0., 0.5, 0.,  0.5,
        0., 0.5, 0., 0.5, 0.,  0.5,
        0., 0.5, 0., 0.5, 0.,  0.5,
        0., 0.5, 0., 0.5, 0.,  0.5,
        0., 0.5, 0., 0.5, 0.,  0.5,
        0., 0.5, 0., 0.5, 0.,  0.5,
        0., 0.5, 0., 0.5, 0.,  0.5
      };

      double rotF[56] = {
        0.,    0.,  0.,   0.,   0.,   0., 0., 0.,
        0.,    0.,  0.,   0.,   0.,   0.,
        0.,    0.,  0.,   0.,   0.,   0.,
        0.,    0.,  0.,   0.,   0.,   0.,
        0.,    0.,  0.,   0.,   0.,   0.,
        0.,    0.,  0.,   0.,   0.,   0.,
        0.,    0.,  0.,   0.,   0.,   0.,
        0.,    0.,  0.,   0.,   0.,   0.,
        0.,    0.,  0.,   0.,   0.,   0
      };

      double rotB[56] = {
        0.,    0.,  0.,   0.,   0.,   0.,  0., 0.,
        0.,    0.,  0.,   0.,   0.,   0.,
        0.,    0.,  0.,   0.,   0.,   0.,
        0.,    0.,  0.,   0.,   0.,   0.,
        0.,    0.,  0.,   0.,   0.,   0.,
        0.,    0.,  0.,   0.,   0.,   0.,
        0.,    0.,  0.,   0.,   0.,   0.,
        0.,    0.,  0.,   0.,   0.,   0.,
        0.,    0.,  0.,   0.,   0.,   0
      };

      double misBackward[56][385][3];
      double misForward[56][385][3];

      double PhiF, PhiB;
      for (int l = 0; l < 56; ++l) {
        double dPhi = 2 * 3.14159265358979323846 / nWires[l];
        for (int w = 0; w < nWires[l]; ++w) {

          PhiF = dPhi * (w + offset[l] + 0.5 * shiftHold[l]);
          misForward[l][w][0] = dxLayer[l] + R[l] * 0.1 * (cos(PhiF + rotF[l] + dPhiLayer[l]) - cos(PhiF));
          misForward[l][w][1] = dyLayer[l] + R[l] * 0.1 * (sin(PhiF + rotF[l] + dPhiLayer[l]) - sin(PhiF));
          misForward[l][w][2] = 0. ;

          PhiB = dPhi * (w + offset[l]);
          misBackward[l][w][0] = dxLayer[l] + R[l] * 0.1 * (cos(PhiB + rotB[l] + dPhiLayer[l]) - cos(PhiB));
          misBackward[l][w][1] = dyLayer[l] + R[l] * 0.1 * (sin(PhiB + rotB[l] + dPhiLayer[l]) - sin(PhiB));
          misBackward[l][w][2] = 0.;
        }
      }
      for (int l = 0; l < 56; ++l) {
        for (int w = 0; w < nWires[l]; ++w) {

          auto wire = WireID(l, w);
          add(wire, wireFwdX, misForward[l][w][0]);
          add(wire, wireFwdY, misForward[l][w][1]);
          add(wire, wireFwdZ, misForward[l][w][2]);

          add(wire, wireBwdX, misBackward[l][w][0]);
          add(wire, wireBwdY, misBackward[l][w][1]);
          add(wire, wireBwdZ, misBackward[l][w][2]);
        }
      }
    }

    // ------------------------------------------------------------------------

  private:

    ClassDef(CDCAlignment, 1); /**< Storage for CDC alignment constants (mainly now for Millepede) */

  };

} // end namespace Belle2

