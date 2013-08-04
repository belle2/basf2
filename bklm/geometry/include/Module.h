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
#include "CLHEP/Matrix/Matrix.h"

#include "bklm/geometry/Rect.h"

namespace Belle2 {

  namespace bklm {

    class Sector;

    //! Define the geometry of a BKLM module
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
             Sector*           sectorPtr,
             double            phiStripWidth,
             double            phiStripLength,
             int               phiStripNumber,
             int               phiStripMin,
             int               phiStripMax,
             int               phiOffset,
             double            zStripWidth,
             double            zStripLength,
             int               zStripNumber,
             int               zStripMin,
             int               zStripMax,
             int               zOffset);

      //! Constructor with explicit values (for scint module)
      Module(bool              isForward,
             int               sector,
             int               module,
             CLHEP::Hep3Vector shift,
             double            localX,
             Sector*           sectorPtr,
             double            phiStripWidth,
             double            phiStripLength,
             int               phiStripNumber,
             int               phiOffset,
             double            zStripWidth,
             double            zStripLength,
             int               zStripNumber,
             int               zOffset);

      //! Copy constructor
      Module(const Module& m);

      //! Destructor
      ~Module();

      //! Comparison operator (ordering)
      bool operator<(const Module& m) const;

      //! Comparison operator (equality)
      bool operator==(const Module& m) const { return isSameModule(m); }

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
      int getPhiOffset() const { return m_PhiOffset; }

      //! Get module's z-strip count
      int getZStripNumber() const { return m_ZStripNumber; }

      //! Get module's z-strip minimum index
      int getZStripMin() const { return m_ZStripMin; }

      //! Get module's z-strip maximum index
      int getZStripMax() const { return m_ZStripMax; }

      //! Get module's offset direction along z for the strip/scint envelope
      int getZOffset() const { return m_ZOffset; }

      //! Get module's shift, nominally (0,0,0) (in local coordinates)
      const CLHEP::Hep3Vector getShift() const { return m_Shift; }

      //! Get module's altitude (in local coordinates)
      double getLocalX() const { return m_LocalX; }

      //! Get pointer to the module's containing sector
      const Sector* getSectorPtr(void) const { return m_SectorPtr; }

      //! Determine if two modules are identical
      bool isSameModule(bool isForward, int sector, int layer) const;

      //! Determine if two modules are identical
      bool isSameModule(const Module& m) const;

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
      const CLHEP::HepMatrix getLocalError(int phiStripMult, int zStripMult) const;

      //! Get bounding rectangle of this strip's surface in local coordinates
      const Rect getStripRectLocal(double stripAve, bool isPhiReadout) const;

      //! Get bounding rectangle of this module's surface in local coordinates
      const Rect getModuleRectLocal(void) const;

      //! Determine if space-point (in local coordinates) is in active area of RPC module, and fills phiStrip and zStrip
      bool isInActiveArea(const CLHEP::Hep3Vector& p, int& phiStrip, int& zStrip) const;

      //! Determine if space-point (in local coordinates) is in active area of scint module, and fills either phiStrip or zStrip
      bool isInActiveArea(const CLHEP::Hep3Vector& p, int& phiStrip, int& zStrip, int innerOuter) const;

      //! Fills phiStrip and zStrip with relative position (-0.5..+0.5) of space-point (in local coordinates) along each strip's width
      void getStripDivisions(const CLHEP::Hep3Vector& p, double& phiStrip, double& zStrip) const;

      //! Print module definition
      void printTree(void) const;

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

      //! to store the pointer to this module's containing sector class
      Sector* m_SectorPtr;

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
      int m_PhiOffset;

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
      int m_ZOffset;

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

    };

  } // end of namespace bklm

} // end of namespace Belle2

#endif // BKLMMODULE_H
