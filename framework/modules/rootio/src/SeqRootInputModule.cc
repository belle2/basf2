//+
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 13 - Aug - 2010
//         6 - Sep - 2012     modified to use DataStoreStreamer, clean up
//-

#include <framework/modules/rootio/SeqRootInputModule.h>

#include <framework/core/Environment.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/io/RootIOUtilities.h>

#include <cmath>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SeqRootInput)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SeqRootInputModule::SeqRootInputModule() : Module(), m_streamer(nullptr), m_size(0), m_size2(0)
{
  //Set module properties
  setDescription("Read .sroot files produced by SeqRootOutput.");
  setPropertyFlags(c_Input);

  m_file = 0;
  m_nevt = -1;

  //Parameter definition
  addParam("inputFileName"  , m_inputFileName,
           "Input file name. Can also be a gzip-compressed file (with suffix .gz). "
           "Parameter can be overridden using the -i argument to basf2.",
           string(""));
  vector<string> empty;
  addParam("inputFileNames", m_filelist, "List of input files", empty);
  addParam("fileNameIsPattern", m_fileNameIsPattern, "If true interpret the output "
           "filename as a boost::format pattern instead of the standard where "
           "subsequent files are named .sroot-N. For example 'myfile-f%08d.sroot'",
           false);
}


SeqRootInputModule::~SeqRootInputModule()
{
}

void SeqRootInputModule::initialize()
{
  RootIOUtilities::loadDictionaries();

  // Specify input file(list)
  if (!m_inputFileName.empty() && !m_filelist.empty()) {
    B2FATAL("Cannot specify both 'inputFileName' and 'inputFileNames'");
  }
  const std::vector<std::string>& inputFiles = Environment::Instance().getInputFilesOverride();
  if (!inputFiles.empty()) {  // Override parameter specification
    if (inputFiles.size() > 1) {
      m_filelist = inputFiles;
    }
    m_inputFileName = inputFiles[0];
    m_nfile = m_filelist.size();
  } else if (m_filelist.size() > 0) {
    m_nfile = m_filelist.size();
    m_inputFileName = m_filelist[0];
  } else
    m_nfile = 1;

  // Initialize DataStoreStreamer
  m_streamer = new DataStoreStreamer();

  // Read the first event in SeqRoot file and restore in DataStore.
  // This is necessary to create object tables before TTree initialization
  // if used together with TTree based output (RootOutput module).

  EvtMessage* evtmsg = NULL;
  // Open input file
  m_file = new SeqFile(m_inputFileName.c_str(), "r", nullptr, 0, m_fileNameIsPattern);
  if (m_file->status() <= 0)
    B2FATAL("SeqRootInput : Error in opening input file : " << m_inputFileName);

  B2INFO("SeqRootInput : Open " << m_inputFileName);

  //Read StreamerInfo and the first event
  int info_cnt = 0;
  while (true) {
    char* evtbuf = new char[EvtMessage::c_MaxEventSize];
    int size = m_file->read(evtbuf, EvtMessage::c_MaxEventSize);
    if (size > 0) {
      evtmsg = new EvtMessage(evtbuf);
      m_streamer->restoreDataStore(evtmsg);
      if (evtmsg->type() == MSG_STREAMERINFO) {
        // StreamerInfo was read
        B2INFO("Reading StreamerInfo");
        if (info_cnt != 0) B2FATAL("SeqRootInput : Reading StreamerInfos twice");
        info_cnt++;
      } else {
        // first event was read
        delete[] evtbuf;
        delete evtmsg;
        break;
      }
      delete[] evtbuf;
      delete evtmsg;

    } else {
      B2FATAL("SeqRootInput : Error in reading first event");
    }
  }
  m_fileptr = 0;


  B2INFO("SeqRootInput: initialized.");
}


void SeqRootInputModule::beginRun()
{
  gettimeofday(&m_t0, 0);
  m_size = 0.0;
  m_size2 = 0.0;
  m_nevt = 0;
  B2INFO("SeqRootInput: beginRun called.");
}


