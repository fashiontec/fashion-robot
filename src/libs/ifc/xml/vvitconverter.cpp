/************************************************************************
 **
 **  @file   vvitconverter.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   15 7, 2015
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

#include "vvitconverter.h"

#include <QDomNode>
#include <QDomNodeList>
#include <QDomText>
#include <QFile>
#include <QLatin1String>
#include <QList>
#include <QMap>
#include <QMultiMap>
#include <QStaticStringData>
#include <QStringData>
#include <QStringDataPtr>
#include <QGlobalStatic>

#include "../exception/vexception.h"
#include "../vmisc/def.h"
#include "vabstractmconverter.h"

/*
 * Version rules:
 * 1. Version have three parts "major.minor.patch";
 * 2. major part only for stable releases;
 * 3. minor - 10 or more patch changes, or one big change;
 * 4. patch - little change.
 */

const QString VVITConverter::MeasurementMinVerStr = QStringLiteral("0.2.0");
const QString VVITConverter::MeasurementMaxVerStr = QStringLiteral("0.5.0");
const QString VVITConverter::CurrentSchema        = QStringLiteral("://schema/individual_measurements/v0.5.0.xsd");

//VVITConverter::MeasurementMinVer; // <== DON'T FORGET TO UPDATE TOO!!!!
//VVITConverter::MeasurementMaxVer; // <== DON'T FORGET TO UPDATE TOO!!!!

namespace
{
// The list of all string we use for conversion
// Better to use global variables because repeating QStringLiteral blows up code size
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strTagRead_Only, (QLatin1String("read-only")))
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strGivenName, (QLatin1String("given-name")))
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strFamilyName, (QLatin1String("family-name")))
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strCustomer, (QLatin1String("customer")))
Q_GLOBAL_STATIC_WITH_ARGS(const QString, strPersonal, (QLatin1String("personal")))
}

//---------------------------------------------------------------------------------------------------------------------
VVITConverter::VVITConverter(const QString &fileName)
    :VAbstractMConverter(fileName)
{
    ValidateInputFile(CurrentSchema);
}

