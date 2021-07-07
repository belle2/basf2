/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <klm/calibration/KLMLikelihoodParametersImporter.h>

/* KLM headers. */
#include <klm/dbobjects/KLMLikelihoodParameters.h>
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

void KLMLikelihoodParametersImporter::writeLikelihoodParameters()
{
  B2WARNING("The method KLMLikelihoodParametersImporter::writeMuidParameters() is temporary unavailable, sorry! :(");
  return;
  /** cppcheck-suppress unreachableCode */
  DBImportObjPtr<KLMLikelihoodParameters> likelihoodParameters;
  likelihoodParameters.construct();
  vector<string> const hypotheses = {"Positron", "Electron" , "Deuteron", "Antideuteron", "Proton", "Antiproton", "PionPlus", "PionMinus", "KaonPlus", "KaonMinus", "MuonPlus", "MuonMinus" };
  for (unsigned int hypothesis = 0; hypothesis < hypotheses.size(); hypothesis++) {
    GearDir content("/Detector/Muid/MuidParameters//Experiment[@exp=\"0\"]/");
    content.append(hypotheses[hypothesis]);
    for (int outcome = 1; outcome <= MuidElementNumbers::getMaximalOutcome(); ++outcome) {
      GearDir outcomeContent(content);
      outcomeContent.append((boost::format("/LayerProfile/Outcome[@outcome=\"%1%\"]/") % (outcome)).str());
      for (int lastLayer = 0; lastLayer <= MuidElementNumbers::getMaximalBarrelLayer(); ++lastLayer) {
        if (!(MuidElementNumbers::checkExtrapolationOutcome(outcome, lastLayer)))
          break;
        std::vector<double> layerPDF = outcomeContent.getArray((boost::format("LastLayer[@layer=\"%1%\"]") % (lastLayer)).str());
        likelihoodParameters->setLongitudinalPDF(hypothesis, outcome, lastLayer, layerPDF);
      }
    }
    for (int detector = 0; detector <= MuidElementNumbers::getMaximalDetector(); ++detector) {
      GearDir detectorContent(content);
      if (detector == 0)
        detectorContent.append("/TransversePDF/BarrelAndEndcap");
      if (detector == 1)
        detectorContent.append("/TransversePDF/BarrelOnly");
      if (detector == 2)
        detectorContent.append("/TransversePDF/EndcapOnly");
      for (int halfNdof = 1; halfNdof <= MuidElementNumbers::getMaximalHalfNdof(); ++halfNdof) {
        double reducedChiSquaredThreshold = detectorContent.getDouble((boost::format("DegreesOfFreedom[@ndof=\"%1%\"]/Tail/Threshold") %
                                            (2 * halfNdof)).str());
        double reducedChiSquaredScaleY = detectorContent.getDouble((boost::format("DegreesOfFreedom[@ndof=\"%1%\"]/Tail/ScaleY") %
                                                                    (2 * halfNdof)).str());
        double reducedChiSquaredScaleX = detectorContent.getDouble((boost::format("DegreesOfFreedom[@ndof=\"%1%\"]/Tail/ScaleX") %
                                                                    (2 * halfNdof)).str());
        std::vector<double> reducedChiSquaredPDF = detectorContent.getArray((boost::format("DegreesOfFreedom[@ndof=\"%1%\"]/Histogram") %
                                                   (2 * halfNdof)).str());
        likelihoodParameters->setTransversePDF(hypothesis, detector, halfNdof * 2, reducedChiSquaredPDF);
        likelihoodParameters->setTransverseThreshold(hypothesis, detector, halfNdof * 2, reducedChiSquaredThreshold);
        likelihoodParameters->setTransverseScaleY(hypothesis, detector, halfNdof * 2, reducedChiSquaredScaleY);
        likelihoodParameters->setTransverseScaleX(hypothesis, detector, halfNdof * 2, reducedChiSquaredScaleX);
      }
    }
  }
  IntervalOfValidity Iov(0, 0, -1, -1);
  likelihoodParameters.import(Iov);
}

void KLMLikelihoodParametersImporter::readLikelihoodParameters()
{
  DBObjPtr<KLMLikelihoodParameters> likelihoodParameters;
  vector<string> const hypotheses = {"Positron", "Electron" , "Deuteron", "Antideuteron", "Proton", "Antiproton", "PionPlus", "PionMinus", "KaonPlus", "KaonMinus", "MuonPlus", "MuonMinus" };
  for (unsigned int hypothesis = 0; hypothesis < hypotheses.size(); hypothesis++) {
    B2INFO(" hypothesisName  " << hypotheses[hypothesis]);
    for (int outcome = 1; outcome <= MuidElementNumbers::getMaximalOutcome(); ++outcome) {
      B2INFO(" outcome " << outcome);
      for (int lastLayer = 0; lastLayer <= MuidElementNumbers::getMaximalBarrelLayer(); ++lastLayer) {
        B2INFO(" lastLayer " << lastLayer);
        if (!(MuidElementNumbers::checkExtrapolationOutcome(outcome, lastLayer)))
          break;
        std::vector<double> layerPDF = likelihoodParameters->getLongitudinalPDF(hypothesis, outcome, lastLayer);
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
        B2INFO(" ReducedChiSquaredThreshold  " << likelihoodParameters->getTransverseThreshold(hypothesis, detector, halfNdof * 2));
        B2INFO(" ReducedChiSquaredScaleY " << likelihoodParameters->getTransverseScaleY(hypothesis, detector, halfNdof * 2));
        B2INFO(" ReducedChiSquaredScaleX " << likelihoodParameters->getTransverseScaleX(hypothesis, detector, halfNdof * 2));
        std::vector<double> reducedChiSquaredPDF = likelihoodParameters->getTransversePDF(hypothesis, detector, halfNdof * 2);
        if (reducedChiSquaredPDF.size() != MuidElementNumbers::getSizeReducedChiSquared()) {
          B2ERROR("KLMLikelihoodParametersImporter::TransversePDF vector for hypothesis " << hypotheses[hypothesis] << "  detector " <<
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

