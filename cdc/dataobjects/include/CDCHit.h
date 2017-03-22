/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CDCHIT_H
#define CDCHIT_H

#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
#include <cdc/dataobjects/WireID.h>
// #include <framework/datastore/RelationsObject.h>
#include <framework/dataobjects/DigitBase.h>

namespace Belle2 {
  /** Class that is the result of the unpacker in raw data and the result of the Digitizer in simulation.
   *
   *  This class is optimized for low disc usage. For reconstruction purposes use the corresponding
   *  CDCRecoHit class or create your own class. <br>
   *
   *  It stores the TDC count (timing information of the hit),<br>
   *  the accumulated ADC count of the charge in the hit cell,<br>
   *  and the WireID.
   */
  class CDCHit : public DigitBase {
  public:
    /** Empty constructor for ROOT IO. */
    CDCHit() :
      m_eWire(65535), m_tdcCount(0), m_adcCount(0), m_tdcCount2ndHit(0), m_status(0)
    {
      B2DEBUG(250, "Empty CDCHit Constructor called.");
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
     *  @param tdcCount2ndHit 2nd timing measurement.
     *  @param status         Status of the hit.
     */
    CDCHit(unsigned short tdcCount, unsigned short adcCount,
           unsigned short iSuperLayer, unsigned short iLayer, unsigned short iWire, unsigned short tdcCont2ndHit = 0,
           unsigned short status = 0);

    /** Constructor using the WireID object. */
    CDCHit(unsigned short tdcCount, unsigned short adcCount, const WireID& wireID, unsigned short tdcCount2ndHit = 0,
           unsigned short status = 0)
    {
      setTDCCount(tdcCount);
      setADCCount(adcCount);
      setWireID(wireID);
      setTDCCount2ndHit(tdcCount2ndHit);
      setStatus(status);
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
      B2DEBUG(250, "setWireId called with" << iSuperLayer << ", " << iLayer << ", " << iWire);
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
      B2DEBUG(250, "setStatus called with " << status);
      m_status = status;
    }

    /** Setter for TDC count.
     *
     *  @param tdcCount  Information for timing of the hit.
     */
    void setTDCCount(short tdcCount)
    {
      B2DEBUG(250, "setTDCCount called with " << tdcCount);
      m_tdcCount = tdcCount;
    }

    /** Setter for 2nd TDC count. */
    void setTDCCount2ndHit(short tdc)
    {
      B2DEBUG(250, "setTDCCount2ndHit called with " << tdc);
      m_tdcCount2ndHit = tdc;
    }

    /** Setter for ADC count. */
    void setADCCount(unsigned short adcCount)
    {
      m_adcCount = adcCount;
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

    /** Getter for TDC count. */
    short getTDCCount() const
    {
      return m_tdcCount;
    }

    /** Getter for TDC count of 2nd hit. */
    short getTDCCount2ndHit() const
    {
      return m_tdcCount2ndHit;
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


    /**
     * Implementation of the base class function.
     * Enables BG overlay module to identify uniquely the physical channel of this Digit.
     * @return unique channel ID, the WireID is returned for the CDC.
     */
    unsigned int getUniqueChannelID() const {return static_cast<unsigned int>(m_eWire);}

    /**
     * Implementation of the base class function.
     * Pile-up method.
     * @param bg BG digit
     * @return append status
     */
    DigitBase::EAppendStatus addBGDigit(const DigitBase* bg);


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

    /** TDC count in ns (2nd hit). */
    unsigned short  m_tdcCount2ndHit;

    /** Status of CDCHit. */
    unsigned short  m_status;


  private:
    /** ROOT Macro.*/
    ClassDef(CDCHit, 6);
  };
} // end namespace Belle2

#endif
