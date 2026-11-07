#ifndef FILEENGINE_H
#define FILEENGINE_H

#include <QObject>
#include <QGuiApplication>
#include <QJsonDocument>
#include <QJsonArray>
#include <QClipboard>
#include <QDir>
#include <QProcess>
#include "filelist.h"
#include "fileprocess.h"
#include "worker.h"
#include "clipboard.h"

class FileEngine : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int currentFileIndex READ getCurrentFileIndex WRITE setCurrentFileIndex NOTIFY currentFileIndexChanged)
    Q_PROPERTY(bool rootMode READ getRootMode WRITE setRootMode NOTIFY rootModeChanged)
    // Remote (sshfs) location configured in ~/.config/nanofiles/sshfs.conf
    Q_PROPERTY(bool remoteConfigured READ remoteConfigured NOTIFY remoteStateChanged)
    Q_PROPERTY(QString remoteName READ remoteName NOTIFY remoteStateChanged)
    Q_PROPERTY(QString remoteMountPoint READ remoteMountPoint NOTIFY remoteStateChanged)
    Q_PROPERTY(bool remoteMounted READ remoteMounted NOTIFY remoteStateChanged)
    Q_PROPERTY(bool remoteBusy READ remoteBusy NOTIFY remoteStateChanged)
public:
    explicit FileEngine(QObject *parent = 0);
    ~FileEngine();

    Q_INVOKABLE void resetCurrentFileIndex();

    Q_INVOKABLE void performFileOperation(const QString &fileOperation,
                                          const QStringList &clipboardList,
                                          const QString &clipboardDir,
                                          const QStringList &selectedFiles,
                                          const QString &directory);

    Q_INVOKABLE void renameFiles(const QString &jsonString);
    Q_INVOKABLE void tarFiles(const QString &tarFile, const QString &baseDir, const QStringList &fileList);
    Q_INVOKABLE void createEntries(const QString &jsonString);

    Q_INVOKABLE void copyToClipboard(const QString &string);

    Q_INVOKABLE bool changeFilePermission(QString fullPath, int permissionPos);

    Q_INVOKABLE QString getSdCardMountPath();

    Q_INVOKABLE void refreshRemote();
    Q_INVOKABLE void remoteMount();
    Q_INVOKABLE void remoteUnmount();
    bool remoteConfigured() const { return !_remoteName.isEmpty(); }
    QString remoteName() const { return _remoteName; }
    QString remoteMountPoint() const { return _remoteMountPoint; }
    bool remoteMounted() const { return _remoteMounted; }
    bool remoteBusy() const { return _remoteProcess != nullptr; }

    bool getRootMode();
    void setRootMode(bool rootMode);

    void setCurrentFileIndex(const int &currentFileIndex);
    int getCurrentFileIndex() const;

    FileList *fileList;
    FileProcess *fileProcess;
    Clipboard *clipboard;

private:
    int _currentFileIndex;
    bool _rootMode;
    QString _remoteName;
    QString _remoteMountPoint;
    bool _remoteMounted = false;
    QProcess *_remoteProcess = nullptr;
    void runRemote(const QString &command);

signals:
    void currentFileIndexChanged(const int &currentFileIndex);

    // File operation signals
    void progressTextChanged(const QString &progressText);
    void progressValueChanged(const double &progressValue);
    void currentEntryChanged(const QString &currentEntry);

    void fileOperationFinished();
    void rootModeChanged(bool rootMode);
    void remoteStateChanged();
    void remoteMountFinished(bool mounted);
    void remoteError(const QString &message);

public slots:

};

#endif // FILEENGINE_H
