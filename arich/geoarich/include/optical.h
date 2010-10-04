#include <TObject.h>

#include <string>
#include <vector>
#include <iostream>

#include <sstream>

using namespace std;

struct property {
  string name;
  vector<double> energy;
  vector<double> value;
};


class Optical : public TObject {
public:

  Optical();

  virtual ~Optical();
  void set_property(string namee, vector<double> *energies, vector<double> *values);
  vector<property>* get_properties();
  //INFO("TileSize: " << bla );

  /// clear lists, free pointers, etc., after read from / write to file
private:
  int a;
  vector<property> properties;
};



