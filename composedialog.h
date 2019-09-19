#ifndef COMPOSEDIALOG_H
#define COMPOSEDIALOG_H

#include <QDialog>

namespace Ui {
class ComposeDialog;
}

class ComposeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ComposeDialog(QWidget *parent = nullptr);
    ~ComposeDialog();

private slots:
    void on_discardButton_clicked();

    void on_postButton_clicked();

private:
    Ui::ComposeDialog *ui;
    void saveSettings();
    void loadSettings();
};

#endif // COMPOSEDIALOG_H
