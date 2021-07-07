/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/logging/Logger.h>
#include <cdc/dataobjects/WireID.h>
#include <framework/dataobjects/DigitBase.h>

namespace Belle2 {
  /** Class containing the result of the unpacker in raw data and the result of the digitizer in simulation.
   *
   *  This class is optimized for low disc usage. <br>
   *  For reconstruction purposes, use the corresponding CDCRecoHit class or create your own class. <br>
   *
   *  It stores the TDC count (timing information of the hit),<br>
   *  the accumulated ADC count of the charge in the hit cell,<br>
   *  and the WireID.
   *
   *  Regarding the treatment of 2nd fastest hit timing recorded by the front end, if it exists, <br>
   *  the unpacker creates another individual CDCHit object for the 2nd hit in addition to the CDCHit for the 1st hit. <br>
   *  In both case, the hit timing is stored as the TDC count. <br>
   *  To identify if the CDCHit is 1st hit or 2nd hit, the first bit of member variable, m_status, is assigned. <br>
   *  If such bit is 0(1), The CDCHit belongs to the 1st (2nd) hit. <br>
   *  The method CDCHit::is2ndHit() has to be used for this purpose.
   *
   *  The relastion between the 1st hit object and the 2nd hit object is established with the variable, m_otherHitIndex.<br>
   *  Users can call the CDCHit::getOtherHitIndex() to obtain the index of CDCHit array for the other hit.
   *
   */
  class CDCHit : public DigitBase {
  public:
    /** Empty constructor for ROOT IO. */
    CDCHit() :
      m_eWire(65535), m_tdcCount(0), m_adcCount(0), m_status(0), m_tot(0), m_otherHitIndex(-1), m_adcCountAtLeadingEdge(0)
    {
      B2DEBUG(29, "Empty CDCHit Constructor called.");
    }

    /** Constructor to set all internal variables.
     *
     *  Currently the setters are called for the actual assignment, for reducing the number of places where to encode
     *  the transformation into the internal encoding.
     *
     *  @param tdcCount    Timing measurement of the hit.
     *  @param adcCount    ADC count of the accumulated charge in the drift cell for this hit.
     *  @param iSuperLayer Super Layer of the wire.
     *  @param iLayer      Layer number inside the Super Layer.
     *  @param iWire       Wire number in the Layer.
     *  @param status         Status of the hit.
     *  @param tot            Time over threshold.
     *  @param otherHitIndex  Index to the other hit.
     *  @param leadingEdgeADC FADCcount at a sampling point near the leading edge.
     */
    CDCHit(unsigned short tdcCount, unsigned short adcCount,
           unsigned short iSuperLayer, unsigned short iLayer, unsigned short iWire, unsigned short status = 0, unsigned short tot = 0,
           signed short otherHitIndex = -1, unsigned short leadingEdgeADC = 0);

    /** Constructor using the WireID object. */
    CDCHit(unsigned short tdcCount, unsigned short adcCount, const WireID& wireID, unsigned short status = 0, unsigned short tot = 0,
           signed short otherHitIndex = -1, unsigned short leadingEdgeADC = 0)
    {
      setTDCCount(tdcCount);
      setADCCount(adcCount);
      setWireID(wireID);
      setStatus(status);
      setTOT(tot);
      setOtherHitIndex(otherHitIndex);
      setADCCountAtLeadingEdge(leadingEdgeADC);
    }

    /** Setter for Wire ID.
     *
     *  The numbering scheme is the same as in the one used in
     *  <a href="http://ekpbelle2.physik.uni-karlsruhe.de/~twiki/pub/Detector/CDC/WebHome/cdc_cell_num.pdf">this</a>
     *  document.
     *  @param iSuperLayer Values should be between [0,   8].
     *  @param iLayer      Values should be between [0,   7], depending on the SuperLayer.
     *  @param iWire       Values should be between [0, 511], depending on the SuperLayer.
     */
    void setWireID(unsigned short iSuperLayer, unsigned short iLayer, unsigned short iWire)
    {
      B2DEBUG(29, "setWireId called with" << iSuperLayer << ", " << iLayer << ", " << iWire);
      m_eWire = WireID(iSuperLayer, iLayer, iWire).getEWire();
    }

    /** Setter for Wire ID using the WireID object directly. */
    void setWireID(const WireID& wireID)
    {
      m_eWire = wireID.getEWire();
    }

    /** Setter for CDCHit status.
     *
     *  @param status  indicates the CDCHit object status.
     */
    void setStatus(unsigned short status)
    {
      B2DEBUG(29, "setStatus called with " << status);
      m_status = status;
    }

