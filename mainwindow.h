#ifndef MAIN__MAINWINDOW_H_
#define MAIN__MAINWINDOW_H_

#include <qlabel.h>
#include <QMainWindow>

#include "ConfigClass.h"
#include <string>
#include <QTcpSocket>
#include "structH.h"
#include <QDesktopServices>
#include <QMessageBox>
#include <QVector>
#include <QString>
#include <QStringList>
#include <QMap>
#include <algorithm>
#include <QImageReader>
#include <QtConcurrent/qtconcurrentrun.h>
#include "FluDropDownButton.h"

// #include "./ui_mainwindow.h"
#include <qstandarditemmodel.h>

#include "ComponentTableModel.h"
#include "ElaContentDialog.h"
#include "ElaDockWidget.h"
#include "ElaLineEdit.h"
#include "ElaListView.h"
#include "ElaPivot.h"
#include "ElaProgressBar.h"
#include "ElaPromotionCard.h"
#include "ElaPromotionView.h"
#include "ElaScrollPageArea.h"
#include "ElaSuggestBox.h"
#include "ElaTableView.h"
#include "ElaTabWidget.h"
#include "ElaText.h"
#include "ElaToolButton.h"
#include "ElaWindow.h"
#include "FluBusyProgressRing.h"
#include "FluProgressRing.h"
#include "ResizedTableView.h"
#include "ShowInfoModel .h"
#include "xlsxdatavalidation.h"
#include "thirdLib/QXlsx/QXlsx/header/xlsxdocument.h"

#define CONFIGPATH QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation)
#define INFOPATH QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation)+"/info"

using namespace std;

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

QT_BEGIN_NAMESPACE

namespace Ui {
class MainWindow;
}

QT_END_NAMESPACE

class MainWindow : public ElaWindow {
        Q_OBJECT

    public:
        ComponentTableModel *model;
        ElaToolButton *_resetSearchButton;
        ElaToolButton *_addComponentButton;
        ElaToolButton *_delComponentButton;
        ElaScrollPageArea *_infoDockhArea;
        ElaScrollPageArea *_addComponentDockhArea;
        ElaLineEdit *_addComponent_EditBox;
        ElaDockWidget *_infoDockWidget;
        ElaProgressBar *_addComponent_ProgressBar;
        ElaProgressBar *_addComponentLoadingProgressBar;
        FluProgressRing *_addComponent_DownloadProgressRing;
        ElaText *_addComponent_WaitText;
        ElaToolButton *_addComponentButtonNext;
        ElaToolButton *_addComponent_CancelButton;
        FluBusyProgressRing *_addComponent_busyRing;
        ElaText *_addComponent_EditBoxText;
        ElaText *_addComponent_CheckInfoText;
        ElaText *_addComponent_DownloadText;
        ElaText *_addComponent_OpenText;
        bool isAddingComponent;
        int _addComponentStep;
        QString _AddingComponent_CID;
        ElaText *_addComponent_CheckInfoWidget_Text;
        QWidget *_addComponent_CheckInfoWidget;
        ElaPromotionCard *_addComponent_CheckInfoWidget_Card1;
        ElaPromotionCard *_addComponent_CheckInfoWidget_Card2;
        ElaPromotionCard *_addComponent_CheckInfoWidget_Card3;
        ElaPromotionView *_promotionView;
        QTimer *_addComponent_timer;
        int _addComponent_timeLeft;
        QPushButton * m_pdfButton;
        ElaPromotionView * _showInfo_PNGview;
        ElaPromotionCard * _showInfo_PNGCard1;
        ElaPromotionCard * _showInfo_PNGCard2;
        ElaPromotionCard * _showInfo_PNGCard3;
        ElaTableView * _showInfo_tableView;
        ShowInfoModel * _showInfo_model;

        void initElaWindow();
        void _showInfo_addTableViewRow(const QString &field, const QString &value);
        void updateContent(const QModelIndex &index);
        QLabel *createClickableLabel();
        void copyTextToClipboard();
        explicit MainWindow(QWidget *parent = nullptr);
        static double calculateSimilarity(const QString &a, const QString &b);
        static bool isExactMatch(const component_record_struct &record, const QStringList &searchWords);
        void findClosestRecords(
            const QVector<component_record_struct> &recordsVector,
            const QString &searchString) const;

        void search() const;
        void importExcelToJson();
        void exportJsonToExcel();

        ~MainWindow() override;

        // QVector<component_record_struct> recordsVector;

        // QVector<int> exacIndex;// 精确结果
        // QVector<int> fuzzyIndex;// 模糊结果

    private:
        // Ui::MainWindow *ui_;
        ElaContentDialog *_closeDialog{nullptr};
        ResizedTableView *tableView;
        ElaPivot *_pivot{nullptr};
        ElaTabWidget *_tabWidget{nullptr};
        ElaListView *_iconView;
        ElaPromotionCard *_promotionCard{nullptr};

        ElaToolButton *_enterEditButton{nullptr};
        ElaLineEdit *_searchBox{nullptr};
        ElaToolButton *_importSearchButton;
        std::vector<ConfigClass *> config_device_ini_;
        int device_count_;
        ConfigClass *config_main_ini_;

        void GetConstructConfig();
        void SaveConstructConfig();
        static QLabel *createHyperlinkLabel(const QString &text, const QString &url);
        // void ShowAllComponents();
        // void ShowSomeComponents();
        void loadData() const;
        void SaveData() const;
        bool isExistingComponent(const QString &CID) const;
        void addComponentToLib(const component_record_struct &_addingComponentObj) const;
        static void updateSearchKey(component_record_struct &_addingComponentObj);

        void InitConfig();
        void ShowSuccessInfo(const QString &info, const QString &title = QString());
        void ShowWarningInfo(const QString &info, const QString &title = QString());
        void ShowInfoInfo(const QString &info, const QString &title = QString());
        void ShowErrorInfo(const QString &info, const QString &title = QString());
        void InitSearchDockLogic();
        void getDailySection() const;
        void AddComponentLogic_1();
        void AddComponentLogic_2();
        void AddComponentLogic_3();
        void AddComponentLogic_4();
        void cancelAddComponentLogic();
        void initAddComponentLogic();
        QString version_;
        component_record_struct *_addingComponentObj;
};

#endif // MAIN__MAINWINDOW_H_
