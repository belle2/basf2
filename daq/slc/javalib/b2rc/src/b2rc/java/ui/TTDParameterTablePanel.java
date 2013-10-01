package b2rc.java.ui;

import java.util.Vector;

import javax.swing.JTable;
import javax.swing.event.TableModelEvent;
import javax.swing.event.TableModelListener;
import javax.swing.table.DefaultTableModel;

import b2rc.core.RCNodeSystem;
import b2rc.core.TTDNode;
import b2rc.db.RCDBManager;

public class TTDParameterTablePanel extends ParameterTablePanel {

	public TTDParameterTablePanel(EditorMainPanel editor_panel,VersionControlPanel version_panel, RCNodeSystem system) {
		super(editor_panel, version_panel, system, "ttd_node", "TTD");
		DefaultTableModel model = (DefaultTableModel)getModel();
		model.addColumn("Name");
		//model.addColumn("ProductID");
		//model.addColumn("Location");
		for ( int i = 0; i < 10; i++ ) {
			model.addColumn("FTSW "+i);
		}
		_table.setAutoResizeMode(JTable.AUTO_RESIZE_OFF);
		_table.getModel().addTableModelListener(new TableModelListener() {
			public void tableChanged(TableModelEvent e) {
				int row = e.getFirstRow();
				int col = e.getColumn();
				if ( row <  0 || col < 0) return;
				DefaultTableModel model2 = (DefaultTableModel) getModel();
				try {
					int id = (Integer)model2.getValueAt(row, 0);
					if ( id <  0 ) return;
					TTDNode ttd = _system.getTTDNodes().get(id);
					switch (col) {
					case 1:
						ttd.setUsed((Boolean)model2.getValueAt(row, col));
						break;
					case 2:
						ttd.setName(model2.getValueAt(row, col).toString());
						break;
					default:
					}
				} catch (Exception e1) {
					e1.printStackTrace();
				}
			}
		});
		update();
	}

	private static final long serialVersionUID = 1L;

	public void update() {
		super.update();
		DefaultTableModel model = (DefaultTableModel)getModel();
		for ( TTDNode ttd : _system.getTTDNodes() ) {
			Vector<Object> param_v = new Vector<Object>();
			param_v.add(ttd.getId());
			param_v.add(ttd.isUsed());
			param_v.add(ttd.getName());
			//param_v.add(""+ttd.getHost().getProductID());
			//param_v.add(ttd.getHost().getLocation());
			for ( int i = 0; i < 10; i++ ) {
				int id = (ttd.getFTSW(i) != null)?ttd.getFTSW(i).getChannel():-1;
				param_v.add(id>=0?id:"");
			}
			model.addRow(param_v);
		}
	}

	@Override
	public void upload() throws Exception {
		_version_max++;
		_version = _version_max;
		for ( TTDNode ttd : _system.getTTDNodes() ) {
			ttd.setVersion(_version);
			RCDBManager.get().execute("insert into "+_label+"_conf ("
					+ ttd.getSQLLabels() + ") values ("
					+ ttd.getSQLValues() + "); ");
		}
	}

	@Override
	public void download() throws Exception {
		RCDBManager.get().readTTDNodeTable(_version);
	}

}
