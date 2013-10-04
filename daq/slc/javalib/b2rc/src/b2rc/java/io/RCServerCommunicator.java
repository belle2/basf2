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
		_socket_writer.writeObject(new RunControlMessage(RCCommand.STATECHECK, -1));
		boolean initialized = false;
		while (true) {
			_socket_reader.readObject(msg);
 			RCCommand cmd = msg.getCommand();
 			/*
 			if ( cmd.equal(RCCommand.DATA) ) {
				RCNode node = _system.getNodes().get(msg.getParam(0));
				int npar = msg.getNParams();
				StringBuffer ss = new StringBuffer();
				ss.append("NSM data in "+ node.getName() + "<br/>");
				ss.append("npar = "+ (npar -1) + "<br/>");
				for ( int i = 0; i < npar-1; i++ ) {
					ss.append("par["+i+"] = "+ msg.getParam(i+1) + "<br/>");
				}
				_main_panel.addLog(new Log(ss.toString(), LogLevel.INFO));
 			} else*/ if ( cmd.equal(RCCommand.ERROR) ) {
				RCNode node = _system.getNodes().get(msg.getParam(0));
				_main_panel.addLog(new Log("Node " + node.getName()+ " got ERRPR : <br/>" +
						"<span style='color:red;font-weight:bold;'>" + msg.getData()+"</span>",
						LogLevel.ERROR));
			} else if (cmd.equal(RCCommand.OK)) {
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
						if ( msg.getNParams() > 3 ) {
							int version = msg.getParam(3);
							int exp_no = msg.getParam(4);
							int run_no = msg.getParam(5);
							int start_time = msg.getParam(6);
							_system.setVersion(version);
							_system.setExpNumber(exp_no);
							_system.setRunNumber(run_no);
							_system.setStartTime(start_time);
							if ( msg.getNParams() > 7 ) {
								int end_time = msg.getParam(7);
								_system.setEndTime(end_time);
							}
							String [] str_v = msg.getData().split("\n");
							if ( str_v.length > 0 ) {
								String run_type = str_v[0];
								_system.setRunType(run_type);
							}
							if ( str_v.length > 1 ) {
								String operators = str_v[1];
								_main_panel.getControlSummaryPanel().getConfigurationPanel().setOperators(operators);
								_system.setOperators(operators);
							}
							if ( !initialized ) {
								_main_panel.getControlSummaryPanel().getConfigurationPanel().readDatabase(version);;
							}
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
					RCNode node = _system.getNodes().get(id);
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
				}
			}
		}
	}

	public ControlMainPanel getControlPanel() {
		return _main_panel;
	}

	public RCNodeSystem getNodeSystem() {
		return _system;
	}
	
	public void sendMessage(RunControlMessage msg) throws Exception {
		try {
			_socket_writer.writeObject(msg);
		} catch(IOException e) {
			_main_panel.addLog(new Log("Connection broken", LogLevel.ERROR));
			throw e;
		}
	}

}
 