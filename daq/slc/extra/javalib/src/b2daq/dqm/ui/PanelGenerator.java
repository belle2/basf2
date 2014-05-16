package b2daq.dqm.ui;

import java.awt.Component;
import java.util.ArrayList;

import javax.swing.JFrame;
import javax.swing.UIManager;

import b2daq.dqm.core.Histo1F;
import b2daq.dqm.core.MonObject;
import b2daq.dqm.core.TimedGraph1F;
import b2daq.dqm.graphics.GEnumLabel;
import b2daq.dqm.graphics.GHisto;
import b2daq.dqm.graphics.GMonObject;
import b2daq.dqm.graphics.GRCStateLabel;
import b2daq.dqm.property.CanvasProperty;
import b2daq.dqm.property.GHistoProperty;
import b2daq.dqm.property.GTextProperty;
import b2daq.graphics.GText;
import b2daq.io.ConfigFile;
import b2daq.monitor.io.NSMDataManager;
import b2daq.monitor.io.UpdatableMessage;
import b2daq.ui.UTabPanel;
import b2daq.ui.UTablePanel;
import b2daq.ui.Updatable;

public class PanelGenerator {
	
	private ConfigFile _config;
	private NSMDataManager _manager;
	private ArrayList<Updatable> _panel_v = new ArrayList<Updatable>();
	private GHistoProperty _histo_pro;
	private GTextProperty _text_pro;
	private String _title = "";
	
	public PanelGenerator(String configname) {
		_config = new ConfigFile(configname);
		_histo_pro = new GHistoProperty(_config);
		_text_pro = new GTextProperty(_config);
		_manager = new NSMDataManager();
	}
	
	public Component create() {
		setTitle(_config.getString("rootPanel.title"));
		return create("rootPanel");
	}
	
	public Component create(String label) {
		if (!_config.hasKey(label)) return null;
		String [] str_v = _config.getString(label).split("\\(");
		if (str_v.length > 0) {
			String type = str_v[0];
			if (type.matches("TablePanel")) {
				str_v = str_v[1].replace(")", "").split(",");
				int cols = 1, rows = 1;
				if (str_v.length > 1) {
					cols = Integer.parseInt(str_v[0]);
					rows = Integer.parseInt(str_v[1]);
				}
				UTablePanel panel = new UTablePanel(cols, rows);
				str_v = _config.getString(label+".cels").split(",");
				if (_config.hasKey(label+".cels")) {
					for (String str : str_v) {
						panel.add(create(str));
					}
				}
				if (_config.hasKey(label+".wratio")) {
					panel.setWidthRatio(_config.getString(label+".wratio"));
				}
				if (_config.hasKey(label+".hratio")) {
					panel.setHeightRatio(_config.getString(label+".hratio"));
				}
				return panel;
			} else if (type.matches("TabbedPanel")) {
				UTabPanel panel = new UTabPanel();
				if (_config.hasKey(label+".tabs")) {
					str_v = _config.getString(label+".tabs").split(",");
					for (String str : str_v) {
						if (_config.hasKey(str+".label")) {
							panel.addTab(_config.getString(str+".label"), create(str));
						} else {
							panel.addTab(str, create(str));
						}
					}
				}
				_panel_v.add(panel);
				return panel;
			} else if (type.matches("CanvasPanel")) {
				CanvasPanel canvas = new CanvasPanel();
				canvas.getCanvas().setUseStat(false);
				if (_config.hasKey(label+".objects")) {
					str_v = _config.getString(label+".objects").split(",");
					for (String name : str_v) {
						createObject(canvas, name);
					}
				}
				CanvasProperty cpro = new CanvasProperty(_config);
				cpro.put(canvas.getCanvas(), label);
				_panel_v.add(canvas);
				return canvas;
			} else if (type.matches("HTMLPanel")) {
				int font_size = 12;
				try {
					String str = str_v[1].replace(")", "");
					font_size = Integer.parseInt(str);
				} catch (Exception e) {
					e.printStackTrace();
				}
				MessageBoxPanel panel = new MessageBoxPanel(font_size);
				if (_config.hasKey(label+".title")) {
					panel.add(new UpdatableMessage(_config.getString(label+".title")));
				}
				panel.setHeader(_config.getString(label+".header"));
				panel.setFooter(_config.getString(label+".footer"));
				if (_config.hasKey(label+".sources")) {
					str_v = _config.getString(label+".sources").split(",");
					for (String str : str_v) {
						String title = _config.getString(str+".title");
						String format = _config.getString(str+".format");
						String dataname = _config.getString(str+".data");
						UpdatableMessage message = new UpdatableMessage(title);
						if (dataname.length() > 0) {
							_manager.addMessage(message, dataname, format);
						}
						panel.add(message);
					}
				}
				_panel_v.add(panel);
				return panel;
			}
		}
		_manager.update();
		return null;
	}
	
