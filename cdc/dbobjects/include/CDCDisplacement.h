/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Makoto Uchida                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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

    float getXFwd() const {return m_fwd.X();}
    float getYFwd() const {return m_fwd.Y();}
    float getZFwd() const {return m_fwd.Z();}

    float getXBwd() const {return m_bwd.X();}
    float getYBwd() const {return m_bwd.Y();}
    float getZBwd() const {return m_bwd.Z();}

    float getTension() const {return m_tension;}

  private:
    unsigned short m_wire; /**< Wire ID */
    TVector3 m_fwd; /**< Displacement of forward side */
    TVector3 m_bwd; /**< Displacement of backward side */
    float        m_tension; /**< Displacement of wire tension (grW) */
    ClassDef(CDCDisplacement, 2); /**< ClassDef */
  };

} // end namespace Belle2
