/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <top/reconstruction_cpp/PDF1Dim.h>
#include <top/reconstruction_cpp/TOPRecoManager.h>
#include <framework/logging/Logger.h>
#include <algorithm>
#include <cmath>

namespace Belle2 {
  namespace TOP {

    PDF1Dim::PDF1Dim(const PDFConstructor& pdfConstructor, double binSize, double timeWindow):
      m_moduleID(pdfConstructor.getModuleID()), m_minTime(TOPRecoManager::getMinTime()),
      m_maxTime(TOPRecoManager::getMaxTime())
    {

      if (binSize <= 0) B2FATAL("TOP::PDF1Dim: bin size must be positive");
      if (not pdfConstructor.isValid()) {
        B2ERROR("TOP::PDF1Dim: PDFConstructor is not valid, cannot continue");
        return;
      }

      // binning
      m_numBins = (m_maxTime - m_minTime) / binSize + 1;
      m_binSize = (m_maxTime - m_minTime) / m_numBins;

      // photon times
      for (const auto& hit : pdfConstructor.getSelectedHits()) {
        m_times.push_back(hit.time);
      }
      if (not m_times.empty()) {
        std::sort(m_times.begin(), m_times.end());
        m_tminFot = m_times.front();
        m_tmaxFot = m_times.back();
      }

      m_expectedSignal = pdfConstructor.getExpectedSignalPhotons();
      m_expectedDelta = pdfConstructor.getExpectedDeltaPhotons();
      m_expectedBG = std::max(pdfConstructor.getBkgRate() * timeWindow, 0.1);

      // temporary histogram to make 1D projection of PDF
      TH1F pdfHisto("pdf_temporary", "", m_numBins, m_minTime, m_maxTime);

      // first add background and delta-ray PDF's
      m_bkg = m_expectedBG / timeWindow * m_binSize;
      const auto& deltaRayPDF = pdfConstructor.getDeltaRayPDF();
      for (int i = 0; i < m_numBins; i++) {
        double t = pdfHisto.GetBinCenter(i + 1);
        pdfHisto.SetBinContent(i + 1, m_bkg + m_expectedDelta * deltaRayPDF.getPDFValue(t) * m_binSize);
      }

      // then fill signal PDF
      bool start = true;
      for (const auto& pixelPDF : pdfConstructor.getSignalPDF()) {
        for (const auto& peak : pixelPDF.getPDFPeaks()) {
          pdfHisto.Fill(peak.t0, m_expectedSignal * peak.nph);
          if (start) {
            start = false;
            m_tminPDF = peak.t0;
            m_tmaxPDF = peak.t0;
          }
          m_tminPDF = std::min(m_tminPDF, peak.t0);
          m_tmaxPDF = std::max(m_tmaxPDF, peak.t0);
        }
      }

      // finally make look-up table for log(PDF)
      for (int i = 0; i < m_numBins; i++) {
        m_logF.push_back(log(pdfHisto.GetBinContent(i + 1)));
      }
      m_logBkg = log(m_bkg);

      // determine T0 search region
      m_minT0 = m_tminFot - m_tmaxPDF - 2 * m_binSize;
      double maxT0 = m_tmaxFot - m_tminPDF;
      m_numBinsT0 = std::max(int((maxT0 - m_minT0) / m_binSize), 3) + 1;
      m_maxT0 = m_minT0 + m_binSize * m_numBinsT0;

    }


    double PDF1Dim::getLogL(double timeShift) const
    {
      double logL = 0;
      for (const auto& time : m_times) {
        double f = m_logBkg;
        double t = time - timeShift;
        if (t >= m_minTime and t < m_maxTime) {
          unsigned i = (t - m_minTime) / m_binSize;
          if (i < m_logF.size()) f = m_logF[i];
        }
        logL += f;
      }
      return logL;
    }


    TH1F PDF1Dim::getHistogram(std::string name, std::string title) const
    {
      TH1F h(name.c_str(), title.c_str(), m_logF.size(), m_minTime, m_maxTime);
      for (size_t i = 0; i < m_logF.size(); i++) {
        h.SetBinContent(i + 1, exp(m_logF[i]));
      }
      return h;
    }

  } // end top namespace
} // end Belle2 namespace

