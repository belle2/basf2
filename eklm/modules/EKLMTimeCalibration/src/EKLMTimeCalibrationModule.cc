/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* System headers. */
#include <fcntl.h>
#include <unistd.h>

/* External headers. */
#include <TFile.h>
#include <TTree.h>
#include <TH1F.h>
#include <TF1.h>
#include <TCanvas.h>

/* Belle2 headers. */
#include <eklm/dataobjects/EKLMHit2d.h>
#include <eklm/modules/EKLMTimeCalibration/EKLMTimeCalibrationModule.h>
#include <framework/core/ModuleManager.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Unit.h>
#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/ExtHit.h>

using namespace Belle2;

REG_MODULE(EKLMTimeCalibration)

static double CrystalBall(double* x, double* par)
{
  double d1, d2, t, f;
  if (*x < par[1]) {
    d1 = *x - par[1];
    f = exp(-0.5 * d1 * d1 / par[2] / par[2]);
  } else if (*x < par[4]) {
    d1 = *x - par[1];
    f = exp(-0.5 * d1 * d1 / par[3] / par[3]);
  } else {
    d1 = par[4] - par[1];
    d2 = *x - par[4];
    t = par[5] * par[3] * par[3] / d1;
    f = exp(-0.5 * d1 * d1 / par[3] / par[3]) *
        pow(t / (d2 + t), par[5]);
  }
  return par[0] * f;
}

static double Pol1(double* x, double* par)
{
  return *x * par[0] + par[1];
}

EKLMTimeCalibrationModule::EKLMTimeCalibrationModule() :
  calibration::CalibrationModule()
{
  setDescription("Module for EKLM calibration.");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("PerformDataCollection", m_performDataCollection,
           "Perform data collection.", true);
  addParam("PerformCalibration", m_performCalibration,
           "Read collected data and perform calibration.", true);
  addParam("DataOutputFile", m_dataOutputFileName, "Output file for data "
           "collection and input file for calibration.",
           std::string("eklm_time_calibration.root"));
  addParam("CalibrationOutputFile", m_calibrationOutputFileName,
           "Output file for calibration.", std::string("TimeCalibration.root"));
  m_outputFile = NULL;
  m_nStripDifferent = -1;
  m_Tree = NULL;
  m_ev = {0, 0};
  m_TransformData = NULL;
  m_GeoDat = NULL;
}

EKLMTimeCalibrationModule::~EKLMTimeCalibrationModule()
{
  if (m_TransformData != NULL)
    delete m_TransformData;
}

void EKLMTimeCalibrationModule::Prepare()
{
  m_GeoDat = &(EKLM::GeometryData::Instance());
  m_nStripDifferent = m_GeoDat->getNStripsDifferentLength();
  if (!m_performDataCollection)
    return;
  int i;
  char str[128];
  StoreArray<EKLMHit2d>::required();
  StoreArray<EKLMDigit>::required();
  StoreArray<Track>::required();
  StoreArray<ExtHit>::required();
  m_TransformData = new EKLM::TransformData(true, NULL);
  m_outputFile = new TFile(m_dataOutputFileName.c_str(), "recreate");
  if (m_outputFile->IsZombie())
    B2FATAL("Cannot open output file.");
  m_Tree = new TTree*[m_nStripDifferent];
  for (i = 0; i < m_nStripDifferent; i++) {
    snprintf(str, 128, "t%d", i);
    m_Tree[i] = new TTree(str, "");
    m_Tree[i]->Branch("time", &m_ev.time, "time/F");
    m_Tree[i]->Branch("dist", &m_ev.dist, "dist/F");
  }
}

