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


namespace genfit {


  /** @brief Measurement class implementing a planar hit geometry (1 or 2D) with only a momentum measurement.
   *
   *  @author Christian H&ouml;ppner (Technische Universit&auml;t M&uuml;nchen, original author)
   *  @author Sebastian Neubert  (Technische Universit&auml;t M&uuml;nchen, original author)
   *  @author Johannes Rauch  (Technische Universit&auml;t M&uuml;nchen, original author)
   *  @author Nils Braun (KIT, editor)
   *
   */
  class PlanarMomentumMeasurement : public PlanarMeasurement {

  public:
    PlanarMomentumMeasurement(const PlanarMeasurement& parentElement) : PlanarMeasurement(parentElement)
    {
      rawHitCoords_.ResizeTo(1);
      rawHitCov_.ResizeTo(1, 1);
    }

    virtual AbsMeasurement* clone() const {return new PlanarMomentumMeasurement(*this);}

    virtual const AbsHMatrix* constructHMatrix(const AbsTrackRep*) const;

  public:

    ClassDef(PlanarMomentumMeasurement, 1)

  };

} /* End of namespace genfit */
/** @} */
