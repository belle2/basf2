/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012, 2015 - Belle II Collaboration                       *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guofu Cao, Martin Heck, Tobias Schlüter                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ALIGNABLEPXDRECOHIT_H
#define ALIGNABLEPXDRECOHIT_H


#include <pxd/reconstruction/PXDRecoHit.h>
#include <pxd/dataobjects/PXDCluster.h>
#include <pxd/dataobjects/PXDTrueHit.h>

#include <genfit/AbsMeasurement.h>
#include <genfit/MeasurementOnPlane.h>
#include <genfit/TrackCandHit.h>
#include <genfit/HMatrixU.h>

#include <memory>

#include <genfit/ICalibrationParametersDerivatives.h>

namespace Belle2 {
  /** This class is used to transfer PXD information to the track fit. */
  class AlignablePXDRecoHit : public PXDRecoHit, public genfit::ICalibrationParametersDerivatives  {
    friend class PXDRecoHit;
  public:
    /** Inherit constructors */
    using PXDRecoHit::PXDRecoHit;

    /** Destructor. */
    virtual ~AlignablePXDRecoHit() {}

    genfit::AbsMeasurement* clone() const
    {
      return new AlignablePXDRecoHit(*this);
    }

    /**
     * @brief Labels for global derivatives
     *
     * @return Vector of int labels, same size as derivatives matrix #columns
     */
    virtual std::vector< int > labels();

    /**
     * @brief Get alignment derivatives
     *
     * @param sop Track state on sensor plane
     * @return TMatrixD
     */
    virtual TMatrixD derivatives(const genfit::StateOnPlane* sop);

  private:

    ClassDef(AlignablePXDRecoHit, 2); /**< PXD RecoHit extended for alignment/calibration */
  };
}
#endif