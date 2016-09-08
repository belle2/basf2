/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov, Kirill Chilikin                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Belle2 headers. */
#include <eklm/simulation/Digitizer.h>
#include <framework/gearbox/GearDir.h>
#include <cmath>

using namespace Belle2;

void EKLM::setDefDigitizationParams(struct DigitizationParams* digPar)
{
  GearDir dig("/Detector/DetectorComponent[@name=\"EKLM\"]/"
              "Content/DigitizationParams");
  digPar->ADCRange = dig.getDouble("ADCRange");
  digPar->ADCSamplingTime = dig.getDouble("ADCSamplingTime");
  digPar->nDigitizations = dig.getDouble("nDigitizations");
  digPar->ADCPedestal = dig.getDouble("ADCPedestal");
  digPar->ADCPEAmplitude = dig.getDouble("ADCPEAmplitude");
  digPar->ADCSaturation = dig.getDouble("ADCSaturation");
  digPar->nPEperMeV = dig.getDouble("nPEperMeV");
  digPar->minCosTheta = cos(dig.getDouble("MaxTotalIRAngle") / 180.0 * M_PI);
  digPar->mirrorReflectiveIndex = dig.getDouble("MirrorReflectiveIndex");
  digPar->scintillatorDeExcitationTime = dig.getDouble("ScintDeExTime");
  digPar->fiberDeExcitationTime = dig.getDouble("FiberDeExTime");
  digPar->fiberLightSpeed = dig.getDouble("FiberLightSpeed");
  digPar->attenuationLength = dig.getDouble("AttenuationLength");
  digPar->PEAttenuationFreq = dig.getDouble("PEAttenuationFreq");
  digPar->meanSiPMNoise = dig.getDouble("MeanSiPMNoise");
  digPar->enableConstBkg = dig.getDouble("EnableConstBkg") > 0;
  digPar->timeResolution = dig.getDouble("TimeResolution");
}

