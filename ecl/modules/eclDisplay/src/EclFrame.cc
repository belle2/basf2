#include <ecl/modules/eclDisplay/EclFrame.h>
#include <TGeoVolume.h>
#include <TGComboBox.h>
#include <framework/logging/Logger.h>

// #define DEFAULT_PAINTER PAINTER_CHANNEL_2D

using namespace Belle2;

EclFrame::EclFrame(int mode, EclData* data, bool auto_display)
{
  m_ecl_data = data;
  m_painter_type = (EclPainterType)mode;
  m_ecl_painter = EclPainterFactory::CreatePainter(
                    m_painter_type, data);
  m_gl_view = false;
  m_last_event = -1;

  int phi_id, theta_id;
  for (int i = 0; i < 6912; i++) {
    phi_id = GetPhiId(i);
    theta_id = GetThetaId(i);

    if (phi_id == -1 || theta_id == -1)
      data->ExcludeChannel(i);
  }

//  InitGUI(700, 700, m_painter_type == PAINTER_3D);
  InitGUI(700, 700, m_painter_type == PAINTER_CHANNEL_2D);

  m_auto_display = auto_display;
  m_timer = new TTimer();
  m_timer->Connect("Timeout()", "Belle2::EclFrame", this, "LoadNewData()");
  m_timer->Start(10, kFALSE);

  DoDraw();
}

EclFrame::~EclFrame()
{
  Cleanup();
}

void EclFrame::InitGUI(int w, int h, bool gl_view)
{
  SetLayoutManager(new TGHorizontalLayout(this));

  m_settings = new TGVerticalFrame(this, w / 6, h);

  /* Ahem... Subframe number zero, diagram type selection */
  TGComboBox* diagram_type = new TGComboBox(m_settings, -1);
  diagram_type->SetName("DiagramType");
  const int types_count = EclPainterFactory::GetTypeTitlesCount();
  const char** types_names = EclPainterFactory::GetTypeTitles();
  for (int i = 0; i < types_count; i++)
    diagram_type->AddEntry(types_names[i], i);
  diagram_type->Select(m_painter_type);
  diagram_type->SetHeight(16);
  m_settings->AddFrame(diagram_type, new TGLayoutHints(kLHintsExpandX));
  diagram_type->Connect("Selected(Int_t)", "Belle2::EclFrame", this,
                        "ChangeType(Int_t)");

  /* First settings subframe, information */

  m_frame1 = new MultilineWidget(m_settings, "Info");

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
  prev->Connect("Clicked()", "Belle2::EclFrame", this, "ShowPrevEvents()");
  frame2_2->AddFrame(prev, new TGLayoutHints(kLHintsLeft, 5, 5, 3, 4));
  TGTextButton* next = new TGTextButton(frame2_2, "&Next");
  next->Connect("Clicked()", "Belle2::EclFrame", this, "ShowNextEvents()");
  frame2_2->AddFrame(next, new TGLayoutHints(kLHintsRight, 5, 5, 3, 4));
  m_frame2->AddFrame(frame2_2, new TGLayoutHints(kLHintsExpandX));

  m_settings->AddFrame(m_frame2, new TGLayoutHints(kLHintsExpandX, 2, 2, 2, 2));

  /* Third settings subframe */

  // Contains TGTreeList object which allows user to choose which channels
  // are going to be displayed.
  m_frame3 = new TGGroupFrame(m_settings, "Displayed channels");
  char temp[255];
  TGCanvas* list_canvas = new TGCanvas(m_frame3, 1, 150);
  m_list_tree = new TGListTree(list_canvas, kHorizontalFrame);
  m_list_tree->Associate(m_frame3);
  TGListTreeItem* root = m_list_tree->AddItem(0, "Detector");
  m_list_tree->OpenItem(root);
  for (int i = 0; i < 36; i++) {
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
                       "ChangeRange(TGListTreeItem*, Int_t)");

  m_frame3->AddFrame(list_canvas, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
  m_settings->AddFrame(m_frame3, new TGLayoutHints(kLHintsExpandX, 2, 2, 2, 2));

  /* Fourth settings subframe, channel exclusion */

  m_frame4 = new TGGroupFrame(m_settings, "Channel exclusion");
  m_frame4->SetLayoutManager(new TGHorizontalLayout(m_frame4));
  m_channel_id = new TGNumberEntry(m_frame4, 0, 6, -1, TGNumberFormat::kNESInteger);
  m_frame4->AddFrame(m_channel_id, new TGLayoutHints(kLHintsLeft, 2, 2, 2, 2));
  TGTextButton* exclude = new TGTextButton(m_frame4, "&Exclude");
  exclude->Connect("Clicked()", "Belle2::EclFrame", this, "ExcludeChannel()");
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
  m_draw->Connect("Clicked()", "Belle2::EclFrame", this, "DoDraw()");
  m_settings->AddFrame(m_draw, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));

  /* Adding the entire subframe of settings */

  AddFrame(m_settings);

  /* Canvas widget */

  m_ecanvas = new TRootEmbeddedCanvas("Ecanvas", this, w, h);
  // m_ecanvas->ReparentWindow(this);
  m_ecanvas->GetCanvas()->SetRightMargin(0.125);
  m_ecanvas->GetCanvas()->SetLeftMargin(0.1);
  AddFrame(m_ecanvas, new TGLayoutHints(
             kLHintsExpandX | kLHintsExpandY, 10, 10, 10, 1));

  m_ecanvas->GetCanvas()->
  Connect("TCanvas", "ProcessedEvent(Int_t, Int_t, Int_t, TObject*)",
          "Belle2::EclFrame", this, "UpdateInfo(Int_t, Int_t, Int_t, TObject*)");

  SetGLViewMode(gl_view);

  SetWindowName("ECL Data");
  MapSubwindows();
  Resize(GetDefaultSize());
  MapWindow();

  InitData();
}