	public NSMDataManager getManager() {
		return _manager;
	}
	
	public void update() {
		_manager.update();
		for (Updatable panel : _panel_v) {
			panel.update();
		}
	}

	public GMonObject createObject(CanvasPanel canvas, String name) {
		String str = _config.getString(name);
		String dataname = _config.getString(name+".data");
		GMonObject gobj = null;
		if (str.startsWith("TimedGraph")) {
			str = str.replace("TimedGraph(", "").replace(")", "");
			String [] s_v = str.split(",");
			if (s_v.length > 1) {
				int npoints = Integer.parseInt(s_v[0]) * 2;
				int time_max = Integer.parseInt(s_v[1]);
				String title = _config.getString(name+".title");
				MonObject obj = new TimedGraph1F(name, title, npoints, 0, time_max);
				gobj = canvas.getCanvas().addHisto(obj);
				_histo_pro.put((GHisto)gobj, name);
				_manager.addHisto(gobj, dataname);
				canvas.getCanvas().resetPadding();
			}
		} else if (str.startsWith("Hitogram1D")) {
			str = str.replace("Hitogram1D(", "").replace(")", "");
			String [] s_v = str.split(",");
			if (s_v.length > 2) {
				int nbins = Integer.parseInt(s_v[0]);
				double xmin = Double.parseDouble(s_v[1]);
				double xmax = Double.parseDouble(s_v[2]);
				String title = _config.getString(name+".title");
				MonObject obj = new Histo1F(name, title, nbins, xmin, xmax);
				gobj = canvas.getCanvas().addHisto(obj);
				_histo_pro.put(gobj, name);
				_manager.addHisto((GHisto)gobj, dataname);
				canvas.getCanvas().resetPadding();
			}
		} else if (str.startsWith("MonitoredLabel")) {
			str = str.replace("MonitoredLabel(", "").replace(")", "");
			String [] s_v = str.split(",");
			if (s_v.length > 3) {
				double x = Double.parseDouble(s_v[0]);
				double y = Double.parseDouble(s_v[1]);
				double w = Double.parseDouble(s_v[2]);
				double h = Double.parseDouble(s_v[3]);
				GEnumLabel label = null;
				if (dataname.startsWith("RunState")) {
					label = new GRCStateLabel();
				} else {
					label = new GEnumLabel();
				}
				label.setBounds(x, y, w, h);
				canvas.getCanvas().addShape(label);
				_text_pro.put(label.getText(), name);
				String format = _config.getString(name+".format");
				_manager.addShape(label, dataname, format);
			}
		} else if (str.startsWith("Label")) {
			str = str.replace("Label(", "").replace(")", "");
			String [] s_v = str.split(",");
			System.out.println(str);
			GText label;
			if (s_v.length > 2) {
				String text = s_v[0];
				double x = Double.parseDouble(s_v[1]);
				double y = Double.parseDouble(s_v[2]);
				label = new GText(text.replace("\"", ""), x, y, "");
			} else if (s_v.length > 1) {
				double x = Double.parseDouble(s_v[0]);
				double y = Double.parseDouble(s_v[1]);
				label = new GText("", x, y, "");
			} else {
				label = new GText();
			}
			label.setAligment("middle");
			canvas.getCanvas().addShape(label);
			_text_pro.put(label, name);
		}
		return gobj;
	}
	
	static public void main(String [] argv) throws Exception {
		UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
		JFrame frame = new JFrame();
		frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		frame.setSize(1040, 480);
		frame.setLocationRelativeTo(null);
		PanelGenerator gen = new PanelGenerator("/home/tkonno/storage_env.conf");
		frame.add(gen.create());
		frame.setVisible(true);
	}

	public String getTitle() {
		return _title;
	}

	public void setTitle(String title) {
		_title = title;
	}

}
