
/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef BABAYAGANLOINPUTMODULE_H
#define BABAYAGANLOINPUTMODULE_H

#include <framework/core/Module.h>
#include <framework/logging/Logger.h>

#include <generators/babayaganlo/BabayagaNLO.h>
#include <generators/utilities/InitialParticleGeneration.h>

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
    BabayagaNLOInputModule();//: Module(), m_initial(BeamParameters::c_smearALL) {}

    /** Destructor. */
    virtual ~BabayagaNLOInputModule();

    /** Initializes the module. */
    virtual void initialize() override;

    /** Method is called for each event. */
    virtual void event() override;

    /** Method is called at the end of the event processing. */
    virtual void terminate() override;

    /** Static method to get a random CMS energy (used via extern C from fortran). */
    static double generateCMSEnergy()
    {
      return s_initial.generate().getMass();
    }

  protected:

    double getBeamEnergyCM(double e1, double e2, double angle); /**< Get the CM energy from LER, HER and the crossing angle. */

    /** Module parameters */
    std::string m_order; /**< model: born, alpha or exp. */
    std::string m_model; /**< model: matched or ps. */
    std::string m_mode; /**< mode: weighted or unweighted. */
    std::string m_vacPol; /**< Vacuum polarization: off, hadr5 or hmnt. */
    std::string m_finalState; /**< Final state: ee, mm or gg. */
    double m_eMinFrac; /**< Fractional energy (of cms energy) for leptons in the final state. (overrides m_eMin) */
    double m_eMin; /**< Minimum energy for leptons in the final state, in GeV. */
    double m_epsilon; /**< Soft/hard photon separator in units of CMS/2. */
    double m_maxAcollinearity; /**< maximum acollinearity angle between finale state leptons in degrees. */
    double m_fMax; /**< Maximum differential cross section weight. */
    int m_nSearchMax; /**< Events used to search maximum of differential cross section. */
    int m_nPhot; /**< fixed number of nphot (hard) photons are generated. A negative value means all possible photons. */
    std::vector<double> m_ScatteringAngleRange; /**< Min [0] and Max [1] value for the scat. angle [deg] of the e+/e-. */
    std::string m_fileNameExtraInfo; /**< Extra ROOT file that contains the weight distribution to check overweight bias. */
    double m_Spread; /**< TEMPORARY SOLUTION! Approximate energy spread per beam (CMS). */
    bool m_Uncertainty;   /**< vary all VP related parameters and extracted total uncertainty. */
    std::string m_NSKDataFile; /**< data file for the NSK VP data. */

    std::string m_userMode; /**< User mode similar to TEEGG: ETRON, EGAMMA, GAMMA or PRESCALE or NONE. */
    double m_eemin; /**< Minimum CMS energy of the tagged e-/e+ (GeV). */
    double m_temin; /**< Minimum CMS angle between the tagged e-/e+ and -z axis (deg). */
    double m_egmin; /**< Minimum CMS energy of the gamma (GeV). */
    double m_tgmin; /**< Minimum CMS angle between the gamma and -z axis (deg). */
    double m_eeveto; /**< Minimum CMS energy to veto e-/e+ (GeV). */
    double m_teveto; /**< Maximum CMS theta of e-/e+ in final state (deg). */
    double m_egveto; /**< Minimum CMS energy to veto gamma (GeV). */
    double m_tgveto; /**< Maximum CMS angle between the gamma and -z axis (deg). */
    double m_maxprescale; /**< Maximum prescale value. */

    /** Variables */
    BabayagaNLO m_generator;   /**< The BabayagaNLO generator. */
    MCParticleGraph m_mcGraph; /**< The MCParticle graph object. */
    TFile* m_fileExtraInfo;  /**< Output file. */
    TH1D* m_th1dSDif;   /**< Histograms with the event weights. */

    template <typename T>
    std::vector<T> make_vector(T const& t1, T const& t2);  /**< make_vector. */

    template <typename T>
    std::pair<T, T> vectorToPair(std::vector<T>& vec, const std::string& name = "");

  private:
    /** Method is called to initialize the generator. */
    void initializeGenerator();

    bool m_initialized{false}; /**< True if generator has been initialized. */
    DBObjPtr<BeamParameters> m_beamParams; /**< BeamParameter. */

    static InitialParticleGeneration
    s_initial;   /**< Initial particle for beam parameters, static because we need to call it from FORTRAN here. */
  };


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
      B2ERROR("The parameter " + name + " has to have exactly 2 elements !");
    } else {
      p.first  = vec[0];
      p.second = vec[1];
    }
    return p;  /**< vectorToPair. */
  }

} // end namespace Belle2

#endif /* BABAYAGANLOINPUTMODULE_H */
