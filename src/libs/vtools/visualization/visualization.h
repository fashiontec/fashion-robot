/************************************************************************
 **
 **  @file   visualization.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   15 8, 2014
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

#ifndef VISUALIZATION_H
#define VISUALIZATION_H

#include <qcompilerdetection.h>
#include <QGraphicsItem>
#include <QObject>
#include <QtGlobal>
#include <QLoggingCategory>

#include "../vmisc/def.h"
#include "../vmisc/vabstractapplication.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "../vwidgets/vcurvepathitem.h"
#include "../vwidgets/global.h"
#include "../vgeometry/vabstractcurve.h"

Q_DECLARE_LOGGING_CATEGORY(vVis)

class VScaledEllipse;
class VScaledLine;
class VContainer;
class VInternalVariable;

enum class Mode : qint8 {Creation, Show};

class Visualization : public QObject
{
    Q_OBJECT
public:
    explicit Visualization(const VContainer *data);
    virtual ~Visualization() Q_DECL_EQ_DEFAULT;

    virtual void RefreshGeometry()=0;

    void         setObject1Id(const quint32 &value);
    void         setLineStyle(const Qt::PenStyle &value);
    void         setScenePos(const QPointF &value);
    virtual void VisualMode(const quint32 &pointId);
    void         setMainColor(const QColor &value);

    const VContainer *GetData() const;
    void              SetData(const VContainer *data);

    Mode GetMode() const;
    void SetMode(const Mode &value);

    static qreal FindLengthFromUser(const QString &expression, const QHash<QString,
                                    QSharedPointer<VInternalVariable> > *vars, bool fromUser = true);
    static qreal FindValFromUser(const QString &expression, const QHash<QString,
                                 QSharedPointer<VInternalVariable> > *vars, bool fromUser = true);

    QString CurrentToolTip() const {return toolTip;}
signals:
    void         ToolTip(const QString &toolTip);
public slots:
    void         MousePos(const QPointF &scenePos);
protected:
    const VContainer *data;
    QPointF          scenePos;
    QColor           mainColor;
    QColor           supportColor;
    Qt::PenStyle     lineStyle;
    quint32          object1Id;
    QString          toolTip;
    Mode             mode;

    virtual void InitPen()=0;
    virtual void AddOnScene()=0;

    VScaledEllipse *InitPoint(const QColor &color, QGraphicsItem *parent, qreal z = 0) const;
    void         DrawPoint(QGraphicsEllipseItem *point, const QPointF &pos, const QColor &color,
                           Qt::PenStyle style = Qt::SolidLine);
    virtual void DrawLine(VScaledLine *lineItem, const QLineF &line, const QColor &color,
                          Qt::PenStyle style = Qt::SolidLine);
    void         DrawPath(VCurvePathItem *pathItem, const QPainterPath &path, const QColor &color,
                          Qt::PenStyle style = Qt::SolidLine, Qt::PenCapStyle cap = Qt::SquareCap);
    void         DrawPath(VCurvePathItem *pathItem, const QPainterPath &path,
                          const QVector<DirectionArrow> &directionArrows, const QColor &color,
                          Qt::PenStyle style = Qt::SolidLine, Qt::PenCapStyle cap = Qt::SquareCap);

    template <typename Item>
    void         AddItem(Item *item);

    template <class Item>
    Item         *InitItem(const QColor &color, QGraphicsItem *parent);

    static VScaledEllipse *GetPointItem(QVector<VScaledEllipse *> &points, quint32 i, const QColor &color,
                                        QGraphicsItem *parent);
    static VCurvePathItem *GetCurveItem(QVector<VCurvePathItem *> &curves, quint32 i, const QColor &color,
                                        QGraphicsItem *parent);
private:
    Q_DISABLE_COPY(Visualization)
};

//---------------------------------------------------------------------------------------------------------------------
template <typename Item>
inline void Visualization::AddItem(Item *item)
{
    SCASSERT(item != nullptr)
    VMainGraphicsScene *scene = qobject_cast<VMainGraphicsScene *>(qApp->getCurrentScene());
    SCASSERT(scene != nullptr)

    scene->addItem(item);
    connect(scene, &VMainGraphicsScene::mouseMove, item, &Visualization::MousePos);
}

//---------------------------------------------------------------------------------------------------------------------
template <class Item>
inline Item *Visualization::InitItem(const QColor &color, QGraphicsItem *parent)
{
    Item *item = new Item(parent);

    QPen visPen = item->pen();
    visPen.setColor(color);

    item->setPen(visPen);
    item->setZValue(1);
    item->setFlags(QGraphicsItem::ItemStacksBehindParent);
    item->setVisible(false);
    return item;
}

#endif // VISUALIZATION_H
