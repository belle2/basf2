//+
// File : MonitorDataCOPPER.cc
// Description : Module to monitor raw data
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 2 - Aug - 2013
//-
#include <daq/rawdata/modules/MonitorDataCOPPER.h>


using namespace std;
using namespace Belle2;


//#define DEBUG

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(MonitorDataCOPPER)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

MonitorDataCOPPERModule::MonitorDataCOPPERModule() : HistoModule()
{
  //Set module properties

  setDescription("Monitor Raw Data");
  setPropertyFlags(c_ParallelProcessingCertified);
  m_loop = -1;

}



MonitorDataCOPPERModule::~MonitorDataCOPPERModule()
{
}

void MonitorDataCOPPERModule::defineHisto()
{

  double t_min = 0.;
  double t_max = 600.;
  int t_nbin = 600;

  h_ncpr = new TH1F("h_ncpr", "Number of COPPERs", 10, 0.0, 10.0);
  h_ncpr->SetYTitle("# of Events");
  h_ncpr->SetXTitle("COPPER ID");

  // event number
  h_nevt = new TH1F("h_nevt", "Number of Events; Number of events; # of entries", t_nbin, t_min, t_max);
  h_nevt->SetXTitle("Event Number");
  h_nevt->SetYTitle("# of events");

  // event rate
  h_rate = new TH1F("h_rate", "Event rate", t_nbin, t_min, t_max);
  h_rate->SetXTitle("Time [s]");
  h_rate->SetYTitle("Event rate [1/s]");

  // size / event
  h_size = new TH1F("h_size", "Event Data Size; Data size [byte]; # of entries", 100, 0.0, 2000.0);
  h_size->SetXTitle("Event Data Size[byte]");
  h_size->SetYTitle("# of Events");

  h_size_0 = new TH1F("h_size_0", "Event Data Size; Data size [byte]; # of entries", 100, 0.0, 2000.0);
  h_size_0->SetXTitle("Event Data Size/COPPER[byte]");
  h_size_0->SetYTitle("# of Events");

  h_size_1 = new TH1F("h_size_1", "Event Data Size; Data size [byte]; # of entries", 100, 0.0, 2000.0);
  h_size_1->SetXTitle("Event Data Size/COPPER[byte]");
  h_size_1->SetYTitle("# of Events");

  h_size_2 = new TH1F("h_size_2", "Event Data Size; Data size [byte]; # of entries", 100, 0.0, 2000.0);
  h_size_2->SetXTitle("Event Data Size/COPPER[byte]");
  h_size_2->SetYTitle("# of Events");

  h_size_3 = new TH1F("h_size_3", "Event Data Size; Data size [byte]; # of entries", 100, 0.0, 2000.0);
  h_size_3->SetXTitle("Event Data Size/COPPER[byte]");
  h_size_3->SetYTitle("# of Events");


  h_size_var = new TH1F("h_size_var", "Data Size; Data size [byte]; # of entries", t_nbin, t_min, t_max);
  h_size_var->SetXTitle("Data Size/COPPER[byte]");
  h_size_var->SetYTitle("# of Events");

  h_size_var_0 = new TH1F("h_size_var_0", "Data Size; Data size [byte]; # of entries", t_nbin, t_min, t_max);
  h_size_var_0->SetXTitle("Data Size/COPPER[byte]");
  h_size_var_0->SetYTitle("# of Events");

  h_size_var_1 = new TH1F("h_size_var_1", "Data Size; Data size [byte]; # of entries", t_nbin, t_min, t_max);
  h_size_var_1->SetXTitle("Data Size/COPPER[byte]");
  h_size_var_1->SetYTitle("# of Events");

  h_size_var_2 = new TH1F("h_size_var_2", "Data Size; Data size [byte]; # of entries", t_nbin, t_min, t_max);
  h_size_var_2->SetXTitle("Data Size/COPPER[byte]");
  h_size_var_2->SetYTitle("# of Events");

  h_size_var_3 = new TH1F("h_size_var_3", "Data Size; Data size [byte]; # of entries", t_nbin, t_min, t_max);
  h_size_var_3->SetXTitle("Data Size/COPPER[byte]");
  h_size_var_3->SetYTitle("# of Events");


  h_flow_rate = new TH1F("h_flow_rate", "Data Size; Data size [byte]; # of entries", t_nbin, t_min, t_max);
  h_flow_rate->SetXTitle("Data Size/COPPER[byte]");
  h_flow_rate->SetYTitle("# of Events");

  h_flow_rate_0 = new TH1F("h_flow_rate_0", "Data Size; Data size [byte]; # of entries", t_nbin, t_min, t_max);
  h_flow_rate_0->SetXTitle("Data Size/COPPER[byte]");
  h_flow_rate_0->SetYTitle("# of Events");

  h_flow_rate_1 = new TH1F("h_flow_rate_1", "Data Size; Data size [byte]; # of entries", t_nbin, t_min, t_max);
  h_flow_rate_1->SetXTitle("Data Size/COPPER[byte]");
  h_flow_rate_1->SetYTitle("# of Events");

  h_flow_rate_2 = new TH1F("h_flow_rate_2", "Data Size; Data size [byte]; # of entries", t_nbin, t_min, t_max);
  h_flow_rate_2->SetXTitle("Data Size/COPPER[byte]");
  h_flow_rate_2->SetYTitle("# of Events");

  h_flow_rate_3 = new TH1F("h_flow_rate_3", "Data Size; Data size [byte]; # of entries", t_nbin, t_min, t_max);
  h_flow_rate_3->SetXTitle("Data Size/COPPER[byte]");
  h_flow_rate_3->SetYTitle("# of Events");




  h_size2d = new TH2F("h_size2d", "Data Size vs. Copper;Copper ID;Data size [byte]",  100, 0.0, 2000.0, 5, 0.0, 5.0);
  h_size2d->SetYTitle("Data Size/COPPER[byte]");
  h_size2d->SetXTitle("COPPER ID");



}


