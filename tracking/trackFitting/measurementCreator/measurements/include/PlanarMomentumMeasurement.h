/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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
