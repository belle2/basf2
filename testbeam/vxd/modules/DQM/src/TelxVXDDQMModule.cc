#include "testbeam/vxd/modules/DQM/TelxVXDDQMModule.h"

#include <framework/core/HistoModule.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>

#include <svd/dataobjects/SVDDigit.h>
#include <svd/dataobjects/SVDCluster.h>
#include <testbeam/vxd/dataobjects/TelDigit.h>
#ifdef MAKE_TELCLUSTERS
#include <testbeam/vxd/dataobjects/TelCluster.h>
#endif
#include <pxd/dataobjects/PXDCluster.h>

#include <set>
#include <boost/format.hpp>

#include "TH1F.h"
#include "TH2F.h"
#include "TVector3.h"
#include "TDirectory.h"

using namespace std;
using boost::format;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(TelxVXD)


//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

TelxVXDModule::TelxVXDModule() : HistoModule()
{
  //Set module properties
  setDescription("Tel x VXD DQM module adds some correlation plots between telescopes and nearest VXD sensors.");
  setPropertyFlags(c_ParallelProcessingCertified);  // specify this flag if you need parallel processing
  addParam("Clusters", m_storeTelClustersName, "Name of the telescopes cluster collection",
           std::string("")); // always be explicit about this in case you use PXDClusters
  addParam("histgramDirectoryName", m_histogramDirectoryName, "Name of the directory where histograms will be placed",
           std::string("tel-vxd"));

  //----------------------------------------------------------------
  // define setting options for correlations
  //----------------------------------------------------------------

  // Externally possible to set swapping of axes for correlations and change unit from local [pitch unit] to space points [cm]
  m_SwapAxis = 0;  /**< [0] 0: u<->u, 1: u<->v correlations */
  addParam("SwapAxesUV", m_SwapAxis, "Swap of axes between u and v", m_SwapAxis);
  m_UseSP = 0;     /**< 1: use space points for hitmaps and correlations */
  addParam("UseSpacePoints", m_UseSP, "Use space points for correlations", m_UseSP);
  // Externally possible to set modules for correlations
  m_ModulesForCheck = {4, 5, 8, 2, 9};
  addParam("ModuleNrsForCheck", m_ModulesForCheck, "Numbering of modules for correlations", m_ModulesForCheck);
  m_DUTPXD = m_ModulesForCheck[0];    /**< [4] PXD for correlations: 3,4 */
  m_DUTSVDFwd = m_ModulesForCheck[1]; /**< [5] forward SVD for correlations: 5..8 */
  m_DUTSVDBwd = m_ModulesForCheck[2]; /**< [8] backward SVD for correlations: 5..8 */
  m_DUTTelFwd = m_ModulesForCheck[3]; /**< [2] forward telescope for correlations: 0..2 */
  m_DUTTelBwd = m_ModulesForCheck[4]; /**< [9] backward telescope for correlations: 9..11 */
  m_PXDCutSeedL = 0;
  m_PXDCutSeedH = 100000;
  addParam("PXDCutSeedL", m_PXDCutSeedL, "PXD: seed cut lower border", m_PXDCutSeedL);
  addParam("PXDCutSeedH", m_PXDCutSeedH, "PXD: seed cut higher border", m_PXDCutSeedH);

}


TelxVXDModule::~TelxVXDModule()
{
}

//------------------------------------------------------------------
// Function to define histograms
//-----------------------------------------------------------------

