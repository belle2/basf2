package b2logger.java;

import java.net.Socket;

import b2daq.core.LogLevel;
import b2daq.java.io.SocketDataReader;
import b2daq.java.io.SocketDataWriter;
import b2daq.java.ui.JavaEntoryPoint;
import b2logger.core.LogMessage;
import b2logger.core.SystemLogLevel;
import b2logger.java.ui.LogMainFrame;
import b2logger.java.ui.LogPopupLauncher;

public class Belle2LogViewer extends JavaEntoryPoint {

	public static final String VERSION = "0.0.1";

	static public Belle2LogViewer __VIEWER;
	static public Belle2LogViewer get() {
		return __VIEWER;
	}
	
	private String _host_name;
	private int _port;
	private LogPopupLauncher _generator = new LogPopupLauncher();
	private LogMainFrame _frame = new LogMainFrame();
	private Socket _socket;
	private SocketDataWriter _writer;
	private SocketDataReader _reader;

	public Belle2LogViewer() {
		__VIEWER = this;
	}

	public void run() {
		_frame.setVisible(true);
		_frame.update(new LogMessage("localhost", "GUI",
				SystemLogLevel.INFO, "Viewer started"));
		_frame.setPopupLevel("GaibuViewer", SystemLogLevel.ERROR);
		while (true) {
			try {
				_socket = new Socket(_host_name, _port);
				_writer = new SocketDataWriter(_socket);
				_reader = new SocketDataReader(_socket);
				_writer.writeString("GUI");
				_frame.update(new LogMessage("localhost", "GUI",
						SystemLogLevel.NOTICE, "Connected with LogCollecter."));
				while (true) {
					LogMessage message = new LogMessage();
					try {
						message.readObject(_reader);
						String host_name = message.getNodeName(); 
						LogLevel level = message.getLogLevel(); 
						if ( !_frame.getPopupLevel(host_name).hiegher(level) ) {
							_generator.create(message).start();
						}
						_frame.update(message);
					} catch (Exception e) {
						_frame.update(new LogMessage("localhost", "GUI",
								SystemLogLevel.DEBUG, "Got wong message."));
					}
				}
			} catch (Exception e) {
				e.printStackTrace();
				try { _socket.close(); } catch (Exception e1) {} 
				_frame.update(new LogMessage("localhost", "GUI",
						SystemLogLevel.WARNING, "Failed to reconnect to GaibuServer<br/>" +
						"Tring to reconnect after 3 second"));
				try {
					Thread.sleep(3000);
				} catch (InterruptedException e1) {}
			}
		}
	}

	public void init(String host, int port, String client) {
		_host_name = host;
		_port = port;
	}

	public static void main(String[] argv) {
		Belle2LogViewer gui = new Belle2LogViewer();
		String host = (argv.length > 0) ? argv[0] : "localhost";
		int port = (argv.length > 1) ? Integer.parseInt(argv[1]) : 50200;
		gui.init(host, port, "");
		gui.run();
	}

	public void sendMessage(LogMessage msg) {
		try { 
			_writer.writeString(msg.toGaibu());
		} catch (Exception e) {
			_frame.update(new LogMessage("localhost", "GUI",
					SystemLogLevel.WARNING, "sending : " + msg.toGaibu()));
			e.printStackTrace();
		}
	}
}
