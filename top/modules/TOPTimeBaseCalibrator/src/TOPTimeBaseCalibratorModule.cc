/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <top/modules/TOPTimeBaseCalibrator/TOPTimeBaseCalibratorModule.h>
#include <top/geometry/TOPGeometryPar.h>

// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

// DataStore classes
#include <framework/dataobjects/EventMetaData.h>
#include <top/dataobjects/TOPDigit.h>
#include <top/dbobjects/TOPSampleTimes.h>

// Root
#include "TFile.h"
#include "TProfile.h"
#include "TH1F.h"
#include "TH2F.h"
#include <TSystem.h>
#include <TMatrixDSym.h>

#include <iostream>
#include <sstream>

using namespace std;

namespace Belle2 {

  using namespace TOP;

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(TOPTimeBaseCalibrator)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  TOPTimeBaseCalibratorModule::TOPTimeBaseCalibratorModule() : Module()

  {
    // set module description
    setDescription("Sample time calibrator");
    //    setPropertyFlags(c_ParallelProcessingCertified | c_InitializeInProcess);

    // Add parameters
    addParam("moduleID", m_moduleID, "slot ID to calibrate.");
    addParam("minTimeDiff", m_minTimeDiff,
             "lower bound on time difference [samples].", 0.0);
    addParam("maxTimeDiff", m_maxTimeDiff,
             "upper bound on time difference [samples].", 100.0);
    addParam("directoryName", m_directoryName,
             "name (with path) of the directory for the output root files.",
             string(""));
    addParam("minHits", m_minHits, "minimal required hits per channel.", (unsigned) 1000);
    addParam("method", m_method, "method: 0 - profile histograms only, "
             "1 - matrix inversion, 2 - iterative, "
             "3 - matrix inversion w/ singular value decomposition.", (unsigned) 1);

  }

  TOPTimeBaseCalibratorModule::~TOPTimeBaseCalibratorModule()
  {
  }

  void TOPTimeBaseCalibratorModule::initialize()
  {

    // input
    StoreArray<TOPDigit>::required();

    // checks
    const auto* geo = TOPGeometryPar::Instance()->getGeometry();
    if (!geo->isModuleIDValid(m_moduleID))
      B2ERROR("Invalid module ID: " << m_moduleID);

    // check for existance and mkdir if not
    if (m_directoryName.empty()) m_directoryName = "./";
    if (m_directoryName != "./") gSystem->mkdir(m_directoryName.c_str(), kTRUE);

    // synchronization time corresponding to two ASIC windows
    m_syncTimeBase = geo->getNominalTDC().getSyncTimeBase();

  }


  void TOPTimeBaseCalibratorModule::beginRun()
  {
  }

  void TOPTimeBaseCalibratorModule::event()
  {
    const auto* geo = TOPGeometryPar::Instance()->getGeometry();
    double sampleDivisions = (0x1 << geo->getNominalTDC().getSubBits());
    double sampleWidth = geo->getNominalTDC().getSampleWidth();

    std::vector<std::pair<double, double> > hits[c_NumChannels];

    StoreArray<TOPDigit> digits;
    for (const auto& digit : digits) {
      if (digit.getModuleID() != m_moduleID) continue;
      if (digit.getHitQuality() != TOPDigit::c_CalPulse) continue;
      double t = digit.getTDC() / sampleDivisions + digit.getFirstWindow() * c_WindowSize;
      if (t < 0) {
        B2ERROR("Got negative sample number - digit ignored");
        continue;
      }
      double et = digit.getTimeError() / sampleWidth;
      if (et <= 0) {
        B2ERROR("Time error is not given - digit ignored");
        continue;
      }
      // add quantization error
      et = sqrt(et * et + 1 / sampleDivisions / sampleDivisions / 12);
      unsigned channel = digit.getChannel();
      if (channel < c_NumChannels) hits[channel].push_back(std::make_pair(t, et));
    }

    for (unsigned channel = 0; channel < c_NumChannels; channel++) {
      const auto& channelHits = hits[channel];
      if (channelHits.size() == 2) {
        double t0 = channelHits[0].first;
        double t1 = channelHits[1].first;
        auto diff = fabs(t0 - t1); // since not sorted yet
        if (diff < m_minTimeDiff) continue;
        if (diff > m_maxTimeDiff) continue;
        double sig0 = channelHits[0].second;
        double sig1 = channelHits[1].second;
        double sigma = sqrt(sig0 * sig0 + sig1 * sig1);
        m_ntuples[channel].push_back(TwoTimes(t0, t1, sigma));
      } else if (channelHits.size() > 2) {
        B2WARNING("More than two cal pulses per channel found - ignored");
      }
    }

  }


