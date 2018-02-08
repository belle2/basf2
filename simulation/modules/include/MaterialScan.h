/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef MATERIALSCANMODULE_H_
#define MATERIALSCANMODULE_H_

#include <framework/core/Module.h>
#include <framework/gearbox/Unit.h>

#include <string>
#include <vector>
#include <set>

#include <G4UserSteppingAction.hh>
#include <G4ThreeVector.hh>

#include <TFile.h>
class TH1D;
class TH2D;

namespace Belle2 {

  /** Base class for Material Scans */
  class MaterialScanBase: public G4UserSteppingAction {
  public:
    /** This is indeed the constructor and it creates a TDirectory in the
     * output root file and sets all variables */
    MaterialScanBase(TFile* rootFile, const std::string& name, const std::string& axisLabel):
      m_rootFile(rootFile), m_name(name), m_axisLabel(axisLabel)
    {
      m_rootFile->mkdir(name.c_str());
    }
    /** Return the name of the scan */
    std::string getName() const { return m_name; }
    /** Return the number of rays necessary to perform the scan */
    virtual int getNRays() const = 0;
    /** Get the origin and direction for the next scan particle.
     * @param[out] origin Origin of the next scan particle
     * @param[out[ direction Direction of the next scan particle
     * @return false if the scan is finished
     */
    virtual bool createNext(G4ThreeVector& origin, G4ThreeVector& direction) = 0;

  protected:
    /** check for stuck tracks by looking at the step length */
    bool checkStep(const G4Step* step);
    /** Pointer to the root file for the histograms */
    TFile* m_rootFile;
    /** Name of the scan, will be prefixed to all histogram names */
    std::string m_name;
    /** Labels for the coordinate axes */
    std::string m_axisLabel;
  private:
    /** maximum Step length to be considered zero */
    static constexpr double c_zeroTolerance = 1e-6;
    /** maximum number of consecutive zero steps before nudging the track along */
    static constexpr int c_maxZeroStepsNudge = 10;
    /** maximum number of consecutive zero steps before killing the track */
    static constexpr int c_maxZeroStepsKill = 20;
    /** Count the number of steps with (almost) zero length */
    int m_zeroSteps {0};
  };

  /** Base class to create a Material Scan of the detector geometry.
   * This class is used to record the Material budget encountered by a particle
   * and provide origin and directions of all particles used for the scan.
   */
  class MaterialScan2D: public MaterialScanBase {
  public:
    /**Helper struct to Store Parameters of a Scan. */
    struct ScanParams {
      /** Default Constructor */
      ScanParams(): nU(0), nV(0), minU(0), maxU(0), minV(0), maxV(0), maxDepth(-1), splitByMaterials(false) {}
      /** Number of rays along u coordinate */
      int nU;
      /** Number of rays along v coordinate */
      int nV;
      /** Minimum u value to scan */
      double minU;
      /** Maximum u value to scan */
      double maxU;
      /** Minimum v value to scan */
      double minV;
      /** Maximum v value to scan */
      double maxV;
      /** Maximum depth of the scan */
      double maxDepth;
      /** Names of ignored Materials */
      std::vector<std::string> ignoredMaterials;
      /** If true, split output by Materials (otherwise by region) */
      bool splitByMaterials;
    };

    /** Constructor
     * @param rootFile Pointer to the ROOTFile where the histograms will be stored.
     * @param name Prefix to preprend to all Histograms
     * @param axisLabel Labels for the histogram axes, separated by semicolon
     * @param params Parameters for the scan
     */
    MaterialScan2D(TFile* rootFile, const std::string& name, const std::string& axisLabel, ScanParams params);

    /** Get the origin and direction for the next scan particle.
     * @param[out] origin Origin of the next scan particle
     * @param[out[ direction Direction of the next scan particle
     * @return false if the scan is finished
     */
    bool createNext(G4ThreeVector& origin, G4ThreeVector& direction) override;

    /** Return the number of rays in this scan */
    int getNRays() const override { return m_params.nU * m_params.nV; }

    /** Record the material budget for each step of the particles */
    void UserSteppingAction(const G4Step* step);
  protected:
    /** Get the origin and direction for the next scan particle.
     * To be overridden by descendents
     * @param[out] origin Origin of the next scan particle
     * @param[out[ direction Direction of the next scan particle
     */
    virtual void getRay(G4ThreeVector& origin, G4ThreeVector& direction) = 0;

    /** get histogram for a given name, create if needed.
     * @param name Name of the histogram
     */
    TH2D* getHistogram(const std::string& name);

