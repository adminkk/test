/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright            : (C) 2002 by Zhang Yong                         *
 *   email                : z-yong163@163.com                              *
 ***************************************************************************/

#include "stdafx.h"
#include "filesession.h"
#include "packet.h"

enum {
	TCP_FILE_INFO = 0x1000,
	TCP_FILE_RECEIVE,
	TCP_FILE_DATA,
};


FileSession::FileSession(TcpSessionBase *s)
{
	tcp = s;
	listener = NULL;
	bytesSent = 0;
	file = NULL;
	isSend = s->isSendSession();
}

FileSession::~FileSession()
{
	if (file)
		fclose(file);
	if (tcp)
		tcp->destroy();
}

void FileSession::sendFileInfo(const char *path, const char *name, uint32 size)
{
	fileSize = size;

	OutPacket *out = tcp->createPacket(TCP_FILE_INFO);
	*out << name << size;
	tcp->sendPacket(out);

	file = fopen(path, "rb");
}

void FileSession::onFileInfo(InPacket &in)
{
	if (!listener)
		return;

	string name;
	in >> name >> fileSize;
	const char *path = listener->getPathName(name.c_str(), fileSize);

	if (path) {
		OutPacket *out = tcp->createPacket(TCP_FILE_RECEIVE);
		tcp->sendPacket(out);

		file = fopen(path, "wb");
	}
}

void FileSession::onFileReceive(InPacket &in)
{
	if (listener)
		listener->onFileReceive();
	tcp->enableWrite();
}

void FileSession::onFileData(InPacket &in)
{
	int n;
	const char *data = in.readData(n);
	n = fwrite(data, 1, n, file);

	bytesSent += n;
	if (listener)
		listener->onFileProgress(bytesSent);
}

bool FileSession::onPacketReceived(InPacket &in)
{
	uint16 cmd = in.getCmd();

	switch (cmd) {
	case TCP_FILE_INFO:
		onFileInfo(in);
		break;

	case TCP_FILE_RECEIVE:
		onFileReceive(in);
		break;

	case TCP_FILE_DATA:
		onFileData(in);
		break;
	}
	return true;
}

void FileSession::onSend()
{
	if (!isSend)
		return;

	if (file) {
		char buf[2048];
		int n = fread(buf, 1, sizeof(buf), file);

		OutPacket *out = tcp->createPacket(TCP_FILE_DATA);
		out->writeData(buf, n);
		if (tcp->sendPacket(out)) {
			bytesSent += n;
			if (listener)
				listener->onFileProgress(bytesSent);
			if (bytesSent >= fileSize) {
				fclose(file);
				file = NULL;
			}
		} else {
			fclose(file);
			file = NULL;
		}
	}

	if (!file) {
		tcp->shutdown();
		if (listener)
			listener->onFileFinished();
	} else
		tcp->enableWrite();
}

void FileSession::onClose()
{
	if (file) {
		fclose(file);
		file = NULL;
	}
	if (listener)
		listener->onFileFinished();
}