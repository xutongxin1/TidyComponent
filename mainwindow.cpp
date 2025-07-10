#include "mainwindow.h"

#include <QHeaderView>
#include <QIcon>
#include <qimagereader.h>
#include <qsplitter.h>

#include "ColorDelegate.h"
#include "ElaDockWidget.h"
#include "ElaLineEdit.h"
#include "ElaMessageBar.h"
#include "ElaMessageButton.h"
#include "ElaProgressBar.h"
#include "ElaPromotionView.h"
#include "ElaScrollPageArea.h"
#include "ElaText.h"
#include "FluBusyProgressRing.h"
#include "FluProgressRing.h"
#include "GetFileRequestHandler.h"
#include "GetRequestHandler.h"
#include "JustWrapDelegate.h"
#include <QApplication>
#include <QClipboard>
#include <qstandarditemmodel.h>

#include "ElaScrollArea.h"
#include "ElaScrollBar.h"
#include "ElaScrollPage.h"
#include "ShowInfoModel.h"
#include <QSerialPortInfo>

#include "ElaMenuBar.h"
#include "QColorAllocator.h"
#include "SerialPortManager.h"
int record_DeviceNum = 0, record_WinNum = 0;

MainWindow::MainWindow(QWidget *parent) : ElaWindow(parent) {
    model = new ComponentTableModel();
    tableView = new ResizedTableView(this);
    //初始化ElaUI
    initElaWindow();
    qDebug() << INFOPATH;

    // 禁用tableView修改
    // tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // connect(tableView, &QTableView::doubleClicked, this, [&] {
    //     QString tmp="1";
    // });
    // tableView->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::SelectedClicked);
    // 初始化导入数据
    loadDataFromFolder();
    initializeCoordinatesCache();
    model->showAll = true;
    tableView->setModel(model);

    // 为第 0 列设置自定义委托
    ColorDelegate *colorDelegate = new ColorDelegate(this);
    tableView->setItemDelegateForColumn(0, colorDelegate);

    auto *JustWrapDelegate0 = new JustWrapDelegate(this);
    tableView->setItemDelegateForColumn(1, JustWrapDelegate0);
    tableView->setItemDelegateForColumn(2, JustWrapDelegate0);
    tableView->setItemDelegateForColumn(3, JustWrapDelegate0);
    tableView->setItemDelegateForColumn(4, JustWrapDelegate0);
    tableView->setItemDelegateForColumn(5, JustWrapDelegate0);
    // tableView->setWordWrap(true);
    model->updateData();
    tableView->horizontalHeader()->setStretchLastSection(true);
    tableView->resizeRowsToContents();
    tableView->horizontalHeader()->setMinimumSectionSize(100);

    searchLogicInit();

    //绑定数据更新动作
    // connect(tableView, &QTableView::clicked, this, &MainWindow::updateContent);
    connect(tableView->selectionModel(), &QItemSelectionModel::selectionChanged, this,
            &MainWindow::updateComponentInfo);

    getDailySection();
    initAddComponentLogic();
    colorAllocator=new QColorAllocator();

    //更新tableView的UI比例
    tableView->setColumnWidth(1, 175);
    tableView->setColumnWidth(2, 420);
    tableView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    _showInfo_tableView->horizontalHeader()->setStretchLastSection(true);
    // _showInfo_tableView->resizeRowsToContents();
    _showInfo_tableView->setItemDelegateForColumn(1, JustWrapDelegate0);
    _showInfo_tableView->setColumnWidth(0, 100);
    if (DEBUG) {
        updateTypeStatistics();
        isConnectedToMesh=true;
        // // 前20次分配会优先使用基础颜色池中的高对比度颜色
        // for (int i = 0; i < 25; i++) {
        //     QColor color = colorAllocator->allocateColor(LED_MODE_FLASH_FAST_1);
        //     qDebug() << "分配颜色:" << color.name();
        // }
        //
        // // 查看总可用颜色数
        // qDebug() << "总可用颜色数:" << colorAllocator->getTotalAvailableColors();
    }
    // ui_->label_nowSearch->hide();
    initSerialPort();
    // 最大化
    showMaximized();
}

MainWindow::~MainWindow() {
    // delete ui_;
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

void MainWindow::ShowSuccessInfo(const QString &info, const QString &title) {
    ElaMessageBar::success(ElaMessageBarType::BottomLeft, title, info, 2000, this);
}
void MainWindow::ShowWarningInfo(const QString &info, const QString &title) {
    ElaMessageBar::warning(ElaMessageBarType::BottomLeft, title, info, 2000, this);
}
void MainWindow::ShowInfoInfo(const QString &info, const QString &title) {
    ElaMessageBar::information(ElaMessageBarType::BottomLeft, title, info, 2000, this);
}
void MainWindow::ShowErrorInfo(const QString &info, const QString &title) {
    ElaMessageBar::error(ElaMessageBarType::BottomLeft, title, info, 2000, this);
}
