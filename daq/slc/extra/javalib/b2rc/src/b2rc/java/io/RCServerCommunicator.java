package b2rc.java.io;

import java.io.IOException;
import java.net.Socket;

import b2daq.core.Log;
import b2daq.core.LogLevel;
import b2daq.core.Time;
import b2daq.java.io.SocketDataReader;
import b2daq.java.io.SocketDataWriter;
import b2rc.core.RCCommand;
import b2rc.core.RCConnection;
import b2rc.core.RCNode;
import b2rc.core.RCMaster;
import b2rc.core.RCState;
import b2rc.java.ui.ControlMainPanel;

public class RCServerCommunicator {

	static private RCServerCommunicator __com;
	static public RCServerCommunicator get() { return __com; }
	
	private Socket _socket;
	private SocketDataWriter _socket_writer;
	private SocketDataReader _socket_reader;
	private RCMaster _master;
	private ControlMainPanel _main_panel;

	public RCServerCommunicator(RCMaster system, 
			ControlMainPanel main_panel) {
		_master = system;
		_main_panel = main_panel;
		__com = this;
	}

	public void setSocket(Socket socket) throws IOException {
		_socket = socket;
		_socket_writer = new SocketDataWriter(_socket);
		_socket_reader = new SocketDataReader(_socket);
	}
	
	public void run() throws Exception {
		RunControlMessage msg = new RunControlMessage();
		_master.getNode().setConnection(RCConnection.ONLINE);
		_master.getNode().setState(RCState.UNKNOWN);
		while (true) {
			msg.getCommand().copy(_socket_reader.readInt());
 			RCCommand cmd = msg.getCommand();
 			if ( cmd.equal(RCCommand.DATA) ) {
 				String name = _socket_reader.readString();
 				if ( name.matches(_master.getNode().getName()) ) {
 					_master.getData().readObject(_socket_reader);
 				} else if ( name.matches(_master.getConfig().getClassName()) ) {
 					_master.getConfig().print();
 					_master.getConfig().readObject(_socket_reader);
 					_master.getConfig().print();
 					//_master.load();
 				} else if ( name.matches(_master.getStatus().getClassName()) ) {
 					_master.getStatus().readObject(_socket_reader);
 					_master.getStatus().print();
 					//_master.load();
 				} else {
 					_master.getData().getObject(name).readObject(_socket_reader);
 				}
 			} else if (cmd.equal(RCCommand.ERROR) ) {
				String name = _socket_reader.readString();
				RCNode node = null;
				if ( name.matches(_master.getNode().getName()) ) {
					node = _master.getNode();
				} else {
					node = _master.getNodeByName(name);
				}
				_socket_reader.readObject(msg);
				_master.getStatus().update();
				RCState state = node.getState();
				state.copy(RCState.ERROR_ES);
				_main_panel.addLog(new Log(node.getName() +
						" got <span style='font-weight:bold;'>ERROR</span><br />"+
						msg.getData(), LogLevel.ERROR));
				_main_panel.update();
			} else if (cmd.equal(RCCommand.STATE)) {
				String name = _socket_reader.readString();
				RCNode node = null;
				if ( name.matches(_master.getNode().getName()) ) {
					node = _master.getNode();
				} else {
					node = _master.getNodeByName(name);
				}
				int number = _socket_reader.readInt();
				RCState state_org = new RCState(_master.getNode().getState());
				node.getState().copy(number);
				node.getConnection().copy(_socket_reader.readInt());
				if (node.getConnection().equals(RCConnection.OFFLINE)) {
					node.setState(RCState.UNKNOWN);
				}
				RCState state = _master.getNode().getState();
				_master.getStatus().update();
				_main_panel.update();
				if (!state.equals(state_org) && _main_panel != null) {
					if ( state.isError() ) {
						_main_panel.addLog(new Log("Run Control got <span style='font-weight:bold;'>ERROR</span><br />"+
								msg.getData(), LogLevel.ERROR));
					} else {
						_main_panel.addLog(new Log("Run control is on "
								+ "<span style='color:blue;font-weight:bold;'>"
								+ state.getAlias() + "</span>", LogLevel.INFO));
					}
					_main_panel.update();
					if ( state_org.equals(RCState.UNKNOWN) ) {
						_main_panel.addLog(new Log("Current run configuration:<br/>"
								+ "<span style='color:blue;font-weight:bold;'>"
								+ "Exp # = " + _master.getStatus().getExpNumber() + "<br/>"
								+ "Run # = " + _master.getStatus().getColdNumber() + "<br/>"
								+ "Run type = " + _master.getConfig().getRunType() + " (version = "+_master.getConfig().getConfigNumber()+")<br/>"
								+ "Operators = " + _master.getStatus().getOperators().replace(":", " / ") + "<br/>"
								+ "Start time = " + new Time(_master.getStatus().getStartTime()).toDateString() + "<br/>"
								+ "End time = " + ((_master.getStatus().getEndTime()<0)?"on going":new Time(_master.getStatus().getEndTime()).toDateString()) + "<br/>"
								+ "State = " + state.getAlias() + "</span>", LogLevel.INFO));
					}
					if  ( state_org.equals(RCState.RUNNING_S) && state.equals(RCState.READY_S ) )  {
						_main_panel.addLog(new Log("Run finished.", LogLevel.INFO));
						_main_panel.addLog(new Log("Run Summary:<br/>"
								+ "<span style='color:blue;font-weight:bold;'>"
								+ "Exp # = " + _master.getStatus().getExpNumber() + "<br/>"
								+ "Run # = " + _master.getStatus().getColdNumber() + "<br/>"
								+ "Run type = " + _master.getConfig().getRunType() + " (version = "+_master.getConfig().getConfigNumber()+")<br/>"
								+ "Operators = " + _master.getStatus().getOperators().replace(":", " / ") + "<br/>"
								+ "Start time = " + new Time(_master.getStatus().getStartTime()).toDateString() + "<br/>"
								+ "End time = " + ((_master.getStatus().getEndTime()<0)?"on going":new Time(_master.getStatus().getEndTime()).toDateString()) + "<br/>"
								+ "State = " + state.getAlias() + "</span>", LogLevel.NOTICE));
					}
					_main_panel.update();
				}
			}
		}
	}

	public ControlMainPanel getControlPanel() {
		return _main_panel;
	}

	public RCMaster getMaster() {
		return _master;
	}
	
	public void sendMessage(RunControlMessage msg) throws Exception {
		try {
			_socket_writer.writeInt(msg.getCommand().getId());
			_socket_writer.writeObject(msg);
		} catch(IOException e) {
			_main_panel.addLog(new Log("Connection broken", LogLevel.ERROR));
			throw e;
		}
	}

}
 