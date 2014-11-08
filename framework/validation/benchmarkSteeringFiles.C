/*
<header>
<output></output>
<contact>Christian Pulvermacher, christian.pulvermacher@kit.edu</contact>
</header>
*/
#include <TTimeStamp.h>
#include <TString.h>
#include <TFile.h>
#include <TNtuple.h>
#include <TSystem.h>


//how often do we want to execute stuff? (result of first execution ignored to warm caches)
const int nRuns = 4; //make this >= 3

/** Helper function to get stdout of 'cmd'. */
TString exec(const char* cmd) {
  FILE* pipe = gSystem->OpenPipe(cmd, "r");
  if (!pipe) {
    std::cerr << "Couldn't get pipe!\n";
    exit(1);
  }

  char buffer[128];
  TString result("");
  while(!feof(pipe)) {
    if(fgets(buffer, 128, pipe) != NULL)
      result += buffer;
  }
  gSystem->ClosePipe(pipe);
  return result;
}

/** Run each steering file in a list of files a few times, store execution time in Ntuple. */
void benchmarkSteeringFiles()
{
  const char* files[] = {
    "framework/tests/root_output.py",
    "framework/tests/evtmetadata.py",
    "framework/tests/module_paths.py",
    "framework/tests/seqroot_input.py",
    "framework/tests/chain_input.py",
    "framework/tests/root_input.py"
  };
  TFile* output = TFile::Open("steering_file_benchmark.root", "recreate");
  output->cd();

  const TString tmpdir = exec("mktemp -d").Strip(TString::kTrailing, '\n');

  const int nFiles = sizeof(files)/sizeof(const char*);
  for (int iFile = 0; iFile < nFiles; iFile++) {
    TString path(files[iFile]);
    int lastslash = path.Last('/');
    TNtuple* bench = new TNtuple(path.Remove(0, lastslash+1), "", "time_avg_ms:time_stdev_ms");
    std::vector<double> times_ms;

    for (int iRun = 0; iRun < nRuns; iRun++) {
      //(in /tmp/ to avoid producing .root files in current directory)
      //no idea how it actually finds the steering files after the cd...
      const char* cmd = TString::Format("cd %s && basf2 %s%s", tmpdir.Data(), "../../../", files[iFile]).Data();

      TTimeStamp start;
      //do stuff
      system(cmd);
      TTimeStamp end;

      if (iRun == 0)
        continue;

      const double start_ns = start.GetSec() * 1e9 + start.GetNanoSec();
      const double end_ns = end.GetSec() * 1e9 + end.GetNanoSec();
      times_ms.push_back((end_ns - start_ns) / 1e6);
    }

    double sum = 0;
    double squares = 0;
    for (int i = 0; i < times_ms.size(); i++) {
      sum += times_ms[i];
      squares += times_ms[i] * times_ms[i];
      std::cout << "sum : " << sum << "\n";
      std::cout << "squares : " << squares << "\n";
    }
      std::cout << "size : " << times_ms.size() << "\n";
    const double average = sum / times_ms.size();
    bench->Fill(average, sqrt(squares / times_ms.size() - average * average) / (times_ms.size() - 1));
    bench->SetAlias("Description", TString::Format("Measures average runtime of the '%s' test over %d runs", files[iFile], nRuns));
    bench->SetAlias("Check", "Increases in runtime could indicate performance regressions in the framework.");
    bench->Write();
  }

  system(TString::Format("rm -r %s", tmpdir.Data()).Data());

  delete output;
}
