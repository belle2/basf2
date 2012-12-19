#include <display/modules/display/DisplayUI.h>

#include <framework/core/ModuleParam.h>
#include <framework/core/InputController.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/logging/Logger.h>
#include <display/modules/display/SplitGLView.h>

#include <TApplication.h>
#include <TEveBrowser.h>
#include <TEveManager.h>
#include <TEveEventManager.h>
#include <TEveScene.h>
#include <TEveViewer.h>
#include <TGButton.h>
#include <TGFrame.h>
#include <TGLabel.h>
#include <TGNumberEntry.h>
#include <TGFileDialog.h>
#include <TGInputDialog.h>
#include <TGTextEntry.h>
#include <TGLViewer.h>
#include <TROOT.h>
#include <TSystem.h>


#include <boost/scoped_ptr.hpp>

#include <cmath>
#include <cstdlib>


using namespace Belle2;

DisplayUI::DisplayUI(bool automatic):
  m_currentEntry(0),
  m_guiInitialized(false),
  m_automatic(automatic),
  m_prevButton(0),
  m_nextButton(0)
{
  if ((!gApplication) || (gApplication && gApplication->TestBit(TApplication::kDefaultApplication))) {
    new TApplication("ROOT_application", 0, 0);
  }
  if (!gEve) {
    B2INFO("Creating TEve window ...");
    TEveManager::Create(!m_automatic, "I"); //show window in interactive mode, hide file browser
  }

  TEveBrowser* browser = gEve->GetBrowser();
  browser->SetWindowName("Display");
  browser->HideBottomTab();
  browser->StartEmbedding(TRootBrowser::kRight);
  m_viewer = new SplitGLView(gClient->GetRoot(), 800, 700);
  browser->StopEmbedding();

  m_eventData = new TEveElementList();
}


DisplayUI::~DisplayUI() { }

void DisplayUI::addParameter(const std::string& label, ModuleParam<bool> &param)
{
  m_paramList.push_back(make_pair(label, &param));
}

void DisplayUI::next()
{
  goToEvent(m_currentEntry + 1);
}

void DisplayUI::prev()
{
  if (!InputController::canControlInput())
    return;
  goToEvent(m_currentEntry - 1);
}

void DisplayUI::updateUI()
{
  if (InputController::canControlInput())
    m_currentEntry = InputController::getCurrentEntry();

  //change UI state?
  const long numEntries = InputController::numEntries();
  m_prevButton->SetEnabled(InputController::canControlInput() and m_currentEntry > 0);
  m_nextButton->SetEnabled((m_currentEntry + 1 < numEntries) or !InputController::canControlInput());
  if (m_currentEntry != m_eventNumberWidget->GetIntNumber())
    m_eventNumberWidget->SetIntNumber(m_currentEntry);
  StoreObjPtr<EventMetaData> eventMetaData;
  m_eventLabel->SetTextColor(gROOT->GetColor(kBlack));
  if (!eventMetaData) {
    m_eventLabel->SetText("No EventMetaData object available.");
  } else {
    m_eventLabel->SetText(TString::Format("Event: \t\t%lu\nRun: \t\t%lu\nExperiment: \t%lu", eventMetaData->getEvent(), eventMetaData->getRun(), eventMetaData->getExperiment()));
  }
}

void DisplayUI::goToEvent(Long_t id)
{
  if (id < 0)
    id = 0;
  const long numEntries = InputController::numEntries();
  if (id >= numEntries and InputController::canControlInput())
    id = numEntries - 1;


  if (m_currentEntry == id) return;

  if (!InputController::canControlInput() && m_currentEntry != id - 1) {
    B2ERROR("Cannot switch to event " << id << ", only works in conjunction with RootInput.");
  }

  if (numEntries > 0 && InputController::canControlInput()) {
    B2DEBUG(100, "Switching to event " << id);
    m_eventLabel->SetTextColor(gROOT->GetColor(kRed + 1));
    m_eventLabel->SetText(" \nLoading...\n ");
    InputController::setNextEntry(id);
  } else {
    m_currentEntry++;
  }

  B2DEBUG(100, "exiting event loop now.");
  //exit event loop to allow basf2 to go to next event
  gSystem->ExitLoop();
}

