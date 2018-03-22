/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric, Hiromichi Kichimi and Xiaolong Wang        *
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
#include <top/dataobjects/TOPRawDigit.h>
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
    addParam("minHits", m_minHits,
             "minimal required hits per channel.", (unsigned) 1000);
    addParam("numIterations", m_numIterations,
             "number of iTBC iterations", (unsigned) 100);
    addParam("numConvIter", m_conv_iter,
             "Max number of iTBC iterations for conversion.", (unsigned) 50);
    addParam("minChi2Change", m_dchi2_min,
             "Minimal of chi2 change in iterations.",  0.2);
    addParam("dtMin", m_dt_min,
             "minimum Delta T of raw calpulse in iTBC",  20.0);
    addParam("dtMax", m_dt_max,
             "maximum Delta T of raw calpulse in iTBC",  24.0);
    addParam("xStep", m_xstep,
             "unit for an interation of delta(X_s)",  0.020);
    addParam("devStep", m_dev_step,
             "a step size to calculate the value of d(chisq)/dxval",  0.001);
    addParam("chgStep", m_change_xstep,
             "update m_xstep if m_dchi2dxv < m_change_step",  0.015);
    addParam("newStep", m_new_xstep,
             "a new step for delta(X_s) if m_dchi2dxv < m_change_step",  2.0 * m_xstep);
    addParam("sigm2_exp", m_sigm2_exp,
             "(sigma_0(dT))**2 for nomarlization of chisq = sum{dT^2/sigma^2}",  0.0424 * 0.0424);

    addParam("method", m_method, "method: 0 - profile histograms only, "
             "1 - matrix inversion, 2 - iterative, "
             "3 - matrix inversion w/ singular value decomposition.", (unsigned) 1);
    addParam("useFallingEdge", m_useFallingEdge,
             "if true, use cal pulse falling edge instead of rising edge", false);

  }

  TOPTimeBaseCalibratorModule::~TOPTimeBaseCalibratorModule()
  {
  }

  void TOPTimeBaseCalibratorModule::initialize()
  {

    // input
    m_digits.isRequired();

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
    double sampleWidth = geo->getNominalTDC().getSampleWidth();

    vector<pair<double, double> > hits[c_NumChannels];

    for (const auto& digit : m_digits) {
      if (digit.getModuleID() != m_moduleID) continue;
      if (digit.getHitQuality() != TOPDigit::c_CalPulse) continue;
      double rawTime = digit.getRawTime();
      double errScaleFactor = 1;
      if (m_useFallingEdge) {
        const auto* rawDigit = digit.getRelated<TOPRawDigit>();
        if (!rawDigit) {
          B2ERROR("No relation to TOPRawDigit - can't determine falling edge time error");
          continue;
        }
        // rawTime may include corrections due to window number discontinuity,
        // therefore one must add the width and not just use getCFDFallingTime()
        rawTime += rawDigit->getFWHM();
        errScaleFactor = rawDigit->getCFDFallingTimeError(1.0) / rawDigit->getCFDLeadingTimeError(1.0);
      }
      double t = rawTime + digit.getFirstWindow() * c_WindowSize;
      if (t < 0) {
        B2ERROR("Got negative sample number - digit ignored");
        continue;
      }
      double et = digit.getTimeError() / sampleWidth * errScaleFactor;
      if (et <= 0) {
        B2ERROR("Time error is not given - digit ignored");
        continue;
      }
      unsigned channel = digit.getChannel();
      if (channel < c_NumChannels) hits[channel].push_back(make_pair(t, et));
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

      string fileName = m_directoryName + "/tbcSlot";
      if (m_moduleID < 10) {
        fileName += "0" + to_string(m_moduleID);
      } else {
        fileName += to_string(m_moduleID);
      }
      fileName += "_" + to_string(bs) + "-scrod" +
                  to_string(scrodID) + ".root";
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
          B2INFO("... channel " << chan << " statistics too low ("
                 << ntuple.size() << " double cal pulses)");
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


  bool TOPTimeBaseCalibratorModule::calibrateChannel(vector<TwoTimes>& ntuple,
                                                     unsigned scrodID, unsigned chan,
                                                     TH1F& Hchi2, TH1F& Hndf,
                                                     TH1F& HDeltaT)
  {

    // determine outlayer removal cuts

    string name = "timeDiff_ch" + to_string(chan);
    string forWhat = "scrod " + to_string(scrodID) + " channel " + to_string(chan);
    string title = "Cal pulse time difference vs. sample for " + forWhat;
    TProfile prof(name.c_str(), title.c_str(), c_TimeAxisSize, 0, c_TimeAxisSize,
                  m_minTimeDiff, m_maxTimeDiff, "S");
    prof.SetXTitle("sample number");
    prof.SetYTitle("time difference [samples]");

    name = "sampleOccup_ch" + to_string(chan);
    title = "Occupancy for " + forWhat;
    TH1F hist(name.c_str(), title.c_str(), c_TimeAxisSize, 0, c_TimeAxisSize);
    hist.SetXTitle("sample number");
    hist.SetYTitle("entries per sample");

    vector<double> profMeans(c_TimeAxisSize, (m_maxTimeDiff + m_minTimeDiff) / 2);
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
          hist.Fill(sample);
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
    hist.Write();

    // calculate average time difference

    double meanTimeDifference = 0;
    for (auto& x : profMeans) meanTimeDifference += x;
    meanTimeDifference /= profMeans.size();
    meanTimeDifference -= int(meanTimeDifference / c_TimeAxisSize) * c_TimeAxisSize;

    // perform the fit

    switch (m_method) {
      case 0:
        B2INFO("... channel " << chan << ": "
               << ntuple.size() << " double cal pulses)");
        return false;
      case 1:
        return matrixInversion(ntuple, scrodID, chan, meanTimeDifference,
                               Hchi2, Hndf, HDeltaT);
      case 2:
        return iterativeTBC(ntuple, scrodID, chan, meanTimeDifference,
                            Hchi2, Hndf, HDeltaT);
      case 3:
        B2ERROR("Singuler value decomposition not implemented yet");
        return false;
      default:
        B2ERROR("Unknown method " << m_method);
        return false;
    }

  }


  bool TOPTimeBaseCalibratorModule::matrixInversion(const vector<TwoTimes>& ntuple,
                                                    unsigned scrodID, unsigned chan,
                                                    double meanTimeDifference,
                                                    TH1F& Hchi2, TH1F& Hndf,
                                                    TH1F& HDeltaT)
  {

    // Ax = b: construct matrix A and right side vector b

    TMatrixDSym A(c_TimeAxisSize);
    vector<double> b(c_TimeAxisSize, 0.0);

    for (const auto& twoTimes : ntuple) {
      if (!twoTimes.good) continue;

      vector<double> m(c_TimeAxisSize, 0.0);
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

    string forWhat = "scrod " + to_string(scrodID) + " channel " + to_string(chan);
    saveAsHistogram(A, "matA_ch" + to_string(chan), "Matrix for " + forWhat);
    saveAsHistogram(b, "vecB_ch" + to_string(chan), "Right side for " + forWhat);

    // invert matrix A and solve the equation: x = A^{-1}b

    double det = 0;
    A.Invert(&det);
    if (det == 0) {
      B2INFO("... channel " << chan << " failed");
      return false;
    }

    vector<double> x(c_TimeAxisSize, 0.0);
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

      vector<double> m(c_TimeAxisSize, 0.0);
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

    vector<double> err;
    for (int k = 0; k < c_TimeAxisSize; k++) err.push_back(sqrt(A(k, k)) * DeltaT);

    vector<double> sampleTimes;
    sampleTimes.push_back(0);
    for (auto xi : x) sampleTimes.push_back(xi + sampleTimes.back());

    // save results as histograms

    saveAsHistogram(A, "invA_ch" + to_string(chan), "Inverted matrix for " + forWhat);
    saveAsHistogram(x, err, "dt_ch" + to_string(chan), "Sample time bins for " + forWhat,
                    "sample number", "#Delta t [ns]");
    saveAsHistogram(sampleTimes, "sampleTimes_ch" + to_string(chan),
                    "Time base corrections for " + forWhat, "sample number", "t [ns]");

    // calibrated cal pulse time difference

    string name = "timeDiffcal_ch" + to_string(chan);
    string title = "Calibrated cal pulse time difference vs. sample for " + forWhat;
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


  bool TOPTimeBaseCalibratorModule::iterativeTBC(const std::vector<TwoTimes>& ntuple,
                                                 unsigned scrodID, unsigned chan,
                                                 double meanTimeDifference,
                                                 TH1F& Hchi2, TH1F& Hndf,
                                                 TH1F& HDeltaT)
  {
    std::vector<double> xval(c_TimeAxisSize + 1, 0.0);
    double wx = 2 * m_syncTimeBase / c_TimeAxisSize;
    for (int i = 0; i < c_TimeAxisSize + 1; i++) xval[i] = i * wx;

    B2INFO("TimeBaseCalibration starts for channel#" << chan);

    double pre_chi2 = 10000000.0;
    unsigned   num_small_dev = 0;

    for (unsigned j = 0; j < m_numIterations; j++) {

      Iteration(ntuple, xval);
      double this_chi2 = Chisq(ntuple, xval);
      if (this_chi2 < 0)continue;
      double deltaChi2 = pre_chi2 - this_chi2;
      if (deltaChi2 < -m_dchi2_min) break;
      if (fabs(deltaChi2) < m_deltamin) num_small_dev++;
      if (num_small_dev > m_conv_iter) break;
      pre_chi2 = this_chi2;
    }

    // calculate chi^2

    double chi2 = Chisq(ntuple, xval);
    Hchi2.SetBinContent(chan + 1, chi2);
    Hndf.SetBinContent(chan + 1, m_good);

    // constrain sum of x to 2*syncTimeBase and calculate sample times, not necessary here

    double sum = 0;
    for (auto xi : xval) sum += xi;
    if (sum == 0) {
      B2ERROR("sum == 0");
      return false;
    }

    double DeltaT = meanTimeDifference * (2 * m_syncTimeBase / c_TimeAxisSize);
    HDeltaT.SetBinContent(chan + 1, DeltaT);

    std::vector<double> timeInterval;
    for (int i = 0; i < c_TimeAxisSize; i++)timeInterval.push_back(xval[i + 1] - xval[i]);


    std::vector<double> sampleTimes;
    for (auto xi : xval) sampleTimes.push_back(xi);

    // save results as histograms
    std::string forWhat = "scrod " + to_string(scrodID) + " channel " + to_string(chan);
    saveAsHistogram(timeInterval,  "dt_ch" + to_string(chan), "Sample time bins for " + forWhat,
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

    B2INFO("... channel " << chan << " OK (chi^2/ndf = " << chi2
           << ", ndf = " << m_good << ")");

    return true;
  }

  void TOPTimeBaseCalibratorModule::Iteration(const std::vector<TwoTimes>& ntuple, std::vector<double>& xval)
  {
    for (int i = 0; i < c_TimeAxisSize; i++) {
      double wdth = xval[i + 1] - xval[i];
      if (wdth < m_min_binwidth) {
        xval[i] = xval[i] - 0.5 * fabs(wdth) - 0.5 * m_min_binwidth;
        xval[i + 1] = xval[i + 1] + 0.5 * fabs(wdth) + 0.5 * m_min_binwidth;
      }
      if (wdth > m_max_binwidth) {
        xval[i] = xval[i] - 0.5 * fabs(wdth) - 0.5 * m_max_binwidth;
        xval[i + 1] = xval[i + 1] + 0.5 * fabs(wdth) + 0.5 * m_max_binwidth;
      }
    }

    if (xval[0] != 0)
      for (int i = 0; i < c_TimeAxisSize; i++)  xval[i] = xval[i] - xval[0];

    std::vector<double> xxval(c_TimeAxisSize + 1, 0.0);
    for (int i = 0; i < c_TimeAxisSize + 1; i++)  xxval[i] = xval[i];

    double chi2_0 = Chisq(ntuple, xxval);
    if (chi2_0 < 0) B2ERROR("iTBC chisq_0<0! xval has problem.");

    std::vector<double> dr_chi2(c_TimeAxisSize + 1, 0.0);
    TH1D hdrsamp_try("hdrsamp_try", "dchi2/dx distribution", 100, -0.01, 0.01);

    for (int smp = 1; smp < c_TimeAxisSize; smp++) {
      xxval[smp] = xval[smp] + m_dev_step;
      double chi2_ch = Chisq(ntuple, xxval);
      if (chi2_ch < 0)continue;
      dr_chi2[smp] = (chi2_ch - chi2_0) / m_dev_step;
      hdrsamp_try.Fill(dr_chi2[smp]);
      xxval[smp] = xval[smp];
    }

    for (int smp = 1; smp < c_TimeAxisSize; smp++) {
      double vx_it_step = dr_chi2[smp] * m_xstep;
      xval[smp] = xval[smp] - vx_it_step;
    }

    //save rms of dchi2/dxval.
    m_dchi2dxv = hdrsamp_try.GetRMS();
    //change m_xstep
    if (fabs(m_dchi2dxv) < m_change_xstep) m_xstep = m_new_xstep;
  }

  double TOPTimeBaseCalibratorModule::Chisq(const std::vector<TwoTimes>& ntuple, std::vector<double>& xxval)
  {
    double sum1 = 0.0;
    double sum2 = 0.0; //sum od dt and dt**2

    m_good = 0;

    for (const auto& twoTimes : ntuple) {
      if (!twoTimes.good) continue;

      std::vector<double> m(c_TimeAxisSize, 0.0);

      int i1 = int(twoTimes.t1);
      double fr = twoTimes.t1 - i1;
      double samp0 = i1 % 256;
      double ctdc1 = xxval[samp0] + fr * (xxval[samp0 + 1] - xxval[samp0]);
      int i2 = int(twoTimes.t2);
      fr = twoTimes.t2 - i2;
      double samp1 = i2 % 256;
      double ctdc2 = xxval[samp1] + fr * (xxval[samp1 + 1] - xxval[samp1]);
      double cdt = 0.0;
      if (samp1 > samp0) cdt = ctdc2 - ctdc1;
      else            cdt = ctdc2 - ctdc1 + m_syncTimeBase * 2;

      if (cdt < m_dt_max && cdt > m_dt_min) {
        sum1 += cdt;
        sum2 += cdt * cdt;
        m_good++;
      }
    }

    double mean = 0.0;
    double chi2 = -1.0;

    if (m_good > 10) {
      mean = sum1 / m_good;
      chi2 = (sum2 - m_good * mean * mean) / m_good / m_sigm2_exp;
    }
    return chi2;
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


  void TOPTimeBaseCalibratorModule::saveAsHistogram(const vector<double>& vec,
                                                    const vector<double>& err,
                                                    const string& name,
                                                    const string& title,
                                                    const string& xTitle,
                                                    const string& yTitle) const
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
                                                    const string& name,
                                                    const string& title) const
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

