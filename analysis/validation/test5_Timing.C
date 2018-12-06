/*
<header>
  <input>../GenericB_GENSIMRECtoDST.dst.root,../DSTtoMDST.mdst.root,../MDSTtoUDST.udst.root,../UDSTtoNTUP.ntup.root,FileEventSizes.root</input>
  <output>Timing.root</output>
  <contact>Luis Pesantex, pesantez@uni-bonn.de</contact>
</header>
*/
#include <TTimeStamp.h>
#include <TString.h>
#include <TFile.h>
#include <TNtuple.h>
#include <TSystem.h>

#include <iostream>


//how often do we want to execute stuff? (result of first execution ignored to warm caches)
const int nRuns = 2;
//make this >= 3

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

/** Run each DST analysis steering file in a list of files a few times, store execution time in Ntuple. */
void test5_Timing()
{
    const char* files[] = {
        "analysis/validation/test1_DSTtoMDST.py",
        "analysis/validation/test2_MDSTtoUDST.py",
        "analysis/validation/test3_UDSTtoNTUP.py"
    };
    TFile* output = TFile::Open("Timing.root", "recreate");
    output->cd();

    const TString tmpdir = exec("mktemp -d").Strip(TString::kTrailing, '\n');

    for (int iFile = 0; iFile < 3; iFile++) {
        TString path(files[iFile]);
        int lastslash = path.Last('/');
        TNtuple* bench = new TNtuple(path.Remove(0, lastslash+1), "", "time_avg_ms:time_stdev_ms");
        bench->SetAlias("Description", "Time to execute scripts.");
        bench->SetAlias("Check", "Consistency with previous.");
        bench->SetAlias("Contact", "sam.cunliffe@desy.de;thomas.kuhr@lmu.de");
        std::vector<double> times_ms;

        for (int iRun = 0; iRun < nRuns; iRun++) {
            //(in /tmp/ to avoid producing .root files in current directory)
            //no idea how it actually finds the steering files after the cd...

            const TString cmd = TString::Format("basf2 ../../../%s", files[iFile]);

            TTimeStamp start;
            //do stuff
            if (system(cmd.Data()) != 0)
              exit(1);
            TTimeStamp end;

            if (iRun == 0)
                continue;

            const double start_ns = start.GetSec() * 1e9 + start.GetNanoSec();
            const double end_ns = end.GetSec() * 1e9 + end.GetNanoSec();
            times_ms.push_back((end_ns - start_ns) / 1e6);
        }

        double sum = 0;
        double squares = 0;
        for (unsigned int i = 0; i < times_ms.size(); i++) {
            sum += times_ms[i];
            squares += times_ms[i] * times_ms[i];
            std::cout << "sum : " << sum << "\n";
            std::cout << "squares : " << squares << "\n";
        }
        std::cout << "size : " << times_ms.size() << "\n";
        const double average = sum / times_ms.size();
        bench->Fill(average, sqrt(squares / times_ms.size() - average * average) / (times_ms.size() - 1));
        bench->Write();
    }

    TString cmd = TString::Format("rm -r %s", tmpdir.Data());
    system(cmd.Data());

    delete output;
}

