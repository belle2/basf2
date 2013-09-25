package b2daq.java.ui;

import java.awt.Component;
import java.awt.Dimension;
import java.awt.GraphicsEnvironment;
import java.awt.event.ComponentAdapter;
import java.awt.event.ComponentEvent;

import javax.swing.JPanel;

import b2daq.ui.Updatable;


public class UTablePanel extends JPanel implements Updatable {

	private static final long serialVersionUID = 1L;
	
	private Component [] _panel_v;
	private int _ncol = 0, _nrow = 0;
	private int _npanels = 0;
	private boolean _use_min = false;
	private double _cel_width_ratio = 0.2;
	private double _cel_height_ratio = 0.2;
	private int _cel_width = (int)(GraphicsEnvironment.getLocalGraphicsEnvironment().getMaximumWindowBounds().width * _cel_width_ratio);
	private int _cel_height = (int)(GraphicsEnvironment.getLocalGraphicsEnvironment().getMaximumWindowBounds().height * _cel_height_ratio);
	private float [] _width_ratio;
	private float [] _height_ratio;
	
	public UTablePanel (int ncol, int nrow, boolean use_min) {
		this(ncol, nrow);
		_use_min = use_min;
		addComponentListener(new ComponentAdapter() {
			public void componentResized(ComponentEvent e) {
				setSize( new Dimension(getWidth(), getHeight()) );
			}
    	});
	}
	
	public void setWidthRatio(String str) {
		_width_ratio = new float [_ncol];
		String [] str_v = str.split(",");
		float remain = 0;
		int count = 0;
		for (int n = 0; n < _width_ratio.length; n++ ) {
			if ( n < str_v.length ) {
				try {
					_width_ratio[n] = Float.parseFloat(str_v[n]);
				} catch ( Exception e ) {
					_width_ratio[n] = 0;
				}
				remain += _width_ratio[n];
				count++;
			} else {
				_width_ratio[n] = (1 - remain) / ( _width_ratio.length - count );
			}
		}
	}
	
	public void setHeightRatio(String str) {
		_height_ratio = new float [_nrow];
		String [] str_v = str.split(",");
		float remain = 0;
		int count = 0;
		for (int n = 0; n < _height_ratio.length; n++ ) {
			if ( n < str_v.length ) {
				_height_ratio[n] = Float.parseFloat(str_v[n]);
				remain += _height_ratio[n];
				count++;
			} else {
				_height_ratio[n] = remain / ( _height_ratio.length - count );
			}
		}
	}
	
	public UTablePanel (int ncol, int nrow) {
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
	
	public void setCelWidth(int max_width) {
		_cel_width = max_width;
	}
	
	public void setCelHeight(int max_height) {
		_cel_height = max_height;
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
			if ( _cel_width * _ncol > width ) width = _cel_width * _ncol;
			if ( _cel_height * _nrow > height ) height = _cel_height * _nrow;
		}
		super.setPreferredSize(new Dimension(width, height));
		super.setSize(new Dimension(width, height));
		int dw = (int)(width /((double) _ncol)); 
		int dh = (int)(height /((double) _nrow)); 
		int x = 0;
		for (int nc = 0; nc < _ncol; nc++ ) {
			int y = 0;
			if ( _width_ratio != null ) dw = (int)(width * _width_ratio[nc]);
			for (int nr = 0; nr < _nrow; nr++ ) {
				if ( _height_ratio != null ) dh = (int)(height * _height_ratio[nr]);
				if ( _panel_v[nr * _ncol + nc] != null ) {
					_panel_v[nr * _ncol + nc].setBounds(x, y, dw,  dh);
				}
				y += dh;
			}
			x += dw;				
		}
	}
	public void setUseMinimum(boolean use) {
		_use_min = use;
	}

	public void update() {
		repaint();
	}

	public void setCelWidthRatio(double cel_width_ratio) {
		_cel_width = (int)(cel_width_ratio * GraphicsEnvironment.getLocalGraphicsEnvironment().getMaximumWindowBounds().width);
	}

	public void setCelHeightRatio(double cel_height_ratio) {
		_cel_height = (int)(cel_height_ratio * GraphicsEnvironment.getLocalGraphicsEnvironment().getMaximumWindowBounds().height);
	}

}
