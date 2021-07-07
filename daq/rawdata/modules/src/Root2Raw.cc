/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <daq/rawdata/modules/Root2Raw.h>

#include <daq/dataobjects/SendHeader.h>
#include <daq/dataobjects/SendTrailer.h>
#include <framework/core/Module.h>
#include <framework/core/ModuleParamBase.h>
#include <framework/datastore/StoreArray.h>
#include <framework/logging/Logger.h>
#include <rawdata/dataobjects/RawARICH.h>
#include <rawdata/dataobjects/RawCDC.h>
#include <rawdata/dataobjects/RawECL.h>
#include <rawdata/dataobjects/RawFTSW.h>
#include <rawdata/dataobjects/RawHeader_v1.h>
#include <rawdata/dataobjects/RawKLM.h>
#include <rawdata/dataobjects/RawSVD.h>
#include <rawdata/dataobjects/RawTOP.h>

#include <fcntl.h>

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


  // Fill PXD
  /*
  StoreArray<RawPXD> pxdarray;
  for (int i = 0; i < pxdarray.getEntries(); i++) {
    //  for ( int i=0; i< 1; i++ ) {
    RawPXD* pxd = pxdarray[i];
    //    printf  ( "SVD COPPER %d to be filled\n", i );
    memcpy(databuf, pxd->GetBuffer(0), pxd->GetBlockNwords(0)*sizeof(int));
    nwords += pxd->GetBlockNwords(0);
    databuf += pxd->GetBlockNwords(0);
    nblock++;
  }
  */

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

  // Fill CDC
  StoreArray<RawCDC> cdcarray;
  for (int i = 0; i < cdcarray.getEntries(); i++) {
    //  for ( int i=0; i< 1; i++ ) {
    RawCDC* cdc = cdcarray[i];
    //    printf  ( "SVD COPPER %d to be filled\n", i );
    memcpy(databuf, cdc->GetBuffer(0), cdc->GetBlockNwords(0)*sizeof(int));
    nwords += cdc->GetBlockNwords(0);
    databuf += cdc->GetBlockNwords(0);
    nblock++;
  }

  // Fill TOP
  StoreArray<RawTOP> toparray;
  for (int i = 0; i < toparray.getEntries(); i++) {
    //  for ( int i=0; i< 1; i++ ) {
    RawTOP* top = toparray[i];
    //    printf  ( "SVD COPPER %d to be filled\n", i );
    memcpy(databuf, top->GetBuffer(0), top->GetBlockNwords(0)*sizeof(int));
    nwords += top->GetBlockNwords(0);
    databuf += top->GetBlockNwords(0);
    nblock++;
  }

  // Fill ARICH
  StoreArray<RawARICH> aricharray;
  for (int i = 0; i < aricharray.getEntries(); i++) {
    //  for ( int i=0; i< 1; i++ ) {
    RawARICH* arich = aricharray[i];
    //    printf  ( "SVD COPPER %d to be filled\n", i );
    memcpy(databuf, arich->GetBuffer(0), arich->GetBlockNwords(0)*sizeof(int));
    nwords += arich->GetBlockNwords(0);
    databuf += arich->GetBlockNwords(0);
    nblock++;
  }

  // Fill ECL
  StoreArray<RawECL> eclarray;
  for (int i = 0; i < eclarray.getEntries(); i++) {
    //  for ( int i=0; i< 1; i++ ) {
    RawECL* ecl = eclarray[i];
    //    printf  ( "SVD COPPER %d to be filled\n", i );
    memcpy(databuf, ecl->GetBuffer(0), ecl->GetBlockNwords(0)*sizeof(int));
    nwords += ecl->GetBlockNwords(0);
    databuf += ecl->GetBlockNwords(0);
    nblock++;
  }

  // Fill KLM
  StoreArray<RawKLM> klmarray;
  for (int i = 0; i < klmarray.getEntries(); i++) {
    //  for ( int i=0; i< 1; i++ ) {
    RawKLM* klm = klmarray[i];
    //    printf  ( "SVD COPPER %d to be filled\n", i );
    memcpy(databuf, klm->GetBuffer(0), klm->GetBlockNwords(0)*sizeof(int));
    nwords += klm->GetBlockNwords(0);
    databuf += klm->GetBlockNwords(0);
    nblock++;
  }

  // Fill FTSW
  StoreArray<RawFTSW> ftswarray;
  for (int i = 0; i < ftswarray.getEntries(); i++) {
    //  for ( int i=0; i< 1; i++ ) {
    RawFTSW* ftsw = ftswarray[i];
    //    printf  ( "SVD COPPER %d to be filled\n", i );
    memcpy(databuf, ftsw->GetBuffer(0), ftsw->GetBlockNwords(0)*sizeof(int));
    nwords += ftsw->GetBlockNwords(0);
    databuf += ftsw->GetBlockNwords(0);
    nblock++;
  }

  /*
  // Fill FTSW
  StoreArray<RawFTSW> ftswarray;
  RawFTSW* ftsw = ftswarray[0];
  memcpy(databuf, ftsw->GetBuffer(0), ftsw->GetBlockNwords(0)*sizeof(int));
  nwords += ftsw->GetBlockNwords(0);
  databuf += ftsw->GetBlockNwords(0);
  nblock++;
  */


  //  printf ( "COPPERs filling completed\n" );

  // Fill SendHeader/Trailer

  nwords += trl.GetTrlNwords();

  //  printf ( "nwords = %d\n", nwords );

  hdr.SetNwords(nwords);
  hdr.SetNumEventsinPacket(1);
  hdr.SetNodeID(0);
  hdr.SetNumNodesinPacket(nblock);

  RawHeader_v1 rhdr;
  rhdr.SetBuffer(cdcarray[0]->GetBuffer(0));
  hdr.SetEventNumber(rhdr.GetEveNo());
  hdr.SetNodeID(0);
  hdr.SetExpRunWord(rhdr.GetExpRunSubrun());

  int* hdrbuf = hdr.GetBuffer();
  //  printf ( "%8.8x %8.8x %8.8x %8.8x ", *hdrbuf, *(hdrbuf+1), *(hdrbuf+2), *(hdrbuf+3) );
  //  printf ( "%8.8x %8.8x %8.8x %8.8x\n", *(hdrbuf+4), *(hdrbuf+5), *(hdrbuf+6), *(hdrbuf+7) );


  memcpy(evtbuf, hdr.GetBuffer(), hdr.GetHdrNwords()*sizeof(int));
  memcpy(databuf, trl.GetBuffer(), trl.GetTrlNwords()*sizeof(int));

  int is = write(m_file, evtbuf, nwords * sizeof(int));
  //  databuf = evtbuf+ hdr.GetHdrNwords();
  //  int datanwords = nwords - hdr.GetHdrNwords() - trl.GetTrlNwords();
  //  int is = write ( m_file, databuf, datanwords*sizeof(int) );
  return;

}
