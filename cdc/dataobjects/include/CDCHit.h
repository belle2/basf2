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
#include <TObject.h>

namespace Belle2 {
  /** @addtogroup cdc_dataobjects
   *  @ingroup dataobjects
   *  @{ CDCHit
   *  @todo CDCHit : drift time format needs confirmation; is ns really the unit coming out of the unpacker? Is it signed/unsinged?
   *  @}
   */
  /** Class that is the result of the unpacker in raw data and the result of the Digitizer in simulation.
   *
   *  This class is optimized for low disc usage. For reconstruction purposes use the corresponding
   *  CDCRecoHit class or create your own class. <br>
   *
   *  It stores the "Drift Time",<br>
   *  the accumulated ADC count of the charge in the hit cell,<br>
   *  the Super Layer of which the hit wire is part,<br>
   *  the Layer within the Super Layer,<br>
   *  and the wire number within the Layer.<br>
   */
  class CDCHit : public TObject {
  public:
    /** Empty constructor for ROOT IO. */
    CDCHit() :
      m_eWire(65535), m_driftTime(0), m_adcCount(0) {
      B2DEBUG(250, "Empty CDCHit Constructor called.");
    }

    /** Useful Constructor.
     *
     *  Currently the setters are called for the actual assignment, for reducing the number of places where to encode
     *  the transformation into the internal encoding.
     *
     *  @param driftTime   Actually the time measurement has contributions from different other effects, such as TOF, Event Time T0, ...
     *  @param adcCount    ADC count of the accumulated charge in the drift cell for this hit.
     *  @param iSuperLayer Super Layer of the wire.
     *  @param iLayer      Layer number inside the Super Layer.
     *  @param iWire       Wire number in the Layer.
     */
    CDCHit(short driftTime, unsigned short adcCount,
           unsigned short iSuperLayer, unsigned short iLayer, unsigned short iWire);

    /** Constructor using the WireID object. */
    CDCHit(short driftTime, unsigned short adcCount, const WireID& wireID) {
      setDriftTime(driftTime);
      setADCCount(adcCount);
      setWireID(wireID);
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
    void setWireID(unsigned short iSuperLayer, unsigned short iLayer, unsigned short iWire) {
      B2DEBUG(250, "setWireId called with" << iSuperLayer << ", " << iLayer << ", " << iWire);
      m_eWire = iWire + 512 * iLayer + 4096 * iSuperLayer;
    }

    /** Setter for Wire ID using the WireID object directly. */
    void setWireID(const WireID& wireID) {
      m_eWire = wireID.getEWire();
    }

    /** Setter for Drift Time.
     *
     *  @param driftTime  Measurement of Drift Time in ns.
     */
    void setDriftTime(short driftTime) {
      B2DEBUG(250, "setDriftTime called with " << driftTime);
      m_driftTime = driftTime;
    }

    /** Setter for ADC count. */
    void setADCCount(unsigned short adcCount) {
      m_adcCount = adcCount;
    }

    /** Getter for iWire. */
    unsigned short getIWire() const {
      return WireID(m_eWire).getIWire();
    }

    /** Getter for iLayer. */
    unsigned short getILayer() const {
      return WireID(m_eWire).getILayer();
    }

    /** Getter for iSuperLayer. */
    unsigned short getISuperLayer() const {
      return WireID(m_eWire).getISuperLayer();
    }

    /** Getter for encoded wire number.
     *
     *  This number can be used directly e.g. with the = operator to create a WireID object.
     */
    unsigned short getID() const {
      return m_eWire;
    }

    /** Getter for Drift Time. */
    short getDriftTime() const {
      return m_driftTime;
    }

    /** Getter for integrated charge.
     *
     *  Note: This is the integrated charge over the cell.
     *        In principle, this charge can come from more than just the
     *        track, this hit belongs to.
     */
    unsigned short getADCCount() const {
      return m_adcCount;
    }

  protected:
    /** Wire encoding.
     *
     *  Details are now explained in the separate WireID object.
     *  I save only the encoded wire number instead of an WireID object to avoid streaming issues with ROOT.
     */
    unsigned short m_eWire;

    /** Drift Time in ns. */
    short  m_driftTime;

    /** ADC count of the integrated charge in the cell. */
    unsigned short m_adcCount;

  private:
    /** ROOT Macro.*/
    ClassDef(CDCHit, 1);
  };

} // end namespace Belle2

#endif
