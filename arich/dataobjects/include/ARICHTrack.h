/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef ARICHTRACK_H
#define ARICHTRACK_H

#include <tracking/dataobjects/ExtHit.h>
#include <arich/dataobjects/ARICHPhoton.h>
#include <arich/dataobjects/ARICHAeroHit.h>
#include <vector>

namespace Belle2 {

  /**
   *   Datastore class that holds position and momentum information
   *   of tracks that hit ARICH. It can be created either form ExtHit
   *   (i.e. mdst Track extrapolated to ARICH aerogel plane) or ARICHAeroHit
   *   (i.e. MC hit of particle on ARICH aerogel plane).
   *   If selected, information of individual reconstruced photons associated
   *   with track (cherenkov angles) can be stored (see ARICHPhoton.h)
   */

  class ARICHTrack : public RelationsObject {
  public:

    /**
     *Empty constructor for ROOT IO
     */
    ARICHTrack()
    {}

    /**
     *Useful constructor
     * @param position   position vector of track on aerogel plane
     * @param momentum   momentum vector of track on aerogel plane
     */
    ARICHTrack(TVector3 position,
               TVector3 momentum)
    {

      m_x = (float) position.x();
      m_y = (float) position.y();
      m_z = (float) position.z();
      m_momentum = (float) momentum.Mag();
      momentum = momentum.Unit();
      m_dx = (float) momentum.x();
      m_dy = (float) momentum.y();
      m_dz = (float) momentum.z();

    }


    /**
     * Constructor from ARICHAeroHit
     * @param ARICHAeroHit   ARICHAeroHit
     */
    explicit ARICHTrack(const ARICHAeroHit* aeroHit)
    {

      TVector3 pos = aeroHit->getPosition();
      m_x = (float) pos.X();
      m_y = (float) pos.Y();
      m_z = (float) pos.Z();

      TVector3 mom = aeroHit->getMomentum();
      m_momentum = (float) mom.Mag();
      mom = mom.Unit();
      m_dx = (float) mom.X();
      m_dy = (float) mom.Y();
      m_dz = (float) mom.Z();
    }


    /**
     * Constructor from ExtHit
     * @param ExtHit  ExtHit
     */
    explicit ARICHTrack(const ExtHit* extHit)
    {

      TVector3 pos = extHit->getPosition();
      m_x = (float) pos.X();
      m_y = (float) pos.Y();
      m_z = (float) pos.Z();

      TVector3 mom = extHit->getMomentum();
      m_momentum = (float) mom.Mag();
      mom = mom.Unit();
      m_dx = (float) mom.X();
      m_dy = (float) mom.Y();
      m_dz = (float) mom.Z();
    }


    /**
     * Set information about hit on HAPD window
     * @param extHit extrapolated ExtHit to HAPD plane
     */
    void setHapdWindowHit(const ExtHit* extHit)
    {
      TVector3 pos = extHit->getPosition();
      m_winX = (float) pos.X();
      m_winY = (float) pos.Y();
      m_hitWin = true;
    }

    /**
     * Set information about hit on HAPD window
     * @param x x position of hit
     * @param y y position of hit
     */
    void setHapdWindowHit(double x, double y)
    {
      m_winX = (float) x;
      m_winY = (float) y;
      m_hitWin = true;
    }


    /**
     * returns track position vector
     * @return track position vector
     */
    TVector3 getPosition() const { return TVector3(m_x, m_y, m_z);}

    /**
     * returns track direction vector
     * @return track direction vector
     */
    TVector3 getDirection() const { return TVector3(m_dx, m_dy, m_dz);}

    /**
     * returns track momentum
     * @return track momentum
     */
    double getMomentum() const { return double(m_momentum);}

    /**
     * Returns true if track hits HAPD window
     * @return true if track hits HAPD window
     */
    bool hitsWindow() const { return m_hitWin;}

    /**
     * Get HAPD window hit position
     * @return HAPD window hit position
     */
    TVector2 windowHitPosition() const {return TVector2(m_winX, m_winY);}


    /**
     * Sets the reconstructed value of track parameters.
     * @param r     position of track
     * @param dir   direction vector of track (unit vector)
     * @param p     track momentum
     */
    void  setReconstructedValues(TVector3 r, TVector3 dir, double p)
    {
      m_x = (float) r.X();
      m_y = (float) r.Y();
      m_z = (float) r.Z();
      m_dx = (float) dir.X();
      m_dy = (float) dir.Y();
      m_dz = (float) dir.Z();
      m_momentum = (float) p;
    }

    /**
     * Add ARICHPhoton to collection of photons
     */
    void addPhoton(ARICHPhoton photon)
    {
      m_photons.emplace_back(photon);
    }

    /**
     * Returns vector of ARICHPhoton's associated with track
     * @return vector of ARICHPhoton's
     */
    const std::vector<ARICHPhoton>& getPhotons() const { return m_photons; }

  private:

    float m_x = 0;     /**< Reconstructed position.  */
    float m_y = 0;     /**< Reconstructed position.  */
    float m_z = 0;     /**< Reconstructed position.  */
    float m_dx = 0;   /**< Reconstructed direction. */
    float m_dy = 0;   /**< Reconstructed direction. */
    float m_dz = 0;   /**< Reconstructed direction. */
    float m_momentum;        /**< Reconstructed momentum. */

    bool m_hitWin = 0;    /**< true if track hits HAPD window */
    float m_winX = 0;     /**< x position of track extrapolated to HAPD plane */
    float m_winY = 0;     /**< y position of track extrapolated to HAPD plane  */


    std::vector<ARICHPhoton> m_photons; /**< collection of ARICHPhotons associated with the track */

    ClassDef(ARICHTrack, 2); /**< the class title */

  };
} // namespace Belle2

#endif // ARICHTRACK_H
