package b2rc.java.ui;

import java.util.Vector;

import javax.swing.event.TableModelEvent;
import javax.swing.event.TableModelListener;
import javax.swing.table.DefaultTableModel;

import b2rc.core.FTSW;
import b2rc.core.RCNodeSystem;
import b2rc.core.TTDNode;
import b2rc.db.RCDBManager;

public class FTSWParameterTablePanel extends ParameterTablePanel {

	private TTDParameterTablePanel _ttd_panel;

	public FTSWParameterTablePanel(EditorMainPanel editor_panel,VersionControlPanel version_panel, RCNodeSystem system, TTDParameterTablePanel ttd_panel) {
		super(editor_panel, version_panel, system, "ftsw", "FTSW");
		_ttd_panel = ttd_panel;
		DefaultTableModel model = (DefaultTableModel) getModel();
		// model.addColumn("ProductID");
		// model.addColumn("Location");
		model.addColumn("Host TTD");
		model.addColumn("Channel");
		//model.addColumn("Firmware");
		model.addColumn("Trigger mode");
		model.addColumn("Dummy rate");
		model.addColumn("Trigger limit");
		_table.getModel().addTableModelListener(new TableModelListener() {
			public void tableChanged(TableModelEvent e) {
				int row = e.getFirstRow();
				int col = e.getColumn();
				if ( row <  0 || col < 0) return;
				DefaultTableModel model2 = (DefaultTableModel) getModel();
				try {
					int id = (Integer)model2.getValueAt(row, 0);
					if ( id <  0 ) return;
					FTSW ftsw = _system.getFTSWs().get(id);
					switch (col) {
					case 1:
						ftsw.setUsed((Boolean)model2.getValueAt(row, col));
						break;
					case 3:
						try {
							ftsw.setChannel(Integer.parseInt(model2.getValueAt(row, col).toString()));
						} catch (Exception e1) {
							e1.printStackTrace();
						}
						break;
					//case 4:
					//	ftsw.setFirmware(model2.getValueAt(row, col).toString());
					//	break;
					case 4: {
						String mode_s = model2.getValueAt(row, col).toString();
						int mode = FTSW.TRIG_NORMAL;
						if ( mode_s == "IN" ) mode = FTSW.TRIG_IN;
						else if ( mode_s == "TLU" ) mode = FTSW.TRIG_TLU;
						else if ( mode_s == "PULSE" ) mode = FTSW.TRIG_PULSE;
						else if ( mode_s == "REVO" ) mode = FTSW.TRIG_REVO;
						else if ( mode_s == "RANDOM" ) mode = FTSW.TRIG_RANDOM;
						else if ( mode_s == "POSSION" ) mode = FTSW.TRIG_POSSION;
						else if ( mode_s == "ONCE" ) mode = FTSW.TRIG_ONCE;
						else if ( mode_s == "STOP" ) mode = FTSW.TRIG_STOP;
						ftsw.setTriggerMode(mode);
						break;
					}
					case 5: {
						int rate = Integer.parseInt(model2.getValueAt(row, col).toString());
						ftsw.setDummyRate(rate);
						break;
					}
					case 6: {
						int limit = Integer.parseInt(model2.getValueAt(row, col).toString());
						ftsw.setTriggerLimit(limit);
						break;
					}
					}
				} catch (Exception e1) {
				}
			}
		});
		update();
	}

	private static final long serialVersionUID = 1L;

	public void update() {
		super.update();
		DefaultTableModel model = (DefaultTableModel) getModel();
		for (FTSW ftsw : _system.getFTSWs()) {
			Vector<Object> param_v = new Vector<Object>();
			param_v.add(ftsw.getId());
			param_v.add(ftsw.isUsed());
			// param_v.add(""+ftsw.getProductID());
			// param_v.add(ftsw.getLocation());
			TTDNode host_ttd = null;
			for (TTDNode ttd : _system.getTTDNodes()) {
				for (int slot = 0; slot < 10; slot++) {
					if (ttd.getFTSW(slot) != null
							&& ftsw.getId() == ttd.getFTSW(slot).getId()) {
						host_ttd = ttd;
						break;
					}
				}
				if (host_ttd != null)
					break;
			}
			if (host_ttd != null) {
				param_v.add(host_ttd.getName());
			} else {
				param_v.add("");
			}
			param_v.add(""+ftsw.getChannel());
			param_v.add(ftsw.getFirmware());
			switch (ftsw.getTriggerMode()) {
			case FTSW.TRIG_IN: param_v.add("NORMAL");break;
			case FTSW.TRIG_TLU:param_v.add("TLU");break;
			case FTSW.TRIG_PULSE:param_v.add("PULSE");break;
			case FTSW.TRIG_REVO:param_v.add("REVO");break;
			case FTSW.TRIG_RANDOM:param_v.add("RANDOM");break;
			case FTSW.TRIG_POSSION:param_v.add("POSSION");break;
			case FTSW.TRIG_ONCE:param_v.add("ONCE");break;
			case FTSW.TRIG_STOP:param_v.add("STOP");break;
			default : param_v.add("NORMAL");break;
			}
			param_v.add(ftsw.getDummyRate());
			param_v.add(ftsw.getTriggerLimit());
			model.addRow(param_v);
		}
		if ( _ttd_panel != null ) _ttd_panel.update();
	}

	@Override
	public void upload() throws Exception {
		_version_max++;
		_version = _version_max;
		for (FTSW ftsw : _system.getFTSWs()) {
			ftsw.setVersion(_version);
			RCDBManager.get().execute(
					"insert into "+_label+"_conf (" + ftsw.getSQLLabels()
							+ ") values (" + ftsw.getSQLValues() + "); ");
		}
	}

	@Override
	public void download() throws Exception {
		RCDBManager.get().readFTSWTable(_version);
	}

}
