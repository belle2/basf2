/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* ECL headers. */
#include <ecl/dataobjects/ECLElementNumbers.h>

/* Basf2 headers. */
#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>

/* C++ headers. */
#include <string>

class TTree;
class TFile;

namespace Belle2 {

  class ECLDsp;
  class ECLDigit;
  class ECLTrig;

  /** a module to write ECL waveform and fitted time and amplitude information in a root ntuple */
  class EclCovMatrixNtupleModule : public Module {

  public:

    /**
     * Constructor.
     * Sets the module parameters.
     */
    EclCovMatrixNtupleModule();

    /** Destructor. */
    virtual ~EclCovMatrixNtupleModule() {}

    /** Initializes the module. */
    virtual void initialize() override;

    /** Terminates the module. */
    virtual void terminate() override;

    /** Method is called for each event. */
    virtual void event() override;

  protected:
    /** output root file name (given as Module parameter) */
    std::string m_dataOutFileName;

    /** eclDSPs array name */
    std:: string m_dspArrayName;

    /** eclDigit array name */
    std::string m_digiArrayName;

    /** Root tree name */
    TTree* m_tree{nullptr};
    /** Root file for saving the output */
    TFile* m_rootFile{nullptr};

    /** Energy */
    double m_energy{ -1};
    /** Event number */
    int m_nevt{ -1};
    /** Number of eclHits */
    int m_nhits{ -1};

    /** Crystal ID */
    int m_cellID[ECLElementNumbers::c_NCrystals] = {};
    /** Crystal Theta ID */
    int m_theta[ECLElementNumbers::c_NCrystals] = {};
    /** Crystal Phi ID */
    int m_phi[ECLElementNumbers::c_NCrystals] = {};
    /** eclDsp sample Array */
    int m_DspHit[ECLElementNumbers::c_NCrystals][31] = {};
    /** eclHit Energy */
    double m_hitE[ECLElementNumbers::c_NCrystals] = {};
    /** eclHit Time */
    double m_hitTime[ECLElementNumbers::c_NCrystals] = {};
    /** eclDigit Time */
    int m_DigiTime[ECLElementNumbers::c_NCrystals] = {};
    /** eclTrig Time */
    double m_DeltaT[ECLElementNumbers::c_NCrystals] = {};

  private:
    /** Store array: ECLDsp. */
    StoreArray<ECLDsp> m_eclDspArray;

    /** Store array: ECLDigit. */
    StoreArray<ECLDigit> m_eclDigiArray;

    /** Store array: ECLTrig. */
    StoreArray<ECLTrig> m_eclTrigArray;
  };

} // end namespace Belle2
