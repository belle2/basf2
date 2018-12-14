#include <mdst/dataobjects/TRGSummary.h>

#include <framework/logging/Logger.h>

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

unsigned int TRGSummary::getPrescale(unsigned int bit) const
{
  if (bit > c_trgWordSize * c_ntrgWords) {
    B2ERROR("Requested prescale trigger bit number is out of range" << LogVar("bit", bit));
    return 0;
  }
  int iWord = bit / c_trgWordSize;
  int iBit = bit % c_trgWordSize;
  return m_prescaleBits[iWord][iBit];
}

unsigned int TRGSummary::getInputBitNumber(const std::string& name) const
{
  static std::string inputBitNames[c_trgWordSize * c_ntrgWords] = {
    "t3_0", "t3_1", "t3_2", "t2_0", "t2_1", "t2_2", "cdc_open90", "cdc_active", "cdc_b2b3", "cdc_b2b5",
    "cdc_b2b7", "cdc_b2b9", "ehigh", "elow", "elum", "ecl_bha", "bha_0", "bha_1", "bha_2", "bha_3",
    "bha_4", "bha_5", "bha_6", "bha_7", "bha_8", "bha_9", "bha_10", "bha_11", "bha_12", "bha_13",
    "c_0", "c_1", "c_2", "c_3", "ebg_0", "ebg_1", "ebg_2", "ecl_active", "ecl_tim_fwd", "ecl_tim_brl",
    "ecl_tim_bwd", "ecl_phys", "top_0", "top_1", "top_2", "top_bb", "top_active", "klm_hit", "klm_0", "klm_1",
    "klm_2", "klm_3", "klmb2b_0", "klmb2b_1", "klmb2b_2", "revo", "her_kick", "ler_kick", "bha_delay", "pseud_rand",
    "plsin", "poiin", "periodin", "veto", "n1_0", "n1_1", "n1_2", "n2_0", "n2_1", "n2_2",
    "cdcecl_0", "cdcecl_1", "cdcecl_2", "cdcklm_0", "cdcklm_1", "cdcklm_2", "nim0", "nim1", "nim2", "nim3",
    "t3_3", "t2_3", "n1_3", "n2_3", "cdcecl_3", "cdcklm_3", "u2_0", "u2_1", "u2_2", "ecl_oflo",
    "ecl_3dbha", "lml_0", "lml_1", "lml_2", "lml_3", "lml_4", "lml_5", "lml_6", "lml_7", "lml_8", // 100
    "lml_9", "lml_10", "samhem", "opohem", "d_b2b3", "d_b2b5", "d_b2b7", "d_b2b9", "p_b2b3", "p_b2b5",
    "p_b2b7", "p_b2b9", "track", "trkflt", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A",
    "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A",
    "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A",
    "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A",
    "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A",
    "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A",
    "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A",
    "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A",
    "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A"
  };

  for (unsigned int bit = 0; bit < c_trgWordSize * c_ntrgWords; bit++) {
    if (inputBitNames[bit] == name) {
      return bit;
    }
  }

  B2ERROR("The requested input trigger name does not exist" << LogVar("name", name));
  return c_trgWordSize * c_ntrgWords;
}

unsigned int TRGSummary::getOutputBitNumber(const std::string& name) const
{
  static std::string outputBitNames[c_trgWordSize * c_ntrgWords] = {
    "zzz", "zzzo", "fff", "fffo", "zz", "zzo", "ff", "ffo", "hie", "lowe",
    "lume", "c2", "c3", "c4", "c5", "bha", "bha_trk", "bha_brl", "bha_ecp", "g_high",
    "g_c1", "gg", "mu_pair", "mu_b2b", "revo", "rand", "bg", "ecltiming", "nim0", "nima03",
    "nimo03", "period", "eclnima03", "eclnimo03", "pls", "poi", "klmhit", "f", "fe", "ffe",
    "fc", "ffc", "cdctiming", "cdcbb", "nim1c", "c1n0", "c1n1", "c1n2", "c1n3", "c1n4",
    "c2n1", "c2n2", "c2n3", "c2n4", "cdcecl1", "cdcecl2", "cdcecl3", "cdcecl4", "cdcklm1", "cdcklm2",
    "cdcklm3", "cdcklm4", "ffb", "uuu", "uuuo", "uub", "uuo", "c1hie", "c1lume", "n1hie",
    "n1lume", "c3hie", "c3lume", "n3hie", "n3lume", "eed", "fed", "fp", "bha3d", "shem",
    "ohem", "lml0", "lml1", "lml2", "lml3", "lml4", "lml5", "lml6", "lml7", "lml8",
    "lml9", "lml10",  "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", // 100
    "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A",
    "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A",
    "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A",
    "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A",
    "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A",
    "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A",
    "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A",
    "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A",
    "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A",
    "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A"
  };

  for (unsigned int bit = 0; bit < c_trgWordSize * c_ntrgWords; bit++) {
    if (outputBitNames[bit] == name) {
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
