//========================================================================
//
// Copyright (C) 2020 Matthieu Bruel <Matthieu.Bruel@gmail.com>
//
// This file is a part of scenePacker : https://github.com/mbruel/scenePacker
//
// scenePacker is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; version 3.0 of the License.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// You should have received a copy of the GNU Lesser General Public
// License along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301,
// USA.
//
//========================================================================

#ifndef CRC32_H
#define CRC32_H
#include "PureStaticClass.h"
#include <QtGlobal>

#define CRC32_BUFSIZE 65536

class Crc32 : public PureStaticClass
{
public:
    static quint32 getCRC32(const QString &filePath);

private:
    static char crc32_buf[CRC32_BUFSIZE];
    static const quint32 crc32_tab[256];
};

#endif // CRC32_H
