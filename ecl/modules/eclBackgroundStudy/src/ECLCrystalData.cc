
//This module
#include <ecl/modules/eclBackgroundStudy/ECLCrystalData.h>

//Framework
#include <framework/logging/Logger.h>

//STL
#include <cmath>
#define PI 3.14159265358979323846
#define DEGTORAD 1.74532925199432955e-02

using namespace Belle2;

//ECL has 16-fold symmetry in phi. Multiply this by 16 to get the total number of crystals at each theta location.
int const ECLCrystalData::Ring[69] = {
  3, 3, 4, 4, 4, 6, 6, 6, 6, 6, 6, 9, 9,            //forward calorimeter
  9, 9, 9, 9, 9 , 9, 9, 9, 9, 9 , 9, 9, 9, 9, 9 , 9, 9, 9, 9, 9, //barrel
  9, 9, 9, 9, 9 , 9, 9, 9, 9, 9 , 9, 9, 9, 9, 9 , 9, 9, 9, 9, 9, //barrel
  9, 9, 9, 9, 9 , 9,                                //barrel
  9, 9, 6, 6, 6, 6, 6, 4, 4, 4                      //backward calorimeter
};

int const ECLCrystalData::sumPrevious[24] = {0, 3, 6, 10, 14, 18, 24, 30, 36, 42, 48, 54, 63, 72, 81, 90, 96, 102, 108, 114, 120, 124, 128, 132};

//endcap crystal masses
float const ECLCrystalData::CrystalMassEndcap[132] = {4.99, 5.01, 4.99, 5.82, 5.94, 5.83, 4.75, 4.76, 4.77, 4.74, 5.14, 5.16, 5.16,
                                                      5.14, 5.76, 5.8, 5.79, 5.78, 4.14, 4.28, 4.15, 4.16, 4.28, 4.14, 4.32, 4.34,
                                                      4.34, 4.34, 4.34, 4.32, 4.85, 4.87, 4.87, 4.87, 4.87, 4.85, 4.99, 5.02, 5.02,
                                                      5.02, 5.02, 4.99, 5.37, 5.39, 5.39, 5.39, 5.39, 5.37, 5.89, 5.92, 5.92, 5.92,
                                                      5.92, 5.9, 4.03, 4.12, 4.05, 4.05, 4.12, 4.05, 4.05, 4.12, 4.03, 4.16, 4.18,
                                                      4.18, 4.18, 4.18, 4.18, 4.18, 4.18, 4.16, 4.47, 4.49, 4.49, 4.49, 4.49, 4.49,
                                                      4.49, 4.49, 4.47, 4.26, 4.34, 4.28, 4.28, 4.34, 4.28, 4.28, 4.34, 4.26, 5.88,
                                                      5.91, 5.91, 5.91, 5.91, 5.88, 5.46, 5.48, 5.48, 5.48, 5.48, 5.46, 5.02, 5.04,
                                                      5.04, 5.04, 5.04, 5.02, 4.63, 4.65, 4.65, 4.65, 4.65, 4.63, 4.4, 4.53, 4.42,
                                                      4.42, 4.53, 4.4, 5.74, 5.77, 5.76, 5.74, 5.09, 5.11, 5.11, 5.09, 4.56, 4.58,
                                                      4.58, 4.56
                                                     };

//Barrel crystal masses
float const ECLCrystalData::CrystalMassBarrel[46] = {4.599, 4.619, 4.639, 4.659, 4.680, 4.701, 4.722, 4.744, 4.765,
                                                     4.786, 4.807, 4.828, 4.848, 4.868, 4.887, 4.906, 4.923, 4.940,
                                                     4.955, 4.969, 4.981, 4.992, 5.001, 5.008, 5.013, 5.016, 5.017,
                                                     5.016, 4.455, 4.455, 5.016, 5.017, 5.016, 5.013, 5.008, 5.001,
                                                     4.992, 4.981, 4.969, 4.955, 4.940, 4.923, 4.906, 4.887, 4.868,
                                                     4.848
                                                    };


//Theta ID to theta(deg)
double const ECLCrystalData::theta[69] = {13.9486666667, 15.4862, 17.110575, 18.757625, 20.255425, 21.8637333333,
                                          23.4259666667, 24.96125, 26.4777833333, 27.9407, 29.36275, 30.8382555556,
                                          32.2618444444, 33.667039, 35.062886, 36.509337, 38.007345, 39.557769,
                                          41.161373, 42.818633, 44.530027, 46.295838, 48.115876, 49.990235, 51.91882,
                                          53.900865, 55.935415, 58.021324, 60.157158, 62.3412, 64.571442, 66.8455,
                                          69.160698, 71.513983, 73.902011, 76.321253, 78.767888, 81.237718, 83.726351,
                                          86.229301, 88.741891, 90, 90, 91.258109, 93.770699, 96.273649, 98.762282,
                                          101.232112, 103.678747, 106.097989, 108.486017, 110.839302, 113.154501,
                                          115.428558, 117.658801, 119.842842, 121.978676, 124.064585, 126.099135,
                                          128.2902222222, 130.4424444444, 132.6918333333, 134.9551666667, 137.3811666667,
                                          139.9081666667, 142.5951666667, 145.374, 148.0185, 150.8355
                                         };

