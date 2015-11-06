/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Tadeas Bilka (tadeas.bilka@gmail.com)                    *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <alignment/modules/MillepedeCalibration/MillepedeCalibrationModule.h>

#include <calibration/CalibrationManager.h>
#include <calibration/CalibrationModule.h>

#include <alignment/PedeResult.h>
#include <alignment/GlobalLabel.h>

#include <framework/datastore/StoreArray.h>
#include <genfit/Track.h>
#include <genfit/GblFitter.h>
#include <genfit/MilleBinary.h>
#include <genfit/GblPoint.h>
#include <genfit/GblTrajectory.h>
#include <genfit/GblFitStatus.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/pcore/RootMergeable.h>
#include "alignment/dbobjects/VXDAlignment.h"
#include <TH1D.h>
#include <TFile.h>
#include <TSystem.h>
#include <framework/database/DBObjPtr.h>
#include <framework/database/Database.h>
#include <framework/database/IntervalOfValidity.h>
#include <alignment/dataobjects/MilleData.h>

#include <vxd/geometry/GeoCache.h>

#include <alignment/dataobjects/PedeSteering.h>
#include <alignment/PedeApplication.h>

#include <string>
#include <algorithm>

class TH1D;

using namespace std;
using namespace Belle2;
using namespace calibration;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(MillepedeCalibration)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

MillepedeCalibrationModule::MillepedeCalibrationModule() : CalibrationModule(),
  m_histoProcID("ProcID", DataStore::c_Persistent),
  m_histoPval("Pval", DataStore::c_Persistent),
  m_histoNormChi2("NormChi2", DataStore::c_Persistent),
  m_histoNdf("Ndf", DataStore::c_Persistent),
  m_histoRun("Run", DataStore::c_Persistent),
  m_treeResidual("ResidualMisalignment", DataStore::c_Persistent),
  m_mille("Mille", DataStore::c_Persistent)
{
  setPropertyFlags(c_ParallelProcessingCertified | c_TerminateInAllProcesses);
  // Set module properties
  setDescription("Global Belle2 calibration&alignment using GeneralBrokenLines refit and Millepede II");

  // Parameter definitions
  addParam("tracks", m_tracks, "Name of collection of genfit::Tracks for calibration", std::string(""));
  addParam("steering", m_steering, "Name of PedeSteering persistent object. Produced binary are added by the module.",
           std::string(""));
  addParam("minPvalue", m_minPvalue, "Minimum p-value to write trajectory to Mille binary", double(0.));

  // Dependecies  (empty here)
  addDefaultDependencyList();

}

void MillepedeCalibrationModule::Prepare()
{
  if (isCollector())
    StoreArray<genfit::Track>::required(m_tracks);

  if (isCalibrator())
    StoreObjPtr<PedeSteering>::required(m_steering, DataStore::c_Persistent);


  storables.setBaseName(getBaseName());
  storables.manage<MilleData>(m_mille);
  storables.manage<TH1D>(m_histoProcID, "processID", "MillepedeCalibration: ProcessID in which track was written to Mille file", 65,
                         -1., 64.);
  storables.manage<TH1D>(m_histoPval, "Pval", "MillepedeCalibration: P-value of tracks in Mille file", 100, 0., 1.);
  storables.manage<TH1D>(m_histoNormChi2, "normChi2", "MillepedeCalibration: Chi2/Ndf of tracks in Mille file", 100, 0., 10.);
  storables.manage<TH1D>(m_histoNdf, "ndf", "MillepedeCalibration: NDF of tracks in Mille file", 201, -1., 200.);
  storables.manage<TH1D>(m_histoRun, "runs", "Input runs", 100, 0, 100);
  storables.manage<TTree>(m_treeResidual, "residual_misalignment", "Residual misalignment AFTER calibration");
}


void MillepedeCalibrationModule::CollectData()
{
  // Input tracks (have to be fitted by GBL)
  StoreArray<genfit::Track> tracks(m_tracks);

  if (!m_mille->isOpen()) {
    stringstream str;
    str << m_calibration_iov;
    string iov;
    str >> iov;
    std::replace(iov.begin(), iov.end(), ',', '_');
    m_mille->open(getBaseName() + "_" + iov
                  + ".mille" + ((ProcHandler::parallelProcessingUsed()) ? to_string(ProcHandler::EvtProcID()) : ""));
  }

  std::shared_ptr<genfit::GblFitter> gbl(new genfit::GblFitter());

  StoreObjPtr<EventMetaData> emd;
  m_histoRun->get().Fill(emd->getRun());

  for (auto track : tracks) {
    if (!track.hasFitStatus())
      continue;
    genfit::GblFitStatus* fs = dynamic_cast<genfit::GblFitStatus*>(track.getFitStatus());
    if (!fs)
      continue;

    if (!fs->isFitConvergedFully())
      continue;

    if (fs->getPVal() >= m_minPvalue) {
      using namespace gbl;
      GblTrajectory trajectory(gbl->collectGblPoints(&track, track.getCardinalRep()), fs->hasCurvature());
      //trajectory.milleOut(*m_milleFile);

      m_mille->fill(trajectory);
      m_histoProcID->get().Fill(double(ProcHandler::EvtProcID()));
      m_histoProcID->get().Fill(double(ProcHandler::EvtProcID()));
      m_histoPval->get().Fill(double(fs->getPVal()));
      m_histoNormChi2->get().Fill(double(fs->getChi2() / fs->getNdf()));
      m_histoNdf->get().Fill(double(fs->getNdf()));
    }

  }
}

