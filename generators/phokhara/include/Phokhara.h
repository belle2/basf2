/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef PHOKHARA_H
#define PHOKHARA_H

#include <mdst/dataobjects/MCParticleGraph.h>
#include <TLorentzRotation.h>
#include <utility>

namespace Belle2 {

  /**
   * C++ Interface for the Fortran generator PHOKHARA.
   *
   */

  class Phokhara {
  public:

    /** Constructor.
     * Sets the default settings.
     */
    Phokhara();

    /** Destructor. */
    ~Phokhara();

    /** Sets the default settings for the PHOKHARA generator. */
    void setDefaultSettings();

    /** Sets LO correction mode
      * @param LO Defines LO: 1ph (0, default), Born: 0ph (1), only Born: 0ph (-1).
    */
    void setLO(int LO) { m_LO = LO; }

    /** Sets weighted mode
      * @param weighted generate weighted events.
    */
    void setWeighted(int weighted) { m_weighted = weighted; }

    /** Sets NLO mode
      * @param NLO Defines NLO, for 1ph only: off (0, default), on (1)
    */
    void setNLO(int NLO) { m_NLO = NLO; }

    /** Sets Full NLO mode
      * @param full NLO : No(0), Yes(1)
    */
    void setFullNLO(int FullNLO) { m_fullNLO = FullNLO; }

    /** Sets QED corrections
      * @param QED QED corrections: ISR only(0), ISR+FSR(1), ISR+INT+FSR(2)
    */
    void setQED(int QED) { m_QED = QED; }

    /** Sets IFSNLO options
      * @param IFSNLO: no(0), yes(1)
    */
    void setIFSNLO(int IFSNLO) { m_IFSNLO = IFSNLO; }

    /** Sets alpha qed options
      * @param alpha Vacuum polarization switch: off (0), on (1,[by Fred Jegerlehner], default), on (2,[by Thomas Teubner])
    */
    void setAlpha(int alpha) { m_alpha = alpha; }

    /** Sets pion formfactors
      * @param pionff KS Pionformfactor(0), GS Pionformfactor(1) old, GS Pionformfactor new(2)
    */
    void setPionFF(int pionff) { m_pionff = pionff; }

    /** Sets kaon formfactors
      * @param kaonff KaonFormFactor constrained (0),KaonFormFactor unconstrained (1),KaonFormFactor old (2)
    */
    void setKaonFF(int kaonff) { m_kaonff = kaonff; }

    /** Sets Pion Structure
      * @param pionstructure f0+f0(600): KK model(0), no structure(1), no f0+f0(600)(2), f0 KLOE(3)
    */
    void setPionStructure(int pionstructure) { m_pionstructure = pionstructure; }

    /** Sets narrow resonances
      * @param narres no narrow resonances (0), J/Psi (1), Psi(2S) (2) (narrow resonances only for pion = 0, 1, 6, 7
    */
    void setNarrowRes(int narres) { m_narres = narres; }

    /** Sets Proton formfactors
      * @param protonff f0+f0(600): KK model(0), no structure(1), no f0+f0(600)(2), f0 KLOE(3)
    */
    void setProtonFF(int protonff) { m_protonff = protonff; }

    /** Sets Chi production
      * @param chi_sw: Radiative return(0), Chi production(1), Radiative return + Chi production (2)
    */
    void setChiSW(int chisw) { m_chi_sw = chisw; }

    /** Switches beam resolution for Chi production
      * @param be_r without beam resolution(0), with beam resolution(1). Works only for pion=11 and pion=12
    */
    void setSwitchBeamResolution(int be_r) { m_be_r = be_r; }

    /** Beam resolution for Chi production
      * @param beamres - beam resolution for pion==11 and pion==12 only
    */
    void setBeamResolution(double beamres) { m_beamres = beamres; }

    /** Sets the theta scattering angle range for the photon.
      * @param angleRange A pair of values, representing the min and max theta angle of the photon in [deg].
      */
    void setScatteringAngleRangePhoton(std::pair<double, double> angleRange) { m_ScatteringAngleRangePhoton = angleRange; }

