#include "application.h"
#include <QCommandLineParser>
#include <QDBusPendingCall>
#include <QTranslator>
#include <QLocale>
#include <QIcon>
#include <QDebug>
#include <QDir>

#include "settingsuiadaptor.h"
#include "fontsmodel.h"
#include "fonts/fonts.h"
#include "appearance.h"
#include "battery.h"
#include "batteryhistorymodel.h"
#include "brightness.h"
#include "about.h"
#include "background.h"
#include "language.h"
#include "password.h"
#include "powermanager.h"
#include "touchpad.h"
#include "networkproxy.h"
#include "notifications.h"
#include "defaultapplications.h"

#include "cursor/cursorthememodel.h"
#include "cursor/mouse.h"

#include "datetime/time.h"
#include "datetime/timezonemap.h"

static QObject *passwordSingleton(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(engine);
    Q_UNUSED(scriptEngine);

    Password *object = new Password();
    return object;
}

Application::Application(int &argc, char **argv)
    : QGuiApplication(argc, argv)
{
    qDebug() << "Cutefish Settings starting...";
    
    // 设置图标主题
    // 在Qt6中，需要确保图标主题可用
    // 首先设置搜索路径
    QStringList iconThemePaths;
    iconThemePaths << "/usr/share/icons";
    iconThemePaths << QDir::homePath() + "/.local/share/icons";
    iconThemePaths << "/usr/local/share/icons";
    QIcon::setThemeSearchPaths(iconThemePaths);
    
    // 尝试按优先级设置图标主题
    QStringList preferredThemes = {"cutefish", "Crule", "Crule-dark", "breeze", "Adwaita", "hicolor"};
    QString themeSet = "hicolor"; // 默认回退
    
    for (const QString &theme : preferredThemes) {
        QString themePath = QString("/usr/share/icons/%1").arg(theme);
        if (QDir(themePath).exists()) {
            themeSet = theme;
            break;
        }
    }
    
    QIcon::setThemeName(themeSet);
    qDebug() << "Settings: Icon theme set to:" << QIcon::themeName() << "from search paths:" << QIcon::themeSearchPaths();
    
    // 确保QIcon图像提供者可用于QML
    // 这是image://icontheme/ URL正常工作所必需的
    if (QIcon::themeName().isEmpty()) {
        qWarning() << "Settings: No icon theme set! image://icontheme/ URLs will not work.";
    }
    
    setWindowIcon(QIcon::fromTheme("preferences-system"));
    setOrganizationName("cutefishos");

    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("Cutefish Settings"));
    parser.addHelpOption();

    QCommandLineOption moduleOption("m", "Switch to module", "module");
    parser.addOption(moduleOption);
    parser.process(*this);

    const QString module = parser.value(moduleOption);

    qDebug() << "Attempting to register D-Bus service...";
    if (!QDBusConnection::sessionBus().registerService("com.cutefish.SettingsUI")) {
        qDebug() << "D-Bus service already registered, forwarding to existing instance.";
        QDBusInterface iface("com.cutefish.SettingsUI", "/SettingsUI", "com.cutefish.SettingsUI", QDBusConnection::sessionBus());
        if (iface.isValid())
            iface.call("switchToPage", module);
        return;
    }

    qDebug() << "D-Bus service registered successfully.";
    new SettingsUIAdaptor(this);
    QDBusConnection::sessionBus().registerObject(QStringLiteral("/SettingsUI"), this);

    // QML
    const char *uri = "Cutefish.Settings";
    qmlRegisterType<Appearance>(uri, 1, 0, "Appearance");
    qmlRegisterType<FontsModel>(uri, 1, 0, "FontsModel");
    qmlRegisterType<Brightness>(uri, 1, 0, "Brightness");
    qmlRegisterType<Battery>(uri, 1, 0, "Battery");
    qmlRegisterType<BatteryHistoryModel>(uri, 1, 0, "BatteryHistoryModel");
    qmlRegisterType<CursorThemeModel>(uri, 1, 0, "CursorThemeModel");
    qmlRegisterType<About>(uri, 1, 0, "About");
    qmlRegisterType<Background>(uri, 1, 0, "Background");
    qmlRegisterType<Language>(uri, 1, 0, "Language");
    qmlRegisterType<Fonts>(uri, 1, 0, "Fonts");
    qmlRegisterType<PowerManager>(uri, 1, 0, "PowerManager");
    qmlRegisterType<Mouse>(uri, 1, 0, "Mouse");
    qmlRegisterType<Time>(uri, 1, 0, "Time");
    qmlRegisterType<TimeZoneMap>(uri, 1, 0, "TimeZoneMap");
    qmlRegisterType<Touchpad>(uri, 1, 0, "Touchpad");
    qmlRegisterType<NetworkProxy>(uri, 1, 0, "NetworkProxy");
    qmlRegisterType<Notifications>(uri, 1, 0, "Notifications");
    qmlRegisterType<DefaultApplications>(uri, 1, 0, "DefaultApplications");

    qmlRegisterSingletonType<Password>(uri, 1, 0, "Password", passwordSingleton);

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    qmlRegisterType<QAbstractItemModel>();
#else
    qmlRegisterAnonymousType<QAbstractItemModel>(uri, 1);
#endif

    // Translations
    QLocale locale;
    QString qmFilePath = QString("%1/%2.qm").arg("/usr/share/cutefish-settings/translations/").arg(locale.name());
    if (QFile::exists(qmFilePath)) {
        QTranslator *translator = new QTranslator(QGuiApplication::instance());
        if (translator->load(qmFilePath)) {
            QGuiApplication::installTranslator(translator);
        } else {
            translator->deleteLater();
        }
    }

    qDebug() << "Setting up QML engine...";
    m_engine.addImportPath(QStringLiteral("qrc:/"));
    m_engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));
    qDebug() << "QML loaded, entering event loop...";

    if (!module.isEmpty()) {
        switchToPage(module);
    }

    QGuiApplication::exec();
}

void Application::switchToPage(const QString &name)
{
    QObject *mainObject = m_engine.rootObjects().first();

    if (mainObject) {
        QMetaObject::invokeMethod(mainObject, "switchPageFromName", Q_ARG(QVariant, name));
    }
}
