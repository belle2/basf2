#include <daq/dqm/DqmMasterCallback.h>

#include <daq/slc/base/StringUtil.h>

#include <unistd.h>

#include <sys/stat.h>
#include <sys/types.h>
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

}

DqmMasterCallback::~DqmMasterCallback()
{

}

void DqmMasterCallback::load(const DBObject& /* obj */, const std::string& /* runtype */)
{
}

void DqmMasterCallback::start(int expno, int runno)
{
  m_expno = expno;
  m_runno = runno;
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