    /** Sets the theta scattering angle range for the final state particles.
       * @param angleRange A pair of values, representing the min and max theta angle of the final state particles in [deg].
       */
    void setScatteringAngleRangeFinalStates(std::pair<double, double> angleRange) { m_ScatteringAngleRangeFinalStates = angleRange; }

    /** Sets the minimal hadrons(muons)-gamma-inv mass squared
     * @param MinInvMassHadronsGamma minimal hadrons(muons)-gamma-inv mass squared in [GeV^2].
     */
    void setMinInvMassHadronsGamma(double MinInvMassHadronsGamma) { m_MinInvMassHadronsGamma = MinInvMassHadronsGamma; }

    /** Sets the minimal inv. mass squared of the hadrons(muons)
     * @param MinInvMassHadrons minimal inv. mass squared of the hadrons(muons) in [GeV^2].
     */
    void setm_MinInvMassHadrons(double MinInvMassHadrons) { m_MinInvMassHadrons = MinInvMassHadrons; }

    /** Sets whether to force the minimal invariant mass squared cut.
     *  This cut is ignored by PHOKHARA with LO = 1, NLO = 1.
     * @param[in] forceMinInvMassHadronsCut Whether to force the cut or not.
     */
    void setForceMinInvMassHadronsCut(bool forceMinInvMassHadronsCut)
    { m_ForceMinInvMassHadronsCut = forceMinInvMassHadronsCut; }

    /** Sets the maximal inv. mass squared of the hadrons(muons)
     * @param MaxInvMassHadrons maximal inv. mass squared of the hadrons(muons) in [GeV^2].
     */
    void setm_MaxInvMassHadrons(double MaxInvMassHadrons) { m_MaxInvMassHadrons = MaxInvMassHadrons; }

    /** Sets the minimal photon energy/missing energy
     * @param MinEnergyGamma minimal photon energy/missing energy in [GeV^2].
     */
    void setMinEnergyGamma(double MinEnergyGamma) { m_MinEnergyGamma = MinEnergyGamma; }

    /** Sets the CMS energy.
     * @param cmsEnergy The CMS energy in [GeV].
     */
    void setCMSEnergy(double cmsEnergy) { m_cmsEnergy = cmsEnergy; }

    /** Sets the number of events used to search maximum
     * @param nSearchMax Number of events for maximum search
     */
    void setNSearchMax(int nSearchMax) { m_nSearchMax = nSearchMax; }

    /** Sets soft/hard photon energy separator.
    * @param epsilon soft/hard photon energy separator
    */
    void setEpsilon(double epsilon) { m_epsilon = epsilon; }

    /** Sets final state.
     * @param finalState Final state code.
     */
    void setFinalState(int finalState) { m_finalState = finalState; }

    /** Sets whether to replace muons by a virtual photon.
     * @param replaceMuonsByVirtualPhoton If true, perform the replacement.
     */
    void setReplaceMuonsByVirtualPhoton(bool replaceMuonsByVirtualPhoton)
    { m_replaceMuonsByVirtualPhoton = replaceMuonsByVirtualPhoton; }

    /** Sets number of trials per event.
    * @param nMaxTrials PHOKHARA is very ineffienct when using NLO corrections, adjust number of trials to >1000 per event!
    */
    void setNMaxTrials(int nMaxTrials) { m_nMaxTrials = nMaxTrials; }

    /** Initializes the generator.
     * @param paramFile The path to the input param file for Phokhara.
     */
    void init(const std::string& paramFile);

    /** Generates one single event.
     * @param mcGraph Reference to the MonteCarlo graph into which the generated particles will be stored.
     * @param vertex generated vertex.
     * @param boost generated boost.
     */
    double generateEvent(MCParticleGraph& mcGraph, TVector3 vertex, TLorentzRotation boost);

    /**
     * Terminates the generator.
     * Closes the internal Fortran generator.
     */
    void term();

  protected:
    //constants (unused in PHOKHARA yet!!!!!!!!!!!!!!)
    double m_pi;                    /**< pi=3.1415.... */
    double m_conversionFactor;      /**< Conversion factor for hbarc to nb. */
    double m_alphaQED0;             /**< QED coupling constant at Q=0. */
    double m_massElectron;          /**< muon mass. */
    double m_massMuon;              /**< electron mass. */
    double m_massW;                 /**< W mass  [GeV] for on shell sin2theta and GF. */
    double m_massZ;                 /**< Z mass  [GeV]. */
    double m_widthZ;                /**< Z width [GeV] (may be recalculated by EW library). */

