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

#ifndef SCENEPACKER_H
#define SCENEPACKER_H
#include "CmdOrGuiApp.h"
#include <QCommandLineOption>
#include <QTextStream>
#include <QQueue>
#include <QFileInfo>
#include <QElapsedTimer>
#include <QSettings>
class MainWindow;
class QProcess;

class ScenePacker : public QObject, public CmdOrGuiApp
{
    Q_OBJECT
public:
    enum class Param : char {CmdRar =0, DstPath, RarFolder, SrcFolder,
                             DstChoice, Threads, RarPrefix,
                             GenName, LengthName,
                             GenPass, LengthPass, UseFixedPass, FixedPass,
                             AddRecovery, RecoveryPct,
                             SplitArchive, SplitSize,
                             LockArchive, CompressLevel,
                             Debug, DispSettings,
                             Help, Version
                            };

    enum class DstChoice : bool {SrcFolder = false, DstFolder = true};

private:

    QDir               *_dstDir;

    QTextStream         _cout; //!< stream for stdout
    QTextStream         _cerr; //!< stream for stderr
    QVector<QProcess*>  _extProcs;

    QQueue<QFileInfo>   _entriesToCompress; //!< can also be folders
    int                 _nbTotal;
    int                 _nbCompressed;


    QElapsedTimer       _timeStart;

    QSettings          *_settings;
    bool                _stopProcess;

    QFile              *_logFile;
    QTextStream         _logStream;

    bool                _useWinrar;


public:
    explicit ScenePacker(int &argc, char *argv[]);
    ~ScenePacker() override;

    // pure virtual methods
    inline const char * appName() override;
    bool parseCommandLine(int argc, char *argv[]) override;

    int startHMI() override;

    void processFolders(const QStringList &srcFolders);
    void stopProcessing();

    void setThreads(int nb);
    bool setRarCmd(const QString &path);
    void setSrcFolder(const QString &srcFolder);
    void setRarFolder(const QString &folderName);
    bool setDstFolder(const QString &path);
    void setRarPrefix(const QString &prefix);
    void setDebug(bool debug);
    void setDispSettings(bool disp);
    void setUseDestinationFolder(bool useDstFolder);


    void saveSettings(bool genName = true,
                      int  lengthName = 31,
                      bool genPass = true,
                      int  lengthPass = 21,
                      bool useFixedPass = false,
                      const QString &fixedPath = QString(),
                      bool splitArchive = true,
                      int  splitSize = 42,
                      bool addRecovery = false,
                      int  recoveryPct = 0,
                      bool lockArchive = false,
                      int  compressLevel = 0);



    inline int     threads()       const;
    inline QString srcFolder()     const;
    inline QString rarPath()       const;
    inline QString dstPath()       const;
    inline QString rarPrefix()     const;
    inline DstChoice dstChoice()   const;
    inline bool    useDestinationFolder() const;
    inline QString rarFolder()     const;
    inline bool    genName()       const;
    inline int     lengthName()    const;
    inline bool    genPass()       const;
    inline int     lengthPass()    const;
    inline bool    useFixedPass()  const;
    inline QString fixedPass()     const;
    inline bool    splitArchive()  const;
    inline int     splitSize()     const;
    inline bool    addRecovery()   const;
    inline int     recoveryPct()   const;
    inline bool    lockArchive()   const;
    inline int     compressLevel() const;
    inline bool    debug()         const;
    inline bool    dispSettings()  const;


public slots:
    void onProcFinished(int exitCode);

    void onAbout();
    void onDonate();



private:
    void _processNextFolder(QProcess *extProc);

    inline QString _dstFolderForEntry(const QFileInfo &fi);
    bool _entryExistInDstFolder(const QFileInfo &fi);

    inline QString _archiveName(const QFileInfo &fi);

    bool _setRarFolder(const QString &path);


    void _log(const QString &msg, bool success = false);
    void _error(const QString &msg);

    void _clear();

    void _logTimeElapsed();

    inline void _showVersionASCII();
    void _syntax(char *appName);

    inline QString _randomStr(int length) const;

    bool _allProcessesDone() const;


// statics
private:
    // statics

    static constexpr const char *sAppName   = "scenePacker";
    static constexpr const char *sVersion   = "1.0";
    static constexpr const char *sDesc      = "pack files/folders recursively";

    static constexpr const char *sLogFolder = "./logs";

    static constexpr const char *sPropertyDstFolder = "dstFolder";
    static constexpr const char *sPropertySrcFolder = "srcFolder";


    static const QMap<Param, QString>      sParamNames;
    static const QList<QCommandLineOption> sCmdOptions;
    static const QStringList sRarDefaultArgs;

    static const QString sASCII;

    static const QRegularExpression sRegExpArchiveExtensions;
    static const QRegularExpression sRegExpArchiveFiles;
    static const QRegularExpression sRegExpNumberOne;

    static const QString sDonationURL;


public:
    inline static QString desc(bool useHTML = false);
    inline static QString asciiArtWithVersion();
    inline static const QString &donationURL();

};

