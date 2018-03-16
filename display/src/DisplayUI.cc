//needs to be first
#include <framework/core/ModuleParam.h>

#include <framework/dataobjects/DisplayData.h>
#include <display/DisplayUI.h>
#include <display/VisualRepMap.h>
#include <display/BrowsableWrapper.h>
#include <display/SplitGLView.h>
#include <display/InfoWidget.h>

#include <framework/core/InputController.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/logging/Logger.h>
#include <framework/pcore/AsyncWrapper.h>

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
#include <TGTab.h>
#include <TGTextEntry.h>
#include <TGLViewer.h>
#include <TGLWidget.h>
#include <TObject.h>
#include <TROOT.h>
#include <TSystem.h>

#include <sys/prctl.h>
#include <signal.h>

#include <cassert>


using namespace Belle2;

DisplayUI::DisplayUI(bool automatic):
  m_automatic(automatic)
{
  //ensure GUI thread goes away when parent dies. (root UI loves deadlocks)
  prctl(PR_SET_PDEATHSIG, SIGHUP);

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

  //Without this, our own menu bar entries are not drawn (might appear later)
  browser->Resize(TGDimension(1200, 1000));

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
  // periodically called by auto-advance timer, but we don't want to freeze UI if no events are there
  if (!m_nextButton->IsEnabled())
    return;
  goToEvent(m_currentEntry + 1);
}

void DisplayUI::prev()
{
  if (!m_prevButton->IsEnabled())
    return;
  if (!InputController::canControlInput())
    return;
  goToEvent(m_currentEntry - 1);
}

void DisplayUI::setTitle(const std::string& fileName)
{
  assert(gEve);
  std::string title("Belle II Event Display");
  if (!fileName.empty())
    title += " - " + fileName;

  TEveBrowser* browser = gEve->GetBrowser();
  browser->SetWindowName(title.c_str());
}
void DisplayUI::allowFlaggingEvents(const std::string& description)
{
  std::string label = "Flag this Event";
  if (!description.empty())
    label += " for " + description;

  m_flagEvent = new TGCheckButton(nullptr, label.c_str());
}

/** Return value for current event, only makes sense if allowFlaggingEvents()  was called. */
bool DisplayUI::getReturnValue() const
{
  return m_flagEvent && (m_flagEvent->GetState() == kButtonDown);
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
  if (m_flagEvent)
    m_flagEvent->SetState(kButtonUp);

  StoreObjPtr<EventMetaData> eventMetaData;
  m_eventLabel->SetTextColor(gROOT->GetColor(kBlack));
  if (!eventMetaData) {
    m_eventLabel->SetText("No EventMetaData object available.");
  } else {
    time_t secondsSinceEpoch = eventMetaData->getTime() / 1e9;
    //double subSecondPart = double(eventMetaData->getTime()) / 1e9 - secondsSinceEpoch;
    char date[30] = "<Invalid time>";
    if (secondsSinceEpoch == 0)
      strcpy(date, "");
    else if (auto gmt = gmtime(&secondsSinceEpoch))
      strftime(date, 30, "<%Y-%m-%d %H:%M:%S UTC>", gmt);
    m_eventLabel->SetText(TString::Format("Event: \t\t%u\nRun: \t\t%d\nExperiment: \t%d\n\n%s",
                                          eventMetaData->getEvent(),
                                          eventMetaData->getRun(), eventMetaData->getExperiment(),
                                          date));
  }
  m_eventLabel->Resize();

  gVirtualX->SetCursor(gEve->GetBrowser()->GetId(), gVirtualX->CreateCursor(kPointer));
}

void DisplayUI::goToEvent(Long_t id)
{
  if (id < 0)
    id = 0;
  const long numEntries = InputController::numEntries();
  if (numEntries > 0 and id >= numEntries)
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
  gVirtualX->SetCursor(gEve->GetBrowser()->GetId(), gVirtualX->CreateCursor(kWatch));
  m_nextButton->SetEnabled(false);
  m_prevButton->SetEnabled(false);


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
    gVirtualX->SetCursor(gEve->GetBrowser()->GetId(), gVirtualX->CreateCursor(kWatch));
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
    m_timer = nullptr;
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

  char returnString[256]; //magic length specified by TGInputDialog. Note that it still overwrites the stack if you paste something long enough.
  new TGInputDialog(gEve->GetBrowser()->GetClient()->GetDefaultRoot(), gEve->GetBrowser(),
                    "Jump to event '#evt/#run/#exp':",
                    TString::Format("%u/%d/%d", eventMetaData->getEvent(), eventMetaData->getRun(), eventMetaData->getExperiment()),
                    returnString);
  if (returnString[0] == '\0')
    return; //cancelled

  unsigned int event, run, exp;
  returnString[255] = '\0'; //I don't trust root to terminate the string correctly
  if (sscanf(returnString, "%u/%u/%u", &event, &run, &exp) != 3) {
    B2WARNING("Wrong format!");
    return;
  }
  goToEvent(event, run, exp);
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
  if (VisualRepMap::getInstance()->isCurrentlySelecting())
    return;

  //B2INFO("in selection handler: " << eveObj->GetElementName());

  const TObject* representedObject = VisualRepMap::getInstance()->getDataStoreObject(eveObj);
  if (representedObject)
    m_viewPane->getInfoWidget()->show(representedObject);

  VisualRepMap::getInstance()->selectOnly(eveObj);
  VisualRepMap::getInstance()->selectRelated(eveObj);
}

