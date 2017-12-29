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
#include <CLHEP/Geometry/Transform3D.h>
#include "vector"

namespace Belle2 {

  namespace bklm {

    /*! Define the geometry of a BKLM module
     *  Each sector [octant] contains Modules.  Sectors are numbered 1..8,
     *  moving counterclockwise around the global z axis in the global x-y plane.
     *  The local geometry of a sector and its modules is defined by the orientation
     *  of sector 1 (centred on the global +x axis).
     *
     *       SECTOR 1 GLOBAL COORDINATES        SECTOR s LOCAL COORDINATES
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
     *   The sector's local origin is shifted along the global z axis so that it
     *   is at the boundary between the forward and backward barrel KLM.
     *
     *   The sector's local origin is shifted along the local +x axis so that it
     *   coincides with the middle of the layer-1 gap (=slot between iron layers);
     *   outer layers are at larger values of sector-local x. The module's local
     *   origin is shifted along the local x axis so that it is in the middle of
     *   the module's sensitive volume.  The module's local origin is also shifted
     *   slightly along the local y and z axes so that it is in the middle of the
     *   sensitive volume along the local y axis and at the edge nearest the local
     *   origin of the sensitive volume along the local z axis.
     *
     *   For the backward sectors, the module is rotated by 180 degrees about the +x
     *   axis so that the orientation of the local z axis is flipped from the global
     *   orientation so that all modules extend along the local +z axis from the
     *   local z=0 (which is nearest the forward-backward boundary).
     *
     *   Strip numbering for RPCs:
     *     z-measuring strips: #1 is nearest the I.P., sensors are at the other end
     *     phi-measuring strips: #1 is on the local -y end, sensors are at the local +y end
     *
     *   Strip numbering for scintillatorss:
     *     z-measuring strips: #1 is nearest the I.P., sensors are at the other end
     *     phi-measuring strips:
     *        if phiSensorSide > 0: #1 is on the local -y end, sensors are at the local +y end
     *        if phiSensorSide < 0: #1 is on the local +y end, sensors are at the local -y end
     *
     */
    class Module {

    public:

      //! Empty constructor does nothing
      Module(void);

      //! Constructor with explicit values (for RPC module)
      Module(double                    phiStripWidth,
             int                       phiStripMin,
             int                       phiStripMax,
             double                    zStripWidth,
             int                       zStripNumber,
             const CLHEP::Hep3Vector&  globalOrigin,
             const CLHEP::Hep3Vector&  localReconstructionShift,
             const CLHEP::HepRotation& rotation);

      //! Constructor with explicit values (for scint module)
      Module(double                    stripWidth,
             int                       phiStripNumber,
             int                       phiSensorSide,
             int                       zStripNumber,
             const CLHEP::Hep3Vector&  globalOrigin,
             const CLHEP::Hep3Vector&  localReconstructionShift,
             const CLHEP::HepRotation& rotation,
             bool                      isFlipped);

      //! Copy constructor
      Module(const Module& m);

      //! Destructor
      ~Module();

      //! Determine if this module is flipped by 180 degrees about z axis within its air gap
      bool isFlipped() const { return m_IsFlipped; }

      //! Determine if this module contains RPCs (true) or scintillators (false)
      bool hasRPCs() const { return m_HasRPCs; }

      //! Get module's phi-strip minimum index
      int getPhiStripMin() const { return m_PhiStripMin; }

      //! Get module's phi-strip maximum index
      int getPhiStripMax() const { return m_PhiStripMax; }

      //! Get phi-strip width
      double getPhiStripWidth() const { return m_PhiStripWidth; }

      //! Get module's z-strip minimum index
      int getZStripMin() const { return m_ZStripMin; }

      //! Get module's z-strip maximum index
      int getZStripMax() const { return m_ZStripMax; }

      //! Get z-strip width
      double getZStripWidth() const { return m_ZStripWidth; }

      //! Add one phi-measuring scintillator strip to the module
      void addPhiScint(int scint, double length, double offset, double position);