/** EndCapCrystalID to radius (spherical) */
double const ECLCrystalData::EndcapRadius[132] = {
// 3,3,4,4,4,6,6,6,6,6,6,9,9,                        //forward calorimeter
  217.25, 217.25, 217.25,
  218.72, 218.72, 218.72,
  220.29, 220.29, 220.29, 220.29,
  222.00, 222.00, 222.00, 222.00,
  223.96, 223.96, 223.96, 223.96,
  226.01, 226.01, 226.01, 226.01, 226.01, 226.01,
  228.21, 228.21, 228.21, 228.21, 228.21, 228.21,
  230.67, 230.67, 230.67, 230.67, 230.67, 230.67,
  233.20, 233.20, 233.20, 233.20, 233.20, 233.20,
  235.89, 235.89, 235.89, 235.89, 235.89, 235.89,
  238.89, 238.89, 238.89, 238.89, 238.89, 238.89,
  241.94, 241.94, 241.94, 241.94, 241.94, 241.94, 241.94, 241.94, 241.94,
  245.16, 245.16, 245.16, 245.16, 245.16, 245.16, 245.16, 245.16, 245.16,
// 9,9,6,6,6,6,6,4,4,4                               //backward calorimeter
  172.06, 172.06, 172.06, 172.06, 172.06, 172.06, 172.06, 172.06, 172.06,
  165.69, 165.69, 165.69, 165.69, 165.69, 165.69, 165.69, 165.69, 165.69,
  159.78, 159.78, 159.78, 159.78, 159.78, 159.78,
  154.36, 154.36, 154.36, 154.36, 154.36, 154.36,
  149.37, 149.37, 149.37, 149.37, 149.37, 149.37,
  144.82, 144.82, 144.82, 144.82, 144.82, 144.82,
  140.67, 140.67, 140.67, 140.67, 140.67, 140.67,
  136.89, 136.89, 136.89, 136.89,
  133.49, 133.49, 133.49, 133.49,
  133.49, 133.49, 133.49, 133.49
};


/**BarrelCrystalID to Z */
double const ECLCrystalData::BarrelZ[46] = {210.15, 200.14, 190.44, 181.05, 171.95, 163.03, 154.45, 146.10, 137.98, 129.99, 122.27, 114.72, 107.34, 100.05, 92.96, 86.00, 79.15, 72.36, 65.72, 59.16, 52.69, 46.22, 39.87, 33.56, 27.29, 21.01, 14.81, 8.62, 2.78, -2.78, -14.81, -21.01, -27.29, -33.56, -39.87, -46.22, -52.69, -59.16, -65.72, -72.36, -79.15, -86.00, -92.95, -100.05, -107 - 28};




///Constructors and destructors
ECLCrystalData::ECLCrystalData()
{
  m_cell_ID = -1;
  m_phi_ID = -1;
  m_theta_ID = -1;
  m_phi_idx = -1;
  m_theta_idx = -1;

  m_mass = -1;
  m_volume = -1;
  m_PosR = -1;
  m_PosP = -1;
  m_PosX = -1;
  m_PosY = -1;
  m_PosZ = -1;
}

ECLCrystalData::ECLCrystalData(int cid)
{
  m_cell_ID = cid;
  Mapping(cid);

  Eval();

}

ECLCrystalData::ECLCrystalData(int tid, int pid)
{
  m_phi_idx = -1;
  m_theta_idx = -1;
  m_theta_ID = tid;
  m_phi_ID   = pid;
  m_cell_ID  = GetCellID(tid, pid);

  Eval();
}

ECLCrystalData::~ECLCrystalData()
{
}


///Evaluate all properties from theta_ID and phi_ID
void ECLCrystalData::Eval()
{
  //Crystal properties
  m_mass = EvalMass();
  m_volume = m_mass / 4.51;

  //Crystal Location
  m_PosP = EvalPhi();
  m_PosR = EvalR();

  m_PosZ = EvalZ();
  m_PosX = EvalX();
  m_PosY = EvalY();


}

double ECLCrystalData::EvalX()
{
  return m_PosR * cos(DEGTORAD * m_PosP);
}

