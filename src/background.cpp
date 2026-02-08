#include "background.h"
#include <QtConcurrent>
#include <QTimer>

static QVariantList getBackgroundPaths()
{
    QVariantList list;
    QDirIterator it("/usr/share/backgrounds/cutefishos", QStringList() << "*.jpg" << "*.png", QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QString bg = it.next();
        list.append(QVariant(bg));
    }
    std::sort(list.begin(), list.end(), [](const QVariant &a, const QVariant &b) {
        // 根据实际数据类型选择合适的比较方式
        return a.toString() < b.toString(); // 如果是字符串
        // 或者 return a.toInt() < b.toInt(); // 如果是整数
        // 或者 return a.toReal() < b.toReal(); // 如果是浮点数
    });
    return list;
}

Background::Background(QObject *parent)
    : QObject(parent)
    , m_loadingBackgrounds(false)
    , m_backgroundsWatcher(nullptr)
{
    // Listen for DBus name owner changes so that if the settings service appears later
    // we can initialize and notify QML
    QDBusConnection::sessionBus().connect(QStringLiteral("org.freedesktop.DBus"),
                                          QStringLiteral("/org/freedesktop/DBus"),
                                          QStringLiteral("org.freedesktop.DBus"),
                                          QStringLiteral("NameOwnerChanged"),
                                          this,
                                          SLOT(onNameOwnerChanged(QString,QString,QString)));

    createInterface();
}

Background::~Background()
{
    disconnectInterface();
    if (m_backgroundsWatcher) {
        m_backgroundsWatcher->cancel();
        m_backgroundsWatcher->deleteLater();
    }
}

void Background::createInterface()
{
    m_interface.reset(new QDBusInterface("com.cutefish.Settings",
                                         "/Theme",
                                         "com.cutefish.Theme",
                                         QDBusConnection::sessionBus(), this));
    
    if (m_interface && m_interface->isValid()) {
        m_currentPath = m_interface->property("wallpaper").toString();

        QDBusConnection::sessionBus().connect(m_interface->service(),
                                              m_interface->path(),
                                              m_interface->interface(),
                                              "backgroundTypeChanged", this, SIGNAL(backgroundTypeChanged()));
        QDBusConnection::sessionBus().connect(m_interface->service(),
                                              m_interface->path(),
                                              m_interface->interface(),
                                              "backgroundColorChanged", this, SIGNAL(backgroundColorChanged()));
    }
}

void Background::disconnectInterface()
{
    if (m_interface && m_interface->isValid()) {
        QDBusConnection::sessionBus().disconnect(m_interface->service(),
                                                 m_interface->path(),
                                                 m_interface->interface(),
                                                 "backgroundTypeChanged", this, SIGNAL(backgroundTypeChanged()));
        QDBusConnection::sessionBus().disconnect(m_interface->service(),
                                                 m_interface->path(),
                                                 m_interface->interface(),
                                                 "backgroundColorChanged", this, SIGNAL(backgroundColorChanged()));
    }
    m_interface.reset();
}

QVariantList Background::backgrounds()
{
    // 避免在QML引擎加载时触发异步加载
    // 如果正在加载中，返回空列表，等待加载完成
    if (m_backgrounds.isEmpty() && !m_loadingBackgrounds) {
        // 使用单次定时器延迟加载，避免在QML组件初始化时触发
        QTimer::singleShot(0, this, &Background::loadBackgrounds);
    }
    return m_backgrounds;
}

QString Background::currentBackgroundPath()
{
    return m_currentPath;
}

void Background::setBackground(QString path)
{
    if (m_currentPath != path && !path.isEmpty()) {
        m_currentPath = path;

        if (m_interface && m_interface->isValid()) {
            m_interface->call("setWallpaper", path);
            emit backgroundChanged();
        }
    }
}

int Background::backgroundType()
{
    if (m_interface && m_interface->isValid()) {
        return m_interface->property("backgroundType").toInt();
    }
    return 0;
}

void Background::setBackgroundType(int type)
{
    if (m_interface && m_interface->isValid()) {
        m_interface->call("setBackgroundType", QVariant::fromValue(type));
    }
}

QString Background::backgroundColor()
{
    if (m_interface && m_interface->isValid()) {
        return m_interface->property("backgroundColor").toString();
    }
    return QString();
}

void Background::setBackgroundColor(const QString &color)
{
    if (m_interface && m_interface->isValid()) {
        m_interface->call("setBackgroundColor", QVariant::fromValue(color));
    }
}

void Background::onNameOwnerChanged(const QString &name, const QString &oldOwner, const QString &newOwner)
{
    Q_UNUSED(oldOwner);
    Q_UNUSED(newOwner);

    if (name != QLatin1String("com.cutefish.Settings"))
        return;

    // Reinitialize interface and read current wallpaper
    disconnectInterface();
    createInterface();
    
    if (m_interface && m_interface->isValid()) {
        m_currentPath = m_interface->property("wallpaper").toString();

        emit backgroundChanged();
        emit backgroundTypeChanged();
        emit backgroundColorChanged();
    }
}

void Background::loadBackgrounds()
{
    if (m_loadingBackgrounds) return;
    
    m_loadingBackgrounds = true;
    emit loadingBackgroundsChanged();
    
    // 确保之前的watcher完全清理
    if (m_backgroundsWatcher) {
        disconnect(m_backgroundsWatcher, &QFutureWatcher<QVariantList>::finished,
                   this, &Background::onBackgroundsLoaded);
        m_backgroundsWatcher->cancel();
        m_backgroundsWatcher->waitForFinished();
        m_backgroundsWatcher->deleteLater();
        m_backgroundsWatcher = nullptr;
    }
    
    QFuture<QVariantList> future = QtConcurrent::run(&getBackgroundPaths);
    
    m_backgroundsWatcher = new QFutureWatcher<QVariantList>(this);
    connect(m_backgroundsWatcher, &QFutureWatcher<QVariantList>::finished, 
            this, &Background::onBackgroundsLoaded);
    m_backgroundsWatcher->setFuture(future);
}

void Background::onBackgroundsLoaded()
{
    if (m_backgroundsWatcher) {
        m_backgrounds = m_backgroundsWatcher->result();
        m_backgroundsWatcher->deleteLater();
        m_backgroundsWatcher = nullptr;
    }
    
    m_loadingBackgrounds = false;
    emit loadingBackgroundsChanged();
    emit backgroundsChanged();
}

bool Background::loadingBackgrounds() const
{
    return m_loadingBackgrounds;
}
