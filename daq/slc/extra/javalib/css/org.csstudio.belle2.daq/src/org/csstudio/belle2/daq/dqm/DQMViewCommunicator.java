package org.csstudio.belle2.daq.dqm;

import java.io.IOException;
import java.net.Socket;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

import org.belle2.daq.dqm.Histo;
import org.belle2.daq.dqm.Histo1F;
import org.belle2.daq.dqm.Histo2F;
import org.belle2.daq.io.SocketDataReader;
import org.csstudio.opibuilder.preferences.StringTableFieldEditor;
import org.eclipse.core.runtime.Platform;
import org.eclipse.core.runtime.preferences.IPreferencesService;
import org.eclipse.swt.widgets.Display;

import java.util.logging.Level;
import java.util.logging.Logger;

public class DQMViewCommunicator extends Thread {

	public static final String VERSION = "0.0.2";
	private String host = "localhost";
	private int port = 50100;

	public static final int FLAG_CONFIG = 2;
	public static final int FLAG_UPDATE = 3;

	public static final int YES = 1;
	public static final int NO = -1;

	private Socket socket = null;
	private HashMap<String, ArrayList<Histo>> pack_v = new HashMap<String, ArrayList<Histo>>();
	private static ArrayList<DQMCanvasFigure> figures = new ArrayList<>();
	private static ArrayList<DQMViewChannelHandler> channels = new ArrayList<>();

	private DQMViewCommunicator() {
	}

	private static ArrayList<DQMViewCommunicator> g_store = new ArrayList<>();
	public static ArrayList<DQMViewCommunicator> get() {
		return g_store;
	}

	public HashMap<String, ArrayList<Histo>> getHists() {
		return pack_v;
	}

	public static Histo getHisto(String dirname, String histname) {
		for (DQMViewCommunicator store : g_store) {
			if (store.pack_v != null) {
				for (String dname : store.pack_v.keySet()) {
					if (dname.equals(dirname)) {
						for (Histo h : store.pack_v.get(dname)) {
							if (h.getName().equals(histname))
								return (Histo) h;
						}
					}
				}
			}
		}
		return null;
	}

	public static void startDQM() throws Exception {
		if (g_store.isEmpty()) {
			final IPreferencesService prefs = Platform.getPreferencesService();
			final String s = prefs.getString(DQMViewDataSource.PLUGIN_ID, PreferenceConstants.DQM, "belle-dqm.desy.de,50100", null);
			synchronized (g_store) {
				List<String[]> ss = StringTableFieldEditor.decodeStringTable(s);
				for (String[] cs : ss) {
					try {
						String host = cs[0];
						int port = Integer.parseInt(cs[1]);
						DQMViewCommunicator com = new DQMViewCommunicator();
						com.init(host, port, "");
						com.start();
						g_store.add(com);
					} catch (Exception e) {
						e.printStackTrace();
					}
				}
			}
		}
	}

	public void init(String host, int port, String client) {
		this.host = host;
		this.port = port;
	}

	@Override
	public void run() {
		while (true) {
			try {
				socket = new Socket(host, port);
				SocketDataReader socket_reader = new SocketDataReader(socket);
				while (true) {
					int flag = socket_reader.readInt();
					switch (flag) {
					case FLAG_CONFIG: {
						pack_v = new HashMap<>();
						int nhists = socket_reader.readInt();
						for (int n = 0; n < nhists; n++) {
							String class_name = socket_reader.readString();
							String dir = socket_reader.readString();
							ArrayList<Histo> pack = null;
							if (pack_v.containsKey(dir)) {
								pack = pack_v.get(dir);
							} else {
								pack = new ArrayList<>();
								pack_v.put(dir, pack);
							}
							String name = socket_reader.readString();
							System.out.println("config : " + dir + " / " + name);
							String title = socket_reader.readString();
							int nlabelsx = socket_reader.readInt();
							ArrayList<String> labelx = null;
							if (nlabelsx > 0) {
								labelx = new ArrayList<>();
								for (int ix = 0; ix < nlabelsx; ix++) {
									labelx.add(socket_reader.readString());
								}
							}
							int nbinsx = socket_reader.readInt();
							double xmin = socket_reader.readDouble();
							double xmax = socket_reader.readDouble();
							// System.out.println("config : histogram " + name);
							if (class_name.contains("TH1")) {
								Histo1F h = new Histo1F(name, title, nbinsx, xmin, xmax);
								if (labelx != null) {
									h.getAxisX().setLabels(labelx);
								}
								pack.add(h);
							} else if (class_name.contains("TH2")) {
								int nlabelsy = socket_reader.readInt();
								ArrayList<String> labely = null;
								if (nlabelsx > 0) {
									labely = new ArrayList<>();
									for (int iy = 0; iy < nlabelsy; iy++) {
										labely.add(socket_reader.readString());
									}
								}
								int nbinsy = socket_reader.readInt();
								double ymin = socket_reader.readDouble();
								double ymax = socket_reader.readDouble();
								Histo2F h = new Histo2F(name, title, nbinsx, xmin, xmax, nbinsy, ymin, ymax);
								if (labelx != null) {
									h.getAxisX().setLabels(labelx);
								}
								if (labely != null) {
									h.getAxisY().setLabels(labely);
								}
								pack.add(h);
							}
							int magic = socket_reader.readInt();
							if (magic != 0x7FFF) {
								throw new IOException("Wrong magic:" + magic);
							}
						}
					}
					break;
					case FLAG_UPDATE: {
						int nhists = socket_reader.readInt();
						for (int i = 0; i < nhists; i++) {
							String histname = socket_reader.readString();
							String dirname = socket_reader.readString();
							Histo h = getHisto(dirname, histname);
							if (h == null) {
								throw new IOException("histogram not found : "+ dirname+"/"+histname);
							}
							if (h.getDim() == 1) {
								for (int nx = 0; nx < h.getAxisX().getNbins(); nx++) {
									double v = socket_reader.readFloat();
									h.setBinContent(nx, v);
								}
							} else if (h.getDim() == 2) {
								for (int ny = 0; ny < h.getAxisY().getNbins(); ny++) {
									for (int nx = 0; nx < h.getAxisX().getNbins(); nx++) {
										double v = socket_reader.readFloat();
										h.setBinContent(nx, ny, v);
									}
								}
							}
							int magic = socket_reader.readInt();
							if (magic != 0x7FFF) {
								throw new IOException(h.getName() + ": Wrong magic:" + magic);
							}
						}
					}
						break;
					}
					Display display = Display.getDefault();
					if (display != null && !display.isDisposed()) {
						display.syncExec(new Runnable() {
							public void run() {
								try {
									for (DQMCanvasFigure figure : figures) {
										figure.update();
									}
								} catch (Exception e) {
									//e.printStackTrace();
								}
							}
						});
					}
					for (DQMViewChannelHandler channel : channels) {
						channel.update();
					}
				}
			} catch (IOException e) {
				//e.printStackTrace();
				close();
			}
			try {
				Thread.sleep(10000);
			} catch (InterruptedException e) {
			}
		}
	}

	public void close() {
		try {
			socket.close();
		} catch (Exception ex) {
			Logger.getLogger(DQMViewCommunicator.class.getName()).log(Level.SEVERE, null, ex);
		}
	}

	public static void add(DQMCanvasFigure figure) {
		figures.add(figure);
	}

	public static void add(DQMViewChannelHandler channel) {
		channels.add(channel);
	}

}
