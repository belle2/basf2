/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Poyuan Chen                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <ecl/geometry/ECLGeometryPar.h>
#include <framework/logging/Logger.h>

#include <G4VTouchable.hh>
#include <G4PhysicalVolumeStore.hh>
#include <G4PVReplica.hh>
#include <G4NavigationHistory.hh>
#include <G4Transform3D.hh>
#include <G4Point3D.hh>
#include <G4Vector3D.hh>
#include <iomanip>

using namespace std;
using namespace Belle2;
using namespace ECL;

ECLGeometryPar* ECLGeometryPar::m_B4ECLGeometryParDB = 0;

class Mapping_t {
public:
  static void Mapping(int id, int& ThetaId, int& PhiId)
  {
    ThetaId = m_Theta[((unsigned int)id) >> 4];
    PhiId = id - m_dTheta[ThetaId] * 16 - ThetaId * 128;
  }

  static void Mapping(int id, int& ThetaId, int& PhiId, int& nrep, int& indx)
  {
    Mapping(id, ThetaId, PhiId);

    int off = m_offsets[ThetaId];
    int i = m_tbl[ThetaId];

    int r = m_recip[i];
    int d = m_denom[i];

    nrep = (PhiId * r) >> m_RECIPROCAL_SHIFT;
    indx = off + (PhiId - nrep * d);
  }

  static int CellID(int ThetaId, int PhiId)
  {
    return PhiId + m_dTheta[ThetaId] * 16 + ThetaId * 128;
  }

  static int Offset(int ThetaId)
  {
    return m_dTheta[ThetaId] + ThetaId * 8;
  }

  static int Indx2ThetaId(int indx)
  {
    return m_Theta[indx];
  }

  static int ThetaId2NCry(int ThetaId)  // Theta Id to the number of crystals @ this Id
  {
    return m_denom[m_tbl[ThetaId]];
  }

private:
  static const char m_dTheta[69];
  static const unsigned char m_Theta[546], m_tbl[69], m_offsets[69];

  static const unsigned char m_RECIPROCAL_SHIFT = 16;
  static const unsigned int m_recip[5], m_denom[5];
};

const unsigned char Mapping_t::m_Theta[546] = {
  0, 0, 0,  1, 1, 1,
  2, 2, 2, 2,  3, 3, 3, 3,  4, 4, 4, 4,
  5, 5, 5, 5, 5, 5,  6, 6, 6, 6, 6, 6,  7, 7, 7, 7, 7, 7,
  8, 8, 8, 8, 8, 8,  9, 9, 9, 9, 9, 9,  10, 10, 10, 10, 10, 10,
  11, 11, 11, 11, 11, 11, 11, 11, 11,  12, 12, 12, 12, 12, 12, 12, 12, 12,

  13, 13, 13, 13, 13, 13, 13, 13, 13,  14, 14, 14, 14, 14, 14, 14, 14, 14,  15, 15, 15, 15, 15, 15, 15, 15, 15,
  16, 16, 16, 16, 16, 16, 16, 16, 16,  17, 17, 17, 17, 17, 17, 17, 17, 17,  18, 18, 18, 18, 18, 18, 18, 18, 18,
  19, 19, 19, 19, 19, 19, 19, 19, 19,  20, 20, 20, 20, 20, 20, 20, 20, 20,  21, 21, 21, 21, 21, 21, 21, 21, 21,
  22, 22, 22, 22, 22, 22, 22, 22, 22,  23, 23, 23, 23, 23, 23, 23, 23, 23,  24, 24, 24, 24, 24, 24, 24, 24, 24,
  25, 25, 25, 25, 25, 25, 25, 25, 25,  26, 26, 26, 26, 26, 26, 26, 26, 26,  27, 27, 27, 27, 27, 27, 27, 27, 27,
  28, 28, 28, 28, 28, 28, 28, 28, 28,  29, 29, 29, 29, 29, 29, 29, 29, 29,  30, 30, 30, 30, 30, 30, 30, 30, 30,
  31, 31, 31, 31, 31, 31, 31, 31, 31,  32, 32, 32, 32, 32, 32, 32, 32, 32,  33, 33, 33, 33, 33, 33, 33, 33, 33,
  34, 34, 34, 34, 34, 34, 34, 34, 34,  35, 35, 35, 35, 35, 35, 35, 35, 35,  36, 36, 36, 36, 36, 36, 36, 36, 36,
  37, 37, 37, 37, 37, 37, 37, 37, 37,  38, 38, 38, 38, 38, 38, 38, 38, 38,  39, 39, 39, 39, 39, 39, 39, 39, 39,
  40, 40, 40, 40, 40, 40, 40, 40, 40,  41, 41, 41, 41, 41, 41, 41, 41, 41,  42, 42, 42, 42, 42, 42, 42, 42, 42,
  43, 43, 43, 43, 43, 43, 43, 43, 43,  44, 44, 44, 44, 44, 44, 44, 44, 44,  45, 45, 45, 45, 45, 45, 45, 45, 45,
  46, 46, 46, 46, 46, 46, 46, 46, 46,  47, 47, 47, 47, 47, 47, 47, 47, 47,  48, 48, 48, 48, 48, 48, 48, 48, 48,
  49, 49, 49, 49, 49, 49, 49, 49, 49,  50, 50, 50, 50, 50, 50, 50, 50, 50,  51, 51, 51, 51, 51, 51, 51, 51, 51,
  52, 52, 52, 52, 52, 52, 52, 52, 52,  53, 53, 53, 53, 53, 53, 53, 53, 53,  54, 54, 54, 54, 54, 54, 54, 54, 54,
  55, 55, 55, 55, 55, 55, 55, 55, 55,  56, 56, 56, 56, 56, 56, 56, 56, 56,  57, 57, 57, 57, 57, 57, 57, 57, 57,
  58, 58, 58, 58, 58, 58, 58, 58, 58,

  59, 59, 59, 59, 59, 59, 59, 59, 59,  60, 60, 60, 60, 60, 60, 60, 60, 60,
  61, 61, 61, 61, 61, 61,  62, 62, 62, 62, 62, 62,
  63, 63, 63, 63, 63, 63,  64, 64, 64, 64, 64, 64,
  65, 65, 65, 65, 65, 65,
  66, 66, 66, 66,  67, 67, 67, 67,  68, 68, 68, 68
};