void TelxVXDModule::defineHisto()
{
  // Create a separate histogram directory and cd into it.
  TDirectory* oldDir = gDirectory;
  oldDir->mkdir(m_histogramDirectoryName.c_str())->cd();

  //----------------------------------------------------------------
  // Charge of clusters : hClusterCharge[U/V][PlaneNo]
  //----------------------------------------------------------------

  int iPlane = indexToPlane(m_DUTPXD);
  iPlane = iPlane - 3;
  string name = str(format("hPXDClusterChargeU%1%") % iPlane);
  string title = str(format("PXD cluster charge, plane %1%") % iPlane);
  m_chargePXD2 = new TH1F(name.c_str(), title.c_str(), 200, 0, 200);
  m_chargePXD2->GetXaxis()->SetTitle("charge of clusters [ADU]");
  m_chargePXD2->GetYaxis()->SetTitle("count");

  iPlane = indexToPlane(m_DUTSVDFwd);
  iPlane = iPlane - 3;
  name = str(format("hSVDClusterChargeU%1%") % iPlane);
  title = str(format("SVD cluster charge in U, plane %1%") % iPlane);
  m_chargeUSVD1 = new TH1F(name.c_str(), title.c_str(), 50, 0, 200);
  m_chargeUSVD1->GetXaxis()->SetTitle("charge of u clusters [ADU]");
  m_chargeUSVD1->GetYaxis()->SetTitle("count");

  iPlane = indexToPlane(m_DUTSVDFwd);
  iPlane = iPlane - 3;
  name = str(format("hSVDClusterChargeV%1%") % iPlane);
  title = str(format("SVD cluster charge in V, plane %1%") % iPlane);
  m_chargeVSVD1 = new TH1F(name.c_str(), title.c_str(), 50, 0, 200);
  m_chargeVSVD1->GetXaxis()->SetTitle("charge of v clusters [ADU]");
  m_chargeVSVD1->GetYaxis()->SetTitle("count");

  iPlane = indexToPlane(m_DUTTelFwd);
  name = str(format("hTelClusterCharge%1%") % iPlane);
  title = str(format("Tel cluster charge, plane %1%") % iPlane);
  m_chargeTel3 = new TH1F(name.c_str(), title.c_str(), 20, 0, 20);
  m_chargeTel3->GetXaxis()->SetTitle("charge of clusters [ADU]");
  m_chargeTel3->GetYaxis()->SetTitle("count");

  iPlane = indexToPlane(m_DUTTelBwd);
  iPlane = iPlane - 6;
  name = str(format("hTelClusterCharge%1%") % iPlane);
  title = str(format("Tel cluster charge, plane %1%") % iPlane);
  m_chargeTel4 = new TH1F(name.c_str(), title.c_str(), 20, 0, 20);
  m_chargeTel4->GetXaxis()->SetTitle("charge of clusters [ADU]");
  m_chargeTel4->GetYaxis()->SetTitle("count");

  //----------------------------------------------------------------
  // 2D Hitmaps for some TB detectors in local uv coordinates : m_hitMapxxx
  //----------------------------------------------------------------

  int j = m_DUTTelFwd;
  iPlane = indexToPlane(j);
  name = str(format("hTELHitmap%1%") % iPlane);
  title = str(format("TEL Hitmap, plane %1%") % iPlane);
  int nPixelsU = getInfoTel(j).getUCells();
  int nPixelsV = getInfoTel(j).getVCells();
  m_hitMapTel3 = new TH2F(name.c_str(), title.c_str(), nPixelsU, 0, nPixelsU, nPixelsV, 0, nPixelsV);
  m_hitMapTel3->GetXaxis()->SetTitle("u position [pitch units]");
  m_hitMapTel3->GetYaxis()->SetTitle("v position [pitch units]");
  m_hitMapTel3->GetZaxis()->SetTitle("hits");

  j = m_DUTTelBwd;
  iPlane = indexToPlane(j);
  iPlane = iPlane - 6;
  name = str(format("hTELHitmap%1%") % iPlane);
  title = str(format("TEL Hitmap, plane %1%") % iPlane);
  nPixelsU = getInfoTel(j).getUCells();
  nPixelsV = getInfoTel(j).getVCells();
  m_hitMapTel4 = new TH2F(name.c_str(), title.c_str(), nPixelsU, 0, nPixelsU, nPixelsV, 0, nPixelsV);
  m_hitMapTel4->GetXaxis()->SetTitle("u position [pitch units]");
  m_hitMapTel4->GetYaxis()->SetTitle("v position [pitch units]");
  m_hitMapTel4->GetZaxis()->SetTitle("hits");

  j = m_DUTPXD;
  iPlane = indexToPlane(j);
  iPlane = iPlane - 3;
  name = str(format("hPXDHitmap%1%") % iPlane);
  title = str(format("PXD Hitmap, plane %1%") % iPlane);
  nPixelsU = getInfoPXD(j).getUCells();
  nPixelsV = getInfoPXD(j).getVCells();
  m_hitMapPXD2 = new TH2F(name.c_str(), title.c_str(), nPixelsU, 0, nPixelsU, nPixelsV, 0, nPixelsV);
  m_hitMapPXD2->GetXaxis()->SetTitle("u position [pitch units]");
  m_hitMapPXD2->GetYaxis()->SetTitle("v position [pitch units]");
  m_hitMapPXD2->GetZaxis()->SetTitle("hits");

  j = m_DUTSVDFwd;
  iPlane = indexToPlane(j);
  iPlane = iPlane - 3;
  name = str(format("hSVDHitmap%1%") % iPlane);
  title = str(format("SVD Hitmap, plane %1%") % iPlane);
  nPixelsU = getInfoSVD(j).getUCells();
  nPixelsV = getInfoSVD(j).getVCells();
  m_hitMapSVD1 = new TH2F(name.c_str(), title.c_str(), nPixelsU, 0, nPixelsU, nPixelsV, 0, nPixelsV);
  m_hitMapSVD1->GetXaxis()->SetTitle("u position [pitch units]");
  m_hitMapSVD1->GetYaxis()->SetTitle("v position [pitch units]");
  m_hitMapSVD1->GetZaxis()->SetTitle("hits");

  j = m_DUTSVDBwd;
  iPlane = indexToPlane(j);
  iPlane = iPlane - 3;
  name = str(format("hSVDHitmap%1%") % iPlane);
  title = str(format("SVD Hitmap, plane %1%") % iPlane);
  nPixelsU = getInfoSVD(j).getUCells();
  nPixelsV = getInfoSVD(j).getVCells();
  m_hitMapSVD4 = new TH2F(name.c_str(), title.c_str(), nPixelsU, 0, nPixelsU, nPixelsV, 0, nPixelsV);
  m_hitMapSVD4->GetXaxis()->SetTitle("u position [pitch units]");
  m_hitMapSVD4->GetYaxis()->SetTitle("v position [pitch units]");
  m_hitMapSVD4->GetZaxis()->SetTitle("hits");

  //----------------------------------------------------------------
  // Correlations for some combination of TB detectors in local uv coordinates : m_correlationsxxxxxx[U/V]
  //----------------------------------------------------------------

  for (int i = 0; i < 2; i++) {
    string uAxis;
    string vAxis;
    string Unit;
    if (m_UseSP) {
      Unit = str(format("cm"));
      if (i) {
        uAxis = str(format("z"));
        vAxis = str(format("z"));
      } else {
        uAxis = str(format("y"));
        vAxis = str(format("y"));
      }
      if (m_SwapAxis) {
        if (i) {
          vAxis = str(format("y"));
        } else {
          vAxis = str(format("z"));
        }
      }
    } else {
      Unit = str(format("pitch units"));
      if (i) {
        uAxis = str(format("v"));
        vAxis = str(format("v"));
      } else {
        uAxis = str(format("u"));
        vAxis = str(format("u"));
      }
      if (m_SwapAxis) {
        if (i) {
          vAxis = str(format("u"));
        } else {
          vAxis = str(format("v"));
        }
      }
    }

    //int nPixelsU; int nPixelsV; already defined in scope
    //int nPixelsU;
    //int nPixelsV;
    float nPixelsExtenzion = 20;
    int nPixelsUExtend;
    int nPixelsVExtend;
    float PixelsUStart;
    float PixelsUEnd;
    float PixelsVStart;
    float PixelsVEnd;

    //string name; string title; already defined in scope
    name = str(format("hCorrelationsTel%1%SVD%2%%3%%4%") % (m_DUTTelFwd + 1) % (m_DUTSVDFwd - 3 + 1) % uAxis.data() %
               vAxis.data());
    title = str(format("Corelation Tel%1% SVD%2%, axis %3% %4%") % (m_DUTTelFwd + 1) % (m_DUTSVDFwd - 3 + 1) % uAxis.data() %
                vAxis.data());
    string AxisU = str(format("%1% position Tel%2% [%3%]") % uAxis.data() % (m_DUTTelFwd + 1) % Unit.data());
    string AxisV = str(format("%1% position SVD%2% [%3%]") % vAxis.data() % (m_DUTSVDFwd - 3 + 1) % Unit.data());
    if (i) {
      nPixelsU = getInfoTel(m_DUTTelFwd).getVCells();
      PixelsUEnd = nPixelsU;
      if (m_UseSP)
        PixelsUEnd = getInfoTel(m_DUTTelFwd).getVSize();
      nPixelsV = getInfoSVD(m_DUTSVDFwd).getVCells();
      PixelsVEnd = nPixelsV;
      if (m_UseSP)
        PixelsVEnd = getInfoSVD(m_DUTSVDFwd).getVSize();
      if (m_SwapAxis) {
        nPixelsV = getInfoSVD(m_DUTSVDFwd).getUCells();
        PixelsVEnd = nPixelsV;
        if (m_UseSP)
          PixelsVEnd = getInfoSVD(m_DUTSVDFwd).getUSize();
      }
    } else {
      nPixelsU = getInfoTel(m_DUTTelFwd).getUCells();
      PixelsUEnd = nPixelsU;
      if (m_UseSP)
        PixelsUEnd = getInfoTel(m_DUTTelFwd).getUSize();
      nPixelsV = getInfoSVD(m_DUTSVDFwd).getUCells();
      PixelsVEnd = nPixelsV;
      if (m_UseSP)
        PixelsVEnd = getInfoSVD(m_DUTSVDFwd).getUSize();
      if (m_SwapAxis) {
        nPixelsV = getInfoSVD(m_DUTSVDFwd).getVCells();
        PixelsVEnd = nPixelsV;
        if (m_UseSP)
          PixelsVEnd = getInfoSVD(m_DUTSVDFwd).getVSize();
      }
    }
    nPixelsUExtend = nPixelsU + 2 * nPixelsExtenzion;
    if (m_UseSP) {
      PixelsUStart = PixelsUEnd * (-0.5 - nPixelsExtenzion / nPixelsU);
      PixelsUEnd = - PixelsUStart;
    } else {
      PixelsUStart = - nPixelsExtenzion * PixelsUEnd / nPixelsU;
      PixelsUEnd = PixelsUEnd - PixelsUStart;
    }
    nPixelsVExtend = nPixelsV + 2 * nPixelsExtenzion;
    if (m_UseSP) {
      PixelsVStart = PixelsVEnd * (-0.5 - nPixelsExtenzion / nPixelsV);
      PixelsVEnd = - PixelsVStart;
    } else {
      PixelsVStart = - nPixelsExtenzion * PixelsVEnd / nPixelsV;
      PixelsVEnd = PixelsVEnd - PixelsVStart;
    }

    m_correlationsTel3SVD1[i] = new TH2F(name.c_str(), title.c_str(), nPixelsUExtend, PixelsUStart, PixelsUEnd, nPixelsVExtend,
                                         PixelsVStart, PixelsVEnd);
    m_correlationsTel3SVD1[i]->GetXaxis()->SetTitle(AxisU.data());
    m_correlationsTel3SVD1[i]->GetYaxis()->SetTitle(AxisV.data());
    m_correlationsTel3SVD1[i]->GetZaxis()->SetTitle("hits");

    name = str(format("hCorrelationsTel%1%SVD%2%%3%%4%") % (m_DUTTelBwd - 6 + 1) % (m_DUTSVDBwd - 3 + 1) % uAxis.data() % vAxis.data());
    title = str(format("Corelation Tel%1% SVD%2%, axis %3% %4%") % (m_DUTTelBwd - 6 + 1) % (m_DUTSVDBwd - 3 + 1) % uAxis.data() %
                vAxis.data());
    AxisU = str(format("%1% position Tel%2% [%3%]") % uAxis.data() % (m_DUTTelBwd - 6 + 1) % Unit.data());
    AxisV = str(format("%1% position SVD%2% [%3%]") % vAxis.data() % (m_DUTSVDBwd - 3 + 1) % Unit.data());
    if (i) {
      nPixelsU = getInfoTel(m_DUTTelBwd).getVCells();
      PixelsUEnd = nPixelsU;
      if (m_UseSP)
        PixelsUEnd = getInfoTel(m_DUTTelBwd).getVSize();
      nPixelsV = getInfoSVD(m_DUTSVDBwd).getVCells();
      PixelsVEnd = nPixelsV;
      if (m_UseSP)
        PixelsVEnd = getInfoSVD(m_DUTSVDBwd).getVSize();
      if (m_SwapAxis) {
        nPixelsV = getInfoSVD(m_DUTSVDBwd).getUCells();
        PixelsVEnd = nPixelsV;
        if (m_UseSP)
          PixelsVEnd = getInfoSVD(m_DUTSVDBwd).getUSize();
      }
    } else {
      nPixelsU = getInfoTel(m_DUTTelBwd).getUCells();
      PixelsUEnd = nPixelsU;
      if (m_UseSP)
        PixelsUEnd = getInfoTel(m_DUTTelBwd).getUSize();
      nPixelsV = getInfoSVD(m_DUTSVDBwd).getUCells();
      PixelsVEnd = nPixelsV;
      if (m_UseSP)
        PixelsVEnd = getInfoSVD(m_DUTSVDBwd).getUSize();
      if (m_SwapAxis) {
        nPixelsV = getInfoSVD(m_DUTSVDBwd).getVCells();
        PixelsVEnd = nPixelsV;
        if (m_UseSP)
          PixelsVEnd = getInfoSVD(m_DUTSVDBwd).getVSize();
      }
    }
    nPixelsUExtend = nPixelsU + 2 * nPixelsExtenzion;
    if (m_UseSP) {
      PixelsUStart = PixelsUEnd * (-0.5 - nPixelsExtenzion / nPixelsU);
      PixelsUEnd = - PixelsUStart;
    } else {
      PixelsUStart = - nPixelsExtenzion * PixelsUEnd / nPixelsU;
      PixelsUEnd = PixelsUEnd - PixelsUStart;
    }
    nPixelsVExtend = nPixelsV + 2 * nPixelsExtenzion;
    if (m_UseSP) {
      PixelsVStart = PixelsVEnd * (-0.5 - nPixelsExtenzion / nPixelsV);
      PixelsVEnd = - PixelsVStart;
    } else {
      PixelsVStart = - nPixelsExtenzion * PixelsVEnd / nPixelsV;
      PixelsVEnd = PixelsVEnd - PixelsVStart;
    }

    m_correlationsTel4SVD4[i] = new TH2F(name.c_str(), title.c_str(), nPixelsUExtend, PixelsUStart, PixelsUEnd, nPixelsVExtend,
                                         PixelsVStart, PixelsVEnd);
    m_correlationsTel4SVD4[i]->GetXaxis()->SetTitle(AxisU.data());
    m_correlationsTel4SVD4[i]->GetYaxis()->SetTitle(AxisV.data());
    m_correlationsTel4SVD4[i]->GetZaxis()->SetTitle("hits");

    name = str(format("hCorrelationsTel%1%PXD%2%%3%%4%") % (m_DUTTelFwd + 1) % (m_DUTPXD - 3 + 1) % uAxis.data() % vAxis.data());
    title = str(format("Corelation Tel%1% PXD%2%, axis %3% %4%") % (m_DUTTelFwd + 1) % (m_DUTPXD - 3 + 1) % uAxis.data() %
                vAxis.data());
    AxisU = str(format("%1% position Tel%2% [%3%]") % uAxis.data() % (m_DUTTelFwd + 1) % Unit.data());
    AxisV = str(format("%1% position PXD%2% [%3%]") % vAxis.data() % (m_DUTPXD - 3 + 1) % Unit.data());
    if (i) {
      nPixelsU = getInfoTel(m_DUTTelFwd).getVCells();
      PixelsUEnd = nPixelsU;
      if (m_UseSP)
        PixelsUEnd = getInfoTel(m_DUTTelFwd).getVSize();
      nPixelsV = getInfoPXD(m_DUTPXD).getVCells();
      PixelsVEnd = nPixelsV;
      if (m_UseSP)
        PixelsVEnd = getInfoPXD(m_DUTPXD).getVSize();
      if (m_SwapAxis) {
        nPixelsV = getInfoPXD(m_DUTPXD).getUCells();
        PixelsVEnd = nPixelsV;
        if (m_UseSP)
          PixelsVEnd = getInfoPXD(m_DUTPXD).getUSize();
      }
    } else {
      nPixelsU = getInfoTel(m_DUTTelFwd).getUCells();
      PixelsUEnd = nPixelsU;
      if (m_UseSP)
        PixelsUEnd = getInfoTel(m_DUTTelFwd).getUSize();
      nPixelsV = getInfoPXD(m_DUTPXD).getUCells();
      PixelsVEnd = nPixelsV;
      if (m_UseSP)
        PixelsVEnd = getInfoPXD(m_DUTPXD).getUSize();
      if (m_SwapAxis) {
        nPixelsV = getInfoPXD(m_DUTPXD).getVCells();
        PixelsVEnd = nPixelsV;
        if (m_UseSP)
          PixelsVEnd = getInfoPXD(m_DUTPXD).getVSize();
      }
    }
    nPixelsUExtend = nPixelsU + 2 * nPixelsExtenzion;
    if (m_UseSP) {
      PixelsUStart = PixelsUEnd * (-0.5 - nPixelsExtenzion / nPixelsU);
      PixelsUEnd = - PixelsUStart;
    } else {
      PixelsUStart = - nPixelsExtenzion * PixelsUEnd / nPixelsU;
      PixelsUEnd = PixelsUEnd - PixelsUStart;
    }
    nPixelsVExtend = nPixelsV + 2 * nPixelsExtenzion;
    if (m_UseSP) {
      PixelsVStart = PixelsVEnd * (-0.5 - nPixelsExtenzion / nPixelsV);
      PixelsVEnd = - PixelsVStart;
    } else {
      PixelsVStart = - nPixelsExtenzion * PixelsVEnd / nPixelsV;
      PixelsVEnd = PixelsVEnd - PixelsVStart;
    }

    m_correlationsTel3PXD2[i] = new TH2F(name.c_str(), title.c_str(), nPixelsUExtend, PixelsUStart, PixelsUEnd, nPixelsVExtend,
                                         PixelsVStart, PixelsVEnd);
    m_correlationsTel3PXD2[i]->GetXaxis()->SetTitle(AxisU.data());
    m_correlationsTel3PXD2[i]->GetYaxis()->SetTitle(AxisV.data());
    m_correlationsTel3PXD2[i]->GetZaxis()->SetTitle("hits");

    name = str(format("hCorrelationsSVD%1%PXD%2%%3%%4%") % (m_DUTSVDFwd - 3 + 1) % (m_DUTPXD - 3 + 1) % uAxis.data() % vAxis.data());
    title = str(format("Corelation SVD%1% PXD%2%, axis %3% %4%") % (m_DUTSVDFwd - 3 + 1) % (m_DUTPXD - 3 + 1) % uAxis.data() %
                vAxis.data());
    AxisU = str(format("%1% position SVD%2% [%3%]") % uAxis.data() % (m_DUTSVDFwd - 3 + 1) % Unit.data());
    AxisV = str(format("%1% position PXD%2% [%3%]") % vAxis.data() % (m_DUTPXD - 3 + 1) % Unit.data());
    if (i) {
      nPixelsU = getInfoSVD(m_DUTSVDFwd).getVCells();
      PixelsUEnd = nPixelsU;
      if (m_UseSP)
        PixelsUEnd = getInfoSVD(m_DUTSVDFwd).getVSize();
      nPixelsV = getInfoPXD(m_DUTPXD).getVCells();
      PixelsVEnd = nPixelsV;
      if (m_UseSP)
        PixelsVEnd = getInfoPXD(m_DUTPXD).getVSize();
      if (m_SwapAxis) {
        nPixelsV = getInfoPXD(m_DUTPXD).getUCells();
        PixelsVEnd = nPixelsV;
        if (m_UseSP)
          PixelsVEnd = getInfoPXD(m_DUTPXD).getUSize();
      }
    } else {
      nPixelsU = getInfoSVD(m_DUTSVDFwd).getUCells();
      PixelsUEnd = nPixelsU;
      if (m_UseSP)
        PixelsUEnd = getInfoSVD(m_DUTSVDFwd).getUSize();
      nPixelsV = getInfoPXD(m_DUTPXD).getUCells();
      PixelsVEnd = nPixelsV;
      if (m_UseSP)
        PixelsVEnd = getInfoPXD(m_DUTPXD).getUSize();
      if (m_SwapAxis) {
        nPixelsV = getInfoPXD(m_DUTPXD).getVCells();
        PixelsVEnd = nPixelsV;
        if (m_UseSP)
          PixelsVEnd = getInfoPXD(m_DUTPXD).getVSize();
      }
    }
    nPixelsUExtend = nPixelsU + 2 * nPixelsExtenzion;
    if (m_UseSP) {
      PixelsUStart = PixelsUEnd * (-0.5 - nPixelsExtenzion / nPixelsU);
      PixelsUEnd = - PixelsUStart;
    } else {
      PixelsUStart = - nPixelsExtenzion * PixelsUEnd / nPixelsU;
      PixelsUEnd = PixelsUEnd - PixelsUStart;
    }
    nPixelsVExtend = nPixelsV + 2 * nPixelsExtenzion;
    if (m_UseSP) {
      PixelsVStart = PixelsVEnd * (-0.5 - nPixelsExtenzion / nPixelsV);
      PixelsVEnd = - PixelsVStart;
    } else {
      PixelsVStart = - PixelsVEnd * nPixelsExtenzion / nPixelsV;
      PixelsVEnd = PixelsVEnd - PixelsVStart;
    }

    m_correlationsSVD1PXD2[i] = new TH2F(name.c_str(), title.c_str(), nPixelsUExtend, PixelsUStart, PixelsUEnd, nPixelsVExtend,
                                         PixelsVStart, PixelsVEnd);
    m_correlationsSVD1PXD2[i]->GetXaxis()->SetTitle(AxisU.data());
    m_correlationsSVD1PXD2[i]->GetYaxis()->SetTitle(AxisV.data());
    m_correlationsSVD1PXD2[i]->GetZaxis()->SetTitle("hits");
  }

  oldDir->cd();
}


