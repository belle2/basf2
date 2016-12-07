/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Hiroyuki Nakayama                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <geometry/bfieldmap/BFieldComponentRadial.h>
#include <geometry/bfieldmap/BFieldComponentBeamline.h>

#include <framework/utilities/FileSystem.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>

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
  m_mapBuffer = new BFieldPoint*[m_mapSize[0]];
  for (int i = 0; i < m_mapSize[0]; ++i)
    m_mapBuffer[i] = new BFieldPoint[m_mapSize[1]];

  double r, z, Br, Bz;
  for (int i = 0; i < m_mapSize[0]; ++i) {
    for (int j = 0; j < m_mapSize[1]; j++) {
      fieldMapFile >> r >> z >> Br >> Bz;

      //Since the accelerator group defines zero as the Belle center, move the
      //magnetic field to the IP.
      z += m_mapOffset;

      //Store the values
      m_mapBuffer[i][j].r = Br;
      m_mapBuffer[i][j].z = Bz;
    }
  }

  B2DEBUG(10, "... loaded " << m_mapSize[0] << "x" << m_mapSize[1] << " (r,z) elements.");
}


B2Vector3D BFieldComponentRadial::calculate(const B2Vector3D& point) const
{
  // If both 'Radial' and 'Beamline' components are defined in xml file,
  // '3d' component returns zero field where 'Beamline' component is defined.
  // If no 'Beamline' component is defined in xml file, the following function will never be called.
  if (BFieldComponentBeamline::Instance().isInRange(point)) {
    return B2Vector3D(0.0, 0.0, 0.0);
  }

  //Get the r and z component
  double r = point.Perp();
  double z = point.Z();

  //Check if the point lies inside the magnetic field boundaries
  if ((r < m_mapRegionR[0]) || (r > m_mapRegionR[1]) ||
      (z < m_mapRegionZ[0]) || (z > m_mapRegionZ[1])) {
    return B2Vector3D(0.0, 0.0, 0.0);
  }

  //Calculate the lower index of the point in the grid
  int ir = static_cast<int>(floor((r - m_mapRegionR[0]) / m_gridPitchR));
  int iz = static_cast<int>(floor((z - m_mapRegionZ[0]) / m_gridPitchZ));

  //Check if the index values are within the range
  if (((ir + 1) >= m_mapSize[0]) || ((iz + 1) >= m_mapSize[1])) {
    B2ERROR("The index values for the radial magnetic field map are out of bounds !");
    return B2Vector3D(0.0, 0.0, 0.0);
  }

  //Calculate the distance to the lower grid point
  double dr = r - floor(r / m_gridPitchR) * m_gridPitchR;
  double dz = z - floor(z / m_gridPitchZ) * m_gridPitchZ;

  // special treatment into the EKLM region
  // iron gap
  double dz_eklm = fabs(z - m_mapOffset) - (m_endyokeZMin - m_gapHeight);

  if (r > m_slotRMin && dz_eklm > 0 && dz != 0. && dz != m_gridPitchZ) {
    double dLayer(m_gapHeight + m_ironPlateThickness);
    int layer = static_cast<int>(floor(dz_eklm / dLayer));
    if (layer <= 14) {
      double ddz = dz_eklm - dLayer * layer;

      if (r - m_slotRMin < m_gridPitchR && ddz < m_gapHeight) dr = m_gridPitchR;

      if (ddz < m_gridPitchZ || (ddz >= m_gapHeight && ddz - m_gapHeight < m_gridPitchZ)) {
        if (z > 0)
          dz = m_gridPitchZ;
        else
          dz = 0.;
      } else if (dLayer - ddz < m_gridPitchZ || (ddz < m_gapHeight && m_gapHeight - ddz < m_gridPitchZ)) {
        if (z < 0)
          dz = m_gridPitchZ;
        else
          dz = 0.;
      }
    }
  }


  //Calculate the linear approx. of the magnetic field vector
  double Br1 = m_mapBuffer[ir][iz].r;
  double Br2 = m_mapBuffer[ir][iz + 1].r;
  double Br3 = m_mapBuffer[ir + 1][iz].r;
  double Br4 = m_mapBuffer[ir + 1][iz + 1].r;
  double Br = ((Br1 * (m_gridPitchZ - dz) + Br2 * dz) * (m_gridPitchR - dr) + (Br3 * (m_gridPitchZ - dz) + Br4 * dz) * dr) /
              m_gridPitchZ / m_gridPitchR;

  double Bz1 = m_mapBuffer[ir][iz].z;
  double Bz2 = m_mapBuffer[ir][iz + 1].z;
  double Bz3 = m_mapBuffer[ir + 1][iz].z;
  double Bz4 = m_mapBuffer[ir + 1][iz + 1].z;
  double Bz = ((Bz1 * (m_gridPitchZ - dz) + Bz2 * dz) * (m_gridPitchR - dr) + (Bz3 * (m_gridPitchZ - dz) + Bz4 * dz) * dr) /
              m_gridPitchZ / m_gridPitchR;;

  double Bx = (r > 0.0) ? Br * point.X() / r : 0.0;
  double By = (r > 0.0) ? Br * point.Y() / r : 0.0;

  //Near-axis approximation
  if (false) {
    //if (true) {
    Br = - (m_mapBuffer[0][iz + 1].z - m_mapBuffer[0][iz].z) / m_gridPitchZ * r / 2;
    Bz = (m_mapBuffer[0][iz].z * (m_gridPitchZ - dz) + m_mapBuffer[0][iz + 1].z * dz) / m_gridPitchZ;
    Bx = (r > 0.0) ? Br * point.X() / r : 0.0;
    By = (r > 0.0) ? Br * point.Y() / r : 0.0;
  }

  //Treatment for unrealistic QC2 iron shape used for 2D-map calculation
  if (false) {
    if (((250 < z) && (z < 320)) or ((-220 < z) && (z < -160))) {
      double angle_crossing = 0.0830;

      double angle_HER = - angle_crossing / 2.;
      B2Vector3D pHER(point.X(), point.Y(), point.Z()); pHER.RotateY(angle_HER); pHER.RotateX(M_PI);
      double rHER = pHER.Perp();

      double angle_LER =  angle_crossing / 2.;
      B2Vector3D pLER(point.X(), point.Y(), point.Z()); pLER.RotateY(angle_LER); pLER.RotateX(M_PI);
      double rLER = pLER.Perp();

      Bz = (m_mapBuffer[0][iz].z * (m_gridPitchZ - dz) + m_mapBuffer[0][iz + 1].z * dz) / m_gridPitchZ;
      if (point.X()*point.Z() > 0) {
        Br = - (m_mapBuffer[0][iz + 1].z - m_mapBuffer[0][iz].z) / m_gridPitchZ * rHER / 2;
        Bx = (rHER > 0.0) ? Br * pHER.X() / rHER : 0.0;
        By = (rHER > 0.0) ? Br * pHER.Y() / rHER : 0.0;
      } else {
        Br = - (m_mapBuffer[0][iz + 1].z - m_mapBuffer[0][iz].z) / m_gridPitchZ * rLER / 2;
        Bx = (rLER > 0.0) ? Br * pLER.X() / rLER : 0.0;
        By = (rLER > 0.0) ? Br * pLER.Y() / rLER : 0.0;
      }
    }
  }

  //B2DEBUG(20, "B Radial field is calculated: z= " << z/Unit::m <<"[m] By= "<< By <<"[Tesla].")

  return B2Vector3D(Bx, By, Bz);
}

void BFieldComponentRadial::terminate()
{
  B2DEBUG(10, "De-allocating the memory for the radial magnetic field map loaded from the file '" << m_mapFilename << "'");

  //De-Allocate memory to prevent memory leak
  for (int i = 0; i < m_mapSize[0]; ++i)
    delete [] m_mapBuffer[i];
  delete [] m_mapBuffer;
}
