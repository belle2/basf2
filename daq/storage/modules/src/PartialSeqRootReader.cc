/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <daq/storage/modules/PartialSeqRootReader.h>

#include <iostream>
#include <cstdio>
#include <fcntl.h>
#include <unistd.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PartialSeqRootReader)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PartialSeqRootReaderModule::PartialSeqRootReaderModule() : Module()
{
  m_buf = new char[1024 * 1024 * 100];//100MB
  setDescription("Partial sroot file reader");

  addParam("FilePath", m_path, "File path ", std::string(""));
  addParam("FileNoMin", m_filemin, "Minimum filenumber ", 0);
  addParam("FileNoMax", m_filemax, "Maximum filenumber ", -1);
  m_fd = -1;
  m_fileno = -1;
  B2INFO("PartialSeqRootReader: Constructor done.");
}


PartialSeqRootReaderModule::~PartialSeqRootReaderModule()
{
  close(m_fd);
  delete [] m_buf;
}

int PartialSeqRootReaderModule::openFile(int fileno)
{
  if (m_fd > 0) close(m_fd);
  if (fileno > m_filemax) return -1;
  char filename[200];
  if (fileno > 0) {
    sprintf(filename, "%s-%d", m_path.c_str(), fileno);
  } else if (fileno == 0) {
    sprintf(filename, "%s", m_path.c_str());
  }
  int fd = ::open(filename, O_RDONLY);
  if (fd < 0) {
    B2ERROR("file open error (" << strerror(errno) << "): " << filename);
    return -1;
  }
  std::cout << "SeqFile: " << filename << " opened (fd=" << m_fd << ")" << std::endl;
  B2INFO("SeqFile: " << filename << " opened (fd=" << m_fd << ")");
  return fd;
}

int PartialSeqRootReaderModule::readFile()
{
  int ret = ::read(m_fd, m_buf, sizeof(int));
  if (ret < 0) {
    return ret;
  } else if (ret == 0) {
    m_fd = openFile(m_fileno++);
    if (m_fd <= 0) return -1;
    ret = ::read(m_fd, m_buf, sizeof(int));
  }
  int size = *(int*)m_buf;
  ret = ::read(m_fd, (char*)(m_buf + sizeof(int)), size - sizeof(int));
  if (ret <= 0) {
    return -1;
  }
  return size;
}

void PartialSeqRootReaderModule::initialize()
{
  B2INFO("PartialSeqRootReader: initialize() started.");
  m_streamer = new DataStoreStreamer();
  m_fd = openFile(0);
  m_fileno = m_filemin;
  while (true) {
    if (readFile() < 0) return;
    EvtMessage* evtmsg = new EvtMessage(m_buf);
    m_streamer->restoreDataStore(evtmsg);
    if (evtmsg->type() == MSG_STREAMERINFO) {
      B2INFO("Reading StreamerInfo");
      delete evtmsg;
    } else {
      delete evtmsg;
      break;
    }
  }
  if (m_fileno > 0) {
    m_fd = openFile(m_fileno++);
  } else {
    m_fileno++;
  }
  while (true) {
    if (readFile() < 0) return;
    EvtMessage* evtmsg = new EvtMessage(m_buf);
    m_streamer->restoreDataStore(evtmsg);
    if (evtmsg->type() == MSG_STREAMERINFO) {
      B2INFO("Reading StreamerInfo");
      delete evtmsg;
    } else {
      delete evtmsg;
      break;
    }
  }

  B2INFO("PartialSeqRootReader: initialize() done.");
}

void PartialSeqRootReaderModule::event()
{
  while (true) {
    if (readFile() < 0) return;
    EvtMessage* evtmsg = new EvtMessage(m_buf);
    m_streamer->restoreDataStore(evtmsg);
    if (evtmsg->type() == MSG_STREAMERINFO) {
      B2INFO("Reading StreamerInfo");
      delete evtmsg;
    } else {
      delete evtmsg;
      break;
    }
  }
}

void PartialSeqRootReaderModule::beginRun()
{
  B2INFO("PartialSeqRootReader: beginRun called.");
}

void PartialSeqRootReaderModule::endRun()
{
  B2INFO("PartialSeqRootReader: endRun done.");
}


void PartialSeqRootReaderModule::terminate()
{
  B2INFO("PartialSeqRootReader: terminate called");
}