void TelxVXDModule::initialize()
{
  // Register histograms (calls back defineHisto)
  REG_HISTOGRAM

  //Register collections
  StoreArray<PXDCluster> storePXDClusters(m_storePXDClustersName);
#ifdef MAKE_TELCLUSTERS
  StoreArray<TelCluster> storeTelClusters(m_storeTelClustersName);
#else
  StoreArray<PXDCluster> storeTelClusters(m_storeTelClustersName);
#endif
  StoreArray<SVDCluster> storeSVDClusters(m_storeSVDClustersName);
  StoreArray<SVDDigit> storeSVDDigits(m_storeSVDDigitsName);
  StoreArray<TelDigit> storeTelDigits(m_storeTelDigitsName);

  //storeClusters.isRequired();
  //storeDigits.isRequired();

  RelationArray relSVDClusterDigits(storeSVDClusters, storeSVDDigits);
  RelationArray relTelClusterDigits(storeTelClusters, storeTelDigits);

  //Store names to speed up creation later
  m_storeSVDClustersName = storeSVDClusters.getName();
  m_storeTelClustersName = storeTelClusters.getName();
  m_storeSVDDigitsName = storeSVDDigits.getName();
  m_storeTelDigitsName = storeTelDigits.getName();
  m_relSVDClusterDigitName = relSVDClusterDigits.getName();
  m_relTelClusterDigitName = relTelClusterDigits.getName();
}

