package b2rc.java.ui;

import java.awt.Font;
import java.awt.Toolkit;
import java.util.Enumeration;

import javax.swing.JFrame;
import javax.swing.UIManager;

import b2daq.java.ui.DnDTabbedPane;
import b2daq.ui.Updatable;
import b2rc.core.RCNodeSystem;
import b2rc.db.RCDBManager;
import b2rc.java.Belle2RunController;
import b2rc.xml.XMLRCNodeLoader;

public class RCMainFrame extends JFrame implements Updatable {
	
	private static RCMainFrame __main = null;
	
	private static final long serialVersionUID = 1L;
	
	public static RCMainFrame get() {
		return __main;
	}

	private RCNodeSystem _system;

	private ControlMainPanel _control_panel;

	private EditorMainPanel _editor_panel;
	
	public RCMainFrame(RCNodeSystem system) {
		_system = system;
		setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		__main = this;
	}
	
	public void dispose() {
		super.dispose();
	}
	
	public void init() {
		try {
			UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
		} catch (Exception e) {
			e.printStackTrace();
		}
		setSize(820, 720);
		setTitle("Belle-II Run controller version " + Belle2RunController.VERSION);
		setLocationRelativeTo(null);
		setIconImage(Toolkit.getDefaultToolkit().createImage(getClass().getResource("runcontrol.png")));

		DnDTabbedPane main_tab = new DnDTabbedPane();
		_editor_panel = new EditorMainPanel(_system);
		_control_panel = new ControlMainPanel(_editor_panel, _system);
		main_tab.addTab("Control", _control_panel);
		main_tab.addTab("Editor", _editor_panel);
		add(main_tab);
		_editor_panel.init();
		setVisible(true);
	}
	
	public void update() {
	}	
	
	public ControlMainPanel getControlPanel() {
		return _control_panel;
	}
	
	public static void main(String [] argv) throws Exception {
		final Font font = new Font("Sans",Font.PLAIN,14);
		Enumeration<Object> enumeration = UIManager.getDefaults().keys();
		while(enumeration.hasMoreElements()){
		    Object key = enumeration.nextElement();
		    Object value = UIManager.get(key);
		    if (value instanceof java.awt.Font){
		    	UIManager.put(key.toString(), font);
		    }
		} 
		XMLRCNodeLoader loader = new XMLRCNodeLoader("example/cdc_test");
		loader.load("CDC");
		RCMainFrame frame = new RCMainFrame(loader.getSystem());
		try {
			RCDBManager.get().setNodeSystem(loader.getSystem());
		} catch (Exception e) {}
		frame.init();
	}

}
