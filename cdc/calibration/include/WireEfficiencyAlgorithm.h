/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <TEfficiency.h>
#include <calibration/CalibrationAlgorithm.h>
#include <framework/database/DBObjPtr.h>

#include <cdc/dbobjects/CDCGeometry.h>
#include <cdc/dbobjects/CDCBadWires.h>

#include "string"

namespace Belle2 {
  namespace CDC {
    /**
     * Class for Wire Efficiency estimation.
     */
    class WireEfficiencyAlgorithm: public CalibrationAlgorithm {
    public:
      /// Constructor.
      WireEfficiencyAlgorithm();
      /// Destructor.
      ~WireEfficiencyAlgorithm() { delete m_efficiencyList; }
      /// Set name for histogram output
      void setHistFileName(const std::string& name) {m_outputFileName = "histWireEff_" + name + ".root";}
      /// Set the average occupancy threshold
      void setAverageOccupancyThreshold(const float threshold) { m_averageOccupancyThreshold = threshold; };
    protected:
      /// Run algo on data.
      EResult calibrate() override;
      /// check if there is enough data to run the calibration
      bool hasEnoughData();
      /// create 2D TEfficiency for each wire
      void buildEfficiencies();
      /// detects bad wires.
      void detectBadWires();
      /// chitest
      double chiTest(TGraphAsymmErrors* graph1, TGraphAsymmErrors* graph2, double minVale, double maxValue);
    private:
      float m_averageOccupancyThreshold = 4000.0; /**< Threshold for the average layer occupancy to run the calibration */
      TList* m_efficiencyList = new TList(); /**< TList of efficiencies */
      std::string m_outputFileName = "wire_efficiencies.root"; /**< name of the output file */
      DBObjPtr<CDCGeometry> m_cdcGeo; /**< Geometry of CDC */
      CDCBadWires* m_badWireList = nullptr; /**< BadWireList that willbe built */
    };


  }
}
