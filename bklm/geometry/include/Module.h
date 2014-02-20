/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Leo Piilonen                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef BKLMMODULE_H
#define BKLMMODULE_H

#include "CLHEP/Vector/ThreeVector.h"
#include "CLHEP/Vector/Rotation.h"
#include "CLHEP/Matrix/Matrix.h"

#include "bklm/geometry/Rect.h"

namespace Belle2 {

  namespace bklm {

    /*! Define the geometry of a BKLM module
     *  Each sector [octant] contains Modules.  The local geometry of a sector
     *  and its modules is oriented to the position of sector 0 by a rotation
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
     *   For the backward sectors, the module is rotated by 180 degrees about the +x
     *   axis so that the orientation of the local z axis is flipped from the global
     *   orientation so that all modules extend along the local +z axis from the
     *   local z=0 (which is nearest the IP).
     */
    class Module {

    public:

      //! Empty constructor does nothing
      Module(void);

      //! Constructor with explicit values (for RPC module)
      Module(bool              isForward,
             int               sector,
             int               module,
             CLHEP::Hep3Vector shift,
             double            localX,
             double            phiStripWidth,
             double            phiStripLength,
             int               phiStripNumber,
             int               phiStripMax,
             int               phiOffsetSign,
             double            zStripWidth,
             double            zStripLength,
             int               zStripNumber,
             int               zStripMin,
             int               zStripMax,
             CLHEP::Hep3Vector translation,
             CLHEP::HepRotation rotation);

      //! Constructor with explicit values (for scint module)
      Module(bool              isForward,
             int               sector,
             int               module,
             CLHEP::Hep3Vector shift,
             double            localX,
             double            stripWidth,
             double            phiStripLength,
             int               phiStripNumber,
             int               phiOffsetSign,
             double            zStripLength,
             int               zStripNumber,
             int               zOffsetSign,
             CLHEP::Hep3Vector translation,
             CLHEP::HepRotation rotation);

      //! Copy constructor
      Module(const Module& m);

      //! Destructor
      ~Module();

      //! Get module's detector type (true for RPCs, false for scints)
      bool hasRPCs() const { return m_HasRPCs; }

      //! Get module's end (forward or backward)
      bool isForward() const { return m_IsForward; }

      //! Get module's sector number
      int getSector() const { return m_Sector; }

      //! Get module's layer number
      int getLayer() const { return m_Layer; }

      //! Get module's phi-strip count
      int getPhiStripNumber() const { return m_PhiStripNumber; }

      //! Get module's phi-strip minimum index
      int getPhiStripMin() const { return m_PhiStripMin; }

      //! Get module's phi-strip maximum index
      int getPhiStripMax() const { return m_PhiStripMax; }

      //! Get module's offset direction along phi for the strip/scint envelope
      int getPhiOffsetSign() const { return m_PhiOffsetSign; }

      //! Get module's z-strip count
      int getZStripNumber() const { return m_ZStripNumber; }

      //! Get module's z-strip minimum index
      int getZStripMin() const { return m_ZStripMin; }

      //! Get module's z-strip maximum index
      int getZStripMax() const { return m_ZStripMax; }

      //! Get module's offset direction along z for the strip/scint envelope
      int getZOffsetSign() const { return m_ZOffsetSign; }

      //! Get module's shift, nominally (0,0,0) (in local coordinates)
      const CLHEP::Hep3Vector getShift() const { return m_Shift; }

      //! Get module's altitude (in local coordinates)
      double getLocalX() const { return m_LocalX; }

      //! Add one phi-measuring scintillator strip to the module
      void addPhiScint(int scint, double length, double offset, double position);

      //! Add one z-measuring scintillator strip to the module
      void addZScint(int scint, double length, double offset, double position);

      //! Get the number of strips in this module
      int getNStrips(bool isPhiReadout) const {
        return (isPhiReadout ? m_PhiStripNumber : m_ZStripNumber);
      }

      //! Get the half-length (within the scintillator envelope) of a given phi-measuring scintillator
      double getPhiScintHalfLength(int scint) const { return 0.5 * m_PhiScintLengths[scint]; }

      //! Get the length-offset (within the scintillator envelope) of a given phi-measuring scintillator
      double getPhiScintOffset(int scint) const { return m_PhiScintOffsets[scint]; }

      //! Get the position (within the scintillator envelope) of a given phi-measuring scintillator
      double getPhiScintPosition(int scint) const { return m_PhiScintPositions[scint]; }

      //! Get the half-length (within the scintillator envelope) of a given z-measuring scintillator
      double getZScintHalfLength(int scint) const { return 0.5 * m_ZScintLengths[scint]; }

      //! Get the length-offset (within the scintillator envelope) of a given z-measuring scintillator
      double getZScintOffset(int scint) const { return m_ZScintOffsets[scint]; }

      //! Get the position (within the scintillator envelope) of a given z-measuring scintillator
      double getZScintPosition(int scint) const { return m_ZScintPositions[scint]; }

      //! Convert 1D strip position (0..nStrips) to local coordinate
      double getLocalCoordinate(double stripAve, bool isPhiReadout) const;

