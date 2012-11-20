#include <iostream>
#include <cstdlib>

/** Wrapper for display/examples/display.py steering file.
 *
 *  Show data in given input file using the event display.
 */
int main(int argc, char* argv[])
{
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " input_file.root\n\n";
    std::cerr << "  Show data in given input file using the event display.\n";
    return 1;
  }
  std::string s = "basf2 $BELLE2_LOCAL_DIR/display/examples/display.py -i " + std::string(argv[1]);
  return system(s.c_str());
}
