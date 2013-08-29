#include "vcontainer.h"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <QDebug>
#pragma GCC diagnostic pop
#include "../options.h"

qint64 VContainer::_id = 0;

VContainer::VContainer():base(QMap<QString, qint32>()), points(QMap<qint64, VPointF>()),
    modelingPoints(QMap<qint64, VPointF>()),
    standartTable(QMap<QString, VStandartTableCell>()), incrementTable(QMap<QString, VIncrementTableRow>()),
    lengthLines(QMap<QString, qreal>()), lineAngles(QMap<QString, qreal>()), splines(QMap<qint64, VSpline>()),
    modelingSplines(QMap<qint64, VSpline>()),
    lengthSplines(QMap<QString, qreal>()), arcs(QMap<qint64, VArc>()), modelingArcs(QMap<qint64, VArc>()),
    lengthArcs(QMap<QString, qreal>()),
    splinePaths(QMap<qint64, VSplinePath>()), modelingSplinePaths(QMap<qint64, VSplinePath>()),
    details(QMap<qint64, VDetail>()){
    SetSize(500);
    SetGrowth(1760);
    CreateManTableIGroup ();
}

const VContainer &VContainer::operator =(const VContainer &data){
    setData(data);
    return *this;
}

VContainer::VContainer(const VContainer &data):base(QMap<QString, qint32>()), points(QMap<qint64, VPointF>()),
    modelingPoints(QMap<qint64, VPointF>()),
    standartTable(QMap<QString, VStandartTableCell>()), incrementTable(QMap<QString, VIncrementTableRow>()),
    lengthLines(QMap<QString, qreal>()), lineAngles(QMap<QString, qreal>()), splines(QMap<qint64, VSpline>()),
    modelingSplines(QMap<qint64, VSpline>()),
    lengthSplines(QMap<QString, qreal>()), arcs(QMap<qint64, VArc>()), modelingArcs(QMap<qint64, VArc>()),
    lengthArcs(QMap<QString, qreal>()),
    splinePaths(QMap<qint64, VSplinePath>()), modelingSplinePaths(QMap<qint64, VSplinePath>()),
    details(QMap<qint64, VDetail>()){
    setData(data);
}

void VContainer::setData(const VContainer &data){
    base = *data.DataBase();
    points = *data.DataPoints();
    modelingPoints = *data.DataModelingPoints();
    standartTable = *data.DataStandartTable();
    incrementTable = *data.DataIncrementTable();
    lengthLines = *data.DataLengthLines();
    lineAngles = *data.DataLengthArcs();
    splines = *data.DataSplines();
    modelingSplines = *data.DataModelingSplines();
    lengthSplines = *data.DataLengthSplines();
    arcs = *data.DataArcs();
    modelingArcs = *data.DataModelingArcs();
    lengthArcs = *data.DataLengthArcs();
    splinePaths = *data.DataSplinePaths();
    modelingSplinePaths = *data.DataModelingSplinePaths();
    details = *data.DataDetails();
}

template <typename key, typename val>
val VContainer::GetObject(const QMap<key,val> &obj, key id)const{
    if(obj.contains(id)){
        return obj.value(id);
    } else {
        qCritical()<<"Не можу знайти key = "<<id<<" в таблиці.";
        throw"Не можу знайти об'єкт за ключем.";
    }
}


VPointF VContainer::GetPoint(qint64 id) const{
    return GetObject(points, id);
}

VPointF VContainer::GetModelingPoint(qint64 id) const{
    return GetObject(modelingPoints, id);
}

VStandartTableCell VContainer::GetStandartTableCell(const QString &name) const{
    Q_ASSERT(!name.isEmpty());
    return GetObject(standartTable, name);
}

VIncrementTableRow VContainer::GetIncrementTableRow(const QString& name) const{
    Q_ASSERT(!name.isEmpty());
    return GetObject(incrementTable, name);
}

