/*
<header>
<input>../GenericB_GENSIMRECtoDST.dst.root,../DSTtoMDST.mdst.root,../MDSTtoUDST.udst.root</input>
<output>FileEventSizes.root</output>
<contact>Luis Pesantex, pesantez@uni-bonn.de</contact>
</header>
*/

void test4_FileSizes() {

// open the file with simulated and reconstructed EvtGen particles
    TFile* input = TFile::Open("../GenericB_GENSIMRECtoDST.dst.root");
    TTree* meta = (TTree*) input->Get("tree");

    TH1F* hEvents = new TH1F("hEvents", "number of processed events", 1, 0, 1);
    hEvents->GetXaxis()->SetTitle("Event");
    meta->Draw("EventMetaData.m_event>>hEvents");
    double eventsize = input->GetSize()/1024 / hEvents->GetEntries();

    TFile* inputm = TFile::Open("../DSTtoMDST.mdst.root");
    TTree* metam = (TTree*) inputm->Get("tree");

    TH1F* hEventsm = new TH1F("hEvents", "number of processed events", 1, 0, 1);
    hEventsm->GetXaxis()->SetTitle("Event");
    metam->Draw("EventMetaData.m_event>>hEvents");
    double eventsizem = inputm->GetSize()/1024 / hEventsm->GetEntries();

    TFile* inputu = TFile::Open("../MDSTtoUDST.udst.root");
    TTree* metau = (TTree*) inputu->Get("tree");

    TH1F* hEventsu = new TH1F("hEvents", "number of processed events", 1, 0, 1);
    hEventsu->GetXaxis()->SetTitle("Event");
    metau->Draw("EventMetaData.m_event>>hEvents");
    double eventsizeu = inputu->GetSize()/1024 / hEventsu->GetEntries();



    TFile* output = TFile::Open("FileEventSizes.root", "recreate");
    TNtuple* bench = new TNtuple("DST event size in kB", "tree", "dst:mdst:udst");
    bench->Fill(eventsize,eventsizem,eventsizeu);
    bench->SetAlias("Description", "File sizes in kB");
    bench->SetAlias("Check", "Consistency with previous, no large spikes.");
    bench->SetAlias("Contact", "sam.cunliffe@desy.de;thomas.kuhr@lmu.de");
    bench->Write();
    delete output;


}
