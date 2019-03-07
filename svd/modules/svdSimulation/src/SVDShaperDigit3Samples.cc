/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luigi Corona                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <svd/simulation/SVDShaperDigit3Samples.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SVDShaperDigit3Samples)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SVDShaperDigit3SamplesModule::SVDShaperDigit3SamplesModule() : Module()
{
  B2DEBUG(1, "Constructor");
  // Set module properties
  setDescription(" ");

  // Parameter definitions
  addParam("SVDShaperDigits", m_shaperDigitInput, "StoreArray with the input shaperdigits", std::string("SVDShaperDigits"));
  addParam("StartingSample", m_startingSample, "Starting sample of the three samples (between 0 and 3, the default is 3)", 0);
  addParam("outputArrayName", m_outputArrayName, "StoreArray with the output shaperdigits with 3 samples",
           std::string("SVDShaperDigit3Samples"));
}

SVDShaperDigit3SamplesModule::~SVDShaperDigit3SamplesModule()
{
  B2DEBUG(20, "Destructor");
}


void SVDShaperDigit3SamplesModule::initialize()
{
  if (m_startingSample > 3 || m_startingSample < 0) {
    B2FATAL("The starting sample must be between 0 and 3, you set = " << m_startingSample);
    return;
  }

  B2DEBUG(10, "SVDShaperDigits: " << m_shaperDigitInput);
  B2DEBUG(10, "StartingSample: " << m_startingSample);
  B2DEBUG(10, "outputArrayName: " <<  m_outputArrayName);

  StoreArray<SVDShaperDigit> ShaperDigits(m_shaperDigitInput);
  ShaperDigits.isRequired();
  StoreArray<SVDShaperDigit> ShaperDigit3Samples(m_outputArrayName);
  ShaperDigit3Samples.registerInDataStore();

}


void SVDShaperDigit3SamplesModule::beginRun()
{
}


void SVDShaperDigit3SamplesModule::event()
{
  StoreArray<SVDShaperDigit> ShaperDigit3Samples(m_outputArrayName);
  StoreArray<SVDShaperDigit> ShaperDigits(m_shaperDigitInput);

  for (const SVDShaperDigit& shaper : ShaperDigits) {

    SVDModeByte modeByte = shaper.getModeByte();
    //int DAQMode = modeByte.getDAQMode();
    //B2INFO("DAQMODE: " << DAQMode);
    //B2INFO("MODEBYTE: " << modeByte);
    modeByte.setDAQMode(1);
    Belle2::SVDShaperDigit::APVFloatSamples samples = shaper.getSamples();
    VxdID sensorID = shaper.getSensorID();
    bool side = shaper.isUStrip();
    int cellID = shaper.getCellID();
    int8_t fadcT = shaper.getFADCTime();

    Belle2::SVDShaperDigit::APVFloatSamples threeSamples;

    threeSamples[0] = samples[m_startingSample];
    threeSamples[1] = samples[m_startingSample + 1];
    threeSamples[2] = samples[m_startingSample + 2];
    threeSamples[3] = 0.;
    threeSamples[4] = 0.;
    threeSamples[5] = 0.;

    ShaperDigit3Samples.appendNew(sensorID, side, cellID, threeSamples, fadcT, modeByte);
    //SVDShaperDigit* shaperThree = ShaperDigit3Samples.appendNew(sensorID, side, cellID, threeSamples, fadcT, modeByte);

    //SVDModeByte threeModeByte = shaperThree->getModeByte();
    //int daq = threeModeByte.getDAQMode();
    //B2INFO("MODEBYTE: " << threeModeByte);
    //B2INFO("DAQMODE: " << daq);

  }
}

void SVDShaperDigit3SamplesModule::endRun()
{
}

void SVDShaperDigit3SamplesModule::terminate()
{
}
