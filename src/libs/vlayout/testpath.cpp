/************************************************************************
 **
 **  @file   testpath.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   28 8, 2019
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2019 Valentina project
 **  <https://bitbucket.org/dismine/valentina> All Rights Reserved.
 **
 **  Valentina is free software: you can redistribute it and/or modify
 **  it under the terms of the GNU General Public License as published by
 **  the Free Software Foundation, either version 3 of the License, or
 **  (at your option) any later version.
 **
 **  Valentina is distributed in the hope that it will be useful,
 **  but WITHOUT ANY WARRANTY; without even the implied warranty of
 **  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 **  GNU General Public License for more details.
 **
 **  You should have received a copy of the GNU General Public License
 **  along with Valentina.  If not, see <http://www.gnu.org/licenses/>.
 **
 *************************************************************************/
#include "testpath.h"

#include <QVector>
#include <QJsonArray>
#include <QTemporaryFile>
#include <QJsonObject>
#include <QTextStream>
#include <QJsonDocument>
#include <QPointF>

//---------------------------------------------------------------------------------------------------------------------
#if !defined(V_NO_ASSERT)
// Use for writing tests
void VectorToJson(const QVector<QPointF> &points, QJsonObject &json)
{
    QJsonArray pointsArray;
    for (auto point: points)
    {
        QJsonObject pointObject;
        pointObject[QLatin1String("type")] = "QPointF";
        pointObject[QLatin1String("x")] = point.x();
        pointObject[QLatin1String("y")] = point.y();

        pointsArray.append(pointObject);
    }
    json[QLatin1String("vector")] = pointsArray;
}

//---------------------------------------------------------------------------------------------------------------------
void DumpVector(const QVector<QPointF> &points)
{
    QTemporaryFile temp; // Go to tmp folder to find dump
    temp.setAutoRemove(false); // Remove dump manually
    if (temp.open())
    {
        QJsonObject vectorObject;
        VectorToJson(points, vectorObject);
        QJsonDocument vector(vectorObject);

        QTextStream out(&temp);
        out << vector.toJson();
        out.flush();
    }
}
#endif // !defined(V_NO_ASSERT)
