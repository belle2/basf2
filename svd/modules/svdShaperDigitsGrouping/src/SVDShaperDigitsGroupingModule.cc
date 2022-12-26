/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <svd/modules/svdShaperDigitsGrouping/SVDShaperDigitsGroupingModule.h>

#include <framework/logging/Logger.h>
#include <framework/utilities/FileSystem.h>

#include <svd/dataobjects/SVDEventInfo.h>

#include <iostream>
#include <vector>

using namespace std;
using namespace Belle2;


REG_MODULE(SVDShaperDigitsGrouping);

SVDShaperDigitsGroupingModule::SVDShaperDigitsGroupingModule() :
  Module(),
  m_cutAdjacent(3.0), m_useDB(true)
{
  setDescription("Calculate CoG3 time of each strip and grouping them.");
  setPropertyFlags(c_ParallelProcessingCertified);

  // 1. Collections.
  addParam("EventInfo", m_svdEventInfoName,
           "SVDEventInfo collection name.", string("SVDEventInfo"));
  addParam("ShaperDigits", m_storeShaperDigitsName,
           "SVDShaperDigits collection name.", string(""));

  // 2. Clustering
  addParam("useDB", m_useDB,
           "if False, use clustering and reconstruction configuration module parameters", m_useDB);
  addParam("AdjacentSN", m_cutAdjacent,
           "minimum SNR for strips to be considered for clustering. Overwritten by the dbobject, unless you set useDB = False.",
           m_cutAdjacent);

  // 3. Modification parameters:
  addParam("signalRangeLow", m_signalRangeLow,
           "Expected time range of signal hits.",
           double(-50.));
  addParam("signalRangeHigh", m_signalRangeHigh,
           "Expected time range of signal hits.",
           double(50.));
  addParam("factor", m_factor,
           "Fine divisions of histogram.",
           int(2));
  addParam("iteration", m_iteration,
           "Number of summations of the histogram.",
           int(4));
  addParam("useOnlyOneGroup", m_useOnlyOneGroup,
           "Only one group is kept.",
           bool(false));
  addParam("timeBasedSort", m_timeBasedSort,
           "Clusters belonging to the group nearest to zero is first.",
           bool(false));

}



void SVDShaperDigitsGroupingModule::initialize()
{
  m_storeDigits.isRequired(m_storeShaperDigitsName);

  if (m_useOnlyOneGroup) B2WARNING("Only the group nearest to zero is selected.");
  if (m_factor <= 0) B2WARNING("factor is set to zero or less. Module is ineffective.");
  if (m_signalRangeHigh - m_signalRangeLow < 10.) B2FATAL("signalRange should not be less than 10 (hard-coded).");

  B2DEBUG(1, "SVDShaperDigitsGroupingModule \nsvdShaperDigits: " << m_storeDigits.getName());
  if (m_factor > 0) {
    B2INFO("SVDShaperDigitsGrouping : signalRange               = [" << m_signalRangeLow << "," << m_signalRangeHigh << "]");
    B2INFO("SVDShaperDigitsGrouping : factor                    = " << m_factor);
    B2INFO("SVDShaperDigitsGrouping : iteration                 = " << m_iteration);
  } // if(m_factor > 0) {
}


void SVDShaperDigitsGroupingModule::beginRun()
{
  //read sampling time from HardwareClockSettings
  if (m_hwClock.isValid())
    m_apvClockPeriod = 1. / m_hwClock->getClockFrequency(Const::EDetector::SVD, "sampling");

  if (std::isnan(m_apvClockPeriod))
    B2FATAL("Forbidden state!!");
}

void SVDShaperDigitsGroupingModule::event()
{
  int nDigits = m_storeDigits.getEntries();
  if (nDigits == 0 || m_factor <= 0)
    return;

  m_clsInSignalRange = 0;
  m_tRangeLow        =  10000.;
  m_tRangeHigh       = -10000.;

  //loop over the SVDShaperDigits
  for (SVDShaperDigit& currentDigit : m_storeDigits) {

    calculateStripTime(currentDigit);
    float strptime = currentDigit.getTime();

    if (strptime > m_signalRangeLow && strptime < m_signalRangeHigh) m_clsInSignalRange++;

    if (strptime < m_tRangeLow)  m_tRangeLow  = strptime;
    if (strptime > m_tRangeHigh) m_tRangeHigh = strptime;

  } // for (SVDShaperDigit& currentDigit : m_storeDigits) {

  // assign group number to strips
  assignGroupId();

}




