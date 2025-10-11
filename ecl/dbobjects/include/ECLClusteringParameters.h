/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* ROOT headers. */
#include <TObject.h>

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

    /** Get the energy threshold for seeds in ECLLocalMaximumFinder [MeV]*/
    double getLMEnergyCut() const {return m_LM_energyCut;}

    /** Get the three energy thresholds for ECLCRFinder [MeV]*/
    const std::vector<double>& getCRFEnergyCut() const {return m_CRF_energyCut;}

    /** Get the three time cuts for ECLCRFinder [ns]*/
    const std::vector<double>& getCRFTimeCut() const {return m_CRF_timeCut;}

    /** Get the three max energies for time cuts for ECLCRFinder [MeV]*/
    const std::vector<double>& getCRFTimeCutMaxEnergy() const {return m_CRF_timeCutMaxEnergy;}


    //---------------------------------------------------
    //..Setters for the parmeters

    /** Set the energy threshold in ECLLocalMaximumFinder [MeV]*/
    void setLMEnergyCut(const double LM_energy_cut) {m_LM_energyCut = LM_energy_cut;}

    /** Set the three energy thresholds in ECLCRFinder [MeV]*/
    void setCRFEnergyCut(std::vector<double>& CRF_energy_cut) {m_CRF_energyCut = CRF_energy_cut;}

    /** Set the three time cuts in ECLCRFinder [ns]*/
    void setCRFTimeCut(std::vector<double>& CRF_time_cut) {m_CRF_timeCut = CRF_time_cut;}

    /** Set the three time cuts in ECLCRFinder [ns]*/
    void setCRFTimeCutMaxEnergy(std::vector<double>& CRF_timeCutMaxEnergy) {m_CRF_timeCutMaxEnergy = CRF_timeCutMaxEnergy;}


    //---------------------------------------------------
  private:
    double m_LM_energyCut; /**< energy cut for ECLLocalMaximumFinder seed [MeV] */
    std::vector<double>  m_CRF_energyCut; /**< energy cuts for ECLCRFinder [MeV] */
    std::vector<double>  m_CRF_timeCut; /**< time cuts for ECLCRFinder [ns] */
    std::vector<double> m_CRF_timeCutMaxEnergy; /**< time cuts are only applied below these energies [MeV] */

    ClassDef(ECLClusteringParameters, 1); /**< ClassDef */
  };
}
