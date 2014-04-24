package b2daq.java.ui;

import java.awt.Component;
import java.awt.Dimension;
import java.awt.event.ComponentAdapter;
import java.awt.event.ComponentEvent;
import java.util.ArrayList;

import javax.swing.JPanel;
import javax.swing.JScrollPane;

import b2daq.ui.Updatable;


public class UScrollPanel extends JScrollPane implements Updatable {

	private static final long serialVersionUID = 1L;
	
	private ArrayList<Updatable> _updatable_v = new ArrayList<Updatable>();
	private Component _panel = new JPanel();
	
	public UScrollPanel() {
		addComponentListener(new ComponentAdapter() {
			public void componentResized(ComponentEvent e) {
				setSize(new Dimension(getWidth(), getHeight()));
				repaint();
			}
    	});
		setVerticalScrollBarPolicy(JScrollPane.VERTICAL_SCROLLBAR_ALWAYS);
	}
	
	public void setViewportView(Component panel) {
		super.setViewportView(panel);
		_panel = panel;
	}
	
	public void setSize(Dimension dim) {
		int width = dim.width;
		int height = dim.height;
		_panel.setSize(new Dimension(width - (int)(this.getVerticalScrollBar().getWidth()*1.2), height));
		super.setPreferredSize(null);
	}

	public void addChild(Updatable obj) {
		_updatable_v.add(obj);
	}
	
	public void update() {
		for ( Updatable obj : _updatable_v ) {
			obj.update();
		}
	}
	
}
