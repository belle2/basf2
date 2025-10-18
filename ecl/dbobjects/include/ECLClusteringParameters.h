/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <TObject.h>
#include <array>

namespace Belle2 {

  /** DB object to parameters that store clustering behaviour in ECLCRFinder and ECLLocalMaximumFinder  */
  class ECLClusteringParameters: public TObject {


    //---------------------------------------------------
  public:

    /** Constructor. */
    ECLClusteringParameters() :
      m_LM_energyCut(),
      m_CRF_energyCut(),
      m_CRF_timeCut(),
      m_CRF_timeCutMaxEnergy()
    { };


    //---------------------------------------------------
    //..Getters for the parameters

    /** Get the energy threshold for seeds in ECLLocalMaximumFinder [GeV]*/
    double getLMEnergyCut() const {return m_LM_energyCut;}

    /** Get the three energy thresholds for ECLCRFinder [GeV]*/
    const std::array<double, 3>& getCRFEnergyCut() const {return m_CRF_energyCut;}

    /** Get the three time cuts for ECLCRFinder [ns]*/
    const std::array<double, 3>& getCRFTimeCut() const {return m_CRF_timeCut;}

    /** Get the three max energies for time cuts for ECLCRFinder [GeV]*/
    const std::array<double, 3>& getCRFTimeCutMaxEnergy() const {return m_CRF_timeCutMaxEnergy;}


    //---------------------------------------------------
    //..Setters for the parmeters

    /** Set the energy threshold in ECLLocalMaximumFinder [GeV]*/
    void setLMEnergyCut(const double LM_energy_cut) {m_LM_energyCut = LM_energy_cut;}

    /** Set the three energy thresholds in ECLCRFinder [GeV]*/
    void setCRFEnergyCut(const std::array<double, 3>& CRF_energy_cut) {m_CRF_energyCut = CRF_energy_cut;}

    /** Set the three time cuts in ECLCRFinder [ns]*/
    void setCRFTimeCut(const std::array<double, 3>& CRF_time_cut) {m_CRF_timeCut = CRF_time_cut;}

    /** Set the three time cuts in ECLCRFinder [ns]*/
    void setCRFTimeCutMaxEnergy(const std::array<double, 3>& CRF_timeCutMaxEnergy) {m_CRF_timeCutMaxEnergy = CRF_timeCutMaxEnergy;}


    //---------------------------------------------------
  private:
    double m_LM_energyCut; /**< energy cut for ECLLocalMaximumFinder seed [GeV] */
    std::array<double, 3> m_CRF_energyCut; /**< energy cuts for ECLCRFinder [GeV] */
    std::array<double, 3> m_CRF_timeCut; /**< time cuts for ECLCRFinder [ns] */
    std::array<double, 3> m_CRF_timeCutMaxEnergy; /**< time cuts are only applied below these energies [GeV] */

    ClassDef(ECLClusteringParameters, 1); /**< ClassDef */
  };
}
