#include "ElaScrollArea.h"
#include "ElaScrollBar.h"
#include "mainwindow.h"
//
// Created by xtx on 25-5-20.
//
void MainWindow::initElaWindow() {
    resize(1920, 1080);
    setWindowIcon(QIcon(":/include/Image/Cirno.jpg"));
    setWindowTitle("Component Search");
    setUserInfoCardVisible(false);
    // setNavigationBarDisplayMode(ElaNavigationType::Compact);
    setIsNavigationBarEnable(false);

    //初始化菜单栏
    ElaMenuBar *menuBar = new ElaMenuBar(this);
    menuBar->setFixedHeight(30);
    QWidget *customWidget = new QWidget(this);
    QVBoxLayout *customLayout = new QVBoxLayout(customWidget);
    customLayout->setContentsMargins(0, 0, 0, 0);
    customLayout->addWidget(menuBar);
    customLayout->addStretch();
    // this->setMenuBar(menuBar);
    this->setCustomWidget(ElaAppBarType::MiddleArea, customWidget);
    this->setCustomWidgetMaximumWidth(500);
    connectUserStateAction = menuBar->addElaIconAction(ElaIconType::Plug, "未连接到用户侧");
    connect(connectUserStateAction, &QAction::triggered, this, [&]() {
    });
    // 拦截默认关闭事件
    // _closeDialog = new ElaContentDialog(this);
    // connect(_closeDialog, &ElaContentDialog::rightButtonClicked, this, &MainWindow::closeWindow);
    // connect(_closeDialog, &ElaContentDialog::middleButtonClicked, this, &MainWindow::showMinimized);
    // this->setIsDefaultClosed(false);
    // connect(this, &MainWindow::closeButtonClicked, this, [=]() {
    //     _closeDialog->exec();
    // });

    // 停靠窗口
    _infoDockWidget = new ElaDockWidget(this);
    this->addDockWidget(Qt::RightDockWidgetArea, _infoDockWidget);
    _infoDockWidget->setAllowedAreas(Qt::RightDockWidgetArea | Qt::LeftDockWidgetArea);
    resizeDocks({_infoDockWidget}, {600}, Qt::Vertical);
    resizeDocks({_infoDockWidget}, {400}, Qt::Horizontal);
    _infoDockWidget->setFeatures(QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable);
    //元件信息栏初始化
    _showInfo_Widget = new QWidget(this);
    const auto infoDockLayout = new QVBoxLayout(_showInfo_Widget);

    //初始化滚动栏
    _showInfo_scrollArea = new ElaScrollArea(this);
    _showInfo_scrollArea->setMouseTracking(true);
    _showInfo_scrollArea->setIsAnimation(Qt::Vertical, true);
    _showInfo_scrollArea->setWidgetResizable(true);
    _showInfo_scrollArea->setIsGrabGesture(false, 0);
    _showInfo_scrollArea->setIsOverShoot(Qt::Vertical, true);
    _showInfo_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ElaScrollBar *floatVScrollBar = new ElaScrollBar(_showInfo_scrollArea->verticalScrollBar(), _showInfo_scrollArea);
    floatVScrollBar->setIsAnimation(true);
    _showInfo_scrollArea->setWidget(_showInfo_Widget);

    //初始化无器件的提示文字
    _showInfo_NoComponentTips = new ElaText("请选一个元器件以显示详细信息", this);
    _showInfo_NoComponentTips->setTextPixelSize(20);
    infoDockLayout->addWidget(_showInfo_NoComponentTips);

    // 打开网站按钮和数据手册按钮
    _showInfo_Web_Widget = new QWidget(this);
    const auto _showInfo_Web_Widget_Layout = new QHBoxLayout(_showInfo_Web_Widget);

    _showInfo_OpenWebSiteButton = new ElaToolButton(this);
    _showInfo_OpenWebSiteButton->setIsTransparent(false);
    _showInfo_OpenWebSiteButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    _showInfo_OpenWebSiteButton->setBorderRadius(8);
    _showInfo_OpenWebSiteButton->setText("打开立创详情页");
    _showInfo_OpenWebSiteButton->setElaIcon(ElaIconType::Globe);
    _showInfo_OpenWebSiteButton->setIconSize(QSize(35, 35));
    _showInfo_OpenWebSiteButton->setFixedSize(100, 75);

    _showInfo_OpenPDFButton = new ElaToolButton(this);
    _showInfo_OpenPDFButton->setIsTransparent(false);
    _showInfo_OpenPDFButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    _showInfo_OpenPDFButton->setBorderRadius(8);
    _showInfo_OpenPDFButton->setText("打开数据手册");
    _showInfo_OpenPDFButton->setElaIcon(ElaIconType::FileDoc);
    _showInfo_OpenPDFButton->setIconSize(QSize(35, 35));
    _showInfo_OpenPDFButton->setFixedSize(100, 75);

    _showInfo_updateInfoButton = new ElaToolButton(this);
    _showInfo_updateInfoButton->setIsTransparent(false);
    _showInfo_updateInfoButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    _showInfo_updateInfoButton->setBorderRadius(8);
    _showInfo_updateInfoButton->setText("更新器件数据");
    _showInfo_updateInfoButton->setElaIcon(ElaIconType::ArrowsRotateReverse);
    _showInfo_updateInfoButton->setIconSize(QSize(35, 35));
    _showInfo_updateInfoButton->setFixedSize(100, 75);

    _showInfo_Web_Widget_Layout->addWidget(_showInfo_OpenWebSiteButton);
    _showInfo_Web_Widget_Layout->addWidget(_showInfo_OpenPDFButton);
    _showInfo_Web_Widget_Layout->addWidget(_showInfo_updateInfoButton);
    infoDockLayout->addWidget(_showInfo_Web_Widget);

    //数据表初始化
    _showInfo_tableView = new ElaTableView(this);
    _showInfo_model = new ShowInfoModel(this);
    _showInfo_tableView->setModel(_showInfo_model);
    _showInfo_tableView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    _showInfo_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    _showInfo_tableView->setEditTriggers(QAbstractItemView::NoEditTriggers); // Make the table read-only
    _showInfo_tableView->horizontalHeader()->setHidden(true);
    _showInfo_tableView->verticalHeader()->setHidden(true);
    infoDockLayout->addWidget(_showInfo_tableView);

    //图片初始化
    _showInfo_PNGView = new ElaPromotionView(this);
    _showInfo_SCHPCBview = new ElaPromotionView(this);
    _showInfo_PNGCard1 = new ElaPromotionCard(_showInfo_PNGView);
    _showInfo_PNGCard2 = new ElaPromotionCard(_showInfo_PNGView);
    _showInfo_PNGCard3 = new ElaPromotionCard(_showInfo_PNGView);
    _showInfo_PNGCard4 = new ElaPromotionCard(_showInfo_PNGView);
    _showInfo_PNGCard5 = new ElaPromotionCard(_showInfo_PNGView);
    _showInfo_SCHCard1 = new ElaPromotionCard(_showInfo_SCHPCBview);
    _showInfo_SCHCard2 = new ElaPromotionCard(_showInfo_SCHPCBview);
    _showInfo_SCHCard3 = new ElaPromotionCard(_showInfo_SCHPCBview);
    _showInfo_SCHCard4 = new ElaPromotionCard(_showInfo_SCHPCBview);
    _showInfo_SCHCard5 = new ElaPromotionCard(_showInfo_SCHPCBview);
    _showInfo_PCBCard1 = new ElaPromotionCard(_showInfo_SCHPCBview);
    _showInfo_PCBCard2 = new ElaPromotionCard(_showInfo_SCHPCBview);
    _showInfo_PCBCard3 = new ElaPromotionCard(_showInfo_SCHPCBview);
    _showInfo_PCBCard4 = new ElaPromotionCard(_showInfo_SCHPCBview);
    _showInfo_PCBCard5 = new ElaPromotionCard(_showInfo_SCHPCBview);
    _showInfo_PNGCard1->hide();
    _showInfo_PNGCard2->hide();
    _showInfo_PNGCard3->hide();
    _showInfo_PNGCard4->hide();
    _showInfo_PNGCard5->hide();
    _showInfo_SCHCard1->hide();
    _showInfo_SCHCard2->hide();
    _showInfo_SCHCard3->hide();
    _showInfo_SCHCard4->hide();
    _showInfo_SCHCard5->hide();
    _showInfo_PCBCard1->hide();
    _showInfo_PCBCard2->hide();
    _showInfo_PCBCard3->hide();
    _showInfo_PCBCard4->hide();
    _showInfo_PCBCard5->hide();
    infoDockLayout->addWidget(_showInfo_PNGView);
    infoDockLayout->addWidget(_showInfo_SCHPCBview);
    infoDockLayout->addStretch();

    //默认隐藏策略
    _showInfo_tableView->hide();
    _showInfo_PNGView->hide();
    _showInfo_SCHPCBview->hide();
    _showInfo_Web_Widget->hide();

    //初始化元件添加栏
    InitAddComponentDockUI();

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

    _searchTypeButton = new ElaToolButton(this);
    _searchTypeButton->setIsTransparent(false);
    _searchTypeButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    _searchTypeButton->setBorderRadius(8);
    _searchTypeButton->setText("搜索方式");
    _searchTypeButton->setElaIcon(ElaIconType::Filters);
    _searchTypeButton->setIconSize(QSize(35, 35));
    _searchTypeButton->setFixedSize(100, 75);

    _importSearchButton = new ElaToolButton(this);
    _importSearchButton->setIsTransparent(false);
    _importSearchButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    _importSearchButton->setBorderRadius(8);
    //_toolButton->setPopupMode(QToolButton::MenuButtonPopup);
    _importSearchButton->setText("导入BOM表\n进行搜索");
    _importSearchButton->setElaIcon(ElaIconType::FileImport);
    _importSearchButton->setIconSize(QSize(35, 35));
    _importSearchButton->setFixedSize(120, 75);

    _openEDAChromeButton = new ElaToolButton(this);
    _openEDAChromeButton->setIsTransparent(false);
    _openEDAChromeButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    _openEDAChromeButton->setBorderRadius(8);
    _openEDAChromeButton->setText("打开\nEDA Chrome注入");
    _openEDAChromeButton->setElaIcon(ElaIconType::FileImport);
    _openEDAChromeButton->setIconSize(QSize(35, 35));
    _openEDAChromeButton->setFixedSize(120, 75);

    InitEDAChromeHttpServer();

    QWidget *searchArea = new QWidget(this);
    // searchArea->setMinimumHeight(0);
    // searchArea->setMaximumHeight(QWIDGETSIZE_MAX);
    QHBoxLayout *searchAreaLayout = new QHBoxLayout(searchArea);
    searchAreaLayout->addWidget(_searchBox);
    searchAreaLayout->addWidget(_resetSearchButton);
    searchAreaLayout->addWidget(_searchTypeButton);
    searchAreaLayout->addWidget(_importSearchButton);
    searchAreaLayout->addWidget(_openEDAChromeButton);
    searchAreaLayout->addStretch();

    _return_ALLButton = new ElaToolButton(this);
    _return_ALLButton->setIsTransparent(false);
    _return_ALLButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    _return_ALLButton->setBorderRadius(8);
    _return_ALLButton->setText("归还全部元件");
    _return_ALLButton->setElaIcon(ElaIconType::DownToBracket);
    _return_ALLButton->setIconSize(QSize(35, 35));
    _return_ALLButton->setFixedSize(100, 75);

    _returnButton = new ElaToolButton(this);
    _returnButton->setIsTransparent(false);
    _returnButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    _returnButton->setBorderRadius(8);
    _returnButton->setText("归还该元件");
    _returnButton->setElaIcon(ElaIconType::ArrowDownToBracket);
    _returnButton->setIconSize(QSize(35, 35));
    _returnButton->setFixedSize(100, 75);

    _applyButton = new ElaToolButton(this);
    _applyButton->setIsTransparent(false);
    _applyButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    _applyButton->setBorderRadius(8);
    _applyButton->setText("寻找该元件");
    _applyButton->setElaIcon(ElaIconType::ArrowUpFromBracket);
    _applyButton->setIconSize(QSize(35, 35));
    _applyButton->setFixedSize(100, 75);

    _apply_LightButton = new ElaToolButton(this);
    _apply_LightButton->setIsTransparent(false);
    _apply_LightButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    _apply_LightButton->setBorderRadius(8);
    _apply_LightButton->setText("亮灯寻找该元件");
    _apply_LightButton->setElaIcon(ElaIconType::LightbulbOn);
    _apply_LightButton->setIconSize(QSize(35, 35));
    _apply_LightButton->setFixedSize(100, 75);

    _apply_Light_VoiceButton = new ElaToolButton(this);
    _apply_Light_VoiceButton->setIsTransparent(false);
    _apply_Light_VoiceButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    _apply_Light_VoiceButton->setBorderRadius(8);
    _apply_Light_VoiceButton->setText("声光寻找该元件");
    _apply_Light_VoiceButton->setElaIcon(ElaIconType::LightEmergencyOn);
    _apply_Light_VoiceButton->setIconSize(QSize(35, 35));
    _apply_Light_VoiceButton->setFixedSize(100, 75);

    _noReturnTips = new ElaText("请先选择或搜索一个元件", this);
    _noReturnTips->setWordWrap(false);
    _noReturnTips->setTextPixelSize(20);

    auto *applyArea = new QWidget(this);
    auto *applyAreaLayout = new QHBoxLayout(applyArea);
    applyAreaLayout->addWidget(_return_ALLButton);
    applyAreaLayout->addWidget(_returnButton);
    applyAreaLayout->addWidget(_applyButton);
    applyAreaLayout->addWidget(_applyButton);
    applyAreaLayout->addWidget(_apply_LightButton);
    applyAreaLayout->addWidget(_apply_Light_VoiceButton);
    applyAreaLayout->addWidget(_noReturnTips);
    applyAreaLayout->addStretch();

    _addComponent_BeginButton = new ElaToolButton(this);
    _addComponent_BeginButton->setIsTransparent(false);
    _addComponent_BeginButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    _addComponent_BeginButton->setBorderRadius(8);
    //_toolButton->setPopupMode(QToolButton::MenuButtonPopup);
    _addComponent_BeginButton->setText("添加元器件");
    _addComponent_BeginButton->setElaIcon(ElaIconType::Plus);
    _addComponent_BeginButton->setIconSize(QSize(35, 35));
    _addComponent_BeginButton->setFixedSize(100, 75);

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
    editAreaLayout->addWidget(_addComponent_BeginButton);
    // editAreaLayout->addWidget(_delComponentButton);
    editAreaLayout->addStretch();

    _tabWidget->addTab(homeArea, "首页");
    _tabWidget->addTab(searchArea, "搜索");
    _tabWidget->addTab(applyArea, "元件操作");
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
    InitApplyReturnUI();
}