const char Mapping_t::m_dTheta[69] = {
  0, -5, -10, -14, -18, -22, -24, -26, -28, -30, -32, -34, -33, // forward

  -32, -31, -30, -29, -28, -27, -26, -25, -24,
  -23, -22, -21, -20, -19, -18, -17, -16, -15,
  -14, -13, -12, -11, -10,  -9,  -8,  -7,  -6,
  -5,  -4,  -3,  -2,  -1,   0,   1,   2,   3,
  4,   5,   6,   7,   8,   9,  10,  11,  12, 13,

  14, 15, 16, 14, 12, 10, 8, 6, 2, -2 // backward
};

const unsigned char Mapping_t::m_tbl[69] = { // pointer to the denominator/reciprocal arrays or how many crystals in a phi sector
  1, 1, 2, 2, 2, 3, 3, 3, 3, 3, 3, 4, 4, // forward

  0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

  4, 4, 3, 3, 3, 3, 3, 2, 2, 2           // backward
};

const unsigned char Mapping_t::m_offsets[69] = {
  0,  3,  6, 10, 14, 18, 24, 30, 36, 42, 48, 54, 63, // forward

  132, 134, 136, 138, 140, 142, 144, 146, 148,
  150, 152, 154, 156, 158, 160, 162, 164, 166,
  168, 170, 172, 174, 176, 178, 180, 182, 184,
  186, 188, 190, 192, 194, 196, 198, 200, 202,
  204, 206, 208, 210, 212, 214, 216, 218, 220, 222,

  72, 81, 90, 96, 102, 108, 114, 120, 124, 128       // backward
};

#define pack(x) ((1<<Mapping_t::m_RECIPROCAL_SHIFT)/x+1)
const unsigned int Mapping_t::m_recip[] = {pack(2), pack(3), pack(4), pack(6), pack(9)};
const unsigned int Mapping_t::m_denom[] = {    (2), (3), (4), (6), (9)};
#undef pack


ECLGeometryPar* ECLGeometryPar::Instance()
{
  if (!m_B4ECLGeometryParDB) m_B4ECLGeometryParDB = new ECLGeometryPar();
  return m_B4ECLGeometryParDB;
}

ECLGeometryPar::ECLGeometryPar()
{
  clear();
  // delay crystal positions fetching to a moment when it actually needed and geometry is already built
  //  read();
}

ECLGeometryPar::~ECLGeometryPar()
{
  if (m_B4ECLGeometryParDB) {
    delete m_B4ECLGeometryParDB;
    B2DEBUG(150, "m_B4ECLGeometryParDB deleted ");
  }
}

void ECLGeometryPar::clear()
{
  m_ini_cid = -1;
  mPar_cellID = 0;
  mPar_thetaID = 0;
  mPar_phiID = 0;
}