qreal VContainer::GetLine(const QString &name) const{
    Q_ASSERT(!name.isEmpty());
    return GetObject(lengthLines, name);
}

qreal VContainer::GetLineArc(const QString &name) const{
    Q_ASSERT(!name.isEmpty());
    return GetObject(lineAngles, name);
}

VSpline VContainer::GetSpline(qint64 id) const{
    return GetObject(splines, id);
}

VSpline VContainer::GetModelingSpline(qint64 id) const{
    return GetObject(modelingSplines, id);
}

VArc VContainer::GetArc(qint64 id) const{
    return GetObject(arcs, id);
}

VArc VContainer::GetModelingArc(qint64 id) const{
    return GetObject(modelingArcs, id);
}

VSplinePath VContainer::GetSplinePath(qint64 id) const{
    return GetObject(splinePaths, id);
}

VSplinePath VContainer::GetModelingSplinePath(qint64 id) const{
    return GetObject(modelingSplinePaths, id);
}

VDetail VContainer::GetDetail(qint64 id) const{
    return GetObject(details, id);
}

void VContainer::AddStandartTableCell(const QString& name, const VStandartTableCell& cell){
    standartTable[name] = cell;
}

void VContainer::AddIncrementTableRow(const QString& name, const VIncrementTableRow& cell){
    incrementTable[name] = cell;
}

qint64 VContainer::getId(){
    return _id;
}

qint64 VContainer::getNextId(){
    ++_id;
    return _id;
}

void VContainer::UpdateId(qint64 newId){
    if(newId > _id){
        _id = newId;
    }
}

void VContainer::IncrementReferens(qint64 id, Scene::Type obj){
    switch( obj ){
    case(Scene::Line):
        break;
    case(Scene::Point):{
        VPointF point = GetPoint(id);
        point.incrementReferens();
        UpdatePoint(id, point);
    }
        break;
    case(Scene::Arc):{
        VArc arc = GetArc(id);
        arc.incrementReferens();
        UpdateArc(id, arc);
    }
        break;
    case(Scene::Spline):{
        VSpline spl = GetSpline(id);
        spl.incrementReferens();
        UpdateSpline(id, spl);
    }
        break;
    case(Scene::SplinePath):{
        VSplinePath splPath = GetSplinePath(id);
        splPath.incrementReferens();
        UpdateSplinePath(id, splPath);
    }
        break;
    }
}

QPainterPath VContainer::ContourPath(qint64 idDetail) const{
    VDetail detail = GetDetail(idDetail);
    QVector<QPointF> points;
    for(qint32 i = 0; i< detail.CountNode(); ++i){
        switch(detail[i].getTypeTool()){
        case(Scene::Line):
            break;
        case(Scene::Point):{
            VPointF point = GetModelingPoint(detail[i].getId());
            points.append(point.toQPointF());
        }
            break;
        case(Scene::Arc):{
            VArc arc = GetModelingArc(detail[i].getId());
            points << arc.GetPoints();
        }
            break;
        case(Scene::Spline):{
            VSpline spline = GetModelingSpline(detail[i].getId());
            points << spline.GetPoints();
        }
            break;
        case(Scene::SplinePath):{
            VSplinePath splinePath = GetModelingSplinePath(detail[i].getId());
            points << splinePath.GetPathPoints();
        }
            break;
        }
    }
    QPainterPath path;
    path.moveTo(points[0]);
    for (qint32 i = 1; i < points.count(); ++i){
        path.lineTo(points[i]);
    }
    path.lineTo(points[0]);
    return path;
}

void VContainer::PrepareDetails(QVector<VItem *> &list) const{
    QMapIterator<qint64, VDetail> iDetail(details);
    while (iDetail.hasNext()) {
        iDetail.next();
        list.append(new VItem(ContourPath(iDetail.key()), list.size()));
    }
}

void VContainer::RemoveIncrementTableRow(const QString& name){
    incrementTable.remove(name);
}

