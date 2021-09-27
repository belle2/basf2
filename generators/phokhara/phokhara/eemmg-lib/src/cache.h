/*
 * cache.h - cache classes header
 *
 * this file is part of PJFry library
 * Copyright 2011 Valery Yundin
 */

#ifndef QUL_CACHE_H
#define QUL_CACHE_H

#include "common.h"
#include "kinem.h"
#include "minor.h"
#ifdef USE_GOLEM_MODE
#include "golem.h"
#endif

template <typename TK, typename TV>
class MEntry {
public:
  MEntry() : key(), val() {}
  MEntry(const TK& k, TV& v) : key(k), val(v) {}

  MEntry& operator= (const MEntry& entry)
  {
    key = entry.key;
    val = entry.val;
    return *this;
  }

  TK key;
  mutable TV val; // TODO: remove auto_ptr in the future

  MEntry(const MEntry& entry);
};

class Cache {
protected:
  static const int size6 = 2;

  static const int size5 = size6 * 6;
  static const int size4 = size6 * 15;
  static const int size3 = size6 * 20;
  static const int size2 = size6 * 15;
  static const int size1 = size6 * 6;

};

class ICache : public Cache {
#ifdef USE_GOLEM_MODE
  friend class Golem;
#endif
public:
  typedef struct { ncomplex val[3]; } Ival;

  enum Ecoefs {ee0 = 1, ee1, ee2, ee3, ee4,
               ee00, ee11, ee12, ee13, ee14, ee22, ee23, ee24, ee33, ee34, ee44,
               ee001, ee002, ee003, ee004,
               ee111, ee112, ee113, ee114, ee122, ee123, ee124, ee133, ee134, ee144,
               ee222, ee223, ee224, ee233, ee234, ee244, ee333, ee334, ee344, ee444,
               ee0000, ee0011, ee0012, ee0013, ee0014, ee0022, ee0023, ee0024, ee0033, ee0034, ee0044,
               ee1111, ee1112, ee1113, ee1114, ee1122, ee1123, ee1124, ee1133, ee1134, ee1144,
               ee1222, ee1223, ee1224, ee1233, ee1234, ee1244, ee1333, ee1334, ee1344, ee1444,
               ee2222, ee2223, ee2224, ee2233, ee2234, ee2244, ee2333, ee2334, ee2344, ee2444,
               ee3333, ee3334, ee3344, ee3444, ee4444,
               ee00001, ee00002, ee00003, ee00004, ee00111, ee00112, ee00113, ee00114,
               ee00122, ee00123, ee00124, ee00133, ee00134, ee00144, ee00222, ee00223, ee00224,
               ee00233, ee00234, ee00244, ee00333, ee00334, ee00344, ee00444,
               ee11111, ee11112, ee11113, ee11114, ee11122, ee11123, ee11124, ee11133, ee11134, ee11144,
               ee11222, ee11223, ee11224, ee11233, ee11234, ee11244, ee11333, ee11334, ee11344, ee11444,
               ee12222, ee12223, ee12224, ee12233, ee12234, ee12244, ee12333, ee12334, ee12344, ee12444,
               ee13333, ee13334, ee13344, ee13444, ee14444,
               ee22222, ee22223, ee22224, ee22233, ee22234, ee22244, ee22333, ee22334, ee22344, ee22444,
               ee23333, ee23334, ee23344, ee23444, ee24444, ee33333, ee33334, ee33344, ee33444, ee34444,
               ee44444, eeLAST
              };

  enum Dcoefs {dd0 = 1, dd1, dd2, dd3,
               dd00, dd11, dd12, dd13, dd22, dd23, dd33,
               dd001, dd002, dd003,
               dd111, dd112, dd113, dd122, dd123, dd133, dd222, dd223, dd233, dd333,
               dd0000, dd0011, dd0012, dd0013, dd0022, dd0023, dd0033,
               dd1111, dd1112, dd1113, dd1122, dd1123, dd1133,
               dd1222, dd1223, dd1233, dd1333,
               dd2222, dd2223, dd2233, dd2333,
               dd3333, ddLAST
              };

  enum Ccoefs {cc0 = 1, cc1, cc2,
               cc00, cc11, cc12, cc22,
               cc001, cc002,
               cc111, cc112, cc122, cc222, ccLAST
              };

  enum Bcoefs {bb0 = 1, bb1,
               bb00, bb11,
               bb001,
               bb111, bbLAST
              };

  // Scalars
  static ncomplex getI4(int ep, const Kinem4& k);
  static ncomplex getI3(int ep, const Kinem3& k);
  static ncomplex getI2(int ep, const Kinem2& k);
  static ncomplex getI1(int ep, const Kinem1& k);

  // Tensor PENTAGON
  static ncomplex getE(int ep, const Kinem5& kin);
  static ncomplex getE(int ep, int i, const Kinem5& kin);
  static ncomplex getE(int ep, int i, int j, const Kinem5& kin);
  static ncomplex getE(int ep, int i, int j, int k, const Kinem5& kin);
  static ncomplex getE(int ep, int i, int j, int k, int l, const Kinem5& kin);
  static ncomplex getE(int ep, int i, int j, int k, int l, int m, const Kinem5& kin);

  // Tensor BOX
  static ncomplex getD(int ep, const Kinem4& kin) { return getI4(ep, kin); }
  static ncomplex getD(int ep, int i, const Kinem4& kin);
  static ncomplex getD(int ep, int i, int j, const Kinem4& kin);
  static ncomplex getD(int ep, int i, int j, int k, const Kinem4& kin);
  static ncomplex getD(int ep, int i, int j, int k, int l, const Kinem4& kin);

