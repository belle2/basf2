//
// version of camacConverter used for 2014 KEK cosmic ray test data
//
//   LeCroy 2228A in slot 6, T0 (FTSW) data in channel 5
//

#include <topcaf/modules/AsciiCamacModule/AsciiCamacModule.h>

#include <topcaf/dataobjects/EventHeaderPacket.h>

#include <cstring>
#include <iostream>
#include <iomanip>
#include <vector>
#include "stdio.h"

using namespace Belle2;

REG_MODULE(AsciiCamac);
//Constructor
AsciiCamacModule::AsciiCamacModule() : Module(), m_c3377nhit(0)
{

  setDescription("This module is used to read auxliary ASCII CAMAC data for itop CRT tests from Summer 2015.  Needed to get the FTSW timing for GIGE data.");

  addParam("inputFilename", m_in_filename, "The input .cmc file to be parsed.");
  addParam("tdcUnits", m_ftswTDCtiming, "The units for the TDC values [ns].", 0.0469);

}

//Destructor
AsciiCamacModule::~AsciiCamacModule() {}

void AsciiCamacModule::initialize()
{

  m_camac_data.registerInDataStore();

  LoadCamacFile();

}

void AsciiCamacModule::beginRun()
{

}

void AsciiCamacModule::event()
{
  m_camac_data.create();

  StoreObjPtr<EventHeaderPacket> evtheader_ptr;
  evtheader_ptr.isRequired();
  if (evtheader_ptr) {
    //    B2INFO("ftsw["<<evtheader_ptr->GetEventNumber()<<"]: "<<m_ftsw_map[evtheader_ptr->GetEventNumber()]);
    evtheader_ptr->SetFTSW((m_ftsw_map[evtheader_ptr->GetEventNumber()]*m_ftswTDCtiming));

  }
}

void AsciiCamacModule::terminate()
{
}

//Load file
int AsciiCamacModule::LoadCamacFile()
{
  m_input_file.open(m_in_filename.c_str());
  if (!m_input_file) {
    B2ERROR("Couldn't open input file: " << m_in_filename);
    return -9;
  }  else {
    int ftsw_phase = 0;
    int event_no = 0;
    int c = 0;
    while (m_input_file >> event_no) {
      m_input_file >> ftsw_phase;
      c++;
      //      B2INFO("ftsw data ("<<event_no<<","<<ftsw_phase<<")");
      //      m_ftsw_map[event_no] = ftsw_phase;
      m_ftsw_map[c] = ftsw_phase;
    }
    return 0;
  }

}