// There is no way to get world coordinates of a local point of a physical volume in Geant.
// The only way to check geometry is to trace particle and check volumes it crosses.
// Here particle gun parameters are produced to check crystal positions with the center @ r0 and direction n
void ParticleGunParameters(const G4String& comment, const G4ThreeVector& n, const G4ThreeVector& r0, double dphi)
{
  cout << comment << endl;
  cout << "Center position = " << r0 << ", Direction = " << n << endl;
  // closest point to z-axis
  double t = -(n.z() * r0.z() - n * r0) / (n.z() * n.z() - n * n);
  G4ThreeVector r = n * t + r0;
  cout << "Closest point to z-axis = " << r << endl; // at the moment I do not see tilt in phi
  const double r2d = 180 / M_PI;
  double th = r2d * n.theta();
  double phi = r2d * n.phi();
  double z = r.z();
  dphi *= r2d;
  cout << "    'thetaParams': [" << th << ", " << th << "]," << endl;
  cout << "    'phiParams': [" << phi << "+0*" << dphi << ", " << phi << "+0*" << dphi << "]," << endl;
  cout << "    'zVertexParams': [" << z << ", " << z << "]" << endl;
}

G4Transform3D getT(const G4VPhysicalVolume& v)
{
  return  G4Transform3D(v.GetObjectRotationValue(), v.GetObjectTranslation());
}

G4Transform3D getT(const G4String& n)
{
  G4PhysicalVolumeStore* store = G4PhysicalVolumeStore::GetInstance();
  // int N = store->size();
  // cout<<N<<endl;
  // for(int i=0;i<N;i++){
  //   G4VPhysicalVolume *vv = (*store)[i];
  //   if(vv->GetName()==n){
  //     cout<<i<<" "<<vv->GetName()<<" "<<vv->GetMultiplicity()<<" "<<vv->GetCopyNo()<<endl;
  //     G4PVReplica *r = dynamic_cast<G4PVReplica*>(vv);
  //     if(r) cout<<"Instance = "<<r->GetInstanceID()<<endl;
  //   }
  // }

  G4VPhysicalVolume* v = store->GetVolume(n);
  return getT(*v);
}

void print_trans(const G4Transform3D& t)
{
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 3; j++)
      cout << t(j, i) << " ";
    cout << endl;
  }
}

void ECLGeometryPar::read()
{
  m_crystals.clear();
  m_crystals.reserve(46 * 2 + 132); // 10752 bytes

  // Endcap sectors are rotated since behaviour of G4Replica class. It
  // requires a physical volume during phi replication to be symmetric
  // around phi=0. So we need to rotate it by -((2*pi)/16)/2 angle at the
  // geometry description are return back here.

  G4Point3D p0(0, 0, 0); G4Vector3D v0(0, 0, 1);

  {
    G4Transform3D T0 = getT("ECLForwardPhysical");
    G4Transform3D T1;// = getT("ECLForwardSectorPhysical");
    G4Transform3D T2 = getT("ECLForwardCrystalSectorPhysical_1");
    G4Transform3D T = T0 * T1 * T2;

    G4String tnamef("ECLForwardWrappedCrystal_Physical_");
    for (int i = 0; i < 72; i++) {
      G4String vname(tnamef); vname += to_string(i);
      G4Transform3D cT = T * getT(vname);
      CrystalGeom_t c = {(1 / CLHEP::cm)* (cT * p0), cT * v0};
      m_crystals.push_back(c);
      //      G4cout << i << " " << c.pos << " " << c.dir << G4endl;
    }
  }

  {
    G4Transform3D T0 = getT("ECLBackwardPhysical");
    G4Transform3D T1;// = getT("ECLBackwardSectorPhysical");
    G4Transform3D T2 = getT("ECLBackwardCrystalSectorPhysical_0");
    G4Transform3D T = G4RotateZ3D(M_PI) * T0 * T1 * T2;

    G4String tnamef("ECLBackwardWrappedCrystal_Physical_");
    for (int i = 0; i < 60; i++) {
      G4String vname(tnamef); vname += to_string(i);
      G4Transform3D cT = T * getT(vname);
      CrystalGeom_t c = {(1 / CLHEP::cm)* (cT * p0), cT * v0};
      m_crystals.push_back(c);
      //      G4cout << i << " " << c.pos << " " << c.dir << G4endl;
    }
  }

  {
    // get barrel sector (between two septums) transformation
    G4Transform3D Ts = getT("ECLBarrelSectorPhysical_0");
    // since barrel sector is symmetric around phi=0 we need to
    // translate crystal with negative phi back to positive rotating
    // crystal position by (2*M_PI/72) angle
    G4Transform3D Ts1 = G4RotateZ3D(M_PI / 36) * Ts;

    G4String tname("ECLBarrelWrappedCrystal_Physical_");
    for (int i = 0; i < 2 * 46; i++) {
      G4String vname(tname); vname += to_string(i);
      G4Transform3D cT = ((i % 2) ? Ts1 : Ts) * getT(vname);
      CrystalGeom_t c = {(1 / CLHEP::cm)* (cT * p0), cT * v0};
      m_crystals.push_back(c);
      //      G4cout << i << " " << c.pos << " " <<c.dir<<G4endl;
    }
  }

  B2DEBUG(150, "ECLGeometryPar::read() initialized with " << m_crystals.size() << " crystals.");
}

