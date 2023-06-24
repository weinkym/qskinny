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

    - QskComboBox::Pressed state is missing

    - missing ( dummy implementation only ):

        - QskPageIndicator
        - QskInputPanel
        - QskListView
        - QskScrollView

    - using qskDpToPixels
 */

/*
    The palette is made of a specific configurable colors and
    predefined semitransparent shades of gray. Both need to
    be resolved to opaque colors with the base colors of the sections. 

    Resolving the colors can be done in 2 ways:

        - render time

          This actually means, that we do not create opaque colors and
          create the scene graph nodes with semitransparent colors.

        - definition time

          We create opaque colors for the base colors of the sections
          and set them as skin hints.

    Resolving at render time sounds like the right solution as we
    background colors set in application code will just work.

    Unfortunately we have 2 different sets of grays for light/dark
    base colors and when applications are setting a light color, where a
    dark color ( or v.v ) is expected we might end up with unacceptable
    results: ( white on light or black on dark ).

    So there are pros and cons and we do not have a final opinion
    about waht to do. For the moment we implement resolving at definition
    time as an option to be able to play with both solutions.
 */
#include "QskFluent2Skin.h"
#include "QskFluent2Theme.h"

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
    inline QFont createFont( const QString& name, qreal lineHeight,
        qreal size, qreal tracking, QFont::Weight weight )
    {
        QFont font( name, qRound( size ) );
        font.setPixelSize( qRound( lineHeight ) );

        if( !qskFuzzyCompare( tracking, 0.0 ) )
            font.setLetterSpacing( QFont::AbsoluteSpacing, tracking );

        font.setWeight( weight );

        return font;
    }

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
#if 0
        return rgbSolid( foreground, background );
#else
        Q_UNUSED( background );
        return foreground;
#endif
    }

    class Editor : private QskSkinHintTableEditor
    {
      public:
        Editor( QskSkinHintTable* table )
            : QskSkinHintTableEditor( table )
        {
        }

        void setupMetrics();
        void setupColors( QskAspect::Section, const QskFluent2Theme& );

      private:
        void setupFocusIndicator( const QskFluent2Theme& );
        void setupInputPanel( const QskFluent2Theme& );
        void setupPopup( const QskFluent2Theme& );
        void setupSubWindow( const QskFluent2Theme& );

        void setupBoxMetrics();
        void setupBoxColors( QskAspect::Section, const QskFluent2Theme& );

        void setupCheckBoxMetrics();
        void setupCheckBoxColors( QskAspect::Section, const QskFluent2Theme& );

        void setupComboBoxMetrics();
        void setupComboBoxColors( QskAspect::Section, const QskFluent2Theme& );

        void setupDialogButtonBoxMetrics();
        void setupDialogButtonBoxColors( QskAspect::Section, const QskFluent2Theme& );

        void setupListViewMetrics();
        void setupListViewColors( QskAspect::Section, const QskFluent2Theme& );

        void setupMenuMetrics();
        void setupMenuColors( QskAspect::Section, const QskFluent2Theme& );

        void setupPageIndicatorMetrics();
        void setupPageIndicatorColors( QskAspect::Section, const QskFluent2Theme& );

        void setupProgressBarMetrics();
        void setupProgressBarColors( QskAspect::Section, const QskFluent2Theme& );

        void setupPushButtonMetrics();
        void setupPushButtonColors( QskAspect::Section, const QskFluent2Theme& );

        void setupRadioBoxMetrics();
        void setupRadioBoxColors( QskAspect::Section, const QskFluent2Theme& );

        void setupScrollViewMetrics();
        void setupScrollViewColors( QskAspect::Section, const QskFluent2Theme& );

        void setupSegmentedBarMetrics();
        void setupSegmentedBarColors( QskAspect::Section, const QskFluent2Theme& );

        void setupSeparatorMetrics();
        void setupSeparatorColors( QskAspect::Section, const QskFluent2Theme& );

        void setupSliderMetrics();
        void setupSliderColors( QskAspect::Section, const QskFluent2Theme& );

        void setupSpinBoxMetrics();
        void setupSpinBoxColors( QskAspect::Section, const QskFluent2Theme& );

        void setupSwitchButtonMetrics();
        void setupSwitchButtonColors( QskAspect::Section, const QskFluent2Theme& );

        void setupTabButtonMetrics();
        void setupTabButtonColors( QskAspect::Section, const QskFluent2Theme& );

        void setupTabBarMetrics();
        void setupTabBarColors( QskAspect::Section, const QskFluent2Theme& );

        void setupTabViewMetrics();
        void setupTabViewColors( QskAspect::Section, const QskFluent2Theme& );

        void setupTextInputMetrics();
        void setupTextInputColors( QskAspect::Section, const QskFluent2Theme& );

        void setupTextLabelMetrics();
        void setupTextLabelColors( QskAspect::Section, const QskFluent2Theme& );

        void setupVirtualKeyboardMetrics();
        void setupVirtualKeyboardColors( QskAspect::Section, const QskFluent2Theme& );

#if 0
        inline QRgb sectionColor( QskAspect::Section section )
        {
            const auto& colors = m_theme.palette.background.solid;

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
#endif

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
    };

}

