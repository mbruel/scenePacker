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

#include "CompressionSettings.h"
#include "ui_CompressionSettings.h"
#include "ScenePacker.h"

CompressionSettings::CompressionSettings(QWidget *parent) :
    QGroupBox(parent),
    _ui(new Ui::CompressionSettings), _app(nullptr)
{
    _ui->setupUi(this);

#if defined(__MINGW32__) || defined(__MINGW64__)
    _ui->rarPathLE->setToolTip(tr("path of Rar.exe inside WinRar directory"));
    _ui->rarPathButton->setToolTip(tr("select Rar.exe inside WinRar directory"));
#else
    _ui->rarPathLE->setToolTip(tr("path of rar executable"));
    _ui->rarPathButton->setToolTip(tr("select rar (/usr/bin/rar)"));
#endif

    connect(_ui->rarPathButton,  &QAbstractButton::clicked, this, &CompressionSettings::onRarPath);
    connect(_ui->genPassCB,      &QAbstractButton::toggled, this, &CompressionSettings::onGenPass);
    connect(_ui->fixedPassCB,    &QAbstractButton::toggled, this, &CompressionSettings::onFixedPass);
    connect(_ui->fixedPassButton,&QAbstractButton::clicked, this, &CompressionSettings::onGenFixedPass);
}

CompressionSettings::~CompressionSettings()
{
    delete _ui;
}

void CompressionSettings::init(ScenePacker *app)
{    
    _app = app;
    _ui->rarPathLE->setText(app->rarPath());
    _ui->sfvCB->setChecked(app->genSfv());
    _ui->genNameCB->setChecked(app->genName());
    _ui->nameLengthSB->setValue(app->lengthName());
    _ui->genPassCB->setChecked(app->genPass());
    _ui->passLengthSB->setValue(app->lengthPass());
    _ui->fixedPassCB->setChecked(app->useFixedPass());
    _ui->fixedPassLE->setText(app->fixedPass());
    _ui->splitCB->setChecked(app->splitArchive());
    _ui->splitSB->setValue(app->splitSize());
    _ui->redundancyCB->setChecked(app->addRecovery());
    _ui->redundancySB->setValue(app->recoveryPct());
    _ui->lockCB->setChecked(app->lockArchive());
    _ui->levelSB->setValue(app->compressLevel());
}

QString CompressionSettings::rarPath()       const { return _ui->rarPathLE->text(); }
bool    CompressionSettings::genSfv()        const { return _ui->sfvCB->isChecked(); }
bool    CompressionSettings::genName()       const { return _ui->genNameCB->isChecked(); }
int     CompressionSettings::lengthName()    const { return _ui->nameLengthSB->value(); }
bool    CompressionSettings::genPass()       const { return _ui->genPassCB->isChecked(); }
int     CompressionSettings::lengthPass()    const { return _ui->passLengthSB->value(); }
bool    CompressionSettings::useFixedPass()  const { return _ui->fixedPassCB->isChecked(); }
QString CompressionSettings::fixedPass()     const { return _ui->fixedPassLE->text(); }
bool    CompressionSettings::splitArchive()  const { return _ui->splitCB->isChecked(); }
int     CompressionSettings::splitSize()     const { return _ui->splitSB->value(); }
bool    CompressionSettings::addRecovery()   const { return _ui->redundancyCB->isChecked(); }
int     CompressionSettings::recoveryPct()   const { return _ui->redundancySB->value(); }
bool    CompressionSettings::lockArchive()   const { return _ui->lockCB->isChecked(); }
int     CompressionSettings::compressLevel() const { return _ui->levelSB->value(); }

#include <QFileDialog>
void CompressionSettings::onRarPath()
{
    QString file = QFileDialog::getOpenFileName(
                this,
            #if defined(__MINGW32__) || defined(__MINGW64__)
                tr("Select Rar.exe from WinRar directory"),
                _ui->rarPathLE->text(),
                "*.exe"
            #else
                tr("Select the rar executable"),
                _ui->rarPathLE->text()
            #endif
                );

    if (!file.isEmpty())
        _ui->rarPathLE->setText(file);
}

void CompressionSettings::onGenPass(bool checked)
{
    if (checked)
    {
        _ui->fixedPassCB->setChecked(false);
        onFixedPass(false);
    }
}

void CompressionSettings::onFixedPass(bool checked)
{
    if (checked)
        _ui->genPassCB->setChecked(false);

    _ui->fixedPassLE->setEnabled(checked);
    _ui->fixedPassButton->setEnabled(checked);
}

void CompressionSettings::onGenFixedPass()
{
    _ui->fixedPassLE->setText(_app->randomStr(_ui->passLengthSB->value()));
}
