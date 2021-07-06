/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <daq/hbasf2/utils/HistogramMapping.h>

#include <framework/pcore/MsgHandler.h>
#include <framework/logging/Logger.h>

#include <boost/range/combine.hpp>

#include <TDirectory.h>

using namespace Belle2;

HistogramMapping::HistogramMapping(std::unique_ptr<Belle2::EvtMessage> msg)
{
  m_histograms.clear();

  Belle2::MsgHandler msgHandler;
  std::vector<TObject*> objects;
  std::vector<std::string> names;
  msgHandler.decode_msg(msg.get(), objects, names);

  B2ASSERT("Objects and names need to align", names.size() == objects.size());

  for (const auto& keyValue : boost::combine(names, objects)) {
    std::string key;
    TObject* object;
    boost::tie(key, object) = keyValue;

    TH1* histogram = dynamic_cast<TH1*>(object);
    if (histogram == nullptr) {
      B2WARNING("Object " << key << " is not a histogram!");
      continue;
    }

    m_histograms.insert({key, std::unique_ptr<TH1>(histogram)});
    m_histograms[key]->SetName(key.c_str());
  }
}

void HistogramMapping::operator+=(const HistogramMapping& rhs)
{
  for (auto& keyValue : rhs.m_histograms) {
    const auto& key = keyValue.first;
    const auto& histogram = keyValue.second;

    auto lhsIterator = m_histograms.find(key);
    if (lhsIterator == m_histograms.end()) {
      B2DEBUG(100, "Creating new histogram with name " << key << ".");
      auto* copiedHistogram = dynamic_cast<TH1*>(histogram->Clone());
      m_histograms.insert({key, std::unique_ptr<TH1>(copiedHistogram)});
    } else {
      m_histograms[key]->Add(histogram.get());
    }
  }
}

void HistogramMapping::write() const
{
  for (const auto& [key, histogram] : m_histograms) {
    histogram->SetDirectory(gDirectory);
    histogram->Write();
  }
}

void HistogramMapping::clear()
{
  m_histograms.clear();
}

bool HistogramMapping::empty() const
{
  return m_histograms.empty();
}

void HistogramMapping::printMe() const
{
  for (const auto& [key, histogram] : m_histograms) {
    B2INFO(key << ": " << histogram->GetName() << " -> " << histogram->GetEntries());
  }
}

std::unique_ptr<Belle2::EvtMessage> HistogramMapping::toMessage() const
{
  Belle2::MsgHandler msgHandler;

  int objectCounter = 0;
  for (const auto& [key, histogram] : m_histograms) {
    msgHandler.add(histogram.get(), key);
    objectCounter++;
  }

  std::unique_ptr<Belle2::EvtMessage> msg(msgHandler.encode_msg(Belle2::ERecordType::MSG_EVENT));
  (msg->header())->reserved[0] = 0;
  (msg->header())->reserved[1] = objectCounter;

  return msg;
}