/******************************************************************************
 * QSkinny - Copyright (C) 2016 Uwe Rathmann
 * This file may be used under the terms of the QSkinny License, Version 1.0
 *****************************************************************************/

#include "QskGradient.h"
#include "QskRgbValue.h"

#include <qhashfunctions.h>
#include <qvariant.h>

#include <algorithm>

static void qskRegisterGradient()
{
    qRegisterMetaType< QskGradient >();

#if QT_VERSION < QT_VERSION_CHECK( 6, 0, 0 )
    QMetaType::registerEqualsComparator< QskGradient >();
#endif

    QMetaType::registerConverter< QColor, QskGradient >(
        []( const QColor& color ) { return QskGradient( color ); } );
}

Q_CONSTRUCTOR_FUNCTION( qskRegisterGradient )

static inline QskGradient::Orientation qskOrientation( Qt::Orientation o )
{
    return ( o == Qt::Vertical )
        ? QskGradient::Vertical : QskGradient::Horizontal;
}

static inline bool qskIsGradientValid( const QskGradientStops& stops )
{
    if ( stops.size() < 2 )
        return false;

    if ( stops.first().position() != 0.0 || stops.last().position() != 1.0 )
    {
        return false;
    }

    if ( !stops.first().color().isValid() )
        return false;

    for ( int i = 1; i < stops.size(); i++ )
    {
        if ( stops[ i ].position() < stops[ i - 1 ].position() )
            return false;

        if ( !stops[ i ].color().isValid() )
            return false;
    }

    return true;
}

static inline bool qskIsMonochrome( const QskGradientStops& stops )
{
    for ( int i = 1; i < stops.size(); i++ )
    {
        if ( stops[ i ].color() != stops[ 0 ].color() )
            return false;
    }

    return true;
}

static inline bool qskIsVisible( const QskGradientStops& stops )
{
    for ( const auto& stop : stops )
    {
        const auto& c = stop.color();
        if ( c.isValid() && c.alpha() > 0 )
            return true;
    }

    return false;
}

static inline QColor qskInterpolated(
    const QskGradientStop& s1, const QskGradientStop& s2, qreal pos )
{
    if ( s1.color() == s2.color() )
        return s1.color();

    const qreal ratio = ( pos - s1.position() ) / ( s2.position() - s1.position() );
    return QskRgb::interpolated( s1.color(), s2.color(), ratio );
}

static inline bool qskComparePositions(
    const QskGradientStops& s1, const QskGradientStops& s2 )
{
    if ( s1.count() != s2.count() )
        return false;

    // the first is always at 0.0, the last at 1.0
    for ( int i = 1; i < s1.count() - 1; i++ )
    {
        if ( s1[ i ].position() != s2[ i ].position() )
            return false;
    }

    return true;
}

static inline QskGradientStops qskExpandedStops(
    const QskGradientStops& s1, const QskGradientStops& s2 )
{
    // expand s1 by stops matching to the positions from s2

    if ( qskComparePositions( s1, s2 ) )
        return s1;

    QskGradientStops stops;

    stops += s1.first();

    int i = 1, j = 1;
    while ( ( i < s1.count() - 1 ) || ( j < s2.count() - 1 ) )
    {
        if ( s1[ i ].position() < s2[ j ].position() )
        {
            stops += s1[ i++ ];
        }
        else
        {
            const qreal pos = s2[ j++ ].position();
            stops += QskGradientStop( pos, qskInterpolated( s1[ i - 1 ], s1[ i ], pos ) );
        }
    }

    stops += s1.last();

    return stops;
}

static inline QskGradientStops qskExtractedStops(
    const QskGradientStops& stops, qreal from, qreal to )
{
    QskGradientStops extracted;

    if ( from == to )
        extracted.reserve( 2 );
    else
        extracted.reserve( stops.size() );

    int i = 0;

    if ( from == 0.0 )
    {
        extracted += QskGradientStop( 0.0, stops[i++].color() );
    }
    else
    {
        for ( i = 1; i < stops.count(); i++ )
        {
            if ( stops[i].position() > from )
                break;
        }

        const auto color =
            QskGradientStop::interpolated( stops[i - 1], stops[i], from );

        extracted += QskGradientStop( 0.0, color );
    }

    for ( ; i < stops.count(); i++ )
    {
        const auto& s = stops[i];

        if ( s.position() >= to )
            break;

        const auto pos = ( s.position() - from ) / ( to - from );
        extracted += QskGradientStop( pos, s.color() );
    }

    const auto color = QskGradientStop::interpolated( stops[i - 1], stops[i], to );
    extracted += QskGradientStop( 1.0, color );

    return extracted;
}

