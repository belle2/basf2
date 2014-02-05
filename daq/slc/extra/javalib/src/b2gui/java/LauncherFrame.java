package b2gui.java;

import java.awt.Toolkit;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.util.ArrayList;

import javax.swing.BoxLayout;
import javax.swing.JFrame;
import javax.swing.JOptionPane;
import javax.swing.JPanel;

import b2daq.java.ui.LogViewPanel;

public class LauncherFrame extends JFrame {

	private static LauncherFrame __frame = new LauncherFrame();

	public static LauncherFrame get() {
		return __frame;
	}

	private static final long serialVersionUID = 1L;
	private LauncherMainPanel _main_panel = new LauncherMainPanel();
	private LogViewPanel _log_panel = new LogViewPanel();
	private LauncherLoginPane _login_panel = null;
	private ArrayList<Process> _process_v = new ArrayList<Process>();
	private String _class_path = "b2gui-exe.jar";
	private boolean _use_ssh;

	private LauncherFrame() {
		String hostname = "localhost";
		_login_panel = new LauncherLoginPane(hostname, hostname, 22, "");
		setDefaultCloseOperation(JFrame.DO_NOTHING_ON_CLOSE);
		addWindowListener(new WindowAdapter() {
			public void windowClosing(WindowEvent e) {
				int ans = JOptionPane.showConfirmDialog(null,
						(_use_ssh?"All GUIs may close after the launcher closes.\n":"")+
						"Are you really closing this launcher?");
				if ( ans == JOptionPane.YES_OPTION ) {
					if ( _use_ssh ) {
						for ( Process process : _process_v ) {
							process.destroy();
						}
					}
					System.exit(0);
				}
			}
		});

		setIconImage(Toolkit.getDefaultToolkit().createImage(getClass().getResource("guilauncher.png")));
		setBounds(0, 0, 760, 50 * _main_panel.getPanels().length + 60);
		setLocationRelativeTo(null);
		setTitle("Belle 2 GUI Launcher version " + Belle2GUILauncherJavaGUI.VERSION);
		setJMenuBar(new LauncherMenuBar());
		JPanel panel = new JPanel();
		panel.setLayout(new BoxLayout(panel, BoxLayout.X_AXIS));
		panel.add(_main_panel);
		panel.add(_log_panel);
		add(panel);
	}

	public LauncherPanel[] getLancherPanels() {
		return _main_panel.getPanels();
	}

	public LogViewPanel getLog() {
		return _log_panel;
	}

	public void setHostName(String hostname) {
		for (LauncherPanel panel : _main_panel.getPanels()) {
			panel.setHostName(hostname);
		}
	}

	public LauncherLoginPane getLoginPanel() {
		return _login_panel;
	}

	public void addProcess(Process process) {
		_process_v.add(process);
	}

	public void setClassPath(String class_path) {
		_class_path = class_path;
	}

	public String getClassPath() {
		return _class_path;
	}

	public void setEnableButtons(boolean enabled) {
		_main_panel.setEnableButtons(enabled);
	}

	public void setUseSSH(boolean use_ssh){
		_use_ssh = use_ssh;
	}
	
}
