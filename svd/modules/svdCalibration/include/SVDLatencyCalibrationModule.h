/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef SVDLATENCYCALIBRATIONMODULE_H
#define SVDLATENCYCALIBRATIONMODULE_H

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>

#include <svd/dataobjects/SVDHistograms.h>
#include <svd/dataobjects/SVDShaperDigit.h>

#include <string>
#include <TFile.h>
#include <TH1F.h>

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

    /**  initialize */
    virtual void initialize() override;

    /**  begin run*/
    virtual void beginRun() override;

    /**  event*/
    virtual void event() override;

    /**  end run */
    virtual void endRun() override;


  private:

    /* user-defined parameters */
    std::string m_rootFileName;   /**< root file name */

    /* ROOT file related parameters */
    TFile* m_rootFilePtr = nullptr; /**< pointer at root file used for storing histograms */


    SVDHistograms<TH1F>* m_histo_maxAmplitude = nullptr; /**<vector of histograms containing the max bin distribution*/

    std::string m_shapersListName; /**< shapers list name */
    StoreArray<SVDShaperDigit> m_digits; /**< SVD digits*/

  };
}

#endif /* SVDLATENCYCALIBRATIONMODULE_H */
