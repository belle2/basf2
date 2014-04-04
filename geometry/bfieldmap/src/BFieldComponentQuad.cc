/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Hiroyuki Nakayama                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <geometry/bfieldmap/BFieldComponentQuad.h>

#include <framework/utilities/FileSystem.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>

#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/filter/gzip.hpp>

#include <cmath>

using namespace std;
using namespace Belle2;
namespace io = boost::iostreams;


BFieldComponentQuad::BFieldComponentQuad() : m_mapFilenameHER(""), m_mapFilenameLER(""), m_mapFilenameHERleak(""), m_apertFilenameHER(""), m_apertFilenameLER("")
{

}


BFieldComponentQuad::~BFieldComponentQuad()
{

}

void BFieldComponentQuad::initialize()
{
  // check if input name is not empty
  if (m_mapFilenameHER.empty()) {
    B2FATAL("The filename for the HER quadrupole magnetic field component is empty !")
    return;
  }
  if (m_mapFilenameLER.empty()) {
    B2FATAL("The filename for the LER quadrupole magnetic field component is empty !")
    return;
  }
  if (m_apertFilenameHER.empty()) {
    B2FATAL("The filename for the HER beam pipe aperture is empty !")
    return;
  }
  if (m_apertFilenameLER.empty()) {
    B2FATAL("The filename for the LER beam pipe aperture is empty !")
    return;
  }

  // check if input files exsits
  string fullPathMapHER = FileSystem::findFile("/data/" + m_mapFilenameHER);
  if (!FileSystem::fileExists(fullPathMapHER)) {
    B2FATAL("The HER quadrupole magnetic field map file '" << m_mapFilenameHER << "' could not be found !")
    return;
  }
  string fullPathMapLER = FileSystem::findFile("/data/" + m_mapFilenameLER);
  if (!FileSystem::fileExists(fullPathMapLER)) {
    B2FATAL("The LER quadrupole magnetic field map file '" << m_mapFilenameLER << "' could not be found !")
    return;
  }
  string fullPathMapHERleak = FileSystem::findFile("/data/" + m_mapFilenameHERleak);
  if (!FileSystem::fileExists(fullPathMapHERleak)) {
    B2FATAL("The HERleak quadrupole magnetic field map file '" << m_mapFilenameHERleak << "' could not be found !")
    return;
  }
  string fullPathApertHER = FileSystem::findFile("/data/" + m_apertFilenameHER);
  if (!FileSystem::fileExists(fullPathApertHER)) {
    B2FATAL("The HER aperture file '" << m_apertFilenameHER << "' could not be found !")
    return;
  }
  string fullPathApertLER = FileSystem::findFile("/data/" + m_apertFilenameLER);
  if (!FileSystem::fileExists(fullPathApertLER)) {
    B2FATAL("The LER aperture file '" << m_apertFilenameLER << "' could not be found !")
    return;
  }

  //--------------------------------------------------------------
  // Load the field map files
  //--------------------------------------------------------------

  io::filtering_istream fieldMapFileHER, fieldMapFileLER, fieldMapFileHERleak;
  fieldMapFileHER.push(io::file_source(fullPathMapHER));
  fieldMapFileLER.push(io::file_source(fullPathMapLER));
  fieldMapFileHERleak.push(io::file_source(fullPathMapHERleak));

  string name;
  double s, L, K0, K1, SK0, SK1, ROTATE, DX, DY;

  //Create the parameter map and read the data from the file
  B2DEBUG(10, "Loading the HER quadrupole magnetic field from file '" << m_mapFilenameHER << "' in to the memory...")
  m_mapBufferHER = new ParamPoint[m_mapSizeHER];
  for (int i = 0; i < m_mapSizeHER; ++i) {
    fieldMapFileHER >> name >> s >> L >> K0 >> K1 >> SK0 >> SK1 >> ROTATE >> DX >> DY;
    /* Save parametors in unit [m], not in unit [cm].*/
    m_mapBufferHER[i].s      = s;   // [m]
    m_mapBufferHER[i].L      = L;   // [m]
    m_mapBufferHER[i].K0     = K0;  // [dimensionless]
    m_mapBufferHER[i].K1     = K1;  // [1/m]
    m_mapBufferHER[i].SK0    = SK0; // [dimensionless]
    m_mapBufferHER[i].SK1    = SK1; // [1/m]
    m_mapBufferHER[i].ROTATE = ROTATE; // [degree]
    m_mapBufferHER[i].DX     = DX; // [m]
    m_mapBufferHER[i].DY     = DY; // [m]
    B2DEBUG(10, "... loaded HER SAD element " << name << " at s= " << s << "[m].")
  }
  B2DEBUG(10, "... loaded " << m_mapSizeHER << " elements.")

  B2DEBUG(10, "Loading the LER quadrupole magnetic field from file '" << m_mapFilenameLER << "' in to the memory...")
  m_mapBufferLER = new ParamPoint[m_mapSizeLER];
  for (int i = 0; i < m_mapSizeLER; ++i) {
    fieldMapFileLER >> name >> s >> L >> K0 >> K1 >> SK0 >> SK1 >> ROTATE >> DX >> DY;
    /* Save parametors in unit [m], not in unit [cm].*/
    m_mapBufferLER[i].s      = s;   // [m]
    m_mapBufferLER[i].L      = L;   // [m]
    m_mapBufferLER[i].K0     = K0;  // [dimensionless]
    m_mapBufferLER[i].K1     = K1;  // [1/m]
    m_mapBufferLER[i].SK0    = SK0; // [dimensionless]
    m_mapBufferLER[i].SK1    = SK1; // [1/m]
    m_mapBufferLER[i].ROTATE = ROTATE; // [degree]
    m_mapBufferLER[i].DX     = DX; // [m]
    m_mapBufferLER[i].DY     = DY; // [m]
    B2DEBUG(10, "... loaded LER SAD element " << name << " at s= " << s << "[m].")
  }
  B2DEBUG(10, "... loaded " << m_mapSizeLER << " elements.")

  m_mapBufferHERleak = new ParamPoint[m_mapSizeHERleak];
  for (int i = 0; i < m_mapSizeHERleak; ++i) {
    fieldMapFileHERleak >> name >> s >> L >> K0 >> K1 >> SK0 >> SK1 >> ROTATE >> DX >> DY;
    /* Save parametors in unit [m], not in unit [cm].*/
    m_mapBufferHERleak[i].s      = s;   // [m]
    m_mapBufferHERleak[i].L      = L;   // [m]
    m_mapBufferHERleak[i].K0     = K0;  // [dimensionless]
    m_mapBufferHERleak[i].K1     = K1;  // [1/m]
    m_mapBufferHERleak[i].SK0    = SK0; // [dimensionless]
    m_mapBufferHERleak[i].SK1    = SK1; // [1/m]
    m_mapBufferHERleak[i].ROTATE = ROTATE; // [degree]
    m_mapBufferHERleak[i].DX     = DX; // [m]
    m_mapBufferHERleak[i].DY     = DY; // [m]
    B2DEBUG(10, "... loaded HERleak SAD element " << name << " at s= " << s << "[m].")
  }
  B2DEBUG(10, "... loaded " << m_mapSizeHERleak << " elements.")



  //--------------------------------------------------------------
  // Load the aperture map files
  //--------------------------------------------------------------

  io::filtering_istream apertFileHER, apertFileLER;
  apertFileHER.push(io::file_source(fullPathApertHER));
  apertFileLER.push(io::file_source(fullPathApertLER));

  double r;

  //Create the parameter map and read the data from the file
  B2DEBUG(10, "Loading the HER aperture from file '" << m_apertFilenameHER << "' in to the memory...")
  m_apertBufferHER = new ApertPoint[m_apertSizeHER];
  for (int i = 0; i < m_apertSizeHER; ++i) {
    apertFileHER >> s >> r;
    /* s and r are in [mm] in ApertHER.dat. */
    /* Save parametors in unit [mm], not in basf2 default unit [cm].*/
    m_apertBufferHER[i].s   = s;
    m_apertBufferHER[i].r   = r;
    B2DEBUG(10, "... loaded HER aperture at s = " << s << "[mm], r = " << r << "[mm]")
  }
  B2DEBUG(10, "... loaded " << m_apertSizeHER << " elements.")

  B2DEBUG(10, "Loading the LER aperture from file '" << m_apertFilenameLER << "' in to the memory...")
  m_apertBufferLER = new ApertPoint[m_apertSizeLER];
  for (int i = 0; i < m_apertSizeLER; ++i) {
    apertFileLER >> s >> r;
    /* s and r are in [mm] in ApertLER.dat. */
    /* Save parametors in unit [mm], not in basf2 default unit [cm].*/
    m_apertBufferLER[i].s   = s;
    m_apertBufferLER[i].r   = r;
    B2DEBUG(10, "... loaded LER aperture at s = " << s << "[mm], r = " << r << "[mm]")
  }
  B2DEBUG(10, "... loaded " << m_apertSizeLER << " elements.")


}

