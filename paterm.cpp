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
#include "paterm.h"
#include "ui_paterm.h"
#include "composedialog.h"
#include "ui_composedialog.h"
#include "stationlistdialog.h"
#include "ui_stationlistdialog.h"

#include <QFile>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMenu>
#include <QThread>
#include <QTimer>

#include <QDebug>

PATerm::PATerm(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::PATerm)
{
    ui->setupUi(this);
    settings = new QSettings(qApp->applicationDirPath() + "/PATerm.ini", QSettings::IniFormat, this);
    ui->stationComboBox->addItems(settings->value("stations", "").toStringList());
    ui->dialFreqComboBox->addItems(settings->value("channels", "").toStringList());
    shell = new QProcess(this);
    shell->setProcessChannelMode(QProcess::MergedChannels);
    shell->setProgram("/bin/bash");
    shell->start();
    connect(shell, &QProcess::readyRead, this, &PATerm::on_readyRead);
    ui->lineEdit->installEventFilter(this);
    // set up the file system model for the Tree View
    queue = new QFileSystemModel(this);
    queue->setReadOnly(false);
    queue->setRootPath(QDir::homePath() + "/.wl2k/mailbox");
    queue->setFilter(QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot);
    queue->sort(3);
    ui->msgQueueTreeView->setModel(queue);
    //qDebug()<<"root path"<<queue->rootPath();
    ui->msgQueueTreeView->setRootIndex(queue->index(queue->rootPath()));
    ui->msgQueueTreeView->hideColumn(1);
    ui->msgQueueTreeView->hideColumn(2);
    ui->msgQueueTreeView->hideColumn(3);
    ui->msgQueueTreeView->sortByColumn(3, Qt::DescendingOrder);
    ui->msgQueueTreeView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->msgQueueTreeView, &QTreeView::customContextMenuRequested, this, &PATerm::slot_showMsgQueueContextMenu);
    ui->msgQueueTreeView->installEventFilter(this);
    this->restoreGeometry(settings->value("geometry").toByteArray());
    this->restoreState(settings->value("windowState").toByteArray());
    ui->splitter->restoreState(settings->value("splitterState").toByteArray());
    ui->radioOnlyCheckBox->setChecked(settings->value("radioOnly", true).toBool());
    QTimer::singleShot(200, this, &PATerm::expandAll);
}

PATerm::~PATerm()
{
    delete ui;
}

void PATerm::expandAll()
{
    ui->msgQueueTreeView->expandAll();
}

bool PATerm::eventFilter(QObject* obj, QEvent *event)
{
    if (obj == ui->lineEdit)
    {
        if (event->type() == QEvent::KeyPress)
        {
            QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
            if (keyEvent->key() == Qt::Key_Up)
            {
                //qDebug() << "lineEdit -> Qt::Key_Up";
                ui->lineEdit->setText(lastCmd);
                return true;
            }
        }
        return false;
    }
    else if (obj == ui->msgQueueTreeView)
        {
            QModelIndex i = ui->msgQueueTreeView->currentIndex();
            //qDebug() << "trapped key press on folderContentsListWidget";
            if (event->type() == QEvent::KeyPress)
            {
                QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
                if (keyEvent->key() == Qt::Key_Delete)
                {
                    on_msgDeleteButton_clicked();
                    return true;
                }
                else if (keyEvent->key() == Qt::Key_Up)
                {
                    QModelIndex idx = i.sibling(i.row()-1, i.column());
                    if(idx.isValid())
                    {
                        ui->msgQueueTreeView->setCurrentIndex(idx);
                        on_msgQueueTreeView_clicked(idx);
                    }
                    return true;
                }
                else if (keyEvent->key() == Qt::Key_Down)
                {
                    QModelIndex idx = i.sibling(i.row()+1, i.column());
                    if(idx.isValid())
                    {
                        ui->msgQueueTreeView->setCurrentIndex(idx);
                        on_msgQueueTreeView_clicked(idx);
                    }
                    return true;
                }
            }
            return false;
        }
    return false;
}

void PATerm::slot_showMsgQueueContextMenu(const QPoint& pos)
{
    // abort if nothing selected
    if(ui->msgQueueTreeView->selectionModel()->selectedIndexes().count() == 0)
        return;
    // build and show a context menu for the message queue tree view
    QPoint globalPos = ui->msgQueueTreeView->mapToGlobal(pos);
    // for QAbstractScrollArea and derived classes you would use:
    // QPoint globalPos = myWidget->viewport()->mapToGlobal(pos);
    QModelIndex it = ui->msgQueueTreeView->indexAt(pos);
    // now see if it's a folder or a file and offer an appropriate
    // context menu
    if(queue->isDir(it))
    {
            return;
                    //QMessageBox::information(this, "System Folder", "Cannot change PAT system folders");
    }
    else
    {
        QMenu treeFileMenu;
        treeFileMenu.addAction("Delete Message <Del>");
        treeFileMenu.addAction("Archive Message");
        QAction* selectedItem = treeFileMenu.exec(globalPos);
        if (selectedItem)
        {
            const QString item = selectedItem->text();
            if(item == "Delete Messaeg <Del>")
                on_msgDeleteButton_clicked();
            else if(item == "Archive Message")
                on_msgArchiveButton_clicked();
        }
    }
}