void MainWindow::InitAddComponentDockUI() {
    //添加元件信息栏初始化
    //展示元器件信息的UI不在这里
    _addComponent_DockhArea = new ElaScrollPageArea(this);
    _addComponent_DockhArea->setMinimumHeight(0);
    _addComponent_DockhArea->setMaximumHeight(QWIDGETSIZE_MAX);

    _addComponent_EditBox = new ElaLineEdit(this);
    _addComponent_EditBox->setFixedSize(500, 45);
    _addComponent_EditBox->setFixedHeight(45);
    _addComponent_EditBox->setPlaceholderText("请输入元件的CID，可以直接输入数字部分");

    _addComponent_DockhArea->hide();

    _addComponent_ProgressBar = new ElaProgressBar(this);
    _addComponent_ProgressBar->setValue(20);

    _addComponent_EditBoxText = new ElaText("1. 输入器件CID查找", this);
    _addComponent_EditBoxText->setTextPixelSize(20);
    _addComponent_CheckInfoText = new ElaText("2. 请检查元器件信息是否正确", this);
    _addComponent_CheckInfoText->setTextPixelSize(20);
    _addComponent_DownloadText = new ElaText("3. 下载相关资源", this);
    _addComponent_DownloadText->setTextPixelSize(20);
    _addComponent_OpenText = new ElaText("4. 分配存放点", this);
    _addComponent_OpenText->setTextPixelSize(20);
    _addComponent_WaitText = new ElaText("请在60s内打开闪蓝灯的格子", this);
    _addComponent_WaitText->setTextPixelSize(20);

    //信息渲染
    _addComponent_CheckInfoWidget = new QWidget(this);
    QVBoxLayout *_addComponent_CheckInfoLayout = new QVBoxLayout(_addComponent_CheckInfoWidget);
    _addComponent_CheckInfoLayout->setContentsMargins(0, 0, 0, 0);
    _addComponent_CheckInfoWidget_Text = new ElaText(" ", this);
    _addComponent_CheckInfoWidget_Text->setTextPixelSize(15);

    _addComponent_CheckInfoWidget_Card1 = new ElaPromotionCard(this);
    _addComponent_CheckInfoWidget_Card2 = new ElaPromotionCard(this);
    _addComponent_CheckInfoWidget_Card3 = new ElaPromotionCard(this);

    // _addComponent_CheckInfoWidget_Card1->setFixedWidth(420);
    // _addComponent_CheckInfoWidget_Card1->setFixedHeight(420);
    // _addComponent_CheckInfoWidget_Card2->setFixedWidth(420);
    // _addComponent_CheckInfoWidget_Card2->setFixedHeight(420);
    // _addComponent_CheckInfoWidget_Card3->setFixedWidth(420);
    // _addComponent_CheckInfoWidget_Card3->setFixedHeight(420);
    _addComponent_PNGView = new ElaPromotionView(this);
    _addComponent_PNGView->appendPromotionCard(_addComponent_CheckInfoWidget_Card1);
    _addComponent_PNGView->appendPromotionCard(_addComponent_CheckInfoWidget_Card2);
    _addComponent_PNGView->appendPromotionCard(_addComponent_CheckInfoWidget_Card3);
    // _promotionView->setCardCollapseWidth(420);
    _addComponent_PNGView->setCardExpandWidth(420);
    _addComponent_PNGView->setFixedHeight(420);
    _addComponent_CheckInfoWidget_Card1->resize(_addComponent_CheckInfoWidget_Card1->width(), 393);
    _addComponent_CheckInfoWidget_Card2->resize(_addComponent_CheckInfoWidget_Card2->width(), 393);
    _addComponent_CheckInfoWidget_Card3->resize(_addComponent_CheckInfoWidget_Card3->width(), 393);

    _addComponent_PNGView->setIsAutoScroll(true);

    _addComponent_CheckInfoLayout->addWidget(_addComponent_CheckInfoWidget_Text);
    _addComponent_CheckInfoLayout->addWidget(_addComponent_PNGView);
    // _addComponent_CheckInfoLayout->addSpacing(100);

    _addComponentButtonNext = new ElaToolButton(this);
    _addComponentButtonNext->setIsTransparent(false);
    _addComponentButtonNext->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    _addComponentButtonNext->setBorderRadius(8);
    //_toolButton->setPopupMode(QToolButton::MenuButtonPopup);
    _addComponentButtonNext->setText("下一步");
    _addComponentButtonNext->setElaIcon(ElaIconType::ArrowRight);
    _addComponentButtonNext->setFixedHeight(50);
    // _addComponentButtonNext->setIconSize(QSize(35, 35));
    // _addComponentButtonNext->setFixedSize(100, 50);
    _addComponent_CancelButton = new ElaToolButton(this);
    _addComponent_CancelButton->setIsTransparent(false);
    _addComponent_CancelButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    _addComponent_CancelButton->setBorderRadius(8);
    //_toolButton->setPopupMode(QToolButton::MenuButtonPopup);
    _addComponent_CancelButton->setText("退出向导");
    _addComponent_CancelButton->setElaIcon(ElaIconType::PersonToDoor);
    // _addComponentButtonCancel->setIconSize(QSize(35, 35));
    // _addComponentButtonCancel->setFixedSize(100, 50);
    _addComponent_CancelButton->setFixedHeight(50);

    QWidget *addComponentButtonArea = new QWidget(this);
    QHBoxLayout *addComponentButtonLayout = new QHBoxLayout(addComponentButtonArea);
    addComponentButtonLayout->setContentsMargins(0, 0, 0, 0);
    addComponentButtonLayout->addWidget(_addComponentButtonNext);
    addComponentButtonLayout->addWidget(_addComponent_CancelButton);

    _addComponent_busyRing = new FluBusyProgressRing;
    _addComponent_DownloadProgressRing = new FluProgressRing;
    _addComponent_DownloadProgressRing->setWorking(true);

    QVBoxLayout *addComponentLayout = new QVBoxLayout(_addComponent_DockhArea);
    addComponentLayout->addWidget(_addComponent_EditBoxText);
    addComponentLayout->addWidget(_addComponent_EditBox);

    addComponentLayout->addWidget(_addComponent_CheckInfoText);
    addComponentLayout->addWidget(_addComponent_busyRing);
    addComponentLayout->addWidget(_addComponent_CheckInfoWidget);

    addComponentLayout->addWidget(_addComponent_DownloadText);
    addComponentLayout->addWidget(_addComponent_DownloadProgressRing);

    addComponentLayout->addWidget(_addComponent_OpenText);
    addComponentLayout->addWidget(_addComponent_WaitText);

    addComponentLayout->addStretch();
    addComponentLayout->addWidget(addComponentButtonArea);
    addComponentLayout->addWidget(_addComponent_ProgressBar);

    //默认在元件信息模式
    _infoDockWidget->setWindowTitle("元件信息");
    _infoDockWidget->setWidget(_showInfo_scrollArea);

    _addComponent_timer = new QTimer(this);
}
