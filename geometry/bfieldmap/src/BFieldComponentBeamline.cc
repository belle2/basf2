/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Kazutaka. Sumisawa                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

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

double BFieldComponentBeamline::s_mapRegionR[2] = {0., 0.};
double BFieldComponentBeamline::s_sinBeamCrossAngle = 0.;
double BFieldComponentBeamline::s_cosBeamCrossAngle = 1.;

bool BFieldComponentBeamline::isInRange(const TVector3& point)
{
  if (s_mapRegionR[1] <= 0.) return false;

  double y2 = point.y() * point.y();
  double c = s_cosBeamCrossAngle * point.x(), s = s_sinBeamCrossAngle * point.z();

  double rp = pow(c - s, 2) + y2;
  double rn = pow(c + s, 2) + y2;

  const double R02 = pow(s_mapRegionR[0], 2);
  const double R12 = pow(s_mapRegionR[1], 2);

  if (rp >= R02 && rp <= R12)
    return true;
  else if (rn >= R02 && rn <= R12)
    return true;
  else
    return false;
}

void BFieldComponentBeamline::initialize_beamline(int isher)
{
  BFieldPoint** *mapBuffer;
  InterpolationPoint** *interBuffer;
  std::string mapFilename, interFilename;

  if (isher == 1) {
    mapBuffer = &m_mapBuffer_her;
    interBuffer = &m_interBuffer_her;
    mapFilename = m_mapFilename_her;
    interFilename = m_interFilename_her;
  } else {
    mapBuffer = &m_mapBuffer_ler;
    interBuffer = &m_interBuffer_ler;
    mapFilename = m_mapFilename_ler;
    interFilename = m_interFilename_ler;
  }

  if (mapFilename.empty()) {
    B2ERROR("The filename for the beamline magnetic field component is empty !");
    return;
  }
  if (mapFilename.empty()) {
    B2ERROR("The filename for the beamline interpolation component is empty !");
    return;
  }


  string fullPath = FileSystem::findFile("/data/" + mapFilename);

  if (!FileSystem::fileExists(fullPath)) {
    B2ERROR("The beamline magnetic field map file '" << mapFilename << "' could not be found !");
    return;
  }

  //Load the map file
  io::filtering_istream fieldMapFile;
  fieldMapFile.push(io::gzip_decompressor());
  fieldMapFile.push(io::file_source(fullPath));

  //Create the magnetic field map [r,z] and read the data from the file
  B2DEBUG(10, "Loading the beamline magnetic field from file '" << mapFilename << "' in to the memory...");

  fieldMapFile >> m_mapSizeRPhi[isher] >> m_jointR >> m_nGridR >> m_nGridPhi;
  fieldMapFile >> m_mapSizeZ >> m_jointZ >> m_gridPitchZ[0] >> m_gridPitchZ[1];

  m_offsetGridRPhi[isher] = m_mapSizeRPhi[isher] - (m_nGridR + 1) * (m_nGridPhi + 1);
  m_offsetGridZ = static_cast<int>(m_mapSizeZ * 0.5);

  *mapBuffer = new BFieldPoint*[m_mapSizeZ];
  for (int i = 0; i < m_mapSizeZ; ++i)
    (*mapBuffer)[i] = new BFieldPoint[m_mapSizeRPhi[isher]];

  double r, phi, z, Br, Bphi, Bz;
  for (int i = 0; i < m_mapSizeZ; ++i) {
    for (int j = 0; j < m_mapSizeRPhi[isher]; j++) {
      fieldMapFile >> r >> phi >> z >> Br >> Bphi >> Bz;

      //Bphi = 0 at the points with phi=0 or phi=180
      if (phi == 0. || phi == 180.) Bphi = 0.;

      //Store the values
      (*mapBuffer)[i][j].r   = r * Unit::m;
      (*mapBuffer)[i][j].phi = phi * M_PI / 180;
      //(*mapBuffer)[i][j].z   = z * Unit::m;
      (*mapBuffer)[i][j].Br   = - Br;   // flip the sign because I-parity in Yamaoka-san's data is wrong
      (*mapBuffer)[i][j].Bphi = - Bphi; // flip the sign because I-parity in Yamaoka-san's data is wrong
      (*mapBuffer)[i][j].Bz   = - Bz;   // flip the sign because I-parity in Yamaoka-san's data is wrong
    }
  }

  //map for interpolation
  fullPath = FileSystem::findFile("/data/" + interFilename);

  if (!FileSystem::fileExists(fullPath)) {
    B2ERROR("The beamline interpolation map file '" << interFilename << "' could not be found !");
    return;
  }

  //Load the map file
  io::filtering_istream interMapFile;
  interMapFile.push(io::file_source(fullPath));

  int nbinx, nbiny;
  interMapFile >> m_interMaxRadius >> m_interGridSize >> m_interSizeX >> nbiny;

  nbinx = m_interSizeX;
  *interBuffer = new InterpolationPoint*[nbinx];
  std::vector<int> vnbiny(nbinx, 0);
  for (int j = 0; j < nbinx; ++j) {
    const double x(m_interGridSize * (j + 0.5) - m_interMaxRadius);
    double y2 = m_interMaxRadius * m_interMaxRadius - x * x;
    if (y2 < 0.) y2 = 0.;
    vnbiny[j] = static_cast<int>(ceil(sqrt(y2) / m_interGridSize));
    (*interBuffer)[j] = new InterpolationPoint[vnbiny[j]];
  }

  while (!interMapFile.eof()) {
    int i1, i2;
    InterpolationPoint p;
    p.p.resize(4);
    interMapFile >> i1 >> i2 >> p.p[0] >> p.p[1] >> p.p[2] >> p.p[3];

    if (!(i1 < nbinx && i2 < vnbiny[i1]))
      B2ERROR("Something wrong '" << interFilename << "': grid(" << i1 << ", " << i2 << ") " << "but maximum grid for y = " <<
              vnbiny[i1]);
    (*interBuffer)[i1][i2] = p;
  }

  B2DEBUG(10, "... loaded " << mapFilename << "and" << interFilename << "files");
}


