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

bool DataStorePackage::decode(MsgHandler& msghandler)
{
  int nboard = m_data.getNBoard();
  if (nboard == 1) {
    m_data_hlt.setBuffer(m_data.getBuffer());
  } else if (nboard > 1) {
    m_data_hlt.setBuffer(m_data.getBody());
  }
  if (m_data_hlt.getBuffer() == NULL || m_data_hlt.getTrailerMagic() != BinData::TRAILER_MAGIC) {
    B2ERROR(__FILE__ << ":" << __LINE__ << " Bad tarailer magic for HLT = " << m_data_hlt.getTrailerMagic());
    return false;
  }
  m_objlist = std::vector<TObject*>();
  m_namelist = std::vector<std::string>();
  EvtMessage* msg = new EvtMessage((char*)m_data_hlt.getBody());
  if (msg->type() == MSG_TERMINATE) {
    B2INFO("Got Termination message");
    delete msg;
    return false;
  }
  bool hasEvtMetaData = false;
  msghandler.decode_msg(msg, m_objlist, m_namelist);
  m_nobjs = (msg->header())->reserved[1];
  m_narrays = (msg->header())->reserved[2];
  for (int i = 0; i < m_nobjs + m_narrays; i++) {
    if (m_objlist.at(i) != NULL && m_namelist.at(i) == "EventMetaData") {
      hasEvtMetaData = true;
      break;
    }
  }
  if (!hasEvtMetaData) {
    B2WARNING("No event meta data found");
    for (int i = 0; i < m_nobjs + m_narrays; i++) {
      if (m_objlist.at(i) != NULL) {
        TObject* obj = m_objlist.at(i);
        delete obj;
      }
    }
    delete msg;
    return false;
  }
  m_durability = (DataStore::EDurability)(msg->header())->reserved[0];
  if (nboard > 1) {
    m_data_pxd.setBuffer(m_data.getBuffer() + m_data_hlt.getWordSize() + m_data.getHeaderWordSize());
    if (m_data_pxd.getBody()[0] != ONSENBinData::MAGIC) {
      B2ERROR(__FILE__ << ":" << __LINE__ << " Bad ONSEN magic for PXD = " << m_data_pxd.getTrailerMagic());
      return false;
    }
    if (m_data_pxd.getTrailerMagic() != BinData::TRAILER_MAGIC) {
      B2ERROR(__FILE__ << ":" << __LINE__ << " Bad tarailer magic for PXD = " << m_data_pxd.getTrailerMagic());
      return false;
    }
  } else {
    m_data_pxd.setBuffer(NULL);
  }
  delete msg;
  return true;
}

void DataStorePackage::restore(bool init)
{
  bool is_array = false;
  for (int i = 0; i < m_nobjs + m_narrays; i++) {
    if (m_objlist.at(i) != NULL) {
      is_array = (dynamic_cast<TClonesArray*>(m_objlist.at(i)) != 0);
      TObject* obj = m_objlist.at(i);
      const TClass* cl = obj->IsA();
      if (is_array)
        cl = static_cast<TClonesArray*>(obj)->GetClass();
      if (!init) {
        bool is_transient = obj->TestBit(c_IsTransient);
        DataStore::Instance().registerEntry(m_namelist.at(i), m_durability, cl, is_array, is_transient, false);
      }
      bool is_null = !obj->TestBit(c_IsNull);
      if (is_null) {
        DataStore::Instance().createObject(obj, true, StoreAccessorBase(m_namelist.at(i),
                                           m_durability, cl, is_array));
        B2DEBUG(100, "restoreDS: " << (is_array ? "Array" : "Object") << ": " << m_namelist.at(i) << " stored");

        obj->SetBit(c_IsTransient, false);
        obj->SetBit(c_IsNull, false);
      } else {
        delete obj;
      }
    } else {
      B2ERROR("restoreDS: " << (is_array ? "Array" : "Object") << ": " << m_namelist.at(i) << " is NULL!");
    }
  }
  if (m_data_pxd.getBuffer() != NULL) {
    StoreArray<RawPXD> rawpxdary;
    rawpxdary.appendNew(RawPXD((int*)m_data_pxd.getBody(), m_data_pxd.getBodyByteSize()));
  }
}

void DataStorePackage::copy(DataStorePackage& package)
{
  m_serial = package.m_serial;
  memcpy(m_buf, package.m_buf, package.m_data.getByteSize());
  m_data.setBuffer(m_buf);
  m_data_hlt.setBuffer(m_data.getBody());
  m_objlist = package.m_objlist;
  m_namelist = package.m_namelist;
  m_nobjs = package.m_nobjs;
  m_narrays = package.m_narrays;
  m_durability = package.m_durability;
  if (m_data.getNBoard() > 1 && package.m_data_pxd.getBuffer() != NULL) {
    int nword = package.m_data_hlt.getWordSize() + package.m_data.getHeaderWordSize();
    m_data_pxd.setBuffer(m_data.getBuffer() + nword);
  } else {
    m_data_pxd.setBuffer(NULL);
  }
}
