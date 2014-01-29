package b2gui.java;

import javax.swing.UIManager;

public class Belle2GUILauncherJavaGUI {

	public static final String VERSION = "0.0.1";

	public static void main(String[] argv) {
		try {
			UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
		} catch (Exception e) {}
		LauncherFrame frame = LauncherFrame.get();
		frame.setVisible(true);
		/*
		LauncherLoginPane.showPane();
		if ( argv.length > 0 ) LauncherFrame.get().setClassPath(argv[0]);
		*/
	}

}
