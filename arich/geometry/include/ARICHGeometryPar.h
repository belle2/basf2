/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef ARICHGEOMETRYPAR_H
#define ARICHGEOMETRYPAR_H

#include <vector>
#include <string>
#include <map>
#include <unordered_set>
#include <framework/gearbox/GearDir.h>
#include "TVector3.h"
#include "TVector2.h"
#include <G4ThreeVector.hh>

#define MAX_N_ALAYERS 5
#define MAXPTS_QE 100

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
    static ARICHGeometryPar* Instance();
    //! calculates detector parameters needed for geometry build and reconstruction.
    void Initialize(const GearDir& content);
    //! calculates detector parameters needed for geometry build and reconstruction - the mirrorinfo can be in separate directory
    void Initialize(const GearDir& content, const GearDir& mirrorinfo);

    //! Clears
    void clear(void);

    //! Print some debug information
    void Print(void) const;

    //! gets geometry parameters from gearbox.
    void read(const GearDir& content);

    //! read parameters of each module from gearbox.
    void readModuleInfo(const GearDir& content);

    //! get photocathode quantum efficiency at energy e.
    double QE(double e) const;

    //! get HAPD collection efficiency.
    double getColEffi() const;

    //! get size of detector sensitive surface (size of two chips + gap between)
    double getSensitiveSurfaceSize() const;
    //! get the total number of HAPD modules
    int getNMCopies() const;
    //! get the copy number of HAPD module containing point "hit"
    int getCopyNo(TVector3 hit);
    //! get the position of copyno-th HAPD module origin
    TVector3 getOrigin(int copyno);
    //! get the position of copyNo-th HAPD module origin (returns G4ThreeVector)
    G4ThreeVector getOriginG4(int copyNo);
    //! get the angle of copyno-th HAPD rotation
    double getModAngle(int copyno);
    //! get ID number of channel containing point "hit" (hit is in detector module local coordinates)
    int getChannelID(TVector2 hit);
    //! get center position of chID channel (in detector module local coordinates)
    TVector2 getChannelCenterLoc(int chID);
    //! get channel quantum efficiency
    double getChannelQE(int moduleID, int channelID);
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
    //! get transmission length of "ilayer" aerogel layer
    double getAerogelTransmissionLength(int ilayer);
    //! get refractive index of "ilayer" aerogel layer
    double getAerogelRefIndex(int ilayer);
    //! get thickness of "ilayer" aerogel layer
    double getAerogelThickness(int ilayer);
    //! get z position of "ilayer" aerogel layer
    double getAerogelZPosition(int ilayer);
    //! get number of mirror plates
    int getNMirrors();
    //! get the phi angle of first corner of mirror polygon
    double getMirrorsStartAngle();
    //! get the z position of mirror plates (starting z)
    double getMirrorsZPosition();
    //! get thickness of detector module window
    double getDetectorWindowThickness();
    //! get refractive index of detector module window
    double getDetectorWindowRefIndex();
    //! get number of aerogel layers
    int getNumberOfAerogelRadiators();
    //! get detectors module pad size
    double getDetectorPadSize();
    //! get z position of detector plane (starting z)
    double getDetectorZPosition();
    //! get number of pads of detector module (in one direction)
    int getDetectorXPadNumber();
    //! get QE scaling factor for photons internally reflected in HAPD window
    double getQEScaling();
    //! get absorbtion probability for photons internally reflected in HAPD window
    double getWindowAbsorbtion();
    //! get factor to suppress p.e. detection efficiency due to negative polarity crosstalk among chip channels
    double getChipNegativeCrosstalk();
    //! get number of aerogel tiles in radial direction (number of rings)
    int getNrTiles();
    //! get number of aerogel tiles in each radial ring
    int getNphiTiles(int nRing);
    //! get size of the gap between aerogel tiles
    double getTileGap();
    //! get the inner size of aerogel tube
    double getAeroTubeRin();
    //! get the outer size of aerogel tube
    double getAeroTubeRout();
    //! set transmission length of "ilayer" aerogel layer
    void setAeroTransLength(int ilayer, double trlen);
    //! set refractive index of "ilayer" aerogel layer
    void setAeroRefIndex(int ilayer, double n);
    //! set thickness of "ilayer" aerogel layer
    void setAerogelThickness(int ilayer, double thick);
    //! set z position of "ilayer" aerogel layer
    void setAerogelZPosition(int ilayer, double zPos);
    //! set detector module window refractive index
    void setWindowRefIndex(double refInd);
    //! returns true if parametrisation is initialized, false otherwise
    bool isInit();
    //! returns true if parametrisation is initialized with simple geometry (beamtest)
    bool isSimple();
    //! initialize detector mask
    void initDetectorMask(int nmodules);
    //! set the channel on/off
    void setActive(int module, int channel,  bool val);
    //! check the activity of the channel
    bool isActive(int module, int channel);

    //! returns merger board ID from cooperID and finesse
    int getMergerFromCooper(int cooperID, int finesse);
    //! returns front-end board ID from merger ID and slot
    int getBoardFromMerger(int mergerID, int slot);
    //! returns number of front-end boards connected to the merger
    int getNBoardsOnMerger(int mergerID);

    //! returns ID number of aerogel tile containing locpos (x-y) point.
    int getAerogelTileID(TVector2 locpos);

    //! Returns a set of copper ID's
    const std::unordered_set<unsigned int>& getCopperIDs() const
    {
      return m_copperIDs;
    }

  private:

    std::string m_version;       /*!< The version of geometry parameters. */
    double m_padSize;            /*!< Detector pad size */
    double m_chipGap;            /*!< Gap between chips in detector module */
    double m_detInnerRadius;               /*!< Inner radius of detector tube */
    double m_detOuterRadius;               /*!< Outer radius of detector tube */
    double m_detZpos;                      /*!< Z position of detector plane */
    double m_modXSize;                     /*!< Detector module length */
    double m_modZSize;                     /*!< Detector module height */
    double m_winThick;                     /*!< Thickness of detector module window */
    double m_winRefInd;                    /*!< Detector window refractive index */
    double m_mirrorOuterRad;               /*!< Radius of circle outscribed to mirrors polygon */
    double m_mirrorThickness;              /*!< Thickness of mirror plates */
    double m_mirrorStartAng;               /*!< The angle of first corner of mirror plates polygon */
    int m_nMirrors;                        /*!< Number of mirrors segments */
    double  m_mirrorZPos;                  /*!< Z position of mirror plates (starting z) */
    int m_nPadX;                           /*!< Number of detector module pads in one direction */
    int m_nRad;                            /*!< Number of aerogel layers */
    bool m_init;                          /*!< True if parametrization is already initialized */
    bool m_simple;                        /*!< True if parametrization initialized with simple geometry (beamtest) */
    double m_aeroTrLength[MAX_N_ALAYERS];  /*!< Array of aerogel transmission lenths */
    double m_aeroRefIndex[MAX_N_ALAYERS];  /*!< Array of aerogel refracive indices */
    double m_aeroZPosition[MAX_N_ALAYERS]; /*!< Array of aerogel Z positions */
    double m_aeroThickness[MAX_N_ALAYERS]; /*!< Array of aerogel thickness */
    int  m_nPads;                         /*!< total number of pads in a sensor */
    std::vector<uint32_t> m_DetectorMask; /*!< Detector Mask of inactive channels */
    std::vector<uint8_t>  m_ChannelQE;    /*!< Channel QE at 400nm */

    double m_ColEffi;                     /*!< collection efficiency */
    double m_LambdaFirst;                 /*!< wavelength [nm]: first QE data point */
    double m_LambdaStep;                  /*!< wavelength [nm]: step */
    int m_NpointsQE;                      /*!< number of QE data points */
    double m_qeScale;                     /*!< QE scale factor for photons internally reflected in HAPD window */
    double m_windowAbsorbtion;            /*!< absorbtion probability for photons internally reflected in HAPD window */
    double m_chipNegativeCrosstalk;       /*!< to simulate opposite polarity crosstalk among channels on chip */
    double m_QE[MAXPTS_QE];               /*!< quantum efficiency curve */

    int m_tileNr;                         /*!< Number of aerogel tiles in radial direction */
    int m_tileNphi[5];                    /*!< Number of aerogel tiles in phi direction of each "radial" ring */
    double m_tileGap;                     /*!< Gap size between two aerogel tiles */
    double m_aeroRin;                     /*!< Inner radius of aerogel tube */
    double m_aeroRout;                    /*!< Outer radius of aerogel tube */

    //! calculates the positions of HAPD modules, with the parameters from xml.
    void modulesPosition(const GearDir& content);

    //! gets modules positions directly from xml file (for simple "beamtest" geometry).
    void modulesPositionSimple(const GearDir& content);

    //! calculates the centers of chips in detector module local coordinates
    void chipLocPosition();

    //! calculates the centers of channels in local (detector module) and global coordinates
    void padPositions();
    //! calculates parameters of all mirror planes (normal vector and point on plane)

    //! Calculates mirror positions (normal vectors and on point of every mirror plate) and stores them.
    void mirrorPositions(const GearDir& content);

    //! Reads mirror plates alignment parameters
    void readMirrorAlignment(const GearDir& content);

    //! Gets mirrors positions directly from xml file (in case of simple "beamtest" geometry).
    void mirrorPositionSimple(const GearDir& content);

    //! reads front-end board to merger to cooper mapping from an xml file
    void frontEndMapping(const GearDir& content);

    static ARICHGeometryPar* p_B4ARICHGeometryParDB; /*!< Pointer that saves the instance of this class. */

    // vectors holding information on HAPDs and chips and pads positions.

    std::vector<int>
    m_ncol;         /*!<  m_ncol[i] gives number of detector modules in i-th detector ring (first one is the outer most) */
    std::vector<double> m_fDFi;     /*!< angle covered by one detector module in ring */
    std::vector<double> m_fDR;      /*!< minimal distance between detector modules in radial direction */
    int m_nrow;                     /*!< number of detector rings */
    std::vector<double> m_fR;       /*!< radial coordinate of detector modules */
    std::vector<double> m_fFi;      /*!< angular coordinate of detector modules */
    std::vector<double> m_fFiMod;      /*!< angle of detector module */
    std::vector<TVector2> m_chipLocPos;   /*!< vector holding chip positions (in detector module local coordinates) */
    std::map<int, TVector2> m_padLocPositions; /*!< map holding channel local positions (in detector module local coordinates) */
    std::vector<TVector2> m_padWorldPositions; /*!< map holding channel global positions  */
    std::vector<TVector3> m_mirrornorm;       /*!< vector holding normal vectors of mirror plates */
    std::vector<TVector3> m_mirrorpoint;      /*!< vector holding one point of each mirror plate */

    std::map<std::pair<unsigned, int>, unsigned> m_copper2merger; /*!< mapping of merger boards to cooper boards */
    std::map<int, std::vector<unsigned>> m_merger2feb; /*!< mapping of front-end boards to mergers */
    std::unordered_set<unsigned int> m_copperIDs; /*!< COPPER ID's */
    std::unordered_set<unsigned int> m_mergerIDs; /*!< Merger ID's */
    std::unordered_set<unsigned int> m_boardIDs; /*!< FEB ID's */

  };

  //-----------------------------------------------------------------------------

  inline double ARICHGeometryPar::getColEffi() const
  {
    return m_ColEffi;
  }

  inline double ARICHGeometryPar::getSensitiveSurfaceSize() const
  {
    return m_nPadX * m_padSize + m_chipGap;
  }

  inline int ARICHGeometryPar::getNMCopies() const
  {
    return m_fR.size();
  }

  inline TVector2 ARICHGeometryPar::getChipLocPos(int chipID)
  {
    return m_chipLocPos.at(chipID);
  }

  inline bool ARICHGeometryPar::isInit()
  {
    return m_init;
  }

  inline bool ARICHGeometryPar::isSimple()
  {
    return m_simple;
  }


  inline double ARICHGeometryPar::getAerogelTransmissionLength(int layer)
  {
    return m_aeroTrLength[layer];
  }

  inline double ARICHGeometryPar::getAerogelRefIndex(int layer)
  {
    return m_aeroRefIndex[layer];
  }

  inline double ARICHGeometryPar::getAerogelThickness(int layer)
  {
    return m_aeroThickness[layer];
  }

  inline double ARICHGeometryPar::getAerogelZPosition(int layer)
  {
    return m_aeroZPosition[layer];
  }

  inline int ARICHGeometryPar::getNMirrors()
  {
    return m_nMirrors;
  }

  inline double ARICHGeometryPar::getDetectorWindowThickness()
  {
    return m_winThick;
  }

  inline double ARICHGeometryPar::getDetectorWindowRefIndex()
  {
    return m_winRefInd;
  }

  inline int ARICHGeometryPar::getNumberOfAerogelRadiators()
  {
    return m_nRad;
  }

  inline double ARICHGeometryPar::getDetectorPadSize()
  {
    return m_padSize;
  }

  inline int ARICHGeometryPar::getDetectorXPadNumber()
  {
    return m_nPadX;
  }

  inline double ARICHGeometryPar::getMirrorsStartAngle()
  {
    return m_mirrorStartAng;
  }

  inline double ARICHGeometryPar::getMirrorsZPosition()
  {
    return m_mirrorZPos;
  }

  inline double ARICHGeometryPar::getDetectorZPosition()
  {
    return m_detZpos;
  }

  inline double ARICHGeometryPar::getQEScaling()
  {
    return m_qeScale;
  }

  inline double ARICHGeometryPar::getWindowAbsorbtion()
  {
    return m_windowAbsorbtion;
  }

  inline double ARICHGeometryPar::getChipNegativeCrosstalk()
  {
    return m_chipNegativeCrosstalk;
  }

  inline int ARICHGeometryPar::getNrTiles()
  {
    return m_tileNr;
  }

  inline int ARICHGeometryPar::getNphiTiles(int nRing)
  {
    if (nRing > 4) return 0;
    return m_tileNphi[nRing];
  }

  inline double ARICHGeometryPar::getTileGap()
  {
    return m_tileGap;
  }

  inline double ARICHGeometryPar::getAeroTubeRin()
  {
    return m_aeroRin;
  }

  inline double ARICHGeometryPar::getAeroTubeRout()
  {
    return m_aeroRout;
  }



} // end of namespace Belle2

#endif

