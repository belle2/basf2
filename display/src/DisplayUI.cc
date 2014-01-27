//needs to be first
#include <framework/core/ModuleParam.h>

#include <display/DisplayUI.h>
#include <display/VisualRepMap.h>

#include <framework/core/InputController.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationVector.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/logging/Logger.h>
#include <display/dataobjects/DisplayData.h>
#include <display/async/AsyncWrapper.h>
#include <display/BrowsableWrapper.h>
#include <display/SplitGLView.h>

#include <TApplication.h>
#include <TCanvas.h>
#include <TEveBrowser.h>
#include <TEveManager.h>
#include <TEveEventManager.h>
#include <TEveSelection.h>
#include <TEveScene.h>
#include <TEveViewer.h>
#include <TGButton.h>
#include <TGFileBrowser.h>
#include <TGFrame.h>
#include <TGLabel.h>
#include <TGNumberEntry.h>
#include <TGMsgBox.h>
#include <TGFileDialog.h>
#include <TGInputDialog.h>
#include <TGTextEntry.h>
#include <TGLViewer.h>
#include <TMacro.h>
#include <TObject.h>
#include <TObjArray.h>
#include <TObjString.h>
#include <TROOT.h>
#include <TSystem.h>

#include <boost/scoped_ptr.hpp>

#include <cmath>


using namespace Belle2;

ClassImp(DisplayUI)

DisplayUI::DisplayUI(bool automatic):
  m_currentEntry(0),
  m_guiInitialized(false),
  m_automatic(automatic),
  m_cumulative(false),
  m_prevButton(0),
  m_nextButton(0),
  m_timer(0)
{
  if (!gEve) {
    B2INFO("Creating TEve window.");
    TEveManager::Create(!m_automatic, "I"); //show window in interactive mode, hide file browser
  }

  setTitle(); //set default window title
  TEveBrowser* browser = gEve->GetBrowser();
  browser->HideBottomTab();
  browser->StartEmbedding(TRootBrowser::kRight);
  m_viewPane = new SplitGLView();
  browser->StopEmbedding();

  m_eventData = new TEveElementList();
}


DisplayUI::~DisplayUI()
{
  delete m_timer;
}

void DisplayUI::addParameter(const std::string& label, ModuleParam<bool>& param, int level)
{
  Parameter p;
  p.m_label = label;
  p.m_param = &param;
  p.m_level = level;
  m_paramList.push_back(p);
}

void DisplayUI::next()
{
  if (!m_nextButton->IsEnabled())
    return; // periodically called by auto-advance timer, but we don't want to freeze UI if no events are there
  goToEvent(m_currentEntry + 1);
}

void DisplayUI::prev()
{
  if (!InputController::canControlInput())
    return;
  goToEvent(m_currentEntry - 1);
}

void DisplayUI::setTitle(const std::string& fileName)
{
  std::string title("Belle II Event Display");
  if (!fileName.empty())
    title += " - " + fileName;

  TEveBrowser* browser = gEve->GetBrowser();
  browser->SetWindowName(title.c_str());
}

void DisplayUI::updateUI()
{
  if (InputController::canControlInput()) {
    m_currentEntry = InputController::getCurrentEntry();

    setTitle(InputController::getCurrentFileName());
  }

  //change UI state?
  const long numEntries = InputController::numEntries();
  m_prevButton->SetEnabled(InputController::canControlInput() and m_currentEntry > 0);
  m_nextButton->SetEnabled((m_currentEntry + 1 < numEntries) or !InputController::canControlInput());
  if (m_currentEntry != m_eventNumberWidget->GetIntNumber())
    m_eventNumberWidget->SetIntNumber(m_currentEntry);
  if (m_timer and InputController::canControlInput() and !m_nextButton->IsEnabled()) {
    //reached last file entry in play mode, stop
    togglePlayPause();
  }
  StoreObjPtr<EventMetaData> eventMetaData;
  m_eventLabel->SetTextColor(gROOT->GetColor(kBlack));
  if (!eventMetaData) {
    m_eventLabel->SetText("No EventMetaData object available.");
  } else {
    m_eventLabel->SetText(TString::Format("Event: \t\t%lu\nRun: \t\t%lu\nExperiment: \t%lu", eventMetaData->getEvent(), eventMetaData->getRun(), eventMetaData->getExperiment()));
  }
  m_eventLabel->Resize();
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
    InputController::setNextEntry(id);
  } else {
    m_currentEntry++;
  }
  m_eventLabel->SetTextColor(gROOT->GetColor(kRed + 1));
  m_eventLabel->SetText(" \nLoading...\n ");
  if (m_timer)
    m_timer->Stop(); //apparently timer only deactivates after processing event, so do it manually
  //process remaining events to ensure redraw (only needed if called from Timeout())
  gSystem->ProcessEvents();

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

