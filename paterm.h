// ----------------------------------------------------------------------------
// Copyright (C) 2019 GrizzWorks, LLC
// ALL RIGHTS RESERVED
//
// This file is part of radcommlink software project under the GNU GPLv3
// license.
//
// radcommlink is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//
// Parts of radcommlink may contain source code from Open Source projects.
// See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
#ifndef PATERM_H
#define PATERM_H

#include <QMainWindow>
#include <QProcess>
#include <QSerialPort>
#include <QFileSystemModel>
#include <QSettings>

QT_BEGIN_NAMESPACE
namespace Ui { class PATerm; }
QT_END_NAMESPACE

class PATerm : public QMainWindow
{
    Q_OBJECT

public:
    PATerm(QWidget *parent = nullptr);
    ~PATerm();
    void closeEvent(QCloseEvent *event);
private slots:
    void on_actionE_xit_triggered();
    void on_lineEdit_returnPressed();
    void on_readyRead();
    bool eventFilter(QObject *obj, QEvent *event);
    void slot_showMsgQueueContextMenu(const QPoint &pos);
    void on_msgDeleteButton_clicked();
    void on_msgQueueTreeView_clicked(const QModelIndex &index);
    QString loadStringFromFile(const QString filename);
    void on_action_Compose_triggered();
    void on_connectButton_clicked();
    void on_clearButton_clicked();
    void on_action_Station_List_triggered();
    void on_action_Update_Station_List_triggered();
    void expandAll();
    void on_actionGetting_Started_triggered();
    void on_action_About_PATerm_triggered();

    void on_abortButton_clicked();

private:
    Ui::PATerm *ui;
    QFileSystemModel *queue = nullptr;
    QSettings *settings = nullptr;
    QProcess *shell = nullptr;
    QSerialPort *ptc = nullptr;
    QByteArray CRLF = QByteArrayLiteral("\r\n");
    QString lastCmd;
    bool b_connectMode = false;
    void saveSettings();
};
#endif // PATERM_H
