/******************************************************************************
 * QSkinny - Copyright (C) 2023 Edelhirsch Software GmbH
 *           SPDX-License-Identifier: BSD-3-Clause
 *****************************************************************************/

#include "QskFluent2SkinFactory.h"
#include "QskFluent2Skin.h"
#include "QskFluent2Theme.h"
#include "QskHctColor.h"
#include "QskRgbValue.h"

#define COLOR_THEMES

static const QString nameLight = QStringLiteral( "Fluent2 Light" );
static const QString nameDark = QStringLiteral( "Fluent2 Dark" );

#ifdef COLOR_THEMES
static const QString nameColored = QStringLiteral( "Fluent2 Lemon" );
#endif

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
    QStringList names = { nameLight, nameDark };
#ifdef COLOR_THEMES
    names += nameColored;
#endif
    return names;
}

QskSkin* QskFluent2SkinFactory::createSkin( const QString& skinName )
{
    QskSkin::ColorScheme colorScheme;

    if ( QString::compare( skinName, nameLight, Qt::CaseInsensitive ) == 0 )
    {
        colorScheme = QskSkin::Light;
    }
    else if ( QString::compare( skinName, nameDark, Qt::CaseInsensitive ) == 0 )
    {
        colorScheme = QskSkin::Dark;
    }
#ifdef COLOR_THEMES
    else if ( QString::compare( skinName, nameColored, Qt::CaseInsensitive ) == 0 )
    {
        colorScheme = QskSkin::Unknown;
    }
#endif
    else
    {
        return nullptr;
    }

    struct
    {
        QskSkin::ColorScheme scheme;
        QskFluent2Theme::BaseColors baseColors;
        QskFluent2Theme::AccentColors accentColors;

        QskFluent2Theme theme() const { return { scheme, baseColors, accentColors }; }
    } colors[2];

    switch( colorScheme )
    {
        case QskSkin::Light:
        {
            colors[0].scheme = QskSkin::Light;
            colors[0].baseColors = { rgbGray( 243 ), rgbGray( 249 ), rgbGray( 238 ) };
            colors[0].accentColors = { 0xff0078d4, 0xff005eb7, 0xff003d92, 0xff001968 };

            colors[1].scheme = QskSkin::Light;
            colors[1].baseColors = { rgbGray( 249 ), rgbGray( 249 ), rgbGray( 238 ) };
            colors[1].accentColors = colors[0].accentColors;

            break;
        }
        case QskSkin::Dark:
        {
            colors[0].scheme = QskSkin::Dark;
            colors[0].baseColors = { rgbGray( 32 ), rgbGray( 40 ), rgbGray( 28 ) };
            colors[0].accentColors = { 0xff0078d4, 0xff0093f9, 0xff60ccfe, 0xff98ecfe };

            colors[1].scheme = QskSkin::Dark;
            colors[1].baseColors = { rgbGray( 40 ), rgbGray( 44 ), rgbGray( 28 ) };
            colors[1].accentColors = colors[0].accentColors;

            break;
        }
        default:
        {
            QskHctColor hct;

            colors[0].scheme = QskSkin::Light;

            hct = QskHctColor( QskRgb::LemonChiffon );
            colors[0].baseColors = { hct.toned( 70 ).rgb(),
                hct.toned( 60 ).rgb(), hct.toned( 80 ).rgb() };

            hct = QskHctColor( QskRgb::LightSkyBlue );
            colors[0].accentColors = { hct.rgb(), hct.toned( 20 ).rgb(),
                hct.toned( 40 ).rgb(), hct.toned( 60 ).rgb() };

            colors[1].scheme = QskSkin::Dark;

            hct = QskHctColor( QskRgb::LemonChiffon );
            colors[1].baseColors = { hct.toned( 30 ).rgb(),
                hct.toned( 20 ).rgb(), hct.toned( 40 ).rgb() };

            hct = QskHctColor( QskRgb::Khaki );
            colors[1].accentColors = { hct.rgb(), hct.toned( 70 ).rgb(),
                hct.toned( 80 ).rgb(), hct.toned( 90 ).rgb() };
        }
    }

    auto skin = new QskFluent2Skin();

    skin->addTheme( QskAspect::Body, colors[0].theme() );
    skin->addTheme( QskAspect::Header, colors[1].theme() );
    skin->addTheme( QskAspect::Footer, colors[1].theme() );

    return skin;
}

#include "moc_QskFluent2SkinFactory.cpp"
