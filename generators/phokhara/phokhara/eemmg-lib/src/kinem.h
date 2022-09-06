/*
 * kinem.h - kinematics classes
 *
 * this file is part of PJFry library
 * Copyright 2011 Valery Yundin
 */

#ifndef QUL_KINEM_H
#define QUL_KINEM_H

#include "common.h"
#include <cstdlib>
// #include <cfloat>

template <int R>
class Kinem {
public:
  bool operator == (const Kinem& kinem) const;

  inline
  double mass(int i) const
  {
    return kdata[(i - 1) * (i + 2) / 2];
  }

protected:
  Kinem() {}

  void zero(); // initialize to zero

  typedef union { double d64; int64_t i64; } DI64;
//     static const double mdelta=1e-14;
//     static const int64_t idelta=1+mdelta/DBL_EPSILON; // needs cfloat
  /*
  Comparison tolerance for double
  idelta   fdelta(%)
      5  - 1.11e-15
     46  - 1.02e-14
    451  - 1.00e-13
   4504  - 1.00e-12
  45036  - 1.00e-11
  */
  static const uint64_t idelta = 5; // unsigned !!!

  inline
  bool eq(const double& a, const double& b) const
  {
    const DI64 ia = {a};
    const DI64 ib = {b};
    const int64_t diff = ia.i64 - ib.i64;
    return diff == 0LL || static_cast<uint64_t>(llabs(diff)) <= idelta;
  }
  inline
  bool neq(const double& a, const double& b) const
  {
    const DI64 ia = {a};
    const DI64 ib = {b};
    const int64_t diff = ia.i64 - ib.i64;
    return diff != 0LL && static_cast<uint64_t>(llabs(diff)) > idelta;
  }

  static const int KLEN = R * (R + 1) / 2;
  enum Invar {im1 = 0,
              ip1, im2,
              ip2, ip3, im3,
              ip4, is12, is23, im4,
              ip5, is34, is45, is15, im5,
              ip6, /*   6-point inv    */ im6
             };

  double kdata[KLEN];
};

template <int R>
bool Kinem<R>::operator == (const Kinem<R>& kinem) const
{
  for (int i = 0; i < KLEN; i++) {
    if (not eq(kdata[i], kinem.kdata[i])) return false;
  }
  return true;
}

template <int R>
void Kinem<R>::zero()
{
  for (int i = 0; i < KLEN; i++) {
    kdata[i] = 0;
  }
}

// 1-point kinematics
class Kinem1 : public Kinem<1> {
public:
  Kinem1() { zero(); }
  Kinem1(double xm1)
  {
    kdata[im1] = xm1;
  }

  inline double m1()  const { return kdata[im1];  }
};

// 2-point kinematics
class Kinem2 : public Kinem<2> {
public:
  Kinem2() { zero(); }
  Kinem2(double xp1,
         double xm1,  double xm2)
  {
    kdata[ip1] = xp1;
    kdata[im1] = xm1;
    kdata[im2] = xm2;
  }

  inline double p1()  const { return kdata[ip1];  }
  inline double m1()  const { return kdata[im1];  }
  inline double m2()  const { return kdata[im2];  }
};

// 3-point kinematics
class Kinem3 : public Kinem<3> {
public:
  Kinem3() { zero(); }
  Kinem3(double xp1,  double xp2,  double xp3,
         double xm1,  double xm2,  double xm3)
  {
    kdata[ip1] = xp1;
    kdata[ip2] = xp2;
    kdata[ip3] = xp3;
    kdata[im1] = xm1;
    kdata[im2] = xm2;
    kdata[im3] = xm3;
  }

  inline double p1()  const { return kdata[ip1];  }
  inline double p2()  const { return kdata[ip2];  }
  inline double p3()  const { return kdata[ip3];  }
  inline double m1()  const { return kdata[im1];  }
  inline double m2()  const { return kdata[im2];  }
  inline double m3()  const { return kdata[im3];  }
};

// 4-point kinematics
class Kinem4 : public Kinem<4> {
public:
  Kinem4() { zero(); }
  Kinem4(double xp1,  double xp2,  double xp3,  double xp4,
         double xs12, double xs23,
         double xm1,  double xm2,  double xm3,  double xm4)
  {
    kdata[ip1] = xp1;
    kdata[ip2] = xp2;
    kdata[ip3] = xp3;
    kdata[ip4] = xp4;
    kdata[is12] = xs12;
    kdata[is23] = xs23;
    kdata[im1] = xm1;
    kdata[im2] = xm2;
    kdata[im3] = xm3;
    kdata[im4] = xm4;
  }

  inline double p1()  const { return kdata[ip1];  }
  inline double p2()  const { return kdata[ip2];  }
  inline double p3()  const { return kdata[ip3];  }
  inline double p4()  const { return kdata[ip4];  }
  inline double m1()  const { return kdata[im1];  }
  inline double m2()  const { return kdata[im2];  }
  inline double m3()  const { return kdata[im3];  }
  inline double m4()  const { return kdata[im4];  }
  inline double s12() const { return kdata[is12]; }
  inline double s23() const { return kdata[is23]; }
};

// 5-point kinematics
class Kinem5 : public Kinem<5> {
public:
  Kinem5() { zero(); }
  Kinem5(double xp1,  double xp2,  double xp3,  double xp4,  double xp5,
         double xs12, double xs23, double xs34, double xs45, double xs15,
         double xm1,  double xm2,  double xm3,  double xm4,  double xm5)
  {
    kdata[ip1] = xp1;
    kdata[ip2] = xp2;
    kdata[ip3] = xp3;
    kdata[ip4] = xp4;
    kdata[ip5] = xp5;
    kdata[is12] = xs12;
    kdata[is23] = xs23;
    kdata[is34] = xs34;
    kdata[is45] = xs45;
    kdata[is15] = xs15;
    kdata[im1] = xm1;
    kdata[im2] = xm2;
    kdata[im3] = xm3;
    kdata[im4] = xm4;
    kdata[im5] = xm5;
  }

  inline double p1()  const { return kdata[ip1];  }
  inline double p2()  const { return kdata[ip2];  }
  inline double p3()  const { return kdata[ip3];  }
  inline double p4()  const { return kdata[ip4];  }
  inline double p5()  const { return kdata[ip5];  }
  inline double m1()  const { return kdata[im1];  }
  inline double m2()  const { return kdata[im2];  }
  inline double m3()  const { return kdata[im3];  }
  inline double m4()  const { return kdata[im4];  }
  inline double m5()  const { return kdata[im5];  }
  inline double s12() const { return kdata[is12]; }
  inline double s23() const { return kdata[is23]; }
  inline double s34() const { return kdata[is34]; }
  inline double s45() const { return kdata[is45]; }
  inline double s15() const { return kdata[is15]; }
};

#endif /* QUL_KINEM_H */
