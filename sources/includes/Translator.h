/****************************************************************************
 * Main developer, C# developing:                                           *
 * Copyright (C) 2014-2017 by Sergey Zheigurov                              *
 * Russia, Novy Urengoy                                                     *
 * zheigurov@gmail.com                                                      *
 *                                                                          *
 * C# to Qt portation, Linux developing                                     *
 * Copyright (C) 2015-2017 by Eduard Kalinowski                             *
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

#ifndef LANGUAGE_HEADER
#define LANGUAGE_HEADER

#include <QString>
#include <QVector>



class cTranslator
{
    public:
        bool loadTranslation(const QString fname);
    public:
        static QString translate(int id);
        static QString engText[];
    private:
        QString convertString(const QString &s);
        //         Q_DISABLE_COPY(cTranslator);

    private:
        static QVector<QString> translateTable;
};

//
// all enums TR_CONSTS are for direct access to the text from engText or translateTable
// not over maps or text id's
//
enum TR_CONSTS {
    ID_PROG_NAME = 0,
    ID_ENDSTREAM, //1
    ID_BREAK_CONN, //2
    ID_CONN_SUCCSESS, //3
    ID_DEMO, //4
    ID_CONNECTED, //5
    ID_DEV_NOTFOUND, //6
    ID_START_STREAM, //7
    ID_ALREADY_EXISTS, //8
    ID_NOT_POSSIBLE, //9
    ID_DISCONNECTING, //10
    ID_ERR, //11
    ID_MSG_FOR_DISABLE, //12
    ID_MSG_NO_CONN, //13
    ID_MSG_NO_DATA, //14
    ID_START_PROG, //15
    ID_START_TASK_AT, //16
    ID_END_TASK_AT, //17
    ID_PAUSE, //18
    ID_OPEN_TITLE, //19
    ID_LOAD_FROM_FILE, //20
    ID_DISCONNECT_FROM_DEV, //21
    ID_CONNECT_TO_DEV, //22
    ID_START_TASK, //23
    ID_PAUSE_TASK, //24
    ID_STOP_TASK, //25
    ID_RUN_TASK, //26
    ID_WAIT, //27
    ID_QUEST_START_FROMLINE, //28
    ID_QUEST_START_FROMTOLINE, //29
    ID_RECIEVED_M0, //30
    ID_PAUSE_G4, //31
    ID_PAUSE_ACTIVATED, //32
    ID_POSITION, //33
    ID_NUM_INSTR, //34
    ID_MM_MIN, //35
    ID_NOT_DECODED, //36
    ID_FILEERR, // 37
    ID_ABOUT_TITLE, // 38
    ID_ABOUT_TEXT,  // 39
    ID_CORRECTURE, // 40
    ID_PROPORTION, // 41
    ID_EDITGCODE_TITLE, // 42
    ID_OFFSET_GCODE, // 43
    ID_CORRECT_Z, // 44
    ID_NUMPAD_HELP, // 45
    ID_VELOCITY, // 46
    ID_MOUSE_CONTROL, // 47
    ID_BEG_SURFACE, // 48
    ID_STEP_PROP, // 49
    ID_STEP_Y, // 50
    ID_STEP_X, // 51
    ID_NUM_X, // 52
    ID_NUM_Y, // 53
    ID_SCAN, // 54
    ID_VIEW_ONLY, // 55
    ID_TEST_SCAN, // 56
    ID_RET_MM, // 57
    ID_SPEED, // 58
    ID_TABLE_POINT, // 59
    ID_SET_Z, // 60
    ID_MOVE_TOPOINT, // 61
    ID_SCANNING, // 62
    ID_STOP_SCAN, // 63
    ID_PULSES_PER_MM, // 64
    ID_DEV_SIMULATION, // 65
    ID_DEV_SIM_HELP, // 66
    ID_CANCEL, // 67
    ID_APPLY, // 68
    ID_SETTINGS_TITLE, // 69
    ID_DISPLAY_SPINDLE, // 70
    ID_DISPLAY_GRID, // 71
    ID_STEP, // 72
    ID_BEGIN, // 73
    ID_END, // 74
    ID_DISPLAY_SURFACE, // 75
    ID_DISPLAY_AXES, // 76
    ID_BASIC,  // 77
    ID_RESET, // 78
    ID_MAXIMUM, // 79
    ID_MINIMUM, // 80
    ID_DISPLAY_RANG, // 81
    ID_ADDITIONAL_3D, // 82
    ID_SETTINGS3D_TITLE, // 83
    ID_FILE, // 84
    ID_OPEN_FILE, // 85
    ID_WORK_END, // 86
    ID_CONTROLLER, // 87
    ID_CONNECT, // 88
    ID_DISCONNECT, //89
    ID_CONTR_SETTINGS, // 90
    ID_HELP, // 91
    ID_ABOUT, // 92
    ID_SPINDLE, // 93
    ID_STOP, // 94
    ID_ADDITIONAL, // 95
    ID_SETT_CONTROLLER, // 96
    ID_SETT_3D, // 97
    ID_MOD_GENERATION, // 98
    ID_MANIPULATION, // 99
    ID_POINTS, // 100
    ID_COORDINATES, // 101
    ID_DISPL_LIMITS, // 102
    ID_MIN, // 103
    ID_MAX, // 104
    ID_MANUAL_CONTROL, // 105
    ID_VELO, // 106
    ID_NUMPAD_CONTROL, // 107
    ID_GCODE_RUNNING, // 108
    ID_MANUAL_VELO, // 109
    ID_VELO_TRANSMISSION, // 110
    ID_VELO_MOVING, // 111
    ID_SOURCE_3D, // 112
    ID_GEN_SIGNAL, // 113
    ID_RC, // 114
    ID_HZ, // 115
    ID_OFF, // 116
    ID_VELO_PWM, // 117
    ID_CHAN_PWM, // 118
    ID_SEND_COMMAND, // 119
    ID_ON_SPINDLE, // 120
    ID_BYTE_19, // 121
    ID_BYTE_15, // 122
    ID_BYTE_14, // 123
    ID_MOVING_TOPOINT, // 124
    ID_RUN, // 125
    ID_CLEAN, // 126
    ID_PROG_RUNNING, // 127
    ID_CURRENT_LINE, // 128
    ID_G_CODE, // 129
    ID_YES, // 130
    ID_NO, // 131
    ID_OK, // 132
    ID_WRONGLINE, // 133
    ID_COMMAND, // 134
    ID_STATE, // 135
    ID_INFO, // 136
    ID_ROTATION, // 137
    ID_EXIT, // 138
    ID_LANGUAGE, // 139
    ID_SETTINGS, // 140
    ID_PROGRAM, // 141
    ID_DATA, // 142
    ID_LOG, // 143
    ID_3D_VIEW, // 144
    ID_SUBMISSION, // 145
    ID_MOVING, // 146
    ID_HOTPLUGED, //  147
    ID_DETACHED, // 148
    ID_WARN, // 149
    ID_REALLYQUIT, // 150
    ID_CONTROLPAD_HELP, // 151
    ID_BUTTON, // 152
    ID_PRESS_BUTTON, // 153
    ID_USEDEF_TEXT, // 154
    ID_ALGORITHM_Z, // 155
    ID_GRID, // 156
    ID_ENABLED, // 157
    ID_DISABLED, // 158
    ID_FROM_TO, // 159
    ID_DISPLAY_LINES, // 160
    ID_DISPLAY_POINTS, // 161
    ID_CALC_TITLE, // 162
    ID_UNITS, // 163
    ID_MATERIAL, // 164
    ID_TOOL, // 165
    ID_CUTTING_SPEED, // 166
    ID_DIAMETER, // 167
    ID_FLUTES, // 168
    ID_MAX_DEPTH, // 169
    ID_RANGES, // 170
    ID_RANGE, // 171
    ID_SPINDLE_SPEED, // 172
    ID_CHIPLOAD, // 173
    ID_FEED_RATE, // 174
    ID_MATERIAL_LIST, // 175
    ID_MESSURE_UNIT, // 176
    ID_FEED_INFO, // 177
    ID_STARTVELO, // 178
    ID_ENDVELO, // 179
    ID_ACCELERATION, // 180
    ID_SET, // 181
    ID_COOLANT, // 182
    ID_MIST, // 183
    ID_POS, // 184
    ID_SEQUENCE, // 185
    ID_SOFTLIMITS, // 186
    ID_HARDLIMITS, // 187
    ID_USE, // 188
    ID_WORKTABLE, // 189
    ID_LIMITS, // 190
    ID_PARKING, // 191
    ID_SWAP, // 192
    ID_MOTORS, // 193
    ID_RECENTFILES, // 194
    ID_LOOKAHEAD, // 195
    ID_ARC_SPLITTING, // 196
    ID_IO, // 197
    ID_CHECK_HW_LIMITS, // 198
    ID_GO_HOME_AT_START, // 199
    ID_GO_HOME_AT_END, // 200
    ID_WORKBENCH, // 201
    ID_DIAGNOSTIC, // 202
    ID_VISUALISATION, // 203
    ID_COLORS, // 204
    ID_DISABLE_VISUALISATION, // 205
    ID_COLOR_LIST, //206
    ID_STEP_DISTANCE, // 207
    ID_DISPLAY_COMMAND, // 208
    ID_DISPLAY_WORKBENCH,
    ID_DISPLAY_TRAVERSE, // 210
    ID_SMOOTH_MOVING, // 211
    ID_POINT_SIZE, // 212
    ID_LINE_WIDTH, // 213
    ID_ISO, // 214
    ID_TOP, // 215
    ID_FRONT, // 216
    ID_LEFT, // 217
    ID_FIT, // 218
    ID_SAVE_GCODE, // 219
    ID_PARSER, //220
    ID_REMOVE_REPEAT, // 221
    ID_WORK_TOOL, // 222
    ID_CONTROL, // 223
    ID_SYSTEM, // 224
    ID_SHAFT, // 225
    ID_DESCRIPTION, // 226
    ID_USING, // 227
    ID_TOOL_TABLE, // 228
    ID_SELECT_TOOL, // 229
    ID_MOVING_SETTINGS, // 230
    ID_NUMPAD, // 231
    ID_CONTROLPAD, // 232
    ID_USER_DEFINED, // 233
    ID_JOYPAD, // 234
    ID_SELECT_CONTROL, // 235
    ID_PORT, // 236
    ID_REMOTE_NAME, // 237
    ID_REMOTE_CONNECTION, // 238
    ID_REPEAT_CODE, // 239
    ID_NUM_REPEAT, // 240
    ID_DEPTH_SUM, // 241
    ID_OPTIMIZE_RAPID_WAYS, // 242
    ID_NULL
};


#endif