double BFieldComponentQuad::getApertureHER(double s) const
{
  if (s < m_apertBufferHER[0].s) return m_apertBufferHER[0].r;
  if (s > m_apertBufferHER[m_apertSizeHER - 1].s) return m_apertBufferHER[m_apertSizeHER - 1].r;
  //H.Nakayama: this loop could be modified to binary-search
  for (int i = 0; i < m_apertSizeHER - 1; i++) {
    if ((m_apertBufferHER[i].s <= s) && (s < m_apertBufferHER[i + 1].s)) {
      double s1 = m_apertBufferHER[i].s;
      double s2 = m_apertBufferHER[i + 1].s;
      double r1 = m_apertBufferHER[i].r;
      double r2 = m_apertBufferHER[i + 1].r;
      double r = r1 + (r2 - r1) * (s - s1) / (s2 - s1);
      return r;
    }
  }
  return 0;
}

double BFieldComponentQuad::getApertureLER(double s) const
{
  if (s < m_apertBufferLER[0].s) return m_apertBufferLER[0].r;
  if (s > m_apertBufferLER[m_apertSizeLER - 1].s) return m_apertBufferLER[m_apertSizeLER - 1].r;
  //H.Nakayama: this loop could be modified to binary-search
  for (int i = 0; i < m_apertSizeLER - 1; i++) {
    if ((m_apertBufferLER[i].s <= s) && (s < m_apertBufferLER[i + 1].s)) {
      double s1 = m_apertBufferLER[i].s;
      double s2 = m_apertBufferLER[i + 1].s;
      double r1 = m_apertBufferLER[i].r;
      double r2 = m_apertBufferLER[i + 1].r;
      double r = r1 + (r2 - r1) * (s - s1) / (s2 - s1);
      return r;
    }
  }
  return 0;
}