TVector3 BFieldComponentBeamline::calculate_beamline(const TVector3& point0, int isher) const
{
  BFieldPoint** mapBuffer;
  InterpolationPoint** interBuffer;
  int offsetRPhi;

  //added by nakayama to avoid segV
  if (TMath::Abs(point0.z()) > 399.) return TVector3(0., 0., 0.);

  //from GEANT4 coordinate to ANSYS coordinate
  //fabs(y) is used because field data exist only in y>0
  const double
  tx = -point0.x(),
  ty = std::abs(point0.y()),
  tz = -point0.z();

  double sinBeamCrossAngle = (isher == 1) ? s_sinBeamCrossAngle : -s_sinBeamCrossAngle;
  TVector3 point(tx * s_cosBeamCrossAngle - tz * sinBeamCrossAngle, ty, tz * s_cosBeamCrossAngle + tx * sinBeamCrossAngle);
  if (isher == 1) {
    mapBuffer = m_mapBuffer_her;
    interBuffer = m_interBuffer_her;
  } else {
    mapBuffer = m_mapBuffer_ler;
    interBuffer = m_interBuffer_ler;
  }
  offsetRPhi = m_offsetGridRPhi[isher];

  //Get the r and z component
  double r = point.Perp();
  double phi = point.Phi();
  double z = point.z();
  double absz = fabs(z);

  //Check if the point lies inside the magnetic field boundaries
  if ((r < s_mapRegionR[0]) || (r > s_mapRegionR[1]) ||
      (z < m_mapRegionZ[0]) || (z > m_mapRegionZ[1])) {
    return TVector3(0.0, 0.0, 0.0);
  }

  double Bx, By, Bz;

  int iz;
  double dz;
  if (absz < m_jointZ) {
    iz = static_cast<int>(absz / m_gridPitchZ[0]);
    dz = (absz - m_gridPitchZ[0] * iz);
    dz /= m_gridPitchZ[0];
  } else {
    iz = static_cast<int>((absz - m_jointZ) / m_gridPitchZ[1] + m_jointZ / m_gridPitchZ[0]);
    dz = (absz - m_jointZ) - (iz - static_cast<int>(m_jointZ / m_gridPitchZ[0])) * m_gridPitchZ[1];
    dz /= m_gridPitchZ[1];
  }
  if (z < 0) {
    iz = m_offsetGridZ - 1 - iz;
    dz *= -1.;
  } else {
    iz += m_offsetGridZ;
  }

  if (r < m_jointR) {
    //Calculate the lower index of the point in the grid
    int ix = static_cast<int>((point.x() + m_interMaxRadius) / m_interGridSize);
    int iy = static_cast<int>((fabs(point.y())) / m_interGridSize);

    const InterpolationPoint& ib = interBuffer[ix][iy];

    double phi1 = mapBuffer[iz][ib.p[0]].phi;
    double phi2 = mapBuffer[iz][ib.p[1]].phi;
    double r1 = mapBuffer[iz][ib.p[0]].r;
    double r2 = mapBuffer[iz][ib.p[1]].r;

    if (r1 * r2 != 0. && (fabs(phi1 - phi2) < 0.02 || fabs(r1 - r2) < 0.02)) {
      //Cylindrical Polar Coordinates
      TVector3 point_r(r, phi, z);
      vector<TVector3> vp(4);
      TVectorD vBr1(4), vBphi1(4), vBz1(4), vBr2(4), vBphi2(4), vBz2(4);
      for (int i = 0; i < 4; ++i) {
        vp[i].SetX(mapBuffer[iz][ib.p[i]].r);
        vp[i].SetY(mapBuffer[iz][ib.p[i]].phi);
        vBr1[i] = mapBuffer[iz][ib.p[i]].Br;
        vBphi1[i] = mapBuffer[iz][ib.p[i]].Bphi;
        vBz1[i] = mapBuffer[iz][ib.p[i]].Bz;

        vBr2[i] = mapBuffer[iz + 1][ib.p[i]].Br;
        vBphi2[i] = mapBuffer[iz + 1][ib.p[i]].Bphi;
        vBz2[i] = mapBuffer[iz + 1][ib.p[i]].Bz;
      }

      TVectorD vphi = calculateCoefficientRectangle(point_r, vp);
      if (vphi.Sum() < 0.9) {
        // move to interpolation with triangle mesh, because rectangle shape is complecated.
        vphi = calculateCoefficientTriangle(point_r, vp);
      }

      //Calculate the linear approx. of the magnetic field vector
      const double Br   = dz * Dot(vBr1, vphi)   + (1 - dz) * Dot(vBr2, vphi)  ;
      const double Bphi = dz * Dot(vBphi1, vphi) + (1 - dz) * Dot(vBphi2, vphi);
      Bz = dz * Dot(vBz1, vphi) + (1 - dz) * Dot(vBz2, vphi);

      Bx = Br * cos(phi) - Bphi * sin(phi);
      By = Br * sin(phi) + Bphi * cos(phi);
    } else {
      //orthogonal coordinate
      vector<TVector3> vp(4);
      TVectorD vBx1(4), vBy1(4), vBz1(4), vBx2(4), vBy2(4), vBz2(4);

      for (int i = 0; i < 4; ++i) {
        double cosphi = cos(mapBuffer[iz][ib.p[i]].phi);
        double sinphi = sin(mapBuffer[iz][ib.p[i]].phi);
        vp[i].SetX(mapBuffer[iz][ib.p[i]].r * cosphi);
        vp[i].SetY(mapBuffer[iz][ib.p[i]].r * sinphi);
        vBx1[i] = mapBuffer[iz][ib.p[i]].Br * cosphi - mapBuffer[iz][ib.p[i]].Bphi * sinphi;
        vBy1[i] = mapBuffer[iz][ib.p[i]].Br * sinphi + mapBuffer[iz][ib.p[i]].Bphi * cosphi;
        vBz1[i] = mapBuffer[iz][ib.p[i]].Bz;

        vBx2[i] = mapBuffer[iz + 1][ib.p[i]].Br * cosphi - mapBuffer[iz + 1][ib.p[i]].Bphi * sinphi;
        vBy2[i] = mapBuffer[iz + 1][ib.p[i]].Br * sinphi + mapBuffer[iz + 1][ib.p[i]].Bphi * cosphi;
        vBz2[i] = mapBuffer[iz + 1][ib.p[i]].Bz;
      }

      TVectorD vphi = calculateCoefficientRectangle(point, vp);
      if (vphi.Sum() < 0.9) {
        // move to interpolation with triangle mesh, because rectangle shape is complecated.
        vphi = calculateCoefficientTriangle(point, vp);
      }

      //Calculate the linear approx. of the magnetic field vector
      Bx = dz * Dot(vBx1, vphi) + (1 - dz) * Dot(vBx2, vphi);
      By = dz * Dot(vBy1, vphi) + (1 - dz) * Dot(vBy2, vphi);
      Bz = dz * Dot(vBz1, vphi) + (1 - dz) * Dot(vBz2, vphi);
    }
  } else {
    TVector3 point_r(r, phi, z);

    int ir = static_cast<int>((r - 1.25) / (2.5 - 1.25) * m_nGridR);
    int iphi = static_cast<int>(phi / m_nGridPhi);

    if (ir == m_nGridR) --ir;
    if (iphi == m_nGridPhi) --iphi;

    vector<TVector3> vp(4);
    TVectorD vBr1(4), vBphi1(4), vBz1(4), vBr2(4), vBphi2(4), vBz2(4);
    for (int i = 0; i < 2; ++i) {
      for (int j = 0; j < 2; ++j) {
        const int irphi = (iphi + i) * (m_nGridR + 1) + ir + j + offsetRPhi;
        const int ij = i * 2 + j;
        vp[ij].SetX(mapBuffer[iz][irphi].r);
        vp[ij].SetY(mapBuffer[iz][irphi].phi);
        vBr1[ij]   = mapBuffer[iz][irphi].Br;
        vBphi1[ij] = mapBuffer[iz][irphi].Bphi;
        vBz1[ij]   = mapBuffer[iz][irphi].Bz;

        vBr2[ij]   = mapBuffer[iz + 1][irphi].Br;
        vBphi2[ij] = mapBuffer[iz + 1][irphi].Bphi;
        vBz2[ij]   = mapBuffer[iz + 1][irphi].Bz;
      }
    }
    TVectorD vphi = calculateCoefficientRectangle(point_r, vp);

    //Calculate the linear approx. of the magnetic field vector
    const double Br   = dz * Dot(vBr1, vphi)   + (1 - dz) * Dot(vBr2, vphi)  ;
    const double Bphi = dz * Dot(vBphi1, vphi) + (1 - dz) * Dot(vBphi2, vphi);
    Bz = dz * Dot(vBz1, vphi) + (1 - dz) * Dot(vBz2, vphi);

    Bx = Br * cos(phi) - Bphi * sin(phi);
    By = Br * sin(phi) + Bphi * cos(phi);
  }

  //B2DEBUG(20, "B HER3d field is calculated: z= " << z/Unit::m <<"[m] By= "<< By <<"[Tesla].")

  if (point0.y() == 0) By = 0.;

  //if y<0 fabs(y), By should be flipped
  if (point0.y() < 0) By = -By;

  //from ANSYS Coordinate to GEANT4 Coordinate
  Bx = -Bx;
  Bz = -Bz;
  return TVector3(Bx * s_cosBeamCrossAngle + Bz * sinBeamCrossAngle, By,  Bz * s_cosBeamCrossAngle - Bx * sinBeamCrossAngle);
}