void SVDShaperDigitsGroupingModule::calculateStripTime(SVDShaperDigit& currentDigit)
{

  if (m_useDB)
    m_cutAdjacent = m_ClusterCal.getMinAdjSNR(currentDigit.getSensorID(), currentDigit.isUStrip());

  //Ignore digits with insufficient signal
  float thisNoise = m_NoiseCal.getNoise(currentDigit.getSensorID(), currentDigit.isUStrip(), currentDigit.getCellID());
  int thisCharge = currentDigit.getMaxADCCounts();
  B2DEBUG(20, "Noise = " << thisNoise << " ADC, MaxSample = " << thisCharge << " ADC");

  if ((float)thisCharge / thisNoise < m_cutAdjacent)
    return;

  float stripTime = 0;
  float stripSumAmplitudes = 0;

  Belle2::SVDShaperDigit::APVFloatSamples samples = currentDigit.getSamples();

#define CoG3Alg
#ifdef CoG6Alg

  int firstFrame = 0;
  for (int k = 0; k < static_cast<int>(samples.size()); k ++) {
    stripTime += k * samples.at(k);
    stripSumAmplitudes += samples.at(k);
  }

#endif
#ifdef CoG3Alg

  //take the MaxSum 3 samples
  SVD::SVDMaxSumAlgorithm maxSum = SVD::SVDMaxSumAlgorithm(samples);
  std::vector<float> selectedSamples = maxSum.getSelectedSamples();
  int firstFrame = maxSum.getFirstFrame();

  auto begin = selectedSamples.begin();
  const auto end = selectedSamples.end();

  for (auto step = 0.; begin != end; ++begin, step += m_apvClockPeriod) {
    stripSumAmplitudes += static_cast<float>(*begin);
    stripTime += static_cast<float>(*begin) * step;
  }
#endif

  if (stripSumAmplitudes != 0) {
    stripTime /= (stripSumAmplitudes);
  } else {
    stripTime = -1;
    B2WARNING("Trying to divide by 0 (ZERO)! Sum of amplitudes is nullptr! Skipping this SVDShaperDigit!");
  }

  if (isnan(m_triggerBin))
    B2FATAL("OOPS, we can't continue, you have to set the trigger bin!");

#ifdef CoG6Alg

  stripTime -= m_PulseShapeCal.getPeakTime(currentDigit.getSensorID(), currentDigit.isUStrip(), currentDigit.getCellID());
  stripTime =  m_CoG6TimeCal.getCorrectedTime(currentDigit.getSensorID(), currentDigit.isUStrip(), currentDigit.getCellID(),
                                              stripTime, m_triggerBin);

#endif
#ifdef CoG3Alg

  //cellID = 10 not used for calibration
  stripTime = m_CoG3TimeCal.getCorrectedTime(currentDigit.getSensorID(), currentDigit.isUStrip(), 10, stripTime, m_triggerBin);

#endif

  //first take Event Informations:
  StoreObjPtr<SVDEventInfo> temp_eventinfo(m_svdEventInfoName);
  if (!temp_eventinfo.isValid())
    m_svdEventInfoName = "SVDEventInfoSim";
  StoreObjPtr<SVDEventInfo> eventinfo(m_svdEventInfoName);
  if (!eventinfo) B2ERROR("No SVDEventInfo!");

  // now go into FTSW time reference frame
  stripTime = eventinfo->getTimeInFTSWReference(stripTime, firstFrame);

  currentDigit.setTime(stripTime);
  // std::cout<<" "<<currentDigit.getTime();

}




