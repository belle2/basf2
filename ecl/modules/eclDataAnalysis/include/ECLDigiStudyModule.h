/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

//STL
#include <string>

//Framework
#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>

class TFile;
class TTree;

namespace Belle2 {

  class ECLDsp;
  class ECLDigit;
  class ECLTrig;
  class ECLHit;

  /** A module to analyse digit level information */
  class ECLDigiStudyModule : public Module {

  public:

    /**
     * Constructor.
     * Sets the module parameters.
     */
    ECLDigiStudyModule();

    /** Destructor. */
    virtual ~ECLDigiStudyModule() {}

    /** Initializes the module. */
    virtual void initialize() override;

    /** terminate */
    virtual void terminate() override;

    /** Method is called for each event. */
    virtual void event() override;

  protected:
    /** output root file name (given as Module parameter) */
    std::string m_dataOutFileName,  /**< Root file name for saving the output */
        m_dspArrayName1, /**< Name of first DSP array */
        m_digiArrayName1,  /**< Name of first digit array */
        m_dspArrayName2,  /**< Name of second DSP array */
        m_digiArrayName2;  /**< Name of second digit array */

    TTree* m_tree{nullptr};  /**< Root tree for saving the output */
    TFile* m_rootFile{nullptr};  /**< Root file for saving the output */

    int m_nhits = 8736; /**< Maximum number of hits */
    int m_neclhits{ -1}; /**< Actual number of hits */
    int m_cellId[8736] = {}; /**< Array of cellIDs */
    double m_energy[8736] = {}; /**< Array of deposited MC energy */
    double m_allenergy[8736] = {}; /**< Array of deposited energy */
    double m_time[8736] = {}; /**< Array of digit time */
    int m_theta[8736] = {}; /**< Array oh ThetaID */
    int m_phi[8736] = {}; /**< Array of PhiID */

    int m_DspHit1[8736][31] = {}; /**< WF sampling points for first digit array */
    int m_DspHit2[8736][31] = {};  /**< WF sampling points for second digit array */
    int m_baseline1[8736][16] = {};  /**< Baseline sampling points for first digit array */
    int m_baseline2[8736][16] = {};  /**< Baseline sampling points for second digit array */
    double m_baselineAvg1[8736] = {}; /**< Baseline energy for first digit array */
    double m_baselineAvg2[8736] = {}; /**< Baseline energy for second digit array */
    int m_maxVal1[8736] = {}; /**< WF maximum for first digit array */
    int m_maxVal2[8736] = {}; /**< WF maximum for second digit array */
    int m_digiQual1[8736] = {}; /**< Digit quality for first digit array */
    int m_digiQual2[8736] = {}; /**< Digit quality for second digit array */
    double m_digiTime1[8736] = {}; /**< Digit time for first digit array */
    double m_digiTime2[8736] = {}; /**< Digit time for second digit array */;
    double m_digiE1[8736] = {}; /**< Deposited energy for first digit array */
    double m_digiE2[8736] = {}; /**< Deposited energy for second digit array */
    double m_trig1{ -1}; /**< Trigger time for array 1 */
    double m_trig2{ -1}; /**< Trigger time for array 2 */
  private:
    /** Store array: ECLDsp. */
    StoreArray<ECLDsp> m_eclDspArray1;

    /** Store array: ECLDsp. */
    StoreArray<ECLDsp> m_eclDspArray2;

    /** Store array: ECLDigit. */
    StoreArray<ECLDigit> m_eclDigiArray1;

    /** Store array: ECLDigit. */
    StoreArray<ECLDigit> m_eclDigiArray2;

    /** Store array: ECLTrig. */
    StoreArray<ECLTrig> m_eclTrigArray;

    /** Store array: ECLHit. */
    StoreArray<ECLHit> m_eclHitsArray;

  };

} // end namespace Belle2
