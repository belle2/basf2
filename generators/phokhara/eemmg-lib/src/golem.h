/*
 * cache.h - golem-mode header
 *
 * this file is part of PJFry library
 * Copyright 2011 Valery Yundin
 */

#ifndef QUL_GOLEM_H
#define QUL_GOLEM_H

#include "common.h"

class Golem {
public:
  static void initgolem95(int n);
  static void setmat(int i, int j, double val);
  static void preparesmatrix();
  static double getmat(int i, int j);

  //  5 point base
  static ncomplex ga50(int s, int ep);
  static ncomplex ga51(int i, int s, int ep);
  static ncomplex ga52(int i, int j, int s, int ep);
  static ncomplex gb52(int s, int ep);
  static ncomplex ga53(int i, int j, int k, int s, int ep);
  static ncomplex gb53(int i, int s, int ep);
  static ncomplex ga54(int i, int j, int k, int l, int s, int ep);
  static ncomplex gb54(int i, int j, int s, int ep);
  static ncomplex gc54(int s, int ep);
  static ncomplex ga55(int i, int j, int k, int l, int m, int s, int ep);
  static ncomplex gb55(int i, int j, int k, int s, int ep);
  static ncomplex gc55(int i, int s, int ep);

  static ncomplex ga40(int s, int ep);
  static ncomplex ga41(int i, int s, int ep);
  static ncomplex ga42(int i, int j, int s, int ep);
  static ncomplex gb42(int s, int ep);
  static ncomplex ga43(int i, int j, int k, int s, int ep);
  static ncomplex gb43(int i, int s, int ep);
  static ncomplex ga44(int i, int j, int k, int l, int s, int ep);
  static ncomplex gb44(int i, int j, int s, int ep);
  static ncomplex gc44(int s, int ep);

  static ncomplex ga30(int s, int ep);
  static ncomplex ga31(int i, int s, int ep);
  static ncomplex ga32(int i, int j, int s, int ep);
  static ncomplex gb32(int s, int ep);
  static ncomplex ga33(int i, int j, int k, int s, int ep);
  static ncomplex gb33(int i, int s, int ep);

  static ncomplex ga20(int s, int ep);
  static ncomplex ga21(int i, int s, int ep);
  static ncomplex ga22(int i, int j, int s, int ep);
  static ncomplex gb22(int s, int ep);

private:
  static void prepare6();
  static void prepare5();
  static void prepare4();

  static int N;
  static int caylen;

  static MinorBase* minortbl[128];
  static Minor5::Ptr minors5[6];
  static Minor4::Ptr minors4[15];
  static Minor3::Ptr minors3[20];
  static Minor2::Ptr minors2[15];

  static double Cay[6 * (6 + 1) / 2]; // upto rank 6
  static unsigned int bitfield;
  static unsigned int bitmask;
};

#endif /* QUL_GOLEM_H */
