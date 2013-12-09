package b2daq.java.io;

import java.awt.Component;
import java.awt.Dimension;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.File;

import javax.swing.ButtonGroup;
import javax.swing.JButton;
import javax.swing.JFileChooser;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JRadioButton;
import javax.swing.JTextField;
import javax.swing.UIManager;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;
import javax.swing.filechooser.FileFilter;
import javax.swing.filechooser.FileNameExtensionFilter;

public class ConfigDirectory extends JPanel {

	private static final long serialVersionUID = 1L;
	private JRadioButton radio_local;
	private JRadioButton radio_web;
	private JRadioButton radio_jar;
	private JTextField field_local;
	private JTextField field_web;
	private JTextField field_jar;

	private String _local_path = "";
	private JTextField field_jar_path;
	private JButton button_local;
	private JButton button_jar;
	private String _jar_path;
	
	public ConfigDirectory() {
		this("", "", "", "", "");
	}
	
	public ConfigDirectory(String title, String local_path, String web_path, String jar_path, String config_path) {
		_local_path = local_path;
		_jar_path = jar_path;
		radio_local = new JRadioButton("from local directory");
		radio_web = new JRadioButton("from web");
		radio_jar = new JRadioButton("from jar-file");

		ButtonGroup group = new ButtonGroup();
		group.add(radio_local);
		group.add(radio_web);
		group.add(radio_jar);

		setLayout(null);
		setMaximumSize(new Dimension(240, 250));
		setPreferredSize(new Dimension(240, 250));
		JLabel title_label = new JLabel(title);
		title_label.setBounds(0, 0, 240, 25);
		title_label.setHorizontalAlignment(JLabel.CENTER);
		add(title_label);
		radio_local.setBounds(20, 30, 240, 25);
		add(radio_local);
		radio_local.addChangeListener(new ChangeListener(){
			public void stateChanged(ChangeEvent ev) {
				JRadioButton button = (JRadioButton)(ev.getSource());
				field_local.setEnabled(button.isSelected());
				button_local.setEnabled(button.isSelected());
			}
		});
		field_local = new JTextField(_local_path);
		field_local.setBounds(20, 60, 170, 25);
		add(field_local);
		button_local = new JButton("set");
		button_local.setBounds(195, 60, 40, 25);
		button_local.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				JFileChooser chooser = new JFileChooser(_local_path);
				chooser.setFileSelectionMode(JFileChooser.DIRECTORIES_ONLY);
				int selected = chooser.showOpenDialog(field_local);
				if (selected == JFileChooser.APPROVE_OPTION) {
					File file = chooser.getSelectedFile();
					field_local.setText(file.getAbsolutePath());
				}
			}
		});
		add(button_local);
		radio_web.setBounds(20, 90, 240, 25);
		add(radio_web);
		radio_web.addChangeListener(new ChangeListener(){
			public void stateChanged(ChangeEvent ev) {
				JRadioButton button = (JRadioButton)(ev.getSource());
				field_web.setEnabled(button.isSelected());
			}
		});
		field_web = new JTextField(web_path);
		field_web.setBounds(20, 120, 210, 25);
		add(field_web);
		radio_jar.setBounds(20, 150, 240, 25);
		add(radio_jar);
		radio_jar.addChangeListener(new ChangeListener(){
			public void stateChanged(ChangeEvent ev) {
				JRadioButton button = (JRadioButton)(ev.getSource());
				field_jar.setEnabled(button.isSelected());
				button_jar.setEnabled(button.isSelected());
				field_jar_path.setEnabled(button.isSelected());
			}
		});
		field_jar = new JTextField(_jar_path);
		field_jar.setBounds(20, 180, 170, 25);
		add(field_jar);
		button_jar = new JButton("set");
		button_jar.setBounds(195, 180, 40, 25);
		button_jar.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				JFileChooser chooser = new JFileChooser(_jar_path);
				chooser.setFileSelectionMode(JFileChooser.FILES_ONLY);
				FileFilter filter = new FileNameExtensionFilter("jar file", "jar");
				chooser.addChoosableFileFilter(filter);
				int selected = chooser.showOpenDialog(field_local);
				if (selected == JFileChooser.APPROVE_OPTION) {
					File file = chooser.getSelectedFile();
					field_jar.setText(file.getAbsolutePath());
				}
			}
		});
		add(button_jar);
		JLabel label_jar_path = new JLabel("path : ");
		label_jar_path.setBounds(20, 210, 40, 25);
		add(label_jar_path);
		field_jar_path = new JTextField(config_path);
		field_jar_path.setBounds(60, 210, 120, 25);
		add(field_jar_path);

		radio_local.setSelected(true);
		field_local.setEnabled(true);
		button_local.setEnabled(true);
		field_web.setEnabled(false);
		field_jar.setEnabled(false);
		button_jar.setEnabled(false);
		field_jar_path.setEnabled(false);
	}

	public JRadioButton getRadioLocal() {
		return radio_local;
	}

	public JRadioButton getRadioWeb() {
		return radio_web;
	}

	public JRadioButton getRadioJar() {
		return radio_jar;
	}

	public JTextField getLocalField() {
		return field_local;
	}

	public JTextField getWebField() {
		return field_web;
	}

	public JTextField getJarField() {
		return field_jar;
	}

	public JTextField getJarPathField() {
		return field_jar_path;
	}

	public static String showDialog(Component com, String local_path, String web_path, String jar_path) {
		return showDialog(com, "select config direcotry",local_path, web_path, jar_path);
	}
	
	public static String showDialog(Component com , String title, String local_path, String web_path, String jar_path) {
		ConfigDirectory config_panel = new ConfigDirectory(title, local_path, web_path, jar_path, "config");
		Object[] message = { config_panel };
		String[] options = { "OK" };

		int result = JOptionPane.showOptionDialog(com, message, "DC Login",
				JOptionPane.DEFAULT_OPTION, JOptionPane.PLAIN_MESSAGE, null,
				options, options[0]);
		switch (result) {
		case 0:
			if (config_panel.getRadioLocal().isSelected()) {
				return config_panel.getLocalField().getText();
			} else if (config_panel.getRadioWeb().isSelected()) {
				return config_panel.getWebField().getText();
			} else if (config_panel.getRadioJar().isSelected()) {
				return config_panel.getJarField().getText()+";"
					+config_panel.getJarPathField().getText();
			}
			break;
		default:
			break;
		}
		return "";
	}
	
	public static void main(String[] argv) {
		try {
			UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
		} catch (Exception e) {
		}
		System.out.println(ConfigDirectory.showDialog(null, "C:\\temp", "htpp://google.co.jp", ""));
	}

}
