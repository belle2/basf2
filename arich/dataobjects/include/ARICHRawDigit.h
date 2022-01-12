/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef ARICHRAWDIGIT_H
#define ARICHRAWDIGIT_H

// ROOT
#include <TObject.h>

// std
#include <vector>

namespace Belle2 {

  class ARICHRawDigit : public TObject {

  public:
    struct FEBDigit {
      unsigned char type;
      unsigned char ver;
      unsigned char boardid;
      unsigned char febno; // should be 8
      unsigned int length;
      unsigned int trgno;
      unsigned int febtrgno;
      struct ChannelDigit {
        unsigned char chno;
        unsigned char val;
      };
      mutable std::vector<ChannelDigit> channels;
      const ChannelDigit& operator[](unsigned char i) const
      {
        return channels[i];
      }
      ChannelDigit& operator[](unsigned char i)
      {
        return channels[i];
      }
      unsigned int size() const { return channels.size(); }
      void push_back(unsigned char ich, unsigned char val)
      {
        ChannelDigit channel = {ich, val};
        channels.push_back(channel);
      }
      std::vector<ChannelDigit>& operator()() { return channels; }
      const std::vector<ChannelDigit>& operator()() const { return channels; }
    };

  public:

    //! Default constructor for ROOT IO.
    ARICHRawDigit()
    {
      set(0, 0, 0, 0, 0, 0);
      m_copperid = 0;
      m_hslb = 0;
      m_pcieid = 0;
      m_pciechid = 0;
    }

    ARICHRawDigit(int type, int ver, int boardid, int febno, unsigned int length, unsigned int trgno = 0)
    {
      set(type, ver, boardid, febno, length, trgno);
      m_copperid = 0;
      m_hslb = 0;
      m_pcieid = 0;
      m_pciechid = 0;
    }

    //! Destructor
    ~ARICHRawDigit()
    {
      /*! Does nothing */
    }

    void set(int type, int ver, int boardid, int febno, unsigned int length, unsigned int trgno = 0);

  public:
    int getCopperId() { return m_copperid; }
    int getHslbId() { return m_hslb; }
    int getPcieId() { return m_pcieid; }
    int getPcieChId() { return m_pciechid; }
    int getType() const { return m_type; }
    int getVersion() const { return m_ver; }
    int getBoardId() const { return m_boardid; }
    int getFEBNo() const { return m_febno; }
    unsigned int getLength() const { return m_length; }
    unsigned int getTrgNo() const { return m_trgno; }
    int getNFEBs() const { return m_febs.size(); }
    void addFEB(FEBDigit& feb, int type, int ver, int boardid, int febno,
                unsigned int length, unsigned int trgno, unsigned int febtrgno)
    {
      feb.type = type;
      feb.ver = ver;
      feb.boardid = boardid;
      feb.febno = febno;
      feb.length = length;
      feb.trgno = trgno;
      feb.febtrgno = febtrgno;
      m_febs.push_back(feb);
    }
    void setCopperId(int id) { m_copperid = id; }
    void setHslbId(int id) { m_hslb = id; }
    void setPcieId(int id) { m_pcieid = id; }
    void setPcieChId(int id) { m_pciechid = id; }
    const std::vector<FEBDigit>& getFEBs() const { return m_febs; }
    std::vector<FEBDigit>& getFEBs() { return m_febs; }

  private:
    int m_copperid;
    int m_hslb;
    int m_pcieid;
    int m_pciechid;
    int m_type;
    int m_ver;
    int m_boardid;
    int m_febno; // should be 8
    unsigned int m_length;
    unsigned int m_trgno;
    std::vector<FEBDigit> m_febs;

    ClassDef(ARICHRawDigit, 2); /**< the class title */

  };

} // end namespace Belle2

#endif
