#include "fileengine.h"
#include <unistd.h>
#include <sys/types.h>
#include <QFile>

static const char *REMOTE_HELPER = "/usr/bin/nanofiles-sshfs";

FileEngine::FileEngine(QObject *parent) :
    QObject(parent)
{
    _currentFileIndex = -1;
    _rootMode = false; // root mode removed (no setuid binary)


    fileList = new FileList();
    fileProcess = new FileProcess();
    clipboard = new Clipboard();

    refreshRemote();
}

static bool isMountPoint(const QString &path)
{
    QFile mounts(QStringLiteral("/proc/mounts"));
    if (path.isEmpty() || !mounts.open(QIODevice::ReadOnly))
        return false;
    const QList<QByteArray> lines = mounts.readAll().split('\n');
    for (const QByteArray &line : lines) {
        const QList<QByteArray> f = line.split(' ');
        if (f.size() > 1 && QString::fromLocal8Bit(f.at(1)).replace(QStringLiteral("\\040"), QStringLiteral(" ")) == path)
            return true;
    }
    return false;
}

FileEngine::~FileEngine()
{
    // Disconnect when the app closes: an idle sshfs mount keeps waking the phone.
    if (isMountPoint(_remoteMountPoint))
        QProcess::execute(QString::fromLatin1(REMOTE_HELPER), QStringList() << QStringLiteral("umount"));
}

void FileEngine::refreshRemote()
{
    if (_remoteName.isEmpty()) {
        QProcess info;
        info.start(QString::fromLatin1(REMOTE_HELPER), QStringList() << QStringLiteral("info"));
        if (info.waitForFinished(3000) && info.exitStatus() == QProcess::NormalExit && info.exitCode() == 0) {
            const QStringList lines = QString::fromUtf8(info.readAllStandardOutput()).split(QLatin1Char('\n'));
            if (lines.size() >= 2 && !lines.at(0).isEmpty() && lines.at(1).startsWith(QLatin1Char('/'))) {
                _remoteName = lines.at(0);
                _remoteMountPoint = lines.at(1);
            }
        }
    }
    _remoteMounted = isMountPoint(_remoteMountPoint);
    emit remoteStateChanged();
}

void FileEngine::remoteMount()
{
    runRemote(QStringLiteral("mount"));
}

void FileEngine::remoteUnmount()
{
    runRemote(QStringLiteral("umount"));
}

void FileEngine::runRemote(const QString &command)
{
    if (_remoteProcess || !remoteConfigured())
        return;
    _remoteProcess = new QProcess(this);
    _remoteProcess->setProcessChannelMode(QProcess::MergedChannels);
    connect(_remoteProcess, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            this, [this, command](int exitCode, QProcess::ExitStatus status) {
        const QString output = QString::fromUtf8(_remoteProcess->readAll()).trimmed();
        _remoteProcess->deleteLater();
        _remoteProcess = nullptr;
        refreshRemote();
        if (status != QProcess::NormalExit || exitCode != 0)
            emit remoteError(output.isEmpty() ? QStringLiteral("%1 %2: exit %3").arg(_remoteName, command).arg(exitCode) : output);
        else if (command == QLatin1String("mount"))
            emit remoteMountFinished(_remoteMounted);
    });
    // finished() is not emitted when the helper cannot be started at all
    connect(_remoteProcess, &QProcess::errorOccurred, this, [this](QProcess::ProcessError error) {
        if (error == QProcess::FailedToStart && _remoteProcess) {
            const QString msg = _remoteProcess->errorString();
            _remoteProcess->deleteLater();
            _remoteProcess = nullptr;
            refreshRemote();
            emit remoteError(msg);
        }
    });
    emit remoteStateChanged();
    _remoteProcess->start(QString::fromLatin1(REMOTE_HELPER), QStringList() << command);
}

void FileEngine::performFileOperation(const QString &fileOperation,
                                      const QStringList &clipboardList,
                                      const QString &clipboardDir,
                                      const QStringList &selectedFiles,
                                      const QString &directory)
{
    Worker *worker = new Worker();

    connect(worker, SIGNAL(progressTextChanged(QString)), this, SIGNAL(progressTextChanged(QString)));
    connect(worker, SIGNAL(currentEntryChanged(QString)), this, SIGNAL(currentEntryChanged(QString)));
    connect(worker, SIGNAL(progressValueChanged(double)), this, SIGNAL(progressValueChanged(double)));

    connect(worker, SIGNAL(fileOperationFinished()), this, SIGNAL(fileOperationFinished()));
    connect(worker, SIGNAL(finished()), worker, SLOT(deleteLater()));

    if (fileOperation == "paste")
    {
        bool cut = false;
        if (clipboard->getFileOperation() == "cut")
            cut = true;

        worker->startPasteProcess(clipboardList, directory, clipboardDir, cut);
    }
    else if (fileOperation == "delete")
    {
        worker->startDeleteProcess(selectedFiles);
    }
}

void FileEngine::resetCurrentFileIndex()
{
    setCurrentFileIndex(-1);
}

/*
 *  Rename files
 */
void FileEngine::renameFiles(const QString &jsonString)
{
    QVariantList fileList = QJsonDocument::fromJson(QByteArray(jsonString.toUtf8())).array().toVariantList();

    for (int i=0; i < fileList.length(); i++)
    {
        QMap<QString, QVariant> fileMap = fileList.at(i).toMap();

        QString sourceFullPath = fileMap.value("sourceFullPath").toString();
        QString sourceName = fileMap.value("sourceName").toString();
        QString newName = fileMap.value("newName").toString();

        // Load the file
        QFile file(sourceFullPath);

        file.rename(sourceFullPath.replace(sourceName, newName));
    }
}

