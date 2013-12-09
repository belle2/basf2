package b2rc.java.ui;

import java.awt.Toolkit;

import javax.swing.JFrame;
import javax.swing.UIManager;

import b2daq.java.ui.DnDTabbedPane;
import b2daq.ui.Updatable;
import b2rc.core.RCMaster;
import b2rc.java.Belle2RunController;

public class RCMainFrame extends JFrame implements Updatable {
	
	private static RCMainFrame __main = null;
	
	private static final long serialVersionUID = 1L;
	
	public static RCMainFrame get() {
		return __main;
	}

	private RCMaster _master;
	private ControlMainPanel _control_panel;
	//private EditorMainPanel _editor_panel;
	
	public RCMainFrame(RCMaster master) {
		_master = master;
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
		//_editor_panel = new EditorMainPanel(_system);
		_control_panel = new ControlMainPanel(/*_editor_panel,*/ _master);
		main_tab.addTab("Control", _control_panel);
		//main_tab.addTab("Editor", _editor_panel);
		add(main_tab);
		//_editor_panel.init();
		setVisible(true);
	}
	
	public void update() {
	}	
	
	public ControlMainPanel getControlPanel() {
		return _control_panel;
	}
	
}
