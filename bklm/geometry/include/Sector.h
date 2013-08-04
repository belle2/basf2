/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Leo Piilonen                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef BKLMSECTOR_H
#define BKLMSECTOR_H

#include <vector>

#include "CLHEP/Vector/ThreeVector.h"
#include "CLHEP/Vector/Rotation.h"
#include "CLHEP/Matrix/Matrix.h"

#include "bklm/geometry/Module.h"
#include "bklm/geometry/Rect.h"

namespace Belle2 {

  namespace bklm {

    /*! Define the geometry of a BKLM sector
     *  Each sector [octant] contains Modules.  The local geometry of a
     *  sector is oriented to the position of sector 0 by a rotation about
     *  the global z axis.
     *
     *       SECTOR 0 GLOBAL COORDINATES        SECTOR s LOCAL COORDINATES
     *
     *   +y                  _                               +y    _
     *   ^                  / |                              ^    / |
     *   |                /   |                              |  /   |
     *   |              /     |                              |/     |
     *   |            /       |                             /|      |
     *   |            |   end |                             ||  end |
     *   + - - - - -  |  view | - - > +x         - - - - -  |+ view | - - > +x
     *                |       |                             |       |
     *                \       |                             \       |
     *                  \     |                               \     |
     *                    \   |                                 \   |
     *                      \_|                                   \_|
     *
     *   The local origin is shifted along the +x axis so that it coincides with
     *   the middle of the layer 0 gap (=slot).  Outer layers are at larger values
     *   of local x.  The local origin is shifted along the z axis so that it
     *   lies at the boundary between the forward and backward halves of the barrel.
     *   The orientation of the local z axis is flipped from the global orientation
     *   for the backward sectors so that all sectors extend along the local +z
     *   axis from the local z=0 (which is nearest the IP).
     */
    class Sector {

    public:

      //! Empty constructor
      Sector();

      //! Constructor with explicit values (excludes list of contained modules)
      Sector(bool               isForward,
             int                sector,
             int                nlayers,
             CLHEP::Hep3Vector  translation,
             CLHEP::HepRotation rotation);

      //! Copy constructor (includes list of contained modules)
      Sector(const Sector& s);

      //! Destructor
      ~Sector();

      //! Assignment operator (includes list of contained modules)
      Sector& operator=(const Sector& s);

      //! Comparison operator (ordering)
      bool operator<(const Sector&) const;

      //! Comparison operator (equality)
      bool operator==(const Sector& s) const { return isSameSector(s); }

      //! Add a module to this sector
      void addModule(Module*);

      //! Determine if two sectors are identical
      bool isSameSector(bool isForward, int sector) const;

      //! Determine if two sectors are identical
      bool isSameSector(const Sector& s) const;

      //! Determine if two sectors are identical
      bool isSameSector(const Module& m) const;

      //! Get sector's end (forward or backward)
      bool isForward() const { return m_IsForward; }

      //! Get sector number
      int getSector() const { return m_Sector; }

      //! Get number of layers in this sector
      int getNLayer() const { return m_NLayer; }

      //! Get sector's translation (in global coordinates)
      const CLHEP::Hep3Vector& getTranslation() const { return m_Translation; }

      //! Get sector's orientation (in global coordinates)
      const CLHEP::HepRotation& getRotation() const { return m_Rotation; }

      //! Get the pointer to a module within this sector
      const Module* findModule(int layer) const;

      //! Transform space-point within this sector from local to global coordinates
      const CLHEP::Hep3Vector localToGlobal(const CLHEP::Hep3Vector& v) const;

      //! Transform space-point within this sector from global to local coordinates
      const CLHEP::Hep3Vector globalToLocal(const CLHEP::Hep3Vector& v) const;

      //! Transform space-point within this sector from global to local coordinates
      const CLHEP::Hep3Vector globalToLocal(double x, double y, double z) const;

      //! Transform rotation matrix within this sector from local to global coordinates
      const CLHEP::HepMatrix localToGlobal(const CLHEP::HepMatrix& m) const;

      //! Transform rotation matrix within this sector from global to local coordinates
      const CLHEP::HepMatrix globalToLocal(const CLHEP::HepMatrix& m) const;

      //! Transform space-points of bounding rectangle from local to global coordinates
      const Rect localToGlobal(const Rect& r) const;

      //! Transform space-points of bounding rectangle from global to local coordinates
      const Rect globalToLocal(const Rect& r) const;

      //! Rotate direction or momentum vector from local to global coordinates
      const CLHEP::Hep3Vector rotateToGlobal(const CLHEP::Hep3Vector& v) const;

      //! Rotate direction or momentum vector from global to local coordinates
      const CLHEP::Hep3Vector rotateToLocal(const CLHEP::Hep3Vector& v) const;

      //! Get the unit vector normal to this sector (or any module within), in global coordinates
      const CLHEP::Hep3Vector getNormal() const;

      //! Print sector definition
      void printTree() const;

    private:

      //! to store the axial end (true=forward or false=backward) of this sector
      bool m_IsForward;

      //! to store the sector number of this sector
      int m_Sector;

      //! to store the number of layers in this sector
      int m_NLayer;

      //! to store the position (in global coordinates) of this sector
      CLHEP::Hep3Vector m_Translation;

      //! to store the rotation matrix (in global coordinates) of this sector
      CLHEP::HepRotation m_Rotation;

      //! to store the inverse of the rotation matrix (in global coordinates) of this sector
      CLHEP::HepRotation m_RotationInverse;

      //! to store a copy of the rotation matrix in alternate form
      CLHEP::HepMatrix m_RotationMatrix;

      //! to store a copy of the inverse of the rotation matrix in alternate form
      CLHEP::HepMatrix m_RotationInverseMatrix;

      //! to store the unit normal vector (in global coordinates) of this sector or any module therein
      CLHEP::Hep3Vector m_Normal;

      //! to store the array of modules contained in this sector
      std::vector<Module*> m_Modules;

    };

  } // end of namespace bklm

} // end of namespace Belle2

#endif // BKLMSECTOR_H