void DisplayUI::autoAdvanceDelayChanged()
{
  if (m_timer) {
    m_timer->Stop();
    m_timer->Start((int)(1000.0 * m_autoAdvanceDelay->GetNumber()));
  }
}

void DisplayUI::togglePlayPause()
{
  const TString icondir(Form("%s/icons/", gSystem->Getenv("ROOTSYS")));
  if (m_timer) {
    //pause
    delete m_timer;
    m_timer = 0;
    m_playPauseButton->SetPicture(gClient->GetPicture(icondir + "ed_execute.png"));
  } else {
    //play
    m_timer = new TTimer();
    const int pollIntervalMs = (int)(1000.0 * m_autoAdvanceDelay->GetNumber());
    m_timer->Connect("Timeout()", "Belle2::DisplayUI", this, "next()");
    m_timer->Start(pollIntervalMs);
    m_playPauseButton->SetPicture(gClient->GetPicture(icondir + "ed_interrupt.png"));
  }
}

void DisplayUI::showJumpToEventDialog()
{
  StoreObjPtr<EventMetaData> eventMetaData;
  if (!eventMetaData)
    return; //this should not actually happen.

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
  if (m_cumulative) {
    gEve->AddEvent(new TEveEventManager());
  } else {
    if (gEve->GetCurrentEvent())
      gEve->GetCurrentEvent()->DestroyElements();

    //make sure we delete projected events with the rest of the event scene
    m_eventData->DestroyElements();
  }
}

void DisplayUI::selectionHandler(TEveElement* eveObj)
{
  const TObject* representedObject = m_visualRepMap->getDataStoreObject(eveObj);
  if (representedObject) {
    //representedObject->Dump();

    const RelationVector<TObject>& relatedObjects = DataStore::Instance().getRelationsWithObj<TObject>(representedObject, "ALL");
    for (const TObject & relObj : relatedObjects) {
      //relObj.Print();
      TEveElement* relObjRep = m_visualRepMap->getEveElement(&relObj);
      if (relObjRep and !gEve->GetSelection()->HasChild(relObjRep)) {
        //select this object in addition to existing selection
        gEve->GetSelection()->UserPickedElement(relObjRep, true);
      }
    }
  }

}

