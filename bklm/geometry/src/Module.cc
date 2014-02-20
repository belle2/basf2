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

#include <iostream>

using namespace std;
using namespace CLHEP;

namespace Belle2 {

  namespace bklm {

    Module::Module() :
      m_HasRPCs(false),
      m_IsForward(false),
      m_Sector(0),
      m_Layer(0),
      m_Shift(Hep3Vector()),
      m_LocalX(0.0),
      m_ToleranceX(0.0),
      m_PhiStripWidth(0.0),
      m_PhiStripLength(0.0),
      m_PhiStripNumber(0),
      m_PhiStripMin(0),
      m_PhiStripMax(0),
      m_PhiOffsetSign(0),
      m_ZStripWidth(0.0),
      m_ZStripLength(0.0),
      m_ZStripNumber(0),
      m_ZStripMin(0),
      m_ZStripMax(0),
      m_ZOffsetSign(0),
      m_Translation(Hep3Vector()),
      m_Rotation(HepRotation()),
      m_RotationInverse(HepRotation()),
      m_Normal(Hep3Vector())
    {
      m_PhiScintPositions.clear();
      m_PhiScintLengths.clear();
      m_ZScintPositions.clear();
      m_ZScintLengths.clear();
      m_RotationMatrix = m_Rotation;
      m_RotationInverseMatrix = m_RotationInverse;
    }

    // constructor for RPC module
    Module::Module(bool       isForward,
                   int        sector,
                   int        layer,
                   Hep3Vector shift,
                   double     localX,
                   double     phiStripWidth,
                   double     phiStripLength,
                   int        phiStripNumber,
                   int        phiStripMin,
                   int        phiStripMax,
                   double     zStripWidth,
                   double     zStripLength,
                   int        zStripNumber,
                   int        zStripMin,
                   int        zStripMax,
                   Hep3Vector translation,
                   HepRotation rotation) :
      m_HasRPCs(true),
      m_IsForward(isForward),
      m_Sector(sector),
      m_Layer(layer),
      m_Shift(shift),
      m_LocalX(localX),
      m_ToleranceX(2.0),   // cm
      m_PhiStripWidth(phiStripWidth),
      m_PhiStripLength(phiStripLength),
      m_PhiStripNumber(phiStripNumber),
      m_PhiStripMin(phiStripMin),
      m_PhiStripMax(phiStripMax),
      m_PhiOffsetSign(0),
      m_ZStripWidth(zStripWidth),
      m_ZStripLength(zStripLength),
      m_ZStripNumber(zStripNumber),
      m_ZStripMin(zStripMin),
      m_ZStripMax(zStripMax),
      m_ZOffsetSign(0),
      m_Translation(translation),
      m_Rotation(rotation)
    {
      m_PhiScintLengths.clear();
      m_PhiScintPositions.clear();
      m_PhiScintOffsets.clear();
      m_ZScintLengths.clear();
      m_ZScintPositions.clear();
      m_ZScintOffsets.clear();
      m_RotationInverse = m_Rotation.inverse();
      m_RotationMatrix = m_Rotation;
      m_RotationInverseMatrix = m_RotationInverse;
      m_Normal = m_Rotation(Hep3Vector(1.0, 0.0, 0.0));
    }

    // constructor for scint module
    Module::Module(bool       isForward,
                   int        sector,
                   int        layer,
                   Hep3Vector shift,
                   double     localX,
                   double     stripWidth,
                   double     phiStripLength,
                   int        phiStripNumber,
                   int        phiOffsetSign,
                   double     zStripLength,
                   int        zStripNumber,
                   int        zOffsetSign,
                   Hep3Vector translation,
                   HepRotation rotation) :
      m_HasRPCs(false),
      m_IsForward(isForward),
      m_Sector(sector),
      m_Layer(layer),
      m_Shift(shift),
      m_LocalX(localX),
      m_ToleranceX(2.0),   // cm
      m_PhiStripWidth(stripWidth),
      m_PhiStripLength(phiStripLength),
      m_PhiStripNumber(phiStripNumber),
      m_PhiStripMin(0),
      m_PhiStripMax(0),
      m_PhiOffsetSign(phiOffsetSign),
      m_ZStripWidth(stripWidth),
      m_ZStripLength(zStripLength),
      m_ZStripNumber(zStripNumber),
      m_ZStripMin(0),
      m_ZStripMax(0),
      m_ZOffsetSign(zOffsetSign),
      m_Translation(translation),
      m_Rotation(rotation)
    {
      m_PhiScintLengths.clear();
      m_PhiScintPositions.clear();
      m_PhiScintOffsets.clear();
      m_ZScintLengths.clear();
      m_ZScintPositions.clear();
      m_ZScintOffsets.clear();
      m_RotationInverse = m_Rotation.inverse();
      m_RotationMatrix = m_Rotation;
      m_RotationInverseMatrix = m_RotationInverse;
      m_Normal = m_Rotation(Hep3Vector(1.0, 0.0, 0.0));
    }

