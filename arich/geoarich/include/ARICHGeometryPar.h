/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj, Rok Pestotnik                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ARICHGEOMETRYPAR_H
#define ARICHGEOMETRYPAR_H

#include <vector>
#include <string>

#include "TVector3.h"
#include "TVector2.h"
#include <cmath>
#include <boost/format.hpp>
#include <iostream>
#include <fstream>

using namespace std;
using namespace boost;

#define MAX_N_ALAYERS 5

namespace Belle2 {

//! The Class for ARICH Geometry Parameters
  /*! This class provides ARICH gemetry paramters for simulation, reconstruction and so on.
      These parameters are gotten from gearbox.
  */

  class ARICHGeometryPar {

  public:

    //! Constructor
    ARICHGeometryPar();

    //! Destructor
    virtual ~ARICHGeometryPar();

    //! Static method to get a reference to the ARICHGeometryPar instance.
    /*!
        \return A reference to an instance of this class.
    */
    static ARICHGeometryPar* Instance(void);

    //! Clears
    void clear(void);

    //! Print some debug information
    void Print(void) const;

    //! Gets geometry parameters from gearbox.
    void read();

    //! These functions return parameters

    double GetAerogelTileSize() const;
    double GetAerogelTubeOuterRadius() const;
    double GetAerogelTubeInnerRadius() const;
    double GetDetectorOuterRadius() const;
    double GetDetectorInnerRadius() const;
    double GetAerogelTileGap() const;
    double GetAerogelZPosition(int layerID) const;
    double GetAerogelThickness(int layerID) const;
    double GetAerogelRefractiveIndex(int layerID) ;
    double GetDetectorZPosition() const;
    double GetDetectorModuleSize() const;
    double GetDetectorThickness() const;
    double GetDetectorWindowThickness() const;
    double GetDetectorWindowRefractiveIndex();
    double GetSensitiveSurfaceSize() const;
    double GetDetectorPadSize() const;
    int GetDetectorBackgroundLevel() const;
    int GetNumberOfAerogelRadiators() const;
    int GetDetectorXPadNumber() const;
    int GetDetectorYPadNumber() const;
    double GetMirrorsOuterRadius() const;
    double GetMirrorsThickness() const;
    double GetMirrorsLength() const;
    double GetMirrorsZPosition() const;
    int GetNMirrors() const;
    //! returns the total number of HAPD modules
    int GetNMCopies() const;
    //! returns the number of aerogel tiles in one layer
    int GetNACopies() const;
    //! returns the position of i-th aerogel tile
    TVector2 GetTilePos(int i);
    //! returns the copy number of HAPD module containing point "hit"
    int GetCopyNo(TVector3 hit);
    //! returns the position of copyno-th HAPD module
    TVector3 GetOrigin(int copyno);
    //! returns the ID of channel containing point "position"
    int GetChannelID(TVector3 position);
    //! returns the center position of channel
    TVector3 GetChannelCenter(int chID);

  private:

    std::string _version; /*!< The version of geometry parameters. */
    double _padSize;
    double _tileSize;
    double _tubeInnerRadius;
    double _tubeOuterRadius;
    double _tileGap;
    double _aeroZpos[MAX_N_ALAYERS];
    double _aeroThick[MAX_N_ALAYERS];
    double _detInnerRadius;
    double _detOuterRadius;
    double _detZpos;
    double _modXSize, _modYSize, _modZSize;
    double _winThick;
    double _sensXSize, _sensYSize;
    double _mirrorZpos;
    double _mirrorLength;
    double _mirrorOuterRad;
    double _mirrorThickness;

    int _nMirrors;
    int _nPadX, _nPadY;
    int _nRad;
    //! calculates the positions of HAPD modules, with the parameters from xml.
    void modules_position();
    //! calculates the positions of aerogel tiles. Hexagon tiles are placed as honeycomb structure into the aerogel tube. This computes the center of each hexagon.
    void aerotile_position();

    static ARICHGeometryPar* p_B4ARICHGeometryParDB; /*!< Pointer that saves the instance of this class. */
    // vectors holding information on HAPDs and aergel tiles positions.
    std::vector<int> _ncol;
    std::vector<double> _fDFi;
    std::vector<double> _fDR;
    int _nrow;
    std::vector<double> _fR;
    std::vector<double> _fFi;
    std::vector<TVector2> _tilePos;
  };

  //-----------------------------------------------------------------------------

  inline double ARICHGeometryPar::GetAerogelTileSize() const
  {
    return _tileSize;
  }

  inline double ARICHGeometryPar::GetAerogelTubeOuterRadius() const
  {
    return _tubeOuterRadius;
  }

  inline double ARICHGeometryPar::GetAerogelTubeInnerRadius() const
  {
    return _tubeInnerRadius;
  }

  inline double ARICHGeometryPar::GetAerogelTileGap() const
  {
    return _tileGap;
  }

  inline double ARICHGeometryPar::GetAerogelZPosition(int layerID) const
  {
    return _aeroZpos[layerID];
  }

  inline double ARICHGeometryPar::GetAerogelThickness(int layerID) const
  {
    return _aeroThick[layerID];
  }

  inline double ARICHGeometryPar::GetDetectorInnerRadius() const
  {
    return _detInnerRadius;
  }

  inline double ARICHGeometryPar::GetDetectorOuterRadius() const
  {
    return _detOuterRadius;
  }

  inline double ARICHGeometryPar::GetDetectorZPosition() const
  {
    return _detZpos;
  }

  inline double ARICHGeometryPar::GetDetectorModuleSize() const
  {
    return _modXSize;
  }

  inline double ARICHGeometryPar::GetDetectorThickness() const
  {
    return _modZSize;
  }

  inline double ARICHGeometryPar::GetDetectorWindowThickness() const
  {
    return _winThick;
  }

  inline double ARICHGeometryPar::GetSensitiveSurfaceSize() const
  {
    return _sensXSize;
  }

  inline double ARICHGeometryPar::GetDetectorPadSize() const
  {
    return _padSize;
  }

  inline int ARICHGeometryPar::GetDetectorXPadNumber() const
  {
    return _nPadX;
  }

  inline int ARICHGeometryPar::GetDetectorYPadNumber() const
  {
    return _nPadY;
  }

  inline int ARICHGeometryPar::GetNumberOfAerogelRadiators() const
  {
    return _nRad;
  }

  inline int ARICHGeometryPar::GetNMCopies() const
  {
    return _fR.size();
  }

  inline int ARICHGeometryPar::GetNACopies() const
  {
    return _tilePos.size();
  }
  inline double ARICHGeometryPar::GetMirrorsZPosition() const
  {
    return  _mirrorZpos;
  }

  inline double ARICHGeometryPar::GetMirrorsLength() const
  {
    return  _mirrorLength;
  }

  inline double ARICHGeometryPar::GetMirrorsOuterRadius() const
  {
    return _mirrorOuterRad;
  }

  inline double ARICHGeometryPar::GetMirrorsThickness() const
  {
    return _mirrorThickness;
  }

  inline int ARICHGeometryPar::GetNMirrors() const
  {
    return _nMirrors;
  }

} // end of namespace Belle2

#endif
