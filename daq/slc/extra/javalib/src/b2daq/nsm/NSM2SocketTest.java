package b2daq.nsm;

import javax.swing.JFrame;

import b2daq.database.ConfigObject;
import b2daq.hvcontrol.ui.HVStateMonitorPanel;
import b2daq.hvcontrol.ui.HVVoltageMonitorPanel;
import b2daq.ui.UTablePanel;

public class NSM2SocketTest {

	private NSMSocket m_nsmsocket;
	private NSMNode m_node = new NSMNode("ARICH_HV"); 
	private NSMData m_data = new NSMData("ARICH_HV_STATUS", "hv_status", 1);
	private NSMMessage m_msg = new NSMMessage();
	private ConfigObject m_obj = new ConfigObject();
	private HVStateMonitorPanel m_panel = new HVStateMonitorPanel("ARICH-HV crate 0");
	private HVVoltageMonitorPanel m_vpanel;
	
	private void listen() {
		new Thread() {
			public void run() {
				try {
					while (true) {
						m_nsmsocket.requestGet(m_data);
						//m_nsmsocket.requestGet(m_obj);
						Thread.sleep(2000);
					}
				} catch (Exception e) {
					e.printStackTrace();
				}
			}
		}.start();
	}
	
	public void run() throws Exception {
		  m_nsmsocket = new NSMSocket("localhost", 9090);
		  m_nsmsocket.requestGet(m_data);
		  m_msg = m_nsmsocket.wait(m_msg);
		  m_msg.getData(m_data);
		  m_data.print();

		  System.out.println("hv_status.configid = " + m_data.getInt("configid", 0));
		  m_obj.setNode(m_node.getName());
		  m_obj.setId(m_data.getInt("configid", 0));//16
		  m_nsmsocket.requestGet(m_obj);
		  m_msg = m_nsmsocket.wait(m_msg);
		  m_msg.getData(m_obj);
		  m_obj.print();
		  
		  JFrame frame = new JFrame();
		  frame.setSize(800, 640);
		  frame.setLocationRelativeTo(null);
		  UTablePanel table = new UTablePanel(2, 1);
		  table.add(m_panel);
		  frame.add(table);
		  m_vpanel = new HVVoltageMonitorPanel(m_obj, 0, 1);
		  table.add(m_vpanel);
		  frame.setVisible(true);
		  NSMCommand command = new NSMCommand();
		  listen();
		  //m_nsmsocket.request(new NSMMessage(m_node, RCCommand.PEAK));
		  while (true) {
			  m_msg = m_nsmsocket.wait(m_msg);
			  System.out.println(m_msg.getReqName());
			  command.copy(m_msg.getReqName());
			  if (command.equal(NSMCommand.DBSET)) {
				  m_msg.getData(m_obj);
				  //m_obj.print();
			  } else if (command.equal(NSMCommand.NSMSET)) {
				  m_msg.getData(m_data);
				  //m_data.print();
				  m_panel.update(m_obj, m_data);
				  m_vpanel.update(m_obj, m_data);
			  }
		  }
	}
	
	public static void main(String[] args) throws Exception {
		new NSM2SocketTest().run();
	}

}
