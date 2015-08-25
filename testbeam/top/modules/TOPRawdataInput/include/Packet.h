/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PACKET_H
#define PACKET_H

#include <fstream>
#include <iostream>
#include <vector>

namespace Belle2 {
  namespace TOPTB {

    /**
     * Structure of header packet body (e.g. m_data in class Packet)
     * http://www.phys.hawaii.edu/~kurtisn/doku.php?id=itop:documentation:data_format
     */
    struct HeaderBody {
      unsigned scrod; /**< source scrod ID (bits 0:15) and revision (bits 16:23) */
      unsigned freezeDate;  /**< protocol freeze date (YYYYMMDD in BCD) */
      unsigned eventNumber; /**< event number */
      unsigned triggerType; /**< trigger type (bits 0:7) */
      unsigned flags;       /**< event flags (bits 0:7) */
      unsigned numofWaveforms;  /**< number of waveform packets after header packet */
      unsigned numofAux; /**< number of auxiliary packets after header packet */
    };


    /**
     * Structure of waveform packet header(e.g. first part of m_data in class Packet)
     * http://www.phys.hawaii.edu/~kurtisn/doku.php?id=itop:documentation:data_format
     */
    struct WaveformHead {
      unsigned scrod; /**< source scrod ID (bits 0:15) and revision (bits 16:23) */
      unsigned referenceASIC; /**< reference analog storage window (for reference time) */
      unsigned eventNumber; /**< event number */
      unsigned numofSegments; /**< number of waveform segments in the packet */
    };

    /**
     * Structure of waveform packet segment (e.g. next parts of m_data in class Packet)
     * http://www.phys.hawaii.edu/~kurtisn/doku.php?id=itop:documentation:data_format
     */
    struct WaveformSegment {
      unsigned segmentASIC; /**< analog storage window a waveform was taken from */
      unsigned numofPoints; /**< number of waveform points */
      const unsigned* data; /**< packed waveform ADC values (two values per word) */
      /**
       * Default constructor
       */
      WaveformSegment(): segmentASIC(0), numofPoints(0), data(0)
      {}
    };


    /**
     * Variable-size data packet
     * http://www.phys.hawaii.edu/~kurtisn/doku.php?id=itop:documentation:data_format
     */
    class Packet {

    public:

      /**
       * packet types
       */
      enum EType {c_Header   = 0x65766e74, /**< header packet */
                  c_Waveform = 0x77617665  /**< waveform packet */
                 };

      /**
       * size limit of package data (about 2.2 M words)
       */
      enum {c_MaxSize = 34 * 512 * 128 + 8};

      /**
       * Default constructor
       */
      Packet(): m_header(0), m_size(0), m_type(0), m_data(0), m_checkSum(0), m_bytesRead(0)
      {}

      /**
       * Destructor
       */
      ~Packet()
      {
        if (m_data) delete [] m_data;
      }

      /**
       * Read packet from file
       * @param stream input stream for reading from file
       * @return true on success
       */
      bool read(std::ifstream& stream);

      /**
       * Check header word
       * @return true if header word is the right one
       */
      bool checkHeaderWord() const {return (m_header == 0x00BE11E2);}

      /**
       * Check sum
       * @return true if sum eq check-sum word
       */
      bool checkSum() const
      {
        unsigned sum = m_header + m_size + m_type;
        for (unsigned i = 0; i < m_size - 2; i++) sum += m_data[i];
        return (sum == m_checkSum);
      }

      /**
       * Check if it is header packet
       * @return true if header packet
       */
      bool isHeaderPacket() const {return m_type == c_Header;}

      /**
       * Check if it is waveform packet
       * @return true if waveform packet
       */
      bool isWaveformPacket() const {return m_type == c_Waveform;}

      /**
       * Return number of allocated words for packet data
       * @return number of allocated words
       */
      int getDataSize() const
      {
        if (m_data) return m_size - 2;
        return 0;
      }

      /**
       * Return packet header word
       * @return header word
       */
      unsigned getHeaderWord() const {return m_header;}

      /**
       * Return packet size
       * @return packet size
       */
      unsigned getPacketSize() const {return m_size + 2;}

