#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QTimer>
#include <QLabel>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void Read_Data();

    void on_pushButton_openPort_clicked();

    void on_pushButton_sendData_clicked();

    void on_checkBox_sendHexStr_stateChanged(int arg1);

    void on_checkBox_showInHex_stateChanged(int arg1);

    void on_pushButton_clearInformation_clicked();

    void on_checkBox_sendWithEnter_stateChanged(int arg1);

    void on_checkBox_autoSend_stateChanged(int arg1);

    void autoSend();

    void autoScanPort();

private:
    Ui::MainWindow *ui;
    QSerialPort *serial;
    void StringToHex(QString str, QByteArray &senddata);
    bool flag_sendHexStr = false;
    bool flag_showInHex = false;
    bool flag_sendWithEnter = false;
    bool flag_autoSend = false;
    QTimer *timer_autoScanPort;
    QTimer *timer_autoSend;
    QLabel *label_statusBar_S;
    QLabel *label_statusBar_R;
    long send_num = 0;
    long receive_num = 0;
    QString serial_portName;
};



#endif // MAINWINDOW_H
