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

#include "ScenePacker.h"
#include "MainWindow.h"
#include "About.h"
#include <QApplication>
#include <QProcess>
#include <QThread>
#include <QCommandLineParser>
#include <QDir>
#include <QTime>
#include <cmath>
#include <QSettings>
#include <QDebug>
#include <QDesktopServices>
#include <QUrl>

const QString ScenePacker::sDonationURL = "https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=W2C236U6JNTUA&item_name=scenePacker&currency_code=EUR";

const QMap<ScenePacker::Param, QString> ScenePacker::sParamNames =
{
    {Param::CmdRar,        "cmdRar"},
    {Param::SrcFolder,     "srcFolder"},
    {Param::DstPath,       "dstPath"},
    {Param::DstChoice,     "dstChoice"},
    {Param::RarFolder,     "rarFolder"},
    {Param::Threads,       "threads"},
    {Param::RarPrefix,     "rarPrefix"},
    {Param::GenName,       "genName"},
    {Param::LengthName,    "lengthName"},
    {Param::GenPass,       "genPass"},
    {Param::LengthPass,    "lengthPass"},
    {Param::UseFixedPass,  "useFixedPass"},
    {Param::FixedPass,     "pass"},
    {Param::AddRecovery,   "addRecovery"},
    {Param::RecoveryPct,   "recPct"},
    {Param::SplitArchive,  "split"},
    {Param::SplitSize,     "volSize"},
    {Param::LockArchive,   "lock"},
    {Param::CompressLevel, "compressLevel"},

    {Param::Debug,         "debug"},
    {Param::DispSettings,  "dispPaths"},
    {Param::LogPerRun,     "logPerRun"},
    {Param::Help,          "help"},
    {Param::Version,       "version"}
};

const QList<QCommandLineOption> ScenePacker::sCmdOptions = {
    {{"h", sParamNames[Param::Help]},        tr("Help: display syntax")},
    {{"v", sParamNames[Param::Version]},     tr("app version")},
    {{"i", "input"},                         tr("source folder (can use several -i)"), "input"},
    {{"o", sParamNames[Param::DstPath]},     tr("destination folder"), sParamNames[Param::DstPath]},
    {{"x", sParamNames[Param::RarFolder]},   tr("name of the rar folder where the archives will be created within each source folder"), sParamNames[Param::RarFolder]},
    {{"d", sParamNames[Param::Debug]},       tr("display debug information")},
    {{"t", sParamNames[Param::Threads]},     tr("number of threads (compression in parallel)"), sParamNames[Param::Threads]},
    {{"p", sParamNames[Param::FixedPass]},   tr("use a fixed password for all archives"), sParamNames[Param::FixedPass]},
    {{"s", sParamNames[Param::SplitSize]},   tr("split archive into volume of that size (in MB)"), sParamNames[Param::SplitSize]},
    {{"r", sParamNames[Param::RecoveryPct]}, tr("percentage of recovery records to add to the archives (Winrar -rr option)"), sParamNames[Param::RecoveryPct]},
    {{"l", sParamNames[Param::LockArchive]}, tr("lock archives (Winrar -k option)")},
    { sParamNames[Param::GenName],           tr("generate random name for each archive")},
    { sParamNames[Param::GenPass],           tr("generate random password for each archive")},
    { sParamNames[Param::LengthName],        tr("length of the random name"), sParamNames[Param::LengthName]},
    { sParamNames[Param::LengthPass],        tr("length of the random password"), sParamNames[Param::LengthPass]}
};


const QStringList ScenePacker::sRarDefaultArgs = {"a", "-ep1"};


