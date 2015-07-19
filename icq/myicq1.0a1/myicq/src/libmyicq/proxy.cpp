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

#include "proxy.h"
#include "icqdb.h"

ProxyInfo::ProxyInfo()
{
	port = 0;
	resolve = 0;
}

void ProxyInfo::load(DBInStream &in)
{
	in >> host >> port >> username >> passwd >> resolve;
}

void ProxyInfo::save(DBOutStream &out)
{
	out << host << port << username << passwd << resolve;
}