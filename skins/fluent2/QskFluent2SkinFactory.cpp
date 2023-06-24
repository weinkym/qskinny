/******************************************************************************
 * QSkinny - Copyright (C) 2023 Edelhirsch Software GmbH
 *           SPDX-License-Identifier: BSD-3-Clause
 *****************************************************************************/

#include "QskFluent2SkinFactory.h"
#include "QskFluent2Skin.h"
#include "QskFluent2Theme.h"

static const QString nameLight = QStringLiteral( "Fluent2 Light" );
static const QString nameDark = QStringLiteral( "Fluent2 Dark" );

namespace
{
    inline constexpr QRgb rgbGray( int value )
    {
        return qRgba( value, value, value, 255 );
    } 
}

QskFluent2SkinFactory::QskFluent2SkinFactory( QObject* parent )
    : QskSkinFactory( parent )
{
}

QskFluent2SkinFactory::~QskFluent2SkinFactory()
{
}

QStringList QskFluent2SkinFactory::skinNames() const
{
    return { nameLight, nameDark };
}

QskSkin* QskFluent2SkinFactory::createSkin( const QString& skinName )
{
    QskFluent2Theme::Theme theme;

    if ( QString::compare( skinName, nameLight, Qt::CaseInsensitive ) == 0 )
    {
        theme = QskFluent2Theme::Light;
    }
    else if ( QString::compare( skinName, nameDark, Qt::CaseInsensitive ) == 0 )
    {
        theme = QskFluent2Theme::Dark;
    }
    else
    {
        return nullptr;
    }

    QskFluent2Theme::AccentColors accentColors;
    QskFluent2Theme::BaseColors baseColors[2];

    if ( theme == QskFluent2Theme::Light )
    {
        accentColors = { 0xff0078d4, 0xff005eb7, 0xff003d92, 0xff001968 };
        baseColors[0] = { rgbGray( 243 ), rgbGray( 249 ), rgbGray( 238 ) };
        baseColors[1] = { rgbGray( 249 ), rgbGray( 249 ), rgbGray( 238 ) };
    }
    else
    {
        accentColors = { 0xff0078d4, 0xff0093f9, 0xff60ccfe, 0xff98ecfe };
        baseColors[0] = { rgbGray( 32 ), rgbGray( 40 ), rgbGray( 28 ) };
        baseColors[1] = { rgbGray( 40 ), rgbGray( 44 ), rgbGray( 28 ) };
    }

    auto skin = new QskFluent2Skin();

    skin->addTheme( QskAspect::Body, { theme, baseColors[0], accentColors }  );
    skin->addTheme( QskAspect::Header, { theme, baseColors[1], accentColors } );
    skin->addTheme( QskAspect::Footer, { theme, baseColors[1], accentColors } );

    return skin;
}

#include "moc_QskFluent2SkinFactory.cpp"
