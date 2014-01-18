package b2dqm.java.ui;

import java.awt.Component;
import java.awt.Dimension;
import java.awt.Toolkit;
import java.io.InputStream;
import java.util.ArrayList;

import javax.swing.BoxLayout;
import javax.swing.JFrame;
import javax.swing.JPanel;
import javax.swing.JComponent;

import b2dqm.core.HistoPackage;
import b2dqm.core.PackageInfo;
import b2dqm.java.Belle2DQMBrowser;

public class DQMMainFrame extends JFrame {
	
	private static DQMMainFrame __main = null;
	
	private static final long serialVersionUID = 1L;
	private DQMMainPanel _main_panel = new DQMMainPanel();
	private DQMSidePanel _side_panel = new DQMSidePanel();
	private ArrayList<JFrame> _child_frame_v = new ArrayList<JFrame>(); 
	
	private JPanel _panel = new JPanel();
	private boolean _is_separated = false;
	
	public static DQMMainFrame get() {
		return __main;
	}
	
	public DQMMainFrame() {
		setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		__main = this;
	}
	
	public void dispose() {
		for ( JFrame frame : _child_frame_v ) {
			frame.dispose();
		}
		_child_frame_v.clear();
		_child_frame_v = new ArrayList<JFrame>();
		super.dispose();
	}
	
	public void init(ArrayList<HistoPackage> pack_v, 
			ArrayList<InputStream> istream_v, 
			ArrayList<PackageInfo> info_v) {
		_panel.setLayout(new BoxLayout(_panel,BoxLayout.X_AXIS));
		add(_panel);
		_main_panel.initPanels(pack_v, _side_panel, istream_v);
		_side_panel.init(pack_v);
		_panel.add(_side_panel);
		_panel.add(_main_panel);
		setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		_main_panel.setPreferredSize(new Dimension(1000, 720));
		_side_panel.setPreferredSize(new Dimension(300, 720));
		_side_panel.setMaximumSize(new Dimension(300, 720));
		setSize(1240, 720);
		setTitle("Belle-II DQM browser version " + Belle2DQMBrowser.VERSION);
		setJMenuBar(new DQMMenuBar(this));
		setLocationRelativeTo(null);
		setIconImage(Toolkit.getDefaultToolkit().createImage(getClass().getResource("monitor.png")));
		setVisible(true);
	}
	
	public boolean openSidePanel() {
		if ( _side_panel.isVisible() ) {
			_side_panel.setVisible(false);
		} else {
			_side_panel.setVisible(true);
		}
		return _side_panel.isVisible();
	}
	
	public boolean isSeparated() {
		return _is_separated;
	}
	
	public ArrayList<JFrame> getChildFrames() {
		return _child_frame_v;	
	}
	
	public boolean separatePanels() {
		if ( !_is_separated ) {
			Component [] com_v = _main_panel.getComponents();
			ArrayList<HistoPackage> pack_v = _main_panel.getPackages();
			JFrame frame_tmp = this;
			setSize(240, 720);
			for ( int n = 0; n < com_v.length && n < pack_v.size(); n++ ) {
				JFrame frame = new JFrame();
				frame.setTitle(((JComponent)com_v[n]).getName());
				frame.add(com_v[n]);
				frame.setSize(1000, 720);
				frame.setIconImage(Toolkit.getDefaultToolkit().createImage(getClass().getResource("monitor.png")));
				frame.setLocationRelativeTo(frame_tmp);
				frame_tmp = frame;
				frame.setVisible(true);
				_child_frame_v.add(frame);
			}
			_main_panel.setVisible(false);
			_is_separated = true;
			return true;
		} else {
			for ( JFrame frame : _child_frame_v ) {
				frame.dispose();
				frame.getComponent(0).setName(frame.getTitle());
				_main_panel.addTab(frame.getTitle(), frame.getComponent(0));
			}
			_child_frame_v.clear();
			_child_frame_v = new ArrayList<JFrame>();
			_main_panel.setVisible(true);
			setSize(1240, 720);
			_is_separated = false;
			return false;
		}
	}
	
	public void update() {
		_main_panel.update();
		_side_panel.update();
		for ( JFrame frame : _child_frame_v ) {
			frame.repaint();
		}
	}
	
	
	
}
