/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef BHWIDEINPUTMODULE_H
#define BHWIDEINPUTMODULE_H

#include <framework/core/Module.h>
#include <framework/logging/Logger.h>

#include <generators/bhwide/BHWide.h>

#include <mdst/dataobjects/MCParticleGraph.h>

#include <generators/utilities/InitialParticleGeneration.h>

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
    virtual void initialize() override;

    /** Method is called for each event. */
    virtual void event() override;

    /** Method is called at the end of the event processing. */
    virtual void terminate() override;

  protected:

    double getBeamEnergyCM(double e1, double e2, double angle); /**< Get CM beam energy from LER, HER and crossing angle. */

    /** Module parameters */
    std::vector<double>
    m_ScatteringAngleRangePositron; /**< Min [0] and Max [1] value for the scattering angle [deg] of the positron. */
    std::vector<double>
    m_ScatteringAngleRangeElectron; /**< Min [0] and Max [1] value for the scattering angle [deg] of the electron. */

    bool m_weakCorrections;  /**< EW corrections on/off*/
    std::string m_vacPolString; /**< Vacuum polarization as string*/
    BHWide::PhotonVacPolarization m_vacPol; /**< Vacuum polarization */
    double m_eMin; /**< Minimum energy for leptons in the final state, in GeV. */
    double m_maxAcollinearity; /**< maximum acollinearity angle between finale state leptons in degrees. */
    double m_wtMax; /**< Maximum weight (wtmax). */

    /** Variables */
    BHWide m_generator;        /**< The BHWide generator. */
    MCParticleGraph m_mcGraph; /**< The MCParticle graph object. */

    template <typename T>
    std::vector<T> make_vector(T const& t1, T const& t2);  /**< make_vector. */

    template <typename T>
    std::pair<T, T> vectorToPair(std::vector<T>& vec, const std::string& name = "");

  private:

    /** Method is called to initialize the generator. */
    void initializeGenerator();

    bool m_initialized{false}; /**< True if generator has been initialized. */
    DBObjPtr<BeamParameters> m_beamParams; /**< BeamParameter. */

    InitialParticleGeneration m_initial; /**< initial particle used by BeamParameter class */

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
      B2ERROR("The parameter " + name + " has to have exactly 2 elements !");
    } else {
      p.first  = vec[0];
      p.second = vec[1];
    }
    return p;  /**< vectorToPair. */
  }

} // end namespace Belle2

#endif /* BHWIDEINPUTMODULE_H */