TVector3 BFieldComponentBeamline::calculate(const TVector3& point) const
{

  if (!isInRange(point)) return TVector3(0.0, 0.0, 0.0);


  TVector3 B_her = calculate_her(point);
  TVector3 B_ler = calculate_ler(point);

  if (B_ler.Mag() < 1.e-10)
    return B_her;
  if (B_her.Mag() < 1.e-10)
    return B_ler;
  else
    return (B_her + B_ler) * 0.5;
}

void BFieldComponentBeamline::terminate()
{

  /*
  //================================
  //check beam line magnetic field
  //================================

  //HER
  for (double s = -400; s < 400; s += 0.1) {
    TVector3 pher(0, 0, s);
    pher.RotateY(0.0415); pher.RotateX(M_PI);
    TVector3 dx(((s < 0) ? -0.0007 : 0.0007), 0, 0);
    //TVector3 Bher = calculate(pher+dx);
    TVector3 Bher = calculate(pher);
    Bher.RotateX(-M_PI); Bher.RotateY(-0.0415);
    double SK0 = Bher.X() / (7.0e+9 / 3.0e+8 / 0.01);
    double K0 = Bher.Y() / (7.0e+9 / 3.0e+8 / 0.01);
    printf("SolCheckHER s= %f [m], K0= %10.8f ,SK0= %10.8f, Bz= %f\n", s / 100., K0, SK0, Bher.Z());
    pher.Delete(); Bher.Delete();
  }

  //LER
  for (double s = -400; s < 400; s += 0.1) {
    TVector3 pler(0, 0, s);
    pler.RotateY(-0.0415); pler.RotateX(M_PI);
    TVector3 dy(0, ((s < 0) ? 0.001 : 0.0015), 0);
    //TVector3 Bler = calculate(pler+dy);
    TVector3 Bler = calculate(pler);
    Bler.RotateX(-M_PI); Bler.RotateY(0.0415);
    double SK0 = Bler.X() / (4.0e+9 / 3.0e+8 / 0.01);
    double K0 = Bler.Y() / (4.0e+9 / 3.0e+8 / 0.01);
    printf("SolCheckLER s= %f [m], K0= %10.8f ,SK0= %10.8f, Bz= %f\n", s / 100., K0, SK0, Bler.Z());
    pler.Delete(); Bler.Delete();
  }
  */


  B2DEBUG(10, "De-allocating the memory for the beamline magnetic field map loaded from the file"
          << "'" << m_mapFilename_her << "'"
          << "'" << m_mapFilename_ler << "'"
          << "'" << m_interFilename_her << "'"
          << "'" << m_interFilename_ler << "'");

  //De-Allocate memory to prevent memory leak
  for (int i = 0; i < m_mapSizeZ; ++i) {
    delete [] m_mapBuffer_her[i];
    delete [] m_mapBuffer_ler[i];
  }
  delete [] m_mapBuffer_her;
  delete [] m_mapBuffer_ler;

  for (int i = 0; i < m_interSizeX; ++i) {
    delete [] m_interBuffer_her[i];
    delete [] m_interBuffer_ler[i];
  }
  delete [] m_interBuffer_her;
  delete [] m_interBuffer_ler;
}

