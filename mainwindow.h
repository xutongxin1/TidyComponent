#ifndef MAIN__MAINWINDOW_H_
#define MAIN__MAINWINDOW_H_

#include <qlabel.h>
#include <QMainWindow>

#include "ConfigClass.h"
#include <string>
#include <QTcpSocket>
#include "structH.h"

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
        double calculateSimilarity(const QString &a, const QString &b);
        static bool isExactMatch(const component_record &record, const QStringList &searchWords);
        QVector<component_record> findClosestRecords(
            const QVector<component_record> &recordsVector,
            const QString &searchString);

        void search();
        void importExcelToJson();
        void exportJsonToExcel();

        ~MainWindow() override;



        QVector<component_record> recordsVector;

    private:
        Ui::MainWindow *ui_;

        std::vector<ConfigClass *> config_device_ini_;
        int device_count_;
        ConfigClass *config_main_ini_;

        void GetConstructConfig();
        void SaveConstructConfig();
        QLabel *createHyperlinkLabel(const QString &text, const QString &url);
        void loadData();
        void addButtonToTable(int row, int col, const QString &color);

        void InitConfig();

        QString version_;
};

#endif // MAIN__MAINWINDOW_H_