template <int n>
void sincos(const double* ss, int iphi, double& s, double& c)
{
  int n4 = n / 4;
  int iq = iphi / n4; // quadrant
  int is = iphi % n4;
  int ic = n4 - is;
  if (iq & 1) {int t = is; is = ic; ic = t;}
  double ls = ss[is];
  double lc = ss[ic];
  // check quadrant and assign sign bit accordingly
  if ((((0 << 0) + (0 << 1) + (1 << 2) + (1 << 3)) >> iq) & 1) ls = -ls;
  if ((((0 << 0) + (1 << 1) + (1 << 2) + (0 << 3)) >> iq) & 1) lc = -lc;
  s = ls;
  c = lc;
}

const double ss72[] = {
  0.0000000000000000000000,
  0.0871557427476581735581,
  0.1736481776669303488517,
  0.2588190451025207623489,
  0.3420201433256687330441,
  0.4226182617406994361870,
  0.5000000000000000000000,
  0.5735764363510460961080,
  0.6427876096865393263226,
  0.7071067811865475244008,
  0.7660444431189780352024,
  0.8191520442889917896845,
  0.8660254037844386467637,
  0.9063077870366499632426,
  0.9396926207859083840541,
  0.9659258262890682867497,
  0.9848077530122080593667,
  0.9961946980917455322950,
  1.0000000000000000000000
};

const double ss16[] = {
  0.0000000000000000000000,
  0.3826834323650897717285,
  0.7071067811865475244008,
  0.9238795325112867561282,
  1.0000000000000000000000
};

void ECLGeometryPar::InitCrystal(int cid)
{
  if (m_crystals.size() == 0) read();
  int thetaid, phiid, nreplica, indx;
  Mapping_t::Mapping(cid, thetaid, phiid, nreplica, indx);
  //  cout<<cid<<" "<<thetaid<<" "<<phiid<<" "<<nreplica<<" "<<indx<<endl;
  const CrystalGeom_t& t = m_crystals[indx];
  double s, c;
  if (indx > 131)
    sincos<72>(ss72, nreplica, s, c);
  else
    sincos<16>(ss16, nreplica, s, c);

  double xp = c * t.pos.x() - s * t.pos.y();
  double yp = s * t.pos.x() + c * t.pos.y();
  m_current_crystal.pos.set(xp, yp, t.pos.z());

  double xv = c * t.dir.x() - s * t.dir.y();
  double yv = s * t.dir.x() + c * t.dir.y();
  m_current_crystal.dir.set(xv, yv, t.dir.z());
  //  cout<<t.pos<<" "<<t.dir<<" "<<m_current_crystal.pos<<" "<<m_current_crystal.dir<<endl;
  m_ini_cid = cid;
}

int ECLGeometryPar::GetCellID(int ThetaId, int PhiId)
{
  return Mapping_t::CellID(ThetaId, PhiId);
}

void ECLGeometryPar::Mapping(int cid)
{
  mPar_cellID = cid;
  Mapping_t::Mapping(mPar_cellID, mPar_thetaID, mPar_phiID);
}

int ECLGeometryPar::TouchableDiodeToCellID(const G4VTouchable* touch)
{
  return TouchableToCellID(touch);
}