  // Tensor TRIANGLE
  static ncomplex getC(int ep, const Kinem3& kin) { return getI3(ep, kin); }
  static ncomplex getC(int ep, int i, const Kinem3& kin);
  static ncomplex getC(int ep, int i, int j, const Kinem3& kin);
  static ncomplex getC(int ep, int i, int j, int k, const Kinem3& kin);

  // Tensor BUBBLE
  static ncomplex getB(int ep, const Kinem2& kin) { return getI2(ep, kin); }
  static ncomplex getB(int ep, int i, const Kinem2& kin);
  static ncomplex getB(int ep, int i, int j, const Kinem2& kin);

  // Tadpole
  static ncomplex getA(int ep, const Kinem1& kin) { return getI1(ep, kin); }

  static void Clear();
  static void ClearCC();
  static void ClearIC();

  static double getMu2();
  static double setMu2(const double newmu2);

private:
  static double mu2;
  typedef union { int64_t i64; double  d64; } ID64;
  typedef union { double  d64; int64_t i64; } DI64;
  static const ID64 sNAN;
  friend bool operator==(const double& x, const ICache::ID64& y);

  // ------------------------------
  // Tensor integral coefficients
  // ------------------------------
  // TODO: rethink and optimize layout later
  typedef CArray< ncomplex, eeLAST > Save5;
  typedef MEntry< Kinem5, Save5::Ptr > Entry5;
  typedef DArray< Entry5, size5 > Array5;
  static Array5 ic5[3];
  static Save5* getS5(int ep, const Kinem5& kin, int coefn);

  typedef CArray< ncomplex, ddLAST > Save4;
  typedef MEntry< Kinem4, Save4::Ptr > Entry4;
  typedef DArray< Entry4, size4 > Array4;
  static Array4 ic4[3];
  static Save4* getS4(int ep, const Kinem4& kin, int coefn);

  typedef CArray< ncomplex, ccLAST > Save3;
  typedef MEntry< Kinem3, Save3::Ptr > Entry3;
  typedef DArray< Entry3, size3 > Array3;
  static Array3 ic3[3];
  static Save3* getS3(int ep, const Kinem3& kin, int coefn);

  typedef CArray< ncomplex, bbLAST > Save2;
  typedef MEntry< Kinem2, Save2::Ptr > Entry2;
  typedef DArray< Entry2, size2 > Array2;
  static Array2 ic2[3];
  static Save2* getS2(int ep, const Kinem2& kin, int coefn);

  // ------------------------------
  // Scalar integrals
  // ------------------------------
  typedef MEntry< Kinem4, Ival > EntryS4;
  typedef DArray< EntryS4, size4 > ArrayS4;
  static ArrayS4 ics4;

  typedef MEntry< Kinem3, Ival > EntryS3;
  typedef DArray< EntryS3, size3 > ArrayS3;
  static ArrayS3 ics3;

  typedef MEntry< Kinem2, Ival > EntryS2;
  typedef DArray< EntryS2, size2 > ArrayS2;
  static ArrayS2 ics2;

  typedef MEntry< Kinem1, Ival > EntryS1;
  typedef DArray< EntryS1, size1 > ArrayS1;
  static ArrayS1 ics1;
};

inline
bool operator==(const double& x, const ICache::ID64& y)
{
  const ICache::DI64 ix = {x};
  return ix.i64 == y.i64;
}


class MCache : public Cache {
#ifdef USE_GOLEM_MODE
  friend class Golem;
#endif
public:
  // TODO: may be return by reference here?
  static Minor5::Ptr getMinor5(const Kinem5& k);
  static Minor4::Ptr getMinor4(const Kinem4& k);
  static Minor3::Ptr getMinor3(const Kinem3& k);
  static Minor2::Ptr getMinor2(const Kinem2& k);

  static void insertMinor5(const Kinem5& k, Minor5::Ptr& m);
  static void insertMinor4(const Kinem4& k, Minor4::Ptr& m);
  static void insertMinor3(const Kinem3& k, Minor3::Ptr& m);
  static void insertMinor2(const Kinem2& k, Minor2::Ptr& m);

#ifdef USE_SMART_INSERT
#   define INSERTMINOR3 smartinsertMinor3
#   define INSERTMINOR2 smartinsertMinor2
  static void smartinsertMinor3(const Kinem3& k, Minor3::Ptr& m);
  static void smartinsertMinor2(const Kinem2& k, Minor2::Ptr& m);
#else
#   define INSERTMINOR3 insertMinor3
#   define INSERTMINOR2 insertMinor2
#endif

  static void Clear();

private:

  typedef MEntry< Kinem5, Minor5::Ptr > Entry5;
  typedef DArray< Entry5, size5 > Array5;
  static Array5 cm5;

  typedef MEntry< Kinem4, Minor4::Ptr > Entry4;
  typedef DArray< Entry4, size4 > Array4;
  static Array4 cm4;

  typedef MEntry< Kinem3, Minor3::Ptr > Entry3;
  typedef DArray< Entry3, size3 > Array3;
  static Array3 cm3;

  typedef MEntry< Kinem2, Minor2::Ptr > Entry2;
  typedef DArray< Entry2, size2 > Array2;
  static Array2 cm2;

};

/* =============================================
 *
 *            inline functions
 *
 * =============================================
 */

#define insertMinorN(n) \
  inline \
  void MCache::insertMinor##n(const Kinem##n &k, Minor##n::Ptr &m) \
  { \
    cm##n.insert(Entry##n(k,m)); \
  }

insertMinorN(5)
insertMinorN(4)
insertMinorN(3)
insertMinorN(2)

#undef insertMinorN

#endif /* _QUL_CACHE_H */