void Editor::setupMetrics()
{
    setupBoxMetrics();
    setupCheckBoxMetrics();
    setupComboBoxMetrics();
    setupDialogButtonBoxMetrics();
    setupListViewMetrics();
    setupMenuMetrics();
    setupPageIndicatorMetrics();
    setupProgressBarMetrics();
    setupPushButtonMetrics();
    setupRadioBoxMetrics();
    setupScrollViewMetrics();
    setupSegmentedBarMetrics();
    setupSeparatorMetrics();
    setupSliderMetrics();
    setupSpinBoxMetrics();
    setupSwitchButtonMetrics();
    setupTabButtonMetrics();
    setupTabBarMetrics();
    setupTabViewMetrics();
    setupTextInputMetrics();
    setupTextLabelMetrics();
    setupVirtualKeyboardMetrics();
}

void Editor::setupColors( QskAspect::Section section, const QskFluent2Theme& theme )
{
    if ( section == QskAspect::Body )
    {
        // TODO
        setupFocusIndicator( theme );
        setupInputPanel( theme );
        setupPopup( theme );
        setupSubWindow( theme );
    }

    setupBoxColors( section, theme );
    setupCheckBoxColors( section, theme );
    setupComboBoxColors( section, theme );
    setupDialogButtonBoxColors( section, theme );
    setupListViewColors( section, theme );
    setupMenuColors( section, theme );
    setupPageIndicatorColors( section, theme );
    setupProgressBarColors( section, theme );
    setupPushButtonColors( section, theme );
    setupRadioBoxColors( section, theme );
    setupScrollViewColors( section, theme );
    setupSegmentedBarColors( section, theme );
    setupSeparatorColors( section, theme );
    setupSliderColors( section, theme );
    setupSwitchButtonColors( section, theme );
    setupSpinBoxColors( section, theme );
    setupTabButtonColors( section, theme );
    setupTabBarColors( section, theme );
    setupTabViewColors( section, theme );
    setupTextInputColors( section, theme );
    setupTextLabelColors( section, theme );
    setupVirtualKeyboardColors( section, theme );
};


void Editor::setupBoxMetrics()
{
}

void Editor::setupBoxColors(
    QskAspect::Section section, const QskFluent2Theme& theme )
{
    setGradient( QskBox::Panel | section,
        theme.palette.background.solid.primary );
}

void Editor::setupCheckBoxMetrics()
{
    using Q = QskCheckBox;

    setStrutSize( Q::Panel, 126, 38 );
    setSpacing( Q::Panel, 8 );

    setStrutSize( Q::Box, { 20, 20 } ); // 18 + 2*1 border
    setBoxShape( Q::Box, 4 ); // adapt to us taking the border into account
    setBoxBorderMetrics( Q::Box, 1 );
    setPadding( Q::Box, 5 ); // "icon size"

    setFontRole( Q::Text, QskFluent2Skin::Body );
}

