package b2gui.java;

public class Belle2GUILauncherJavaGUI {

	public static final String VERSION = "0.0.1";

	public static void main(String[] argv) {
		LauncherLoginPane.showPane();
		if ( argv.length > 0 ) LauncherFrame.get().setClassPath(argv[0]);
	}

}
