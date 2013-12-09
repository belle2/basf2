package b2daq.java.io;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;

public class FileDataReader extends DataReader {

	public FileDataReader() {
		super();
	}
	
	public FileDataReader(String path) throws FileNotFoundException {
		super(new FileInputStream(path));
	}
	
	public FileDataReader(File path) throws FileNotFoundException {
		super(new FileInputStream(path));
	}

	public void close() throws IOException {
		_stream.close();
	}
	
}