  void TOPTimeBaseCalibratorModule::endRun()
  {
  }

  void TOPTimeBaseCalibratorModule::terminate()
  {

    const auto& feMapper = TOPGeometryPar::Instance()->getFrontEndMapper();

    for (unsigned bs = 0; bs < c_NumBoardstacks; bs ++) {

      // determine scrod ID

      const auto* feMap = feMapper.getMap(m_moduleID, bs);
      if (!feMap) {
        B2ERROR("No front-end map available for slot " << m_moduleID << ", BS" << bs);
        continue;
      }
      unsigned scrodID = feMap->getScrodID();

      // open output root file

      std::string fileName = m_directoryName + "/tbcScrod" +
                             std::to_string(scrodID) + ".root";
      TFile* fout = TFile::Open(fileName.c_str(), "recreate");
      if (!fout) {
        B2ERROR("Can't open the output file " << fileName);
        continue;
      }

      B2INFO("Fitting time base corrections for SCROD " << scrodID
             << ", output file: " << fileName);

      // book histograms

      TH1F Hchan("channels", "channels with cal pulse",
                 c_NumScrodChannels, 0, c_NumScrodChannels);
      Hchan.SetXTitle("channel number");
      Hchan.SetYTitle("double cal pulse counts");
      TH1F Hsuccess("success", "successfuly fitted channels",
                    c_NumScrodChannels, 0, c_NumScrodChannels);
      Hsuccess.SetXTitle("channel number");
      TH1F Hchi2("chi2", "normalized chi2",
                 c_NumScrodChannels, 0, c_NumScrodChannels);
      Hchi2.SetXTitle("channel number");
      Hchi2.SetYTitle("chi2/ndf");
      TH1F Hndf("ndf", "degrees of freedom",
                c_NumScrodChannels, 0, c_NumScrodChannels);
      Hndf.SetXTitle("channel number");
      Hndf.SetYTitle("ndf");
      TH1F HDeltaT("DeltaT", "Fitted double pulse delay time",
                   c_NumScrodChannels, 0, c_NumScrodChannels);
      HDeltaT.SetXTitle("channel number");
      HDeltaT.SetYTitle("#Delta T [ns]");

      // loop over channels of a single SCROD

      for (unsigned chan = 0; chan < c_NumScrodChannels; chan++) {
        unsigned channel = chan + bs * c_NumScrodChannels;
        auto& ntuple = m_ntuples[channel];
        Hchan.SetBinContent(chan + 1, ntuple.size());
        if (ntuple.size() > m_minHits) {
          bool ok = calibrateChannel(ntuple, scrodID, chan, Hchi2, Hndf, HDeltaT);
          if (ok) Hsuccess.Fill(chan);
        } else {
          B2INFO("... channel " << chan << " statistics too low");
        }
      }

      // write histograms and close the file

      Hchan.Write();
      Hsuccess.Write();
      Hchi2.Write();
      Hndf.Write();
      HDeltaT.Write();
      fout->Close();

    }
  }


  bool TOPTimeBaseCalibratorModule::calibrateChannel(std::vector<TwoTimes>& ntuple,
                                                     unsigned scrodID, unsigned chan,
                                                     TH1F& Hchi2, TH1F& Hndf,
                                                     TH1F& HDeltaT)
  {

    // determine outlayer removal cuts

    std::string name = "timeDiff_ch" + to_string(chan);
    std::string forWhat = "scrod " + to_string(scrodID) + " channel " + to_string(chan);
    std::string title = "Cal pulse time difference vs. sample for " + forWhat;
    TProfile prof(name.c_str(), title.c_str(), c_TimeAxisSize, 0, c_TimeAxisSize,
                  m_minTimeDiff, m_maxTimeDiff, "S");
    prof.SetXTitle("sample number");
    prof.SetYTitle("time difference [samples]");

    std::vector<double> profMeans(c_TimeAxisSize, (m_maxTimeDiff + m_minTimeDiff) / 2);
    double sigma = (m_maxTimeDiff - m_minTimeDiff) / 6;

    for (int iter = 0; iter < 5; iter++) {
      prof.Reset();
      double x = 0;
      double x2 = 0;
      int n = 0;
      for (auto& twoTimes : ntuple) {
        int sample = int(twoTimes.t1) % c_TimeAxisSize;
        double diff = twoTimes.t2 - twoTimes.t1;
        double Ddiff = diff - profMeans[sample];
        if (fabs(Ddiff) < 3 * sigma) {
          prof.Fill(sample, diff);
          x += Ddiff;
          x2 += Ddiff * Ddiff;
          n++;
          twoTimes.good = true;
        } else {
          twoTimes.good = false;
        }
      }
      for (int i = 0; i < c_TimeAxisSize; i++) {
        profMeans[i] = prof.GetBinContent(i + 1);
      }
      if (n == 0) return false;
      x2 /= n;
      x /= n;
      sigma = sqrt(x2 - x * x);
    }
    prof.Write();

    // calculate average time difference

    double meanTimeDifference = 0;
    for (auto& x : profMeans) meanTimeDifference += x;
    meanTimeDifference /= profMeans.size();
    meanTimeDifference -= int(meanTimeDifference / c_TimeAxisSize) * c_TimeAxisSize;

    // perform the fit

    switch (m_method) {
      case 0:
        return false;
      case 1:
        return matrixInversion(ntuple, scrodID, chan, meanTimeDifference,
                               Hchi2, Hndf, HDeltaT);
      case 2:
        B2ERROR("Iterative method not implemented yet");
        return false;
      case 3:
        B2ERROR("Singuler value decomposition not implemented yet");
        return false;
      default:
        B2ERROR("Unknown method " << m_method);
        return false;
    }

  }


