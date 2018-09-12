/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <cdc/geometry/CDCGeometryPar.h>
#include <TRandom.h>
#include <cdc/geometry/CDCGeoControlPar.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Structure to expose some drift time and length functions from the CDCGeometryPar to Python.
    struct DriftTimeUtil {

      /**
       *  Get the realistic drift velocity.
       *  @param driftTime  Drift time (ns).
       *  @param iCLayer    Continuous layer id
       *  @param lr         Left/Right
       *  @param alpha      Incident angle (in rphi plane) w.r.t. the cell (rad).
       *  @param theta      Incident angle (polar angle) (rad).
       */
      static double getDriftV(double driftTime,
                              unsigned short iCLayer,
                              unsigned short lr,
                              double alpha = 0.,
                              double theta = 0.5 * M_PI)
      {
        return CDC::CDCGeometryPar::Instance().getDriftV(driftTime, iCLayer, lr, alpha, theta);
      }

      /**
       *  Return the drift length to the sense wire.
       *  @param driftTime  Drift time (ns).
       *  @param iCLayer    Continuous layer id.
       *  @param lr         Left/Right boolean.
       *  @param alpha      Incident angle (in rphi plane) w.r.t. the cell (rad).
       *  @param theta      Incident angle (polar angle) (rad).
       */
      static double getDriftLength(double driftTime,
                                   unsigned short iCLayer,
                                   unsigned short lr,
                                   double alpha = 0.,
                                   double theta = 0.5 * M_PI)
      {
        return CDC::CDCGeometryPar::Instance().getDriftLength(driftTime, iCLayer, lr, alpha, theta);
      }

      /**
       *  Return the drift time to the sense wire.
       *  @param dist     Drift length (cm).
       *  @param iCLayer  Continuous layer id
       *  @param lr       Left/Right boolean
       *  @param alpha    Incident angle (in rphi plane) w.r.t. the cell (rad).
       *  @param theta    Incident angle (polar angle) (rad).
       */
      static double getDriftTime(double dist,
                                 unsigned short iCLayer,
                                 unsigned short lr,
                                 double alpha,
                                 double theta)
      {
        return CDC::CDCGeometryPar::Instance().getDriftTime(dist, iCLayer, lr, alpha, theta);
      }

      /**
       *  Getter for the in wire propagation time
       *  @param wireID   wire id
       *  @param z        start location of the signal propagation on the wire
       */
      static double getPropTime(const WireID& wireID, double z)
      {
        unsigned short iCLayer = wireID.getICLayer();
        const CDC::CDCGeometryPar& geometryPar = CDC::CDCGeometryPar::Instance();
        const CDC::CDCGeoControlPar& controlPar = CDC::CDCGeoControlPar::getInstance();
        TVector3 backwardWirePos =
          geometryPar.wireBackwardPosition(wireID, CDC::CDCGeometryPar::c_Aligned);
        TVector3 forwardWirePos =
          geometryPar.wireForwardPosition(wireID, CDC::CDCGeometryPar::c_Aligned);
        double zDistance = z - backwardWirePos.Z();

        // Actually are very small correction
        double stereoFactor =
          (forwardWirePos - backwardWirePos).Mag() / (forwardWirePos.Z() - backwardWirePos.Z());

        double distance = zDistance * stereoFactor;
        if (controlPar.getSenseWireZposMode() == 1) {
          distance += geometryPar.getBwdDeltaZ(iCLayer);
        }
        return distance * geometryPar.getPropSpeedInv(iCLayer);
      }

      /**
       *  Returns time-walk
       *  @param wireID   wire id
       *  @param adcCount ADC count
       *  @return         time-walk (in ns)
       */
      static double getTimeWalk(const WireID& wireID, unsigned short adcCount)
      {
        return CDC::CDCGeometryPar::Instance().getTimeWalk(wireID, adcCount);
      }

      /**
       *  Returns the time measured at the readout board
       *  @param wireID   wire id
       *  @param tdcCount TDC count
       *  @return         measured time (in ns)
       */
      static double getMeasuredTime(const WireID& wireID, unsigned short tdcCount, bool smear)
      {
        double smearing = 0;
        if (smear) {
          smearing = gRandom->Rndm() - 0.5;
        }
        double channelT0 = static_cast<double>(CDC::CDCGeometryPar::Instance().getT0(wireID));
        double measuredTime =
          channelT0 - (tdcCount + smearing) * CDC::CDCGeometryPar::Instance().getTdcBinWidth();
        if (measuredTime > 2000) {
          B2INFO("channelT0 " << channelT0);
          B2INFO("measuredTime " << measuredTime);
        }
        return measuredTime;
      }

    };
  }
}