void EclFrame::InitData()
{
  m_events_min->SetLimits(TGNumberFormat::kNELLimitMinMax,
                          0, m_ecl_data->GetLastEventId());
  m_events_max->SetLimits(TGNumberFormat::kNELLimitMinMax,
                          0, m_ecl_data->GetLastEventId());

  m_ev_slider->SetRange(0, m_ecl_data->GetLastEventId());
  m_ev_slider->SetPosition(0, 0);
  m_ev_slider->Connect("TGDoubleHSlider", "PositionChanged()",
                       "Belle2::EclFrame", this, "UpdateEventRange()");
  UpdateEventRange();
}

void EclFrame::MapSubwindows()
{
  TGMainFrame::MapSubwindows();
  if (m_gl_view)
    HideFrame(m_ecanvas);
  switch (m_painter_type) {
    case PAINTER_CHANNEL:
    case PAINTER_SHAPER:
    case PAINTER_COLLECTOR:
      break;
    default:
      m_settings->HideFrame(m_frame3);
      break;
  }
}


void EclFrame::SetGLViewMode(bool mode, bool request_update)
{
  if (!request_update && mode == m_gl_view)
    return;
//  if (mode) {
//    m_gl_view = true;

//    m_glviewer = new TGLSAViewer(this, gPad);
//    m_glviewer->SetResetCamerasOnUpdate(kFALSE);
//    AddFrame(m_glviewer->GetFrame(), new TGLayoutHints(
//               kLHintsExpandX | kLHintsExpandY, 10, 10, 10, 1));
//    gPad->SetViewer3D(m_glviewer);
//    m_glviewer->
//    Connect("TGLSAViewer", "MouseOver(TObject*, UInt_t)",
//            "Belle2::EclFrame", this, "UpdateInfo3D(TObject*, UInt_t)");

//    MapSubwindows();
//  } else {
//    m_gl_view = false;

//    MapSubwindows();

//    RemoveFrame(m_glviewer->GetFrame());
//    m_glviewer->GetFrame()->ReparentWindow(gClient->GetRoot());
//    delete m_glviewer;
//  }
}

void EclFrame::LoadNewData()
{
  if (m_ecl_data->HasExpanded()) {
    m_ev_slider->SetRange(0, m_ecl_data->GetLastEventId());
    m_events_min->SetLimits(TGNumberFormat::kNELLimitMinMax,
                            0, m_ecl_data->GetLastEventId());
    m_events_max->SetLimits(TGNumberFormat::kNELLimitMinMax,
                            0, m_ecl_data->GetLastEventId());

    //printf("Event %d loaded\n", m_ecl_data->GetLastEventId());
    B2DEBUG(50, "New events loaded");
  }

  if (m_last_event < m_ecl_data->GetLastEventId() &&
      (m_auto_display || m_last_event == -1)) {
    m_last_event++;
    m_ev_slider->SetPosition(m_last_event,
                             m_last_event);
    UpdateEventRange();
    DoDraw();
  }
}

