/*
 * integral.h - scalar integrals wrappers
 *
 * this file is part of PJFry library
 * Copyright 2011 Valery Yundin
 */

#include "integral.h"
#include "minor.h"
#include "cache.h"

static Initialize pjinit=Initialize();

#ifdef USE_QCDLOOP
Initialize::Initialize()
{
  printf("PJFRY init\n");
  const double dbl_min=std::numeric_limits<double>::min();

  F77_FUNC(qlinit,QLINIT)();

  if (qlprec.xalogm < dbl_min) {
    qlprec.xalogm=dbl_min;
    qlprec.xalog2=sqrt(dbl_min);
    #ifndef NDEBUG
    printf("Set xalogm to normalized value %e\n", qlprec.xalogm);
    printf("Set xalog2 to normalized value %e\n", qlprec.xalog2);
    #endif
  }
  if (qlprec.xclogm < dbl_min) {
    qlprec.xclogm=dbl_min;
    qlprec.xclog2=sqrt(dbl_min);
    #ifndef NDEBUG
    printf("Set xclogm to normalized value %e\n", qlprec.xclogm);
    printf("Set xclog2 to normalized value %e\n", qlprec.xclog2);
    #endif
  }
  if (qlflag.lwarn) {
    qlflag.lwarn=0;
    #ifndef NDEBUG
    printf("Disable FF warnings %d\n",qlflag.lwarn);
    #endif
  }
}

Initialize::~Initialize()
{
  F77_FUNC(ffexi,FFEXI)();
}

ICache::Ival qlI1(const Kinem1& k)
{
  int ep;
  ICache::Ival ivalue;

  double m1=k.m1();
  double mu2=ICache::getMu2();

#ifdef USE_F2C
  std::complex<double> rslt;
  ep=0;
  F77_FUNC(qli1,QLI1)(&rslt, &m1, &mu2, &ep);
  ivalue.val[0]=rslt;
  ep=-1;
  F77_FUNC(qli1,QLI1)(&rslt, &m1, &mu2, &ep);
  ivalue.val[1]=rslt;
  ep=-2;
  F77_FUNC(qli1,QLI1)(&rslt, &m1, &mu2, &ep);
  ivalue.val[2]=rslt;
#else
  ep=0;
  ivalue.val[0]=F77_FUNC(qli1,QLI1)(&m1, &mu2, &ep);
  ep=-1;
  ivalue.val[1]=F77_FUNC(qli1,QLI1)(&m1, &mu2, &ep);
  ep=-2;
  ivalue.val[2]=F77_FUNC(qli1,QLI1)(&m1, &mu2, &ep);
#endif

  return ivalue;
}

ICache::Ival qlI2(const Kinem2& k)
{
  int ep;
  ICache::Ival ivalue;

  double p1=k.p1();
  double m1=k.m1();
  double m2=k.m2();
  double mu2=ICache::getMu2();

#ifdef USE_F2C
  std::complex<double> rslt;
  ep=0;
  F77_FUNC(qli2,QLI2)(&rslt, &p1, &m1, &m2, &mu2, &ep);
  ivalue.val[0]=rslt;
  ep=-1;
  F77_FUNC(qli2,QLI2)(&rslt, &p1, &m1, &m2, &mu2, &ep);
  ivalue.val[1]=rslt;
  ep=-2;
  F77_FUNC(qli2,QLI2)(&rslt, &p1, &m1, &m2, &mu2, &ep);
  ivalue.val[2]=rslt;
#else
  ep=0;
  ivalue.val[0]=F77_FUNC(qli2,QLI2)(&p1, &m1, &m2, &mu2, &ep);
  ep=-1;
  ivalue.val[1]=F77_FUNC(qli2,QLI2)(&p1, &m1, &m2, &mu2, &ep);
  ep=-2;
  ivalue.val[2]=F77_FUNC(qli2,QLI2)(&p1, &m1, &m2, &mu2, &ep);
#endif

  return ivalue;
}

