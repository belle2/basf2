//-----------------------------------------------------------------------------
// $Id: TRunge.cc 10129 2007-05-18 12:44:41Z katayama $
//-----------------------------------------------------------------------------
// Filename : TRunge.cc
// Section  : Tracking
// Owner    : Kenji Inami
// Email    : inami@bmail.kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a track using Runge Kutta method
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.3  2003/12/25 12:03:34  yiwasaki
// Trasan 3.07 : small cell cdc available in the curl finder; many minor modifications not to produce warning message in compiling
//
// Revision 1.2  2001/12/13 15:31:59  katayama
// MDST_OBS
//
// Revision 1.1  2001/08/02 07:04:17  yiwasaki
// RK fitter from K.Inami
//
//
//-----------------------------------------------------------------------------
#include <cfloat>
#include <cstring>




#include "tracking/modules/trasan/TRunge.h"
#include "tracking/modules/trasan/TRungeFitter.h"
#include "trg/cdc/Wire.h"
#include "tracking/modules/trasan/TTrack.h"

namespace Belle {

  extern const HepGeom::Point3D<double>  ORIGIN;

  const double alpha2 = 333.5640952;  //(cm Tesla /(GeV/c))

  const TRungeFitter TRunge::_fitter = TRungeFitter("TRunge Default fitter");

//const double default_stepSize = 0;
  const double default_stepSize = 0.05; //0.5mm
  const double default_stepSize0 = 0.05;  //0.5mm
  const double default_stepSizeMax = 0.5; //5.0mm
  const double default_stepSizeMin = 0.001;//0.01mm

  const double EPS = 1.0e-6;

  const double default_maxflightlength = 1000; //10m

  TRunge::TRunge()
    : TTrackBase(),
      _pivot(ORIGIN),
      _a(CLHEP::HepVector(5, 0)),
      _Ea(CLHEP::HepSymMatrix(5, 0)),
      _chi2(0),
      _ndf(0),
      _bfieldID(21),
      _maxflightlength(default_maxflightlength),
      _stepSize(default_stepSize),
      _eps(EPS),
      _stepSizeMax(default_stepSizeMax),
      _stepSizeMin(default_stepSizeMin),
      _mass(0.140),
      _charge(1),
      _Nstep(0)
  {

    //...Set a default fitter...
    fitter(& TRunge::_fitter);

    _fitted = false;
    _fittedWithCathode = false;

    _bfield = Bfield::getBfield(_bfieldID);

    _mass2 = _mass * _mass;

    _yscal[0] = 0.1; //1mm  -> error = 1mm*EPS
    _yscal[1] = 0.1; //1mm
    _yscal[2] = 0.1; //1mm
    _yscal[3] = 0.001; //1MeV
    _yscal[4] = 0.001; //1MeV
    _yscal[5] = 0.001; //1MeV
  }

  TRunge::TRunge(const TTrack& a)
    : TTrackBase((TTrackBase&) a),
      _pivot(a.helix().pivot()),
      _a(a.helix().a()),
      _Ea(a.helix().Ea()),
      _chi2(0),
      _ndf(0),
      _bfieldID(21),
      _maxflightlength(default_maxflightlength),
      _stepSize(default_stepSize),
      _eps(EPS),
      _stepSizeMax(default_stepSizeMax),
      _stepSizeMin(default_stepSizeMin),
      _mass(0.140),
      _Nstep(0)
  {

    //...Set a default fitter...
    fitter(& TRunge::_fitter);

    _fitted = false;
    _fittedWithCathode = false;

    if (_a[2] < 0) _charge = -1;
    else        _charge = 1;

    _bfield = Bfield::getBfield(_bfieldID);

    _mass2 = _mass * _mass;

    _yscal[0] = 0.1; //1mm  -> error = 1mm*EPS
    _yscal[1] = 0.1; //1mm
    _yscal[2] = 0.1; //1mm
    _yscal[3] = 0.001; //1MeV
    _yscal[4] = 0.001; //1MeV
    _yscal[5] = 0.001; //1MeV

    //SetFlightLength();
    //std::cout<<"TR:: _maxflightlength="<<_maxflightlength<<std::endl;
  }

  TRunge::TRunge(const THelix& h)
    : TTrackBase(),
      _pivot(h.pivot()),
      _a(h.a()),
      _Ea(h.Ea()),
      _chi2(0),
      _ndf(0),
      _bfieldID(21),
      _maxflightlength(default_maxflightlength),
      _stepSize(default_stepSize),
      _eps(EPS),
      _stepSizeMax(default_stepSizeMax),
      _stepSizeMin(default_stepSizeMin),
      _mass(0.140),
      _Nstep(0)
  {

    //...Set a default fitter...
    fitter(& TRunge::_fitter);

    _fitted = false;
    _fittedWithCathode = false;

    if (_a[2] < 0) _charge = -1;
    else        _charge = 1;

    _bfield = Bfield::getBfield(_bfieldID);

    _mass2 = _mass * _mass;

    _yscal[0] = 0.1; //1mm  -> error = 1mm*EPS
    _yscal[1] = 0.1; //1mm
    _yscal[2] = 0.1; //1mm
    _yscal[3] = 0.001; //1MeV
    _yscal[4] = 0.001; //1MeV
    _yscal[5] = 0.001; //1MeV
  }

