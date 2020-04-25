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

#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "ScenePacker.h"
#include <QThread>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QFileInfo>
#include <QMessageBox>
#include <QProgressBar>
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , _ui(new Ui::MainWindow),
      _progressBar(new QProgressBar(this)),
      _app(nullptr),
      _state(STATE::IDLE)
{
    _ui->setupUi(this);
    setAcceptDrops(true);

    _ui->foldersBox->setTitle(tr("Folders"));
    _ui->foldersBox->setStyleSheet(sGroupBoxStyle);

    _ui->compressionBox->setTitle(tr("Compression"));
    _ui->compressionBox->setStyleSheet(sGroupBoxStyle);

    _ui->logBox->setTitle(tr("Logs"));
    _ui->logBox->setStyleSheet(sGroupBoxStyle);

    _ui->splitter->setStyleSheet(sSplitterStyle);
    _ui->splitter->setSizes({500, 100});

    connect(_ui->dstButton,  &QAbstractButton::clicked, this, &MainWindow::onDstPath);

    connect(_ui->debugCB,    &QAbstractButton::toggled, this, &MainWindow::onDebugToggled);
    connect(_ui->dispCompressionCB, &QAbstractButton::toggled, this, &MainWindow::onDispCompressionPaths);

    connect(_ui->clearSrcButton, &QAbstractButton::clicked, _ui->srcList,    &SignedListWidget::onDeleteSelectedItems);
    connect(_ui->addSrcButton,   &QAbstractButton::clicked, this,            &MainWindow::onAddSrc);
    connect(_ui->clearLogButton, &QAbstractButton::clicked, _ui->logBrowser, &QTextBrowser::clear);

    connect(_ui->launchButton,   &QAbstractButton::clicked, this,            &MainWindow::onLaunch);

    _ui->srcList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    connect(_ui->srcList, &SignedListWidget::rightClick, this, &MainWindow::onAddSrc);

    connect(_ui->packInDestRB, &QAbstractButton::toggled, this, &MainWindow::onPackInDstFolder);
    connect(_ui->packInSrcRB,  &QAbstractButton::toggled, this, &MainWindow::onPackInSrcFolder);

    statusBar()->addPermanentWidget(_progressBar, 2);
}

MainWindow::~MainWindow()
{
    delete _ui;
}

void MainWindow::setAsciiSignature(const QString &ascii)
{
    _ui->srcList->setAsciiSignature(ascii);
}

void MainWindow::init(ScenePacker *app)
{
    _app = app;

    _ui->nbThreadsSB->setMaximum(QThread::idealThreadCount());

#ifdef __DEBUG__
    _ui->srcList->addPathIfNotInList("/tmp/testScenePacker", _ui->srcList->count(), true);
#endif

    setIDLE();
    setProgressMax(100);

    _ui->compressionBox->init(app);
    _ui->dstLE->setText(_app->dstPath());
    _ui->nbThreadsSB->setValue(_app->threads());
    _ui->debugCB->setChecked(_app->debug());
    _ui->dispCompressionCB->setChecked(_app->dispSettings());
    if (_app->useDestinationFolder())
    {
        _ui->packInDestRB->setChecked(true);
        onPackInDstFolder(true);
    }
    else
    {
        _ui->packInSrcRB->setChecked(true);
        onPackInSrcFolder(true);
    }
    onDispCompressionPaths(_app->dispSettings());

    _ui->dstChoiceBox->setStyleSheet("border: none");

    connect(_ui->aboutButton,  &QAbstractButton::clicked, app, &ScenePacker::onAbout);
    connect(_ui->donateButton, &QAbstractButton::clicked, app, &ScenePacker::onDonate);
}

void MainWindow::setIDLE()
{
    _state = STATE::IDLE;
    _ui->launchButton->setText(tr("Let's Pack!"));
}

void MainWindow::setProgressMax(int max)
{
    _progressBar->setRange(0, max);
    _progressBar->setValue(0);
}

void MainWindow::setProgress(int value)
{
    _progressBar->setValue(value);
}

void MainWindow::log(const QString &msg)
{
    _ui->logBrowser->append(msg);
}
void MainWindow::success(const QString &msg)
{
    _ui->logBrowser->append(QString("<font color='darkgreen'>%1</font>").arg(msg));
}

void MainWindow::error(const QString &msg)
{
    _ui->logBrowser->append(QString("<font color='darkred'>%1</font>").arg(msg));
}

