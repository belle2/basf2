/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <top/reconstruction/TOP1Dpdf.h>
#include <algorithm>
#include <framework/datastore/StoreArray.h>
#include <top/dataobjects/TOPDigit.h>


namespace Belle2 {
  namespace TOP {

    TOP1Dpdf::TOP1Dpdf(TOPreco& reco, int moduleID, double binSize): m_moduleID(moduleID)
    {

      if (binSize <= 0) B2FATAL("TOP1Dpdf: bin size must be positive");

      // time window and binning
      reco.getTimeWindow(m_minTime, m_maxTime);
      m_numBins = (m_maxTime - m_minTime) / binSize + 1;
      m_binSize = (m_maxTime - m_minTime) / m_numBins;

      // temporary histogram to make 1D projection of PDF
      TH1F pdfHisto("pdf_temporary", "", m_numBins, m_minTime, m_maxTime);

      // first add background
      m_bkg = reco.getExpectedBG() / m_numBins;
      for (int i = 0; i < m_numBins; i++) pdfHisto.SetBinContent(i + 1, m_bkg);

      // then fill signal PDF
      bool start = true;
      for (int pix = 1; pix <= 512; pix++) {
        for (int k = 0; k < reco.getNumofPDFPeaks(pix); k++) {
          float pos = 0;
          float wid = 0;
          float nph = 0;
          reco.getPDFPeak(pix, k, pos, wid, nph);
          pdfHisto.Fill(pos, nph);
          if (start) {
            start = false;
            m_tminPDF = pos;
            m_tmaxPDF = pos;
          }
          m_tminPDF = std::min(m_tminPDF, static_cast<double>(pos));
          m_tmaxPDF = std::max(m_tmaxPDF, static_cast<double>(pos));
        }
      }

      // finally make look-up table for log(PDF)
      for (int i = 0; i < m_numBins; i++) {
        m_logF.push_back(log(pdfHisto.GetBinContent(i + 1)));
      }
      m_logBkg = log(m_bkg);

      // photon times
      StoreArray<TOPDigit> digits;
      if (digits.getEntries() == 0) return;
      m_tminFot = m_tmaxFot = digits[0]->getTime();
      for (const auto& digit : digits) {
        if (digit.getHitQuality() == TOPDigit::c_Good) {
          double t = digit.getTime();
          m_tminFot = std::min(m_tminFot, t);
          m_tmaxFot = std::max(m_tmaxFot, t);
          if (digit.getModuleID() == m_moduleID) m_times.push_back(t);
        }
      }

      // T0 search region
      m_minT0 = m_tminFot - m_tmaxPDF - 2 * m_binSize;
      double maxT0 = m_tmaxFot - m_tminPDF;
      m_numBinsT0 = std::max(int((maxT0 - m_minT0) / m_binSize), 3) + 1;
      m_maxT0 = m_minT0 + m_binSize * m_numBinsT0;

    }


    double TOP1Dpdf::getLogL(double timeShift)
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


    TH1F TOP1Dpdf::getHistogram(std::string name, std::string title) const
    {
      TH1F h(name.c_str(), title.c_str(), m_logF.size(), m_minTime, m_maxTime);
      for (size_t i = 0; i < m_logF.size(); i++) {
        h.SetBinContent(i + 1, exp(m_logF[i]));
      }
      return h;
    }


  } // end top namespace
} // end Belle2 namespace