void DisplayUI::goToEvent(Long_t event, Long_t run, Long_t experiment)
{
  const long numEntries = InputController::numEntries();
  if (numEntries > 0 && InputController::canControlInput()) {
    B2DEBUG(100, "Switching to event " << event << " in run " << run << ", experiment " << experiment);
    m_eventLabel->SetTextColor(gROOT->GetColor(kRed + 1));
    m_eventLabel->SetText(" \nLoading...\n ");
    InputController::setNextEntry(experiment, run, event);
  }
  B2DEBUG(100, "exiting event loop now.");
  gSystem->ExitLoop();
}

void DisplayUI::goToEventWidget()
{
  goToEvent(m_eventNumberWidget->GetIntNumber());
}

void DisplayUI::showJumpToEventDialog()
{
  StoreObjPtr<EventMetaData> eventMetaData;
  if (!eventMetaData)
    return; //this should not actually happen.

  //m_eventLabel->SetText(TString::Format("Event: \t\t%lu\nRun: \t\t%lu\nExperiment: \t%lu", eventMetaData->getEvent(), eventMetaData->getRun(), eventMetaData->getExperiment()));
  char returnString[256];
  new TGInputDialog(gEve->GetBrowser()->GetClient()->GetDefaultRoot(), gEve->GetBrowser(),
                    "Jump to event '#evt/#run/#exp':",
                    TString::Format("%lu/%lu/%lu", eventMetaData->getEvent(), eventMetaData->getRun(), eventMetaData->getExperiment()),
                    returnString);
  if (returnString[0] == '\0')
    return; //cancelled

  boost::scoped_ptr<TObjArray> stringList(TString(returnString).Tokenize("/"));
  if (stringList->GetEntriesFast() != 3) {
    B2WARNING("Wrong format!");
    return;
  }
  const TString& evt = static_cast<TObjString*>(stringList->At(0))->GetString();
  const TString& run = static_cast<TObjString*>(stringList->At(1))->GetString();
  const TString& exp = static_cast<TObjString*>(stringList->At(2))->GetString();
  if (!evt.IsDec()) {
    B2WARNING("Event number is not numeric?");
    return;
  }
  if (!run.IsDec()) {
    B2WARNING("Run number is not numeric?");
    return;
  }
  if (!exp.IsDec()) {
    B2WARNING("Experiment number is not numeric?");
    return;
  }
  goToEvent(evt.Atoll(), run.Atoll(), exp.Atoll());
}

void DisplayUI::clearEvent()
{
  if (!gEve)
    return;

  if (gEve->GetCurrentEvent())
    gEve->GetCurrentEvent()->DestroyElements();

  //make sure we delete projected events with the rest of the event scene
  m_eventData->DestroyElements();
}

bool DisplayUI::startDisplay()
{
  m_reshowCurrentEvent = false;
  if (!m_guiInitialized) {
    makeGui();

    //import the geometry in the projection managers (only needs to be done once
    B2INFO("Creating projections ...");
    TEveScene* gs = gEve->GetGlobalScene();
    TEveProjectionManager* rphiManager = getViewer()->getRPhiMgr();
    if (rphiManager) {
      rphiManager->ImportElements(gs);
    }
    TEveProjectionManager* rhozManager = getViewer()->getRhoZMgr();
    if (rhozManager) {
      rhozManager->ImportElements(gs);
    }
  }

  updateUI(); //update button state

  m_eventData->AddElement(m_viewer->getRPhiMgr()->ImportElements((TEveElement*)gEve->GetEventScene()));
  m_eventData->AddElement(m_viewer->getRhoZMgr()->ImportElements((TEveElement*)gEve->GetEventScene()));

  if (!m_automatic) {
    gEve->Redraw3D(false); //do not reset camera when redrawing

    //make display interactive
    gApplication->Run(true); //return from Run()
    //interactive part done, event data removed from scene
  } else {
    automaticEvent();
  }

  return m_reshowCurrentEvent;
}



