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
static const QString nameOrchid = QStringLiteral( "Fluent2 Orchid" );
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
    names += nameOrchid;
#endif
    return names;
}

QskSkin* QskFluent2SkinFactory::createSkin( const QString& skinName )
{
#ifdef COLOR_THEMES
    if ( QString::compare( skinName, nameOrchid, Qt::CaseInsensitive ) == 0 )
    {
        auto skin = new QskFluent2Skin();

        {
            QskFluent2Theme::AccentColors accentColors;
            QskFluent2Theme::BaseColors baseColors;

            auto hct = QskHctColor( QskRgb::LemonChiffon );
            baseColors = { hct.toned( 70 ).rgb(),
                hct.toned( 60 ).rgb(), hct.toned( 80 ).rgb() };

            hct = QskHctColor( QskRgb::LightSkyBlue );
            accentColors = { hct.rgb(), hct.toned( 20 ).rgb(),
                hct.toned( 40 ).rgb(), hct.toned( 60 ).rgb() };

            skin->addTheme( QskAspect::Body,
                { QskFluent2Theme::Light, baseColors, accentColors }  );
        }

        {
            QskFluent2Theme::AccentColors accentColors;
            QskFluent2Theme::BaseColors baseColors;

            auto hct = QskHctColor( QskRgb::LemonChiffon );
            baseColors = { hct.toned( 30 ).rgb(),
                hct.toned( 20 ).rgb(), hct.toned( 40 ).rgb() };

            hct = QskHctColor( QskRgb::Khaki );
            accentColors = { hct.rgb(), hct.toned( 70 ).rgb(),
                hct.toned( 80 ).rgb(), hct.toned( 90 ).rgb() };

            skin->addTheme( QskAspect::Header,
                { QskFluent2Theme::Dark, baseColors, accentColors }  );

            skin->addTheme( QskAspect::Footer,
                { QskFluent2Theme::Dark, baseColors, accentColors }  );
        }

        return skin;
    }
#endif

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
