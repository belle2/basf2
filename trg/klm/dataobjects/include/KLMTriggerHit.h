/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Dmitri Liventsev                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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
      m_fwd(false),
      m_sector(0),
      m_layer(0),
      m_phiStrip(0),
      m_zStrip(0),
      m_x(0.0),
      m_y(0.0),
      m_z(0.0)
    { }

    //! Constructor with initial values for a hit
    //! @param fwd detector end (forward=true and backward=false)
    //! @param sector sector number
    //! @param layer layer number
    //! @param nphi phi strip number
    //! @param nz z strip number
    //! @param x x coordinate of the hit
    //! @param y y coordinate of the hit
    //! @param z x coordinate of the hit
    KLMTriggerHit(bool fwd, int sector, int layer, int nphi, int nz, double x = 0.0, double y = 0.0, double z = 0.0):
      m_fwd(fwd),
      m_sector(sector),
      m_layer(layer),
      m_phiStrip(nphi),
      m_zStrip(nz),
      m_x(x),
      m_y(y),
      m_z(z)
    { }

    //! Destructor
    virtual ~KLMTriggerHit() {}

    // accessors

    //! Get detector end
    //! @return detector end (forward=true and backward=false)
    bool isForward() const { return m_fwd; }

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

  private:
    //! forward-backward
    bool m_fwd;

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

    //! Needed to make the ROOT object storable
    ClassDef(KLMTriggerHit, 1);
  };
} // end of namespace Belle2

#endif //KLMTRIGGERHIT_H