ICache::Ival qlI3(const Kinem3& k)
{
  int ep;
  ICache::Ival ivalue;

  double p1=k.p1();
  double p2=k.p2();
  double p3=k.p3();
  double m1=k.m1();
  double m2=k.m2();
  double m3=k.m3();
  double mu2=ICache::getMu2();

#ifdef USE_F2C
  std::complex<double> rslt;
  ep=0;
  F77_FUNC(qli3,QLI3)(&rslt, &p1, &p2, &p3, &m1, &m2, &m3, &mu2, &ep);
  ivalue.val[0]=rslt;
  ep=-1;
  F77_FUNC(qli3,QLI3)(&rslt, &p1, &p2, &p3, &m1, &m2, &m3, &mu2, &ep);
  ivalue.val[1]=rslt;
  ep=-2;
  F77_FUNC(qli3,QLI3)(&rslt, &p1, &p2, &p3, &m1, &m2, &m3, &mu2, &ep);
  ivalue.val[2]=rslt;
#else
  ep=0;
  ivalue.val[0]=F77_FUNC(qli3,QLI3)(&p1, &p2, &p3, &m1, &m2, &m3, &mu2, &ep);
  ep=-1;
  ivalue.val[1]=F77_FUNC(qli3,QLI3)(&p1, &p2, &p3, &m1, &m2, &m3, &mu2, &ep);
  ep=-2;
  ivalue.val[2]=F77_FUNC(qli3,QLI3)(&p1, &p2, &p3, &m1, &m2, &m3, &mu2, &ep);
#endif

  return ivalue;
}

ICache::Ival qlI4(const Kinem4& k)
{
  int ep;
  ICache::Ival ivalue;

  double p1=k.p1();
  double p2=k.p2();
  double p3=k.p3();
  double p4=k.p4();
  double s12=k.s12();
  double s23=k.s23();
  double m1=k.m1();
  double m2=k.m2();
  double m3=k.m3();
  double m4=k.m4();
  double mu2=ICache::getMu2();

  if (s12==0. || s23==0.) {
    if (p1!=0. && p3!=0.) {
#   ifdef USE_F2C
      std::complex<double> rslt;
      ep=0;
      F77_FUNC(qli4,QLI4)(&rslt, &s12, &p2, &s23, &p4, &p1, &p3, &m1, &m3, &m2, &m4, &mu2, &ep);
      ivalue.val[0]=rslt;
      ep=-1;
      F77_FUNC(qli4,QLI4)(&rslt, &s12, &p2, &s23, &p4, &p1, &p3, &m1, &m3, &m2, &m4, &mu2, &ep);
      ivalue.val[1]=rslt;
      ep=-2;
      F77_FUNC(qli4,QLI4)(&rslt, &s12, &p2, &s23, &p4, &p1, &p3, &m1, &m3, &m2, &m4, &mu2, &ep);
      ivalue.val[2]=rslt;
#   else
      ep=0;
      ivalue.val[0]=F77_FUNC(qli4,QLI4)(&s12, &p2, &s23, &p4, &p1, &p3, &m1, &m3, &m2, &m4, &mu2, &ep);
      ep=-1;
      ivalue.val[1]=F77_FUNC(qli4,QLI4)(&s12, &p2, &s23, &p4, &p1, &p3, &m1, &m3, &m2, &m4, &mu2, &ep);
      ep=-2;
      ivalue.val[2]=F77_FUNC(qli4,QLI4)(&s12, &p2, &s23, &p4, &p1, &p3, &m1, &m3, &m2, &m4, &mu2, &ep);
#   endif
    } else if (p2!=0. && p4!=0.) {
#   ifdef USE_F2C
      std::complex<double> rslt;
      ep=0;
      F77_FUNC(qli4,QLI4)(&rslt, &s23, &p3, &s12, &p1, &p2, &p4, &m2, &m4, &m3, &m1, &mu2, &ep);
      ivalue.val[0]=rslt;
      ep=-1;
      F77_FUNC(qli4,QLI4)(&rslt, &s23, &p3, &s12, &p1, &p2, &p4, &m2, &m4, &m3, &m1, &mu2, &ep);
      ivalue.val[1]=rslt;
      ep=-2;
      F77_FUNC(qli4,QLI4)(&rslt, &s23, &p3, &s12, &p1, &p2, &p4, &m2, &m4, &m3, &m1, &mu2, &ep);
      ivalue.val[2]=rslt;
#   else
      ep=0;
      ivalue.val[0]=F77_FUNC(qli4,QLI4)(&s23, &p3, &s12, &p1, &p2, &p4, &m2, &m4, &m3, &m1, &mu2, &ep);
      ep=-1;
      ivalue.val[1]=F77_FUNC(qli4,QLI4)(&s23, &p3, &s12, &p1, &p2, &p4, &m2, &m4, &m3, &m1, &mu2, &ep);
      ep=-2;
      ivalue.val[2]=F77_FUNC(qli4,QLI4)(&s23, &p3, &s12, &p1, &p2, &p4, &m2, &m4, &m3, &m1, &mu2, &ep);
#   endif
    } else { assert(0); }
  } else {
# ifdef USE_F2C
    std::complex<double> rslt;
    ep=0;
    F77_FUNC(qli4,QLI4)(&rslt, &p1, &p2, &p3, &p4, &s12, &s23, &m1, &m2, &m3, &m4, &mu2, &ep);
    ivalue.val[0]=rslt;
    ep=-1;
    F77_FUNC(qli4,QLI4)(&rslt, &p1, &p2, &p3, &p4, &s12, &s23, &m1, &m2, &m3, &m4, &mu2, &ep);
    ivalue.val[1]=rslt;
    ep=-2;
    F77_FUNC(qli4,QLI4)(&rslt, &p1, &p2, &p3, &p4, &s12, &s23, &m1, &m2, &m3, &m4, &mu2, &ep);
    ivalue.val[2]=rslt;
# else
    ep=0;
    ivalue.val[0]=F77_FUNC(qli4,QLI4)(&p1, &p2, &p3, &p4, &s12, &s23, &m1, &m2, &m3, &m4, &mu2, &ep);
    ep=-1;
    ivalue.val[1]=F77_FUNC(qli4,QLI4)(&p1, &p2, &p3, &p4, &s12, &s23, &m1, &m2, &m3, &m4, &mu2, &ep);
    ep=-2;
    ivalue.val[2]=F77_FUNC(qli4,QLI4)(&p1, &p2, &p3, &p4, &s12, &s23, &m1, &m2, &m3, &m4, &mu2, &ep);
# endif
  }
  return ivalue;
}

