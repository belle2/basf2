/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jake Bennett
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef DEDXCORRECTIONMODULE_H
#define DEDXCORRECTIONMODULE_H

#include <reconstruction/modules/DedxCorrection/DedxCorrectionParameters.h>
#include <reconstruction/dataobjects/DedxConstants.h>

#include <framework/core/Module.h>
#include <framework/gearbox/Const.h>

#include <framework/datastore/StoreArray.h>
#include <framework/database/DBObjPtr.h>
#include <framework/database/DBArray.h>

#include <reconstruction/dbobjects/CDCDedxWireGain.h>
#include <reconstruction/dbobjects/CDCDedxRunGain.h>
#include <reconstruction/dbobjects/CDCDedxCosineCor.h>

#include <string>
#include <vector>
#include <map>

namespace Belle2 {
  class CDCDedxTrack;

  /** This module may be used to apply the corrections to dE/dx per the
   * calibration constants. The interface with the calibration database
   * is currently under construction and will be included when ready. For
   * now, placeholders are used so the structure may be developed.
   */

  class DedxCorrectionModule : public Module {

  public:

    /** Constructor */
    DedxCorrectionModule();
    /** Destructor */
    virtual ~DedxCorrectionModule();

    /** Initialize the Module.
     * This method is called only once before the actual event processing
     * starts. */
    virtual void initialize();

    /** This method is called for each event. All processing of the event
     * has to take place in this method. */
    virtual void event();

    /** End of the event processing. */
    virtual void terminate();

    /** Perform a run gain correction */
    void RunGainCorrection(double& dedx) const;

    /** Perform a wire gain correction */
    void WireGainCorrection(int wireID, double& dedx) const;

    /** Perform the cosine correction */
    void CosineCorrection(double costheta, double& dedx) const;

    /** Perform a hadron saturation correction.
     * (Set the peak of the truncated mean for electrons to 1) */
    void HadronCorrection(double costheta,  double& dedx) const;

    /** Perform a standard set of corrections */
    void StandardCorrection(int wireID, double costheta, double& dedx) const;

    /** Saturation correction:
     * convert the measured ionization (D) to actual ionization (I) */
    double D2I(const double cosTheta, const double D) const;

    /** Saturation correction:
     * convert the actural ionization (I) to measured ionization (D) */
    double I2D(const double cosTheta, const double I) const;

  private:

    /** Store array: CDCDedxTrack */
    StoreArray<CDCDedxTrack> m_cdcDedxTracks;

    //parameters: calibration constants
    DBObjPtr<CDCDedxWireGain> m_DBWireGains; /**< Wire gain DB object */
    DBObjPtr<CDCDedxRunGain> m_DBRunGain; /**< Run gain DB object */
    DBObjPtr<CDCDedxCosineCor> m_DBCosineCor; /**< Electron saturation correction DB object */

    /** Recalculate the dE/dx mean values after corrections */
    void calculateMeans(double* mean, double* truncatedMean, double* truncatedMeanErr, const std::vector<double>& dedx) const;

    /** lower bound for truncated mean */
    double m_removeLowest;
    /** upper bound for truncated mean */
    double m_removeHighest;

    /** saturation correction parameter: alpha */
    double  m_alpha;
    /** saturation correction parameter: gamma */
    double  m_gamma;
    /** saturation correction parameter: delta */
    double  m_delta;
    /** saturation correction parameter: power on cos(theta) */
    double  m_power;
    /** saturation correction parameter: ratio */
    double  m_ratio;

  };
} // Belle2 namespace
#endif
