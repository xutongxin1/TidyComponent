#include "mainwindow.h"
#include <QIcon>


int record_DeviceNum = 0, record_WinNum = 0;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui_(new Ui::MainWindow) {
    ui_->setupUi(this);
    // 设置列数
    ui_->tableWidget->setColumnCount(16);

    // 设置水平表头标签
    QStringList headerLabels = {
        "显示", "名称", "嘉立创ID", "淘宝链接", "值", "封装", "别名1", "别名2", "别名3", "别名4",
        "别名5", "别名6", "别名7", "别名8", "别名9", "别名10", "淘宝链接"
    };
    ui_->tableWidget->setHorizontalHeaderLabels(headerLabels);

    // 禁用tableWidget修改
    ui_->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // 初始化导入数据
    loadData();

    // 绑定导入导出按钮
    connect(ui_->btn_export, &QPushButton::clicked, this, &MainWindow::exportJsonToExcel);
    connect(ui_->btn_import, &QPushButton::clicked, this, &MainWindow::importExcelToJson);

    connect(ui_->input_search, &QLineEdit::returnPressed, this, &MainWindow::search);
    connect(ui_->input_search, &QLineEdit::editingFinished, this, &MainWindow::search);

    ui_->label_nowSearch->hide();
}

MainWindow::~MainWindow() {
    delete ui_;
}

void MainWindow::GetConstructConfig() {
}

void MainWindow::SaveConstructConfig() {
}




void MainWindow::addButtonToTable(int row,int col, const QString &color) {
    // 创建按钮
    auto button = new FluDropDownButton(this);
    // connect(button, &QPushButton::clicked, [this, row, button]() { handleButtonClicked(row, button); });

    // 将十六进制颜色字符串转换为RGB
    int r = color.mid(0, 2).toInt(nullptr, 16);
    int g = color.mid(2, 2).toInt(nullptr, 16);
    int b = color.mid(4, 2).toInt(nullptr, 16);
    QString newBackgroundRgba = QString("background: rgba(%1, %2, %3, 0.7);").arg(r).arg(g).arg(b);

    // 设置按钮样式
    button->setStyleSheet(button->styleSheet().replace("background: rgba(255, 255, 255, 0.7);", newBackgroundRgba));

    button->setIcon(QIcon(FluIconUtils::getFluentIcon(FluAwesomeType::Mail)));
    button->addIconTextItem(QIcon(FluIconUtils::getFluentIcon(FluAwesomeType::Send)), "Send");
    button->addIconTextItem(QIcon(FluIconUtils::getFluentIcon(FluAwesomeType::MailReply)), "Reply");
    button->addIconTextItem(QIcon(FluIconUtils::getFluentIcon(FluAwesomeType::MailReplyAll)), "Reply All");

    // 添加按钮到单元格
    ui_->tableWidget->setCellWidget(row, col, button);


}

void MainWindow::InitConfig() {
    config_main_ini_ = new ConfigClass("main.ini", QSettings::IniFormat);
    device_count_ = config_main_ini_->value("/Device/device_num").toInt();
    config_device_ini_.emplace_back();
    for (int i = 1; i <= device_count_; i++) {
        config_device_ini_.push_back(new ConfigClass("Device" + QString::number(i) + ".ini",
                                                     QSettings::IniFormat));
        //        QSettings *tmp=new QSettings("config/Device" + QString::number(i) + ".ini",
        //                                       QSettings::IniFormat);
        //        config_device_ini_[i]->setValue("/ee/connect_mode",123);
        //        config_device_ini_[i]->sync();
    }
}
