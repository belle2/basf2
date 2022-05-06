/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <mdst/dataobjects/MCParticleGraph.h>
#include <Math/LorentzRotation.h>
#include <utility>

namespace Belle2 {

  /**
   * C++ Interface for the Fortran Bhabha scattering generator BHWide.
   *
   * IMPOTRANT NOTE: In the case of generating UNWEIGHTED events,
   * the user should check in the OUTPUT if the number of
   * OVERWEIGHTED events and/or the fraction of the corresponding
   * cross section is ACCEPTABLE for his/her desired PRECISION.
   * If not, the value of WTMAX has to be adjusted (increased)
   * accordingly (making a histogram of the weight might be helpful).
   */
  class BHWide {
  public:

    /** Call needed channels. */
    enum Channel {
      CH_BOTH = 0,    /**< both s and t-channels + interferences. */
      CH_S_ONLY = 1,  /**< only s-channel (for tests!), with model ==  HM_CALKUL.*/
      CH_T_ONLY = 2   /**< only t-channel (for tests!), with model ==  HM_CALKUL. */
    };

    /** Call random generator */
    enum RandomGenerator {
      RG_RANMAR = 1, /**< Ranmar generator. */
      RG_RANECU = 2  /**< Ranecu generator. */
    };

    /** Call electroweak correction */
    enum EWCorrectionLib {
      EC_BABAMC = 1, /**< ElectroWeak Corr. from BABAMC (obsolete). */
      EC_ALIBABA = 2 /**< ElectroWeak Corr. from ALIBABA, RECOMMENDED. */
    };

    /** Call Brem Model */
    enum HardBremsModel {
      HM_HELICITY_AMP = 1, /**< obtained by the BHWide authors (helicity amplitudes). */
      HM_CALKUL = 2        /**< from CALKUL, Nucl. Phys. B206 (1982) 61. */
    };

    /** Call photon polarization */
    enum PhotonVacPolarization {
      PP_OFF = 0,       /**< Photon vacuum polarization is off. */
      PP_BHLUMI = 1,    /**< Burkhardt et.al. 1989, as in BHLUMI 2.0x. */
      PP_EIDELMAN = 2,  /**< S. Eidelman, F. Jegerlehner, Z.Phys.C(1995) */
      PP_BURKHARDT = 3  /**< Burkhardt and Pietrzyk 1995 (Moriond). */
    };

    /** Constructor.
     * Sets the default settings.
     */
    BHWide();

    /** Destrucotr. */
    ~BHWide();

    /** Sets the default settings for the BhWide Fortran generator. */
    void setDefaultSettings();

    /** Enabled the Z contribution.
     * @param zContribution Set to true to enabled the Z contribution.
     */
    void enableZContribution(bool zContribution = true) { m_zContribution = zContribution; }

    /** Set the type of channel which should be used.
     * @param channel The type of channel: s, t or both.
     */
    void setChannel(Channel channel) { m_channel = channel; }

    /** Enable the use of weighted events.
     * @param weighted Set to false to generate events for an unweighted MonteCarlo detector simulation.
     */
    void enableWeights(bool weighted = true) { m_weighted = weighted; }

    /** Set the type of the randomN nmber generator.
     * @param randomGenerator The type of the random number generator: Ranmar or Ranecu.
     */
    void setRandomGenerator(RandomGenerator randomGenerator) { m_randomGenerator = randomGenerator; }

    /** Enable the use of weak corrections.
     * @param weakCorrections Set to true to use weak corrections.
     */
    void enableWeakCorrections(bool weakCorrections = true) { m_weakCorrections = weakCorrections; }

    /** Set the type of the electro weak correction library.
     * @param ewCorrectionLib The electro weak correction library which is used: BABAMC or ALIBABA.
     */
    void setElectroWeakCorrectionsLib(EWCorrectionLib ewCorrectionLib) { m_ewCorrectionLib = ewCorrectionLib; }

