/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Yinghui Guan                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Own header. */
#include <klm/calibration/MuidParameterDBReaderWriter.h>

/* KLM headers. */
#include <klm/dbobjects/MuidParameters.h>
#include <klm/muid/MuidElementNumbers.h>

/* Belle 2 headers. */
#include <framework/database/DBImportObjPtr.h>
#include <framework/database/DBObjPtr.h>
#include <framework/gearbox/GearDir.h>
#include <framework/database/IntervalOfValidity.h>
#include <framework/logging/Logger.h>

/* C++ headers. */
#include <fstream>
#include <string>
#include <vector>

using namespace std;
using namespace Belle2;

void MuidParameterDBReaderWriter::writeMuidParameters()
{
  B2WARNING("The method MuidParameterDBReaderWriter::writeMuidParameters() is temporary unavailable, sorry! :(");
  return;

  DBImportObjPtr<MuidParameters> muidPar;
  muidPar.construct();

  vector<string> const hypotheses = {"Positron", "Electron" , "Deuteron", "Antideuteron", "Proton", "Antiproton", "PionPlus", "PionMinus", "KaonPlus", "KaonMinus", "MuonPlus", "MuonMinus" };
  for (unsigned int hypothesis = 0; hypothesis < hypotheses.size(); hypothesis++) {
    GearDir content("/Detector/Muid/MuidParameters//Experiment[@exp=\"0\"]/");
    content.append(hypotheses[hypothesis]);
    for (int outcome = 1; outcome <= MuidElementNumbers::getMaximalOutcome(); ++outcome) {
      GearDir outcomeContent(content);
      outcomeContent.append((boost::format("/LayerProfile/Outcome[@outcome=\"%1%\"]/") % (outcome)).str());
      for (int lastLayer = 0; lastLayer <= MuidElementNumbers::getMaximalBarrelLayer(); ++lastLayer) {
        if ((outcome == MuidElementNumbers::c_StopInBarrel)
            && (lastLayer > MuidElementNumbers::getMaximalBarrelLayer() - 1)) break; // barrel stop: never in layer 14
        if ((outcome == MuidElementNumbers::c_StopInForwardEndcap)
            && (lastLayer > MuidElementNumbers::getMaximalEndcapForwardLayer() - 1)) break; // forward endcap stop: never in layer 13
        if ((outcome == MuidElementNumbers::c_ExitBarrel)
            && (lastLayer > MuidElementNumbers::getMaximalBarrelLayer())) break; // barrel exit: no layers 15+
        if ((outcome == MuidElementNumbers::c_ExitForwardEndcap)
            && (lastLayer > MuidElementNumbers::getMaximalEndcapForwardLayer())) break; // forward endcap exit: no layers 14+
        if ((outcome == MuidElementNumbers::c_StopInBackwardEndcap)
            && (lastLayer > MuidElementNumbers::getMaximalEndcapBackwardLayer() - 1)) break; // backward endcap stop: never in layer 11
        if ((outcome == MuidElementNumbers::c_ExitBackwardEndcap)
            && (lastLayer > MuidElementNumbers::getMaximalEndcapBackwardLayer())) break; // backward endcap exit: no layers 12+
        if ((outcome >= MuidElementNumbers::c_CrossBarrelStopInForwardMin)
            && (outcome <=  MuidElementNumbers::c_CrossBarrelStopInForwardMax)
            && (lastLayer > MuidElementNumbers::getMaximalEndcapForwardLayer() - 1)) break; // like outcome == 2
        if ((outcome >= MuidElementNumbers::c_CrossBarrelStopInBackwardMin)
            && (outcome <=  MuidElementNumbers::c_CrossBarrelStopInBackwardMax)
            && (lastLayer > MuidElementNumbers::getMaximalEndcapBackwardLayer() - 1)) break; // like outcome == 5
        if ((outcome >= MuidElementNumbers::c_CrossBarrelExitForwardMin)
            && (outcome <=  MuidElementNumbers::c_CrossBarrelExitForwardMax)
            && (lastLayer > MuidElementNumbers::getMaximalEndcapForwardLayer())) break; // like outcome == 4
        if ((outcome >= MuidElementNumbers::c_CrossBarrelExitBackwardMin)
            && (outcome <=  MuidElementNumbers::c_CrossBarrelExitBackwardMax)
            && (lastLayer > MuidElementNumbers::getMaximalEndcapBackwardLayer())) break; // like outcome == 6
        std::vector<double> layerPDF = outcomeContent.getArray((boost::format("LastLayer[@layer=\"%1%\"]") % (lastLayer)).str());
        muidPar->setLayerProfile(hypothesis, outcome, lastLayer, layerPDF);
      }
    }

    for (int detector = 0; detector <= MuidElementNumbers::getMaximalDetector(); ++detector) {
      GearDir detectorContent(content);
      if (detector == 0) detectorContent.append("/TransversePDF/BarrelAndEndcap");
      if (detector == 1) detectorContent.append("/TransversePDF/BarrelOnly");
      if (detector == 2) detectorContent.append("/TransversePDF/EndcapOnly");
      for (int halfNdof = 1; halfNdof <= MuidElementNumbers::getMaximalHalfNdof(); ++halfNdof) {
        double reducedChiSquaredThreshold = detectorContent.getDouble((boost::format("DegreesOfFreedom[@ndof=\"%1%\"]/Tail/Threshold") %
                                            (2 * halfNdof)).str());
        double reducedChiSquaredScaleY = detectorContent.getDouble((boost::format("DegreesOfFreedom[@ndof=\"%1%\"]/Tail/ScaleY") %
                                                                    (2 * halfNdof)).str());
        double reducedChiSquaredScaleX = detectorContent.getDouble((boost::format("DegreesOfFreedom[@ndof=\"%1%\"]/Tail/ScaleX") %
                                                                    (2 * halfNdof)).str());
        std::vector<double> reducedChiSquaredPDF = detectorContent.getArray((boost::format("DegreesOfFreedom[@ndof=\"%1%\"]/Histogram") %
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
    for (int outcome = 1; outcome <= MuidElementNumbers::getMaximalOutcome(); ++outcome) {
      B2INFO(" outcome " << outcome);
      for (int lastLayer = 0; lastLayer <= MuidElementNumbers::getMaximalBarrelLayer(); ++lastLayer) {
        B2INFO(" lastLayer " << lastLayer);
        if ((outcome == MuidElementNumbers::c_StopInBarrel)
            && (lastLayer > MuidElementNumbers::getMaximalBarrelLayer() - 1)) break; // barrel stop: never in layer 14
        if ((outcome == MuidElementNumbers::c_StopInForwardEndcap)
            && (lastLayer > MuidElementNumbers::getMaximalEndcapForwardLayer() - 1)) break; // forward endcap stop: never in layer 13
        if ((outcome == MuidElementNumbers::c_ExitBarrel)
            && (lastLayer > MuidElementNumbers::getMaximalBarrelLayer())) break; // barrel exit: no layers 15+
        if ((outcome == MuidElementNumbers::c_ExitForwardEndcap)
            && (lastLayer > MuidElementNumbers::getMaximalEndcapForwardLayer())) break; // forward endcap exit: no layers 14+
        if ((outcome == MuidElementNumbers::c_StopInBackwardEndcap)
            && (lastLayer > MuidElementNumbers::getMaximalEndcapBackwardLayer() - 1)) break; // backward endcap stop: never in layer 11
        if ((outcome == MuidElementNumbers::c_ExitBackwardEndcap)
            && (lastLayer > MuidElementNumbers::getMaximalEndcapBackwardLayer())) break; // backward endcap exit: no layers 12+
        if ((outcome >= MuidElementNumbers::c_CrossBarrelStopInForwardMin)
            && (outcome <= MuidElementNumbers::c_CrossBarrelStopInForwardMax)
            && (lastLayer > MuidElementNumbers::getMaximalEndcapForwardLayer() - 1)) break; // like outcome == 2
        if ((outcome >= MuidElementNumbers::c_CrossBarrelStopInBackwardMin)
            && (outcome <= MuidElementNumbers::c_CrossBarrelStopInBackwardMax)
            && (lastLayer > MuidElementNumbers::getMaximalEndcapBackwardLayer() - 1)) break; // like outcome == 5
        if ((outcome >= MuidElementNumbers::c_CrossBarrelExitForwardMin)
            && (outcome <= MuidElementNumbers::c_CrossBarrelExitForwardMax)
            && (lastLayer > MuidElementNumbers::getMaximalEndcapForwardLayer())) break; // like outcome == 4
        if ((outcome >= MuidElementNumbers::c_CrossBarrelExitBackwardMin)
            && (outcome <= MuidElementNumbers::c_CrossBarrelExitBackwardMax)
            && (lastLayer > MuidElementNumbers::getMaximalEndcapBackwardLayer())) break; // like outcome == 6
        std::vector<double> layerPDF = m_muidParameters->getProfile(hypothesis, outcome, lastLayer);
        B2INFO(" layerPDF:  ");
        for (unsigned int layer = 0; layer < layerPDF.size(); ++layer) {
          B2INFO(layerPDF[layer] << " , ");
        }
      }
    }

    const char* detectorNames[] = {"BarrelAndEndcap", "BarrelOnly", "EndcapOnly"};
    for (int detector = 0; detector <= MuidElementNumbers::getMaximalDetector(); ++detector) {
      B2INFO(" detectorName  " << detectorNames[detector]);
      for (int halfNdof = 1; halfNdof <= MuidElementNumbers::getMaximalHalfNdof(); ++halfNdof) {
        B2INFO(" Ndof  " << halfNdof * 2);
        B2INFO(" ReducedChiSquaredThreshold  " << m_muidParameters->getThreshold(hypothesis, detector, halfNdof * 2));
        B2INFO(" ReducedChiSquaredScaleY " << m_muidParameters->getScaleY(hypothesis, detector, halfNdof * 2));
        B2INFO(" ReducedChiSquaredScaleX " << m_muidParameters->getScaleX(hypothesis, detector, halfNdof * 2));
        std::vector<double> reducedChiSquaredPDF = m_muidParameters->getPDF(hypothesis, detector, halfNdof * 2);
        if (reducedChiSquaredPDF.size() != MuidElementNumbers::getSizeReducedChiSquared()) {
          B2ERROR("MuidParameterDBReaderWriter::TransversePDF vector for hypothesis " << hypotheses[hypothesis] << "  detector " <<
                  detectorNames[detector]
                  << " has " << reducedChiSquaredPDF.size() << " entries; should be " << MuidElementNumbers::getSizeReducedChiSquared());
        } else {
          for (int i = 0; i < MuidElementNumbers::getSizeReducedChiSquared(); ++i) {
            B2INFO(" PDF  " << reducedChiSquaredPDF[i]);
          }
        }
      }
    }

  }

}

