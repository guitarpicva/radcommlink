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
#include "composedialog.h"
#include "ui_composedialog.h"

#include <QCryptographicHash>
#include <QDateTime>
#include <QSettings>
#include <QRegExp>
#include <QDir>
#include <QSaveFile>

#include <QDebug>

ComposeDialog::ComposeDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ComposeDialog)
{
    ui->setupUi(this);
    this->setAttribute(Qt::WA_DeleteOnClose);
    loadSettings();
    ui->toComboBox->setFocus();
    //qDebug()<<"QCH length:"<<QCryptographicHash::hashLength(QCryptographicHash::Md4);
}

ComposeDialog::~ComposeDialog()
{
    delete ui;
}

void ComposeDialog::closeEvent(QCloseEvent *)
{
    saveSettings();
}

void ComposeDialog::on_discardButton_clicked()
{
    QString tmp = ui->fromComboBox->currentText().trimmed().toUpper();
    ui->fromComboBox->addItem(tmp, tmp);
    tmp = ui->toComboBox->currentText().trimmed().toUpper();
    ui->toComboBox->addItem(tmp, tmp);
    saveSettings();
    close();
}

void ComposeDialog::on_postButton_clicked()
{
    // compile the message text and post to the user's outbox
    QString tmp;
    QString out = "Mid: ";
    QString from = ui->fromComboBox->currentText().trimmed().toUpper();
    if(ui->fromComboBox->findText(from, Qt::MatchExactly) == 0)
        ui->fromComboBox->addItem(from, from);
    tmp = from;
    tmp.append("-").append(QDateTime::currentDateTime().time().toString("hh:mm:ss"));
    QString mid = tmp;
    mid = QCryptographicHash::hash(mid.toLatin1(), QCryptographicHash::Md5).toBase64(QByteArray::Base64Encoding | QByteArray::OmitTrailingEquals);
    mid.truncate(12);
    mid = mid.toUpper();
    // now walk the mid and replace punctuation with A-Z0-9
    int occurrence = QDateTime::currentDateTime().date().dayOfWeek();
    for (int i = 0; i < 12; i++)
    {
        if(mid.at(i).isLetterOrNumber())
        {
            continue;
        }
        else
        {
            mid.replace(i, 1, QString::number(occurrence));
            occurrence = (occurrence + 3) % 9;
        }
    }
    const QString CRLF = "\r\n";
    out.append(mid).append(CRLF);
    const QString body = ui->plainTextEdit->toPlainText().trimmed();
    out.append("Body: ").append(QString::number(body.length() + 2)).append(CRLF);
    out.append("Content-Transfer-Encoding: 8bit\r\n");
    out.append("Content-Type: text/plain; charset=ISO-8859-1\r\n");
    // Date:
    out.append("Date: ").append(QDateTime::currentDateTimeUtc().toString("yyyy/MM/dd hh:mm")).append(CRLF);
    // From:
    out.append("From: ").append(from).append(CRLF);
    // To:
    tmp = ui->toComboBox->currentText().trimmed().toUpper();
    if(ui->toComboBox->findText(tmp,Qt::MatchExactly) == 0)
        ui->toComboBox->addItem(tmp, tmp);
    // Source:
    out.append("Mbo: ").append(ui->fromComboBox->currentText().trimmed().toUpper()).append(CRLF);
    // RMS Routing:
    out.append("RMS Routing: Radio-only").append(CRLF);
    // Subject:
    out.append("Subject: //WL2K ").append(ui->lineEdit->text().trimmed()).append(" /auto/").append(CRLF);
    // To:
    out.append("To: ").append(tmp).append(CRLF);
    // Type:
    out.append("Type: Private").append(CRLF);
    if(ui->p2pCheckBox->isChecked())
        out.append("X-P2ponly: true\r\n");
    // body text of message
    out.append(CRLF).append(body).append(CRLF);
    // Now write to a file in the mailbox for this user
    QSaveFile post(QDir::homePath() + "/.wl2k/mailbox/" + from + "/out/" + mid + ".b2f");
    if(post.open(QSaveFile::WriteOnly))
    {
        post.write(out.toLatin1());
        if(post.commit())
        {
            ui->plainTextEdit->appendHtml("Message Written to Outbox..." + mid + CRLF);
        }
        else
        {
            ui->plainTextEdit->appendHtml("ERROR: Unable to write Message to Outbox..." + mid + CRLF);
        }
        ui->plainTextEdit->moveCursor(QTextCursor::End);
    }
    saveSettings();
    close();
}

void ComposeDialog::saveSettings()
{
    QSettings settings("PATerm.ini", QSettings::IniFormat);
    QStringList items;
    for(int i = 0; i < ui->toComboBox->count(); i++)
    {
        if(items.contains(ui->toComboBox->itemText(i)))
            continue;
        items<<ui->toComboBox->itemText(i);
    }
    settings.setValue("addressees", items);
    items.clear();
    for(int i = 0; i < ui->fromComboBox->count(); i++)
    {
        //qDebug()<<"item:"<<ui->fromComboBox->itemText(i);
        if(items.contains(ui->fromComboBox->itemText(i)))
            continue;
        items<<ui->fromComboBox->itemText(i);
    }
    settings.setValue("myaddresses", items);
}

void ComposeDialog::loadSettings()
{
    QSettings s("PATerm.ini", QSettings::IniFormat);
    ui->fromComboBox->clear();
    ui->fromComboBox->addItems(s.value("myaddresses", "").toStringList());
    if(ui->fromComboBox->count() > 0)
        ui->fromComboBox->setCurrentIndex(1);
    ui->toComboBox->clear();
    ui->toComboBox->addItems(s.value("addressees", "").toStringList());

}

void ComposeDialog::on_ics213TemplateButton_clicked()
{
    ui->plainTextEdit->setPlainText(ICS213TEMPLATE);
}
