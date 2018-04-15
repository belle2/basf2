/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Milkail Remnev, Dmitry Matvienko                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 ***************************************************************************/
//This module
#include <ecl/modules/eclDisplay/EclFrame.h>

//ROOT
#include <TFile.h>
#include <TCanvas.h>
#include <TSystem.h>
#include <TRootEmbeddedCanvas.h>
#include <TGDoubleSlider.h>
#include <TGNumberEntry.h>
#include <TGListTree.h>
#include <TGFileDialog.h>
#include <TStyle.h>
#include <TGLabel.h>
#include <TGComboBox.h>
#include <TGeoVolume.h>
#include <TGMenu.h>

//ECL
#include <ecl/modules/eclDisplay/geometry.h>
#include <ecl/utility/ECLChannelMapper.h>
#include <ecl/modules/eclDisplay/MultilineWidget.h>
#include <ecl/modules/eclDisplay/EclPainter.h>
#include <ecl/modules/eclDisplay/EclPainter1D.h>

using namespace Belle2;
using namespace ECL;
using namespace ECLDisplayUtility;

const char* EclFrame::filetypes[] = {
  "PDF",                        "*.pdf",
  "PostScript",                 "*.ps",
  "Encapsulated PostScript",    "*.eps",
  "SVG",                        "*.svg",
  "TeX",                        "*.tex",
  "GIF",                        "*.gif",
  "ROOT macros",                "*.C",
  "ROOT files",                 "*.root",
  "XML",                        "*.xml",
  "PNG",                        "*.png",
  "XPM",                        "*.xpm",
  "JPEG",                       "*.jpg",
  "TIFF",                       "*.tiff",
  "XCF",                        "*.xcf",
  0,                            0
};

EclFrame::EclFrame(int painter_type, EclData* data, bool auto_display, ECL::ECLChannelMapper* mapper)
{
  m_open = true;
  m_subsys = EclData::ALL;
  m_ecl_data = data;
  m_mapper = mapper;
  m_painter_type = (EclPainterType)painter_type;
  m_ecl_painter = EclPainterFactory::createPainter(m_painter_type, data,
                                                   m_mapper, m_subsys);
  m_last_event = -1;

  for (int i = 1; i <= data->getCrystalCount(); i++) {
    int phi_id = data->getPhiId(i);//GetPhiId(i);
    int theta_id = data->getThetaId(i);//GetThetaId(i);

    if (phi_id == -1 || theta_id == -1)
      data->excludeChannel(i);
  }

  initGUI(1000, 700);

  m_auto_display = auto_display;

  gStyle->SetOptStat(0);

  doDraw();
}

EclFrame::~EclFrame()
{
  // TODO: Fix this, right now using this for testing.
  Cleanup();
}

