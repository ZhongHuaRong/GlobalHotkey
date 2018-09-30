#include "GlobalHotkey.h"
#include <QApplication>
#include <QSystemTrayIcon>
#include <QStyle>
#include <QAction>
#include <QMenu>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    GlobalHotkey w;
    w.show();

    QSystemTrayIcon tray;
    tray.setIcon( w.style()->standardIcon(QStyle::SP_DirIcon));
//    tray.contextMenu()->addAction(&QAction("Exit"));
    tray.show();
    QObject::connect(&tray,&QSystemTrayIcon::activated,[&](QSystemTrayIcon::ActivationReason reason){
        switch(reason){
        case QSystemTrayIcon::Trigger:
            w.show();
            w.showNormal();
            break;
        case QSystemTrayIcon::Context:
            qApp->exit(0);
        }
    });
    return a.exec();
}
