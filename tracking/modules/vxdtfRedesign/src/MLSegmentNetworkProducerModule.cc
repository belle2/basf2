/*************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2016 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Thomas Madlener                                          *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <tracking/modules/vxdtfRedesign/MLSegmentNetworkProducerModule.h>

#include <framework/logging/Logger.h>

#include <fstream>

using namespace Belle2;
using namespace std;

REG_MODULE(MLSegmentNetworkProducer)

MLSegmentNetworkProducerModule::MLSegmentNetworkProducerModule() : Module()
{
  setDescription("SegmentNetwork Producer Module with a machine learning classifier as three hit filter.");

  addParam("FBDTFileName", m_PARAMfbdtFileName, "file where the FastBDT classifier is stored");
  // addParam("collectMode", m_PARAMcollectMode, "set to true for collecting training data, false for applying the filter", false);

  addParam("networkInputName",
           m_PARAMnetworkInputName,
           "Name of the StoreObjPtr where the network container used in this module is stored", std::string(""));

  addParam("sectorMapName", m_PARAMsecMapName,
           "The name of the SectorMap used for this instance.", string("testMap"));

  addParam("cutValue", m_PARAMcutVal,
           "Cut value to be used for dividing the classifier output into signal (above) and background (below)",
           0.5);

}

void MLSegmentNetworkProducerModule::initialize()
{
  m_network.isRequired(m_PARAMnetworkInputName);

  if (m_PARAMcutVal < 0. || m_PARAMcutVal > 1.) {
    B2ERROR("cutValue set to " << m_PARAMcutVal << " but has to be in [0,1]!");
  }

  setupClassifier(m_PARAMfbdtFileName);
  setupFilter();
}

void MLSegmentNetworkProducerModule::event()
{
  DirectedNodeNetwork<TrackNode, VoidMetaInfo>& hitNetwork = m_network->accessHitNetwork();
  DirectedNodeNetwork<Segment<TrackNode>, CACell>& segmentNetwork = m_network->accessSegmentNetwork();
  vector<Segment<TrackNode>* >& segments = m_network->accessSegments();

  unsigned nAccepted{}, nRejected{}, nLinked{};

  for (const auto& outerHit : hitNetwork.getNodes()) {
    for (const auto& centerHit : outerHit->getInnerNodes()) {
      bool alreadyAdded = false; // skip adding Nodes twice into the network
      for (const auto& innerHit : centerHit->getInnerNodes()) {
        bool accepted = m_filter->accept(*(innerHit->getEntry().m_spacePoint),
                                         *(centerHit->getEntry().m_spacePoint),
                                         *(outerHit->getEntry().m_spacePoint));

        if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 499, PACKAGENAME())) {
          const auto& sp1 = innerHit->getEntry().m_spacePoint;
          const auto& sp2 = centerHit->getEntry().m_spacePoint;
          const auto& sp3 = outerHit->getEntry().m_spacePoint;
          std::array<double, 9> coords{{ sp1->X(), sp1->Y(), sp1->Z(), sp2->X(), sp2->Y(), sp2->Z(), sp3->X(), sp3->Y(), sp3->Z() }};
          double classOut = m_classifier->analyze(coords);
          B2DEBUG(499, "Classifier output: " << classOut << ", cutValue: " << m_PARAMcutVal);
        }

        if (!accepted) { nRejected++; continue; } // don't store combinations which have not been accepted
        nAccepted++;

        Segment<TrackNode>* innerSegment = new Segment<TrackNode>(centerHit->getEntry().m_sector->getFullSecID(),
                                                                  innerHit->getEntry().m_sector->getFullSecID(),
                                                                  &centerHit->getEntry(),
                                                                  &innerHit->getEntry());
        B2DEBUG(999, "buildSegmentNetwork: innerSegment: " << innerSegment->getName());
        DirectedNode<Segment<TrackNode>, CACell>* tempInnerSegmentnode = segmentNetwork.getNode(innerSegment->getID());
        if (tempInnerSegmentnode == nullptr) {
          segments.push_back(innerSegment);
          segmentNetwork.addNode(innerSegment->getID(), *innerSegment);
        } else {
          delete innerSegment;
          innerSegment = &(tempInnerSegmentnode->getEntry());
        }

        if (!alreadyAdded) {
          // create outerSector
          Segment<TrackNode>* outerSegment = new Segment<TrackNode>(outerHit->getEntry().m_sector->getFullSecID(),
                                                                    centerHit->getEntry().m_sector->getFullSecID(),
                                                                    &outerHit->getEntry(),
                                                                    &centerHit->getEntry());
          B2DEBUG(999, "buildSegmentNetwork: outerSegment(freshly created): " << outerSegment->getName() <<
                  " to be linked with inner segment: " << innerSegment->getName());

          DirectedNode<Segment<TrackNode>, CACell>* tempOuterSegmentnode = segmentNetwork.getNode(outerSegment->getID());
          if (tempOuterSegmentnode == nullptr) {
            segments.push_back(outerSegment);
            segmentNetwork.addNode(outerSegment->getID(), *outerSegment);
          } else {
            delete outerSegment;
            outerSegment = &(tempOuterSegmentnode->getEntry());
          }

          B2DEBUG(999, "buildSegmentNetwork: outerSegment (after duplicate check): " << outerSegment->getName() <<
                  " to be linked with inner segment: " << innerSegment->getName());
          segmentNetwork.linkNodes(outerSegment->getID(), innerSegment->getID());
          nLinked++;
          alreadyAdded = true;
          continue;
        }
        segmentNetwork.addInnerToLastOuterNode(innerSegment->getID());
      } // end inner loop
    } // end center loop
  } // end outer loop


  B2DEBUG(10, "MLSegmentNetworkProducerModule::buildSegmentNetwork(): nAccepted/nRejected: " << nAccepted << "/" << nRejected <<
          ", size of nLinked/hitNetwork: " << nLinked << "/" << segmentNetwork.size());

}

void MLSegmentNetworkProducerModule::terminate()
{

}


void MLSegmentNetworkProducerModule::setupClassifier(const std::string& filename)
{
  ifstream filestr(filename);
  if (!filestr.is_open())  {
    B2FATAL("Could not open file: " << filename << " for reading in a FBDTClassifier");
  }

  auto classifier = std::unique_ptr<FBDTClassifier<9> >(new FBDTClassifier<9>());
  classifier->readFromStream(filestr);
  filestr.close();

  m_classifier = std::move(classifier);
}

void MLSegmentNetworkProducerModule::setupFilter()
{
  using RangeT = MLRange<FBDTClassifier<9>, 9, double>;

  auto filter = std::unique_ptr<MLFilter>(new MLFilter(RangeT(m_classifier.get(), m_PARAMcutVal)));
  m_filter = std::move(filter);
}