/*
 *  Create tar.gz with files
 */
void FileEngine::tarFiles(const QString &tarFile, const QString &baseDir, const QStringList &fileList)
{
    QProcess *proc = new QProcess();
    QString exec = "tar";
    QStringList params;

    QString dir = baseDir;
    dir = dir.remove(QRegExp("/$")) + "/";
    QRegExp baseDirRe = QRegExp("^" + dir);

    params << "-c";
    params << "-z";
    params << "-C" << dir;
    params << "-f" << tarFile;
    for (int i=0; i < fileList.length(); i++)
    {
        QString file = fileList.at(i);
        params << file.remove(baseDirRe);
    }
    proc->start(exec, params);
}

/*
 *  Create files/directories based on the provided array
 */
void FileEngine::createEntries(const QString &jsonString)
{
    QVariantList entryList = QJsonDocument::fromJson(QByteArray(jsonString.toUtf8())).array().toVariantList();

    for (int i=0; i < entryList.length(); i++)
    {
        QMap<QString, QVariant> entryMap = entryList.at(i).toMap();

        QString type = entryMap.value("type").toString();
        QString path = entryMap.value("path").toString();
        QString name = entryMap.value("name").toString();

        // Create directories
        if (type == "directory")
        {
            QDir dir(path);
            dir.mkdir(QString("%1/%2").arg(path, name));
        }
        else if (type == "file")
        {
            QFile file(QString("%1/%2").arg(path, name));
            file.open(QFile::WriteOnly);
        }
    }
}

/*
 *  Copies a string to the clipboard
 */
void FileEngine::copyToClipboard(const QString &string)
{
    QClipboard *clipboard = QGuiApplication::clipboard();

    clipboard->setText(string);
}

/*
 *  Change a single file permission in a file
 */
bool FileEngine::changeFilePermission(QString fullPath, int permissionPos)
{
    QFile file(fullPath);

    QFileDevice::Permissions filePermissions = file.permissions();

    switch (permissionPos)
    {
        case 0:
            if (filePermissions & QFileDevice::ReadUser) filePermissions = filePermissions & ~QFileDevice::ReadUser;
            else filePermissions = filePermissions | QFileDevice::ReadUser;
            break;
        case 1:
            if (filePermissions & QFileDevice::WriteUser) filePermissions = filePermissions & ~QFileDevice::WriteUser;
            else filePermissions = filePermissions | QFileDevice::WriteUser;
            break;
        case 2:
            if (filePermissions & QFileDevice::ExeUser) filePermissions = filePermissions & ~QFileDevice::ExeUser;
            else filePermissions = filePermissions | QFileDevice::ExeUser;
            break;
        case 3:
            if (filePermissions & QFileDevice::ReadGroup) filePermissions = filePermissions & ~QFileDevice::ReadGroup;
            else filePermissions = filePermissions | QFileDevice::ReadGroup;
            break;
        case 4:
            if (filePermissions & QFileDevice::WriteGroup) filePermissions = filePermissions & ~QFileDevice::WriteGroup;
            else filePermissions = filePermissions | QFileDevice::WriteGroup;
            break;
        case 5:
            if (filePermissions & QFileDevice::ExeGroup) filePermissions = filePermissions & ~QFileDevice::ExeGroup;
            else filePermissions = filePermissions | QFileDevice::ExeGroup;
            break;
        case 6:
            if (filePermissions & QFileDevice::ReadOwner) filePermissions = filePermissions & ~QFileDevice::ReadOwner;
            else filePermissions = filePermissions | QFileDevice::ReadOwner;
            break;
        case 7:
            if (filePermissions & QFileDevice::WriteOwner) filePermissions = filePermissions & ~QFileDevice::WriteOwner;
            else filePermissions = filePermissions | QFileDevice::WriteOwner;
            break;
        case 8:
            if (filePermissions & QFileDevice::ExeOwner) filePermissions = filePermissions & ~QFileDevice::ExeOwner;
            else filePermissions = filePermissions | QFileDevice::ExeOwner;
            break;
    }

    bool success = file.setPermissions(filePermissions);

    return success;
}

/*
 *  Check if the SD card is mounted
 *  If it is, return the mount point path
 *  If not, return an empty string
 */
QString FileEngine::getSdCardMountPath()
{
    QString path = "";

    if (QDir("/run/media/defaultuser").exists())
        path = "/run/media/defaultuser";
    else if (QDir("/run/media/nemo").exists())
        path = "/run/media/nemo";
    else if (QDir("/media/sdcard").exists())
        path = "/media/sdcard";

    return path;
}

bool FileEngine::getRootMode()
{
    return _rootMode;
}

void FileEngine::setRootMode(bool rootMode)
{
    // Root mode disabled: binary is not setuid, never escalate privileges.
    Q_UNUSED(rootMode)
}

/*
 *  currentFileIndex - the file index for the currently opened item
 */
void FileEngine::setCurrentFileIndex(const int &currentFileIndex)
{
    if (_currentFileIndex != currentFileIndex)
    {
        _currentFileIndex = currentFileIndex;
        emit currentFileIndexChanged(currentFileIndex);
    }
}

int FileEngine::getCurrentFileIndex() const
{
    return _currentFileIndex;
}
