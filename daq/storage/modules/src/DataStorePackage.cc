#include "daq/storage/modules/DataStorePackage.h"
#include "daq/storage/ONSENBinData.h"

#include <framework/logging/Logger.h>
#include <framework/datastore/StoreObjPtr.h>

#include "framework/datastore/StoreObjPtr.h"
#include "framework/dataobjects/EventMetaData.h"

#include <framework/datastore/StoreArray.h>
#include <rawdata/dataobjects/RawPXD.h>

#include <daq/slc/base/StringUtil.h>

#include <TClonesArray.h>
#include <TClass.h>

#include <cstring>

using namespace Belle2;

bool DataStorePackage::restore()
{
  int nboard = m_data.getNBoard();
  if (nboard == 1) {
    m_data_hlt.setBuffer(m_data.getBuffer());
  } else if (nboard > 1) {
    m_data_hlt.setBuffer(m_data.getBody());
  }
  if (m_data_hlt.getBuffer() == NULL || m_data_hlt.getTrailerMagic() != BinData::TRAILER_MAGIC) {
    B2ERROR(__FILE__ << ":" << __LINE__ << " Bad tarailer magic for HLT = "
            << m_data_hlt.getTrailerMagic());
    return false;
  }
  EvtMessage* msg = new EvtMessage((char*)m_data_hlt.getBody());
  if (msg->type() == MSG_TERMINATE) {
    B2INFO("Got Termination message");
    delete msg;
    return false;
  }
  m_streamer.restoreDataStore(msg);
  delete msg;
  if (nboard > 1) {
    m_data_pxd.setBuffer(m_data.getBuffer() + m_data_hlt.getWordSize() + m_data.getHeaderWordSize());
    if (m_data_pxd.getBody()[0] != ONSENBinData::MAGIC) {
      B2ERROR(__FILE__ << ":" << __LINE__ << " Bad ONSEN magic for PXD = " << m_data_pxd.getTrailerMagic());
      return false;
    } else if (m_data_pxd.getTrailerMagic() != BinData::TRAILER_MAGIC) {
      B2ERROR(__FILE__ << ":" << __LINE__ << " Bad tarailer magic for PXD = " << m_data_pxd.getTrailerMagic());
      return false;
    }
    if (m_data_pxd.getBuffer() != NULL) {
      StoreArray<RawPXD> rawpxdary;
      rawpxdary.appendNew(RawPXD((int*)m_data_pxd.getBody(), m_data_pxd.getBodyByteSize()));
    }
  } else {
    m_data_pxd.setBuffer(NULL);
  }
  return true;
}