ScenePacker::ScenePacker(int &argc, char *argv[]):
    QObject(), CmdOrGuiApp (argc, argv),
    _dstDir(nullptr),
    _cout(stdout), _cerr(stderr),
    _extProcs(),
    _entriesToCompress(),
    _nbTotal(0), _nbCompressed(0),
    _timeStart(),
    _settings(nullptr),
    _stopProcess(false),
    _logFile(nullptr), _logStream(),
    _useWinrar(false),
    _logPerRun(false)
{
#if defined(__MINGW32__) || defined(__MINGW64__)
    _settings = new QSettings(QString("%1.ini").arg(appName()), QSettings::Format::IniFormat);
#else
    _settings = new QSettings(QSettings::NativeFormat, QSettings::UserScope, sAppName, sVersion);
#endif

    if (!_settings->value(sParamNames[Param::CmdRar]).isValid())
    {
#if defined(WIN32) || defined(__MINGW64__)
//        _settings->setValue(sParamNames[Param::CmdRar], "select 'Rar.exe' inside Winrar directory");
#else
        _settings->setValue(sParamNames[Param::CmdRar], "/usr/bin/rar");
#endif

//        setDstFolder("./dst");
        saveSettings();
        setThreads(QThread::idealThreadCount()/2);
        setRarFolder("_rar");
        setRarPrefix("RAR_");
    }

    if (_settings->value(sParamNames[Param::LogPerRun]).isValid())
        _logPerRun = _settings->value(sParamNames[Param::LogPerRun]).toBool();

    if (_logPerRun && !QFileInfo(sLogFolder).exists())
        QDir(".").mkdir(sLogFolder);


    if (_hmi)
    {
        _hmi->setWindowTitle(QString("%1 v%2 - %3").arg(sAppName).arg(sVersion).arg(sDesc));
        _hmi->setWindowIcon(QIcon(":/icons/appIcon.png"));
        _hmi->setAsciiSignature(sASCII);
    }
}

ScenePacker::~ScenePacker()
{
    _clear();

    _settings->sync();
    delete _settings;

    if (_dstDir)
        delete _dstDir;
}


bool ScenePacker::parseCommandLine(int argc, char *argv[])
{
    QString appVersion = QString("%1_v%2").arg(sAppName).arg(sVersion);
    QCommandLineParser parser;
    parser.setApplicationDescription(appVersion);
    parser.addOptions(sCmdOptions);


    // Process the actual command line arguments given by the user
    QStringList args;
    for (int i = 0; i < argc; ++i)
        args << argv[i];

    bool res = parser.parse(args);
#ifdef __DEBUG__
    qDebug() << "args: " << args
             << "=> parsing: " << res << " (error: " << parser.errorText() << ")";
#endif


    if (!parser.parse(args))
    {
        _error(tr("Error syntax: %1\nTo list the available options use: %2 --help\n").arg(parser.errorText()).arg(argv[0]));
        return false;
    }

    if (parser.isSet(sParamNames[Param::Help]))
    {
        _showVersionASCII();
        _syntax(argv[0]);
        return false;
    }

    if (parser.isSet(sParamNames[Param::Version]))
    {
        _showVersionASCII();
        return false;
    }

    if (!parser.isSet("input"))
    {
        _error(tr("you need to provide at least one input folder..."));
        return false;
    }

    if (!parser.isSet(sParamNames[Param::DstPath]) && !parser.isSet(sParamNames[Param::RarFolder]))
    {
        _error(tr("you need to provide either a destination folder (-o) or the name of the rar folders (-x)"));
        return false;
    }

    setDebug(parser.isSet(sParamNames[Param::Debug]));

    if (parser.isSet(sParamNames[Param::DstPath]))
    {
        if (!setDstFolder(parser.value(sParamNames[Param::DstPath])))
        {
            _error(tr("Please provide a writable directory for the destination folder"));
            return false;
        }
    }
    else if (parser.isSet(sParamNames[Param::RarFolder]))
        setRarFolder(parser.value(sParamNames[Param::RarFolder]));


    _settings->setValue(sParamNames[Param::GenName], parser.isSet(sParamNames[Param::GenName]));
    _settings->setValue(sParamNames[Param::GenPass], parser.isSet(sParamNames[Param::GenPass]));
    _settings->setValue(sParamNames[Param::LockArchive], parser.isSet(sParamNames[Param::LockArchive]));

    if (parser.isSet(sParamNames[Param::FixedPass]))
    {
        _settings->setValue(sParamNames[Param::UseFixedPass], true);
        _settings->setValue(sParamNames[Param::FixedPass], parser.value(sParamNames[Param::FixedPass]));
    }
    else
        _settings->setValue(sParamNames[Param::UseFixedPass], false);

    bool ok = false;
    if (parser.isSet(sParamNames[Param::LengthName]))
    {
        int nb = parser.value(sParamNames[Param::LengthName]).toInt(&ok);
        if (ok)
            _settings->setValue(sParamNames[Param::LengthName], nb);
        else
        {
            _error(tr("you should provide a integer for the length name"));
            return false;
        }
    }
    if (parser.isSet(sParamNames[Param::LengthPass]))
    {
        int nb = parser.value(sParamNames[Param::LengthPass]).toInt(&ok);
        if (ok)
            _settings->setValue(sParamNames[Param::LengthPass], nb);
        else
        {
            _error(tr("you should provide a integer for the length pass"));
            return false;
        }
    }
    if (parser.isSet(sParamNames[Param::SplitSize]))
    {
        int nb = parser.value(sParamNames[Param::SplitSize]).toInt(&ok);
        if (ok)
            _settings->setValue(sParamNames[Param::SplitSize], nb);
        else
        {
            _error(tr("you should provide a integer for size of the archive volumes"));
            return false;
        }
    }
    _settings->setValue(sParamNames[Param::SplitArchive], parser.isSet(sParamNames[Param::SplitSize]));
    _settings->setValue(sParamNames[Param::AddRecovery],  parser.isSet(sParamNames[Param::RecoveryPct]));
    if (parser.isSet(sParamNames[Param::RecoveryPct]))
    {
        int nb = parser.value(sParamNames[Param::RecoveryPct]).toInt(&ok);
        if (ok)
            _settings->setValue(sParamNames[Param::RecoveryPct], nb);
        else
        {
            _error(tr("you should provide a integer for the percentage of recovery records"));
            return false;
        }
    }

    if (parser.isSet(sParamNames[Param::Threads]))
    {
        int nb = parser.value(sParamNames[Param::RecoveryPct]).toInt(&ok);
        if (ok)
            setThreads(nb);
        else
        {
            _error(tr("you should provide a integer for the number of threads"));
            return false;
        }
    }


    QStringList srcFolders;
    for (const QString &path : parser.values("input"))
    {
        QFileInfo fi(path);
        if (fi.exists() && fi.isDir() && fi.isReadable())
            srcFolders << path;
        else
        {
            _error(tr("the input '%1' is not a readable directory...").arg(path));
            return false;
        }
    }

    processFolders(srcFolders);
    return true;
}