void SeqRootInputModule::event()
{
  m_nevt++;
  // First event is already loaded
  if (m_nevt == 0) return;

  // Get a SeqRoot record from the file
  char* evtbuf = new char[EvtMessage::c_MaxEventSize];
  EvtMessage* evtmsg = NULL;
  int size = m_file->read(evtbuf, EvtMessage::c_MaxEventSize);
  if (size < 0) {
    B2ERROR("SeqRootInput : file read error");
    delete m_file;
    m_file = 0;
    delete[] evtbuf;
    evtbuf = NULL;
    return;
  } else if (size == 0) {
    B2INFO("SeqRootInput : EOF detected");
    delete m_file;
    m_file = 0;
    m_fileptr++;
    if (m_fileptr >= m_nfile) {
      delete[] evtbuf;
      evtbuf = NULL;
      return;
    }
    printf("fileptr = %d ( of %d )\n", m_fileptr, m_nfile);
    fflush(stdout);
    m_inputFileName = m_filelist[m_fileptr];
    m_file = new SeqFile(m_inputFileName, "r");
    if (m_file->status() <= 0)
      B2FATAL("SeqRootInput : Error in opening input file : " << m_inputFileName);
    B2INFO("SeqRootInput : Open " << m_inputFileName);
    evtmsg = new EvtMessage(evtbuf);
    // Skip the first record (StreamerInfo)
    int is = m_file->read(evtbuf, EvtMessage::c_MaxEventSize);
    if (is <= 0) {
      B2FATAL("SeqRootInput : Error in reading file. error code = " << is);
    }
    // Read next record
    is = m_file->read(evtbuf, EvtMessage::c_MaxEventSize);
    if (is <= 0) {
      B2FATAL("SeqRootInput : Error in reading file. error code = " << is);
    }
  } else {
    //    printf("SeqRootInput : read = %d\n", size);
    evtmsg = new EvtMessage(evtbuf);
  }

  // Statistics
  double dsize = (double)size / 1000.0;
  m_size += dsize;
  m_size2 += dsize * dsize;

  if (evtmsg->type() == MSG_STREAMERINFO) {
    B2WARNING("SeqRootInput : StreamerInfo is found in the middle of *.sroot-* files. Skip record");
    int is = m_file->read(evtbuf, EvtMessage::c_MaxEventSize);
    if (is <= 0) {
      B2FATAL("SeqRootInput : Error in reading file. error code = " << is);
    }
    evtmsg = new EvtMessage(evtbuf);
  }

  // Restore objects in DataStore
  m_streamer->restoreDataStore(evtmsg);

  // Delete buffers
  delete[] evtbuf;
  evtbuf = NULL;
  delete evtmsg;
  evtmsg = NULL;
}

void SeqRootInputModule::endRun()
{
  // End time
  gettimeofday(&m_tend, 0);
  double etime = (double)((m_tend.tv_sec - m_t0.tv_sec) * 1000000 +
                          (m_tend.tv_usec - m_t0.tv_usec));

  // Statistics
  // Sigma^2 = Sum(X^2)/n - (Sum(X)/n)^2

  double flowmb = m_size / etime * 1000.0;
  double evrate = (double)m_nevt / (etime / 1000.0);
  double avesize = m_size / (double)m_nevt;
  double avesize2 = m_size2 / (double)m_nevt;
  double sigma2 = avesize2 - avesize * avesize;
  double sigma = sqrt(sigma2);

  //  printf ( "m_size = %f, m_size2 = %f, m_nevt = %d\n", m_size, m_size2, m_nevt );
  //  printf ( "avesize2 = %f, avesize = %f, avesize*avesize = %f\n", avesize2, avesize, avesize*avesize );
  B2INFO("SeqRootInput :  " << m_nevt << " events read with total bytes of " << m_size << " kB");
  B2INFO("SeqRootInput : event rate = " << evrate << " (KHz)");
  B2INFO("SeqRootInput : flow rate = " << flowmb << " (MB/s)");
  B2INFO("SeqRootInput : event size = " << avesize << " +- " << sigma << " (kB)");

  B2INFO("SeqRootInput: endRun done.");
}


void SeqRootInputModule::terminate()
{
  delete m_streamer;
  delete m_file;
  B2INFO("SeqRootInput: terminate called");
}

