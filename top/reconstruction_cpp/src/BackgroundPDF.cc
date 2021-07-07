/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <top/reconstruction_cpp/BackgroundPDF.h>
#include <top/reconstruction_cpp/TOPRecoManager.h>
#include <framework/logging/Logger.h>

using namespace std;

namespace Belle2 {
  namespace TOP {

    BackgroundPDF::BackgroundPDF(int moduleID):
      m_moduleID(moduleID)
    {
      const auto* yScanner = TOPRecoManager::getYScanner(moduleID);
      if (not yScanner) B2FATAL("TOP::BackgroundPDF: invalid slot number, moduleID = " << moduleID);

      const auto& pixelPositions = yScanner->getPixelPositions().getPixels();
      const auto& pixelMasks = yScanner->getPixelMasks().getMasks();
      const auto& pixelEfficiencies = yScanner->getPixelEfficiencies().getEfficiencies();

      if (pixelMasks.size() != pixelPositions.size() or pixelEfficiencies.size() != pixelPositions.size()) {
        B2FATAL("TOP::BackgroundPDF: pixel positions, masks and efficiencies have different sizes for slot " << moduleID
                << LogVar("pixelPositions.size()", pixelPositions.size())
                << LogVar("pixelMasks.size()", pixelMasks.size())
                << LogVar("pixelEfficiencies.size()", pixelEfficiencies.size()));
      }

      m_pdf.resize(pixelPositions.size(), 0);
      set();
    }


    void BackgroundPDF::set()
    {
      const auto* yScanner = TOPRecoManager::getYScanner(m_moduleID);
      const auto& pixelPositions = yScanner->getPixelPositions().getPixels();
      const auto& pixelMasks = yScanner->getPixelMasks().getMasks();
      const auto& pixelEfficiencies = yScanner->getPixelEfficiencies().getEfficiencies();

      double S = 0;
      for (size_t i = 0; i < m_pdf.size(); i++) {
        const auto& pixel = pixelPositions[i];
        double s = pixel.Dx * pixel.Dy;
        S += s;
        if (pixelMasks[i]) {
          m_pdf[i] = s * pixelEfficiencies[i];
        } else {
          m_pdf[i] = 0;
        }
      }

      double sum = 0;
      for (auto pdf : m_pdf) sum += pdf;
      if (sum == 0) return;

      for (auto& pdf : m_pdf) pdf /= sum;
      m_effi = sum / S;
    }


    double BackgroundPDF::getPDFValue(int pixelID) const
    {
      unsigned k = pixelID - 1;
      if (k < m_pdf.size()) return m_pdf[k] / TOPRecoManager::getTimeWindowSize();
      return 0;
    }

  } // namespace TOP
} // namespace Belle2


