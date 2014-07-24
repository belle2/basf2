#ifndef DATASTOREPACKAGE_H
#define DATASTOREPACKAGE_H

#include <vector>
#include <queue>
#include <string>

#include <framework/datastore/DataStore.h>
#include <framework/pcore/EvtMessage.h>
#include <framework/pcore/MsgHandler.h>
#include <framework/pcore/DataStoreStreamer.h>

#include <daq/slc/system/Mutex.h>
#include <rawdata/dataobjects/RawPXD.h>

#include "daq/storage/BinData.h"

namespace Belle2 {

  class DataStorePackage {

  public:
    static const unsigned int MAX_BUFFER_WORDS = 1000000;

  private:
    static Mutex g_mutex;
    static bool g_init;
    enum ETObjectBits {
      c_IsTransient = BIT(19),
      c_IsNull = BIT(20)
    };

  public:
    DataStorePackage(unsigned int serial = 0)
      : m_serial(serial) {
      m_buf = new int [MAX_BUFFER_WORDS];
      m_data.setBuffer(m_buf);
      m_data_hlt.setBuffer(NULL);
      m_data_pxd.setBuffer(NULL);
    }
    ~DataStorePackage() {
      delete[] m_buf;
    }

  public:
    bool restore();
    unsigned int getSerial() const { return m_serial; }
    void setSerial(unsigned int serial) { m_serial = serial; }
    const BinData& getData() const { return m_data; }
    const BinData& getHLTData() const { return  m_data_hlt; }
    const BinData& getPXDData() const { return m_data_pxd; }

  private:
    unsigned int m_serial;
    BinData m_data;
    BinData m_data_hlt;
    BinData m_data_pxd;
    int* m_buf;
    DataStoreStreamer m_streamer;

  };

}

#endif
