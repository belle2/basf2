/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * CDC hit filter module                                                  *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Hauth                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <cdc/modules/cdcHitFilter/CDCHitFilterModule.h>
#include <cdc/dataobjects/CDCHit.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>

using namespace Belle2;
using namespace CDC;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(CDCHitFilter)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

CDCHitFilterModule::CDCHitFilterModule() :
  Module()
{

  //Set module properties
  setDescription("Filter CDCHits");

  addParam("inputCDCHitListName", m_inputCDCHitListName,
           "Name of the CDCHit List to filter", std::string(""));
  addParam("outputCDCHitListName", m_outputCDCHitListName,
           "Name of the CDCHit list, which will contain the hits passing all filter criteria",
           std::string(""));

  // Limit hits to parts of the CDC, useful if not the whole CDC
  // is defined in the packer/unpacker mapping file
  addParam("filterSuperLayer", m_filterSuperLayer,
           "Only perform digitization for a specific super layer or for all, if the value is -1",
           -1);
  addParam("filterWireMax", m_filterWireMax,
           "Only perform digitization up to a specific number of wire or for all wires, if the value is -1",
           -1);
  addParam("filterLayerMax", m_filterLayerMax,
           "Only perform digitization for a specific layer or for all, if the value is -1",
           -1);
}

CDCHitFilterModule::~CDCHitFilterModule()
{
}

void CDCHitFilterModule::initialize()
{
  m_inputCDCHits.isRequired(m_inputCDCHitListName);
  m_outputCDCHits.registerInDataStore(m_outputCDCHitListName);
}

void CDCHitFilterModule::event()
{

  for (auto const& hit : m_inputCDCHits) {
    // check if filtering for super-layers, layers or wires is active
    if ((m_filterSuperLayer >= 0)
        && (m_filterSuperLayer != hit.getISuperLayer())) {
      continue;
    }
    if ((m_filterLayerMax >= 0) && (m_filterLayerMax <= hit.getILayer())) {
      continue;
    }
    if ((m_filterWireMax >= 0) && (m_filterWireMax <= hit.getIWire())) {
      continue;
    }

    m_outputCDCHits.appendNew(hit);
  }
}