    /** Set the type of the hard bremsstrahlung model.
     * @param hardBremsModel The model which is used for the hard bremsstrahlung: helicity amplitudes or CALKUL.
     */
    void setHardBremsModel(HardBremsModel hardBremsModel) { m_hardBremsModel = hardBremsModel; }

    /** Set the model for the photon vacuum polarization.
     * @param photonVacPol The model for the photon vacuum polarization: Off, BHLUMI, Eidelman or Burkhardt.
     */
    void setPhotonVacPolarization(PhotonVacPolarization photonVacPol) { m_photonVacPol = photonVacPol; }  /**< photonVacPol. */

    /** Sets the CMS energy.
     * @param cmsEnergy The CMS energy in [GeV].
     */
    void setCMSEnergy(double cmsEnergy) { m_cmsEnergy = cmsEnergy; }

    /** Sets the theta scattering angle range for the scattered positron.
     * @param angleRange A pair of values, representing the min and max theta angle of the scattered positron in [deg].
     */
    void setScatAnglePositron(std::pair<double, double> angleRange) { m_ScatteringAngleRangePositron = angleRange; }

    /** Sets the theta scattering angle range for the scattered electron.
     * @param angleRange A pair of values, representing the min and max theta angle of the scattered electron in [deg].
     */
    void setScatAngleElectron(std::pair<double, double> angleRange) { m_ScatteringAngleRangeElectron = angleRange; }

    /** Sets the minimal energy for the scattered positron.
     * @param minEnergyFinalStatePos The minimal energy for the scattered positron in [GeV].
     */
    void setMinEnergyFinalStatePos(double minEnergyFinalStatePos) { m_minEnergyFinalStatePos = minEnergyFinalStatePos; }

    /** Sets the minimal energy for the scattered electron.
     * @param minEnergyFinalStateElc The minimal energy for the scattered electron in [GeV].
     */
    void setMinEnergyFinalStateElc(double minEnergyFinalStateElc) { m_minEnergyFinalStateElc = minEnergyFinalStateElc; }

    /** Sets the max acollinearity angle.
     * @param maxAcollinearity The max acollinearity angle in [deg].
     */
    void setMaxAcollinearity(double maxAcollinearity) { m_maxAcollinearity = maxAcollinearity; }

    /** Sets the CMS energy on which an infrared cut is performed.
     * @param infCutCMSEnergy The CMS energy in [Gev] at which an infrared cut is performed.
     */
    void setInfCutCMSEnergy(double infCutCMSEnergy) { m_infCutCMSEnergy = infCutCMSEnergy; }

    /** Sets the max weight at which events are rejected.
     * @param maxRejectionWeight The weight at which events are rejected.
     */
    void setMaxRejectionWeight(double maxRejectionWeight) { m_maxRejectionWeight = maxRejectionWeight; }

    /** Sets the mass of the Z Boson.
     * @param massZ The mass of the Z Boson in [Gev].
     */
    void setMassZ(double massZ) { m_massZ = massZ; }

    /** Sets the width of the Z Boson.
     * @param widthZ The width of the Z Boson in [GeV].
     */
    void setWidthZ(double widthZ) { m_widthZ = widthZ; }

    /** Sets the value for the sinW2.
     * @param sinW2 The value for the sinW2.
     */
    void setSinW2(double sinW2) { m_sinW2 = sinW2; }

    /** Sets the mass for the top quark.
     * @param massTop The mass for the top quark in [GeV].
     */
    void setMassTop(double massTop) { m_massTop = massTop; }

    /** Sets the mass for the Higgs Boson.
     * @param massHiggs The mass for the Higgs Boson in [GeV].
     */
    void setMassHiggs(double massHiggs) { m_massHiggs = massHiggs; }

    /** Initializes the generator.
     * Sets the default values for the internal Fortran generator.
     */
    void init();

