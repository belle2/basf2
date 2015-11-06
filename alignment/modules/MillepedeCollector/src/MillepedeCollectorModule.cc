/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: tadeas                                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <alignment/modules/MillepedeCollector/MillepedeCollectorModule.h>

#include <framework/datastore/StoreArray.h>
#include <framework/pcore/ProcHandler.h>
#include <framework/core/FileCatalog.h>

#include <alignment/dataobjects/MilleData.h>

#include <genfit/Track.h>
#include <genfit/GblFitter.h>


using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(MillepedeCollector)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

MillepedeCollectorModule::MillepedeCollectorModule() : CalibrationCollectorModule()
{
  setPropertyFlags(c_ParallelProcessingCertified);
  setDescription("Calibration data collector for Millepede Algorithm");

  addParam("tracks", m_tracks, "Name of collection of genfit::Tracks for calibration", std::string(""));
}

void MillepedeCollectorModule::prepare()
{
  StoreObjPtr<EventMetaData>::required();
  StoreArray<genfit::Track>::required(m_tracks);

  // Register Mille output
  registerObject<MilleData>("mille", new MilleData());

  registerObject<TH1F>("chi2/ndf", new TH1F("chi2/ndf", "chi2/ndf", 200, 0., 50.));
  registerObject<TH1F>("pval", new TH1F("pval", "pval", 100, 0., 1.));
}

void MillepedeCollectorModule::collect()
{
  // Input tracks (have to be fitted by GBL)
  StoreArray<genfit::Track> tracks(m_tracks);

  // Open new file on request (at start or after being closed)
  auto& mille = getObject<MilleData>("mille");
  if (!mille.isOpen())
    mille.open(getUniqueMilleName());

  std::shared_ptr<genfit::GblFitter> gbl(new genfit::GblFitter());

  for (auto track : tracks) {
    if (!track.hasFitStatus())
      continue;
    genfit::GblFitStatus* fs = dynamic_cast<genfit::GblFitStatus*>(track.getFitStatus());
    if (!fs)
      continue;

    if (!fs->isFitConvergedFully())
      continue;

    getObject<TH1F>("chi2/ndf").Fill(fs->getChi2() / fs->getNdf());
    getObject<TH1F>("pval").Fill(fs->getPVal());

    using namespace gbl;
    GblTrajectory trajectory(gbl->collectGblPoints(&track, track.getCardinalRep()), fs->hasCurvature());
    mille.fill(trajectory);
  }
}

void MillepedeCollectorModule::endRun()
{
  // We close the file at end of run, producing
  // one file per run (and process id) which is more
  // convenient than one large binary block.
  auto& mille = getObject<MilleData>("mille");
  if (mille.isOpen())
    mille.close();
}

std::string MillepedeCollectorModule::getUniqueMilleName()
{
  StoreObjPtr<EventMetaData> emd;
  string name = getName();

  name += "-e"   + to_string(emd->getExperiment());
  name += "-r"   + to_string(emd->getRun());
  name += "-ev"  + to_string(emd->getEvent());

  if (ProcHandler::parallelProcessingUsed())
    name += "-pid" + to_string(ProcHandler::EvtProcID());

  name += ".mille";

  return name;
}