int ScenePacker::startHMI()
{
    _hmi->init(this);
    _hmi->show();
    return _app->exec();
}

void ScenePacker::processFolders(const QStringList &srcFolders)
{
    _stopProcess = false;

    QIODevice::OpenMode openMode = QIODevice::WriteOnly|QIODevice::Text;

    QString logFileName;
    if (_logPerRun)
        logFileName = QString("./%1/%2_%3.csv").arg(
                    sLogFolder).arg(
                    sAppName).arg(
                    QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));
    else
    {
        logFileName = QString("./%1_history.csv").arg(sAppName);
        openMode |= QIODevice::Append;
    }

    bool logFileExists = QFileInfo(logFileName).exists();
    _logFile = new QFile(logFileName);
    if (_logFile->open(openMode))
    {
        _logStream.setDevice(_logFile);
        if (_logPerRun)
            _logStream << "source;destination;archive name;password\n" << flush;
        else if (!logFileExists)
            _logStream << "date;source;destination;archive name;password\n" << flush;
    }
    else
    {
        _error("Issue creating log file...");
        return;
    }

    _timeStart.start();

    _entriesToCompress.clear();
    bool isDebug = debug();
    QString rarSubFolder = rarFolder();
    bool    useRarFolder = !useDestinationFolder();
    for (const QString &srcFolder : srcFolders)
    {
        if (useRarFolder && !_setRarFolder(srcFolder))
        {
            _error(tr("Couldn't create rar folder in: %1").arg(srcFolder));
            continue;
        }

        QDir dir(srcFolder);
        for (const auto &fi : dir.entryInfoList(
                 QDir::Dirs|QDir::Files|QDir::Readable|QDir::Hidden|QDir::NoDotAndDotDot|QDir::NoSymLinks,
                 QDir::Name|QDir::DirsFirst))
        {
            if (_entryExistInDstFolder(fi))
            {
                if (isDebug)
                    _error(tr("skip %1 has it is already present in destination folder").arg(fi.fileName()));
            }
            else if (fi.fileName() != rarSubFolder)
                _entriesToCompress << fi;
        }

    }
    _nbCompressed = 0;
    _nbTotal      = _entriesToCompress.size();


    if (_hmi)
        _hmi->setProgressMax(_nbTotal);


    if (_nbTotal > 0)
    {
        int nbThreads = std::min(threads(), _nbTotal);
        _extProcs.reserve(nbThreads);
        _log(tr("<b>There are %1 items to compress using %2 threads</b>").arg(_nbTotal).arg(nbThreads));
        for (int i = 0 ; i < nbThreads ; ++i)
        {
            QProcess *extProc = new QProcess();
            connect(extProc, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
                    this, &ScenePacker::onProcFinished, Qt::QueuedConnection); // queued to avoid stack overflow

            _extProcs << extProc;
            _processNextFolder(extProc);
        }
    }
    else
    {
        _log(tr("<b>There are no items to compress...</b>"));
        if (_hmi)
        {
            _hmi->setProgressMax(1);
            _hmi->setProgress(0);
            _hmi->setIDLE();
        }
        else
            qApp->quit();
    }

}



