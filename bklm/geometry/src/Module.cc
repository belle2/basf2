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
      m_IsForward(false),
      m_Sector(0),
      m_Layer(0),
      m_Shift(Hep3Vector()),
      m_LocalX(0.0),
      m_ToleranceX(0.0),
      m_SectorPtr(NULL),
      m_PhiStripWidth(0.0),
      m_PhiStripLength(0.0),
      m_PhiStripNumber(0),
      m_PhiStripMin(0),
      m_PhiStripMax(0),
      m_ZStripWidth(0.0),
      m_ZStripLength(0.0),
      m_ZStripNumber(0),
      m_ZStripMin(0),
      m_ZStripMax(0)
    {}

    Module::Module(bool       isForward,
                   int        sector,
                   int        layer,
                   Hep3Vector shift,
                   double     localX,
                   Sector*    sectorPtr,
                   double     phiStripWidth,
                   double     phiStripLength,
                   int        phiStripNumber,
                   int        phiStripMin,
                   int        phiStripMax,
                   double     zStripWidth,
                   double     zStripLength,
                   int        zStripNumber,
                   int        zStripMin,
                   int        zStripMax) :
      m_IsForward(isForward),
      m_Sector(sector),
      m_Layer(layer),
      m_Shift(shift),
      m_LocalX(localX),
      m_ToleranceX(2.0),   // cm
      m_SectorPtr(sectorPtr),
      m_PhiStripWidth(phiStripWidth),
      m_PhiStripLength(phiStripLength),
      m_PhiStripNumber(phiStripNumber),
      m_PhiStripMin(phiStripMin),
      m_PhiStripMax(phiStripMax),
      m_ZStripWidth(zStripWidth),
      m_ZStripLength(zStripLength),
      m_ZStripNumber(zStripNumber),
      m_ZStripMin(zStripMin),
      m_ZStripMax(zStripMax)
    {}

    Module::Module(const Module& m) :
      m_IsForward(m.m_IsForward),
      m_Sector(m.m_Sector),
      m_Layer(m.m_Layer),
      m_Shift(m.m_Shift),
      m_LocalX(m.m_LocalX),
      m_ToleranceX(m.m_ToleranceX),
      m_SectorPtr(m.m_SectorPtr),
      m_PhiStripWidth(m.m_PhiStripWidth),
      m_PhiStripLength(m.m_PhiStripLength),
      m_PhiStripNumber(m.m_PhiStripNumber),
      m_PhiStripMin(m.m_PhiStripMin),
      m_PhiStripMax(m.m_PhiStripMax),
      m_ZStripWidth(m.m_ZStripWidth),
      m_ZStripLength(m.m_ZStripLength),
      m_ZStripNumber(m.m_ZStripNumber),
      m_ZStripMin(m.m_ZStripMin),
      m_ZStripMax(m.m_ZStripMax)
    {}

    Module::~Module()
    {
    }

    /* must be a nonstatic member function
    Module::Module& operator=( const Module& m ) {
      if ( this != &m ) {
        m_IsForward = m.m_IsForward;
        m_Sector = m.m_Sector;
        m_Layer = m.m_Layer;
        m_Shift = m.m_Shift;
        m_LocalX = m.m_LocalX;
        m_SectorPtr = m.m_SectorPtr;
        m_PhiStripWidth = m.m_PhiStripWidth;
        m_PhiStripLength = m.m_PhiStripLength;
        m_PhiStripNumber = m.m_PhiStripNumber;
        m_PhiStripMin = m.m_PhiStripMin;
        m_PhiStripMax = m.m_PhiStripMax;
        m_ZStripWidth = m.m_ZStripWidth;
        m_ZStripLength = m.m_ZStripLength;
        m_ZStripNumber = m.m_ZStripNumber;
        m_ZStripMin = m.m_ZStripMin;
        m_ZStripMax = m.m_ZStripMax;
      }
      return *this;
    }
    */

    bool Module::operator<(const Module& m) const
    {
      if (m_IsForward != m.m_IsForward) return (m_IsForward == 0);
      if (m_Sector    != m.m_Sector)    return (m_Sector < m.m_Sector);
      if (m_Layer     != m.m_Layer)     return (m_Layer < m.m_Layer);
      return false;
    }

    bool Module::isSameModule(bool isForward, int sector, int layer) const
    {
      return (isForward == m_IsForward) &&
             (sector    == m_Sector) &&
             (layer     == m_Layer);
    }

    bool Module::isSameModule(const Module& m) const
    {
      return (m.isForward() == m_IsForward) &&
             (m.getSector() == m_Sector) &&
             (m.getLayer()  == m_Layer);
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
