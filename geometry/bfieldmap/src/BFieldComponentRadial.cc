/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <geometry/bfieldmap/BFieldComponentRadial.h>
#include <geometry/bfieldmap/BFieldComponentBeamline.h>

#include <framework/utilities/FileSystem.h>
#include <framework/logging/Logger.h>

#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/filter/gzip.hpp>

#include <cmath>

using namespace std;
using namespace Belle2;
namespace io = boost::iostreams;

void BFieldComponentRadial::initialize()
{
  if (m_mapFilename.empty()) {
    B2ERROR("The filename for the radial magnetic field component is empty !");
    return;
  }

  string fullPath = FileSystem::findFile("/data/" + m_mapFilename);

  if (!FileSystem::fileExists(fullPath)) {
    B2ERROR("The radial magnetic field map file '" << m_mapFilename << "' could not be found !");
    return;
  }

  //Load the map file
  io::filtering_istream fieldMapFile;
  fieldMapFile.push(io::gzip_decompressor());
  fieldMapFile.push(io::file_source(fullPath));

  //Create the magnetic field map [r,z] and read the data from the file
  B2DEBUG(10, "Loading the radial magnetic field from file '" << m_mapFilename << "' in to the memory...");
  m_mapBuffer.clear();
  m_mapBuffer.reserve(m_mapSize[0]*m_mapSize[1]);

  double r, z, Br, Bz;
  for (int i = 0; i < m_mapSize[0]; ++i) {
    for (int j = 0; j < m_mapSize[1]; j++) {
      fieldMapFile >> r >> z >> Br >> Bz;
      //Store the values
      m_mapBuffer.push_back({Br, Bz});
    }
  }

  m_igridPitchR = 1 / m_gridPitchR;
  m_igridPitchZ = 1 / m_gridPitchZ;
  m_Layer = m_gapHeight + m_ironPlateThickness;
  m_iLayer = 1 / m_Layer;

  B2DEBUG(10, "... loaded " << m_mapSize[0] << "x" << m_mapSize[1] << " (r,z) elements.");
}

namespace Belle2 {
  /** multiply a radial bfield point by a real number */
  inline BFieldComponentRadial::BFieldPoint operator*(const BFieldComponentRadial::BFieldPoint& v, double a)
  {
    return {v.r * a, v.z * a};
  }

  /** Add two radial bfield points together */
  inline BFieldComponentRadial::BFieldPoint operator+(const BFieldComponentRadial::BFieldPoint& u,
                                                      const BFieldComponentRadial::BFieldPoint& v)
  {
    return {u.r + v.r, u.z + v.z};
  }
}

B2Vector3D BFieldComponentRadial::calculate(const B2Vector3D& point) const
{
  B2Vector3D B;
  // If both 'Radial' and 'Beamline' components are defined in xml file,
  // '3d' component returns zero field where 'Beamline' component is defined.
  // If no 'Beamline' component is defined in xml file, the following function will never be called.
  if (BFieldComponentBeamline::Instance().isInRange(point)) return B;

  //Get the z component
  double z = point.Z();
  //Check if the point lies inside the magnetic field boundaries
  if ((z < m_mapRegionZ[0]) || (z > m_mapRegionZ[1])) return B;

  //Get the r component
  double r2 = point.Perp2();
  //Check if the point lies inside the magnetic field boundaries
  if ((r2 < m_mapRegionR[0]*m_mapRegionR[0]) || (r2 > m_mapRegionR[1]*m_mapRegionR[1])) return B;

  double r = sqrt(r2);
  //Calculate the distance to the lower grid point
  double wr = (r - m_mapRegionR[0]) * m_igridPitchR;
  double wz = (z - m_mapRegionZ[0]) * m_igridPitchZ;
  //Calculate the lower index of the point in the grid
  int ir = static_cast<int>(wr);
  int iz = static_cast<int>(wz);

  double za = z - m_mapOffset;
  double dz_eklm = abs(za) - (m_endyokeZMin - m_gapHeight);
  if (dz_eklm > 0 && r > m_slotRMin) {
    double wl = dz_eklm * m_iLayer;
    int il = static_cast<int>(wl);
    if (il <= 16) {
      double L = m_Layer * il;
      double l = dz_eklm - L;
      int ingap = l < m_gapHeight;
      ir += ingap & (r < m_slotRMin + m_gridPitchR);

      double zlg = L + m_endyokeZMin, zl0 = zlg - m_gapHeight, zl1 = zlg + m_ironPlateThickness;
      if (za < 0) {
        zl0 = -zl0;
        zlg = -zlg;
        zl1 = -zl1;
      }
      double zoff = m_mapOffset - m_mapRegionZ[0];
      int izl0 = static_cast<int>((zl0 + zoff) * m_igridPitchZ);
      int izlg = static_cast<int>((zlg + zoff) * m_igridPitchZ);
      int izl1 = static_cast<int>((zl1 + zoff) * m_igridPitchZ);
      int ig = izlg == iz;
      int idz = (izl0 == iz) - (izl1 == iz) + (ingap ? -ig : ig);
      iz += (za > 0) ? idz : -idz;
    }
  }

  //Bring the index values within the range
  ir = min(m_mapSize[0] - 2, ir);
  iz = min(m_mapSize[1] - 2, iz);

  wr -= ir;
  wz -= iz;

  double wz0 = 1 - wz, wr0 = 1 - wr;
  //Calculate the linear approx. of the magnetic field vector
  const unsigned int strideZ = m_mapSize[1];
  const unsigned int i00 = ir * strideZ + iz, i01 = i00 + 1, i10 = i00 + strideZ, i11 = i01 + strideZ;
  const BFieldPoint* H = m_mapBuffer.data();
  double w00 = wz0 * wr0, w01 = wz * wr0, w10 = wz0 * wr, w11 = wz * wr;
  BFieldPoint Brz = H[i00] * w00 + H[i01] * w01 + H[i10] * w10 + H[i11] * w11;

  if (r > 0.0) {
    double norm = Brz.r / r;
    B.SetXYZ(point.X()*norm, point.Y()*norm, Brz.z);
  } else {
    B.SetZ(Brz.z);
  }

  return B;
}

void BFieldComponentRadial::terminate()
{
}
