#ifndef STATIONLISTDIALOG_H
#define STATIONLISTDIALOG_H

#include <QDialog>

namespace Ui {
class StationListDialog;
}

class StationListDialog : public QDialog
{
    Q_OBJECT

public:
    explicit StationListDialog(QWidget *parent = nullptr);
    ~StationListDialog();

private slots:
    void loadRMSList();
    void on_closeButton_clicked();

private:
    Ui::StationListDialog *ui;
};

#endif // STATIONLISTDIALOG_H
