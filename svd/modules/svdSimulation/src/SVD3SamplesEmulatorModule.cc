/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luigi Corona, Giulia Casarosa                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <svd/modules/svdSimulation/SVD3SamplesEmulatorModule.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SVD3SamplesEmulator)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SVD3SamplesEmulatorModule::SVD3SamplesEmulatorModule() : Module()
{
  B2DEBUG(1, "Constructor");
  // Set module properties
  setDescription("This module takes the SVDShaperDigit as input and select three consecutive samples starting from the one choosen by the user. The modules creates a new StoreArray of the class ShaperDigit whit three samples only, selected from the original ShaperDigits. The three samples are stored in the first three positions of the APVSamples store array, and the last three are set to 0.");

  // Parameter definitions
  addParam("SVDShaperDigits", m_shaperDigitInputName, "StoreArray with the input shaperdigits", std::string("SVDShaperDigits"));
  addParam("StartingSample", m_startingSample, "Starting sample of the three samples (between 0 and 3, the default is 0)", int(0));
  addParam("outputSVDShaperDigits", m_outputArrayName, "StoreArray with the output shaperdigits with 3 samples",
           std::string("SVDShaperDigit3Samples"));
  addParam("SVDEventInfo", m_svdEventInfoName, "input SVDEventInfo name", std::string(""));
  addParam("outputSVDEventInfo", m_svdEventInfoOutName, "output SVDEventInfo name", std::string("SVDEventInfo3Samples"));
}

SVD3SamplesEmulatorModule::~SVD3SamplesEmulatorModule()
{
}


void SVD3SamplesEmulatorModule::initialize()
{
  if (m_startingSample > 3 || m_startingSample < 0) {
    B2FATAL("The starting sample must be between 0 and 3, you set = " << m_startingSample);
    return;
  }

  B2DEBUG(10, "SVDShaperDigits: " << m_shaperDigitInputName);
  B2DEBUG(10, "StartingSample: " << m_startingSample);
  B2DEBUG(10, "outputSVDShaperDigits: " <<  m_outputArrayName);
  B2DEBUG(10, "outputSVDEventInfo: " <<  m_svdEventInfoOutName);

  B2INFO("The starting sample from which start to select the three samples:  " << m_startingSample);
  B2INFO("The three samples selected are: " << m_startingSample << " " << m_startingSample + 1 << " " << m_startingSample + 2);

  m_ShaperDigit.isRequired(m_shaperDigitInputName);
  StoreArray<SVDShaperDigit> ShaperDigit3Samples(m_outputArrayName);
  ShaperDigit3Samples.registerInDataStore();

  if (!m_storeSVDEvtInfo.isOptional(m_svdEventInfoName)) m_svdEventInfoName = "SVDEventInfoSim";
  m_storeSVDEvtInfo.isRequired(m_svdEventInfoName);

  //Register the new EventInfo with ModeByte for 3 samples in the data store
  m_storeSVDEvtInfo3samples.registerInDataStore(m_svdEventInfoOutName, DataStore::c_ErrorIfAlreadyRegistered);
}


void SVD3SamplesEmulatorModule::beginRun()
{
}


void SVD3SamplesEmulatorModule::event()
{
  if (!m_storeSVDEvtInfo.isValid()) B2ERROR("No valid SVDEventInfo object is present!");

  SVDModeByte modeByte = m_storeSVDEvtInfo->getModeByte();
  StoreArray<SVDShaperDigit> ShaperDigit3Samples(m_outputArrayName);

  int DAQMode = modeByte.getDAQMode();
  if (DAQMode != 2) {
    B2FATAL("The DAQMode is = " << DAQMode << " The number of samples of the input shaperdigits is NOT 6!");
    return;
  }

  m_storeSVDEvtInfo3samples.create();
  m_storeSVDEvtInfo3samples->setTriggerType(m_storeSVDEvtInfo->getTriggerType());
  modeByte.setDAQMode(int(1));
  m_storeSVDEvtInfo3samples->setModeByte(modeByte);


  for (const SVDShaperDigit& shaper : m_ShaperDigit) {

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

  }
}

void SVD3SamplesEmulatorModule::endRun()
{
}

void SVD3SamplesEmulatorModule::terminate()
{
}
