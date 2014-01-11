package b2rc.java.ui;

import java.awt.Component;
import java.awt.Dimension;

import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.UIManager;

import b2daq.core.Log;
import b2daq.core.LogLevel;
import b2daq.java.ui.CheckBoxPanel;
import b2daq.java.ui.ComboBoxPanel;
import b2daq.java.ui.TextBoxPanel;
import b2rc.core.DataObject;
import b2rc.core.RCCommand;
import b2rc.core.RCNode;
import b2rc.java.io.RCServerCommunicator;
import b2rc.java.io.RunControlMessage;

public class ConfigEdiftorPanel extends JPanel {
	
	private static final long serialVersionUID = 1L;

	DataObject _obj;
	
	public ConfigEdiftorPanel(DataObject obj) {
		super();
		_obj = obj;
		setSize(300, 300);
		setLayout(new BoxLayout(this, BoxLayout.Y_AXIS));
		JPanel title_panel = new JPanel();
		JLabel title_label = new JLabel(" Trigger setting");
		title_label.setHorizontalAlignment(JLabel.CENTER);
		title_panel.setLayout(new BoxLayout(title_panel, BoxLayout.X_AXIS));
		title_panel.add(title_label);
		add(title_panel);
		add(Box.createRigidArea(new Dimension(0,20)));
		for ( String name : obj.getParams().keySet()) {
			DataObject.ParamInfo info = obj.getParams().get(name);
			JPanel panel = null; 
			switch (info.type) {
				case DataObject.BOOL: {
					panel = new CheckBoxPanel(name, obj.getBool(name), 120, 30, 150, 30);
				}
				break;
				case DataObject.TEXT: {
					panel = new TextBoxPanel(name, obj.getText(name), 120, 30, 150, 30);
				}
				break;
				case DataObject.ENUM: {
					System.out.println(name);
					String [] str_v = new String [obj.
					                              getEnumList(name).
					                              size()];
					int i = 0;
					for (String ename : obj.getEnumList(name).keySet()) {
						str_v[i++] = ename; 
					}
					panel = new ComboBoxPanel(name, str_v, 120, 30, 100, 30);
				}
				break;
				case DataObject.OBJECT: 
					break;
				case DataObject.CHAR: {
					panel = new TextBoxPanel(name, "" + obj.getChar(name), 120, 30, 80, 30);
				}
				break;
				case DataObject.SHORT: {
					panel = new TextBoxPanel(name, "" + obj.getShort(name), 120, 30, 80, 30);
				}
				break;
				case DataObject.INT: {
					panel = new TextBoxPanel(name, "" + obj.getInt(name), 120, 30, 80, 30);
				}
				break;
				case DataObject.LONG: {
					panel = new TextBoxPanel(name, "" + obj.getLong(name), 120, 30, 80, 30);
				}
				break;
				case DataObject.UCHAR: {
					panel = new TextBoxPanel(name, "" + obj.getUChar(name), 120, 30, 80, 30);
				}
				break;
				case DataObject.USHORT: {
					panel = new TextBoxPanel(name, "" + obj.getUShort(name), 120, 30, 80, 30);
				}
				break;
				case DataObject.UINT: {
					panel = new TextBoxPanel(name, "" + obj.getUInt(name), 120, 30, 80, 30);
				}
				break;
				case DataObject.ULONG: {
					panel = new TextBoxPanel(name, "" + obj.getULong(name), 120, 30, 80, 30);
				}
				break;
				case DataObject.FLOAT: {
					panel = new TextBoxPanel(name, "" + obj.getFloat(name), 120, 30, 80, 30);
				}
				break;
				case DataObject.DOUBLE: {
					panel = new TextBoxPanel(name, "" + obj.getDouble(name), 120, 30, 80, 30);
				}
				break;
			}
			if (panel != null) {
				add(panel);
				add(Box.createRigidArea(new Dimension(0,5)));
			}
		}
		add(Box.createRigidArea(new Dimension(0,20)));
	}
	
	public static boolean showLoginDialog(Component window, DataObject obj) {
		try {
			UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
		} catch (Exception e) {}

		ConfigEdiftorPanel panel = new ConfigEdiftorPanel(obj);
		Object[] message = { panel };
		String[] options = { "Submit", "Cancel" };

		int result = JOptionPane.showOptionDialog(window,
				message,
				" Trigger setting", 
				JOptionPane.DEFAULT_OPTION,
				JOptionPane.PLAIN_MESSAGE,
				null,
				options,
				options[0]);
		switch(result) {
		   case 0:
			   return true;
		   default:
			   return false;
		}
	}
	
}
