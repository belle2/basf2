/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <genfit/PlanarMeasurement.h>
#include <genfit/RKTrackRep.h>
#include <tracking/trackFitting/measurementCreator/measurements/HMatrixQP.h>
#include <framework/logging/Logger.h>


namespace Belle2 {

  /**
   * Measurement class implementing a planar hit geometry (1 or 2D) with only a momentum measurement.
   */
  class PlanarMomentumMeasurement : public genfit::PlanarMeasurement {

  public:
    /** Default contructor with 1 dim. */
    PlanarMomentumMeasurement() : genfit::PlanarMeasurement(1) {}

    /** Copy from other planar measurement and resize the hit coords to one dimension. */
    explicit PlanarMomentumMeasurement(const genfit::PlanarMeasurement& parentElement) :
      genfit::PlanarMeasurement(parentElement)
    {
      rawHitCoords_.ResizeTo(1);
      rawHitCov_.ResizeTo(1, 1);
    }

    /** Clone. */
    virtual genfit::AbsMeasurement* clone() const override {return new PlanarMomentumMeasurement(*this);}

    /** Construct the QP matrix. */
    virtual const genfit::AbsHMatrix* constructHMatrix(const genfit::AbsTrackRep* rep) const override
    {
      if (dynamic_cast<const genfit::RKTrackRep*>(rep) == nullptr) {
        B2FATAL("PlanarMomentumMeasurement default implementation can only handle state vectors of type RKTrackRep!");
      }

      return new HMatrixQP();
    }
  };
}
