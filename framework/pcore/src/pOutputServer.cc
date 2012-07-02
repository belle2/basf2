//+
// File : pOutputServer.cc
// Description : Base class for Output module with parallel processing support
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 2 - Jun - 2010
//-

#include <framework/pcore/pOutputServer.h>

using namespace std;
using namespace Belle2;

pOutputServer::pOutputServer() :
  Module()
{
  cout << "constructor called" << endl;
}


pOutputServer::~pOutputServer()
{
  cout << "destructor called" << endl;
}