template <typename val>
void VContainer::UpdateObject(QMap<qint64, val> &obj, const qint64 &id, const val& point){
    obj[id] = point;
    UpdateId(id);
}

void VContainer::UpdatePoint(qint64 id, const VPointF& point){
    UpdateObject(points, id, point);
}

void VContainer::UpdateModelingPoint(qint64 id, const VPointF &point){
    UpdateObject(modelingPoints, id, point);
}

void VContainer::UpdateDetail(qint64 id, const VDetail &detail){
    UpdateObject(details, id, detail);
}

void VContainer::UpdateSpline(qint64 id, const VSpline &spl){
    UpdateObject(splines, id, spl);
}

void VContainer::UpdateModelingSpline(qint64 id, const VSpline &spl){
    UpdateObject(modelingSplines, id, spl);
}

void VContainer::UpdateSplinePath(qint64 id, const VSplinePath &splPath){
    UpdateObject(splinePaths, id, splPath);
}

void VContainer::UpdateModelingSplinePath(qint64 id, const VSplinePath &splPath){
    UpdateObject(modelingSplinePaths, id, splPath);
}

void VContainer::UpdateArc(qint64 id, const VArc &arc){
    UpdateObject(arcs, id, arc);
}

void VContainer::UpdateModelingArc(qint64 id, const VArc &arc){
    UpdateObject(modelingArcs, id, arc);
}

void VContainer::UpdateStandartTableCell(const QString& name, const VStandartTableCell& cell){
    standartTable[name] = cell;
}

void VContainer::UpdateIncrementTableRow(const QString& name, const VIncrementTableRow& cell){
    incrementTable[name] = cell;
}

void VContainer::AddLengthSpline(const QString &name, const qreal &value){
    Q_ASSERT(!name.isEmpty());
    lengthSplines[name] = value;
}

void VContainer::AddLengthArc(const qint64 &center, const qint64 &id){
    AddLengthArc(GetNameArc(center, id), GetArc(id).GetLength());
}

void VContainer::AddLengthArc(const QString &name, const qreal &value){
    Q_ASSERT(!name.isEmpty());
    lengthArcs[name] = value;
}

void VContainer::AddLineAngle(const QString &name, const qreal &value){
    Q_ASSERT(!name.isEmpty());
    lineAngles[name] = value;
}

qreal VContainer::GetValueStandartTableCell(const QString& name) const{
    VStandartTableCell cell =  GetStandartTableCell(name);
    qreal k_size 	= ( static_cast<qreal> (size()/10.0) - 50.0 ) / 2.0;
    qreal k_growth  = ( static_cast<qreal> (growth()/10.0) - 176.0 ) / 6.0;
    qreal value = cell.GetBase() + k_size*cell.GetKsize() 	+ k_growth*cell.GetKgrowth();
    return value;
}

qreal VContainer::GetValueIncrementTableRow(const QString& name) const{
    VIncrementTableRow cell =  GetIncrementTableRow(name);
    qreal k_size 	= ( static_cast<qreal> (size()/10.0) - 50.0 ) / 2.0;
    qreal k_growth  = ( static_cast<qreal> (growth()/10.0) - 176.0 ) / 6.0;
    qreal value = cell.getBase() + k_size*cell.getKsize() + k_growth*cell.getKgrowth();
    return value;
}

void VContainer::Clear(){
    _id = 0;
    standartTable.clear();
    incrementTable.clear();
    lengthLines.clear();
    lengthArcs.clear();
    lineAngles.clear();
    details.clear();
    modelingArcs.clear();
    modelingPoints.clear();
    modelingSplinePaths.clear();
    modelingSplines.clear();
    ClearObject();
    CreateManTableIGroup ();
}

void VContainer::ClearObject(){
    points.clear();
    splines.clear();
    arcs.clear();
    splinePaths.clear();
}

void VContainer::ClearIncrementTable(){
    incrementTable.clear();
}

void VContainer::ClearLengthLines(){
    lengthLines.clear();
}

