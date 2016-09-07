/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Remnev Mikhail                                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <ecl/modules/eclDisplay/EclDisplayModule.h>
#include <ecl/modules/eclDisplay/EclData.h>
#include <ecl/modules/eclDisplay/EclFrame.h>
#include <TStyle.h>

using namespace Belle2;

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
  setDescription("DQM and event display module for ECL.");

  // Parameter definitions
  addParam("displayEnergy", m_displayEnergy,
           "If true, energy distribution per channel (shaper, crate) is displayed. Otherwise, number of counts is displayed", false);
  addParam("displayMode", m_displayMode,
           "Default display mode. Can be later changed in GUI.", 5);
  addParam("autoDisplay", m_autoDisplay,
           "If true, events are displayed as soon as they are loaded.", true);


}

void EclDisplayModule::startThread(int in)
{
  if (fork() == 0) {
    char buf[1024];
    int temp[3];
    read(in, buf, 1024);
    sscanf(buf, "%d %d %d", &temp[0], &temp[1], &temp[2]);
    m_displayEnergy = temp[0];
    m_displayMode   = temp[1];
    m_autoDisplay   = temp[2];
    fcntl(in, F_SETFL, O_NONBLOCK, 1);

    TApplication app("tapp", 0, 0);

    SetMode(m_displayEnergy);

    EclData* data = new EclData(in);
    // Hide statistics box.
    gStyle->SetOptStat(0);

    EclFrame* frame = new EclFrame(m_displayMode, data, m_autoDisplay);

    frame->Connect("CloseWindow()", "TApplication", &app, "Terminate()");
    app.Run();
    //printf("Application terminated\n");
    gSystem->Exit(0);
  }
}

EclDisplayModule::~EclDisplayModule()
{
}

void EclDisplayModule::initialize()
{
  char buf[1024];
  int len = snprintf(buf, 1024, "%d %d %d", m_displayEnergy, m_displayMode, m_autoDisplay);
  write(m_out, buf, len + 1);

  int filedes[2];
  pipe(filedes);
  m_out = filedes[1];
  startThread(filedes[0]);

}

void EclDisplayModule::beginRun()
{
}

void EclDisplayModule::event()
{
  StoreArray<RawECL> raw_eclarray;

  // printf("raw_eclarray.getEntries() == %d\n", raw_eclarray.getEntries());

  AddEclCluster(raw_eclarray);
}

void EclDisplayModule::terminate()
{
}

void EclDisplayModule::serializeAndSend(int ch, int amp, int time, int evtn)
{
  char buf[256];
  int len = snprintf(buf, 256, "!%d %d %d %d ", ch, amp, time, evtn);
  write(m_out, buf, len + 1);
}

void EclDisplayModule::ReadEclEvent(RawCOPPER* raw_copper, int evt_num)
{
  int shnum, buf_length;
  int dspnum;
  int pos, ich; // position in buffer, channel iterator
  int ID;//current channel mask, copper ID;
  int channel, time, amp, evtn;
  int shaper;

  unsigned int evnum = raw_copper->GetEveNo(evt_num);
  ID = (int)raw_copper->GetNodeID(evt_num) - 83886081 + 1;// some magic

  for (int finesse_num = 0; finesse_num < 4; finesse_num++) {
    buf_length = raw_copper->GetDetectorNwords(evt_num, finesse_num);
    if (buf_length <= 0) {
      continue;
    }
    int* buf = raw_copper->GetDetectorBuffer(evt_num, finesse_num);
    pos = 0;//position of the COLLECTOR header (1 word)
    if (pos == buf_length)continue;

    for (shnum = 0; shnum < 12; shnum++) {
      pos++;
      if (pos == buf_length)continue;
      pos++;
      if (pos == buf_length)continue;
      shaper = (shnum + 1) + (ID - 1) * 24 + finesse_num * 12;
      dspnum = (buf[pos] >> 8) & 0x1F;
      evtn = evnum;
      pos++;
      if (pos == buf_length) continue;
      pos++;
      if (pos == buf_length) continue;

      for (ich = 0; ich < dspnum; ich++) {
        channel = (shaper - 1) * 16 + ich;
        pos++;
        if (pos == buf_length) continue;
        time = (buf[pos] >> 18) & 0xFFF;
        amp  = (buf[pos] & 0x3FFFF) - 128;
        if ((amp > 50 && amp < 3000) || amp == -128)
          serializeAndSend(channel, amp, time, evtn);
      }
    }
  }
}

void EclDisplayModule::AddEclCluster(StoreArray<RawECL>& raw_eclarray)
{
  for (int i = 0; i < raw_eclarray.getEntries(); i++) {
    for (int j = 0; j < raw_eclarray[i]->GetNumEntries(); j++) {
      ReadEclEvent(raw_eclarray[ i ], j);
    }
  }
  gSystem->Sleep(0);
}

