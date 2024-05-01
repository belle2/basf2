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

  /**
   * Class of ARICH raw digits
   */
  class ARICHRawDigit : public TObject {

  public:
    /**
     * Struct for front-end board
     */
    struct FEBDigit {
      unsigned char type; /**< type */
      unsigned char ver; /**< version */
      unsigned char boardid; /**< board ID */
      unsigned char febno; /**< FEB number which should be 8 */
      unsigned int length; /**< length */
      unsigned int trgno; /**< trigger number */
      unsigned int febtrgno; /**< front-end board trigger number */

      /**
       * Struct for ChannelDigit within FEBDigit struct
       */
      struct ChannelDigit {
        unsigned char chno; /**< channel number */
        unsigned char val; /**< value */
      };

      mutable std::vector<ChannelDigit> channels; /**< vector of ChannelDigits */

      /**
       * Access given ChannelDigit (const version)
       */
      const ChannelDigit& operator[](unsigned char i) const
      {
        return channels[i];
      }

      /**
       * Access given ChannelDigit
       */
      ChannelDigit& operator[](unsigned char i)
      {
        return channels[i];
      }

      /**
       * Get number of channels
       */
      unsigned int size() const { return channels.size(); }

      /**
       * Add channel
       */
      void push_back(unsigned char ich, unsigned char val)
      {
        ChannelDigit channel = {ich, val};
        channels.push_back(channel);
      }

      /**
       * Get vector of ChannelDigits
       */
      std::vector<ChannelDigit>& operator()() { return channels; }

      /**
       * Get vector of ChannelDigits (const version)
       */
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

    //! Constructor
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

    /**
     * Set properties
     */
    void set(int type, int ver, int boardid, int febno, unsigned int length, unsigned int trgno = 0);

  public:
    /**
     * Get COPPER ID
     */
    int getCopperId() { return m_copperid; }

    /**
     * Get HSLB ID
     */
    int getHslbId() { return m_hslb; }

    /**
     * Get PCIe ID
     */
    int getPcieId() { return m_pcieid; }

    /**
     * Get PCIe channel ID
     */
    int getPcieChId() { return m_pciechid; }

    /**
     * Get type
     */
    int getType() const { return m_type; }

    /**
     * Get version
     */
    int getVersion() const { return m_ver; }

    /**
     * Get board ID
     */
    int getBoardId() const { return m_boardid; }

    /**
     * Get FEB number
     */
    int getFEBNo() const { return m_febno; }

    /**
     * Get length
     */
    unsigned int getLength() const { return m_length; }

    /**
     * Get trigger number
     */
    unsigned int getTrgNo() const { return m_trgno; }

    /**
     * Get number of FEBs
     */
    int getNFEBs() const { return m_febs.size(); }

    /**
     * Add properties of FEB
     */
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

    /**
     * Set COPPER ID
     */
    void setCopperId(int id) { m_copperid = id; }

    /**
     * Set HSLB ID
     */
    void setHslbId(int id) { m_hslb = id; }

    /**
     * Set PCIe ID
     */
    void setPcieId(int id) { m_pcieid = id; }

    /**
     * Set PCIe channel ID
     */
    void setPcieChId(int id) { m_pciechid = id; }

    /**
     * Get const vector of FEB digits
     */
    const std::vector<FEBDigit>& getFEBs() const { return m_febs; }

    /**
     * Get vector of FEB digits
     */
    std::vector<FEBDigit>& getFEBs() { return m_febs; }

  private:
    int m_copperid; /**< COPPER ID */
    int m_hslb; /**< High-speed link board */
    int m_pcieid; /**< PCIe ID */
    int m_pciechid; /**< PCIe channel ID */
    int m_type; /**< type */
    int m_ver; /**< version */
    int m_boardid; /**< board ID */
    int m_febno; /**< FEB number which should be 8 */
    unsigned int m_length; /**< length */
    unsigned int m_trgno; /**< trigger number */
    std::vector<FEBDigit> m_febs; /**< vector of front-end board digits */

    ClassDef(ARICHRawDigit, 2); /**< the class title */

  };

} // end namespace Belle2

#endif