void ScenePacker::stopProcessing()
{
    _stopProcess = true;
    for (QProcess *extProc : _extProcs)
    {
        if (extProc->state()!= QProcess::NotRunning)
            extProc->terminate();
    }

    if (_hmi)
        _error(tr("Job stopped with %1 0days extracted").arg(_nbCompressed));
}




void ScenePacker::_log(const QString &msg, bool success)
{
    _cout << msg << endl << flush;
    if (_hmi)
    {
        if (success)
            _hmi->success(msg);
        else
            _hmi->log(msg);
    }
}


void ScenePacker::_error(const QString &msg)
{
    _cerr << msg << endl << flush;
    if (_hmi)
        _hmi->error(msg);
}




void ScenePacker::_clear()
{
    if (_logFile)
    {
        _logStream.setDevice(nullptr);
        _logFile->close();
        delete _logFile;
        _logFile = nullptr;
    }

    for (QProcess *extProc : _extProcs)
    {
        if (extProc->state()!= QProcess::NotRunning)
        {
            extProc->terminate();
            extProc->waitForFinished();
        }
    }
    qDeleteAll(_extProcs);
    _extProcs.clear();
}


void ScenePacker::onAbout()
{
    About about(this);
    about.exec();
}


void ScenePacker::onDonate()
{
    QDesktopServices::openUrl(sDonationURL);
}

bool ScenePacker::_allProcessesDone() const
{
    for (QProcess *extProc: _extProcs)
    {
        if (extProc->state() != QProcess::NotRunning)
            return false;
    }
    return true;
}

void ScenePacker::_processNextFolder(QProcess *extProc)
{
    if (_stopProcess || _entriesToCompress.isEmpty())
    {
        if (_allProcessesDone())
        {
            _clear();
            _logTimeElapsed();
            if (_hmi)
            {
                _hmi->setProgress(_nbCompressed);
                _hmi->setIDLE();
            }
            else
                qApp->quit();
        }
    }
    else
    {
        // 0.: Get the entry (file or folder) and create the destination folder
        QFileInfo fi = _entriesToCompress.dequeue();
        QString dstFolder(_dstFolderForEntry(fi));
        if (!useDestinationFolder() && !_setRarFolder(fi.absolutePath()))
        {
            _error(tr("Couldn't create rar folder in: %1").arg(fi.absolutePath()));
            _processNextFolder(extProc);
        }
        if (!_dstDir->mkdir(dstFolder))
        {
            _error(tr("Issue creating dst folder: %1").arg(dstFolder));
            _processNextFolder(extProc);
        }


        QStringList args = sRarDefaultArgs;
        if (_useWinrar)
            args << "-ibck"; // to avoid popups ;)

        // 1.: set compression level
        args << QString("-m%1").arg(compressLevel());

        // 2.: is there a password?
        QString pass;
        if (genPass())
            pass = _randomStr(lengthPass());
        else if (useFixedPass() && !fixedPass().isEmpty())
            pass = fixedPass();
        if (!pass.isEmpty())
            args << QString("-hp%1").arg(pass);

        // 3.: shall we split into several volumes
        if (splitArchive() && splitSize() > 0)
            args << QString("-v%1m").arg(splitSize());

        // 4.: is it a dir and thus recursive?
        if (fi.isDir())
            args << "-r";

        // 5.: shall we lock the archive?
        if (lockArchive())
            args << "-k";

        // 6.: shall we create recovery records?
        if (addRecovery() && recoveryPct() > 0)
            args << QString("-rr%1p").arg(recoveryPct());

        // 7.: destination
        QString archiveName = _archiveName(fi);
        dstFolder = QString("%1/%2").arg(_dstDir->absolutePath()).arg(dstFolder);
        args << QString("%1/%2").arg(dstFolder).arg(archiveName);

        // 8.: add the entry
        args << fi.absoluteFilePath();

        if (debug())
            _log(QString("%1 %2").arg(rarPath()).arg(args.join(" ")));
        else
            _log(tr("Compressing %1").arg(fi.absoluteFilePath()));

        extProc->setProperty(sPropertySrcFolder,   fi.absoluteFilePath());
        extProc->setProperty(sPropertyDstFolder,   dstFolder);
        extProc->setProperty(sPropertyArchiveName, archiveName.left(archiveName.size()-4)); // remove ".rar"
        extProc->setProperty(sPropertyPassword,    pass);
        extProc->start(rarPath(), args);
    }
}