    //PHOKHARA
    int m_finalState;  /**< final state, called 'pion' in Phokhara, dont get confused. */
    bool m_replaceMuonsByVirtualPhoton; /**< Replace muons by a virtual photon. */
    int m_nMaxTrials;  /**< Events before loop is aborted. */
    int m_nSearchMax;  /**< Events used to search maximum of differential cross section. */
    double m_cmsEnergy;         /**< CMS Energy = 2*Ebeam [GeV]. */
    double m_epsilon;           /**< Soft/hard photon separator in units of CMS/2., called 'w' in Phokhara */
    int m_weighted;  /**< generate weighted events */
    int m_LO;  /**< LO: 1ph(0, default), Born: 0ph(1), only Born: 0ph(-1) */
    int m_NLO;  /**< NLO, for 1ph only: off (0, default), on (1) */
    int m_fullNLO;  /**< NLO, full NLO : No(0), Yes(1) */
    int m_QED;  /**< ISR only(0), ISR+FSR(1), ISR+INT+FSR(2) */
    int m_IFSNLO;  /**< IFSNLO: no(0), yes(1) */
    int m_alpha;  /**< vacuum polarization switch: off (0), on (1,[by Fred Jegerlehner]), on (2,[by Thomas Teubner]) */
    int m_pionff;  /**< FF_pion: KS PionFormFactor(0),GS old (1),GS new (2) */
    int m_pionstructure;  /**< for pi+pi- only: f0+f0(600): K+K- model(0), "no structure" model(1), no f0+f0(600)(2), f0 KLOE(3) */
    int m_kaonff;  /**< FF_kaon: KaonFormFactor constrained(0), KaonFormFactor unconstrained(1) KaonFormFactor old(2) */
    int m_narres;  /**< narr_res: no narrow resonances (0), J/psi (1) and psi(2S) (2) only for m_finalState = 0,1,6,7 */
    int m_protonff;  /**< ProtonFormFactor old(0), ProtonFormFactor new(1) */
    int m_chi_sw;  /**< chi_sw: Radiative return(0), Chi production(1), Radiative return + Chi production (2) */
    int m_be_r;  /**< be_r: without beam resolution(0), with beam resolution(1) */

    std::pair<double, double> m_ScatteringAngleRangePhoton; /**< Minimal/Maximal photon angle/missing momentum angle. */
    std::pair<double, double> m_ScatteringAngleRangeFinalStates; /**< Minimal/Maximal pions(muons,nucleons,kaons) momentum angle. */
    double m_MinInvMassHadronsGamma; /**< minimum mass of the hadron-gamma system [GeV^2] */
    double m_MinInvMassHadrons; /**< minimum mass of the hadron system [GeV^2] */
    bool m_ForceMinInvMassHadronsCut; /**< Force application of the above cut. */
    double m_MaxInvMassHadrons; /**< maximum mass of the hadron system [GeV^2] */
    double m_MinEnergyGamma; /**< minimum gamma energy [GeV] */
    double m_beamres; /**< beam resolution for chi2 studies*/

    /**
    * Apply the settings to the internal Fortran generator.
    */
    void applySettings();

    /** Store a single generated particle into the MonteCarlo graph.
     * @param mcGraph Reference to the MonteCarlo graph into which the particle should be stored.
     * @param mom The 3-momentum of the particle in [GeV].
     * @param pdg The PDG code of the particle.
     * @param vertex generated vertex.
     * @param boost generated boost.
     * @param isVirtual If the particle is a virtual particle, such as the incoming particles, set this to true.
     * @param isInitial If the particle is a initial particle for ISR, set this to true.
     */
    void storeParticle(MCParticleGraph& mcGraph, const double* mom, int pdg, TVector3 vertex, TLorentzRotation boost,
                       bool isVirtual = false, bool isInitial = false);

  private:

    int    m_npar[100];  /**< Integer parameters for PHOKHARA. */
    double m_xpar[100];  /**< Double parameters for PHOKHARA */
  };
}


#endif /* PHOKHARA_H */
