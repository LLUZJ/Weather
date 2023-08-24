#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QCursor>
#include<QUrl>
#include<QDebug>
#include<QMessageBox>

#include<QJsonDocument>
#include<QJsonObject>
#include<QJsonArray>

#include<QDateTime>

#include "weathertool.h"

#include<QPainter>
#include<QPen>
#include<QPoint>
#include<QBrush>

#define INCREMENT 3     //温度每升高/降低1°，y轴坐标的增量
#define POINT_RADIUS 3  //曲线描点（空心圆）的大小
#define TEXT_OFFSET_X 12    //X轴偏移
#define TEXT_OFFSET_Y 12    //Y轴偏移

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //1.设置窗口属性
    setWindowFlag(Qt::FramelessWindowHint);             //设置窗口无边框
    setFixedSize(width(),height());                     //设置固定窗口大小


    //2.构建右键菜单
    mExitMenu =new QMenu(this);
    mExitAct =new QAction();

    mExitAct->setText("退出");                           //设置菜单项名称为退出
    mExitAct->setIcon(QIcon(":/res/close.png"));        //设置菜单项图标

    mExitMenu->addAction(mExitAct);                     //把菜单项添加到菜单中

    connect(mExitAct,&QAction::triggered,this,[=](){	//连接槽函数，实现点击即退出
        qApp->exit(0);
    });


    //3.网络通信请求（http通信）
    mNetAccessManager =new QNetworkAccessManager(this);
    connect(mNetAccessManager,&QNetworkAccessManager::finished,this,&MainWindow::onReplied);

    //直接在构造中，请求天气数据，（前期测试）
    //getWeatherInfo("101010100");        //北京
    //getWeatherInfo("101280101");        //广州
    getWeatherInfo("广州");

    //4.UI初始化
    //将控件添加到控件数组，方便循环处理
    //星期和日期
    mWeekList<<ui->lblWeek0<<ui->lblWeek1<<ui->lblWeek2<<ui->lblWeek3<<ui->lblWeek4<<ui->lblWeek5;
    mDateList<<ui->lblDate0<<ui->lblDate1<<ui->lblDate2<<ui->lblDate3<<ui->lblDate4<<ui->lblDate5;
    //天气和天气图标
    mTypeList<<ui->lblType0<<ui->lblType1<<ui->lblType2<<ui->lblType3<<ui->lblType4<<ui->lblType5;
    mTypeIconList<<ui->lblTypeIcon0<<ui->lblTypeIcon1<<ui->lblTypeIcon2<<ui->lblTypeIcon3<<ui->lblTypeIcon4<<ui->lblTypeIcon5;
    //天气指数
    mAqiList<<ui->lblQuality0<<ui->lblQuality1<<ui->lblQuality2<<ui->lblQuality3<<ui->lblQuality4<<ui->lblQuality5;
    //风向和风力
    mFxList<<ui->lblFx0<<ui->lblFx1<<ui->lblFx2<<ui->lblFx3<<ui->lblFx4<<ui->lblFx5;
    mFlList<<ui->lblFl0<<ui->lblFl1<<ui->lblFl2<<ui->lblFl3<<ui->lblFl4<<ui->lblFl5;

    //天气图标设置
    //以天气类型作为key，以资源路径作为value,初始化MAP
    mTypeMap.insert("暴雪",":/res/type/BaoXue.png");
    mTypeMap.insert("暴雨",":/res/type/BaoYu.png");
    mTypeMap.insert("暴雨到大暴雨",":/res/type/BaoYuDaoDaBaoYu.png");
    mTypeMap.insert("大暴雨",":/res/type/DaBaoYu.png");
    mTypeMap.insert("大暴雨到特大暴雨",":/res/type/DaBaoYuDaoTeDaBaoYu.png");
    mTypeMap.insert("大到暴雪",":/res/type/DaDaoBaoXue.png");
    mTypeMap.insert("大到暴雨",":/res/type/DaDaoBaoYu.png");
    mTypeMap.insert("大雪",":/res/type/DaXue.png");
    mTypeMap.insert("大雨",":/res/type/DaYu.png");
    mTypeMap.insert("冻雨",":/res/type/DongYu.png");
    mTypeMap.insert("多云",":/res/type/DuoYun.png");
    mTypeMap.insert("浮尘",":/res/type/FuChen.png");
    mTypeMap.insert("雷阵雨",":/res/type/LeiZhenYu.png");
    mTypeMap.insert("雷阵雨伴有冰雹",":/res/type/LeiZhenYuBanYouBingBao.png");
    mTypeMap.insert("霾",":/res/type/Mai.png");
    mTypeMap.insert("强沙尘暴",":/res/type/QiangShaChenBao.png");
    mTypeMap.insert("晴",":/res/type/Qing.png");
    mTypeMap.insert("沙尘暴",":/res/type/ShaChenBao.png");
    mTypeMap.insert("特大暴雨",":/res/type/TeDaBaoYu.png");
    mTypeMap.insert("undefined",":/res/type/undefined.png");
    mTypeMap.insert("雾",":/res/type/Wu.png");
    mTypeMap.insert("小到中雪",":/res/type/XiaoDaoZhongXue.png");
    mTypeMap.insert("小到中雨",":/res/type/XiaoDaoZhongYu.png");
    mTypeMap.insert("小雪",":/res/type/XiaoXue.png");
    mTypeMap.insert("小雨",":/res/type/XiaoYu.png");
    mTypeMap.insert("雪",":/res/type/Xue.png");
    mTypeMap.insert("扬沙",":/res/type/YangSha.png");
    mTypeMap.insert("阴",":/res/type/Yin.png");
    mTypeMap.insert("雨",":/res/type/Yu.png");
    mTypeMap.insert("雨夹雪",":/res/type/YuJiaXue.png");
    mTypeMap.insert("阵雪",":/res/type/ZhenXue.png");
    mTypeMap.insert("阵雨",":/res/type/ZhenYu.png");
    mTypeMap.insert("中到大雪",":/res/type/ZhongDaoDaXue.png");
    mTypeMap.insert("中到大雨",":/res/type/ZhongDaoDaYu.png");
    mTypeMap.insert("中雪",":/res/type/ZhongXue.png");
    mTypeMap.insert("中雨",":/res/type/ZhongYu.png");


    //5.给标签添加事件过滤器
    //给高低温曲线添加事件过滤器，参数指定位this,也就是当前窗口对象MainWindow
    //指定了当前窗口，就给当前窗口重写eventFiter方法，当有标签的事件到来时候，当前窗口就会对它进行过滤，当前窗口捕获了这个事件之后，就可以在当前窗口进行绘制曲线
    ui->lblHighCurve->installEventFilter(this);
    ui->lblLowCurve->installEventFilter(this);

    initsystemtrayIcon();       //系统托盘的初始化操作

}


MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::contextMenuEvent(QContextMenuEvent *event)     //重写父类的contextMenuEvent虚函数，其父类中默认是忽略右键菜单事件
{
    //弹出右键菜单
    mExitMenu->exec(QCursor::pos());        //跟踪鼠标，在鼠标右键的地方弹出这个菜单项

    event->accept();                        //调用accept，表示这个事件已经处理，不需要向上传递
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    mOffset =event->globalPos() - this->pos();          //偏移量=鼠标点击处距离桌面左上角的距离-窗口左上角距离桌面左上角的距离
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    this->move(event->globalPos() - mOffset);           //设置窗口移动到 (当前鼠标点击处距离桌面左上角的距离 - 偏移量)的位置
}


//void MainWindow::getWeatherInfo(QString cityCode)       //获取天气数据
//{
//    QUrl url("http://t.weather.itboy.net/api/weather/city/"+cityCode);
//    mNetAccessManager->get(QNetworkRequest(url));       //请求数据  ，请求成功就会调用onReplied
//}
void MainWindow::getWeatherInfo(QString cityName)       //获取天气数据
{
    QString cityCode=WeatherTool::getCityCode(cityName);        //通过getCityCode方法利用key：cityName去获取到cityCode

    //当输入城市错误时进行判断
    if(cityCode.isEmpty()){
        QMessageBox::warning(this,"天气","请检查输入是否正确！",QMessageBox::Ok);
        return;
    }

    QUrl url("http://t.weather.itboy.net/api/weather/city/"+cityCode);
    mNetAccessManager->get(QNetworkRequest(url));       //请求数据  ，请求成功就会调用onReplied
}


