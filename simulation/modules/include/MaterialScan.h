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

#include <G4UserSteppingAction.hh>
#include <G4ThreeVector.hh>

#include <TFile.h>
class TH2D;

namespace Belle2 {

  /** Base class to create a Material Scan of the detector geometry.
   * This class is used to record the Material budget encountered by a particle
   * and provide origin and directions of all particles used for the scan.
   */
  class MaterialScan: public G4UserSteppingAction {
  public:
    /**Helper struct to Store Parameters of a Scan. */
    struct ScanParams {
      /** Default Constructor */
      ScanParams(): nU(0), nV(0), minU(0), maxU(0), minV(0), maxV(0), maxDepth(-1) {}
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
    };

    /** Constructor
     * @param rootFile Pointer to the ROOTFile where the histograms will be stored.
     * @param name Prefix to preprend to all Histograms
     * @param axisLabel Labels for the histogram axes, separated by semicolon
     * @param params Parameters for the scan
     */
    MaterialScan(TFile* rootFile, const std::string& name, const std::string& axisLabel, ScanParams params);

    /** virtual destructor */
    virtual ~MaterialScan() {};

    /** Get the origin and direction for the next scan particle.
     * @param[out] origin Origin of the next scan particle
     * @param[out[ direction Direction of the next scan particle
     * @return false if the scan is finished
     */
    bool createNext(G4ThreeVector& origin, G4ThreeVector& direction);

    /** Return the name of the scan */
    std::string getName() const { return m_name; }
    /** Return the number of rays necessary to perform the scan */
    int getNRays() const { return m_params.nU * m_params.nV; }

    /** Record the material budget for each step of the particles */
    void UserSteppingAction(const G4Step* step);
  protected:
    /** Get the origin and direction for the next scan particle.
     * To be overridden by descendents
     * @param[out] origin Origin of the next scan particle
     * @param[out[ direction Direction of the next scan particle
     */
    virtual void getRay(G4ThreeVector& origin, G4ThreeVector& direction) = 0;

    /** Fill the recorded material budget into the corresponding histogram
     * @param name Name of the histogram
     * @param value Value to store
     */
    void fillValue(const std::string& name, double value);

    /** Pointer to the root file for the histograms */
    TFile* m_rootFile;
    /** Name of the scan, will be prefixed to all histogram names */
    std::string m_name;
    /** Labels for the coordinate axes */
    std::string m_axisLabel;
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
  class MaterialScanSpherical: public MaterialScan {
  public:
    /** Create a Spherical Scan object with the given parameters
     * @param rootFile pointer to the ROOT File containing the histograms
     * @param origin Origin for the spherical scan
     * @param params Parameters of the scan
     */
    MaterialScanSpherical(TFile* rootFile, const G4ThreeVector& origin, ScanParams params):
      MaterialScan(rootFile, "Spherical", "#theta [deg];#phi [deg]", params), m_origin(origin) {}
  protected:
    /** Create a ray with the current parameter values according to a spherical distribution */
    void getRay(G4ThreeVector& origin, G4ThreeVector& direction);

    /** Origin for the spherical scan */
    G4ThreeVector m_origin;
  };

  /** Specific implementaion of MaterialScan to scan parallel to a given plane.
   *
   * Particles will be created in a grid on the given plane and will be traced
   * perpendicluar to that plane The plane is defined by one origin and the
   * directions of the grid coordinates u and v. The flight direction is
   * determined by the cross product between the u and v axis.
   */
  class MaterialScanPlanar: public MaterialScan {
  public:
    /** Create a Planar Scan object with the given parameters
     * @param rootFile pointer to the ROOT File containing the histograms
     * @param origin Origin of the plane to shoot rays from
     * @param dirU Direction of the u coordinate
     * @param dirV Direction of the v coordinate
     * @param params Parameters of the scan
     */
    MaterialScanPlanar(TFile* rootFile, const G4ThreeVector& origin, const G4ThreeVector& dirU, const G4ThreeVector& dirV, ScanParams params):
      MaterialScan(rootFile, "Planar", "u [cm];v [cm]", params), m_origin(origin), m_dirU(dirU.unit()), m_dirV(dirV.unit()), m_dirW(m_dirU.cross(m_dirV)) {
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
    MaterialScan::ScanParams m_spherical;
    /** Scan parameters for the planar scan */
    MaterialScan::ScanParams m_planar;
    /** Custom plane definition if m_planName is "custom" */
    std::vector<double> m_customPlane;
    std::vector<double> m_sphericalOrigin;
  };
}

#endif /* MATERIALSCANMODULE_H_ */