void PATerm::closeEvent(QCloseEvent *event)
{
    saveSettings();
    if(shell)
        shell->close();
    event->accept();
}

void PATerm::on_actionE_xit_triggered()
{
    close();
}

void PATerm::on_lineEdit_returnPressed()
{
    // send the command to pat
    const QString cmd = ui->lineEdit->text().trimmed().toLower();
    lastCmd = cmd;
    if(!cmd.isEmpty())
        shell->write(cmd.toLatin1()); // + BG);
    shell->write(LF);
    ui->lineEdit->clear();
    if(cmd.contains("pat connect"))
    {
        b_connectMode = true;
    }
    else if(cmd.contains("pat compose"))
    {
        // open the compose dialog
        on_action_Compose_triggered();
    }
    else
    {
        b_connectMode = false;
    }
}

void PATerm::on_readyRead()
{
    //qDebug()<<"readyRead"<<shell->readAllStandardOutput();
    if(b_connectMode)
    {
        ui->receiveTextArea->appendPlainText(shell->readAllStandardOutput());
        ui->receiveTextArea->moveCursor(QTextCursor::End);
    }
    else
    {
        ui->patConsole->appendPlainText(shell->readAllStandardOutput());
        ui->patConsole->moveCursor(QTextCursor::End);
    }
}


