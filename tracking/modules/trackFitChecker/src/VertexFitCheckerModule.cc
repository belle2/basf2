#include <tracking/modules/trackFitChecker/VertexFitCheckerModule.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>

#include <geometry/GeometryManager.h>
#include <geometry/bfieldmap/BFieldMap.h>
#include <generators/dataobjects/MCParticle.h>
#include <tracking/gfbfield/GFGeant4Field.h>
#include <tracking/modules/trackFitChecker/TrackFitCheckerModule.h>
//Genfit stuff
#include <GFTrack.h>
#include <GFException.h>
#include <GFTrackCand.h>
#include <GFConstField.h>
#include <GFFieldManager.h>
#include <GFTools.h>
#include <GFMaterialEffects.h>
#include <GFTGeoMaterialInterface.h>
#include <GFRaveVertexFactory.h>
#include <GFRaveVertex.h>
#include <GFTGeoMaterialInterface.h>
//root stuff
#include <TVector3.h>
#include <TMatrixD.h>
#include <TGeoManager.h>
#include <Math/ProbFunc.h>

//C++ std stuff
#include <iostream>
#include <cmath>
#include <numeric>
//#include <Python.h>


using namespace Belle2;

using std::string;
using std::vector;

using std::cout;
using std::endl;
using std::sqrt;
using std::abs;
using std::fixed;

using boost::accumulators::mean;
using boost::accumulators::median;
using boost::accumulators::count;
using boost::accumulators::variance;
REG_MODULE(VertexFitChecker)


VertexFitCheckerModule::VertexFitCheckerModule() : Module()
{
  setDescription("Checks the correctness of the fitted vertices produced by the Vertexer module (GFRave) by conducting several statistical tests on them");
  setPropertyFlags(c_ParallelProcessingCertified | c_InitializeInProcess);

  addParam("robustTests", m_robust, "activate additional robust statistical tests (median and MAD)", false);
  addParam("writeToRootFile", m_writeToRootFile, "Set to True if you want the data from the statistical tests written into a root file", false);
  addParam("writeToTextFile", m_writeToFile, "Set to True if you want the results of the statistical tests written out in a normal text file", false);
  addParam("outputFileName", m_dataOutFileName, "A common name for all output files of this module. Suffixes to distinguish them will be added automatically", string("vertexFitChecker"));
  addParam("trackPValueCut", m_trackPValueCut, "if one track in a vertex has a p value lower than pValueCut, the vertex will be excluded from the statistical tests", -1.0);
  addParam("vertexPValueCut", m_vertexPValueCut, "if a vertex has a p value lower than vertexPValueCut, the vertex will be excluded from the statistical tests", -1.0);
  addParam("trunctationRatio", m_trunctationRatio, "Ratio of the data sample that will be cut away before in the trunctated", 0.01);

}


void VertexFitCheckerModule::initialize()
{

  StoreArray<GFTrack>::required();
  StoreArray<GFRaveVertex>::required();
  StoreArray<MCParticle>::required();
  if (gGeoManager == NULL) { //setup geometry and B-field for Genfit if not already there
    geometry::GeometryManager& geoManager = geometry::GeometryManager::getInstance();
    geoManager.createTGeoRepresentation();
    //pass the magnetic field to genfit
    GFFieldManager::getInstance()->init(new GFGeant4Field());
    GFMaterialEffects::getInstance()->init(new GFTGeoMaterialInterface());
    GFMaterialEffects::getInstance()->setMscModel("Highland");
  }
  //configure the output
  m_textOutput.precision(4);
  if (m_writeToRootFile == true) {
    string testDataFileName = m_dataOutFileName + "StatData.root";
    m_rootFilePtr = new TFile(testDataFileName.c_str(), "RECREATE");
    m_statDataTreePtr = new TTree("m_statDataTreePtr", "treeFromVertexFitChecker");
    //init objects to store track wise data
    registerVertexWiseData("pValue");
    registerVertexWiseVecData("res_vertexPos", 3);
    registerVertexWiseVecData("pullsAndChi2_vertexPos", 4);
    registerInt("vertexFitStatus");
    registerVertexWiseVecData("res_MCVertexPos", 3);

  } else {
    m_rootFilePtr = NULL;
    m_statDataTreePtr = NULL;
  }

  if (m_robust == true) { //set the scaling factors for the MAD. No MAD will be caclulated when
    m_madScalingFactors["res_vertexPos"] = 1.4826; //scaling factor for normal distribute variables
    m_madScalingFactors["pullsAndChi2_vertexPos"] = 1.4826;
    m_madScalingFactors["res_MCVertexPos"] = 1.4826;
    m_trunctationRatios["res_vertexPos"] = m_trunctationRatio; //scaling factor for normal distribute variables
    m_trunctationRatios["pullsAndChi2_vertexPos"] = m_trunctationRatio;
    m_trunctationRatios["res_MCVertexPos"] = m_trunctationRatio;
  }

  m_processedVertices = 0;
  m_badVertexPValueVertices = 0;
  m_badTrackPValueVertices = 0;
  m_vertexNotPureCounter = 0;


  // get parameter from python script
//       PyObject* m = PyImport_AddModule("__main__");
//       if (m) {
//         vector<double> averageagel;;
//         PyObject* v = PyObject_GetAttrString(m, "xVertexParams");
//         if (v) {
//           averageagel = PyInt_AsLong(v);
//           Py_DECREF(v);
//         }
//         //B2INFO("Python averageagel = " << averageagel);
//         //m_arichgp->setAverageAgel(averageagel > 0);
//       }


}