double ECLCrystalData::EvalY()
{
  return m_PosR * sin(DEGTORAD * m_PosP);
}


//Determine mass of crystal
float ECLCrystalData::EvalMass()
{
  if (IsEndCap()) {
    return CrystalMassEndcap[GetCrystalIndex()];

  } else {
    return CrystalMassBarrel[GetCrystalIndex()];
  }
}

int ECLCrystalData::GetCrystalIndex()
{
  if (m_theta_ID <= 12) {                     //forward ECL crystal
    return m_phi_ID % Ring[m_theta_ID] + sumPrevious[m_theta_ID];

  } else if (m_theta_ID >= 59) {               //backward ECL crystal
    return  m_phi_ID % Ring[m_theta_ID] + sumPrevious[m_theta_ID - 46];

  } else if (12 < m_theta_ID  && m_theta_ID < 59) { //barrel ECL crystal
    return m_theta_ID - 13;
  }

  B2INFO("ERROR: invalid theta_ID=" << m_theta_ID);
  return -1;
}

inline bool ECLCrystalData::IsEndCap()
{
  return m_theta_ID < 13 || m_theta_ID > 58 ;
}

inline bool ECLCrystalData::IsBarrel()
{
  return !IsEndCap();
}


inline int ECLCrystalData::GetCellID()
{
  return m_cell_ID;
}

double ECLCrystalData::EvalPhi()
{
  return 22.5 * m_phi_ID / Ring[m_theta_ID];
}

double ECLCrystalData::EvalZ()
{
  if (IsEndCap()) {
    return EndcapRadius[GetCrystalIndex()] * cos(DEGTORAD * theta[m_theta_ID]);
  }

  return BarrelZ[GetCrystalIndex()];
}

double ECLCrystalData::EvalR()
{
  if (IsEndCap()) {
    return EndcapRadius[GetCrystalIndex()] * sin(DEGTORAD * theta[m_theta_ID]);
  }

  return 147.7;
}


//Get CellID from theta and phi IDs
int ECLCrystalData::GetCellID(int ThetaId, int PhiId)
{
  /// 0-12  forward
  /// 13-58 barrel
  /// 59-68 backward
  if (ThetaId < 13) {
    int forwRing[13] = {0, 3, 6, 10, 14, 18, 24, 30, 36, 42, 48, 54, 63 };
    return forwRing[ThetaId] * 16 + PhiId;

  } else if (ThetaId > 58) {
    int backRing[10] = {0, 9, 18, 24, 30, 36, 42, 48, 52, 56} ;
    return 7776 + backRing[ThetaId - 59] * 16 + PhiId;

  } else if (ThetaId > 12 && ThetaId < 59) {
    return 1152 + 144 * (ThetaId - 13)  + PhiId;

  } else
    B2INFO("ERROR (ECLCrystalData): CellID int ThetaId " << ThetaId << " int PhiId " << PhiId << ". Out of range.");

  return -1;
}

