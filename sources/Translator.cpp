/****************************************************************************
 * Main developer:                                                          *
 * Copyright (C) 2014-2015 by Sergej Zheigurov                              *
 * Russia, Novy Urengoy                                                     *
 * zheigurov@gmail.com                                                      *
 *                                                                          *
 * Qt developing                                                            *
 * Copyright (C) 2015 by Eduard Kalinowski                                  *
 * Germany, Lower Saxony, Hanover                                           *
 * eduard_kalinowski@yahoo.de                                               *
 *                                                                          *
 * ported from C# project CNC-controller-for-mk1                            *
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

#include <QDir>
#include <QDebug>
#include <QTextStream>

#include "includes/MainWindow.h"
#include "includes/Translator.h"

// extern MainWidget* mainWidget;

QVector<QString> cTranslator::translateTable;


QString cTranslator::engText[] = {
    "CNC-Qt", // 0
    "End of controller management",
    "Read data error from controller, disconnect!",
    "Connecting to controller, successfully",
    "...DEMO MODE started...",
    "Connected to controller",
    "Connected controller not found.",
    "Starting data flow, work with controller",
    "Connection exists!",
    "Second connection is not possible, until the current is interrupted!",
    "End of connection to controller!", // 10
    "Error",
    "Before exit from program, please disconnect from CNC!",
    "No connection to controller!",
    "No data for execution!",
    "Start execution of program",
    "Start task at ",
    "End of task at ",
    "Pause",
    "CNC-Qt: ",
    "Loading data from file: ", // 20
    "Connect to controller",
    "Disconnect from controller",
    "Start of task",
    "Pause execution",
    "Abort of task",
    "Task running",
    "waiting",
    "Start of G-code execution form line: %1\n and to the end?",
    "Start of G-code execution form line: %1 to line: %2 ?",
    "Recieved the command 'M0' to stop! To further execution press the button 'pause'", // 30
    "Pause %1 msec. during command 'G4'",
    "PAUSE activated! Place instrument nr:%1 with diameter: %2 mm. and press the button 'pause'",
    "Position in 3D: x=%1 y=%2 z=%3 scale.: %4",
    "Instruction nr: ",
    " mm./min.",
    "In line: %1 commands are not decoded: ",
    "File error",
    "About program",
    "Program for MK1 controller management, information about controller: %1\ninformation about the C# version of program: %2\n and: %3\nQt project: %4",
    "APPLY DATA CORRECTURE", // 40
    "Proportions (%)",
    "Correcture G-code",
    "G-code offset (mm.)",
    "Correcture height Z in view of\nscanned surface",
    "For manual controlling used numerical keyboard (NumPad)\n\nKey '1' -X and -Y moving\nKey '2' -Y moving\nKey '3' +X and -Y moving\nKey '6' +X moving\nKey '9' +X and +Y moving\nKey '8' +Y moving\nKey '7' -X and +Y moving\nKey '4' -X moving\n\nKey '5' +Z moving\nKey '0'  -Z",
    "moving velocity",
    "Mouse controlling",
    "Start point surface",
    "Size and number of steps",
    "STEP Y:", // 50
    "STEP X:",
    "Num. X:",
    "Num. Y:",
    "Scanning",
    "Table is read only",
    "Test scanning",
    "Return mm.",
    "Speed",
    "Editing of point form table",
    "Set Z", // 60
    "move to point",
    "Scanning surface",
    "Interrupt",
    "Number of impulses per 1 mm",
    "Device emulation (virtual controller)",
    "In simulation mode, if hardware controller exists, using of virtual controller!!!",
    "Cancel",
    "Apply",
    "Settings of program",
    "Display instrument (spindle)", // 70
    "Display grid (mm.)",
    "step",
    "begin",
    "end",
    "Display scanned surface",
    "Display axis X,Y,Z",
    "Main",
    "Reset regions",
    "maximum",
    "minimum", // 80
    "display regions of working area",
    "Additional settings 3D",
    "3D Settings",
    "&File",
    "Open file",
    "End of work",
    "&Controller",
    "Connect",
    "Disconnect",
    "Settings controller", // 90
    "Help",
    "About program",
    "SPINDLE",
    "STOP",
    "Additional",
    "Settings controller",
    "Settings 3D displaying",
    "Code generation module",
    "G-code manipulation",
    "Points set", // 100
    "Coordinates",
    "Limits indication",
    "min",
    "max",
    "Manual control",
    "Speed:",
    "NumPad control",
    "Execution G-code ",
    "Manual control velocity",
    "Transmission speed:", // 110
    "Moving speed:",
    "Initial 3D-View",
    "Signal generation",
    "RC",
    "Hz",
    "off",
    "PWM speed",
    "PWM channel number",
    "Send command",
    "spindle on", // 120
    "Byte 19 (E-sop)",
    "Byte 15 (axis limits)",
    "Byte 14 (coollant on)",
    "moving to point",
    "RUN",
    "Clean",
    "Execution of program",
    "Line nr:",
    "G-code",
    "Yes", // 130
    "No",
    "Ok",
    "Error in line:",
    "Command",
    "State",
    "Info",
    "Rotation",
    "Exit",
    "Language",
    "Settings", // 140
    "Program",
    "Data",
    "Log",
    "3D View",
    "Submission",
    "Movement",
    "Device hotplugged",
    "Device detached",
    "Warning",
    "Do you really want to close the program?",
    ""
};


bool cTranslator::loadTranslation(const QString fname)
{
    QFile langFile(fname);
    short vecSize = (short)(sizeof(engText) / sizeof(char*));
    translateTable.clear();
    translateTable.resize(vecSize + 1);

    if (!langFile.open(QIODevice::ReadOnly)) {
        return false;
    }

    //     qDebug() << fname;

    QTextStream stream(&langFile);
    stream.setCodec("UTF-8");

    QString llEng = stream.readLine();

    if (llEng.length() > 0) {
        llEng.remove("\n");
    }

    if (llEng == "LANGUAGE_NAME") {
        translateTable << llEng;
    } else {
        MessageBox::exec(this, translate(_FILEERR), "File is not a language file!", QMessageBox::Warning);

        langFile.close();
        return (false);
    }

    bool begTranslation = false;

    while (!stream.atEnd()) {
        llEng = stream.readLine();
        llEng = convertString(llEng);

        if (llEng.indexOf(QRegExp("t[0-9]{3}=")) == 0) {
            int num, pos;
            QString str;
            pos = llEng.indexOf("=");

            if (pos > 0) {
                str = llEng.mid(pos + 1);
                //
                num = llEng.mid(1, pos - 1).toShort();

                //                     qDebug() << num << str;
                if (num >= vecSize) {
                    qDebug() << "size too big";
                } else {
                    translateTable[num] = str;
                }
            }
        } else {
            if (llEng == "Ok") { // translation part beginns with "Ok"
                begTranslation = true;
            }

            if (begTranslation == false) {
                continue;
            }

            bool found = false;
            QString llTranslate = stream.readLine();

            if (llTranslate.length() == 0) {
                continue;
            }

            llTranslate = convertString(llTranslate);

            for(int i = 0; i < vecSize; i++) { // suche
                QString tmpEng = engText[i];

                if (tmpEng.indexOf("%HOME%") >= 0) {
                    tmpEng.replace("%HOME%", QDir::homePath());
                }

                if ( tmpEng == llEng) {
                    translateTable[i] = llTranslate;
                    found = true;
                    break;
                }
            }

            if (found == false) {
                qDebug() << "no translation found" << llEng;
            }

        }
    }

    langFile.close();

    return (true);
}


QString cTranslator::convertString(const QString &s)
{
    QString res = s;

    if (res.length() > 0) {
        res.replace( "\\r\\n", "\n");
        res.replace( "\\n", "\n");
        res.replace( "\\t", "\t");
        res.replace( "\\\\", "/");
        res.replace( "&amp;", "&");
        res.replace( "&gt;", ">");
        res.replace( "&lt;", "<");
    }

    if (res.length() == 0) {
        return res;
    }

    //     if (res.indexOf("winhttrack") >= 0) {
    //         res.replace("winhttrack", "httraqt");
    //     }

    if (res.indexOf("C:/") >= 0) {
        res.replace("C:/", "%HOME%/");
    }

    if (res.indexOf("c:/") >= 0) {
        res.replace("c:/", "%HOME%/");
    }

    if (res.indexOf("%HOME%") >= 0) {
        res.replace("%HOME%", QDir::homePath());
    }

    return res;
}


QString cTranslator::translate( int id )
{
    QString m;

    if (id < 0) {
        return "";
    }

    if (translateTable.size() == 0) {
        return QString(engText[id]);
    }

    if ((id >= 0) && (id < (int)(sizeof(engText) / sizeof(char*)))) {
        if(translateTable[id].length() == 0) {
            m = QString(engText[id]);
        } else {
            m = translateTable[id];
        }

        if (m.indexOf("%HOME%") >= 0) {
            m.replace("%HOME%", QDir::homePath());
        }
    }

    return m;
}


