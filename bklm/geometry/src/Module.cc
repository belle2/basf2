/*************************************************************************
*  BASF2 (Belle Analysis Framework 2)                                    *
*  Copyright(C) 2010 - Belle II Collaboration                            *
*                                                                        *
*  Author: The Belle II Collaboration                                    *
*  Contributors: Leo Piilonen                                            *
*                                                                        *
*  This software is provided "as is" without any warranty.               *
*************************************************************************/

#include "bklm/geometry/Module.h"

#include <framework/logging/Logger.h>
#include <framework/gearbox/Const.h>

#include <iostream>

using namespace std;

namespace Belle2 {

  namespace bklm {

    Module::Module() :
      m_HasRPCs(false),
      m_IsFlipped(false),
      m_PhiStripWidth(0.0),
      m_PhiStripMin(0),
      m_PhiStripMax(0),
      m_PhiPositionBase(0.0),
      m_ZStripWidth(0.0),
      m_ZStripMin(0),
      m_ZStripMax(0),
      m_ZPositionBase(0.0),
      m_PhiSensorSide(0),
      m_SignalSpeed(0.0),
      m_GlobalOrigin(CLHEP::Hep3Vector()),
      m_LocalReconstructionShift(CLHEP::Hep3Vector()),
      m_Rotation(CLHEP::HepRotation()),
      m_Alignment(HepGeom::Transform3D()),
      m_AlignmentRotation(CLHEP::HepRotation()),
      m_AlignmentTranslation(CLHEP::Hep3Vector()),
      m_AlignmentInverse(HepGeom::Transform3D()),
      m_AlignmentRotationInverse(CLHEP::HepRotation()),
      m_DisplacedGeo(HepGeom::Transform3D()),
      m_DisplacedGeoRotation(CLHEP::HepRotation()),
      m_DisplacedGeoTranslation(CLHEP::Hep3Vector()),
      m_DisplacedGeoInverse(HepGeom::Transform3D()),
      m_DisplacedGeoRotationInverse(CLHEP::HepRotation())
    {
      m_RotationInverse = m_Rotation.inverse();
      m_PhiScintPositions.clear();
      m_PhiScintLengths.clear();
      m_PhiScintOffsets.clear();
      m_ZScintPositions.clear();
      m_ZScintLengths.clear();
      m_ZScintOffsets.clear();
    }

    // constructor for RPC module
    Module::Module(double                    phiStripWidth,
                   int                       phiStripMin,
                   int                       phiStripMax,
                   double                    zStripWidth,
                   int                       zStripNumber,
                   const CLHEP::Hep3Vector&  globalOrigin,
                   const CLHEP::Hep3Vector&  localReconstructionShift,
                   const CLHEP::HepRotation& rotation) :
      m_HasRPCs(true),
      m_IsFlipped(false),
      m_PhiStripWidth(phiStripWidth),
      m_PhiStripMin(phiStripMin),
      m_PhiStripMax(phiStripMax),
      m_PhiPositionBase(0.5 * (phiStripMin + phiStripMax - 1) + 1.0), // start at #1
      m_ZStripWidth(zStripWidth),
      m_ZStripMin(1),
      m_ZStripMax(zStripNumber),
      m_ZPositionBase(1.0), // start at #1
      m_PhiSensorSide(1),
      m_SignalSpeed(0.5 * Const::speedOfLight),
      m_GlobalOrigin(globalOrigin),
      m_LocalReconstructionShift(localReconstructionShift),
      m_Rotation(rotation),
      m_Alignment(HepGeom::Transform3D()),
      m_AlignmentRotation(CLHEP::HepRotation()),
      m_AlignmentTranslation(CLHEP::Hep3Vector()),
      m_AlignmentInverse(HepGeom::Transform3D()),
      m_AlignmentRotationInverse(CLHEP::HepRotation()),
      m_DisplacedGeo(HepGeom::Transform3D()),
      m_DisplacedGeoRotation(CLHEP::HepRotation()),
      m_DisplacedGeoTranslation(CLHEP::Hep3Vector()),
      m_DisplacedGeoInverse(HepGeom::Transform3D()),
      m_DisplacedGeoRotationInverse(CLHEP::HepRotation())
    {
      m_RotationInverse = m_Rotation.inverse();
      m_PhiScintLengths.clear();
      m_PhiScintPositions.clear();
      m_PhiScintOffsets.clear();
      m_ZScintLengths.clear();
      m_ZScintPositions.clear();
      m_ZScintOffsets.clear();
    }

