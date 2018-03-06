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
  static int count = 0;
  //(m_sndhdrary.appendNew())->SetBuffer(m_data.getBuffer());
  int nboard = m_data.getNBoard();
  if (m_eb2 == 0 && nboard == 1) {
    m_data_hlt.setBuffer(m_data.getBuffer());
  } else if (m_eb2 > 0 || nboard > 1) {
    m_data_hlt.setBuffer(m_data.getBody());
  }

  if (m_data_hlt.getBuffer() == NULL || m_data_hlt.getTrailerMagic() != BinData::TRAILER_MAGIC) {
    B2FATAL("Bad tarailer magic for HLT = " << m_data_hlt.getTrailerMagic()
            << " count = " << count);
  }
  EvtMessage* msg = new EvtMessage((char*)m_data_hlt.getBody());
  if (msg->type() == MSG_TERMINATE) {
    B2INFO("Got Termination message");
    delete msg;
    return false;
  }
  //B2INFO("nbyte : " << m_data_hlt.getBody()[0]);
  m_streamer->restoreDataStore(msg);
  delete msg;
  if (nboard > 1) {
    unsigned int offset = m_data_hlt.getWordSize() + m_data.getHeaderWordSize();
    for (unsigned int i = 0; i < nboard - 1; i++) {
      m_data_pxd.setBuffer(m_data.getBuffer() + offset);
      offset += m_data_pxd.getWordSize();
      if (m_data_pxd.getBody()[0] != ONSENBinData::MAGIC) {
        B2FATAL("Bad ONSEN magic for PXD = " << m_data_pxd.getTrailerMagic());
        return false;
      } else if (m_data_pxd.getTrailerMagic() != BinData::TRAILER_MAGIC) {
        B2FATAL("Bad tarailer magic for PXD = " << m_data_pxd.getTrailerMagic());
        return false;
      }
      if (m_data_pxd.getBuffer() != NULL) {
        m_rawpxdary.appendNew((int*)m_data_pxd.getBody(), m_data_pxd.getBodyByteSize());
        //(m_sndhdrary.appendNew())->SetBuffer(m_data_pxd.getBuffer());
      }
    }
  } else {
    m_data_pxd.setBuffer(NULL);
  }
  return true;
}