  bool TOPTimeBaseCalibratorModule::matrixInversion(const std::vector<TwoTimes>& ntuple,
                                                    unsigned scrodID, unsigned chan,
                                                    double meanTimeDifference,
                                                    TH1F& Hchi2, TH1F& Hndf,
                                                    TH1F& HDeltaT)
  {

    // Ax = b: construct matrix A and right side vector b

    TMatrixDSym A(c_TimeAxisSize);
    std::vector<double> b(c_TimeAxisSize, 0.0);

    for (const auto& twoTimes : ntuple) {
      if (!twoTimes.good) continue;

      std::vector<double> m(c_TimeAxisSize, 0.0);
      int i1 = int(twoTimes.t1);
      m[i1 % c_TimeAxisSize] = 1.0 - (twoTimes.t1 - i1);
      int i2 = int(twoTimes.t2);
      m[i2 % c_TimeAxisSize] = twoTimes.t2 - i2;
      i2 = i1 + (i2 - i1) % c_TimeAxisSize;
      for (int k = i1 + 1; k < i2; k++) m[k % c_TimeAxisSize] = 1;

      double relSigma = twoTimes.sigma / meanTimeDifference;
      double sig2 = relSigma * relSigma;

      for (int jj = i1; jj < i2 + 1; jj++) {
        int j = jj % c_TimeAxisSize;
        for (int kk = i1; kk < i2 + 1; kk++) {
          int k = kk % c_TimeAxisSize;
          A(j, k) += m[j] * m[k] / sig2;
        }
      }
      for (int k = 0; k < c_TimeAxisSize; k++) b[k] += m[k] / sig2;
    }

    // save as histograms

    std::string forWhat = "scrod " + to_string(scrodID) + " channel " + to_string(chan);
    saveAsHistogram(A, "matA_ch" + to_string(chan), "Matrix for " + forWhat);
    saveAsHistogram(b, "vecB_ch" + to_string(chan), "Right side for " + forWhat);

    // invert matrix A and solve the equation: x = A^{-1}b

    double det = 0;
    A.Invert(&det);
    if (det == 0) {
      B2INFO("... channel " << chan << " failed");
      return false;
    }

    std::vector<double> x(c_TimeAxisSize, 0.0);
    for (int k = 0; k < c_TimeAxisSize; k++) {
      for (int j = 0; j < c_TimeAxisSize; j++) {
        x[k] += A(k, j) * b[j];
      }
    }

    // calculate chi^2

    double chi2 = 0;
    int ndf = -c_TimeAxisSize;

    for (const auto& twoTimes : ntuple) {
      if (!twoTimes.good) continue;

      std::vector<double> m(c_TimeAxisSize, 0.0);
      int i1 = int(twoTimes.t1);
      m[i1 % c_TimeAxisSize] = 1.0 - (twoTimes.t1 - i1);
      int i2 = int(twoTimes.t2);
      m[i2 % c_TimeAxisSize] = twoTimes.t2 - i2;
      i2 = i1 + (i2 - i1) % c_TimeAxisSize;
      for (int k = i1 + 1; k < i2; k++) m[k % c_TimeAxisSize] = 1;
      double s = -1.0;
      for (int k = 0; k < c_TimeAxisSize; k++) s += m[k] * x[k];
      double relSigma = twoTimes.sigma / meanTimeDifference;
      double sig2 = relSigma * relSigma;
      chi2 += s * s / sig2;
      ndf++;
    }
    Hchi2.SetBinContent(chan + 1, chi2 / ndf);
    Hndf.SetBinContent(chan + 1, ndf);

    // constrain sum of x to 2*syncTimeBase and calculate sample times

    double sum = 0;
    for (auto xi : x) sum += xi;
    if (sum == 0) {
      B2ERROR("sum == 0");
      return false;
    }
    double DeltaT = 2 * m_syncTimeBase / sum;
    for (auto& xi : x) xi *= DeltaT;
    HDeltaT.SetBinContent(chan + 1, DeltaT);

    std::vector<double> err;
    for (int k = 0; k < c_TimeAxisSize; k++) err.push_back(sqrt(A(k, k)) * DeltaT);

    std::vector<double> sampleTimes;
    sampleTimes.push_back(0);
    for (auto xi : x) sampleTimes.push_back(xi + sampleTimes.back());

    // save results as histograms

    saveAsHistogram(A, "invA_ch" + to_string(chan), "Inverted matrix for " + forWhat);
    saveAsHistogram(x, err, "dt_ch" + to_string(chan), "Sample time bins for " + forWhat,
                    "sample number", "#Delta t [ns]");
    saveAsHistogram(sampleTimes, "sampleTimes_ch" + to_string(chan),
                    "Time base corrections for " + forWhat, "sample number", "t [ns]");

    // calibrated cal pulse time difference

    std::string name = "timeDiffcal_ch" + to_string(chan);
    std::string title = "Calibrated cal pulse time difference vs. sample for " + forWhat;
    TH2F Hcor(name.c_str(), title.c_str(), c_TimeAxisSize, 0, c_TimeAxisSize,
              100, DeltaT - 0.5, DeltaT + 0.5);
    Hcor.SetXTitle("sample number");
    Hcor.SetYTitle("time difference [ns]");
    Hcor.SetStats(kTRUE);

    TOPSampleTimes timeBase;
    timeBase.setTimeAxis(sampleTimes, sampleTimes.back() / 2);

    for (const auto& twoTimes : ntuple) {
      if (!twoTimes.good) continue;
      double dt = timeBase.getDeltaTime(0, twoTimes.t2, twoTimes.t1);
      int sample = int(twoTimes.t1) % c_TimeAxisSize;
      Hcor.Fill(sample, dt);
    }
    Hcor.Write();

    B2INFO("... channel " << chan << " OK (chi^2/ndf = " << chi2 / ndf
           << ", ndf = " << ndf << ")");

    return true;
  }