void MillepedeCalibrationModule::closeParallelFiles()
{
  m_mille->close();
}

void MillepedeCalibrationModule::storeData()
{
  storables.writeAll(getCalibrationFile());
}

void MillepedeCalibrationModule::resetData()
{
  storables.resetAll();
}

void MillepedeCalibrationModule::loadData()
{
  storables.readAll(getCalibrationFile());
}

CalibrationModule::ECalibrationModuleResult MillepedeCalibrationModule::Calibrate()
{
  m_mille->close();
  //if (m_histoPval->get().GetEntries() < 10)
  //  return CalibrationModule::c_NotEnoughData;

  StoreObjPtr<PedeSteering> steer(m_steering, DataStore::c_Persistent);
  // Make a copy of the steering and add our files only to it
  PedeSteering mysteer(*steer);
  vector<string> allFiles = m_mille->getFiles();
  vector<string> files;
  stringstream str;
  str << m_calibration_iov;
  string iov;
  str >> iov;
  std::replace(iov.begin(), iov.end(), ',', '_');
  string basename = getBaseName() + "_" + iov + ".mille" ;
  for (auto file : allFiles) {
    if (file.compare(0, basename.length(), basename) == 0)
      files.push_back(file);
  }
  mysteer.getFiles() = files;


  alignment::PedeApplication pede;
  pede.run(mysteer);

  B2INFO("Millepede exit code: " << pede.getExitCode());
  B2INFO(pede.getExitMessage());

  if (pede.success())
    return CalibrationModule::c_Success;
  else
    return CalibrationModule::c_Failure;
}

bool MillepedeCalibrationModule::StoreInDataBase()
{
  B2INFO("Starting StoreInDataBase for calibration module " << getName());
  VXDAlignment* alignment = new VXDAlignment();

  DBObjPtr<VXDAlignment> init;
  if (!init.isValid()) {
    alignment::PedeResult result("millepede.res");
    for (int ipar = 0; ipar < result.getNoParameters(); ipar++) {
      if (!result.isParameterDetermined(ipar)) continue;

      GlobalLabel param(result.getParameterLabel(ipar));
      if (!param.isVXD())
        continue;
      double old = 0.;
      alignment->set(param.getVxdID(), param.getParameterId() - 1, old + result.getParameterCorrection(ipar));
    }
  } else {
    alignment::PedeResult result("millepede.res");
    for (int ipar = 0; ipar < result.getNoParameters(); ipar++) {
      if (!result.isParameterDetermined(ipar)) continue;

      GlobalLabel param(result.getParameterLabel(ipar));
      if (!param.isVXD())
        continue;
      double old = init->get(param.getVxdID(), param.getParameterId() - 1);
      alignment->set(param.getVxdID(), param.getParameterId() - 1, old + result.getParameterCorrection(ipar));
    }
  }
  IntervalOfValidity storeIOV = m_calibration_iov;
  m_finalAlignment = *alignment;
  return Database::Instance().storeData("VXDAlignment", (TObject*) alignment, storeIOV);
}

CalibrationModule::ECalibrationModuleMonitoringResult MillepedeCalibrationModule::Monitor()
{
  B2INFO("Accumulated mean of Chi2/Ndf histogram: " << m_histoNormChi2->get().GetMean());
  B2INFO("                     Number of entries: " << m_histoNormChi2->get().GetEntries());

  //TODO: Hard-coded name
  DBObjPtr<VXDAlignment> misalignment("VXDMisalignment");

  if (misalignment.isValid()) {
    int layer, ladder, sensor, param;
    double x, y, z;
    double residual, error;
    m_treeResidual->get().Branch<int>("layer", &layer);
    m_treeResidual->get().Branch<int>("ladder", &ladder);
    m_treeResidual->get().Branch<int>("sensor", &sensor);
    m_treeResidual->get().Branch<int>("param", &param);
    m_treeResidual->get().Branch<double>("residual", &residual);
    m_treeResidual->get().Branch<double>("error", &error);
    m_treeResidual->get().Branch<double>("x", &x);
    m_treeResidual->get().Branch<double>("y", &y);
    m_treeResidual->get().Branch<double>("z", &z);

    for (VxdID vxdid : VXD::GeoCache::getInstance().getListOfSensors()) {
      layer = vxdid.getLayerNumber();
      ladder = vxdid.getLadderNumber();
      sensor = vxdid.getSensorNumber();
      // Center of the sensor in global coordinates
      TVector3 position = VXD::GeoCache::getInstance().get(vxdid).pointToGlobal(TVector3(0., 0., 0.));
      x = position[0];
      y = position[1];
      z = position[2];
      for (int ipar = 0; ipar < 6; ipar++) {
        param = ipar;
        residual = misalignment->get(vxdid, ipar) - m_finalAlignment.get(vxdid, ipar);

        m_treeResidual->get().Fill();
      }
    }
  }

  alignment::PedeResult result("millepede.res");
  for (int ipar = 0; ipar < result.getNoParameters(); ipar++) {
    if (!result.isParameterDetermined(ipar)) continue;
    if (fabs(result.getParameterCorrection(ipar)) > 1.e-2)
      return CalibrationModule::c_MonitoringIterateCalibration;
  }

  return CalibrationModule::c_MonitoringSuccess;
}