void Editor::setupCheckBoxColors(
    QskAspect::Section section, const QskFluent2Theme& theme )
{
    using Q = QskCheckBox;
    using A = QskAspect;

    const auto& pal = theme.palette;

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
                const auto box = Q::Box | section | states | state3;
                const auto text = Q::Text | section | states | state3;
                const auto indicator = Q::Indicator | section | states | state3;

#if 1
                if ( state3 == Q::Error && !( states & Q::Disabled ) )
                {
                    borderColor = QskRgb::IndianRed;
                    if ( states & Q::Checked )
                        fillColor = QskRgb::DarkRed;
                }
#endif
                fillColor = rgbSolid2( fillColor, pal.background.solid.primary );
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

void Editor::setupComboBoxMetrics()
{
    using Q = QskComboBox;

    setStrutSize( Q::Panel, { -1, 32 } );
    setBoxBorderMetrics( Q::Panel, 1 );
    setBoxShape( Q::Panel, 3 );
    setPadding( Q::Panel, { 11, 0, 11, 0 } );

    setStrutSize( Q::Icon, 12, 12 );
    setPadding( Q::Icon, { 0, 0, 8, 0 } );

    setAlignment( Q::Text, Qt::AlignLeft | Qt::AlignVCenter );
    setFontRole( Q::Text, QskFluent2Skin::Body );

    setStrutSize( Q::StatusIndicator, 12, 12 );
    setSymbol( Q::StatusIndicator, symbol( "spin-box-arrow-down" ) );
    setSymbol( Q::StatusIndicator | Q::PopupOpen, symbol( "spin-box-arrow-up" ) );

    // Using Focused (Pressed doesn't exist yet):
    setBoxBorderMetrics( Q::Panel | Q::Focused, { 1, 1, 1, 2 } );
}

void Editor::setupComboBoxColors(
    QskAspect::Section section, const QskFluent2Theme& theme )
{
    using Q = QskComboBox;
    using W = QskFluent2Skin;

    const auto& pal = theme.palette;

    for ( const auto state : { QskAspect::NoState, Q::Hovered, Q::Focused, Q::Disabled } )
    {
        QRgb panelColor, borderColor1, borderColor2, textColor;

        if ( state == QskAspect::NoState )
        {
            panelColor = pal.fillColor.control.defaultColor;
            borderColor1 = pal.elevation.control.border[0];
            borderColor2 = pal.elevation.control.border[1];
            textColor = pal.fillColor.text.primary;

        }
        else if ( state == Q::Hovered )
        {
            panelColor = pal.fillColor.control.secondary;
            borderColor1 = pal.elevation.textControl.border[0];
            borderColor2 = pal.elevation.textControl.border[1];
            textColor = pal.fillColor.text.primary;
        }
        else if ( state == Q::Focused )
        {

            panelColor = pal.fillColor.control.inputActive;
            borderColor1 = pal.elevation.textControl.border[0];
            borderColor2 = pal.fillColor.accent.defaultColor;
            textColor = pal.fillColor.text.primary;
        }
        else if ( state == Q::Disabled )
        {
            panelColor = pal.fillColor.control.disabled;
            borderColor2 = borderColor1 = pal.strokeColor.control.defaultColor;
            textColor = pal.fillColor.text.disabled;
        }

        const auto panel = Q::Panel | section | state;
        const auto text = Q::Text | section | state;
        const auto icon = Q::Icon | section | state;
        const auto indicator = Q::StatusIndicator | section | state;

        panelColor = rgbSolid2( panelColor, pal.background.solid.primary );

        setGradient( panel, panelColor );
        setBoxBorderGradient( panel, borderColor1, borderColor2, panelColor );

        setColor( text, textColor );

        if ( state == Q::Disabled )
        {
            setGraphicRole( icon, W::GraphicRoleFillColorTextDisabled );
            setGraphicRole( indicator, W::GraphicRoleFillColorTextDisabled );
        }
        else
        {
            setGraphicRole( icon, W::GraphicRoleFillColorTextPrimary );
            setGraphicRole( indicator, W::GraphicRoleFillColorTextSecondary );
        }
    }
}

void Editor::setupDialogButtonBoxMetrics()
{
    setPadding( QskDialogButtonBox::Panel, 20 );
}

void Editor::setupDialogButtonBoxColors(
    QskAspect::Section section, const QskFluent2Theme& theme )
{
    setGradient( QskDialogButtonBox::Panel | section,
        theme.palette.background.solid.primary );
}

void Editor::setupFocusIndicator( const QskFluent2Theme& theme )
{
    using Q = QskFocusIndicator;
    const auto& pal = theme.palette;

    /*
        When having sections with dark and others with light colors
        we need a focus indicator that works on both. TODO ...
     */

    setBoxBorderMetrics( Q::Panel, 2 );
    setPadding( Q::Panel, 3 );
    setBoxShape( Q::Panel, 4 );
    setBoxBorderColors( Q::Panel, pal.strokeColor.focus.outer );
}

void Editor::setupInputPanel( const QskFluent2Theme& theme )
{
    Q_UNUSED( theme );
}

void Editor::setupListViewMetrics()
{
}

void Editor::setupListViewColors( QskAspect::Section, const QskFluent2Theme& )
{
}

void Editor::setupMenuMetrics()
{
    using Q = QskMenu;

    setPadding( Q::Panel, { 4, 6, 4, 6 } );
    setBoxBorderMetrics( Q::Panel, 1 );
    setBoxShape( Q::Panel, 7 );

    setPadding( Q::Segment, { 0, 10, 0, 10 } );
    setSpacing( Q::Segment, 15 );
    setBoxBorderMetrics( Q::Segment | Q::Selected, { 3, 0, 0, 0 } );

    setFontRole( Q::Text, QskFluent2Skin::Body );

    setStrutSize( Q::Icon, 12, 12 );
    setPadding( Q::Icon, { 8, 8, 0, 8 } );
}

void Editor::setupMenuColors(
    QskAspect::Section section, const QskFluent2Theme& theme )
{
    Q_UNUSED( section );

    using Q = QskMenu;

#if 1
    setShadowMetrics( Q::Panel, theme.shadow.flyout.metrics );
#endif

    const auto& pal = theme.palette;

    setBoxBorderColors( Q::Panel, pal.strokeColor.surface.flyout );
    setGradient( Q::Panel, pal.background.flyout.defaultColor );
    setShadowColor( Q::Panel, theme.shadow.flyout.color );

    setGradient( Q::Segment | Q::Selected, pal.fillColor.subtle.secondary );

    QskGradient selectedGradient( { { 0.0, pal.fillColor.subtle.secondary },
                                    { 0.25, pal.fillColor.subtle.secondary },
                                    { 0.25, pal.fillColor.accent.defaultColor },
                                    { 0.75, pal.fillColor.accent.defaultColor },
                                    { 0.75, pal.fillColor.subtle.secondary },
                                    { 1.0, pal.fillColor.subtle.secondary } } );
    setBoxBorderColors( Q::Segment | Q::Selected, selectedGradient );

    setColor( Q::Text, pal.fillColor.text.primary );

    setGraphicRole( Q::Icon, QskFluent2Skin::GraphicRoleFillColorTextPrimary );
}

void Editor::setupPageIndicatorMetrics()
{
    /*
        This code has absolitely nothing to do with the Fluent2 specs.
        It is simply a placeholder, so that we can see something until
        the implementation has been done
     */
    using Q = QskPageIndicator;

    setSpacing( Q::Panel, 3 );
    setPadding( Q::Panel, 4 );
    setBoxShape( Q::Panel, 6, Qt::AbsoluteSize );

    setStrutSize( Q::Bullet, 8, 8 );

    // circles, without border
    setBoxShape( Q::Bullet, 100, Qt::RelativeSize );
    setBoxBorderMetrics( Q::Bullet, 0 );

    setMargin( Q::Bullet, 1 );
    setMargin( Q::Bullet | Q::Selected, 0 );
}

void Editor::setupPageIndicatorColors(
    QskAspect::Section section, const QskFluent2Theme& theme )
{
    using Q = QskPageIndicator;

    const auto& pal = theme.palette;

    auto panelColor = pal.fillColor.control.secondary;
#if 0
    panelColor = rgbSolid2( panelColor, pal.background.solid.base );
#endif

    const auto panel = Q::Panel | section;
    const auto bullet = Q::Bullet | section;

    setGradient( panel, panelColor );

    setGradient( bullet, pal.fillColor.controlStrong.defaultColor );
    setGradient( bullet | Q::Selected, pal.fillColor.accent.defaultColor );
}

void Editor::setupPopup( const QskFluent2Theme& theme )
{
    using Q = QskPopup;

    const auto& pal = theme.palette;

    setGradient( Q::Overlay, pal.background.overlay.defaultColor );
}

void Editor::setupProgressBarMetrics()
{
    using Q = QskProgressBar;
    using A = QskAspect;

    setMetric( Q::Groove | A::Size, 1 );
    setBoxShape( Q::Groove, 100, Qt::RelativeSize );

    setMetric( Q::Bar | A::Size, 3 );
    setBoxShape( Q::Bar, 100, Qt::RelativeSize );
}

void Editor::setupProgressBarColors(
    QskAspect::Section, const QskFluent2Theme& theme )
{
    using Q = QskProgressBar;

    const auto& pal = theme.palette;

    setGradient( Q::Groove, pal.strokeColor.controlStrong.defaultColor );
    setGradient( Q::Bar, pal.fillColor.accent.defaultColor );
}

void Editor::setupPushButtonMetrics()
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
}