void MainWindow::saveParams()
{
    _app->setRarCmd(_ui->compressionBox->rarPath());
    _app->setThreads(_ui->nbThreadsSB->value());
    _app->setDebug(_ui->debugCB->isChecked());
    _app->setDispSettings(_ui->dispCompressionCB->isChecked());
    _app->setUseDestinationFolder(_ui->packInDestRB->isChecked());
    if (_ui->packInDestRB->isChecked())
        _app->setDstFolder(_ui->dstLE->text());
    else
        _app->setRarFolder(_ui->dstLE->text());

    _app->saveSettings(
                _ui->compressionBox->genName(),
                _ui->compressionBox->lengthName(),
                _ui->compressionBox->genPass(),
                _ui->compressionBox->lengthPass(),
                _ui->compressionBox->useFixedPass(),
                _ui->compressionBox->fixedPass(),
                _ui->compressionBox->splitArchive(),
                _ui->compressionBox->splitSize(),
                _ui->compressionBox->addRecovery(),
                _ui->compressionBox->recoveryPct(),
                _ui->compressionBox->lockArchive(),
                _ui->compressionBox->compressLevel()
                );
}

void MainWindow::onLaunch()
{
    if (_state == STATE::IDLE)
    {
        if (!_updateParams())
            return;

        QStringList folders;
        for (int i = 0 ; i < _ui->srcList->count() ; ++i)
            folders << _ui->srcList->item(i)->text();

        if (folders.isEmpty())
            QMessageBox::warning(nullptr,
                                 tr("No source folder..."),
                                 tr("Please select some source folder(s)!"));
        else
        {
            _state = STATE::RUNNING;
            _ui->launchButton->setText(tr("Stop"));
            _app->processFolders(folders);
        }
    }
    else
        _app->stopProcessing();
}


void MainWindow::onDebugToggled(bool checked)
{
    _app->setDebug(checked);
}



void MainWindow::onDstPath()
{
    QString folder = QFileDialog::getExistingDirectory(
                this,
                tr("Select a Folder"),
                _ui->dstLE->text(),
                QFileDialog::ShowDirsOnly);

    if (!folder.isEmpty())
        _ui->dstLE->setText(folder);
}

void MainWindow::onDispCompressionPaths(bool display)
{
    _ui->compressionBox->setVisible(display);
}

void MainWindow::onPackInDstFolder(bool checked)
{
    _ui->dstLbl->setText(tr("Destination Folder:"));
    _ui->dstLE->setText(_app->dstPath());
    _ui->dstButton->show();
}

void MainWindow::onPackInSrcFolder(bool checked)
{
    _ui->dstLbl->setText(tr("Rar sub Folder:"));
    _ui->dstLE->setText(_app->rarFolder());
    _ui->dstButton->hide();
}

void MainWindow::onAddSrc()
{
    QString folder = QFileDialog::getExistingDirectory(
                this,
                tr("Select a Folder"),
                _app->srcFolder(),
                QFileDialog::ShowDirsOnly);

    if (!folder.isEmpty())
    {
        _ui->srcList->addPathIfNotInList(folder, _ui->srcList->count(), true);
        _app->setSrcFolder(folder);
    }
}


void MainWindow::dragEnterEvent(QDragEnterEvent *e)
{
    if (e->mimeData()->hasUrls())
        e->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent *e)
{
    int currentNbFiles = _ui->srcList->count();
    for (const QUrl &url : e->mimeData()->urls())
    {
        QString fileName = url.toLocalFile();
        if (QFileInfo(fileName).isDir()) // we only add folders
            _ui->srcList->addPathIfNotInList(fileName, currentNbFiles, true);
    }
}


void MainWindow::closeEvent(QCloseEvent *event)
{
    saveParams();

    if (_state == STATE::RUNNING)
    {
        int res = QMessageBox::question(this,
                                        tr("Close while still unpacking?"),
                                        tr("%1 is currently unpacking.\nAre you sure you want to quit?").arg(_app->appName()),
                                        QMessageBox::Yes,
                                        QMessageBox::No);
        if (res == QMessageBox::Yes)
        {            
            _app->stopProcessing();
            qApp->processEvents();
            event->accept();
        }
        else
            event->ignore();
    }
    else
        event->accept();
}


bool MainWindow::_updateParams()
{
    if (_ui->packInDestRB->isChecked())
    {
        if (!_app->setDstFolder(_ui->dstLE->text()))
        {
            QMessageBox::warning(nullptr,
                                 tr("dest folder not valid..."),
                                 tr("Please set the destination folder to a writable folder"));
            return false;
        }
    }
    else
    {
        if (_ui->dstLE->text().isEmpty())
        {
            QMessageBox::warning(nullptr,
                                 tr("rar sub folder empty..."),
                                 tr("Please provide the name of the RAR sub folder"));
            return false;
        }
    }

    saveParams();

    return true;
}


const QString MainWindow::sGroupBoxStyle =  "\
QGroupBox {\
        font: bold; \
        border: 1px solid silver;\
        border-radius: 6px;\
        margin-top: 6px;\
}\
QGroupBox::title {\
        subcontrol-origin:  margin;\
        left: 7px;\
        padding: 0 5px 0 5px;\
}";


const QString MainWindow::sSplitterStyle =  "\
QSplitter::handle:horizontal,\
QSplitter::handle:vertical{\
  background-color: rgb(85, 170, 255);\
    border: 1px solid #101010;\
    border-radius: 1px;\
    border-style: dotted;\
}";
