#include <framework/utilities/FileSystem.h>

#include <iostream>
#include <cstdlib>

/** Wrapper for display/examples/display.py steering file.
 *
 *  Show data in given input file using the event display.
 */
int main(int argc, char* argv[])
{
  // if you modify the text below, please update the CLI documentation in
  // display/doc/index.rst
  if (argc != 2 or std::string(argv[1]) == "--help") {
    std::cerr << "Usage: " << argv[0] << " input_file.root\n\n"
              "  Show data in given input file using the event display.\n\n"

              "  You can also open multiple files using wildcards, though you have to\n"
              "  pass the string with wildcards to b2display to prevent expansion by the shell, e.g.\n"
              "    b2display \"my/files/mdst_*.root\".\n\n"

              "  Using .sroot files, http:// or root:// URLs is also possible.\n\n"

              "  Configuration of the display can be changed from the defaults by\n"
              "  editing display/examples/display.py.\n"
              "  See output of 'basf2 -m Display' for available options.\n";
    return 1;
  }
  std::string fullPath = Belle2::FileSystem::findFile("/display/examples/display.py");
  std::string s = "basf2 " + fullPath + " -i \"" + std::string(argv[1]) + "\"";
  return system(s.c_str());
}