static inline QskGradientStops qskGradientStops( const QGradientStops& qtStops )
{
    QskGradientStops stops;
    stops.reserve( qtStops.count() );

    for ( const auto& s : qtStops )
        stops += QskGradientStop( s.first, s.second );

    return stops;
}

static inline QskGradientStops qskColorStops(
    const QRgb* rgb, int count, bool discrete )
{
    QskGradientStops stops;

    if ( discrete )
        stops.reserve( 2 * count - 2 );
    else
        stops.reserve( count );

    stops += QskGradientStop( 0.0, rgb[0] );

    if ( discrete )
    {
        const auto step = 1.0 / count;

        for ( int i = 1; i < count; i++ )
        {
            const qreal pos = i * step;
            stops += QskGradientStop( pos, rgb[i - 1] );
            stops += QskGradientStop( pos, rgb[i] );
        }
    }
    else
    {
        const auto step = 1.0 / ( count - 1 );

        for ( int i = 1; i < count - 1; i++ )
            stops += QskGradientStop( i * step, rgb[i] );
    }

    stops += QskGradientStop( 1.0, rgb[count - 1] );

    return stops;
}

QskGradient::QskGradient( Orientation orientation ) noexcept
    : m_orientation( orientation )
    , m_isDirty( false )
    , m_isValid( false )
    , m_isMonchrome( true )
    , m_isVisible( false )
{
}

QskGradient::QskGradient( const QColor& color )
    : QskGradient( Vertical )
{
    setStops( color );
}

QskGradient::QskGradient( Qt::Orientation orientation,
        const QColor& startColor, const QColor& stopColor )
    : QskGradient( qskOrientation( orientation ), startColor, stopColor )
{
}

QskGradient::QskGradient( Orientation orientation,
        const QColor& startColor, const QColor& stopColor )
    : QskGradient( orientation )
{
    setStops( startColor, stopColor );
}

QskGradient::QskGradient( Qt::Orientation orientation, const QskGradientStops& stops )
    : QskGradient( qskOrientation( orientation ), stops )
{
}

QskGradient::QskGradient( Orientation orientation, const QskGradientStops& stops )
    : QskGradient( orientation )
{
    setStops( stops );
}

QskGradient::QskGradient( Qt::Orientation orientation, QGradient::Preset preset )
    : QskGradient( qskOrientation( orientation ), preset )
{
}

QskGradient::QskGradient( Orientation orientation, QGradient::Preset preset )
    : QskGradient( orientation )
{
    setStops( qskGradientStops( QGradient( preset ).stops() ) );
}

QskGradient::~QskGradient()
{
}

bool QskGradient::operator==( const QskGradient& other ) const noexcept
{
    return ( m_orientation == other.m_orientation ) && ( m_stops == other.m_stops );
}

void QskGradient::updateStatusBits() const
{
    // doing all bits in one loop ?
    m_isValid = qskIsGradientValid( m_stops );

    if ( m_isValid )
    {
        m_isMonchrome = qskIsMonochrome( m_stops );
        m_isVisible = qskIsVisible( m_stops );
    }
    else
    {
        m_isMonchrome = true;
        m_isVisible = false;
    }

    m_isDirty = false;
}

bool QskGradient::isValid() const noexcept
{
    if ( m_isDirty )
        updateStatusBits();

    return m_isValid;
}

bool QskGradient::isMonochrome() const noexcept
{
    if ( m_isDirty )
        updateStatusBits();

    return m_isMonchrome;
}

bool QskGradient::isVisible() const noexcept
{
    if ( m_isDirty )
        updateStatusBits();

    return m_isVisible;
}

void QskGradient::setOrientation( Qt::Orientation orientation ) noexcept
{
    setOrientation( qskOrientation( orientation ) );
}

void QskGradient::setOrientation( Orientation orientation ) noexcept
{
    // does not change m_isDirty
    m_orientation = orientation;
}