void DisplayUI::makeGui()
{
  m_guiInitialized = true;
  TEveBrowser* browser = gEve->GetBrowser();

  //workaround for regression in root 5.34/03: closing display window from WM wouldn't stop event loop
  browser->Connect("CloseWindow()", "TSystem", gSystem, "ExitLoop()");

  browser->StartEmbedding(TRootBrowser::kLeft);

  TGMainFrame* frmMain = new TGMainFrame(gClient->GetRoot(), 240, 600);
  frmMain->SetWindowName("Event Control main frame");
  frmMain->SetCleanup(kDeepCleanup);

  TGGroupFrame* event_frame = new TGGroupFrame(frmMain);
  event_frame->SetTitle("Event");
  {
    TGHorizontalFrame* hf = new TGHorizontalFrame(event_frame);
    {
      TString icondir(Form("%s/icons/", gSystem->Getenv("ROOTSYS")));

      m_prevButton = new TGPictureButton(hf, gClient->GetPicture(icondir + "GoBack.gif"));
      hf->AddFrame(m_prevButton, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 5, 5, 5, 5));
      m_prevButton->Connect("Clicked()", "Belle2::DisplayUI", this, "prev()");

      const long numEntries = InputController::numEntries();
      m_eventNumberWidget = new TGNumberEntry(hf, 0, 4, 999, TGNumberFormat::kNESInteger,
                                              TGNumberFormat::kNEANonNegative,
                                              TGNumberFormat::kNELLimitMinMax,
                                              0, numEntries - 1);
      m_eventNumberWidget->SetState(InputController::canControlInput());
      hf->AddFrame(m_eventNumberWidget, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 5, 5, 5, 5));
      //note: parameter of ValueSet signal is _not_ the number just set.
      m_eventNumberWidget->Connect("ValueSet(Long_t)", "Belle2::DisplayUI", this, "goToEventWidget()");
      m_eventNumberWidget->GetNumberEntry()->Connect("ReturnPressed()", "Belle2::DisplayUI", this, "goToEventWidget()");


      if (InputController::canControlInput()) {
        TGLabel* maxEvents = new TGLabel(hf, TString::Format("/%ld", numEntries - 1));
        hf->AddFrame(maxEvents, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 5, 5, 5, 5));
      }

      m_nextButton = new TGPictureButton(hf, gClient->GetPicture(icondir + "GoForward.gif"));
      hf->AddFrame(m_nextButton, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 5, 5, 5, 5));
      m_nextButton->Connect("Clicked()", "Belle2::DisplayUI", this, "next()");
    }
    event_frame->AddFrame(hf, new TGLayoutHints(kLHintsExpandX | kLHintsCenterY, 5, 5, 5, 5));

    TGButton* jumpToEventButton = new TGTextButton(event_frame, "Jump to event/run/exp");
    jumpToEventButton->SetEnabled(InputController::canControlInput());
    event_frame->AddFrame(jumpToEventButton, new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 5, 5, 5, 5));
    jumpToEventButton->Connect("Clicked()", "Belle2::DisplayUI", this, "showJumpToEventDialog()");

    m_eventLabel = new TGLabel(event_frame);
    event_frame->AddFrame(m_eventLabel, new TGLayoutHints(kLHintsExpandX | kLHintsCenterY, 5, 5, 5, 5));
  }
  frmMain->AddFrame(event_frame, new TGLayoutHints(kLHintsExpandX, 5, 5, 5, 5));

  TGGroupFrame* viewer_frame = new TGGroupFrame(frmMain);
  viewer_frame->SetTitle("Current Viewer");
  {
    TGButton* b = 0;
    TGHorizontalFrame* hf = new TGHorizontalFrame(viewer_frame);
    {
      b = new TGTextButton(hf, "Dark/light colors");
      hf->AddFrame(b, new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 5, 5, 5, 5));
      b->Connect("Clicked()", "Belle2::DisplayUI", this, "toggleColorScheme()");
    }
    viewer_frame->AddFrame(hf, new TGLayoutHints(kLHintsExpandX | kLHintsCenterY, 5, 5, 5, 5));

    hf = new TGHorizontalFrame(viewer_frame);
    {
      b = new TGTextButton(hf, "Save Picture");
      hf->AddFrame(b, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 5, 5, 5, 5));
      b->Connect("Clicked()", "Belle2::DisplayUI", this, "savePicture()");

      b = new TGTextButton(hf, "Save Hi-Res Picture");
      hf->AddFrame(b, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 5, 5, 5, 5));
      b->Connect("Clicked()", "Belle2::DisplayUI", this, "saveHiResPicture()");

    }
    viewer_frame->AddFrame(hf, new TGLayoutHints(kLHintsExpandX | kLHintsCenterY, 5, 5, 5, 5));
  }
  frmMain->AddFrame(viewer_frame, new TGLayoutHints(kLHintsExpandX, 5, 5, 5, 5));

  TGGroupFrame* param_frame = new TGGroupFrame(frmMain);
  param_frame->SetTitle("Options");
  {
    const int nParams = m_paramList.size();
    for (int i = 0; i < nParams; i++) {
      TGCheckButton* b = new TGCheckButton(param_frame, m_paramList[i].first.c_str(), i);
      b->SetToolTipText(m_paramList[i].second->getDescription().c_str());
      b->SetState(m_paramList[i].second->getValue() ? kButtonDown : kButtonUp);
      b->Connect("Clicked()", "Belle2::DisplayUI", this, TString::Format("handleParameterChange(=%d)", i));
      param_frame->AddFrame(b, new TGLayoutHints(kLHintsExpandX | kLHintsCenterY, 5, 5, 5, 5));
    }
  }
  frmMain->AddFrame(param_frame, new TGLayoutHints(kLHintsExpandX, 5, 5, 5, 5));

  TGGroupFrame* automatisation_frame = new TGGroupFrame(frmMain);
  automatisation_frame->SetTitle("Automatic Saving");
  {
    TGHorizontalFrame* hf = new TGHorizontalFrame(automatisation_frame);
    {
      TGLabel* prefixLabel = new TGLabel(hf, "Prefix:");
      hf->AddFrame(prefixLabel, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 5, 5, 5, 5));

      m_autoFileNamePrefix = new TGTextEntry(hf, "display_");
      hf->AddFrame(m_autoFileNamePrefix, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 5, 5, 5, 5));
    }
    automatisation_frame->AddFrame(hf, new TGLayoutHints(kLHintsExpandX | kLHintsCenterY, 5, 5, 5, 5));

    TGButton* b = 0;
    hf = new TGHorizontalFrame(automatisation_frame);
    {
      TGLabel* widthLabel = new TGLabel(hf, "Width (px):");
      hf->AddFrame(widthLabel, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 5, 5, 5, 5));

      m_autoPictureWidth = new TGNumberEntry(hf, 800, 5, 998, TGNumberFormat::kNESInteger,
                                             TGNumberFormat::kNEANonNegative,
                                             TGNumberFormat::kNELLimitMinMax,
                                             100, 6000);
      hf->AddFrame(m_autoPictureWidth, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 5, 5, 5, 5));

      b = new TGTextButton(hf, "Save PNGs");
      hf->AddFrame(b, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 5, 5, 5, 5));
      b->Connect("Clicked()", "Belle2::DisplayUI", this, "startAutomaticRun()");
    }
    automatisation_frame->AddFrame(hf, new TGLayoutHints(kLHintsExpandX | kLHintsCenterY, 5, 5, 5, 5));

  }
  frmMain->AddFrame(automatisation_frame, new TGLayoutHints(kLHintsExpandX, 5, 5, 5, 5));

  //this will be shown at the very bottom
  TGGroupFrame* exit_frame = new TGGroupFrame(frmMain);
  exit_frame->SetTitle("Closing");
  {
    TGHorizontalFrame* hf = new TGHorizontalFrame(exit_frame);
    {
      TGButton* b = new TGTextButton(hf, "Continue without display");
      hf->AddFrame(b, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 5, 5, 5, 5));
      b->Connect("Clicked()", "Belle2::DisplayUI", this, "closeAndContinue()");

      b = new TGTextButton(hf, "Exit");
      hf->AddFrame(b, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 5, 5, 5, 5));
      b->Connect("Clicked()", "Belle2::DisplayUI", this, "exit()");

    }
    exit_frame->AddFrame(hf, new TGLayoutHints(kLHintsExpandX | kLHintsCenterY, 5, 5, 5, 5));

  }
  frmMain->AddFrame(exit_frame, new TGLayoutHints(kLHintsExpandX | kLHintsBottom, 5, 5, 5, 5));

  frmMain->MapSubwindows();
  frmMain->Resize();
  frmMain->MapWindow();
  browser->StopEmbedding();

  browser->SetTabTitle("Event Control", 0);
}

