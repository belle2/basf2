/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Frank Meier                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <TF1.h>
#include <TObject.h>

namespace Belle2 {

  /**
   * Class to hold the parameterizations of the RMS
   * for the difference in polar and azimuthal angle between tracks and ECL clusters.
   */
  class ECLTrackClusterMatchingParameterizations: public TObject {

  public:

    /** Default constructor */
    ECLTrackClusterMatchingParameterizations() {};

    /** Constructor */
    ECLTrackClusterMatchingParameterizations(const TF1& RMSParameterizationThetaFWDCROSS,
                                             const TF1& RMSParameterizationThetaFWDDL,
                                             const TF1& RMSParameterizationThetaFWDNEAR,
                                             const TF1& RMSParameterizationThetaBRLCROSS,
                                             const TF1& RMSParameterizationThetaBRLDL,
                                             const TF1& RMSParameterizationThetaBRLNEAR,
                                             const TF1& RMSParameterizationThetaBWDCROSS,
                                             const TF1& RMSParameterizationThetaBWDDL,
                                             const TF1& RMSParameterizationThetaBWDNEAR,
                                             const TF1& RMSParameterizationPhiFWDCROSS,
                                             const TF1& RMSParameterizationPhiFWDDL,
                                             const TF1& RMSParameterizationPhiFWDNEAR,
                                             const TF1& RMSParameterizationPhiBRLCROSS,
                                             const TF1& RMSParameterizationPhiBRLDL,
                                             const TF1& RMSParameterizationPhiBRLNEAR,
                                             const TF1& RMSParameterizationPhiBWDCROSS,
                                             const TF1& RMSParameterizationPhiBWDDL,
                                             const TF1& RMSParameterizationPhiBWDNEAR):
      m_RMSParameterizationThetaFWDCROSS(RMSParameterizationThetaFWDCROSS),
      m_RMSParameterizationThetaFWDDL(RMSParameterizationThetaFWDDL),
      m_RMSParameterizationThetaFWDNEAR(RMSParameterizationThetaFWDNEAR),
      m_RMSParameterizationThetaBRLCROSS(RMSParameterizationThetaBRLCROSS),
      m_RMSParameterizationThetaBRLDL(RMSParameterizationThetaBRLDL),
      m_RMSParameterizationThetaBRLNEAR(RMSParameterizationThetaBRLNEAR),
      m_RMSParameterizationThetaBWDCROSS(RMSParameterizationThetaBWDCROSS),
      m_RMSParameterizationThetaBWDDL(RMSParameterizationThetaBWDDL),
      m_RMSParameterizationThetaBWDNEAR(RMSParameterizationThetaBWDNEAR),
      m_RMSParameterizationPhiFWDCROSS(RMSParameterizationPhiFWDCROSS),
      m_RMSParameterizationPhiFWDDL(RMSParameterizationPhiFWDDL),
      m_RMSParameterizationPhiFWDNEAR(RMSParameterizationPhiFWDNEAR),
      m_RMSParameterizationPhiBRLCROSS(RMSParameterizationPhiBRLCROSS),
      m_RMSParameterizationPhiBRLDL(RMSParameterizationPhiBRLDL),
      m_RMSParameterizationPhiBRLNEAR(RMSParameterizationPhiBRLNEAR),
      m_RMSParameterizationPhiBWDCROSS(RMSParameterizationPhiBWDCROSS),
      m_RMSParameterizationPhiBWDDL(RMSParameterizationPhiBWDDL),
      m_RMSParameterizationPhiBWDNEAR(RMSParameterizationPhiBWDNEAR)
    {
    }

    /** Destructor. */
    ~ECLTrackClusterMatchingParameterizations() {};

    /** Get theta RMS parameterization for FWD CROSS */
    TF1 getThetaFWDCROSSRMSParameterization() const {return m_RMSParameterizationThetaFWDCROSS;};

    /** Get theta RMS parameterization for FWD DL */
    TF1 getThetaFWDDLRMSParameterization() const {return m_RMSParameterizationThetaFWDDL;};

    /** Get theta RMS parameterization for FWD NEAR */
    TF1 getThetaFWDNEARRMSParameterization() const {return m_RMSParameterizationThetaFWDNEAR;};

    /** Get theta RMS parameterization for BRL CROSS */
    TF1 getThetaBRLCROSSRMSParameterization() const {return m_RMSParameterizationThetaBRLCROSS;};

    /** Get theta RMS parameterization for BRL DL */
    TF1 getThetaBRLDLRMSParameterization() const {return m_RMSParameterizationThetaBRLDL;};