  TRunge::TRunge(const TRunge& a)
    : TTrackBase((TTrackBase&) a),
      _pivot(a.pivot()),
      _a(a.a()),
      _Ea(a.Ea()),
      _chi2(a.chi2()),
      _ndf(a.ndf()),
      _bfieldID(a.BfieldID()),
      _maxflightlength(a.MaxFlightLength()),
      _stepSize(a.StepSize()),
      _eps(a.Eps()),
      _stepSizeMax(a.StepSizeMax()),
      _stepSizeMin(a.StepSizeMin()),
      _mass(a.Mass()),
      _Nstep(0)
  {

    //...Set a default fitter...
    fitter(& TRunge::_fitter);

    _fitted = false;
    _fittedWithCathode = false;

    if (_a[2] < 0) _charge = -1;
    else        _charge = 1;

    _bfield = Bfield::getBfield(_bfieldID);

    _mass2 = _mass * _mass;

    for (unsigned i = 0; i < 6; i++) _yscal[i] = a.Yscal()[i];
  }

//destructor
  TRunge::~TRunge()
  {
  }

  double TRunge::dr(void) const
  {
    return(_a[0]);
  }

  double TRunge::phi0(void) const
  {
    return(_a[1]);
  }

  double TRunge::kappa(void) const
  {
    return(_a[2]);
  }

  double TRunge::dz(void) const
  {
    return(_a[3]);
  }

  double TRunge::tanl(void) const
  {
    return(_a[4]);
  }

  const Point3D& TRunge::pivot(void) const
  {
    return(_pivot);
  }

  const CLHEP::HepVector& TRunge::a(void) const
  {
    return(_a);
  }

  const CLHEP::HepSymMatrix& TRunge::Ea(void) const
  {
    return(_Ea);
  }

  THelix TRunge::helix(void) const
  {
    return(THelix(_pivot, _a, _Ea));
  }

  unsigned TRunge::ndf(void) const
  {
    return _ndf;
  }

  double TRunge::chi2(void) const
  {
    return _chi2;
  }

  double TRunge::reducedchi2(void) const
  {
    if (_ndf == 0) {
      std::cout << "error at TRunge::reducedchi2  ndf=0" << std::endl;
      return 0;
    }
    return (_chi2 / _ndf);
  }

  int TRunge::BfieldID(void) const
  {
    return(_bfieldID);
  }

  double TRunge::StepSize(void) const
  {
    return(_stepSize);
  }

  const double* TRunge::Yscal(void) const
  {
    return(_yscal);
  }
  double TRunge::Eps(void) const
  {
    return(_eps);
  }
  double TRunge::StepSizeMax(void) const
  {
    return(_stepSizeMax);
  }
  double TRunge::StepSizeMin(void) const
  {
    return(_stepSizeMin);
  }

  float TRunge::Mass(void) const
  {
    return(_mass);
  }

  double TRunge::MaxFlightLength(void) const
  {
    return(_maxflightlength);
  }

  const Point3D& TRunge::pivot(const Point3D& newpivot)
  {
    /// !!!!! under construction !!!!!
    ///   track parameter should be extracted after track propagation.
    THelix tTHelix(helix());
    tTHelix.pivot(newpivot);
    _pivot = newpivot;
    _a = tTHelix.a();
    _Ea = tTHelix.Ea();
    _Nstep = 0;
    return(_pivot);
  }

  const CLHEP::HepVector& TRunge::a(const CLHEP::HepVector& ta)
  {
    _a = ta;
    if (_a[2] < 0) _charge = -1;
    else        _charge = 1;
    _Nstep = 0;
    return(_a);
  }

  const CLHEP::HepSymMatrix& TRunge::Ea(const CLHEP::HepSymMatrix& tEa)
  {
    _Ea = tEa;
    _Nstep = 0;
    return(_Ea);
  }

  int TRunge::BfieldID(int id)
  {
    _bfieldID = id;
    _bfield = Bfield::getBfield(_bfieldID);
    _Nstep = 0;
    return(_bfieldID);
  }

  double TRunge::StepSize(double step)
  {
    _stepSize = step;
    _Nstep = 0;
    return(_stepSize);
  }

  const double* TRunge::Yscal(const double y[6])
  {
    for (unsigned i = 0; i < 6; i++) _yscal[i] = y[i];
    _Nstep = 0;
    return(_yscal);
  }
  double TRunge::Eps(double eps)
  {
    _eps = eps;
    _Nstep = 0;
    return(_eps);
  }
  double TRunge::StepSizeMax(double step)
  {
    _stepSizeMax = step;
    _Nstep = 0;
    return(_stepSizeMax);
  }
  double TRunge::StepSizeMin(double step)
  {
    _stepSizeMin = step;
    _Nstep = 0;
    return(_stepSizeMin);
  }

  float TRunge::Mass(float mass)
  {
    _mass = mass;
    _mass2 = _mass * _mass;
    return(_mass);
  }

  double TRunge::MaxFlightLength(double length)
  {
    if (length > 0) _maxflightlength = length;
    else _maxflightlength = default_maxflightlength;
    _Nstep = 0;
    return(_maxflightlength);
  }

  int TRunge::approach(TLink& l, bool doSagCorrection) const
  {
    float tof;
    HepGeom::Vector3D<double> p;
    return(approach(l, tof, p, doSagCorrection));
  }

