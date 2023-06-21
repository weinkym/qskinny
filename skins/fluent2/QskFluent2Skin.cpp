/******************************************************************************
 * QSkinny - Copyright (C) 2023 Edelhirsch Software GmbH
 *           SPDX-License-Identifier: BSD-3-Clause
 *****************************************************************************/

/*
    TODO:

    - we have a lot of lines with 1 pixels. Unfortunately OpenGL does some sort
      of antialiasing, when a line is not a pixel position. So we need to
      align the position of the line to device pixel metrics - or find something else ...

    - QskCheckBox::Error is not properly supported
 */

#include "QskFluent2Skin.h"

#include <QskSkinHintTableEditor.h>

#include <QskBox.h>
#include <QskCheckBox.h>
#include <QskComboBox.h>
#include <QskColorFilter.h>
#include <QskDialogButtonBox.h>
#include <QskFocusIndicator.h>
#include <QskFunctions.h>
#include <QskGraphic.h>
#include <QskGraphicIO.h>
#include <QskInputPanelBox.h>
#include <QskListView.h>
#include <QskMenu.h>
#include <QskPageIndicator.h>
#include <QskPushButton.h>
#include <QskProgressBar.h>
#include <QskRadioBox.h>
#include <QskScrollView.h>
#include <QskSegmentedBar.h>
#include <QskSeparator.h>
#include <QskShadowMetrics.h>
#include <QskSlider.h>
#include <QskSpinBox.h>
#include <QskStandardSymbol.h>
#include <QskSubWindow.h>
#include <QskSwitchButton.h>
#include <QskSwitchButtonSkinlet.h>
#include <QskTabBar.h>
#include <QskTabButton.h>
#include <QskTabView.h>
#include <QskTextInput.h>
#include <QskTextLabel.h>
#include <QskVirtualKeyboard.h>

#include <QskAnimationHint.h>
#include <QskAspect.h>
#include <QskBoxBorderColors.h>
#include <QskBoxBorderMetrics.h>
#include <QskBoxShapeMetrics.h>
#include <QskMargins.h>
#include <QskRgbValue.h>

#include <QskNamespace.h>
#include <QskPlatform.h>

#include <QGuiApplication>
#include <QScreen>

namespace
{
    inline constexpr QRgb rgbGray( int value, qreal opacity = 1.0 )
    {
        return qRgba( value, value, value, qRound( opacity * 255 ) );
    }

    inline constexpr QRgb rgbSolid( QRgb foreground, QRgb background )
    {
        const auto r2 = qAlpha( foreground ) / 255.0;
        const auto r1 = 1.0 - r2;

        const auto r = qRound( r1 * qRed( background ) + r2 * qRed( foreground ) );
        const auto g = qRound( r1 * qGreen( background ) + r2 * qGreen( foreground ) );
        const auto b = qRound( r1 * qBlue( background ) + r2 * qBlue( foreground ) );

        return qRgb( r, g, b );
    }

    inline constexpr QRgb rgbSolid2( QRgb foreground, QRgb background )
    {
        /*
            dummy method, so that we can compare the results with
            or without resolving the foreground alpha value
         */
#if 1
        return rgbSolid( foreground, background );
#else
        return foreground;
#endif
    }

    class Editor : private QskSkinHintTableEditor
    {
      public:
        Editor( QskSkinHintTable* table, const QskFluent2Theme& palette )
            : QskSkinHintTableEditor( table )
            , theme( palette )
        {
        }

        void setup();

      private:
        void setupBox();
        void setupCheckBox();
        void setupCheckBoxColors( QskAspect::Section );
        void setupComboBox();
        void setupDialogButtonBox();
        void setupFocusIndicator();
        void setupInputPanel();
        void setupListView();
        void setupMenu();
        void setupPageIndicator();
        void setupPopup();
        void setupProgressBar();
        void setupPushButton();
        void setupPushButtonColors( QskAspect::Section );
        void setupRadioBox();
        void setupScrollView();
        void setupSegmentedBar();
        void setupSeparator();
        void setupSlider();
        void setupSpinBox();
        void setupSubWindow();
        void setupSwitchButton();
        void setupTabButton();
        void setupTabBar();
        void setupTabView();
        void setupTextInput();
        void setupTextLabel();
        void setupVirtualKeyboard();

        inline QRgb sectionColor( QskAspect::Section section )
        {
            const auto& colors = theme.palette.background.solid;

            switch( section )
            {
                case QskAspect::Header:
                case QskAspect::Footer:
                    return colors.tertiary;

                case QskAspect::Card:
                case QskAspect::Floating:
                    return colors.base; // TODO ...

                default:
                    return colors.base;
            }
        }

        inline QskGraphic symbol( const char* name ) const
        {
            const QString path = QStringLiteral( ":fluent2/icons/qvg/" )
                + name + QStringLiteral( ".qvg" );

            return QskGraphicIO::read( path );
        }

        inline void setBoxBorderGradient( QskAspect aspect,
            QRgb border1, QRgb border2, QRgb baseColor )
        {
            border1 = rgbSolid( border1, baseColor );
            border2 = rgbSolid( border2, baseColor );

            setBoxBorderColors( aspect, { border1, border1, border1, border2 } );
        }

        inline void setBoxBorderGradient( QskAspect aspect,
            const QskFluent2Theme::BorderGradient& gradient, QRgb baseColor )
        {
            setBoxBorderGradient( aspect, gradient[ 0 ], gradient[ 1 ], baseColor );
        }

        const QskFluent2Theme& theme;
    };

    QFont createFont( const QString& name, qreal lineHeight,
        qreal size, qreal tracking, QFont::Weight weight )
    {
        QFont font( name, qRound( size ) );
        font.setPixelSize( qRound( lineHeight ) );

        if( !qskFuzzyCompare( tracking, 0.0 ) )
            font.setLetterSpacing( QFont::AbsoluteSpacing, tracking );

        font.setWeight( weight );

        return font;
    }
}

void Editor::setup()
{
    setupBox();
    setupCheckBox();
    setupComboBox();
    setupDialogButtonBox();
    setupFocusIndicator();
    setupInputPanel();
    setupListView();
    setupMenu();
    setupPageIndicator();
    setupPopup();
    setupProgressBar();
    setupPushButton();
    setupRadioBox();
    setupScrollView();
    setupSegmentedBar();
    setupSeparator();
    setupSlider();
    setupSpinBox();
    setupSubWindow();
    setupSwitchButton();
    setupTabButton();
    setupTabBar();
    setupTabView();
    setupTextInput();
    setupTextLabel();
    setupVirtualKeyboard();
}

void Editor::setupBox()
{
    using Q = QskBox;
    using A = QskAspect;

    setGradient( Q::Panel, sectionColor( A::Body ) );
    setGradient( Q::Panel | A::Header, sectionColor( A::Header ) );
    setGradient( Q::Panel | A::Footer, sectionColor( A::Footer ) );
}

void Editor::setupCheckBox()
{
    using Q = QskCheckBox;
    using A = QskAspect;

    setStrutSize( Q::Panel, 126, 38 );
    setSpacing( Q::Panel, 8 );

    setStrutSize( Q::Box, { 20, 20 } ); // 18 + 2*1 border
    setBoxShape( Q::Box, 4 ); // adapt to us taking the border into account
    setBoxBorderMetrics( Q::Box, 1 );
    setPadding( Q::Box, 5 ); // "icon size"

    setFontRole( Q::Text, QskFluent2Skin::Body );

    // colors

    const auto baseBody = sectionColor( A::Body );
    setupCheckBoxColors( A::Body );

    for ( int i = A::Body + 1; i <= A::Floating; i++ )
    {
        const auto section = static_cast< A::Section >( i );

        const auto baseColor = sectionColor( section );
        if ( baseColor != baseBody )
            setupCheckBoxColors( section );
    }
}

