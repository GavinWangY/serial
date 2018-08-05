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
        QSerialPort serial_test;
        serial_test.setPort(info);
        if(serial_test.open(QIODevice::ReadWrite))
        {
            ui->comboBox_port->addItem(serial_test.portName());
            serial_portName.append(serial_test.portName());
            serial_test.close();
        }
    }

    qDebug() << "auto scan port.";
    timer_autoScanPort = new QTimer(this);
    connect(timer_autoScanPort, SIGNAL(timeout()), this, SLOT(autoScanPort()));
    timer_autoScanPort->start(1000);

    /*//设置波特率下拉菜单默认显示第三项
    ui->BaudBox->setCurrentIndex(3);
    //关闭发送按钮的使能
    ui->sendButton->setEnabled(false);
    qDebug() << tr("界面设定成功！");
    */

    timer_autoSend = new QTimer(this);

    label_statusBar_S = new QLabel;
    label_statusBar_S->setMinimumSize(80, 20); // 设置标签最小大小
    label_statusBar_S->setFrameShape(QFrame::WinPanel); // 设置标签形状
    label_statusBar_S->setFrameShadow(QFrame::Sunken); // 设置标签阴影
    ui->statusBar->addWidget(label_statusBar_S);
    label_statusBar_S->setText(tr("S:0"));

    label_statusBar_R = new QLabel;
    label_statusBar_R->setMinimumSize(80, 20); // 设置标签最小大小
    label_statusBar_R->setFrameShape(QFrame::WinPanel); // 设置标签形状
    label_statusBar_R->setFrameShadow(QFrame::Sunken); // 设置标签阴影
    ui->statusBar->addWidget(label_statusBar_R);
    label_statusBar_R->setText(tr("R:0"));
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::Read_Data()
{
    QByteArray buf;
    buf = serial->readAll();
    receive_num += buf.count();
    label_statusBar_R->setText("R:" + QString::number(receive_num));
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
    qDebug() << "on_pushButton_openPort_clicked enter" ;
    if(ui->comboBox_port->currentText() == NULL){
        qDebug() << "Error: no valid port.";
        return;
    }
    if(ui->pushButton_openPort->text()==tr("Open Port"))
    {
        bool ret;
        serial = new QSerialPort;
        //设置串口名
        serial->setPortName(ui->comboBox_port->currentText());
        qDebug() << ui->comboBox_port->currentText();
        //打开串口
        if(false == serial->open(QIODevice::ReadWrite)){
            qDebug() << "Error: open failed.";
            return;
        }
        //设置波特率
        if(false == serial->setBaudRate(ui->comboBox_baudRate->currentText().toInt())){
            qDebug() << "Error: setBaudRate failed.";
            return;
        }
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
        timer_autoSend->stop();
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
        send_num += dataStr.length()/2;
        QByteArray sendData;
        StringToHex(dataStr, sendData);
        serial->write(sendData);
        for(int i = 0; i < sendData.count(); i ++){
            qDebug() << sendData.at(i);
        }
    }else{
        send_num += ui->textEdit_send->toPlainText().toLatin1().length();
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
    label_statusBar_S->setText("S:" + QString::number(send_num));
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
    send_num = 0;
    receive_num = 0;
    label_statusBar_S->setText("S:" + QString::number(send_num));
    label_statusBar_R->setText("R:" + QString::number(receive_num));
}

void MainWindow::on_checkBox_sendWithEnter_stateChanged(int arg1)
{
    flag_sendWithEnter = ui->checkBox_sendWithEnter->isChecked();
}

void MainWindow::on_checkBox_autoSend_stateChanged(int arg1)
{
    qDebug() << "on_checkBox_autoSend_stateChanged enter.";
    flag_autoSend = ui->checkBox_autoSend->isChecked();
    qDebug() << flag_autoSend;

    if(flag_autoSend == true){
        connect(timer_autoSend, SIGNAL(timeout()), this, SLOT(autoSend()));
        qDebug() << ui->lineEdit_timeTrans->text().toInt();
        timer_autoSend->start(ui->lineEdit_timeTrans->text().toInt());
    }else{
        qDebug() << "timer_autoSend->stop";
        timer_autoSend->stop();
    }
}

void MainWindow::autoSend()
{
    qDebug() << "autoSend enter." ;
    on_pushButton_sendData_clicked();
}

void MainWindow::autoScanPort()
{
    qDebug() << "autoScanPort enter.";
    ui->comboBox_port->clear();

    if(serial_portName.isEmpty() == false){
        ui->comboBox_port->addItem(serial_portName);
    }
    foreach(const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        QSerialPort serial_test;
        serial_test.setPort(info);
        if(serial_test.open(QIODevice::ReadWrite))
        {
            ui->comboBox_port->addItem(serial_test.portName());
            serial_test.close();
        }
    }
}
