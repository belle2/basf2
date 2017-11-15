//+
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 13 - Aug - 2010
//         6 - Sep - 2012,  Use of DataStoreStreamer, clean up
//-

#include <framework/modules/rootio/SeqRootOutputModule.h>
#include <framework/datastore/DataStore.h>
#include <framework/core/Environment.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <cmath>
#include <fcntl.h>

#include <TClass.h>
#include <TList.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SeqRootOutput)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SeqRootOutputModule::SeqRootOutputModule() : Module(), m_nevt(0), m_streamer(nullptr), m_size(0), m_size2(0)
{
  //Set module properties
  setDescription("Save a sequential ROOT file (non-standard I/O format used in DAQ). See https://confluence.desy.de/display/BI/Software+PersistencyModules for further information and a comparison with the .root format.");
  m_file = 0;
  m_msghandler = 0;
  m_streamerinfo = NULL;
  m_streamerinfo_size = 0;

  vector<string> emptyvector;
  //Parameter definition
  addParam("outputFileName"  , m_outputFileName,
           "Output file name. Add a .gz suffix to save a gzip-compressed file. Parameter can be overridden using the -o argument to basf2.",
           string("SeqRootOutput.sroot"));
  addParam("compressionLevel", m_compressionLevel,
           "Compression Level: 0 for no, 1 for low, 9 for high compression. Level 1 usually reduces size by 50%, higher levels have no noticable effect. NOTE: Because of a ROOT bug ( https://sft.its.cern.ch/jira/browse/ROOT-4550 ), this option currently causes memory leaks and is disabled.",
           0);
  addParam("saveObjs", m_saveObjs, "List of objects/arrays to be saved", emptyvector);
  addParam("fileNameIsPattern", m_fileNameIsPattern, "If true interpret the output filename as a boost::format pattern "
           "instead of the standard where subsequent files are named .sroot-N. For example 'myfile-f%08d.sroot'", false);
}


SeqRootOutputModule::~SeqRootOutputModule()
{
  if (m_streamerinfo != NULL) delete m_streamerinfo;
}

void SeqRootOutputModule::initialize()
{
  const std::string& outputFileArgument = Environment::Instance().getOutputFileOverride();
  if (!outputFileArgument.empty())
    m_outputFileName = outputFileArgument;

  // Open output file


  // Message handler to encode serialized object
  m_msghandler = new MsgHandler(m_compressionLevel);

  // DataStoreStreamer
  m_streamer = new DataStoreStreamer(m_compressionLevel);
  m_streamer->setStreamingObjects(m_saveObjs);

  //Write StreamerInfo at the beginning of a file
  getStreamerInfos();

  m_file = new SeqFile(m_outputFileName.c_str(), "w", m_streamerinfo, m_streamerinfo_size, m_fileNameIsPattern);

  B2INFO("SeqRootOutput: initialized.");
}


void SeqRootOutputModule::beginRun()
{

  // Statistics
  gettimeofday(&m_t0, 0);
  m_size = 0.0;
  m_size2 = 0.0;
  m_nevt = 0;

  B2INFO("SeqRootOutput: beginRun called.");
}

void SeqRootOutputModule::event()
{
  // Stream DataStore in EvtMessage
  EvtMessage* msg = m_streamer->streamDataStore(false);

  // Store EvtMessage
  int stat = m_file->write(msg->buffer());

  // Clean up EvtMessage
  delete msg;

  // Statistics
  double dsize = (double)stat / 1000.0;
  m_size += dsize;
  m_size2 += dsize * dsize;
  m_nevt++;
}

void SeqRootOutputModule::endRun()
{
  //fill Run data

  // End time
  gettimeofday(&m_tend, 0);
  double etime = (double)((m_tend.tv_sec - m_t0.tv_sec) * 1000000 +
                          (m_tend.tv_usec - m_t0.tv_usec));

  // Statistics
  // Sigma^2 = Sum(X^2)/n - (Sum(X)/n)^2

  double flowmb = m_size / etime * 1000.0;
  double avesize = m_size / (double)m_nevt;
  double avesize2 = m_size2 / (double)m_nevt;
  double sigma2 = avesize2 - avesize * avesize;
  double sigma = sqrt(sigma2);

  B2INFO("SeqRootOutput :  " << m_nevt << " events written with total bytes of " << m_size << " kB");
  B2INFO("SeqRootOutput : flow rate = " << flowmb << " (MB/s)");
  B2INFO("SeqRootOutput : event size = " << avesize << " +- " << sigma << " (kB)");

  B2INFO("SeqRootOutput: endRun done.");
}


void SeqRootOutputModule::terminate()
{
  delete m_msghandler;
  delete m_streamer;
  delete m_file;

  B2INFO("terminate called");
}


void SeqRootOutputModule::getStreamerInfos()
{
  //
  // Write StreamerInfo to a file
  // Copy from RootOutputModule::initialize() and TSocket::SendStreamerInfos()
  //

  if (!m_msghandler) {
    B2FATAL("DataStoreStreamer : m_msghandler is NULL.");
    return;
  }

  TList* minilist = 0 ;
  for (int durability = 0; durability < DataStore::c_NDurabilityTypes; durability++) {
    DataStore::StoreEntryMap& map = DataStore::Instance().getStoreEntryMap(DataStore::EDurability(durability));

    for (DataStore::StoreEntryIter iter = map.begin(); iter != map.end(); ++iter) {
      const TClass* entryClass = iter->second.objClass;
      TVirtualStreamerInfo* vinfo = entryClass->GetStreamerInfo();
      B2INFO("Recording StreamerInfo : durability " << durability << " : Class Name " << entryClass->GetName());
      if (!minilist) minilist  =  new TList();
      minilist->Add((TObject*)vinfo);
    }
  }

  if (minilist) {
    //       TMessage messinfo(kMESS_STREAMERINFO);
    //       messinfo.WriteObject(minilist);
    m_msghandler->add(minilist, "StreamerInfo");
    //      EvtMessage* msg = m_msghandler->encode_msg(MSG_EVENT);
    EvtMessage* msg = m_msghandler->encode_msg(MSG_STREAMERINFO);
    (msg->header())->nObjects = 1;       // No. of objects
    (msg->header())->nArrays = 0;    // No. of arrays

    //    int size = m_file->write(msg->buffer());
    m_streamerinfo_size = *((int*)(msg->buffer()));     // nbytes in the buffer at the beginning

    //copy the steamerINfo for later use
    if (m_streamerinfo_size > 0) {
      B2INFO("Get StreamerInfo from DataStore : " << m_streamerinfo_size << "bytes");
      if (m_streamerinfo != NULL) {
        B2FATAL("getStreamerInfo() is called twice in the same run ");
      } else {
        m_streamerinfo = new char[ m_streamerinfo_size ];
      }
      memcpy(m_streamerinfo, msg->buffer(), m_streamerinfo_size);
    } else {
      B2FATAL("Invalid size of StreamerInfo : " << m_streamerinfo_size << "bytes");
    }
    delete minilist;
    delete msg;
  } else {
    B2FATAL("Failed to get StreamerInfo : ");
  }

  return;
}