void MainWindow::parseJson(QByteArray &byteArray)       //数据解析
{
    QJsonParseError err;
    QJsonDocument doc =QJsonDocument::fromJson(byteArray,&err);     //如果出错会把出错信息存到err中
    if(err.error != QJsonParseError::NoError)           //如果err不等于NoError 就出错退出
    {
        return;
    }

    //开始解析
    QJsonObject rootObj  =doc.object();
    qDebug()<<rootObj.value("message").toString();

    //1.解析日期和城市
    mToday.date=rootObj.value("date").toString();                                   //解析出date
    mToday.city=rootObj.value("cityInfo").toObject().value("city").toString();      //先获取到json对象cityInfo 在在其中获取到city

    //2.解析yesterday
    QJsonObject objData=rootObj.value("data").toObject();
    QJsonObject objYesterday = objData.value("yesterday").toObject();       //先获取到json对象data,在去获取json对象yesterday

    mDay[0].week =objYesterday.value("week").toString();        //mDay是0-5的数组，其中对应yesterday的是第0元素
    mDay[0].date =objYesterday.value("ymd").toString();

    mDay[0].type =objYesterday.value("type").toString();

        //最高温最低温        高温低温读取到的数据是 "高温 18℃" 存在中文字符空格，这里只要获取其中的数字其中℃占一个字符
    QString s;
    s=objYesterday.value("high").toString().split(" ").at(1);       //获得高温字段的字符串，之后用split进行分割，分割依据是空格，取后面的数据，即01中的1，就可以取得18℃
    s=s.left(s.length()-1);       //将s缩短长度，即利用left方法，把s的长度-1就可以获得前面的18了
    //s=s.left(2);                //left方法：从左到右获取count个字符
    mDay[0].high=s.toInt();     //把s转为int型赋给high

    s=objYesterday.value("low").toString().split(" ").at(1);       //18℃
    s=s.left(s.length()-1);       //18
    //s=s.left(2);
    mDay[0].low=s.toInt();

        //风向风力
    mDay[0].fx=objYesterday.value("fx").toString();
    mDay[0].fl=objYesterday.value("fl").toString();

        //污染指数 aqi
    mDay[0].aqi=objYesterday.value("aqi").toDouble();

    //3.解析forcast中五天的数据
    QJsonArray forecastArr=objData.value("forecast").toArray();     //forecast 是一个json数组

    for(int i=0;i<5;i++)        //获取五天的数据，从今天开始的五天，不包括昨天
    {
        QJsonObject objForecast=forecastArr[i].toObject();      //获取forecast中的josn对象，进行赋值

        //不获取昨天的数据 所以应该是从1开始，所以参数应该是i+1开始
        mDay[i+1].week=objForecast.value("week").toString();
        mDay[i+1].date=objForecast.value("ymd").toString();

        //天气类型
        mDay[i+1].type=objForecast.value("type").toString();

        //高温低温
        QString s;
        s=objForecast.value("high").toString().split(" ").at(1);    //18℃
        s=s.left(s.length()-1);       //18
        //s=s.left(2);
        mDay[i+1].high=s.toInt();

        s=objForecast.value("low").toString().split(" ").at(1);    //18℃
        s=s.left(s.length()-1);       //18
        //s=s.left(2);
        mDay[i+1].low=s.toInt();

        //风向风力
        mDay[i+1].fx=objForecast.value("fx").toString();
        mDay[i+1].fl=objForecast.value("fl").toString();

        //污染指数 aqi
        mDay[i+1].aqi=objForecast.value("aqi").toDouble();

    }

    //4.解析今天的数据 用mToday去存储
    mToday.ganmao=objData.value("ganmao").toString();

    mToday.wendu=objData.value("wendu").toString().toInt();
    mToday.shidu=objData.value("shidu").toString();
    mToday.pm25=objData.value("pm25").toDouble();
    mToday.quality=objData.value("quality").toString();

    //5.forcast中第一个数组元素，也是今天的数据
    //今天的数据也属于五天之中的数据，所以要把今天的数据mToday也给到mDay
    mToday.type = mDay[1].type;

    mToday.fx = mDay[1].fx;
    mToday.fl = mDay[1].fl;

    mToday.high = mDay[1].high;
    mToday.low = mDay[1].low;


    //6.更新UI
    updateUI();

    //6.2
    //绘制高温曲线
    ui->lblHighCurve->update();     //刷新
    ui->lblLowCurve->update();
}

