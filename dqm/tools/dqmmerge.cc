//+
// File : dqmmerge.cc
// Description : Merge DQM histogram files
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 29 - Aug - 2016
//-
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <TFileMerger.h>
#include <TFile.h>
#include <Riostream.h>

using namespace std;

int main(int argc, char** argv)
{
  if (argc < 6) {
    printf("Usage : dqmmerge topdir histofile nnodes startnode outfile\n");
    exit(-1);
  }
  string topdir = string(argv[1]);
  string file = string(argv[2]);
  int nnodes = atoi(argv[3]);
  int startnode = atoi(argv[4]);
  string outfile = string(argv[5]);

  // Set up merger with output file
  TFileMerger merger(false, false);
  if (!merger.OutputFile(outfile.c_str())) {
    printf("RbTupleManager:: error to open output file %s\n", file.c_str());
    return -1;
  }

  // Make file list
  for (int i = 0; i < nnodes; i++) {
    char histofile[1024];
    sprintf(histofile, "%s/evp_hltwk%2.2d/%s",
            topdir.c_str(), startnode + i,
            file.c_str());
    struct stat statbuf;
    if (stat(histofile, &statbuf) == 0)
      merger.AddFile(histofile);
  }

  // Do Merge
  if (!merger.Merge()) {
    printf("RbTupleManager:: error to merge files\n");
    return -1;
  }
}


