/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/logging/Logger.h>

#include <limits>

#include <framework/datastore/RelationsObject.h>

namespace Belle2 {

  /** @addtogroup CDC_dataobjects
   *  @ingroup dataobjects
   *  @{ WireID
   *  @todo WireID : Rootification might not be necessary; On the other hand, it opens as well the use via Python.
   *  @}
   */
  /** Class to identify a wire inside the CDC.
   *
   *  There are three representations for this: <br>
   *  - Super-Layer, Layer within the Super-Layer, Wire within the Layer as documented
   *  <a href="http://ekpbelle2.physik.uni-karlsruhe.de/~twiki/pub/Detector/CDC/WebHome/cdc_cell_num.pdf">here</a>, <br>
   *  - Layer as continuous counted through all Super-Layers, Wire within the Layer, <br>
   *  - Wire number encoded into a single unsigned short. This works as following: <br>
   *  SuperLayer: bits 1 -  4 (/4096)           <br>
   *  Layer:      bits 5 -  7 (% 4096, / 512)   <br>
   *  Wire:       bits 8 - 16 (% 512)           <br>
   *  Note: These operations could as well be achieved by bit-shifting operations.
   */
  class WireID : public RelationsObject {
  public:
    //--- Constructors ---------------------------------------------
    /** Constructor taking the encoded wire number.
     *
    *  @param eWire  Number of wire using the encoded format. Careful - wires are not continuously within the encoded number.
    *                The default is set to the maximum allowed value, as 0 corresponds already to some wire.
    *                This assigned value does NOT correspond to any real wire.
    */


    explicit WireID(unsigned short eWire = 65535) {
      m_eWire = eWire;
    }

    /** Copy constructor. */
    WireID(const WireID& wireID) : RelationsObject(wireID) {
      m_eWire = wireID.m_eWire;
    }

    /** Constructor using the official numbering scheme.
     *
     *  @param iSuperLayer  Number of Super-Layer.
     *  @param iLayer   Number of Layer within the Super-Layer.
     *  @param iWire    Wire number within the layer.
     */
    WireID(unsigned short iSuperLayer, unsigned short iLayer, unsigned short iWire);

    /** Constructor using continuous layer numbers as used in the geometry build-up for the simulation.
     *
     *  @param iCLayer  Number of Layer counted continuously over the full CDC.
     *  @param iWire    Number of Wire within the layer equivalenty to the official numbering.
     */
    WireID(unsigned short iCLayer, unsigned short iWire);


    //--- Operators for various purposes. ---------------------------------------------------------------------------------------
    /** Assignment operator. */
    WireID& operator=(const WireID& wireID) {
      m_eWire = wireID.m_eWire;
      static_cast<RelationsObject>(*this) = wireID;
      return *this;
    }

    /** Assignment from unsigned short.
     *
     *  @param eWire  This parameter should be the wire number in the encoded format.
     */
    WireID& operator=(unsigned short eWire) {
      m_eWire = eWire;
      return *this;
    }

    /** Convert to unsigned short. */
    operator unsigned short() const {
      return getEWire();
    }

    /** Check for equality. */
    bool operator==(const WireID& rhs) const {
      return getEWire() == rhs.getEWire();
    }

    /** Order by unique id */
    bool operator<(const WireID& rhs) const {
      return getEWire() < rhs.getEWire();
    }


    //--- Setters ---------------------------------------------------------------------------------------------------------------
    /** Setter using official numbering. */
    void setWireID(unsigned short iSuperLayer, unsigned short iLayer, unsigned short iWire) {
      B2DEBUG(250, "setWireID called with " << iSuperLayer << ", " << iLayer << ", " << iWire);
      m_eWire = iWire + 512 * iLayer + 4096 * iSuperLayer;
    }

    /** Setter using numbering of geometry build-up. */
    void setWireID(unsigned short iCLayer, unsigned short iWire);

    /** Setter using the encoded wire number. */
    void setWireID(unsigned short eWire) {
      B2DEBUG(250, "setWireID called with " << eWire);
      m_eWire = eWire;
    }

    //--- Getters -------------------------------------------------------------------------------------------------
    /** Getter for Super-Layer. */
    unsigned short getISuperLayer() const {
      return (m_eWire / 4096);
    }

    /** Getter for layer within the Super-Layer. */
    unsigned short getILayer() const {
      return ((m_eWire % 4096) / 512);
    }

    /** Getter for wire within the layer.
     *
     *  This getter works for the official numbering scheme as well as the one used in the geometry build-up.
     */
    unsigned short getIWire() const {
      return (m_eWire % 512);
    }

    /** Getter for continuous layer numbering. */
    unsigned short getICLayer() const;

    /** Getter for encoded wire number. */
    unsigned short getEWire() const {
      return m_eWire;
    }

  private:
    /** Internal storage for the wire identifier in the encoded form. */
    unsigned short m_eWire;

    /** ROOT Macro.*/
    ClassDef(WireID, 0);
  };
}