void Editor::setupPushButtonColors(
    QskAspect::Section section, const QskFluent2Theme& theme )
{
    using Q = QskPushButton;
    using W = QskFluent2Skin;

    const auto& pal = theme.palette;

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

            panelColor = rgbSolid2( panelColor, pal.background.solid.primary );

            setGradient( panel | state, panelColor );

            setBoxBorderGradient( panel | state,
                borderColor1, borderColor2, panelColor );

            setColor( text | state, textColor );
            setGraphicRole( icon | state, graphicRole );
        }
    }
}

void Editor::setupRadioBoxMetrics()
{
    using Q = QskRadioBox;

    setSpacing( Q::Button, 8 );
    setStrutSize( Q::Button, { 115, 38 } );

    /*
        We do not have an indicator - states are indicated by the panel border

        However the colors of the inner side of the border are not solid for
        the selected states and we use a dummy indicator to get this done.

        How to solve this in a better way, TODO ... 
     */

    setBoxShape( Q::CheckIndicator, 100, Qt::RelativeSize );
    setBoxBorderMetrics( Q::CheckIndicator, 0 );
    setBoxBorderMetrics( Q::CheckIndicator | Q::Selected, 1 );
    setBoxBorderMetrics( Q::CheckIndicator | Q::Pressed | Q::Selected, 1 );

    setBoxShape( Q::CheckIndicatorPanel, 100, Qt::RelativeSize );
    setStrutSize( Q::CheckIndicatorPanel, { 20, 20 } );

    setBoxBorderMetrics( Q::CheckIndicatorPanel, 1 );

    setBoxBorderMetrics( Q::CheckIndicatorPanel | Q::Selected, 0 );
    setPadding( Q::CheckIndicatorPanel | Q::Selected, { 5, 5 } ); // indicator "strut size"

    setPadding( Q::CheckIndicatorPanel | Q::Hovered | Q::Selected, { 4, 4 } ); // indicator "strut size"
    setPadding( Q::CheckIndicatorPanel | Q::Pressed, { 7, 7 } ); // indicator "strut size"

    setBoxBorderMetrics( Q::CheckIndicatorPanel | Q::Pressed | Q::Selected, 0 );
    setPadding( Q::CheckIndicatorPanel | Q::Pressed | Q::Selected, { 6, 6 } ); // indicator "strut size"

    setBoxBorderMetrics( Q::CheckIndicatorPanel | Q::Disabled | Q::Selected, 0 );
    setPadding( Q::CheckIndicatorPanel | Q::Disabled | Q::Selected, { 6, 6 } ); // indicator "strut size"

    setFontRole( Q::Text, QskFluent2Skin::Body );
}

