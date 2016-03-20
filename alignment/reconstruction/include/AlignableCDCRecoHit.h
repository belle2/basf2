/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012, 2015 - Belle II Collaboration                       *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Tadeas Bilka                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ALIGNABLECDCRECOHIT_H
#define ALIGNABLECDCRECOHIT_H


#include <cdc/dataobjects/CDCRecoHit.h>
#include <cdc/dataobjects/CDCHit.h>

#include <genfit/AbsMeasurement.h>
#include <genfit/MeasurementOnPlane.h>
#include <genfit/TrackCandHit.h>
#include <genfit/HMatrixU.h>

#include <genfit/ICalibrationParametersDerivatives.h>

#include <memory>


namespace Belle2 {
  /// This class is used to transfer CDC information to the track fit and Millepede.
  class AlignableCDCRecoHit : public CDCRecoHit, public genfit::ICalibrationParametersDerivatives  {

  public:
    /** Inherit constructors */
    using CDCRecoHit::CDCRecoHit;

    /// Destructor
    ~AlignableCDCRecoHit() {}

    /// Creating a copy of this hit.
    AlignableCDCRecoHit* clone() const
    {
      return new AlignableCDCRecoHit(*this);
    }

    /**
     * @brief Labels for global derivatives
     *
     * @return Vector of int labels, same size as derivatives matrix #columns
     */
    virtual std::vector< int > labels();

    /**
     * @brief Derivatives for (global) alignment/calibration parameters
     *
     * @param sop State on virtual plane to calculate derivatives
     * @return TMatrixD of global derivatives, #columns=#params, #row=2 (or measurement dimension if > 2)
     */
    virtual TMatrixD derivatives(const genfit::StateOnPlane* sop);

    /**
     * @brief Derivatives for (local) fit parameters
     *
     * @param sop State on virtual plane to calculate derivatives
     * @return TMatrixD of local derivatives, #columns=#params, #row=2 (or measurement dimension if > 2)
     */
    virtual TMatrixD localDerivatives(const genfit::StateOnPlane* sop);

    /**
     * @brief Labels for (local) alignment/calibration parameters
     *
     * @return Vector of ints, one per each parameter
     */
    virtual std::vector< int > localLabels();

  private:
    /** ROOT Macro.*/
    ClassDef(AlignableCDCRecoHit, 1);
  };
}
#endif