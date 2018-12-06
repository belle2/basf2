/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Yinghui Guan                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/calibration/MuidParameterDBReaderWriter.h>
#include <tracking/dbobjects/MuidParameters.h>
#include <tracking/trackExtrapolateG4e/MuidPar.h>
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
using namespace boost;

void MuidParameterDBReaderWriter::writeMuidParameters()
{
  DBImportObjPtr<MuidParameters> muidPar;
  muidPar.construct();

  vector<string> const hypotheses = {"Positron", "Electron" , "Deuteron", "Antideuteron", "Proton", "Antiproton", "PionPlus", "PionMinus", "KaonPlus", "KaonMinus", "MuonPlus", "MuonMinus" };
  for (unsigned int hypothesis = 0; hypothesis < hypotheses.size(); hypothesis++) {
    GearDir content("/Detector/Tracking/MuidParameters//Experiment[@exp=\"0\"]/");
    content.append(hypotheses[hypothesis]);
    for (int outcome = 1; outcome <= MUID_MaxOutcome; ++outcome) {
      GearDir outcomeContent(content);
      outcomeContent.append((format("/LayerProfile/Outcome[@outcome=\"%1%\"]/") % (outcome)).str());
      for (int lastLayer = 0; lastLayer <= MUID_MaxBarrelLayer; ++lastLayer) {
        if ((outcome == MuidPar::EMuidOutcome::c_StopInBarrel)
            && (lastLayer > MUID_MaxBarrelLayer - 1)) break; // barrel stop: never in layer 14
        if ((outcome == MuidPar::EMuidOutcome::c_StopInForwardEndcap)
            && (lastLayer > MUID_MaxForwardEndcapLayer - 1)) break; // forward endcap stop: never in layer 13
        if ((outcome == MuidPar::EMuidOutcome::c_ExitBarrel) && (lastLayer > MUID_MaxBarrelLayer)) break; // barrel exit: no layers 15+
        if ((outcome == MuidPar::EMuidOutcome::c_ExitForwardEndcap)
            && (lastLayer > MUID_MaxForwardEndcapLayer)) break; // forward endcap exit: no layers 14+
        if ((outcome == MuidPar::EMuidOutcome::c_StopInBackwardEndcap)
            && (lastLayer > MUID_MaxBackwardEndcapLayer - 1)) break; // backward endcap stop: never in layer 11
        if ((outcome == MuidPar::EMuidOutcome::c_ExitBackWardEndcap)
            && (lastLayer > MUID_MaxBackwardEndcapLayer)) break; // backward endcap exit: no layers 12+
        if ((outcome >= MuidPar::EMuidOutcome::c_CrossBarrelStopInForwardMin)
            && (outcome <=  MuidPar::EMuidOutcome::c_CrossBarrelStopInForwardMax)
            && (lastLayer > MUID_MaxForwardEndcapLayer - 1)) break; // like outcome == 2
        if ((outcome >= MuidPar::EMuidOutcome::c_CrossBarrelStopInBackwardMin)
            && (outcome <=  MuidPar::EMuidOutcome::c_CrossBarrelStopInBackwardMax)
            && (lastLayer > MUID_MaxBackwardEndcapLayer - 1)) break; // like outcome == 5
        if ((outcome >= MuidPar::EMuidOutcome::c_CrossBarrelExitForwardMin)
            && (outcome <=  MuidPar::EMuidOutcome::c_CrossBarrelExitForwardMax)
            && (lastLayer > MUID_MaxForwardEndcapLayer)) break; // like outcome == 4
        if ((outcome >= MuidPar::EMuidOutcome::c_CrossBarrelExitBackwardMin)
            && (outcome <=  MuidPar::EMuidOutcome::c_CrossBarrelExitBackwardMax)
            && (lastLayer > MUID_MaxBackwardEndcapLayer)) break; // like outcome == 6
        std::vector<double> layerPDF = outcomeContent.getArray((format("LastLayer[@layer=\"%1%\"]") % (lastLayer)).str());
        muidPar->setLayerProfile(hypothesis, outcome, lastLayer, layerPDF);
      }
    }

    for (int detector = 0; detector <= MUID_MaxDetector; ++detector) {
      GearDir detectorContent(content);
      if (detector == 0) detectorContent.append("/TransversePDF/BarrelAndEndcap");
      if (detector == 1) detectorContent.append("/TransversePDF/BarrelOnly");
      if (detector == 2) detectorContent.append("/TransversePDF/EndcapOnly");
      for (int halfNdof = 1; halfNdof <= MUID_MaxHalfNdof; ++halfNdof) {
        double reducedChiSquaredThreshold = detectorContent.getDouble((format("DegreesOfFreedom[@ndof=\"%1%\"]/Tail/Threshold") %
                                            (2 * halfNdof)).str());
        double reducedChiSquaredScaleY = detectorContent.getDouble((format("DegreesOfFreedom[@ndof=\"%1%\"]/Tail/ScaleY") %
                                                                    (2 * halfNdof)).str());
        double reducedChiSquaredScaleX = detectorContent.getDouble((format("DegreesOfFreedom[@ndof=\"%1%\"]/Tail/ScaleX") %
                                                                    (2 * halfNdof)).str());
        std::vector<double> reducedChiSquaredPDF = detectorContent.getArray((format("DegreesOfFreedom[@ndof=\"%1%\"]/Histogram") %
                                                   (2 * halfNdof)).str());
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

void MuidParameterDBReaderWriter::readMuidParameters()
{

  DBObjPtr<MuidParameters> m_muidParameters("MuidParameters");
  vector<string> const hypotheses = {"Positron", "Electron" , "Deuteron", "Antideuteron", "Proton", "Antiproton", "PionPlus", "PionMinus", "KaonPlus", "KaonMinus", "MuonPlus", "MuonMinus" };
  for (unsigned int hypothesis = 0; hypothesis < hypotheses.size(); hypothesis++) {
    B2INFO(" hypothesisName  " << hypotheses[hypothesis]);
    for (int outcome = 1; outcome <= MUID_MaxOutcome; ++outcome) {
      B2INFO(" outcome " << outcome);
      for (int lastLayer = 0; lastLayer <= MUID_MaxBarrelLayer; ++lastLayer) {
        B2INFO(" lastLayer " << lastLayer);
        if ((outcome == MuidPar::EMuidOutcome::c_StopInBarrel)
            && (lastLayer > MUID_MaxBarrelLayer - 1)) break; // barrel stop: never in layer 14
        if ((outcome == MuidPar::EMuidOutcome::c_StopInForwardEndcap)
            && (lastLayer > MUID_MaxForwardEndcapLayer - 1)) break; // forward endcap stop: never in layer 13
        if ((outcome == MuidPar::EMuidOutcome::c_ExitBarrel) && (lastLayer > MUID_MaxBarrelLayer)) break; // barrel exit: no layers 15+
        if ((outcome == MuidPar::EMuidOutcome::c_ExitForwardEndcap)
            && (lastLayer > MUID_MaxForwardEndcapLayer)) break; // forward endcap exit: no layers 14+
        if ((outcome == MuidPar::EMuidOutcome::c_StopInBackwardEndcap)
            && (lastLayer > MUID_MaxBackwardEndcapLayer - 1)) break; // backward endcap stop: never in layer 11
        if ((outcome == MuidPar::EMuidOutcome::c_ExitBackWardEndcap)
            && (lastLayer > MUID_MaxBackwardEndcapLayer)) break; // backward endcap exit: no layers 12+
        if ((outcome >= MuidPar::EMuidOutcome::c_CrossBarrelStopInForwardMin)
            && (outcome <= MuidPar::EMuidOutcome::c_CrossBarrelStopInForwardMax)
            && (lastLayer > MUID_MaxForwardEndcapLayer - 1)) break; // like outcome == 2
        if ((outcome >= MuidPar::EMuidOutcome::c_CrossBarrelStopInBackwardMin)
            && (outcome <= MuidPar::EMuidOutcome::c_CrossBarrelStopInBackwardMax)
            && (lastLayer > MUID_MaxBackwardEndcapLayer - 1)) break; // like outcome == 5
        if ((outcome >= MuidPar::EMuidOutcome::c_CrossBarrelExitForwardMin)
            && (outcome <= MuidPar::EMuidOutcome::c_CrossBarrelExitForwardMax)
            && (lastLayer > MUID_MaxForwardEndcapLayer)) break; // like outcome == 4
        if ((outcome >= MuidPar::EMuidOutcome::c_CrossBarrelExitBackwardMin)
            && (outcome <= MuidPar::EMuidOutcome::c_CrossBarrelExitBackwardMax)
            && (lastLayer > MUID_MaxBackwardEndcapLayer)) break; // like outcome == 6
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
          B2ERROR("MuidParameterDBReaderWriter::TransversePDF vector for hypothesis " << hypotheses[hypothesis] << "  detector " <<
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

