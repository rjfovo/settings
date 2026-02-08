#ifndef BACKGROUND_H
#define BACKGROUND_H

#include <QObject>
#include <QList>
#include <QVariant>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QDirIterator>
#include <QDir>
#include <QScopedPointer>
#include <QFutureWatcher>

class Background : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString currentBackgroundPath READ currentBackgroundPath WRITE setBackground NOTIFY backgroundChanged)
    Q_PROPERTY(QVariantList backgrounds READ backgrounds NOTIFY backgroundsChanged)

    Q_PROPERTY(int backgroundType READ backgroundType WRITE setBackgroundType NOTIFY backgroundTypeChanged)
    Q_PROPERTY(QString backgroundColor READ backgroundColor WRITE setBackgroundColor NOTIFY backgroundColorChanged)
    Q_PROPERTY(bool loadingBackgrounds READ loadingBackgrounds NOTIFY loadingBackgroundsChanged)

public:
    explicit Background(QObject *parent = nullptr);
    ~Background();

    QVariantList backgrounds();
    QString currentBackgroundPath();
    Q_INVOKABLE void setBackground(QString newBackgroundPath);

    int backgroundType();
    void setBackgroundType(int type);

    QString backgroundColor();
    void setBackgroundColor(const QString &color);
    
    bool loadingBackgrounds() const;

signals:
    void backgroundChanged();
    void backgroundColorChanged();
    void backgroundTypeChanged();
    void backgroundsChanged();
    void loadingBackgroundsChanged();

private slots:
    void onNameOwnerChanged(const QString &name, const QString &oldOwner, const QString &newOwner);
    void onBackgroundsLoaded();

private:
    void createInterface();
    void disconnectInterface();
    void loadBackgrounds();
    
    QScopedPointer<QDBusInterface> m_interface;
    QString m_currentPath;
    QVariantList m_backgrounds;
    bool m_loadingBackgrounds;
    QFutureWatcher<QVariantList> *m_backgroundsWatcher;
};

#endif
