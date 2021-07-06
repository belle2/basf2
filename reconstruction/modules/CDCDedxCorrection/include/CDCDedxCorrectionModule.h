/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>

#include <framework/datastore/StoreArray.h>
#include <framework/database/DBObjPtr.h>

#include <reconstruction/dbobjects/CDCDedxScaleFactor.h>
#include <reconstruction/dbobjects/CDCDedxMomentumCor.h>
#include <reconstruction/dbobjects/CDCDedxWireGain.h>
#include <reconstruction/dbobjects/CDCDedxRunGain.h>
#include <reconstruction/dbobjects/CDCDedxCosineCor.h>
#include <reconstruction/dbobjects/CDCDedx2DCell.h>
#include <reconstruction/dbobjects/CDCDedx1DCell.h>
#include <reconstruction/dbobjects/CDCDedxHadronCor.h>
#include <reconstruction/dbobjects/CDCDedxADCNonLinearity.h> //new in rel5
#include <reconstruction/dbobjects/CDCDedxCosineEdge.h> //new in rel5

#include <vector>

namespace Belle2 {
  class CDCDedxTrack;

  /** This module may be used to apply the corrections to dE/dx per the
   * calibration constants. The interface with the calibration database
   * is currently under construction and will be included when ready. For
   * now, placeholders are used so the structure may be developed.
   */

  class CDCDedxCorrectionModule : public Module {

  public:

    /** Constructor */
    CDCDedxCorrectionModule();
    /** Destructor */
    virtual ~CDCDedxCorrectionModule();

    /** Initialize the Module.
     * This method is called only once before the actual event processing
     * starts. */
    virtual void initialize() override;

    /** This method is called for each event. All processing of the event
     * has to take place in this method. */
    virtual void event() override;

    /** End of the event processing. */
    virtual void terminate() override;

    /** Perform a run gain correction */
    void RunGainCorrection(double& dedx) const;

    /** Perform a wire gain correction */
    void WireGainCorrection(int wireID, double& dedx) const;

    /** Perform a 2D correction */
    void TwoDCorrection(int layer, double doca, double enta, double& dedx) const;

    /** Perform a wire gain correction */
    void OneDCorrection(int layer, double enta, double& dedx) const;

    /** Perform the cosine correction */
    void CosineCorrection(double costheta, double& dedx) const;

    /** Perform the cosine edge correction */
    void CosineEdgeCorrection(double costh, double& dedx) const;

    /** Perform a hadron saturation correction.
     * (Set the peak of the truncated mean for electrons to 1) */
    void HadronCorrection(double costheta,  double& dedx) const;

    /** Perform a standard set of corrections */
    void StandardCorrection(int adc, int layer, int wireID, double doca, double enta, double length, double costheta,
                            double& dedx) const;

    /** Get the standard set of corrections */
    double GetCorrection(int& adc, int layer, int wireID, double doca, double enta, double costheta) const;

    /** Saturation correction:
     * convert the measured ionization (D) to actual ionization (I) */
    double D2I(const double cosTheta, const double D) const;

    /** Saturation correction:
     * convert the actural ionization (I) to measured ionization (D) */
    double I2D(const double cosTheta, const double I) const;

  private:

    bool m_relative; /**< boolean to apply relative or absolute correction */

    bool m_momCor; /**< boolean to apply momentum correction */
    bool m_useDBMomCor; /**< boolean to apply momentum correction from DB */
    bool m_scaleCor; /**< boolean to apply scale factor */
    bool m_cosineCor; /**< boolean to apply cosine correction */
    bool m_wireGain; /**< boolean to apply wire gains */
    bool m_runGain; /**< boolean to apply run gains */
    bool m_twoDCell; /**< boolean to apply 2D correction */
    bool m_oneDCell; /**< boolean to apply 1D correction */
    bool m_cosineEdge; /**< boolean to apply cosine edge */
    bool m_nonlADC; /**< boolean to apply non linear ADC */

    StoreArray<CDCDedxTrack> m_cdcDedxTracks; /**< Store array: CDCDedxTrack */

    //parameters: calibration constants
    DBObjPtr<CDCDedxScaleFactor> m_DBScaleFactor; /**< Scale factor to make electrons ~1 */
    DBObjPtr<CDCDedxMomentumCor> m_DBMomentumCor; /**< Momentum correction DB object */
    DBObjPtr<CDCDedxWireGain> m_DBWireGains; /**< Wire gain DB object */
    DBObjPtr<CDCDedxRunGain> m_DBRunGain; /**< Run gain DB object */
    DBObjPtr<CDCDedxCosineCor> m_DBCosineCor; /**< Electron saturation correction DB object */
    DBObjPtr<CDCDedx2DCell> m_DB2DCell; /**< 2D correction DB object */
    DBObjPtr<CDCDedx1DCell> m_DB1DCell; /**< 1D correction DB object */
    DBObjPtr<CDCDedxHadronCor> m_DBHadronCor; /**< hadron saturation parameters */
    DBObjPtr<CDCDedxADCNonLinearity> m_DBNonlADC; /**< hadron saturation non linearity */
    DBObjPtr<CDCDedxCosineEdge> m_DBCosEdgeCor; /**< cosine edge calibration */

    std::vector<double> m_hadronpars; /**< hadron saturation parameters */

    /** Recalculate the dE/dx mean values after corrections */
    void calculateMeans(double* mean, double* truncatedMean, double* truncatedMeanErr, const std::vector<double>& dedx) const;

    /** lower bound for truncated mean */
    double m_removeLowest;
    /** upper bound for truncated mean */
    double m_removeHighest;

  };
} // Belle2 namespace
