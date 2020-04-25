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

#ifndef COMPRESSIONSETTINGS_H
#define COMPRESSIONSETTINGS_H

#include <QGroupBox>
class ScenePacker;

namespace Ui {
class CompressionSettings;
}

class CompressionSettings : public QGroupBox
{
    Q_OBJECT

public:
    explicit CompressionSettings(QWidget *parent = nullptr);
    ~CompressionSettings();

    void init(ScenePacker *app);

    QString rarPath() const;
    bool genName() const;
    int  lengthName() const;
    bool genPass() const;
    int  lengthPass() const;
    bool useFixedPass() const;
    QString fixedPass() const;
    bool splitArchive() const;
    int splitSize() const;
    bool addRecovery() const;
    int recoveryPct() const;
    bool lockArchive() const;
    int compressLevel() const;


private:
    Ui::CompressionSettings *_ui;
};



#endif // COMPRESSIONSETTINGS_H
