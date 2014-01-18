package b2dqm.java;

import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.net.Socket;
import java.util.ArrayList;
import java.util.TimeZone;

import b2daq.java.io.SocketDataReader;
import b2daq.java.io.SocketDataWriter;
import b2daq.java.io.ZlibInflater;
import b2daq.java.ui.JavaEntoryPoint;
import b2daq.java.ui.LoginPanel;
import b2dqm.core.HistoPackage;
import b2dqm.core.PackageInfo;
import b2dqm.java.ui.DQMMainFrame;
import b2dqm.java.ui.PackageSelectPanel;

public class Belle2DQMBrowser extends JavaEntoryPoint {

	public static final String VERSION = "0.0.1";
	static private String __host = "loclhost";
	static private int __port = 50100;

	public static final int FLAG_ALL = 0;
	public static final int FLAG_UPDATE = 1;
	public static final int FLAG_LIST = 2;

	public static final int YES = 1;
	public static final int NO = -1;
	
	private Socket _socket = null;
	private DQMMainFrame _frame = null;
	private ArrayList<HistoPackage> _pack_v = null;
	private ArrayList<PackageInfo> _info_v = null;
	
	public Belle2DQMBrowser() {
	}

	public void init(String host, int port, String client) {
		__host = host;
		__port = port;
	}

	public void run() {
		try {
			//TimeZone.setDefault(TimeZone.getTimeZone("Asia/Tokyo"));
		} catch (Exception e) {}
		try {
			_socket = new Socket(__host, __port);
			SocketDataWriter socket_writer = new SocketDataWriter(_socket);
			SocketDataReader socket_reader = new SocketDataReader(_socket);
			//ZlibInflater inflater = new ZlibInflater();
			while (true) {
				int flag = socket_reader.readInt();
				switch ( flag ) {
				case FLAG_LIST : {
					if ( _frame != null) _frame.dispose();
					_frame = new DQMMainFrame();		
					_pack_v = new ArrayList<HistoPackage>();
					int npacks = socket_reader.readInt();
					for ( int n = 0; n < npacks; n++ ) {
						String name = socket_reader.readString();
						HistoPackage pack = new HistoPackage(name);
						_pack_v.add(pack);
					}
					ArrayList<Boolean> monitored_v = PackageSelectPanel.showPane(_pack_v);
					for ( int n = 0; n < monitored_v.size(); n++ ) {
						//System.out.println("n");
						socket_writer.writeByte(monitored_v.get(n)?(byte)1:0);
					}
				}
				break;
				case FLAG_ALL : {
					int npacks = socket_reader.readInt();
					System.out.println("npacks="+npacks);
					_pack_v = new ArrayList<HistoPackage>();
					_info_v = new ArrayList<PackageInfo>();
					ArrayList<InputStream> istream_v = new ArrayList<InputStream>();
					for (int n = 0; n < npacks; n++) {
						HistoPackage pack = new HistoPackage();
						//inflater.readBuffer(socket_reader);
						//pack.readConfig(inflater);
						//inflater.readBuffer(socket_reader);
						//pack.readContents(inflater);
						pack.readConfig(socket_reader);
						pack.readContents(socket_reader);
						pack.setUpdateTime(0);
						_pack_v.add(pack);
						PackageInfo info = new PackageInfo();
						_info_v.add(info);
						StringBuffer buf = new StringBuffer();
						while (true) {
							char c = socket_reader.readChar();
							if (c == '\0') {
								istream_v.add(new ByteArrayInputStream(buf.toString().getBytes()));
								break;
							} else {
								//System.out.print(c);
								buf.append(c);
							}
						}
					}
					_frame.init(_pack_v, istream_v, _info_v);
				}
				break;
				case FLAG_UPDATE : {
					int id = socket_reader.readInt();
					if ( id >= 0 ) {
						_pack_v.get(id).readContents(socket_reader);
						_frame.update();
					}
				}
				break;
				}
			}
		} catch (Exception e) {
			e.printStackTrace();
			try {
				_socket.close();
			} catch (IOException e1) {}
			if (_frame != null)_frame.dispose();
			if ( !LoginPanel.showLoginDialog(_frame, __host, __port, 
					"Belle-II DQM browser Login",
					"System ERROR! Try reconnect?",
					new Belle2DQMBrowser()) ) {
				System.exit(-1);
			}
		}
	}
	
	public static void main(String[] argv) {
		Belle2DQMBrowser gui = new Belle2DQMBrowser();
		String host = (argv.length > 0)? argv[0]:"192.168.99.45";//"130.87.227.252";//
		int port = (argv.length > 1)? Integer.parseInt(argv[1]):50100;
		gui.init(host, port, "");
		gui.run();
	}
}
