//+
// File : hcollect.cc
// Description : Collect dumped histograms and merge them
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 23 - Apr - 2013
//-
#include <string>
#include <stdio.h>
#include <stdlib.h>

#include <daq/dqm/HistoServer.h>
#include <framework/pcore/RbTuple.h>

using namespace Belle2;
using namespace std;

int main(int argc, char** argv)
{
  if (argc < 3) {
    printf("Usage : hcollect filename nprocs\n");
    exit(-1);
  }
  string file = string(argv[1]);
  int nprocs = atoi(argv[2]);
  printf("hcollect here.... %s, nprocs=%d\n", file.c_str(), nprocs);

  RbTupleManager rbt(nprocs, file.c_str());
  rbt.hadd(false);

}