#endif /* USE_QCDLOOP */

#ifdef USE_ONELOOP
Initialize::Initialize()
{
  printf("PJFRY init\n");

  double mu=sqrt(ICache::getMu2());
  F77_FUNC_(avh_olo_mu_set,AVH_OLO_MU_SET)(&mu);

  double thrs=Minor5::getmeps();
  F77_FUNC_(avh_olo_onshell,AVH_OLO_ONSHELL)(&thrs);
}

ICache::Ival qlI1(const Kinem1& k)
{
  ICache::Ival ivalue;

  double m1=k.m1();
  std::complex<double> rslt[3];
  F77_FUNC_(avh_olo_a0m,AVH_OLO_A0M)(rslt, &m1);

  ivalue.val[0]=rslt[0];
  ivalue.val[1]=rslt[1];
  ivalue.val[2]=rslt[2];
  return ivalue;
}

ICache::Ival qlI2(const Kinem2& k)
{
  ICache::Ival ivalue;

  double p1=k.p1();
  double m1=k.m1();
  double m2=k.m2();
  std::complex<double> rslt[3];
  F77_FUNC_(avh_olo_b0m,AVH_OLO_B0M)(rslt, &p1, &m1, &m2);

  ivalue.val[0]=rslt[0];
  ivalue.val[1]=rslt[1];
  ivalue.val[2]=rslt[2];
  return ivalue;
}

ICache::Ival qlI3(const Kinem3& k)
{
  ICache::Ival ivalue;

  double p1=k.p1();
  double p2=k.p2();
  double p3=k.p3();
  double m1=k.m1();
  double m2=k.m2();
  double m3=k.m3();
  std::complex<double> rslt[3];
  F77_FUNC_(avh_olo_c0m,AVH_OLO_C0M)(rslt, &p1, &p2, &p3, &m1, &m2, &m3);

  ivalue.val[0]=rslt[0];
  ivalue.val[1]=rslt[1];
  ivalue.val[2]=rslt[2];
  return ivalue;
}

ICache::Ival qlI4(const Kinem4& k)
{
  ICache::Ival ivalue;

  double p1=k.p1();
  double p2=k.p2();
  double p3=k.p3();
  double p4=k.p4();
  double s12=k.s12();
  double s23=k.s23();
  double m1=k.m1();
  double m2=k.m2();
  double m3=k.m3();
  double m4=k.m4();
  std::complex<double> rslt[3];
  if (s12==0. || s23==0.) {
    if (p1!=0. && p3!=0.) {
      F77_FUNC_(avh_olo_d0m,AVH_OLO_D0M)(rslt, &s12, &p2, &s23, &p4, &p1, &p3, &m1, &m3, &m2, &m4);
    } else if (p2!=0. && p4!=0.) {
      F77_FUNC_(avh_olo_d0m,AVH_OLO_D0M)(rslt, &s23, &p3, &s12, &p1, &p2, &p4, &m2, &m4, &m3, &m1);
    } else { assert(0); }
  } else {
    F77_FUNC_(avh_olo_d0m,AVH_OLO_D0M)(rslt, &p1, &p2, &p3, &p4, &s12, &s23, &m1, &m2, &m3, &m4);
  }

  ivalue.val[0]=rslt[0];
  ivalue.val[1]=rslt[1];
  ivalue.val[2]=rslt[2];
  return ivalue;
}

#endif /* USE_QCDLOOP1 */