void QskGradient::setStops( const QColor& color )
{
    m_stops.clear();
    m_stops.reserve( 2 );

    m_stops.append( QskGradientStop( 0.0, color ) );
    m_stops.append( QskGradientStop( 1.0, color ) );

    m_isDirty = true;
}

void QskGradient::setStops( const QColor& startColor, const QColor& stopColor )
{
    m_stops.clear();
    m_stops.reserve( 2 );

    m_stops.append( QskGradientStop( 0.0, startColor ) );
    m_stops.append( QskGradientStop( 1.0, stopColor ) );

    m_isDirty = true;
}

void QskGradient::setStops( const QskGradientStops& stops )
{
    if ( !stops.isEmpty() && !qskIsGradientValid( stops ) )
    {
        qWarning( "Invalid gradient stops" );
        m_stops.clear();
    }
    else
    {
        m_stops = stops;
    }

    m_isDirty = true;
}

int QskGradient::stopCount() const
{
    return m_stops.count();
}

qreal QskGradient::stopAt( int index ) const
{
    if ( index >= m_stops.size() )
        return -1.0;

    return m_stops[ index ].position();
}

QColor QskGradient::colorAt( int index ) const
{
    if ( index >= m_stops.size() )
        return QColor();

    return m_stops[ index ].color();
}

void QskGradient::setAlpha( int alpha )
{
    for ( auto& stop : m_stops )
    {
        auto c = stop.color();
        if ( c.isValid() && c.alpha() )
        {
            c.setAlpha( alpha );
            stop.setColor( c );
        }
    }

    m_isDirty = true;
}

bool QskGradient::hasStopAt( qreal value ) const noexcept
{
    // better use binary search TODO ...
    for ( auto& stop : m_stops )
    {
        if ( stop.position() == value )
            return true;

        if ( stop.position() > value )
            break;
    }

    return false;
}

void QskGradient::reverse()
{
    if ( isMonochrome() )
        return;

    std::reverse( m_stops.begin(), m_stops.end() );
    for( auto& stop : m_stops )
        stop.setPosition( 1.0 - stop.position() );
}

QskGradient QskGradient::reversed() const
{
    auto gradient = *this;
    gradient.reverse();

    return gradient;
}

QskGradient QskGradient::extracted( qreal from, qreal to ) const
{
    if ( from > to )
        return QskGradient( m_orientation );

    if ( isMonochrome() || ( from <= 0.0 && to >= 1.0 ) )
        return *this;

    from = qMax( from, 0.0 );
    to = qMin( to, 1.0 );

    const auto stops = qskExtractedStops( m_stops, from, to );
    return QskGradient( orientation(), stops );
}

