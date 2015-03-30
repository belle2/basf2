/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: tadeas                                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef MILLEPEDECALIBRATIONMODULE_H
#define MILLEPEDECALIBRATIONMODULE_H

#include <calibration/CalibrationModule.h>
#include <genfit/MilleBinary.h>
#include <framework/pcore/RootMergeable.h>
#include <TH1D.h>
#include <framework/datastore/StoreObjPtr.h>

namespace Belle2 {
  /**
   * Global Belle2 calibration&alignment using GeneralBrokenLines refit and Millepede II
   *
   * This calibration modules will temporarily include all Millepede calibration tasks. It will be separated to more modules
   * as it will grow.
   *
   */
  class MillepedeCalibrationModule : public calibration::CalibrationModule {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    MillepedeCalibrationModule();
    /**
     * Prepare data collection.
     */
    void Prepare() override;
    /**
     * Collect event data.
     */
    void CollectData() override;
    /**
     * Run calibration.
     */
    CalibrationModule::ECalibrationModuleResult Calibrate() override;
    /**
     * Monitor calibration result.
     */
    CalibrationModule::ECalibrationModuleMonitoringResult Monitor() override;
    /**
     * Store calibration constants.
     */
    bool StoreInDataBase() override;
    /**
     * Close file opened in event processes.
     */
    void closeParallelFiles() override;

  private:
    /**
     * Read Pede result and convert to XML
     * @return bool True if success.
     */
    bool readResultWriteXml(const std::string& xml_filename);




  private:

    std::string m_tracks;  /**< Name of collection of genfit::Tracks for calibration */
    std::string m_binary;  /**< Name of Mille binary file with calibration data */
    std::string m_steering; /**< Name of Pede steering file with calibration settings */
    double m_minPvalue; /**< Minimal p-value to write trajectiry to Mille binary */
    gbl::MilleBinary* m_milleFile; /**< Pointer to Mille binary file with calibration data */
    StoreObjPtr<RootMergeable<TH1D>> m_histoProcID; /**< Mergeable histogram for monitoring processeses */
    StoreObjPtr<RootMergeable<TH1D>> m_histoPval; /**< Mergeable histogram for monitoring p-value of tracks for calibration */
    StoreObjPtr<RootMergeable<TH1D>> m_histoNormChi2; /**< Mergeable histogram for monitoring Chi2/Ndf of tracks for calibration */
    StoreObjPtr<RootMergeable<TH1D>> m_histoNdf; /**< Mergeable histogram for monitoring Ndf of tracks for calibration */
  };
}

#endif /* MILLEPEDECALIBRATIONMODULE_H */
