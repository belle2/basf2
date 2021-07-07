/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <svd/dataobjects/SVDShaperDigit.h>
#include <svd/dataobjects/SVDAPVHistograms.h>

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/SoftwareTriggerResult.h>

#include <TFile.h>
#include <TH1F.h>
#include <TTree.h>

namespace Belle2 {

  /**The module is used to create a TTree to study the number of strips fired per event per APV chip.
   *
   */
  class SVDMaxStripTTreeModule : public Module {

  public:
    /** Constructor */
    SVDMaxStripTTreeModule();
    /** Register input and output data */
    void initialize() override;
    /** Write the TTrees to the file*/
    void terminate() override;
    /** Compute the variables and fill the tree*/
    void event() override;
    /** Define APVHistogram*/
    void beginRun() override;

  private:


    StoreObjPtr<SoftwareTriggerResult> m_resultStoreObjectPointer; /**< Store Object to read the trigger decision */
    bool m_skipRejectedEvents = false; /**< if true skip events rejected by HLT*/

    std::string m_rootFileName = "";   /**< root file name */
    TFile* m_rootFilePtr = nullptr; /**< pointer at root file used for storing histograms */

    std::string m_shapersStoreArrayName{"SVDShaperDigits"};  /**< storeArray name of the input ShaperDigits */
    StoreArray<SVDShaperDigit> m_shapers; /**< array of SVDShaperDigits*/

    SVDAPVHistograms<TH1F>* m_hHits = nullptr; /**< hits per APV*/

    TTree* m_t = nullptr;  /**< tree containing info related to the U and V side strips*/
    /* Branches */
    unsigned int m_svdLayer = 0;  /**< layer */
    unsigned int m_svdLadder = 0; /**< ladder */
    unsigned int m_svdSensor = 0; /**< sensor */
    unsigned int m_svdSide = -1; /**< side */
    unsigned int m_svdHits = 0; /**< APV hits per event */
    unsigned int m_svdChip = -1; /**< chip   */
    unsigned int m_event = -1; /**<event number*/
  };
}

