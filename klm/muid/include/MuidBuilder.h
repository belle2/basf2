/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

/* KLM headers. */
#include <klm/dbobjects/MuidParameters.h>
#include <klm/muid/MuidElementNumbers.h>

/* Belle 2 headers. */
#include <framework/database/DBObjPtr.h>

namespace Belle2 {

  class KLMMuidLikelihood;

  //! Provides muid parameters (from Database)
  class MuidBuilder {

  public:

    //! Constructor with arguments
    MuidBuilder(int);

    //! Destructor
    ~MuidBuilder();

    //! Get the PDF for a particular hypothesis
    double getPDF(const KLMMuidLikelihood*) const;

  private:

    //! Hidden constructor
    MuidBuilder();

    //! Hidden copy constructor
    MuidBuilder(MuidBuilder&);

    //! Hidden copy assignment
    MuidBuilder& operator=(const MuidBuilder&);

    //! Get probability density functions for this particleID hypothesis from Dababase
    void fillPDFs(int);

    //! Construct spline interpolation coefficients (first, second, third derivatives)
    void spline(int, double, double*, double*, double*, double*);

    //! Get the per-layer PDF for a particular hypothesis
    double getPDFLayer(const KLMMuidLikelihood*) const;

    //! Get the transverse-coordinate PDF for a particular hypothesis
    double getPDFRchisq(const KLMMuidLikelihood*) const;

    //! Per-layer (longitudinal) probability density function
    double m_LayerPDF[MuidElementNumbers::getMaximalOutcome() + 1][MuidElementNumbers::getMaximalBarrelLayer() +
        1][MuidElementNumbers::getMaximalBarrelLayer() + MuidElementNumbers::getMaximalEndcapForwardLayer() + 2];

    //! Reduced chi-squared (transverse) probability density function (analytical): threshold
    double m_ReducedChiSquaredThreshold[MuidElementNumbers::getMaximalDetector() + 1][MuidElementNumbers::getMaximalHalfNdof() + 1];

    //! Reduced chi-squared (transverse) probability density function (analytical): horizontal scale ~ 1
    double m_ReducedChiSquaredScaleX[MuidElementNumbers::getMaximalDetector() + 1][MuidElementNumbers::getMaximalHalfNdof() + 1];

    //! Reduced chi-squared (transverse) probability density function (analytical): vertical scale
    double m_ReducedChiSquaredScaleY[MuidElementNumbers::getMaximalDetector() + 1][MuidElementNumbers::getMaximalHalfNdof() + 1];

    //! Reduced chi-squared (transverse) probability density function (overflows in last bin)
    double m_ReducedChiSquaredPDF[MuidElementNumbers::getMaximalDetector() + 1][MuidElementNumbers::getMaximalHalfNdof() +
        1][MuidElementNumbers::getSizeReducedChiSquared()];

    //! First derivative of reduced chi-squared PDF (for spline interpolation)
    double m_ReducedChiSquaredD1[MuidElementNumbers::getMaximalDetector() + 1][MuidElementNumbers::getMaximalHalfNdof() +
        1][MuidElementNumbers::getSizeReducedChiSquared()];

    //! Second derivative of reduced chi-squared PDF (for spline interpolation)
    double m_ReducedChiSquaredD2[MuidElementNumbers::getMaximalDetector() + 1][MuidElementNumbers::getMaximalHalfNdof() +
        1][MuidElementNumbers::getSizeReducedChiSquared()];

    //! Third derivative of reduced chi-squared PDF (for spline interpolation)
    double m_ReducedChiSquaredD3[MuidElementNumbers::getMaximalDetector() + 1][MuidElementNumbers::getMaximalHalfNdof() +
        1][MuidElementNumbers::getSizeReducedChiSquared()];

    //! Reduced chi-squared (transverse) probability density function's bin size
    double m_ReducedChiSquaredDx;

    //! Muid parameters from database
    DBObjPtr<MuidParameters> m_muidParameters;

  };

} // end of namespace Belle2