void VertexFitCheckerModule::beginRun()
{


}

void VertexFitCheckerModule::event()
{
  StoreObjPtr<EventMetaData> eventMetaDataPtr("EventMetaData", DataStore::c_Event);
  const int eventCounter = eventMetaDataPtr->getEvent();

  B2DEBUG(100, "********   VertexFitCheckerModule  processing event number: " << eventCounter << " ************");

  StoreArray<MCParticle> mcParticles;
  //const int nMcParticles = mcParticles.getEntries();
  StoreArray<GFTrack> gfTracks;
  const int nGfTracks = gfTracks.getEntries();
  //input

  //find all real vertices that led to fitted tracks with the simulation information
  vector<const MCParticle*> motherParticles;
  for (int i = 0; i not_eq nGfTracks; ++i) {
    int mcIndex = gfTracks[i]->getCand().getMcTrackId();
    const MCParticle* motherParticle = mcParticles[mcIndex]->getMother();
    motherParticles.push_back(motherParticle);
  }

//  struct TrueVertex {
//    TVector3 pos;
//    //oder MCParicle*
//    vector<const GFTrack*> tracks;
//
//  };

  StoreArray<GFRaveVertex> vertices;

  const int nVertices = vertices.getEntries();
  B2DEBUG(100, "there are " << nVertices << " in this event");

  for (int iVertex = 0; iVertex not_eq nVertices; ++iVertex) {

    const GFRaveVertex* aGFRaveVertexPtr = vertices[iVertex];
    const double chi2 = aGFRaveVertexPtr->getChi2();
    const double ndf = aGFRaveVertexPtr->getNdf();
    const double pValue = ROOT::Math::chisquared_cdf_c(chi2, ndf);
//    cout << "chi2,ndf,p: " << chi2 << " " << ndf << " " << pValue << endl;
    if (pValue < m_vertexPValueCut) {
      ++m_badVertexPValueVertices;
      continue; // goto next vertex
    }
    //now extract information from the tracks that rave assigned to the vertex
    vector<const MCParticle*> particlesCommingFromVertex;
    const int nTracks = aGFRaveVertexPtr->getNTracks();
    bool skipVertex = false;
    for (int i = 0; i not_eq nTracks; ++i) {
      const GFRaveTrackParameters* const trackInfo = aGFRaveVertexPtr->getParameters(i);
//      cout << "trackInfo " << trackInfo << endl;
      double pValueOfTrack = trackInfo->getRep()->getPVal();
      B2DEBUG(100, "p value of track " << i << " in vertex " << iVertex << " is " << pValueOfTrack);
      if (pValueOfTrack < m_trackPValueCut) { // if contributing track is bad ignore this vertex
        ++m_badTrackPValueVertices;
        skipVertex = true;
        break;
      }
      particlesCommingFromVertex.push_back(mcParticles[trackInfo->getTrack()->getCand().getMcTrackId()]);
    }
    if (skipVertex == true) {
      continue;
    }

    fillVertexWiseData("pValue", pValue);

    // check if all tracks associated with the vertex coming form the same mother particle
    // if not check if all tracks coming from the same coordinates

    const int nParticlesCommingFromVertex = particlesCommingFromVertex.size();
    const MCParticle* const motherParticle = particlesCommingFromVertex[0]->getMother();
    const TVector3 mcVertexPos =  particlesCommingFromVertex[0]->getVertex();
    bool sameMother = true;
    if (motherParticle == NULL) {
      sameMother = false;
    } else {
      for (int i = 1; i not_eq nParticlesCommingFromVertex; ++i) {
        const MCParticle* const currentMotherParticle = particlesCommingFromVertex[i]->getMother();
        if (currentMotherParticle not_eq motherParticle) {
          sameMother = false;
          break;
        }
      }
    }


    bool samePos = true;
    if (sameMother == false) {
      for (int i = 1; i not_eq nParticlesCommingFromVertex; ++i) {
        TVector3 currentVertexPos = particlesCommingFromVertex[i]->getVertex();
        if (currentVertexPos not_eq mcVertexPos) {
          samePos = false;
          break;
        }
      }
    }

    //cout << sameMother << " " << samePos << "\n";
    if (samePos == false and sameMother == false) {
      ++m_vertexNotPureCounter;
      fillInt("vertexFitStatus", -1);
      ++m_processedVertices;
      m_statDataTreePtr->Fill();
      continue; // goto next vertex
    }

    fillInt("vertexFitStatus", 0); //status 0 means all tracks rave associated with one vertex are really belong to that vertex
    const TVector3 vertexPos = aGFRaveVertexPtr->getPos();
    const TMatrixDSym vertexCov = aGFRaveVertexPtr->getCov();
    //cout << "fitted vertex pos, cov" << endl;
//    vertexPos.Print();
//    vertexCov.Print();
    vector<double> resPos(3);
    resPos[0] = vertexPos[0] - mcVertexPos[0];
    resPos[1] = vertexPos[1] - mcVertexPos[1];
    resPos[2] = vertexPos[2] - mcVertexPos[2];
    fillVertexWiseVecData("res_vertexPos", resPos);

    vector<double> pullPos(4);
    pullPos[0] = resPos[0] / sqrt(vertexCov[0][0]);
    pullPos[1] = resPos[1] / sqrt(vertexCov[1][1]);
    pullPos[2] = resPos[2] / sqrt(vertexCov[2][2]);
    // now the chi2

    TMatrixD resPosMatrix(3, 1, &resPos[0]);
    pullPos[3] = calcChi2(resPosMatrix, vertexCov);
    fillVertexWiseVecData("pullsAndChi2_vertexPos", pullPos);

    vector<double> resMCVertex(3);
    resMCVertex[0] = mcVertexPos[0];
    resMCVertex[1] = mcVertexPos[1];
    resMCVertex[2] = mcVertexPos[2];

    fillVertexWiseVecData("res_MCVertexPos", resMCVertex); //only makes sense with the particle gun...


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

  B2INFO("Now following the results from the VertexFitChecker module");

  if (m_badVertexPValueVertices not_eq 0) {
    B2WARNING(m_badVertexPValueVertices << " vertices had a p value smaller than " << m_vertexPValueCut << " and were not included in the statistical tests");
  }
  if (m_badTrackPValueVertices not_eq 0) {
    B2WARNING(m_badTrackPValueVertices << " vertices had at least on track with a p value smaller than " << m_trackPValueCut << " and were not included in the statistical tests");
  }

  if (m_processedVertices <= 1 or m_processedVertices == m_vertexNotPureCounter) {
    B2WARNING("Only " << m_processedVertices - m_vertexNotPureCounter << " vertices were processed. Statistics cannot be computed.");
  } else {
    m_textOutput << "Number of processed Vertices: " << m_processedVertices << "\n";
    if (m_vertexNotPureCounter not_eq 0) {
      m_textOutput << "The vertex finder assigned wrong tracks to " << m_vertexNotPureCounter << " of the processed vertices\n";
    }

    // loop over all vertex wise data and print it to terminal and/or to text file
    std::map<std::string, StatisticsContainer >::iterator  iter =  m_vertexWiseDataSamples.begin();
    std::map<std::string, StatisticsContainer >::const_iterator iterMax = m_vertexWiseDataSamples.end();
    while (iter not_eq iterMax) {
      printVertexWiseStatistics(iter->first, true);
      ++iter;
    }
    //now the vertexWiseVecData
    vector<string> varNames(3);
    varNames[0] = "x";
    varNames[1] = "y";
    varNames[2] = "z";
    vector<string> varNamesWithChi2 = varNames;
    varNamesWithChi2.push_back("chi2");
    printVertexWiseVecStatistics("res_vertexPos", varNames, true);
    printVertexWiseVecStatistics("pullsAndChi2_vertexPos", varNamesWithChi2, true);
    printVertexWiseVecStatistics("res_MCVertexPos", varNames, true);
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



void VertexFitCheckerModule::printVertexWiseVecStatistics(const string& nameOfDataSample, const vector<string>& varNames, const bool count)
{
  vector<StatisticsContainer>& dataSample = m_vertexWiseVecDataSamples[nameOfDataSample];

  const int nOfVars = dataSample.size();
  m_textOutput << "Information on " << nameOfDataSample << "\n\tmean\tstd";
  if (m_robust == true) {
    m_textOutput << "\tmedian\tMAD std\toutlier\ttr mean\ttr std\tignored";
  }
  if (count == true) {
    m_textOutput << "\tcount";
  }
  m_textOutput << "\n";
  double madScalingFactor =  m_madScalingFactors[nameOfDataSample];
  double trunctationRatio = m_trunctationRatios[nameOfDataSample];
  for (int i = 0; i not_eq nOfVars; ++i) {
    m_textOutput << fixed << varNames[i] << "\t" << mean(dataSample[i]) << "\t" << sqrt(variance(dataSample[i]));
    if (m_robust == true) {

      vector<double>& data = m_vertexWiseVecData[nameOfDataSample][i];
      if (madScalingFactor > 1E-100) {
        double aMedian = median(dataSample[i]);
        double scaledMad = madScalingFactor * calcMad(data, aMedian);
        int nOutliers = countOutliers(data, aMedian, scaledMad, 4);
        m_textOutput << "\t" << aMedian << "\t" << scaledMad << "\t" << nOutliers; //<< "\t" << calcMedian(data);
      } else {
        m_textOutput << "\tno scaling for MAD";
      }
      if (trunctationRatio > 1E-100) {
        double mean = 0;
        double std = 0;

        int nCutAwayTracks = trunctatedMeanAndStd(data, trunctationRatio, true, mean, std);
        m_textOutput << "\t" << mean << "\t" << std << "\t" << nCutAwayTracks;

      } else {
        m_textOutput << "\tno cut away ratio given";
      }
    } else if (count == true) {
      m_textOutput << "\t\t\t\t\t\t";
    }
    if (count == true) {
      m_textOutput << "\t" << boost::accumulators::count(dataSample[i]);
    }
    m_textOutput << "\n";
  }

}

void VertexFitCheckerModule::registerVertexWiseVecData(const string& nameOfDataSample, const int nVarsToTest)
{
  m_vertexWiseVecDataSamples[nameOfDataSample].resize(nVarsToTest);
  if (m_writeToRootFile == true) {
    m_vertexWiseVecDataForRoot[nameOfDataSample] = new std::vector<float>(nVarsToTest);
    m_statDataTreePtr->Branch(nameOfDataSample.c_str(), "std::vector<float>", &(m_vertexWiseVecDataForRoot[nameOfDataSample]));
  }
  if (m_robust == true) {
    m_vertexWiseVecData[nameOfDataSample].resize(nVarsToTest);
  }
}

void VertexFitCheckerModule::fillVertexWiseVecData(const string& nameOfDataSample, const vector<double>& newData)
{
  const int nNewData = newData.size();
  for (int i = 0; i not_eq nNewData; ++i) {
    m_vertexWiseVecDataSamples[nameOfDataSample][i](newData[i]);
  }
  if (m_writeToRootFile == true) {
    for (int i = 0; i not_eq nNewData; ++i) {
      (*m_vertexWiseVecDataForRoot[nameOfDataSample])[i] = float(newData[i]);
    }
  }
  if (m_robust == true) {
    for (int i = 0; i not_eq nNewData; ++i) {
      m_vertexWiseVecData[nameOfDataSample][i].push_back(newData[i]);
    }
  }
}

void VertexFitCheckerModule::registerInt(const std::string& nameOfDataSample)
{
  m_intForRoot[nameOfDataSample] = int(-999);
  m_statDataTreePtr->Branch(nameOfDataSample.c_str(), &(m_intForRoot[nameOfDataSample]));
}

void VertexFitCheckerModule::fillInt(const std::string& nameOfDataSample, const int newData)
{
  if (m_writeToRootFile == true) {
    m_intForRoot[nameOfDataSample] = newData;
  }
}


int VertexFitCheckerModule::countOutliers(const vector<double>& dataSample, const double mean, const double sigma, const double widthScaling)
{

  int n = dataSample.size();
  int nOutliers = 0;
  double halfInterval = widthScaling * sigma;
  assert(not(halfInterval < 0.0)); //both widthScaling and sigma must be positive therefore halfInterval, too!
  double lowerCut = mean - halfInterval;
  double upperCut = mean + halfInterval;
  B2DEBUG(100, "n=" << n << ",mean=" << mean << ",sigma=" << sigma << ",lowerCut=" << lowerCut << ",upperCut=" << upperCut);
  for (int i = 0; i not_eq n; ++i) {
    if (dataSample[i] < lowerCut or dataSample[i] > upperCut) {
      ++nOutliers;
    }
  }
  return nOutliers;
}

double VertexFitCheckerModule::calcMad(const std::vector<double>& data, const double& median)
{
  const int n = data.size();
  vector<double> absRes(n);

  for (int i = 0; i not_eq n; ++i) {
    absRes[i] = abs(data[i] - median);
  }

  const int mid = n / 2;
  sort(absRes.begin(), absRes.end());
  double mad = n % 2 == 0 ? (absRes[mid] + absRes[mid - 1]) / 2.0 : absRes[mid];
  return mad;
}

//double VertexFitCheckerModule::calcMedian(std::vector<double> data)
//{
//  const int n = data.size();
//  const int mid = n / 2;
//  sort(data.begin(), data.end());
//  return  n % 2 == 0 ? (data[mid] + data[mid - 1]) / 2.0 : data[mid];
//}

// calculate a chi2 value from a residuum and it's covariance matrix R
double VertexFitCheckerModule::calcChi2(const TMatrixT<double>& res, const TMatrixT<double>& R) const
{
  TMatrixT<double> invR = TrackFitCheckerModule::invertMatrix(R);
  TMatrixT<double> resT(TMatrixT<double>::kTransposed, res);
  return (resT * invR * res)[0][0];
}

int VertexFitCheckerModule::trunctatedMeanAndStd(std::vector<double> data, const double cutRatio, const bool symmetric, double& mean, double& std)
{

  const int n = data.size();
  int truncN = -1;
  //  if ((n* cutRatio < 2 and symmetric == true) or(n* cutRatio < 1 and symmetric == false)) {
  //    return 0;
  //  }
  sort(data.begin(), data.end());
  std = 0;
  int i = 0;
  int iMax = 0;
  double sum = 0;

  if (symmetric == true) { // cut will be applied to both sides of the sorted sample
    int cut = int(cutRatio * n / 2.0 + 0.5);
    truncN = n - cut - cut;
    i = cut;
    iMax = n - cut;
    sum = std::accumulate(data.begin() + cut, data.end() - cut, 0.0);
  } else { // cut will only be applied to the side with the larger values of the sorted sample
    int cut = int(cutRatio * n + 0.5);
    truncN = n - cut;
    i = 0;
    iMax = n - cut;
    sum = std::accumulate(data.begin(), data.end() - cut, 0.0);
  }
  mean = sum / truncN;
  while (i not_eq iMax) {
    double diff = data[i] - mean;
    std += diff * diff;
    ++i;
  }
  std = sqrt(1.0 / (truncN - 1) * std);
  return n - truncN;
}