//---------------------------------------------------------------------------------------------------------------------
QString VVITConverter::XSDSchema(int ver) const
{
    QHash <int, QString> schemas =
    {
        std::make_pair(FORMAT_VERSION(0, 2, 0), QStringLiteral("://schema/individual_measurements/v0.2.0.xsd")),
        std::make_pair(FORMAT_VERSION(0, 3, 0), QStringLiteral("://schema/individual_measurements/v0.3.0.xsd")),
        std::make_pair(FORMAT_VERSION(0, 3, 1), QStringLiteral("://schema/individual_measurements/v0.3.1.xsd")),
        std::make_pair(FORMAT_VERSION(0, 3, 2), QStringLiteral("://schema/individual_measurements/v0.3.2.xsd")),
        std::make_pair(FORMAT_VERSION(0, 3, 3), QStringLiteral("://schema/individual_measurements/v0.3.3.xsd")),
        std::make_pair(FORMAT_VERSION(0, 4, 0), QStringLiteral("://schema/individual_measurements/v0.4.0.xsd")),
        std::make_pair(FORMAT_VERSION(0, 5, 0), CurrentSchema),
    };

    if (schemas.contains(ver))
    {
        return schemas.value(ver);
    }
    else
    {
        InvalidVersion(ver);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VVITConverter::ApplyPatches()
{
    switch (m_ver)
    {
        case (FORMAT_VERSION(0, 2, 0)):
            ToV0_3_0();
            ValidateXML(XSDSchema(FORMAT_VERSION(0, 3, 0)));
            Q_FALLTHROUGH();
        case (FORMAT_VERSION(0, 3, 0)):
            ToV0_3_1();
            ValidateXML(XSDSchema(FORMAT_VERSION(0, 3, 1)));
            Q_FALLTHROUGH();
        case (FORMAT_VERSION(0, 3, 1)):
            ToV0_3_2();
            ValidateXML(XSDSchema(FORMAT_VERSION(0, 3, 2)));
            Q_FALLTHROUGH();
        case (FORMAT_VERSION(0, 3, 2)):
            ToV0_3_3();
            ValidateXML(XSDSchema(FORMAT_VERSION(0, 3, 3)));
            Q_FALLTHROUGH();
        case (FORMAT_VERSION(0, 3, 3)):
            ToV0_4_0();
            ValidateXML(XSDSchema(FORMAT_VERSION(0, 4, 0)));
            Q_FALLTHROUGH();
        case (FORMAT_VERSION(0, 4, 0)):
            ToV0_5_0();
            ValidateXML(XSDSchema(FORMAT_VERSION(0, 5, 0)));
            Q_FALLTHROUGH();
        case (FORMAT_VERSION(0, 5, 0)):
            break;
        default:
            InvalidVersion(m_ver);
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VVITConverter::DowngradeToCurrentMaxVersion()
{
    SetVersion(MeasurementMaxVerStr);
    Save();
}

//---------------------------------------------------------------------------------------------------------------------
bool VVITConverter::IsReadOnly() const
{
    // Check if attribute read-only was not changed in file format
    Q_STATIC_ASSERT_X(VVITConverter::MeasurementMaxVer == FORMAT_VERSION(0, 5, 0),
                      "Check attribute read-only.");

    // Possibly in future attribute read-only will change position etc.
    // For now position is the same for all supported format versions.
    // But don't forget to keep all versions of attribute until we support that format versions

    return UniqueTagText(*strTagRead_Only, falseStr) == trueStr;
}

//---------------------------------------------------------------------------------------------------------------------
void VVITConverter::AddNewTagsForV0_3_0()
{
    // TODO. Delete if minimal supported version is 0.3.0
    Q_STATIC_ASSERT_X(VVITConverter::MeasurementMinVer < FORMAT_VERSION(0, 3, 0),
                      "Time to refactor the code.");

    QDomElement rootElement = this->documentElement();
    QDomNode refChild = rootElement.firstChildElement(QStringLiteral("version"));

    refChild = rootElement.insertAfter(CreateElementWithText(QStringLiteral("read-only"), falseStr), refChild);
    refChild = rootElement.insertAfter(createElement(QStringLiteral("notes")), refChild);

    rootElement.insertAfter(CreateElementWithText(QStringLiteral("unit"), MUnitV0_2_0()), refChild);
}

//---------------------------------------------------------------------------------------------------------------------
QString VVITConverter::MUnitV0_2_0()
{
    // TODO. Delete if minimal supported version is 0.3.0
    Q_STATIC_ASSERT_X(VVITConverter::MeasurementMinVer < FORMAT_VERSION(0, 3, 0),
                      "Time to refactor the code.");

    return UniqueTagText(QStringLiteral("unit"), QStringLiteral("cm"));
}

//---------------------------------------------------------------------------------------------------------------------
void VVITConverter::ConvertMeasurementsToV0_3_0()
{
    // TODO. Delete if minimal supported version is 0.3.0
    Q_STATIC_ASSERT_X(VVITConverter::MeasurementMinVer < FORMAT_VERSION(0, 3, 0),
                      "Time to refactor the code.");

    const QString tagBM = QStringLiteral("body-measurements");

    QDomElement bm = createElement(tagBM);

    const QMultiMap<QString, QString> names = OldNamesToNewNames_InV0_3_0();
    const QList<QString> keys = names.uniqueKeys();
    for (auto &key : keys)
    {
        qreal resValue = 0;

        // This has the same effect as a .values(), just isn't as elegant
        const QList<QString> list = names.values( key );
        for(const auto &val : list)
        {
            const QDomNodeList nodeList = this->elementsByTagName(val);
            if (nodeList.isEmpty())
            {
                continue;
            }

            const qreal value = GetParametrDouble(nodeList.at(0).toElement(), QStringLiteral("value"), "0.0");

            if (not qFuzzyIsNull(value))
            {
                resValue = value;
            }
        }

        bm.appendChild(AddMV0_3_0(key, resValue));
    }

    QDomElement rootElement = this->documentElement();
    const QDomNodeList listBM = elementsByTagName(tagBM);
    rootElement.replaceChild(bm, listBM.at(0));
}

//---------------------------------------------------------------------------------------------------------------------
QDomElement VVITConverter::AddMV0_3_0(const QString &name, qreal value)
{
    // TODO. Delete if minimal supported version is 0.3.0
    Q_STATIC_ASSERT_X(VVITConverter::MeasurementMinVer < FORMAT_VERSION(0, 3, 0),
                      "Time to refactor the code.");

    QDomElement element = createElement(QStringLiteral("m"));

    SetAttribute(element, QStringLiteral("name"), name);
    SetAttribute(element, QStringLiteral("value"), QString().setNum(value));
    SetAttribute(element, QStringLiteral("description"), QString());
    SetAttribute(element, QStringLiteral("full_name"), QString());

    return element;
}

//---------------------------------------------------------------------------------------------------------------------
void VVITConverter::GenderV0_3_1()
{
    // TODO. Delete if minimal supported version is 0.3.1
    Q_STATIC_ASSERT_X(VVITConverter::MeasurementMinVer < FORMAT_VERSION(0, 3, 1),
                      "Time to refactor the code.");

    const QDomNodeList nodeList = this->elementsByTagName(QStringLiteral("sex"));
    QDomElement sex = nodeList.at(0).toElement();
    QDomElement parent = sex.parentNode().toElement();
    parent.replaceChild(CreateElementWithText(QStringLiteral("gender"), sex.text()), sex);
}

//---------------------------------------------------------------------------------------------------------------------
void VVITConverter::PM_SystemV0_3_2()
{
    // TODO. Delete if minimal supported version is 0.3.2
    Q_STATIC_ASSERT_X(VVITConverter::MeasurementMinVer < FORMAT_VERSION(0, 3, 2),
                      "Time to refactor the code.");

    const QDomNodeList nodeList = this->elementsByTagName(QStringLiteral("personal"));
    QDomElement personal = nodeList.at(0).toElement();

    QDomElement parent = personal.parentNode().toElement();
    parent.insertBefore(CreateElementWithText(QStringLiteral("pm_system"), QStringLiteral("998")), personal);
}

//---------------------------------------------------------------------------------------------------------------------
void VVITConverter::ConvertMeasurementsToV0_3_3()
{
    // TODO. Delete if minimal supported version is 0.3.3
    Q_STATIC_ASSERT_X(VVITConverter::MeasurementMinVer < FORMAT_VERSION(0, 3, 3),
                      "Time to refactor the code.");

    const QMap<QString, QString> names = OldNamesToNewNames_InV0_3_3();
    auto i = names.constBegin();
    while (i != names.constEnd())
    {
        const QDomNodeList nodeList = this->elementsByTagName(QStringLiteral("m"));
        if (nodeList.isEmpty())
        {
            ++i;
            continue;
        }

        for (int ii = 0; ii < nodeList.size(); ++ii)
        {
            const QString attrName = QStringLiteral("name");
            QDomElement element = nodeList.at(ii).toElement();
            const QString name = GetParametrString(element, attrName);
            if (name == i.value())
            {
                SetAttribute(element, attrName, i.key());
            }
        }

        ++i;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VVITConverter::ConverCustomerNameToV0_4_0()
{
    // Find root tag
    const QDomNodeList personalList = this->elementsByTagName(*strPersonal);
    if (personalList.isEmpty())
    {
        return;
    }

    QDomNode personal = personalList.at(0);

    // Given name
    QString givenName;
    const QDomNodeList givenNameList = this->elementsByTagName(*strGivenName);
    if (not givenNameList.isEmpty())
    {
        QDomNode givenNameNode = givenNameList.at(0);
        givenName = givenNameNode.toElement().text();
        personal.removeChild(givenNameNode);
    }

    // Family name
    QString familyName;
    const QDomNodeList familyNameList = this->elementsByTagName(*strFamilyName);
    if (not familyNameList.isEmpty())
    {
        QDomNode familyNameNode = familyNameList.at(0);
        familyName = familyNameNode.toElement().text();
        personal.removeChild(familyNameNode);
    }

    QString customerName;
    if (not givenName.isEmpty() && not familyName.isEmpty())
    {
        customerName = givenName + QLatin1Char(' ') + familyName;
    }
    else if (not givenName.isEmpty() && familyName.isEmpty())
    {
        customerName = givenName;
    }
    else if (givenName.isEmpty() && not familyName.isEmpty())
    {
        customerName = familyName;
    }

    personal.insertBefore(CreateElementWithText(*strCustomer, not customerName.isEmpty() ? customerName : QString()),
                          personal.firstChild());
}

//---------------------------------------------------------------------------------------------------------------------
void VVITConverter::ToV0_3_0()
{
    // TODO. Delete if minimal supported version is 0.3.0
    Q_STATIC_ASSERT_X(VVITConverter::MeasurementMinVer < FORMAT_VERSION(0, 3, 0),
                      "Time to refactor the code.");

    AddRootComment();
    SetVersion(QStringLiteral("0.3.0"));
    AddNewTagsForV0_3_0();
    ConvertMeasurementsToV0_3_0();
    Save();
}

//---------------------------------------------------------------------------------------------------------------------
void VVITConverter::ToV0_3_1()
{
    // TODO. Delete if minimal supported version is 0.3.1
    Q_STATIC_ASSERT_X(VVITConverter::MeasurementMinVer < FORMAT_VERSION(0, 3, 1),
                      "Time to refactor the code.");

    SetVersion(QStringLiteral("0.3.1"));
    GenderV0_3_1();
    Save();
}

//---------------------------------------------------------------------------------------------------------------------
void VVITConverter::ToV0_3_2()
{
    // TODO. Delete if minimal supported version is 0.3.2
    Q_STATIC_ASSERT_X(VVITConverter::MeasurementMinVer < FORMAT_VERSION(0, 3, 2),
                      "Time to refactor the code.");

    SetVersion(QStringLiteral("0.3.2"));
    PM_SystemV0_3_2();
    Save();
}

//---------------------------------------------------------------------------------------------------------------------
void VVITConverter::ToV0_3_3()
{
    // TODO. Delete if minimal supported version is 0.3.3
    Q_STATIC_ASSERT_X(VVITConverter::MeasurementMinVer < FORMAT_VERSION(0, 3, 3),
                      "Time to refactor the code.");

    SetVersion(QStringLiteral("0.3.3"));
    ConvertMeasurementsToV0_3_3();
    Save();
}

//---------------------------------------------------------------------------------------------------------------------
void VVITConverter::ToV0_4_0()
{
    // TODO. Delete if minimal supported version is 0.4.0
    Q_STATIC_ASSERT_X(VVITConverter::MeasurementMinVer < FORMAT_VERSION(0, 4, 0),
                      "Time to refactor the code.");

    SetVersion(QStringLiteral("0.4.0"));
    ConverCustomerNameToV0_4_0();
    Save();
}

//---------------------------------------------------------------------------------------------------------------------
void VVITConverter::ToV0_5_0()
{
    // TODO. Delete if minimal supported version is 0.5.0
    Q_STATIC_ASSERT_X(VVITConverter::MeasurementMinVer < FORMAT_VERSION(0, 5, 0),
                      "Time to refactor the code.");

    SetVersion(QStringLiteral("0.5.0"));
    Save();
}