  int TRunge::approach(TLink& l, float& tof, HepGeom::Vector3D<double> & p,
                       bool doSagCorrection) const
  {
//cnv  HepGeom::Point3D<double> xw;
//   HepGeom::Point3D<double> wireBackwardPosition;
//   HepGeom::Vector3D<double> v;
//   HepGeom::Point3D<double> onWire,onTrack;
//   double onWire_y, onWire_z, zwf, zwb;

//   const Belle2::TRGCDCWire& w=*l.wire();
//   xw = w.xyPosition();
//   wireBackwardPosition = w.backwardPosition();
//   v = w.direction();

//   unsigned stepNum=0;
//   if(approach_line(wireBackwardPosition,v,onWire,onTrack,tof,p,stepNum)<0){
//     //std::cout<<"TR::error approach_line"<<std::endl;
//     return(-1);
//   }

//   zwf = w.forwardPosition().z();
//   zwb = w.backwardPosition().z();
//   // protection for strange wire hit info. (Only 'onWire' is corrected)
//   if(onWire.z() > zwf)
//     w.wirePosition(zwf,onWire,wireBackwardPosition,(Vector3D&)v);
//   else if(onWire.z() < zwb)
//     w.wirePosition(zwb,onWire,wireBackwardPosition,(Vector3D&)v);

//   // onWire,onTrack filled

//   if(!doSagCorrection){
//     l.positionOnWire(onWire);
//     l.positionOnTrack(onTrack);
//     return(0);       // no sag correction
//   }
//   // Sag correction
//   //   loop for sag correction
//   onWire_y = onWire.y();
//   onWire_z = onWire.z();

//   unsigned nTrial = 1;
//   while(nTrial<100){
//     //std::cout<<"TR: nTrial "<<nTrial<<std::endl;
//     w.wirePosition(onWire_z,xw,wireBackwardPosition,(Vector3D&)v);
//     if(approach_line(wireBackwardPosition,v,onWire,onTrack,tof,p,stepNum)<0)
//       return(-1);
//     if(fabs(onWire_y - onWire.y())<0.0001) break;  // |dy|< 1 micron
//     onWire_y = onWire.y();
//     onWire_z += (onWire.z()-onWire_z)/2;
//     // protection for strange wire hit info.
//     if(onWire_z > zwf)      onWire_z=zwf;
//     else if(onWire_z < zwb) onWire_z=zwb;

//     nTrial++;
//   }
//   //  std::cout<<"TR  nTrial="<<nTrial<<std::endl;

//   l.positionOnWire(onWire);
//   l.positionOnTrack(onTrack);
//   return(nTrial);
    return 0;
  }

  int TRunge::approach_line(const Point3D& w0, const HepGeom::Vector3D<double> & v,
                            Point3D& onLine, Point3D& onTrack) const
  {
    float tof;
    HepGeom::Vector3D<double> p;
    return(approach_line(w0, v, onLine, onTrack, tof, p));
  }

  int TRunge::approach_line(const Point3D& w0, const HepGeom::Vector3D<double> & v,
                            Point3D& onLine, Point3D& onTrack,
                            float& tof, HepGeom::Vector3D<double> & p) const
  {
    unsigned stepNum = 0;
    return(approach_line(w0, v, onLine, onTrack, tof, p, stepNum));
  }

