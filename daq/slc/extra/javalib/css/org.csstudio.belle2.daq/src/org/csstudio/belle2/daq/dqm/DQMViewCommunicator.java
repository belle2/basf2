package org.csstudio.belle2.daq.dqm;

import org.belle2.daq.base.Reader;
import java.io.IOException;
import java.net.Socket;
import java.util.ArrayList;

import org.belle2.daq.dqm.Histo;
import org.belle2.daq.dqm.Histo1F;
import org.belle2.daq.dqm.Histo2F;
import org.belle2.daq.dqm.HistoPackage;
import org.belle2.daq.dqm.MonObject;
import org.belle2.daq.dqm.io.PackageInfo;
import org.belle2.daq.io.SocketDataReader;
import org.belle2.daq.io.ZlibInflater;
import org.csstudio.belle2.daq.nsm.PreferenceConstants;
import org.eclipse.core.runtime.Platform;
import org.eclipse.core.runtime.preferences.IPreferencesService;
import org.eclipse.swt.widgets.Display;

import java.util.logging.Level;
import java.util.logging.Logger;
import java.util.zip.DataFormatException;

public class DQMViewCommunicator extends Thread {

	public static final String VERSION = "0.0.2";
	private String host = "localhost";
	private int port = 50100;

	public static final int FLAG_LIST = 1;
	public static final int FLAG_CONFIG = 2;
	public static final int FLAG_UPDATE = 3;

	public static final int YES = 1;
	public static final int NO = -1;

	private Socket socket = null;
	private ArrayList<HistoPackage> _pack_v = null;
	private ArrayList<PackageInfo> _info_v = null;
	private int _expno = 0;
	private int _runno = 0;
	private static ArrayList<DQMCanvasFigure> figures = new ArrayList<>();
	private static ArrayList<DQMViewChannelHandler> channels = new ArrayList<>();

	private DQMViewCommunicator() {
	}

	private static DQMViewCommunicator g_store;

	public static ArrayList<HistoPackage> getHistoPackages() {
		if (g_store != null)
			return g_store._pack_v;
		return null;
	}

	public static Histo getHisto(String packname, String hname) {
		if (g_store != null && g_store._pack_v != null) {
			for (HistoPackage pack : g_store._pack_v) {
				if (packname.equals(pack.getName())) {
					for (MonObject h : pack.getMonObjects()) {
						if (h instanceof Histo) {
							if (h.getName().equals(hname))
								return (Histo) h;
						}
					}
				}
			}
		}
		return null;
	}

	public static ArrayList<PackageInfo> getPackageInfos() {
		if (g_store != null)
			return g_store._info_v;
		return null;
	}

	public static int getExpNumber() {
		if (g_store != null)
			return g_store._expno;
		return -1;
	}

	public static int getRunNumber() {
		if (g_store != null)
			return g_store._runno;
		return -1;
	}

