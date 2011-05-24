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

#include <framework/core/Environment.h>
#include <framework/core/ModuleUtils.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>

#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/filter/gzip.hpp>

#include <cmath>

using namespace std;
using namespace Belle2;
namespace io = boost::iostreams;


BFieldComponentQuad::BFieldComponentQuad() : m_mapFilenameHER(""), m_mapFilenameLER(""), m_apertFilenameHER(""), m_apertFilenameLER("")
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
  string fullPathMapHER = Environment::Instance().getDataSearchPath() + "/" + m_mapFilenameHER;
  if (!ModuleUtils::fileNameExists(fullPathMapHER)) {
    B2FATAL("The HER quadrupole magnetic field map file '" << m_mapFilenameHER << "' could not be found !")
    return;
  }
  string fullPathMapLER = Environment::Instance().getDataSearchPath() + "/" + m_mapFilenameLER;
  if (!ModuleUtils::fileNameExists(fullPathMapLER)) {
    B2FATAL("The LER quadrupole magnetic field map file '" << m_mapFilenameLER << "' could not be found !")
    return;
  }
  string fullPathApertHER = Environment::Instance().getDataSearchPath() + "/" + m_apertFilenameHER;
  if (!ModuleUtils::fileNameExists(fullPathApertHER)) {
    B2FATAL("The HER aperture file '" << m_apertFilenameHER << "' could not be found !")
    return;
  }
  string fullPathApertLER = Environment::Instance().getDataSearchPath() + "/" + m_apertFilenameLER;
  if (!ModuleUtils::fileNameExists(fullPathApertLER)) {
    B2FATAL("The LER aperture file '" << m_apertFilenameLER << "' could not be found !")
    return;
  }

  //--------------------------------------------------------------
  // Load the field map files
  //--------------------------------------------------------------

  io::filtering_istream fieldMapFileHER, fieldMapFileLER;
  fieldMapFileHER.push(io::file_source(fullPathMapHER));
  fieldMapFileLER.push(io::file_source(fullPathMapLER));

  string name;
  double s, L, K0, K1, SK0, SK1, ROTATE;

  //Create the parameter map and read the data from the file
  B2DEBUG(10, "Loading the HER quadrupole magnetic field from file '" << m_mapFilenameHER << "' in to the memory...")
  m_mapBufferHER = new ParamPoint[m_mapSizeHER];
  for (int i = 0; i < m_mapSizeHER; ++i) {
    fieldMapFileHER >> name >> s >> L >> K0 >> K1 >> SK0 >> SK1 >> ROTATE;
    /* Save parametors in unit [m], not in unit [cm].*/
    m_mapBufferHER[i].s      = s;   // [m]
    m_mapBufferHER[i].L      = L;   // [m]
    m_mapBufferHER[i].K0     = K0;  // [dimensionless]
    m_mapBufferHER[i].K1     = K1;  // [1/m]
    m_mapBufferHER[i].SK0    = SK0; // [dimensionless]
    m_mapBufferHER[i].SK1    = SK1; // [1/m]
    m_mapBufferHER[i].ROTATE = ROTATE; // [radian]
    B2DEBUG(10, "... loaded HER SAD element " << name << " at s= " << s << "[m].")
  }
  B2DEBUG(10, "... loaded " << m_mapSizeHER << " elements.")

  B2DEBUG(10, "Loading the LER quadrupole magnetic field from file '" << m_mapFilenameLER << "' in to the memory...")
  m_mapBufferLER = new ParamPoint[m_mapSizeLER];
  for (int i = 0; i < m_mapSizeLER; ++i) {
    fieldMapFileLER >> name >> s >> L >> K0 >> K1 >> SK0 >> SK1 >> ROTATE;
    /* Save parametors in unit [m], not in unit [cm].*/
    m_mapBufferLER[i].s      = s;   // [m]
    m_mapBufferLER[i].L      = L;   // [m]
    m_mapBufferLER[i].K0     = K0;  // [dimensionless]
    m_mapBufferLER[i].K1     = K1;  // [1/m]
    m_mapBufferLER[i].SK0    = SK0; // [dimensionless]
    m_mapBufferLER[i].SK1    = SK1; // [1/m]
    m_mapBufferLER[i].ROTATE = ROTATE; // [radian]
    B2DEBUG(10, "... loaded LER SAD element " << name << " at s= " << s << "[m].")
  }
  B2DEBUG(10, "... loaded " << m_mapSizeLER << " elements.")


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
    /* Save parametors in unit [mm], not in unit [cm].*/
    m_apertBufferHER[i].s   = s;
    m_apertBufferHER[i].r   = r;
    B2DEBUG(10, "... loaded HER aperture at s = " << s << "[mm].")
  }
  B2DEBUG(10, "... loaded " << m_apertSizeHER << " elements.")

  B2DEBUG(10, "Loading the LER aperture from file '" << m_apertFilenameLER << "' in to the memory...")
  m_apertBufferLER = new ApertPoint[m_apertSizeLER];
  for (int i = 0; i < m_apertSizeLER; ++i) {
    apertFileLER >> s >> r;
    /* Save parametors in unit [mm], not in unit [cm].*/
    m_apertBufferLER[i].s   = s;
    m_apertBufferLER[i].r   = r;
    B2DEBUG(10, "... loaded LER aperture at s = " << s << "[mm].")
  }
  B2DEBUG(10, "... loaded " << m_apertSizeLER << " elements.")


}

