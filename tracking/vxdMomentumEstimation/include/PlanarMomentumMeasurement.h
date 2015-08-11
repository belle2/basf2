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


namespace Belle2 {


  /** @brief Measurement class implementing a planar hit geometry (1 or 2D) with only a momentum measurement.
   *
   *  @author Christian H&ouml;ppner (Technische Universit&auml;t M&uuml;nchen, original author)
   *  @author Sebastian Neubert  (Technische Universit&auml;t M&uuml;nchen, original author)
   *  @author Johannes Rauch  (Technische Universit&auml;t M&uuml;nchen, original author)
   *  @author Nils Braun (KIT, editor)
   *
   */
  class PlanarMomentumMeasurement : public genfit::PlanarMeasurement {

  public:
    explicit PlanarMomentumMeasurement(const genfit::PlanarMeasurement& parentElement) : genfit::PlanarMeasurement(parentElement)
    {
      rawHitCoords_.ResizeTo(1);
      rawHitCov_.ResizeTo(1, 1);
    }

    virtual genfit::AbsMeasurement* clone() const {return new PlanarMomentumMeasurement(*this);}

    virtual const genfit::AbsHMatrix* constructHMatrix(const genfit::AbsTrackRep*) const;

  };

}
