/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef BHWIDEINPUTMODULE_H
#define BHWIDEINPUTMODULE_H

#include <framework/core/Module.h>
#include <framework/logging/Logger.h>

#include <generators/bhwide/BHWide.h>

#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/MCParticleGraph.h>

#include <vector>

namespace Belle2 {

  /**
   * The BHWide Generator module.
   * Generates radiative Bhabha scattering events using the BHWide FORTRAN generator.
   */
  class BHWideInputModule : public Module {

  public:

    /**
     * Constructor.
     * Sets the module parameters.
     */
    BHWideInputModule();

    /** Destructor. */
    virtual ~BHWideInputModule();

    /** Initializes the module. */
    virtual void initialize();

    /** Method is called for each event. */
    virtual void event();

    /** Method is called at the end of the event processing. */
    virtual void terminate();

  protected:

    /** Module parameters */
    int m_boostMode; /**< The mode of the boost (0 = no boost, 1 = Belle II, 2 = Belle). */
    std::vector<double> m_ScatteringAngleRangePositron; /**< Min [0] and Max [1] value for the scattering angle [deg] of the positron. */
    std::vector<double> m_ScatteringAngleRangeElectron; /**< Min [0] and Max [1] value for the scattering angle [deg] of the electron. */

    /** Variables */
    BHWide m_generator;        /**< The BHWide generator. */
    MCParticleGraph m_mcGraph; /**< The MCParticle graph object. */

    template <typename T>
    std::vector<T> make_vector(T const& t1, T const& t2);  /**< make_vector. */

    template <typename T>
    std::pair<T, T> vectorToPair(std::vector<T>& vec, const std::string& name = "");
  }; /**< vectorToPair. */

  template <typename T>
  inline std::vector<T> BHWideInputModule::make_vector(T const& t1, T const& t2)
  {
    std::vector<T> v;
    v.push_back(t1);
    v.push_back(t2);
    return v; /**< make_vector */
  }

  template <typename T>
  inline std::pair<T, T> BHWideInputModule::vectorToPair(std::vector<T>& vec, const std::string& name)
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

#endif /* BHWIDEINPUTMODULE_H */
