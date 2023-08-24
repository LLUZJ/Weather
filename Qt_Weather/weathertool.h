#ifndef WEATHERTOOL_H
#define WEATHERTOOL_H
#include<QMap>
#include<QFile>

#include<QJsonArray>
#include<QJsonDocument>
#include<QJsonObject>
#include<QJsonParseError>
#include<QJsonValue>

class WeatherTool{

private:
    static QMap<QString,QString> mCityMap;      //声明一个静态成员，QMap

    static void initCityMap(){                  //初始化CityMap
        //1.读取文件
        QString filePath ="E:\\Project\\QT_Weather\\Qt_Weather\\citycode.json";

        QFile file(filePath);
        file.open(QIODevice::ReadOnly | QIODevice::Text);
        QByteArray json=file.readAll();
        file.close();

        //2.解析并写入map
        QJsonParseError err;
        QJsonDocument doc =QJsonDocument::fromJson(json,&err);         //如果解析错误，把错误信息存到err中
        if(err.error!= QJsonParseError::NoError)
        {
            return;
        }
        if(!doc.isArray())      //如果不是数组
        {
            return;
        }

        //开始解析
        QJsonArray cities=doc.array();      //把doc转换成QJsonArray格式
        for(int i=0;i<cities.size();i++)
        {
            QString city=cities[i].toObject().value("city_name").toString();
            QString code=cities[i].toObject().value("city_code").toString();

            //把获取到的json数据转换成QString格式然后存入QMap中
            if(code.size() > 0){                    //citycode.json文件当中，省份的city_code是空，不将其存入map
                mCityMap.insert(city,code);
            }
        }
    }

public:
    static QString getCityCode(QString cityName){       //声明静态成员函数getCityCode，通过key:city_name值去获得value:city_code
        if(mCityMap.isEmpty()){                         //如果位空就初始化citymap
            initCityMap();
        }
        QMap<QString,QString>::iterator it =mCityMap.find(cityName);        //获取iterator，进而拿到key和value
        //输入北京/北京市
        if(it==mCityMap.end()){
            it =mCityMap.find(cityName + "市");          //在遍历寻找北京市
        }
        if(it!=mCityMap.end()){
            return it.value();
        }
        return "";
    }
};

QMap<QString,QString> WeatherTool::mCityMap={};







#endif // WEATHERTOOL_H