    // copy constructor
    Module::Module(const Module& m) :
      m_HasRPCs(m.m_HasRPCs),
      m_IsForward(m.m_IsForward),
      m_Sector(m.m_Sector),
      m_Layer(m.m_Layer),
      m_Shift(m.m_Shift),
      m_LocalX(m.m_LocalX),
      m_ToleranceX(m.m_ToleranceX),
      m_PhiStripWidth(m.m_PhiStripWidth),
      m_PhiStripLength(m.m_PhiStripLength),
      m_PhiStripNumber(m.m_PhiStripNumber),
      m_PhiStripMin(m.m_PhiStripMin),
      m_PhiStripMax(m.m_PhiStripMax),
      m_PhiOffsetSign(m.m_PhiOffsetSign),
      m_ZStripWidth(m.m_ZStripWidth),
      m_ZStripLength(m.m_ZStripLength),
      m_ZStripNumber(m.m_ZStripNumber),
      m_ZStripMin(m.m_ZStripMin),
      m_ZStripMax(m.m_ZStripMax),
      m_ZOffsetSign(m.m_ZOffsetSign),
      m_PhiScintLengths(m.m_PhiScintLengths),
      m_PhiScintPositions(m.m_PhiScintPositions),
      m_PhiScintOffsets(m.m_PhiScintOffsets),
      m_ZScintLengths(m.m_ZScintLengths),
      m_ZScintPositions(m.m_ZScintPositions),
      m_ZScintOffsets(m.m_ZScintOffsets),
      m_Translation(m.m_Translation),
      m_Rotation(m.m_Rotation),
      m_RotationInverse(m.m_RotationInverse),
      m_Normal(m.m_Normal)
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

    double Module::getLocalCoordinate(double stripAve, bool isPhiReadout) const
    {
      if (isPhiReadout) {
        return (stripAve + 0.5 - m_PhiStripNumber * 0.5) * m_PhiStripWidth;
      }
      return (stripAve + 0.5) * m_ZStripWidth;

    }

    const Hep3Vector Module::getLocalPosition(double phiStripAve, double zStripAve) const
    {
      return Hep3Vector(m_LocalX,
                        getLocalCoordinate(phiStripAve, true),
                        getLocalCoordinate(zStripAve, false)) + m_Shift;
    }

    /* unused function
        const HepMatrix Module::getLocalError(int phiStripMultiplicity, int zStripMultiplicity) const
        {
          HepMatrix error(3, 3, 0);

          // Original calculation... has been replaced below with measured values
          // double phiStripFactor = m_phiStripWidth * phiStripMultiplicity;
          // double   zStripFactor = m_zStripWidth   *   zStripMultiplicity;
          // error[1][1] = (1 / 12) * phiStripFactor * phiStripFactor;
          // error[2][2] = (1 / 12) *   zStripFactor *   zStripFactor;
          // Corrected to give better approximation of error at Leo's advice.
          // error[1][1] = (1 / 12) * m_phiStripWidth * m_phiStripWidth * phiStripMultiplicity;
          // error[2][2] = (1 / 12) * m_zStripWidth   * m_zStripWidth   *   zStripMultiplicity;
          // Measured values for multiplicities up to 4: S. Schrenk 28 May 1999 (Belle local run 1174)

          switch (phiStripMultiplicity) {
            case 1:  error[1][1] = 1.3 * 1.3; break;
            case 2:  error[1][1] = 1.3 * 1.3; break;
            case 3:  error[1][1] = 2.0 * 2.0; break;
            case 4:  error[1][1] = 3.3 * 3.3; break;
            default: error[1][1] = phiStripMultiplicity * m_PhiStripWidth * m_PhiStripWidth / 12.0; break;
          }

          switch (zStripMultiplicity) {
            case 1:  error[2][2] = 1.1 * 1.1; break;
            case 2:  error[2][2] = 1.1 * 1.1; break;
            case 3:  error[2][2] = 1.7 * 1.7; break;
            case 4:  error[2][2] = 2.9 * 2.9; break;
            default: error[2][2] = zStripMultiplicity * m_ZStripWidth * m_ZStripWidth / 12.0; break;
          }

          return error;

        }
    */

    /* unused function
        const Rect Module::getStripRectLocal(double stripAve, bool isPhiReadout) const
        {
          Rect rect;

          rect.corner[0].setX(m_LocalX);
          rect.corner[1].setX(m_LocalX);
          rect.corner[2].setX(m_LocalX);
          rect.corner[3].setX(m_LocalX);

          if (isPhiReadout) {
            rect.corner[0].setY(getLocalCoordinate(stripAve + 0.5, true));
            rect.corner[1].setY(getLocalCoordinate(stripAve + 0.5, true));
            rect.corner[2].setY(getLocalCoordinate(stripAve - 0.5, true));
            rect.corner[3].setY(getLocalCoordinate(stripAve - 0.5, true));
            rect.corner[0].setZ(0.0);
            rect.corner[1].setZ(m_PhiStripLength);
            rect.corner[2].setZ(m_PhiStripLength);
            rect.corner[3].setZ(0.0);
          } else {
            rect.corner[0].setY(m_ZStripLength * 0.5);
            rect.corner[1].setY(-m_ZStripLength * 0.5);
            rect.corner[2].setY(-m_ZStripLength * 0.5);
            rect.corner[3].setY(m_ZStripLength * 0.5);
            rect.corner[0].setZ(getLocalCoordinate(stripAve + 0.5, false));
            rect.corner[1].setZ(getLocalCoordinate(stripAve + 0.5, false));
            rect.corner[2].setZ(getLocalCoordinate(stripAve - 0.5, false));
            rect.corner[3].setZ(getLocalCoordinate(stripAve - 0.5, false));
          }

          rect.corner[0] += m_Shift;
          rect.corner[1] += m_Shift;
          rect.corner[2] += m_Shift;
          rect.corner[3] += m_Shift;

          return rect;
        }
    */