void EclFrame::initGUI(int w, int h)
{
  B2DEBUG(100, "EclFrame:: initializing GUI.");

  SetLayoutManager(new TGVerticalLayout(this));
  TGCompositeFrame* frame_container = new TGCompositeFrame(this, w, h, kHorizontalFrame);

  /* Menu bar */

  TGPopupMenu* menu_file = new TGPopupMenu(gClient->GetRoot());
  menu_file->AddEntry("&Open...", M_FILE_OPEN);
  menu_file->AddEntry("&Export TTree...", M_FILE_EXPORT_TREE);
  menu_file->AddEntry("&Save As...", M_FILE_SAVE);
  menu_file->AddSeparator();
  menu_file->AddEntry("&Exit", M_FILE_EXIT);
  TGPopupMenu* menu_view = new TGPopupMenu(gClient->GetRoot());
  menu_view->AddEntry("&Show event counts in histograms", M_VIEW_EVENTS);
  menu_view->AddEntry("&Show energy in histograms", M_VIEW_ENERGY);
  menu_view->AddSeparator();
  menu_view->AddEntry("&Show events from all ECL subsystems", M_VIEW_DET_FULL);
  menu_view->AddEntry("&Show events from ECL barrel", M_VIEW_DET_BARR);
  menu_view->AddEntry("&Show events from ECL forward endcap", M_VIEW_DET_FORW);
  menu_view->AddEntry("&Show events from ECL backward endcap", M_VIEW_DET_BACK);

  TGMenuBar* menubar = new TGMenuBar(this, w, 30);
  menubar->AddPopup("&File", menu_file, new TGLayoutHints(kLHintsTop | kLHintsLeft));
  menubar->AddPopup("&View", menu_view, new TGLayoutHints(kLHintsTop | kLHintsLeft));

  menu_file->Connect("Activated(Int_t)", "Belle2::EclFrame",
                     this, "handleMenu(Int_t)");
  menu_view->Connect("Activated(Int_t)", "Belle2::EclFrame",
                     this, "handleMenu(Int_t)");

  AddFrame(menubar, new TGLayoutHints(kLHintsExpandX | kLHintsTop, 0, 0, 1, 1));

  /* Settings */

  m_settings = new TGVerticalFrame(frame_container, w / 6, h);

  /* Zeroth subframe, diagram type selection */

  TGComboBox* diagram_type = new TGComboBox(m_settings, -1);
  diagram_type->SetName("DiagramType");
  const int types_count = EclPainterFactory::getTypeTitlesCount();
  const char** types_names = EclPainterFactory::getTypeTitles();
  for (int i = 0; i < types_count; i++)
    diagram_type->AddEntry(types_names[i], i);
  diagram_type->Select(m_painter_type);
  diagram_type->SetHeight(16);
  m_settings->AddFrame(diagram_type, new TGLayoutHints(kLHintsExpandX));
  diagram_type->Connect("Selected(Int_t)", "Belle2::EclFrame", this,
                        "changeType(Int_t)");

  /* First settings subframe, information */

  m_frame1 = new MultilineWidget(m_settings, "Info");
  m_frame1->setLineCount(4);

  m_settings->AddFrame(m_frame1, new TGLayoutHints(kLHintsExpandX));

  /* Second settings subframe, settings for range of events displayed */

  m_frame2 = new TGGroupFrame(m_settings, "Range of displayed events");
  m_frame2->SetLayoutManager(new TGVerticalLayout(m_frame2));

  TGHorizontalFrame* frame2_1 = new TGHorizontalFrame(m_frame2);
  TGLabel* ev_min_label = new TGLabel(frame2_1, "Min: ");
  frame2_1->AddFrame(ev_min_label, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 2, 2, 2, 2));
  m_events_min = new TGNumberEntry(frame2_1, 0, 6, -1, TGNumberFormat::kNESInteger);
  frame2_1->AddFrame(m_events_min, new TGLayoutHints(kLHintsLeft, 2, 2, 2, 2));
  TGLabel* ev_max_label = new TGLabel(frame2_1, "Max: ");
  frame2_1->AddFrame(ev_max_label, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 2, 2, 2, 2));
  m_events_max = new TGNumberEntry(frame2_1, 0, 6, -1, TGNumberFormat::kNESInteger);
  frame2_1->AddFrame(m_events_max, new TGLayoutHints(kLHintsLeft, 2, 2, 2, 2));
  m_frame2->AddFrame(frame2_1);

  m_ev_slider = new TGDoubleHSlider(m_frame2, w / 6, 2);
  m_frame2->AddFrame(m_ev_slider, new TGLayoutHints(kLHintsExpandX, 5, 5, 3, 4));

  TGHorizontalFrame* frame2_2 = new TGHorizontalFrame(m_frame2);
  TGTextButton* prev = new TGTextButton(frame2_2, "&Prev");
  prev->Connect("Clicked()", "Belle2::EclFrame", this, "showPrevEvents()");
  frame2_2->AddFrame(prev, new TGLayoutHints(kLHintsLeft, 5, 5, 3, 4));
  TGTextButton* next = new TGTextButton(frame2_2, "&Next");
  next->Connect("Clicked()", "Belle2::EclFrame", this, "showNextEvents()");
  frame2_2->AddFrame(next, new TGLayoutHints(kLHintsRight, 5, 5, 3, 4));
  m_frame2->AddFrame(frame2_2, new TGLayoutHints(kLHintsExpandX));

  m_settings->AddFrame(m_frame2, new TGLayoutHints(kLHintsExpandX, 2, 2, 2, 2));

  /* Third settings subframe */

  // Contains TGTreeList object which allows user to choose which channels
  // are going to be displayed.
  m_frame3 = new TGGroupFrame(m_settings, "Displayed channels");
  char temp[255];
  TGCanvas* list_canvas = new TGCanvas(m_frame3, 1, 100);
  m_list_tree = new TGListTree(list_canvas, kHorizontalFrame);
  m_list_tree->Associate(m_frame3);
  TGListTreeItem* root = m_list_tree->AddItem(0, "Detector");
  m_list_tree->OpenItem(root);
  for (int i = 0; i < 52; i++) {
    sprintf(temp, "Collector %d", i);
    TGListTreeItem* parent = m_list_tree->AddItem(root, temp);
    parent->SetUserData((void*)((intptr_t)i));
    for (int j = 0; j < 12; j++) {
      sprintf(temp, "Shaper %d", i * 12 + j);
      TGListTreeItem* item = m_list_tree->AddItem(parent, temp);
      item->SetUserData((void*)((intptr_t)j));
    }
  }

  m_list_tree->Connect("Clicked(TGListTreeItem*, Int_t)", "Belle2::EclFrame", this,
                       "changeRange(TGListTreeItem*, Int_t)");

  m_frame3->AddFrame(list_canvas, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
  m_settings->AddFrame(m_frame3, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 2, 2, 2, 2));

  /* Fourth settings subframe, channel exclusion */

  m_frame4 = new TGGroupFrame(m_settings, "Channel exclusion");
  m_frame4->SetLayoutManager(new TGHorizontalLayout(m_frame4));
  m_channel_id = new TGNumberEntry(m_frame4, 0, 6, -1, TGNumberFormat::kNESInteger);
  m_frame4->AddFrame(m_channel_id, new TGLayoutHints(kLHintsLeft, 2, 2, 2, 2));
  TGTextButton* exclude = new TGTextButton(m_frame4, "&Exclude");
  exclude->Connect("Clicked()", "Belle2::EclFrame", this, "excludeChannel()");
  m_frame4->AddFrame(exclude, new TGLayoutHints(kLHintsRight, 5, 5, 3, 4));

  m_settings->AddFrame(m_frame4, new TGLayoutHints(kLHintsExpandX, 2, 2, 2, 2));

  /* Fifth settings subframe, energy threshold */

  m_frame5 = new TGGroupFrame(m_settings, "Energy threshold");
  TGHorizontalFrame* frame5_1 = new TGHorizontalFrame(m_frame5);
  frame5_1->SetLayoutManager(new TGHorizontalLayout(frame5_1));
  m_min_en_threshold = new TGNumberEntry(frame5_1, 2.5, 6, -1);
  TGLabel* min_lab = new TGLabel(frame5_1, "MeV");
  m_max_en_threshold = new TGNumberEntry(frame5_1, 150, 6, -1);
  TGLabel* max_lab = new TGLabel(frame5_1, "MeV");
  frame5_1->AddFrame(m_min_en_threshold, new TGLayoutHints(kLHintsLeft, 2, 2, 2, 2));
  frame5_1->AddFrame(min_lab, new TGLayoutHints(kLHintsLeft, 2, 2, 2, 2));
  frame5_1->AddFrame(max_lab, new TGLayoutHints(kLHintsRight, 2, 2, 2, 2));
  frame5_1->AddFrame(m_max_en_threshold, new TGLayoutHints(kLHintsRight, 2, 2, 2, 2));

  m_frame5->AddFrame(frame5_1);
  m_threshold_switch = new TGCheckButton(m_frame5, "Enable energy threshold");
  m_threshold_switch->SetState(kButtonDown);
  m_frame5->AddFrame(m_threshold_switch);

  m_settings->AddFrame(m_frame5, new TGLayoutHints(kLHintsExpandX, 2, 2, 2, 2));

  /* Sixth settings subframe, "Draw" button */

  m_draw = new TGTextButton(m_settings, "&Draw");
  m_draw->Connect("Clicked()", "Belle2::EclFrame", this, "doDraw()");
  m_settings->AddFrame(m_draw, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));

  m_draw_all = new TGTextButton(m_settings, "&Draw All");
  m_draw_all->Connect("Clicked()", "Belle2::EclFrame", this, "doDrawAll()");
  m_settings->AddFrame(m_draw_all, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));

  /* Adding the entire subframe of settings */

  frame_container->AddFrame(m_settings);

  /* Canvas widget */

  m_ecanvas = new TRootEmbeddedCanvas("Ecanvas", frame_container, w / 2, h / 2);
  m_ecanvas->GetCanvas()->SetRightMargin(0.125);
  m_ecanvas->GetCanvas()->SetLeftMargin(0.1);
  frame_container->AddFrame(m_ecanvas, new TGLayoutHints(
                              kLHintsExpandX | kLHintsExpandY, 10, 10, 10, 1));

  m_ecanvas->GetCanvas()->
  Connect("TCanvas", "ProcessedEvent(Int_t, Int_t, Int_t, TObject*)",
          "Belle2::EclFrame", this, "updateInfo(Int_t, Int_t, Int_t, TObject*)");

  /* Adding frame container for settings and canvas to the main window */

  AddFrame(frame_container, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY,
                                              1, 1, 1, 1));

  B2DEBUG(100, "EclFrame:: GUI initialized.");

  frame_container->SetMinWidth(w / 2);
  // Add this if Root still throws BadDrawable errors.
  // frame_container->SetMinHeight(h);

  SetWindowName("ECL Data");
  MapSubwindows();
  Resize(w, h);
  MapWindow();

  B2DEBUG(100, "EclFrame:: Initializing data.");
  initData();
  B2DEBUG(100, "EclFrame:: Data initialized.");
}

