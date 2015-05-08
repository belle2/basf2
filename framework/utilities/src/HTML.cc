#include <framework/utilities/HTML.h>
#include <framework/logging/Logger.h>

#include <boost/algorithm/string/replace.hpp>
#include <boost/regex.hpp>

#include <TMatrixFBase.h>

#include <iomanip>
#include <sstream>

using namespace Belle2;
using namespace boost::algorithm;

std::string HTML::getString(const TMatrixFBase& matrix, int precision, bool color)
{
  std::stringstream stream;

  int oldprecision = stream.precision(precision);
  stream << "<table padding=0>";
  const float max = matrix.Max();
  const float min = matrix.Min();
  for (int i = 0; i < matrix.GetNrows(); i++) {
    stream << "<tr>";
    for (int k = 0; k < matrix.GetNcols(); k++) {
      const float value = matrix(i, k);
      stream << "<td align=right";
      if (color and value != 0.0) {
        //0 white, min cyan, max red
        int b = 255;
        int r = 255;
        int g = 255;
        if (value > 0)
          b = g = 255 - 180 * value / ((value > 0) ? max : min);
        else
          r = 255 - 200 * value / ((value > 0) ? max : min);

        stream << " bgcolor=" << std::setfill('0') << std::hex;
        stream << std::setw(2) << r;
        stream << std::setw(2) << g;
        stream << std::setw(2) << b;
        stream << std::dec << std::setfill(' ');
      }
      stream << ">" << std::setw(11) << value;
      stream << "</td>";
    }
    stream << "</tr>";
  }
  stream << "</table>";
  stream << "<br>";

  stream.precision(oldprecision);
  return stream.str();
}

std::string HTML::htmlToPlainText(const std::string& html)
{
  std::string tmp = html;
  //conversions to plaintext
  replace_all(tmp, "<br>", "\n");
  replace_all(tmp, "</p>", "\n");
  replace_all(tmp, "</tr>", "\n");

  //remove all unknown tags (non-greedy match)
  const static boost::regex tagRegex("<.*?>");
  tmp = boost::regex_replace(tmp, tagRegex, "");

  //replace entities (important ones at least)
  replace_all(tmp, "&nbsp;", " ");
  replace_all(tmp, "&gt;", ">");
  replace_all(tmp, "&lt;", "<");
  replace_all(tmp, "&amp;", "&"); //must be last
  return tmp;
}