      //! Add one z-measuring scintillator strip to the module
      void addZScint(int scint, double length, double offset, double position);

      //! Get the half-length (within the scintillator envelope) of a given phi-measuring scintillator
      double getPhiScintHalfLength(int scint) const { return 0.5 * m_PhiScintLengths[scint]; }

      //! Get the longitudinal offset (within the scintillator envelope) of a given phi-measuring scintillator
      double getPhiScintOffset(int scint) const { return m_PhiScintOffsets[scint]; }

      //! Get the transverse position (within the scintillator envelope) of a given phi-measuring scintillator
      double getPhiScintPosition(int scint) const { return m_PhiScintPositions[scint]; }

      //! Get the half-length (within the scintillator envelope) of a given z-measuring scintillator
      double getZScintHalfLength(int scint) const { return 0.5 * m_ZScintLengths[scint]; }

      //! Get the longitudinal offset (within the scintillator envelope) of a given z-measuring scintillator
      double getZScintOffset(int scint) const { return m_ZScintOffsets[scint]; }

      //! Get the transverse position (within the scintillator envelope) of a given z-measuring scintillator
      double getZScintPosition(int scint) const { return m_ZScintPositions[scint]; }

      //! Convert 2D strip position (0..nStrips along each axis) to local coordinates
      const CLHEP::Hep3Vector getLocalPosition(double phiStripAve, double zStripAve) const;

      //! Convert local coordinates to signal-propagation time (ns)
      const CLHEP::Hep3Vector getPropagationTimes(const CLHEP::Hep3Vector&) const;

      //! Return phi strip (including fractional part) corresponding to local phi coordinate
      double getPhiStrip(const CLHEP::Hep3Vector& p) const { return p.y() / m_PhiStripWidth + m_PhiPositionBase; }

      //! Return z strip (including fractional part) corresponding to local z coordinate
      double getZStrip(const CLHEP::Hep3Vector& p) const { return p.z() / m_ZStripWidth + m_ZPositionBase; }

      //! Transform space-point within this module from local to global coordinates
      //! @param reco set transformation for reconstruction including alignment correctoin (true) or nominal without alignment correction (false)
      const CLHEP::Hep3Vector localToGlobal(const CLHEP::Hep3Vector& v, bool reco = false) const;

      //! Rotate a vector from global to local system
      const CLHEP::Hep3Vector RotateToLocal(const CLHEP::Hep3Vector& v) const { return m_RotationInverse * v;  }

      //! Rotate a vector from local to global system
      const CLHEP::Hep3Vector RotateToGlobal(const CLHEP::Hep3Vector& v) const { return m_Rotation * v;  }

      //! Transform space-point within this module from global to local coordinates
      //! @param reco set transformation for reconstruction including alignment correctoin (true) or nominal without alignment correction (false)
      const CLHEP::Hep3Vector globalToLocal(const CLHEP::Hep3Vector& v,  bool reco = false) const;

      //! Return the local-coordinate real-vs-ideal translation of this module's sensitive volume; nominally (0,0,0)
      const CLHEP::Hep3Vector getLocalReconstructionShift() const { return m_LocalReconstructionShift; }

      //! Return the position (in global coordinates) of this module's sensitive-volume origin
      const CLHEP::Hep3Vector getGlobalOrigin() const {return m_GlobalOrigin;}

      //! Switch on/off the alignment correction
      // void setAlignmentCorrectionSwitch(bool ifreo ) { m_AlignmentCorrectionSwitch = ifreo;}

      //! Set the alignment Transformation
      void setAlignment(const HepGeom::Transform3D& moduleAlignment);

      //! Set the displaced geometry Transformation
      void setDisplacedGeo(const HepGeom::Transform3D& moduleDisplacedGeo);

      //! Get the alignment Transformation
      const HepGeom::Transform3D getAlignment() const { return m_Alignment; }

      //! Get the displaced geometry Transformation
      const HepGeom::Transform3D getDisplacedGeo() const { return m_DisplacedGeo; }