TVector3 BFieldComponentQuad::calculate(const TVector3& point) const
{
  //assume point is given in [cm]

  //Conversion to LER/HER SAD coordinates
  //GearDir her("/Detector/SuperKEKB/HER/");
  //GearDir ler("/Detector/SuperKEKB/LER/");
  //double angle_HER = her.getDouble("angle"); //  0.0415
  //double angle_LER = ler.getDouble("angle"); // -0.0415
  double angle_HER =  0.0415;
  double angle_LER = -0.0415;

  //double p0_HER = her.getDouble("energy") / Unit::eV; // 7.0e+9 [eV]
  //double p0_LER = ler.getDouble("energy") / Unit::eV; // 4.0e+9 [eV]
  double p0_HER = 7.0e+9;
  double p0_LER = 4.0e+9;
  double c = Const::speedOfLight / (Unit::m / Unit::s);  // 3.0e+8 [m/s]

  TVector3 pHER(point.X(), point.Y(), point.Z()); pHER.RotateY(-angle_HER); pHER.RotateX(M_PI);
  TVector3 pLER(point.X(), point.Y(), point.Z()); pLER.RotateY(-angle_LER); pLER.RotateX(M_PI);

  //Check if the point lies inside HER or LER beam pipe
  bool HERflag = false;
  double s_HER = pHER.Z() / Unit::mm;
  double r_HER = sqrt(pHER.X() * pHER.X() + pHER.Y() * pHER.Y()) / Unit::mm;
  if (getApertureHER(s_HER) > r_HER) HERflag = true;

  bool LERflag = false;
  double s_LER = pLER.Z() / Unit::mm;
  double r_LER = sqrt(pLER.X() * pLER.X() + pLER.Y() * pLER.Y()) / Unit::mm;
  if (getApertureLER(s_LER) > r_LER) LERflag = true;

  double X, Y, s; // [m]
  double K0 = 0., K1 = 0., SK0 = 0., SK1 = 0.;
  double L = 0., ROTATE = 0., DX = 0., DY = 0.;

  /* in case the point is outside of both LER and HER, returns zero field*/
  if ((!HERflag) && (!LERflag)) return TVector3(0, 0, 0);

  /* in case the point is INSIDE of both LER and HER,
     this happenes around IP where no quadrupole field exists, returns zero field*/
  if ((HERflag) && (LERflag)) {

    B2DEBUG(20, "Return zero fields since inside both HER and LER at (x,y,z)=("
            << point.X() / Unit::m << "," << point.Y() / Unit::m << "," << point.Z() / Unit::m << "),"
            << " HER(X,Y,s)=(" << pHER.X() << "," << pHER.Y() << "," << pHER.Z()  << ")[cm], r_HER=" << r_HER << " [mm]  and "
            << " LER(X,Y,s)=(" << pLER.X() << "," << pLER.Y() << "," << pLER.Z()  << ")[cm], r_LER=" << r_LER << " [mm] . ")

    return TVector3(0, 0, 0);
  }


  int ROTATE_DIRECTION = 1; // 1: default, 0: rotate-off, -1: inversely rotate

  /* in case the point is inside HER*/
  if (HERflag) {
    X = pHER.X() / Unit::m; // in [m]
    Y = pHER.Y() / Unit::m; // in [m]
    s = pHER.Z() / Unit::m; // in [m]

    bool foundflag = false;
    //H.Nakayama: this loop could be modified to binary-search
    for (int i = 0; i < m_mapSizeHER; i++) {
      if ((m_mapBufferHER[i].s < s) && (s < m_mapBufferHER[i].s + m_mapBufferHER[i].L)) {
        K0     = m_mapBufferHER[i].K0;
        K1     = m_mapBufferHER[i].K1;
        SK0    = m_mapBufferHER[i].SK0;
        SK1    = m_mapBufferHER[i].SK1;
        L      = m_mapBufferHER[i].L;
        ROTATE = m_mapBufferHER[i].ROTATE; ROTATE *= ROTATE_DIRECTION;
        DX     = m_mapBufferHER[i].DX;
        DY     = m_mapBufferHER[i].DY;
        foundflag = true;
        break;
      }
    }

    TVector3 B(0, 0, 0);
    if (foundflag) {
      //rotate after subtracting DX,DY
      TVector3 p_tmp(X - DX, Y - DY, s);
      p_tmp.RotateZ(-ROTATE / 180.*M_PI);
      X = p_tmp.X();
      Y = p_tmp.Y();

      double Bs = 0;
      double BX = (p0_HER / c / L) * (K1 * Y + SK1 * X + SK0);
      double BY = (p0_HER / c / L) * (K1 * X - SK1 * Y + K0);
      B.SetXYZ(BX, BY, Bs);
      B.RotateZ(ROTATE / 180.*M_PI);
      B.RotateX(-M_PI); B.RotateY(angle_HER);

      B2DEBUG(20, "HER quadrupole fields calculated at (x,y,z)=("
              << point.X() / Unit::m << "," << point.Y() / Unit::m << "," << point.Z() / Unit::m
              << ") i.e. (X,Y,s)=(" << X << "," << Y << "," << s
              << ") is (Bx,By,Bz)=(" << B.X() << "," << B.Y() << "," << B.Z() << ").")
    }

    //=====================
    // HER leak component
    //=====================

    X = pHER.X() / Unit::m; // in [m]
    Y = pHER.Y() / Unit::m; // in [m]
    s = pHER.Z() / Unit::m; // in [m]
    foundflag = false;
    //H.Nakayama: this loop could be modified to binary-search
    for (int i = 0; i < m_mapSizeHERleak; i++) {
      if ((m_mapBufferHERleak[i].s < s) && (s < m_mapBufferHERleak[i].s + m_mapBufferHERleak[i].L)) {
        K0     = m_mapBufferHERleak[i].K0;
        K1     = m_mapBufferHERleak[i].K1;
        SK0    = m_mapBufferHERleak[i].SK0;
        SK1    = m_mapBufferHERleak[i].SK1;
        L      = m_mapBufferHERleak[i].L;
        ROTATE = m_mapBufferHERleak[i].ROTATE; ROTATE *= ROTATE_DIRECTION;
        DX     = m_mapBufferHERleak[i].DX;
        DY     = m_mapBufferHERleak[i].DY;
        foundflag = true;
        break;
      }
    }


    TVector3 Bleak(0, 0, 0);
    if (foundflag) {
      //rotate after subtracting DX,DY
      TVector3 p_tmp_leak(X - DX, Y - DY, s);
      p_tmp_leak.RotateZ(-ROTATE / 180.*M_PI);
      X = p_tmp_leak.X();
      Y = p_tmp_leak.Y();

      double Bs = 0;
      double BX = (p0_HER / c / L) * (K1 * Y + SK1 * X + SK0);
      double BY = (p0_HER / c / L) * (K1 * X - SK1 * Y + K0);


      Bleak.SetXYZ(BX, BY, Bs);
      Bleak.RotateZ(ROTATE / 180.*M_PI);
      Bleak.RotateX(-M_PI); Bleak.RotateY(angle_HER);

      B2DEBUG(20, "HER quadrupole leak fields calculated at (x,y,z)=("
              << point.X() / Unit::m << "," << point.Y() / Unit::m << "," << point.Z() / Unit::m
              << ") i.e. (X,Y,s)=(" << X << "," << Y << "," << s
              << ") is (Bx,By,Bz)=(" << Bleak.X() << "," << Bleak.Y() << "," << Bleak.Z() << ").")
    }


    return B + Bleak;
  }

  /* in case the point is inside LER*/
  if (LERflag) {

    X = pLER.X() / Unit::m; // in [m];
    Y = pLER.Y() / Unit::m; // in [m];
    s = pLER.Z() / Unit::m; // in [m];

    bool foundflag = false;
    //H.Nakayama: this loop could be modified to binary-search
    for (int i = 0; i < m_mapSizeLER; i++) {
      if ((m_mapBufferLER[i].s < s) && (s < m_mapBufferLER[i].s + m_mapBufferLER[i].L)) {
        K0     = m_mapBufferLER[i].K0;
        K1     = m_mapBufferLER[i].K1;
        SK0    = m_mapBufferLER[i].SK0;
        SK1    = m_mapBufferLER[i].SK1;
        L      = m_mapBufferLER[i].L;
        ROTATE = m_mapBufferLER[i].ROTATE; ROTATE *= ROTATE_DIRECTION;
        DX     = m_mapBufferLER[i].DX;
        DY     = m_mapBufferLER[i].DY;
        foundflag = true;
        break;
      }
    }


    TVector3 B(0, 0, 0);
    if (foundflag) {
      TVector3 p_tmp(X - DX, Y - DY, s);
      p_tmp.RotateZ(-ROTATE / 180.*M_PI);
      X = p_tmp.X();
      Y = p_tmp.Y();

      double Bs = 0;
      double BX = (p0_LER / c / L) * (K1 * Y + SK1 * X + SK0);
      double BY = (p0_LER / c / L) * (K1 * X - SK1 * Y + K0);
      B.SetXYZ(BX, BY, Bs);
      B.RotateZ(ROTATE / 180.*M_PI);
      B.RotateX(-M_PI); B.RotateY(angle_LER);

      B2DEBUG(20, "LER quadrupole fields calculated at (x,y,z)=("
              << point.X() / Unit::m << "," << point.Y() / Unit::m << "," << point.Z() / Unit::m
              << ") i.e. (X,Y,s)=(" << X << "," << Y << "," << s
              << ") is (Bx,By,Bz)=(" << B.X() << "," << B.Y() << "," << B.Z() << ").")
    }

    return B;
  }

  /* No chance to reach here, but put this line just in case*/
  B2FATAL("Quad magnetic field caluculation reaches to the abnormal position in the code")

  return TVector3(0, 0, 0);
}

void BFieldComponentQuad::terminate()
{
  B2DEBUG(10, "De-allocating the memory for the HER quadrupole magnetic field map loaded from the file '" << m_mapFilenameHER << "'")
  //De-Allocate memory to prevent memory leak
  delete [] m_mapBufferHER;

  B2DEBUG(10, "De-allocating the memory for the LER quadrupole magnetic field map loaded from the file '" << m_mapFilenameLER << "'")
  delete [] m_mapBufferLER;

  B2DEBUG(10, "De-allocating the memory for the HER leakage field map loaded from the file '" << m_mapFilenameHERleak << "'")
  delete [] m_mapBufferHERleak;

  B2DEBUG(10, "De-allocating the memory for the HER aperture map loaded from the file '" << m_apertFilenameHER << "'")
  delete [] m_apertBufferHER;

  B2DEBUG(10, "De-allocating the memory for the LER aperture map loaded from the file '" << m_apertFilenameLER << "'")
  delete [] m_apertBufferLER;

}
