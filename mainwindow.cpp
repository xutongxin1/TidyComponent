#include "mainwindow.h"

#include <QHeaderView>
#include <QIcon>
#include <qimagereader.h>
#include <qsplitter.h>

#include "ColorDelegate.h"
#include "ElaDockWidget.h"
#include "ElaLineEdit.h"
#include "ElaScrollPageArea.h"
#include "GetFileRequestHandler.h"
#include "GetRequestHandler.h"

int record_DeviceNum = 0, record_WinNum = 0;

MainWindow::MainWindow(QWidget *parent) : ElaWindow(parent) {
    model = new ComponentTableModel();
    tableView = new ElaTableView(this);
    //初始化ElaUI
    initElaWindow();

    // 禁用tableView修改
    // ui_->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableView->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::SelectedClicked);
    // 初始化导入数据
    loadData();
    model->showAll = true;
    tableView->setModel(model);

    // 为第 0 列设置自定义委托
    ColorDelegate *colorDelegate = new ColorDelegate(this);
    tableView->setItemDelegateForColumn(0, colorDelegate);

    model->updateData();

    // 绑定导入导出按钮
    // connect(ui_->btn_export, &QPushButton::clicked, this, &MainWindow::exportJsonToExcel);
    // connect(ui_->btn_import, &QPushButton::clicked, this, &MainWindow::importExcelToJson);
    //
    connect(_searchBox, &QLineEdit::textChanged, this, &MainWindow::search);
    connect(_searchBox, &QLineEdit::editingFinished, this, [&] {
        if (_searchBox->text().isEmpty()) {
            search(); //防止刷新两次
        }
    });

    //获取
    getRequest("https://whyta.cn/api/tx/one?key=cc8cba0a7069",
               [&](QString data) {
                   // 处理成功的响应数据（QString）
                   // qDebug() << "Success:" << data;
                   QJsonDocument jsonDoc = QJsonDocument::fromJson(data.toUtf8());
                   if (jsonDoc.isNull() || !jsonDoc.isObject()) {
                       qWarning() << "JSON解析失败";
                       return -1;
                   }

                   // 获取顶层对象
                   QJsonObject jsonObj = jsonDoc.object();
                   QJsonObject resultObj = jsonObj.value("result").toObject();

                   // 提取字段
                   QString word = resultObj.value("word").toString();
                   QString imgurl = resultObj.value("imgurl").toString();
                   if (word.isEmpty() || imgurl.isEmpty())
                       qWarning() << "无法解析每日一言";
                   // 输出字段
                   // qDebug() << "Word:" << word;
                   // qDebug() << "Image URL:" << imgurl;
                   // qDebug() << "系统缓存目录路径:" << QStandardPaths::standardLocations(QStandardPaths::CacheLocation);
                   getFileRequest(imgurl,
                                  [&](QString filePath) {
                                      // qDebug() << "File downloaded to:" << filePath;
                                      QImageReader reader(filePath);
                                      reader.setAutoTransform(true);
                                      const QImage img = reader.read();
                                      _promotionCard->setFixedSize(img.size().width() * 350.0 / img.size().height(),
                                                                   350);
                                      _promotionCard->setCardPixmap(QPixmap(filePath));
                                  },
                                  [](QNetworkReply::NetworkError error) {
                                      // 处理失败的错误信息
                                      qWarning() << "无法获取每日一言 " << error;
                                  },
                                  "Daily.jpg",
                                  QStandardPaths::standardLocations(QStandardPaths::CacheLocation)[0]);
                   _promotionCard->setSubTitle(word);
               },
               [](QNetworkReply::NetworkError error) {
                   // 处理失败的错误信息
                   qWarning() << "无法获取每日一言 " << error;
               });

    // ui_->label_nowSearch->hide();
}

