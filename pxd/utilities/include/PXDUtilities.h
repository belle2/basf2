/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <genfit/MeasuredStateOnPlane.h>
#include <TVector3.h>
//#include <limits>
#include <math.h>

namespace Belle2 {

  namespace PXD {

    /** Const and Const expressions
     * Only valid when g_mol is the default unit.
     */
    const int Z_Si = 14; /**< Atomic number of silicon */
    const double A_Si = 28.085; /**< Atomic mass of silicon in g mol^-1 */
    const double rho_Si = 2.3290 * Unit::g_cm3; /**< Silicon density in g cm^-3 */

    /** xi = (K/2)*(Z/A)*z*z*(rho*L)/beta2 in MeV
     * @param Z Atomic number of absorber
     * @param A Atomic mass of absorber in g*mol^{-1}
     * @param rho Density of the absorber in g*cm^{-3}
     * @param z Charge number of incident particle
     * @return xi*beta^2/L in MeV/cm where L is track length
     */
    inline double xiBeta2_L(const int Z = Z_Si,
                            const double A = A_Si,
                            const double rho = rho_Si,
                            const int z = 1)
    {
      const double K = 0.307075 * Unit::MeV * pow(Unit::cm, 2);
      return K / 2 * Z / A * z * z * rho;
    }

    /** hbarWp = sqrt(rho*Z/A)*28.816 in eV
     * @param Z Atomic number of absorber
     * @param A Atomic mass of absorber in g*mol^{-1}
     * @param rho Density of the absorber in g*cm^{-3}
     * @return plasma energy
     */
    inline double hbarWp(const int Z = Z_Si,
                         const double A = A_Si,
                         const double rho = rho_Si)
    {
      return std::sqrt(rho * Z / A) * 28.816 * Unit::eV;
    }

    /** helper function to estimate the most probable energy loss for a given track length.
     * @param mom Magnitude of the momentum
     * @param length Track path length
     * @param mass Mass of the incident particle, using e- as default
     * @return the most probable energy
     */
    inline double getDeltaP(const double mom, const double length, const double mass = Const::electronMass)
    {
      double betaGamma = mom / mass;
      if (betaGamma <= 100) return 0.0; // requirement of the formula.
      double beta2 = 1. / (1. + 1. / betaGamma / betaGamma);
      double xi = xiBeta2_L() * length / beta2;
      return xi * log(2 * mass * xi / pow(hbarWp(), 2) + 0.2);
    }

    /** Helper function to get DHE id like module id from VxdID */
    inline unsigned short getPXDModuleID(const VxdID& sensorID)
    {
      return sensorID.getLayerNumber() * 1000 +
             sensorID.getLadderNumber() * 10 +
             sensorID.getSensorNumber();
    }

    /** Helper function to get VxdID from DHE id like module iid */
    inline VxdID getVxdIDFromPXDModuleID(const unsigned short& id)
    {
      return VxdID(id / 1000, (id % 1000) / 10, id % 10);
    }

    /** Typedef TrackState (genfit::MeasuredStateOnPlane) */
    typedef genfit::MeasuredStateOnPlane TrackState;

    /** Helper function to get a track state on a module
     * @param pxdSensorInfo of the PXD module intersecting with the track.
     * @param recoTrack the recoTrack to be extrapolated.
     * @param lambda the extrapolation length from track POCA.
     * @return the shared pointer of the intersection track state on the module.
     */
    std::shared_ptr<TrackState> getTrackStateOnModule(const VXD::SensorInfoBase& pxdSensorInfo,
                                                      RecoTrack& recoTrack, double lambda = 0.0);

    /** Helper function to check if a pixel is close to the border
     * @param u uID of the pixel of interest
     * @param v vID of the pixel of interest
     * @param checkDistance the distance along u/v
     * @return true if the pixel is close to border
     */
    bool isCloseToBorder(int u, int v, int checkDistance);
    /** Helper function to check if a defective (hot/dead) pixel is close
     * @param u uID of the pixel of interest
     * @param v vID of the pixel of interest
     * @param checkDistance the distance along u/v
     * @param moduleID VxdID of the module
     * @return true if a defective pixel is found in matrix
     */
    bool isDefectivePixelClose(int u, int v, int checkDistance, const VxdID& moduleID);

    /** Helper function to check if one of the end pixels are at the edge of the sensor
     * @param id VxdID of the sensor
     * @param umin minimum uID within the cluster
     * @param umax maximum uID within the cluster
     * @return true if one of the end pixels are at the edge of the sensor
     */
    inline bool isClusterAtUEdge(VxdID id, unsigned int umin, unsigned int umax)
    {
      unsigned int uedgemax = Belle2::VXD::GeoCache::getInstance().get(id).getUCells();
      return (umin == 0 || umax == (uedgemax - 1));
    }
    /** Helper function to check if one of the end pixels are at the edge of the sensor
     * @param id VxdID of the sensor
     * @param vmin minimum vID within the cluster
     * @param vmax maximum vID within the cluster
     * @return true if one of the end pixels are at the edge of the sensor
     */
    inline bool isClusterAtVEdge(VxdID id, unsigned int vmin, unsigned int vmax)
    {
      unsigned int vedgemax = Belle2::VXD::GeoCache::getInstance().get(id).getVCells();
      return ((id.getSensorNumber() == 1 && vmax == (vedgemax - 1))
              || (id.getSensorNumber() == 2 && vmin == 0));
    }
    /** Helper function to check if one of the end pixels are at the ladder joint
     * @param id VxdID of the sensor
     * @param vmin minimum vID within the cluster
     * @param vmax maximum vID within the cluster
     * @return true if one of the end pixels are at the ladder joint
     */
    inline bool isClusterAtLadderJoint(VxdID id, unsigned int vmin, unsigned int vmax)
    {
      unsigned int vedgemax = Belle2::VXD::GeoCache::getInstance().get(id).getVCells();
      return ((id.getSensorNumber() == 2 && vmax == (vedgemax - 1))
              || (id.getSensorNumber() == 1 && vmin == 0));
    }

    /** Function to return a bin number for equal sized binning in U
     * @param id VxdID of the sensor
     * @param uid uID of the pixel
     * @param vid vID of the pixel
     * @param nBinsU number of bins in U within a sensor
     */
    inline unsigned short getBinU(VxdID id, unsigned int uid, unsigned int vid, unsigned short nBinsU)
    {
      unsigned int drainsPerBin = 4 * Belle2::VXD::GeoCache::getInstance().get(id).getUCells() / nBinsU;
      return (uid * 4 + vid % 4) / drainsPerBin;
    }
    /** Function to return a bin number for equal sized binning in V
     * @param id VxdID of the sensor
     * @param vid vID of the pixel
     * @param nBinsV number of bins in V within a sensor
     */
    inline unsigned short getBinV(VxdID id, unsigned int vid, unsigned short nBinsV)
    {
      unsigned int rowsPerBin = Belle2::VXD::GeoCache::getInstance().get(id).getVCells() / nBinsV;
      return vid / rowsPerBin;
    }

  } // end namespace PXD
} // end namespace Belle2
