package b2daq.java.ui;

import java.awt.Component;
import java.awt.Dimension;
import java.awt.GraphicsEnvironment;
import java.awt.event.ComponentAdapter;
import java.awt.event.ComponentEvent;

import javax.swing.JPanel;

public class TablePanel extends JPanel {

	private static final long serialVersionUID = 1L;
	public static final int WIDTH_MIN = (int)(GraphicsEnvironment.getLocalGraphicsEnvironment().getMaximumWindowBounds().width / 3 * 0.8);
	public static final int HEIGHT_MIN = (int)(GraphicsEnvironment.getLocalGraphicsEnvironment().getMaximumWindowBounds().height / 2 * 0.8);
	
	private Component [] _panel_v;
	private int _ncol = 0, _nrow = 0;
	private int _npanels = 0;
	private boolean _use_min = false;
	
	public TablePanel (int ncol, int nrow, boolean use_min) {
		this(ncol, nrow);
		_use_min = use_min;
	}
	
	public TablePanel (int ncol, int nrow) {
		setLayout(null);
		_panel_v = new Component[ncol * nrow];
		_ncol = ncol;
		_nrow = nrow;
		addComponentListener(new ComponentAdapter() {
			public void componentResized(ComponentEvent e) {
				setSize( new Dimension(getWidth(), getHeight()) );
			}
    	});
	}
	
	public Component add(Component panel) {
		super.add(panel);
		if ( _npanels < _panel_v.length) {
			_panel_v[_npanels] = panel;
		} 
		_npanels++;
		return panel;
	}
	
	public void setSize(Dimension dim){
		int width = dim.width;
		int height = dim.height;
		if ( _use_min ) {
			if ( WIDTH_MIN * _ncol > width ) width = WIDTH_MIN * _ncol;
			if ( HEIGHT_MIN * _nrow > height ) height = HEIGHT_MIN * _nrow;
		}
		super.setPreferredSize(new Dimension(width, height));
		super.setSize(new Dimension(width, height));
		int dw = (int)(width /((double) _ncol)); 
		int dh = (int)(height /((double) _nrow)); 
		for (int nc = 0; nc < _ncol; nc++ ) {
			for (int nr = 0; nr < _nrow; nr++ ) {
				if ( _panel_v[nr * _ncol + nc] != null ) {
					_panel_v[nr * _ncol + nc].setBounds(nc*dw, nr*dh, dw, dh);
				} 
			}
		}
	}
	public void setUseMinimum(boolean use) {
		_use_min = use;
	}
}
