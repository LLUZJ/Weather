#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include<QContextMenuEvent>
#include<QMouseEvent>

#include<QNetworkAccessManager>
#include<QNetworkReply>

#include"weatherdata.h"
#include<QLabel>

#include<QSystemTrayIcon>



QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

protected:
    void contextMenuEvent(QContextMenuEvent *event);    //重写父类菜单事件
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

    void getWeatherInfo(QString cityCode);  //获取天气数据

    void parseJson(QByteArray& byteArray);  //数据解析

    void updateUI();                        //更新UI

    bool eventFilter(QObject *watched,QEvent *event);       //重写父类的eventFilter方法
    //绘制高低温曲线
    void paintHighCurve();
    void paintLowCurve();




private slots:
    void onReplied(QNetworkReply *reply);   //当获取到网络请求的数据，就调用

    void on_btnSearch_clicked();

private:
    //右键退出
    QMenu *mExitMenu;       //右键退出的菜单
    QAction *mExitAct;      //退出的行为-菜单项

    //窗口移动
    QPoint mOffset;         //窗口移动时，鼠标与窗口左上角的偏移

    //http请求
    QNetworkAccessManager *mNetAccessManager;       //网络请求的私有成员变量

    //将获取的数据放入这两个类中，方便后续展示
    Today mToday;
    Day mDay[6];

    //创建一些控件数组
    //星期和日期
    QList<QLabel*> mWeekList;
    QList<QLabel*> mDateList;
    //天气和天气图标
    QList<QLabel*> mTypeList;
    QList<QLabel*> mTypeIconList;
    //天气污染指数
    QList<QLabel*> mAqiList;
    //风力风向
    QList<QLabel*> mFxList;
    QList<QLabel*> mFlList;

    //天气图标
    QMap<QString,QString> mTypeMap;     //用于英文到中文的转换


    //系统托盘
    QSystemTrayIcon *mysystemTray;
    void systemtrayiconActivated(QSystemTrayIcon::ActivationReason reason);     //响应系统托盘的动作（双击操作）
    void initsystemtrayIcon();          //系统托盘的初始化操作
    //void quitmusicplayer();             //退出应用程序



};
#endif // MAINWINDOW_H
