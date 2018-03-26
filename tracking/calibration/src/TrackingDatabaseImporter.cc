/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:                                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/calibration/TrackingDatabaseImporter.h>
#include <tracking/dbobjects/MuidParameters.h>
#include <tracking/trackExtrapolateG4e/MuidPar.h>
#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
#include <framework/database/IntervalOfValidity.h>
#include <framework/database/Database.h>
#include <framework/database/DBObjPtr.h>
#include <framework/database/DBImportObjPtr.h>

#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <iostream>

using namespace std;
using namespace Belle2;

TrackingDatabaseImporter::TrackingDatabaseImporter()
{}

void TrackingDatabaseImporter::importMuidParameters()
{
  DBImportObjPtr<MuidParameters> muidPar;
  muidPar.construct();

  char line[128];
  const char* hypothesisName[] = {"Positron", "Electron" , "Deuteron", "Antideuteron", "Proton", "Antiproton", "PionPlus", "PionMinus", "KaonPlus", "KaonMinus", "MuonPlus", "MuonMinus" };
  for (int hypothesis = 0; hypothesis < 12 ; hypothesis++) {
    GearDir content("/Detector/Tracking/MuidParameters");
    sprintf(line, "/Experiment[@exp=\"%d\"]/%s/", 0, hypothesisName[hypothesis]);
    content.append(line);
    //read layer profile
    for (int outcome = 1; outcome <= MUID_MaxOutcome; ++outcome) {
      sprintf(line, "LayerProfile/Outcome[@outcome=\"%d\"]/", outcome);
      GearDir outcomeContent(content);
      outcomeContent.append(line);
      for (int lastLayer = 0; lastLayer <= MUID_MaxBarrelLayer; ++lastLayer) {
        if ((outcome == 1) && (lastLayer > MUID_MaxBarrelLayer - 1)) break; // barrel stop: never in layer 14
        if ((outcome == 2) && (lastLayer > MUID_MaxForwardEndcapLayer - 1)) break; // forward endcap stop: never in layer 13
        if ((outcome == 3) && (lastLayer > MUID_MaxBarrelLayer)) break; // barrel exit: no layers 15+
        if ((outcome == 4) && (lastLayer > MUID_MaxForwardEndcapLayer)) break; // forward endcap exit: no layers 14+
        if ((outcome == 5) && (lastLayer > MUID_MaxBackwardEndcapLayer - 1)) break; // backward endcap stop: never in layer 11
        if ((outcome == 6) && (lastLayer > MUID_MaxBackwardEndcapLayer)) break; // backward endcap exit: no layers 12+
        if ((outcome >= 7) && (outcome <= 21) && (lastLayer > MUID_MaxForwardEndcapLayer - 1)) break; // like outcome == 2
        if ((outcome >= 22) && (outcome <= 36) && (lastLayer > MUID_MaxBackwardEndcapLayer - 1)) break; // like outcome == 5
        if ((outcome >= 37) && (outcome <= 51) && (lastLayer > MUID_MaxForwardEndcapLayer)) break; // like outcome == 4
        if ((outcome >= 52) && (outcome <= 66) && (lastLayer > MUID_MaxBackwardEndcapLayer)) break; // like outcome == 6
        sprintf(line, "LastLayer[@layer=\"%d\"]", lastLayer);
        std::vector<double> layerPDF = outcomeContent.getArray(line);
        muidPar->setLayerProfile(hypothesis, outcome, lastLayer, layerPDF);
      }
    }

    //read transverse PDF
    for (int detector = 0; detector <= MUID_MaxDetector; ++detector) {
      sprintf(line, "TransversePDF/BarrelAndEndcap");
      if (detector == 1) { sprintf(line, "TransversePDF/BarrelOnly"); }
      if (detector == 2) { sprintf(line, "TransversePDF/EndcapOnly"); }
      GearDir detectorContent(content);
      detectorContent.append(line);
      for (int halfNdof = 1; halfNdof <= MUID_MaxHalfNdof; ++halfNdof) {
        sprintf(line, "DegreesOfFreedom[@ndof=\"%d\"]/Tail/Threshold", 2 * halfNdof);
        double reducedChiSquaredThreshold = detectorContent.getDouble(line);
        sprintf(line, "DegreesOfFreedom[@ndof=\"%d\"]/Tail/ScaleY", 2 * halfNdof);
        double reducedChiSquaredScaleY = detectorContent.getDouble(line);
        sprintf(line, "DegreesOfFreedom[@ndof=\"%d\"]/Tail/ScaleX", 2 * halfNdof);
        double reducedChiSquaredScaleX = detectorContent.getDouble(line);
        sprintf(line, "DegreesOfFreedom[@ndof=\"%d\"]/Histogram", 2 * halfNdof);
        std::vector<double> reducedChiSquaredPDF = detectorContent.getArray(line);
        muidPar->setPDF(hypothesis, detector, halfNdof * 2, reducedChiSquaredPDF);
        muidPar->setThreshold(hypothesis, detector, halfNdof * 2, reducedChiSquaredThreshold);
        muidPar->setScaleY(hypothesis, detector, halfNdof * 2, reducedChiSquaredScaleY);
        muidPar->setScaleX(hypothesis, detector, halfNdof * 2, reducedChiSquaredScaleX);
      }
    }
  }

  IntervalOfValidity Iov(0, 0, -1, -1);
  muidPar.import(Iov);

}

