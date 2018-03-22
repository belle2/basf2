/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Mikhail Remnev, Dmitry Matvienko                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <ecl/modules/eclDisplay/EclDisplayModule.h>
#include <framework/utilities/FileSystem.h>

using namespace Belle2;
using namespace ECLDisplayUtility;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(EclDisplay)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

EclDisplayModule::EclDisplayModule() : Module()
{
  // Set module properties
  setDescription("Event display module for ECL.");

  // Parameter definitions
  addParam("displayEnergy", m_displayEnergy,
           "If true, energy distribution per channel (shaper, crate) is displayed. Otherwise, number of counts is displayed", false);
  addParam("displayMode", m_displayMode,
           "Default display mode. Can be later changed in GUI.", 9);
  addParam("autoDisplay", m_autoDisplay,
           "If true, events are displayed as soon as they are loaded.", true);
  addParam("InitFileName", m_eclMapperInitFileName,
           "Initialization file for eclMapper", std::string("ecl/data/ecl_channels_map.txt"));

  m_evtNum = 0;
}

EclDisplayModule::~EclDisplayModule()
{
}

void EclDisplayModule::initialize()
{
  eclarray.isRequired();

  // Loading code from ECLUnpacker
  std::string ini_file_name = FileSystem::findFile(m_eclMapperInitFileName);
  if (!FileSystem::fileExists(ini_file_name)) {
    B2FATAL("ECL Display : eclChannelMapper initialization file " << ini_file_name << " doesn't exist");
  }
  // TODO: DB initialization is necessary.
  if (!m_mapper.initFromFile(ini_file_name.data())) {
    B2FATAL("ECL Display:: Can't initialize eclChannelMapper");
  }

  initFrame();
}

void EclDisplayModule::initFrame()
{
  SetMode(m_displayEnergy);
  m_app   = new TApplication("ECLDisplay App", 0, 0);
  m_data  = new EclData();
  m_frame = new EclFrame(m_displayMode, m_data, m_autoDisplay, &m_mapper);

  m_frame->Connect("CloseWindow()", "Belle2::EclDisplayModule", this, "handleClosedFrame()");

  m_frame_closed = false;

  B2DEBUG(100, "EclDisplayModule::create ECLFrame");
}

void EclDisplayModule::handleClosedFrame()
{
  m_frame_closed = true;
}

void EclDisplayModule::beginRun()
{
}

void EclDisplayModule::event()
{
  // EclFrame is closed, skipping data reading.
  if (m_frame_closed) return;

  int added_entries = 0;

  for (int i = 0; i < eclarray.getEntries(); i++) {
    ECLCalDigit* record = eclarray[i];
    if (record->getEnergy() >= 1e-4) { //TODO: Move to constant ENERGY_THRESHOLD.
      if (m_data->addEvent(record, m_evtNum) == 0) {
        added_entries++;
      }
    }
  }

  if (m_autoDisplay) {
    m_data->update(true);
    gSystem->ProcessEvents();
    if (!m_frame_closed)
      m_frame->loadNewData();
  }
  if (added_entries > 0)
    m_evtNum++;
}

void EclDisplayModule::endRun()
{
}

void EclDisplayModule::terminate()
{
  if (!m_frame_closed) {
    m_data->update(false);
    m_frame->loadNewData();
  }

  while (!m_frame_closed) {
    gSystem->ProcessEvents();
    gSystem->Sleep(0);
  }

  delete m_frame;
  delete m_data;
}