      /**
       * Return packet type
       * @return packet type word
       */
      unsigned getType() const {return m_type;}

      /**
       * Return packet data
       * @return pointer to packet data
       */
      const unsigned* getData() const {return m_data;}

      /**
       * Return packet check-sum
       * @return checkSum word
       */
      unsigned getCheckSumWord() const {return m_checkSum;}

      /**
       * Return number of bytes read from file
       * @return number of bytes
       */
      unsigned getBytesRead() const {return m_bytesRead;}

      /**
       * Return header packet body
       * @return pointer to header packet body or NULL
       */
      const HeaderBody* getHeaderBody() const
      {
        if (!isHeaderPacket()) return 0;
        if ((int)(sizeof(HeaderBody) / sizeof(unsigned)) > getDataSize()) return 0;
        return (HeaderBody*) m_data;
      }

      /**
       * Return waveform packet header
       * @return pointer to waveform header or NULL
       */
      const WaveformHead* getWaveformHead() const
      {
        if (!isWaveformPacket()) return 0;
        if ((int)(sizeof(WaveformHead) / sizeof(unsigned)) > getDataSize()) return 0;
        return (WaveformHead*) m_data;
      }

      /**
       * Return waveform packet i-th segment.
       * Note: pointer to ADC values is NULL if segment doesn't exist or corrupted
       * @param i segment number (0-based)
       * @return waveform segment structure
       */
      WaveformSegment getWaveformSegment(int i) const;

    private:

      /**
       * Allocate memory for packet data
       * @return number of allocated words
       */
      int allocateData()
      {
        int n = m_size - 2;
        if (m_data) return n;
        if (n > 0 and n < c_MaxSize) {
          m_data = new unsigned[n];
          return n;
        }
        return 0;
      }

      unsigned m_header;   /**< header word: must be 0x00BE11E2 */
      unsigned m_size;     /**< number of words following this one */
      unsigned m_type;     /**< packet type */
      unsigned* m_data;    /**< packet data (dimension = size - 2) */
      unsigned m_checkSum; /**< 32-bit check sum */

      unsigned m_bytesRead; /**< number of bytes read */

    };


    /**
     * Packed data from one SCROD
     */
    class ScrodData {

    public:

      /**
       * Default constructor
       */
      ScrodData(): m_size(0), m_data(0), m_dimension(0), m_eventNumber(0),
        m_bytesRead(0), m_packetsRead()
      {}

      /**
       * Destructor
       */
      ~ScrodData()
      {
        if (m_data) {delete [] m_data;}
      }

      /**
       * Read packets of one SCROD from file
       * @param stream input stream for reading from file
       * @return true on success
       */
      bool read(std::ifstream& stream);

      /**
       * Return size of data
       * @return size of data
       */
      int getDataSize() const {return m_size;}

      /**
       * Return packed SCROD data
       * @return pointer to data
       */
      //const int* getData() const {return m_data;}
      int* getData() const {return m_data;}

      /**
       * Return event number
       * @return event number
       */
      unsigned getEventNumber() const {return m_eventNumber;}

      /**
       * Return number of bytes read from file
       * @return number of bytes
       */
      unsigned getBytesRead() const {return m_bytesRead;}

      /**
       * Return number of packets read from file
       * @return number of packets
       */
      unsigned getPacketsRead() const {return m_packetsRead;}

    private:

      /**
       * Allocate memory for data
       * @return number of allocated words
       */
      int allocateData(int n)
      {
        if (m_data) return m_dimension;
        if (n > 0) {
          m_data = new int[n];
          m_dimension = n;
          return n;
        }
        return 0;
      }

      /**
       * Add data word to m_data
       * @return true on success
       */
      bool add(unsigned dataWord)
      {
        if (m_size < m_dimension) {
          m_data[m_size] = dataWord;
          m_size++;
          return true;
        }
        return false;
      }

      int m_size;      /**< data size */
      int* m_data;     /**< data */
      int m_dimension; /**< number of allocated words of m_data */
      unsigned m_eventNumber; /**< event number */
      unsigned m_bytesRead; /**< number of bytes read */
      unsigned m_packetsRead; /**< number of good packets read */

    };


  } // TOPTB namespace
} // Belle2 namespace

#endif
