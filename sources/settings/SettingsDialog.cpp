/****************************************************************************
 * Main developer, C# developing:                                           *
 * Copyright (C) 2014-2016 by Sergey Zheigurov                              *
 * Russia, Novy Urengoy                                                     *
 * zheigurov@gmail.com                                                      *
 *                                                                          *
 * C# to Qt portation, Linux developing                                     *
 * Copyright (C) 2015-2019 by Eduard Kalinowski                             *
 * Germany, Lower Saxony, Hanover                                           *
 * eduard_kalinowski@yahoo.de                                               *
 *                                                                          *
 * C# project CNC-controller-for-mk1                                        *
 * https://github.com/selenur/CNC-controller-for-mk1                        *
 *                                                                          *
 * The Qt project                                                           *
 * https://github.com/eduard-x/cnc-qt                                       *
 *                                                                          *
 * CNC-Qt is free software; may be distributed and/or modified under the    *
 * terms of the GNU General Public License version 3 as published by the    *
 * Free Software Foundation and appearing in the file LICENSE_GPLv3         *
 * included in the packaging of this file.                                  *
 *                                                                          *
 * This program is distributed in the hope that it will be useful,          *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 * GNU General Public License for more details.                             *
 *                                                                          *
 * You should have received a copy of the GNU Lesser General Public         *
 * License along with CNC-Qt. If not, see  http://www.gnu.org/licenses      *
 ****************************************************************************/

#include <QtGui>

#include <QColorDialog>

#include "SettingsDialog.h"

/******************************************************************************
** SettingsDialog
*/


