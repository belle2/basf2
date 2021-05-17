#include <daq/dqm/DqmMasterCallback.h>
#include <framework/pcore/EvtMessage.h>
#include <framework/pcore/MsgHandler.h>

#include <TText.h>
#include <TH1.h>
#include <TKey.h>

#include <unistd.h>

#include <sys/wait.h>

using namespace Belle2;
using namespace std;

static DqmMasterCallback* s_dqmmaster = NULL;

static int m_running = 0;

//-----------------------------------------------------------------
// Rbuf-Read Thread Interface
//-----------------------------------------------------------------
void* RunDqmMasterLogger(void*)
{
}

DqmMasterCallback::DqmMasterCallback(ConfigFile& config)
{
  m_hltdir = config.get("dqmmaster.hltdir");
  m_erecodir = config.get("dqmmaster.erecodir");
  m_running = 0;
  printf("DqmMasterCallback : hltdir = %s, erecodir = %s\n", m_hltdir.c_str(), m_erecodir.c_str());

  // Open MemFile
  m_hltdqm = new DqmMemFile("dqmhisto_hlt");
  m_erecodqm = new DqmMemFile("dqmhisto_reco1");

  // Open sockets to hservers
  m_sock_hlt = new EvtSocketSend("localhost", 9991);
  m_sock_reco = new EvtSocketSend("localhost", 9992);
}

DqmMasterCallback::~DqmMasterCallback()
{

}

void DqmMasterCallback::load(const DBObject& /* obj */, const std::string& runtype)
{
  m_runtype = runtype;
}

void DqmMasterCallback::start(int expno, int runno)
{
  m_expno = expno;
  m_runno = runno;

  MsgHandler hdl(0);
  int numobjs = 0;

  TText rc_clear(0, 0, "DQMRC:CLEAR");
  hdl.add(&rc_clear, "DQMRC:CLEAR");
  numobjs++;
  TText subdir(0, 0, "DQMInfo");
  hdl.add(&subdir, "SUBDIR:DQMInfo") ;
  numobjs++;
  TH1F h_expno("expno", to_string(m_expno).c_str(), 1, 0, 1);
  hdl.add(&h_expno, "expno");
  numobjs++;
  TH1F h_runno("runno", to_string(m_runno).c_str(), 1, 0, 1);
  hdl.add(&h_runno, "runno");
  numobjs++;
  TH1F h_rtype("rtype", m_runtype.c_str(), 1, 0, 1);
  hdl.add(&h_rtype, "rtype");
  numobjs++;
  TText command(0, 0, "COMMAND:EXIT");
  hdl.add(&command, "SUBDIR:EXIT");
  numobjs++;
  TText rc_merge(0, 0, "DQMRC:MERGE");
  hdl.add(&rc_merge, "DQMRC:MERGE");
  numobjs++;

  EvtMessage* msg = hdl.encode_msg(MSG_EVENT);
  (msg->header())->reserved[0] = 0;
  (msg->header())->reserved[1] = numobjs;

  m_sock_hlt->send(msg);
  m_sock_reco->send(msg);
  delete(msg);

  printf("expno = %d, runno = %d\n", m_expno, m_runno);
  m_running = 1;
}

void DqmMasterCallback::stop()
{

  if (m_running == 0) return;

  m_running = 0;

  char outfile[1024];
  //  m_expno = getExpNumber();
  //  m_runno = getRunNumber();

  // Connect TMemFile
  TMemFile* hlttmem = m_hltdqm->LoadMemFile();
  TMemFile* erecotmem = m_erecodqm->LoadMemFile();

  // Dump HLT DQM
  int proc1 = fork();
  if (proc1 == 0) {
    //  TMemFile* tmem = m_hltdqm->LoadMemFile();
    sprintf(outfile, "%s/hltdqm_e%4.4dr%6.6d.root", m_hltdir.c_str(), m_expno, m_runno);
    TFile* dqmtfile = new TFile(outfile, "RECREATE");
    printf("HLT dqm file = %s\n", outfile);

    // Copy all histograms in TFile
    TIter next(hlttmem->GetListOfKeys());
    TKey* key = NULL;
    while ((key = (TKey*)next())) {
      TH1* hist = (TH1*)key->ReadObj();
      printf("HistTitle %s : entries = %f\n", hist->GetName(), hist->GetEntries());
      TH1* cpyhst = (TH1*)hist->Clone();
    }

    // Close TFile
    dqmtfile->Write();
    dqmtfile->Close();
    delete dqmtfile;
    //    delete hlttmem;
    exit(0);
  } else if (proc1 < 0) {
    perror("DQMMASTER : fork HLTDQM writing");
  }

  // Dump ERECO DQM
  int proc2 = fork();
  if (proc2 == 0) {
    //    TMemFile* tmem = m_erecodqm->LoadMemFile();
    sprintf(outfile, "%s/erecodqm_e%4.4dr%6.6d.root", m_erecodir.c_str(), m_expno, m_runno);
    //  sprintf(outfile, "hltdqm_e%4.4dr%6.6d.root", m_expno, m_runno);
    TFile* erdqmtfile = new TFile(outfile, "RECREATE");
    printf("ERECO dqm file = %s\n", outfile);

    // Copy all histograms in TFile
    TIter ernext(erecotmem->GetListOfKeys());
    TKey* erkey = NULL;
    while ((erkey = (TKey*)ernext())) {
      TH1* hist = (TH1*)erkey->ReadObj();
      printf("HistTitle %s : entries = %f\n", hist->GetName(), hist->GetEntries());
      TH1* cpyhst = (TH1*)hist->Clone();
    }

    // Close TFile
    erdqmtfile->Write();
    erdqmtfile->Close();
    delete erdqmtfile;
    //    delete erecotmem;
    exit(0);
  } else if (proc2 < 0) {
    perror("DQMMASTER : fork ERECODQM writing");
  }

  // Wait completion
  int status1, status2;
  waitpid(proc1, &status1, 0);
  waitpid(proc2, &status2, 0);

  //  delete hlttmem;
  //  delete erecotmem;
}

void DqmMasterCallback::abort()
{
  stop();
}