void DisplayUI::handleEvent(Event_t* event)
{
  if (event->fType == kGKeyPress) {
    //B2DEBUG(100, "event type " << event->fType << ", code: " << event->fCode << ", state: " << event->fState);
    switch (event->fCode) {
      case 117: //Page Down
        next();
        break;
      case 112: //Page Up
        prev();
        break;
      case 65: //Space bar
        togglePlayPause();
        break;
      case 47: // s
        saveHiResPicture();
        break;
      case 53: //Ctrl + q
        if (event->fState & kKeyControlMask)
          exit();
        break;
    }
  }
}

bool DisplayUI::startDisplay()
{
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
    gEve->GetSelection()->Connect("SelectionRepeated(TEveElement*)", "Belle2::DisplayUI", this, "selectionHandler(TEveElement*)");
  }

  updateUI(); //update button state
  m_viewPane->getInfoWidget()->update();

  m_eventData->AddElement(getViewPane()->getRPhiMgr()->ImportElements(gEve->GetEventScene()));
  m_eventData->AddElement(getViewPane()->getRhoZMgr()->ImportElements(gEve->GetEventScene()));

  for (std::string name : m_hideObjects) {
    TGListTreeItem* eventItem = gEve->GetListTree()->FindItemByPathname("Event");
    TGListTreeItem* item = gEve->GetListTree()->FindChildByName(eventItem, name.c_str());
    if (item) {
      B2INFO("hiding object '" << name << "'.");
      TEveElement* eveItem = static_cast<TEveElement*>(item->GetUserData());
      eveItem->SetRnrSelfChildren(false, false);
    } else {
      B2ERROR("hideObjects: '" << name << "' not found.");
    }
  }

  m_reshowCurrentEvent = false;
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

  browser->Connect("CloseWindow()", "Belle2::DisplayUI", this, "exit()");

  //add handler for keyboard events, needs to be done for browser TGFrame as well as frames of all TGLViewers
  browser->Connect("ProcessedEvent(Event_t*)", "Belle2::DisplayUI", this, "handleEvent(Event_t*)");
  TEveViewerList* viewers = gEve->GetViewers();
  TEveElement::List_ci end_it = viewers->EndChildren();
  for (TEveElement::List_i it = viewers->BeginChildren(); it != end_it; ++it) {
    TEveViewer* v = static_cast<TEveViewer*>(*it);
    TGLViewer* glv = v->GetGLViewer();
    glv->GetGLWidget()->Connect("ProcessedEvent(Event_t*)", "Belle2::DisplayUI", this, "handleEvent(Event_t*)");
  }

  //----------Event Control
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
      m_prevButton->SetToolTipText("Go to previous event (Page Up)");
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


      if (numEntries > 0) {
        TGLabel* maxEvents = new TGLabel(hf, TString::Format("/%ld", numEntries - 1));
        hf->AddFrame(maxEvents, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, margin, margin, margin, margin));
      }

      m_nextButton = new TGPictureButton(hf, gClient->GetPicture(icondir + "GoForward.gif"));
      m_nextButton->SetToolTipText("Go to next event (Page Down)");
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

    if (m_flagEvent) {
      TString descr = m_flagEvent->GetString();
      delete m_flagEvent;
      m_flagEvent = new TGCheckButton(event_frame, descr);
      m_flagEvent->SetToolTipText("Set return value to true for this event");
      m_flagEvent->SetState(kButtonUp);
      event_frame->AddFrame(m_flagEvent, new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, margin, margin, margin, margin));
    }
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
  viewer_frame->SetTitle("Current Viewer");
  {
    TGHorizontalFrame* hf = new TGHorizontalFrame(viewer_frame);
    TGButton* b = 0;
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

    hf = new TGHorizontalFrame(viewer_frame);
    {
      b = new TGTextButton(hf, "Dock/Undock Viewer");
      b->SetToolTipText("Move current viewer into it's own window, or back to its original position");
      hf->AddFrame(b, new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, margin, margin, margin, margin));
      b->Connect("Clicked()", "Belle2::DisplayUI", this, "toggleUndock()");
    }
    viewer_frame->AddFrame(hf, new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 0, 0, 0, 0));
  }
  frmMain->AddFrame(viewer_frame, new TGLayoutHints(kLHintsExpandX, 0, 0, 0, 0));


  TGGroupFrame* visOptionsFrame = new TGGroupFrame(frmMain);
  visOptionsFrame->SetTitle("Visualisation Options");
  {
    TGButton* b = 0;
    TGHorizontalFrame* hf = new TGHorizontalFrame(visOptionsFrame);
    {
      b = new TGTextButton(hf, "Dark/light colors");
      b->SetToolTipText("Toggle background color");
      hf->AddFrame(b, new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, margin, margin, margin, margin));
      b->Connect("Clicked()", "Belle2::DisplayUI", this, "toggleColorScheme()");
    }
    visOptionsFrame->AddFrame(hf, new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 0, 0, 0, 0));

    {
      TGCheckButton* c = new TGCheckButton(visOptionsFrame, "Cumulative mode (experimental)");
      c->SetToolTipText("Do not erase previous event, i.e. show data from multiple events. This is quite unstable and will crash sooner or later.");
      c->SetState(m_cumulative ? kButtonDown : kButtonUp);
      c->Connect("Clicked()", "Belle2::DisplayUI", this, "toggleCumulative()");
      visOptionsFrame->AddFrame(c, new TGLayoutHints(kLHintsExpandX | kLHintsCenterY, 0, margin, margin, margin));
    }
  }
  frmMain->AddFrame(visOptionsFrame, new TGLayoutHints(kLHintsExpandX, 0, 0, 0, 0));


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
      TGButton* b = new TGTextButton(hf, "     Exit     ");
      b->SetToolTipText("Close the display and stop basf2 after this event.");
      hf->AddFrame(b, new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, margin, margin, margin, margin));
      b->Connect("Clicked()", "Belle2::DisplayUI", this, "exit()");

    }
    exit_frame->AddFrame(hf, new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 0, 0, 0, 0));

  }
  frmMain->AddFrame(exit_frame, new TGLayoutHints(kLHintsExpandX | kLHintsBottom, 0, 0, 0, 0));

  //magic to prevent the frame being empty.
  frmMain->MapSubwindows();
  frmMain->Resize();
  frmMain->MapWindow();
  browser->StopEmbedding("Event Control");
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