void MainWindow::updateUI()
{
    //1.更新日期，右上角
    //ui->lblDate->setText(mToday.date)         //直接获取json的数据 得到的日期是20220210的格式，做一个转换
    ui->lblDate->setText(QDateTime::fromString(mToday.date,"yyyyMMdd").toString("yyyy/MM/dd")+" "+mDay[1].week);    //显示右上角的日期+星期

    //2.更新城市,右边 城市框
    ui->lblCity->setText(mToday.city);

    //3.更新今天数据  左侧今天的数据
    ui->lblTemp->setText(QString::number(mToday.wendu)+"°");            //温度
    qDebug()<<"-----------------------------------------";
    //qDebug()<<"今天温度："<<mToday.wendu;

    ui->lblTypeIcon->setPixmap(mTypeMap[mToday.type]);              //天气图标，以通过type取到key，进到mTypeMap去获得它的value 继而获得图片资源地址，在贴上去，进而更新图标
    ui->lblType->setText(mToday.type);                              //天气文本
    ui->lblLowHigh->setText(QString::number(mToday.low)+"°"+"~"+QString::number(mToday.high)+"°");     //最低温到最高温
    //qDebug()<<"最低温度："<<mToday.low << "最高温度：" <<mToday.high;

    ui->lblGanMao->setText("感冒指数: "+mToday.ganmao);               //感冒指数
    ui->lblWindFx->setText(mToday.fx);                              //风向
    ui->lblWindFl->setText(mToday.fl);                              //风力

    ui->lblPM25->setText(QString::number(mToday.pm25));             //PM2.5
    qDebug()<<"pm2.5:"<<mToday.pm25;

    ui->lblShiDu->setText(mToday.shidu);                            //湿度
    ui->lblQuality->setText(mToday.quality);                        //空气质量

    //4.更新六天数据,右侧界面
    for(int i=0;i<6;i++)
    {
        //4.1更新日期和时间
        mWeekList[i]->setText("周"+mDay[i].week.right(1));           //转换格式将星期几 转成周几
        ui->lblWeek0->setText("昨天");                                //将前三天固定写为昨天今天明天
        ui->lblWeek1->setText("今天");
        ui->lblWeek2->setText("明天");

        QStringList ymdList=mDay[i].date.split("-");                //json数据日期格式为：2023-02-18    以"-"为分割依据，将其分为3部分012
        mDateList[i]->setText(ymdList[1]+"/"+ymdList[2]);           //取ymdList的12部分，即02/18

        //4.2更新天气类型
        mTypeList[i]->setText(mDay[i].type);
        mTypeIconList[i]->setPixmap(mTypeMap[mDay[i].type]);        //取到type作为key,到mTypeMap中找到value

        //4.3更新空气质量
        //对aqi进行判断，赋予相应等级
        if(mDay[i].aqi >= 0 && mDay[i].aqi <= 50)
        {
            mAqiList[i]->setText("优");                              //0~50  优
            mAqiList[i]->setStyleSheet("background-color: rgb(121, 184, 0);");      //设置风格样式，改变颜色
        }
        else if(mDay[i].aqi >= 50 && mDay[i].aqi <= 100)
        {
            mAqiList[i]->setText("良");                              //50~100  良
            mAqiList[i]->setStyleSheet("background-color: rgb(255, 187, 23);");      //设置风格样式，改变颜色
        }
        else if(mDay[i].aqi >= 100 && mDay[i].aqi <= 150)
        {
            mAqiList[i]->setText("轻度");                              //100~150  轻度
            mAqiList[i]->setStyleSheet("background-color: rgb(255, 87, 97);");      //设置风格样式，改变颜色
        }
        else if(mDay[i].aqi >= 150 && mDay[i].aqi <= 200)
        {
            mAqiList[i]->setText("中度");                              //150~200  中度
            mAqiList[i]->setStyleSheet("background-color: rgb(235, 17, 27);");      //设置风格样式，改变颜色
        }
        else if(mDay[i].aqi >= 200 && mDay[i].aqi <= 250)
        {
            mAqiList[i]->setText("重度");                              //200~250  重度
            mAqiList[i]->setStyleSheet("background-color: rgb(170, 0, 0);");      //设置风格样式，改变颜色
        }
        else
        {
            mAqiList[i]->setText("严重");                              //大于250  严重
            mAqiList[i]->setStyleSheet("background-color: rgb(110, 0, 0);");      //设置风格样式，改变颜色
        }

        //4.4 更新风力风向
        mFxList[i]->setText(mDay[i].fx);
        mFlList[i]->setText(mDay[i].fl);



    }
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)       //事件过滤器，捕获事件
{
    //对绘制高低温曲线的控件进行监听，捕获其事件，以绘制曲线
    if(watched==ui->lblHighCurve && event->type()==QEvent::Paint){
        paintHighCurve();
    }
    if(watched==ui->lblLowCurve && event->type()==QEvent::Paint){
        paintLowCurve();
    }
    return QWidget::eventFilter(watched,event);
}

