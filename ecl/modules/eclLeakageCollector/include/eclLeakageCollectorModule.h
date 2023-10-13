/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* Basf2 headers. */
#include <calibration/CalibrationCollectorModule.h>
#include <framework/database/DBObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <ecl/dbobjects/ECLnOptimal.h>
#include <TH2F.h>

namespace Belle2 {
  class ECLCrystalCalib;
  class ECLShower;
  class MCParticle;
  class EventMetaData;

  namespace ECL {
    class ECLLeakagePosition;
  }



  /** Store information needed to calculate ECL energy leakage corrections */
  class eclLeakageCollectorModule : public CalibrationCollectorModule {

  public:

    /** Constructor: Sets the description, the properties and the parameters of the module */
    eclLeakageCollectorModule();

    /** Define histograms and read payloads from DB */
    virtual void prepare() override;

    /** Accumulate TTree */
    virtual void collect() override;

  private:

    /** Parameters to pass along to the algorithm */
    int m_position_bins; /**< number of crystal subdivisions in theta & phi (29) */
    int m_number_energies; /**< number of generated energies (8) */
    std::vector<double> m_energies_forward; /**< generated photon energies, forward */
    std::vector<double> m_energies_barrel; /**< generated photon energies, barrel */
    std::vector<double> m_energies_backward; /**< generated photon energies, backward */

    /** Required arrays */
    std::string m_showerArrayName;  /**< Name of ECLShower StoreArray data object */
    StoreArray<ECLShower> m_eclShowerArray; /**< Required input array of ECLShowers */
    StoreArray<MCParticle> m_mcParticleArray; /**< Required input array of MCParticles */
    StoreObjPtr<EventMetaData> m_evtMetaData; /**< dataStore EventMetaData */

    /** Some other useful quantities */
    const int nLeakReg = 3; /**< 3 ECL regions: 0 = forward, 1 = barrel, 2 = backward */
    bool storeCalib = true; /**< store parameters first event */
    std::vector< std::vector<int> > i_energies;/**< Generated energies in MeV in each region */

    ECL::ECLLeakagePosition* leakagePosition{nullptr}; /**< location of position of cluster */
    int m_nDump = 0; /**< Number of events with diagnostic printouts */

    /** nOptimal payload */
    DBObjPtr<ECLnOptimal> m_eclNOptimal;
    TH2F m_nOptimal2D; /**< 2D hist of nOptimal for Ebin vs groupID */
    TH2F m_peakFracEnergy; /**< 2D histogram of peak fractional contained energy */
    TH2F m_bias; /**< 2D histogram of bias = sum of ECLCalDigit energy minus true (GeV) */
    TH2F m_logPeakEnergy; /**< log of peak contained energy in GeV */

    std::vector<int> m_groupNumber; /**< group number for each crystal */
    const int m_nLeakReg = 3; /**< 3 ECL regions: 0 = forward, 1 = barrel, 2 = backward */
    int m_nEnergyBins = 0; /**< number of energies bins in nOptimal payload */
    std::vector< std::vector<float> > m_eBoundaries; /**< energy boundaries each region */

    /** For TTree */
    int t_cellID = 0; /**< cellID of photon */
    int t_thetaID = 0; /**< thetaID of photon */
    int t_region = 0; /**< region of photon 0=forward 1=barrel 2=backward*/
    int t_thetaBin = -1; /**< binned location in theta relative to crystal edge */
    int t_phiBin =
      -1; /**< binned location in phi relative to crystal edge. Starts from edge with mechnical structure, or else lower edge */
    int t_phiMech = -1; /**< 0: mechanical structure next to phi edge; 1: no mech structure */
    int t_energyBin = -1; /**< generated energy point */
    int t_nCrys = -1; /**< number of crystals used to calculate energy */
    float t_energyFrac = 0.; /**< measured energy without leakage correction divided by generated */
    float t_origEnergyFrac = 0.; /**< original leakage-corrected energy / generated */
    float t_locationError = 999.; /**< reconstructed minus generated position (cm) */
  };
}