int ECLGeometryPar::TouchableToCellID(const G4VTouchable* touch)
{
  //  touch->GetCopyNumber() is a virtual call, avoid it by using
  //  directly G4NavigationHistory so we will have only one virtual
  //  call instead of three here
  const G4NavigationHistory* h = touch->GetHistory();
  int hd = h->GetDepth();
  int i1 = h->GetReplicaNo(hd - 1); // index of each volume is set at physical volume creation
  int i2 = h->GetReplicaNo(hd - 2); // go up in volume hierarchy

  int ThetaId = Mapping_t::Indx2ThetaId(i1);
  int NCryst  = Mapping_t::ThetaId2NCry(ThetaId); // the number of crystals in a sector at given ThetaId
  int Offset  = Mapping_t::Offset(ThetaId);

  int ik = i1 - Offset;
  int PhiId;
  if (NCryst == 2) {
    PhiId = (i2 - (ik % 2)) * NCryst + ik;
    if (PhiId < 0) PhiId += 144;
  } else {
    int i3 = h->GetReplicaNo(hd - 3); // go up in volume hierarchy
    if (ThetaId < 13)
      PhiId = (i2 + 2 * i3) * NCryst + ik;
    else {
      // int tt[] = {3,2,1,0,7,6,5,4};
      // if(i3r!=tt[i3]){
      //  cout<<i3<<" "<<i3<<" "<<tt[i3]<<endl;
      //  exit(0);
      // }
      int i3r = (3 - (i3 % 4)) + 4 * (i3 / 4);
      PhiId = (2 * i3r + (1 - i2)) * NCryst + ik;
    }
  }

  int cellID = Offset * 16 + PhiId;
  //    cout<<"ECLGeometryPar::TouchableToCellID "<<h->GetVolume(hd-1)->GetName()<<" "<<i1<<" "<<i2<<" "<<h->GetReplicaNo(hd - 3)<<" "<<ThetaId<<" "<<NCryst<<" "<<Offset<<" "<<PhiId<<endl;

  // test of the position and direction of crystal
  if (0) {
    G4AffineTransform t = h->GetTopTransform();
    G4ThreeVector o(0, 0, 0), n(0, 0, 1);
    G4ThreeVector ro = t.Inverse().TransformPoint(o);
    G4ThreeVector rn = t.Inverse().TransformAxis(n);

    InitCrystal(cellID);
    ro *= 1 / CLHEP::cm;

    G4ThreeVector dr = m_current_crystal.pos - ro, dn = m_current_crystal.dir - rn;
    if (dr.mag() > 1e-10 || dn.mag() > 1e-10) {
      cout << "Missmatch " << h->GetVolume(hd - 1)->GetName() << " " << cellID << " " << ThetaId << " " << PhiId << " " << NCryst << " "
           << hd << " " << i2 << " " << i1 << " " << m_current_crystal.pos << " " << ro << " " << rn << " " << dr << " " << dn << endl;

      for (int i = 0; i < 144; i++) {
        int ci = Mapping_t::CellID(ThetaId, i);
        InitCrystal(ci);
        dr = m_current_crystal.pos - ro;
        if (dr.mag() < 1e-10) cout << "best PhiId = " << i << endl;
      }
    }
  }
  return cellID;
}

int ECLGeometryPar::ECLVolumeToCellID(const G4VTouchable* touch)
{
  int depth = touch->GetHistoryDepth();
  if ((depth != 3) && (depth != 5)) {
    B2WARNING("ECLGeometryPar::ECLVolumeToCellID: History depth = " << depth << " is out of range: should be 3 or 5.");
    return -1;
  }
  const G4String& vname = touch->GetVolume()->GetName();
  std::size_t pos0 = vname.find("lv_forward_crystal_");
  std::size_t pos1 = vname.find("lv_barrel_crystal_");
  std::size_t pos2 = vname.find("lv_backward_crystal_");
  if (pos0 == string::npos && pos1 == string::npos && pos2 == string::npos) {
    B2WARNING("ECLGeometryPar::ECLVolumeToCellID: Volume name does not match pattern. NAME=" << vname);
    return -1;
  }
  return TouchableToCellID(touch);
}

double ECLGeometryPar::time2sensor(int cid, const G4ThreeVector& hit_pos)
{
  if (cid != m_ini_cid) InitCrystal(cid);
  double z = 15. - (hit_pos - m_current_crystal.pos) * m_current_crystal.dir; // position along the vector of crystal axis
  double dt = 6.05 + z * (0.0749 - z * 0.00112); // flight time to diode sensor in nanoseconds
  return dt;
}

EclNbr::EclNbr() :
  m_nbrs(*new std::vector< Identifier >)
{
  mNbr_cellID = 0;
  mNbr_thetaID = 0;
  mNbr_phiID = 0;
}

EclNbr::EclNbr(const EclNbr& aNbr) :
  m_nbrs(*new std::vector< Identifier > (aNbr.m_nbrs)) ,
  m_nearSize(aNbr.m_nearSize)
{
  mNbr_cellID = 0;
  mNbr_thetaID = 0;
  mNbr_phiID = 0;
}

EclNbr::EclNbr(
  const std::vector< Identifier >&           aNbrs     ,
  const std::vector< Identifier >::size_type aNearSize
) :
  m_nbrs(*new std::vector< Identifier > (aNbrs)) ,
  m_nearSize(aNearSize)
{
  // sort vector separately for near, nxt-near nbrs
  std::sort(m_nbrs.begin() , m_nbrs.begin() + aNearSize , std::less< Identifier >()) ;
  std::sort(m_nbrs.begin() + aNearSize ,   m_nbrs.end() , std::less< Identifier >()) ;
}

EclNbr::~EclNbr()
{
  delete &m_nbrs ;
}
std::ostream& operator<<(std::ostream& os, const EclNbr& aNbr)
{
  os << "N(" ;
  unsigned short i(0) ;
  for (std::vector< EclNbr::Identifier >::const_iterator
       iNbr(aNbr.nbrs().begin()) ;
       iNbr != aNbr.nbrs().end() ; ++iNbr) {
    ++i;
    if (iNbr != aNbr.nbrs().begin() && i != aNbr.nearSize() + 1) os << "," ;
    if (i == aNbr.nearSize() + 1) os << "|" ;
    os << std::setw(4) << (*iNbr) ;
  }
  os << ")" ;
  return os ;
}


