/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : MakeDUmHSLBData.h
// Description : Module to make a binary file from Raw*** events for input of wirte-dumhsb
//
// Author : Satoru Yamada, IPNS, KEK
// Date : Nov. 22, 2015
//-
//#include <daq/rawdata/modules/DAQConsts.h>

#include <rawdata/dataobjects/RawPXD.h>
#include <rawdata/modules/MakeDumHSLBData.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>


using namespace std;
using namespace Belle2;

//#define DEBUG

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(MakeDumHSLBData)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

MakeDumHSLBDataModule::MakeDumHSLBDataModule() : PrintDataTemplateModule()
{
  addParam("OutputFileName", m_out_fname, "Name of an output file", string("dumhslb.dat"));
  m_filefd = 0;
}

MakeDumHSLBDataModule::~MakeDumHSLBDataModule()
{
}

void MakeDumHSLBDataModule::initialize()
{
  B2INFO("MakeDumHSLBData: initialize() started.");

  struct stat statbuf;
  m_filefd = open(m_out_fname.c_str(), O_WRONLY | O_CREAT);
  if (m_filefd < 0) {
    printf("Error : cannot open %s: %s\n", m_out_fname.c_str(), strerror(errno));
    exit(1);
  }
  if (fstat(m_filefd, &statbuf) < 0) {
    perror("fstat");
    exit(1);
  }

  B2INFO("MakeDumHSLBData: initialize() done.");

}

void MakeDumHSLBDataModule::writeData(RawCOPPER* raw_copper, int i)
{
  unsigned int eve_num = raw_copper->GetEveNo(i);
  unsigned int cpr_id = raw_copper->GetNodeID(i);
  for (int j = 0 ; j < 4; j++) {
    int nwords = raw_copper->GetDetectorNwords(i, j);
    if (nwords > 0) {
      printf("===== Detector Buffer(FINESSE A) 0x%x words \n", raw_copper->GetDetectorNwords(i, 0));
      int header = 0xCAFEBABE;
      int len = 0;
      if ((len = write(m_filefd, &header, sizeof(int))) != sizeof(int)) {
        perror("write error");
        exit(1);
      }
      if ((len = write(m_filefd, &eve_num, sizeof(int))) != sizeof(int)) {
        perror("write error");
        exit(1);
      }
      if ((len = write(m_filefd, &cpr_id, sizeof(int))) != sizeof(int)) {
        perror("write error");
        exit(1);
      }
      if ((len = write(m_filefd, &nwords, sizeof(int))) != sizeof(int)) {
        perror("write error");
        exit(1);
      }
      write(m_filefd, raw_copper->GetDetectorBuffer(i, 0), sizeof(int)*nwords);
      printf("hdr 0x%.8x eve %10u cprid 0x%.8x nwrods %d\n", header, eve_num, cpr_id, nwords);
    }
  }
}

void MakeDumHSLBDataModule::event()
{


  B2INFO("MakeDumHSLBData: event() started.");
  //
  //  Data from COPPER ( data from any detectors(e.g. CDC, SVD, ... ))
  //
  StoreArray<RawCOPPER> rawcprarray;
  for (int i = 0; i < rawcprarray.getEntries(); i++) {
    for (int j = 0; j < rawcprarray[ i ]->GetNumEntries(); j++) {
      printf("\n===== DataBlock(RawCOPPER): Block # %d ", i);
      writeData(rawcprarray[ i ], j);
    }
  }

  //
  // Data from COPPER named as RawSVD by software
  //
  StoreArray<RawSVD> raw_svdarray;
  for (int i = 0; i < raw_svdarray.getEntries(); i++) {
    for (int j = 0; j < raw_svdarray[ i ]->GetNumEntries(); j++) {
      printf("\n===== DataBlock(RawSVD) : Block # %d ", i);
      writeData(raw_svdarray[ i ], j);
    }
  }

  //
  // Data from COPPER named as RawCDC by software
  //
  StoreArray<RawCDC> raw_cdcarray;
  for (int i = 0; i < raw_cdcarray.getEntries(); i++) {
    for (int j = 0; j < raw_cdcarray[ i ]->GetNumEntries(); j++) {
      printf("\n===== DataBlock(RawCDC) : Block # %d ", i);
      writeData(raw_cdcarray[ i ], j);
    }
  }

  //
  // Data from COPPER named as RawPXD by software
  //
  StoreArray<RawPXD> raw_pxdarray;
  for (int i = 0; i < raw_pxdarray.getEntries(); i++) {
    printf("\n===== DataBlock(RawPXD) : Block # %d ", i);
    printPXDEvent(raw_pxdarray[ i ]);
  }

  StoreArray<RawTOP> raw_bpidarray;
  for (int i = 0; i < raw_bpidarray.getEntries(); i++) {
    for (int j = 0; j < raw_bpidarray[ i ]->GetNumEntries(); j++) {
      printf("\n===== DataBlock(RawTOP) : Block # %d ", i);
      writeData(raw_bpidarray[ i ], j);
    }
  }

  StoreArray<RawARICH> raw_epidarray;
  for (int i = 0; i < raw_epidarray.getEntries(); i++) {
    for (int j = 0; j < raw_epidarray[ i ]->GetNumEntries(); j++) {
      printf("\n===== DataBlock(RawARICH) : Block # %d ", i);
      writeData(raw_epidarray[ i ], j);
    }
  }

  StoreArray<RawKLM> raw_klmarray;
  for (int i = 0; i < raw_klmarray.getEntries(); i++) {
    for (int j = 0; j < raw_klmarray[ i ]->GetNumEntries(); j++) {
      printf("\n===== DataBlock(RawKLM) : Block # %d ", i);
      writeData(raw_klmarray[ i ], j);
    }
  }

  StoreArray<RawECL> raw_eclarray;
  for (int i = 0; i < raw_eclarray.getEntries(); i++) {
    for (int j = 0; j < raw_eclarray[ i ]->GetNumEntries(); j++) {
      printf("\n===== DataBlock(RawECL) : Block # %d ", i);
      writeData(raw_eclarray[ i ], j);
    }
  }

  StoreArray<RawTRG> raw_trgarray;
  for (int i = 0; i < raw_trgarray.getEntries(); i++) {
    for (int j = 0; j < raw_trgarray[ i ]->GetNumEntries(); j++) {
      printf("\n===== DataBlock(RawTRG) : Block # %d ", i);
      writeData(raw_trgarray[ i ], j);
    }
  }

  //  printf("loop %d\n", n_basf2evt);
  n_basf2evt++;

}
