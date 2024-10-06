#include "mainwindow.h"
#include <QIcon>


int record_DeviceNum = 0, record_WinNum = 0;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui_(new Ui::MainWindow) {
    ui_->setupUi(this);

    // 禁用tableView修改
    ui_->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // 初始化导入数据
    loadData();

    ui_->tableView->setModel(&model);
    model.updateData();

    //选中时一行整体选中
    ui_->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
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