void Editor::setupCheckBoxColors( QskAspect::Section section )
{
    using Q = QskCheckBox;
    using A = QskAspect;

    const auto& pal = theme.palette;
    const auto baseColor = sectionColor( section );

    const auto checkMark = symbol( "checkmark" );

    for ( const auto state1 : { A::NoState, Q::Hovered, Q::Pressed, Q::Disabled } )
    {
        QRgb fillColor, borderColor, textColor;

        for ( const auto state2 : { A::NoState, Q::Checked } )
        {
            const auto states = state1 | state2;

            if ( states == A::NoState )
            {
                fillColor = pal.fillColor.controlAlt.secondary;
                borderColor = pal.strokeColor.controlStrong.defaultColor;
                textColor = pal.fillColor.text.primary;
            }
            else if ( states == Q::Hovered )
            {
                fillColor = pal.fillColor.controlAlt.tertiary;
                borderColor = pal.strokeColor.controlStrong.defaultColor;
                textColor = pal.fillColor.text.primary;
            }
            else if ( states == ( Q::Hovered | Q::Checked ) )
            {
                fillColor = pal.fillColor.accent.secondary;
                borderColor = fillColor;
                textColor = pal.fillColor.text.primary;
            }
            else if ( states == Q::Checked )
            {
                fillColor = pal.fillColor.accent.defaultColor;
                borderColor = pal.fillColor.accent.defaultColor;
                textColor = pal.fillColor.text.primary;
            }
            else if ( states == Q::Pressed )
            {
                fillColor = pal.fillColor.controlAlt.quaternary;
                borderColor = pal.strokeColor.controlStrong.disabled;
                textColor = pal.fillColor.text.primary;
            }
            else if ( states == ( Q::Pressed | Q::Checked ) )
            {
                fillColor = pal.fillColor.accent.tertiary;
                borderColor = pal.fillColor.accent.tertiary;
                textColor = pal.fillColor.text.primary;

                setSymbol( Q::Indicator | states, checkMark );
            }
            else if ( states == Q::Disabled )
            {
                fillColor = pal.fillColor.controlAlt.disabled;
                borderColor = pal.strokeColor.controlStrong.disabled;
                textColor = pal.fillColor.text.disabled;
            }
            else if ( states == ( Q::Disabled | Q::Checked ) )
            {
                fillColor = pal.fillColor.accent.disabled;
                borderColor = pal.fillColor.accent.disabled;
                textColor = pal.fillColor.text.disabled;
            }

            /*
                Support for QskCheckBox::Error is not properly defined.
                Doing some basic definitions, so that we can at least
                see the boxes with this state. TODO ...
             */
            for ( const auto state3 : { A::NoState, Q::Error } )
            {
                const auto box = Q::Box | states | state3;
                const auto text = Q::Text | states | state3;
                const auto indicator = Q::Indicator | states | state3;

#if 1
                if ( state3 == Q::Error && !( states & Q::Disabled ) )
                {
                    borderColor = QskRgb::IndianRed;
                    if ( states & Q::Checked )
                        fillColor = QskRgb::DarkRed;
                }
#endif
                fillColor = rgbSolid2( fillColor, baseColor );
                setGradient( box, fillColor );

                borderColor = rgbSolid2( borderColor, fillColor );
                setBoxBorderColors( box, borderColor );

                setColor( text, textColor );

                if ( states & Q::Checked )
                {
                    setGraphicRole( indicator, ( states & Q::Disabled )
                        ? QskFluent2Skin::GraphicRoleFillColorTextOnAccentDisabled
                        : QskFluent2Skin::GraphicRoleFillColorTextOnAccentPrimary );

                    setSymbol( indicator, checkMark );
                }
            }
        }
    }
}

void Editor::setupComboBox()
{
    using Q = QskComboBox;

    const auto& pal = theme.palette;

    setStrutSize( Q::Panel, { -1, 32 } );
    setBoxBorderMetrics( Q::Panel, 1 );
    setBoxShape( Q::Panel, 3 );
    setPadding( Q::Panel, { 11, 0, 11, 0 } );

    setGradient( Q::Panel, pal.fillColor.control.defaultColor );
    setBoxBorderGradient( Q::Panel,
        pal.elevation.control.border, pal.fillColor.control.defaultColor );

    setStrutSize( Q::Icon, 12, 12 );
    setPadding( Q::Icon, { 0, 0, 8, 0 } );
    setGraphicRole( Q::Icon, QskFluent2Skin::GraphicRoleFillColorTextPrimary );

    setAlignment( Q::Text, Qt::AlignLeft | Qt::AlignVCenter );
    setFontRole( Q::Text, QskFluent2Skin::Body );
    setColor( Q::Text, pal.fillColor.text.primary );

    setStrutSize( Q::StatusIndicator, 12, 12 );
    setSymbol( Q::StatusIndicator, symbol( "spin-box-arrow-down" ) );
    setSymbol( Q::StatusIndicator | Q::PopupOpen, symbol( "spin-box-arrow-up" ) );

    setGraphicRole( Q::StatusIndicator, QskFluent2Skin::GraphicRoleFillColorTextSecondary );

    // Hovered:

    setGradient( Q::Panel | Q::Hovered, pal.fillColor.control.secondary );
    setBoxBorderGradient( Q::Panel | Q::Hovered,
        pal.elevation.textControl.border, pal.fillColor.control.secondary );


    // Focused (Pressed doesn't exist yet):

    setBoxBorderMetrics( Q::Panel | Q::Focused, { 1, 1, 1, 2 } );

    setGradient( Q::Panel | Q::Focused, pal.fillColor.control.inputActive );

    auto gradient = pal.elevation.textControl.border;
    gradient.at( 1 ) = pal.fillColor.accent.defaultColor;

    setBoxBorderGradient( Q::Panel | Q::Focused, gradient, pal.fillColor.control.inputActive );

    // Disabled:

    setGradient( Q::Panel | Q::Disabled, pal.fillColor.control.disabled );
    setBoxBorderColors( Q::Panel | Q::Disabled, pal.strokeColor.control.defaultColor );

    setColor( Q::Text | Q::Disabled, pal.fillColor.text.disabled );
    setGraphicRole( Q::Icon | Q::Disabled, QskFluent2Skin::GraphicRoleFillColorTextDisabled );

    setGraphicRole( Q::StatusIndicator | Q::Disabled,
        QskFluent2Skin::GraphicRoleFillColorTextDisabled );
}

void Editor::setupDialogButtonBox()
{
    using Q = QskDialogButtonBox;
    const auto& pal = theme.palette;

    setPadding( Q::Panel, 24 );
    setGradient( Q::Panel, pal.background.solid.base );
    setPadding(Q::Panel, 20 );
}

void Editor::setupFocusIndicator()
{
    using Q = QskFocusIndicator;
    const auto& pal = theme.palette;

    setBoxBorderMetrics( Q::Panel, 2 );
    setPadding( Q::Panel, 3 );
    setBoxShape( Q::Panel, 4 );
    setBoxBorderColors( Q::Panel, pal.strokeColor.focus.outer );
}

void Editor::setupInputPanel()
{
}

void Editor::setupListView()
{
}

