/************************************************************************
 **
 **  @file   vposition.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   20 1, 2015
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2015 Valentina project
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

#ifndef VPOSITION_H
#define VPOSITION_H

#include <qcompilerdetection.h>
#include <QRunnable>
#include <QVector>
#include <QtGlobal>
#include <atomic>

#include "vbestsquare.h"
#include "vcontour.h"
#include "vlayoutdef.h"
#include "vlayoutpiece.h"

struct VPositionData
{
    VContour gContour{};
    VLayoutPiece detail{};
    int i{-1};
    int j{-1};
    bool rotate{false};
    int rotationNumber{0};
    bool followGrainline{false};
};

class VPosition : public QRunnable
{
public:
    VPosition(const VPositionData &data, std::atomic_bool *stop, bool saveLength);
    virtual ~VPosition() override = default;

    virtual void run() override;

#ifdef LAYOUT_DEBUG
    quint32 getPaperIndex() const;
    void setPaperIndex(quint32 value);

    quint32 getFrame() const;
    void setFrame(quint32 value);

    quint32 getDetailsCount() const;
    void setDetailsCount(quint32 value);

    void setDetails(const QVector<VLayoutPiece> &details);
#endif

    VBestSquare getBestResult() const;

#ifdef LAYOUT_DEBUG
    static void DrawDebug(const VContour &contour, const VLayoutPiece &detail, int frame, quint32 paperIndex,
                          int detailsCount, const QVector<VLayoutPiece> &details = QVector<VLayoutPiece>());

    static int Bias(int length, int maxLength);
#endif

private:
    Q_DISABLE_COPY(VPosition)
    VBestSquare m_bestResult;
    VPositionData m_data;
#ifdef LAYOUT_DEBUG
    quint32 paperIndex;
    quint32 frame;
    quint32 detailsCount;
    QVector<VLayoutPiece> details;
#endif
    std::atomic_bool *stop;
    /**
     * @brief angle_between keep angle between global edge and detail edge. Need for optimization rotation.
     */
    qreal angle_between;

    enum class CrossingType : char
    {
        NoIntersection = 0,
        Intersection = 1,
        EdgeError = 2
    };

    enum class InsideType : char
    {
        Outside = 0,
        Inside = 1,
        EdgeError = 2
    };

    void SaveCandidate(VBestSquare &bestResult, const VLayoutPiece &detail, int globalI, int detJ, BestFrom type);

    bool CheckCombineEdges(VLayoutPiece &detail, int j, int &dEdge);
    bool CheckRotationEdges(VLayoutPiece &detail, int j, int dEdge, qreal angle) const;

    void RotateOnAngle(qreal angle);

    CrossingType Crossing(const VLayoutPiece &detail) const;
    bool         SheetContains(const QRectF &rect) const;

    void CombineEdges(VLayoutPiece &detail, const QLineF &globalEdge, int dEdge);
    void RotateEdges(VLayoutPiece &detail, const QLineF &globalEdge, int dEdge, qreal angle) const;

    void Rotate(int number);
    void FollowGrainline();

    QLineF FabricGrainline() const;

    void FindBestPosition();
};

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VPosition::FabricGrainline return fabric gainline accoding to paper orientation
 * @return fabric gainline line
 */
inline QLineF VPosition::FabricGrainline() const
{
    return m_data.gContour.GetHeight() >= m_data.gContour.GetWidth() ? QLineF(10, 10, 10, 100) :
                                                                       QLineF(10, 10, 100, 10);
}

#endif // VPOSITION_H
