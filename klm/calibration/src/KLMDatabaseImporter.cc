/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* C++ headers. */
#include <cmath>

/* External headers. */
#include <TFile.h>
#include <TTree.h>

/* Belle2 headers. */
#include <framework/database/IntervalOfValidity.h>
#include <framework/database/DBImportObjPtr.h>
#include <framework/gearbox/GearDir.h>
#include <klm/calibration/KLMDatabaseImporter.h>
#include <klm/dbobjects/KLMScintillatorSimulationParameters.h>

using namespace Belle2;

KLMDatabaseImporter::KLMDatabaseImporter() :
  m_ExperimentLow(0),
  m_RunLow(0),
  m_ExperimentHigh(-1),
  m_RunHigh(-1)
{
}

KLMDatabaseImporter::~KLMDatabaseImporter()
{
}

void KLMDatabaseImporter::setIOV(int experimentLow, int runLow,
                                 int experimentHigh, int runHigh)
{
  m_ExperimentLow = experimentLow;
  m_RunLow = runLow;
  m_ExperimentHigh = experimentHigh;
  m_RunHigh = runHigh;
}

void KLMDatabaseImporter::importScintillatorSimulationParameters()
{
  DBImportObjPtr<KLMScintillatorSimulationParameters> simPar;
  simPar.construct();
  GearDir d("/Detector/DetectorComponent[@name=\"KLM\"]/"
            "Content/ScintillatorSimulationParams");
  simPar->setADCRange(d.getInt("ADCRange"));
  simPar->setADCSamplingFrequency(d.getDouble("ADCSamplingFrequency"));
  simPar->setNDigitizations(d.getInt("nDigitizations"));
  simPar->setADCPedestal(d.getDouble("ADCPedestal"));
  simPar->setADCPEAmplitude(d.getDouble("ADCPEAmplitude"));
  simPar->setADCThreshold(d.getInt("ADCThreshold"));
  simPar->setADCSaturation(d.getInt("ADCSaturation"));
  simPar->setNPEperMeV(d.getDouble("nPEperMeV"));
  simPar->setMinCosTheta(cos(d.getDouble("MaxTotalIRAngle") / 180.0 * M_PI));
  simPar->setMirrorReflectiveIndex(d.getDouble("MirrorReflectiveIndex"));
  simPar->setScintillatorDeExcitationTime(d.getDouble("ScintDeExTime"));
  simPar->setFiberDeExcitationTime(d.getDouble("FiberDeExTime"));
  simPar->setFiberLightSpeed(d.getDouble("FiberLightSpeed"));
  simPar->setAttenuationLength(d.getDouble("AttenuationLength"));
  simPar->setPEAttenuationFrequency(d.getDouble("PEAttenuationFreq"));
  simPar->setMeanSiPMNoise(d.getDouble("MeanSiPMNoise"));
  simPar->setEnableConstBkg(d.getDouble("EnableConstBkg") > 0);
  IntervalOfValidity iov(m_ExperimentLow, m_RunLow,
                         m_ExperimentHigh, m_RunHigh);
  simPar.import(iov);
}

void KLMDatabaseImporter::importTimeConversion(
  const KLMTimeConversion* timeConversion)
{
  DBImportObjPtr<KLMTimeConversion> timeConversionImport;
  timeConversionImport.construct(*timeConversion);
  IntervalOfValidity iov(m_ExperimentLow, m_RunLow,
                         m_ExperimentHigh, m_RunHigh);
  timeConversionImport.import(iov);
}

