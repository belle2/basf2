/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012, 2015 - Belle II Collaboration                       *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guofu Cao, Martin Heck, Tobias Schlüter                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ALIGNABLESVDRECOHIT_H
#define ALIGNABLESVDRECOHIT_H


#include <svd/reconstruction/SVDRecoHit.h>
#include <svd/dataobjects/SVDCluster.h>
#include <svd/dataobjects/SVDTrueHit.h>

#include <genfit/AbsMeasurement.h>
#include <genfit/MeasurementOnPlane.h>
#include <genfit/TrackCandHit.h>
#include <genfit/HMatrixU.h>

#include <genfit/ICalibrationParametersDerivatives.h>

#include <memory>


namespace Belle2 {
  /** This class is used to transfer SVD information to the track fit. */
  class AlignableSVDRecoHit : public SVDRecoHit, public genfit::ICalibrationParametersDerivatives  {
    friend class SVDRecoHit;
  public:
    /** Inherit constructors */
    using SVDRecoHit::SVDRecoHit;

    /** Destructor. */
    virtual ~AlignableSVDRecoHit() {}

    genfit::AbsMeasurement* clone() const
    {
      return new AlignableSVDRecoHit(*this);
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

    /** ROOT Macro.*/
    ClassDef(AlignableSVDRecoHit, 1);
  };
}
#endif