void SVDShaperDigitsGroupingModule::assignGroupId()
{

  B2DEBUG(1, " m_clsInSignalRange " << m_clsInSignalRange
          << " m_tRangeLow " << m_tRangeLow << " m_tRangeHigh " << m_tRangeHigh);

  int xbin = m_clsInSignalRange * (m_tRangeHigh - m_tRangeLow) / (m_signalRangeHigh - m_signalRangeLow);
  xbin *= m_factor;
  if (xbin < 2) xbin = 2;


  /** declaring histogram */
  TH1D h_clsTime[2];
  h_clsTime[0] = TH1D("h_clsTime_0", "h_clsTime_0", xbin, m_tRangeLow, m_tRangeHigh);
  h_clsTime[1] = TH1D("h_clsTime_1", "h_clsTime_1", xbin, m_tRangeLow, m_tRangeHigh);
  for (SVDShaperDigit& currentDigit : m_storeDigits)
    h_clsTime[0].Fill(currentDigit.getTime());

  bool currentHisto = false;
  int counter = 0;
  bool isWhile = true;
  while (isWhile) {
    for (int ij = 1; ij <= xbin; ij++) {
      double sumc = h_clsTime[currentHisto].GetBinContent(ij);
      double suml = (ij == 1    ? 0. : h_clsTime[currentHisto].GetBinContent(ij - 1));
      double sumr = (ij == xbin ? 0. : h_clsTime[currentHisto].GetBinContent(ij + 1));
      double sum = suml + sumc + sumr;
      h_clsTime[!currentHisto].SetBinContent(ij, sum);
    } // for(int ij=1;ij<=xbin;ij++) {
    currentHisto = !currentHisto;
    counter++;
    if (counter > m_iteration) isWhile = false;
  } // while(isWhile) {


  /** finalized the groups */
  int groupBegin = -1; int groupEnd = -1;
  std::vector<std::tuple<double, double, int>> groupInfo; // start, end, totCls
  for (int ij = 1; ij <= xbin; ij++) {
    double sum = h_clsTime[currentHisto].GetBinContent(ij);
    // finding group
    if (sum > 0 && groupBegin < 0 && groupEnd < 0) { groupBegin = ij;}
    if ((sum <= 0 || ij == xbin) && groupBegin > 0 && groupEnd < 0) {
      groupEnd = ij - 1;
      int clsInGroup = h_clsTime[currentHisto].Integral(groupBegin, groupEnd);
      double beginPos = h_clsTime[currentHisto].GetXaxis()->GetBinLowEdge(groupBegin);
      double endPos   = h_clsTime[currentHisto].GetXaxis()->GetBinLowEdge(groupEnd) +
                        h_clsTime[currentHisto].GetXaxis()->GetBinWidth(groupEnd);
      if (clsInGroup > 1) {
        groupInfo.push_back(std::make_tuple(beginPos, endPos, clsInGroup));
        B2DEBUG(1, " group " << ij
                << " beginPos " << beginPos << " endPos " << endPos
                << " totCls " << clsInGroup);
      }
      groupBegin = groupEnd = -1; // reset for new group
    }
  }

  // sorting groups in descending cluster-counts or distance from zero
  // this should help speed up the next process
  std::tuple<double, double, int> key;
  for (int ij = 1; ij < int(groupInfo.size()); ij++) {
    key = groupInfo[ij];
    int kj = ij - 1;
    if (!m_timeBasedSort) // sort in cluster size
      while ((kj >= 0) &&
             ((std::get<2>(groupInfo[kj])) < (std::get<2>(key)))) {
        groupInfo[kj + 1] = groupInfo[kj];
        kj--;
      } else    // sort in distance from zero
      while ((kj >= 0) &&
             std::min(std::fabs(std::get<0>(groupInfo[kj])), std::fabs(std::get<1>(groupInfo[kj]))) > std::min(std::fabs(std::get<0>(key)),
                 std::fabs(std::get<1>(key)))) {
        groupInfo[kj + 1] = groupInfo[kj];
        kj--;
      }
    groupInfo[kj + 1] = key;
  }

  if (m_useOnlyOneGroup && int(groupInfo.size())) // keep only one group
    groupInfo.resize(1);

  std::vector<int> rejectedCls;
  rejectedCls.assign(int(m_storeDigits.getEntries()), -1);
  int totGroups = int(groupInfo.size());
  for (int ij = 0; ij < totGroups; ij++) {
    auto beginPos    = std::get<0>(groupInfo[ij]);
    auto endPos      = std::get<1>(groupInfo[ij]);
    auto totCls      = std::get<2>(groupInfo[ij]);

    B2DEBUG(1, " group " << ij
            << " beginPos " << beginPos << " endPos " << endPos
            << " totCnt " << totCls);
    int rejectedCount = 0;
    for (int jk = 0; jk < int(rejectedCls.size()); jk++) {
      int place = rejectedCls[jk] < 0 ? jk : rejectedCls[jk];
      double strptime = m_storeDigits[place]->getTime();
      if (strptime >= beginPos && strptime <= endPos) {
        m_storeDigits[place]->setTimeGroupId(ij);
        B2DEBUG(1, "   accepted strip " << place
                << " time " << strptime);
      } else {
        B2DEBUG(1, "     rejected strip " << place
                << " time " << strptime);
        if (ij == totGroups - 1) { // leftover clusters
          m_storeDigits[place]->setTimeGroupId(-1);
          B2DEBUG(1, "     leftover strip " << place
                  << " GroupId " << m_storeDigits[place]->getTimeGroupId());
        } else {
          rejectedCls[rejectedCount++] = place;
        }
      }
    } // for(int jk=0;jk<int(rejectedCls.size());jk++) {
    rejectedCls.resize(rejectedCount);
  } // for(int ij=0;ij<int(groupInfo.size());ij++) {



}