void EclFrame::initData()
{
  m_events_min->SetLimits(TGNumberFormat::kNELLimitMinMax,
                          0, m_ecl_data->getLastEventId());
  m_events_max->SetLimits(TGNumberFormat::kNELLimitMinMax,
                          0, m_ecl_data->getLastEventId());

  B2DEBUG(500, "Last event id: " << m_ecl_data->getLastEventId());
  m_ev_slider->SetRange(0, m_ecl_data->getLastEventId());
  m_ev_slider->SetPosition(0, 0);
  m_ev_slider->Connect("TGDoubleHSlider", "PositionChanged()",
                       "Belle2::EclFrame", this, "updateEventRange()");
  updateEventRange();
}

void EclFrame::handleMenu(int id)
{
  static TString dir(".");
  TGFileInfo fi;
  TFile* file;

  switch (id) {
    case M_FILE_OPEN:
      fi.fFileTypes = 0;
      fi.fIniDir    = StrDup(dir);
      new TGFileDialog(gClient->GetRoot(), this, kFDOpen, &fi);
      if (fi.fFilename) {
        if (gSystem->AccessPathName(fi.fFilename, kFileExists) == 0) {
          B2DEBUG(50, "ECLFrame:: Opening file " << fi.fFilename);
          m_ecl_data->loadRootFile(fi.fFilename);
          loadNewData();
        }
      }
      // doDraw();
      break;
    case M_FILE_SAVE:
      fi.fFileTypes = filetypes;
      fi.fIniDir    = StrDup(dir);
      new TGFileDialog(gClient->GetRoot(), this, kFDSave, &fi);
      B2DEBUG(50, "Save file: " << fi.fFilename
              << "(dir: " << fi.fIniDir << ")");
      m_ecanvas->GetCanvas()->SaveAs(fi.fFilename);
      break;
    case M_FILE_EXPORT_TREE:
      static const char* filetypes_root[] = {"Root", "*.root", 0, 0};
      fi.fFileTypes = filetypes_root;
      fi.fIniDir    = StrDup(dir);
      new TGFileDialog(gClient->GetRoot(), this, kFDSave, &fi);
      B2DEBUG(50, "Save file: " << fi.fFilename
              << "(dir: " << fi.fIniDir << ")");
      file = new TFile(fi.fFilename, "RECREATE");
      m_ecl_data->getTree()->Write("tree");
      file->Close();
      break;
    case M_FILE_EXIT:
      CloseWindow();
      break;
    case M_VIEW_ENERGY:
      SetMode(1);
      changeType((EclPainterType)m_painter_type);
      break;
    case M_VIEW_EVENTS:
      SetMode(0);
      changeType((EclPainterType)m_painter_type);
      break;
    case M_VIEW_DET_FULL:
      m_subsys = EclData::ALL;
      m_ecl_painter->setDisplayedSubsystem(m_subsys);
      doDraw();
      break;
    case M_VIEW_DET_BARR:
      m_subsys = EclData::BARR;
      m_ecl_painter->setDisplayedSubsystem(m_subsys);
      doDraw();
      break;
    case M_VIEW_DET_FORW:
      m_subsys = EclData::FORW;
      m_ecl_painter->setDisplayedSubsystem(m_subsys);
      doDraw();
      break;
    case M_VIEW_DET_BACK:
      m_subsys = EclData::BACKW;
      m_ecl_painter->setDisplayedSubsystem(m_subsys);
      doDraw();
      break;
    default:
      break;
  }
}