const char *ScenePacker::appName() { return sAppName; }


const QString &ScenePacker::donationURL() { return sDonationURL; }


void ScenePacker::_showVersionASCII()
{
    _cout << asciiArtWithVersion() << "\n\n" << flush;
}

QString ScenePacker::_randomStr(int length) const
{
    if (length < 5)
        length = 5;
    QString str, alphabet("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");
    int nbLetters = alphabet.length();
    for (int i = 0 ; i < length ; ++i)
        str.append(alphabet.at(std::rand()%nbLetters));

    return str;
}



QString ScenePacker::desc(bool useHTML)
{
    QString desc;
    if (useHTML)
        desc = QString("%1<br/>%2<ul><li>%3</li><li>%4</li><li>%5</li></ul>%6<br/><br/>");
    else
        desc = QString("%1\n%2\n  - %3\n  - %4\n  - %5\n%6\n");
    return desc.arg(
            tr("Command line / GUI tool that packs every files/folders of the input folders individually using RAR")).arg(
            tr("The archives will either be in ONE destination folder or inside a 'rar subfolder' of each inputs")).arg(
            tr("you can generate a random name for the archives and set its length")).arg(
            tr("idem for the password but you can also use a fixed one for all the archives")).arg(
            tr("split the archives into several volumes, lock it, add recovery records...")).arg(
            tr("for more details, cf %1").arg(
                    useHTML ? "<a href=\"https://github.com/mbruel/scenePacker/\">https://github.com/mbruel/scenePacker</a>"
                            : "https://github.com/mbruel/scenePacker"));}

QString ScenePacker::asciiArtWithVersion()
{
    return QString("%1\n                                         v%2").arg(sASCII).arg(sVersion);
}

int     ScenePacker::threads()       const { return _settings->value(sParamNames[Param::Threads]).toInt(); }

QString ScenePacker::srcFolder()     const { return _settings->value(sParamNames[Param::SrcFolder]).toString(); }
QString ScenePacker::rarPath()       const { return _settings->value(sParamNames[Param::CmdRar]).toString(); }
QString ScenePacker::dstPath()       const { return _settings->value(sParamNames[Param::DstPath]).toString(); }
QString ScenePacker::rarPrefix()     const { return _settings->value(sParamNames[Param::RarPrefix]).toString(); }
ScenePacker::DstChoice ScenePacker::dstChoice() const { return static_cast<DstChoice>(_settings->value(sParamNames[Param::DstChoice]).toBool()); }
bool    ScenePacker::useDestinationFolder() const { return dstChoice() == DstChoice::DstFolder; }
QString ScenePacker::rarFolder()     const { return _settings->value(sParamNames[Param::RarFolder]).toString(); }
bool    ScenePacker::genName()       const { return _settings->value(sParamNames[Param::GenName]).toBool(); }
int     ScenePacker::lengthName()    const { return _settings->value(sParamNames[Param::LengthName]).toInt(); }
bool    ScenePacker::genPass()       const { return _settings->value(sParamNames[Param::GenPass]).toBool(); }
int     ScenePacker::lengthPass()    const { return _settings->value(sParamNames[Param::LengthPass]).toInt(); }
bool    ScenePacker::useFixedPass()  const { return _settings->value(sParamNames[Param::UseFixedPass]).toBool(); }
QString ScenePacker::fixedPass()     const { return _settings->value(sParamNames[Param::FixedPass]).toString(); }
bool    ScenePacker::splitArchive()  const { return _settings->value(sParamNames[Param::SplitArchive]).toBool(); }
int     ScenePacker::splitSize()     const { return _settings->value(sParamNames[Param::SplitSize]).toInt(); }
bool    ScenePacker::addRecovery()   const { return _settings->value(sParamNames[Param::AddRecovery]).toBool(); }
int     ScenePacker::recoveryPct()   const { return _settings->value(sParamNames[Param::RecoveryPct]).toInt(); }
bool    ScenePacker::lockArchive()   const { return _settings->value(sParamNames[Param::LockArchive]).toBool(); }
int     ScenePacker::compressLevel() const { return _settings->value(sParamNames[Param::CompressLevel]).toInt(); }
bool    ScenePacker::debug()         const { return _settings->value(sParamNames[Param::Debug]).toBool(); }
bool    ScenePacker::dispSettings()  const { return _settings->value(sParamNames[Param::DispSettings]).toBool(); }


QString ScenePacker::_dstFolderForEntry(const QFileInfo &fi)
{
    if (useDestinationFolder())
        return QString("%1%2").arg(rarPrefix()).arg(fi.isDir() ? fi.fileName() : fi.completeBaseName());
    else
        return fi.isDir() ? fi.fileName() : fi.completeBaseName();
}

QString ScenePacker::_archiveName(const QFileInfo &fi)
{
    if (genName())
        return QString("%1.rar").arg(_randomStr(lengthName()));
    else
        return QString("%1.rar").arg(fi.isDir() ? fi.fileName() : fi.completeBaseName());
}

#endif // SCENEPACKER_H
