/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Tomoyuki Konno
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ARICHRAWDIGIT_H
#define ARICHRAWDIGIT_H

// ROOT
#include <TObject.h>

// std
#include <vector>
#include <map>

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
    }

    ARICHRawDigit(int type, int ver, int boardid, int febno, unsigned int length, unsigned int trgno = 0)
    {
      set(type, ver, boardid, febno, length, trgno);
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
    const std::vector<FEBDigit>& getFEBs() const { return m_febs; }
    std::vector<FEBDigit>& getFEBs() { return m_febs; }

  private:
    int m_copperid;
    int m_hslb;
    int m_type;
    int m_ver;
    int m_boardid;
    int m_febno; // should be 8
    unsigned int m_length;
    unsigned int m_trgno;
    std::vector<FEBDigit> m_febs;

    ClassDef(ARICHRawDigit, 1); /**< the class title */

  };

} // end namespace Belle2

#endif
