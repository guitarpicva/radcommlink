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
#include "stationlistdialog.h"
#include "ui_stationlistdialog.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTableWidgetItem>
#include <QFile>
#include <QMap>

#include <QDebug>

StationListDialog::StationListDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::StationListDialog)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    ui->tableWidget->setColumnWidth(1, 370);
    loadRMSList();
}

StationListDialog::~StationListDialog()
{
    delete ui;
}

void StationListDialog::loadRMSList()
{
    // TODO: make this configurable and robust
    /*
     * [{\"Timestamp\":\"\\/Date(1568493300000)\\/\",\"Callsign\":\"NNA0AK\",\"BaseCallsign\":\"NNA0AK\",\"Latitude\":64.886517803837,\"Longitude\":-147.73029018442,\"HoursSinceStatus\":3,\"LastStatus\":\"Sat, 14 Sep 2019 20:35:00 UTC\",\"Comments\":\"\",\"RequestedMode\":\"AnyAll\",\"GatewayChannels\":
     * */
    QString rawjson;
    QFile rmslist("/home/mitch/.wl2k/rmslist.json");
    if(rmslist.open(QFile::ReadOnly))
    {
        rawjson = rmslist.readAll();
        rmslist.close();
    }
    QStringList stations;
    QMap<QString,QString> freqs;
    QMap <QString, QString> freqMap;
    //qDebug()<<"rmslist:\n"<<rawjson;
    QJsonDocument jd = QJsonDocument::fromJson(rawjson.toLatin1());
    QJsonObject jo = jd.object();
    QJsonArray ja = jd["Gateways"].toArray();
    QVariantList vl = ja.toVariantList();
    //qDebug()<<vl.at(0)<<"\n";
    for (int i = 0; i < vl.count(); i++)
    {
        //qDebug()<<"[Station Record]";
        QVariantMap vm = vl.at(i).toMap();
        QString currStation;
        foreach(const QVariant s, vm.keys())
        {
            if(s.toString() == "BaseCallsign")
            {
                currStation = vm.value(s.toString()).toString();
                //output to  our map/file
                stations<<currStation;
            }
            else if(s.toString() == "GatewayChannels")
            {
                QStringList flist; // list of channels for one station
                //qDebug()<<"\n[Gateway Channels for " + currStation +"]";
                QVariantList gl = vm.value("GatewayChannels").toList();
                for (int j = 0; j < gl.count(); j++)
                {
                    //qDebug()<<"";
                    QVariantMap gm = gl.at(j).toMap();
                    foreach (const QVariant g, gm.keys())
                    {
                        const QString gVal = gm.value(g.toString()).toString();
                        //qDebug()<<g.toString()<<gVal;
                        if(g.toString() == "Frequency")
                        {
                            flist<<gVal;
                            freqs.insert(gVal, gVal);
                        }
                    }

                }
                freqMap.insert(currStation, flist.join(", "));
                //qDebug()<<"\n[End Gateway Channels]\n\n";
            }
//            else
//                qDebug()<<s.toString()<<vm.value(s.toString()).toString();
        }
        //qDebug()<<"\n\n";
    }
    stations.sort();
    //qDebug()<<"Station List"<<freqMap;
    //qDebug()<<"Freq List"<<freqs.keys();
//    ui->stationComboBox->clear();
//    ui->stationComboBox->addItems(stations);
    int i = 0;
    foreach (const QString s, freqMap.keys())
    {
        QTableWidgetItem * it = new QTableWidgetItem(); // call signs
        QTableWidgetItem * it2 = new QTableWidgetItem(); // center freqs
        QTableWidgetItem * it1 = new QTableWidgetItem(); // dial freqs
        it->setText(s);
        it2->setText(freqMap.values(s).join(","));
        QString centerlist = freqMap.values(s).at(0);
        QStringList centers = centerlist.split(",");
        QStringList formatted;
        //qDebug()<<"centers"<<centers;
        for(int i = 0; i < centers.length(); i++)
        {
            QString rawfreq = centers.at(i).trimmed();
            int work = rawfreq.toInt();
            formatted<<rawfreq.insert(-3, ',');
            //qDebug()<<centers.at(i);
            work -= 1500;
            centers[i] = QString::number(work).insert(-3, '.');
        }
        it1->setText(centers.join(","));
        it2->setText(formatted.join(","));
        ui->tableWidget->insertRow(i);
        ui->tableWidget->setItem(i, 0, it);
        ui->tableWidget->setItem(i, 1, it1);
        ui->tableWidget->setItem(i, 2, it2);
        i++;
    }
}

void StationListDialog::on_closeButton_clicked()
{
    close();
}