void EclNbr::printNbr()
{
  unsigned short Nri(0) ;
  cout << "(";
  for (std::vector< EclNbr::Identifier >::const_iterator
       iNbr(m_nbrs.begin()) ;
       iNbr != m_nbrs.end() ; ++iNbr) {
    ++Nri;
    if (iNbr != m_nbrs.begin() && Nri != m_nearSize + 1) cout << "," ;
    if (Nri == m_nearSize + 1) cout << "|" ;
    cout << std::setw(4) << (*iNbr) ;
  }
  cout << ")" << endl;
}
//
// assignment operators


EclNbr& EclNbr::operator=(const EclNbr& aNbr)
{
  if (this != &aNbr) {
    mNbr_cellID  = aNbr.mNbr_cellID;
    mNbr_thetaID = aNbr.mNbr_thetaID;
    mNbr_phiID   = aNbr.mNbr_phiID;

    m_nbrs     = aNbr.m_nbrs     ;
    m_nearSize = aNbr.m_nearSize ;
  }
  return *this ;
}

//
// member functions
//

//
// const member functions
//

const std::vector< EclNbr::Identifier >&
EclNbr::nbrs()      const
{
  return m_nbrs ;
}

const std::vector< EclNbr::Identifier >::const_iterator
EclNbr::nearBegin() const
{
  return m_nbrs.begin() ;
}

const std::vector< EclNbr::Identifier >::const_iterator
EclNbr::nearEnd()   const
{
  return m_nbrs.begin() + m_nearSize ;
}

const std::vector< EclNbr::Identifier >::const_iterator
EclNbr::nextBegin() const
{
  return m_nbrs.begin() + m_nearSize ;
}

const std::vector< EclNbr::Identifier >::const_iterator
EclNbr::nextEnd()   const
{
  return m_nbrs.end() ;
}

std::vector< EclNbr::Identifier >::size_type
EclNbr::nearSize()  const
{
  return m_nearSize ;
}

std::vector< EclNbr::Identifier >::size_type
EclNbr::nextSize()  const
{
  return (m_nbrs.size() - m_nearSize) ;
}


int EclNbr::GetCellID(int ThetaId, int PhiId)
{
  mNbr_cellID = Mapping_t::CellID(ThetaId, PhiId);
  mNbr_thetaID = ThetaId;
  mNbr_phiID =  PhiId ;
  return mNbr_cellID;
}

void EclNbr::Mapping(int cid)
{
  mNbr_cellID = cid;
  Mapping_t::Mapping(mNbr_cellID, mNbr_thetaID, mNbr_phiID);
}

