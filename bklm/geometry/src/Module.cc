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
using namespace CLHEP;

namespace Belle2 {

  namespace bklm {

    Module::Module() :
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
      m_GlobalOrigin(Hep3Vector()),
      m_LocalReconstructionShift(Hep3Vector()),
      m_Rotation(HepRotation())
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
    Module::Module(double     phiStripWidth,
                   int        phiStripMin,
                   int        phiStripMax,
                   double     zStripWidth,
                   int        zStripNumber,
                   Hep3Vector globalOrigin,
                   Hep3Vector localReconstructionShift,
                   HepRotation rotation) :
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
      m_Rotation(rotation)
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
    Module::Module(double     stripWidth,
                   int        phiStripNumber,
                   int        phiSensorSide,
                   int        zStripNumber,
                   Hep3Vector globalOrigin,
                   Hep3Vector localReconstructionShift,
                   HepRotation rotation) :
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
      m_Rotation(rotation)
    {
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
      m_PhiStripWidth(m.m_PhiStripWidth),
      m_PhiStripMin(m.m_PhiStripMin),
      m_PhiStripMax(m.m_PhiStripMax),
      m_PhiPositionBase(m.m_PhiPositionBase),
      m_ZStripWidth(m.m_ZStripWidth),
      m_ZStripMin(m.m_ZStripMin),
      m_ZStripMax(m.m_ZStripMax),
      m_ZPositionBase(m.m_ZPositionBase),
      m_GlobalOrigin(m.m_GlobalOrigin),
      m_LocalReconstructionShift(m.m_LocalReconstructionShift),
      m_Rotation(m.m_Rotation),
      m_RotationInverse(m.m_RotationInverse),
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

    const Hep3Vector Module::getLocalPosition(double phiStripAve, double zStripAve) const
    {
      // "+0.5" assures that the local position is in the middle of the strip
      return Hep3Vector(0.0,
                        (phiStripAve - m_PhiPositionBase + 0.5) * m_PhiStripWidth,
                        (zStripAve - m_ZPositionBase + 0.5) * m_ZStripWidth);
    }

    const Hep3Vector Module::getPropagationTimes(const Hep3Vector& local) const
    {
      double dy = m_PhiPositionBase * m_PhiStripWidth - m_PhiSensorSide * local.y();
      double dz = m_ZStripMax * m_ZStripWidth - local.z();
      return Hep3Vector(0.0, dz / m_SignalSpeed, dy / m_SignalSpeed);
    }

  } // end of namespace bklm

} // end of namespace Belle2
