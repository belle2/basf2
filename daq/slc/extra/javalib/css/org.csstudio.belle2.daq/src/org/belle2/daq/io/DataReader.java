package org.belle2.daq.io;

import java.io.BufferedInputStream;
import java.io.DataInputStream;
import java.io.IOException;
import java.io.InputStream;

import org.belle2.daq.base.Reader;


public class DataReader extends Reader {

    protected DataInputStream _stream;

    public DataReader() {
    }

    public DataReader(InputStream stream) {
        _stream = new DataInputStream(new BufferedInputStream(stream));
    }

    @Override
    public boolean readBoolean() throws IOException {
        return _stream.readByte() == 1;
    }

    @Override
    public byte readByte() throws IOException {
        return _stream.readByte();
    }

    @Override
    public char readChar() throws IOException {
        return (char) _stream.readByte();
    }

    @Override
    public double readDouble() throws IOException {
        return _stream.readDouble();
    }

    @Override
    public float readFloat() throws IOException {
        return _stream.readFloat();
    }

    @Override
    public long readLong() throws IOException {
        return _stream.readLong();
    }

    @Override
    public int readInt() throws IOException {
        return _stream.readInt();
    }

    @Override
    public short readShort() throws IOException {
        return _stream.readShort();
    }

    @Override
    public String readString() throws IOException {
        int length = readInt();
        byte c;
        StringBuilder buffer = new StringBuilder();
        for (int i = 0; i < length; i++) {
            c = readByte();
            if (c == '\n' || c == '\r' || (c >= ' ' && c <= '~')) {
                buffer.append((char) c);
            } else {
                System.out.println((char) c);
            }
        }
        return buffer.toString();
    }

    public boolean available() throws IOException {
        if (_stream.available() > 0) {
            return true;
        } else {
            return false;
        }
    }

}
