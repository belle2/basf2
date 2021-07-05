/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <TObject.h>

namespace Belle2 {

  /** The payload class for PXD cluster charge calibrations
   *
   *  The payload class stores the median cluster charge values
   *  for PXD sensors on a grid nBinsU x nBinsV. The values are
   *  stored in raw ADC units (called ADU).
   *
   *  The granularity of the grid is user adjustable. The default
   *  value of a missing calibraiton is -1.0.
   */

  class TRGGDLDBUnpacker: public TObject {
  public:

    /** Default constructor */
    TRGGDLDBUnpacker(): m_nLeafs(0), m_nLeafsExtra(0), m_nClks(0), m_nBits(0), m_conf(0), m_LeafNames{0}, m_LeafMap{0}, m_BitMap{0},
      m_nword_header(0), m_nword_input(0), m_nword_output(0), m_BitMap_extra{0} {}

    /** copy constructor */
    TRGGDLDBUnpacker(const TRGGDLDBUnpacker& b): TObject(b)
    {
      m_nLeafs = b.m_nLeafs;
      m_nLeafsExtra = b.m_nLeafsExtra;
      m_nClks = b.m_nClks;
      for (int i = 0; i < 320; i++) {
        strcpy(m_LeafNames[i], b.m_LeafNames[i]);
        m_LeafMap[i] = b.m_LeafMap[i];
      }
      for (int i = 0; i < 320; i++) {
        for (int j = 0; j < 2; j++) {
          m_BitMap[i][j] = b.m_BitMap[i][j];
        }
      }
      for (int i = 0; i < 320; i++) {
        for (int j = 0; j < 3; j++) {
          m_BitMap_extra[i][j] = b.m_BitMap_extra[i][j];
        }
      }
      m_nword_header   = b.m_nword_header;
      m_nword_input    = b.m_nword_input;
      m_nword_output   = b.m_nword_output;
    }
    /** assignment operator */
    TRGGDLDBUnpacker& operator=(const TRGGDLDBUnpacker& b)
    {
      m_nLeafs = b.m_nLeafs;
      m_nLeafsExtra = b.m_nLeafsExtra;
      m_nClks = b.m_nClks;
      m_nBits = b.m_nBits;
      m_conf  = b.m_conf;
      for (int i = 0; i < 320; i++) {
        strcpy(m_LeafNames[i], b.m_LeafNames[i]);
        m_LeafMap[i] = b.m_LeafMap[i];
      }
      for (int i = 0; i < 320; i++) {
        for (int j = 0; j < 2; j++) {
          m_BitMap[i][j] = b.m_BitMap[i][j];
        }
      }
      for (int i = 0; i < 320; i++) {
        for (int j = 0; j < 3; j++) {
          m_BitMap_extra[i][j] = b.m_BitMap_extra[i][j];
        }
      }
      m_nword_header   = b.m_nword_header;
      m_nword_input    = b.m_nword_input;
      m_nword_output   = b.m_nword_output;
      return *this;
    }

    void setnLeafs(int i)
    {
      m_nLeafs = i;
    }
    void setnLeafsExtra(int i)
    {
      m_nLeafsExtra = i;
    }
    void setnClks(int i)
    {
      m_nClks = i;
    }
    void setnBits(int i)
    {
      m_nBits = i;
    }
    void setconf(int i)
    {
      m_conf = i;
    }
    void setLeafName(int i, const char* c)
    {
      strcpy(m_LeafNames[i], c);
    }
    void setLeafMap(int i, int j)
    {
      m_LeafMap[i] = j;
    }
    void setBitMap(int i, int j, int k)
    {
      m_BitMap[i][j] = k;
    }
    void set_nword_header(int i)
    {
      m_nword_header = i;
    }
    void set_nword_input(int i)
    {
      m_nword_input = i;
    }
    void set_nword_output(int i)
    {
      m_nword_output = i;
    }
    void setBitMap_extra(int i, int j, int k)
    {
      m_BitMap_extra[i][j] = k;
    }

    int getnLeafs() const
    {
      return m_nLeafs;
    }
    int getnLeafsExtra() const
    {
      return m_nLeafsExtra;
    }
    int getnClks() const
    {
      return m_nClks;
    }
    int getnBits() const
    {
      return m_nBits;
    }
    int getconf() const
    {
      return m_conf;
    }
    const char* getLeafnames(int i) const
    {
      return m_LeafNames[i];
    }
    int getLeafMap(int i) const
    {
      return m_LeafMap[i];
    }
    int getBitMap(int i, int j) const
    {
      return m_BitMap[i][j];
    }
    int get_nword_header() const
    {
      return m_nword_header;
    }
    int get_nword_input() const
    {
      return m_nword_input;
    }
    int get_nword_output() const
    {
      return m_nword_output;
    }
    int getBitMap_extra(int i, int j) const
    {
      return m_BitMap_extra[i][j];
    }



  private:

    /** num of leafs in data_b2l **/
    int m_nLeafs;

    /** num of leafs for others **/
    int m_nLeafsExtra;

    /** num of clk time window **/
    int m_nClks;

    /** num of Bit **/
    int m_nBits = 0;

    /** conf **/
    int m_conf = 0;

    /** leaf names **/
    char m_LeafNames[320][100];

    /** leaf map**/
    int  m_LeafMap[320];

    /** data bit map **/
    int m_BitMap[320][2];

    /** header size **/
    int m_nword_header;

    /** DAM input word size **/
    int m_nword_input;

    /** DAM output word size **/
    int m_nword_output;

    /** extra leaf bit map **/
    int m_BitMap_extra[320][3];


    ClassDef(TRGGDLDBUnpacker, 1);  /**< ClassDef, must be the last term before the closing {}*/
  };

} // end of namespace Belle2