    // constructor for scint module
    Module::Module(double                    stripWidth,
                   int                       phiStripNumber,
                   int                       phiSensorSide,
                   int                       zStripNumber,
                   const CLHEP::Hep3Vector&  globalOrigin,
                   const CLHEP::Hep3Vector&  localReconstructionShift,
                   const CLHEP::HepRotation& rotation,
                   bool                      isFlipped) :
      m_HasRPCs(false),
      m_IsFlipped(isFlipped),
      m_PhiStripWidth(stripWidth),
      m_PhiStripMin(1),
      m_PhiStripMax(phiStripNumber),
      m_PhiPositionBase(0.5 * phiStripNumber + 1.0), // start at #1
      m_ZStripWidth(stripWidth),
      m_ZStripMin(1),
      m_ZStripMax(zStripNumber),
      m_ZPositionBase(1.0), // start at #1
      m_PhiSensorSide(phiSensorSide),
      m_SignalSpeed(0.5671 * Const::speedOfLight), // m_firstPhotonlightSpeed, from EKLM
      m_GlobalOrigin(globalOrigin),
      m_LocalReconstructionShift(localReconstructionShift),
      m_Rotation(rotation),
      m_Alignment(HepGeom::Transform3D()),
      m_AlignmentRotation(CLHEP::HepRotation()),
      m_AlignmentTranslation(CLHEP::Hep3Vector()),
      m_AlignmentInverse(HepGeom::Transform3D()),
      m_AlignmentRotationInverse(CLHEP::HepRotation()),
      m_DisplacedGeo(HepGeom::Transform3D()),
      m_DisplacedGeoRotation(CLHEP::HepRotation()),
      m_DisplacedGeoTranslation(CLHEP::Hep3Vector()),
      m_DisplacedGeoInverse(HepGeom::Transform3D()),
      m_DisplacedGeoRotationInverse(CLHEP::HepRotation())
    {
      if (isFlipped) m_Rotation.rotateZ(M_PI);
      m_RotationInverse = m_Rotation.inverse();
      m_PhiScintLengths.clear();
      m_PhiScintPositions.clear();
      m_PhiScintOffsets.clear();
      m_ZScintLengths.clear();
      m_ZScintPositions.clear();
      m_ZScintOffsets.clear();
    }

    // copy constructor
    Module::Module(const Module& m) :
      m_HasRPCs(m.m_HasRPCs),
      m_IsFlipped(m.m_IsFlipped),
      m_PhiStripWidth(m.m_PhiStripWidth),
      m_PhiStripMin(m.m_PhiStripMin),
      m_PhiStripMax(m.m_PhiStripMax),
      m_PhiPositionBase(m.m_PhiPositionBase),
      m_ZStripWidth(m.m_ZStripWidth),
      m_ZStripMin(m.m_ZStripMin),
      m_ZStripMax(m.m_ZStripMax),
      m_ZPositionBase(m.m_ZPositionBase),
      m_PhiSensorSide(m.m_PhiSensorSide),
      m_SignalSpeed(m.m_SignalSpeed),
      m_GlobalOrigin(m.m_GlobalOrigin),
      m_LocalReconstructionShift(m.m_LocalReconstructionShift),
      m_Rotation(m.m_Rotation),
      m_RotationInverse(m.m_RotationInverse),
      m_Alignment(m.m_Alignment),
      m_AlignmentRotation(m.m_AlignmentRotation),
      m_AlignmentTranslation(m.m_AlignmentTranslation),
      m_AlignmentInverse(m.m_AlignmentInverse),
      m_AlignmentRotationInverse(m.m_AlignmentRotationInverse),
      m_DisplacedGeo(m.m_DisplacedGeo),
      m_DisplacedGeoRotation(m.m_DisplacedGeoRotation),
      m_DisplacedGeoTranslation(m.m_DisplacedGeoTranslation),
      m_DisplacedGeoInverse(m.m_DisplacedGeoInverse),
      m_DisplacedGeoRotationInverse(m.m_DisplacedGeoRotationInverse),
      m_PhiScintLengths(m.m_PhiScintLengths),
      m_PhiScintPositions(m.m_PhiScintPositions),
      m_PhiScintOffsets(m.m_PhiScintOffsets),
      m_ZScintLengths(m.m_ZScintLengths),
      m_ZScintPositions(m.m_ZScintPositions),
      m_ZScintOffsets(m.m_ZScintOffsets)
    {
    }

    Module::~Module()
    {
    }

    void Module::addPhiScint(int scint, double length, double offset, double position)
    {
      while (m_PhiScintLengths.size() <= (unsigned int)scint) {
        m_PhiScintLengths.push_back(0.0);
        m_PhiScintOffsets.push_back(0.0);
        m_PhiScintPositions.push_back(0.0);
      }
      m_PhiScintLengths[scint] = length;
      m_PhiScintOffsets[scint] = offset;
      m_PhiScintPositions[scint] = position;
    }