void MonitorDataCOPPERModule::initialize()
{
  REG_HISTOGRAM
}


void MonitorDataCOPPERModule::beginRun()
{

}




void MonitorDataCOPPERModule::endRun()
{
  //fill Run data


}


void MonitorDataCOPPERModule::terminate()
{

}

double MonitorDataCOPPERModule::GetTimeSec()
{
  struct timeval t;
  gettimeofday(&t, NULL);
  return (t.tv_sec + t.tv_usec * 1.e-6);
}



//
// User defined functions
//


void MonitorDataCOPPERModule::event()
{
  if (m_loop <= 0) {
    m_start_time = GetTimeSec();
    m_loop = 0;
    m_nevt = 0;
  }

  h_nevt->Fill((float)m_loop);
  StoreArray<RawCOPPER> rawcprarray;
  StoreArray<RawDataBlock> raw_dblkarray;

  int ncpr = raw_dblkarray.getEntries();

  for (int j = 0; j < ncpr; j++) {
    printf("%d word %d numnode %d numeve %d preall %d \n", j,
           raw_dblkarray[ j ]->TotalBufNwords(),
           raw_dblkarray[ j ]->GetNumNodes(),
           raw_dblkarray[ j ]->GetNumEvents(),
           raw_dblkarray[ j ]->GetPreAllocFlag());

//     printf("*******BODY**********\n");
//     printf("\n%.8d : ", 0);

//     int* temp_buf = raw_dblkarray[ j ]->GetWholeBuffer();
//     for (int i = 0; i < raw_dblkarray[ j ]->TotalBufNwords(); i++) {
//       printf("0x%.8x ", temp_buf[ i ] );
//       if ((i + 1) % 10 == 0) {
//  printf("\n%.8d : ", i + 1);
//       }
//     }
//     printf("\n");
//     printf("\n");

    for (int i = 0; i < raw_dblkarray[ j ]->GetNumEntries(); i++) {

//       RawHeader rawhdr;
//       int* buf;
//       int size_byte = 0;
//       printf("check 1\n");
//       fflush(stdout);
//       buf = raw_dblkarray[ j ]->GetBuffer(i);
//       printf("check 2\n");
//       fflush(stdout);
//       rawhdr.SetBuffer(raw_dblkarray[ j ]->GetRawHdrBufPtr(i));
//       printf("check 3\n");
//       fflush(stdout);
//       size_byte = raw_dblkarray[ j ]->GetBlockNwords(i) * sizeof(int);
//       printf("check 4\n");
//       fflush(stdout);

//       h_ncpr->Fill((float)i);
//       h_size->Fill((float)size_byte);
//       h_size2d->Fill((float)size_byte, (float)i);

//       switch (i) {
//         case 0 :
//           h_size_0->Fill((float)size_byte);
//           m_size_byte_0 += size_byte;
//           break;
//         case 1 :
//           h_size_1->Fill((float)size_byte);
//           m_size_byte_1 += size_byte;
//           break;
//         case 2 :
//           h_size_2->Fill((float)size_byte);
//           m_size_byte_2 += size_byte;
//           break;
//         case 3 :
//           h_size_3->Fill((float)size_byte);
//           m_size_byte_3 += size_byte;
//           break;
//         default  :
//           break;

//       }

      /*
      printf("=== event====\n exp %d run %d eve %d copperNode %d type %d size %d byte\n",
       rawhdr.GetExpNo(),
       rawhdr.GetRunNo(),
       rawhdr.GetEveNo(),
       rawhdr.GetSubsysId(),
       rawhdr.GetDataType(),
       size_byte);
      */

//       int* finnesse_buf_1st = rawcprarray[ j ]->Get1stFINNESSEBuffer(i);
//       int* finnesse_buf_2nd = rawcprarray[ j ]->Get2ndFINNESSEBuffer(i);
//       int* finnesse_buf_3rd = rawcprarray[ j ]->Get3rdFINNESSEBuffer(i);
//       int* finnesse_buf_4th = rawcprarray[ j ]->Get4thFINNESSEBuffer(i);
      //    printf("FEEbuf %p %p %p %p\n", fee_buf_1st, fee_buf_2nd, fee_buf_3rd, fee_buf_4th);
    }

  }

//   if (m_loop % 100 == 0) {
//     double cur_time = GetTimeSec();
//     double tdiff_cur = cur_time - m_start_time;
//     double tdiff_prev = m_prev_time - m_start_time;
//     int bin_cur = h_rate->GetBin(tdiff_cur);
//     int bin_prev = h_rate->GetBin(tdiff_prev);

//     double rate = (m_nevt - m_prev_nevt) / (tdiff_cur - tdiff_prev);
//     double flow_rate_0 = (m_size_byte_0 - m_prev_size_byte_0) / (tdiff_cur - tdiff_prev);
//     double flow_rate_1 = (m_size_byte_1 - m_prev_size_byte_1) / (tdiff_cur - tdiff_prev);
//     double flow_rate_2 = (m_size_byte_2 - m_prev_size_byte_2) / (tdiff_cur - tdiff_prev);
//     double flow_rate_3 = (m_size_byte_3 - m_prev_size_byte_3) / (tdiff_cur - tdiff_prev);

//     double size_var_0 = (m_size_byte_0 - m_prev_size_byte_0) / (m_nevt - m_prev_nevt);
//     double size_var_1 = (m_size_byte_1 - m_prev_size_byte_1) / (m_nevt - m_prev_nevt);
//     double size_var_2 = (m_size_byte_2 - m_prev_size_byte_2) / (m_nevt - m_prev_nevt);
//     double size_var_3 = (m_size_byte_3 - m_prev_size_byte_3) / (m_nevt - m_prev_nevt);

//     for (int i = bin_prev + 1; i <= bin_cur; i++) {
//       h_rate->SetBinContent(i, rate);
//       h_flow_rate_0->SetBinContent(i, flow_rate_0);
//       h_flow_rate_1->SetBinContent(i, flow_rate_1);
//       h_flow_rate_2->SetBinContent(i, flow_rate_2);
//       h_flow_rate_3->SetBinContent(i, flow_rate_3);
//       h_flow_rate->SetBinContent(i, flow_rate_0 + flow_rate_1 + flow_rate_2 + flow_rate_3);

//       h_size_var_0->SetBinContent(i, size_var_0);
//       h_size_var_1->SetBinContent(i, size_var_1);
//       h_size_var_2->SetBinContent(i, size_var_2);
//       h_size_var_3->SetBinContent(i, size_var_3);
//       h_size_var->SetBinContent(i, size_var_0 + size_var_1 + size_var_2 + size_var_3);

//       h_nevt->SetBinContent(i, (float)m_nevt);
//     }

//     m_prev_nevt = m_nevt;
//     m_prev_time = cur_time;

//   }
  m_loop++;
}
