package b2dqm.java.ui;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.JMenu;
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;
import javax.swing.JPopupMenu;

import b2dqm.graphics.GAxis;

public class AxisPopupMenu extends JPopupMenu {

	private static final long serialVersionUID = 1L;
	private GAxis _axis;
	private CanvasPanel _panel;

	public AxisPopupMenu(String name, GAxis axis, CanvasPanel panel) {
		_axis = axis;
		_panel = panel;
		add(name);
		JMenu menu = new AxisRangeSettingMenu("Set Range", axis, panel);
		add(menu);
		menu = new FontSettingMenu("Set Title", axis.getFont(), panel);
		JMenuItem item = new JMenuItem("Set Position");
		item.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				String str = JOptionPane.showInputDialog( arg0.getSource() , _axis.getTitlePositionString());
				if ( str != null ) {
					_axis.setTitlePosition(str);
					_panel.repaint();
				}
			}
		});
		menu.insert(item, 0);
		item = new JMenuItem("Set Text");
		item.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				String str = JOptionPane.showInputDialog( arg0.getSource() , _axis.get().getTitle());
				if ( str != null ) {
					_axis.get().setTitle(str);
					_panel.repaint();
				}
			}
		});
		menu.insert(item, 0);
		add(menu);
		menu = new FontSettingMenu("Set Labels", axis.getLabelsFont(), panel);
		item = new JMenuItem("Set Alignment");
		item.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				String str = JOptionPane.showInputDialog( arg0.getSource() , _axis.getLabelAlignment());
				if ( str != null ) {
					_axis.setLabelAlignment(str);
					_panel.repaint();
				}
			}
		});
		menu.insert(item, 0);
		add(menu);
		menu = new LineSettingMenu("Set Ticks", axis.getLine(), panel);
		item = new JMenuItem("Set Length");
		item.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				String str = JOptionPane.showInputDialog( arg0.getSource() , _axis.getTickLength());
				if ( str != null ) {
					_axis.setTickLength(Double.parseDouble(str));
					_panel.repaint();
				}
			}
		});
		menu.insert(item, 0);
		item = new JMenuItem("Set Ndivision");
		item.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				String str = JOptionPane.showInputDialog( arg0.getSource() , _axis.getNdivisions());
				if ( str != null ) {
					_axis.setNdivisions(Integer.parseInt(str));
					_panel.repaint();
				}
			}
		});
		menu.insert(item, 0);
		add(menu);
		if ( _axis.get().getMin() > 0 ) {
			if (!_axis.isLogScale()) item = new JMenuItem("Set Log Scale");
			else item = new JMenuItem("Unset Log Scale");
			item.addActionListener(new ActionListener() {
				public void actionPerformed(ActionEvent arg0) {
					_axis.setLogScale(!_axis.isLogScale());
					_panel.repaint();
				}
			});
			add(item);
		}
	}

}