double BFieldComponentQuad::getApertureHER(double s) const
{
  if (s < m_apertBufferHER[0].s) return m_apertBufferHER[0].r;
  if (s > m_apertBufferHER[m_apertSizeHER-1].s) return m_apertBufferHER[m_apertSizeHER-1].r;
  //H.Nakayama: this loop could be modified to binary-search
  for (int i = 0; i < m_apertSizeHER - 1; i++) {
    if ((m_apertBufferHER[i].s <= s) && (s < m_apertBufferHER[i+1].s)) {
      double s1 = m_apertBufferHER[i].s;
      double s2 = m_apertBufferHER[i+1].s;
      double r1 = m_apertBufferHER[i].r;
      double r2 = m_apertBufferHER[i+1].r;
      double r = r1 + (r2 - r1) * (s - s1) / (s2 - s1);
      return r;
    }
  }
  return 0;
}

double BFieldComponentQuad::getApertureLER(double s) const
{
  if (s < m_apertBufferLER[0].s) return m_apertBufferLER[0].r;
  if (s > m_apertBufferLER[m_apertSizeLER-1].s) return m_apertBufferLER[m_apertSizeLER-1].r;
  //H.Nakayama: this loop could be modified to binary-search
  for (int i = 0; i < m_apertSizeLER - 1; i++) {
    if ((m_apertBufferLER[i].s <= s) && (s < m_apertBufferLER[i+1].s)) {
      double s1 = m_apertBufferLER[i].s;
      double s2 = m_apertBufferLER[i+1].s;
      double r1 = m_apertBufferLER[i].r;
      double r2 = m_apertBufferLER[i+1].r;
      double r = r1 + (r2 - r1) * (s - s1) / (s2 - s1);
      return r;
    }
  }
  return 0;
}