    void Module::addZScint(int scint, double length, double offset, double position)
    {
      while (m_ZScintLengths.size() <= (unsigned int)scint) {
        m_ZScintLengths.push_back(0.0);
        m_ZScintOffsets.push_back(0.0);
        m_ZScintPositions.push_back(0.0);
      }
      m_ZScintLengths[scint] = length;
      m_ZScintOffsets[scint] = offset;
      m_ZScintPositions[scint] = position;
    }

    const CLHEP::Hep3Vector Module::getLocalPosition(double phiStripAve, double zStripAve) const
    {
      // "+0.5" assures that the local position is in the middle of the strip
      return CLHEP::Hep3Vector(0.0,
                               (phiStripAve - m_PhiPositionBase + 0.5) * m_PhiStripWidth,
                               (zStripAve - m_ZPositionBase + 0.5) * m_ZStripWidth);
    }

    const CLHEP::Hep3Vector Module::getPropagationTimes(const CLHEP::Hep3Vector& local) const
    {
      double dy = m_PhiPositionBase * m_PhiStripWidth - m_PhiSensorSide * local.y();
      double dz = m_ZStripMax * m_ZStripWidth - local.z();
      return CLHEP::Hep3Vector(0.0, dz / m_SignalSpeed, dy / m_SignalSpeed);
    }

    const CLHEP::Hep3Vector Module::localToGlobal(const CLHEP::Hep3Vector& v, bool m_reco) const
    {
      CLHEP::Hep3Vector vlocal(v.x(), v.y(), v.z());
      if (m_reco) {
        vlocal = m_AlignmentRotation * vlocal + m_AlignmentTranslation; // to nominal + displaced local
        vlocal = m_DisplacedGeoRotation * vlocal + m_DisplacedGeoTranslation; //to nominal local
        return m_Rotation * vlocal + m_GlobalOrigin; //to global
      } else {
        vlocal = m_DisplacedGeoRotation * vlocal + m_DisplacedGeoTranslation; // to nominal local
        return m_Rotation * vlocal + m_GlobalOrigin; //to global
      }

    }

    const CLHEP::Hep3Vector Module::globalToLocal(const CLHEP::Hep3Vector& v, bool reco) const
    {
      CLHEP::Hep3Vector vlocal = m_RotationInverse * (v - m_GlobalOrigin); //now in nominal local
      vlocal = m_DisplacedGeoRotationInverse * (vlocal - m_DisplacedGeoTranslation); //now in displaced local
      if (reco) {
        vlocal = m_AlignmentRotationInverse * (vlocal - m_AlignmentTranslation); //now in real local
      }
      return vlocal;
    }

    void Module::setAlignment(const HepGeom::Transform3D& moduleAlignment)
    {
      m_Alignment = moduleAlignment;
      m_AlignmentRotation = getRotationFromTransform3D(moduleAlignment);
      m_AlignmentTranslation = getTranslationFromTransform3D(moduleAlignment);
      m_AlignmentInverse = moduleAlignment.inverse();
      m_AlignmentRotationInverse = getRotationFromTransform3D(m_AlignmentInverse);
    }

    void Module::setDisplacedGeo(const HepGeom::Transform3D& moduleDisplacedGeo)
    {
      m_DisplacedGeo = moduleDisplacedGeo;
      m_DisplacedGeoRotation = getRotationFromTransform3D(moduleDisplacedGeo);
      m_DisplacedGeoTranslation = getTranslationFromTransform3D(moduleDisplacedGeo);
      m_DisplacedGeoInverse = moduleDisplacedGeo.inverse();
      m_DisplacedGeoRotationInverse = getRotationFromTransform3D(m_DisplacedGeoInverse);
    }

    const CLHEP::Hep3Vector Module::getTranslationFromTransform3D(const HepGeom::Transform3D& trans) const
    {
      HepGeom::Scale3D scale;
      HepGeom::Rotate3D rotation;
      HepGeom::Translate3D translation;
      trans.getDecomposition(scale, rotation, translation);
      CLHEP::Hep3Vector t(translation.dx(), translation.dy(), translation.dz());
      return t;
    }

    const CLHEP::HepRotation Module::getRotationFromTransform3D(const HepGeom::Transform3D& trans) const
    {
      HepGeom::Scale3D scale;
      HepGeom::Rotate3D rotation;
      HepGeom::Translate3D translation;
      trans.getDecomposition(scale, rotation, translation);
      CLHEP::HepRep3x3 rRep(rotation.xx(), rotation.xy(), rotation.xz(),
                            rotation.yx(), rotation.yy(), rotation.yz(),
                            rotation.zx(), rotation.zy(), rotation.zz()
                           );
      CLHEP::HepRotation r;
      r.set(rRep);
      return r;
    }

  } // end of namespace bklm

} // end of namespace Belle2