void EclFrame::loadNewData()
{
  m_ev_slider->SetRange(0, m_ecl_data->getLastEventId());
  m_events_min->SetLimits(TGNumberFormat::kNELLimitMinMax,
                          0, m_ecl_data->getLastEventId());
  m_events_max->SetLimits(TGNumberFormat::kNELLimitMinMax,
                          0, m_ecl_data->getLastEventId());

  if (m_last_event < m_ecl_data->getLastEventId() &&
      (m_auto_display || m_last_event == -1)) {
    m_last_event++;
    if (m_last_event <= 1) {
      // Draw the first loaded event and update GUI.
      doDraw();
      m_settings->MapSubwindows();
      MapSubwindows();

      m_ev_slider->SetPosition(0, 0);
    }
    updateEventRange();
  }
}

void EclFrame::updateCanvas()
{
  TCanvas* fCanvas = m_ecanvas->GetCanvas();

  fCanvas->SetLogz();
  fCanvas->cd();
  m_ecl_painter->Draw();
  updateInfo(51, 0, 0, 0);
  fCanvas->Update();
}

void EclFrame::updateEventRange()
{
  Float_t ev_min, ev_max;
  m_ev_slider->GetPosition(&ev_min, &ev_max);

  m_events_min->SetNumber(ev_min);
  m_events_max->SetNumber(ev_max);

  m_ev_slider->MapWindow();
  m_ev_slider->MapSubwindows();
}

