package b2rc.java;

import java.net.Socket;
import java.util.TimeZone;

import b2daq.java.io.SocketDataReader;
import b2daq.java.io.SocketDataWriter;
import b2daq.java.ui.JavaEntoryPoint;
import b2daq.java.ui.LoginPanel;
import b2rc.core.DataObject;
import b2rc.core.RCConnection;
import b2rc.core.RCMaster;
import b2rc.core.RCNode;
import b2rc.core.RunConfig;
import b2rc.core.RunStatus;
import b2rc.java.io.RCServerCommunicator;
import b2rc.java.ui.RCMainFrame;

public class Belle2RunController extends JavaEntoryPoint {

	private RCMainFrame _frame = null;
	
	public static final String VERSION = "0.0.1";
	static private String __host = "loclhost";
	static private int __port = 50100;

	public Belle2RunController() {}

	public void init(String host, int port, String client) {
		__host = host;
		__port = port;
	}

	public void run() {
		try {
			TimeZone.setDefault(TimeZone.getTimeZone("Asia/Tokyo"));
		} catch (Exception e) {}
		try {
			Socket socket = new Socket(__host, __port);
			SocketDataReader socket_reader = new SocketDataReader(socket);
			SocketDataWriter socket_writer = new SocketDataWriter(socket);
			socket_writer.writeString("GUI");
			DataObject data = new DataObject();
			RunStatus run_status = new RunStatus();
			RunConfig run_config = new RunConfig();
			run_status.setConfig(run_config);
			String name = socket_reader.readString();
			RCMaster master = new RCMaster(new RCNode(name), run_config, run_status);
			master.getNode().setConnection(RCConnection.ONLINE);
			data.readObject(socket_reader);
			master.setData(data);
			data.print();
			run_status.readObject(socket_reader);
			run_status.print();
			run_config.readObject(socket_reader);
			run_config.print();
			master.load();
			_frame = new RCMainFrame(master);
			_frame.init();
			RCServerCommunicator _communicator = 
					new RCServerCommunicator(master, _frame.getControlPanel());
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
		String host = (argv.length > 0)? argv[0]:"localhost";//"b2slow2.kek.jp";
		int port = (argv.length > 1)? Integer.parseInt(argv[1]):50000;
		gui.init(host, port, "");
		gui.run();
	}
}
