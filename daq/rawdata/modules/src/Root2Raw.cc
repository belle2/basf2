//+
// File : PrintData.cc
// Description : Module to get data from DataStore and send it to another network node
//
// Author : Satoru Yamada Itoh, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#include <daq/rawdata/modules/Root2Raw.h>

using namespace std;
using namespace Belle2;

//#define DEBUG

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(Root2Raw)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

Root2RawModule::Root2RawModule()
{
  addParam("outputFileName", m_filename, "Output binary filename", string("datafile.binary"));
}

Root2RawModule::~Root2RawModule()
{
}

void Root2RawModule::initialize()
{
  B2INFO("Root2Raw: initialize() started.");

  m_file = open(m_filename.c_str(), O_CREAT | O_RDWR | O_TRUNC, 0644);
  if (m_file <= 0) {
    perror("open");
    exit(-1);
  }

  B2INFO("Root2Raw: initialize() done.");
}

void Root2RawModule::endRun()
{
  //fill Run data
  //  fclose( m_fp_out );
  B2INFO("endRun done.");
}


void Root2RawModule::terminate()
{
  close(m_file);
  B2INFO("terminate called");
}


void Root2RawModule::event()
{


  B2INFO("Root2Raw: event() started.");

  // Prepare SendHeader
  SendHeader hdr;
  hdr.Initialize();
  SendTrailer trl;
  trl.Initialize();

  int* evtbuf = (int*) malloc(MAXEVTSIZE);
  int* databuf = evtbuf + hdr.GetHdrNwords();

  int nwords = hdr.GetHdrNwords();
  int nblock = 0;


  // Fill SVD
  StoreArray<RawSVD> svdarray;
  for (int i = 0; i < svdarray.getEntries(); i++) {
    //  for ( int i=0; i< 1; i++ ) {
    RawSVD* svd = svdarray[i];
    //    printf  ( "SVD COPPER %d to be filled\n", i );
    memcpy(databuf, svd->GetBuffer(0), svd->GetBlockNwords(0)*sizeof(int));
    nwords += svd->GetBlockNwords(0);
    databuf += svd->GetBlockNwords(0);
    nblock++;
  }

  // Fill FTSW
  StoreArray<RawFTSW> ftswarray;
  RawFTSW* ftsw = ftswarray[0];
  memcpy(databuf, ftsw->GetBuffer(0), ftsw->GetBlockNwords(0)*sizeof(int));
  nwords += ftsw->GetBlockNwords(0);
  databuf += ftsw->GetBlockNwords(0);
  nblock++;


  //  printf ( "COPPERs filling completed\n" );

  // Fill SendHeader/Trailer

  nwords += trl.GetTrlNwords();

  //  printf ( "nwords = %d\n", nwords );

  hdr.SetNwords(nwords);
  hdr.SetNumEventsinPacket(1);
  hdr.SetNodeID(0);
  hdr.SetNumNodesinPacket(nblock);
  RawHeader rhdr;
  rhdr.SetBuffer(svdarray[0]->GetBuffer(0));
  hdr.SetEventNumber(rhdr.GetEveNo());
  hdr.SetNodeID(0);
  hdr.SetExpRunWord(rhdr.GetExpRunNumberWord());

  memcpy(evtbuf, hdr.GetBuffer(), hdr.GetHdrNwords()*sizeof(int));
  memcpy(databuf, trl.GetBuffer(), trl.GetTrlNwords()*sizeof(int));

  int is = write(m_file, evtbuf, nwords * sizeof(int));
  //  databuf = evtbuf+ hdr.GetHdrNwords();
  //  int datanwords = nwords - hdr.GetHdrNwords() - trl.GetTrlNwords();
  //  int is = write ( m_file, databuf, datanwords*sizeof(int) );
  return;

}
