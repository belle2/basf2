package b2daq.apps;

import java.awt.Component;
import java.io.File;
import java.net.Socket;
import java.util.ArrayList;

import javax.swing.JFrame;
import javax.swing.UIManager;

import b2daq.dqm.ui.PanelGenerator;
import b2daq.io.SocketDataReader;
import b2daq.nsm.NSMData;
import b2daq.ui.JavaEntoryPoint;
import b2daq.ui.UTabPanel;

public class Belle2EnvMonitor extends JavaEntoryPoint {

	static final public String VERSION = "0.0.1";
	static private String __host = "localhost";
	static private int __port = 50300;

	public Belle2EnvMonitor() {
	}

	public void init(String host, int port, String client) {
		__host = host;
		__port = port;
	}

	public void run() {
		try {
			Socket _socket = new Socket(__host, __port);
			SocketDataReader socket_reader = new SocketDataReader(_socket);
			int ndata = socket_reader.readInt();
			UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
			JFrame frame = new JFrame();
			frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
			frame.setSize(920, 480);
			frame.setTitle("Belle-II Environment monitor version "
					+ Belle2EnvMonitor.VERSION);
			frame.setLocationRelativeTo(null);
			final String str_dir = "/home/tkonno/config";
			File dir = new File(str_dir);
			String[] path_v = dir.list();
			ArrayList<PanelGenerator> gen_v = new ArrayList<PanelGenerator>();
			UTabPanel tabpanel = new UTabPanel();
			for (String path : path_v) {
				if (path.endsWith("conf")) {
					String filepath = str_dir + "/"+ path;
					PanelGenerator gen = new PanelGenerator(filepath);
					System.out.println(filepath);
					gen_v.add(gen);
					Component panel = gen.create();
					tabpanel.addTab(gen.getTitle(), panel);
				}
			}
			frame.add(tabpanel);
			frame.setVisible(true);
			ArrayList<NSMData> data_v = new ArrayList<NSMData>();
			for (int n = 0; n < ndata; n++) {
				data_v.add(new NSMData());
			}
			while (true) {
				int n = socket_reader.readInt();
				NSMData data = data_v.get(n);
				data.readObject(socket_reader);
				for (PanelGenerator gen : gen_v) {
					if (!gen.getManager().hasData(data.getName())) {
						gen.getManager().add(data);
					}
					gen.update();
				}
			}
		} catch (Exception e) {
			e.printStackTrace();
		}
	}
	public static void main(String[] argv) {
		Belle2EnvMonitor gui = new Belle2EnvMonitor();
		String host = (argv.length > 0)? argv[0]:"localhost";//"130.87.227.252";//
		int port = (argv.length > 1)? Integer.parseInt(argv[1]):50300;
		gui.init(host, port, "");
		gui.run();
	}
}
