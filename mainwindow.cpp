#include "mainwindow.h"
#include <QIcon>

#include "ColorDelegate.h"
#include "ElaDockWidget.h"

int record_DeviceNum = 0, record_WinNum = 0;


MainWindow::MainWindow(QWidget *parent) : ElaWindow(parent) {

    tableView = new ElaTableView(this);
    //初始化ElaUI
    initElaWindow();


    // 禁用tableView修改
    // ui_->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableView->setEditTriggers(QAbstractItemView::DoubleClicked  | QAbstractItemView::SelectedClicked);
    // 初始化导入数据
    loadData();
    model.showAll=true;
    tableView->setModel(&model);

    // 为第 0 列设置自定义委托
    ColorDelegate *colorDelegate = new ColorDelegate(this);
    tableView->setItemDelegateForColumn(0, colorDelegate);

    model.updateData();
    //选中时一行整体选中
    // ui_->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    // 绑定导入导出按钮
    // connect(ui_->btn_export, &QPushButton::clicked, this, &MainWindow::exportJsonToExcel);
    // connect(ui_->btn_import, &QPushButton::clicked, this, &MainWindow::importExcelToJson);
    //
    // connect(ui_->input_search, &QLineEdit::returnPressed, this, &MainWindow::search);
    // connect(ui_->input_search, &QLineEdit::editingFinished, this, &MainWindow::search);

    // ui_->label_nowSearch->hide();
}

MainWindow::~MainWindow() {
    // delete ui_;
}
void MainWindow::initElaWindow() {
    setWindowIcon(QIcon(":/include/Image/Cirno.jpg"));
    setWindowTitle("Component Search");
    setUserInfoCardVisible(false);
    setNavigationBarDisplayMode(ElaNavigationType::Maximal);
    setNavigationViewWidth(200);
    // 拦截默认关闭事件
    _closeDialog = new ElaContentDialog(this);
    connect(_closeDialog, &ElaContentDialog::rightButtonClicked, this, &MainWindow::closeWindow);
    connect(_closeDialog, &ElaContentDialog::middleButtonClicked, this, &MainWindow::showMinimized);
    this->setIsDefaultClosed(false);
    connect(this, &MainWindow::closeButtonClicked, this, [=]() {
        _closeDialog->exec();
    });

    //移动到中心
    moveToCenter();

    //停靠窗口
    ElaDockWidget* logDockWidget = new ElaDockWidget( this);
    logDockWidget->setWidget(tableView);
    logDockWidget->setFeatures(QDockWidget::NoDockWidgetFeatures );
    logDockWidget->titleBarWidget()->hide();
    this->addDockWidget(Qt::RightDockWidgetArea, logDockWidget);
    resizeDocks({logDockWidget}, {600}, Qt::Horizontal);
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