void EKLMTimeCalibrationModule::CollectData()
{
  if (!m_performDataCollection)
    return;
  int i, j, k, n, n2, vol;
  double l, hitTime;
  TVector3 hitPosition;
  HepGeom::Point3D<double> hitGlobal, hitLocal;
  StoreArray<Track> tracks;
  StoreArray<EKLMHit2d> hit2ds;
  std::multimap<int, ExtHit*> mapExtHit;
  std::multimap<int, ExtHit*>::iterator it, itLower, itUpper;
  ExtHit* extHit, *entryHit[2], *exitHit[2];
  const HepGeom::Transform3D* tr;
  n = tracks.getEntries();
  for (i = 0; i < n; i++) {
    RelationVector<ExtHit> extHits = tracks[i]->getRelationsTo<ExtHit>();
    n2 = extHits.size();
    for (j = 0; j < n2; j++) {
      if (extHits[j]->getDetectorID() != Const::EDetector::KLM)
        continue;
      if (!m_GeoDat->hitInEKLM(extHits[j]->getPosition().Z()))
        continue;
      mapExtHit.insert(std::pair<int, ExtHit*>(extHits[j]->getCopyID(),
                                               extHits[j]));
    }
  }
  n = hit2ds.getEntries();
  for (i = 0; i < n; i++) {
    RelationVector<EKLMDigit> digits = hit2ds[i]->getRelationsTo<EKLMDigit>();
    if (digits.size() != 2)
      B2FATAL("Wrong number of related EKLMDigits.");
    for (j = 0; j < 2; j++) {
      entryHit[j] = NULL;
      exitHit[j] = NULL;
      vol = digits[j]->getVolumeID();
      itLower = mapExtHit.lower_bound(vol);
      itUpper = mapExtHit.upper_bound(vol);
      for (it = itLower; it != itUpper; ++it) {
        extHit = it->second;
        switch (extHit->getStatus()) {
          case EXT_ENTER:
            if (entryHit[j] == NULL) {
              entryHit[j] = extHit;
            } else {
              if (extHit->getTOF() < entryHit[j]->getTOF())
                entryHit[j] = extHit;
            }
            break;
          case EXT_EXIT:
            if (exitHit[j] == NULL) {
              exitHit[j] = extHit;
            } else {
              if (extHit->getTOF() > exitHit[j]->getTOF())
                exitHit[j] = extHit;
            }
            break;
          default:
            break;
        }
      }
    }
    if (entryHit[0] == NULL || exitHit[0] == NULL ||
        entryHit[1] == NULL || exitHit[1] == NULL)
      continue;
    for (j = 0; j < 2; j++) {
      hitTime = 0.5 * (entryHit[j]->getTOF() + exitHit[j]->getTOF());
      hitPosition = 0.5 * (entryHit[j]->getPosition() +
                           exitHit[j]->getPosition());
      l = m_GeoDat->getStripLength(digits[j]->getStrip()) / CLHEP::mm *
          Unit::mm;
      hitGlobal.setX(hitPosition.X() / Unit::mm * CLHEP::mm);
      hitGlobal.setY(hitPosition.Y() / Unit::mm * CLHEP::mm);
      hitGlobal.setZ(hitPosition.Z() / Unit::mm * CLHEP::mm);
      tr = m_TransformData->getStripGlobalToLocal(digits[j]);
      hitLocal = (*tr) * hitGlobal;
      m_ev.time = digits[j]->getTime() - hitTime;
      m_ev.dist = 0.5 * l - hitLocal.x() / CLHEP::mm * Unit::mm;
      k = m_GeoDat->getStripLengthIndex(digits[j]->getStrip() - 1);
      m_Tree[k]->Fill();
    }
  }
}

void EKLMTimeCalibrationModule::closeParallelFiles()
{
}