  int TRunge::approach_line(const Point3D& w0, const HepGeom::Vector3D<double> & v,
                            Point3D& onLine, Point3D& onTrack,
                            float& tof, HepGeom::Vector3D<double> & p, unsigned& stepNum) const
  {
    //  line = [w0] + t * [v]    -> [onLine]
    if (_Nstep == 0) {
      if (_stepSize == 0) Fly_SC();
      else Fly();
    }

    //std::cout<<"TR::approach_line stepNum="<<stepNum<<std::endl;

    const double w0x = w0.x();
    const double w0y = w0.y();
    const double w0z = w0.z();
    //std::cout<<"TR::line w0="<<w0x<<","<<w0y<<","<<w0z<<std::endl;
    const double vx = v.x();
    const double vy = v.y();
    const double vz = v.z();
    const double v_2 = vx * vx + vy * vy + vz * vz;

    const float clight = 29.9792458; //[cm/ns]
    //const float M2=_mass*_mass;
    //const float& M2=_mass2;
    const float p2 = _y[0][3] * _y[0][3] + _y[0][4] * _y[0][4] + _y[0][5] * _y[0][5];
    const float tof_factor = 1. / clight * sqrt(1 + _mass2 / p2);

    // search for the closest point in cache   (point - line)
    //  unsigned stepNum;
    double l2_old = DBL_MAX;
    if (stepNum > _Nstep) stepNum = 0;
    unsigned stepNumLo;
    unsigned stepNumHi;
    if (stepNum == 0 && _stepSize != 0) { // skip
      const double dx = _y[0][0] - w0x;
      const double dy = _y[0][1] - w0y;
      stepNum = (unsigned)
                (sqrt((dx * dx + dy * dy) * (1 + _a[4] * _a[4])) / _stepSize);
    }
    unsigned mergin;
    if (_stepSize == 0) {
      mergin = 10; //10 step back
    } else {
      mergin = (unsigned)(1.0 / _stepSize); //1mm back
    }
    if (stepNum > mergin) stepNum -= mergin;
    else           stepNum = 0;
    if (stepNum >= _Nstep) stepNum = _Nstep - 1;
    // hunt
    //  unsigned inc=1;
    unsigned inc = (mergin >> 1) + 1;
    stepNumLo = stepNum;
    stepNumHi = stepNum;
    for (;;) {
      const double dx = _y[stepNumHi][0] - w0x;
      const double dy = _y[stepNumHi][1] - w0y;
      const double dz = _y[stepNumHi][2] - w0z;
      const double t = (dx * vx + dy * vy + dz * vz) / v_2;
      const double l2 = (dx * dx + dy * dy + dz * dz) - (t * t) / v_2;
      if (l2 > l2_old) break;
      l2_old = l2;
      stepNumLo = stepNumHi;
      //    inc+=inc;
      stepNumHi += inc;
      if (stepNumHi >= _Nstep) {
        stepNumHi = _Nstep;
        break;
      }
    }
    // locate (2-bun-hou, bisection method)
    while (stepNumHi - stepNumLo > 1) {
      unsigned j = (stepNumHi + stepNumLo) >> 1;
      const double dx = _y[j][0] - w0x;
      const double dy = _y[j][1] - w0y;
      const double dz = _y[j][2] - w0z;
      const double t = (dx * vx + dy * vy + dz * vz) / v_2;
      const double l2 = (dx * dx + dy * dy + dz * dz) - (t * t) / v_2;
      if (l2 > l2_old) {
        stepNumHi = j;
      } else {
        l2_old = l2;
        stepNumLo = j;
      }
    }
    //stepNum=stepNumHi;
    stepNum = stepNumLo;
    /*
    for(;stepNum<_Nstep;stepNum++){
      const double dx = _y[stepNum][0]-w0x;
      const double dy = _y[stepNum][1]-w0y;
      const double dz = _y[stepNum][2]-w0z;
      const double t = (dx*vx+dy*vy+dz*vz)/v_2;
      const double l2 = (dx*dx+dy*dy+dz*dz)-(t*t)/v_2;
      if(l2 > l2_old) break;
      l2_old=l2;
      //    const float p2 = _y[stepNum][3]*_y[stepNum][3]+
      //                      _y[stepNum][4]*_y[stepNum][4]+
      //                      _y[stepNum][5]*_y[stepNum][5];
      //    tof+=_stepSize/clight*sqrt(1+M2/p2);
    }
    */
    //  std::cout<<"TR  stepNum="<<stepNum<<std::endl;
    //if(stepNum>=_Nstep) return(-1); // not found
    //stepNum--;
    if (_stepSize == 0) {
      double dstep = 0;
      for (unsigned i = 0; i < stepNum; i++) dstep += _h[i];
      tof = dstep * tof_factor;
    } else {
      tof = stepNum * _stepSize * tof_factor;
    }

    // propagate the track and search for the closest point
    //2-bun-hou (bisection method)
    /*
    double y[6],y_old[6];
    for(unsigned i=0;i<6;i++) y[i]=_y[stepNum][i];
    double step=_stepSize;
    double step2=0;
    for(;;){
      for(unsigned i=0;i<6;i++) y_old[i]=y[i];
      Propagate(y,step);
      const double dx = y[0]-w0x;
      const double dy = y[1]-w0y;
      const double dz = y[2]-w0z;
      const double t = (dx*vx+dy*vy+dz*vz)/v_2;
      const double l2 = (dx*dx+dy*dy+dz*dz)-(t*t)/v_2;
      if(l2 > l2_old){  // back
        for(unsigned i=0;i<6;i++) y[i]=y_old[i];
      }else{        // propagate
        l2_old=l2;
        //      const float p2=y[3]*y[3]+y[4]*y[4]+y[5]*y[5];
        //      tof+=step/clight*sqrt(1+M2/p2);
        step2+=step;
      }
      step/=2;
      if(step < 0.0001) break;  // step < 1 [um]
    }
    */
    // Hasamiuchi-Hou (false position method)
    /*
    double y[6],y1[6],y2[6];
    for(unsigned i=0;i<6;i++) y1[i]=_y[stepNum][i];
    for(unsigned i=0;i<6;i++) y2[i]=_y[stepNum+2][i];
    double minStep=0;
    double maxStep=_stepSize*2;
    double step2=0;
    const double A[3][3]={{1-vx*vx/v_2, -vx*vy/v_2, -vx*vz/v_2},
        { -vy*vx/v_2,1-vy*vy/v_2, -vy*vz/v_2},
        { -vz*vx/v_2, -vz*vy/v_2,1-vz*vz/v_2}};

    double Y1=0;
    {
      const double dx = y1[0]-w0x;
      const double dy = y1[1]-w0y;
      const double dz = y1[2]-w0z;
      const double t = (dx*vx+dy*vy+dz*vz)/v_2;
      const double d[3]={dx-t*vx,dy-t*vy,dz-t*vz};
      const double l = sqrt( (dx*dx+dy*dy+dz*dz)-(t*t)/v_2 );
      const double pmag=sqrt(y1[3]*y1[3]+y1[4]*y1[4]+y1[5]*y1[5]);
      for(int j=0;j<3;j++){
        double g=0;
        for(int k=0;k<3;k++) g+=A[j][k]*d[k];
        Y1+=y1[j+3]*g;
      }
      Y1=Y1/pmag/l;
    }
    double Y2=0;
    {
      const double dx = y2[0]-w0x;
      const double dy = y2[1]-w0y;
      const double dz = y2[2]-w0z;
      const double t = (dx*vx+dy*vy+dz*vz)/v_2;
      const double d[3]={dx-t*vx,dy-t*vy,dz-t*vz};
      const double l = sqrt( (dx*dx+dy*dy+dz*dz)-(t*t)/v_2 );
      const double pmag=sqrt(y2[3]*y2[3]+y2[4]*y2[4]+y2[5]*y2[5]);
      for(int j=0;j<3;j++){
        double g=0;
        for(int k=0;k<3;k++) g+=A[j][k]*d[k];
        Y2+=y2[j+3]*g;
      }
      Y2=Y2/pmag/l;
    }
    if(Y1*Y2>=0) std::cout<<"TR  fatal error!"<<std::endl;
    double step_old= DBL_MAX;
    for(;;){
      const double step=(minStep*Y2-maxStep*Y1)/(Y2-Y1);
      if(fabs(step-step_old)<0.0001) break;
      step_old=step;
      for(unsigned i=0;i<6;i++) y[i]=y1[i];
      Propagate(y,step-minStep);
      double Y=0;
      {
        const double dx = y[0]-w0x;
        const double dy = y[1]-w0y;
        const double dz = y[2]-w0z;
        const double t = (dx*vx+dy*vy+dz*vz)/v_2;
        const double d[3]={dx-t*vx,dy-t*vy,dz-t*vz};
        const double l = sqrt( (dx*dx+dy*dy+dz*dz)-(t*t)/v_2 );
        const double pmag=sqrt(y[3]*y[3]+y[4]*y[4]+y[5]*y[5]);
        for(int j=0;j<3;j++){
    double g=0;
    for(int k=0;k<3;k++) g+=A[j][k]*d[k];
    Y+=y[j+3]*g;
        }
        Y=Y/pmag/l;
      }
      if(Y1*Y<0){ //Y->Y2
        Y2=Y;
        for(unsigned i=0;i<6;i++) y2[i]=y[i];
        maxStep=step;
      }else{  //Y->Y1
        Y1=Y;
        for(unsigned i=0;i<6;i++) y1[i]=y[i];
        minStep=step;
      }
      if(Y1==Y2) break;
    }
    step2=step_old;
    */
    // Newton method
    double y[6];
    for (unsigned i = 0; i < 6; i++) y[i] = _y[stepNum][i];
    //memcpy(y,_y[stepNum],sizeof(double)*6);
    double step2 = 0;
    const double A[3][3] = {{1 - vx* vx / v_2, -vx* vy / v_2, -vx* vz / v_2},
      { -vy* vx / v_2, 1 - vy* vy / v_2, -vy* vz / v_2},
      { -vz* vx / v_2, -vz* vy / v_2, 1 - vz* vz / v_2}
    };
    double factor = 1;
    double g[3];
    double f[6];
    double Af[3], Af2[3];
    unsigned i, j, k;
    for (i = 0; i < 10; i++) {
      //std::cout<<"TR::line "<<y[0]<<","<<y[1]<<","<<y[2]<<","<<y[3]<<","<<y[4]<<","<<y[5]<<std::endl;
      const double dx = y[0] - w0x;
      const double dy = y[1] - w0y;
      const double dz = y[2] - w0z;
      const double t = (dx * vx + dy * vy + dz * vz) / v_2;
      const double d[3] = {dx - t* vx, dy - t* vy, dz - t* vz};
      const double l2 = (dx * dx + dy * dy + dz * dz) - (t * t) / v_2;
      for (j = 0; j < 3; j++) {
        g[j] = 0;
        for (k = 0; k < 3; k++) g[j] += A[j][k] * d[k];
      }
      //std::cout<<"g="<<g[0]<<","<<g[1]<<","<<g[2]<<std::endl;
      Function(y, f);
      //std::cout<<"f="<<f[0]<<","<<f[1]<<","<<f[2]<<","<<f[3]<<","<<f[4]<<","<<f[5]<<std::endl;
      //std::cout<<"A="<<A[0][0]<<","<<A[0][1]<<","<<A[0][2]<<std::endl;
      //std::cout<<"A="<<A[1][0]<<","<<A[1][1]<<","<<A[1][2]<<std::endl;
      //std::cout<<"A="<<A[2][0]<<","<<A[2][1]<<","<<A[2][2]<<std::endl;
      double Y = 0;
      for (j = 0; j < 3; j++) Y += y[j + 3] * g[j];
      double dYds = 0;
      for (j = 0; j < 3; j++) dYds += f[j + 3] * g[j];
      //std::cout<<"dYds="<<dYds<<std::endl;
      for (j = 0; j < 3; j++) {
        Af[j] = 0;
        Af2[j] = 0;
      }
      for (j = 0; j < 3; j++) {
        //Af[j]=0;
        //Af2[j]=0;
        for (k = 0; k < 3; k++) Af[j] += (A[j][k] * f[k]);
        for (k = 0; k < 3; k++) Af2[j] += (A[j][k] * Af[k]);
        dYds += y[j + 3] * Af2[j];
        //std::cout<<j<<" dYds="<<dYds<<std::endl;
      }
      //std::cout<<"dYds="<<dYds<<std::endl;
      const double step = -Y / dYds * factor;
      //std::cout<<"TR  step="<<step<<" i="<<i<<std::endl;
      if (fabs(step) < 0.0001) break; // step < 1 [um]
      if (l2 > l2_old) factor /= 2;
      l2_old = l2;
      Propagate(y, step);
      step2 += step;
    }

    tof += step2 * tof_factor;

    onTrack.setX(y[0]);
    onTrack.setY(y[1]);
    onTrack.setZ(y[2]);
    p.setX(y[3]);
    p.setY(y[4]);
    p.setZ(y[5]);

    const double dx = y[0] - w0x;
    const double dy = y[1] - w0y;
    const double dz = y[2] - w0z;
    const double s = (dx * vx + dy * vy + dz * vz) / v_2;
    onLine.setX(w0x + s * vx);
    onLine.setY(w0y + s * vy);
    onLine.setZ(w0z + s * vz);

    return(0);
  }

