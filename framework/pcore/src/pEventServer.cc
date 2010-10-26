//+
// File : pEventServer.cc
// Description : Base class for Input module with parallel processing support
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 2 - Jun - 2010
//-

#include <framework/pcore/pEventServer.h>

using namespace std;
using namespace Belle2;

pEventServer::pEventServer() :
    Module()
{
  cout << "constructor called" << endl;
}


pEventServer::~pEventServer()
{
  cout << "destructor called" << endl;
}