void TelxVXDModule::beginRun()
{
  //auto geo = VXD::GeoCache::getInstance();
  //VXD::GeoCache& geo = VXD::GeoCache::getInstance();
  m_chargePXD2->Reset();
  m_chargeUSVD1->Reset();
  m_chargeVSVD1->Reset();
  m_chargeTel3->Reset();
  m_chargeTel4->Reset();

  m_hitMapTel3->Reset();
  m_hitMapTel4->Reset();
  m_hitMapPXD2->Reset();
  m_hitMapSVD1->Reset();
  m_hitMapSVD4->Reset();

  // Just to make sure, reset all the histograms.
  for (int i = 0; i < 2; i++) {
    m_correlationsTel3SVD1[i]->Reset();
    m_correlationsTel4SVD4[i]->Reset();
    m_correlationsTel3PXD2[i]->Reset();
    m_correlationsSVD1PXD2[i]->Reset();
  }

}


void TelxVXDModule::event()
{

  const StoreArray<SVDDigit> storeSVDDigits(m_storeSVDDigitsName);
  const StoreArray<TelDigit> storeTelDigits(m_storeTelDigitsName);
  const StoreArray<SVDCluster> storeSVDClusters(m_storeSVDClustersName);
  const StoreArray<PXDCluster> storePXDClusters(m_storePXDClustersName);
#ifdef MAKE_TELCLUSTERS
  const StoreArray<TelCluster> storeTelClusters(m_storeTelClustersName);
#else
  const StoreArray<PXDCluster> storeTelClusters(m_storeTelClustersName);
#endif

  const RelationArray relSVDClusterDigits(storeSVDClusters, storeSVDDigits, m_relSVDClusterDigitName);
  const RelationArray relTelClusterDigits(storeTelClusters, storeTelDigits, m_relTelClusterDigitName);

  // m_chargeUSVD1, m_chargeVSVD1
  for (const SVDCluster& cluster : storeSVDClusters) {
    int iPlane = cluster.getSensorID().getLayerNumber();
    if ((iPlane + 3 < c_firstSVDPlane) || (iPlane + 3 > c_lastSVDPlane)) continue;
    int index = planeToIndex(iPlane + 3);
    if (index != m_DUTSVDFwd) continue;
    if (cluster.isUCluster()) {
      m_chargeUSVD1->Fill(cluster.getCharge());
    } else {
      m_chargeVSVD1->Fill(cluster.getCharge());
    }
  }

//  m_chargePXD2, m_hitMapPXD2
  for (const PXDCluster& cluster : storePXDClusters) {
    int iPlane = cluster.getSensorID().getLayerNumber();
    if ((iPlane + 3 < c_firstPXDPlane) || (iPlane + 3 > c_lastPXDPlane)) continue;
    int index = planeToIndex(iPlane + 3);
    if (index != m_DUTPXD) continue;
    if ((cluster.getSeedCharge() >= m_PXDCutSeedL) && (cluster.getSeedCharge() <= m_PXDCutSeedH)) {
      m_chargePXD2->Fill(cluster.getCharge());
      m_hitMapPXD2->Fill(getInfoPXD(index).getUCellID(cluster.getU()), getInfoPXD(index).getVCellID(cluster.getV()));
    }
  }
//  m_chargeTel3, m_chargeTel4, m_hitMapTel3, m_hitMapTel4
  for (auto cluster : storeTelClusters) {
    int iPlane = cluster.getSensorID().getLayerNumber();
    int iPlane2 = cluster.getSensorID().getSensorNumber();
    if (iPlane != 7) continue;
    if ((iPlane2 < 1) || (iPlane2 > 6)) continue;
    if (iPlane2 == m_DUTTelFwd + 1) {
      int index = m_DUTTelFwd;
      if ((getInfoTel(index).getUCellID(cluster.getU()) < -1) || (getInfoTel(index).getVCellID(cluster.getV()) < -1)) continue;
      m_chargeTel3->Fill(cluster.getCharge());
      m_hitMapTel3->Fill(getInfoTel(index).getUCellID(cluster.getU()), getInfoTel(index).getVCellID(cluster.getV()));
    }
    if (iPlane2 == m_DUTTelBwd - 6 + 1) {
      int index = m_DUTTelBwd;
      if ((getInfoTel(index).getUCellID(cluster.getU()) < -1) || (getInfoTel(index).getVCellID(cluster.getV()) < -1)) continue;
      m_chargeTel4->Fill(cluster.getCharge());
      m_hitMapTel4->Fill(getInfoTel(index).getUCellID(cluster.getU()), getInfoTel(index).getVCellID(cluster.getV()));
    }
  }

// m_hitMapSVD1, m_hitMapSVD4
  for (int i1 = 0; i1 < storeSVDClusters.getEntries(); i1++) {
    // preparing of first value for correlation plots with postfix "1":
    float fPosU1 = 0.0;
    int index1 = 0;
    int iIsU1 = 0;
    int iPlane1 = 0;
    const SVDCluster& cluster1 = *storeSVDClusters[i1];
    iPlane1 = cluster1.getSensorID().getLayerNumber();
    if ((iPlane1 + 3 < c_firstSVDPlane) || (iPlane1 + 3 > c_lastSVDPlane)) continue;
    index1 = planeToIndex(iPlane1 + 3);
    if (cluster1.isUCluster()) {
      iIsU1 = 1;
      fPosU1 = getInfoSVD(index1).getUCellID(cluster1.getPosition());
    }
    for (int i2 = 0; i2 < storeSVDClusters.getEntries(); i2++) {
      // preparing of second value for correlation plots with postfix "2":
      float fPosV2 = 0.0;
      int index2 = 0;
      int iIsV2 = 0;
      int iPlane2 = 0;
      const SVDCluster& cluster2 = *storeSVDClusters[i2];
      iPlane2 = cluster2.getSensorID().getLayerNumber();
      if ((iPlane2 + 3 < c_firstSVDPlane) || (iPlane2 + 3 > c_lastSVDPlane)) continue;
      index2 = planeToIndex(iPlane2 + 3);
      if (!cluster2.isUCluster()) {
        iIsV2 = 1;
        fPosV2 = getInfoSVD(index2).getVCellID(cluster2.getPosition());
      }
      // ready to fill correlation histograms and hit maps:
      if ((index1 == index2) && (iIsU1 == 1) && (iIsV2 == 1)) {
        // hit maps:
        if (index1 == m_DUTSVDFwd)
          m_hitMapSVD1->Fill(fPosU1, fPosV2);
        if (index1 == m_DUTSVDBwd)
          m_hitMapSVD4->Fill(fPosU1, fPosV2);
      }
    }
  }

  // Correlations for local u v or global y z or swap coordinates

// m_correlationsSVD1PXD2[u/v]
  int SelDet1 = m_DUTSVDFwd;  // SVD1
  int SelDet2 = m_DUTPXD;  // PXD2
  for (int i1 = 0; i1 < storeSVDClusters.getEntries() + storePXDClusters.getEntries(); i1++) {
    // preparing of first value for correlation plots with postfix "1":
    float fPosU1 = 0.0;
    float fPosV1 = 0.0;
    int index1 = 0;
    int iIsU1 = 0;
    int iIsV1 = 0;
    int iPlane1 = 0;
    if (i1 < storePXDClusters.getEntries()) {  // PXD clusters:
      auto clusterPXD1 = *storePXDClusters[i1];
      iPlane1 = clusterPXD1.getSensorID().getLayerNumber();
      if ((iPlane1 + 3 < c_firstPXDPlane) || (iPlane1 + 3 > c_lastPXDPlane)) continue;
      if ((clusterPXD1.getSeedCharge() < m_PXDCutSeedL) || (clusterPXD1.getSeedCharge() > m_PXDCutSeedH)) continue;
      index1 = planeToIndex(iPlane1 + 3);
      iIsU1 = 1;
      iIsV1 = 1;
      fPosU1 = getInfoPXD(index1).getUCellID(clusterPXD1.getU());
      fPosV1 = getInfoPXD(index1).getVCellID(clusterPXD1.getV());
      if (m_UseSP) {
        TVector3 rLocal(clusterPXD1.getU(), clusterPXD1.getV(), 0);
        TVector3 rGlobal = getInfoPXD(index1).pointToGlobal(rLocal);
        fPosU1 = rGlobal.Y();
        fPosV1 = rGlobal.Z();
      }
      if (m_SwapAxis) {
        swap(fPosU1, fPosV1);
        swap(iIsU1, iIsV1);
      }
    } else {                                  // SVD clusters:
      const SVDCluster& cluster1 = *storeSVDClusters[i1 - storePXDClusters.getEntries()];
      iPlane1 = cluster1.getSensorID().getLayerNumber();
      if ((iPlane1 + 3 < c_firstSVDPlane) || (iPlane1 + 3 > c_lastSVDPlane)) continue;
      index1 = planeToIndex(iPlane1 + 3);
      if (cluster1.isUCluster()) {
        iIsU1 = 1;
        fPosU1 = getInfoSVD(index1).getUCellID(cluster1.getPosition());
        if (m_UseSP) {
          TVector3 rLocal(cluster1.getPosition(), 0, 0);
          TVector3 rGlobal = getInfoSVD(index1).pointToGlobal(rLocal);
          fPosU1 = rGlobal.Y();
        }
      } else {
        iIsV1 = 1;
        fPosV1 = getInfoSVD(index1).getVCellID(cluster1.getPosition());
        if (m_UseSP) {
          TVector3 rLocal(0, cluster1.getPosition(), 0);
          TVector3 rGlobal = getInfoSVD(index1).pointToGlobal(rLocal);
          fPosV1 = rGlobal.Z();
        }
      }
    }
    if (!((index1 == SelDet1) || (index1 == SelDet2))) continue;
    for (int i2 = 0; i2 < storeSVDClusters.getEntries() + storePXDClusters.getEntries(); i2++) {
      // preparing of second value for correlation plots with postfix "2":
      float fPosU2 = 0.0;
      float fPosV2 = 0.0;
      int index2 = 0;
      int iIsU2 = 0;
      int iIsV2 = 0;
      int iPlane2 = 0;
      if (i2 < storePXDClusters.getEntries()) {  // PXD clusters:
        auto clusterPXD2 = *storePXDClusters[i2];
        iPlane2 = clusterPXD2.getSensorID().getLayerNumber();
        if ((iPlane2 + 3 < c_firstPXDPlane) || (iPlane2 + 3 > c_lastPXDPlane)) continue;
        if ((clusterPXD2.getSeedCharge() < m_PXDCutSeedL) || (clusterPXD2.getSeedCharge() > m_PXDCutSeedH)) continue;
        index2 = planeToIndex(iPlane2 + 3);
        iIsU2 = 1;
        iIsV2 = 1;
        fPosU2 = getInfoPXD(index2).getUCellID(clusterPXD2.getU());
        fPosV2 = getInfoPXD(index2).getVCellID(clusterPXD2.getV());
        if (m_UseSP) {
          TVector3 rLocal(clusterPXD2.getU(), clusterPXD2.getV(), 0);
          TVector3 rGlobal = getInfoPXD(index2).pointToGlobal(rLocal);
          fPosU2 = rGlobal.Y();
          fPosV2 = rGlobal.Z();
        }
        if (m_SwapAxis) {
          swap(fPosU2, fPosV2);
          swap(iIsU2, iIsV2);
        }
      } else {                                  // SVD clusters:
        const SVDCluster& cluster2 = *storeSVDClusters[i2 - storePXDClusters.getEntries()];
        iPlane2 = cluster2.getSensorID().getLayerNumber();
        if ((iPlane2 + 3 < c_firstSVDPlane) || (iPlane2 + 3 > c_lastSVDPlane)) continue;
        index2 = planeToIndex(iPlane2 + 3);
        if (cluster2.isUCluster()) {
          iIsU2 = 1;
          fPosU2 = getInfoSVD(index2).getUCellID(cluster2.getPosition());
          if (m_UseSP) {
            TVector3 rLocal(cluster2.getPosition(), 0, 0);
            TVector3 rGlobal = getInfoSVD(index2).pointToGlobal(rLocal);
            fPosU2 = rGlobal.Y();
          }
        } else {
          iIsV2 = 1;
          fPosV2 = getInfoSVD(index2).getVCellID(cluster2.getPosition());
          if (m_UseSP) {
            TVector3 rLocal(0, cluster2.getPosition(), 0);
            TVector3 rGlobal = getInfoSVD(index2).pointToGlobal(rLocal);
            fPosV2 = rGlobal.Z();
          }
        }
      }
      if (!((index2 == SelDet1) || (index2 == SelDet2))) continue;
      // ready to fill correlation histograms and hit maps:
      if ((index1 < index2) && (iIsU1 == iIsU2) && (iIsU1 == 1)) {
        // correlations for u
        m_correlationsSVD1PXD2[0]->Fill(fPosU2, fPosU1);
      } else if ((index1 > index2) && (iIsV1 == iIsV2) && (iIsV1 == 1)) {
        // correlations for v
        m_correlationsSVD1PXD2[1]->Fill(fPosV1, fPosV2);
      }
    }
  }

// m_correlationsTel3PXD2[u/v]
  SelDet1 = m_DUTTelFwd;  // Tel3
  SelDet2 = m_DUTPXD;  // PXD2
  for (int i1 = 0; i1 < storeTelClusters.getEntries() + storePXDClusters.getEntries(); i1++) {
    // preparing of first value for correlation plots with postfix "1":
    float fPosU1 = 0.0;
    float fPosV1 = 0.0;
    int index1 = 0;
    int iIsU1 = 0;
    int iIsV1 = 0;
    int iPlane1 = 0;
    if (i1 < storePXDClusters.getEntries()) {  // PXD clusters:
      auto clusterPXD1 = *storePXDClusters[i1];
      iPlane1 = clusterPXD1.getSensorID().getLayerNumber();
      if ((iPlane1 + 3 < c_firstPXDPlane) || (iPlane1 + 3 > c_lastPXDPlane)) continue;
      if ((clusterPXD1.getSeedCharge() < m_PXDCutSeedL) || (clusterPXD1.getSeedCharge() > m_PXDCutSeedH)) continue;
      index1 = planeToIndex(iPlane1 + 3);
      iIsU1 = 1;
      iIsV1 = 1;
      fPosU1 = getInfoPXD(index1).getUCellID(clusterPXD1.getU());
      fPosV1 = getInfoPXD(index1).getVCellID(clusterPXD1.getV());
      if (m_UseSP) {
        TVector3 rLocal(clusterPXD1.getU(), clusterPXD1.getV(), 0);
        TVector3 rGlobal = getInfoPXD(index1).pointToGlobal(rLocal);
        fPosU1 = rGlobal.Y();
        fPosV1 = rGlobal.Z();
      }
      if (m_SwapAxis) {
        swap(fPosU1, fPosV1);
        swap(iIsU1, iIsV1);
      }
    } else {                                  // Tel clusters:
      auto clusterPXD1 = *storeTelClusters[i1 - storePXDClusters.getEntries()];
      iPlane1 = clusterPXD1.getSensorID().getLayerNumber();
      int iPlane1s = clusterPXD1.getSensorID().getSensorNumber();
      if (iPlane1 != 7) continue;
      if ((iPlane1s < 1) || (iPlane1s > 6)) continue;
      if (iPlane1s <= 3)
        index1 = iPlane1s - 1;
      else
        index1 = iPlane1s - 1 + 6;
      if ((getInfoTel(index1).getUCellID(clusterPXD1.getU()) < -1) || (getInfoTel(index1).getVCellID(clusterPXD1.getV()) < -1)) continue;
      iIsU1 = 1;
      iIsV1 = 1;
      fPosU1 = getInfoTel(index1).getUCellID(clusterPXD1.getU());
      fPosV1 = getInfoTel(index1).getVCellID(clusterPXD1.getV());
      if (m_UseSP) {
        TVector3 rLocal(clusterPXD1.getU(), clusterPXD1.getV(), 0);
        TVector3 rGlobal = getInfoTel(index1).pointToGlobal(rLocal);
        fPosU1 = rGlobal.Y();
        fPosV1 = rGlobal.Z();
      }
    }
    if (!((index1 == SelDet1) || (index1 == SelDet2))) continue;
    for (int i2 = 0; i2 < storeTelClusters.getEntries() + storePXDClusters.getEntries(); i2++) {
      // preparing of second value for correlation plots with postfix "2":
      float fPosU2 = 0.0;
      float fPosV2 = 0.0;
      int index2 = 0;
      int iIsU2 = 0;
      int iIsV2 = 0;
      int iPlane2 = 0;
      if (i2 < storePXDClusters.getEntries()) {  // PXD clusters:
        auto clusterPXD2 = *storePXDClusters[i2];
        iPlane2 = clusterPXD2.getSensorID().getLayerNumber();
        if ((iPlane2 + 3 < c_firstPXDPlane) || (iPlane2 + 3 > c_lastPXDPlane)) continue;
        if ((clusterPXD2.getSeedCharge() < m_PXDCutSeedL) || (clusterPXD2.getSeedCharge() > m_PXDCutSeedH)) continue;
        index2 = planeToIndex(iPlane2 + 3);
        iIsU2 = 1;
        iIsV2 = 1;
        fPosU2 = getInfoPXD(index2).getUCellID(clusterPXD2.getU());
        fPosV2 = getInfoPXD(index2).getVCellID(clusterPXD2.getV());
        if (m_UseSP) {
          TVector3 rLocal(clusterPXD2.getU(), clusterPXD2.getV(), 0);
          TVector3 rGlobal = getInfoPXD(index2).pointToGlobal(rLocal);
          fPosU2 = rGlobal.Y();
          fPosV2 = rGlobal.Z();
        }
        if (m_SwapAxis) {
          swap(fPosU2, fPosV2);
          swap(iIsU2, iIsV2);
        }
      } else {                                  // Tel clusters:
        auto clusterPXD2 = *storeTelClusters[i2 - storePXDClusters.getEntries()];
        iPlane2 = clusterPXD2.getSensorID().getLayerNumber();
        int iPlane2s = clusterPXD2.getSensorID().getSensorNumber();
        if (iPlane2 != 7) continue;
        if ((iPlane2s < 1) || (iPlane2s > 6)) continue;
        if (iPlane2s <= 3)
          index2 = iPlane2s - 1;
        else
          index2 = iPlane2s - 1 + 6;
        if ((getInfoTel(index2).getUCellID(clusterPXD2.getU()) < -1) || (getInfoTel(index2).getVCellID(clusterPXD2.getV()) < -1)) continue;
        iIsU2 = 1;
        iIsV2 = 1;
        fPosU2 = getInfoTel(index2).getUCellID(clusterPXD2.getU());
        fPosV2 = getInfoTel(index2).getVCellID(clusterPXD2.getV());
        if (m_UseSP) {
          TVector3 rLocal(clusterPXD2.getU(), clusterPXD2.getV(), 0);
          TVector3 rGlobal = getInfoTel(index2).pointToGlobal(rLocal);
          fPosU2 = rGlobal.Y();
          fPosV2 = rGlobal.Z();
        }
      }
      if (!((index2 == SelDet1) || (index2 == SelDet2))) continue;
      // ready to fill correlation histograms and hit maps:
      if ((index1 < index2) && (iIsU1 == iIsU2) && (iIsU1 == 1)) {
        // correlations for u
        m_correlationsTel3PXD2[0]->Fill(fPosU1, fPosU2);
      } else if ((index1 > index2) && (iIsV1 == iIsV2) && (iIsV1 == 1)) {
        // correlations for v
        m_correlationsTel3PXD2[1]->Fill(fPosV2, fPosV1);
      }
    }
  }


// m_correlationsTel4SVD4[u/v]
  SelDet1 = m_DUTTelBwd;  // Tel4
  SelDet2 = m_DUTSVDBwd;  // SVD4
  for (int i1 = 0; i1 < storeSVDClusters.getEntries() + storeTelClusters.getEntries(); i1++) {
    // preparing of first value for correlation plots with postfix "1":
    float fPosU1 = 0.0;
    float fPosV1 = 0.0;
    int index1 = 0;
    int iIsU1 = 0;
    int iIsV1 = 0;
    int iPlane1 = 0;
    if (i1 < storeTelClusters.getEntries()) {  // Tel clusters:
      auto clusterPXD1 = *storeTelClusters[i1];
      iPlane1 = clusterPXD1.getSensorID().getLayerNumber();
      int iPlane1s = clusterPXD1.getSensorID().getSensorNumber();
      if (iPlane1 != 7) continue;
      if ((iPlane1s < 1) || (iPlane1s > 6)) continue;
      if (iPlane1s <= 3)
        index1 = iPlane1s - 1;
      else
        index1 = iPlane1s - 1 + 6;
      if ((getInfoTel(index1).getUCellID(clusterPXD1.getU()) < -1) || (getInfoTel(index1).getVCellID(clusterPXD1.getV()) < -1)) continue;
      iIsU1 = 1;
      iIsV1 = 1;
      fPosU1 = getInfoTel(index1).getUCellID(clusterPXD1.getU());
      fPosV1 = getInfoTel(index1).getVCellID(clusterPXD1.getV());
      if (m_UseSP) {
        TVector3 rLocal(clusterPXD1.getU(), clusterPXD1.getV(), 0);
        TVector3 rGlobal = getInfoTel(index1).pointToGlobal(rLocal);
        fPosU1 = rGlobal.Y();
        fPosV1 = rGlobal.Z();
      }
    } else {                                  // SVD clusters:
      const SVDCluster& cluster1 = *storeSVDClusters[i1 - storeTelClusters.getEntries()];
      iPlane1 = cluster1.getSensorID().getLayerNumber();
      if ((iPlane1 + 3 < c_firstSVDPlane) || (iPlane1 + 3 > c_lastSVDPlane)) continue;
      index1 = planeToIndex(iPlane1 + 3);
      if (cluster1.isUCluster()) {
        iIsU1 = 1;
        fPosU1 = getInfoSVD(index1).getUCellID(cluster1.getPosition());
        if (m_UseSP) {
          TVector3 rLocal(cluster1.getPosition(), 0, 0);
          TVector3 rGlobal = getInfoSVD(index1).pointToGlobal(rLocal);
          fPosU1 = rGlobal.Y();
        }
      } else {
        iIsV1 = 1;
        fPosV1 = getInfoSVD(index1).getVCellID(cluster1.getPosition());
        if (m_UseSP) {
          TVector3 rLocal(0, cluster1.getPosition(), 0);
          TVector3 rGlobal = getInfoSVD(index1).pointToGlobal(rLocal);
          fPosV1 = rGlobal.Z();
        }
      }
      if (m_SwapAxis) {
        swap(fPosU1, fPosV1);
        swap(iIsU1, iIsV1);
      }
    }
    if (!((index1 == SelDet1) || (index1 == SelDet2))) continue;
    for (int i2 = 0; i2 < storeSVDClusters.getEntries() + storeTelClusters.getEntries(); i2++) {
      // preparing of second value for correlation plots with postfix "2":
      float fPosU2 = 0.0;
      float fPosV2 = 0.0;
      int index2 = 0;
      int iIsU2 = 0;
      int iIsV2 = 0;
      int iPlane2 = 0;
      if (i2 < storeTelClusters.getEntries()) {  // Tel clusters:
        auto clusterPXD2 = *storeTelClusters[i2];
        iPlane2 = clusterPXD2.getSensorID().getLayerNumber();
        int iPlane2s = clusterPXD2.getSensorID().getSensorNumber();
        if (iPlane2 != 7) continue;
        if ((iPlane2s < 1) || (iPlane2s > 6)) continue;
        if (iPlane2s <= 3)
          index2 = iPlane2s - 1;
        else
          index2 = iPlane2s - 1 + 6;
        if ((getInfoTel(index2).getUCellID(clusterPXD2.getU()) < -1) || (getInfoTel(index2).getVCellID(clusterPXD2.getV()) < -1)) continue;
        iIsU2 = 1;
        iIsV2 = 1;
        fPosU2 = getInfoTel(index2).getUCellID(clusterPXD2.getU());
        fPosV2 = getInfoTel(index2).getVCellID(clusterPXD2.getV());
        if (m_UseSP) {
          TVector3 rLocal(clusterPXD2.getU(), clusterPXD2.getV(), 0);
          TVector3 rGlobal = getInfoTel(index2).pointToGlobal(rLocal);
          fPosU2 = rGlobal.Y();
          fPosV2 = rGlobal.Z();
        }
      } else {                                  // SVD clusters:
        const SVDCluster& cluster2 = *storeSVDClusters[i2 - storeTelClusters.getEntries()];
        iPlane2 = cluster2.getSensorID().getLayerNumber();
        if ((iPlane2 + 3 < c_firstSVDPlane) || (iPlane2 + 3 > c_lastSVDPlane)) continue;
        index2 = planeToIndex(iPlane2 + 3);
        if (cluster2.isUCluster()) {
          iIsU2 = 1;
          fPosU2 = getInfoSVD(index2).getUCellID(cluster2.getPosition());
          if (m_UseSP) {
            TVector3 rLocal(cluster2.getPosition(), 0, 0);
            TVector3 rGlobal = getInfoSVD(index2).pointToGlobal(rLocal);
            fPosU2 = rGlobal.Y();
          }
        } else {
          iIsV2 = 1;
          fPosV2 = getInfoSVD(index2).getVCellID(cluster2.getPosition());
          if (m_UseSP) {
            TVector3 rLocal(0, cluster2.getPosition(), 0);
            TVector3 rGlobal = getInfoSVD(index2).pointToGlobal(rLocal);
            fPosV2 = rGlobal.Z();
          }
        }
        if (m_SwapAxis) {
          swap(fPosU2, fPosV2);
          swap(iIsU2, iIsV2);
        }
      }
      if (!((index2 == SelDet1) || (index2 == SelDet2))) continue;
      // ready to fill correlation histograms and hit maps:
      if ((index1 < index2) && (iIsU1 == iIsU2) && (iIsU1 == 1)) {
        // correlations for u
        m_correlationsTel4SVD4[0]->Fill(fPosU2, fPosU1);
      } else if ((index1 > index2) && (iIsV1 == iIsV2) && (iIsV1 == 1)) {
        // correlations for v
        m_correlationsTel4SVD4[1]->Fill(fPosV1, fPosV2);
      }
    }
  }

// m_correlationsTel3SVD1[u/v]
  SelDet1 = m_DUTTelFwd;  // Tel3
  SelDet2 = m_DUTSVDFwd;  // SVD4
  for (int i1 = 0; i1 < storeSVDClusters.getEntries() + storeTelClusters.getEntries(); i1++) {
    // preparing of first value for correlation plots with postfix "1":
    float fPosU1 = 0.0;
    float fPosV1 = 0.0;
    int index1 = 0;
    int iIsU1 = 0;
    int iIsV1 = 0;
    int iPlane1 = 0;
    if (i1 < storeTelClusters.getEntries()) {  // Tel clusters:
      auto clusterPXD1 = *storeTelClusters[i1];
      iPlane1 = clusterPXD1.getSensorID().getLayerNumber();
      int iPlane1s = clusterPXD1.getSensorID().getSensorNumber();
      if (iPlane1 != 7) continue;
      if ((iPlane1s < 1) || (iPlane1s > 6)) continue;
      if (iPlane1s <= 3)
        index1 = iPlane1s - 1;
      else
        index1 = iPlane1s - 1 + 6;
      if ((getInfoTel(index1).getUCellID(clusterPXD1.getU()) < -1) || (getInfoTel(index1).getVCellID(clusterPXD1.getV()) < -1)) continue;
      iIsU1 = 1;
      iIsV1 = 1;
      fPosU1 = getInfoTel(index1).getUCellID(clusterPXD1.getU());
      fPosV1 = getInfoTel(index1).getVCellID(clusterPXD1.getV());
      if (m_UseSP) {
        TVector3 rLocal(clusterPXD1.getU(), clusterPXD1.getV(), 0);
        TVector3 rGlobal = getInfoTel(index1).pointToGlobal(rLocal);
        fPosU1 = rGlobal.Y();
        fPosV1 = rGlobal.Z();
      }
    } else {                                  // SVD clusters:
      const SVDCluster& cluster1 = *storeSVDClusters[i1 - storeTelClusters.getEntries()];
      iPlane1 = cluster1.getSensorID().getLayerNumber();
      if ((iPlane1 + 3 < c_firstSVDPlane) || (iPlane1 + 3 > c_lastSVDPlane)) continue;
      index1 = planeToIndex(iPlane1 + 3);
      if (cluster1.isUCluster()) {
        iIsU1 = 1;
        fPosU1 = getInfoSVD(index1).getUCellID(cluster1.getPosition());
        if (m_UseSP) {
          TVector3 rLocal(cluster1.getPosition(), 0, 0);
          TVector3 rGlobal = getInfoSVD(index1).pointToGlobal(rLocal);
          fPosU1 = rGlobal.Y();
        }
      } else {
        iIsV1 = 1;
        fPosV1 = getInfoSVD(index1).getVCellID(cluster1.getPosition());
        if (m_UseSP) {
          TVector3 rLocal(0, cluster1.getPosition(), 0);
          TVector3 rGlobal = getInfoSVD(index1).pointToGlobal(rLocal);
          fPosV1 = rGlobal.Z();
        }
      }
      if (m_SwapAxis) {
        swap(fPosU1, fPosV1);
        swap(iIsU1, iIsV1);
      }
    }
    if (!((index1 == SelDet1) || (index1 == SelDet2))) continue;
    for (int i2 = 0; i2 < storeSVDClusters.getEntries() + storeTelClusters.getEntries(); i2++) {
      // preparing of second value for correlation plots with postfix "2":
      float fPosU2 = 0.0;
      float fPosV2 = 0.0;
      int index2 = 0;
      int iIsU2 = 0;
      int iIsV2 = 0;
      int iPlane2 = 0;
      if (i2 < storeTelClusters.getEntries()) {  // Tel clusters:
        auto clusterPXD2 = *storeTelClusters[i2];
        iPlane2 = clusterPXD2.getSensorID().getLayerNumber();
        int iPlane2s = clusterPXD2.getSensorID().getSensorNumber();
        if (iPlane2 != 7) continue;
        if ((iPlane2s < 1) || (iPlane2s > 6)) continue;
        if (iPlane2s <= 3)
          index2 = iPlane2s - 1;
        else
          index2 = iPlane2s - 1 + 6;
        if ((getInfoTel(index2).getUCellID(clusterPXD2.getU()) < -1) || (getInfoTel(index2).getVCellID(clusterPXD2.getV()) < -1)) continue;
        iIsU2 = 1;
        iIsV2 = 1;
        fPosU2 = getInfoTel(index2).getUCellID(clusterPXD2.getU());
        fPosV2 = getInfoTel(index2).getVCellID(clusterPXD2.getV());
        if (m_UseSP) {
          TVector3 rLocal(clusterPXD2.getU(), clusterPXD2.getV(), 0);
          TVector3 rGlobal = getInfoTel(index2).pointToGlobal(rLocal);
          fPosU2 = rGlobal.Y();
          fPosV2 = rGlobal.Z();
        }
      } else {                                  // SVD clusters:
        const SVDCluster& cluster2 = *storeSVDClusters[i2 - storeTelClusters.getEntries()];
        iPlane2 = cluster2.getSensorID().getLayerNumber();
        if ((iPlane2 + 3 < c_firstSVDPlane) || (iPlane2 + 3 > c_lastSVDPlane)) continue;
        index2 = planeToIndex(iPlane2 + 3);
        if (cluster2.isUCluster()) {
          iIsU2 = 1;
          fPosU2 = getInfoSVD(index2).getUCellID(cluster2.getPosition());
          if (m_UseSP) {
            TVector3 rLocal(cluster2.getPosition(), 0, 0);
            TVector3 rGlobal = getInfoSVD(index2).pointToGlobal(rLocal);
            fPosU2 = rGlobal.Y();
          }
        } else {
          iIsV2 = 1;
          fPosV2 = getInfoSVD(index2).getVCellID(cluster2.getPosition());
          if (m_UseSP) {
            TVector3 rLocal(0, cluster2.getPosition(), 0);
            TVector3 rGlobal = getInfoSVD(index2).pointToGlobal(rLocal);
            fPosV2 = rGlobal.Z();
          }
        }
        if (m_SwapAxis) {
          swap(fPosU2, fPosV2);
          swap(iIsU2, iIsV2);
        }
      }
      if (!((index2 == SelDet1) || (index2 == SelDet2))) continue;
      // ready to fill correlation histograms and hit maps:
      if ((index1 < index2) && (iIsU1 == iIsU2) && (iIsU1 == 1)) {
        // correlations for u
        m_correlationsTel3SVD1[0]->Fill(fPosU1, fPosU2);
      } else if ((index1 > index2) && (iIsV1 == iIsV2) && (iIsV1 == 1)) {
        // correlations for v
        m_correlationsTel3SVD1[1]->Fill(fPosV2, fPosV1);
      }
    }
  }

}


void TelxVXDModule::endRun()
{
}


void TelxVXDModule::terminate()
{
}