bool ScenePacker::_entryExistInDstFolder(const QFileInfo &fi)
{
    QString dstFolderPath = QString("%1/%2").arg(_dstDir->absolutePath()).arg(_dstFolderForEntry(fi));
    return QFileInfo(dstFolderPath).exists();
}

void ScenePacker::onProcFinished(int exitCode)
{    
    qDebug() << "rar exit code: " <<  exitCode;

    ++_nbCompressed;
    if (_hmi)
        _hmi->setProgress(_nbCompressed);

    QProcess *extProc = static_cast<QProcess*>(sender());
    QString dstFolder = extProc->property(sPropertyDstFolder).toString();
    if (exitCode != 0)
    {
        _error(tr("Error during compression of %1: #%2").arg(dstFolder).arg(exitCode));

        QDir dir(dstFolder);
        if (!dir.removeRecursively())
            _error(tr("Error removing broken folder %1").arg(dstFolder));
    }
    else
    {
        if (!_logPerRun)
            _logStream << QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss") << ";";
        _logStream << extProc->property(sPropertySrcFolder).toString() << ";"
                   << dstFolder << ";"
                   << extProc->property(sPropertyArchiveName).toString() << ";"
                   << extProc->property(sPropertyPassword).toString()
                   << endl << flush;
    }


    _processNextFolder(extProc);
}


void ScenePacker::_syntax(char *appName)
{
    QString app = QFileInfo(appName).fileName();
    _cout << desc() << "\n"
          << tr("Syntax: ") << app << " (options)* (-i <src_folder>)+ (-o <dst_path>| -x <rar_folder>)\n";
    for (const QCommandLineOption & opt : sCmdOptions)
    {
        if (opt.names().size() == 1)
            _cout << QString("\t--%1: %2\n").arg(opt.names().first(), -17).arg(tr(opt.description().toLocal8Bit().constData()));
        else
            _cout << QString("\t-%1: %2\n").arg(opt.names().join(" or --"), -18).arg(tr(opt.description().toLocal8Bit().constData()));
    }
    _cout << "\n"
          << "Examples: \n"
          << "  1.: using dst path:   " << appName << " -i ~/Downloads/folder1 -i ~/Downloads/folder2 -o /tmp/archives --genName --genPass --lengthPass 17\n"
          << "  2.: using rar folder: " << appName << " -i ~/Downloads/folder1 -i ~/Downloads/folder2 -x _rar\n"
          << endl
          << "The first example will create all archives from both folder1 and folder2 inside the destination folder /tmp/archives\n"
          << "The second example, will create a '_rar' folder in both folder1 et folder2 with their respective archives\n"
          << endl << flush;
}


void ScenePacker::_logTimeElapsed()
{
    int duration = static_cast<int>(_timeStart.elapsed());
    double sec = static_cast<double>(duration)/1000.;

    _log(tr("<br/><b> => %1/%2 entries compressed in %3 sec (%4)</b>").arg(
             _nbCompressed).arg(_nbTotal).arg(
             std::round(sec)).arg(QTime::fromMSecsSinceStartOfDay(duration).toString("hh:mm:ss.zzz")));
}


void ScenePacker::setThreads(int nb)
{
    if( nb < 1)
        nb  = 1;
    else if (nb > QThread::idealThreadCount())
        nb = QThread::idealThreadCount();

    _settings->setValue(sParamNames[Param::Threads], nb);
}