void VContainer::ClearLengthSplines(){
    lengthSplines.clear();
}

void VContainer::ClearLengthArcs(){
    lengthArcs.clear();
}

void VContainer::ClearLineAngles(){
    lineAngles.clear();
}

void VContainer::SetSize(qint32 size){
    base["Сг"] = size;
}

void VContainer::SetGrowth(qint32 growth){
    base["Р"] = growth;
}

qint32 VContainer::size() const{
    return base.value("Сг");
}

qint32 VContainer::growth() const{
    return base.value("Р");
}

bool VContainer::IncrementTableContains(const QString& name){
    return incrementTable.contains(name);
}

qreal VContainer::FindVar(const QString &name, bool *ok)const{
    if(base.contains(name)){
        *ok = true;
        return base.value(name);
    }

    if(standartTable.contains(name)){
        *ok = true;
        return GetValueStandartTableCell(name);
    }
    if(incrementTable.contains(name)){
        *ok = true;
        return GetValueIncrementTableRow(name);
    }
    if(lengthLines.contains(name)){
        *ok = true;
        return lengthLines.value(name);
    }
    if(lengthArcs.contains(name)){
        *ok = true;
        return lengthArcs.value(name);
    }
    if(lineAngles.contains(name)){
        *ok = true;
        return lineAngles.value(name);
    }
    *ok = false;
    return 0;
}

const QMap<qint64, VPointF> *VContainer::DataPoints() const{
    return &points;
}

const QMap<qint64, VPointF> *VContainer::DataModelingPoints() const{
    return &modelingPoints;
}

const QMap<qint64, VSpline> *VContainer::DataSplines() const{
    return &splines;
}

const QMap<qint64, VSpline> *VContainer::DataModelingSplines() const{
    return &modelingSplines;
}

const QMap<qint64, VArc> *VContainer::DataArcs() const{
    return &arcs;
}

const QMap<qint64, VArc> *VContainer::DataModelingArcs() const{
    return &modelingArcs;
}

const QMap<QString, qint32> *VContainer::DataBase() const{
    return &base;
}

const QMap<QString, VStandartTableCell> *VContainer::DataStandartTable() const{
    return &standartTable;
}

const QMap<QString, VIncrementTableRow> *VContainer::DataIncrementTable() const{
    return &incrementTable;
}

const QMap<QString, qreal> *VContainer::DataLengthLines() const{
    return &lengthLines;
}

const QMap<QString, qreal> *VContainer::DataLengthSplines() const{
    return &lengthSplines;
}

const QMap<QString, qreal> *VContainer::DataLengthArcs() const{
    return &lengthArcs;
}

const QMap<QString, qreal> *VContainer::DataLineAngles() const{
    return &lineAngles;
}

const QMap<qint64, VSplinePath> *VContainer::DataSplinePaths() const{
    return &splinePaths;
}

const QMap<qint64, VSplinePath> *VContainer::DataModelingSplinePaths() const{
    return &modelingSplinePaths;
}

const QMap<qint64, VDetail> *VContainer::DataDetails() const{
    return &details;
}

void VContainer::AddLine(const qint64 &firstPointId, const qint64 &secondPointId){
    QString nameLine = GetNameLine(firstPointId, secondPointId);
    VPointF firstPoint = GetPoint(firstPointId);
    VPointF secondPoint = GetPoint(secondPointId);
    AddLengthLine(nameLine, QLineF(firstPoint.toQPointF(), secondPoint.toQPointF()).length()/PrintDPI*25.4);
    nameLine = GetNameLineAngle(firstPointId, secondPointId);
    AddLineAngle(nameLine, QLineF(firstPoint.toQPointF(), secondPoint.toQPointF()).angle());
}

template <typename key, typename val>
qint64 VContainer::AddObject(QMap<key, val> &obj, const val& value){
    qint64 id = getNextId();
    obj[id] = value;
    return id;
}

