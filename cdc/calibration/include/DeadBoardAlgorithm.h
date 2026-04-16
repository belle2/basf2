/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <calibration/CalibrationAlgorithm.h>
#include <framework/database/DBObjPtr.h>

#include <cdc/dbobjects/CDCGeometry.h>
#include <cdc/dbobjects/CDCBadBoards.h>

#include <string>

namespace Belle2 {
  namespace CDC {

    /**
     * Class for dead board detection.
     */
    class DeadBoardAlgorithm : public CalibrationAlgorithm {
    public:
      /// Constructor
      DeadBoardAlgorithm();

      /// Destructor
      ~DeadBoardAlgorithm() override = default;

      /// Set histogram name
      void setHistName(const std::string& name) { m_histName = name; }

      /// Set threshold for dead board decision
      void setThreshold(const float threshold) { m_threshold = threshold; }

    protected:
      /// Run algorithm on data
      EResult calibrate() override;

      /// Detect dead boards from histogram
      void detectDeadBoards();

    private:
      std::string m_histName = "CDCboardIDs_1"; /**< input histogram name */
      float m_threshold = 0.0;                  /**< dead-board threshold */

      DBObjPtr<CDCGeometry> m_cdcGeo;           /**< Geometry of CDC */
      CDCBadBoards* m_badBoardList = nullptr;   /**< bad-board list to be saved */
    };

  }
}