void PATerm::on_msgDeleteButton_clicked()
{
    // delete the selected message if any
    QModelIndexList qmil = ui->msgQueueTreeView->selectionModel()->selectedIndexes();
    if(qmil.size() < 1)
        return; // nothing selected
//    QStringList list;
//    QModelIndex lastIdx;
//    foreach(const QModelIndex i, qmil)
//    {
//        // make a list of files to delete
//        if(i.isValid())
//        {
//            list<<queue->filePath(i);
//        }
//        lastIdx = i;
//    }
//    foreach(const QString fName, list)
//    {
    QModelIndex lastIdx = qmil.at(0);
    QString fName = queue->filePath(lastIdx);
        // delete each file by moving it to the TRASH folder
        if (QMessageBox::Yes == QMessageBox::question(this, "Delete Message?", "Really delete message \"" + fName.mid(fName.lastIndexOf("/") + 1) + "\"?"))
        {
            QFile::remove(fName);
        }
//    }
    // try to select the nearest index that is left
    if(lastIdx.isValid())
    {
        ui->msgQueueTreeView->selectionModel()->select(ui->msgQueueTreeView->indexBelow(lastIdx), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
        ui->msgQueueTreeView->clicked(ui->msgQueueTreeView->indexBelow(lastIdx));
    }
    else
    {
        ui->msgQueueTreeView->selectionModel()->select(queue->index(0, 0), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
        ui->msgQueueTreeView->clicked(queue->index(0, 0));
    }
}

void PATerm::on_msgQueueTreeView_clicked(const QModelIndex &index)
{
    //qDebug()<<"clicked";
    if(index.isValid())
    {
        const QString fName = queue->filePath(index);
        if(fName.contains(QRegExp("[A-Z0-9]{12}.b2f")))
        {
            ui->receiveTextArea->clear();
            ui->receiveTextArea->setPlainText(loadStringFromFile(fName));
            ui->receiveTextArea->moveCursor(QTextCursor::Start);
        }
    }
}

QString PATerm::loadStringFromFile(const QString filename) {
    // this should be used for text files only
    QString msgText;
    int counter = 0;
    if (filename.isEmpty())
        return "";
    QFile f(filename);
    while (!f.open(QIODevice::ReadOnly)) {
        //qDebug() << "Waiting for file open...";
        QThread::msleep(100);//sleep for a bit
        qApp->processEvents();
        counter++;
        if(counter>10) {
            ui->receiveTextArea->appendPlainText("radcommlink is in Unable to load the text from the file.");
            ui->receiveTextArea->moveCursor(QTextCursor::End);
            return msgText; // empty
        }
    }// end while file is not open

    // Load the file through a text stream to normalize
    // the line ends to ACP required CRLF and to trim
    // white space
    QTextStream in(&f);
    QString line;
    while (!in.atEnd()) {
        line = in.readLine(); // strips line ends
        msgText.append(line.toLatin1()).append("\r\n");
    }
    //msgText = loadedFile->readAll();
    f.close();
    return msgText;
}

void PATerm::on_action_Compose_triggered()
{
    ComposeDialog * msg = new ComposeDialog(this);
    msg->show();
}

void PATerm::on_connectButton_clicked()
{
    QByteArray out;
    const QString to = ui->stationComboBox->currentText().trimmed().toUpper();
    const QString transport = ui->transportComboBox->currentText();
    if(transport != "telnet" && to.isEmpty())
        return;
    if(transport == "telnet")
    {
        out.append(QString("pat connect telnet").toLatin1()); // simple cheater link
    }
    else
    {
        const QString middle = ui->radioOnlyCheckBox->isChecked()? "--radio-only " : " ";
        out.append(QString("pat " + middle + "connect " + transport + ":///"+ to).toLatin1()); // requires use of aliases
    }
    if(ui->dialFreqComboBox->currentText() != "")
    {
        const QString freq = ui->dialFreqComboBox->currentText().trimmed();
        out.append("?freq=").append(freq);
        if(ui->dialFreqComboBox->findText(freq) < 0)
            ui->dialFreqComboBox->addItem(freq, freq);
    }
    //qDebug()<<"connect:"<<out;
    shell->write(out);
    shell->write(LF); // for the shell, remember!
    shell->write("ps ax|grep pat|grep connect|grep pactor:");
    shell->write(LF);
    if(ui->stationComboBox->findText(to) < 0)
        ui->stationComboBox->addItem(to, to);
}

void PATerm::saveSettings()
{
    QStringList items;
    for(int i = 0; i < ui->stationComboBox->count(); i++)
    {
        items<<ui->stationComboBox->itemText(i);
    }
    settings->setValue("stations", items);
    items.clear();
    for(int i = 0; i < ui->dialFreqComboBox->count(); i++)
    {
        items<<ui->dialFreqComboBox->itemText(i);
    }
    settings->setValue("channels", items);
    settings->setValue("windowState", saveState());
    settings->setValue("geometry", saveGeometry());
    settings->setValue("splitterState", ui->splitter->saveState());
    settings->setValue("radioOnly", ui->radioOnlyCheckBox->isChecked());
}

void PATerm::on_clearButton_clicked()
{
    ui->receiveTextArea->clear();
}

void PATerm::on_action_Station_List_triggered()
{
    StationListDialog *sl = new StationListDialog(this);
    sl->show();
}

void PATerm::on_action_Update_Station_List_triggered()
{
    shell->write("pat rmslist -d\n");
}

void PATerm::on_actionGetting_Started_triggered()
{
    QMessageBox::information(this, "Getting Started with radcommlink", \
                             "Step One: Install and configure PAT, from https://getpat.io.\n" \
"If you installed PAT in a default manner, you are ready to use radcommlink.\n\n" \
"In other words, the .wl2k folder MUST be in the current user's home directory."
                             );
}

void PATerm::on_action_About_PATerm_triggered()
{
    QMessageBox::information(this, "About radcommlink", \
                             "Copyright 2019, GrizzWorks, LLC\n\n" \
"radcommlink is offered under the GPL v3 license included in the distribution\n\n" \
"or which can be found at https://www.gnu.org/licenses/gpl-3.0.txt."
                             );
}

void PATerm::on_abortButton_clicked()
{
    if(shell)
    {
        //QString s_pid = QString::number(shell->processId());
//        auto ret = system(QString("killall pat"));
//        qApp->processEvents();
//        //ret = system(QString("kill -2 " + s_pid).toLatin1().data());
        auto ret = system(KILLPAT);
    }
}

void PATerm::on_msgArchiveButton_clicked()
{
    // archive the selected message if any
    QModelIndexList qmil = ui->msgQueueTreeView->selectionModel()->selectedIndexes();
    if(qmil.size() < 1)
        return; // nothing selected
//    QStringList list;
    QModelIndex lastIdx;

    QString fName;
    lastIdx = qmil.at(0);
    fName = queue->filePath(lastIdx);
    //qDebug()<<"fName:"<<fName;
    QString base = fName.mid(0, fName.lastIndexOf("/"));
    base = base.mid(0, base.lastIndexOf("/"));
    QFile::rename(fName, base + "/archive/" + fName.mid(fName.lastIndexOf("/")));
//    }
    // try to select the nearest index that is left
    if(lastIdx.isValid())
    {
        ui->msgQueueTreeView->selectionModel()->select(ui->msgQueueTreeView->indexBelow(lastIdx), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
        ui->msgQueueTreeView->clicked(ui->msgQueueTreeView->indexBelow(lastIdx));
    }
    else
    {
        ui->msgQueueTreeView->selectionModel()->select(queue->index(0, 0), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
        ui->msgQueueTreeView->clicked(queue->index(0, 0));
    }
}