void Editor::setupMenu()
{
    using Q = QskMenu;
    const auto& pal = theme.palette;

    setPadding( Q::Panel, { 4, 6, 4, 6 } );
    setBoxBorderMetrics( Q::Panel, 1 );
    setBoxBorderColors( Q::Panel, pal.strokeColor.surface.flyout );
    setBoxShape( Q::Panel, 7 );
    setGradient( Q::Panel, pal.background.flyout.defaultColor );
    setShadowMetrics( Q::Panel, theme.shadow.flyout.metrics );
    setShadowColor( Q::Panel, theme.shadow.flyout.color );

    setPadding( Q::Segment, { 0, 10, 0, 10 } );
    setSpacing( Q::Segment, 15 );

    setGradient( Q::Segment | Q::Selected, pal.fillColor.subtle.secondary );
    setBoxBorderMetrics( Q::Segment | Q::Selected, { 3, 0, 0, 0 } );

    QskGradient selectedGradient( { { 0.0, pal.fillColor.subtle.secondary },
                                    { 0.25, pal.fillColor.subtle.secondary },
                                    { 0.25, pal.fillColor.accent.defaultColor },
                                    { 0.75, pal.fillColor.accent.defaultColor },
                                    { 0.75, pal.fillColor.subtle.secondary },
                                    { 1.0, pal.fillColor.subtle.secondary } } );
    setBoxBorderColors( Q::Segment | Q::Selected, selectedGradient );

    setFontRole( Q::Text, QskFluent2Skin::Body );
    setColor( Q::Text, pal.fillColor.text.primary );

    setStrutSize( Q::Icon, 12, 12 );
    setPadding( Q::Icon, { 8, 8, 0, 8 } );
    setGraphicRole( Q::Icon, QskFluent2Skin::GraphicRoleFillColorTextPrimary );
}

void Editor::setupPageIndicator()
{
}

void Editor::setupPopup()
{
    using Q = QskPopup;
    const auto& pal = theme.palette;

    setGradient( Q::Overlay, pal.background.overlay.defaultColor );
}

void Editor::setupProgressBar()
{
    using Q = QskProgressBar;
    using A = QskAspect;
    const auto& pal = theme.palette;

    setMetric( Q::Groove | A::Size, 1 );
    setBoxShape( Q::Groove, 100, Qt::RelativeSize );
    setGradient( Q::Groove, pal.strokeColor.controlStrong.defaultColor );

    setMetric( Q::Bar | A::Size, 3 );
    setBoxShape( Q::Bar, 100, Qt::RelativeSize );
    setGradient( Q::Bar, pal.fillColor.accent.defaultColor );
}

void Editor::setupPushButton()
{
    using Q = QskPushButton;
    using W = QskFluent2Skin;

    setStrutSize( Q::Panel, { 120, 32 } );
    setBoxShape( Q::Panel, 4 );
    setBoxBorderMetrics( Q::Panel, 1 );
    setBoxBorderMetrics( Q::Panel | W::Accent | Q::Disabled, 0 );

    // Fluent buttons don't really have icons,

    setStrutSize( Q::Icon, 12, 12 );
    setPadding( Q::Icon, { 0, 0, 8, 0 } );

    setFontRole( Q::Text, W::Body );

    const auto baseBody = sectionColor( QskAspect::Body );
    setupPushButtonColors( QskAspect::Body );

    for ( int i = QskAspect::Body + 1; i <= QskAspect::Floating; i++ )
    {
        const auto section = static_cast< QskAspect::Section >( i );

        const auto baseColor = sectionColor( section );
        if ( baseColor != baseBody )
            setupPushButtonColors( section );
    }
}

void Editor::setupPushButtonColors( QskAspect::Section section )
{
    using Q = QskPushButton;
    using W = QskFluent2Skin;

    const auto& pal = theme.palette;
    const auto baseColor = sectionColor( section );

    for ( const auto variation : { QskAspect::NoVariation, W::Accent } )
    {
        const auto panel = Q::Panel | section | variation;
        const auto text = Q::Text | section | variation;
        const auto icon = Q::Icon | section | variation;

        for ( const auto state : { QskAspect::NoState, Q::Hovered, Q::Pressed, Q::Disabled } )
        {
            QRgb panelColor, borderColor1, borderColor2, textColor;
            int graphicRole;

            if ( variation == W::Accent )
            {
                if ( state == Q::Hovered )
                {
                    panelColor = pal.fillColor.accent.secondary;
                    borderColor1 = pal.elevation.accentControl.border[0];
                    borderColor2 = pal.elevation.accentControl.border[1];
                    textColor = pal.fillColor.textOnAccent.primary;
                    graphicRole = W::GraphicRoleFillColorTextOnAccentPrimary;
                }
                else if ( state == Q::Pressed )
                {
                    panelColor = pal.fillColor.accent.tertiary;
                    borderColor1 = borderColor2 = pal.strokeColor.control.onAccentDefault;
                    textColor = pal.fillColor.textOnAccent.secondary;
                    graphicRole = W::GraphicRoleFillColorTextOnAccentSecondary;
                }
                else if ( state == Q::Disabled )
                {
                    panelColor = pal.fillColor.accent.disabled;
                    borderColor1 = borderColor2 = panelColor; // irrelevant: width is 0
                    textColor = pal.fillColor.textOnAccent.disabled;
                    graphicRole = W::GraphicRoleFillColorTextOnAccentDisabled;
                }
                else
                {
                    panelColor = pal.fillColor.accent.defaultColor;
                    borderColor1 = pal.elevation.accentControl.border[0];
                    borderColor2 = pal.elevation.accentControl.border[1];
                    textColor = pal.fillColor.textOnAccent.primary;
                    graphicRole = W::GraphicRoleFillColorTextOnAccentPrimary;
                }
            }
            else
            {
                if ( state == Q::Hovered )
                {
                    panelColor = pal.fillColor.control.secondary;
                    borderColor1 = pal.elevation.control.border[0];
                    borderColor2 = pal.elevation.control.border[1];
                    textColor = pal.fillColor.text.primary;
                    graphicRole = W::GraphicRoleFillColorTextPrimary;
                }
                else if ( state == Q::Pressed )
                {
                    panelColor = pal.fillColor.control.tertiary;
                    borderColor1 = borderColor2 = pal.strokeColor.control.defaultColor;
                    textColor = pal.fillColor.text.secondary;
                    graphicRole = W::GraphicRoleFillColorTextSecondary;
                }
                else if ( state == Q::Disabled )
                {
                    panelColor = pal.fillColor.control.disabled;
                    borderColor1 = borderColor2 = pal.strokeColor.control.defaultColor;
                    textColor = pal.fillColor.text.disabled;
                    graphicRole = W::GraphicRoleFillColorTextDisabled;
                }
                else
                {
                    panelColor = pal.fillColor.control.defaultColor;
                    borderColor1 = pal.elevation.control.border[0];
                    borderColor2 = pal.elevation.control.border[0];
                    textColor = pal.fillColor.text.primary;
                    graphicRole = W::GraphicRoleFillColorTextPrimary;
                }
            }

            panelColor = rgbSolid2( panelColor, baseColor );

            setGradient( panel | state, panelColor );

            setBoxBorderGradient( panel | state,
                borderColor1, borderColor2, panelColor );

            setColor( text | state, textColor );
            setGraphicRole( icon | state, graphicRole );
        }
    }
}

