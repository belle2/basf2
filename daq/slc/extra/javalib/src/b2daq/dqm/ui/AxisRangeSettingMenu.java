package b2daq.dqm.ui;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.JMenu;
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;

import b2daq.dqm.graphics.GAxis;


public class AxisRangeSettingMenu extends JMenu {

	private static final long serialVersionUID = 1L;
	private GAxis _axis;
	private CanvasPanel _panel;

	public AxisRangeSettingMenu(String title, GAxis axis, CanvasPanel panel) {
		super(title);
		_axis = axis;
		_panel = panel;
		JMenuItem item = new JMenuItem("Set Maximum");
		item.addActionListener(new ActionListener() {
		public void actionPerformed(ActionEvent arg0) {
			String str = JOptionPane.showInputDialog( arg0.getSource() , _axis.get().getMax());
			if ( str != null ) {
				_axis.get().fixMax(false);
				_axis.get().setMax(Double.parseDouble(str));
				_axis.get().fixMax(true);
				_panel.repaint();
			}
		}
		});
		add(item);
		item = new JMenuItem("Set Minimum");
		item.addActionListener(new ActionListener() {
		public void actionPerformed(ActionEvent arg0) {
			String str = JOptionPane.showInputDialog( arg0.getSource() , _axis.get().getMin());
			if ( str != null ) {
				_axis.get().fixMin(false);
				_axis.get().setMin(Double.parseDouble(str));
				_axis.get().fixMin(true);
				_panel.repaint();
			}
		}
		});
		add(item);
	}

}