void ECLCrystalData::Mapping(int cid)
{
  if (cid < 0) {
    B2INFO("ECL ECLGeometryPar Mapping  " << cid << ". Out of range.");

  } else if (cid < 3 * 16) { //Forkward start
    m_theta_ID = 0;
    m_phi_ID = cid;
    m_phi_idx = m_phi_ID % 3;
    m_theta_idx = m_phi_ID / 3;
  } else if (cid < 6 * 16) {
    m_theta_ID = 1;
    m_phi_ID = cid - 3 * 16;
    m_phi_idx = m_phi_ID % 3 + 3;
    m_theta_idx = m_phi_ID / 3;
  } else if (cid < 10 * 16) {
    m_theta_ID = 2;
    m_phi_ID = cid - 6 * 16 ;
    m_phi_idx = m_phi_ID % 4 + 6;
    m_theta_idx = m_phi_ID / 4;
  } else if (cid < 14 * 16) {
    m_theta_ID = 3;
    m_phi_ID = cid - 10 * 16 ;
    m_phi_idx = m_phi_ID % 4 + 10;
    m_theta_idx = m_phi_ID / 4;
  } else if (cid < 18 * 16) {
    m_theta_ID = 4;
    m_phi_ID = cid - 14 * 16 ;
    m_phi_idx = m_phi_ID % 4 + 14;
    m_theta_idx = m_phi_ID / 4;
  } else if (cid < 24 * 16) {
    m_theta_ID = 5;
    m_phi_ID = cid - 18 * 16 ;
    m_phi_idx = m_phi_ID % 6 + 18;
    m_theta_idx = m_phi_ID / 6;
  } else if (cid < 30 * 16) {
    m_theta_ID = 6;
    m_phi_ID = cid - 24 * 16 ;
    m_phi_idx = m_phi_ID % 6 + 24;
    m_theta_idx = m_phi_ID / 6;
  } else if (cid < 36 * 16) {
    m_theta_ID = 7;
    m_phi_ID = cid - 30 * 16 ;
    m_phi_idx = m_phi_ID % 6 + 30;
    m_theta_idx = m_phi_ID / 6;
  } else if (cid < 42 * 16) {
    m_theta_ID = 8;
    m_phi_ID = cid - 36 * 16 ;
    m_phi_idx = m_phi_ID % 6 + 36;
    m_theta_idx = m_phi_ID / 6;
  } else if (cid < 48 * 16) {
    m_theta_ID = 9;
    m_phi_ID = cid - 42 * 16 ;
    m_phi_idx = m_phi_ID % 6 + 42;
    m_theta_idx = m_phi_ID / 6;
  } else if (cid < 54 * 16) {
    m_theta_ID = 10;
    m_phi_ID = cid - 48 * 16 ;
    m_phi_idx = m_phi_ID % 6 + 48;
    m_theta_idx = m_phi_ID / 6;
  } else if (cid < 63 * 16) {
    m_theta_ID = 11;
    m_phi_ID = cid - 54 * 16 ;
    m_phi_idx = m_phi_ID % 9 + 54;
    m_theta_idx = m_phi_ID / 9;
  } else if (cid < 72 * 16) {
    m_theta_ID = 12;
    m_phi_ID = cid - 63 * 16 ;
    m_phi_idx = m_phi_ID % 9 + 63;
    m_theta_idx = m_phi_ID / 9;
  } else if (cid < 7776) {//Barrel start
    m_phi_ID = (cid - 1152) % 144;
    m_theta_ID = (cid - 1152) / 144 + 13;
    m_theta_idx = (cid - 1152) / 144;
    m_phi_idx = m_phi_ID;
  } else if (cid < 7776 + 9 * 16) { //Backward start
    m_theta_ID = 59;
    m_phi_ID =  cid - 7776 ;
    m_phi_idx = m_phi_ID % 9 + 72;
    m_theta_idx = m_phi_ID / 9;
  } else if (cid < 7776 + 18 * 16) {
    m_theta_ID = 60;
    m_phi_ID =   cid - 7776 - 9 * 16 ;
    m_phi_idx = m_phi_ID % 9 + 81;
    m_theta_idx = m_phi_ID / 9;
  } else if (cid < 7776 + 24 * 16) {
    m_theta_ID = 61;
    m_phi_ID =   cid - 7776 - 18 * 16 ;
    m_phi_idx = m_phi_ID % 6 + 90;
    m_theta_idx = m_phi_ID / 6;
  } else if (cid < 7776 + 30 * 16) {
    m_theta_ID = 62;
    m_phi_ID =   cid - 7776 - 24 * 16 ;
    m_phi_idx = m_phi_ID % 6 + 96;
    m_theta_idx = m_phi_ID / 6;
  } else if (cid < 7776 + 36 * 16) {
    m_theta_ID = 63;
    m_phi_ID =   cid - 7776 - 30 * 16 ;
    m_phi_idx = m_phi_ID % 6 + 102;
    m_theta_idx = m_phi_ID / 6;
  } else if (cid < 7776 + 42 * 16) {
    m_theta_ID = 64;
    m_phi_ID =   cid - 7776 - 36 * 16 ;
    m_phi_idx = m_phi_ID % 6 + 108;
    m_theta_idx = m_phi_ID / 6;
  } else if (cid < 7776 + 48 * 16) {
    m_theta_ID = 65;
    m_phi_ID =   cid - 7776 - 42 * 16 ;
    m_phi_idx = m_phi_ID % 6 + 114;
    m_theta_idx = m_phi_ID / 6;
  } else if (cid < 7776 + 52 * 16) {
    m_theta_ID = 66;
    m_phi_ID =   cid - 7776 - 48 * 16 ;
    m_phi_idx = m_phi_ID % 4 + 120;
    m_theta_idx = m_phi_ID / 4;
  } else if (cid < 7776 + 56 * 16) {
    m_theta_ID = 67;
    m_phi_ID =   cid - 7776 - 52 * 16 ;
    m_phi_idx = m_phi_ID % 4 + 124;
    m_theta_idx = m_phi_ID / 4;
  } else if (cid < 7776 + 60 * 16) {
    m_theta_ID = 68;
    m_phi_ID =   cid - 7776 - 56 * 16 ;
    m_phi_idx = m_phi_ID % 4 + 128;
    m_theta_idx  = m_phi_ID / 4;
  } else {
    B2INFO("ECL ECLCrystalData Mapping  " << cid << ". Out of range.");
  }
}