void Editor::setupRadioBox()
{
    using Q = QskRadioBox;
    const auto& pal = theme.palette;

    setSpacing( Q::Button, 8 );
    setStrutSize( Q::Button, { 115, 38 } );

    setStrutSize( Q::CheckIndicatorPanel, { 20, 20 } );
    setBoxShape( Q::CheckIndicatorPanel, 100, Qt::RelativeSize );
    setBoxBorderMetrics( Q::CheckIndicatorPanel, 1 );
    setFontRole( Q::Text, QskFluent2Skin::Body );
    setColor( Q::Text, pal.fillColor.text.primary );

    // Rest

    setGradient( Q::CheckIndicatorPanel, pal.fillColor.controlAlt.secondary );
    setBoxBorderColors( Q::CheckIndicatorPanel, pal.strokeColor.controlStrong.defaultColor );

    setGradient( Q::CheckIndicatorPanel | Q::Selected, pal.fillColor.accent.defaultColor );
    setBoxBorderMetrics( Q::CheckIndicatorPanel | Q::Selected, 0 );

    setPadding( Q::CheckIndicatorPanel | Q::Selected, { 5, 5 } ); // indicator "strut size"

    setBoxShape( Q::CheckIndicator | Q::Selected, 100, Qt::RelativeSize );
    setBoxBorderMetrics( Q::CheckIndicator | Q::Selected, 1 );
    setGradient( Q::CheckIndicator | Q::Selected, pal.fillColor.textOnAccent.primary );

    setBoxBorderGradient( Q::CheckIndicator | Q::Selected,
        pal.elevation.circle.border, pal.fillColor.accent.defaultColor );


    // Hover

    setGradient( Q::CheckIndicatorPanel | Q::Hovered, pal.fillColor.controlAlt.tertiary );

    setGradient( Q::CheckIndicatorPanel | Q::Hovered | Q::Selected,
        pal.fillColor.accent.secondary );
    setPadding( Q::CheckIndicatorPanel | Q::Hovered | Q::Selected, { 4, 4 } ); // indicator "strut size"

    setBoxBorderGradient( Q::CheckIndicator | Q::Hovered,
        pal.elevation.circle.border, pal.fillColor.accent.secondary );

    // Pressed

    setGradient( Q::CheckIndicatorPanel | Q::Pressed, pal.fillColor.controlAlt.quaternary );
    setBoxBorderColors( Q::CheckIndicatorPanel | Q::Pressed,
        pal.strokeColor.controlStrong.disabled );

    setPadding( Q::CheckIndicatorPanel | Q::Pressed, { 7, 7 } ); // indicator "strut size"

    setBoxShape( Q::CheckIndicator | Q::Pressed, 100, Qt::RelativeSize );
    setBoxBorderMetrics( Q::CheckIndicator | Q::Pressed, 0 );
    setGradient( Q::CheckIndicator | Q::Pressed, pal.fillColor.textOnAccent.primary );

    setGradient( Q::CheckIndicatorPanel | Q::Pressed | Q::Selected,
        pal.fillColor.accent.tertiary );

    setBoxBorderMetrics( Q::CheckIndicatorPanel | Q::Pressed | Q::Selected, 0 );

    setPadding( Q::CheckIndicatorPanel | Q::Pressed | Q::Selected, { 6, 6 } ); // indicator "strut size"
    setBoxBorderMetrics( Q::CheckIndicator | Q::Pressed, 1 );

    setBoxBorderGradient( Q::CheckIndicator | Q::Pressed | Q::Selected,
        pal.elevation.circle.border, pal.fillColor.accent.tertiary );

    // Disabled

    setGradient( Q::CheckIndicatorPanel | Q::Disabled, pal.fillColor.controlAlt.disabled );
    setBoxBorderColors( Q::CheckIndicatorPanel | Q::Disabled,
        pal.strokeColor.controlStrong.disabled );

    setGradient( Q::CheckIndicatorPanel | Q::Disabled | Q::Selected,
        pal.fillColor.accent.disabled );
    setBoxBorderMetrics( Q::CheckIndicatorPanel | Q::Disabled | Q::Selected, 0 );

    setPadding( Q::CheckIndicatorPanel | Q::Disabled | Q::Selected, { 6, 6 } ); // indicator "strut size"

    setBoxBorderMetrics( Q::CheckIndicator | Q::Disabled | Q::Selected, 0 );
    setGradient( Q::CheckIndicator | Q::Disabled | Q::Selected,
        pal.fillColor.textOnAccent.primary );
    setBoxShape( Q::CheckIndicator | Q::Disabled | Q::Selected, 100, Qt::RelativeSize );

    setColor( Q::Text | Q::Disabled, pal.fillColor.text.disabled );
}

void Editor::setupScrollView()
{
}

void Editor::setupSegmentedBar()
{
    using Q = QskSegmentedBar;
    using A = QskAspect;
    const auto& pal = theme.palette;

    const QSizeF segmentStrutSize( 120, 32 );

    setBoxBorderMetrics( Q::Panel, 1 );

    setBoxBorderGradient( Q::Panel, pal.elevation.control.border,
        pal.fillColor.control.defaultColor );

    setGradient( Q::Panel, pal.fillColor.control.defaultColor );
    setSpacing( Q::Panel, 8 );

    setStrutSize( Q::Icon, { 12, 12 } );
    setGraphicRole( Q::Icon, QskFluent2Skin::GraphicRoleFillColorTextPrimary );

    setFontRole( Q::Text, QskFluent2Skin::Body );
    setColor( Q::Text, pal.fillColor.text.primary );

    setStrutSize( Q::Segment | A::Horizontal, segmentStrutSize );
    setStrutSize( Q::Segment | A::Vertical, segmentStrutSize.transposed() );
    setBoxShape( Q::Segment, 4 );
    setPadding( Q::Segment, { 8, 0, 8, 0 } );

    // Hovered:
    setGradient( Q::Segment | Q::Hovered, pal.fillColor.control.secondary );

    setBoxBorderGradient( Q::Segment | Q::Hovered, pal.elevation.control.border,
        pal.fillColor.control.secondary );

    // Selected:
    setGradient( Q::Segment | Q::Selected, pal.fillColor.accent.defaultColor );
    setGraphicRole( Q::Icon | Q::Selected,
        QskFluent2Skin::GraphicRoleFillColorTextOnAccentPrimary );
    setColor( Q::Text | Q::Selected, pal.fillColor.textOnAccent.primary );

    // Disabled:
    const QRgb standardDisabledBorderColor =
        rgbSolid( pal.strokeColor.control.defaultColor, pal.fillColor.control.disabled );

    setBoxBorderColors( Q::Segment | Q::Disabled, standardDisabledBorderColor );

    setGradient( Q::Segment | Q::Disabled, pal.fillColor.control.disabled );
    setColor( Q::Text | Q::Disabled, pal.fillColor.text.disabled );
    setGraphicRole( Q::Icon | Q::Disabled, QskFluent2Skin::GraphicRoleFillColorTextDisabled );


    setGradient( Q::Segment | Q::Selected | Q::Disabled, pal.fillColor.accent.disabled );
    setColor( Q::Text | Q::Selected | Q::Disabled, pal.fillColor.textOnAccent.disabled );
    setGraphicRole( Q::Icon | Q::Selected | Q::Disabled,
        QskFluent2Skin::GraphicRoleFillColorTextOnAccentDisabled );
    setBoxBorderMetrics( Q::Panel | Q::Selected | Q::Disabled, 0 );
}

void Editor::setupSeparator()
{
    using A = QskAspect;
    using Q = QskSeparator;

    const auto& pal = theme.palette;

    for ( auto variation : { A::Horizontal, A::Vertical } )
    {
        const auto aspect = Q::Panel | variation;

        setMetric( aspect | A::Size, 1 );
        setBoxShape( Q::Panel, 0 );
        setBoxBorderMetrics( Q::Panel, 0 );
        setGradient( aspect, pal.strokeColor.divider.defaultColor );
    }
}

