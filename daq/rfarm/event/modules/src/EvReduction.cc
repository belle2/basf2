/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <daq/rfarm/event/modules/EvReduction.h>
#include <TSystem.h>

#include <rawdata/dataobjects/RawTLU.h>
#include <rawdata/dataobjects/RawTRG.h>

// #define DESY

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(EvReduction)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

EvReductionModule::EvReductionModule() : Module()
{
  //Set module properties
  setDescription("Encode DataStore into RingBuffer");
  setPropertyFlags(c_Input);

  addParam("EventReductionFactor", m_red_factor, "Event reduction factor:  ", 3);


  m_nevt = -1;

  //Parameter definition
  B2INFO("Rx: Constructor done.");
}


EvReductionModule::~EvReductionModule()
{
}

void EvReductionModule::initialize()
{
  gSystem->Load("libdataobjects");

  // Initialize EvtMetaData

  // Initialize Array of RawCOPPER
  m_rawDataBlock.registerInDataStore();
  m_rawCOPPER.registerInDataStore();
  m_rawSVD.registerInDataStore();
  m_rawCDC.registerInDataStore();
  m_rawTOP.registerInDataStore();
  m_rawARICH.registerInDataStore();
  m_rawECL.registerInDataStore();
  m_rawKLM.registerInDataStore();
  m_rawFTSW.registerInDataStore();

#ifdef DESY
  m_rawTLU.registerInDataStore();
#endif
  B2INFO("Rx initialized.");
}


void EvReductionModule::beginRun()
{
  B2INFO("beginRun called.");
}


void EvReductionModule::event()
{

  // Skip first event since it is read in initialize();
  StoreArray<RawDataBlock> dblk_ary;
  StoreArray<RawFTSW> ftsw_ary;
  StoreArray<RawTLU> tlu_ary;
  StoreArray<RawCDC> cdc_ary;
  StoreArray<RawSVD> svd_ary;
  StoreArray<RawECL> ecl_ary;
  StoreArray<RawTOP> top_ary;
  StoreArray<RawARICH> arich_ary;
  StoreArray<RawKLM> klm_ary;
  StoreArray<RawTRG> trg_ary;
  StoreArray<RawCOPPER> cpr_ary;

  if ((m_nevt %  m_red_factor) != 0) {
    ftsw_ary.clear();
    tlu_ary.clear();
    cdc_ary.clear();
    svd_ary.clear();
    ecl_ary.clear();
    top_ary.clear();
    arich_ary.clear();
    klm_ary.clear();
    trg_ary.clear();
    cpr_ary.clear();
  }

  m_nevt++;
  return;
}

void EvReductionModule::endRun()
{
  //fill Run data

  B2INFO("EvReduction: endRun done.");
}


void EvReductionModule::terminate()
{
  B2INFO("EvReduction: terminate called");
}

