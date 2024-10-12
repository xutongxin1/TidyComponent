#include "mainwindow.h"
#include <QIcon>
#include <qsplitter.h>

#include "ColorDelegate.h"
#include "ElaDockWidget.h"

int record_DeviceNum = 0, record_WinNum = 0;

MainWindow::MainWindow(QWidget *parent) : ElaWindow(parent) {
    tableView = new ElaTableView(this);
    //初始化ElaUI
    initElaWindow();

    // 禁用tableView修改
    // ui_->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableView->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::SelectedClicked);
    // 初始化导入数据
    loadData();
    model.showAll = true;
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
    // setNavigationBarDisplayMode(ElaNavigationType::Compact);
    setIsNavigationBarEnable(false);
    // 拦截默认关闭事件
    _closeDialog = new ElaContentDialog(this);
    connect(_closeDialog, &ElaContentDialog::rightButtonClicked, this, &MainWindow::closeWindow);
    connect(_closeDialog, &ElaContentDialog::middleButtonClicked, this, &MainWindow::showMinimized);
    this->setIsDefaultClosed(false);
    connect(this, &MainWindow::closeButtonClicked, this, [=]() {
        _closeDialog->exec();
    });

    // 移动到中心
    moveToCenter();

    // 停靠窗口
    // ElaDockWidget *logDockWidget = new ElaDockWidget(this);
    // logDockWidget->setWidget(tableView);
    // logDockWidget->setFeatures(QDockWidget::NoDockWidgetFeatures);
    // logDockWidget->titleBarWidget()->hide();
    // this->addDockWidget(Qt::BottomDockWidgetArea, logDockWidget);
    // resizeDocks({logDockWidget}, {600}, Qt::Vertical);

    //_pivot界面
    // _pivot = new ElaPivot(this);
    // _pivot->setPivotSpacing(8);
    // _pivot->setMarkWidth(75);
    // _pivot->appendPivot("本地歌曲");
    // _pivot->appendPivot("下载歌曲");
    // _pivot->appendPivot("下载视频");
    // _pivot->appendPivot("正在下载");
    // _pivot->setCurrentIndex(0);


    _tabWidget = new ElaTabWidget(this);
    // _tabWidget->setFixedHeight(500);
    QLabel* page1 = new QLabel("新标签页1", this);
    page1->setAlignment(Qt::AlignCenter);
    QFont font = page1->font();
    font.setPixelSize(75);
    page1->setFont(font);
    QLabel* page2 = new QLabel("新标签页2", this);
    page2->setFont(font);
    page2->setAlignment(Qt::AlignCenter);
    QLabel* page3 = new QLabel("新标签页3", this);
    page3->setFont(font);
    page3->setAlignment(Qt::AlignCenter);
    QLabel* page4 = new QLabel("新标签页4", this);
    page4->setFont(font);
    page4->setAlignment(Qt::AlignCenter);
    _tabWidget->addTab(page1, QIcon(":/Resource/Image/Cirno.jpg"), "新标签页1");
    _tabWidget->addTab(page2, "新标签页2");
    _tabWidget->addTab(page3, "新标签页3");
    _tabWidget->addTab(page4, "新标签页4");

    _tabWidget->setTabsClosable(false);



    QSplitter *splitter = new QSplitter(Qt::Vertical);
    splitter->addWidget(_tabWidget);
    splitter->addWidget(tableView);

    addPageNode("HOME", splitter, ElaIconType::House);
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
