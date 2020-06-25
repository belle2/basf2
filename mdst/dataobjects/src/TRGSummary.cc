#include <mdst/dataobjects/TRGSummary.h>

#include <framework/logging/Logger.h>
#include <framework/database/DBObjPtr.h>
#include <mdst/dbobjects/TRGGDLDBInputBits.h>
#include <mdst/dbobjects/TRGGDLDBFTDLBits.h>

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

bool TRGSummary::test() const
{
  for (unsigned int word = 0; word < c_ntrgWords; word++) {
    if (m_psnmBits[word] != 0) return true;
  }
  return false;
}

bool TRGSummary::testInput(unsigned int bit) const
{
  if (bit > c_trgWordSize * c_ntrgWords) {
    B2ERROR("Requested input trigger bit number is out of range" << LogVar("bit", bit));
    return false;
  }
  int iWord = bit / c_trgWordSize;
  int iBit = bit % c_trgWordSize;
  return (m_inputBits[iWord] & (1 << iBit)) != 0;
}

bool TRGSummary::testFtdl(unsigned int bit) const
{
  if (bit > c_trgWordSize * c_ntrgWords) {
    B2ERROR("Requested ftdl trigger bit number is out of range" << LogVar("bit", bit));
    return false;
  }
  int iWord = bit / c_trgWordSize;
  int iBit = bit % c_trgWordSize;
  return (m_ftdlBits[iWord] & (1 << iBit)) != 0;
}

bool TRGSummary::testPsnm(unsigned int bit) const
{
  if (bit > c_trgWordSize * c_ntrgWords) {
    B2ERROR("Requested psnm trigger bit number is out of range" << LogVar("bit", bit));
    return false;
  }
  int iWord = bit / c_trgWordSize;
  int iBit = bit % c_trgWordSize;
  return (m_psnmBits[iWord] & (1 << iBit)) != 0;
}

unsigned int TRGSummary::getInputBitNumber(const std::string& name) const
{
  static DBObjPtr<TRGGDLDBInputBits> inputBits;

  for (unsigned int bit = 0; bit < c_trgWordSize * c_ntrgWords; bit++) {
    if (std::string(inputBits->getinbitname((int)bit)) == name) {
      return bit;
    }
  }

  B2ERROR("The requested input trigger name does not exist" << LogVar("name", name));
  return c_trgWordSize * c_ntrgWords;
}

unsigned int TRGSummary::getOutputBitNumber(const std::string& name) const
{
  static DBObjPtr<TRGGDLDBFTDLBits> ftdlBits;

  for (unsigned int bit = 0; bit < c_trgWordSize * c_ntrgWords; bit++) {
    if (std::string(ftdlBits->getoutbitname((int)bit)) == name) {
      return bit;
    }
  }

  B2ERROR("The requested output trigger name does not exist" << LogVar("name", name));
  return c_trgWordSize * c_ntrgWords;
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
      (getFtdlBits(currentWord) & ((unsigned int)1 << currentBitInWord)) > 0;
    const auto psnmBit =
      (getPsnmBits(currentWord) & ((unsigned int)1 << currentBitInWord)) > 0;
    const auto inputBit = (getInputBits(currentWord)
                           & ((unsigned int)1 << currentBitInWord)) > 0;

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