void MainWindow::paintHighCurve()
{
    QPainter painter(ui->lblHighCurve);     //绘画区域

    //抗锯齿
    painter.setRenderHint(QPainter::Antialiasing,true);     //抗锯齿效果，true是开关

    //1.获取x坐标
    int pointX[6] = {0};        //定义x坐标的数组
    for(int i=0;i<6;i++){
        pointX[i]=mWeekList[i]->pos().x()+mWeekList[i]->width()/2;  //x坐标=上方日期控件的中心位置=日期控件的坐标(左上角顶点)+控件宽度的一半，就是中心位置
    }

    //2.获取y坐标
    int tempSum = 0;
    int tempAverage=0;
    for(int i=0;i<6;i++){
        tempSum += mDay[i].high;        //对6天的高温进行求和
    }
    tempAverage=tempSum/6;          //最高温的平均值
    //图中显示的曲线，在lbael框中，中心位置应该为平均值，若高于平均值在其上方，低于在下方；高一度就向上移动3像素
        //计算y轴坐标
    int pointY[6]={0};              //定义y坐标的数组
    int yCenter =ui->lblHighCurve->height()/2;      //确定平均值所在的区域，为label区域高度一半的位置
    for(int i=0;i<6;i++){
        //Qt中 坐标计算，向下是+，向上是-
        pointY[i] =yCenter-((mDay[i].high-tempAverage) * INCREMENT);        //y坐标=中心值高度-（温度-平均值）*3像素      如果平均值16，当日温度17则y=h-(17-16)*3=h-3,即在中心位置向上移动3像素
    }

    //3.开始绘制
    //3.1初始化画笔相关工具
    QPen pen=painter.pen();                 //获取画笔
    pen.setWidth(1);                        //设置画笔的宽度
    pen.setColor(QColor(255,170,0));        //设置画笔的颜色

    painter.setPen(pen);                    //设置画笔
    painter.setBrush(QColor(255,170,0));    //设置画刷颜色，填充内部

    //3.2画点，文本
    for(int i=0;i<6;i++){
        //显示点
        painter.drawEllipse(QPoint(pointX[i],pointY[i]),POINT_RADIUS,POINT_RADIUS);         //绘制点（圆心坐标，xy半径（这里是xy的偏移）），但此时是空心圆，需要画刷填充

        //显示温度
        painter.drawText(pointX[i]-TEXT_OFFSET_X,pointY[i]-TEXT_OFFSET_Y,QString::number(mDay[i].high)+"°");        //绘制文本，使其位于点的上方，（绘制点坐标（要一定偏移，不然和点重合了），文本信息）
    }
    //3.3 连线    两点之间确定一条直线，一共绘制五次
    for(int i=0;i<5;i++){       //不能到6，因为素组是0-5的，如果i可以到5的话，pointX[i+1]就到6了，会越界
        if(i==0){
            pen.setStyle(Qt::DotLine);      //虚线
            painter.setPen(pen);
        }
        else{
            pen.setStyle(Qt::SolidLine);    //实线
            painter.setPen(pen);
        }
        painter.drawLine(pointX[i],pointY[i],pointX[i+1],pointY[i+1]);              //（x1,y1,x2,y2）
    }
}