void EclFrame::showPrevEvents()
{
  Float_t ev_min, ev_max, diff;

  ev_min = m_events_min->GetNumber();
  ev_max = m_events_max->GetNumber();

  if (ev_min <= 0)
    return;

  diff = ev_max - ev_min + 1;
  ev_min -= diff;
  ev_max -= diff;
  if (ev_min < 0)
    ev_max = 0;

  m_ev_slider->SetPosition(ev_min, ev_max);
  updateEventRange();
  doDraw();
}

void EclFrame::showNextEvents()
{
  Float_t ev_min, ev_max, diff;

  ev_min = m_events_min->GetNumber();
  ev_max = m_events_max->GetNumber();

  if (ev_max >= m_ecl_data->getLastEventId())
    return;

  diff = ev_max - ev_min + 1;
  ev_min += diff;
  ev_max += diff;
  if (ev_max > m_ecl_data->getLastEventId())
    ev_max = m_ecl_data->getLastEventId();

  m_ev_slider->SetPosition(ev_min, ev_max);
  updateEventRange();
  doDraw();
}

void EclFrame::excludeChannel()
{
  int ch = m_channel_id->GetNumber();
  m_ecl_data->excludeChannel(ch, true);
  doDraw();
}

void EclFrame::doDraw()
{
  m_ecl_data->setEventRange(m_events_min->GetIntNumber(),
                            m_events_max->GetIntNumber());

  float en_min = m_min_en_threshold->GetNumber();
  float en_max = m_max_en_threshold->GetNumber();
  if (m_threshold_switch->GetState() == kButtonDown)
    m_ecl_data->setEnergyThreshold(en_min, en_max);
  else
    m_ecl_data->setEnergyThreshold(0, -1);

  updateCanvas();
}

void EclFrame::doDrawAll()
{
  m_ecl_data->setEventRange(0, m_ecl_data->getLastEventId());

  updateCanvas();
}

void EclFrame::updateInfo(int event, int px, int py, TObject*)
{
  if (event == 11) {
    EclPainter* new_painter = m_ecl_painter->handleClick(px, py);

    if (new_painter != NULL) {
      delete m_ecl_painter;
      m_ecl_painter = new_painter;
      doDraw();

      Layout();
      MapSubwindows();
    }
  }
  // On mouse move
  if (event == 51) {
    m_ecl_painter->getInformation(px, py, m_frame1);

    m_frame1->Layout();
  }
}

void EclFrame::changeRange(TGListTreeItem* entry, int)
{
  m_frame1->setLineCount(0);
  Layout();
  MapSubwindows();

  TGListTreeItem* parent = entry->GetParent();
  if (!parent) {
    // Root entry (detector) has been selected.
    changeType(PAINTER_COLLECTOR);
  } else {
    TGListTreeItem* grandparent = parent->GetParent();
    if (!grandparent) {
      // Crate entry had been selected.
      changeType(PAINTER_SHAPER, false);
      long crate = (long)entry->GetUserData();
      m_ecl_painter->setXRange(crate * 12, crate * 12 + 11);
      doDraw();
    } else {
      // Shaper entry had been selected.
      changeType(PAINTER_CHANNEL, false);
      long shaper = (long)entry->GetUserData();
      long crate  = (long)parent->GetUserData();
      shaper = 12 * crate + shaper;
      ((EclPainter1D*)m_ecl_painter)->setShaper(crate + 1, shaper + 1);
      doDraw();
    }
  }
}

void EclFrame::changeType(int type, bool redraw)
{
  EclPainter* new_painter =
    EclPainterFactory::createPainter((EclPainterType)type, m_ecl_data,
                                     m_mapper, m_subsys);

  if (new_painter) {
    m_painter_type = (EclPainterType)type;

    delete m_ecl_painter;
    m_ecl_painter = new_painter;
    m_frame1->setLineCount(0);
    m_ecanvas->GetCanvas()->Clear();

    updateInfo(51, 0, 0, 0);

    Layout();
    MapSubwindows();
    Layout();

    if (redraw)
      doDraw();
  }
}
