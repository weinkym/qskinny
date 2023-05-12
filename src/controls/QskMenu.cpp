/******************************************************************************
 * QSkinny - Copyright (C) 2016 Uwe Rathmann
 *           SPDX-License-Identifier: BSD-3-Clause
 *****************************************************************************/

#include "QskMenu.h"

#include "QskGraphicProvider.h"
#include "QskLabelData.h"
#include "QskTextOptions.h"
#include "QskGraphic.h"
#include "QskColorFilter.h"
#include "QskSkinlet.h"
#include "QskEvent.h"
#include "QskPlatform.h"

#include <qvector.h>
#include <qvariant.h>
#include <qeventloop.h>

QSK_SUBCONTROL( QskMenu, Overlay )
QSK_SUBCONTROL( QskMenu, Panel )
QSK_SUBCONTROL( QskMenu, Segment )
QSK_SUBCONTROL( QskMenu, Cursor )
QSK_SUBCONTROL( QskMenu, Text )
QSK_SUBCONTROL( QskMenu, Icon )
QSK_SUBCONTROL( QskMenu, Separator )

QSK_SYSTEM_STATE( QskMenu, Selected, QskAspect::FirstSystemState << 2 )

QVector< int > qskSeparators( const QVector< QskLabelData >& options )
{
    QVector< int > separators;

    for ( int i = 0; i < options.count(); i++ )
    {
        if ( options[i].isEmpty() )
            separators += i;
    }

    return separators;
}

class QskMenu::PrivateData
{
  public:
    QPointF origin;

    QVector< QskLabelData > options;
    QVector< int > separators;

    int triggeredIndex = -1;
    int currentIndex = -1;
    bool isPressed = false;
};

QskMenu::QskMenu( QQuickItem* parent )
    : Inherited( parent )
    , m_data( new PrivateData )
{
    setModal( true );
    setFaderAspect( QskMenu::Panel | QskAspect::Position | QskAspect::Metric );

    setPopupFlag( QskPopup::CloseOnPressOutside, true );
    setPopupFlag( QskPopup::DeleteOnClose, true );

    setSubcontrolProxy( Inherited::Overlay, Overlay );

    initSizePolicy( QskSizePolicy::Fixed, QskSizePolicy::Fixed );

    // we hide the focus indicator while fading
    connect( this, &QskMenu::fadingChanged, this,
        &QskControl::focusIndicatorRectChanged );

    connect( this, &QskMenu::opened, this,
        [this]() { m_data->triggeredIndex = -1; } );
}

QskMenu::~QskMenu()
{
}

#if 1

// has no effect as we do not offer submenus yet. TODO ...
bool QskMenu::isCascading() const
{
    return flagHint( QskMenu::Panel | QskAspect::Style, qskMaybeDesktopPlatform() );
}

void QskMenu::setCascading( bool on )
{
    if ( setFlagHint( QskMenu::Panel | QskAspect::Style, on ) )
        Q_EMIT cascadingChanged( on );
}

void QskMenu::resetCascading()
{
    if ( resetSkinHint( QskMenu::Panel | QskAspect::Style ) )
        Q_EMIT cascadingChanged( isCascading() );
}

#endif

void QskMenu::setOrigin( const QPointF& origin )
{
    if ( origin != m_data->origin )
    {
        m_data->origin = origin;
        Q_EMIT originChanged( origin );
    }
}

QPointF QskMenu::origin() const
{
    return m_data->origin;
}

void QskMenu::setTextOptions( const QskTextOptions& textOptions )
{
    setTextOptionsHint( Text, textOptions );
}

QskTextOptions QskMenu::textOptions() const
{
    return textOptionsHint( Text );
}

int QskMenu::addOption( const QString& graphicSource, const QString& text )
{
    return addOption( QskLabelData( text, graphicSource ) );
}

int QskMenu::addOption( const QUrl& graphicSource, const QString& text )
{
    return addOption( QskLabelData( text, graphicSource ) );
}

int QskMenu::addOption( const QskLabelData& option )
{
    m_data->options += option;

    if ( option.isEmpty() )
        m_data->separators += m_data->options.count() - 1;

    resetImplicitSize();
    update();

    if ( isComponentComplete() )
        Q_EMIT optionsChanged();

    return m_data->options.count() - 1;
}

void QskMenu::setOptions( const QStringList& options )
{
    setOptions( qskCreateLabelData( options ) );
}

void QskMenu::setOptions( const QVector< QskLabelData >& options )
{
    m_data->options = options;
    m_data->separators = qskSeparators( options );

    if ( m_data->currentIndex >= 0 )
    {
        m_data->currentIndex = -1;

        if ( isComponentComplete() )
            Q_EMIT currentIndexChanged( m_data->currentIndex );
    }

    resetImplicitSize();
    update();

    if ( isComponentComplete() )
        Q_EMIT optionsChanged();
}

void QskMenu::clear()
{
    setOptions( QVector< QskLabelData >() );
}

QVector< QskLabelData > QskMenu::options() const
{
    return m_data->options;
}

QskLabelData QskMenu::optionAt( int index ) const
{
    return m_data->options.value( index );
}

int QskMenu::optionsCount() const
{
    return m_data->options.count();
}

void QskMenu::addSeparator()
{
    addOption( QskLabelData() );
}

QVector< int > QskMenu::separators() const
{
    return m_data->separators;
}

