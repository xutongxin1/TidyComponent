#ifndef MAIN__MAINWINDOW_H_
#define MAIN__MAINWINDOW_H_

#include "ConfigClass.h"
#include <QString>
#include <QtConcurrent/qtconcurrentrun.h>

// #include "./ui_mainwindow.h"

#include "ComponentTableModel.h"
#include "EDAChromeHttpServer.h"
#include "ElaContentDialog.h"
#include "ElaDockWidget.h"
#include "ElaLineEdit.h"
#include "ElaListView.h"
#include "ElaMenuBar.h"
#include "ElaPivot.h"
#include "ElaProgressBar.h"
#include "ElaProgressRing.h"
#include "ElaPromotionCard.h"
#include "ElaPromotionView.h"
#include "ElaScrollPage.h"
#include "ElaScrollPageArea.h"
#include "ElaTableView.h"
#include "ElaTabWidget.h"
#include "ElaToolButton.h"
#include "ElaWindow.h"
#include "ResizedTableView.h"
#include "ShowInfoModel.h"
#include "ElaText.h"
#include "QColorAllocator.h"
#include "SerialPortManager.h"

#define CONFIGPATH QCoreApplication::applicationDirPath()+QString("/config/")
#define DBPATH QCoreApplication::applicationDirPath()+QString("/config/db/")
#define INFOPATH  QCoreApplication::applicationDirPath()+QString("/config/info")
#define ChromePATH QCoreApplication::applicationDirPath()+QString("/config/Chrome/")

#define TEMP_PATH QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation)
#define DATASHEET_PATH QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation)+"/info"
using namespace std;

enum apply_type {
    apply_type_normal = 1,
    apply_type_light = 2,
    apply_type_voice = 3,
};

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

QT_BEGIN_NAMESPACE

namespace Ui {
class MainWindow;
}

QT_END_NAMESPACE
class ShowInfoModel;
class MainWindow : public ElaWindow {
        Q_OBJECT

    public:
        SerialPortManager *serialManager;
        ComponentTableModel *model;
        ElaToolButton *_resetSearchButton;

        ElaToolButton *_delComponentButton;
        QWidget *_showInfo_Widget;
        ElaDockWidget *_infoDockWidget;

        ElaScrollPageArea *_addComponent_DockhArea;
        ElaLineEdit *_addComponent_EditBox;
        ElaProgressBar *_addComponent_ProgressBar;
        ElaProgressBar *_addComponentLoadingProgressBar;
        ElaProgressRing *_addComponent_DownloadProgressRing;
        ElaText *_addComponent_WaitText;
        ElaToolButton *_addComponentButtonNext;
        ElaToolButton *_addComponent_CancelButton;
        ElaProgressRing *_addComponent_busyRing;
        ElaText *_addComponent_EditBoxText;
        ElaText *_addComponent_CheckInfoText;
        ElaText *_addComponent_DownloadText;
        ElaText *_addComponent_OpenText;
        QString _addingComponent_CID;
        ElaText *_addComponent_CheckInfoWidget_Text;
        QWidget *_addComponent_CheckInfoWidget;
        ElaPromotionCard *_addComponent_CheckInfoWidget_Card1;
        ElaPromotionCard *_addComponent_CheckInfoWidget_Card2;
        ElaPromotionCard *_addComponent_CheckInfoWidget_Card3;
        ElaPromotionView *_addComponent_PNGView;
        QTimer *_addComponent_timer;
        int _addComponent_timeLeft;
        int _addComponentStep;
        bool _addComponent_isPutInComponent;

