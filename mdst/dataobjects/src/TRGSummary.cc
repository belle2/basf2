#include <mdst/dataobjects/TRGSummary.h>

#include <TROOT.h>
#include <TColor.h>

#include <sstream>

using namespace Belle2;

TRGSummary::TRGSummary(unsigned int inputBits[10],
                       unsigned int ftdlBits[10],
                       unsigned int psnmBits[10],
                       ETimingType timType)
{
  for (int i = 0; i < 10; i++) {
    m_inputBits[i] = inputBits[i];
    m_ftdlBits[i] = ftdlBits[i];
    m_psnmBits[i] = psnmBits[i];
  }
  m_timType = timType;
}

std::string TRGSummary::getInfoHTML() const
{
  std::stringstream htmlOutput;

  htmlOutput << "<table>";
  htmlOutput
      << "<tr><td></td><td bgcolor='#cccccc'>GDL Input</td><td bgcolor='#cccccc' colspan='2'>GDL Output</td></tr>";
  htmlOutput
      << "<tr><td>Bit</td><td>Input Bits</td><td>Final Trg DL</td><td>Prescaled Trg and Mask</td></tr>";

  for (unsigned int currentBit = 0;
       currentBit < (c_ntrgWords * c_trgWordSize); currentBit++) {
    htmlOutput << "<tr>";

    const auto currentWord = currentBit / c_trgWordSize;
    const auto currentBitInWord = currentBit % c_trgWordSize;

    const auto ftdlBit =
      (getFtdlBits(currentWord) & (1 << currentBitInWord)) > 0;
    const auto psnmBit =
      (getPsnmBits(currentWord) & (1 << currentBitInWord)) > 0;
    const auto inputBit = (getInputBits(currentWord)
                           & (1 << currentBitInWord)) > 0;

    htmlOutput << "<td>" << currentBit << "(word " << currentWord << " bit "
               << currentBitInWord << ")</td>";
    htmlOutput << outputBitWithColor(inputBit);
    htmlOutput << outputBitWithColor(ftdlBit);
    htmlOutput << outputBitWithColor(psnmBit);
    htmlOutput << "</tr>";
  }
  htmlOutput << "</table>";

  return htmlOutput.str();
}


/** return the td part of an HTML table with green of the bit is > 0 */
std::string TRGSummary::outputBitWithColor(bool bit) const
{
  const std::string colorNeutral = gROOT->GetColor(kWhite)->AsHexString();
  const std::string colorAccept = gROOT->GetColor(kGreen)->AsHexString();

  std::string color = bit > 0 ? colorAccept : colorNeutral;
  std::stringstream outStream;
  outStream << "<td bgcolor=\"" << color << "\">" << bit << "</td>";
  return outStream.str();
}
