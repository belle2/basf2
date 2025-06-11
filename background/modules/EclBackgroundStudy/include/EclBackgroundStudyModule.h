/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <ecl/dataobjects/ECLSimHit.h>
#include <ecl/dataobjects/ECLHit.h>
#include <framework/core/Module.h>
#include <trg/ecl/TrgEclMapping.h>
#include <framework/datastore/StoreArray.h>
#include <framework/logging/Logger.h>

#include <string>

#include <Math/Vector3D.h>
#include <TFile.h>
#include <TTree.h>

namespace Belle2 {

  /**
   * EclBackgroundStudy module
   */
  class EclBackgroundStudyModule : public Module {
  public:

    //! Constructor.
    EclBackgroundStudyModule();

    //! Destructor.
    virtual ~EclBackgroundStudyModule();

    /**
    * Initialize the Module.
    *
    * This method is called at the beginning of data processing.
    */
    virtual void initialize();

    /**
    * Called when entering a new run.
    *
    * Set run dependent things like run header parameters, alignment, etc.
    */
    virtual void beginRun();

    /**
    * Event processor.
    *
    * Convert reads information from ECLHits/ECKSimHits and writes tree file.
    */
    virtual void event();

    /**
    * End-of-run action.
    *
    * Save run-related stuff, such as statistics.
    */
    virtual void endRun();

    /**
    * Termination action.
    *
    * Clean-up, close files, summarize statistics, etc.
    */
    virtual void terminate();

    /**
    *Prints module parameters.
    */
    void printModuleParams() const;

  private:

    std::string m_filename; /**< Output file name.   */

    std::vector<int>  m_CellId; /**< Vector of CellId */
    std::vector<int>  m_TcId; /**< Vector of TcId */
    std::vector<int>  m_Pdg; /**< Vector of PDG codes */
    std::vector<double>  m_Edep; /**< Vector of deposited energies */
    std::vector<double>  m_TimeAve; /**< Vector of average times */
    std::vector<double>  m_FlightTime; /**< Vector of flight times */
    std::vector<double>  m_Hadronedep; /**< Vector of hadronic deposited energies */

    Int_t m_iEntry; /**< Entry identifier */

    TFile* m_ff;    /**< Output root file.   */
    TTree* m_tree1;   /**< Output tree */
    TTree* m_tree2;   /**< Output tree */
    TrgEclMapping* m_TCMap; /**< TRGECL mapping */
    StoreArray<ECLSimHit>   m_ECLSimHits; /**< ECL simHits */
    StoreArray<ECLHit>      m_ECLHits; /**< ECL hits */
  };
} // Belle2 namespace
