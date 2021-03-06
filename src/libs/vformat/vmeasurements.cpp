/************************************************************************
 **
 **  @file   vmeasurements.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   14 7, 2015
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2015 Valentina project
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

#include "vmeasurements.h"

#include <qnumeric.h>
#include <QDate>
#include <QDomNode>
#include <QDomNodeList>
#include <QDomText>
#include <QLatin1Char>
#include <QMessageLogger>
#include <QScopedPointer>
#include <QSet>
#include <QStaticStringData>
#include <QStringData>
#include <QStringDataPtr>
#include <QtDebug>
#include <QGlobalStatic>

#include "../ifc/exception/vexceptionemptyparameter.h"
#include "../ifc/xml/vvitconverter.h"
#include "../ifc/xml/vvstconverter.h"
#include "../ifc/ifcdef.h"
#include "../qmuparser/qmutokenparser.h"
#include "../qmuparser/qmuparsererror.h"
#include "../vpatterndb/calculator.h"
#include "../vpatterndb/variables/vmeasurement.h"
#include "../vpatterndb/vcontainer.h"
#include "../vpatterndb/measurements.h"
#include "../vpatterndb/pmsystems.h"
#include "../vmisc/projectversion.h"

const QString VMeasurements::TagVST              = QStringLiteral("vst");
const QString VMeasurements::TagVIT              = QStringLiteral("vit");
const QString VMeasurements::TagBodyMeasurements = QStringLiteral("body-measurements");
const QString VMeasurements::TagNotes            = QStringLiteral("notes");
const QString VMeasurements::TagSize             = QStringLiteral("size");
const QString VMeasurements::TagHeight           = QStringLiteral("height");
const QString VMeasurements::TagPersonal         = QStringLiteral("personal");
const QString VMeasurements::TagCustomer         = QStringLiteral("customer");
const QString VMeasurements::TagBirthDate        = QStringLiteral("birth-date");
const QString VMeasurements::TagGender           = QStringLiteral("gender");
const QString VMeasurements::TagPMSystem         = QStringLiteral("pm_system");
const QString VMeasurements::TagEmail            = QStringLiteral("email");
const QString VMeasurements::TagReadOnly         = QStringLiteral("read-only");
const QString VMeasurements::TagMeasurement      = QStringLiteral("m");

const QString VMeasurements::AttrBase           = QStringLiteral("base");
const QString VMeasurements::AttrValue          = QStringLiteral("value");
const QString VMeasurements::AttrSizeIncrease   = QStringLiteral("size_increase");
const QString VMeasurements::AttrHeightIncrease = QStringLiteral("height_increase");
const QString VMeasurements::AttrDescription    = QStringLiteral("description");
const QString VMeasurements::AttrName           = QStringLiteral("name");
const QString VMeasurements::AttrFullName       = QStringLiteral("full_name");

const QString VMeasurements::GenderMale    = QStringLiteral("male");
const QString VMeasurements::GenderFemale  = QStringLiteral("female");
const QString VMeasurements::GenderUnknown = QStringLiteral("unknown");

namespace
{
Q_GLOBAL_STATIC_WITH_ARGS(const QString, defBirthDate, (QLatin1String("1800-01-01")))

//---------------------------------------------------------------------------------------------------------------------
QString FileComment()
{
    return QString("Measurements created with Valentina v%1 (https://valentinaproject.bitbucket.io/).")
            .arg(APP_VERSION_STR);
}
}

//---------------------------------------------------------------------------------------------------------------------
VMeasurements::VMeasurements(VContainer *data)
    :VDomDocument(),
      data(data),
      type(MeasurementsType::Unknown)
{
    SCASSERT(data != nullptr)
}

//---------------------------------------------------------------------------------------------------------------------
VMeasurements::VMeasurements(Unit unit, VContainer *data)
    :VDomDocument(),
      data(data),
      type(MeasurementsType::Individual)
{
    SCASSERT(data != nullptr)

    CreateEmptyIndividualFile(unit);
}

//---------------------------------------------------------------------------------------------------------------------
VMeasurements::VMeasurements(Unit unit, int baseSize, int baseHeight, VContainer *data)
    :VDomDocument(),
      data(data),
      type(MeasurementsType::Multisize)
{
    SCASSERT(data != nullptr)

    CreateEmptyMultisizeFile(unit, baseSize, baseHeight);
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurements::setXMLContent(const QString &fileName)
{
    VDomDocument::setXMLContent(fileName);
    type = ReadType();
}

//---------------------------------------------------------------------------------------------------------------------
bool VMeasurements::SaveDocument(const QString &fileName, QString &error)
{
    // Update comment with Valentina version
    QDomNode commentNode = documentElement().firstChild();
    if (commentNode.isComment())
    {
        QDomComment comment = commentNode.toComment();
        comment.setData(FileComment());
    }

    return VDomDocument::SaveDocument(fileName, error);
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurements::AddEmpty(const QString &name, const QString &formula)
{
    const QDomElement element = MakeEmpty(name, formula);

    const QDomNodeList list = elementsByTagName(TagBodyMeasurements);
    list.at(0).appendChild(element);
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurements::AddEmptyAfter(const QString &after, const QString &name, const QString &formula)
{
    const QDomElement element = MakeEmpty(name, formula);
    const QDomElement sibling = FindM(after);

    const QDomNodeList list = elementsByTagName(TagBodyMeasurements);

    if (sibling.isNull())
    {
        list.at(0).appendChild(element);
    }
    else
    {
        list.at(0).insertAfter(element, sibling);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurements::Remove(const QString &name)
{
    const QDomNodeList list = elementsByTagName(TagBodyMeasurements);
    list.at(0).removeChild(FindM(name));
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurements::MoveTop(const QString &name)
{
    const QDomElement node = FindM(name);
    if (not node.isNull())
    {
        const QDomNodeList mList = elementsByTagName(TagMeasurement);
        if (mList.size() >= 2)
        {
            const QDomNode top = mList.at(0);
            if (not top.isNull())
            {
                const QDomNodeList list = elementsByTagName(TagBodyMeasurements);
                list.at(0).insertBefore(node, top);
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurements::MoveUp(const QString &name)
{
    const QDomElement node = FindM(name);
    if (not node.isNull())
    {
        const QDomElement prSibling = node.previousSiblingElement(TagMeasurement);
        if (not prSibling.isNull())
        {
            const QDomNodeList list = elementsByTagName(TagBodyMeasurements);
            list.at(0).insertBefore(node, prSibling);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurements::MoveDown(const QString &name)
{
    const QDomElement node = FindM(name);
    if (not node.isNull())
    {
        const QDomElement nextSibling = node.nextSiblingElement(TagMeasurement);
        if (not nextSibling.isNull())
        {
            const QDomNodeList list = elementsByTagName(TagBodyMeasurements);
            list.at(0).insertAfter(node, nextSibling);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurements::MoveBottom(const QString &name)
{
    const QDomElement node = FindM(name);
    if (not node.isNull())
    {
        const QDomNodeList mList = elementsByTagName(TagMeasurement);
        if (mList.size() >= 2)
        {
            const QDomNode bottom = mList.at(mList.size()-1);
            if (not bottom.isNull())
            {
                const QDomNodeList list = elementsByTagName(TagBodyMeasurements);
                list.at(0).insertAfter(node, bottom);
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurements::StoreNames(bool store)
{
    m_keepNames = store;
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurements::ReadMeasurements(qreal height, qreal size) const
{
    // For conversion values we must first calculate all data in measurement file's unit.
    // That's why we need two containers: one for converted values, second for real data.

    // Container for values in measurement file's unit
    QScopedPointer<VContainer> tempData(new VContainer(data->GetTrVars(), data->GetPatternUnit(),
                                                       VContainer::UniqueNamespace()));

    const QDomNodeList list = elementsByTagName(TagMeasurement);
    for (int i=0; i < list.size(); ++i)
    {
        const QDomElement dom = list.at(i).toElement();

        const QString name = GetParametrString(dom, AttrName).simplified();
        const QString description = GetParametrEmptyString(dom, AttrDescription);
        const QString fullName = GetParametrEmptyString(dom, AttrFullName);

        QSharedPointer<VMeasurement> meash;
        QSharedPointer<VMeasurement> tempMeash;
        if (type == MeasurementsType::Multisize)
        {
            qreal base = GetParametrDouble(dom, AttrBase, QChar('0'));
            qreal ksize = GetParametrDouble(dom, AttrSizeIncrease, QChar('0'));
            qreal kheight = GetParametrDouble(dom, AttrHeightIncrease, QChar('0'));

            tempMeash = QSharedPointer<VMeasurement>(new VMeasurement(static_cast<quint32>(i), name, BaseSize(),
                                                                      BaseHeight(), base, ksize, kheight));
            tempMeash->SetSize(size);
            tempMeash->SetHeight(height);
            tempMeash->SetUnit(data->GetPatternUnit());

            base = UnitConvertor(base, MUnit(), *data->GetPatternUnit());
            ksize = UnitConvertor(ksize, MUnit(), *data->GetPatternUnit());
            kheight = UnitConvertor(kheight, MUnit(), *data->GetPatternUnit());

            const qreal baseSize = UnitConvertor(BaseSize(), MUnit(), *data->GetPatternUnit());
            const qreal baseHeight = UnitConvertor(BaseHeight(), MUnit(), *data->GetPatternUnit());

            meash = QSharedPointer<VMeasurement>(new VMeasurement(static_cast<quint32>(i), name, baseSize, baseHeight,
                                                                  base, ksize, kheight, fullName, description));
            meash->SetSize(size);
            meash->SetHeight(height);
            meash->SetUnit(data->GetPatternUnit());
        }
        else
        {
            const QString formula = GetParametrString(dom, AttrValue, QChar('0'));
            bool ok = false;
            qreal value = EvalFormula(tempData.data(), formula, &ok);

            tempMeash = QSharedPointer<VMeasurement>(new VMeasurement(tempData.data(), static_cast<quint32>(i), name,
                                                                      value, formula, ok));

            value = UnitConvertor(value, MUnit(), *data->GetPatternUnit());
            meash = QSharedPointer<VMeasurement>(new VMeasurement(data, static_cast<quint32>(i), name, value, formula,
                                                                  ok, fullName, description));
        }
        if (m_keepNames)
        {
            tempData->AddUniqueVariable(tempMeash);
            data->AddUniqueVariable(meash);
        }
        else
        {
            tempData->AddVariable(tempMeash);
            data->AddVariable(meash);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurements::ClearForExport()
{
    const QDomNodeList list = elementsByTagName(TagMeasurement);

    for (int i=0; i < list.size(); ++i)
    {
        QDomElement domElement = list.at(i).toElement();
        if (domElement.isNull() == false)
        {
            if (qmu::QmuTokenParser::IsSingle(domElement.attribute(AttrValue)))
            {
                SetAttribute(domElement, AttrValue, QChar('0'));
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
MeasurementsType VMeasurements::Type() const
{
    return type;
}

//---------------------------------------------------------------------------------------------------------------------
int VMeasurements::BaseSize() const
{
    if (type == MeasurementsType::Multisize)
    {
        return static_cast<int>(UniqueTagAttr(TagSize, AttrBase, 50));
    }
    else
    {
        return 0;
    }
}

//---------------------------------------------------------------------------------------------------------------------
int VMeasurements::BaseHeight() const
{
    if (type == MeasurementsType::Multisize)
    {
        return static_cast<int>(UniqueTagAttr(TagHeight, AttrBase, 176));
    }
    else
    {
        return 0;
    }
}

//---------------------------------------------------------------------------------------------------------------------
QString VMeasurements::Notes() const
{
    return UniqueTagText(TagNotes, QString());
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurements::SetNotes(const QString &text)
{
    if (not IsReadOnly())
    {
        setTagText(TagNotes, text);
    }
}

//---------------------------------------------------------------------------------------------------------------------
QString VMeasurements::Customer() const
{
    return UniqueTagText(TagCustomer, QString());
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurements::SetCustomer(const QString &text)
{
    if (not IsReadOnly())
    {
        setTagText(TagCustomer, text);
    }
}

//---------------------------------------------------------------------------------------------------------------------
QDate VMeasurements::BirthDate() const
{
    return QDate::fromString(UniqueTagText(TagBirthDate, *defBirthDate), "yyyy-MM-dd");
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurements::SetBirthDate(const QDate &date)
{
    if (not IsReadOnly())
    {
        setTagText(TagBirthDate, date.toString("yyyy-MM-dd"));
    }
}

//---------------------------------------------------------------------------------------------------------------------
GenderType VMeasurements::Gender() const
{
    return StrToGender(UniqueTagText(TagGender, GenderUnknown));
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurements::SetGender(const GenderType &gender)
{
    if (not IsReadOnly())
    {
        setTagText(TagGender, GenderToStr(gender));
    }
}

//---------------------------------------------------------------------------------------------------------------------
QString VMeasurements::PMSystem() const
{
    return UniqueTagText(TagPMSystem, ClearPMCode(p998_S));
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurements::SetPMSystem(const QString &system)
{
    if (not IsReadOnly())
    {
        setTagText(TagPMSystem, ClearPMCode(system));
    }
}

//---------------------------------------------------------------------------------------------------------------------
QString VMeasurements::Email() const
{
    return UniqueTagText(TagEmail, QString());
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurements::SetEmail(const QString &text)
{
    if (not IsReadOnly())
    {
        setTagText(TagEmail, text);
    }
}

//---------------------------------------------------------------------------------------------------------------------
bool VMeasurements::IsReadOnly() const
{
    return UniqueTagText(TagReadOnly, falseStr) == trueStr;
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurements::SetReadOnly(bool ro)
{
    setTagText(TagReadOnly, ro ? trueStr : falseStr);
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurements::SetMName(const QString &name, const QString &text)
{
    QDomElement node = FindM(name);
    if (not node.isNull())
    {
        SetAttribute(node, AttrName, text);
    }
    else
    {
        qWarning() << tr("Can't find measurement '%1'").arg(name);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurements::SetMValue(const QString &name, const QString &text)
{
    QDomElement node = FindM(name);
    if (not node.isNull())
    {
        SetAttribute(node, AttrValue, text);
    }
    else
    {
        qWarning() << tr("Can't find measurement '%1'").arg(name);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurements::SetMBaseValue(const QString &name, double value)
{
    QDomElement node = FindM(name);
    if (not node.isNull())
    {
        SetAttribute(node, AttrBase, value);
    }
    else
    {
        qWarning() << tr("Can't find measurement '%1'").arg(name);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurements::SetMSizeIncrease(const QString &name, double value)
{
    QDomElement node = FindM(name);
    if (not node.isNull())
    {
        SetAttribute(node, AttrSizeIncrease, value);
    }
    else
    {
        qWarning() << tr("Can't find measurement '%1'").arg(name);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurements::SetMHeightIncrease(const QString &name, double value)
{
    QDomElement node = FindM(name);
    if (not node.isNull())
    {
        SetAttribute(node, AttrHeightIncrease, value);
    }
    else
    {
        qWarning() << tr("Can't find measurement '%1'").arg(name);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurements::SetMDescription(const QString &name, const QString &text)
{
    QDomElement node = FindM(name);
    if (not node.isNull())
    {
        SetAttribute(node, AttrDescription, text);
    }
    else
    {
        qWarning() << tr("Can't find measurement '%1'").arg(name);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurements::SetMFullName(const QString &name, const QString &text)
{
    QDomElement node = FindM(name);
    if (not node.isNull())
    {
        SetAttribute(node, AttrFullName, text);
    }
    else
    {
        qWarning() << tr("Can't find measurement '%1'").arg(name);
    }
}

//---------------------------------------------------------------------------------------------------------------------
QString VMeasurements::GenderToStr(const GenderType &sex)
{
    switch (sex)
    {
        case GenderType::Male:
            return GenderMale;
        case GenderType::Female:
            return GenderFemale;
        case GenderType::Unknown:
        default:
            return GenderUnknown;
    }
}

//---------------------------------------------------------------------------------------------------------------------
GenderType VMeasurements::StrToGender(const QString &sex)
{
    const QStringList genders = QStringList() << GenderMale << GenderFemale << GenderUnknown;
    switch (genders.indexOf(sex))
    {
        case 0: // GenderMale
            return GenderType::Male;
        case 1: // GenderFemale
            return GenderType::Female;
        case 2: // GenderUnknown
        default:
            return GenderType::Unknown;
    }
}

//---------------------------------------------------------------------------------------------------------------------
QStringList VMeasurements::ListAll() const
{
    QStringList listNames;
    const QDomNodeList list = elementsByTagName(TagMeasurement);

    for (int i=0; i < list.size(); ++i)
    {
        const QDomElement domElement = list.at(i).toElement();
        if (domElement.isNull() == false)
        {
            listNames.append(domElement.attribute(AttrName));
        }
    }

    return listNames;
}

//---------------------------------------------------------------------------------------------------------------------
QStringList VMeasurements::ListKnown() const
{
    QStringList listNames;
    const QStringList list = ListAll();
    for (int i=0; i < list.size(); ++i)
    {
        if (list.at(i).indexOf(CustomMSign) != 0)
        {
            listNames.append(list.at(i));
        }
    }

    return listNames;
}

//---------------------------------------------------------------------------------------------------------------------
bool VMeasurements::IsDefinedKnownNamesValid() const
{
    QStringList names = AllGroupNames();

    QSet<QString> set;
    for (const auto &var : names)
    {
        set.insert(var);
    }

    names = ListKnown();
    for (const auto &var : qAsConst(names))
    {
        if (not set.contains(var))
        {
            return false;
        }
    }

    return true;
}

//---------------------------------------------------------------------------------------------------------------------
VContainer *VMeasurements::GetData() const
{
    return data;
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurements::CreateEmptyMultisizeFile(Unit unit, int baseSize, int baseHeight)
{
    this->clear();
    QDomElement mElement = this->createElement(TagVST);

    mElement.appendChild(createComment(FileComment()));
    mElement.appendChild(CreateElementWithText(TagVersion, VVSTConverter::MeasurementMaxVerStr));
    mElement.appendChild(CreateElementWithText(TagReadOnly, falseStr));
    mElement.appendChild(createElement(TagNotes));
    mElement.appendChild(CreateElementWithText(TagUnit, UnitsToStr(unit)));
    mElement.appendChild(CreateElementWithText(TagPMSystem, ClearPMCode(p998_S)));

    QDomElement size = createElement(TagSize);
    SetAttribute(size, AttrBase, QString().setNum(baseSize));
    mElement.appendChild(size);

    QDomElement height = createElement(TagHeight);
    SetAttribute(height, AttrBase, QString().setNum(baseHeight));
    mElement.appendChild(height);

    mElement.appendChild(createElement(TagBodyMeasurements));

    this->appendChild(mElement);
    insertBefore(createProcessingInstruction(QStringLiteral("xml"),
                                             QStringLiteral("version=\"1.0\" encoding=\"UTF-8\"")), this->firstChild());
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurements::CreateEmptyIndividualFile(Unit unit)
{
    this->clear();
    QDomElement mElement = this->createElement(TagVIT);

    mElement.appendChild(createComment(FileComment()));
    mElement.appendChild(CreateElementWithText(TagVersion, VVITConverter::MeasurementMaxVerStr));
    mElement.appendChild(CreateElementWithText(TagReadOnly, falseStr));
    mElement.appendChild(createElement(TagNotes));
    mElement.appendChild(CreateElementWithText(TagUnit, UnitsToStr(unit)));
    mElement.appendChild(CreateElementWithText(TagPMSystem, ClearPMCode(p998_S)));

    QDomElement personal = createElement(TagPersonal);
    personal.appendChild(createElement(TagCustomer));
    personal.appendChild(CreateElementWithText(TagBirthDate, *defBirthDate));
    personal.appendChild(CreateElementWithText(TagGender, GenderToStr(GenderType::Unknown)));

    personal.appendChild(createElement(TagEmail));
    mElement.appendChild(personal);

    mElement.appendChild(createElement(TagBodyMeasurements));

    this->appendChild(mElement);
    insertBefore(createProcessingInstruction(QStringLiteral("xml"),
                                             QStringLiteral("version=\"1.0\" encoding=\"UTF-8\"")), this->firstChild());
}

//---------------------------------------------------------------------------------------------------------------------
qreal VMeasurements::UniqueTagAttr(const QString &tag, const QString &attr, qreal defValue) const
{
    const qreal defVal = UnitConvertor(defValue, Unit::Cm, MUnit());

    const QDomNodeList nodeList = this->elementsByTagName(tag);
    if (nodeList.isEmpty())
    {
        return defVal;
    }
    else
    {
        const QDomNode domNode = nodeList.at(0);
        if (domNode.isNull() == false && domNode.isElement())
        {
            const QDomElement domElement = domNode.toElement();
            if (domElement.isNull() == false)
            {
                return GetParametrDouble(domElement, attr, QString("%1").arg(defVal));
            }
        }
    }
    return defVal;
}

//---------------------------------------------------------------------------------------------------------------------
QDomElement VMeasurements::MakeEmpty(const QString &name, const QString &formula)
{
    QDomElement element = createElement(TagMeasurement);

    SetAttribute(element, AttrName, name);

    if (type == MeasurementsType::Multisize)
    {
        SetAttribute(element, AttrBase, QChar('0'));
        SetAttribute(element, AttrSizeIncrease, QChar('0'));
        SetAttribute(element, AttrHeightIncrease, QChar('0'));
    }
    else
    {
        SetAttribute(element, AttrValue, formula.isEmpty() ? QChar('0') : formula);
    }

    return element;
}

//---------------------------------------------------------------------------------------------------------------------
QDomElement VMeasurements::FindM(const QString &name) const
{
    if (name.isEmpty())
    {
        qWarning() << tr("The measurement name is empty!");
        return QDomElement();
    }

    QDomNodeList list = elementsByTagName(TagMeasurement);

    for (int i=0; i < list.size(); ++i)
    {
        const QDomElement domElement = list.at(i).toElement();
        if (domElement.isNull() == false)
        {
            const QString parameter = domElement.attribute(AttrName);
            if (parameter == name)
            {
                return domElement;
            }
        }
    }

    return QDomElement();
}

//---------------------------------------------------------------------------------------------------------------------
MeasurementsType VMeasurements::ReadType() const
{
    QDomElement root = documentElement();
    if (root.tagName() == TagVST)
    {
        return MeasurementsType::Multisize;
    }
    else if (root.tagName() == TagVIT)
    {
        return MeasurementsType::Individual;
    }
    else
    {
        return MeasurementsType::Unknown;
    }
}

//---------------------------------------------------------------------------------------------------------------------
qreal VMeasurements::EvalFormula(VContainer *data, const QString &formula, bool *ok) const
{
    if (formula.isEmpty())
    {
        *ok = true;
        return 0;
    }
    else
    {
        try
        {
            QScopedPointer<Calculator> cal(new Calculator());
            const qreal result = cal->EvalFormula(data->DataVariables(), formula);

            (qIsInf(result) || qIsNaN(result)) ? *ok = false : *ok = true;
            return result;
        }
        catch (qmu::QmuParserError &e)
        {
            Q_UNUSED(e)
            *ok = false;
            return 0;
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
QString VMeasurements::ClearPMCode(const QString &code) const
{
    QString clear = code;
    const int index = clear.indexOf(QLatin1Char('p'));
    if (index == 0)
    {
        clear.remove(0, 1);
    }
    return clear;
}
