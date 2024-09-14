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
#include <QtConcurrent/qtconcurrentrun.h>
#include "FluDropDownButton.h"

#include "./ui_mainwindow.h"
#include "xlsxdatavalidation.h"
#include "thirdLib/QXlsx/QXlsx/header/xlsxdocument.h"

using namespace std;

QT_BEGIN_NAMESPACE

namespace Ui {
class MainWindow;
}

QT_END_NAMESPACE

class MainWindow : public QMainWindow {
        Q_OBJECT

    public:
        struct component_record {
            QString name;
            QString color;
            QString jlcid;
            QString tbLink;
            QString value;
            QString package;
            QVector<QString> aliases;
        };
        explicit MainWindow(QWidget *parent = nullptr);
        static double calculateSimilarity(const QString &a, const QString &b);
        static bool isExactMatch(const component_record &record, const QStringList &searchWords);
        void findClosestRecords(
            const QVector<component_record> &recordsVector,
            const QString &searchString);

        void search();
        void importExcelToJson();
        void exportJsonToExcel();

        ~MainWindow() override;

        QVector<component_record> recordsVector;

        QVector<int> exacIndex;// 精确结果
        QVector<int> fuzzyIndex;// 模糊结果

    private:
        Ui::MainWindow *ui_;

        std::vector<ConfigClass *> config_device_ini_;
        int device_count_;
        ConfigClass *config_main_ini_;

        void GetConstructConfig();
        void SaveConstructConfig();
        static QLabel *createHyperlinkLabel(const QString &text, const QString &url);
        void ShowAllComponents();
        void ShowSomeComponents();
        void loadData();
        void addButtonToTable(int row,int col, const QString &color);

        void InitConfig();

        QString version_;
};

#endif // MAIN__MAINWINDOW_H_
