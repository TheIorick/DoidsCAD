#include "mainwindow.h"

#include <QApplication>
#include <QSettings>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setOrganizationName(QStringLiteral("DOIDS"));
    a.setApplicationName(QStringLiteral("DoidsCAD"));

    const QString lang = QSettings().value(QStringLiteral("language"), QStringLiteral("en")).toString();
    QTranslator translator;
    if (lang != QStringLiteral("en"))
    {
        if (translator.load(QStringLiteral("doids_") + lang, QCoreApplication::applicationDirPath()))
            a.installTranslator(&translator);
    }

    MainWindow w;
    w.show();
    return a.exec();
}
