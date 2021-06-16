#include <framework/utilities/ColorPalette.h>

#include <TColor.h>

#include <string>
#include <map>
#include <array>
#include <stdexcept>

namespace Belle2::TangoPalette {
  const char* getHex(const std::string& tangoName, int tangoId)
  {
    using ColorTuple = std::array<const char*, 3>;
    static std::map<std::string, ColorTuple> tango;
    if (tango.empty()) {
      tango["Aluminium"] =   ColorTuple({"#eeeeec",  "#d3d7cf",  "#babdb6"});
      tango["Butter"] =      ColorTuple({"#fce94f",  "#edd400",  "#c4a000"});
      tango["Chameleon"] =   ColorTuple({"#8ae234",  "#73d216",  "#4e9a06"});
      tango["Orange"] =      ColorTuple({"#fcaf3e",  "#f57900",  "#ce5c00"});
      tango["Chocolate"] =   ColorTuple({"#e9b96e",  "#c17d11",  "#8f5902"});
      tango["Sky Blue"] =    ColorTuple({"#729fcf",  "#3465a4",  "#204a87"});
      tango["Plum"] =        ColorTuple({"#ad7fa8",  "#75507b",  "#5c3566"});
      tango["Slate"] =       ColorTuple({"#888a85",  "#555753",  "#2e3436"});
      tango["Scarlet Red"] = ColorTuple({"#ef2929",  "#cc0000",  "#a40000"});
    }

    if (tangoId < 1 or tangoId > 3)
      throw std::runtime_error("tangoId must be 1, 2, or 3!");
    return tango.at(tangoName)[tangoId - 1];
  }

  int getTColorID(const std::string& tangoName, int tangoId)
  {
    return TColor::GetColor(getHex(tangoName, tangoId));
  }
}