  void TOPTimeBaseCalibratorModule::saveAsHistogram(const std::vector<double>& vec,
                                                    const std::string& name,
                                                    const std::string& title,
                                                    const std::string& xTitle,
                                                    const std::string& yTitle) const
  {
    if (vec.empty()) return;

    TH1F h(name.c_str(), title.c_str(), vec.size(), 0, vec.size());
    h.SetXTitle(xTitle.c_str());
    h.SetYTitle(yTitle.c_str());
    if (name.find("Fit") != string::npos) h.SetLineColor(2);

    for (unsigned i = 0; i < vec.size(); i++) h.SetBinContent(i + 1, vec[i]);

    h.Write();
  }


  void TOPTimeBaseCalibratorModule::saveAsHistogram(const std::vector<double>& vec,
                                                    const std::vector<double>& err,
                                                    const std::string& name,
                                                    const std::string& title,
                                                    const std::string& xTitle,
                                                    const std::string& yTitle) const
  {
    if (vec.empty()) return;

    TH1F h(name.c_str(), title.c_str(), vec.size(), 0, vec.size());
    h.SetXTitle(xTitle.c_str());
    h.SetYTitle(yTitle.c_str());

    for (unsigned i = 0; i < vec.size(); i++) h.SetBinContent(i + 1, vec[i]);
    for (unsigned i = 0; i < err.size(); i++) h.SetBinError(i + 1, err[i]);

    h.Write();
  }


  void TOPTimeBaseCalibratorModule::saveAsHistogram(const TMatrixDSym& M,
                                                    const std::string& name,
                                                    const std::string& title) const
  {
    int n = M.GetNrows();
    TH2F h(name.c_str(), title.c_str(), n, 0, n, n, 0, n);
    h.SetXTitle("columns");
    h.SetYTitle("rows");

    for (int j = 0; j < n; j++) {
      for (int k = 0; k < n; k++) {
        h.SetBinContent(j + 1, n - k, M(j, k));
      }
    }

    h.Write();
  }



} // end Belle2 namespace

