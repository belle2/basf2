/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <TObject.h>
#include <TVector3.h>
#include <cdc/dataobjects/WireID.h>


namespace Belle2 {

  /**
   * Database object for displacement of sense wire position.
   */
  class CDCDisplacement: public TObject {
  public:

    /**
     * Default constructor.
     */
    CDCDisplacement() :
      m_wire(65535), m_fwd(), m_bwd(), m_tension()
    {
    }


    /**
     * Constructor.
     */
    CDCDisplacement(const WireID& wire, TVector3 fwd, TVector3 bwd, float tension) :
      m_wire(wire), m_fwd(fwd), m_bwd(bwd), m_tension(tension)
    {
    }

    /** Getter for EWireID. */
    unsigned short getEWire() const
    {
      return m_wire;
    }

    //TODO: remove the following 4 functions (not needed).

    /** Getter for Super-Layer. */
    unsigned short getISuperLayer() const
    {
      return (m_wire / 4096);
    }

    /** Getter for layer within the Super-Layer. */
    unsigned short getILayer() const
    {
      return ((m_wire % 4096) / 512);
    }

    /** Getter for wire within the layer.
     *
     *  This getter works for the official numbering scheme as well as the one used in the geometry build-up.
     */
    unsigned short getIWire() const
    {
      return (m_wire % 512);
    }

    /** Getter for continuous layer numbering. */
    int getICLayer() const
    {
      if (getISuperLayer() == 0) { return getILayer(); }
      return 8 + (getISuperLayer() - 1) * 6 + getILayer();
    }

    /** Getter for fwd x-position. */
    float getXFwd() const {return m_fwd.X();}

    /** Getter for fwd y-position. */
    float getYFwd() const {return m_fwd.Y();}

    /** Getter for fwd z-position. */
    float getZFwd() const {return m_fwd.Z();}

    /** Getter for bwd x-position. */
    float getXBwd() const {return m_bwd.X();}

    /** Getter for bwd y-position. */
    float getYBwd() const {return m_bwd.Y();}

    /** Getter for bwd z-position. */
    float getZBwd() const {return m_bwd.Z();}

    /** Getter for wire tension. */
    float getTension() const {return m_tension;}

  private:
    unsigned short m_wire; /**< Wire ID */
    //TODO: replace TVector3s with simple arrays to save memory.
    TVector3 m_fwd; /**< Displacement of forward side */
    TVector3 m_bwd; /**< Displacement of backward side */
    float        m_tension; /**< Displacement of wire tension (grW) */
    ClassDef(CDCDisplacement, 3); /**< ClassDef */
  };

} // end namespace Belle2
