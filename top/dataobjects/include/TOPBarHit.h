/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Petric, Marko Staric                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TOPBARHIT_H
#define TOPBARHIT_H

#include <TObject.h>
#include <TVector3.h>

namespace Belle2 {
  /** \addtogroup dataobjects
   * @{
   */

  /*! Class to store MC particle track parameters at impact to TOP
   * relation to MCParticle
   * filled in top/simulation/src/SensitiveTrack.cc
   */

  class TOPBarHit : public TObject {
  public:

    /*! Default constructor
     */
    TOPBarHit():
      m_trackID(-1),
      m_particleID(-1),
      m_charge(0),
      m_position(0, 0, 0),
      m_vposition(0, 0, 0),
      m_momentum(0, 0, 0),
      m_vmomentum(0, 0, 0),
      m_barID(-1),
      m_length(0.),
      m_globaltime(0.),
      m_localtime(0.) {
    }

    /*!  Full constructor.
     * @param trackID    Geant4 track id
     * @param particleID PDG encoding
     * @param charge     charge
     * @param position   impact point
     * @param vposition  production point
     * @param momentum   impact momentum vector
     * @param vmomentum  production momentum vector
     * @param barID      bar ID
     * @param length     track length
     * @param globaltime global time
     * @param localtime  local time
     */
    TOPBarHit(
      int trackID,
      int particleID,
      int charge,
      TVector3 position,
      TVector3 vposition,
      TVector3 momentum,
      TVector3 vmomentum,
      int barID,
      double length,
      double globaltime,
      double localtime
    ) {
      m_trackID = trackID;
      m_particleID = particleID;
      m_charge = charge;
      m_position = position;
      m_vposition = vposition;
      m_momentum = momentum;
      m_vmomentum = vmomentum;
      m_barID = barID;
      m_length = length;
      m_globaltime = globaltime;
      m_localtime = localtime;
    }

    /*! Get G4 track ID
     * @return G4 track ID
     */
    int getTrackID() const { return m_trackID; }

    /*! Get PDG code
     * @return PDG code
     */
    int getParticleID() const { return m_particleID; }

    /*! Get particle charge
     * @return charge
     */
    int getCharge() const { return m_charge;}

    /*! Get impact point
     * @return impact position
     */
    const TVector3& getPosition() const { return m_position; }

    /*! Get production point
     * @return production position
     */
    const TVector3& getVPosition() const { return m_vposition; }

    /*! Get impact momentum
     * @return impact momentum vector
     */
    const TVector3& getMomentum() const { return m_momentum; }

    /*! Get production momentum
     * @return production momentum vector
     */
    const TVector3& getVMomentum() const { return m_vmomentum; }

    /*! Get impact bar ID
     * @return bar ID
     */
    int getBarID() const { return m_barID;}

    /*! Get tract length from production to impact point
     * @return track length
     */
    double getLength() const { return m_length; }

    /*! Get time since the begining of event
     * @return global time
     */
    double getGlobalTime() const { return m_globaltime; }

    /*! Get time since production of particle
     * @return local time
     */
    double getLocalTime() const { return m_localtime; }

  private:

    int m_trackID;             /**< G4 track id */
    int m_particleID;          /**< PDG code */
    int m_charge;              /**< charge */
    TVector3 m_position;       /**< impact point */
    TVector3 m_vposition;      /**< production point */
    TVector3 m_momentum;       /**< impact momentum vector */
    TVector3 m_vmomentum;      /**< production momentum vector */
    int m_barID;               /**< bar ID */
    double m_length;           /**< track length */
    double m_globaltime;       /**< global time at impact */
    double m_localtime;        /**< local time at impact */

    ClassDef(TOPBarHit, 1);     /**< ClassDef */

  };

  /** @}*/

} // end namespace Belle2

#endif

