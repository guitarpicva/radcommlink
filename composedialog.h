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

    void closeEvent(QCloseEvent *);
private slots:
    void on_discardButton_clicked();

    void on_postButton_clicked();

    void on_ics213TemplateButton_clicked();

private:
    Ui::ComposeDialog *ui;
    const QString ICS213TEMPLATE = "1. \r\n2. \r\n3. \r\n4. \r\n5. \r\n6. \r\n7. \r\n8. \r\n9. \r\n10. ";
    void saveSettings();
    void loadSettings();
};

#endif // COMPOSEDIALOG_H
