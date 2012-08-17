#include <tracking/modules/trackFitChecker/VertexFitCheckerModule.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>

#include <geometry/GeometryManager.h>
#include <geometry/bfieldmap/BFieldMap.h>
//#include <generators/dataobjects/MCParticle.h>
#include <tracking/gfbfield/GFGeant4Field.h>


#include <GFTrack.h>
#include <GFException.h>
#include <GFTrackCand.h>
#include <GFConstField.h>
#include <GFFieldManager.h>

#include <GFRaveVertexFactory.h>
#include <GFRaveVertex.h>

#include <TVector3.h>
#include <TMatrixD.h>

#include <iostream>


using namespace Belle2;

using std::string;
using std::vector;

using std::cout;
using std::endl;


using boost::accumulators::mean;
using boost::accumulators::median;
using boost::accumulators::count;
using boost::accumulators::variance;
REG_MODULE(VertexFitChecker)


VertexFitCheckerModule::VertexFitCheckerModule() : Module()
{
  setDescription("Wrapper module for GFRave. Will get GFTracks from the dataStore and put fitted GFRaveVertices onto the dataStore.");
  setPropertyFlags(c_ParallelProcessingCertified | c_InitializeInProcess);

  addParam("robustTests", m_robust, "activate additional robust statistical tests (median and MAD)", false);
  addParam("writeToRootFile", m_writeToRootFile, "Set to True if you want the data from the statistical tests written into a root file", false);
  addParam("writeToTextFile", m_writeToFile, "Set to True if you want the results of the statistical tests written out in a normal text file", false);
  addParam("outputFileName", m_dataOutFileName, "A common name for all output files of this module. Suffixes to distinguish them will be added automatically", string("vertexFitChecker"));
  addParam("trackPValueCut", m_trackPValueCut, "if one track in a vertex has a p value lower than pValueCut, the vertex will be excluded from the statistical tests", -1.0);
  addParam("tertexPValueCut", m_vertexPValueCut, "if a vertex has a p value lower than vertexPValueCut, the vertex will be excluded from the statistical tests", -1.0);

}


void VertexFitCheckerModule::initialize()
{
  //setup genfit geometry and magneic field in case you what to used data saved on disc because then the genifitter modul was not run
  // convert the geant4 geometry to a TGeo geometry
  geometry::GeometryManager& geoManager = geometry::GeometryManager::getInstance();
  geoManager.createTGeoRepresentation();
  //pass the magnetic field to genfit
  GFFieldManager::getInstance()->init(new GFGeant4Field());

  if (m_writeToRootFile == true) {
    string testDataFileName = m_dataOutFileName + "StatData.root";
    m_rootFilePtr = new TFile(testDataFileName.c_str(), "RECREATE");
    m_statDataTreePtr = new TTree("m_statDataTreePtr", "treeFromVertexFitChecker");
    //init objects to store track wise data
    registerVertexWiseData("pValue");
  } else {
    m_rootFilePtr = NULL;
    m_statDataTreePtr = NULL;
  }
  m_processedVertices = 0;
  m_badVertexPValueVertices = 0;
  m_badTrackPValueVertices = 0;



}

void VertexFitCheckerModule::beginRun()
{


}