	public static void startDQM() {
		if (g_store == null) {
			g_store = new DQMViewCommunicator();
			final IPreferencesService prefs = Platform.getPreferencesService();
			final String host = prefs.getString(DQMViewDataSource.PLUGIN_ID, PreferenceConstants.HOST, "localhost", null);
			final Integer port = prefs.getInt(DQMViewDataSource.PLUGIN_ID, PreferenceConstants.PORT, 9090, null);
			g_store.init(host, port, "");
			g_store.start();
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
				// SocketDataWriter socket_writer = new SocketDataWriter(socket);
				SocketDataReader socket_reader = new SocketDataReader(socket);
				ZlibInflater inflater = new ZlibInflater();
				// boolean configured = false;
				while (true) {
					int flag = socket_reader.readInt();
					switch (flag) {
					case FLAG_LIST: {
						_pack_v = new ArrayList<>();
						int npacks = socket_reader.readInt();
						System.out.println("npacks=" + npacks);
						for (int n = 0; n < npacks; n++) {
							String name = socket_reader.readString();
							HistoPackage pack = new HistoPackage(name);
							_pack_v.add(pack);
						}
						int magic = socket_reader.readInt();
						if (magic != 0x7FFF) {
							throw new IOException("Wrong magic:" + magic);
						}
					}
						break;
					case FLAG_CONFIG: {
						int npacks = socket_reader.readInt();
						System.out.println("config : # of packages =" + npacks);
						_pack_v = new ArrayList<>();
						_info_v = new ArrayList<>();
						for (int n = 0; n < npacks; n++) {
							String name = socket_reader.readString();
							HistoPackage pack = new HistoPackage(name);
							int nhists = socket_reader.readInt();
							System.out.println("config : # of histograms for " + name + " : " + nhists);
							for (int i = 0; i < nhists; i++) {
								String class_name = socket_reader.readString();
								String dir = socket_reader.readString();
								name = socket_reader.readString();
								if (dir.isEmpty()) {
									dir = pack.getName() + ":default";
								}
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
									pack.addHisto(dir, h);
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
									pack.addHisto(dir, h);
								}
								int magic = socket_reader.readInt();
								if (magic != 0x7FFF) {
									throw new IOException("Wrong magic:" + magic);
								}
							}
							int magic = socket_reader.readInt();
							if (magic != 0x7FFF) {
								throw new IOException("Wrong magic:" + magic);
							}
							_pack_v.add(pack);
							PackageInfo info = new PackageInfo();
							_info_v.add(info);
						}
						int magic = socket_reader.readInt();
						if (magic != 0x7FFF) {
							throw new IOException("Wrong magic:" + magic);
						}
						// configured = true;
					}
						break;
					case FLAG_UPDATE: {
						int expno = socket_reader.readInt();
						int runno = socket_reader.readInt();
						// int stateno =
						socket_reader.readInt();
						if (_expno != expno || _runno != runno) {
							_expno = expno;
							_runno = runno;
							for (HistoPackage pack : _pack_v) {
								pack.reset();
							}
						}
						while (true) {
							int n = socket_reader.readInt();
							if (n < 0) {
								break;
							}
							inflater.readBuffer(socket_reader);
							HistoPackage pack = _pack_v.get(n);
							readContents(inflater, pack);
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
									e.printStackTrace();
								}
							}
						});
					}
					for (DQMViewChannelHandler channel : channels) {
						channel.update();
					}
				}
			} catch (IOException | DataFormatException e) {
				e.printStackTrace();
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
		} catch (IOException ex) {
			Logger.getLogger(DQMViewCommunicator.class.getName()).log(Level.SEVERE, null, ex);
		}
	}

	private void readContents(Reader reader, HistoPackage pack) throws IOException {
		String name = reader.readString();
		if (!name.matches(pack.getName())) {
			throw new IOException("Wrong package name :" + name + " for " + pack.getName());
		}
		int nhists = reader.readInt();
		for (int i = 0; i < nhists; i++) {
			name = reader.readString();
			Histo h = (Histo) pack.getHisto(i);
			Histo h_diff = (Histo) pack.getHisto(h.getName() + ":diff");
			Histo h_tmp = (Histo) pack.getHisto(h.getName() + ":tmp");
			if (h.getDim() == 1) {
				for (int nx = 0; nx < h.getAxisX().getNbins(); nx++) {
					double v = reader.readFloat();
					h.setBinContent(nx, v);
				}
			} else if (h.getDim() == 2) {
				for (int ny = 0; ny < h.getAxisY().getNbins(); ny++) {
					for (int nx = 0; nx < h.getAxisX().getNbins(); nx++) {
						double v = reader.readFloat();
						h.setBinContent(nx, ny, v);
					}
				}
			}
			// System.out.println(pack.getName() + ":" + h.getName() + " : " +
			// h.getEntries());
			if (h_tmp != null) {
				double entries = h_tmp.getEntries();
				if (h.getEntries() != entries) {
					h_diff.reset();
					h_diff.add(h_tmp, -1);
					h_diff.add(h);
					h_tmp.reset();
					h_tmp.add(h);
				} else {
					h_diff.reset();
					h_tmp.reset();
					h_tmp.add(h);
				}
			} else {
			}
			int magic = reader.readInt();
			if (magic != 0x7FFF) {
				throw new IOException(h.getName() + ": Wrong magic:" + magic);
			}
		}
		int magic = reader.readInt();
		if (magic != 0x7FFF) {
			throw new IOException("Wrong magic:" + magic);
		}
	}

	public static void add(DQMCanvasFigure figure) {
		figures.add(figure);
	}

	public static void add(DQMViewChannelHandler channel) {
		channels.add(channel);
	}

}
