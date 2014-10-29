#ifndef _TNiel_
#define _TNiel_

#include <string>

class TNiel {

private:

  double nielfactor[1710]; // the longer file is the one for neutron with 1708 values
  double E_nielfactor[1710];
  int niel_N;

public:

  TNiel(const std::string FileName);
  double getNielFactor(double EMeV);

};

#endif