    /** Get theta RMS parameterization for BRL NEAR */
    TF1 getThetaBRLNEARRMSParameterization() const {return m_RMSParameterizationThetaBRLNEAR;};

    /** Get theta RMS parameterization for BWD CROSS */
    TF1 getThetaBWDCROSSRMSParameterization() const {return m_RMSParameterizationThetaBWDCROSS;};

    /** Get theta RMS parameterization for BWD DL */
    TF1 getThetaBWDDLRMSParameterization() const {return m_RMSParameterizationThetaBWDDL;};

    /** Get theta RMS parameterization for BWD NEAR */
    TF1 getThetaBWDNEARRMSParameterization() const {return m_RMSParameterizationThetaBWDNEAR;};

    /** Get phi RMS parameterization for FWD CROSS */
    TF1 getPhiFWDCROSSRMSParameterization() const {return m_RMSParameterizationPhiFWDCROSS;};

    /** Get phi RMS parameterization for FWD DL */
    TF1 getPhiFWDDLRMSParameterization() const {return m_RMSParameterizationPhiFWDDL;};

    /** Get phi RMS parameterization for FWD NEAR */
    TF1 getPhiFWDNEARRMSParameterization() const {return m_RMSParameterizationPhiFWDNEAR;};

    /** Get phi RMS parameterization for BRL CROSS */
    TF1 getPhiBRLCROSSRMSParameterization() const {return m_RMSParameterizationPhiBRLCROSS;};

    /** Get phi RMS parameterization for BRL DL */
    TF1 getPhiBRLDLRMSParameterization() const {return m_RMSParameterizationPhiBRLDL;};

    /** Get phi RMS parameterization for BRL NEAR */
    TF1 getPhiBRLNEARRMSParameterization() const {return m_RMSParameterizationPhiBRLNEAR;};

    /** Get phi RMS parameterization for BWD CROSS */
    TF1 getPhiBWDCROSSRMSParameterization() const {return m_RMSParameterizationPhiBWDCROSS;};

    /** Get phi RMS parameterization for BWD DL */
    TF1 getPhiBWDDLRMSParameterization() const {return m_RMSParameterizationPhiBWDDL;};

    /** Get phi RMS parameterization for BWD NEAR */
    TF1 getPhiBWDNEARRMSParameterization() const {return m_RMSParameterizationPhiBWDNEAR;};

  private:

    const TF1 m_RMSParameterizationThetaFWDCROSS; /**< Parameterization of theta RMS for FWD CROSS */
    const TF1 m_RMSParameterizationThetaFWDDL; /**< Parameterization of theta RMS for FWD DL */
    const TF1 m_RMSParameterizationThetaFWDNEAR; /**< Parameterization of theta RMS for FWD NEAR */
    const TF1 m_RMSParameterizationThetaBRLCROSS; /**< Parameterization of theta RMS for BRL CROSS */
    const TF1 m_RMSParameterizationThetaBRLDL; /**< Parameterization of theta RMS for BRL DL */
    const TF1 m_RMSParameterizationThetaBRLNEAR; /**< Parameterization of theta RMS for BRL NEAR */
    const TF1 m_RMSParameterizationThetaBWDCROSS; /**< Parameterization of theta RMS for BWD CROSS */
    const TF1 m_RMSParameterizationThetaBWDDL; /**< Parameterization of theta RMS for BWD DL */
    const TF1 m_RMSParameterizationThetaBWDNEAR; /**< Parameterization of theta RMS for BWD NEAR */
    const TF1 m_RMSParameterizationPhiFWDCROSS; /**< Parameterization of phi RMS for FWD CROSS */
    const TF1 m_RMSParameterizationPhiFWDDL; /**< Parameterization of phi RMS for FWD DL */
    const TF1 m_RMSParameterizationPhiFWDNEAR; /**< Parameterization of phi RMS for FWD NEAR */
    const TF1 m_RMSParameterizationPhiBRLCROSS; /**< Parameterization of phi RMS for BRL CROSS */
    const TF1 m_RMSParameterizationPhiBRLDL; /**< Parameterization of phi RMS for BRL DL */
    const TF1 m_RMSParameterizationPhiBRLNEAR; /**< Parameterization of phi RMS for BRL NEAR */
    const TF1 m_RMSParameterizationPhiBWDCROSS; /**< Parameterization of phi RMS for BWD CROSS */
    const TF1 m_RMSParameterizationPhiBWDDL; /**< Parameterization of phi RMS for BWD DL */
    const TF1 m_RMSParameterizationPhiBWDNEAR; /**< Parameterization of phi RMS for BWD NEAR */

    ClassDef(ECLTrackClusterMatchingParameterizations, 1); /**< ClassDef */
  };
} // namespace Belle2