void Editor::setupSlider()
{
    using Q = QskSlider;
    using A = QskAspect;
    const auto& pal = theme.palette;

    const qreal extent = 22;
    setMetric( Q::Panel | A::Size, extent );
    setBoxShape( Q::Panel, 0 );
    setBoxBorderMetrics( Q::Panel, 0 );
    setGradient( Q::Panel, {} );

    setPadding( Q::Panel | A::Horizontal, QskMargins( 0.5 * extent, 0 ) );
    setPadding( Q::Panel | A::Vertical, QskMargins( 0, 0.5 * extent ) );

    setMetric( Q::Groove | A::Size, 4 );
    setGradient( Q::Groove, pal.fillColor.controlStrong.defaultColor );
    setBoxShape( Q::Groove, 100, Qt::RelativeSize );

    setMetric( Q::Fill | A::Size, 4 );
    setGradient( Q::Fill, pal.fillColor.accent.defaultColor );
    setBoxShape( Q::Fill, 100, Qt::RelativeSize );

    setStrutSize( Q::Handle, { 22, 22 } );
    setGradient( Q::Handle, pal.fillColor.controlSolid.defaultColor );
    setBoxShape( Q::Handle, 100, Qt::RelativeSize );
    setBoxBorderMetrics( Q::Handle, 1 );
    setBoxBorderGradient( Q::Handle, pal.elevation.circle.border,
        pal.fillColor.controlSolid.defaultColor );

    setStrutSize( Q::Ripple, { 12, 12 } );
    setGradient( Q::Ripple, pal.fillColor.accent.defaultColor );
    setBoxShape( Q::Ripple, 100, Qt::RelativeSize );

    setStrutSize( Q::Ripple | Q::Hovered, { 14, 14 } );

    setStrutSize( Q::Ripple | Q::Pressed, { 10, 10 } );
    setGradient( Q::Ripple | Q::Pressed, pal.fillColor.accent.tertiary );

    setGradient( Q::Groove | Q::Disabled, pal.fillColor.controlStrong.disabled );
    setGradient( Q::Fill | Q::Disabled, pal.fillColor.accent.disabled );
    setGradient( Q::Ripple | Q::Disabled, pal.fillColor.controlStrong.disabled );
}

void Editor::setupSpinBox()
{
    using Q = QskSpinBox;
    const auto& pal = theme.palette;

    setHint( Q::Panel | QskAspect::Style, Q::ButtonsRight );
    setStrutSize( Q::Panel, { -1, 32 } );
    setBoxBorderMetrics( Q::Panel, 1 );
    setBoxShape( Q::Panel, 3 );
    setPadding( Q::Panel, { 11, 0, 11, 0 } );

    setGradient( Q::Panel, pal.fillColor.control.defaultColor );
    setBoxBorderGradient( Q::Panel,
        pal.elevation.control.border, pal.fillColor.control.defaultColor );

    setAlignment( Q::Text, Qt::AlignLeft );
    setFontRole( Q::Text, QskFluent2Skin::Body );
    setColor( Q::Text, pal.fillColor.text.primary );

    setPadding( Q::TextPanel, { 11, 5, 0, 0 } );

    setStrutSize( Q::UpPanel, 16, 16 );
    setStrutSize( Q::DownPanel, 16, 16 );

    setStrutSize( Q::UpPanel, 32, 20 );
    setPadding( Q::UpPanel, { 11, 7, 11, 7 } );
    setStrutSize( Q::DownPanel, 34, 20 );
    setPadding( Q::DownPanel, { 11, 7, 13, 7 } );

    setSymbol( Q::UpIndicator, symbol( "spin-box-arrow-up" ) );
    setSymbol( Q::DownIndicator, symbol( "spin-box-arrow-down" ) );

    setGraphicRole( Q::UpIndicator, QskFluent2Skin::GraphicRoleFillColorTextSecondary );
    setGraphicRole( Q::DownIndicator, QskFluent2Skin::GraphicRoleFillColorTextSecondary );

    // Hovered:

    setGradient( Q::Panel | Q::Hovered, pal.fillColor.control.secondary );
    setBoxBorderGradient( Q::Panel | Q::Hovered,
        pal.elevation.textControl.border, pal.fillColor.control.secondary );

    // Focused (Pressed doesn't exist yet):

    setBoxBorderMetrics( Q::Panel | Q::Focused, { 1, 1, 1, 2 } );

    setGradient( Q::Panel | Q::Focused, pal.fillColor.control.inputActive );

    auto gradient = pal.elevation.textControl.border;
    gradient.at( 1 ) = pal.fillColor.accent.defaultColor;

    setBoxBorderGradient( Q::Panel | Q::Focused, gradient,
        pal.fillColor.control.inputActive );

    // Disabled:

    setGradient( Q::Panel | Q::Disabled, pal.fillColor.control.disabled );
    setBoxBorderColors( Q::Panel | Q::Disabled,
        pal.strokeColor.control.defaultColor );

    setColor( Q::Text | Q::Disabled, pal.fillColor.text.disabled );

    setGraphicRole( Q::UpIndicator | Q::Disabled,
        QskFluent2Skin::GraphicRoleFillColorTextDisabled );

    setGraphicRole( Q::DownIndicator | Q::Disabled,
        QskFluent2Skin::GraphicRoleFillColorTextDisabled );
}

void Editor::setupTabBar()
{
    setGradient( QskTabBar::Panel, sectionColor( QskAspect::Body ) );
}

void Editor::setupTabButton()
{
    using Q = QskTabButton;
    const auto& pal = theme.palette;

    setStrutSize( Q::Panel, { -1, 31 } );
    setPadding( Q::Panel, { 7, 0, 7, 0 } );
    setBoxShape( Q::Panel, { 7, 7, 0, 0 } );

    setAlignment( Q::Text, Qt::AlignLeft | Qt::AlignVCenter );

    const auto baseColor = sectionColor( QskAspect::Body );

    setBoxBorderMetrics( Q::Panel, { 0, 0, 0, 1 } );

    for ( const auto state : { QskAspect::NoState, 
        Q::Checked, Q::Hovered, Q::Pressed, Q::Disabled } )
    {
        QRgb panelColor, textColor;
        int fontRole = QskFluent2Skin::Body;

        if ( state == Q::Checked )
        {
            panelColor = pal.background.solid.tertiary;
            textColor = pal.fillColor.text.primary;
            fontRole = QskFluent2Skin::BodyStrong;
        }
        else if ( state == Q::Hovered )
        {
            panelColor = pal.fillColor.subtle.secondary;
            textColor = pal.fillColor.text.secondary;
        }
        else if ( state == Q::Pressed )
        {
            panelColor = pal.fillColor.subtle.tertiary;
            textColor = pal.fillColor.text.secondary;
        }
        else if ( state == Q::Disabled )
        {
            panelColor = pal.fillColor.control.disabled;
            textColor = pal.fillColor.text.disabled;
        }
        else
        {
            panelColor = pal.fillColor.subtle.tertiary;
            textColor = pal.fillColor.text.secondary;
        }

        const auto panel = Q::Panel | state;
        const auto text = Q::Text | state;

        panelColor = rgbSolid2( panelColor, baseColor );
        setGradient( panel, panelColor );

        const auto borderColor = rgbSolid2( pal.strokeColor.card.defaultColor, panelColor );
        setBoxBorderColors( panel, borderColor );

        if ( state == Q::Checked )
            setBoxBorderMetrics( panel, { 1, 1, 1, 0 } );
        else
            setBoxBorderMetrics( panel, { 0, 0, 0, 1 } );

        setFontRole( text, fontRole );
        setColor( text, textColor );
    }
}

void Editor::setupTabView()
{
    using Q = QskTabView;
    const auto& pal = theme.palette;

    const auto baseColor = sectionColor( QskAspect::Body );
    const auto pageColor = rgbSolid2( pal.background.solid.tertiary, baseColor );

    setGradient( Q::Page, pageColor );
}

void Editor::setupTextLabel()
{
    using Q = QskTextLabel;
    const auto& pal = theme.palette;

    setPadding( Q::Panel, 10 );

    setFontRole( Q::Text, QskFluent2Skin::Body );
    setColor( Q::Text, pal.fillColor.text.primary );
}

