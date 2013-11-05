package b2rc.java.ui;

import java.util.Vector;

import javax.swing.event.TableModelEvent;
import javax.swing.event.TableModelListener;
import javax.swing.table.DefaultTableModel;

import b2rc.core.COPPERNode;
import b2rc.core.HSLB;
import b2rc.core.RCNodeSystem;
import b2rc.db.RCDBManager;

public class HSLBParameterTablePanel extends ParameterTablePanel {

	private COPPERParameterTablePanel _copper_panel;

	public HSLBParameterTablePanel(EditorMainPanel editor_panel,VersionControlPanel version_panel, RCNodeSystem system, COPPERParameterTablePanel copper_panel) {
		super(editor_panel, version_panel, system, "hslb", "HSLB");
		_copper_panel = copper_panel;
		DefaultTableModel model = (DefaultTableModel) getModel();
		model.addColumn("Host COPPER");
		model.addColumn("Firmware");
		model.addColumn("Data type");
		model.addColumn("FEEModule type");
		model.addColumn("FEEModule ID");
		_table.getModel().addTableModelListener(new TableModelListener() {
			public void tableChanged(TableModelEvent e) {
				int row = e.getFirstRow();
				int col = e.getColumn();
				if ( row <  0 || col < 0) return;
				DefaultTableModel model2 = (DefaultTableModel) getModel();
				try {
					int id = (Integer)model2.getValueAt(row, 0);
					if ( id <  0 ) return;
					HSLB hslb = _system.getHSLBs().get(id);
					switch (col) {
					case 1:
						hslb.setUsed((Boolean)model2.getValueAt(row, col));
						_copper_panel.update();
						break;
					case 3:
						hslb.setFirmware(model2.getValueAt(row, col).toString());
					case 4: {
						String mode_s = model2.getValueAt(row, col).toString();
						if ( mode_s.toUpperCase().matches("SIMPLE") ) {
							System.out.println(mode_s.toUpperCase());
							hslb.setTriggerMode(0x07);
						} else if ( mode_s.toUpperCase().matches("VERBOSE") ) {
							System.out.println(mode_s.toUpperCase()+" = VERBOSE");
							hslb.setTriggerMode(0x08);
						} 
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
		DefaultTableModel model = (DefaultTableModel) getModel();
		for (HSLB hslb : _system.getHSLBs()) {
			Vector<Object> param_v = new Vector<Object>();
			param_v.add(hslb.getId());
			param_v.add(hslb.isUsed());
			COPPERNode host_copper = null;
			int slot = 0;
			for (COPPERNode copper : _system.getCOPPERNodes()) {
				slot = 0;
				for (; slot < 4; slot++) {
					if (copper.getHSLB(slot) != null
							&& hslb.getId() == copper.getHSLB(slot).getId()) {
						host_copper = copper;
						break;
					}
				}
				if (host_copper != null)
					break;
			}
			if (host_copper != null) {
				param_v.add(host_copper.getName() + ":" + (char) ('a' + slot));
			} else {
				param_v.add("");
			}
			param_v.add(hslb.getFirmware());
			int trigger_mode = hslb.getTriggerMode();
			if ( trigger_mode == 0x07 ) param_v.add("SIMPLE");
			else if ( trigger_mode == 0x08 ) param_v.add("VERBOSE");
			else param_v.add("SIMPLE");
			if (hslb.getFEEModule() != null) {
				param_v.add(hslb.getFEEModule().getType());
				param_v.add("" + hslb.getFEEModule().getId());
			} else {
				param_v.add("");
				param_v.add("");
			}
			model.addRow(param_v);
		}
		if (_copper_panel != null) _copper_panel.update();
	}

	@Override
	public void upload() throws Exception {
		_version_max++;
		_version = _version_max;
		for ( HSLB hslb : _system.getHSLBs() ) {
			hslb.setVersion(_version);
		    RCDBManager.get().execute("insert into "+_label+"_conf ("
		       + hslb.getSQLLabels() + ") values ("
		       + hslb.getSQLValues() + "); ");
		}
	}

	@Override
	public void download() throws Exception {
		RCDBManager.get().readHSLBTable(_version);
	}

}
