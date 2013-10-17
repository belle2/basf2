package b2rc.java.ui;

import java.util.ArrayList;
import java.util.Vector;

import javax.swing.JTable;
import javax.swing.event.TableModelEvent;
import javax.swing.event.TableModelListener;
import javax.swing.table.DefaultTableModel;

import b2rc.core.RONode;
import b2rc.core.RCNodeSystem;
import b2rc.db.RCDBManager;

public class ROParameterTablePanel extends ParameterTablePanel {

	private ArrayList<RONode> _recv_v; 
	
	public ROParameterTablePanel(EditorMainPanel editor_panel,VersionControlPanel version_panel, RCNodeSystem system) {
		super(editor_panel, version_panel, system, "ro_node", "ROPC");
		_recv_v = system.getReceiverNodes();
		DefaultTableModel model = (DefaultTableModel)getModel();
		model.addColumn("NSM name");
		model.addColumn("Host name");
		//model.addColumn("ProductID");
		//model.addColumn("Location");
		model.addColumn("Readout script");
		for ( int i = 0; i < 10; i++ ) {
			model.addColumn("Sender "+i);
		}
		_table.getModel().addTableModelListener(new TableModelListener() {
			public void tableChanged(TableModelEvent e) {
				int row = e.getFirstRow();
				int col = e.getColumn();
				if ( row <  0 || col < 0) return;
				DefaultTableModel model2 = (DefaultTableModel) getModel();
				try {
					int id = (Integer)model2.getValueAt(row, 0);
					if ( id <  0 ) return;
					RONode recv = _recv_v.get(id);
					switch (col) {
					case 1:
						recv.setUsed((Boolean)model2.getValueAt(row, col));
						break;
					case 4:
						recv.setScript(model2.getValueAt(row, col).toString());
						break;
					default:
						if ( col > 4 ) {
							int i = col - 5;
							recv.setSender(i, model2.getValueAt(row, col).toString());
						}
					}
				} catch (Exception e1) {}
			}
		});
		_table.setAutoResizeMode(JTable.AUTO_RESIZE_OFF);
		update();
	}

	private static final long serialVersionUID = 1L;

	public void update() {
		super.update();
		DefaultTableModel model = (DefaultTableModel)getModel();
		for ( RONode recv : _recv_v ) {
			Vector<Object> param_v = new Vector<Object>();
			param_v.add(recv.getId());
			param_v.add(recv.isUsed());
			param_v.add(recv.getName());
			param_v.add(recv.getHost().getName());
			//param_v.add(""+recv.getHost().getProductID());
			//param_v.add(recv.getHost().getLocation());
			param_v.add(recv.getScript());
			for ( String sender : recv.getSenders() ) {
				if ( sender != null) param_v.add(sender);
				else param_v.add("");
			}
			model.addRow(param_v);
		}
	}

	@Override
	public void upload() throws Exception {
		_version_max++;
		_version = _version_max;
		for ( RONode recv : _recv_v ) {
			recv.setVersion(_version);
		    RCDBManager.get().execute("insert into "+_label+"_conf ("
		       + recv.getSQLLabels() + ") values ("
		       + recv.getSQLValues() + "); ");
		}
	}

	@Override
	public void download() throws Exception {
		RCDBManager.get().readRONodeTable(_version);
	}

}