SettingsDialog::SettingsDialog(QWidget *p, int tabNum)
    : QDialog(p)
{
    setupUi(this);

    parent = static_cast<MainWindow*>(p);

    cnc = parent->mk1;

    setStyleSheet(parent->programStyleSheet);


    sParser = new SettingsParser(p);
    scrollAreaParser->setWidget(sParser);

    sControl = new SettingsControl(p);
    scrollAreaControl->setWidget(sControl);

    sVis = new SettingsVis(p);
    scrollAreaVis->setWidget(sVis);

    sSpeed = new SettingsSpeed(p);
    scrollAreaDriver->setWidget(sSpeed);

    sMaterial = new SettingsMaterial(p);
    scrollAreaTool->setWidget(sMaterial);

    sSystem = new SettingsSystem(p);
    scrollAreaSystem->setWidget(sSystem);

    sWorkbench = new SettingsWorkbench(p);
    scrollAreaWorkbench->setWidget(sWorkbench);

    sIO = new SettingsIO(p);
    scrollAreaIO->setWidget(sIO);

    grpArr.clear();
    grpArr << (QVector <QGroupBox*>() << sWorkbench->groupRanges << sWorkbench->groupHome << sWorkbench->groupSoftwareLimits); // workbench
    grpArr << (QVector <QGroupBox*>() << sSpeed->groupSpeed << sSpeed->groupDirections); // moving
    grpArr << (QVector <QGroupBox*>() << sIO->groupHardwareLimits << sIO->groupConnectors << sIO->groupOutput << sIO->groupJog << sIO->groupExtPin); // I/O
    grpArr << (QVector <QGroupBox*>() << sSystem->groupBacklash << sSystem->groupLookahead); // system
    grpArr << (QVector <QGroupBox*>() << sParser->groupBoxOptimize << sParser->groupBoxRepeat << sParser->groupBoxArc); // parser
    grpArr << (QVector <QGroupBox*>() << sMaterial->groupTool << sMaterial->groupMaterial << sMaterial->groupCalc); // tool
    grpArr << (QVector <QGroupBox*>() << sVis->groupViewing << sVis->groupBoxColors << sVis->groupBoxGrid << sVis->groupBoxShowRang); // 3d
    grpArr << (QVector <QGroupBox*>() << sControl->groupRemote << sControl->groupKeyboard); // control

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(onSave()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    adjustSize();

    translateDialog();

    QTreeWidgetItem * item;
    item = treeWidget->topLevelItem ( tabNum );

    if (item != NULL) {
        treeWidget->setCurrentItem( item );
    }
}

SettingsDialog::~SettingsDialog()
{
    delete sIO;
    delete sWorkbench;
    delete sParser;
    delete sSpeed;
    delete sControl;
    delete sMaterial;
    delete sSystem;
    delete sVis;
}


/**
 * @brief translate the window elements
 */
void SettingsDialog::translateDialog()
{
    setWindowTitle(translate(ID_SETTINGS_TITLE));

    //    menu items

    QStringList menuList;
    menuList << translate(ID_WORKBENCH);
    menuList << translate(ID_MOVING_SETTINGS);
    menuList << translate(ID_IO);
    menuList << translate(ID_SYSTEM);
    menuList << translate(ID_PARSER);
    menuList << translate(ID_WORK_TOOL);
    menuList << translate(ID_VISUALISATION);
    menuList << translate(ID_CONTROL);


    treeWidget->setColumnCount(1);
    QList<QTreeWidgetItem *> items;

    foreach (QString s, menuList) {
        QStringList sub = s.split(";");
        QVector <QString> v = sub.toVector();
        menuArr << v;
        QTreeWidgetItem *m = new QTreeWidgetItem(treeWidget, QStringList(sub.at(0)));
        items.append(m);

        if (sub.count() > 1) {
            sub.removeAt(0);

            foreach (QString ssub, sub) {
                items.append(new QTreeWidgetItem(m, QStringList(ssub)));
            }

            //             m->setExpanded(true);
        }
    }

    treeWidget->header()->close();

    for (int i = 0; i < menuArr.count(); i++) {
        if ((grpArr.at(i).count() == menuArr.at(i).count() - 1)) {
            for (int j = 0; j < menuArr.at(i).count() - 1; j++) {
                ((QGroupBox*)(grpArr.at(i).at(j)))->setTitle(menuArr.at(i).at(j + 1));
                ((QGroupBox*)(grpArr.at(i).at(j)))->adjustSize();
            }
        }
    }

    int width = treeWidget->sizeHint().width();
    treeWidget->setFixedWidth(width);

    connect(treeWidget, SIGNAL(currentItemChanged ( QTreeWidgetItem *, QTreeWidgetItem * )), this, SLOT(onSelection(QTreeWidgetItem*, QTreeWidgetItem *)));

    // end of menu items


    tabWidget->setStyleSheet("QTabBar::tab { height: 0px; width: 0px; border: 0px solid #333; }" );


    QList<QAbstractButton*> l = buttonBox->buttons();
    QStringList strl = (QStringList() << translate(ID_SET) << translate(ID_CANCEL));

    for(int i = 0; i < l.count(); i++) {
        l[i]->setText(strl.at(i));
    }
}

/**
 * @brief selection in menu tree
 *
 */
void SettingsDialog::onSelection(QTreeWidgetItem* it, QTreeWidgetItem * ip)
{
    QString mainText;
    QString childText;

    disconnect(treeWidget, SIGNAL(currentItemChanged ( QTreeWidgetItem *, QTreeWidgetItem * )), this, SLOT(onSelection(QTreeWidgetItem*, QTreeWidgetItem *)));

    if (it->parent() != NULL) {
        mainText =  it->parent()->text(0);
        childText = it->text(0);

        if (ip != NULL) {
            ip->setSelected(false);
        }

        it->setSelected(true);
    } else {
        mainText = it->text(0);

        //         treeWidget->collapseAll();

        if (ip != NULL) {
            ip->setSelected(false);
        }

        it->setExpanded(true);
        it->setSelected(true);
    }

    //     qDebug() << mainText << childText;

    for (int idxRow = 0; idxRow < menuArr.count(); idxRow++) {
        if (menuArr.at(idxRow).at(0) == mainText) { // selected element was found
            tabWidget->setCurrentIndex(idxRow);

            // display selected widget
            if (childText.length() > 0) {
                for (int idxCol = 1; idxCol < menuArr.at(idxRow).count(); ++idxCol) {
                    // check the sizing of arrays
                    if ((grpArr.count() == menuArr.count()) && (grpArr.at(idxRow).count() == menuArr.at(idxRow).count() - 1)) {
                        if (childText == menuArr.at(idxRow).at(idxCol)) {
                            grpArr.at(idxRow).at(idxCol - 1)->setHidden(false);
                        } else {
                            grpArr.at(idxRow).at(idxCol - 1)->setHidden(true);
                        }
                    }
                }
            } else { // display all widgets
                for (int idxCol = 1; idxCol < menuArr.at(idxRow).count(); ++idxCol) {
                    // check the sizing of arrays
                    if ((grpArr.count() == menuArr.count()) && (grpArr.at(idxRow).count() == menuArr.at(idxRow).count() - 1)) {
                        grpArr.at(idxRow).at(idxCol - 1)->setHidden(false);
                    }
                }
            }

            break;
        }
    }

    connect(treeWidget, SIGNAL(currentItemChanged ( QTreeWidgetItem *, QTreeWidgetItem * )), this, SLOT(onSelection(QTreeWidgetItem*, QTreeWidgetItem *)));
}


/**
 * @brief save serrings
 *
 */
void SettingsDialog::onSave()
{
    sIO->getSettings();
    sWorkbench->getSettings();
    sParser->getSettings();
    sSpeed->getSettings();
    sControl->getSettings();
    sMaterial->getSettings();
    sSystem->getSettings();
    sVis->getSettings();

    accept();
}

