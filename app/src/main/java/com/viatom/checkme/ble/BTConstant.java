package com.viatom.checkme.ble;

public class BTConstant {

	// Bluetooth Command length 
	public final static int WRITE_CONTENT_PKG_DATA_LENGTH = 1024;
	public final static int READ_CONTENT_ACK_DATA_LENGTH = 1024;
	public final static int COMMON_PKG_LENGTH = 8;
	public final static int COMMON_ACK_PKG_LENGTH = 12;
	public final static int READ_CONTENT_ACK_PKG_FRONT_LENGTH = 8;
	public final static int GET_INFO_ACK_PKG_LENGTH = 8 + 256;
	
	public final static int ACK_CMD_OK = 0;
	public final static int ACK_CMD_BAD = 1;
	
	// Bluetooth Max file name length
	public final static byte BT_WRITE_FILE_NAME_MAX_LENGTH = 30;
	public final static byte BT_READ_FILE_NAME_MAX_LENGTH = 30;
	
	// Bluetooth Command word
	public final static byte CMD_WORD_START_WRITE = 0x00;
	public final static byte CMD_WORD_WRITE_CONTENT = 0x01;
	public final static byte CMD_WORD_END_WRITE = 0x02;
	public final static byte CMD_WORD_START_READ = 0x03;
	public final static byte CMD_WORD_READ_CONTENT = 0x04;
	public final static byte CMD_WORD_END_READ = 0x05;
	public final static byte CMD_WORD_DEL_FILE = 0x06;
	public final static byte CMD_WORD_LIST_START = 0x07;
	public final static byte CMD_WORD_LIST_DATA = 0x08;
	public final static byte CMD_WORD_LIST_END = 0x09;
	public final static byte CMD_WORD_LANG_UPDATE_START = 0x0A;
	public final static byte CMD_WORD_LANG_UPDATE_DATA = 0x0B;
	public final static byte CMD_WORD_LANG_UPDATE_END = 0x0C;
	public final static byte CMD_WORD_APP_UPDATE_START = 0x0D;
	public final static byte CMD_WORD_APP_UPDATE_DATA = 0x0E;
	public final static byte CMD_WORD_APP_UPDATE_END = 0x0F;
	public final static byte CMD_WORD_GET_INFO = 0x14;
	public final static byte CMD_WORD_PING = 0x15;
	public final static byte CMD_WORD_PARA_SYNC = 0x16;
}
