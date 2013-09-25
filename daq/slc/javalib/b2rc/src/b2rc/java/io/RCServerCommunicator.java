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
import b2rc.core.RCNodeSystem;
import b2rc.core.RCState;
import b2rc.java.ui.ControlMainPanel;

public class RCServerCommunicator {

	static private RCServerCommunicator __com;
	static public RCServerCommunicator get() { return __com; }
	
	private Socket _socket;
	private SocketDataWriter _socket_writer;
	private SocketDataReader _socket_reader;
	private RCNodeSystem _system;
	private ControlMainPanel _main_panel;

	public RCServerCommunicator(RCNodeSystem system, 
			ControlMainPanel main_panel) {
		_system = system;
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
		_system.getRunControlNode().setConnection(RCConnection.ONLINE);
		_system.getRunControlNode().setState(RCState.UNKNOWN);
		while (true) {
			_socket_reader.readObject(msg);
			RCCommand cmd = msg.getCommand();
			if (cmd.equal(RCCommand.STATE)) {
				int id = msg.getParam(0);
				if (id < 0) {
					RCState state_org = new RCState(_system.getRunControlNode().getState());
					_system.getRunControlNode().setConnection(msg.getParam(1));
					_system.getRunControlNode().setState(msg.getParam(2));
					RCState state = _system.getRunControlNode().getState();
					if (!state.equals(state_org) && _main_panel != null) {
						if ( state.isError() ) {
							_main_panel.addLog(new Log("Run Control got <span style='font-weight:bold;'>ERROR</span>",
												LogLevel.ERROR));
						} else {
							_main_panel.addLog(new Log("Run control is on "
									+ "<span style='color:blue;font-weight:bold;'>"
									+ state.getAlias() + "</span>", LogLevel.INFO));
						}
						_main_panel.update();
						if ( state_org.equals(RCState.UNKNOWN) ) {
							_main_panel.addLog(new Log("Current run configuration:<br/>"
									+ "<span style='color:blue;font-weight:bold;'>"
									+ "Exp # = " + _system.getExpNumber() + "<br/>"
									+ "Run # = " + _system.getRunNumber() + "<br/>"
									+ "Run type = " + _system.getRunType() + " (version = "+_system.getVersion()+")<br/>"
									+ "Operators = " + _system.getOperators().replace(":", " / ") + "<br/>"
									+ "Start time = " + new Time(_system.getStartTime()).toDateString() + "<br/>"
									+ "End time = " + ((_system.getEndTime()<0)?"on going":new Time(_system.getEndTime()).toDateString()) + "<br/>"
									+ "State = " + state.getAlias() + "</span>", LogLevel.INFO));
						}
						if  ( state_org.equals(RCState.RUNNING_S) && state.equals(RCState.STOPPING_TS ) )  {
							_main_panel.addLog(new Log("Run finished.", LogLevel.INFO));
							_main_panel.addLog(new Log("Run Summary:<br/>"
									+ "<span style='color:blue;font-weight:bold;'>"
									+ "Exp # = " + _system.getExpNumber() + "<br/>"
									+ "Run # = " + _system.getRunNumber() + "<br/>"
									+ "Run type = " + _system.getRunType() + " (version = "+_system.getVersion()+")<br/>"
									+ "Operators = " + _system.getOperators().replace(":", " / ") + "<br/>"
									+ "Start time = " + new Time(_system.getStartTime()).toDateString() + "<br/>"
									+ "End time = " + ((_system.getEndTime()<0)?"on going":new Time(_system.getEndTime()).toDateString()) + "<br/>"
									+ "State = " + state.getAlias() + "</span>", LogLevel.NOTICE));
						}
					}
				} else {
					RCNode node;
					try {
						node = _system.getNodes().get(id);
						RCState state_org = new RCState(node.getState());
						node.setConnection(msg.getParam(1));
						node.setState(msg.getParam(2));
						RCState state = node.getState();
						if (!state.equals(state_org) && _main_panel != null) {
							if ( state.isError() ) {
								_main_panel.addLog(new Log("Node " + node.getName()+
										" got <span style='color:blue;font-weight:bold;'>ERRPR</span> <br/>" +
										"Message = " + msg.getData() , LogLevel.ERROR));
							}
						}
						_main_panel.update();
					} catch (Exception e) {
						e.printStackTrace();
					}
				}
			} else if (cmd.equal(RCCommand.SET)) {
				int flag = msg.getParam(0);
				if ( flag == RunControlMessage.FLAG_OPERATORS ) {
					_system.setOperators("'"+msg.getData()+"'");
					_main_panel.getControlSummaryPanel().getConfigurationPanel().setOperators(msg.getData());
				} else if ( flag == RunControlMessage.FLAG_RUN_TYPE ) {
					_system.setRunType(msg.getData());
				} else {
					int value = msg.getParam(1);
					if ( flag == RunControlMessage.FLAG_MODE ) {
						_system.setOperationMode(value);
					} else if ( flag == RunControlMessage.FLAG_RUN_VERSION ) {
						_system.setVersion(value);
						_main_panel.getControlSummaryPanel().getConfigurationPanel().setVerionCombo(value);
					} else if ( flag == RunControlMessage.FLAG_RUN_LENGTH ) {
						_system.setVersion(value);                                                                              
					} else if ( flag == RunControlMessage.FLAG_EXP_NO ) {
						_system.setExpNumber(value);
						_main_panel.getControlSummaryPanel().getStatusPanel().setExpNumber(value);
					} else if ( flag == RunControlMessage.FLAG_RUN_NO ) {
						_system.setRunNumber(value);
						_main_panel.getControlSummaryPanel().getStatusPanel().setRunNumber(value);
					} else if ( flag == RunControlMessage.FLAG_START_TIME ) {
						_system.setStartTime(value);
						_main_panel.getControlSummaryPanel().getStatusPanel().setStartTime(value);
					} else if ( flag == RunControlMessage.FLAG_END_TIME ) {
						_system.setEndTime(value);
						_main_panel.getControlSummaryPanel().getStatusPanel().setEndTime(value);
					} else if ( flag == RunControlMessage.FLAG_TOTAL_TRIGGER ) {
						//_system.setTotalTriggers(value);
					}
				}
				_main_panel.update();
			}
		}
	}

	public ControlMainPanel getControlPanel() {
		return _main_panel;
	}
	
	public void sendMessage(RunControlMessage msg) throws Exception {
		_socket_writer.writeObject(msg);
	}

}
