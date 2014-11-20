#include <topcaf/dataobjects/topFileMetaData.h>
#include <framework/logging/Logger.h>
#include <TString.h>

using namespace Belle2;

ClassImp(topFileMetaData)

void topFileMetaData::set(std::string dir, std::string filename)
{

  TString fname(filename.c_str());

  TString exp_prefix(fname(0, fname.First('-')));
  fname = fname(fname.First('-') + 1, fname.First('.'));

  TString run_type(fname(0, fname.First('-')));
  fname = fname(fname.First('-') + 1, fname.First('.'));

  TString exp_name(fname(0, fname.First('r')));
  fname = fname(fname.First('r'), fname.First('.'));
  //  TString exp_sub(exp_name(1,exp_name.Length()));
  //  int exp_number = exp_sub.Atoi();


  TString run_name(fname(0, fname.First('-')));
  fname = fname(fname.First('-') + 1, fname.First('.'));
  //  TString run_sub(run_name(1,run_name.Length()));
  //  int run_number = run_sub.Atoi();

  std::string experiment(Form("%s-%s", exp_prefix.Data(), exp_name.Data()));
  std::string run(run_name.Data());
  std::string runtype(run_type.Data());


  m_dir = dir;
  m_filename = filename;
  m_experiment = experiment;
  m_run = run;
  m_runtype = runtype;

  B2INFO("top file meta data " << m_dir << m_filename << "; experiment: " << m_experiment << "\trun: " << m_run << "\trun type: " << runtype);
}