void TrackingDatabaseImporter::exportMuidParameters()
{

  DBObjPtr<MuidParameters> m_muidParameters("MuidParameters");
  const char* hypothesisNames[] = {"Positron", "Electron" , "Deuteron", "Antideuteron", "Proton", "Antiproton", "PionPlus", "PionMinus", "KaonPlus", "KaonMinus", "MuonPlus", "MuonMinus" };

  for (int hypothesis = 0; hypothesis < 12 ; hypothesis++) {
    B2INFO(" hypothesisName  " << hypothesisNames[hypothesis]);
    for (int outcome = 1; outcome <= MUID_MaxOutcome; ++outcome) {
      B2INFO(" outcome " << outcome);
      for (int lastLayer = 0; lastLayer <= MUID_MaxBarrelLayer; ++lastLayer) {
        B2INFO(" lastLayer " << lastLayer);
        if ((outcome == 1) && (lastLayer > MUID_MaxBarrelLayer - 1)) break; // barrel stop: never in layer 14
        if ((outcome == 2) && (lastLayer > MUID_MaxForwardEndcapLayer - 1)) break; // forward endcap stop: never in layer 13
        if ((outcome == 3) && (lastLayer > MUID_MaxBarrelLayer)) break; // barrel exit: no layers 15+
        if ((outcome == 4) && (lastLayer > MUID_MaxForwardEndcapLayer)) break; // forward endcap exit: no layers 14+
        if ((outcome == 5) && (lastLayer > MUID_MaxBackwardEndcapLayer - 1)) break; // backward endcap stop: never in layer 11
        if ((outcome == 6) && (lastLayer > MUID_MaxBackwardEndcapLayer)) break; // backward endcap exit: no layers 12+
        if ((outcome >= 7) && (outcome <= 21) && (lastLayer > MUID_MaxForwardEndcapLayer - 1)) break; // like outcome == 2
        if ((outcome >= 22) && (outcome <= 36) && (lastLayer > MUID_MaxBackwardEndcapLayer - 1)) break; // like outcome == 5
        if ((outcome >= 37) && (outcome <= 51) && (lastLayer > MUID_MaxForwardEndcapLayer)) break; // like outcome == 4
        if ((outcome >= 52) && (outcome <= 66) && (lastLayer > MUID_MaxBackwardEndcapLayer)) break; // like outcome == 6
        std::vector<double> layerPDF = m_muidParameters->getProfile(hypothesis, outcome, lastLayer);
        B2INFO(" layerPDF:  ");
        for (unsigned int layer = 0; layer < layerPDF.size(); ++layer) {
          B2INFO(layerPDF[layer] << " , ");
        }
      }
    }

    const char* detectorNames[] = {"BarrelAndEndcap", "BarrelOnly", "EndcapOnly"};
    for (int detector = 0; detector <= MUID_MaxDetector; ++detector) {
      B2INFO(" detectorName  " << detectorNames[detector]);
      for (int halfNdof = 1; halfNdof <= MUID_MaxHalfNdof; ++halfNdof) {
        B2INFO(" Ndof  " << halfNdof * 2);
        B2INFO(" ReducedChiSquaredThreshold  " << m_muidParameters->getThreshold(hypothesis, detector, halfNdof * 2));
        B2INFO(" ReducedChiSquaredScaleY " << m_muidParameters->getScaleY(hypothesis, detector, halfNdof * 2));
        B2INFO(" ReducedChiSquaredScaleX " << m_muidParameters->getScaleX(hypothesis, detector, halfNdof * 2));
        std::vector<double> reducedChiSquaredPDF = m_muidParameters->getPDF(hypothesis, detector, halfNdof * 2);
        if (reducedChiSquaredPDF.size() != MUID_ReducedChiSquaredNbins) {
          B2ERROR("TrackingDatabaseImporter::TransversePDF vector for hypothesis " << hypothesisNames[hypothesis] << "  detector " <<
                  detectorNames[detector]
                  << " has " << reducedChiSquaredPDF.size() << " entries; should be " << MUID_ReducedChiSquaredNbins);
        } else {
          for (int i = 0; i < MUID_ReducedChiSquaredNbins; ++i) {
            B2INFO(" PDF  " << reducedChiSquaredPDF[i]);
          }
        }
      }
    }

  }

}

