/************************************************************************
 **
 **  @file   dialogpointofintersection.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   November 15, 2013
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2015 Valentina project
 **  <https://gitlab.com/smart-pattern/valentina> All Rights Reserved.
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

#ifndef DIALOGPOINTOFINTERSECTION_H
#define DIALOGPOINTOFINTERSECTION_H

#include <qcompilerdetection.h>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QtGlobal>

#include "../vmisc/def.h"
#include "dialogtool.h"

namespace Ui
{
    class DialogPointOfIntersection;
}

/**
 * @brief The DialogPointOfIntersection class dialog for ToolPointOfIntersection. Help create point and edit option.
 */
class DialogPointOfIntersection : public DialogTool
{
    Q_OBJECT
public:
    DialogPointOfIntersection(const VContainer *data, quint32 toolId, QWidget *parent = nullptr);
    virtual ~DialogPointOfIntersection() override;

    QString        GetPointName() const;
    void           SetPointName(const QString &value);

    quint32        GetFirstPointId() const;
    void           SetFirstPointId(quint32 value);

    quint32        GetSecondPointId() const;
    void           SetSecondPointId(quint32 value);
public slots:
    virtual void   ChosenObject(quint32 id, const SceneObject &type) override;
    virtual void   PointNameChanged() override;
protected:
    virtual void   ShowVisualization() override;
    /**
     * @brief SaveData Put dialog data in local variables
     */
    virtual void   SaveData() override;
    virtual bool   IsValid() const final;
private:
    Q_DISABLE_COPY(DialogPointOfIntersection)

    /** @brief ui keeps information about user interface */
    Ui::DialogPointOfIntersection *ui;

    QString pointName;

    bool flagName;
    bool flagError;
};

//---------------------------------------------------------------------------------------------------------------------
inline bool DialogPointOfIntersection::IsValid() const
{
    return flagName && flagError;
}

#endif // DIALOGPOINTOFINTERSECTION_H
