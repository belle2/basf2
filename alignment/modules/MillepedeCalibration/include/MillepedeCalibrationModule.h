/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Tadeas Bilka (tadeas.bilka@gmail.com)                    *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef MILLEPEDECALIBRATIONMODULE_H
#define MILLEPEDECALIBRATIONMODULE_H

#include <calibration/CalibrationModule.h>
#include <genfit/MilleBinary.h>
#include <framework/pcore/RootMergeable.h>
#include <TH1D.h>
#include <TTree.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>


#include <calibration/ManagedStorableList.h>
#include <alignment/dbobjects/VXDAlignment.h>

#include <genfit/MilleBinary.h>

namespace Belle2 {

  /**
   * Global Belle2 calibration & alignment using General Broken Lines refit and Millepede II.
   *
   * This calibration modules will temporarily include all Millepede calibration tasks.
   * It will be separated to more modules as it will grow.
   *
   */
  class MillepedeCalibrationModule : public calibration::CalibrationModule {

  public:

    /**
     * Constructor: sets the description, the properties and the parameters of the module.
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

    void storeData() override;
    void resetData() override;
    void loadData() override;

  private:
    template<class ...Args> void manageMille(StoreObjPtr<MilleData>& obj, string name, Args&& ... params)
    {
      obj.registerInDataStore(name, DataStore::c_DontWriteOut);
      if (!obj.isValid()) {
        obj.construct(std::forward<Args>(params)...);
      }


      ManagedStorableList::write_prototype write =
      [ = ](TFile * file) -> void {
        StoreObjPtr< MilleData > object(name, DataStore::c_Persistent);
        file->cd();
        object->Write(name.c_str());
      };

      ManagedStorableList::reset_prototype reset =
      [ = ]() -> void {
        StoreObjPtr< MilleData > object(name, DataStore::c_Persistent);
        object->clear();
      };

      ManagedStorableList::read_prototype read =
      [ = ](TFile * file) -> bool {
        if (!file || !file->IsOpen())
          return false;
        file->cd();
        StoreObjPtr< MilleData > object(name, DataStore::c_Persistent);
        object->Read(name.c_str());
        return true;
      };

      ManagedStorableList::StorableFunctors functors;
      functors.read = read;
      functors.reset = reset;
      functors.write = write;

      storables.addObject(name, functors);
    }

    std::string m_tracks;  /**< Name of collection of genfit::Tracks for calibration */
    std::string m_binary;  /**< Name of Mille binary file with calibration data */
    std::string m_steering; /**< Name of StoreObjPtr<PedeSteering> with commands */
    double m_minPvalue; /**< Minimal p-value to write trajectory to Mille binary */
    StoreObjPtr<RootMergeable<TH1D>> m_histoProcID; /**< Mergeable histogram for monitoring processes */
    StoreObjPtr<RootMergeable<TH1D>> m_histoPval; /**< Mergeable histogram for monitoring p-value of tracks for calibration */
    StoreObjPtr<RootMergeable<TH1D>> m_histoNormChi2; /**< Mergeable histogram for monitoring Chi2/Ndf of tracks for calibration */
    StoreObjPtr<RootMergeable<TH1D>> m_histoNdf; /**< Mergeable histogram for monitoring Ndf of tracks for calibration */
    StoreObjPtr<RootMergeable<TH1D>> m_histoRun;
    StoreObjPtr<RootMergeable<TTree>> m_treeResidual;

    VXDAlignment m_finalAlignment;

    StoreObjPtr<MilleData> m_mille;

    ManagedStorableList storables;


  };
}

#endif /* MILLEPEDECALIBRATIONMODULE_H */
