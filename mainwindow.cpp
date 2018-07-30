#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <stdio.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    foreach(const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        QSerialPort serial;
        serial.setPort(info);
        if(serial.open(QIODevice::ReadWrite))
        {
            ui->comboBox_port->addItem(serial.portName());
            serial.close();
        }
    }
    /*//设置波特率下拉菜单默认显示第三项
    ui->BaudBox->setCurrentIndex(3);
    //关闭发送按钮的使能
    ui->sendButton->setEnabled(false);
    qDebug() << tr("界面设定成功！");
    */
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::Read_Data()
{
    QByteArray buf;
    buf = serial->readAll();
    if(!buf.isEmpty())
    {
        if(flag_showInHex == true){
            qDebug() << "buf.count: " << buf.count();
            int i = 0;
            QString str_buf;
            for(i = 0; i < buf.count(); i++){
                QString s;
                s.sprintf("%02X ", (unsigned char)buf.at(i));
                str_buf += s;
            }
            qDebug() << str_buf;

            ui->textBrowser_receive->setText(ui->textBrowser_receive->document()->toPlainText() + str_buf);
            ui->textBrowser_receive->moveCursor(QTextCursor::End);
        }else{
            QString str = ui->textBrowser_receive->toPlainText();
            str+=tr(buf);
            ui->textBrowser_receive->clear();
            ui->textBrowser_receive->append(str);
        }

    }
    buf.clear();
}

void MainWindow::on_pushButton_openPort_clicked()
{

    if(ui->pushButton_openPort->text()==tr("Open Port"))
    {
        bool ret;
        serial = new QSerialPort;
        //设置串口名
        serial->setPortName(ui->comboBox_port->currentText());
        qDebug() << ui->comboBox_port->currentText();
        //打开串口
        ret = serial->open(QIODevice::ReadWrite);
        qDebug() << "open: " << ret;
        //设置波特率
        serial->setBaudRate(ui->comboBox_baudRate->currentText().toInt());
        qDebug() << "setBaudRate: " << ret;
        //设置数据位数
        switch(ui->comboBox_byteSize->currentIndex())
        {
            case 8: serial->setDataBits(QSerialPort::Data8); break;
            default: break;
        }
        //设置奇偶校验
        switch(ui->comboBox_parity->currentIndex())
        {
            case 0: serial->setParity(QSerialPort::NoParity); break;
            default: break;
        }
        //设置停止位
        switch(ui->comboBox_stopBits->currentIndex())
        {
            case 1: serial->setStopBits(QSerialPort::OneStop); break;
            case 2: serial->setStopBits(QSerialPort::TwoStop); break;
            default: break;
        }


        //设置流控制
        serial->setFlowControl(QSerialPort::NoFlowControl);
        //关闭设置菜单使能
        ui->comboBox_port->setEnabled(false);
        ui->comboBox_baudRate->setEnabled(false);
        ui->comboBox_byteSize->setEnabled(false);
        ui->comboBox_parity->setEnabled(false);
        ui->comboBox_stopBits->setEnabled(false);
        ui->pushButton_openPort->setText(tr("Close Port"));
        ui->comboBox_flowControl->setEnabled(false);
        ui->pushButton_sendData->setEnabled(true);
        //连接信号槽
        QObject::connect(serial, &QSerialPort::readyRead, this, &MainWindow::Read_Data);
    }
    else
    {
        //关闭串口
        serial->clear();
        serial->close();
        serial->deleteLater();
        //恢复设置使能
        ui->comboBox_port->setEnabled(true);
        ui->comboBox_baudRate->setEnabled(true);
        ui->comboBox_byteSize->setEnabled(true);
        ui->comboBox_parity->setEnabled(true);
        ui->comboBox_stopBits->setEnabled(true);
        ui->pushButton_openPort->setText(tr("Open Port"));
        ui->comboBox_flowControl->setEnabled(true);
        ui->pushButton_sendData->setEnabled(false);
    }
}


void MainWindow::on_pushButton_sendData_clicked()
{
    if(flag_sendHexStr == true){
        //serial->write(ui->textEdit_send->toPlainText().toLatin1());
        QString dataStr = ui->textEdit_send->toPlainText();
        //如果发送的数据个数为奇数的，则在前面最后落单的字符前添加一个字符0
        if (dataStr.length() % 2){
            dataStr.insert(dataStr.length()-1, '0');
        }
        QByteArray sendData;
        StringToHex(dataStr, sendData);
        serial->write(sendData);
        for(int i = 0; i < sendData.count(); i ++){
            qDebug() << sendData.at(i);
        }
    }else{
        if(flag_sendWithEnter == true){
            QString str;
            str = ui->textEdit_send->toPlainText().toLatin1();
            str.append('\r\n');
            serial->write(str.toLatin1());
            qDebug() << ui->textEdit_send->toPlainText().toLatin1();
        }else{
            serial->write(ui->textEdit_send->toPlainText().toLatin1());
            qDebug() << ui->textEdit_send->toPlainText().toLatin1();
        }
    }
    //RxLCD->display(RxLCD->value() + sendData.size());
}

void MainWindow::StringToHex(QString str, QByteArray &senddata) //字符串转换为十六进制数据0-F
{
    int hexdata,lowhexdata;
    int hexdatalen = 0;
    int len = str.length();
    senddata.resize(len / 2);
    char lstr,hstr;
    for (int i = 0; i < len; ) {
        hstr = str[i].toLatin1();
        if (hstr == ' ') {
            ++i;
            continue;
        }
        ++i;
        if (i  >= len) break;
        lstr = str[i].toLatin1();
        hexdata = hstr - 0x30;
        lowhexdata = lstr -0x30;
        if ((hexdata == 16) || (lowhexdata == 16))
            break;
        else
            hexdata = hexdata*16 + lowhexdata;
        ++i;
        senddata[hexdatalen] = (char)hexdata;
        ++hexdatalen;
    }
    senddata.resize(hexdatalen);
}

void MainWindow::on_checkBox_sendHexStr_stateChanged(int arg1)
{
    //ui->checkBox_sendHexStr
    flag_sendHexStr = ui->checkBox_sendHexStr->isChecked();
}

void MainWindow::on_checkBox_showInHex_stateChanged(int arg1)
{
    flag_showInHex = ui->checkBox_showInHex->isChecked();
}

void MainWindow::on_pushButton_clearInformation_clicked()
{
    ui->textBrowser_receive->clear();
}

void MainWindow::on_checkBox_sendWithEnter_stateChanged(int arg1)
{
    flag_sendWithEnter = ui->checkBox_sendWithEnter->isChecked();
}
