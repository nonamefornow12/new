#include <QApplication>
#include <QStyleFactory>
#include <QPalette>
#include <QFont>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Set application properties
    app.setApplicationName("PandaBlur");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("PandaBlur Security");

    // Set modern font
    QFont font("Segoe UI", 10);
    app.setFont(font);

    MainWindow window;
    window.show();

    return app.exec();
}