void MainWindow::paintLowCurve()            //绘制低温曲线
{
    QPainter painter(ui->lblLowCurve);

    //抗锯齿
    painter.setRenderHint(QPainter::Antialiasing,true);

    //1.获取x坐标
    int pointX[6] = {0};
    for(int i=0;i<6;i++){
        pointX[i]=mWeekList[i]->pos().x()+mWeekList[i]->width()/2;
    }

    //2.获取y坐标
    int tempSum = 0;
    int tempAverage=0;
    for(int i=0;i<6;i++){
        tempSum += mDay[i].low;
    }
    tempAverage=tempSum/6;          //最高温的平均值

    //计算y轴坐标
    int pointY[6]={0};
    int yCenter =ui->lblLowCurve->height()/2;
    for(int i=0;i<6;i++){
        pointY[i] =yCenter-((mDay[i].low-tempAverage) * INCREMENT);
    }

    //3.开始绘制
    //3.1初始化画笔相关工具
    QPen pen=painter.pen();
    pen.setWidth(1);                        //设置画笔的宽度
    pen.setColor(QColor(0,255,255));        //设置画笔的颜色

    painter.setPen(pen);
    painter.setBrush(QColor(0,255,255));    //设置画刷颜色，填充内部

    //3.2画点，文本
    for(int i=0;i<6;i++){
        //显示点
        painter.drawEllipse(QPoint(pointX[i],pointY[i]),POINT_RADIUS,POINT_RADIUS);         //绘制点，但此时是空心圆，需要画刷填充

        //显示温度
        painter.drawText(pointX[i]-TEXT_OFFSET_X,pointY[i]-TEXT_OFFSET_Y,QString::number(mDay[i].low)+"°");        //绘制文本，使其位于点的上方，利用偏移
    }
    //3.3 连线
    for(int i=0;i<5;i++){       //不能到6，因为素组是0-5的，如果i可以到5的话，pointX[i+1]就到6了，会越界
        if(i==0){
            pen.setStyle(Qt::DotLine);      //虚线
            painter.setPen(pen);
        }
        else{
            pen.setStyle(Qt::SolidLine);    //实线
            painter.setPen(pen);
        }
        painter.drawLine(pointX[i],pointY[i],pointX[i+1],pointY[i+1]);
    }
}

void MainWindow::onReplied(QNetworkReply *reply)        //数据处理
{
    qDebug()<<"onReplied success";

    //响应状态码为200，表示请求成功
    int status_code=reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    qDebug()<<"operation: "<<reply->operation();        //请求方式
    qDebug()<<"status code: "<<status_code;             //状态码
    qDebug()<<"url: "<<reply->url();                    //url
    qDebug()<<"raw header"<<reply->rawHeaderList();     //header

    //如果指定的城市编码不存在，就会报错
    //"Error transfering  http://t.weather.itboy.net/api/weather/city/000000000 -server replied: Not Found"
    if(reply->error()!= QNetworkReply::NoError || status_code !=200){
        qDebug("%s(%d) error:%s",__FUNCTION__,__LINE__,reply->errorString().toLatin1().data());
        QMessageBox::warning(this,"天气","请求数据失败！",QMessageBox::Ok);
    }
    else{
        //获取响应信息
        QByteArray byteArray=reply->readAll();
        qDebug()<<"readAll:"<<byteArray.data();
        parseJson(byteArray);                       //调用parseJson解析数据
    }

    reply->deleteLater();

}


void MainWindow::on_btnSearch_clicked()
{
    QString cityName =ui->leCity->text();
    getWeatherInfo(cityName);
}


//系统托盘
void MainWindow::systemtrayiconActivated(QSystemTrayIcon::ActivationReason reason)      //响应系统托盘的动作（双击操作）
{
    switch(reason){
    case QSystemTrayIcon::DoubleClick:      //双击
        //显示  隐藏  界面
        if(isHidden())
        {
            show();
        }
        else
        {
            hide();
        }
        break;
    default:
        break;
    }
}

void MainWindow::initsystemtrayIcon()       //系统托盘初始化操作
{
    mysystemTray =new QSystemTrayIcon(this);
    mysystemTray->setIcon(QIcon(":/res/WeaFor.png"));       //设置图标
    connect(mysystemTray,&QSystemTrayIcon::activated,this,&MainWindow::systemtrayiconActivated);

    //添加应该退出应用程序菜单
    QAction  *actionsystemquit =new QAction(QIcon(":/res/close.png"),u8"退出程序");     //创建一个功能--退出程序以及它的图标
    connect(actionsystemquit,&QAction::triggered,this,[=](){
        qApp->exit(0);
    });

    QMenu *pcontextmenu=new QMenu(this);        //创建菜单项
    pcontextmenu->addAction(actionsystemquit);      //给这个菜单项增加退出程序的功能
    mysystemTray->setContextMenu(pcontextmenu);     //把这个菜单项加入系统托盘
    mysystemTray->show();

}