        ElaPromotionView *_showInfo_PNGView;
        ElaPromotionView *_showInfo_SCHPCBview;
        ElaPromotionCard *_showInfo_PNGCard1;
        ElaPromotionCard *_showInfo_PNGCard2;
        ElaPromotionCard *_showInfo_PNGCard3;
        ElaPromotionCard *_showInfo_PNGCard4;
        ElaPromotionCard *_showInfo_PNGCard5;
        ElaPromotionCard *_showInfo_SCHCard1;
        ElaPromotionCard *_showInfo_SCHCard2;
        ElaPromotionCard *_showInfo_SCHCard3;
        ElaPromotionCard *_showInfo_SCHCard4;
        ElaPromotionCard *_showInfo_SCHCard5;
        ElaPromotionCard *_showInfo_PCBCard1;
        ElaPromotionCard *_showInfo_PCBCard2;
        ElaPromotionCard *_showInfo_PCBCard3;
        ElaPromotionCard *_showInfo_PCBCard4;
        ElaPromotionCard *_showInfo_PCBCard5;
        ElaTableView *_showInfo_tableView;
        ShowInfoModel *_showInfo_model;
        ElaText *_showInfo_NoComponentTips;
        ElaToolButton *_showInfo_OpenPDFButton;
        ElaScrollArea *_showInfo_scrollArea;
        QWidget *_showInfo_Web_Widget;
        ElaToolButton *_showInfo_OpenWebSiteButton;
        ElaToolButton *_showInfo_updateInfoButton;
        QAction *connectUserStateAction;
        ElaToolButton *_searchTypeButton;
        ElaToolButton *_return_ALLButton;
        ElaText *_returnTipsB53;
        ElaToolButton *_applyButton;
        ElaToolButton *_apply_LightButton;
        ElaToolButton *_apply_Light_VoiceButton;
        ElaText *_noReturnTips;
        bool _serial_isOK;
        bool _serial_isMESH_OK;
        bool _serial_is_inWriteMOde;
        EDAChromeHttpServer *EDA_Server;
        ElaToolButton *_openEDAChromeButton;
        ElaToolButton *_addComponent_B53_Button;
        ElaToolButton * _addComponent_A42_Button;
        QPair<QString, int> _addComponent_Allocate;
        bool isConnectedToMesh = false;
        QColorAllocator *colorAllocator;
        // ElaText *_addComponent_NFCText;
        bool _addComponent_isNFC_Write_success;


        void initElaWindow();
        explicit MainWindow(QWidget *parent = nullptr);
        void fuzzy_search_records(
            const QString &searchString) const;
        void UpdateApplyLogic();
        void UpdateApplyLogic(component_record_struct *record);
        void exact_search_records(
            const QString &searchString) const;
        void search() const;
        void searchBoxClear();
        void searchLogicInit();
        QVector<component_record_struct *> search_records_with_strategy(const QString &searchString) const;
        void bomSearch();
        void InitEDAChromeHttpServer();

        ~MainWindow() override;

        // 设备配置结构体
        struct DeviceInfo {
            QString MAC;
            QList<int> coordinates;
            DeviceType type;
            int B53_N = 0;
            int coordinates_total = 0;
        };

        // 设备配置管理结构体
        struct DeviceConfig {
            QVector<DeviceInfo> devices;
            QHash<QString, DeviceInfo *> deviceMap; // 快速查找
            QHash<DeviceType, int> typeCoordinateTotal; // 统计每种类型的坐标数量
        };
        DeviceConfig _device_config;

    private:
        int ApplyComponentNum = 0;

        // Ui::MainWindow *ui_;
        ElaContentDialog *_closeDialog{nullptr};
        ResizedTableView *tableView;
        ElaPivot *_pivot{nullptr};
        ElaTabWidget *_tabWidget{nullptr};
        ElaListView *_iconView;
        ElaPromotionCard *_promotionCard{nullptr};

        ElaToolButton *_enterEditButton{nullptr};
        ElaLineEdit *_searchBox{nullptr};
        ElaToolButton *_bomSearchButton;
        std::vector<ConfigClass *> config_device_ini_;
        int device_count_;
        ConfigClass *config_main_ini_;
        DeviceType _addComponent_Type = DeviceType_B53;

        void GetConstructConfig();
        void SaveConstructConfig();
        void initSerialPort();
        void CX02_SerialRecive(const QString &message, DeviceType device_type);
        void CX03_SerialRecive(const QString &message, DeviceType device_type);
        void CX04_SerialRecive(const QString &message, DeviceType device_type);
        void CX01Error_SerialRecive(const QString &message, DeviceType device_type);
        void updateComponentShowInfo_Clear();
        void updateComponentShowInfo(const QItemSelection &selected, const QItemSelection &deselected);
        static void AddCardToShow(ElaPromotionView *view, ElaPromotionCard *card, const QString &fileURL,
                                  bool isSVG = false);
        void updateComponentColor(component_record_struct *record, QColor color);
        void onShowInfoTableViewDoubleClicked(const QModelIndex &index);