void Editor::setupRadioBoxColors(
    QskAspect::Section section, const QskFluent2Theme& theme )
{
    using Q = QskRadioBox;
    using A = QskAspect;

    const auto& pal = theme.palette;

    for ( const auto state1 : { A::NoState, Q::Hovered, Q::Pressed, Q::Disabled } )
    {
        for ( const auto state2 : { A::NoState, Q::Selected } )
        {
            const auto states = state1 | state2;

            auto indicatorColor = pal.fillColor.textOnAccent.primary;
            if ( !( states & Q::Selected ) )
                indicatorColor = QskRgb::toTransparent( indicatorColor, 0 );

            auto textColor = pal.fillColor.text.primary;
            if ( states & Q::Disabled )
                textColor = pal.fillColor.text.disabled;

            QRgb panelBorderColor; 
            if ( states & ( Q::Disabled | Q::Pressed ) )
                panelBorderColor = pal.strokeColor.controlStrong.disabled;
            else
                panelBorderColor = pal.strokeColor.controlStrong.defaultColor;

            auto panelColor = pal.fillColor.accent.defaultColor;

            if ( states == A::NoState )
            {
                panelColor = pal.fillColor.controlAlt.secondary;
            }
            else if ( states == Q::Selected )
            {
            }
            else if ( states == Q::Hovered )
            {
                panelColor = pal.fillColor.controlAlt.tertiary;
            }
            else if ( states == ( Q::Hovered | Q::Selected ) )
            {
                panelColor = pal.fillColor.accent.secondary;
            }
            else if ( states == Q::Pressed )
            {
                panelColor = pal.fillColor.controlAlt.quaternary;
            }
            else if ( states == ( Q::Pressed | Q::Selected ) )
            {
                panelColor = pal.fillColor.accent.tertiary;
            }
            else if ( states == Q::Disabled )
            {
                panelColor = pal.fillColor.controlAlt.disabled;
            }
            else if ( states == ( Q::Disabled | Q::Selected ) )
            {
                panelColor = pal.fillColor.accent.disabled;
            }

            const auto panel = Q::CheckIndicatorPanel | section | states;
            const auto indicator = Q::CheckIndicator | section | states;
            const auto text = Q::Text | section | states;


#if 0
            // we have different colors when making colors solid early. TODO ...
            panelColor = rgbSolid2( panelColor, pal.background.solid.base );
            indicatorColor = rgbSolid2( indicatorColor, pal.background.solid.base );
#endif
            setBoxBorderGradient( indicator, pal.elevation.circle.border, panelColor );

            setGradient( panel, panelColor );
            setBoxBorderColors( panel, panelBorderColor );

            setGradient( indicator, indicatorColor );

            setColor( text, textColor );
        }
    }
}

void Editor::setupScrollViewMetrics()
{
}

void Editor::setupScrollViewColors( QskAspect::Section, const QskFluent2Theme& )
{
}

void Editor::setupSegmentedBarMetrics()
{
    using Q = QskSegmentedBar;
    using A = QskAspect;

    const QSizeF segmentStrutSize( 120, 32 );

    setBoxBorderMetrics( Q::Panel, 1 );
    setBoxBorderMetrics( Q::Panel | Q::Selected | Q::Disabled, 0 );
    setSpacing( Q::Panel, 8 );

    setStrutSize( Q::Icon, { 12, 12 } );

    setFontRole( Q::Text, QskFluent2Skin::Body );

    setStrutSize( Q::Segment | A::Horizontal, segmentStrutSize );
    setStrutSize( Q::Segment | A::Vertical, segmentStrutSize.transposed() );
    setBoxShape( Q::Segment, 4 );
    setPadding( Q::Segment, { 8, 0, 8, 0 } );
}

void Editor::setupSegmentedBarColors(
    QskAspect::Section section, const QskFluent2Theme& theme )
{
    using Q = QskSegmentedBar;
    using A = QskAspect;
    using W = QskFluent2Skin;

    const auto& pal = theme.palette;

    auto panelColor = pal.fillColor.control.defaultColor;
    panelColor = rgbSolid2( panelColor, pal.background.solid.primary );

    setGradient( Q::Panel, panelColor );

    for ( const auto state1 : { A::NoState, Q::Hovered, Q::Disabled } )
    {
        for ( const auto state2 : { A::NoState, Q::Selected } )
        {
            const auto states = state1 | state2;

            QRgb segmentColor, borderColor1, borderColor2, textColor;
            int graphicRole;

            if ( states == A::NoState )
            {
                segmentColor = pal.fillColor.control.defaultColor;
                borderColor1 = pal.elevation.control.border[0];
                borderColor2 = pal.elevation.control.border[1];
                textColor = pal.fillColor.text.primary;

                graphicRole = W::GraphicRoleFillColorTextPrimary;
            }
            else if ( states & Q::Hovered )
            {
                segmentColor = pal.fillColor.control.secondary;
                borderColor1 = pal.elevation.control.border[0];
                borderColor2 = pal.elevation.control.border[1];
                textColor = pal.fillColor.text.primary;

                graphicRole = W::GraphicRoleFillColorTextPrimary;
            }
            else if ( states == ( Q::Selected | Q::Disabled ) )
            {
                segmentColor = pal.fillColor.accent.disabled;
                borderColor1 = borderColor2 = pal.strokeColor.control.defaultColor;
                textColor = pal.fillColor.textOnAccent.disabled;

                graphicRole = W::GraphicRoleFillColorTextOnAccentDisabled;
            }
            else if ( states & Q::Selected )
            {
                segmentColor = pal.fillColor.accent.defaultColor;
                borderColor1 = pal.elevation.control.border[0];
                borderColor2 = pal.elevation.control.border[1];
                textColor = pal.fillColor.textOnAccent.primary;

                graphicRole = W::GraphicRoleFillColorTextOnAccentPrimary;
            }
            else if ( states == Q::Disabled )
            {
                segmentColor = pal.fillColor.control.disabled;
                borderColor1 = borderColor2 = pal.strokeColor.control.defaultColor;
                textColor = pal.fillColor.text.disabled;
                graphicRole = W::GraphicRoleFillColorTextDisabled;
            }

            const auto segment = Q::Segment | section | states;
            const auto text = Q::Text | section | states;
            const auto icon = Q::Icon | section | states;

            segmentColor = rgbSolid2( segmentColor, pal.background.solid.primary );

            setGradient( segment, segmentColor );
            setBoxBorderGradient( segment, borderColor1, borderColor2, panelColor );

            setColor( text, textColor );

            setGraphicRole( icon, graphicRole );
        }
    }
}