    /** Fill the recorded material budget into the corresponding histogram
     * @param name Name of the histogram
     * @param value Value to store
     */
    void fillValue(const std::string& name, double value);

    /** Parameters for the scan */
    ScanParams m_params;
    /** Current value of the parametetr u */
    double m_curU;
    /** Stepsize for the parameter u */
    double m_stepU;
    /** Current value of the parametetr v */
    double m_curV;
    /** Stepsize for the parameter v */
    double m_stepV;
    /** Tracklength of the current Ray */
    double m_curDepth;
    /** Map holding pointers to all created histograms */
    std::map<std::string, TH2D*> m_regions;
  };

  /** Specific implementation of MaterialScan to do Spherical scanning.
   * That is shooting rays from the origin with varying aximuth and polar angle.
   */
  class MaterialScanSpherical: public MaterialScan2D {
  public:
    /** Create a Spherical Scan object with the given parameters
     * @param rootFile pointer to the ROOT File containing the histograms
     * @param origin Origin for the spherical scan
     * @param params Parameters of the scan
     */
    MaterialScanSpherical(TFile* rootFile, const G4ThreeVector& origin, ScanParams params, bool doCosTheta):
      MaterialScan2D(rootFile, "Spherical", doCosTheta ? "cos(#theta);#phi [deg]" : "#theta [deg];#phi [deg]", params), m_origin(origin),
      m_doCosTheta(doCosTheta)
    {
      if (doCosTheta) {
        m_params.minU = cos(m_params.minU * Unit::deg);
        m_params.maxU = cos(m_params.maxU * Unit::deg);
        if (m_params.minU > m_params.maxU) std::swap(m_params.minU, m_params.maxU);
        m_stepU = (m_params.maxU - m_params.minU) / m_params.nU;
        m_curU  = m_params.minU - m_stepU / 2.;
      }
    }
  protected:
    /** Create a ray with the current parameter values according to a spherical distribution */
    void getRay(G4ThreeVector& origin, G4ThreeVector& direction);

    /** Origin for the spherical scan */
    G4ThreeVector m_origin;

    /** Flag to indicate if polar-angular sampling is uniform in cos(theta) rather than theta */
    bool m_doCosTheta;
  };

  /** Specific implementaion of MaterialScan to scan parallel to a given plane.
   *
   * Particles will be created in a grid on the given plane and will be traced
   * perpendicluar to that plane The plane is defined by one origin and the
   * directions of the grid coordinates u and v. The flight direction is
   * determined by the cross product between the u and v axis.
   */
  class MaterialScanPlanar: public MaterialScan2D {
  public:
    /** Create a Planar Scan object with the given parameters
     * @param rootFile pointer to the ROOT File containing the histograms
     * @param origin Origin of the plane to shoot rays from
     * @param dirU Direction of the u coordinate
     * @param dirV Direction of the v coordinate
     * @param params Parameters of the scan
     */
    MaterialScanPlanar(TFile* rootFile, const G4ThreeVector& origin, const G4ThreeVector& dirU, const G4ThreeVector& dirV,
                       ScanParams params):
      MaterialScan2D(rootFile, "Planar", "u [cm];v [cm]", params), m_origin(origin), m_dirU(dirU.unit()), m_dirV(dirV.unit()),
      m_dirW(m_dirU.cross(m_dirV))
    {
    }
  protected:
    /** Create a ray with the current parameter values according to a planar distribution */
    void getRay(G4ThreeVector& origin, G4ThreeVector& direction);
  protected:
    /** Origin of the scan plane */
    G4ThreeVector m_origin;
    /** u direction of the scan plane */
    G4ThreeVector m_dirU;
    /** v direction of the scan plane */
    G4ThreeVector m_dirV;
    /** direction perpendicluar to u and v */
    G4ThreeVector m_dirW;
  };

  /** MaterialScan implementation to shoot one ray along a defined direction
   * and record the Material as a function of travel depth.
   *
   * In contrast to the other implementations this produces a 1D histogram per
   * region (plus a cumulative one) which contains the X_0 per bin so that the
   * integral of the histogram (\sum_i binwidth_i * bincontent_i) is equal to
   * the total number of X_0 seen.
   */
  class MaterialScanRay: public MaterialScanBase {
  public:
    /** Construct a new instance and set all parameters */
    MaterialScanRay(TFile* rootFile, const G4ThreeVector& origin, const G4ThreeVector& dir, double opening,
                    int count, double sampleDepth, double maxDepth, bool splitByMaterials,
                    const std::vector<std::string>& ignoredMaterials):
      MaterialScanBase(rootFile, "Ray", "ray length [cm]; material budget [X_0/cm]"),
      m_origin(origin), m_dir(dir), m_opening(opening), m_count(std::max(1, count)),
      m_sampleDepth(sampleDepth), m_maxDepth(maxDepth), m_splitByMaterials(splitByMaterials),
      m_ignoredMaterials(ignoredMaterials.begin(), ignoredMaterials.end())
    {
      if (m_opening <= 0) m_count = 1;
    }

