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
#include <map>

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

    //! gets geometry parameters from gearbox.
    void read();

    //! get outer radius of aerogel tube
    double getAerogelTubeOuterRadius() const;
    //! get inner radius of arogel tube
    double getAerogelTubeInnerRadius() const;
    //! get outer radius of detector tube
    double getDetectorOuterRadius() const;
    //! get inner radius of detector tube
    double getDetectorInnerRadius() const;
    //! get number of aerogel layers
    int getNumberOfAerogelRadiators() const;
    //! get z position of layerID aerogel layer (beginning)
    double getAerogelZPosition(int layerID) const;
    //! get thicknes of layerID aerogel layer
    double getAerogelThickness(int layerID) const;
    //! get refractive index of layerID aerogel layer
    double getAerogelRefractiveIndex(int layerID) const;
    //! get transmission length of layerID aerogel layer
    double getAerogelTransmissionLength(int layerID) const;
    //! get figure of merit of layerID aerogel layer
    double getAerogelFigureOfMerit(int layerID) const;
    //! get z position of photon detectors (begininng of HAPD window)
    double getDetectorZPosition() const;
    //! get size of the detector module (HAPD side size)
    double getDetectorModuleSize() const;
    //! get detector module thickness (HAPD height)
    double getDetectorThickness() const;
    //! get thickness of detector module window
    double getDetectorWindowThickness() const;
    //! get refractive index of detector module window
    double getDetectorWindowRefractiveIndex() const;
    //! get size of detector sensitive surface (size of two chips + gap between)
    double getSensitiveSurfaceSize() const;
    //! get detector pad size
    double getDetectorPadSize() const;
    //! get number of detector pads in one direction (all pads = this * this)
    int getDetectorXPadNumber() const;
    //! get radius of mirrors polygon outscribed circle
    double getMirrorsOuterRadius() const;
    //! get the thickness of mirror plate
    double getMirrorsThickness() const;
    //! get length of mirror plates ( ~ from end of aerogel tube to begininng of detector tube)
    double getMirrorsLength() const;
    //! get z position of mirrors (starting point)
    double getMirrorsZPosition() const;
    //! get the angle of the first corner of mirror plates polygon.
    double getMirrorsStartAngle() const;
    //! get track position resolution of tracking
    double getTrackPositionResolution() const;
    //! get track direction resolution of tracking
    double getTrackDirectionResolution() const;
    //! get single photon resolution without pad (spread of the Cerenkov photons on detector plane due to different emission points)
    double getSinglePhotonResolutionWoPad() const;
    //! get detector background level (number of photons / m^2)
    double getDetectorBackgroundLevel() const;
    //! get normalisation factor. Factor to adjust the expected number of photons.
    double getNormalisationFactor() const;
    //! get number of mirrors segments
    int getNMirrors() const;
    //! get the total number of HAPD modules
    int getNMCopies() const;
    //! get the copy number of HAPD module containing point "hit"
    int getCopyNo(TVector3 hit);
    //! get the position of copyno-th HAPD module origin
    TVector3 getOrigin(int copyno);
    //! get the angle of copyno-th HAPD rotation
    double getModAngle(int copyno);
    //! get ID number of channel containing point "hit" (hit is in detector module local coordinates)
    int getChannelID(TVector2 hit);
    //! get center position of chID channel (in detector module local coordinates)
    TVector2 getChannelCenterLoc(int chID);
    //! get center position of chipID-th chip of detector module (in detector module local coordinates)
    TVector2 getChipLocPos(int chipID);
    //! get ID number of chip containing point "locpos"
    int getChipID(TVector2 locpos);
    //! get center of chanID channel of modID detector module (in global coordinates)
    TVector3 getChannelCenterGlob(int modID, int chanID);
    //! get normal vector of mirID-th mirror plate
    TVector3 getMirrorNormal(int mirID);
    //! get one point lying on mirID-th mirror plate
    TVector3 getMirrorPoint(int mirID);

  private:

    std::string _version;       /*!< The version of geometry parameters. */
    double _padSize;            /*!< Detector pad size */
    double _chipGap;            /*!< Gap between chips in detector module */
    double _tubeInnerRadius;    /*!< Inner radius of aerogel tube */
    double _tubeOuterRadius;    /*!< Outer radius of aerogel tube */
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
    void modulesPosition();

    //! calculates the centers of chips in detector module local coordinates
    void chipLocPosition();

    //! calculates the centers of channels in local (detector module) and global coordinates
    void padPositions();
    //! calculates parameters of all mirror planes (normal vector and point on plane)

    //! Calculates mirror positions (normal vectors and on point of every mirror plate) and stores them.
    void mirrorPositions();

    static ARICHGeometryPar* p_B4ARICHGeometryParDB; /*!< Pointer that saves the instance of this class. */

    // vectors holding information on HAPDs and chips and pads positions.

    std::vector<int> _ncol;         /*!<  _ncol[i] gives number of detector modules in i-th detector ring (first one is the outer most) */
    std::vector<double> _fDFi;     /*!< angle covered by one detector module in ring */
    std::vector<double> _fDR;      /*!< minimal distance between detector modules in radial direction */
    int _nrow;                     /*!< number of detector rings */
    std::vector<double> _fR;       /*!< radial coordinate of detector modules */
    std::vector<double> _fFi;      /*!< angular coordinate of detector modules */
    std::vector<TVector2> _chipLocPos;   /*!< vector holding chip positions (in detector module local coordinates) */
    std::map<int, TVector2> _padLocPositions; /*!< map holding channel local positions (in detector module local coordinates) */
    std::map<std::pair<int, int>, TVector3> _padWorldPositions; /*!< map holding channel global positions  */
    std::vector<TVector3> _mirrornorm;       /*!< vector holding normal vectors of mirror plates */
    std::vector<TVector3> _mirrorpoint;      /*!< vector holding one point of each mirror plate */

  };

  //-----------------------------------------------------------------------------

  inline double ARICHGeometryPar::getAerogelTubeOuterRadius() const
  {
    return _tubeOuterRadius;
  }

  inline double ARICHGeometryPar::getAerogelTubeInnerRadius() const
  {
    return _tubeInnerRadius;
  }

  inline double ARICHGeometryPar::getAerogelZPosition(int layerID) const
  {
    return _aeroZpos[layerID];
  }

  inline double ARICHGeometryPar::getAerogelThickness(int layerID) const
  {
    return _aeroThick[layerID];
  }

  inline double ARICHGeometryPar::getAerogelRefractiveIndex(int layerID) const
  {
    return _aeroRefInd[layerID];
  }

  inline double ARICHGeometryPar::getDetectorWindowRefractiveIndex() const
  {
    return _winRefInd;
  }

  inline double ARICHGeometryPar::getAerogelTransmissionLength(int layerID) const
  {
    return _aeroTrLen[layerID];
  }

  inline double ARICHGeometryPar::getAerogelFigureOfMerit(int layerID) const
  {
    return _aeroFigMerit[layerID];
  }


  inline double ARICHGeometryPar::getDetectorInnerRadius() const
  {
    return _detInnerRadius;
  }

  inline double ARICHGeometryPar::getDetectorOuterRadius() const
  {
    return _detOuterRadius;
  }

  inline double ARICHGeometryPar::getDetectorZPosition() const
  {
    return _detZpos;
  }

  inline double ARICHGeometryPar::getDetectorModuleSize() const
  {
    return _modXSize;
  }

  inline double ARICHGeometryPar::getDetectorThickness() const
  {
    return _modZSize;
  }

  inline double ARICHGeometryPar::getDetectorWindowThickness() const
  {
    return _winThick;
  }

  inline double ARICHGeometryPar::getSensitiveSurfaceSize() const
  {
    return _nPadX*_padSize + _chipGap;
  }

  inline double ARICHGeometryPar::getDetectorPadSize() const
  {
    return _padSize;
  }

  inline int ARICHGeometryPar::getDetectorXPadNumber() const
  {
    return _nPadX;
  }

  inline int ARICHGeometryPar::getNumberOfAerogelRadiators() const
  {
    return _nRad;
  }

  inline int ARICHGeometryPar::getNMCopies() const
  {
    return _fR.size();
  }

  inline double ARICHGeometryPar::getMirrorsZPosition() const
  {
    return  _mirrorZpos;
  }

  inline double ARICHGeometryPar::getMirrorsLength() const
  {
    return  _mirrorLength;
  }

  inline double ARICHGeometryPar::getMirrorsOuterRadius() const
  {
    return _mirrorOuterRad;
  }

  inline double ARICHGeometryPar::getMirrorsThickness() const
  {
    return _mirrorThickness;
  }

  inline int ARICHGeometryPar::getNMirrors() const
  {
    return _nMirrors;
  }

  inline double ARICHGeometryPar::getMirrorsStartAngle() const
  {
    return _mirrorStartAng;
  }

  inline TVector2 ARICHGeometryPar::getChipLocPos(int chipID)
  {
    return _chipLocPos.at(chipID);
  }
  inline double ARICHGeometryPar::getTrackPositionResolution() const
  {
    return _trackPosRes;
  }
  inline double ARICHGeometryPar::getTrackDirectionResolution() const
  {
    return _trackAngRes;
  }
  inline double ARICHGeometryPar::getSinglePhotonResolutionWoPad() const
  {
    return _photRes;
  }
  inline double ARICHGeometryPar::getDetectorBackgroundLevel() const
  {
    return _detBack;
  }

  inline double ARICHGeometryPar::getNormalisationFactor() const
  {
    return _normFact;
  }

} // end of namespace Belle2

#endif

