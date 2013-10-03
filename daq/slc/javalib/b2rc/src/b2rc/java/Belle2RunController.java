package b2rc.java;

import java.io.ByteArrayInputStream;
import java.net.Socket;
import java.util.TimeZone;

import b2daq.java.io.SocketDataReader;
import b2daq.java.ui.JavaEntoryPoint;
import b2daq.java.ui.LoginPanel;
import b2rc.db.RCDBManager;
import b2rc.java.io.RCServerCommunicator;
import b2rc.java.ui.DBConfigPanel;
import b2rc.java.ui.RCMainFrame;
import b2rc.xml.XMLRCNodeLoader;

public class Belle2RunController extends JavaEntoryPoint {

	public static final String VERSION = "0.0.1";
	static private String __host = "loclhost";
	static private int __port = 50100;

	public static final int FLAG_ALL = 0;
	public static final int FLAG_UPDATE = 1;
	public static final int FLAG_LIST = 2;

	public static final int YES = 1;
	public static final int NO = -1;
	
	private XMLRCNodeLoader _loader;
	private RCMainFrame _frame;
	private RCServerCommunicator _communicator;

	public Belle2RunController() {
	}

	public void init(String host, int port, String client) {
		__host = host;
		__port = port;
	}

	public void run() {
		try {
			TimeZone.setDefault(TimeZone.getTimeZone("Asia/Tokyo"));
		} catch (Exception e) {}
		_loader = new XMLRCNodeLoader();
		try {
			Socket socket = new Socket(__host, __port);
			SocketDataReader socket_reader = new SocketDataReader(socket);
			int nfiles = socket_reader.readInt();
			for (int n = 0; n < nfiles; n++) {
				String label = socket_reader.readString();
				StringBuffer buf = new StringBuffer();
				while (true) {
					char c = socket_reader.readChar();
					if (c == '\0') {
						_loader.addInputstream(label, new ByteArrayInputStream(buf.toString().getBytes()));
						break;
					} else {
						buf.append(c);
					}
				}
			}
			_loader.load(socket_reader.readString());
			String hostname = socket_reader.readString();
			String database = socket_reader.readString();
			String username = socket_reader.readString();
			String password = socket_reader.readString();
			int port = socket_reader.readInt();
			if ( !DBConfigPanel.showLoginDialog(_frame, hostname, 
						database, username, password, port) ) System.exit(1);
			RCDBManager.open(hostname, database, username, password, port);
			RCDBManager.get().setNodeSystem(_loader.getSystem());
			_frame = new RCMainFrame(_loader.getSystem());
			_frame.init();
			_communicator = new RCServerCommunicator(_loader.getSystem(), 
						_frame.getControlPanel());
			_communicator.setSocket(socket);
			_communicator.run();
		} catch (Exception e) {
			e.printStackTrace();
			if ( _frame != null ) _frame.dispose();
			if ( !LoginPanel.showLoginDialog(_frame, __host, __port, 
					"Belle-II Run controller Login",
					"System ERROR! Try reconnect?",
					new Belle2RunController()) ) {
				System.exit(-1);
			}
		}
	}
	
	static public void main(String[] argv) {
		Belle2RunController gui = new Belle2RunController();
		String host = (argv.length > 0)? argv[0]:"localhost";
		int port = (argv.length > 1)? Integer.parseInt(argv[1]):50000;
		gui.init(host, port, "");
		gui.run();
	}
}
