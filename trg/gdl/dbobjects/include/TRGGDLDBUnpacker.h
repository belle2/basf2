/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Schwenker                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <TObject.h>
#include <unordered_map>
#include <vector>

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
    TRGGDLDBUnpacker(): m_nLeafs(0), m_nLeafsExtra(0), m_nClks(0), m_LeafMap{0}, m_LeafNames{0}, m_BitMap{0},
      m_nword_header(0), m_conf(0), m_bufid_firmid{0}, m_bufid_firmver{0},
      m_bufid_finalrvc{0}, m_bufid_drvc{0}, m_bufid_gdll1rvc{0},
      m_bufid_coml1rvc{0}, m_bufid_b2ldly{0}, m_bufid_maxrvc{0} {}

    /** copy constructor */
    TRGGDLDBUnpacker(const TRGGDLDBUnpacker& b)
    {
      m_nLeafs = b.m_nLeafs;
      m_nLeafsExtra = b.m_nLeafsExtra;
      m_nClks = b.m_nClks;
      for (int i = 0; i < 200; i++) {
        strcpy(m_LeafNames[i], b.m_LeafNames[i]);
        m_LeafMap[i] = b.m_LeafMap[i];
      }
      for (int i = 0; i < 200; i++) {
        for (int j = 0; j < 2; j++) {
          m_BitMap[i][j] = b.m_BitMap[i][j];
        }
      }
      m_nword_header   = b.m_nword_header;
      m_conf           = b.m_conf;
      for (int i = 0; i < 3; i++) {
        m_bufid_firmid  [i] = b.m_bufid_firmid[i];
        m_bufid_firmver [i] = b.m_bufid_firmver[i];
        m_bufid_finalrvc[i] = b.m_bufid_finalrvc[i];
        m_bufid_drvc    [i] = b.m_bufid_drvc[i]    ;
        m_bufid_gdll1rvc[i] = b.m_bufid_gdll1rvc[i];
        m_bufid_coml1rvc[i] = b.m_bufid_coml1rvc[i];
        m_bufid_b2ldly[i] = b.m_bufid_b2ldly[i];
        m_bufid_maxrvc[i] = b.m_bufid_maxrvc[i];
      }
    }
    /** assignment operator */
    TRGGDLDBUnpacker& operator=(const TRGGDLDBUnpacker& b)
    {
      m_nLeafs = b.m_nLeafs;
      m_nLeafsExtra = b.m_nLeafsExtra;
      m_nClks = b.m_nClks;
      for (int i = 0; i < 200; i++) {
        strcpy(m_LeafNames[i], b.m_LeafNames[i]);
        m_LeafMap[i] = b.m_LeafMap[i];
      }
      for (int i = 0; i < 200; i++) {
        for (int j = 0; j < 2; j++) {
          m_BitMap[i][j] = b.m_BitMap[i][j];
        }
      }
      m_nword_header   = b.m_nword_header;
      m_conf           = b.m_conf        ;
      for (int i = 0; i < 3; i++) {
        m_bufid_firmid  [i] = b.m_bufid_firmid[i];
        m_bufid_firmver [i] = b.m_bufid_firmver[i];
        m_bufid_finalrvc[i] = b.m_bufid_finalrvc[i];
        m_bufid_drvc    [i] = b.m_bufid_drvc[i]    ;
        m_bufid_gdll1rvc[i] = b.m_bufid_gdll1rvc[i];
        m_bufid_coml1rvc[i] = b.m_bufid_coml1rvc[i];
        m_bufid_b2ldly[i] = b.m_bufid_b2ldly[i];
        m_bufid_maxrvc[i] = b.m_bufid_maxrvc[i];
      }
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
    void setconf(int i)
    {
      m_conf = i;
    }
    void setfirmid(int i, int j)
    {
      m_bufid_firmid[i] = j;
    }
    void setfinalrvc(int i, int j)
    {
      m_bufid_finalrvc[i] = j;
    }
    void setfirmver(int i, int j)
    {
      m_bufid_firmver[i] = j;
    }
    void setdrvc(int i, int j)
    {
      m_bufid_drvc[i] = j;
    }
    void setgdll1rvc(int i, int j)
    {
      m_bufid_gdll1rvc[i] = j;
    }
    void setcoml1rvc(int i, int j)
    {
      m_bufid_coml1rvc[i] = j;
    }
    void setb2ldly(int i, int j)
    {
      m_bufid_b2ldly[i] = j;
    }
    void setmaxrvc(int i, int j)
    {
      m_bufid_maxrvc[i] = j;
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
    int getconf() const
    {
      return m_conf;
    }
    int getfirmid(int i) const
    {
      return m_bufid_firmid[i];
    }
    int getfinalrvc(int i) const
    {
      return m_bufid_finalrvc[i];
    }
    int getfirmver(int i) const
    {
      return m_bufid_firmver[i];
    }
    int getdrvc(int i) const
    {
      return m_bufid_drvc[i];
    }
    int getgdll1rvc(int i) const
    {
      return m_bufid_gdll1rvc[i];
    }
    int getcoml1rvc(int i) const
    {
      return m_bufid_coml1rvc[i];
    }
    int getb2ldly(int i) const
    {
      return m_bufid_b2ldly[i];
    }
    int getmaxrvc(int i) const
    {
      return m_bufid_maxrvc[i];
    }


    //void setLeafPointersArray(TRGGDLUnpackerStore* store, int** bitArray);

  private:

    /** num of leafs in data_b2l **/
    int m_nLeafs;

    /** num of leafs for others **/
    int m_nLeafsExtra;

    /** num of clk time window **/
    int m_nClks;

    /** CONF2 leaf names **/
    char m_LeafNames[200][100];
    int  m_LeafMap[200];

    //enum m_EBits;

    int m_BitMap[200][2];

    int m_nword_header;
    int m_conf;
    int m_bufid_firmid[3];
    int m_bufid_firmver[3];
    int m_bufid_finalrvc[3];
    int m_bufid_drvc[3];
    int m_bufid_gdll1rvc[3];
    int m_bufid_coml1rvc[3];
    int m_bufid_b2ldly[3];
    int m_bufid_maxrvc[3];

    ClassDef(TRGGDLDBUnpacker, 1);  /**< ClassDef, must be the last term before the closing {}*/
  };

} // end of namespace Belle2