      //! Convert 2D strip position (0..nStrips along each axis) to local coordinates
      const CLHEP::Hep3Vector getLocalPosition(double phiStripAve, double zStripAve) const;

      //! Get 2D position covariance matrix in local coordinates, given strip multiplicities
      //! unused function      const CLHEP::HepMatrix getLocalError(int phiStripMult, int zStripMult) const;

      //! Get bounding rectangle of this strip's surface in local coordinates
      //! unused function      const Rect getStripRectLocal(double stripAve, bool isPhiReadout) const;

      //! Get bounding rectangle of this module's surface in local coordinates
      //! unused function      const Rect getModuleRectLocal(void) const;

      //! Determine if space-point (in local coordinates) is in active area of RPC module, and fills phiStrip and zStrip
      bool isInActiveArea(const CLHEP::Hep3Vector& p, int& phiStrip, int& zStrip) const;

      //! Determine if space-point (in local coordinates) is in active area of scint module, and fills either phiStrip or zStrip
      bool isInActiveArea(const CLHEP::Hep3Vector& p, int& phiStrip, int& zStrip, int innerOuter) const;

      //! Fills phiStrip and zStrip with relative position (-0.5..+0.5) of space-point (in local coordinates) along each strip's width
      void getStripDivisions(const CLHEP::Hep3Vector& p, double& phiStrip, double& zStrip) const;

      //! Transform space-point within this module from local to global coordinates
      const CLHEP::Hep3Vector localToGlobal(const CLHEP::Hep3Vector& v) const;

      //! Transform space-point within this module from global to local coordinates
      const CLHEP::Hep3Vector globalToLocal(const CLHEP::Hep3Vector& v) const;

      //! Transform space-point within this module from global to local coordinates
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

      //! Get the unit vector normal to this module, in global coordinates
      const CLHEP::Hep3Vector getNormal() const;

      //! Print the module's definition
      void printTree() const;

    private:

      //! to store the detector type (true=RPCs, false=scints) of this module
      bool m_HasRPCs;

      //! to store the axial end (true=forward or false=backward) of this module
      bool m_IsForward;

      //! to store the sector number of this module
      int m_Sector;

      //! to store the layer number of this module
      int m_Layer;

      //! to store the shift (in global coordinates) of this module; nominally (0,0,0)
      CLHEP::Hep3Vector m_Shift;

      //! to store the local altitude of this module
      double m_LocalX;

      //! to store the tolerance in variation from local height of this module
      double m_ToleranceX;

      //! to store the width (in cm) of each phi strip this module
      double m_PhiStripWidth;

      //! to store the length (in cm) of each phi strip in this module
      double m_PhiStripLength;

      //! to store the number of phi strips in this module
      int m_PhiStripNumber;

      //! to store the minimum phi strip number in this module
      int m_PhiStripMin;

      //! to store the maximum phi strip number in this module
      int m_PhiStripMax;

      //! to store the offset direction along phi of the strip/scint envelope in this module
      int m_PhiOffsetSign;

      //! to store the width (in cm) of each z strip in this module
      double m_ZStripWidth;

      //! to store the length (in cm) of each z strip in this module
      double m_ZStripLength;

      //! to store the number of z strips in this module
      int m_ZStripNumber;

      //! to store the minimum z strip number in this module
      int m_ZStripMin;

      //! to store the maximum z strip number in this module
      int m_ZStripMax;

      //! to store the offset direction along z of the strip/scint envelope in this module
      int m_ZOffsetSign;

      //! to store the length of each phi-measuring scintillator
      std::vector<double> m_PhiScintLengths;

      //! to store the position (within scintillator envelope) of each phi-measuring scintillator
      std::vector<double> m_PhiScintPositions;

      //! to store the length-offset (within scintillator envelope) of each phi-measuring scintillator
      std::vector<double> m_PhiScintOffsets;

      //! to store the length of each z-measuring scintillator
      std::vector<double> m_ZScintLengths;

      //! to store the position (within scintillator envelope) of each z-measuring scintillator
      std::vector<double> m_ZScintPositions;

      //! to store the length-offset (within scintillator envelope) of each z-measuring scintillator
      std::vector<double> m_ZScintOffsets;

      //! to store the position (in global coordinates) of this module's sector
      CLHEP::Hep3Vector m_Translation;

      //! to store the rotation matrix (in global coordinates) of this module's sector
      CLHEP::HepRotation m_Rotation;

      //! to store the inverse of the rotation matrix (in global coordinates) of this module's sector
      CLHEP::HepRotation m_RotationInverse;

      //! to store a copy of the rotation matrix in alternate form
      CLHEP::HepMatrix m_RotationMatrix;

      //! to store a copy of the inverse of the rotation matrix in alternate form
      CLHEP::HepMatrix m_RotationInverseMatrix;

      //! to store the unit normal vector (in global coordinates) of this module
      CLHEP::Hep3Vector m_Normal;

    };

  } // end of namespace bklm

} // end of namespace Belle2

#endif // BKLMMODULE_H