qint64 VContainer::AddPoint(const VPointF& point){
    return AddObject(points, point);
}

qint64 VContainer::AddModelingPoint(const VPointF &point){
    return AddObject(modelingPoints, point);
}

qint64 VContainer::AddDetail(const VDetail &detail){
    return AddObject(details, detail);
}

qint64 VContainer::AddSpline(const VSpline &spl){
    return AddObject(splines, spl);
}

qint64 VContainer::AddModelingSpline(const VSpline &spl){
    return AddObject(modelingSplines, spl);
}

qint64 VContainer::AddSplinePath(const VSplinePath &splPath){
    return AddObject(splinePaths, splPath);
}

qint64 VContainer::AddModelingSplinePath(const VSplinePath &splPath){
    return AddObject(modelingSplinePaths, splPath);
}

qint64 VContainer::AddArc(const VArc &arc){
    return AddObject(arcs, arc);
}

qint64 VContainer::AddModelingArc(const VArc &arc){
    return AddObject(modelingArcs, arc);
}

QString VContainer::GetNameLine(const qint64 &firstPoint, const qint64 &secondPoint) const{
    VPointF first = GetPoint(firstPoint);
    VPointF second = GetPoint(secondPoint);
    return QString("Line_%1_%2").arg(first.name(), second.name());
}

QString VContainer::GetNameLineAngle(const qint64 &firstPoint, const qint64 &secondPoint) const{
    VPointF first = GetPoint(firstPoint);
    VPointF second = GetPoint(secondPoint);
    return QString("AngleLine_%1_%2").arg(first.name(), second.name());
}

QString VContainer::GetNameSpline(const qint64 &firstPoint, const qint64 &secondPoint) const{
    VPointF first = GetPoint(firstPoint);
    VPointF second = GetPoint(secondPoint);
    return QString("Spl_%1_%2").arg(first.name(), second.name());
}

QString VContainer::GetNameSplinePath(const VSplinePath &path) const{
    if(path.Count() == 0){
        return QString();
    }
    QString name("SplPath");
    for(qint32 i = 1; i <= path.Count(); ++i){
        VSpline spl = path.GetSpline(i);
        VPointF first = GetPoint(spl.GetP1());
        VPointF second = GetPoint(spl.GetP4());
        QString splName = QString("_%1_%2").arg(first.name(), second.name());
        name.append(splName);
    }
    return name;
}

QString VContainer::GetNameArc(const qint64 &center, const qint64 &id) const{
    VPointF centerPoint = GetPoint(center);
    return QString ("Arc(%1)%2").arg(centerPoint.name()).arg(id);
}

void VContainer::AddLengthLine(const QString &name, const qreal &value){
    Q_ASSERT(!name.isEmpty());
    lengthLines[name] = value;
}

void VContainer::AddLengthSpline(const qint64 &firstPointId, const qint64 &secondPointId){
    QString nameLine = GetNameSpline(firstPointId, secondPointId);
    VPointF firstPoint = GetPoint(firstPointId);
    VPointF secondPoint = GetPoint(secondPointId);
    AddLengthSpline(nameLine, QLineF(firstPoint.toQPointF(), secondPoint.toQPointF()).length());
}