QskGradient QskGradient::interpolated(
    const QskGradient& to, qreal value ) const
{
    if ( !( isValid() && to.isValid() ) )
    {
        if ( !isValid() && !to.isValid() )
            return to;

        qreal progress;
        const QskGradient* gradient;

        if ( to.isValid() )
        {
            progress = value;
            gradient = &to;
        }
        else
        {
            progress = 1.0 - value;
            gradient = this;
        }

        /*
            We interpolate as if the invalid gradient would be
            a transparent version of the valid gradient
         */

        auto stops = gradient->m_stops;
        for ( auto& stop : stops )
        {
            auto c = stop.color();
            c.setAlpha( c.alpha() * progress );

            stop.setColor( c );
        }

        return QskGradient( gradient->orientation(), stops );
    }

    if ( isMonochrome() && to.isMonochrome() )
    {
        const auto c = QskRgb::interpolated(
            m_stops[ 0 ].color(), to.m_stops[ 0 ].color(), value );

        return QskGradient( to.orientation(), c, c );
    }

    if ( isMonochrome() )
    {
        // we can ignore our stops

        const auto c = m_stops[ 0 ].color();

        auto s2 = to.m_stops;
        for ( int i = 0; i < s2.count(); i++ )
        {
            const auto c2 = QskRgb::interpolated( c, s2[ i ].color(), value );
            s2[ i ].setColor( c2 );
        }

        return QskGradient( to.orientation(), s2 );
    }

    if ( to.isMonochrome() )
    {
        // we can ignore the stops of to

        const auto c = to.m_stops[ 0 ].color();

        auto s2 = m_stops;
        for ( int i = 0; i < s2.count(); i++ )
        {
            const auto c2 = QskRgb::interpolated( s2[ i ].color(), c, value );
            s2[ i ].setColor( c2 );
        }

        return QskGradient( orientation(), s2 );
    }

    if ( m_orientation == to.m_orientation )
    {
        /*
            we need to have the same number of stops
            at the same positions
         */

        const auto s1 = qskExpandedStops( m_stops, to.m_stops );
        auto s2 = qskExpandedStops( to.m_stops, m_stops );

        for ( int i = 0; i < s1.count(); i++ )
        {
            const auto c2 = QskRgb::interpolated(
                s1[ i ].color(), s2[ i ].color(), value );

            s2[ i ].setColor( c2 );
        }

        return QskGradient( orientation(), s2 );
    }
    else
    {
        /*
            The interpolation is devided into 2 steps. First we
            interpolate into a monochrome gradient and then change
            the orientation before we continue in direction of the
            final gradient.
         */

        const auto c = m_stops[ 0 ].color();

        if ( value <= 0.5 )
        {
            auto s2 = m_stops;

            for ( int i = 0; i < s2.count(); i++ )
            {
                const auto c2 = QskRgb::interpolated(
                    s2[ i ].color(), c, 2 * value );

                s2[ i ].setColor( c2 );
            }

            return QskGradient( orientation(), s2 );
        }
        else
        {
            auto s2 = to.m_stops;

            for ( int i = 0; i < s2.count(); i++ )
            {
                const auto c2 = QskRgb::interpolated(
                    c, s2[ i ].color(), 2 * ( value - 0.5 ) );

                s2[ i ].setColor( c2 );
            }

            return QskGradient( to.orientation(), s2 );
        }
    }
}

QVariant QskGradient::interpolate(
    const QskGradient& from, const QskGradient& to, qreal progress )
{
    return QVariant::fromValue( from.interpolated( to, progress ) );
}

QskGradientStops QskGradient::colorStops(
    const QVector< QRgb >& rgb, bool discrete )
{
    const int count = rgb.count();

    if ( count == 0 )
        return QskGradientStops();

    if ( count == 1 )
    {
        QskGradientStops stops;
        stops.reserve( 2 );

        stops += QskGradientStop( 0.0, rgb[0] );
        stops += QskGradientStop( 1.0, rgb[0] );

        return stops;
    }

    return qskColorStops( rgb.constData(), count, discrete );
}

QGradientStops QskGradient::qtStops() const
{
    QGradientStops qstops;
    qstops.reserve( m_stops.count() );

    for ( const auto& stop : m_stops )
        qstops += { stop.position(), stop.color() };

    return qstops;
}

void QskGradient::clearStops()
{
    if ( !m_stops.isEmpty() )
    {
        m_stops.clear();
        m_isDirty = true;
    }
}

QskHashValue QskGradient::hash( QskHashValue seed ) const
{
    if ( m_stops.isEmpty() )
        return seed;

    const auto o = orientation();

    auto hash = qHashBits( &o, sizeof( o ), seed );
    for ( const auto& stop : m_stops )
        hash = stop.hash( hash );

    return hash;
}


#ifndef QT_NO_DEBUG_STREAM

#include <qdebug.h>

QDebug operator<<( QDebug debug, const QskGradient& gradient )
{
    QDebugStateSaver saver( debug );
    debug.nospace();

    debug << "Gradient";

    if ( !gradient.isValid() )
    {
        debug << "()";
    }
    else
    {
        debug << "( ";

        if ( gradient.isMonochrome() )
        {
            QskRgb::debugColor( debug, gradient.startColor() );
        }
        else
        {
            const char o[] = { 'H', 'V', 'D' };
            debug << o[ gradient.orientation() ] << ", ";

            if ( gradient.stops().count() == 2 )
            {
                QskRgb::debugColor( debug, gradient.startColor() );
                debug << ", ";
                QskRgb::debugColor( debug, gradient.endColor() );
            }
            else
            {
                const auto& s = gradient.stops();
                for ( int i = 0; i < s.count(); i++ )
                {
                    if ( i != 0 )
                        debug << ", ";

                    debug << s[i];
                }
            }
        }
        debug << " )";
    }

    return debug;
}

#endif

#include "moc_QskGradient.cpp"
