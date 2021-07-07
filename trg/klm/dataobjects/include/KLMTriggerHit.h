/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef KLMTRIGGERHIT_H
#define KLMTRIGGERHIT_H

#include <framework/datastore/RelationsObject.h>

namespace Belle2 {
  //! Store KLM TRG hit information as a ROOT object
  class KLMTriggerHit : public RelationsObject {

  public:
    //! Empty constructor for ROOT IO (needed to make the class storable)
    KLMTriggerHit():
      m_section(0),
      m_sector(0),
      m_layer(0),
      m_phiStrip(0),
      m_zStrip(0),
      m_x(0.0),
      m_y(0.0),
      m_z(0.0),
      m_xInt(0),
      m_yInt(0),
      m_zInt(0)
    { }

    //! Constructor with initial values for a hit
    //! @param section detector end (forward=true and backward=false)
    //! @param sector sector number
    //! @param layer layer number
    //! @param phiStrip phi strip number
    //! @param zStrip z strip number
    KLMTriggerHit(int section, int sector, int layer, int phiStrip, int zStrip):
      m_section(section),
      m_sector(sector),
      m_layer(layer),
      m_phiStrip(phiStrip),
      m_zStrip(zStrip),
      m_x(0.0),
      m_y(0.0),
      m_z(0.0),
      m_xInt(0),
      m_yInt(0),
      m_zInt(0)
    { }

    //! Destructor
    virtual ~KLMTriggerHit() {}

    // accessors

    //! Get detector end
    //! @return detector end (forward=true and backward=false)
    int getSection() const { return m_section; }

    //! Get sector number
    //! @return sector number of the hit (0..7)
    int getSector() const { return m_sector; }

    //! Get layer number
    //! @return layer number of this strip (0..14)
    int getLayer() const { return m_layer; }

    //! Get phi strip number
    //! @return phi strip number (0..63)
    int getPhiStrip() const { return m_phiStrip; }

    //! Get z strip number
    //! @return z strip number (0..63)
    int getZStrip() const { return m_zStrip; }

    //! Get x coordinate
    //! @return x coordinate of the hit
    double getX() const { return m_x; }

    //! Get y coordinate
    //! @return y coordinate of the hit
    double getY() const { return m_y; }

    //! Get z coordinate
    //! @return z coordinate of the hit
    double getZ() const { return m_z; }

    //! Get x integer coordinate
    //! @return x integer coordinate of the hit in 1/8 cm
    int getXInt() const { return m_xInt; }

    //! Get y integer coordinate
    //! @return y integer coordinate of the hit in 1/8 cm
    int getYInt() const { return m_yInt; }

    //! Get z integer coordinate
    //! @return z integer coordinate of the hit in 1/8 cm
    int getZInt() const { return m_zInt; }

    // modifiers

    //! Set x coordinate
    //! @param x x coordinate of the hit
    void setX(double x) { m_x = x; }

    //! Set y coordinate
    //! @param y y coordinate of the hit
    void setY(double y) { m_y = y; }

    //! Set z coordinate
    //! @param z z coordinate of the hit
    void setZ(double z) { m_z = z; }

    //! Set x integer coordinate
    //! @param x x integer coordinate of the hit in 1/8 cm
    void setXInt(int xInt) { m_xInt = xInt; }

    //! Set y integer coordinate
    //! @param y y integer coordinate of the hit in 1/8 cm
    void setYInt(int yInt) { m_yInt = yInt; }

    //! Set z integer coordinate
    //! @param z z integer coordinate of the hit in 1/8 cm
    void setZInt(int zInt) { m_zInt = zInt; }

  private:
    //! section number
    int m_section;

    //! sector number
    int m_sector;

    //! layer
    int m_layer;

    //! phi channel number
    int m_phiStrip;

    //! z channel number
    int m_zStrip;

    //! x coordinate
    double m_x;

    //! y coordinate
    double m_y;

    //! z coordinate
    double m_z;

    //! x integer coordinate in 1/8 cm
    int m_xInt;

    //! y integer coordinate in 1/8 cm
    int m_yInt;

    //! z integer coordinate in 1/8 cm
    int m_zInt;

    //! Needed to make the ROOT object storable
    ClassDef(KLMTriggerHit, 3);
  };
} // end of namespace Belle2

#endif //KLMTRIGGERHIT_H
