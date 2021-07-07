/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* KLM headers. */
#include <klm/dataobjects/bklm/BKLMElementNumbers.h>
#include <klm/dataobjects/eklm/EKLMElementNumbers.h>
#include <klm/dataobjects/KLMElementNumbers.h>

/* C++ headers. */
#include <vector>

namespace Belle2 {

  /**
   * Muid element numbers.
   */
  class MuidElementNumbers {

  public:

    /**
     * Outcome of the track extrapolation.
     */
    enum Outcome {
      c_NotReached = 0,
      c_StopInBarrel = 1,
      c_StopInForwardEndcap = 2,
      c_ExitBarrel = 3,
      c_ExitForwardEndcap = 4,
      c_StopInBackwardEndcap = 5,
      c_ExitBackwardEndcap = 6,
      c_CrossBarrelStopInForwardMin = 7,
      c_CrossBarrelStopInForwardMax = 21,
      c_CrossBarrelStopInBackwardMin = 22,
      c_CrossBarrelStopInBackwardMax = 36,
      c_CrossBarrelExitForwardMin = 37,
      c_CrossBarrelExitForwardMax = 51,
      c_CrossBarrelExitBackwardMin = 52,
      c_CrossBarrelExitBackwardMax = 66
    };

    /**
     * Labels for detectors crossed.
     */
    enum DetectorsCrossed {
      c_Both = 0,
      c_OnlyBarrel = KLMElementNumbers::c_BKLM,
      c_OnlyEndcap = KLMElementNumbers::c_EKLM,
    };

    /**
     * Hypothesis number.
     */
    enum Hypothesis {
      c_NotValid = -1,
      c_Positron = 0,
      c_Electron = 1,
      c_Deuteron = 2,
      c_AntiDeuteron = 3,
      c_Proton = 4,
      c_AntiProton = 5,
      c_PionPlus = 6,
      c_PionMinus = 7,
      c_KaonPlus = 8,
      c_KaonMinus = 9,
      c_MuonPlus = 10,
      c_MuonMinus = 11,
    };

    /**
     * Bits used for unique identifiers in getLongitudinalID() and in getTransverseID().
     */
    enum IdentifierBits {
      /** Number of bits occupied by outcome in getLongitudinalID(). */
      c_OutcomeBit = 7,
      /** Number of bits occupied by lastLayer in getLongitudinalID(). */
      c_LastLayerBit = 4,
      /** Number of bits occupied by detector in getTransverseID(). */
      c_DetectorBit = 2,
      /** Number of bits occupied by degreesOfFreedom in getTransverseID(). */
      c_DegreesOfFreedomBit = 6
    };

    /**
     * Constructor.
     */
    MuidElementNumbers();

    /**
     * Destructor.
     */
    ~MuidElementNumbers();

    /**
     * Check the track extrapolation outcome.
     * @param[in] outcome   Track extrapolation outcome.
     * @param[in] lastLayer Last layer crossed during the extrapolation.
     */
    static bool checkExtrapolationOutcome(unsigned int outcome, int lastLayer);

    /**
     * Calculate the track extrapolation outcome.
     * @param[in] isForward       Forward or backward.
     * @param[in] escaped         Escaped or not from KLM volumes.
     * @param[in] lastBarrelLayer Last barrel layer crossed during the extrapolation.
     * @param[in] lastEndcapLayer Last endcap layer crossed during the extrapolation.
     */
    static unsigned int calculateExtrapolationOutcome(bool isForward, bool escaped, int lastBarrelLayer, int lastEndcapLayer);

    /**
     * Calculate hypothesis number from PDG code.
     * @param[in] pdg PDG code.
     */
    static Hypothesis calculateHypothesisFromPDG(int pdg);

    /**
     * Get maximal barrel layer number (0-based).
     */
    static constexpr int getMaximalBarrelLayer()
    {
      return m_MaximalBarrelLayer;
    }

