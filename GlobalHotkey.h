#ifndef GLOBALHOTKEY_H
#define GLOBALHOTKEY_H

#include <QWidget>
#include <QAbstractNativeEventFilter>
#include <QThread>
#include <QTime>
#include <QTimer>
#include <QDebug>
#include <Windows.h>
#include <WinUser.h>

namespace Ui {
class GlobalHotkey;
}

class MouseClick :public QObject{
    Q_OBJECT
public:
    explicit MouseClick():
        totalClick(0),
        curClick(0),
        isUnlimited(false){
        connect(&timer,&QTimer::timeout,this,&MouseClick::click);
    }
    MouseClick(MouseClick&) = delete;
    ~MouseClick(){
        //
    }
    const QTime & run(int n = 100,int button = Qt::LeftButton){
        startTime = QTime::currentTime();
        curClick = 0;
        isUnlimited = n == 0;
        this->button = button;
        timer.setSingleShot(isUnlimited);
        timer.start(isUnlimited?1:1000.0/qreal(n));
        return startTime;
    }
    const QTime & stop(){
        endTime = QTime::currentTime();
        isUnlimited = false;
        timer.stop();
        totalClick += curClick;
        return endTime;
    }
    const QTime & getStartTime(){        return startTime;    }
    const QTime & getEndTime(){        return endTime;    }
    const qint64 & getCurClick(){        return curClick;    }
    const qint64 & getTotalClick(){        return totalClick;    }
    void setMouseX(const int& value){   mouseX = value;     }
    void setMouseY(const int& value){   mouseY = value;     }
private slots:
    void click(){
        do{
            ++curClick;
            SetCursorPos(mouseX,mouseY);
            switch(button){
            case Qt::LeftButton:
                mouse_event(MOUSEEVENTF_LEFTDOWN,0,0,0,0);//按下左键
                mouse_event(MOUSEEVENTF_LEFTUP,mouseX,mouseY,0,0);//松开左键
                break;
            case Qt::MiddleButton:
                mouse_event(MOUSEEVENTF_MIDDLEDOWN,0,0,0,0);//按下中键
                mouse_event(MOUSEEVENTF_MIDDLEUP,0,0,0,0);//松开中键
                break;
            case Qt::RightButton:
                mouse_event(MOUSEEVENTF_RIGHTDOWN,0,0,0,0);//按下右键
                mouse_event(MOUSEEVENTF_RIGHTUP,0,0,0,0);//松开右键
                break;
            default:
                return;
            }
        }while(isUnlimited);
    }
private:
    QTime startTime;
    QTime endTime;
    qint64 totalClick;
    qint64 curClick;
    bool isUnlimited;
    int button;
    QTimer timer;
    int mouseX;
    int mouseY;
};

class GlobalHotkey : public QWidget
{
    Q_OBJECT
public:
    enum class HotKeyFunc{
        StartEvent = 0x02B,
        EndEvent,
        SaveMousePosition,
        SaveAllSetting
    };
public:
    explicit GlobalHotkey(QWidget *parent = 0);
    bool nativeEvent(const QByteArray &eventType, void *message, long *result);
    ~GlobalHotkey();

    void run();
    void stop();
    void addMsg(const QString & str);
    void setMousePosition(const int &x,const int &y);
    int getButton();
protected:
    virtual void mouseMoveEvent(QMouseEvent *event) override;
    virtual void changeEvent(QEvent *event);

private:
    void showHotKeyConnectResult(HotKeyFunc,bool result);
private:
    Ui::GlobalHotkey *ui;

    bool isF5;
    bool isF6;
    bool isF7;
    bool isF8;

    QThread *thread;
    MouseClick clickEvent;
};

#endif // GLOBALHOTKEY_H