void DisplayUI::handleParameterChange(int id)
{
  if (id >= (int)m_paramList.size()) {
    B2ERROR("widget ID too large!");
    return;
  }
  //toggle value
  m_paramList[id].second->setValue(!m_paramList[id].second->getValue());

  //reprocess current event
  m_reshowCurrentEvent = true;
  gSystem->ExitLoop();
}

void DisplayUI::toggleColorScheme()
{
  TEveViewerList* viewers = gEve->GetViewers();
  TEveElement::List_ci end_it = viewers->EndChildren();
  for (TEveElement::List_i it = viewers->BeginChildren(); it != end_it; ++it) {
    TEveViewer* v = static_cast<TEveViewer*>(*it);
    TGLViewer* glv = v->GetGLViewer();

    bool dark = glv->ColorSet().Background().GetColorIndex() != kWhite;
    glv->RefLightColorSet().Background().SetColor(kWhite);
    glv->RefDarkColorSet().Background().SetColor(kBlack);
    if (dark)
      glv->UseLightColorSet();
    else
      glv->UseDarkColorSet();
    glv->DoDraw();
  }
}

void DisplayUI::savePicture(bool highres)
{
  TGFileInfo fi;
  //deleting the pointer crashes, so I'm assuming this is magically cleaned up at some point
  new TGFileDialog(gEve->GetBrowser()->GetClient()->GetDefaultRoot(), gEve->GetBrowser(), kFDSave, &fi);
  if (!fi.fFilename)
    return; //cancelled
  TGLViewer* v = m_viewer->getActiveGLViewer();
  if (!highres) {
    v->SavePicture(fi.fFilename);
  } else {
    B2INFO("Saving high-resolution picture..."); //may take a while
    v->SavePictureWidth(fi.fFilename, 4000);
  }

  B2INFO("Saved picture in: " << fi.fFilename)

  //file dialog leaves empty box, redraw
  gEve->Redraw3D(false); //do not reset camera when redrawing
}

