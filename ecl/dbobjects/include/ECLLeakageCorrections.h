/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * DB object to store leakage corrections, including dependence on nCrys     *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christopher Hearty (hearty@physics.ubc.ca)               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#include <TObject.h>
#include <framework/logging/Logger.h>
#include <TH2F.h>

namespace Belle2 {

  /** DB object to store leakage corrections, including nCrys dependence  */
  class ECLLeakageCorrections: public TObject {


//---------------------------------------------------
  public:

    /** Constructor. */
    ECLLeakageCorrections() :
      m_logEnergiesFwd(),
      m_logEnergiesBrl(),
      m_logEnergiesBwd(),
      m_thetaBoundaries(),
      m_thetaCor(),
      m_phiCor(),
      m_nCrysCor()
    { };

    //---------------------------------------------------
    //..Getters for the various calibration components

    /** Get vector of log(energies) used to evaluate the leakage corrections in the forward endcap*/
    const std::vector<float>& getlogEnergiesFwd() const {return m_logEnergiesFwd;}

    /** Get vector of log(energies) used in the barrel. Same number as forward, but different values*/
    const std::vector<float>& getlogEnergiesBrl() const {return m_logEnergiesBrl;}

    /** Get vector of log(energies) used in the backward endcap. Same number as forward, but different values*/
    const std::vector<float>& getlogEnergiesBwd() const {return m_logEnergiesBwd;}

    /** Get vector containing theta (deg) separating theta rings */
    const std::vector<float>& getThetaBoundaries() const {return m_thetaBoundaries;}

    /** Get 2D histogram of theta-dependent constants  (one column per thetaID & energy) */
    const TH2F getThetaCor() const {return m_thetaCor;}

    /** Get 2D histogram of phi-dependent constants  (one column per thetaID & energy in barrel, 2 in endcaps) */
    const TH2F getPhiCor() const {return m_phiCor;}

    /** Get 2D histogram of nCrys dependent constants (one column per thetaID & energy) */
    const TH2F getnCrysCor() const {return m_nCrysCor;}


    //---------------------------------------------------
    //..Setters for the various calibration components

    /** Set the vector of energies used to evaluate the leakage corrections in the forward endcap*/
    void setlogEnergiesFwd(const std::vector<float>& logEnergiesFwd) {m_logEnergiesFwd = logEnergiesFwd;}

    /** Set the vector of energies used to evaluate the leakage corrections in the barrel*/
    void setlogEnergiesBrl(const std::vector<float>& logEnergiesBrl) {m_logEnergiesBrl = logEnergiesBrl;}

    /** Set the vector of energies used to evaluate the leakage corrections in the backward endcap*/
    void setlogEnergiesBwd(const std::vector<float>& logEnergiesBwd) {m_logEnergiesBwd = logEnergiesBwd;}

    /** Set the 70-element vector of theta lab values bounding the 69 theta rings */
    void setThetaBoundaries(const std::vector<float>& thetaBoundaries)
    {
      if (thetaBoundaries.size() != 70) {B2FATAL("ECLLeakageCorrections: wrong size vector uploaded for thetaBoundaries, " << thetaBoundaries.size() << " instead of 70");}        m_thetaBoundaries
        = thetaBoundaries;
    }

    /** Set the 2D histogram containing the theta corrections for each thetaID and energy*/
    void setThetaCor(const TH2F& thetaCor) {m_thetaCor = thetaCor;}

    /** Set the 2D histogram containing the phi corrections for each thetaID and energy*/
    void setPhiCor(const TH2F& phiCor) {m_phiCor = phiCor;}

    /** Set the 2D histogram containing the nCrys corrections for each thetaID and energy*/
    void setnCrysCor(const TH2F& nCrysCor) {m_nCrysCor = nCrysCor;}


//---------------------------------------------------
  private:
    std::vector<float> m_logEnergiesFwd; /**< log(energies) used to find constants in the forward endcap (GeV) */
    std::vector<float> m_logEnergiesBrl; /**< log(energies) used to find constants in the barrel (GeV) */
    std::vector<float> m_logEnergiesBwd; /**< log(energies) used to find constants in the backward endcap (GeV) */
    std::vector<float> m_thetaBoundaries; /**< 70 values of theta that bound the 69 theta rings (rad) */
    TH2F m_thetaCor; /**< 2D histogram containing theta-dependent correction for each thetaID & energy */
    TH2F m_phiCor; /**< 2D histogram containing phi-dependent correction for each thetaID & energy */
    TH2F m_nCrysCor; /**< 2D histogram containing nCrys-dependent correction for each thetaID & energy */

    ClassDef(ECLLeakageCorrections, 1); /**< ClassDef */
  };
}