bool ScenePacker::setRarCmd(const QString &path)
{
    QFileInfo fi(path);
    if (fi.exists() && fi.isFile() && fi.isExecutable())
    {
        _settings->setValue(sParamNames[Param::CmdRar], path);
        if (path.toLower().endsWith("winrar.exe"))
            _useWinrar = true;
        return true;
    }
    else
        return false;
}

void ScenePacker::setSrcFolder(const QString &srcFolder)
{
    _settings->setValue(sParamNames[Param::SrcFolder], srcFolder);
}

void ScenePacker::setRarFolder(const QString &folderName)
{
    _settings->setValue(sParamNames[Param::RarFolder], folderName);
}



bool ScenePacker::setDstFolder(const QString &path)
{
    QFileInfo fi(path);
    if (fi.exists() && fi.isDir() && fi.isWritable())
    {
        if (_dstDir)
            delete _dstDir;
        _dstDir = new QDir(path);
        _settings->setValue(sParamNames[Param::DstPath], path);
        return true;
    }
    else
        return false;
}

void ScenePacker::setRarPrefix(const QString &prefix)
{
    _settings->setValue(sParamNames[Param::RarPrefix], prefix);
}

bool ScenePacker::_setRarFolder(const QString &path)
{
    const QString dstFolder(rarFolder());
    QFileInfo fi(QString("%1/%2").arg(path).arg(dstFolder));
    if (!fi.exists())
    {
        QDir dir(path);
        if (!dir.mkdir(dstFolder))
        {
//            _error(tr("Couldn't create rar folder: %1").arg(fi.absoluteFilePath()));
            return false;
        }
    }
    if (fi.exists() && fi.isDir() && fi.isWritable())
    {
        if (_dstDir)
            delete _dstDir;
        _dstDir = new QDir(fi.absoluteFilePath());
        return true;
    }
    else
        return false;
}

void ScenePacker::setDebug(bool debug) { _settings->setValue(sParamNames[Param::Debug], debug); }
void ScenePacker::setDispSettings(bool disp) { _settings->setValue(sParamNames[Param::DispSettings], disp); }
void ScenePacker::setUseDestinationFolder(bool useDstFolder) { _settings->setValue(sParamNames[Param::DstChoice], useDstFolder); }

void ScenePacker::saveSettings(bool genName,
                               int lengthName,
                               bool genPass,
                               int lengthPass,
                               bool useFixedPass,
                               const QString &fixedPath,
                               bool splitArchive,
                               int splitSize,
                               bool addRecovery,
                               int recoveryPct,
                               bool lockArchive,
                               int compressLevel)
{
    _settings->setValue(sParamNames[Param::GenName],      genName);
    _settings->setValue(sParamNames[Param::LengthName],   lengthName);
    _settings->setValue(sParamNames[Param::GenPass],      genPass);
    _settings->setValue(sParamNames[Param::LengthPass],   lengthPass);
    _settings->setValue(sParamNames[Param::UseFixedPass], useFixedPass);
    _settings->setValue(sParamNames[Param::FixedPass],    fixedPath);
    _settings->setValue(sParamNames[Param::SplitArchive], splitArchive);
    _settings->setValue(sParamNames[Param::SplitSize],    splitSize);
    _settings->setValue(sParamNames[Param::AddRecovery],  addRecovery);
    _settings->setValue(sParamNames[Param::RecoveryPct],  recoveryPct);
    _settings->setValue(sParamNames[Param::LockArchive],  lockArchive);
    _settings->setValue(sParamNames[Param::CompressLevel],compressLevel);
    _settings->setValue(sParamNames[Param::LogPerRun],    _logPerRun);
}

const QString ScenePacker::sASCII = "\
              __________               __\n\
   ______ ____\\______   \\____    ____ |  | __ ___________\n\
  /  ___// ___\\|     ___|__  \\ _/ ___\\|  |/ // __ \\_  __ \\\n\
  \\___ \\\\  \\___|    |    / __ \\\\  \\___|    <\\  ___/|  | \\/\n\
 /____  >\\___  >____|   (____  /\\___  >__|_ \\\\___  >__|\n\
      \\/     \\/              \\/     \\/     \\/    \\/\
";
