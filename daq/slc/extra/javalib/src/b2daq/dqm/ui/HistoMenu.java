package b2daq.dqm.ui;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.JMenu;
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;

import b2daq.dqm.core.Histo;
import b2daq.dqm.graphics.GHisto;


public class HistoMenu extends JMenu {

	private static final long serialVersionUID = 1L;
	private GHisto _histo = null;
	private CanvasPanel _panel;
	
	public HistoMenu(GHisto ghisto, CanvasPanel panel) {
		super(ghisto.get().getName());
		_histo = ghisto;
		_panel = panel;
		JMenuItem item = new JMenuItem("Set Title");
		item.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				Histo histo = (Histo)_histo.get();
				String str = JOptionPane.showInputDialog( arg0.getSource() , histo.getTitle());
				if ( str != null ) {
					histo.setTitle(str);
					_panel.repaint();
				}
			}
		});
		add(item);
		JMenu menu = new FontSettingMenu("Set Font", _histo.getFont(), panel);
		add(menu);
		menu = new LineSettingMenu("Set Line", _histo.getLine(), panel);
		add(menu);
		menu = new FillSettingMenu("Set Fill", _histo.getFill(), panel);
		add(menu);
		item = new JMenuItem("Set Draw Option");
		item.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				String str = JOptionPane.showInputDialog( arg0.getSource() ,_histo.getDrawOption());
				if ( str != null ) {
					_histo.setDrawOption(str);
					_panel.repaint();
				}
			}
		});
		add(item);
	}
	
}