bool DisplayUI::startDisplay()
{
  m_reshowCurrentEvent = false;
  if (!m_guiInitialized) {
    makeGui();
    if (AsyncWrapper::isAsync()) {
      //continually check for new events and enable/disable '->' button accordingly
      TTimer* t = new TTimer();
      const int pollIntervalMs = 300;
      t->Connect("Timeout()", "Belle2::DisplayUI", this, "pollNewEvents()");
      t->Start(pollIntervalMs);
    }

    //import the geometry in the projection managers (only needs to be done once)
    TEveScene* gs = gEve->GetGlobalScene();
    TEveProjectionManager* rphiManager = getViewPane()->getRPhiMgr();
    if (rphiManager) {
      rphiManager->ImportElements(gs);
    }
    TEveProjectionManager* rhozManager = getViewPane()->getRhoZMgr();
    if (rhozManager) {
      rhozManager->ImportElements(gs);
    }

    //We want to do special things when objects are selected
    gEve->GetSelection()->Connect("SelectionAdded(TEveElement*)", "Belle2::DisplayUI", this, "selectionHandler(TEveElement*)");
  }

  updateUI(); //update button state

  m_eventData->AddElement(getViewPane()->getRPhiMgr()->ImportElements((TEveElement*)gEve->GetEventScene()));
  m_eventData->AddElement(getViewPane()->getRhoZMgr()->ImportElements((TEveElement*)gEve->GetEventScene()));

  if (!m_automatic) {
    gEve->Redraw3D(false); //do not reset camera when redrawing

    //restart auto-advance timer after loading event (interval already set)
    if (m_timer)
      m_timer->Start(-1);

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
  const int margin = 3;

  //workaround for regression in root 5.34/03: closing display window from WM wouldn't stop event loop
  browser->Connect("CloseWindow()", "TSystem", gSystem, "ExitLoop()");
  browser->Connect("CloseWindow()", "Belle2::DisplayUI", this, "closeAndContinue()");

  browser->StartEmbedding(TRootBrowser::kLeft);

  TGMainFrame* frmMain = new TGMainFrame(gClient->GetRoot(), 240, 600);
  frmMain->SetWindowName("Event control main frame");
  frmMain->SetCleanup(kDeepCleanup);

  const TString icondir(Form("%s/icons/", gSystem->Getenv("ROOTSYS")));

  TGGroupFrame* event_frame = new TGGroupFrame(frmMain);
  event_frame->SetTitle("Event");
  {
    TGHorizontalFrame* hf = new TGHorizontalFrame(event_frame);
    {
      m_prevButton = new TGPictureButton(hf, gClient->GetPicture(icondir + "GoBack.gif"));
      m_prevButton->SetToolTipText("Go to previous event");
      hf->AddFrame(m_prevButton, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, margin, margin, margin, margin));
      m_prevButton->Connect("Clicked()", "Belle2::DisplayUI", this, "prev()");

      const long numEntries = InputController::numEntries();
      m_eventNumberWidget = new TGNumberEntry(hf, 0, 4, 999, TGNumberFormat::kNESInteger,
                                              TGNumberFormat::kNEANonNegative,
                                              TGNumberFormat::kNELLimitMinMax,
                                              0, numEntries - 1);
      m_eventNumberWidget->SetState(InputController::canControlInput());
      hf->AddFrame(m_eventNumberWidget, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, margin, margin, margin, margin));
      //note: parameter of ValueSet signal is _not_ the number just set.
      m_eventNumberWidget->Connect("ValueSet(Long_t)", "Belle2::DisplayUI", this, "goToEventWidget()");
      m_eventNumberWidget->GetNumberEntry()->Connect("ReturnPressed()", "Belle2::DisplayUI", this, "goToEventWidget()");


      if (InputController::canControlInput()) {
        TGLabel* maxEvents = new TGLabel(hf, TString::Format("/%ld", numEntries - 1));
        hf->AddFrame(maxEvents, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, margin, margin, margin, margin));
      }

      m_nextButton = new TGPictureButton(hf, gClient->GetPicture(icondir + "GoForward.gif"));
      m_nextButton->SetToolTipText("Go to next event");
      hf->AddFrame(m_nextButton, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, margin, margin, margin, margin));
      m_nextButton->Connect("Clicked()", "Belle2::DisplayUI", this, "next()");
    }
    event_frame->AddFrame(hf, new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 0, 0, 0, 0));

    hf = new TGHorizontalFrame(event_frame);
    {
      //const bool file = InputController::canControlInput();
      const bool async = AsyncWrapper::isAsync();

      TGLabel* delayLabel = new TGLabel(hf, "Delay (s):");
      hf->AddFrame(delayLabel, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, margin, margin, margin, margin));

      const double valueSeconds = async ? 0.5 : 3.5;
      m_autoAdvanceDelay = new TGNumberEntry(hf, valueSeconds, 3, 999, TGNumberFormat::kNESRealOne,
                                             TGNumberFormat::kNEAPositive,
                                             TGNumberFormat::kNELLimitMin,
                                             0.1); //minimum
      //m_autoAdvanceDelay->SetState(file or async);
      hf->AddFrame(m_autoAdvanceDelay, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, margin, margin, margin, margin));
      //note: parameter of ValueSet signal is _not_ the number just set.
      m_autoAdvanceDelay->Connect("ValueSet(Long_t)", "Belle2::DisplayUI", this, "autoAdvanceDelayChanged()");
      m_autoAdvanceDelay->GetNumberEntry()->Connect("ReturnPressed()", "Belle2::DisplayUI", this, "autoAdvanceDelayChanged()");

      m_playPauseButton = new TGPictureButton(hf, gClient->GetPicture(icondir + "ed_execute.png"));
      //m_playPauseButton->SetEnabled(file or async);
      m_playPauseButton->SetToolTipText("Advance automatically to next event after the given delay.");
      hf->AddFrame(m_playPauseButton, new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, margin, margin, margin, margin));
      m_playPauseButton->Connect("Clicked()", "Belle2::DisplayUI", this, "togglePlayPause()");
    }
    event_frame->AddFrame(hf, new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 0, 0, 0, 0));

    TGButton* jumpToEventButton = new TGTextButton(event_frame, "Jump to event/run/exp...");
    jumpToEventButton->SetEnabled(InputController::canControlInput());
    jumpToEventButton->SetToolTipText("Find a given entry identified by an event / run / experiment triplet in the current file");
    event_frame->AddFrame(jumpToEventButton, new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, margin, margin, margin, margin));
    jumpToEventButton->Connect("Clicked()", "Belle2::DisplayUI", this, "showJumpToEventDialog()");

    m_eventLabel = new TGLabel(event_frame);
    event_frame->AddFrame(m_eventLabel, new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, margin, margin, margin, margin));
  }
  frmMain->AddFrame(event_frame, new TGLayoutHints(kLHintsExpandX, 0, 0, 0, 0));

  TGGroupFrame* param_frame = new TGGroupFrame(frmMain);
  param_frame->SetTitle("Options");
  {
    const int nParams = m_paramList.size();
    for (int i = 0; i < nParams; i++) {
      TGCheckButton* b = new TGCheckButton(param_frame, m_paramList[i].m_label.c_str(), i);
      b->SetToolTipText(m_paramList[i].m_param->getDescription().c_str());
      b->SetState(m_paramList[i].m_param->getValue() ? kButtonDown : kButtonUp);
      b->Connect("Clicked()", "Belle2::DisplayUI", this, TString::Format("handleParameterChange(=%d)", i));
      int indentation = 15 * m_paramList[i].m_level;
      param_frame->AddFrame(b, new TGLayoutHints(kLHintsExpandX | kLHintsCenterY, indentation, margin, margin, margin));
    }

  }
  frmMain->AddFrame(param_frame, new TGLayoutHints(kLHintsExpandX, 0, 0, 0, 0));

  TGGroupFrame* viewer_frame = new TGGroupFrame(frmMain);
  viewer_frame->SetTitle("Viewer");
  {
    TGButton* b = 0;
    TGHorizontalFrame* hf = new TGHorizontalFrame(viewer_frame);
    {
      b = new TGTextButton(hf, "Dark/light colors");
      b->SetToolTipText("Toggle background color");
      hf->AddFrame(b, new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, margin, margin, margin, margin));
      b->Connect("Clicked()", "Belle2::DisplayUI", this, "toggleColorScheme()");
    }
    viewer_frame->AddFrame(hf, new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 0, 0, 0, 0));


    hf = new TGHorizontalFrame(viewer_frame);
    {
      b = new TGTextButton(hf, "Save As...");
      b->SetToolTipText("Save a bitmap graphic for the current viewer");
      hf->AddFrame(b, new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, margin + 1, margin, margin, margin));
      b->Connect("Clicked()", "Belle2::DisplayUI", this, "savePicture()");

      b = new TGTextButton(hf, "Save As (High-Res)... ");
      b->SetToolTipText("Save a bitmap graphic for the current viewer with user-specified size");
      hf->AddFrame(b, new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, margin, margin, margin, margin));
      b->Connect("Clicked()", "Belle2::DisplayUI", this, "saveHiResPicture()");

    }
    viewer_frame->AddFrame(hf, new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 0, 0, 0, 0));

    {
      TGCheckButton* b = new TGCheckButton(viewer_frame, "Cumulative mode (experimental)");
      b->SetToolTipText("Do not erase previous event, i.e. show data from multiple events.");
      b->SetState(m_cumulative ? kButtonDown : kButtonUp);
      b->Connect("Clicked()", "Belle2::DisplayUI", this, "toggleCumulative()");
      viewer_frame->AddFrame(b, new TGLayoutHints(kLHintsExpandX | kLHintsCenterY, 0, margin, margin, margin));
    }

  }
  frmMain->AddFrame(viewer_frame, new TGLayoutHints(kLHintsExpandX, 0, 0, 0, 0));

  TGGroupFrame* automatisation_frame = new TGGroupFrame(frmMain);
  automatisation_frame->SetTitle("Automatic Saving (experimental)");
  {
    TGHorizontalFrame* hf = new TGHorizontalFrame(automatisation_frame);
    {
      TGLabel* prefixLabel = new TGLabel(hf, "Prefix:");
      hf->AddFrame(prefixLabel, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, margin, margin, margin, margin));

      m_autoFileNamePrefix = new TGTextEntry(hf, "display_");
      hf->AddFrame(m_autoFileNamePrefix, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, margin, margin, margin, margin));
    }
    automatisation_frame->AddFrame(hf, new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 0, 0, 0, 0));

    hf = new TGHorizontalFrame(automatisation_frame);
    {
      TGLabel* widthLabel = new TGLabel(hf, "Width (px):");
      hf->AddFrame(widthLabel, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, margin, margin, margin, margin));

      m_autoPictureWidth = new TGNumberEntry(hf, 800, 5, 998, TGNumberFormat::kNESInteger,
                                             TGNumberFormat::kNEANonNegative,
                                             TGNumberFormat::kNELLimitMinMax,
                                             100, 6000);
      hf->AddFrame(m_autoPictureWidth, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, margin, margin, margin, margin));

      TGButton* b = new TGTextButton(hf, "Save PNGs");
      b->SetToolTipText("Save bitmap graphics for all further events. Cannot be aborted. (EXPERIMENTAL)");
      hf->AddFrame(b, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, margin, margin, margin, margin));
      b->Connect("Clicked()", "Belle2::DisplayUI", this, "startAutomaticRun()");
    }
    automatisation_frame->AddFrame(hf, new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 0, 0, 0, 0));

  }
  frmMain->AddFrame(automatisation_frame, new TGLayoutHints(kLHintsExpandX, 0, 0, 0, 0));

  //this will be shown at the very bottom
  TGGroupFrame* exit_frame = new TGGroupFrame(frmMain);
  exit_frame->SetTitle("Closing");
  {
    TGHorizontalFrame* hf = new TGHorizontalFrame(exit_frame);
    {
      TGButton* b = new TGTextButton(hf, "Continue without display");
      b->SetToolTipText("Close the display but continue producing/reading events.");
      hf->AddFrame(b, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, margin, margin, margin, margin));
      b->Connect("Clicked()", "Belle2::DisplayUI", this, "closeAndContinue()");

      b = new TGTextButton(hf, "Exit");
      b->SetToolTipText("Close the display and stop basf2 after this event.");
      hf->AddFrame(b, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, margin, margin, margin, margin));
      b->Connect("Clicked()", "Belle2::DisplayUI", this, "exit()");

    }
    exit_frame->AddFrame(hf, new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 0, 0, 0, 0));

  }
  frmMain->AddFrame(exit_frame, new TGLayoutHints(kLHintsExpandX | kLHintsBottom, 0, 0, 0, 0));

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
  m_paramList[id].m_param->setValue(!m_paramList[id].m_param->getValue());

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
  const char* filetypes[] = {
    "PNG (bitmap)",    "*.png",
    "PDF (experimental!)",   "*.pdf",
    "All files",     "*",
    0,               0
  };
  TGFileInfo fi;
  fi.fFileTypes = filetypes;

  //deleting the pointer crashes, so I'm assuming this is magically cleaned up at some point
  new TGFileDialog(gEve->GetBrowser()->GetClient()->GetDefaultRoot(), gEve->GetBrowser(), kFDSave, &fi);
  if (!fi.fFilename)
    return; //cancelled
  TGLViewer* v = getViewPane()->getActiveGLViewer();
  bool success = false;
  if (!highres) {
    success = v->SavePicture(fi.fFilename);
  } else {
    char returnString[256];
    new TGInputDialog(gEve->GetBrowser()->GetClient()->GetDefaultRoot(), gEve->GetBrowser(),
                      "Bitmap width (pixels) [Note: Values larger than ~5000 may cause crashes.]",
                      "4000", //default
                      returnString);
    if (returnString[0] == '\0')
      return; //cancelled
    const TString t(returnString);
    if (!t.IsDigit()) {
      B2ERROR("Given width is not a number!");
      return;
    }
    const int width = t.Atoi();
    B2INFO("Saving bitmap (width: " << width << "px)..."); //may take a while
    success = v->SavePictureWidth(fi.fFilename, width, false); // don't scale pixel objects
  }

  if (success) {
    B2INFO("Saved image in: " << fi.fFilename)
  } else {
    new TGMsgBox(gEve->GetBrowser()->GetClient()->GetDefaultRoot(), gEve->GetBrowser(), "Saving image failed", TString::Format("Couldn't save to '%s'! Please verify you used an appropriate image file extension in the file name. Check console output for further information.", fi.fFilename));
  }

  //file dialog leaves empty box, redraw
  gEve->FullRedraw3D(false); //do not reset camera when redrawing
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
    projectionName.ReplaceAll("/", "");
    const int width = m_autoPictureWidth->GetIntNumber();
    glv->SavePictureWidth(TString::Format("%s_%s_%d.png", m_autoFileNamePrefix->GetText(), projectionName.Data(), i), width, false); // don't scale pixel objects
  }

  i++;
}

