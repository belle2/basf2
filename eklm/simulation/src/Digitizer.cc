/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov, Kirill Chilikin                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* C++ headers. */
#include <cmath>

/* Belle2 headers. */
#include <eklm/simulation/Digitizer.h>
#include <framework/gearbox/GearDir.h>

using namespace Belle2;

void EKLM::setDefDigitizationParams(EKLMDigitizationParameters* digPar)
{
  GearDir dig("/Detector/DetectorComponent[@name=\"EKLM\"]/"
              "Content/DigitizationParams");
  digPar->setADCRange(dig.getDouble("ADCRange"));
  digPar->setADCSamplingTime(dig.getDouble("ADCSamplingTime"));
  digPar->setNDigitizations(dig.getDouble("nDigitizations"));
  digPar->setADCPedestal(dig.getDouble("ADCPedestal"));
  digPar->setADCPEAmplitude(dig.getDouble("ADCPEAmplitude"));
  digPar->setADCSaturation(dig.getDouble("ADCSaturation"));
  digPar->setNPEperMeV(dig.getDouble("nPEperMeV"));
  digPar->setMinCosTheta(cos(dig.getDouble("MaxTotalIRAngle") / 180.0 * M_PI));
  digPar->setMirrorReflectiveIndex(dig.getDouble("MirrorReflectiveIndex"));
  digPar->setScintillatorDeExcitationTime(dig.getDouble("ScintDeExTime"));
  digPar->setFiberDeExcitationTime(dig.getDouble("FiberDeExTime"));
  digPar->setFiberLightSpeed(dig.getDouble("FiberLightSpeed"));
  digPar->setAttenuationLength(dig.getDouble("AttenuationLength"));
  digPar->setPEAttenuationFrequency(dig.getDouble("PEAttenuationFreq"));
  digPar->setMeanSiPMNoise(dig.getDouble("MeanSiPMNoise"));
  digPar->setEnableConstBkg(dig.getDouble("EnableConstBkg") > 0);
  digPar->setTimeResolution(dig.getDouble("TimeResolution"));
}