    /** Return the number of rays. We have one extra ray to scan the maximum depth */
    int getNRays() const override { return m_count + 1; }
    /** Record the material budget for each step of the particles */
    void UserSteppingAction(const G4Step* step) override;
    /** Implement shooting along the ray */
    bool createNext(G4ThreeVector& origin, G4ThreeVector& direction) override;
  private:
    /** get histogram for a given name, create if needed.
     * @param name Name of the histogram
     */
    TH1D* getHistogram(const std::string& name);
    /** Fill the recorded material budget into the corresponding histogram
     * @param name Name of the histogram
     * @param value Value to store
     * @param steplength The Steplength which produced the value (for correct subsampling)
     */
    void fillValue(const std::string& name, double value, double steplength);
    /** Origin of the scan */
    G4ThreeVector m_origin;
    /** Direction of the ray */
    G4ThreeVector m_dir;
    /** Opening angle in radian */
    double m_opening;
    /** Amount of rays to shoot */
    int m_count;
    /** The ray length after which to sample. Basically the bin width of the histogram */
    double m_sampleDepth;
    /** Maximum depth for each ray after which it will be stopped. 0=no limit. */
    double m_maxDepth{0};
    /** If true Split by materials instead of regions */
    bool m_splitByMaterials;
    /** Materials ignored when scanning */
    std::set<std::string> m_ignoredMaterials;
    /** Current depth of the current ray */
    double m_curDepth{0};
    /** The first ray does not record any material but just checks for the
     * maximum useful depth to not get a plot which contains the PXD at the
     * front and then continues for 500 more cm without any content. This
     * variable stores the maximum depth seen by this ray in non-ignored
     * matierals */
    double m_scanDepth{0};
    /** Current Ray number: 0 = scan for maximum depth, 1..N = record materials */
    int m_curRay{ -1};
    /** Map holding pointers to all created histograms */
    std::map<std::string, TH1D*> m_regions;
  };

  /**
   * The MaterialScan module.
   *
   * This Module is intended to scan the material budget of the geometry.
   * Currently, there are two different kinds of scans available: Spherical and Planar scan.
   *
   * - Spherical scan will shoot rays from the origin of the detector and scan
   *   along the polar and azimuth angle.
   * - Planar scan will shoot rays perpendicular to a given plane.
   */
  class MaterialScanModule : public Module {

  public:

    /* The constructor of the module.
     * Sets the description and the parameters of the module.
     */
    MaterialScanModule();

    /** Initialize the output file and check the parameters. */
    void initialize();

    /** Do the actual scanning */
    void beginRun();

    /** Save the output */
    void terminate();
  private:
    /** Return a vector along the axis with the given name
     * @param name Name of the axis */
    G4ThreeVector getAxis(char name);

    /** Pointer to the ROOT file for the histograms */
    TFile* m_rootFile;
    /** Filename for the ROOT output */
    std::string m_filename;
    /** Wether or not to do a spherical scan */
    bool m_doSpherical;
    /** Wether or not to do a planar scan */
    bool m_doPlanar;
    /** Name of the plane to use for scanning */
    std::string m_planeName;
    /** Scan parameters for the spherical scan */
    MaterialScan2D::ScanParams m_spherical;
    /** Scan parameters for the planar scan */
    MaterialScan2D::ScanParams m_planar;
    /** Custom plane definition if m_planName is "custom" */
    std::vector<double> m_customPlane;       /** Custom plane parameter  */
    std::vector<double> m_sphericalOrigin;   /** original position in spherical coordinate  */
    /** Perform the spherical scan uniform in cos(theta) instead of theta */
    bool m_doCosTheta;

    bool m_doRay;
    std::vector<double> m_rayOrigin{0, 0, 0};
    double m_rayTheta{0};
    double m_rayPhi{0};
    boost::optional<std::vector<double>> m_rayDirection;
    double m_rayOpening{0};
    unsigned int m_rayCount{0};
    double m_raySampleDepth{0.1};
    double m_rayMaxDepth{1000};
    bool m_raySplitByMaterials{false};
    std::vector<std::string> m_rayIgnoredMaterials;
  };
}

#endif /* MATERIALSCANMODULE_H_ */