calibration::CalibrationModule::ECalibrationModuleResult
EKLMTimeCalibrationModule::Calibrate()
{
  int i;
  if (m_performDataCollection) {
    m_outputFile->cd();
    for (i = 0; i < m_nStripDifferent; i++) {
      m_Tree[i]->Write();
      delete m_Tree[i];
    }
    delete[] m_Tree;
    delete m_outputFile;
  }
  if (m_performCalibration) {
    TFile* fIn = new TFile(m_dataOutputFileName.c_str());
    TFile* fOut = new TFile(m_calibrationOutputFileName.c_str(), "recreate");
    TTree* tOut = new TTree("t_calibration", "recreate");
    const int maxbins = 40;
    const int nbins = 25;
    int i, j, n2, bin;
    float mint[maxbins], time, dist, *timeArray, *distArray;
    float p0, p1, maxval;
    char str[128];
    double len;
    TH1F* h[50], *h2[maxbins], *h3;
    TF1* fcn, *fcn2;
    TTree* t2;
    TCanvas* c1 = new TCanvas();
    tOut->Branch("p0", &p0, "p0/F");
    tOut->Branch("p1", &p1, "p1/F");
    fcn = new TF1("fcn", CrystalBall, 0, 10, 6);
    fcn2 = new TF1("fcn2", Pol1, 0, 10, 2);
    for (i = 0; i < m_nStripDifferent; i++) {
      len = m_GeoDat->getStripLength(m_GeoDat->getStripLengthIndex(i) + 1);
      for (j = 0; j < nbins; j++) {
        mint[j] = 200;
      }
      h[i] = new TH1F("h", "", nbins, 0, len);
      snprintf(str, 128, "t%d", i);
      t2 = (TTree*)fIn->Get(str);
      t2->SetBranchAddress("time", &time);
      t2->SetBranchAddress("dist", &dist);
      n2 = t2->GetEntries();
      timeArray = new float[n2];
      distArray = new float[n2];
      for (j = 0; j < n2; j++) {
        t2->GetEntry(j);
        timeArray[j] = time;
        distArray[j] = dist;
      }
      for (j = 0; j < n2; j++) {
        if (distArray[j] >= len || distArray[j] < 0)
          continue;
        bin = floor(distArray[j] / len * nbins);
        if (mint[bin] > timeArray[j])
          mint[bin] = timeArray[j];
      }
      for (j = 0; j < nbins; j++) {
        snprintf(str, 128, "h2_%d", j);
        h2[j] = new TH1F(str, "", 100, mint[j], mint[j] + 10);
      }
      for (j = 0; j < n2; j++) {
        if (distArray[j] >= len || distArray[j] < 0)
          continue;
        bin = floor(distArray[j] / len * nbins);
        h2[bin]->Fill(timeArray[j]);
      }
      for (j = 0; j < nbins; j++) {
        fcn->SetParameter(0, h2[j]->Integral());
        fcn->SetParameter(1, h2[j]->GetMean());
        fcn->SetParameter(2, h2[j]->GetRMS());
        fcn->SetParameter(3, h2[j]->GetRMS());
        fcn->FixParameter(4, h2[j]->GetMean() + 1.0);
        fcn->FixParameter(5, 1.0);
        h2[j]->Fit("fcn");
        fcn->ReleaseParameter(4);
        fcn->ReleaseParameter(5);
        h2[j]->Fit("fcn");
        h[i]->SetBinContent(j + 1, fcn->GetParameter(1));
        h[i]->SetBinError(j + 1, fcn->GetParError(1));
        h2[j]->Draw();
        c1->Print("fit1.eps");
      }
      h[i]->Fit("fcn2");
      h[i]->Draw();
      c1->Print("fit2.eps");
      p0 = fcn2->GetParameter(0);
      /*
       * Shift of the most probable value is corrected from
       * the distribution for the entire strip.
       */
      h3 = new TH1F("h3", "", 130, -3., 10.);
      for (j = 0; j < n2; j++) {
        if (distArray[j] >= len || distArray[j] < 0)
          continue;
        h3->Fill(timeArray[j] - distArray[j] * p0);
      }
      maxval = -1;
      for (j = 1; j <= 130; j++) {
        if (h3->GetBinContent(j) > maxval) {
          maxval = h3->GetBinContent(j);
          p1 =  h3->GetBinCenter(j);
        }
      }
      tOut->Fill();
      delete timeArray;
      delete distArray;
      snprintf(str, 128, "time - (%g * dist + %g)", p0, p1);
      t2->Draw(str, (std::string(str) + "< 10").c_str());
      snprintf(str, 128, "corrtime%d.eps", i);
      c1->Print(str);
    }
    fOut->cd();
    tOut->Write();
    delete tOut;
    delete fOut;
    delete fIn;
  }
  return calibration::CalibrationModule::c_Success;
}

calibration::CalibrationModule::ECalibrationModuleMonitoringResult
EKLMTimeCalibrationModule::Monitor()
{
  return calibration::CalibrationModule::c_MonitoringSuccess;
}

bool EKLMTimeCalibrationModule::StoreInDataBase()
{
  return true;
}