    /** Setter for 2nd hit flag. */
    void set2ndHitFlag()
    {
      m_status |= 0x01;
    }

    /** Setter for already-checked flag. */
    void setAlreadyCheckedFlag()
    {
      m_status |= 0x02;
    }

    /** Setter for TDC count.
     *
     *  @param tdcCount  Information for timing of the hit.
     */
    void setTDCCount(short tdcCount)
    {
      B2DEBUG(29, "setTDCCount called with " << tdcCount);
      m_tdcCount = tdcCount;
    }

    /** Setter for ADC count. */
    void setADCCount(unsigned short adcCount)
    {
      m_adcCount = adcCount;
    }

    /** Setter for the other hit index. */
    void setOtherHitIndex(signed short index)
    {
      m_otherHitIndex = index;
    }

    /** Setter for the other hit indices. */
    void setOtherHitIndices(CDCHit* otherHit)
    {
      m_otherHitIndex = otherHit->getArrayIndex();
      otherHit->setOtherHitIndex(this->getArrayIndex());
    }

    /** Setter for ADCcount at leading edge. */
    void setADCCountAtLeadingEdge(unsigned short adcCount)
    {
      m_adcCountAtLeadingEdge = adcCount;
    }

    /** Setter for TOT. */
    void setTOT(unsigned short tot)
    {
      m_tot = tot;
    }

    /** Getter for iWire. */
    unsigned short getIWire() const
    {
      return WireID(m_eWire).getIWire();
    }

    /** Getter for iLayer. */
    unsigned short getILayer() const
    {
      return WireID(m_eWire).getILayer();
    }

    /** Getter for iCLayer (0-55). */
    unsigned short getICLayer() const
    {
      return WireID(m_eWire).getICLayer();
    }

    /** Getter for iSuperLayer. */
    unsigned short getISuperLayer() const
    {
      return WireID(m_eWire).getISuperLayer();
    }

    /** Getter for encoded wire number.
     *
     *  This number can be used directly e.g. with the = operator to create a WireID object.
     */
    unsigned short getID() const
    {
      return m_eWire;
    }

    /** Getter for CDCHit status. */
    unsigned short getStatus() const
    {
      return m_status;
    }

    /** Getter for 2nd hit flag. */
    bool is2ndHit() const
    {
      bool tOrf = (m_status & 0x01) ? true : false;
      return tOrf;
    }

    /** Getter for already-checked flag. */
    bool isAlreadyChecked() const
    {
      bool tOrf = (m_status & 0x02) ? true : false;
      return tOrf;
    }

    /** Getter for TDC count. */
    short getTDCCount() const
    {
      return m_tdcCount;
    }

    /** Getter for integrated charge.
     *
     *  Note: This is the integrated charge over the cell.
     *        In principle, this charge can come from more than just the
     *        track, this hit belongs to.
     */
    unsigned short getADCCount() const
    {
      return m_adcCount;
    }

    /** Getter for otherHitIndex. */
    signed short getOtherHitIndex() const
    {
      return m_otherHitIndex;
    }

    /** Getter for adcCountAtLeadingEdge. */
    unsigned short getADCCountAtLeadingEdge() const
    {
      return m_adcCountAtLeadingEdge;
    }

    /** Getter for TOT. */
    unsigned short getTOT() const
    {
      return m_tot;
    }

    /**
     * Implementation of the base class function.
     * Enables BG overlay module to identify uniquely the physical channel of this Digit.
     * @return unique channel ID, the WireID is returned for the CDC.
     */
    unsigned int getUniqueChannelID() const override {return static_cast<unsigned int>(m_eWire);}

    /**
     * Implementation of the base class function.
     * Overlay method.
     * @param bg BG digit
     * @return append status
     */
    DigitBase::EAppendStatus addBGDigit(const DigitBase* bg) override;


  protected:

    /** Wire encoding.
     *
     *  Details are now explained in the separate WireID object.
     *  I save only the encoded wire number instead of an WireID object to avoid streaming issues with ROOT.
     */
    unsigned short m_eWire;

    /** TDC count in ns. */
    unsigned short  m_tdcCount;

    /** ADC count of the integrated charge in the cell. */
    unsigned short m_adcCount;

    /** Status of CDCHit. */
    unsigned short  m_status;

    /** Time over threshod. */
    unsigned short  m_tot;

    /** Index to the other hit. */
    signed short  m_otherHitIndex;

    /** ADC count at leading edge. */
    unsigned short  m_adcCountAtLeadingEdge;

  private:
    /** ROOT Macro.*/
    ClassDefOverride(CDCHit, 8);
  };
} // end namespace Belle2