    /* unused function
        const Rect Module::getModuleRectLocal(void) const
        {
          Rect rect;

          rect.corner[0].setX(m_LocalX);
          rect.corner[1].setX(m_LocalX);
          rect.corner[2].setX(m_LocalX);
          rect.corner[3].setX(m_LocalX);

          rect.corner[0].setY(m_ZStripLength * 0.5);
          rect.corner[1].setY(-m_ZStripLength * 0.5);
          rect.corner[2].setY(-m_ZStripLength * 0.5);
          rect.corner[3].setY(m_ZStripLength * 0.5);

          rect.corner[0].setZ(0.0);
          rect.corner[1].setZ(0.0);
          rect.corner[2].setZ(m_PhiStripLength);
          rect.corner[3].setZ(m_PhiStripLength);

          rect.corner[0] += m_Shift;
          rect.corner[1] += m_Shift;
          rect.corner[2] += m_Shift;
          rect.corner[3] += m_Shift;

          return rect;
        }
    */

    bool Module::isInActiveArea(const Hep3Vector& pos, int& phiStrip, int& zStrip) const
    {
      Hep3Vector posShifted = pos - m_Shift;

      phiStrip = int(floor((posShifted.y() / m_PhiStripWidth) + m_PhiStripNumber * 0.5));

      zStrip = int(floor(posShifted.z() / m_ZStripWidth));

      return (phiStrip >= m_PhiStripMin) && (phiStrip <= m_PhiStripMax) &&
             (zStrip   >= m_ZStripMin)   && (zStrip   <= m_ZStripMax) &&
             (fabs(posShifted.x() - m_LocalX) <= m_ToleranceX);
    }

    void Module::getStripDivisions(const Hep3Vector& pos, double& phiStripDiv, double& zStripDiv) const
    {
      Hep3Vector posShifted = pos - m_Shift;

      phiStripDiv = remainder((posShifted.y() / m_PhiStripWidth) + 0.5, 1.0);
      zStripDiv   = remainder((posShifted.z() / m_ZStripWidth) + 0.5, 1.0);
    }

    const Hep3Vector Module::localToGlobal(const Hep3Vector& v) const
    {
      return rotateToGlobal(v) + m_Translation;
    }

    const Hep3Vector Module::globalToLocal(const Hep3Vector& v) const
    {
      return rotateToLocal(v - m_Translation);
    }

    const Hep3Vector Module::globalToLocal(double x, double y, double z) const
    {
      return rotateToLocal(Hep3Vector(x, y, z) - m_Translation);
    }

    const HepMatrix Module::localToGlobal(const HepMatrix& m) const
    {
      return (m_RotationInverseMatrix * m * m_RotationMatrix);
    }

    const HepMatrix Module::globalToLocal(const HepMatrix& m) const
    {
      return (m_RotationMatrix * m * m_RotationInverseMatrix);
    }

    const Rect Module::localToGlobal(const Rect& r) const
    {
      Rect global;
      global.corner[0] = localToGlobal(r.corner[0]);
      global.corner[1] = localToGlobal(r.corner[1]);
      global.corner[2] = localToGlobal(r.corner[2]);
      global.corner[3] = localToGlobal(r.corner[3]);
      return global;
    }

    const Rect Module::globalToLocal(const Rect& r) const
    {
      Rect local;
      local.corner[0] = globalToLocal(r.corner[0]);
      local.corner[1] = globalToLocal(r.corner[1]);
      local.corner[2] = globalToLocal(r.corner[2]);
      local.corner[3] = globalToLocal(r.corner[3]);
      return local;
    }

    const Hep3Vector Module::rotateToGlobal(const Hep3Vector& v) const
    {
      return m_Rotation * v;
    }

    const Hep3Vector Module::rotateToLocal(const Hep3Vector& v) const
    {
      return m_RotationInverse * v;
    }

    const Hep3Vector Module::getNormal() const
    {
      return m_Rotation * Hep3Vector(1.0, 0.0, 0.0);
    }

    void Module::printTree() const
    {
      B2INFO("Module: BKLM-"   << (m_IsForward == 0 ? 'F' : 'B')
             << "-S"   << m_Sector
             << (m_Layer < 10 ? "-L0" : "-L") << m_Layer
             << "   "  << m_Shift
             << " x: " << m_LocalX);
    }

  } // end of namespace bklm

} // end of namespace Belle2