void Editor::setupSeparatorMetrics()
{
    using A = QskAspect;
    using Q = QskSeparator;

    setMetric( Q::Panel | A::Size, 1 );
    setBoxShape( Q::Panel, 0 );
    setBoxBorderMetrics( Q::Panel, 0 );
}

void Editor::setupSeparatorColors(
    QskAspect::Section, const QskFluent2Theme& theme )
{
    using Q = QskSeparator;

    const auto& pal = theme.palette;
    setGradient( Q::Panel, pal.strokeColor.divider.defaultColor );
}

void Editor::setupSliderMetrics()
{
    using Q = QskSlider;
    using A = QskAspect;

    const qreal extent = 22;
    setMetric( Q::Panel | A::Size, extent );
    setBoxShape( Q::Panel, 0 );
    setBoxBorderMetrics( Q::Panel, 0 );

    setPadding( Q::Panel | A::Horizontal, QskMargins( 0.5 * extent, 0 ) );
    setPadding( Q::Panel | A::Vertical, QskMargins( 0, 0.5 * extent ) );

    setMetric( Q::Groove | A::Size, 4 );
    setBoxShape( Q::Groove, 100, Qt::RelativeSize );

    setMetric( Q::Fill | A::Size, 4 );
    setBoxShape( Q::Fill, 100, Qt::RelativeSize );

    setStrutSize( Q::Handle, { 22, 22 } );
    setBoxShape( Q::Handle, 100, Qt::RelativeSize );
    setBoxBorderMetrics( Q::Handle, 1 );

    setStrutSize( Q::Ripple, { 12, 12 } );
    setBoxShape( Q::Ripple, 100, Qt::RelativeSize );

    setStrutSize( Q::Ripple | Q::Hovered, { 14, 14 } );

    setStrutSize( Q::Ripple | Q::Pressed, { 10, 10 } );
}

void Editor::setupSliderColors(
    QskAspect::Section, const QskFluent2Theme& theme )
{
    using Q = QskSlider;
    const auto& pal = theme.palette;

    setGradient( Q::Panel, {} );
    setGradient( Q::Groove, pal.fillColor.controlStrong.defaultColor );

    setGradient( Q::Fill, pal.fillColor.accent.defaultColor );
    setGradient( Q::Handle, pal.fillColor.controlSolid.defaultColor );

    setBoxBorderGradient( Q::Handle, pal.elevation.circle.border,
        pal.fillColor.controlSolid.defaultColor );

    setGradient( Q::Ripple, pal.fillColor.accent.defaultColor );
    setGradient( Q::Ripple | Q::Pressed, pal.fillColor.accent.tertiary );

    setGradient( Q::Groove | Q::Disabled, pal.fillColor.controlStrong.disabled );
    setGradient( Q::Fill | Q::Disabled, pal.fillColor.accent.disabled );
    setGradient( Q::Ripple | Q::Disabled, pal.fillColor.controlStrong.disabled );
}

void Editor::setupSpinBoxMetrics()
{
    using Q = QskSpinBox;

    setHint( Q::Panel | QskAspect::Style, Q::ButtonsRight );
    setStrutSize( Q::Panel, { -1, 32 } );
    setBoxBorderMetrics( Q::Panel, 1 );
    setBoxShape( Q::Panel, 3 );
    setPadding( Q::Panel, { 11, 0, 11, 0 } );

    setAlignment( Q::Text, Qt::AlignLeft );
    setFontRole( Q::Text, QskFluent2Skin::Body );

    setPadding( Q::TextPanel, { 11, 5, 0, 0 } );

    setStrutSize( Q::UpPanel, 32, 20 );
    setPadding( Q::UpPanel, { 11, 7, 11, 7 } );

    setStrutSize( Q::DownPanel, 34, 20 );
    setPadding( Q::DownPanel, { 11, 7, 13, 7 } );

    setSymbol( Q::UpIndicator, symbol( "spin-box-arrow-up" ) );
    setSymbol( Q::DownIndicator, symbol( "spin-box-arrow-down" ) );

    // Focused (Pressed doesn't exist yet):
    setBoxBorderMetrics( Q::Panel | Q::Focused, { 1, 1, 1, 2 } );
}

