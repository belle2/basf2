/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Petric, Marko Staric                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <testbeam/top/modules/TOPRawdataInput/Packet.h>
#include <top/RawDataTypes.h>
#include <framework/logging/Logger.h>

namespace Belle2 {
  namespace TOPTB {

    bool Packet::read(std::ifstream& stream)
    {

      stream.read((char*) &m_header, sizeof(unsigned));
      m_bytesRead += stream.gcount();
      if (!stream.good()) {
        if (!stream.eof()) B2ERROR("Error reading packet header word");
        return false;
      }
      if (!checkHeaderWord()) {
        B2ERROR("Unknown packet: headerWord = " << m_header);
        return false;
      }

      stream.read((char*) &m_size, sizeof(unsigned));
      m_bytesRead += stream.gcount();
      if (!stream.good()) {
        B2ERROR("Error reading packet size word");
        return false;
      }

      stream.read((char*) &m_type, sizeof(unsigned));
      m_bytesRead += stream.gcount();
      if (!stream.good()) {
        B2ERROR("Error reading packet type word");
        return false;
      }

      int numWords = allocateData();
      if (numWords == 0) {
        B2ERROR("Invalid packet size: packetSize = " << m_size);
        return false;
      }

      stream.read((char*) m_data, numWords * sizeof(unsigned));
      m_bytesRead += stream.gcount();
      if (!stream.good()) {
        B2ERROR("Error reading packet data words");
        return false;
      }

      stream.read((char*) &m_checkSum, sizeof(unsigned));
      m_bytesRead += stream.gcount();
      if (!stream.good()) {
        B2ERROR("Error reading packet check-sum word");
        return false;
      }
      if (!checkSum()) {
        B2ERROR("Corrupted packet: check sum validation failed");
        return false;
      }

      const WaveformHead* head = 0;
      WaveformSegment segm;
      switch (m_type) {
        case c_Header:
          if (sizeof(HeaderBody) / sizeof(unsigned) != getDataSize()) {
            B2ERROR("Invalid size of header packet: " << getPacketSize() << " words");
            return false;
          }
          break;
        case c_Waveform:
          head = getWaveformHead();
          if (!head) {
            B2ERROR("Invalid size of waveform packet header: " << getPacketSize()
                    << " words");
            return false;
          }
          segm = getWaveformSegment(head->numofSegments - 1);
          if (!segm.data) {
            B2ERROR("Corrupted waveform packet");
            return false;
          }
          break;
        default:
          B2INFO("Unknown packet: packetType = " << m_type);
      }

      return true;

    }


    WaveformSegment Packet::getWaveformSegment(int iseg) const
    {
      WaveformSegment segment;
      if (!isWaveformPacket()) return segment;
      if (iseg < 0) return segment;

      int headerSize = sizeof(WaveformSegment) / sizeof(unsigned) -
                       sizeof(unsigned*) / sizeof(unsigned);
      int last = getDataSize() - headerSize - 1;
      int k = sizeof(WaveformHead) / sizeof(unsigned);
      for (int i = 0; i < iseg; i++) {
        if (k > last or k < 0) return segment;
        WaveformSegment* seg = (WaveformSegment*) &m_data[k];
        k += (headerSize + (seg->numofPoints + 1) / 2);
      }
      if (k > last or k < 0) return segment;
      WaveformSegment* seg = (WaveformSegment*) &m_data[k];
      k += headerSize;
      if ((int)(k + (seg->numofPoints + 1) / 2) != getDataSize()) return segment;

      segment = *seg;
      segment.data = &m_data[k];
      return segment;

    }


    bool ScrodData::read(std::ifstream& stream)
    {

      std::vector<Packet*> packets;

      // read header packet

      Packet* packet = new Packet();
      bool ok = packet->read(stream);
      m_bytesRead += packet->getBytesRead();
      if (!ok) {
        delete packet;
        return false;
      }
      if (!packet->isHeaderPacket()) {
        B2ERROR("Expect header packet, got packetType = " << packet->getType());
        delete packet;
        return false;
      }
      packets.push_back(packet);
      m_packetsRead++;

      // read waveform packets

      const HeaderBody* header = packet->getHeaderBody();
      m_eventNumber = header->eventNumber;
      unsigned scrod = header->scrod;
      int n = header->numofWaveforms + header->numofAux;
      for (int i = 0; i < n; i++) {
        Packet* packet = new Packet();
        bool ok = packet->read(stream);
        m_bytesRead += packet->getBytesRead();
        if (!ok) {
          delete packet;
          for (auto& packet : packets) delete packet;
          return false;
        }
        m_packetsRead++;
        if (packet->isHeaderPacket()) {
          B2ERROR("Expect waveform packet, got header packet");
          delete packet;
          for (auto& packet : packets) delete packet;
          return false;
        }
        if (packet->isWaveformPacket()) {
          const WaveformHead* waveform = packet->getWaveformHead();
          if (waveform->scrod == scrod and waveform->eventNumber == m_eventNumber) {
            packets.push_back(packet);
            continue;
          } else {
            B2ERROR("Waveform packet with different SCROD ID or event number found "
                    "- packet dropped");
          }
        }
        delete packet;
      }

      // pack packets' data

      int dataSize = 1;
      for (const auto& packet : packets) {
        dataSize +=  packet->getDataSize();
      }
      allocateData(dataSize);

      unsigned scrodID = scrod & 0xFFFF;
      unsigned dataFormat = static_cast<unsigned>(TOP::RawDataType::c_IRS3B);
      ok = add(scrodID + (dataFormat << 16));
      ok = add(header->scrod);
      ok = add(header->freezeDate);
      ok = add(header->eventNumber);
      ok = add(header->triggerType);
      ok = add(header->flags);
      ok = add(packets.size() - 1);
      for (auto& packet : packets) {
        const WaveformHead* head = packet->getWaveformHead();
        if (!head) continue;
        ok = add(head->referenceASIC);
        ok = add(head->numofSegments);
        for (unsigned iseg = 0; iseg < head->numofSegments; iseg++) {
          WaveformSegment segment = packet->getWaveformSegment(iseg);
          ok = add(segment.segmentASIC);
          ok = add(segment.numofPoints);
          for (unsigned i = 0; i < (segment.numofPoints + 1) / 2; i++) {
            ok = add(segment.data[i]);
          }
        }
      }
      if (!ok) {
        B2FATAL("Bug: under-dimensioned array in ScrodData!");
      }

      for (auto& packet : packets) delete packet;
      return true;
    }

  } // end TOPTB namespace
} // end Belle2 namespace