      //! Get the rotation from Transform3D
      const CLHEP::HepRotation getRotationFromTransform3D(const HepGeom::Transform3D& trans) const;

      //! Get the translation from Transform3D
      const CLHEP::Hep3Vector getTranslationFromTransform3D(const HepGeom::Transform3D& trans) const;

    private:

      //! flag to indicate whether this module contains RPCs (true) or scintillators (false)
      bool m_HasRPCs;

      //! flag to indicate if this module is flipped by 180 degrees about z axis within its air gap
      bool m_IsFlipped;

      //! to store the width (in cm) of each phi strip this module
      double m_PhiStripWidth;

      //! to store the minimum phi strip number in this module
      int m_PhiStripMin;

      //! to store the maximum phi strip number in this module
      int m_PhiStripMax;

      //! to store the base position (in strip-# units) along phi coordinate of the edge of first phi-measuring strip
      double m_PhiPositionBase;

      //! to store the width (in cm) of each z strip in this module
      double m_ZStripWidth;

      //! to store the minimum z strip number in this module
      int m_ZStripMin;

      //! to store the maximum z strip number in this module
      int m_ZStripMax;

      //! to store the base position (in strip-# units) along z coordinate of the edge of first z-measuring strip
      double m_ZPositionBase;

      //! to store the sensor side for phi scintillators
      int m_PhiSensorSide;

      //! to store the signal-propagation speed (cm/ns) along the strip
      double m_SignalSpeed;

      //! to store the position (in global coordinates) of this module's sensitive-volume origin
      CLHEP::Hep3Vector m_GlobalOrigin;

      //! to store the local-coordinate real-vs-ideal translation of this module's sensitive volume
      CLHEP::Hep3Vector m_LocalReconstructionShift;

      //! to store the rotation matrix (in global coordinates) of this module's sector
      CLHEP::HepRotation m_Rotation;

      //! to store the inverse of the rotation matrix (in global coordinates) of this module's sector
      CLHEP::HepRotation m_RotationInverse;

      //! alignment transformation (rotation + shift)
      HepGeom::Transform3D m_Alignment;

      //! rotation of the alignment transformation
      CLHEP::HepRotation m_AlignmentRotation;

      //! translation of the alignment transformation
      CLHEP::Hep3Vector m_AlignmentTranslation;

      //! inverse of alignment transformation
      HepGeom::Transform3D m_AlignmentInverse;

      //! inverse of the rotation of the alignment transformation
      CLHEP::HepRotation m_AlignmentRotationInverse;

      //! displaced geometry w.r.t the nominal geometry, as transformation (rotation + shift)
      HepGeom::Transform3D m_DisplacedGeo;

      //! rotation of the displaced geometry transformation
      CLHEP::HepRotation m_DisplacedGeoRotation;

      //! translation of the displaced geometry transformation
      CLHEP::Hep3Vector m_DisplacedGeoTranslation;

      //! inverse of displaced geometry transformation
      HepGeom::Transform3D m_DisplacedGeoInverse;

      //! inverse of the rotation of displaced geometry transformation
      CLHEP::HepRotation m_DisplacedGeoRotationInverse;

      //! to store the length of each phi-measuring scintillator
      std::vector<double> m_PhiScintLengths;

      //! to store the transverse position (within scintillator envelope) of each phi-measuring scintillator
      std::vector<double> m_PhiScintPositions;

      //! to store the longitudinal offset (within scintillator envelope) of each phi-measuring scintillator
      std::vector<double> m_PhiScintOffsets;

      //! to store the length of each z-measuring scintillator
      std::vector<double> m_ZScintLengths;

      //! to store the transverse position (within scintillator envelope) of each z-measuring scintillator
      std::vector<double> m_ZScintPositions;

      //! to store the longitudinal offset (within scintillator envelope) of each z-measuring scintillator
      std::vector<double> m_ZScintOffsets;

    };

  } // end of namespace bklm

} // end of namespace Belle2

#endif // BKLMMODULE_H
