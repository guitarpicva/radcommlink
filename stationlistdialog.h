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