void VertexFitCheckerModule::event()
{
  StoreObjPtr<EventMetaData> eventMetaDataPtr("EventMetaData", DataStore::c_Event);
  const int eventCounter = eventMetaDataPtr->getEvent();

  B2DEBUG(100, "********   VertexFitCheckerModule  processing event number: " << eventCounter << " ************");
//   StoreArray<GFTrack> gfTracks;
//   const int nGfTracks = gfTracks.getEntries();
  //input
  StoreArray<GFRaveVertex> vertices;

  const int nVertices = vertices.getEntries();

  for (int iVertex = 0; iVertex not_eq nVertices; ++iVertex) {

    const GFRaveVertex* aGFRaveVertexPtr = vertices[iVertex];
    const double chi2 = aGFRaveVertexPtr->getChi2();
    const int ndf = aGFRaveVertexPtr->getNdf();
    const double pValue = TMath::Prob(chi2, ndf);
    //cout << "chi2,ndf,p: " << chi2 << " " << ndf << " " << pValue << endl;
    if (pValue < m_vertexPValueCut) {
      ++m_badVertexPValueVertices;
      return;
    }
    const int nTracks = aGFRaveVertexPtr->getNTracks();
    for (int i = 0; i not_eq nTracks; ++i) {
      double pValueOfTrack = aGFRaveVertexPtr->getParameters(i)->getRep()->getPVal();
      if (pValueOfTrack < m_trackPValueCut) { // if contributing track is bad ignore this vertex
        ++m_badTrackPValueVertices;
        return;
      }
    }


    fillVertexWiseData("pValue", pValue);
    const TVector3 vertexPos = aGFRaveVertexPtr->getPos();
    const TMatrixD vertexCov = aGFRaveVertexPtr->getCov();


    if (m_writeToRootFile == true) {
      m_statDataTreePtr->Fill();
    }
    ++m_processedVertices;
  }

}
void VertexFitCheckerModule::endRun()
{




}
void VertexFitCheckerModule::terminate()
{

  B2INFO("Now following the results from the vertexFitChecker module");

  if (m_badVertexPValueVertices not_eq 0) {
    B2WARNING(m_badVertexPValueVertices << " vertices had a p value smaller than " << m_vertexPValueCut << " and were not included in the statistical tests");
  }
  if (m_badTrackPValueVertices not_eq 0) {
    B2WARNING(m_badTrackPValueVertices << " vertices had at least on track with a p value smaller than " << m_trackPValueCut << " and were not included in the statistical tests");
  }
  if (m_processedVertices <= 1) {
    B2WARNING("Only " << m_processedVertices << " vertices were processed. Statistics cannot be computed.");
  } else {
    m_textOutput << "Number of processed Vertices: " << m_processedVertices << "\n";

    // loop over all vertex wise data and print it to terminal and/or to text file
    std::map<std::string, StatisticsContainer >::iterator  iter =  m_vertexWiseDataSamples.begin();
    std::map<std::string, StatisticsContainer >::const_iterator iterMax = m_vertexWiseDataSamples.end();
    while (iter not_eq iterMax) {
      printVertexWiseStatistics(iter->first);
      ++iter;
    }
    //write out the test results
    B2INFO("\n" << m_textOutput.str());
    if (m_writeToFile == true) {
      std::ofstream testOutputToFile((m_dataOutFileName + "StatTests.txt").c_str());
      testOutputToFile << m_textOutput.str();
      testOutputToFile.close();
    }

  }

  if (m_statDataTreePtr not_eq NULL) {
    m_rootFilePtr->cd(); //important! without this the framework root I/O (SimpleOutput etc) could mix with the root I/O of this module
    m_statDataTreePtr->Write();
    m_rootFilePtr->Close();


  }
}

void VertexFitCheckerModule::printVertexWiseStatistics(const string& nameOfDataSample, const bool count)
{
  StatisticsContainer&  dataSample = m_vertexWiseDataSamples[nameOfDataSample];

  m_textOutput << "Information on " << nameOfDataSample << "\nmean\tstd";
  if (m_robust == true) {
    m_textOutput << "\tmedian\tMAD std\toutlier";
  }
  if (count == true) {
    m_textOutput << "\tcount";
  }
  m_textOutput << "\n";
  m_textOutput << std::fixed << mean(dataSample) << "\t" << sqrt(variance(dataSample));

  if (m_robust == true) {
    double madScalingFactor =  m_madScalingFactors[nameOfDataSample];
    if (madScalingFactor > 0.001) {
      double aMedian = median(dataSample);
      double scaledMad = madScalingFactor * calcMad(m_vertexWiseData[nameOfDataSample], aMedian);
      int nOutliers = countOutliers(m_vertexWiseData[nameOfDataSample], aMedian, scaledMad, 4);
      m_textOutput << "\t" << aMedian << "\t" << scaledMad << "\t" << nOutliers; // << "\t" << calcMedian(m_vertexWiseData[nameOfDataSample]);
    } else {
      m_textOutput << "\tcannot compute MAD std";
    }
  }
  if (count == true) {
    m_textOutput << "\t" << boost::accumulators::count(dataSample);
  }
  m_textOutput << "\n";
}

void VertexFitCheckerModule::registerVertexWiseData(const string& nameOfDataSample)
{
  if (m_writeToRootFile == true) {
    m_vertexWiseDataForRoot[nameOfDataSample] = float(-999);
    m_statDataTreePtr->Branch(nameOfDataSample.c_str(), &(m_vertexWiseDataForRoot[nameOfDataSample]));
  }
}

void VertexFitCheckerModule::fillVertexWiseData(const string& nameOfDataSample, const double newData)
{
  m_vertexWiseDataSamples[nameOfDataSample](newData);
  if (m_writeToRootFile == true) {
    m_vertexWiseDataForRoot[nameOfDataSample] = float(newData);
  }
  if (m_robust == true) {
    m_vertexWiseData[nameOfDataSample].push_back(newData);
  }
}