void Editor::setupTextInput()
{
    using Q = QskTextInput;
    const auto& pal = theme.palette;

    setStrutSize( Q::Panel, { -1, 30 } );
    setBoxBorderMetrics( Q::Panel, 1 );
    setBoxShape( Q::Panel, 3 );
    setPadding( Q::Panel, { 11, 0, 11, 0 } );

    setAlignment( Q::Text, Qt::AlignLeft | Qt::AlignVCenter );
    setFontRole( Q::Text, QskFluent2Skin::Body );
    setColor( Q::Text, pal.fillColor.text.secondary );

    setGradient( Q::Panel, pal.fillColor.control.defaultColor );
    setBoxBorderGradient( Q::Panel,
        pal.elevation.textControl.border, pal.fillColor.control.defaultColor );

    setColor( Q::PanelSelected, pal.fillColor.accent.selectedTextBackground );
    setColor( Q::TextSelected, pal.fillColor.textOnAccent.selectedText );

    // Hovered:

    setGradient( Q::Panel | Q::Hovered, pal.fillColor.control.secondary );
    setBoxBorderGradient( Q::Panel | Q::Hovered,
        pal.elevation.textControl.border, pal.fillColor.control.secondary );


    // Pressed & Focused:

    for( const auto& state : { Q::Focused, Q::Editing } )
    {
        setBoxBorderMetrics( Q::Panel | state, { 1, 1, 1, 2 } );

        setGradient( Q::Panel | state, pal.fillColor.control.inputActive );

        auto gradient = pal.elevation.textControl.border;
        gradient.at( 1 ) = pal.fillColor.accent.defaultColor;

        setBoxBorderGradient( Q::Panel | state,
            gradient, pal.fillColor.control.inputActive );
    }

    // Disabled:

    setGradient( Q::Panel | Q::Disabled, pal.fillColor.control.disabled );
    setBoxBorderColors( Q::Panel | Q::Disabled, pal.strokeColor.control.defaultColor );

    setColor( Q::Text | Q::Disabled, pal.fillColor.text.disabled );
}

void Editor::setupSwitchButton()
{
    using Q = QskSwitchButton;
    using A = QskAspect;
    const auto& pal = theme.palette;

    const QSizeF strutSize( 38, 18 );
    setStrutSize( Q::Groove | A::Horizontal, strutSize );
    setStrutSize( Q::Groove | A::Vertical, strutSize.transposed() );
    setBoxShape( Q::Groove, 100, Qt::RelativeSize );
    setBoxBorderMetrics( Q::Groove, 1 );
    setBoxBorderMetrics( Q::Groove | Q::Checked, 0 );

    setBoxShape( Q::Handle, 100, Qt::RelativeSize );
    setPosition( Q::Handle, 0.1, { QskStateCombination::CombinationNoState, Q::Disabled } );
    setPosition( Q::Handle | Q::Checked, 0.9,
        { QskStateCombination::CombinationNoState, Q::Disabled } );
    setAnimation( Q::Handle | A::Metric, 100 );

    setBoxBorderMetrics( Q::Handle | Q::Checked, 1 );

    // ### big size during animation

    setGradient( Q::Groove, pal.fillColor.controlAlt.secondary );
    setGradient( Q::Groove | Q::Checked, pal.fillColor.accent.defaultColor );
    setBoxBorderColors( Q::Groove, pal.strokeColor.controlStrong.defaultColor );

    setStrutSize( Q::Handle, 12, 12 );
    setGradient( Q::Handle, pal.strokeColor.controlStrong.defaultColor );
    setGradient( Q::Handle | Q::Checked, pal.fillColor.textOnAccent.primary );

    setBoxBorderGradient( Q::Handle | Q::Checked,
        pal.elevation.circle.border, pal.fillColor.accent.defaultColor );

    setGradient( Q::Groove | Q::Hovered, pal.fillColor.controlAlt.tertiary );
    setGradient( Q::Groove | Q::Hovered | Q::Checked, pal.fillColor.accent.secondary );
    setBoxBorderColors( Q::Groove | Q::Hovered, pal.fillColor.text.secondary );

    setStrutSize( Q::Handle | Q::Hovered, 14, 14,
        { QskStateCombination::CombinationNoState, Q::Checked } );
    setGradient( Q::Handle | Q::Hovered, pal.fillColor.text.secondary );
    // Handle | Hovered | Checked is the same as in Rest state

    setBoxBorderGradient( Q::Handle | Q::Hovered | Q::Checked,
        pal.elevation.circle.border, pal.fillColor.accent.secondary );

    setGradient( Q::Groove | Q::Pressed, pal.fillColor.controlAlt.quaternary );
    setGradient( Q::Groove | Q::Pressed | Q::Checked, pal.fillColor.accent.tertiary );
    setBoxBorderColors( Q::Groove | Q::Pressed, pal.strokeColor.controlStrong.defaultColor );

    const QSizeF pressedSize( 17, 14 );

    setStrutSize( Q::Handle | Q::Pressed | A::Horizontal,
        pressedSize, { QskStateCombination::CombinationNoState, Q::Checked }  );

    setStrutSize( Q::Handle | Q::Pressed | A::Vertical,
        pressedSize.transposed(), { QskStateCombination::CombinationNoState, Q::Checked }  );

    setGradient( Q::Handle | Q::Pressed, pal.strokeColor.controlStrong.defaultColor );
    // Handle | Pressed | Checked is the same as in Rest state

    setBoxBorderGradient( Q::Handle | Q::Pressed | Q::Checked,
        pal.elevation.circle.border, pal.fillColor.accent.tertiary );

    setGradient( Q::Groove | Q::Disabled, pal.fillColor.controlAlt.disabled );
    setBoxBorderColors( Q::Groove | Q::Disabled, pal.fillColor.text.disabled );
    setGradient( Q::Groove | Q::Disabled | Q::Checked, pal.fillColor.accent.disabled );
    setBoxBorderColors( Q::Groove | Q::Disabled | Q::Checked, pal.fillColor.accent.disabled );

    setStrutSize( Q::Handle | Q::Disabled, 12, 12,
        { QskStateCombination::CombinationNoState, Q::Checked } );

    setGradient( Q::Handle | Q::Disabled, pal.fillColor.text.disabled );
    setGradient( Q::Handle | Q::Disabled | Q::Checked, pal.fillColor.textOnAccent.disabled );
    setBoxBorderMetrics( Q::Handle | Q::Disabled | Q::Checked, 1 );
}

void Editor::setupSubWindow()
{
    using Q = QskSubWindow;
    const auto& pal = theme.palette;

    setPadding( Q::Panel, { 0, 31, 0, 0 } );
    setBoxShape( Q::Panel, 7 );
    setBoxBorderMetrics( Q::Panel, 1 );
    setBoxBorderColors( Q::Panel, pal.strokeColor.surface.defaultColor );
    setGradient( Q::Panel, pal.background.layer.alt );
    setShadowMetrics( Q::Panel, theme.shadow.dialog.metrics );
    setShadowColor( Q::Panel, theme.shadow.dialog.color );

    setHint( Q::TitleBarPanel | QskAspect::Style, Q::TitleBar | Q::Title );
    setPadding( Q::TitleBarPanel, { 24, 31, 24, 0 } );

    setFontRole( Q::TitleBarText, QskFluent2Skin::Subtitle );
    setColor( Q::TitleBarText, pal.fillColor.text.primary );
    setAlignment( Q::TitleBarText, Qt::AlignLeft );
    setTextOptions( Q::TitleBarText, Qt::ElideRight, QskTextOptions::NoWrap );
}

void Editor::setupVirtualKeyboard()
{
    using Q = QskVirtualKeyboard;
    const auto& pal = theme.palette;

    setMargin( Q::ButtonPanel, 2 );
    setGradient( Q::ButtonPanel, pal.fillColor.control.defaultColor );
    setGradient( Q::ButtonPanel | Q::Hovered, pal.fillColor.control.secondary );
    setGradient( Q::ButtonPanel | QskPushButton::Pressed, pal.fillColor.control.tertiary );

    setColor( Q::ButtonText, pal.fillColor.text.primary );
    setFontRole( Q::ButtonText, QskFluent2Skin::BodyLarge );
    setColor( Q::ButtonText | QskPushButton::Pressed, pal.fillColor.text.secondary );

    setGradient( Q::Panel, pal.background.solid.secondary );
    setPadding( Q::Panel, 8 );
}

