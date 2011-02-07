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

    //! Get size of aerogel hexagon tile (radius of hexagon inscribed circle)
    double GetAerogelTileSize() const;
    //! Get outer radius of aerogel tube
    double GetAerogelTubeOuterRadius() const;
    //! Get inner radius of arogel tube
    double GetAerogelTubeInnerRadius() const;
    //! Get outer radius of detector tube
    double GetDetectorOuterRadius() const;
    //! Get inner radius of detector tube
    double GetDetectorInnerRadius() const;
    //! Get the gap between two aerogel tiles edges
    double GetAerogelTileGap() const;
    //! Get number of aerogel layers
    int GetNumberOfAerogelRadiators() const;
    //! Get z position of layerID aerogel layer (beginning)
    double GetAerogelZPosition(int layerID) const;
    //! Get thicknes of layerID aerogel layer
    double GetAerogelThickness(int layerID) const;
    //! Get refractive index of layerID aerogel layer
    double GetAerogelRefractiveIndex(int layerID) const;
    //! Get transmission length of layerID aerogel layer
    double GetAerogelTransmissionLength(int layerID) const;
    //! Get figure of merit of layerID aerogel layer
    double GetAerogelFigureOfMerit(int layerID) const;
    //! Get z position of photon detectors (begininng of HAPD window)
    double GetDetectorZPosition() const;
    //! Get size of the detector module (HAPD side size)
    double GetDetectorModuleSize() const;
    //! Get detector module thickness (HAPD height)
    double GetDetectorThickness() const;
    //! Get thickness of detector module window
    double GetDetectorWindowThickness() const;
    //! Get refractive index of detector module window
    double GetDetectorWindowRefractiveIndex() const;
    //! Get size of detector sensitive surface (size of two chips + gap between)
    double GetSensitiveSurfaceSize() const;
    //! Get detector pad size
    double GetDetectorPadSize() const;
    //! Get number of detector pads in one direction (all pads = this * this)
    int GetDetectorXPadNumber() const;
    //! Get radius of mirrors polygon outscribed circle
    double GetMirrorsOuterRadius() const;
    //! Get the thickness of mirror plate
    double GetMirrorsThickness() const;
    //! Get length of mirror plates ( ~ from end of aerogel tube to begininng of detector tube)
    double GetMirrorsLength() const;
    //! Get z position of mirrors (starting point)
    double GetMirrorsZPosition() const;
    //! Get the angle of the first corner of mirror plates polygon.
    double GetMirrorsStartAngle() const;
    //! Get track position resolution of tracking
    double GetTrackPositionResolution() const;
    //! Get track direction resolution of tracking
    double GetTrackDirectionResolution() const;
    //! Get single photon resolution without pad (spread of the Cerenkov photons on detector plane due to different emission points)
    double GetSinglePhotonResolutionWoPad() const;
    //! Get detector background level (number of photons / m^2)
    double GetDetectorBackgroundLevel() const;
    //! Get normalisation factor. Factor to adjust the expected number of photons.
    double GetNormalisationFactor() const;
    //! Get number of mirrors segments
    int GetNMirrors() const;
    //! Get the total number of HAPD modules
    int GetNMCopies() const;
    //! Get the number of aerogel tiles in one layer
    int GetNACopies() const;
    //! Get the position of i-th aerogel tile
    TVector2 GetTilePos(int i);
    //! Get the copy number of HAPD module containing point "hit"
    int GetCopyNo(TVector3 hit);
    //! Get the position of copyno-th HAPD module origin
    TVector3 GetOrigin(int copyno);
    //! Get the angle of copyno-th HAPD rotation
    double GetModAngle(int copyno);
    //! Get ID number of channel containing point "hit" (hit is in detector module local coordinates)
    int GetChannelID(TVector2 hit);
    //! Get center position of chID channel (in detector module local coordinates)
    TVector2 GetChannelCenterLoc(int chID);
    //! Get center position of chipID-th chip of detector module (in detector module local coordinates)
    TVector2 GetChipLocPos(int chipID);
    //! Get ID number of chip containing point "locpos"
    int GetChipID(TVector2 locpos);
    //! Get center of chanID channel of modID detector module (in global coordinates)
    TVector3 GetChannelCenterGlob(int modID, int chanID);
    //! Get normal vector of mirID-th mirror plate
    TVector3 GetMirrorNormal(int mirID);
    //! Get one point lying on mirID-th mirror plate
    TVector3 GetMirrorPoint(int mirID);

  private:

    std::string _version;       /*!< The version of geometry parameters. */
    double _padSize;            /*!< Detector pad size */
    double _chipGap;            /*!< Gap between chips in detector module */
    double _tileSize;           /*!< Aerogel tile size (hexagon inscribed circle) */
    double _tubeInnerRadius;    /*!< Inner radius of aerogel tube */
    double _tubeOuterRadius;    /*!< Outer radius of aerogel tube */
    double _tileGap;            /*!< Gap between the edges of neighbor aerogel tiles */
    double _aeroZpos[MAX_N_ALAYERS];      /*!< Z position of aerogel layers */
    double _aeroThick[MAX_N_ALAYERS];     /*!< Thickness of aerogel layers */
    double _aeroRefInd[MAX_N_ALAYERS];    /*!< Refractive index of aerogel layers */
    double _aeroTrLen[MAX_N_ALAYERS];     /*!< Transmission length of aerogel layers */
    double _aeroFigMerit[MAX_N_ALAYERS];  /*!< Figure of merit of aerogel layers */
    double _detInnerRadius;               /*!< Inner radius of detector tube */
    double _detOuterRadius;               /*!< Outer radius of detector tube */
    double _detZpos;                      /*!< Z position of detector plane */
    double _modXSize;                     /*!< Detector module length */
    double _modZSize;                     /*!< Detector module height */
    double _winThick;                     /*!< Thickness of detector module window */
    double _winRefInd;                    /*!< Refractive index of detector module window */
    double _mirrorZpos;                   /*!< Z position of mirrors structure (start point) */
    double _mirrorLength;                 /*!< Length of mirror plates */
    double _mirrorOuterRad;               /*!< Radius of circle outscribed to mirrors polygon */
    double _mirrorThickness;              /*!< Thickness of mirror plates */
    double _mirrorStartAng;               /*!< The angle of first corner of mirror plates polygon */
    double _trackPosRes;                  /*!< Track position resolution from tracking */
    double _trackAngRes;                  /*!< Track direction resolution from tracking */
    double _photRes;                      /*!< Single photon resolution without pad */
    double _detBack;                      /*!< Detector background level */
    double _normFact;                     /*!< Normalisation factor (for expected number of photons) */
    int _nMirrors;                        /*!< Number of mirrors segments */
    int _nPadX;                           /*!< Number of detector module pads in one direction */
    int _nRad;                            /*!< Number of aerogel layers */

    //! calculates the positions of HAPD modules, with the parameters from xml.
    void modules_position();

    //! calculates the positions of aerogel tiles. Hexagon tiles are placed as honeycomb structure into the aerogel tube. This computes the center of each hexagon.
    void aerotile_position();

    //! calculates the centers of chips in detector module local coordinates
    void chipLocPosition();

    //! calculates the centers of channels in local (detector module) and global coordinates
    void PadPositions();
    //! calculates parameters of all mirror planes (normal vector and point on plane)

    //! Calculates mirror positions (normal vectors and on point of every mirror plate) and stores them.
    void MirrorPositions();

    static ARICHGeometryPar* p_B4ARICHGeometryParDB; /*!< Pointer that saves the instance of this class. */

    // vectors holding information on HAPDs and aergel tiles positions.

    std::vector<int> _ncol;         /*!<  _ncol[i] gives number of detector modules in i-th detector ring (first one is the outer most) */
    std::vector<double> _fDFi;     /*!< angle covered by one detector module in ring */
    std::vector<double> _fDR;      /*!< minimal distance between detector modules in radial direction */
    int _nrow;                     /*!< number of detector rings */
    std::vector<double> _fR;       /*!< radial coordinate of detector modules */
    std::vector<double> _fFi;      /*!< angular coordinate of detector modules */
    std::vector<TVector2> _tilePos;  /*!< vector holding aerogel tile center positions */
    std::vector<TVector2> _chipLocPos;   /*!< vector holding chip positions (in detector module local coordinates) */
    std::map<int, TVector2> _padLocPositions; /*!< map holding channel local positions (in detector module local coordinates) */
    std::map<std::pair<int, int>, TVector3> _padWorldPositions;  /*!< map holding channel global positions  */
    std::vector<TVector3> _mirrornorm;       /*!< vector holding normal vectors of mirror plates */
    std::vector<TVector3> _mirrorpoint;      /*!< vector holding one point of each mirror plate */

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

  inline double ARICHGeometryPar::GetAerogelRefractiveIndex(int layerID) const
  {
    return _aeroRefInd[layerID];
  }

  inline double ARICHGeometryPar::GetDetectorWindowRefractiveIndex() const
  {
    return _winRefInd;
  }

  inline double ARICHGeometryPar::GetAerogelTransmissionLength(int layerID) const
  {
    return _aeroTrLen[layerID];
  }

  inline double ARICHGeometryPar::GetAerogelFigureOfMerit(int layerID) const
  {
    return _aeroFigMerit[layerID];
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
    return _nPadX*_padSize + _chipGap;
  }

  inline double ARICHGeometryPar::GetDetectorPadSize() const
  {
    return _padSize;
  }

  inline int ARICHGeometryPar::GetDetectorXPadNumber() const
  {
    return _nPadX;
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

  inline double ARICHGeometryPar::GetMirrorsStartAngle() const
  {
    return _mirrorStartAng;
  }

  inline TVector2 ARICHGeometryPar::GetChipLocPos(int chipID)
  {
    return _chipLocPos.at(chipID);
  }
  inline double ARICHGeometryPar::GetTrackPositionResolution() const
  {
    return _trackPosRes;
  }
  inline double ARICHGeometryPar::GetTrackDirectionResolution() const
  {
    return _trackAngRes;
  }
  inline double ARICHGeometryPar::GetSinglePhotonResolutionWoPad() const
  {
    return _photRes;
  }
  inline double ARICHGeometryPar::GetDetectorBackgroundLevel() const
  {
    return _detBack;
  }

  inline double ARICHGeometryPar::GetNormalisationFactor() const
  {
    return _normFact;
  }

} // end of namespace Belle2

#endif