  int TRunge::approach_point(const Point3D& p0, Point3D& onTrack) const
  {
    if (_Nstep == 0) {
      if (_stepSize == 0) Fly_SC();
      else Fly();
    }

    double x0 = p0.x();
    double y0 = p0.y();
    double z0 = p0.z();

    //tof=0;
    //const float clight=29.9792458; //[cm/ns]
    //const float M2=_mass*_mass;

    // search for the closest point in cache
    unsigned stepNum;
    double l2_old = DBL_MAX;
    for (stepNum = 0; stepNum < _Nstep; stepNum++) {
      double l2 = (_y[stepNum][0] - x0) * (_y[stepNum][0] - x0) +
                  (_y[stepNum][1] - y0) * (_y[stepNum][1] - y0) +
                  (_y[stepNum][2] - z0) * (_y[stepNum][2] - z0);
      if (l2 > l2_old) break;
      l2_old = l2;
      //const double p2 = _y[stepNum][3]*_y[stepNum][3]+
      //                  _y[stepNum][4]*_y[stepNum][4]+
      //                  _y[stepNum][5]*_y[stepNum][5];
      //tof+=_stepSize/clight*sqrt(1+M2/p2);
    }
    if (stepNum >= _Nstep) return(-1); // not found
    stepNum--;

    // propagate the track and search for the closest point
    double y[6], y_old[6];
    for (unsigned i = 0; i < 6; i++) y[i] = _y[stepNum][i];
    double step = _stepSize;
    for (;;) {
      for (unsigned i = 0; i < 6; i++) y_old[i] = y[i];
      Propagate(y, step);
      double l2 = (y[0] - x0) * (y[0] - x0) + (y[1] - y0) * (y[1] - y0) + (y[2] - z0) * (y[2] - z0);
      if (l2 > l2_old) { // back
        for (unsigned i = 0; i < 6; i++) y[i] = y_old[i];
      } else {       // propagate
        l2_old = l2;
        //const double p2=y[3]*y[3]+y[4]*y[4]+y[5]*y[5];
        //tof+=step/clight*sqrt(1+M2/p2);
      }
      step /= 2;
      if (step < 0.0001) break; // step < 1 [um]
    }

    onTrack.setX(y[0]);
    onTrack.setY(y[1]);
    onTrack.setZ(y[2]);
    //p.setX(y[3]);
    //p.setY(y[4]);
    //p.setZ(y[5]);
    return(0);
  }

