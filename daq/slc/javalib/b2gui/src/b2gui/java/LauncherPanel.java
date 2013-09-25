package b2gui.java;

import java.awt.Dimension;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.net.URLDecoder;

import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JButton;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JPanel;

import b2daq.core.Log;
import b2daq.core.LogLevel;

import com.jcraft.jsch.JSch;
import com.jcraft.jsch.JSchException;
import com.jcraft.jsch.Session;
import com.jcraft.jsch.UserInfo;

public class LauncherPanel extends JPanel {

	private static final long serialVersionUID = 1L;
	private JLabel label;
	private String _host = "";
	private int _port = 0;
	private String _main_class;
	private JButton button;
	private int _count = 0;
	private String _version = "0.0.0";
		
	public LauncherPanel(String title, int port, String main_class, String version) {
		_port = port;
		_main_class  = main_class;
		_version = version;
		setLayout(new BoxLayout(this, BoxLayout.X_AXIS));
		label = new JLabel(title);
		label.setPreferredSize(new Dimension(160, 30));
		label.setMaximumSize(new Dimension(160, 30));
		add(Box.createRigidArea(new Dimension(10, 5)));
		add(label);
		add(Box.createRigidArea(new Dimension(10, 5)));
		button = new JButton("launch");
		add(button);
		button.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				new Thread() {
					public void run() {
						LauncherLoginPane login_panel = LauncherFrame.get().getLoginPanel();
						try {
							if ( login_panel.useSSH() ) {
								LauncherFrame.get().getLog().add(
										new Log("creating ssh (" + login_panel.getHostName() +
												", " + getPort() +
												") via " + login_panel.getProxyName()));
								JSch jsch = new JSch();
								Session session = jsch.getSession(
										login_panel.getAccount(),
										login_panel.getProxyName(),
										login_panel.getSSHPort());
								UserInfo ui = new KUserInfo(login_panel);
								session.setUserInfo(ui);
								session.connect();
								int port = getPort();
								int assinged_port = session.setPortForwardingL(
										port+_count, login_panel.getHostName(), port);
								LauncherFrame.get().getLog().add(
										new Log("done localhost:" + assinged_port
										+ " to " + login_panel.getHostName() + ":" + port));
								_count++;
							}
							String path = Belle2GUILauncherJavaGUI.class.getResource("Belle2GUILauncherJavaGUI.class").getPath()
							.replace("file:", "").replace("!", "")
							.replace("/b2gui/java/Belle2GUILauncherJavaGUI.class", "");
							try {
								String[] option_v = { "java", "-classpath", 
										URLDecoder.decode(path, "UTF-8"), 
										_main_class, _host, ""+getPort()};
								System.out.println("java -classpath " + URLDecoder.decode(path, "UTF-8") + " " + _main_class + " " + _host + " " + getPort());
								LauncherFrame.get().addProcess(Runtime.getRuntime().exec(option_v));
							} catch (Exception e) {
								e.printStackTrace();
							}
							LauncherFrame.get().getLog().add(new Log("started " + getTitle() + " GUI version " + _version));
						} catch (JSchException e) {
							e.printStackTrace();
							LauncherFrame.get().getLog().add(
									new Log("ssh connection boroken", LogLevel.EXCEPTION));
						}
					}
				}.start();
			}
		});
		add(Box.createRigidArea(new Dimension(10, 5)));
	}

	public void setTitle(String title) {
		label.setText(title);
	}

	public void setHostName(String host) {
		_host = host;
	}

	public String getTitle() {
		return label.getText();
	}

	public void setPort(int port) {
		_port = port;
	}

	public int getPort() {
		return _port;
	}

	public void setEnabled(boolean enabled) {
		button.setEnabled(enabled);
	}

	public static class KUserInfo implements UserInfo {

		private LauncherLoginPane _login_panel;

		public KUserInfo(LauncherLoginPane login_panel) {
			_login_panel = login_panel;
		}

		public String getPassphrase() {
			return null;
		}

		public String getPassword() {
			return _login_panel.getPassword();
		}

		public boolean promptPassphrase(String arg0) {
			return false;
		}

		public boolean promptPassword(String arg0) {
			return true;
		}

		public boolean promptYesNo(String arg0) {
			return true;
		}

		public void showMessage(String arg0) {
			JOptionPane.showMessageDialog(null, arg0);
			System.out.println(arg0);
		}

	}
}