int QskMenu::currentIndex() const
{
    return m_data->currentIndex;
}

void QskMenu::setCurrentIndex( int index )
{
    if( index < 0 || index >= m_data->options.count() )
    {
        index = -1;
    }
    else
    {
        if ( m_data->options[index].isEmpty() ) // a seperator
            index = -1;
    }

    if( index != m_data->currentIndex )
    {
        setPositionHint( Cursor, index );

        m_data->currentIndex = index;
        update();

        Q_EMIT currentIndexChanged( index );
        Q_EMIT focusIndicatorRectChanged();
    }
}

QString QskMenu::currentText() const
{
    return optionAt( m_data->currentIndex ).text();
}

int QskMenu::triggeredIndex() const
{
    return m_data->triggeredIndex;
}

QString QskMenu::triggeredText() const
{
    return optionAt( m_data->triggeredIndex ).text();
}

void QskMenu::keyPressEvent( QKeyEvent* event )
{
    if( m_data->currentIndex < 0 )
        return;

    int key = event->key();

    switch( key )
    {
        case Qt::Key_Up:
        {
            traverse( -1 );
            break;
        }

        case Qt::Key_Down:
        {
            traverse( 1 );
            break;
        }

        case Qt::Key_Select:
        case Qt::Key_Space:
        case Qt::Key_Return:
        case Qt::Key_Enter:
        {
            m_data->isPressed = true;
            return;
        }

        case Qt::Key_Escape:
        case Qt::Key_Cancel:
        {
            close();
            return;
        }

        default:
        {
            if ( const int steps = qskFocusChainIncrement( event ) )
                traverse( steps );
        }
    }
}

void QskMenu::keyReleaseEvent( QKeyEvent* )
{
    if( m_data->isPressed )
    {
        m_data->isPressed = false;

        if ( m_data->currentIndex >= 0 )
        {
            close();
            trigger( m_data->currentIndex );
        }
    }
}

#ifndef QT_NO_WHEELEVENT

void QskMenu::wheelEvent( QWheelEvent* event )
{
    const auto steps = qskWheelSteps( event );
    traverse( -steps );
}

#endif

void QskMenu::traverse( int steps )
{
    const auto count = m_data->options.count();

    const auto n = count - m_data->separators.count();
    if ( ( n <= 0 ) || ( steps % n == 0 ) )
        return;

    auto index = m_data->currentIndex;
    for ( auto i : m_data->separators )
    {
        if ( i < index )
            index--;
    }

    index += steps;

    auto newIndex = index % n;
    if ( newIndex < 0 )
        newIndex += n;

    for ( int i = 0; i < newIndex; i++)
    {
        if ( m_data->options[i].isEmpty() )
            newIndex++;
    }

    // when cycling we want slide in

    int startIndex = m_data->currentIndex;

    if ( index < 0 )
        startIndex = count;
    else if ( index >= n )
        startIndex = -1;

    movePositionHint( Cursor, startIndex, newIndex );
    setCurrentIndex( newIndex );
}

void QskMenu::mousePressEvent( QMouseEvent* event )
{
    // QGuiApplication::styleHints()->setFocusOnTouchRelease ??

    if ( event->button() == Qt::LeftButton )
    {
        const auto index = indexAtPosition( qskMousePosition( event ) );
        if ( index >= 0 )
        {
            setCurrentIndex( index );
            m_data->isPressed = true;
        }

        return;
    }

    Inherited::mousePressEvent( event );
}

void QskMenu::mouseUngrabEvent()
{
    m_data->isPressed = false;
    Inherited::mouseUngrabEvent();
}

void QskMenu::mouseReleaseEvent( QMouseEvent* event )
{
    if ( event->button() == Qt::LeftButton )
    {
        if( m_data->isPressed )
        {
            m_data->isPressed = false;

            const auto index = m_data->currentIndex;

            if ( ( index >= 0 )
                && ( index == indexAtPosition( qskMousePosition( event ) ) ) )
            {
                close();
                trigger( m_data->currentIndex );
            }
        }

        return;
    }

    Inherited::mouseReleaseEvent( event );
}

void QskMenu::aboutToShow()
{
    setGeometry( QRectF( m_data->origin, sizeConstraint() ) );

    if ( m_data->currentIndex < 0 )
        setCurrentIndex( 0 );

    Inherited::aboutToShow();
}

QRectF QskMenu::focusIndicatorRect() const
{
    if ( isFading() )
        return QRectF();

    if( currentIndex() >= 0 )
    {
        return effectiveSkinlet()->sampleRect( this,
            contentsRect(), Segment, currentIndex() );
    }

    return Inherited::focusIndicatorRect();
}

QRectF QskMenu::cellRect( int index ) const
{
    return effectiveSkinlet()->sampleRect(
        this, contentsRect(), QskMenu::Segment, index );
}

int QskMenu::indexAtPosition( const QPointF& pos ) const
{
    return effectiveSkinlet()->sampleIndexAt(
        this, contentsRect(), QskMenu::Segment, pos );
}

void QskMenu::trigger( int index )
{
    if ( index >= 0 && index < m_data->options.count()  )
    {
        m_data->triggeredIndex = index;
        Q_EMIT triggered( index );
    }
}

int QskMenu::exec()
{
    (void) execPopup();
    return m_data->triggeredIndex;
}

#include "moc_QskMenu.cpp"
