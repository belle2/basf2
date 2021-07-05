/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <unistd.h>
#include <cstdlib>
#include <cstring>

#include <framework/logging/Logger.h>

#include <daq/storage/BinData.h>

#include <daq/slc/system/PThread.h>
#include <daq/slc/system/Cond.h>
#include <daq/slc/system/Mutex.h>
#include <daq/slc/system/TCPServerSocket.h>
#include <daq/slc/system/TCPSocket.h>
#include <daq/slc/system/TCPSocketReader.h>
#include <daq/slc/system/TCPSocketWriter.h>
#include <daq/slc/system/Time.h>
#include <daq/slc/system/LogFile.h>
#include <daq/slc/base/IOException.h>

const unsigned int NWORD_BUFFER = 10000000;//10Mword

namespace Belle2 {

  Mutex g_mutex;
  Cond g_cond;

  class EvtBuffer {

  public:
    EvtBuffer(int id)
    {
      m_id = id;
      m_evtbuf = new int[NWORD_BUFFER];
      m_writable = true;
      m_readable = false;
    }

  public:
    bool write(int* buf, int nbyte)
    {
      m_mutex.lock();
      bool succeded = false;
      if (m_writable) {
        memcpy(m_evtbuf, buf, nbyte);
        m_cond.signal();
        m_writable = false;
        m_readable = true;
        succeded = true;
      }
      m_mutex.unlock();
      return succeded;
    }
    bool writable()
    {
      m_mutex.lock();
      bool succeded = false;
      if (m_writable) {
        succeded = true;
      }
      m_mutex.unlock();
      return succeded;
    }
    int* buffer()
    {
      m_writable = false;
      m_readable = true;
      return m_evtbuf;
    }
    void read(Writer& writer)
    {
      m_mutex.lock();
      while (true) {
        if (!m_readable) {
          m_cond.wait(m_mutex);
        } else {
          //int evtno = m_evtbuf[4];
          //LogFile::info("write %d %d", m_id, evtno);
          writer.write(m_evtbuf, m_evtbuf[0] * 4);
          m_writable = true;
          m_readable = false;
          g_mutex.lock();
          g_cond.signal();
          g_mutex.unlock();
          break;
        }
      }
      m_mutex.unlock();
    }
    void lock() { m_mutex.lock(); }
    void unlock() { m_mutex.unlock(); }
    void signal() { m_cond.signal(); }

  private:
    int m_id;
    int* m_evtbuf;
    Mutex m_mutex;
    Cond m_cond;
    bool m_writable;
    bool m_readable;

  };

  class Sender {
  public:
    Sender(int port, EvtBuffer* buf)
      : m_port(port), m_buf(buf) {}

  public:
    void run()
    {
      TCPServerSocket server("0.0.0.0", m_port);
      server.open();
      while (true) {
        TCPSocket socket;
        try {
          socket = server.accept();
          socket.setBufferSize(32 * 1024 * 1024);
          TCPSocketWriter writer(socket);
          while (true) {
            m_buf->read(writer);
          }
        } catch (const IOException& e) {
          socket.close();
        }
      }
    }

  private:
    int m_port;
    EvtBuffer* m_buf;

  };

}

using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 4) {
    LogFile::debug("%s : hostname port nsenders portbase", argv[0]);
    return 1;
  }

  const int nsenders = atoi(argv[3]);
  const int portbase = atoi(argv[4]);
  std::vector<EvtBuffer*> buf_v;

  for (int i = 0; i < nsenders; i++) {
    EvtBuffer* buf = new EvtBuffer(i);
    buf_v.push_back(buf);
    PThread(new Sender(portbase + i + 1, buf));
  }

  TCPSocket socket(argv[1], atoi(argv[2]));
  int ntried = 0;
  while (true) {
    while (socket.get_fd() <= 0) {
      try {
        socket.connect();
        B2INFO("Connected to upstream");
        socket.setBufferSize(32 * 1024 * 1024);
        ntried = 0;
        break;
      } catch (const IOException& e) {
        socket.close();
        if (ntried < 5)
          B2WARNING("failed to connect to upstream (try=" << ntried++ << ")");
        sleep(5);
      }
    }
    try {
      TCPSocketReader reader(socket);
      B2INFO("storagein: Cconnected to eb2.");
      int count = 0;
      unsigned long ilast = 0;
      BinData data;
      int* evtbuf = new int[NWORD_BUFFER];
      unsigned int nremains = 0;
      double nbyteall = 0;
      Time t0;
      while (true) {
        unsigned int nword = socket.read_once(evtbuf + nremains,
                                              (NWORD_BUFFER - nremains) * sizeof(int)) / sizeof(int)
                             + nremains;
        if (nword == 0) continue;
        nbyteall += nword * 4;
        //B2INFO("nword = " << nword << " nremains = " << nremains);
        unsigned int offset = 0;
        while (offset < nword && nword >= evtbuf[offset] + offset) {
          if (offset > 0 && evtbuf[offset - 1] != (int)BinData::TRAILER_MAGIC) {
            B2FATAL("Invalid trailer magic" << evtbuf[offset - 1] << "!=" << (int)BinData::TRAILER_MAGIC);
            return 1;
          }
          int nword_evt = evtbuf[offset];
          int evtno = evtbuf[offset + 4];
          while (true) {
            int i = ilast % nsenders;
            ilast++;
            if (buf_v[i]->writable()) {
              buf_v[i]->write(&evtbuf[offset], nword_evt * 4);
              offset += evtbuf[offset];
              count++;
              break;
            }
            if (i == 0 && ilast > 0) {
              g_mutex.lock();
              g_cond.wait(g_mutex);
              g_mutex.unlock();
            }
          }
          if (count < 1000000 && (count < 10 || (count > 10 && count < 100 && count % 10 == 0) ||
                                  (count > 100 && count < 1000 && count % 100 == 0) ||
                                  (count > 1000 && count < 10000 && count % 1000 == 0) ||
                                  (count > 10000 && count < 100000 && count % 10000 == 0) ||
                                  (count > 100000 && count < 1000000 && count % 100000 == 0))) {
            B2INFO("Event count = " << count << " nword = " << nword_evt << " evtno = " << evtno);
          }
          const int nth = 100000;
          if (count % nth == 0) {
            Time t;
            double dt = (t.get() - t0.get());
            double freq = nth / dt  / 1000.;
            double rate = nbyteall / dt / 1000. / 1000.;
            printf("[DEBUG] Serial = %d Freq = %f [kHz], Rate = %f [MB/s], DataSize = %f [kB/event]\n",
                   count, freq, rate, nbyteall / 1000. / nth);
            t0 = t;
            nbyteall = 0;
          }
        }
        nremains = nword - offset;
        if (nremains > 0) {
          memmove(evtbuf, evtbuf + offset, nremains * sizeof(int));
        }
      }
    } catch (const IOException& e) {
      socket.close();
      B2WARNING("Connection to upstream broken.");
      sleep(5);
    }
  }
  return 0;
}

