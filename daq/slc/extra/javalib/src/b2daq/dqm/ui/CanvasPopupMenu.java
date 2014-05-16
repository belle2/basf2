package b2daq.dqm.ui;

import java.awt.Graphics2D;
import java.awt.RenderingHints;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.image.BufferedImage;
import java.io.File;
import java.io.IOException;

import javax.imageio.ImageIO;
import javax.swing.JFileChooser;
import javax.swing.JMenu;
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;
import javax.swing.JPopupMenu;
import javax.swing.filechooser.FileFilter;
import javax.swing.filechooser.FileNameExtensionFilter;

import b2daq.dqm.graphics.Canvas;
import b2daq.dqm.io.RootFileWriter;
import b2daq.graphics.SVGCanvas;
import b2daq.io.FileDataWriter;


public class CanvasPopupMenu extends JPopupMenu {

	private static final long serialVersionUID = 1L;
	private Canvas _canvas;
	private CanvasPanel _panel;

	public CanvasPopupMenu(String name, Canvas canvas, CanvasPanel panel) {
		super(name);
		_canvas = canvas;
		_panel = panel;
		JMenu menu = new FontSettingMenu("Set Title", canvas.getFont(), panel);
		JMenuItem item = new JMenuItem("Set Position");
		item.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				String str = JOptionPane.showInputDialog( arg0.getSource() , _canvas.getTitlePosition());
				if ( str != null ) {
					_canvas.setTitlePosition(str);
					_panel.repaint();
				}
			}
		});
		menu.insert(item, 0);
		item = new JMenuItem("Set Text");
		item.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				String str = JOptionPane.showInputDialog( arg0.getSource() , _canvas.getTitle());
				if ( str != null ) {
					_canvas.setTitle(str);
					_panel.repaint();
				}
			}
		});
		menu.insert(item, 0);
		add(menu);
		menu = new LineSettingMenu("Set Line", canvas.getLine(), panel);
		add(menu);
		menu = new FillSettingMenu("Set Fill", canvas.getFill(), panel);
		add(menu);
		menu = new JMenu("Save as");
		item = new JMenuItem(".png");
		item.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				JFileChooser filechooser = new JFileChooser();
				filechooser.setDialogTitle("Save as a png file");
				FileFilter filter = new FileNameExtensionFilter("PNG file(*.png)", "png");
				filechooser.setFileFilter(filter);
				if (filechooser.showSaveDialog(_panel) == JFileChooser.APPROVE_OPTION) {
					File file = filechooser.getSelectedFile();
					try {
						BufferedImage image = _panel.getImage();
						Graphics2D g2 = image.createGraphics();
						g2.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);
						_panel.paint(g2);
						ImageIO.write(image, "png", file);
						g2.dispose();
					} catch (IOException ex) {
						ex.printStackTrace();
					}
				}
			}
		});
		menu.add(item);
		item = new JMenuItem(".svg");
		item.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				JFileChooser filechooser = new JFileChooser();
				filechooser.setDialogTitle("Save as a SVG file");
				FileFilter filter = new FileNameExtensionFilter("SVG file(*.SVG)", "SVG");
				filechooser.setFileFilter(filter);
				if (filechooser.showSaveDialog(_panel) == JFileChooser.APPROVE_OPTION) {
					File file = filechooser.getSelectedFile();
					try {
						FileDataWriter writer = new FileDataWriter(file);
						SVGCanvas svg = new SVGCanvas(_panel.getWidth(), _panel.getHeight());
						_canvas.draw(svg);
						writer.writeString(svg.toString());
					} catch (IOException ex) {
						ex.printStackTrace();
					} catch (Exception e1) {
						e1.printStackTrace();
					}
				}
			}
		});
		menu.add(item);
		item = new JMenuItem(".C");
		item.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				JFileChooser filechooser = new JFileChooser();
				filechooser.setDialogTitle("Save as a C file");
				FileFilter filter = new FileNameExtensionFilter("C file(*.C)", "C");
				filechooser.setFileFilter(filter);
				if (filechooser.showSaveDialog(_panel) == JFileChooser.APPROVE_OPTION) {
					File file = filechooser.getSelectedFile();
					try {
						FileDataWriter writer = new FileDataWriter(file);
						writer.writeString(RootFileWriter.convert(_panel));
					} catch (IOException ex) {
						ex.printStackTrace();
					} catch (Exception e1) {
						e1.printStackTrace();
					}
				}
			}
		});
		menu.add(item);
		add(menu);
	}

}
