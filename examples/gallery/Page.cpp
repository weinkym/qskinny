/******************************************************************************
 * QSkinny - Copyright (C) The authors
 *           SPDX-License-Identifier: BSD-3-Clause
 *****************************************************************************/

#include "Page.h"
#include <QskRgbValue.h>

Page::Page( QQuickItem* parent )
    : Page( Qt::Vertical, parent )
{
}

Page::Page( Qt::Orientation orientation, QQuickItem* parent )
    : QskLinearBox( orientation, parent )
{
    setMargins( 20 );
    setPadding( 10 );
    setSpacing( 10 );

    if ( !qgetenv( "QSK_PAGE_COLOR" ).isEmpty() )
    {
        setPanel( true );
        setGradientHint( Panel, QskRgb::Coral );
        //setGradientHint( Panel, Qt::black );
    }
}
