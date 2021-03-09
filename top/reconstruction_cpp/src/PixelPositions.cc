/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <top/reconstruction_cpp/PixelPositions.h>
#include <top/geometry/TOPGeometryPar.h>
#include <framework/logging/Logger.h>
#include <algorithm>

namespace Belle2 {
  namespace TOP {

    PixelPositions::PixelPositions(int moduleID):
      m_moduleID(moduleID)
    {
      const auto* geo = TOPGeometryPar::Instance()->getGeometry();
      if (not geo->isModuleIDValid(moduleID)) {
        B2FATAL("TOP::PixelPositions: invalid slot number, moduleID = " << moduleID);
        return;
      }

      const auto& module = geo->getModule(moduleID);
      const auto& pmtArray = module.getPMTArray();
      m_NRows = pmtArray.getNumPixelRows();
      m_NColumns = pmtArray.getNumPixelColumns();

      const auto& prism = module.getPrism();
      double Ah = prism.getWidth() / 2;
      double yUp = prism.getThickness() / 2;
      double yDown = yUp - prism.getExitThickness();

      const auto& pmtArrayDispl = module.getPMTArrayDisplacement();
      double x0 = pmtArrayDispl.getX();
      double y0 = pmtArrayDispl.getY() + (yUp + yDown) / 2;

      const auto& pmt = pmtArray.getPMT();
      double a = pmt.getDx();
      double b = pmt.getDy();

      std::vector<double> xCenters;
      for (unsigned aColumn = 1; aColumn <= pmtArray.getNumColumns(); aColumn++) {
        for (unsigned pmtColumn = 1; pmtColumn <= pmt.getNumColumns(); pmtColumn++) {
          xCenters.push_back(pmtArray.getX(aColumn) + pmt.getX(pmtColumn) + x0);
        }
      }

      std::vector<double> yCenters;
      for (unsigned aRow = 1; aRow <= pmtArray.getNumRows(); aRow++) {
        for (unsigned pmtRow = 1; pmtRow <= pmt.getNumRows(); pmtRow++) {
          yCenters.push_back(pmtArray.getY(aRow) + pmt.getY(pmtRow) + y0);
        }
      }

      for (auto y : yCenters) {
        double y1 = std::max(y - b / 2, yDown);
        double y2 = std::min(y + b / 2, yUp);
        double Dy = std::max(y2 - y1, 0.0);
        double yc = (Dy > 0) ? (y1 + y2) / 2 : y;
        for (auto x : xCenters) {
          double x1 = std::max(x - a / 2, -Ah);
          double x2 = std::min(x + a / 2, Ah);
          double Dx = std::max(x2 - x1, 0.0);
          double xc = (Dx > 0) ? (x1 + x2) / 2 : x;
          m_pixels.push_back(PixelData(xc, yc, Dx, Dy));
        }
      }

      const auto& mapper = TOPGeometryPar::Instance()->getChannelMapper();
      for (size_t i = 0; i < m_pixels.size(); i++) {
        int pixelID = i + 1;
        m_pixels[i].ID = pixelID;
        int pmtID = mapper.getPmtID(pixelID);
        m_pixels[i].pmtType = TOPGeometryPar::Instance()->getPMTType(moduleID, pmtID);
      }

    }

  } // TOP
} // Belle2
