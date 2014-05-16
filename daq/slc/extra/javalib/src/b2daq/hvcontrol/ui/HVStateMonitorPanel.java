package b2daq.hvcontrol.ui;

import javax.swing.border.EmptyBorder;

import b2daq.nsm.NSMData;
import b2daq.database.ConfigObject;
import b2daq.dqm.core.Histo2I;
import b2daq.dqm.graphics.GColorAxis;
import b2daq.dqm.graphics.GHisto2;
import b2daq.dqm.graphics.GValuedText;
import b2daq.dqm.ui.CanvasPanel;
import b2daq.graphics.HtmlColor;

public class HVStateMonitorPanel extends CanvasPanel {

  private static final long serialVersionUID = 1L;

  private GHisto2 _histo = null;
  private HtmlColor[] _pattern = { HtmlColor.LIGHT_GRAY,
		  								HtmlColor.CYAN,
		                            HtmlColor.MAGENTA,
                                   HtmlColor.BLUE,
                                   HtmlColor.DARK_GREEN,
                                   HtmlColor.RED };

  private GValuedText[] _text_x_v = { new GValuedText("0", 0, "center top"),
                                      new GValuedText("3", 3, "center top"),
                                      new GValuedText("6", 6, "center top"),
                                      new GValuedText("9", 9, "center top"),
                                      new GValuedText("12", 12, "center top"),
                                      new GValuedText("15", 15, "center top") };
  private GValuedText[] _text_y_v = { new GValuedText("22", 2, "center top"),
                                      new GValuedText("20", 4, "center top"),
                                      new GValuedText("18", 6, "center top"),
                                      new GValuedText("16", 8, "center top"),
                                      new GValuedText("14", 10, "center top"),
                                      new GValuedText("12", 12, "center top"),
                                      new GValuedText("10", 14, "center top"),
                                      new GValuedText("8", 16, "center top"),
                                      new GValuedText("6", 18, "center top"),
                                      new GValuedText("4", 20, "center top"),
                                      new GValuedText("2", 22, "center top"),
                                      new GValuedText("0", 24, "center top") };

  public HVStateMonitorPanel(String title) {
    this(title, 400, 480);
  }

  public HVStateMonitorPanel(String title, int width, int height) {
    setSize(width, height);
    setBorder(new EmptyBorder(0, 0, 0, 0));
    getCanvas().setUseStat(false);
    _histo = (GHisto2) getCanvas().addHisto(
             new Histo2I("histo", title + ";Slot;Channel", 16, 0, 16, 24, 0,24));
    _histo.setLineColor(HtmlColor.WHITE);
    _histo.setLineWidth(3);
    _histo.getHisto().setMinimum(1);
    _histo.getHisto().setMaximum(_pattern.length);
    getCanvas().getPad().setX(0.16);
    getCanvas().getPad().setWidth(0.75);
    getCanvas().getPad().setY(0.09);
    getCanvas().getPad().setHeight(0.83);
    getCanvas().resetPadding();
    ((GColorAxis) (getCanvas().getColorAxis())).setColorPattern(_pattern);
    getCanvas().getColorAxis().get().setMin(1);
    getCanvas().getColorAxis().get().setMax(_pattern.length);
    getCanvas().getColorAxis().get().fixMin(true);
    getCanvas().getColorAxis().get().fixMax(true);
    getCanvas().getColorAxis().setVisible(false);
    getCanvas().getAxisX().setLabels(_text_x_v);
    getCanvas().getAxisY().setLabels(_text_y_v);
    getCanvas().setFillColor(new HtmlColor(237, 237, 237));
    for (int nx = 0; nx < 16; nx++) {
        for (int ny = 0; ny < 24; ny++) {
        	_histo.get().setBinContent(nx, ny, 1);
        }
    	
    }
  }

  public void setState(int nslot, int nchannel, int state) {
    _histo.getHisto().setBinContent(nslot, 23 - nchannel, state);
  }

  public void update(ConfigObject obj, NSMData data) {
	  if (!obj.hasObject("channel")) return;
	  for (ConfigObject cobj : obj.getObjects("channel")) {
		  int state = 1;
		  int bstate = data.getObject("channel_status", cobj.getIndex()).getInt("state");
		  switch (bstate) {
		  case 2: state = 1; break;
		  case 11:
		  case 12:
		  case 13: state = 4; break;
		  case 14: state = 5; break;
		  case 31: 
		  case 32: state = 3; break;
		  case 33: state = 2; break;
		  case 41:
		  case 42:
		  case 43:
		  case 44: state = 6; break;
		  }
		  setState(cobj.getInt("slot"), cobj.getInt("channel"), state);
	  }
	  update();
  }

}
