package b2daq.hvcontrol.ui;

import java.util.ArrayList;

import javax.swing.border.EmptyBorder;

import b2daq.nsm.NSMData;
import b2daq.core.Time;
import b2daq.database.ConfigObject;
import b2daq.dqm.core.TimedGraph1;
import b2daq.dqm.core.TimedGraph1D;
import b2daq.dqm.graphics.GTimedGraph1;
import b2daq.dqm.ui.CanvasPanel;

public class HVVoltageMonitorPanel extends CanvasPanel {

	private static final long serialVersionUID = 1L;

	private ArrayList<GTimedGraph1> m_graph_v = new ArrayList<GTimedGraph1>();
	private int m_crate = 0;
	private int m_slot = 0;

	public HVVoltageMonitorPanel(ConfigObject obj, int crate, int slot) {
		setBorder(new EmptyBorder(0, 0, 0, 0));
		getCanvas().setUseStat(false);
		m_crate = crate;
		m_slot = slot;
		for (ConfigObject cobj : obj.getObjects("channel")) {
			if (cobj.getInt("crate") == m_crate && cobj.getInt("channel") == m_slot) {
				m_graph_v.add((GTimedGraph1)getCanvas().addHisto(new TimedGraph1D("ch_"+cobj.getInt("slot"), "voltage", 600, 0, 120*60)));
			}
		}
		getCanvas().resetPadding();
	}

	public void update(ConfigObject obj, NSMData data) {
		long time = new Time().getSecond();
		getCanvas().setUpdateTime(time);
		int i = 0, j = 0;
		for (ConfigObject cobj : obj.getObjects("channel")) {
			if (cobj.getInt("crate") == m_crate && cobj.getInt("channel") == m_slot) {
				double value = data.getObject("channel_status", j).getFloat("voltage_mon");
				TimedGraph1 gr = ((TimedGraph1)(m_graph_v.get(i).getHisto()));
				gr.addPoint(time, value);
				gr.setUpdateTime(time);
				i++;
			}
			j++;
		}
		getCanvas().resetPadding();
		update();
	}

}