TVector3 BFieldComponentQuad::calculate(const TVector3& point) const
{

  //Check if the point lies inside HER or LER beam pipe
  double angle_HER = -0.0415;   //H.Nakayama: hard-coded parameters should be moved to XML
  TVector3 pHER(point.X(), point.Y(), point.Z()); pHER.RotateY(angle_HER);
  bool HERflag = false;
  double s_HER = pHER.Z() / Unit::mm;
  double r_HER = sqrt(pHER.X() * pHER.X() + pHER.Y() * pHER.Y()) / Unit::mm;
  if (getApertureHER(s_HER) > r_HER) HERflag = true;

  bool LERflag = false;
  double angle_LER = 0.0415 - 3.141592; //H.Nakayama: hard-coded parameters should be moved to XML
  TVector3 pLER(point.X(), point.Y(), point.Z()); pLER.RotateY(angle_LER);
  double s_LER = pLER.Z() / Unit::mm;
  double r_LER = sqrt(pLER.X() * pLER.X() + pLER.Y() * pLER.Y()) / Unit::mm;
  if (getApertureLER(s_LER) > r_LER) LERflag = true;

  double x, y, s; // [m]
  double K0, K1, SK0, SK1, L, ROTATE;
  double p0_HER = 7.0e+9; // [eV] //H.Nakayama: hard-coded parameters should be moved to XML
  double p0_LER = 4.0e+9; // [eV] //H.Nakayama: hard-coded parameters should be moved to XML
  double c = 3.0e+8;// [m/s]      //H.Nakayama: hard-coded parameters should be moved to XML

  /* in case the point is outside of both LER and HER, returns zero field*/
  if ((!HERflag) && (!LERflag)) return TVector3(0, 0, 0);

  /* in case the point is INSIDE of both LER and HER,
     this happenes around IP where no quadrupole field exists, returns zero field*/
  if ((HERflag) && (LERflag)) return TVector3(0, 0, 0);

  bool ROTATEflag = false;
  bool OFFSETflag = false;
  double OrbitOffsetHER = 0.0007;

  /* in case the point is inside HER*/
  if (HERflag) {
    x = pHER.X() / Unit::m; // in [m]
    y = pHER.Y() / Unit::m; // in [m]
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
        ROTATE = m_mapBufferHER[i].ROTATE;
        foundflag = true;
        break;
      }
    }

    if ((OFFSETflag) && (abs(s) > 1.18)) x -= OrbitOffsetHER;

    TVector3 p_tmp(x, y, s); p_tmp.RotateZ(ROTATE);
    if (ROTATEflag) x = p_tmp.X();
    if (ROTATEflag) y = p_tmp.Y();

    if (foundflag) {
      double Bz = 0;
      double Bx = (p0_HER / c / L) * (K1 * y - SK1 * x - SK0);
      double By = (p0_HER / c / L) * (K1 * x + SK1 * y + K0);
      TVector3 B(Bx, By, Bz);
      if (ROTATEflag) B.RotateZ(-ROTATE);
      B.RotateY(-angle_HER);
      B2DEBUG(20, "HER quadrupole fields calculated at (x,y,z)=("
              << point.X() / Unit::m << "," << point.Y() / Unit::m << "," << point.Z() / Unit::m
              << " i.e. (x',y',s)=(" << x << "," << y << "," << s
              << ") is (Bx,By,Bz)=(" << B.X() << "," << B.Y() << "," << B.Z() << ").")
      return B;
    } else {
      return TVector3(0, 0, 0);
    }
  }

  /* in case the point is inside LER*/
  if (LERflag) {

    x = pLER.X() / Unit::m; // in [m];
    y = pLER.Y() / Unit::m; // in [m];
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
        ROTATE = m_mapBufferLER[i].ROTATE;
        foundflag = true;
        break;
      }
    }
    TVector3 p_tmp(x, y, s); p_tmp.RotateZ(ROTATE);
    if (ROTATEflag) x = p_tmp.X();
    if (ROTATEflag) y = p_tmp.Y();

    if (foundflag) {
      double Bz = 0;
      double Bx = (p0_LER / c / L) * (K1 * y - SK1 * x - SK0);
      double By = (p0_LER / c / L) * (K1 * x + SK1 * y + K0);
      TVector3 B(Bx, By, Bz);
      if (ROTATEflag) B.RotateZ(-ROTATE);
      B.RotateY(-angle_LER);
      B2DEBUG(20, "LER quadrupole fields calculated at (x,y,z)=("
              << point.X() / Unit::m << "," << point.Y() / Unit::m << "," << point.Z() / Unit::m
              << " i.e. (x',y',s)=(" << x << "," << y << "," << s
              << ") is (Bx,By,Bz)=(" << B.X() << "," << B.Y() << "," << B.Z() << ").")
      return B;
    } else {
      return TVector3(0, 0, 0);
    }
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

  B2DEBUG(10, "De-allocating the memory for the HER aperture map loaded from the file '" << m_apertFilenameHER << "'")
  delete [] m_apertBufferHER;

  B2DEBUG(10, "De-allocating the memory for the LER aperture map loaded from the file '" << m_apertFilenameLER << "'")
  delete [] m_apertBufferLER;

}