    /**
     * Get maximal endcap-forward layer number (0-based).
     */
    static constexpr int getMaximalEndcapForwardLayer()
    {
      return m_MaximalEndcapForwardLayer;
    }

    /**
     * Get maximal endcap-forward layer number (0-based).
     */
    static constexpr int getMaximalEndcapBackwardLayer()
    {
      return m_MaximalEndcapBackwardLayer;
    }

    /**
     * Get maximal value of the track extrapolation outcome.
     */
    static constexpr int getMaximalOutcome()
    {
      return m_MaximalOutcome;
    }

    /**
     * Get maximal value of the detector selector (for transverse scattering).
     */
    static constexpr int getMaximalDetector()
    {
      return m_MaximalDetector;
    }

    /**
     * Get maximal value of NDof/2 (for transverse scattering).
     */
    static constexpr int getMaximalHalfNdof()
    {
      return m_MaximalHalfNDof;
    }

    /**
     * Get size of array with reduced chi-squared values (for transverse scattering).
     */
    static constexpr int getSizeReducedChiSquared()
    {
      return m_SizeReducedChiSquared;
    }

    /**
     * Get maximal value of reduced chi-squared (for transverse scattering).
     */
    static constexpr double getMaximalReducedChiSquared()
    {
      return m_MaximalReducedChiSquared;
    }

    /**
     * Get a vector with all the hypothesis PDG codes used for Muid.
     * Only the codes of the selected charge are returned.
     * @param[in] charge Only codes of the selected charge are returned.
     */
    static std::vector<int> getPDGVector(int charge);

    /**
     * Get a vector with all the hypothesis PDG codes used for Muid.
     * Both positive and negative charges are returned.
     */
    static std::vector<int> getPDGVector();

    /**
     * Get the unique longitudinal identifier for given hypothesis, outcome and last layer crossed.
     * bits 0-3 for hypothesis;
     * bits 4-10 for outcome;
     * bits 11-14 for last layer crossed.
     * @param[in] hypothesis Hypothesis number.
     * @param[in] outcome    Track extrapolation outcome.
     * @param[in] lastLayer  Last layer crossed during the extrapolation..
     */
    static int getLongitudinalID(int hypothesis, int outcome, int lastLayer);

    /**
     * Get the unique transverse identifier for given hypothesis, detector and degrees of freedom.
     * bit 0-3 for hypothesis;
     * bit 4-5 for detector;
     * bit 6-11 for degrees of freedom.
     * @param[in] hypothesis       Hypothesis number.
     * @param[in] detector         Detector number.
     * @param[in] degreesOfFreedom Number of degrees of freedom.
     */
    static int getTransverseID(int hypothesis, int detector, int degreesOfFreedom);

  protected:

    /** Maximal barrel layer number (0-based). */
    static constexpr int m_MaximalBarrelLayer = BKLMElementNumbers::getMaximalLayerNumber() - 1;

    /** Maximal endcap-forward layer number (0-based). */
    static constexpr int m_MaximalEndcapForwardLayer = EKLMElementNumbers::getMaximalLayerNumber() - 1;

    /** Maximal endcap-backward layer number (0-based). */
    static constexpr int m_MaximalEndcapBackwardLayer = m_MaximalEndcapForwardLayer - 2;

    /** Maximal value of the outcome extrapolation. */
    static constexpr int m_MaximalOutcome = c_CrossBarrelExitBackwardMax;

    /** Maximal value of the detector selector (for transverse scattering). */
    static constexpr int m_MaximalDetector = c_OnlyEndcap;

    /** Maximal value of NDof/2 (for transverse scattering). */
    static constexpr int m_MaximalHalfNDof = 18;

    /** Size of array with reduced chi-squared values (for transverse scattering). */
    static constexpr int m_SizeReducedChiSquared = 100;

    /** Maximal value of reduced chi-squared (for transverse scattering). */
    static constexpr double m_MaximalReducedChiSquared = 10.;

  };

}