QskFluent2Theme::QskFluent2Theme( Theme lightness )
    : QskFluent2Theme( lightness,
                        { // default Fluent accent colors:
                          0xff98ecfe,
                          0xff60ccfe,
                          0xff0093f9,
                          0xff0078d4,
                          0xff005eb7,
                          0xff003d92,
                          0xff001968
                        } )
{
}

QskFluent2Theme::QskFluent2Theme( Theme theme,
    const std::array< QRgb, NumAccentColors >& accentColors )
{
    using namespace QskRgb;

    if( theme == Light )
    {
        {
            auto& colors = palette.fillColor;

            colors.text.primary = rgbGray( 0, 0.8956 );
            colors.text.secondary = rgbGray( 0, 0.6063 );
            colors.text.tertiary = rgbGray( 0, 0.4458 );
            colors.text.disabled = rgbGray( 0, 0.3614 );

            colors.accentText.primary = accentColors[ AccentDark2 ];
            colors.accentText.secondary = accentColors[ AccentDark3 ];
            colors.accentText.tertiary = accentColors[ AccentDark1 ];
            colors.accentText.disabled = rgbGray( 0, 0.3614 );

            colors.textOnAccent.primary = rgbGray( 255 );
            colors.textOnAccent.secondary = rgbGray( 255, 0.70 );
            colors.textOnAccent.disabled = rgbGray( 255 );
            colors.textOnAccent.selectedText = rgbGray( 255 );

            colors.control.defaultColor = rgbGray( 255, 0.70 );
            colors.control.secondary = rgbGray( 249, 0.50 );
            colors.control.tertiary = rgbGray( 249, 0.30 );
            colors.control.inputActive = rgbGray( 255 );
            colors.control.disabled = rgbGray( 249, 0.30 );

            colors.controlStrong.defaultColor = rgbGray( 0, 0.4458 );
            colors.controlStrong.disabled = rgbGray( 0, 0.3173 );

            colors.subtle.secondary = rgbGray( 0, 0.0373 );
            colors.subtle.tertiary = rgbGray( 0, 0.0241 );
            colors.subtle.disabled = Qt::transparent;

            colors.controlSolid.defaultColor = rgbGray( 255 );

            colors.controlAlt.secondary = rgbGray( 0, 0.0241 );
            colors.controlAlt.tertiary = rgbGray( 0, 0.0578 );
            colors.controlAlt.quaternary = rgbGray( 0, 0.0924 );
            colors.controlAlt.disabled = Qt::transparent;

            colors.accent.defaultColor = accentColors[ AccentDark1 ];
            colors.accent.secondary = toTransparentF( accentColors[ AccentDark1 ], 0.90 );
            colors.accent.tertiary = toTransparentF( accentColors[ AccentDark1 ], 0.80 );
            colors.accent.disabled = rgbGray( 0, 0.2169 );
            colors.accent.selectedTextBackground = accentColors[ AccentBase ];
        }

#if 0
        {
            // system colors

            critical = 0xffc42b1c;
            success = 0xff0f7b0f;
            attention = 0xff005fb7;
            caution = 0xff9d5d00;
            attentionBackground = rgbGray( 246, 0.50 );
            successBackground = 0xffdff6dd;
            cautionBackground = 0xfffff4ce;
            criticalBackground = 0xfffde7e9;
            neutral = rgbGray( 0, 0.4458 );
            neutralBackground = rgbGray( 0, 0.0241 );

            solidNeutral = rgbGray( 138 );
            solidAttentionBackground = rgbGray( 247 );
            solidNeutralBackground = rgbGray( 243 );
        }
#endif

        {
            auto& colors = palette.elevation;

            colors.control.border = { rgbGray( 0, 0.0578 ), rgbGray( 0, 0.1622 ) };
            colors.circle.border = { rgbGray( 0, 0.0578 ), rgbGray( 0, 0.1622 ) };
            colors.textControl.border = { rgbGray( 0, 0.0578 ), palette.fillColor.text.secondary };
            colors.textControl.borderFocused = { rgbGray( 0, 0.0578 ), rgbGray( 0, 0.0578 ) };
            colors.accentControl.border = { rgbGray( 255, 0.08 ), rgbGray( 0, 0.40 ) };
        }

        {
            auto& colors = palette.strokeColor;

            colors.control.defaultColor = rgbGray( 0, 0.0578 );
            colors.control.secondary = rgbGray( 0, 0.1622 );
            colors.control.onAccentDefault = rgbGray( 255.08 );
            colors.control.onAccentSecondary = rgbGray( 0, 0.40 );
            colors.control.onAccentTertiary = rgbGray( 0, 0.2169 );
            colors.control.onAccentDisabled = rgbGray( 0, 0.0578 );

            colors.controlStrong.defaultColor = rgbGray( 0, 0.4458 );
            colors.controlStrong.disabled = rgbGray( 0, 0.2169 );

            colors.card.defaultColor = rgbGray( 0, 0.0578 );
            colors.card.defaultSolid = rgbGray( 235 );

            colors.divider.defaultColor = rgbGray( 0, 0.0803 );

            colors.surface.defaultColor = rgbGray( 117, 0.40 );
            colors.surface.flyout = rgbGray( 0, 0.0578 );

            colors.focus.outer = rgbGray( 0, 0.8956 );
            colors.focus.inner = rgbGray( 255 );
        }

        {
            auto& colors = palette.background;

            colors.card.defaultColor = rgbGray( 255, 0.70 );
            colors.card.secondary = rgbGray( 246, 0.50 );
            colors.card.tertiary = rgbGray( 255 );

            colors.overlay.defaultColor = rgbGray( 0, 0.30 );

            colors.layer.alt = rgbGray( 255 );

            colors.flyout.defaultColor = rgbGray( 252, 0.85 );

            colors.solid.base = rgbGray( 243 );
            colors.solid.secondary = rgbGray( 238 );
            colors.solid.tertiary = rgbGray( 249 );
            colors.solid.quaternary = rgbGray( 255 );
        }

        // Shadow:

        shadow.cardRest = { QskShadowMetrics( 0, 4, QPointF( 0, 2 ) ), rgbGray( 0, 0.04 ) };
        shadow.cardHover = { QskShadowMetrics( 0, 4, QPointF( 0, 2 ) ), rgbGray( 0, 0.10 ) };
        shadow.flyout = { QskShadowMetrics( 0, 16, QPointF( 0, 8 ) ), rgbGray( 0, 0.14 ) };
        // ### should actually be drawn twice with different values:
        shadow.dialog = { QskShadowMetrics( 0, 21, QPointF( 0, 2 ) ), rgbGray( 0, 0.1474 ) };
    }
    else if( theme == Dark )
    {
        {
            auto& colors = palette.fillColor;

            colors.text.primary = rgbGray( 255 );
            colors.text.secondary = rgbGray( 255, 0.786 );
            colors.text.tertiary = rgbGray( 255, 0.5442 );
            colors.text.disabled = rgbGray( 255, 0.3628 );

            colors.accentText.primary = accentColors[ AccentLight3 ];
            colors.accentText.secondary = accentColors[ AccentLight3 ];
            colors.accentText.tertiary = accentColors[ AccentLight2 ];
            colors.accentText.disabled = rgbGray( 255, 0.3628 );

            colors.textOnAccent.primary = rgbGray( 0 );
            colors.textOnAccent.secondary = rgbGray( 0, 0.50 );
            colors.textOnAccent.disabled = rgbGray( 255, 0.5302 );
            colors.textOnAccent.selectedText = rgbGray( 255 );

            colors.control.defaultColor = rgbGray( 255, 0.0605 );
            colors.control.secondary = rgbGray( 255, 0.0837 );
            colors.control.tertiary = rgbGray( 255, 0.0326 );
            colors.control.inputActive = rgbGray( 30, 0.70 );
            colors.control.disabled = rgbGray( 255, 0.0419 );

            colors.controlStrong.defaultColor = rgbGray( 255, 0.5442 );
            colors.controlStrong.disabled = rgbGray( 255, 0.2465 );

            colors.subtle.secondary = rgbGray( 255, 0.0605 );
            colors.subtle.tertiary = rgbGray( 255, 0.0419 );
            colors.subtle.disabled = Qt::transparent;

            colors.controlSolid.defaultColor = rgbGray( 69 );

            colors.controlAlt.secondary = rgbGray( 0, 0.10 );
            colors.controlAlt.tertiary = rgbGray( 255, 0.0419 );
            colors.controlAlt.quaternary = rgbGray( 255, 0.0698 );
            colors.controlAlt.disabled = Qt::transparent;

            colors.accent.defaultColor = accentColors[ AccentLight2 ];
            colors.accent.secondary = toTransparentF( accentColors[ AccentLight2 ], 0.90 );
            colors.accent.tertiary = toTransparentF( accentColors[ AccentLight2 ], 0.80 );
            colors.accent.disabled = rgbGray( 255, 0.1581 );
            colors.accent.selectedTextBackground = accentColors[ AccentBase ];
        }

#if 0
        {
            // system colors

            critical = 0xffff99a4;
            success = 0xff6ccb5f;
            attention = 0xff60cdff;
            caution = 0xfffce100;
            attentionBackground = rgbGray( 255, 0.0326 );
            successBackground = 0xff393d1b;
            cautionBackground = 0xff433519;
            criticalBackground = 0xff442726;
            neutral = rgbGray( 255, 0.5442 );
            neutralBackground = rgbGray( 255, 0.0326 );
            solidNeutral = rgbGray( 157 );
            solidAttentionBackground = rgbGray( 46 );
            solidNeutralBackground = rgbGray( 46 );
        }
#endif

        {
            auto& colors = palette.elevation;

            colors.control.border = { rgbGray( 255, 0.093 ), rgbGray( 255, 0.0698 ) };
            colors.circle.border = { rgbGray( 255, 0.093 ), rgbGray( 255, 0.0698 ) };

            colors.textControl.border = { rgbGray( 255, 0.08 ), palette.fillColor.text.secondary };

            colors.textControl.borderFocused = { rgbGray( 255, 0.08 ), rgbGray( 255, 0.08 ) };
            colors.accentControl.border = { rgbGray( 255, 0.08 ), rgbGray( 0, 0.14 ) };
        }

        {
            auto& colors = palette.strokeColor;

            colors.control.defaultColor = rgbGray( 255, 0.0698 );
            colors.control.secondary = rgbGray( 255, 0.093 );
            colors.control.onAccentDefault = rgbGray( 255, 0.08 );
            colors.control.onAccentSecondary = rgbGray( 0, 0.14 );
            colors.control.onAccentTertiary = rgbGray( 0, 0.2169 );
            colors.control.onAccentDisabled = rgbGray( 0, 0.20 );

            colors.controlStrong.defaultColor = rgbGray( 255, 0.5442 );
            colors.controlStrong.disabled = rgbGray( 255, 0.1581 );

            colors.card.defaultColor = rgbGray( 255, 0.0578 );
            colors.card.defaultSolid = rgbGray( 235 );

            colors.divider.defaultColor = rgbGray( 255, 0.0837 );

            colors.surface.defaultColor = rgbGray( 117, 0.40 );
            colors.surface.flyout = rgbGray( 0, 0.20 );

            colors.focus.outer = rgbGray( 255 );
            colors.focus.inner = rgbGray( 0, 0.70 );
        }

        {
            auto& colors = palette.background;

            colors.card.defaultColor = rgbGray( 255, 0.0512 );
            colors.card.secondary = rgbGray( 255, 0.0326 );
            colors.card.tertiary = rgbGray( 255 ); // not set in Figma

            colors.overlay.defaultColor = rgbGray( 0, 0.30 );

            colors.layer.alt = rgbGray( 255, 0.0538 );

            colors.flyout.defaultColor = rgbGray( 44, 0.96 );

            colors.solid.base = rgbGray( 32 );
            colors.solid.secondary = rgbGray( 28 );
            colors.solid.tertiary = rgbGray( 40 );
            colors.solid.quaternary = rgbGray( 44 );
        }

        // Shadow:

        shadow.cardRest = { QskShadowMetrics( 0, 4, QPointF( 0, 2 ) ), rgbGray( 0, 0.13 ) };
        shadow.cardHover = { QskShadowMetrics( 0, 4, QPointF( 0, 2 ) ), rgbGray( 0, 0.26 ) };
        shadow.flyout = { QskShadowMetrics( 0, 16, QPointF( 0, 8 ) ), rgbGray( 0, 0.26 ) };
        // ### should actually be drawn twice with different values:
        shadow.dialog = { QskShadowMetrics( 0, 21, QPointF( 0, 2 ) ), rgbGray( 0, 0.37 ) };
    }
}

