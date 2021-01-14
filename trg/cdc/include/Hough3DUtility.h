#ifndef HOUGH3DUTILITY_H
#include <vector>
#include <TVectorD.h>
#define HOUGH3DUTILITY_H

namespace Belle2 {
  class TRGCDCJSignal;
  class TRGCDCJLUT;
  class TRGCDCJSignalData;
}

/// A class to finded stereo TS hits related to 2D tracks.
class Hough3DFinder {
public:
  /// 3D finder constructor.
  Hough3DFinder(void);
  /// 3D finder destructor.
  ~Hough3DFinder(void);
  /// Copy constructor, deleted.
  Hough3DFinder(Hough3DFinder&) = delete;
  /// Assignment operator, deleted.
  Hough3DFinder& operator=(Hough3DFinder&) = delete;
  /// Sets which 3D finder to use.
  void setMode(int mode);
  /// Gets which 3D finder is used.
  int getMode(void);
  /// Geometry variables. [rr0, rr1, rr2, rr3, anglest0, anglest1, anglest2, anglest3, ztostraw0, ztostraw1, ztostraw2, ztostraw3]
  /// Init variables. [cotStart, cotEnd, z0Start, z0End, nCotSteps, nZ0Steps]
  /// Initializes the 3D finder.
  void initialize(const TVectorD& geometryVariables , std::vector<float >& initVariables);
  /// Destructs the 3D finder.
  void destruct(void);
  /// Track variables. [charge, rho, phi0]
  /// Stereo TS candidates[layer][TS ID]
  /// Uses the 3D finder.
  void runFinder(const std::vector<double>& trackVariables, std::vector<std::vector<double> >& stTSs,
                 const std::vector<std::vector<int> >& stTSDrift);
  /// Init variables. [cotStart, cotEnd, z0Start, z0Ent, nCotSteps, nZ0Steps]
  /// Initializes the 3D finder for mode 1.
  void initVersion1(const std::vector<float >& initVariables);
  /// Initializes the 3D finder for mode 2.
  void initVersion2(std::vector<float >& initVariables);
  /// Initializes the 3D finder for mode 3.
  void initVersion3(std::vector<float >& initVariables);
  /// Sets the config file for the GeoFinder.
  void setInputFileName(const std::string& inputFileName);
  /// Destructs the 3D finder for mode 1.
  void destVersion1(void);
  /// Destructs the 3D finder for mode 2.
  void destVersion2(void);
  /// Destructs the 3D finder for mode 3.
  void destVersion3(void);
  /// Uses the 3D finder for mode 1.
  void runFinderVersion1(const std::vector<double>& trackVariables, const std::vector<std::vector<double> >& stTSs,
                         const std::vector<double>& tsArcS,
                         const std::vector<std::vector<double> >& tsZ);
  /// Uses the 3D finder for mode 2.
  void runFinderVersion2(const std::vector<double>& trackVariables, std::vector<std::vector<double> >& stTSs,
                         const std::vector<std::vector<int> >& stTSDrift);
  /// Uses the 3D finder for mode 3.
  void runFinderVersion3(const std::vector<double>& trackVariables, std::vector<std::vector<double> >& stTSs,
                         const std::vector<std::vector<int> >& stTSDrift);
  /// Gets results from the 3D finder.
  void getValues(const std::string& input, std::vector<double>& result);
  /// Gets the Hough plane for the 3D finder.
  void getHoughMeshLayer(bool***& houghMeshLayer);
  /// Members.
  /// Value that holds which mode the 3D finder is in.
  int m_mode;
  /// Holds the number of wires for stereo superlayer.
  int m_nWires[4];
  /// Holds the radius for stereo super layer in cm.
  double m_rr[4];
  /// Holds the length of for stereo super layer from center in cm
  double m_ztostraw[4];
  /// Holds the tan theta of streo super layer in radian.
  double m_anglest[4];
  /// Hold the PI value.
  double m_Trg_PI;
  /// Hough finder variables.
  /// Hough mesh cot start range.
  double m_cotStart;
  /// Hough mesh cot end range.
  double m_cotEnd;
  /// Hough mesh z0 start range.
  double m_z0Start;
  /// Hough mesh z0 end range.
  double m_z0End;
  /// Number of Hough meshes for cot.
  int m_nCotSteps;
  /// Number of Hough meshes for z0.
  int m_nZ0Steps;
  /// Holds the size of Hough mesh for cot.
  double m_cotStepSize;
  /// Holds the size of Hough mesh for z0.
  double m_z0StepSize;
  /// Hold the minimum z differences for the Hough vote in a stereo superlayer.
  float*** m_houghMeshLayerDiff;
  /// Map to check if there is a Hough vote in a stereo superlayer.
  bool*** m_houghMeshLayer;
  /// Map that combines the number of Hough votes for all stereo superlayers.
  int** m_houghMesh;
  /// Map that combines the z differences for all stereo superlayers.
  float** m_houghMeshDiff;
  /// Hit map for all streo superlayers.
  bool** m_hitMap;
  /// Drift map for all streo superlayers.
  int** m_driftMap;
  /// GeoFinder Variables.
  /// The index for stereo superlayer hits.
  std::vector< std::vector< int> >* m_geoCandidatesIndex;
  /// The phi for stereo superlayer hits.
  std::vector< std::vector< double> >* m_geoCandidatesPhi;
  /// The number of wires difference from fitted axial phi location.
  std::vector< std::vector< double> >* m_geoCandidatesDiffStWires;
  /// The fitted axial phi for stereo superlayers.
  double m_stAxPhi[4];
  /// Finder results.
  /// The found cot value for track.
  double m_bestCot;
  /// The found z0 value for track.
  double m_bestZ0;
  /// The maximum vote number for track.
  double m_houghMax;
  /// The minimum z diff between all Hough votes that have maximum vote number.
  double m_minDiffHough;
  /// The z location for stereo superlayer using found z0 and cot values for track.
  double m_foundZ[4];
  /// The phi location for streo superlayer using found z0 and cot values for track.
  double m_foundPhiSt[4];
  /// The hit index of the found TSs.
  int m_bestTSIndex[4];
  /// The phi location of the found TSs.
  double m_bestTS[4];
  /// Version3 (GeoFinder Integer space)
  /// GeoFinder input file for parameters.
  std::string m_inputFileName;
  /// [rho, phi0, sign]
  /// Holds input values of the GeoFinder.
  std::vector<double> m_FPGAInput;
  /// [arcCos0, arcCos1, arcCos2, arcCos3,
  ///  bestTSIndex0, bestTSIndex1, bestTSIndex2, bestTSIndex3]
  /// Holds output values of the GeoFinder.
  std::vector<double> m_FPGAOutput;
  /// Find min and max values
  /// Holds the maximum value for rho.
  double m_findRhoMax;
  /// Holds the minimum value for rho.
  double m_findRhoMin;
  /// Holds the maximum value for integer rho.
  double m_findRhoIntMax;
  /// Holds the minimum value for integer rho.
  double m_findRhoIntMin;
  /// Holds the maximum value for phi0.
  double m_findPhi0Max;
  /// Holds the minimum value for phi0.
  double m_findPhi0Min;
  /// Holds the maximum value for integer phi0.
  double m_findPhi0IntMax;
  /// Holds the minimum value for integer phi0.
  double m_findPhi0IntMin;
  /// Holds the maximum value for arc cos(radius/2/rho).
  double m_findArcCosMax;
  /// Holds the minimum value for arc cos(radius/2/rho).
  double  m_findArcCosMin;
  /// Holds the maximum value for intger arc cos(radius/2/rho).
  double m_findArcCosIntMax;
  /// Holds the minimum value for intger arc cos(radius/2/rho).
  double m_findArcCosIntMin;
  /// Holds the maximum for fitted axial phi location between superlayers.
  double m_findPhiZMax;
  /// Holds the minimum value for fitted axial phi location between superlayers.
  double  m_findPhiZMin;
  /// Holds the maximum value for fitted integer axial phi location between superlayers.
  double m_findPhiZIntMax;
  /// Holds the minimum value for fitted integer axial phi location between superlayers.
  double m_findPhiZIntMin;
  /// Integer space
  /// The rho max value for integer geo finder.
  double m_rhoMax;
  /// The rho min value for integer geo finder.
  double m_rhoMin;
  /// The number of bits of rho for integer geo finder.
  int m_rhoBit;
  /// The phi0 max value for integer geo finder.
  double m_phi0Max;
  /// The phi0 min value for integer geo finder.
  double m_phi0Min;
  /// The number of bits of phi0 for integer geo finder.
  int m_phi0Bit;
  /// A factor that changes phi space to wire space.
  int m_stAxWireFactor;
  /// FPGA LUTs
  /// A flag to check if acos LUT and wire convert LUT was set for geo finder.
  bool m_LUT;
  /// Memory for acos LUT.
  int** m_arcCosLUT;
  /// Memory for wire convert LUT.
  int** m_wireConvertLUT;

  /// Map to hold JSignals.
  std::map<std::string, Belle2::TRGCDCJSignal> m_mSignalStorage;
  /// Map to hold JLuts.
  std::map<std::string, Belle2::TRGCDCJLUT*> m_mLutStorage;
  /// For VHDL code.
  Belle2::TRGCDCJSignalData* m_commonData;
  /// Array of saved signals.
  std::map<std::string, std::vector<signed long long> > m_mSavedSignals;
  /// Array of I/O signals.
  std::map<std::string, std::vector<signed long long> > m_mSavedIoSignals;
  /// Map to hold input options.
  std::map<std::string, bool> m_mBool;
  /// Output directory for vhdl
  std::string m_outputVhdlDirname;
  /// Output directory for luts
  std::string m_outputLutDirname;


};

#ifdef __CINT__
#include "../src/Hough3DUtility.cc"
#endif

#endif