EclNbr
EclNbr::getNbr(const Identifier aCellId)
{
  // generate nbr lists. always easier here to work with theta-phi

  const int cellID = aCellId;
  Mapping(cellID);
  const int thetaId = GetThetaID();
  const int phiId = GetPhiID();
  std::vector< EclNbr::Identifier >::size_type nearSize(0);
  std::vector< EclNbr::Identifier > vNbr;

  vNbr.reserve(24) ;   // except for extreme endcaps, always 24

  int t00 = thetaId;
  int tm1 = thetaId - 1;
  int tm2 = thetaId - 2;
  int tp1 = thetaId + 1;
  int tp2 = thetaId + 2;

  if (aCellId > 1151 && aCellId < 7776) {
    // barrel
    //
    //   12 13 14 15 16      ^ theta
    //   11  2  3  4 17      |
    //   10  1  0  5 18      +--> phi    X--+    view from inside
    //    9  8  7  6 19                     |    (foot pointing e- dir)
    //   24 23 22 21 20                     Z
    int f00 = phiId;
    int fm1 = (phiId + 143) % 144;
    int fp1 = (phiId + 1) % 144;
    int fm2 = (phiId + 142) % 144;
    int fp2 = (phiId + 2) % 144;

    vNbr.push_back(GetCellID(t00 , fm1));
    vNbr.push_back(GetCellID(tp1 , fm1));
    vNbr.push_back(GetCellID(tp1 , f00));
    vNbr.push_back(GetCellID(tp1 , fp1));
    vNbr.push_back(GetCellID(t00 , fp1));
    vNbr.push_back(GetCellID(tm1 , fp1));
    vNbr.push_back(GetCellID(tm1 , f00));
    vNbr.push_back(GetCellID(tm1 , fm1));

    nearSize = vNbr.size();

    vNbr.push_back(GetCellID(tm1 , fm2));
    vNbr.push_back(GetCellID(t00 , fm2));
    vNbr.push_back(GetCellID(tp1 , fm2));
    vNbr.push_back(GetCellID(tp2 , fm2));
    vNbr.push_back(GetCellID(tp2 , fm1));
    vNbr.push_back(GetCellID(tp2 , f00));
    vNbr.push_back(GetCellID(tp2 , fp1));
    vNbr.push_back(GetCellID(tp2 , fp2));
    vNbr.push_back(GetCellID(tp1 , fp2));
    vNbr.push_back(GetCellID(t00 , fp2));
    vNbr.push_back(GetCellID(tm1 , fp2));
    vNbr.push_back(GetCellID(tm2 , fp2));
    vNbr.push_back(GetCellID(tm2 , fp1));
    vNbr.push_back(GetCellID(tm2 , f00));
    vNbr.push_back(GetCellID(tm2 , fm1));
    vNbr.push_back(GetCellID(tm2 , fm2));
  }//if( aCellId > 1152 && aCellId < 7777 )
  else {
    // endcap -- not always 24!
    int n00 = 1000;
    int np1 = 1000;
    int np2 = 1000;
    int nm1 = 1000;
    int nm2 = 1000;
    if (aCellId < 1153) { // forward
      const EclIdentifier mPerRingForward[]
        = { 48, 48, 64, 64, 64, 96, 96, 96, 96, 96, 96, 144, 144, 144, 144 };
      if (thetaId > 1) nm2 = mPerRingForward[ thetaId - 2 ];
      if (thetaId > 0) nm1 = mPerRingForward[ thetaId - 1 ];
      n00 = mPerRingForward[ thetaId     ];
      np1 = mPerRingForward[ thetaId + 1 ];
      np2 = mPerRingForward[ thetaId + 2 ];
    } else { // backward
      const EclIdentifier mPerRingBackward[]
        = { 64, 64, 64, 96, 96, 96, 96, 96, 144, 144, 144, 144 };
      if (thetaId < 67) np2 = mPerRingBackward[ 66 - thetaId ];
      if (thetaId < 68) np1 = mPerRingBackward[ 67 - thetaId ];
      n00 = mPerRingBackward[ 68 - thetaId ];
      nm1 = mPerRingBackward[ 69 - thetaId ];
      nm2 = mPerRingBackward[ 70 - thetaId ];
    }
    // f-- are phi's, t-- are thetas
    // all calculations should be integer arith - pcs
    // f(th,phi)
    // criterion: center -> next bin
    int f0000 = phiId;
    int fp100 = (f0000 * np1 + np1 / 2) / n00;
    int fp200 = (f0000 * np2 + np2 / 2) / n00;
    int fm100 = (f0000 * nm1 + nm1 / 2) / n00;
    int fm200 = (f0000 * nm2 + nm2 / 2) / n00;

    int f00m1 = (f0000 + n00 - 1) % n00;  // should be exact
    int f00m2 = (f0000 + n00 - 2) % n00;
    int f00p1 = (f0000 + 1) % n00;
    int f00p2 = (f0000 + 2) % n00;

    int fp1m1 = (fp100 + np1 - 1) % np1;
    int fp1m2 = (fp100 + np1 - 2) % np1;
    int fp1p1 = (fp100 + 1) % np1;
    int fp1p2 = (fp100 + 2) % np1;

    int fm1m1 = (fm100 + nm1 - 1) % nm1;
    int fm1m2 = (fm100 + nm1 - 2) % nm1;
    int fm1p1 = (fm100 + 1) % nm1;
    int fm1p2 = (fm100 + 2) % nm1;

    int fp2m1 = (fp200 + np2 - 1) % np2;
    int fp2m2 = (fp200 + np2 - 2) % np2;
    int fp2p1 = (fp200 + 1) % np2;
    int fp2p2 = (fp200 + 2) % np2;

    int fm2m1 = (fm200 + nm2 - 1) % nm2;
    int fm2m2 = (fm200 + nm2 - 2) % nm2;
    int fm2p1 = (fm200 + 1) % nm2;
    int fm2p2 = (fm200 + 2) % nm2;
    int delta = n00 / 16;
//     int sector = phiId/delta;  // 0..15
    int nth = phiId % delta;

    switch (thetaId) {
      case 0:
        if (nth == 1)
          fp2p2 = 1000;
        break;
      case 1:
        if (nth == 1) {
          fp2p2 = 1000;
          fp1p2 = fp1p1;
          fp1p1 = 1000;
        }
        break;
      case 2:
        if ((nth == 0) || (nth == 1)) {
          fm2p2 = 1000;
          fm1p2 = fm1p1;
          fm1p1 = 1000;
        } else if ((nth == 2) || (nth == 3)) {
          fm2m2 = 1000;
          fm1m2 = fm1m1;
          fm1m1 = 1000;
        }
        break;
      case 3:
        if ((nth == 0) || (nth == 3)) {
          fm2p2 = fm2m2 = 1000;
        } else if (nth == 1) {
          fm2p2 = 1000;
        } else if (nth == 2) {
          fm2m2 = 1000;
        }
        break;
      case 5:
        if ((nth == 2) || (nth == 5)) {
          fm2m2 = 1000;
          fm1m2 = fm1m1;
          fm1m1 = 1000;
        } else {
          fm2p2 = 1000;
          fm1p2 = fm1p1;
          fm1p1 = 1000;
        }
        break;
      case 6:
        fm2p2 = 1000;
        if ((nth == 0) || (nth == 2) || (nth == 3) || (nth == 5)) {
          fm2m2 = 1000;
        }
        break;
      case 11:
        if ((nth == 2) || (nth == 5) || (nth == 8)) {
          fm2m2 = 1000;
          fm1m2 = fm1m1;
          fm1m1 = 1000;
        } else {
          fm2p2 = 1000;
          fm1p2 = fm1p1;
          fm1p1 = 1000;
        }
        break;
      case 12:
        fm2p2 = 1000;
        if ((nth == 0) || (nth == 2) || (nth == 3)
            || (nth == 5) || (nth == 6) || (nth == 8))
          fm2m2 = 1000;
        break;
      case 65:
        if ((nth == 2) || (nth == 5)) {
          fp2m2 = 1000;
          fp1m2 = fp1m1;
          fp1m1 = 1000;
        } else {
          fp2p2 = 1000;
          fp1p2 = fp1p1;
          fp1p1 = 1000;
        }
        break;
      case 64:
        fp2p2 = 1000;
        if ((nth ==  0) || (nth == 2) || (nth == 3) || (nth == 5))
          fp2m2 = 1000;
        break;
      case 60:
        if ((nth == 2) || (nth == 5) || (nth == 8)) {
          fp2m2 = 1000;
          fp1m2 = fp1m1;
          fp1m1 = 1000;
        } else {
          fp2p2 = 1000;
          fp1p2 = fp1p1;
          fp1p1 = 1000;
        }
        break;
      case 59:
        fp2p2 = 1000;
        if ((nth ==  0) || (nth == 2) || (nth == 3) || (nth == 5)
            || (nth == 6) || (nth == 8))
          fp2m2 = 1000;
        break;
    }//switch

    // insert near-nbrs
    vNbr.push_back(GetCellID(t00, f00m1));
    vNbr.push_back(GetCellID(t00, f00p1));
    if (nm1 < 999) {
      vNbr.push_back(GetCellID(tm1 , fm100));
      if (fm1m1 < 999)
        vNbr.push_back(GetCellID(tm1 , fm1m1));
      if (fm1p1 < 999)
        vNbr.push_back(GetCellID(tm1 , fm1p1));
    }
    if (np1 < 999) {
      vNbr.push_back(GetCellID(tp1 , fp100));
      if (fp1m1 < 999)
        vNbr.push_back(GetCellID(tp1 , fp1m1));
      if (fp1p1 < 999)
        vNbr.push_back(GetCellID(tp1 , fp1p1));
    }
    nearSize = vNbr.size() ;

    // now on to next-near neighbors
    if (nm2 < 999) {
      vNbr.push_back(GetCellID(tm2 , fm200));
      if (fm2m1 < 999)
        vNbr.push_back(GetCellID(tm2 , fm2m1));
      if (fm2p1 < 999)
        vNbr.push_back(GetCellID(tm2 , fm2p1));
      if (fm2m2 < 999)
        vNbr.push_back(GetCellID(tm2 , fm2m2));
      if (fm2p2 < 999)
        vNbr.push_back(GetCellID(tm2 , fm2p2));
    }
    if (nm1 < 999) {
      if (fm1m2 < 999)
        vNbr.push_back(GetCellID(tm1 , fm1m2));
      if (fm1p2 < 999)
        vNbr.push_back(GetCellID(tm1 , fm1p2));
    }
    vNbr.push_back(GetCellID(t00 , f00m2));
    vNbr.push_back(GetCellID(t00 , f00p2));
    if (np1 < 999) {
      if (fp1m2 < 999)
        vNbr.push_back(GetCellID(tp1, fp1m2));
      if (fp1p2 < 999)
        vNbr.push_back(GetCellID(tp1, fp1p2));
    }
    if (np2 < 999) {
      vNbr.push_back(GetCellID(tp2, fp200));
      if (fp2m1 < 999)
        vNbr.push_back(GetCellID(tp2, fp2m1));
      if (fp2p1 < 999)
        vNbr.push_back(GetCellID(tp2, fp2p1));
      if (fp2m2 < 999)
        vNbr.push_back(GetCellID(tp2, fp2m2));
      if (fp2p2 < 999)
        vNbr.push_back(GetCellID(tp2, fp2p2));
    }
  }//else( aCellId > 1152 && aCellId < 7777 )
  return
    EclNbr(vNbr, nearSize);
}