TVectorD BFieldComponentBeamline::calculateCoefficientRectangle(const TVector3& x, const vector<TVector3>& vx) const
{
  TVectorD b(4);
  b(0) = 1;
  b(1) = x.X();
  b(2) = x.Y();
  b(3) = x.X() * x.Y();

  TMatrixD A(4, 4);
  for (int i = 0; i < 4; ++i) {
    A(i, 0) = 1;
    A(i, 1) = vx[i].X();
    A(i, 2) = vx[i].Y();
    A(i, 3) = vx[i].X() * vx[i].Y();
  }

  double det;
  TMatrixD ATbar = A;
  ATbar.T();
  ATbar.Invert(&det);
  TVectorD phi = b;
  phi *= ATbar;

  if (phi(0) < -0.5 || phi(1) < -0.5 || phi(2) < -0.5 || phi(3) < -0.5 ||
      phi(0) > 1.5 || phi(1) > 1.5 || phi(2) > 1.5 || phi(3) > 1.5) {
    //Bad coefficient. So return 0 values.
    double zero(0.);
    return TVectorD(4, &zero);
  }
  return phi;
}

TVectorD BFieldComponentBeamline::calculateCoefficientTriangle(const TVector3& x, const vector<TVector3>& vx) const
{
  TVectorD b(3);
  b(0) = 1;
  b(1) = x.X();
  b(2) = x.Y();

  TMatrixD A(3, 3);
  for (int i = 0; i < 3; ++i) {
    A(i, 0) = 1;
    A(i, 1) = vx[i].X();
    A(i, 2) = vx[i].Y();
  }

  double det;
  TMatrixD ATbar = A;
  ATbar.T();
  ATbar.Invert(&det);
  TVectorD phi0 = b;
  phi0 *= ATbar;

  TVectorD phi(4);
  phi(0) = phi0(0);
  phi(1) = phi0(1);
  phi(2) = phi0(2);
  phi(3) = 0.;
  return phi;
}

