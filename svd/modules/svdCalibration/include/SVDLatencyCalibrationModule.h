/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: casarosa                                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SVDLATENCYCALIBRATIONMODULE_H
#define SVDLATENCYCALIBRATIONMODULE_H

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>

#include <svd/dataobjects/SVDHistograms.h>
#include <svd/dataobjects/SVDShaperDigit.h>

#include <string>
#include <TTree.h>
#include <TFile.h>
#include <TH1F.h>
#include <TList.h>

// forward declarations
class TTree;
class TFile;


namespace Belle2 {
  /**
   * this module perfoms an analysis to find the APV25 latency
   *
   */
  class SVDLatencyCalibrationModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    SVDLatencyCalibrationModule();

    /**  */
    virtual void initialize() override;

    /**  */
    virtual void beginRun() override;

    /**  */
    virtual void event() override;

    /**  */
    virtual void endRun() override;

    /* user-defined parameters */
    std::string m_rootFileName;   /**< root file name */

    /* ROOT file related parameters */
    TFile* m_rootFilePtr; /**< pointer at root file used for storing histograms */

  private:

    SVDHistograms<TH1F>* m_histo_maxAmplitude;

    std::string m_shapersListName; /**< shapers list name */
    StoreArray<SVDShaperDigit> m_digits; /**< SVD digits*/

  };
}

#endif /* SVDLATENCYCALIBRATIONMODULE_H */