void DisplayUI::startAutomaticRun()
{
  B2INFO("Starting automatic run.");
  m_automatic = true;

  //save current event, too
  automaticEvent();

  gSystem->ExitLoop();
}

void DisplayUI::automaticEvent()
{
  static int i = 0;
  B2INFO("Saving event " << i);

  //force immediate redraw
  gEve->FullRedraw3D();

  TEveViewerList* viewers = gEve->GetViewers();
  TEveElement::List_ci end_it = viewers->EndChildren();
  for (TEveElement::List_i it = viewers->BeginChildren(); it != end_it; ++it) {
    TEveViewer* v = static_cast<TEveViewer*>(*it);
    TGLViewer* glv = v->GetGLViewer();

    TString projectionName(v->GetName());
    projectionName.ReplaceAll(" viewer", "");
    const int width = m_autoPictureWidth->GetIntNumber();
    glv->SavePictureWidth(TString::Format("%s%d_%s.png", m_autoFileNamePrefix->GetText(), i, projectionName.Data()), width);
  }

  i++;
}

void DisplayUI::closeAndContinue()
{
  gEve->GetBrowser()->SendCloseMessage();
}

void DisplayUI::exit()
{
  gEve->GetBrowser()->SendCloseMessage();

  //stop event processing after current event
  StoreObjPtr<EventMetaData> eventMetaData;
  eventMetaData->setEndOfData();
}

ClassImp(DisplayUI)
