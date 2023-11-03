/******************************************************************************
 * QSkinny - Copyright (C) 2016 Uwe Rathmann
 *           SPDX-License-Identifier: BSD-3-Clause
 *****************************************************************************/

#include "SkinnyShortcut.h"

#include <QskRgbValue.h>
#include <QskControl.h>
#include <QskDrawer.h>
#include <QskPushButton.h>
#include <QskFocusIndicator.h>
#include <QskWindow.h>
#include <QskEvent.h>
#include <QskAnimationHint.h>

#include <QGuiApplication>

namespace
{
    class Drawer : public QskDrawer
    {
      public:
        Drawer( Qt::Edge edge,  QQuickItem* parent )
            : QskDrawer( parent )
        {
            setEdge( edge );

            auto content = new QskControl( this );
            content->setObjectName( "Content" );
            content->setAutoLayoutChildren( true );
            content->setMargins( 20 );

            auto button = new QskPushButton( "Push Me", content );
            button->setSizePolicy( QskSizePolicy::Fixed, QskSizePolicy::Fixed );
            button->setLayoutAlignmentHint( Qt::AlignCenter );

            const auto size = content->sizeHint();

            switch( edge )
            {
                case Qt::LeftEdge:
                    setPanel( QskRgb::Tomato );
                    break;

                case Qt::RightEdge:
                    setPanel( QskRgb::Orchid );
                    content->setFixedWidth( 1.5 * size.width() );
                    break;

                case Qt::TopEdge:
                    setPanel( QskRgb::Chartreuse );
                    break;

                case Qt::BottomEdge:
                    setPanel( QskRgb::Wheat );
                    content->setFixedHeight( 2 * size.height() );
                    break;
            }
        }

      private:
        void setPanel( const QColor& color )
        {
            setGradientHint( Panel, color );
        }
    };

    class DrawerBox : public QskControl
    {
      public:
        DrawerBox( QQuickItem* parent = nullptr )
            : QskControl( parent )
        {
            setBackgroundColor( QskRgb::LightSteelBlue );

            setMargins( 10 );
            setAutoLayoutChildren( true );

            for ( int i = 0; i < 4; i++ )
            {
                const auto edge = static_cast< Qt::Edge >( 1 << i );

                auto dragMargin = 30; // the default setting is pretty small
                if ( edge == Qt::TopEdge )
                {
                    // to check if dragging works above the button
                    dragMargin = 120;
                }

                auto drawer = new Drawer( edge, this );
                drawer->setDragMargin( dragMargin );

                connect( drawer, &QskPopup::openChanged,
                    this, &DrawerBox::setDrawersLocked );
                
                m_drawers[i] = drawer;
            }

            auto button = new QskPushButton( "Push Me", this );
            button->setPreferredHeight( 100 );
        }

      private:
        void setDrawersLocked( bool locked )
        {
            for ( auto drawer : m_drawers )
            {
                if ( !drawer->isOpen() )
                    drawer->setInteractive( !locked );
            }
        }

        Drawer* m_drawers[4];
    };

    class MainBox : public QskControl
    {
      public:
        MainBox( QQuickItem* parent = nullptr )
            : QskControl( parent )
        {
            setMargins( 40 );
            setAutoLayoutChildren( true );

            ( void ) new DrawerBox( this );
        }
    };
}

int main( int argc, char* argv[] )
{
    QGuiApplication app( argc, argv );

    SkinnyShortcut::enable( SkinnyShortcut::AllShortcuts );

    QskWindow window;
    window.addItem( new QskFocusIndicator() );
    window.addItem( new MainBox() );
    window.resize( 600, 600 );
    window.show();

    return app.exec();
}
