
/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef TEEGGINPUTMODULE_H
#define TEEGGINPUTMODULE_H

#include <framework/core/Module.h>
#include <framework/logging/Logger.h>

#include <generators/teegg/Teegg.h>
#include <generators/utilities/InitialParticleGeneration.h>

#include <mdst/dataobjects/MCParticleGraph.h>

#include <vector>

namespace Belle2 {

  /**
   * The TEEGG Generator module.
   */
  class TeeggInputModule : public Module {

  public:

    /**
     * Constructor.
     * Sets the module parameters.
     */
    TeeggInputModule();

    /** Destructor. */
    virtual ~TeeggInputModule();

    /** Initializes the module. */
    virtual void initialize() override;

    /** Method is called for each event. */
    virtual void event() override;

    /** Method is called at the end of the event processing. */
    virtual void terminate() override;

  protected:

    /** Module parameters */
    std::string m_sVACPOL; /**< Vacuum polarization: off, hlmnt (default) or nsk via weights. */
    double m_cmsEnergy; /**< CMS energy. */
    double m_TEVETO; /**< maximum theta of e+ in final state (in radians)*/
    double m_TEMIN;  /**< minimum angle between the e- and -z axis (egamma conf. only)*/
    double m_TGMIN;  /**< minimum angle between the gamma and -z axis*/
    double m_TGVETO; /**< maximum angle between the gamma and -z axis(etron conf. only)*/
    double m_EEMIN;  /**< minimum energy of the e- (egamma & etron configurations)*/
    double m_EGMIN;  /**< minimum energy of the gamma (egamma & gamma configurations)*/
    double m_PEGMIN; /**< minimum phi sep of e-gamma (egamma config with hard rad corr)*/
    double m_EEVETO; /**< minimum energy to veto(gamma  config with hard rad corr)*/
    double m_EGVETO; /**< minimum energy to veto(etron/gamma config with hard rad corr)*/
    double m_PHVETO; /**< minimum phi sep to veto(etron/gamma config with hard rad corr*/
    double m_CUTOFF; /**< cutoff energy for radiative corrections (in CM frame)*/
    double m_EPS; /**< param. epsilon_s (smaller val. increases sampling of k_s^pbc)*/
    double m_FRAPHI; /**< fraction of time phi_ks is generated with peak(hard rad corr)*/
    double m_EPSPHI; /**< param. epsilon_phi ('cutoff' of the phi_ks peak)*/
    double m_WGHT1M; /**< maximum weight for generation of QP0, cos(theta QP)*/
    double m_WGHTMX; /**< maximum weight for the trial events*/
    std::string m_sRADCOR; /**< specifies radiative correction (NONE SOFT or HARD)*/
    std::string m_sCONFIG; /**< specifies the event configuration (EGAMMA GAMMA or ETRON)*/
    std::string m_sMATRIX; /**< specifies which eeg matrix element (BK BKM2 TCHAN or EPA)*/
    std::string m_sMTRXGG; /**< specifies which eegg matrix element (EPADC BEEGG or MEEGG)*/
    int m_UNWGHT; /**< logical variable. If true then generate unweighted events.*/

    /** Variables */
    Teegg m_generator;   /**< The Teegg generator. */
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
  inline std::vector<T> TeeggInputModule::make_vector(T const& t1, T const& t2)
  {
    std::vector<T> v;
    v.push_back(t1);
    v.push_back(t2);
    return v; /**< make_vector */
  }

  template <typename T>
  inline std::pair<T, T> TeeggInputModule::vectorToPair(std::vector<T>& vec, const std::string& name)
  {
    std::pair<T, T> p;
    if (vec.size() != 2) {
      B2ERROR("The parameter " + name + " has to have exactly 2 elements!");
    } else {
      p.first  = vec[0];
      p.second = vec[1];
    }
    return p;  /**< vectorToPair. */
  }

} // end namespace Belle2

#endif /* TEEGGINPUTMODULE_H */