MainWindow::~MainWindow() {
    // delete ui_;
}
void MainWindow::initElaWindow() {
    resize(1200, 740);
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
    ElaDockWidget *infoDockWidget = new ElaDockWidget(this);
    infoDockWidget->setWindowTitle("元件信息");
    infoDockWidget->setAllowedAreas(Qt::RightDockWidgetArea | Qt::LeftDockWidgetArea);

    // logDockWidget->setFeatures(QDockWidget::NoDockWidgetFeatures);
    // logDockWidget->titleBarWidget()->hide();
    this->addDockWidget(Qt::RightDockWidgetArea, infoDockWidget);
    resizeDocks({infoDockWidget}, {600}, Qt::Vertical);
    resizeDocks({infoDockWidget}, {400}, Qt::Horizontal);
    ElaScrollPageArea *infoDockhArea = new ElaScrollPageArea(this);
    infoDockhArea->setMinimumHeight(0);
    infoDockhArea->setMaximumHeight(QWIDGETSIZE_MAX);
    QVBoxLayout *infoDockLayout = new QVBoxLayout(infoDockhArea);

    infoDockLayout->addStretch();
    infoDockWidget->setWidget(infoDockhArea);
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
    _tabWidget->setFixedHeight(350);

    //修改切换时效果
    connect(_tabWidget, &ElaTabWidget::currentChanged, this, [=](int index) {
        if (index == 0) {
            _tabWidget->setFixedHeight(350); //每日一言
        } else {
            _tabWidget->setFixedHeight(150); //其他窗口
        }
    });

    //首页栏

    _promotionCard = new ElaPromotionCard(this);

    _promotionCard->setCardTitle("每日一言");
    // _promotionCard->setPromotionTitle("SONG~");
    // _promotionCard->setTitle("STYX HELIX");

    _enterEditButton = new ElaToolButton(this);
    _enterEditButton->setIsTransparent(false);
    _enterEditButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    _enterEditButton->setBorderRadius(8);
    //_toolButton->setPopupMode(QToolButton::MenuButtonPopup);
    _enterEditButton->setText("进入编辑模式");
    _enterEditButton->setElaIcon(ElaIconType::Wrench);
    _enterEditButton->setIconSize(QSize(35, 35));
    _enterEditButton->setFixedSize(100, 75);

    ElaScrollPageArea *homeArea = new ElaScrollPageArea(this);
    homeArea->setMinimumHeight(0);
    homeArea->setMaximumHeight(QWIDGETSIZE_MAX);
    QHBoxLayout *homeAreaLayout = new QHBoxLayout(homeArea);
    homeAreaLayout->setContentsMargins(0, 0, 0, 0);
    homeAreaLayout->addWidget(_promotionCard);
    homeAreaLayout->addWidget(_enterEditButton);
    homeAreaLayout->addStretch();

    //搜索栏
    _searchBox = new ElaLineEdit(this);
    _searchBox->setFixedSize(500, 45);
    _searchBox->setFixedHeight(45);
    _searchBox->setPlaceholderText("今天要来点什么元器件呢？");

    _resetSearchButton = new ElaToolButton(this);
    _resetSearchButton->setIsTransparent(false);
    _resetSearchButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    _resetSearchButton->setBorderRadius(8);
    //_toolButton->setPopupMode(QToolButton::MenuButtonPopup);
    _resetSearchButton->setText("搜索复位");
    _resetSearchButton->setElaIcon(ElaIconType::FilterSlash);
    _resetSearchButton->setIconSize(QSize(35, 35));
    _resetSearchButton->setFixedSize(100, 75);

    _importSearchButton = new ElaToolButton(this);
    _importSearchButton->setIsTransparent(false);
    _importSearchButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    _importSearchButton->setBorderRadius(8);
    //_toolButton->setPopupMode(QToolButton::MenuButtonPopup);
    _importSearchButton->setText("导入BOM表\n进行搜索");
    _importSearchButton->setElaIcon(ElaIconType::FileImport);
    _importSearchButton->setIconSize(QSize(35, 35));
    _importSearchButton->setFixedSize(130, 75);

    QWidget *searchArea = new QWidget(this);
    // searchArea->setMinimumHeight(0);
    // searchArea->setMaximumHeight(QWIDGETSIZE_MAX);
    QHBoxLayout *searchAreaLayout = new QHBoxLayout(searchArea);
    searchAreaLayout->addWidget(_searchBox);
    searchAreaLayout->addWidget(_resetSearchButton);
    searchAreaLayout->addWidget(_importSearchButton);
    searchAreaLayout->addStretch();



    _addComponentButton = new ElaToolButton(this);
    _addComponentButton->setIsTransparent(false);
    _addComponentButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    _addComponentButton->setBorderRadius(8);
    //_toolButton->setPopupMode(QToolButton::MenuButtonPopup);
    _addComponentButton->setText("添加元器件");
    _addComponentButton->setElaIcon(ElaIconType::Plus);
    _addComponentButton->setIconSize(QSize(35, 35));
    _addComponentButton->setFixedSize(100, 75);

    // _delComponentButton = new ElaToolButton(this);
    // _delComponentButton->setIsTransparent(false);
    // _delComponentButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    // _delComponentButton->setBorderRadius(8);
    // //_toolButton->setPopupMode(QToolButton::MenuButtonPopup);
    // _delComponentButton->setText("删除元器件");
    // _delComponentButton->setElaIcon(ElaIconType::FilterSlash);
    // _delComponentButton->setIconSize(QSize(35, 35));
    // _delComponentButton->setFixedSize(100, 75);


    QWidget *editArea = new QWidget(this);
    // searchArea->setMinimumHeight(0);
    // searchArea->setMaximumHeight(QWIDGETSIZE_MAX);
    QHBoxLayout *editAreaLayout = new QHBoxLayout(editArea);
    editAreaLayout->addWidget(_addComponentButton);
    // editAreaLayout->addWidget(_delComponentButton);
    editAreaLayout->addStretch();


    _tabWidget->addTab(homeArea, "首页");
    _tabWidget->addTab(searchArea, "搜索");
    _tabWidget->addTab(editArea, "编辑模式");
    // _tabWidget->addTab(page4, "元器件操作");


    _tabWidget->setTabsClosable(false);

    QWidget *centerWidget = new QWidget(this);
    QVBoxLayout *centerVBoxLayout = new QVBoxLayout(centerWidget);
    centerVBoxLayout->addWidget(_tabWidget);
    centerVBoxLayout->addWidget(tableView);
    centerVBoxLayout->setContentsMargins(0, 0, 0, 0);

    addPageNode("HOME", centerWidget, ElaIconType::House);

    tableView->setSelectionBehavior(QAbstractItemView::SelectRows); //选中时一行整体选中

    // tableView->setTextElideMode(Qt::ElideNone);
    // connect(model, &ComponentTableModel::dataChanged, tableView, &ElaTableView::resizeRowsToContents);
    // connect(tableView->horizontalHeader(), &QHeaderView::sectionResized, this, [=](int index, int /*oldSize*/, int /*newSize*/) {
    //     qDebug()<<tableView->styleSheet();
    //     tableView->setWordWrap(true);
    //     tableView->resizeRowsToContents();
    // });
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