void DisplayUI::pollNewEvents()
{
  if (!AsyncWrapper::isAsync())
    return;


  int numEvents = AsyncWrapper::numAvailableEvents();
  bool state = m_nextButton->IsEnabled();
  //only call SetEnabled() if state changes (interrupts UI interactions otherwise)
  if (state != (numEvents > 0))
    m_nextButton->SetEnabled(numEvents > 0);
}

void DisplayUI::closeAndContinue()
{
  m_cumulative = false;

  if (!gEve)
    return;
  gEve->GetBrowser()->UnmapWindow();
  gEve->GetBrowser()->SendCloseMessage();
}

void DisplayUI::exit()
{
  closeAndContinue();

  //stop event processing after current event
  StoreObjPtr<EventMetaData> eventMetaData;
  eventMetaData->setEndOfData();

  if (AsyncWrapper::isAsync())
    AsyncWrapper::stopMainProcess();
}

void DisplayUI::showUserData(const DisplayData& displayData)
{
  static TGFileBrowser* fileBrowser = NULL;
  static std::map<std::string, BrowsableWrapper*> wrapperMap;
  for (auto & entry : wrapperMap) {
    //doesn't do anything
    //fileBrowser->RecursiveRemove(wrappers[i]);

    entry.second = NULL;
  }

  if (!displayData.m_histograms.empty()) {
    if (!fileBrowser) {
      gEve->GetBrowser()->StartEmbedding(0);
      fileBrowser = gEve->GetBrowser()->MakeFileBrowser();
      gEve->GetBrowser()->StopEmbedding("Histograms");


      //create new tab with canvas
      gEve->GetBrowser()->StartEmbedding(TRootBrowser::kRight);
      TEveWindowSlot* slot = TEveWindow::CreateWindowMainFrame();
      gEve->GetBrowser()->StopEmbedding();
      slot->StartEmbedding();
      new TCanvas;
      slot->StopEmbedding("Canvas");
    }

    //invert pad -> name map
    const std::map<TVirtualPad*, std::string>& padMap = BrowsableWrapper::getPads();
    std::map<std::string, TVirtualPad*> nameMap;
    for (const auto & entry : padMap) {
      nameMap[entry.second] = entry.first;
    }

    for (unsigned int i = 0; i < displayData.m_histograms.size(); i++) {
      std::string name(displayData.m_histograms.at(i)->GetName());
      if (!wrapperMap[name])
        wrapperMap[name] = new BrowsableWrapper(displayData.m_histograms.at(i));
      else
        wrapperMap[name]->setWrapped(displayData.m_histograms.at(i));
      BrowsableWrapper* wrapper = wrapperMap[name];

      if (nameMap.find(name) != nameMap.end()) {
        TVirtualPad* oldGpad = gPad;
        //redraw
        nameMap[name]->cd();
        wrapper->Browse(fileBrowser->Browser());

        //restore state
        oldGpad->cd();
      }
      fileBrowser->Add(wrapper);

    }
  }

  for (const auto & pair : displayData.m_selectedObjects) {
    //convert the name, index pair back into pointer
    StoreArray<TObject> array(pair.first);
    const TObject* obj = array[pair.second];
    if (obj) {
      select(array[pair.second]);
    } else {
      B2WARNING("Cannot select object " << pair.first << "[" << pair.second << "], not found. Is the array available?");
    }
  }

  /*
    TMacro* m = new TMacro;
    m->AddLine("{ TEveWindowSlot* currentWindow = gEve->GetWindowManager()->GetCurrentWindowAsSlot(); \
                  if (currentWindow) { \
                     std::cout << \" this is a slot!\\n\"; \
                     currentWindow->StartEmbedding(); \
                     TCanvas* c = new TCanvas; \
                     c->cd(); \
                     currentWindow->StopEmbedding(\"Canvas\"); \
                   } else { std::cout << \" not a slot!\\n\"; } \
                 }");
    m->SetName("Add canvas to active window");
    fileBrowser->Add(m);
    */
}

void DisplayUI::select(const TObject* object)
{
  TEveElement* elem = m_visualRepMap->getEveElement(object);
  if (elem and !gEve->GetSelection()->HasChild(elem)) {
    //select this object in addition to existing selection
    gEve->GetSelection()->UserPickedElement(elem, true);
  }
}