  unsigned TRunge::Fly(void) const
  {
    double y[6];
    unsigned Nstep;
    double flightlength;

    flightlength = 0;
    SetFirst(y);
    for (Nstep = 0; Nstep < TRunge_MAXstep; Nstep++) {
      for (unsigned j = 0; j < 6; j++) _y[Nstep][j] = y[j];
      //memcpy(_y[Nstep],y,sizeof(double)*6);

      Propagate(y, _stepSize);

      if (y[2] > 160 || y[2] < -80 || (y[0]*y[0] + y[1]*y[1]) > 8100) break;
      //if position is out side of CDC, stop to fly
      //  R>90cm, z<-80cm,160cm<z

      flightlength += _stepSize;
      if (flightlength > _maxflightlength) break;

      _h[Nstep] = _stepSize;
    }
    _Nstep = Nstep + 1;
    return(_Nstep);
  }

  void TRunge::Propagate(double y[6], const double& step) const
  {
    //  y[6] = (x,y,z,px,py,pz)
    double f1[6], f2[6], f3[6], f4[6], yt[6];
    double hh;
    double h6;
    unsigned i;

    hh = step * 0.5;
    //std::cout<<"TR:Pro hh="<<hh<<std::endl;
    Function(y, f1);
    for (i = 0; i < 6; i++) yt[i] = y[i] + hh * f1[i];
    //{
    //  register double *a=y;
    //  register double *b=f1;
    //  register double *t=yt;
    //  register double *e=y+6;
    //  for(;a<e;a++,b++,t++) (*t)=(*a)+hh*(*b);
    //}
    Function(yt, f2);
    for (i = 0; i < 6; i++) yt[i] = y[i] + hh * f2[i];
    //{
    //  register double *a=y;
    //  register double *b=f2;
    //  register double *t=yt;
    //  register double *e=y+6;
    //  for(;a<e;a++,b++,t++) (*t)=(*a)+hh*(*b);
    //}
    Function(yt, f3);
    for (i = 0; i < 6; i++) yt[i] = y[i] + step * f3[i];
    //{
    //  register double *a=y;
    //  register double *b=f3;
    //  register double *t=yt;
    //  register double *e=y+6;
    //  for(;a<e;a++,b++,t++) (*t)=(*a)+step*(*b);
    //}
    Function(yt, f4);

    h6 = step / 6;
    for (i = 0; i < 6; i++) y[i] += h6 * (f1[i] + 2 * f2[i] + 2 * f3[i] + f4[i]);
    //{
    //  register double *a=f1;
    //  register double *b=f2;
    //  register double *c=f3;
    //  register double *d=f4;
    //  register double *t=y;
    //  register double *e=y+6;
    //  for(;t<e;a++,b++,c++,d++,t++)
    //    (*t)+=h6*((*a)+2*(*b)+2*(*c)+(*d));
    //}
  }

  void TRunge::Function(const double y[6], double f[6]) const
  {
    // return the value of formula
    //  y[6] = (x,y,z,px,py,pz)
    //  f[6] = ( dx[3]/ds, dp[3]/ds )
    //  dx/ds = p/|p|
    //  dp/ds = e/|e| 1/alpha (p x B)/|p||B|
    //    alpha = 1/cB = 333.5640952/B[Tesla]  [cm/(GeV/c)]
    //  const float Bx = _bfield->bx(y[0],y[1],y[2]);
    //  const float By = _bfield->by(y[0],y[1],y[2]);
    //  const float Bz = _bfield->bz(y[0],y[1],y[2]);  //[kGauss]
    float B[3];
    //const float B[3]={0,0,15.0};
    float pos[3];
    double pmag;
    double factor;

    pos[0] = (float)y[0];
    pos[1] = (float)y[1];
    pos[2] = (float)y[2];
    //std::cout<<"TR::pos="<<pos[0]<<","<<pos[1]<<","<<pos[2]<<std::endl;
    _bfield->fieldMap(pos, B);
    //std::cout<<"TR::B="<<B[0]<<","<<B[1]<<","<<B[2]<<std::endl;
    //  const double Bmag = sqrt(Bx*Bx+By*By+Bz*Bz);

    pmag = sqrt(y[3] * y[3] + y[4] * y[4] + y[5] * y[5]);
    f[0] = y[3] / pmag; // p/|p|
    f[1] = y[4] / pmag;
    f[2] = y[5] / pmag;

    //  const factor = _charge/(alpha2/Bmag)/pmag/Bmag;
    factor = ((double)_charge) / alpha2 / 10.; //[(GeV/c)/(cm kG)]
    //  f[3] = factor*(f[1]*Bz-f[2]*By);
    //  f[4] = factor*(f[2]*Bx-f[0]*Bz);
    //  f[5] = factor*(f[0]*By-f[1]*Bx);
    f[3] = factor * (f[1] * B[2] - f[2] * B[1]);
    f[4] = factor * (f[2] * B[0] - f[0] * B[2]);
    f[5] = factor * (f[0] * B[1] - f[1] * B[0]);
  }

