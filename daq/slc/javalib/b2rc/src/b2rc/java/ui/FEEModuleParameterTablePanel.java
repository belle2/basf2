package b2rc.java.ui;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Vector;

import javax.swing.JTable;
import javax.swing.event.TableModelEvent;
import javax.swing.event.TableModelListener;
import javax.swing.table.DefaultTableModel;

import b2rc.core.COPPERNode;
import b2rc.core.FEEModule;
import b2rc.core.HSLB;
import b2rc.core.RCNodeSystem;
import b2rc.db.RCDBManager;

public class FEEModuleParameterTablePanel extends ParameterTablePanel {

	private ArrayList<FEEModule> _module_v; 
	private HashMap<Integer, Integer> _slot_m = new HashMap<Integer, Integer>();
	private HashMap<Integer, Integer> _ch_m = new HashMap<Integer, Integer>();
	
	public FEEModuleParameterTablePanel(EditorMainPanel editor_panel,VersionControlPanel version_panel, RCNodeSystem system, String label) {
		super(editor_panel, version_panel, system, label, label);
		_module_v = _system.getModules(label);
		DefaultTableModel model = (DefaultTableModel)getModel();
		model.addColumn("Host COPPER");
		FEEModule module = _module_v.get(0);
		int index = 3;
		for ( int slot = 0; slot < module.getRegisters().size(); slot++) {
			FEEModule.Register reg = module.getRegister(slot);
			for (int i = 0; i < reg.length(); i++) {
				_slot_m.put(index, slot);
				_ch_m.put(index, i);
				index++;
				if ( reg.length() == 1 ) 
					model.addColumn(reg.getName());
				else 
					model.addColumn(reg.getName() + "_" + i);
			}
		}
		_table.getModel().addTableModelListener(new TableModelListener() {
			public void tableChanged(TableModelEvent e) {
				int row = e.getFirstRow();
				int col = e.getColumn();
				if ( row <  0 || col < 0) return;
				DefaultTableModel model2 = (DefaultTableModel) getModel();
				try {
					switch (col) {
					case 0: break;
					case 1: break;
					case 2: break;
					default:
						int value = (Integer)model2.getValueAt(row, col);
						int id = (Integer)model2.getValueAt(row, 0);
						if ( id <  0 ) return;
						FEEModule module = _module_v.get(id);
						FEEModule.Register reg = module.getRegister(_slot_m.get(col));
						reg.setValue(_ch_m.get(col), value);
					}
				} catch (Exception e1) {
					e1.printStackTrace();
				}
			}
		});

		_table.setAutoResizeMode(JTable.AUTO_RESIZE_OFF);
		update();
	}

	private static final long serialVersionUID = 1L;

	public void update() {
		super.update();
		DefaultTableModel model = (DefaultTableModel)getModel();
		for ( FEEModule module : _module_v ) {
			Vector<Object> param_v = new Vector<Object>();
			param_v.add(module.getId());
			param_v.add(true);
			ArrayList<HSLB> hslb_v = _system.getHSLBs();
			HSLB host_hslb = null;
			for ( HSLB hslb : hslb_v ) {
				if ( hslb.getFEEModule() != null && 
					  module.getId() == hslb.getFEEModule().getId() ) {
					host_hslb = hslb;
					break;
				}
			}
			COPPERNode host_copper = null;
			int slot = 0;
			for ( COPPERNode copper : _system.getCOPPERNodes() ) {
				slot = 0;
				for ( ; slot < 4; slot++ ) {
					if ( copper.getHSLB(slot)!= null && 
						  host_hslb.getId() == copper.getHSLB(slot).getId() ) {
						host_copper = copper;
						break;
					}
				}
				if ( host_copper != null ) break;
			}
			if ( host_copper != null ) {
				param_v.add(host_copper.getName()+":"+(char)('a'+slot));
			} else {
				param_v.add("");
			}
			for ( slot = 0; slot < module.getRegisters().size(); slot++) {
				FEEModule.Register reg = module.getRegister(slot);
				for (int i = 0; i < reg.length(); i++) {
					param_v.add(reg.getValue(i));
				}
			}
			model.addRow(param_v);
		}
	}

	@Override
	public void upload() throws Exception {
		_version_max++;
		_version = _version_max;
		  for ( FEEModule module : _module_v ) {
			  module.setVersion(_version);
			  RCDBManager.get().execute("insert into "+_label+"_conf ("
					  + module.getSQLLabels() + ") values ("
					  + module.getSQLValues() + "); ");
		  }
	}

	@Override
	public void download() throws Exception {
		if ( _module_v == null ) _module_v = _system.getModules(_label);
		RCDBManager.get().readFEEModuleTable(getLabel(), _module_v, _version);
	}

}
