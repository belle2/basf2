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
    m_mapBufferHER[i].ROTATE = ROTATE; // [degree]
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
    m_mapBufferLER[i].ROTATE = ROTATE; // [degree]
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
  //Conversion to LER/HER SAD coordinates
  double angle_crossing = 0.0830; //H.Nakayama: sorry, hard-coded
  double angle_HER = - angle_crossing / 2.;
  TVector3 pHER(point.X(), point.Y(), point.Z()); pHER.RotateY(angle_HER); pHER.RotateX(M_PI);
  double angle_LER =  angle_crossing / 2.;
  TVector3 pLER(point.X(), point.Y(), point.Z()); pLER.RotateY(angle_LER); pLER.RotateX(M_PI);

  //Check if the point lies inside HER or LER beam pipe
  bool HERflag = false;
  double s_HER = - pHER.Z() / Unit::mm; // inverse s-direction in SAD and ApertHER.dat
  double r_HER = sqrt(pHER.X() * pHER.X() + pHER.Y() * pHER.Y()) / Unit::mm;
  if (getApertureHER(s_HER) > r_HER) HERflag = true;

  bool LERflag = false;
  double s_LER = pLER.Z() / Unit::mm;
  double r_LER = sqrt(pLER.X() * pLER.X() + pLER.Y() * pLER.Y()) / Unit::mm;
  if (getApertureLER(s_LER) > r_LER) LERflag = true;

  double X, Y, s; // [m]
  double K0, K1, SK0, SK1, L, ROTATE;
  double p0_HER = 7.00729e+9; // [eV] //H.Nakayama: sorry, hard-coded
  double p0_LER = 4.00000e+9; // [eV] //H.Nakayama: sorry, hard-coded
  double c = 3.0e+8;// [m/s]          //H.Nakayama: sorry, hard-coded

  /* in case the point is outside of both LER and HER, returns zero field*/
  if ((!HERflag) && (!LERflag)) return TVector3(0, 0, 0);

  /* in case the point is INSIDE of both LER and HER,
     this happenes around IP where no quadrupole field exists, returns zero field*/
  if ((HERflag) && (LERflag)) return TVector3(0, 0, 0);

  int ROTATE_DIRECTION = 1; // 1: default, 0: rotate-off, -1: inversely rotate

  bool OFFSETflag = false;
  double OrbitOffsetHER = 0.0007; //H.Nakayama: sorry, hard-coded

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
        ROTATE = m_mapBufferHER[i].ROTATE;
        ROTATE *= ROTATE_DIRECTION;
        foundflag = true;
        break;
      }
    }

    if ((OFFSETflag) && (s >  1.18)) X += OrbitOffsetHER; //H.Nakayama: sorry, hard-coded
    if ((OFFSETflag) && (s < -1.18)) X -= OrbitOffsetHER; //H.Nakayama: sorry, hard-coded

    TVector3 p_tmp(X, Y, s); p_tmp.RotateZ(-ROTATE / 180.*M_PI);
    X = p_tmp.X();
    Y = p_tmp.Y();

    if (foundflag) {
      double Bs = 0;
      double BX = (p0_HER / c / L) * (K1 * Y + SK1 * X + SK0);
      double BY = (p0_HER / c / L) * (K1 * X - SK1 * Y + K0);
      TVector3 B(BX, BY, Bs);
      B.RotateZ(ROTATE / 180.*M_PI);
      B.RotateX(-M_PI); B.RotateY(-angle_HER);

      B2DEBUG(20, "HER quadrupole fields calculated at (x,y,z)=("
              << point.X() / Unit::m << "," << point.Y() / Unit::m << "," << point.Z() / Unit::m
              << " i.e. (X,Y,s)=(" << X << "," << Y << "," << s
              << ") is (Bx,By,Bz)=(" << B.X() << "," << B.Y() << "," << B.Z() << ").")

      return B;
    } else {
      return TVector3(0, 0, 0);
    }
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
        ROTATE = m_mapBufferLER[i].ROTATE;
        ROTATE *= ROTATE_DIRECTION;
        foundflag = true;
        break;
      }
    }
    TVector3 p_tmp(X, Y, s); p_tmp.RotateZ(-ROTATE / 180.*M_PI);
    X = p_tmp.X();
    Y = p_tmp.Y();

    //hoge
    if (foundflag) {
      double Bs = 0;
      double BX = (p0_LER / c / L) * (K1 * Y + SK1 * X + SK0);
      double BY = (p0_LER / c / L) * (K1 * X - SK1 * Y + K0);
      TVector3 B(BX, BY, Bs);
      B.RotateZ(ROTATE / 180.*M_PI);
      B.RotateX(-M_PI); B.RotateY(-angle_LER);

      B2DEBUG(20, "LER quadrupole fields calculated at (x,y,z)=("
              << point.X() / Unit::m << "," << point.Y() / Unit::m << "," << point.Z() / Unit::m
              << " i.e. (X,Y,s)=(" << X << "," << Y << "," << s
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
