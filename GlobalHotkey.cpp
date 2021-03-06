#include "GlobalHotkey.h"
#include "ui_GlobalHotkey.h"
#include <QDebug>
#include <QMouseEvent>
#include <QDesktopWidget>
#include <QPoint>
#include <QSettings>
#include <Windows.h>
#include <WinUser.h>

GlobalHotkey::GlobalHotkey(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GlobalHotkey)
{
    ui->setupUi(this);
    isF5 = RegisterHotKey((HWND)this->winId(),
                          (int)HotKeyFunc::StartEvent,
                          MOD_NOREPEAT,
                          VK_F5);
    isF6 = RegisterHotKey((HWND)this->winId(),
                          (int)HotKeyFunc::EndEvent,
                          MOD_NOREPEAT,
                          VK_F6);
    isF7 = RegisterHotKey((HWND)this->winId(),
                          (int)HotKeyFunc::SaveMousePosition,
                          MOD_NOREPEAT,
                          VK_F7);
    isF8 = RegisterHotKey((HWND)this->winId(),
                          (int)HotKeyFunc::SaveAllSetting,
                          MOD_NOREPEAT,
                          VK_F8);
    showHotKeyConnectResult(HotKeyFunc::StartEvent,isF5);
    showHotKeyConnectResult(HotKeyFunc::EndEvent,isF6);
    showHotKeyConnectResult(HotKeyFunc::SaveMousePosition,isF7);
    showHotKeyConnectResult(HotKeyFunc::SaveAllSetting,isF8);


    auto rect = qApp->desktop()->screenGeometry();
    ui->lcdNumber_XM->display(rect.width());
    ui->lcdNumber_YM->display(rect.height());

    thread = new QThread(this);
    clickEvent.moveToThread(thread);
    thread->start();

    QSettings settings("config.ini",QSettings::IniFormat);
    setMousePosition(settings.value("mouseX").toInt(),
                     settings.value("mouseY").toInt());
    ui->lineEdit->setText(settings.value("frequency").toString());
    switch(settings.value("button").toInt()){
    case Qt::LeftButton:
        ui->leftButton->setChecked(true);
        break;
    case Qt::MiddleButton:
        ui->middleButton->setChecked(true);
        break;
    case Qt::RightButton:
        ui->rightButton->setChecked(true);
        break;
    }
}

GlobalHotkey::~GlobalHotkey(){
    delete ui;
    if(thread->isRunning()){
        thread->exit();
        thread->wait();
    }
}

void GlobalHotkey::run()
{
    auto num = ui->lineEdit->text();
    int n = num.toInt();
    if(n ==0){
        addMsg(QStringLiteral("开启鼠标点击(频率:%1 次每秒)").arg(QStringLiteral("无上限")));
    }
    else{
        addMsg(QStringLiteral("开启鼠标点击(频率:%1 次每秒)").arg(QString::number(n)));
    }
    auto t = clickEvent.run(n,getButton());
    addMsg(QString("(%1)").arg(t.toString("hh:mm:ss zzz")));
}

void GlobalHotkey::stop()
{
    auto t1 = clickEvent.getStartTime();
    auto t2 = clickEvent.stop();
    addMsg(QStringLiteral("关闭点击"));
    addMsg(QString("(%1)").arg(t2.toString("hh:mm:ss zzz")));
    QString msg;
    msg.append(QStringLiteral("持续时间:%1ms 一共点击次数:%2次 每秒次数:%3次/s"));
    auto t = t1.msecsTo(t2);
    auto n = clickEvent.getCurClick();
    addMsg(msg.arg(QString::number(t)).
           arg(QString::number(n)).
           arg(QString::number(n*1000.0/t)));
}

void GlobalHotkey::addMsg(const QString &str)
{
    ui->textEdit->append(str);
    ui->textEdit->moveCursor(QTextCursor::End);
}

void GlobalHotkey::setMousePosition(const int &x, const int &y)
{
    ui->lcdNumber_X->display(x);
    ui->lcdNumber_Y->display(y);
    clickEvent.setMouseX(x);
    clickEvent.setMouseY(y);
}

int GlobalHotkey::getButton()
{
    if(ui->leftButton->isChecked())
        return Qt::LeftButton;
    else if(ui->middleButton->isChecked())
        return Qt::MiddleButton;
    else if(ui->rightButton->isChecked())
        return Qt::RightButton;
}

bool GlobalHotkey::nativeEvent(const QByteArray &eventType,
                                     void *message, long *)
{
    if(eventType == "windows_generic_MSG"){
        //Windows
        auto msg = (MSG*)message;
        if(msg->message == WM_HOTKEY){
            switch(msg->wParam){
            case (int)HotKeyFunc::StartEvent:
                run();
                break;
            case (int)HotKeyFunc::EndEvent:
                stop();
                break;
            case (int)HotKeyFunc::SaveMousePosition:{
                auto pos = QCursor::pos();
                setMousePosition(pos.x(),pos.y());
                addMsg(QStringLiteral("重置鼠标位置(%1,%2)").
                       arg(QString::number(pos.x())).
                       arg(QString::number(pos.y())));
                break;
            }
            case (int)HotKeyFunc::SaveAllSetting:
                QSettings settings("config.ini",QSettings::IniFormat);
                settings.setValue("mouseX",(int)ui->lcdNumber_X->value());
                settings.setValue("mouseY",(int)ui->lcdNumber_Y->value());
                settings.setValue("frequency",ui->lineEdit->text());
                settings.setValue("button",getButton());
                addMsg(QStringLiteral("保存设置成功"));
                break;
            }
            return true;
        }
    }
    else if(eventType == "NSEvent"){
        //macOS
    }
    return false;
}

void GlobalHotkey::mouseMoveEvent(QMouseEvent *event)
{
    QWidget::mouseMoveEvent(event);
    //qDebug()<<mapToGlobal(event->pos());
}

void GlobalHotkey::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::WindowStateChange){
        if(this->windowState() == Qt::WindowMinimized)
            this->hide();
    }
}

void GlobalHotkey::showHotKeyConnectResult(GlobalHotkey::HotKeyFunc key, bool result)
{
    QString msg;
    switch(key){
    case (int)HotKeyFunc::StartEvent:
        msg.append("F5");
        break;
    case (int)HotKeyFunc::EndEvent:
        msg.append("F6");
        break;
    case (int)HotKeyFunc::SaveMousePosition:
        msg.append("F7");
        break;
    case (int)HotKeyFunc::SaveAllSetting:
        msg.append("F8");
        break;
    }
    if(result){
        msg.append(QStringLiteral("热键绑定成功"));
    }
    else{
        msg.append(QStringLiteral("热键绑定失败"));
    }
    addMsg(msg);
}