void VContainer::CreateManTableIGroup (){
    AddStandartTableCell("Pkor", VStandartTableCell(84, 0, 3));
    AddStandartTableCell("Pkor", VStandartTableCell(84, 0, 3));
    AddStandartTableCell("Vtos", VStandartTableCell(1450, 2, 51));
    AddStandartTableCell("Vtosh", VStandartTableCell(1506, 2, 54));
    AddStandartTableCell("Vpt", VStandartTableCell(1438, 3, 52));
    AddStandartTableCell("Vst", VStandartTableCell(1257, -1, 49));
    AddStandartTableCell("Vlt", VStandartTableCell(1102, 0, 43));
    AddStandartTableCell("Vk", VStandartTableCell(503, 0, 22));
    AddStandartTableCell("Vsht", VStandartTableCell(1522, 2, 54));
    AddStandartTableCell("Vzy", VStandartTableCell(1328, 0, 49));
    AddStandartTableCell("Vlop", VStandartTableCell(1320, 0, 49));
    AddStandartTableCell("Vps", VStandartTableCell(811, -1, 36));
    AddStandartTableCell("Osh", VStandartTableCell(404,8, 2));
    AddStandartTableCell("OgI", VStandartTableCell(1034, 36, 4));
    AddStandartTableCell("OgII", VStandartTableCell(1044, 38, 2));
    AddStandartTableCell("OgIII", VStandartTableCell(1000, 40, 0));
    AddStandartTableCell("Ot", VStandartTableCell(780, 40, 0));
    AddStandartTableCell("Ob", VStandartTableCell(984, 30, 10));
    AddStandartTableCell("ObI", VStandartTableCell(964, 24, 12));
    AddStandartTableCell("Obed", VStandartTableCell(566, 18, 6));
    AddStandartTableCell("Ok", VStandartTableCell(386, 8, 8));
    AddStandartTableCell("Oi", VStandartTableCell(380, 8, 6));
    AddStandartTableCell("Osch", VStandartTableCell(234, 4, 4));
    AddStandartTableCell("Os", VStandartTableCell(350, 2, 8));
    AddStandartTableCell("Dsb", VStandartTableCell(1120, 0, 44));
    AddStandartTableCell("Dsp", VStandartTableCell(1110, 0, 43));
    AddStandartTableCell("Dn", VStandartTableCell(826, -3, 37));
    AddStandartTableCell("Dps", VStandartTableCell(316, 4, 7));
    AddStandartTableCell("Dpob", VStandartTableCell(783, 14, 15));
    AddStandartTableCell("Ds", VStandartTableCell(260, 1, 6));
    AddStandartTableCell("Op", VStandartTableCell(316, 12, 0));
    AddStandartTableCell("Ozap", VStandartTableCell(180, 4, 0));
    AddStandartTableCell("Pkis", VStandartTableCell(250, 4, 0));
    AddStandartTableCell("SHp", VStandartTableCell(160, 1, 4));
    AddStandartTableCell("Dlych", VStandartTableCell(500, 2, 15));
    AddStandartTableCell("Dzap", VStandartTableCell(768, 2, 24));
    AddStandartTableCell("DIIIp", VStandartTableCell(970, 2, 29));
    AddStandartTableCell("Vprp", VStandartTableCell(214, 3, 3));
    AddStandartTableCell("Vg", VStandartTableCell(262, 8, 3));
    AddStandartTableCell("Dtp", VStandartTableCell(460, 7, 9));
    AddStandartTableCell("Dp", VStandartTableCell(355, 5, 5));
    AddStandartTableCell("Vprz", VStandartTableCell(208, 3, 5));
    AddStandartTableCell("Dts", VStandartTableCell(438, 2, 10));
    AddStandartTableCell("DtsI", VStandartTableCell(469, 2, 10));
    AddStandartTableCell("Dvcht", VStandartTableCell(929, 9, 19));
    AddStandartTableCell("SHg", VStandartTableCell(370, 14, 4));
    AddStandartTableCell("Cg", VStandartTableCell(224, 6, 0));
    AddStandartTableCell("SHs", VStandartTableCell(416, 10, 2));
    AddStandartTableCell("dpzr", VStandartTableCell(121, 6, 0));
    AddStandartTableCell("Ogol", VStandartTableCell(576, 4, 4));
    AddStandartTableCell("Ssh1", VStandartTableCell(205, 5, 0));
    AddStandartTableCell("St", VStandartTableCell(410, 20, 0));
    AddStandartTableCell("Drzap", VStandartTableCell(594, 3, 19));
    AddStandartTableCell("DbII", VStandartTableCell(1020, 0, 44));
    AddStandartTableCell("Sb", VStandartTableCell(504, 15, 4));
}