QskFluent2Skin::QskFluent2Skin( const QskFluent2Theme& palette, QObject* parent )
    : Inherited( parent )
{
    setupFonts();
    setupGraphicFilters( palette );

    Editor editor( &hintTable(), palette );
    editor.setup();
}

QskFluent2Skin::~QskFluent2Skin()
{
}

void QskFluent2Skin::setupFonts()
{
    static QString fontName( QStringLiteral( "Segoe UI Variable" ) );
    Inherited::setupFonts( fontName );

    setFont( Caption, createFont( fontName, 12, 16, 0.0, QFont::Normal ) );
    setFont( Body, createFont( fontName, 14, 20, 0.0, QFont::Normal ) );
    setFont( BodyStrong, createFont( fontName, 14, 20, 0.0, QFont::DemiBold ) );
    setFont( BodyLarge, createFont( fontName, 18, 24, 0.0, QFont::Medium ) );
    setFont( Subtitle, createFont( fontName, 20, 28, 0.0, QFont::DemiBold ) );
    setFont( Title, createFont( fontName, 28, 36, 0.0, QFont::DemiBold ) );
    setFont( TitleLarge, createFont( fontName, 40, 52, 0.0, QFont::DemiBold ) );
    setFont( Display, createFont( fontName, 68, 92, 0.0, QFont::DemiBold ) );
}

void QskFluent2Skin::setGraphicColor( GraphicRole role, QRgb rgb )
{
    QskColorFilter colorFilter;
    colorFilter.setMask( QskRgb::RGBAMask );
    colorFilter.addColorSubstitution( QskRgb::Black, rgb );

    setGraphicFilter( role, colorFilter );
}

void QskFluent2Skin::setupGraphicFilters( const QskFluent2Theme& theme )
{
    const auto& colors = theme.palette.fillColor;

    setGraphicColor( GraphicRoleFillColorTextDisabled, colors.text.disabled );
    setGraphicColor( GraphicRoleFillColorTextOnAccentDisabled, colors.textOnAccent.disabled );
    setGraphicColor( GraphicRoleFillColorTextOnAccentPrimary, colors.textOnAccent.primary );
    setGraphicColor( GraphicRoleFillColorTextOnAccentSecondary, colors.textOnAccent.secondary );
    setGraphicColor( GraphicRoleFillColorTextPrimary, colors.text.primary );
    setGraphicColor( GraphicRoleFillColorTextSecondary, colors.text.secondary );
}

#include "moc_QskFluent2Skin.cpp"
