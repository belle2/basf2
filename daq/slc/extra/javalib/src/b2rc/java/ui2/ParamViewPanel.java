package b2rc.java.ui2;

import java.awt.Dimension;
import java.awt.Font;
import java.net.Socket;
import java.util.ArrayList;

import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.UIManager;
import javax.swing.plaf.FontUIResource;

import b2daq.core.NSMCommand;
import b2daq.core.NSMData;
import b2daq.core.NSMMessage;
import b2daq.core.RCCommand;
import b2daq.database.DBObject;
import b2daq.database.FieldInfo;
import b2daq.java.io.SocketDataReader;
import b2daq.java.io.SocketDataWriter;
import b2daq.java.ui.CheckBoxPanel;
import b2daq.java.ui.ComboBoxPanel;
import b2daq.java.ui.FloatBoxPanel;
import b2daq.java.ui.IntegerBoxPanel;
import b2daq.java.ui.TextBoxPanel;

public class ParamViewPanel extends JPanel {

	private static final long serialVersionUID = 1L;
	
	public ParamViewPanel(String tablename, DBObject obj) {
		setLayout(new BoxLayout(this, BoxLayout.Y_AXIS));
		JPanel title_panel = new JPanel();
		JLabel title_label = new JLabel(obj.getConfigName() + " setting");
		title_label.setHorizontalAlignment(JLabel.CENTER);
		title_panel.setLayout(new BoxLayout(title_panel, BoxLayout.X_AXIS));
		title_panel.add(title_label);
		add(title_panel);
		add(Box.createRigidArea(new Dimension(0, 20)));
		add(new TextBoxPanel("runtype", obj.getConfigName()));
		add(Box.createRigidArea(new Dimension(0, 20)));
		for (String name : obj.getFieldNames()) {
			int type = obj.getType(name);
			switch (type) {
			case FieldInfo.BOOL: add(new CheckBoxPanel(name, obj.getBool(name))); break;
			case FieldInfo.SHORT: add(new IntegerBoxPanel(name, ""+obj.getShort(name))); break;
			case FieldInfo.INT: add(new IntegerBoxPanel(name, ""+obj.getInt(name))); break;
			case FieldInfo.LONG: add(new IntegerBoxPanel(name, ""+obj.getLong(name))); break;
			case FieldInfo.FLOAT: add(new FloatBoxPanel(name, ""+obj.getFloat(name))); break;
			case FieldInfo.TEXT: add(new TextBoxPanel(name, obj.getText(name))); break;
			case FieldInfo.ENUM: {
				String [] value_v = FieldInfo.getInfo(tablename+"."+name).getValue().split(",");
				add(new ComboBoxPanel(name, value_v));
				break;
			}
			case FieldInfo.OBJECT: add(new SelectButtonBoxPanel2(name, obj.getObjects(name))); break;
			case FieldInfo.TABLE: add(new SelectButtonBoxPanel2(name, obj.getObjects(name))); break;
			}
			add(Box.createRigidArea(new Dimension(0, 5)));
		}
	}
	static int showSelectPanel(String tablename, DBObject obj) {
		Object[] message2 = { new ParamViewPanel(tablename, obj) };
		String[] options2 = { "OK", "Cancel" };
		UIManager.put("OptionPane.buttonFont", new FontUIResource(new Font("Sans", Font.PLAIN, 13)));
		return JOptionPane.showOptionDialog(null, message2,
					"Edit RunType", JOptionPane.DEFAULT_OPTION,
					JOptionPane.PLAIN_MESSAGE, null, options2, options2[0]);
	}
	
	static public void main(String [] argv) throws Exception {
		NSMMessage msg = new NSMMessage();
		DBObject obj = new DBObject();
		RCCommand command = new RCCommand();
		Socket socket = new Socket("b2slow2.kek.jp", 9090);
		SocketDataWriter writer = new SocketDataWriter(socket);
		SocketDataReader reader = new SocketDataReader(socket);
		
		msg = new NSMMessage(NSMCommand.NSMGET, "");
		writer.writeObject(msg);
		msg = new NSMMessage(NSMCommand.DBGET, "field");
		writer.writeObject(msg);
		msg = new NSMMessage(NSMCommand.DBGET, "config SVD runcontrol");
		writer.writeObject(msg);
		msg = new NSMMessage(NSMCommand.DBGET, "data SVD runcontrol default");
		writer.writeObject(msg);
		while (true) {
			reader.readObject(msg);
			System.out.println(msg.getReqName());
			command.copy(msg.getReqName());
			if (command.equals(NSMCommand.DBSET)) {
				msg.getData(obj);
				if (obj.hasField("field")) {
					FieldInfo.clearInfoList();
					for (DBObject cobj: obj.getObjects("field").get()) {
						String name = cobj.getText("name");
						int type = cobj.getInt("type");
						String table = cobj.getText("table");
						String value = cobj.getText("value");
						FieldInfo info = new FieldInfo(name, type, value, table);
						FieldInfo.addInfo(table+"."+name, info);
					}
				}
				if (obj.hasField("config")) {
					ArrayList<String> runtype_v = new ArrayList<String>();
					for (DBObject cobj: obj.getObjects("config").get()) {
						runtype_v.add(cobj.getText("name"));
						System.out.println(cobj.getText("name"));
					}
				}
				if (obj.hasField("data")) {
					ParamViewPanel.showSelectPanel("runcontrol", obj.getObjects("data").get(0));
				}
			} else if (command.equals(NSMCommand.NSMSET)) {
				NSMData data = new NSMData();
				msg.getData(data);
				data.print();
			}
		}


		//
	}

}
