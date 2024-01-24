/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <daq/storage/modules/SeqRootMergerModule.h>

#include <framework/core/Environment.h>

#include <cstdlib>
#include <cstdio>
#include <fcntl.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SeqRootMerger);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SeqRootMergerModule::SeqRootMergerModule() : Module()
{
  B2INFO("SeqRootMerger: Constructor done.");
}


SeqRootMergerModule::~SeqRootMergerModule()
{

}

void SeqRootMergerModule::initialize()
{
  const std::vector<std::string>& inputFiles = Environment::Instance().getInputFilesOverride();
  for (size_t i = 0; i < inputFiles.size(); i++) {
    m_file.push_back(new SeqFile(inputFiles[i].c_str(), "r"));
  }
  m_streamer = new DataStoreStreamer();
  readFile();
}

int SeqRootMergerModule::readFile()
{
  B2INFO("SeqRootMerger: initialize() started.");

  char* evtbuf = new char[EvtMessage::c_MaxEventSize];
  while (true) {
    if (m_file.size() == 0) return 0;
    int i = rand() % m_file.size();
    int size = m_file[i]->read(evtbuf, EvtMessage::c_MaxEventSize);
    if (size > 0) {
      EvtMessage* evtmsg = new EvtMessage(evtbuf);
      m_streamer->restoreDataStore(evtmsg);
      if (evtmsg->type() == MSG_STREAMERINFO) {
        B2INFO("Reading StreamerInfo");
        delete evtmsg;
      } else {
        delete evtmsg;
        break;
      }
    } else {
      m_file.erase(m_file.begin() + i);
    }
  }
  delete [] evtbuf;

  return m_file.size();
}

void SeqRootMergerModule::event()
{
  readFile();
}

void SeqRootMergerModule::beginRun()
{
  B2INFO("SeqRootMerger: beginRun called.");
}

void SeqRootMergerModule::endRun()
{
  B2INFO("SeqRootMerger: endRun done.");
}


void SeqRootMergerModule::terminate()
{
  B2INFO("SeqRootMerger: terminate called");
}


