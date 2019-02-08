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
#include <klm/dbobjects/KLMScintillatorDigitizationParameters.h>

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

void KLMDatabaseImporter::importScintillatorDigitizationParameters(
  const KLMScintillatorDigitizationParameters* digitizationParameters)
{
  DBImportObjPtr<KLMScintillatorDigitizationParameters> digPar;
  digPar.construct(*digitizationParameters);
  IntervalOfValidity iov(m_ExperimentLow, m_RunLow,
                         m_ExperimentHigh, m_RunHigh);
  digPar.import(iov);
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