        void loadDataFromFolder();
        void SaveDataToFolder();
        void SaveSingleComponent(component_record_struct record);
        void SaveSingleComponent(const QString &jlcid);
        void updateDeviceConfig(const QString &MAC, const int &coordinate, const DeviceType &type);
        bool addDevice(const QString &MAC, const DeviceType &type);
        bool saveDeviceConfig();
        void loadDeviceConfig();
        DeviceInfo *getDeviceByMAC(const QString &MAC) const;
        void deleteSingleComponent(const QString &jlcid) const;

        // 缓存每种类型的所有可能坐标
        QHash<DeviceType, QList<int> > _typeCoordinatesCache;

        // 辅助函数
        void initializeCoordinatesCache();
        QList<int> generateAllCoordinatesForType(const DeviceType &type);

        // 按类型统计坐标
        int getUsedCoordinatesCountForType(const DeviceType &type);
        int getDeviceCountForType(const DeviceType &type);
        QList<int> getAvailableCoordinatesForType(const DeviceType &type);

        // 坐标分配和释放
        QPair<QString, int> allocateCoordinateForType(const DeviceType &type);
        QPair<QString, int> allocateNextAvailableCoordinateForType(const DeviceType &type);
        bool releaseCoordinate(const DeviceType &type, const int &coordinate);
        bool releaseCoordinateByMAC(const QString &MAC, const int &coordinate);

        // 获取类型的所有设备及其坐标使用情况
        QVector<QPair<QString, int> > getDeviceUsageForType(const DeviceType &type);

        int getTotalCoordinatesCountForType(DeviceType type);
        void updateTypeCoordinateTotal();
        // 统计和报告
        void updateTypeStatistics();
        QHash<DeviceType, QPair<int, int> > getAllTypeStatistics(); // type -> (used, total)
        QVector<QPair<QString, int> > allocateMultipleCoordinatesForType(const DeviceType &type, int count);
        QString findDeviceByCoordinate(const DeviceType &type, const int &coordinate);

        bool isExistingComponent(const QString &CID) const;
        void reactComponentHash() const;
        void addComponentToLib(const component_record_struct &_addingComponentObj, bool isReacting = false) const;
        void replaceComponentToLib(component_record_struct _replacingComponentObj);
        static void updateSearchKey(component_record_struct &record);
        void updateOneComponent(const QString &CID);

        void InitConfig();
        void ShowSuccessInfo(const QString &info, const QString &title = QString());
        void ShowWarningInfo(const QString &info, const QString &title = QString());
        void ShowInfoInfo(const QString &info, const QString &title = QString());
        void ShowErrorInfo(const QString &info, const QString &title = QString());
        void UpdateApplyReturnUI();
        void InitApplyReturnUI();
        void ApplyComponentOUT(component_record_struct *record, apply_type apply_type, led_mode_t led_mode);
        void ApplyComponentIN(component_record_struct *record, apply_type apply_type, led_mode_t led_mode);
        void ApplyComponentIN_AddingCompnent(component_record_struct *record);
        void InitAddComponentDockUI();
        void SerialDataReceived(const QString &data) = delete;
        void getDailySection() const;
        void AnalyzeComponentData(const QString &CID, const QJsonObject &json, component_record_struct &component);
        void AnalyzeAddingComponentData(const QString &CID, const QJsonObject &json,
                                        component_record_struct &component);
        void AddComponentLogic_0(const DeviceType &type);
        void AddComponentLogic_1();
        void AddComponentLogic_2();
        void AddComponentLogic_3();
        void AddComponentLogic_4();
        void AddComponentLogic_5();
        void cancelAddComponentLogic();
        void initAddComponentLogic();
        QString version_;
        component_record_struct *_addingComponentObj;
        bool isDownloadPDF = false;
};

#endif // MAIN__MAINWINDOW_H_
