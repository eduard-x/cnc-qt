/****************************************************************************
 * Main developer, C# developing:                                           *
 * Copyright (C) 2014-2016 by Sergey Zheigurov                              *
 * Russia, Novy Urengoy                                                     *
 * zheigurov@gmail.com                                                      *
 *                                                                          *
 * C# to Qt portation, Linux developing                                     *
 * Copyright (C) 2015-2018 by Eduard Kalinowski                             *
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


#ifndef DXF_H
#define DXF_H


#include <stdlib.h>
#include <string>
#include <fstream>

#include <QString>
#include <QList>
#include <QVector>
#include <QVector3D>


/* Externalize variables used by the scanner and parser. */


///
/// class for gerber file
///
class DXFData
{
        ///
        /// messure units, mm or inches
    public:
        QString UnitsType;

        ///
        /// spline types
        ///
        //         QList<typeSpline> typeSplines;

        ///
        /// points from file
        ///
        //         QList<grbPoint> points;

        // length of number
        int countDigitsX;
        int countDigitsY;
        // length of digs after dec.point
        int countPdigX;
        int countPdigY;

        int X_min;
        int X_max;

        int Y_min;
        int Y_max;

    public:
        DXFData();
        //         void CalculateGatePoints(int _accuracy);

};

// #include "GData.h"

#include "DataManager.h"


#if !defined(yyFlexLexerOnce)
#include <FlexLexer.h>
#endif

#include "parse_dxf.h"


using std::string;

// Tell flex which function to define
#ifdef  YY_DECL
# undef YY_DECL
#endif
#define YY_DECL                                 \
    int dxf::scanner::lex(                       \
            dxf::parser::semantic_type* yylval,  \
            dxf::parser::location_type* yylloc,   \
            dxf::driver& checker)

class cDataManager;

namespace dxf
{
// class scanner;

class driver
{
    public:
        explicit driver (cDataManager *p);
        virtual ~driver ();

        // Handling the parser.
        void parse (const QString& f);
        //         void parse (std::istream& f);
        bool trace_parsing;

    public:
        string file;

        // To demonstrate pure handling of parse errors, instead of
        // simply dumping them on the standard error output, we will pass
        // them to the driver using the following two member functions.
        // Finally, we close the class declaration and CPP guard.

        // Error handling.
        void error (const dxf::parser::location_type& l, const string& m);
        void error (const string& m);

        dxf::scanner *lexer;
        bool result;

        // is static???
        QVector<DXFData> dxfVector;

        cDataManager *parent;
};

// To feed data back to bison, the yylex method needs yylval and
// yylloc parameters. Since the yyFlexLexer class is defined in the
// system header <FlexLexer.h> the signature of its yylex() method
// can not be changed anymore. This makes it necessary to derive a
// scanner class that provides a method with the desired signature:

#include <iostream>

class scanner : public yyFlexLexer
{
    public:
        explicit scanner(std::istream* in = 0, std::ostream* out = 0);
        virtual ~scanner();

        virtual int lex(dxf::parser::semantic_type* yylval,
                        dxf::parser::location_type* yylloc,
                        dxf::driver& checker);
};

#ifdef  yylex
# undef yylex
#endif
#define yylex checker.lexer->lex

}

#endif // GERBER_H