void DisplayUI::toggleUndock()
{
  TGLViewer* activeGLviewer = getViewPane()->getActiveGLViewer();
  TEveViewerList* viewers = gEve->GetViewers();
  TEveElement::List_ci end_it = viewers->EndChildren();
  TEveViewer* activeViewer = nullptr;
  for (TEveElement::List_i it = viewers->BeginChildren(); it != end_it; ++it) {
    TEveViewer* v = static_cast<TEveViewer*>(*it);
    if (v->GetGLViewer() == activeGLviewer) {
      activeViewer = v;
      break;
    }
  }

  if (!activeViewer) {
    B2ERROR("No active viewer. Please select one by clicking on it.");
    return;
  }

  TEveCompositeFrameInMainFrame* packFrame = dynamic_cast<TEveCompositeFrameInMainFrame*>(activeViewer->GetEveFrame());
  if (!packFrame) {
    //we're docked
    activeViewer->UndockWindow();
  } else {
    //we're undocked
    packFrame->MainFrameClosed();
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
    B2INFO("Saved image in: " << fi.fFilename);
  } else {
    new TGMsgBox(gEve->GetBrowser()->GetClient()->GetDefaultRoot(), gEve->GetBrowser(), "Saving image failed",
                 TString::Format("Couldn't save to '%s'! Please verify you used an appropriate image file extension in the file name. Check console output for further information.",
                                 fi.fFilename));
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
    const bool scalePixelObjects = false;
    TString name = TString::Format("%s_%s_%d.png", m_autoFileNamePrefix->GetText(), projectionName.Data(), i);
    glv->SavePictureWidth(name, width, scalePixelObjects);
  }

  i++;
}

void DisplayUI::pollNewEvents()
{
  if (!AsyncWrapper::isAsync())
    return;
  if (!gEve)
    return;

  int numEvents = AsyncWrapper::numAvailableEvents();
  bool state = m_nextButton->IsEnabled();
  //only call SetEnabled() if state changes (interrupts UI interactions otherwise)
  if (state != (numEvents > 0))
    m_nextButton->SetEnabled(numEvents > 0);
}

void DisplayUI::exit()
{
  gSystem->ExitLoop();
  gROOT->SetInterrupt();
  m_cumulative = false;

  if (!gEve)
    return;

  // avoid emittting signals at end
  gEve->GetBrowser()->Disconnect();

  gEve->GetSelection()->Disconnect();
  gEve->GetHighlight()->Disconnect();

  gEve->GetBrowser()->UnmapWindow();
  gEve->GetBrowser()->SendCloseMessage();

  //stop event processing after current event
  StoreObjPtr<EventMetaData> eventMetaData;
  eventMetaData->setEndOfData();
}

void DisplayUI::showUserData(const DisplayData& displayData)
{
  static TGFileBrowser* fileBrowser = NULL;
  static std::map<std::string, BrowsableWrapper*> wrapperMap;
  for (auto& entry : wrapperMap) {
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
    for (const auto& entry : padMap) {
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

  for (const auto& pair : displayData.m_selectedObjects) {
    //convert the name, index pair back into pointer
    StoreArray<TObject> array(pair.first);
    const TObject* obj = array[pair.second];
    if (obj) {
      VisualRepMap::getInstance()->select(array[pair.second]);
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
