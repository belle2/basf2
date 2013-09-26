package b2rc.java.ui;

import java.util.ArrayList;
import java.util.Vector;

import javax.swing.event.TableModelEvent;
import javax.swing.event.TableModelListener;
import javax.swing.table.DefaultTableModel;

import b2rc.core.COPPERNode;
import b2rc.core.RCNodeSystem;
import b2rc.db.RCDBManager;

public class COPPERParameterTablePanel extends ParameterTablePanel {

	private ArrayList<COPPERNode> _copper_v;
	
	public COPPERParameterTablePanel(EditorMainPanel editor_panel,
			VersionControlPanel version_panel, RCNodeSystem system) {
		super(editor_panel, version_panel, system, "copper_node", "COPPER");
		_copper_v = system.getCOPPERNodes();
		DefaultTableModel model = (DefaultTableModel)getModel();
		model.addColumn("NSM name");
		model.addColumn("Host name");
		model.addColumn("Basf2 script");
		model.addColumn("Slot:A");
		model.addColumn("Slot:B");
		model.addColumn("Slot:C");
		model.addColumn("Slot:D");
		_table.getModel().addTableModelListener(new TableModelListener() {
			public void tableChanged(TableModelEvent e) {
				int row = e.getFirstRow();
				int col = e.getColumn();
				if ( row <  0 || col < 0) return;
				DefaultTableModel model2 = (DefaultTableModel) getModel();
				try {
					int id = (Integer)model2.getValueAt(row, 0);
					if ( id <  0 ) return;
					COPPERNode copper = _copper_v.get(id);
					switch (col) {
					case 1:
						copper.setUsed((Boolean)model2.getValueAt(row, col));
						break;
					case 2:
						copper.setName(model2.getValueAt(row, col).toString());
						break;
					case 4:
						copper.getSender().setScript(model2.getValueAt(row, col).toString());
						break;
					default:
						if ( col > 4 ) {
							//int slot = col - 5;
							//copper.getHSLB(slot).setUsed(model2.getValueAt(row, col).toString().matches("true"));
						}
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
		if ( _copper_v == null ) _copper_v = _system.getCOPPERNodes();
		for ( COPPERNode copper : _copper_v ) {
			Vector<Object> param_v = new Vector<Object>();
			param_v.add(copper.getId());
			param_v.add(copper.isUsed());
			param_v.add(copper.getName());
			param_v.add(copper.getHost().getName());
			param_v.add(copper.getSender().getScript());
			for ( int slot = 0; slot < 4; slot++ ) {
				param_v.add(copper.isUsed() && copper.getHSLB(slot) != null && copper.getHSLB(slot).isUsed() );
			}
			model.addRow(param_v);
		}
	}

	@Override
	public void upload() throws Exception {
		_version_max++;
		_version = _version_max;
		for ( COPPERNode copper : _copper_v ) {
			copper.setVersion(_version);
			RCDBManager.get().execute("insert into "+_label+"_conf ("
					+ copper.getSQLLabels() + ") values ("
					+ copper.getSQLValues() + "); ");
		}
	}

	@Override
	public void download() throws Exception {
		RCDBManager.get().readCOPPERNodeTable(_version);
	}

}