  void TRunge::SetFirst(double y[6]) const
  {
    //  y[6] = (x,y,z,px,py,pz)
    const double cosPhi0 = cos(_a[1]);
    const double sinPhi0 = sin(_a[1]);
    const double invKappa = 1 / abs(_a[2]);

    y[0] = _pivot.x() + _a[0] * cosPhi0; //x
    y[1] = _pivot.y() + _a[0] * sinPhi0; //y
    y[2] = _pivot.z() + _a[3]; //z
    y[3] = -sinPhi0 * invKappa; //px
    y[4] =  cosPhi0 * invKappa; //py
    y[5] = _a[4] * invKappa;  //pz
  }

//  const double alpha = alpha2/1.5;  //[cm/(GeV/c)]  #### 1.5T fix ####!!!!
//  The unit of kappa is defined by this constant. [about 1/(GeV/c)]

  unsigned TRunge::Nstep(void) const
  {
    return(_Nstep);
  }

  int TRunge::GetXP(unsigned stepNum, double y[6]) const
  {
    if (stepNum >= _Nstep || stepNum >= TRunge_MAXstep) return(-1);

    for (unsigned i = 0; i < 6; i++) y[i] = _y[stepNum][i];
    return(0);
  }
  int TRunge::GetStep(unsigned stepNum, double& step) const
  {
    if (stepNum >= _Nstep || stepNum >= TRunge_MAXstep) return(-1);
    step = _h[stepNum];
    return(0);
  }

  void TRunge::Propagate1(const double y[6], const double dydx[6],
                          const double& step, double yout[6]) const
  {
    //  y[6] = (x,y,z,px,py,pz)
    double f2[6], f3[6], f4[6], yt[6];
    double hh;
    double h6;
    unsigned i;

    hh = step * 0.5;
    for (i = 0; i < 6; i++) yt[i] = y[i] + hh * dydx[i]; // 1st step
    //{
    //  const register double *a=y;
    //  const register double *b=dydx;
    //  register double *t=yt;
    //  const register double *e=y+6;
    //  for(;a<e;a++,b++,t++) (*t)=(*a)+hh*(*b);
    //}
    Function(yt, f2);       // 2nd step
    for (i = 0; i < 6; i++) yt[i] = y[i] + hh * f2[i];
    //{
    //  const register double *a=y;
    //  const register double *b=f2;
    //  register double *t=yt;
    //  const register double *e=y+6;
    //  for(;a<e;a++,b++,t++) (*t)=(*a)+hh*(*b);
    //}
    Function(yt, f3);       // 3rd step
    for (i = 0; i < 6; i++) yt[i] = y[i] + step * f3[i];
    //{
    //  const register double *a=y;
    //  const register double *b=f3;
    //  register double *t=yt;
    //  const register double *e=y+6;
    //  for(;a<e;a++,b++,t++) (*t)=(*a)+step*(*b);
    //}
    Function(yt, f4);       // 4th step

    h6 = step / 6;
    for (i = 0; i < 6; i++) yout[i] = y[i] + h6 * (dydx[i] + 2 * f2[i] + 2 * f3[i] + f4[i]);
    //{
    //  const register double *a=dydx;
    //  const register double *b=f2;
    //  const register double *c=f3;
    //  const register double *d=f4;
    //  const register double *e=y;
    //  register double *t=yout;
    //  const register double *s=yout+6;
    //  for(;t<s;a++,b++,c++,d++,e++,t++)
    //    (*t)=(*e)+h6*((*a)+2*(*b)+2*(*c)+(*d));
    //}
  }

#define PGROW -0.20
#define PSHRNK -0.25
#define FCOR 0.06666666   // 1.0/15.0
#define SAFETY 0.9
#define ERRCON 6.0e-4   // (4/SAFETY)^(1/PGROW)
  void TRunge::Propagate_QC(double y[6], double dydx[6], const double& steptry,
                            const double& eps, const double yscal[6],
                            double& stepdid, double& stepnext) const
  {
    //propagate with quality check, step will be scaled automatically
    double ysav[6], dysav[6], ytemp[6];
    double h, hh, errmax, temp;
    unsigned i;

    for (i = 0; i < 6; i++) ysav[i] = y[i];
    //memcpy(ysav,y,sizeof(double)*6);
    for (i = 0; i < 6; i++) dysav[i] = dydx[i];
    //memcpy(dysav,dydx,sizeof(double)*6);

    h = steptry;
    for (;;) {
      hh = h * 0.5;   // 2 half step
      Propagate1(ysav, dysav, hh, ytemp);
      Function(ytemp, dydx);
      Propagate1(ytemp, dydx, hh, y);
      //if  check step size
      Propagate1(ysav, dysav, h, ytemp); // 1 full step
      // error calculation
      errmax = 0.0;
      for (i = 0; i < 6; i++) {
        ytemp[i] = y[i] - ytemp[i];
        temp = fabs(ytemp[i] / yscal[i]);
        if (errmax < temp) errmax = temp;
      }
      //std::cout<<"TR: errmax="<<errmax<<std::endl;
      errmax /= eps;
      if (errmax <= 1.0) {    // step O.K.  calc. for next step
        stepdid = h;
        stepnext = (errmax > ERRCON ? SAFETY * h * exp(PGROW * log(errmax)) : 4.0 * h);
        break;
      }
      h = SAFETY * h * exp(PSHRNK * log(errmax));
    }
    for (i = 0; i < 6; i++) y[i] += ytemp[i] * FCOR;
    //{
    //  register double *a=ytemp;
    //  register double *t=y;
    //  register double *e=y+6;
    //  for(;t<e;a++,t++) (*t)+=(*a)*FCOR;
    //}

  }

#define TINY 1.0e-30
//#define EPS 1.0e-6
  unsigned TRunge::Fly_SC(void) const //fly the particle track with stepsize control
  {
    double y[6], dydx[6]; // ,yscal[6];
    unsigned Nstep;
    double step, stepdid, stepnext;
    unsigned i;
    double flightlength;

    //yscal[0]=0.1; //1mm  -> error = 1mm*EPS
    //yscal[1]=0.1; //1mm
    //yscal[2]=0.1; //1mm
    //yscal[3]=0.001; //1MeV
    //yscal[4]=0.001; //1MeV
    //yscal[5]=0.001; //1MeV

    step = default_stepSize0;
    flightlength = 0;
    SetFirst(y);
    for (Nstep = 0; Nstep < TRunge_MAXstep; Nstep++) {
      for (i = 0; i < 6; i++) _y[Nstep][i] = y[i]; // save each step
      //memcpy(_y[Nstep],y,sizeof(double)*6);

      Function(y, dydx);
      //for(i=0;i<6;i++) yscal[i]=abs(y[i])+abs(dydx[i]*step)+TINY;

      Propagate_QC(y, dydx, step, _eps, _yscal, stepdid, stepnext);

      if (y[2] > 160 || y[2] < -80 || (y[0]*y[0] + y[1]*y[1]) > 8100) break;
      //if position is out side of CDC, stop to fly
      //  R>90cm, z<-80cm,160cm<z

      flightlength += stepdid;
      if (flightlength > _maxflightlength) break;

      _h[Nstep] = stepdid;
      //std::cout<<"TR:"<<Nstep<<":step="<<stepdid<<std::endl;
      if (stepnext < _stepSizeMin)      step = _stepSizeMin;
      else if (stepnext > _stepSizeMax) step = _stepSizeMax;
      else step = stepnext;
    }
    _Nstep = Nstep + 1;
    //std::cout<<"TR: Nstep="<<_Nstep<<std::endl;
    return(_Nstep);
  }

