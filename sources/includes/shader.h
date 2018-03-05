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

#ifndef GLES_SHADER_H
#define GLES_SHADER_H


const char *vsrc =
    "#ifdef GL_ES\n"
    "// Set default precision to medium\n"
    "precision mediump int;\n"
    "precision mediump float;\n"
    "#endif\n"
    "\n"
    "uniform mat4 mvp_matrix;\n"
    "uniform mat4 mv_matrix;\n"
    "uniform float point_size;\n"
    "\n"
    "attribute vec4 a_position;\n"
    "attribute vec4 a_color;\n"
    "attribute vec4 a_start;\n"
    "\n"
    "varying vec4 v_color;\n"
    "varying vec2 v_position;\n"
    "varying vec2 v_start;\n"
    "\n"
    "bool isNan(float val)\n"
    "{\n"
    "    return (val > 65535.0);\n"
    "}\n"
    "\n"
    "void main()\n"
    "{\n"
    "    // Calculate interpolated vertex position & line start point\n"
    "    v_position = (mv_matrix * a_position).xy;\n"
    "\n"
    "    if (!isNan(a_start.x)) v_start = (mv_matrix * a_start).xy;\n"
    "    else v_start = a_start.xy;\n"
    "\n"
    "    // Calculate vertex position in screen space\n"
    "    gl_Position = mvp_matrix * a_position;\n"
    "\n"
    "    // Value will be automatically interpolated to fragments inside polygon faces\n"
    "    v_color = a_color;\n"
    "\n"
    "    // Set point size\n"
    "    gl_PointSize = point_size;\n"
    "}\n"
    "\n";

const char *fsrc =
    "#ifdef GL_ES\n"
    "// Set default precision to medium\n"
    "precision mediump int;\n"
    "precision mediump float;\n"
    "#endif\n"
    "\n"
    "//Dash grid (px) = factor * pi;\n"
    "const float factor = 2.0;\n"
    "\n"
    "uniform float point_size;\n"
    "\n"
    "varying vec4 v_color;\n"
    "varying vec2 v_position;\n"
    "varying vec2 v_start;\n"
    "\n"
    "bool isNan(float val)\n"
    "{\n"
    "    return (val > 65535.0);\n"
    "}\n"
    "\n"
    "void main()\n"
    "{\n"
    "    // Draw dash lines\n"
    "    if (!isNan(v_start.x)) {\n"
    "        vec2 sub = v_position - v_start;\n"
    "        float coord = length(sub.x) > length(sub.y) ? gl_FragCoord.x : gl_FragCoord.y;\n"
    //         "        if (cos(coord / factor) > 0.0) discard;\n"
    "    }\n"
    "#ifdef GL_ES\n"
    "    if (point_size > 0.0) {\n"
    "        vec2 coord = gl_PointCoord.st - vec2(0.5, 0.5);\n"
    "        if (length(coord) > 0.5) discard;\n"
    "    }\n"
    "#endif\n"
    "\n"
    "    // Set fragment color from texture\n"
    "    gl_FragColor = v_color;\n"
    "}\n";


#endif