void Editor::setupSpinBoxColors(
    QskAspect::Section, const QskFluent2Theme& theme )
{
    using Q = QskSpinBox;
    const auto& pal = theme.palette;

    setGradient( Q::Panel, pal.fillColor.control.defaultColor );
    setBoxBorderGradient( Q::Panel,
        pal.elevation.control.border, pal.fillColor.control.defaultColor );

    setColor( Q::Text, pal.fillColor.text.primary );

    setGraphicRole( Q::UpIndicator, QskFluent2Skin::GraphicRoleFillColorTextSecondary );
    setGraphicRole( Q::DownIndicator, QskFluent2Skin::GraphicRoleFillColorTextSecondary );

    // Hovered:

    setGradient( Q::Panel | Q::Hovered, pal.fillColor.control.secondary );
    setBoxBorderGradient( Q::Panel | Q::Hovered,
        pal.elevation.textControl.border, pal.fillColor.control.secondary );

    // Focused (Pressed doesn't exist yet):

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

void Editor::setupTabBarMetrics()
{
}

void Editor::setupTabBarColors( QskAspect::Section, const QskFluent2Theme& theme )
{
    setGradient( QskTabBar::Panel, theme.palette.background.solid.primary );
}

void Editor::setupTabButtonMetrics()
{
    using Q = QskTabButton;

    setStrutSize( Q::Panel, { -1, 31 } );
    setPadding( Q::Panel, { 7, 0, 7, 0 } );
    setBoxShape( Q::Panel, { 7, 7, 0, 0 } );

    setAlignment( Q::Text, Qt::AlignLeft | Qt::AlignVCenter );

    setBoxBorderMetrics( Q::Panel, { 0, 0, 0, 1 } );
    setBoxBorderMetrics( Q::Panel | Q::Checked, { 1, 1, 1, 0 } );

    setFontRole( Q::Text, QskFluent2Skin::Body );
    setFontRole( Q::Text | Q::Checked, QskFluent2Skin::BodyStrong );
}

void Editor::setupTabButtonColors(
    QskAspect::Section section, const QskFluent2Theme& theme )
{
    using Q = QskTabButton;
    const auto& pal = theme.palette;

    for ( const auto state : { QskAspect::NoState, 
        Q::Checked, Q::Hovered, Q::Pressed, Q::Disabled } )
    {
        QRgb panelColor, textColor;

        if ( state == Q::Checked )
        {
            panelColor = pal.background.solid.secondary;
            textColor = pal.fillColor.text.primary;
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

        const auto panel = Q::Panel | section | state;
        const auto text = Q::Text | section | state;

        panelColor = rgbSolid2( panelColor, pal.background.solid.primary );
        setGradient( panel, panelColor );

        const auto borderColor = rgbSolid2(
            pal.strokeColor.tab.defaultColor, pal.background.solid.primary );

        setBoxBorderColors( panel, borderColor );

        setColor( text, textColor );
    }
}

void Editor::setupTabViewMetrics()
{
}

void Editor::setupTabViewColors(
    QskAspect::Section section, const QskFluent2Theme& theme )
{
    using Q = QskTabView;
    setGradient( Q::Page | section, theme.palette.background.solid.secondary );
}

void Editor::setupTextLabelMetrics()
{
    using Q = QskTextLabel;

    setPadding( Q::Panel, 10 );
    setFontRole( Q::Text, QskFluent2Skin::Body );
}

void Editor::setupTextLabelColors(
    QskAspect::Section, const QskFluent2Theme& theme )
{
    using Q = QskTextLabel;
    const auto& pal = theme.palette;

    setColor( Q::Text, pal.fillColor.text.primary );
}

void Editor::setupTextInputMetrics()
{
    using Q = QskTextInput;

    setStrutSize( Q::Panel, { -1, 30 } );
    setPadding( Q::Panel, { 11, 0, 11, 0 } );

    setBoxBorderMetrics( Q::Panel, 1 );
    for( const auto& state : { Q::Focused, Q::Editing } )
        setBoxBorderMetrics( Q::Panel | state, { 1, 1, 1, 2 } );

    setBoxShape( Q::Panel, 3 );

    setAlignment( Q::Text, Qt::AlignLeft | Qt::AlignVCenter );
    setFontRole( Q::Text, QskFluent2Skin::Body );
}

void Editor::setupTextInputColors(
    QskAspect::Section, const QskFluent2Theme& theme )
{
    using Q = QskTextInput;
    const auto& pal = theme.palette;

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

void Editor::setupSwitchButtonMetrics()
{
    using Q = QskSwitchButton;
    using A = QskAspect;

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

    setStrutSize( Q::Handle, 12, 12 );

    setStrutSize( Q::Handle | Q::Hovered, 14, 14,
        { QskStateCombination::CombinationNoState, Q::Checked } );

    const QSizeF pressedSize( 17, 14 );

    setStrutSize( Q::Handle | Q::Pressed | A::Horizontal,
        pressedSize, { QskStateCombination::CombinationNoState, Q::Checked }  );

    setStrutSize( Q::Handle | Q::Pressed | A::Vertical,
        pressedSize.transposed(), { QskStateCombination::CombinationNoState, Q::Checked }  );

    setStrutSize( Q::Handle | Q::Disabled, 12, 12,
        { QskStateCombination::CombinationNoState, Q::Checked } );

    setBoxBorderMetrics( Q::Handle | Q::Disabled | Q::Checked, 1 );
}

void Editor::setupSwitchButtonColors(
    QskAspect::Section, const QskFluent2Theme& theme )
{
    using Q = QskSwitchButton;

    const auto& pal = theme.palette;

    setGradient( Q::Groove, pal.fillColor.controlAlt.secondary );
    setGradient( Q::Groove | Q::Checked, pal.fillColor.accent.defaultColor );
    setBoxBorderColors( Q::Groove, pal.strokeColor.controlStrong.defaultColor );

    setGradient( Q::Handle, pal.strokeColor.controlStrong.defaultColor );
    setGradient( Q::Handle | Q::Checked, pal.fillColor.textOnAccent.primary );

    setBoxBorderGradient( Q::Handle | Q::Checked,
        pal.elevation.circle.border, pal.fillColor.accent.defaultColor );

    setGradient( Q::Groove | Q::Hovered, pal.fillColor.controlAlt.tertiary );
    setGradient( Q::Groove | Q::Hovered | Q::Checked, pal.fillColor.accent.secondary );
    setBoxBorderColors( Q::Groove | Q::Hovered, pal.fillColor.text.secondary );

    setGradient( Q::Handle | Q::Hovered, pal.fillColor.text.secondary );

    setBoxBorderGradient( Q::Handle | Q::Hovered | Q::Checked,
        pal.elevation.circle.border, pal.fillColor.accent.secondary );

    setGradient( Q::Groove | Q::Pressed, pal.fillColor.controlAlt.quaternary );
    setGradient( Q::Groove | Q::Pressed | Q::Checked, pal.fillColor.accent.tertiary );
    setBoxBorderColors( Q::Groove | Q::Pressed, pal.strokeColor.controlStrong.defaultColor );

    setGradient( Q::Handle | Q::Pressed, pal.strokeColor.controlStrong.defaultColor );

    setBoxBorderGradient( Q::Handle | Q::Pressed | Q::Checked,
        pal.elevation.circle.border, pal.fillColor.accent.tertiary );

    setGradient( Q::Groove | Q::Disabled, pal.fillColor.controlAlt.disabled );
    setBoxBorderColors( Q::Groove | Q::Disabled, pal.fillColor.text.disabled );
    setGradient( Q::Groove | Q::Disabled | Q::Checked, pal.fillColor.accent.disabled );
    setBoxBorderColors( Q::Groove | Q::Disabled | Q::Checked, pal.fillColor.accent.disabled );

    setGradient( Q::Handle | Q::Disabled, pal.fillColor.text.disabled );
    setGradient( Q::Handle | Q::Disabled | Q::Checked, pal.fillColor.textOnAccent.disabled );
}

void Editor::setupSubWindow( const QskFluent2Theme& theme )
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

void Editor::setupVirtualKeyboardMetrics()
{
    using Q = QskVirtualKeyboard;

    setMargin( Q::ButtonPanel, 2 );
    setFontRole( Q::ButtonText, QskFluent2Skin::BodyLarge );
    setPadding( Q::Panel, 8 );
}

void Editor::setupVirtualKeyboardColors(
    QskAspect::Section, const QskFluent2Theme& theme )
{
    using Q = QskVirtualKeyboard;

    const auto& pal = theme.palette;

    setGradient( Q::ButtonPanel, pal.fillColor.control.defaultColor );
    setGradient( Q::ButtonPanel | Q::Hovered, pal.fillColor.control.secondary );
    setGradient( Q::ButtonPanel | QskPushButton::Pressed, pal.fillColor.control.tertiary );

    setColor( Q::ButtonText, pal.fillColor.text.primary );
    setColor( Q::ButtonText | QskPushButton::Pressed, pal.fillColor.text.secondary );

    setGradient( Q::Panel, pal.background.solid.tertiary );
}

QskFluent2Skin::QskFluent2Skin( QObject* parent )
    : Inherited( parent )
{
    setupFonts();

    Editor editor( &hintTable() );
    editor.setupMetrics();
}

void QskFluent2Skin::addTheme( QskAspect::Section section, const QskFluent2Theme& theme )
{
    if ( section == QskAspect::Body )
    {
        // design flaw: we can't have section sensitive filters. TODO ..
        setupGraphicFilters( theme );
    }

    Editor editor( &hintTable() );
    editor.setupColors( section, theme );
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