    /** Generates one single event.
     * @param mcGraph Reference to the MonteCarlo graph into which the generated particles will be stored.
     * @param vertex generated vertex.
     * @param boost generated boost.
     */
    void generateEvent(MCParticleGraph& mcGraph, TVector3 vertex, ROOT::Math::LorentzRotation boost);

    /**
     * Terminates the generator.
     * Closes the internal Fortran generator and retrieves the total cross section.
     */
    void term();

    /** Returns the total cross section of the generated process.
     * @return The total cross section.
     */
    double getCrossSection() { return m_crossSection; }

    /** Returns the error on the total cross section of the generated process.
     * @return The error on the total cross section.
     */
    double getCrossSectionError() { return m_crossSectionError; }


  protected:

    bool m_zContribution;                 /**< Z-contribution ON/OFF. */
    Channel m_channel;                    /**< Channel choice. */
    bool m_weighted;                      /**< Switch for constant, variable weight. Unweighted events for detector simulation, or weighted events. */
    RandomGenerator m_randomGenerator;    /**< Type of random number generator. */
    bool m_weakCorrections;               /**< Switching ON/OFF weak corrections. true = all ElectroWeak Corrections included, false = only QED corrections included.*/
    EWCorrectionLib m_ewCorrectionLib;    /**< Option for ElectroWeak Corrections Library. */
    HardBremsModel m_hardBremsModel;      /**< type of MODEL subprogram and QED matrix element for hard bremsstrahlung. */
    PhotonVacPolarization m_photonVacPol; /**< Photon vacuum polarization switch. */

    double m_cmsEnergy; /**< CMS Energy = 2*Ebeam [GeV]. */
    std::pair<double, double> m_ScatteringAngleRangePositron; /**< Min and Max value for the scattering angle [deg] of the positron. */
    std::pair<double, double> m_ScatteringAngleRangeElectron; /**< Min and Max value for the scattering angle [deg] of the electron. */

    double m_minEnergyFinalStatePos; /**< Minimum energy [GeV] for final state positron. */
    double m_minEnergyFinalStateElc; /**< Minimum energy [GeV] for final state electron. */
    double m_maxAcollinearity;       /**< Maximum acollinearity [deg] of final state e+e-. */
    double m_infCutCMSEnergy;        /**< Dimensionless infrared cut on CMS energy of soft photons, ( E_phot > CMSENE*EPSCMS/2 ). */
    double m_maxRejectionWeight;     /**< Maximum Weight for rejection (if <= 0, it is reset inside the program). */

    double m_massZ;     /**< Z mass  [GeV]. */
    double m_widthZ;    /**< Z width [GeV] (may be recalculated by EW library). */
    double m_sinW2;     /**< sin^2(theta_W) (may be recalculated by EW library). */
    double m_massTop;   /**< top quark mass [GeV]. */
    double m_massHiggs; /**< Higgs mass [GeV] */

    double m_crossSection;      /**< The cross section of the generated bhabha scattering events. */
    double m_crossSectionError; /**< The error on the cross section of the generated bhabha scattering events. */

    /** Apply the settings to the internal Fortran generator. */
    void applySettings();

    /** Store a single generated particle into the MonteCarlo graph.
     * @param mcGraph Reference to the MonteCarlo graph into which the particle should be stored.
     * @param mom The 3-momentum of the particle in [GeV].
     * @param pdg The PDG code of the particle.
     * @param isVirtual If the particle is a virtual particle, such as the incoming particles, set this to true.
     * @param isInitial If the particle is a initial particle for ISR, set this to true.
     */
    void storeParticle(MCParticleGraph& mcGraph, const double* mom, int pdg, TVector3 vertex, ROOT::Math::LorentzRotation boost,
                       bool isVirtual = false, bool isInitial = false);

  private:

    int    m_npar[100];  /**< Integer parameters for BHWide. */
    double m_xpar[100];  /**< Double parameters for BHWide. */
  };
}