  double TRunge::SetFlightLength(void)
  {
//cnv
//   // get a list of links to a wire hit
//   const AList<TLink>& cores=this->cores();
//   //std::cout<<"TR:: cores.length="<<cores.length()<<std::endl;

//   const THelix hel(this->helix());
//   double tanl=hel.tanl();
//   //double curv=hel.curv();
//   double rho = THelix::ConstantAlpha / hel.kappa();

//   // search max phi
//   double phi_max=- DBL_MAX;
//   double phi_min= DBL_MAX;
//   // loop over link
//   for(int j=0;j<cores.length();j++){
//     TLink& l=*cores[j];
//     // fitting valid?
//     const Belle2::TRGCDCWire& wire=*l.wire();
//     double Wz=0;
//     //double mindist;
//     HepGeom::Point3D<double> onWire;
//     HepGeom::Point3D<double> dummy_bP;
//     Vector3D dummy_dir;
//     THelix th(hel);
//     for(int i=0;i<10;i++){
//       wire.wirePosition(Wz,onWire,dummy_bP,dummy_dir);
//       th.pivot(onWire);
//       if(abs(th.dz())<0.1){  //<1mm
//  //mindist=abs(hel.dr());
//  break;
//       }
//       Wz+=th.dz();
//     }
//     //onWire is closest point , th.x() is closest point on trk
//     //Wz is z position of closest point
//     //Z->dphi
//     /*
//     // Calculate position (x,y,z) along helix.
//     // x = x0 + dr * cos(phi0) + (alpha / kappa) * (cos(phi0) - cos(phi0+phi))
//     // y = y0 + dr * sin(phi0) + (alpha / kappa) * (sin(phi0) - sin(phi0+phi))
//     // z = z0 + dz             - (alpha / kappa) * tan(lambda) * phi
//     std::cout<<"TR:: Wz="<<Wz<<" tanl="<<tanl<<std::endl;
//     double phi=( hel.pivot().z()+hel.dz()-Wz )/( curv*tanl );
//     */
//     //...Cal. dPhi to rotate...
//     const HepGeom::Point3D<double> & xc = hel.center();
//     const HepGeom::Point3D<double> & xt = hel.x();
//     Vector3D v0, v1;
//     v0 = xt - xc;
//     v1 = th.x() - xc;
//     const double vCrs = v0.x() * v1.y() - v0.y() * v1.x();
//     const double vDot = v0.x() * v1.x() + v0.y() * v1.y();
//     double phi = atan2(vCrs, vDot);

// //    double tz=hel.x(phi).z();
//     //std::cout<<"TR::  Wz="<<Wz<<" tz="<<tz<<std::endl;

//     if(phi>phi_max) phi_max=phi;
//     if(phi<phi_min) phi_min=phi;
//     //std::cout<<"TR:: phi="<<phi<<std::endl;
//   }
//   //std::cout<<"TR:: phi_max="<<phi_max<<" phi_min="<<phi_min
//   //    <<" rho="<<rho<<" tanl="<<tanl<<std::endl;
//   //phi->length, set max filght length
//   //tanl*=1.2; // for mergin
//   _maxflightlength=
//     abs( rho*(phi_max-phi_min)*sqrt(1+tanl*tanl) )*1.2; //x1.1 mergin

//  return(_maxflightlength);
    return(0);
  }

} // namespace Belle