void EclFrame::UpdateEventRange()
{
  Float_t ev_min, ev_max;
  m_ev_slider->GetPosition(&ev_min, &ev_max);

  m_events_min->SetNumber(ev_min);
  m_events_max->SetNumber(ev_max);
}

void EclFrame::ShowPrevEvents()
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
  UpdateEventRange();
  DoDraw();
}

void EclFrame::ShowNextEvents()
{
  Float_t ev_min, ev_max, diff;

  ev_min = m_events_min->GetNumber();
  ev_max = m_events_max->GetNumber();

  if (ev_max >= m_ecl_data->GetLastEventId())
    return;

  diff = ev_max - ev_min + 1;
  ev_min += diff;
  ev_max += diff;
  if (ev_max > m_ecl_data->GetLastEventId())
    ev_max = m_ecl_data->GetLastEventId();

  m_ev_slider->SetPosition(ev_min, ev_max);
  UpdateEventRange();
  DoDraw();
}

void EclFrame::ExcludeChannel()
{
  int ch = m_channel_id->GetNumber();
  m_ecl_data->ExcludeChannel(ch, true);
  DoDraw();
}

void EclFrame::DoDraw()
{
  m_ecl_data->SetEventRange(m_events_min->GetIntNumber(),
                            m_events_max->GetIntNumber());

  float en_min = m_min_en_threshold->GetNumber();
  float en_max = m_max_en_threshold->GetNumber();
  if (m_threshold_switch->GetState() == kButtonDown)
    m_ecl_data->SetEnergyThreshold(en_min, en_max);
  else
    m_ecl_data->SetEnergyThreshold(0, -1);

  TCanvas* fCanvas = m_ecanvas->GetCanvas();
  //if (GetMode())
  fCanvas->SetLogz();
  fCanvas->cd();
  m_ecl_painter->Draw();
  UpdateInfo(51, 0, 0, 0);
  fCanvas->Update();
}

void EclFrame::UpdateInfo(int event, int px, int py, TObject*)
{
  // On mouse click
  if (event == 1) {

  }
  // On mouse release
  if (event == 11) {
    EclPainter* new_painter = m_ecl_painter->HandleClick(px, py);

    if (new_painter != NULL) {
      delete m_ecl_painter;
      m_ecl_painter = new_painter;
      DoDraw();

      Layout();
      MapSubwindows();
    }
  }
  // On mouse move
  if (event == 51) {
    m_ecl_painter->GetInformation(px, py, m_frame1);

    Layout();
    MapSubwindows();
  }
}


void EclFrame::UpdateInfo3D(TObject* obj, UInt_t)
{
  if (obj) {
    TGeoVolume* seg = (TGeoVolume*)obj;
    UpdateInfo(51, seg->GetNumber(), 0, 0);
  }
}


void EclFrame::ChangeRange(TGListTreeItem* entry, int)
{
  SetGLViewMode(false);

  m_frame1->SetLineCount(0);
  Layout();
  MapSubwindows();

  TGListTreeItem* parent = entry->GetParent();
  if (!parent) {
    // Root entry (detector) has been selected.
    ChangeType(PAINTER_COLLECTOR);
  } else {
    TGListTreeItem* grandparent = parent->GetParent();
    if (!grandparent) {
      // Crate entry had been selected.
      ChangeType(PAINTER_SHAPER, false);
//      int crate = (int)entry->GetUserData();
//      m_ecl_painter->SetXRange(crate * 12, crate * 12 + 11);
      DoDraw();
    } else {
      // Shaper entry had been selected.
      ChangeType(PAINTER_CHANNEL, false);
//      int shaper = (int)entry->GetUserData();
//      int crate  = (int)parent->GetUserData();
//      shaper = 12 * crate + shaper;
//      m_ecl_painter->SetXRange(shaper * 16, shaper * 16 + 15);
      DoDraw();
    }
  }
}

void EclFrame::ChangeType(int type, bool redraw)
{
  EclPainter* new_painter =
    EclPainterFactory::CreatePainter((EclPainterType)type, m_ecl_data);

  if (new_painter) {
    m_painter_type = (EclPainterType)type;
// SetGLViewMode(m_painter_type == PAINTER_3D);

    SetGLViewMode(m_painter_type == PAINTER_CHANNEL_2D);

    delete m_ecl_painter;
    m_ecl_painter = new_painter;
    m_frame1->SetLineCount(0);
    m_ecanvas->GetCanvas()->Clear();

    MapSubwindows();

    if (redraw)
      DoDraw();
  }
}
