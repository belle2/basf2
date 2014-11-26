
/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef BABAYAGANLOINPUTMODULE_H
#define BABAYAGANLOINPUTMODULE_H

#include <framework/core/Module.h>
#include <framework/logging/Logger.h>

#include <generators/babayaganlo/BabayagaNLO.h>

#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/MCParticleGraph.h>

#include <vector>
#include <TFile.h>
#include <TH1D.h>


namespace Belle2 {

  /**
   * The Babayaga.NLO Generator module.
   * Generates radiative Bhabha scattering and exlcusive photon pairs events using the Babayaga.NLO FORTRAN generator.
   */
  class BabayagaNLOInputModule : public Module {

  public:

    /**
     * Constructor.
     * Sets the module parameters.
     */
    BabayagaNLOInputModule();

    /** Destructor. */
    virtual ~BabayagaNLOInputModule();

    /** Initializes the module. */
    virtual void initialize();

    /** Method is called for each event. */
    virtual void event();

    /** Method is called at the end of the event processing. */
    virtual void terminate();

  protected:

    /** Module parameters */
    std::string m_finalState; /**< Final state: ee, mm or gg. */
    double m_eMinFrac; /**< Fractional energy (of cms energy) for leptons in the final state. (overrides m_eMin) */
    double m_eMin; /**< Minimum energy for leptons in the final state, in GeV. */
    double m_epsilon; /**< Soft/hard photon separator in units of CMS/2. */
    double m_maxAcollinearity; /**< maximum acollinearity angle between finale state leptons in degrees. */
    double m_cmsEnergy; /**< CMS energy. */
    int m_boostMode; /**< The mode of the boost (0 = no boost, 1 = Belle II, 2 = Belle). */
    double m_fMax; /**< Maximum differential cross section weight. */
    int m_nSearchMax; /**< Events used to search maximum of differential cross section. */
    std::vector<double> m_ScatteringAngleRange; /**< Min [0] and Max [1] value for the scattering angle [deg] of the electron/positron. */
    std::string m_fileNameExtraInfo; /**< Extra ROOT file that contains the weight distribution to check overweight bias. */

    /** Variables */
    BabayagaNLO m_generator;   /**< The BabayagaNLO generator. */
    MCParticleGraph m_mcGraph; /**< The MCParticle graph object. */

    /** Variables */
    TFile* m_fileExtraInfo;
    TH1D* m_th1dSDif;

    template <typename T>
    std::vector<T> make_vector(T const& t1, T const& t2);  /**< make_vector. */

    template <typename T>
    std::pair<T, T> vectorToPair(std::vector<T>& vec, const std::string& name = "");
  }; /**< vectorToPair. */

  template <typename T>
  inline std::vector<T> BabayagaNLOInputModule::make_vector(T const& t1, T const& t2)
  {
    std::vector<T> v;
    v.push_back(t1);
    v.push_back(t2);
    return v; /**< make_vector */
  }

  template <typename T>
  inline std::pair<T, T> BabayagaNLOInputModule::vectorToPair(std::vector<T>& vec, const std::string& name)
  {
    std::pair<T, T> p;
    if (vec.size() != 2) {
      B2ERROR("The parameter " + name + " has to have exactly 2 elements !")
    } else {
      p.first  = vec[0];
      p.second = vec[1];
    }
    return p;  /**< vectorToPair. */
  }

} // end namespace Belle2

#endif /* BABAYAGANLOINPUTMODULE_H */
