package b2daq.java.io;

import java.io.BufferedReader;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.net.URL;
import java.net.URLConnection;
import java.util.ArrayList;
import java.util.LinkedHashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.Set;
import java.util.jar.JarEntry;
import java.util.jar.JarFile;

public class ConfigFile {

	private Map<String, String> _map = new LinkedHashMap<String, String>();
	private ArrayList<String> _directory_v = new ArrayList<String>();
	private String _base = "";

	public ConfigFile() {
	}

	public ConfigFile(String is) throws IOException {
		this();
		read(is);
	}

	public void read(String path) throws IOException {
		String [] param = path.split(";");
		if ( param.length > 1) {
			JarFile jarfile = new JarFile(param[0]);
			JarEntry entry = (JarEntry) jarfile.getEntry(param[1]);
			read(jarfile.getInputStream(entry));
		} else {
			if (path.startsWith("http://") || path.startsWith("https://")) {
				URL url = new URL(path);
				URLConnection conn = url.openConnection();
				read(conn.getInputStream());
			} else {
				read(new FileInputStream(path));
			}
		}
	}

	public void read(InputStream is) throws IOException {
		BufferedReader br = new BufferedReader(new InputStreamReader(is));
		String format = "";
		String line = "";
		while ((line = br.readLine()) != null) {
			if (line.length() == 0)
				continue;
			if (line.charAt(0) == '#')
				continue;
			line = line.replace("\t", " ");
			String[] str_v = line.split(" ");
			if (str_v.length < 1) continue;
			if (str_v[0].matches("!cd")) {
				format = "";
				if (str_v.length > 1) {
					for (int n = 1; n < str_v.length; n++) {
						if (str_v[n].length() > 0) {
							format = str_v[n] + "/";
							_directory_v.add(str_v[n]);
						}
					}
				}
				continue;
			}
			String name = "", value = "NULL";
			boolean hasName = false;
			for (int n = 0; n < str_v.length; n++) {
				if (str_v[n].length() > 0) {
					if (!hasName) {
						name = format + str_v[n];
						hasName = true;
					} else {
						value = str_v[n];
						break;
					}
				}
			}
			if (value.startsWith(":")) {
				if (line.indexOf("#") > line.indexOf(":") + 1)
					value = line.substring(line.indexOf(":") + 1,
							line.indexOf("#"));
				else
					value = line.substring(line.indexOf(":") + 1);
			}
			_map.put(name, value);
		}
	}

	public boolean hasKey(String key) {
		return (_map.get(_base + key) != null);
	}
	
	public int getInt(String key) throws IOException {
		try {
			return Integer.parseInt(getString(key));
		} catch (NumberFormatException e) {
			return -1;
		}
	}

	public double getDouble(String key) throws IOException {
		try {
			return Double.parseDouble(getString(key));
		} catch (NumberFormatException e) {
			return -1;
		}
	}

	public String getString(String key) throws IOException {
		String str = _map.get(_base + key);
		if (str == null)
			return "null";
		else
			return str;
	}

	public void cd() {
		_base = "";
	}

	public void cd(String base) {
		_base = base + "/";
	}

	public ArrayList<String> getDirectories() {
		return _directory_v;
	}

	public Set<String> getKeyList() {
		return _map.keySet();
	}

	public String toString() {
		Iterator<String> iter = _map.keySet().iterator();
		StringBuffer ss = new StringBuffer();
		ss.append("{\n");
		while (iter.hasNext()) {
			String key = iter.next();
			ss.append("'" + key + "': '" + _map.get(key) + "'");
			if (iter.hasNext())
				ss.append(",\n");
		}
		ss.append("\n}");
		return ss.toString();
	}

}
