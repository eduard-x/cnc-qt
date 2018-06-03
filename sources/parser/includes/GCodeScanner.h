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


#ifndef GCODE_H
#define GCODE_H

#include <stdlib.h>
#include <string>
#include <fstream>

#include <QString>
#include <QMap>
#include <QList>
#include <QVector>
#include <QVector3D>

#include "DataManager.h"


#if !defined(yyFlexLexerOnce)
#include <FlexLexer.h>
#endif

#include "parse_gcode.h"

using std::string;


// Tell flex which function to define
# undef YY_DECL
# define YY_DECL int gcode::scanner::lex(       \
        gcode::parser::semantic_type* yylval,   \
        gcode::parser::location_type* yylloc,   \
        gcode::driver& checker)


class cDataManager;


namespace gcode
{

class driver
{
    public:
        explicit driver (cDataManager *p);
        virtual ~driver ();

        // Handling the parser.
        void parse (const QString& f);
        bool trace_parsing;

    public:
        // Error handling.
        void  error (const gcode::parser::location_type& l, const string& m);
        void  error (const string& m);

        bool  find_key(const QString &s);
        float get_vari(const QString &s);
        void  add_varimap(const QString &s, float f);
        void  add_gcode(GData &g);
        void  dataChecker();

    public:
        gcode::scanner *lexer;
        string file;

    private:
        bool result;

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
        explicit scanner(std::istream* arg_yyin = 0,
                         std::ostream* arg_yyout = 0 );

        virtual ~scanner();

        virtual int lex(gcode::parser::semantic_type* yylval,
                        gcode::parser::location_type* yylloc,
                        gcode::driver& checker);
};

#ifdef  yylex
#undef yylex
#endif
#define yylex checker.lexer->lex

}

#endif
