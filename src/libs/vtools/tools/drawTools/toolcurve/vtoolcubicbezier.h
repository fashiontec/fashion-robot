/************************************************************************
 **
 **  @file   vtoolcubicbezier.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   10 3, 2016
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2016 Valentina project
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

#ifndef VTOOLCUBICBEZIER_H
#define VTOOLCUBICBEZIER_H

#include <qcompilerdetection.h>
#include <QGraphicsItem>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QtGlobal>

#include "../vmisc/def.h"
#include "../ifc/xml/vabstractpattern.h"
#include "vabstractspline.h"

class VCubicBezier;
template <class T> class QSharedPointer;

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Weffc++")

struct VToolCubicBezierInitData : VAbstractToolInitData
{
    VToolCubicBezierInitData()
        : VAbstractToolInitData(),
          spline(nullptr)
    {}

    VCubicBezier *spline;
};

QT_WARNING_POP

class VToolCubicBezier : public VAbstractSpline
{
    Q_OBJECT
public:
    virtual ~VToolCubicBezier() Q_DECL_EQ_DEFAULT;
    virtual void setDialog() override;
    static VToolCubicBezier *Create(const QPointer<DialogTool> &dialog, VMainGraphicsScene *scene,
                                    VAbstractPattern *doc, VContainer *data);
    static VToolCubicBezier *Create(VToolCubicBezierInitData initData);
    static const QString ToolType;
    virtual int  type() const override {return Type;}
    enum { Type = UserType + static_cast<int>(Tool::CubicBezier)};

    QString FirstPointName() const;
    QString SecondPointName() const;
    QString ThirdPointName() const;
    QString ForthPointName() const;

    VCubicBezier getSpline()const;
    void         setSpline(const VCubicBezier &spl);

    virtual void ShowVisualization(bool show) override;
protected slots:
    virtual void ShowContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id=NULL_ID) override;
protected:
    virtual void RemoveReferens() override;
    virtual void SaveDialog(QDomElement &domElement, QList<quint32> &oldDependencies,
                            QList<quint32> &newDependencies) override;
    virtual void SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj) override;
    virtual void SetVisualization() override;
    virtual void RefreshGeometry() override;
private:
    Q_DISABLE_COPY(VToolCubicBezier)

    VToolCubicBezier(const VToolCubicBezierInitData &initData, QGraphicsItem *parent = nullptr);

    void SetSplineAttributes(QDomElement &domElement, const VCubicBezier &spl);
};

#endif // VTOOLCUBICBEZIER_H
