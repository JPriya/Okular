/***************************************************************************
 *   Copyright (C) 2006 by Chu Xiaodong <xiaodongchu@gmail.com>            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

// qt/kde includes
#include <qframe.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qheaderview.h>
#include <kcolorbutton.h>
#include <kicon.h>
#include <klocale.h>
#include <knuminput.h>

// local includes
#include "core/document.h"
#include "core/page.h"
#include "core/annotations.h"
#include "annotationpropertiesdialog.h"
#include "annotationwidgets.h"


AnnotsPropertiesDialog::AnnotsPropertiesDialog( QWidget *parent, KPDFDocument *document, int docpage, Annotation *ann )
    : KPageDialog( parent ), m_document( document ), m_page( docpage ), modified( false )
{
    setFaceType( Tabbed );
    m_annot=ann;
    setCaptionTextbyAnnotType();
    setButtons( Ok | Apply | Cancel );
    enableButton( Apply, false );
    connect( this, SIGNAL( applyClicked() ), this, SLOT( slotapply() ) );
    connect( this, SIGNAL( okClicked() ), this, SLOT( slotapply() ) );

    m_annotWidget = AnnotationWidgetFactory::widgetFor( ann );

    QLabel* tmplabel;
  //1. Appearance
    //BEGIN tab1
    QFrame *page = new QFrame();
    addPage( page, i18n( "&Appearance" ) );
    QVBoxLayout * lay = new QVBoxLayout( page );

    QHBoxLayout * hlay = new QHBoxLayout();
    lay->addLayout( hlay );
    tmplabel = new QLabel( i18n( "&Color:" ), page );
    hlay->addWidget( tmplabel );
    colorBn = new KColorButton( page );
    colorBn->setColor( ann->style.color );
    tmplabel->setBuddy( colorBn );
    hlay->addWidget( colorBn );

    hlay = new QHBoxLayout();
    lay->addLayout( hlay );
    tmplabel = new QLabel( i18n( "&Opacity:" ), page );
    hlay->addWidget( tmplabel );
    m_opacity = new KIntNumInput( page );
    m_opacity->setRange( 0, 100, 1, true );
    m_opacity->setValue( (int)( ann->style.opacity * 100 ) );
    tmplabel->setBuddy( m_opacity );
    hlay->addWidget( m_opacity );

    if ( m_annotWidget )
        lay->addWidget( m_annotWidget->widget() );

    lay->addItem( new QSpacerItem( 5, 5, QSizePolicy::Fixed, QSizePolicy::Expanding ) );
    //END tab1
    
    //BEGIN tab 2
    page = new QFrame();
    addPage( page, i18n( "&General" ) );
//    m_tabitem[1]->setIcon( KIcon( "fonts" ) );
    QGridLayout * gridlayout = new QGridLayout( page );
    tmplabel = new QLabel( i18n( "&Author:" ), page );
    AuthorEdit = new QLineEdit( ann->author, page );
    tmplabel->setBuddy( AuthorEdit );
    gridlayout->addWidget( tmplabel, 0, 0 );
    gridlayout->addWidget( AuthorEdit, 0, 1 );
    
    tmplabel = new QLabel( i18n( "Created:" ), page );
    gridlayout->addWidget( tmplabel, 1, 0 );
    tmplabel = new QLabel( KGlobal::locale()->formatDateTime( ann->creationDate, false, true ), page );//time
    gridlayout->addWidget( tmplabel, 1, 1 );
    
    tmplabel = new QLabel( i18n( "Modified:" ), page );
    gridlayout->addWidget( tmplabel, 2, 0 );
    tmplabel = new QLabel( KGlobal::locale()->formatDateTime( ann->modifyDate, false, true ), page );//time
    gridlayout->addWidget( tmplabel, 2, 1 );

    gridlayout->addItem( new QSpacerItem( 5, 5, QSizePolicy::Fixed, QSizePolicy::Expanding ), 3, 0 );
    //END tab 2
    //BEGIN advance properties:
    page = new QFrame();
    addPage( page, i18n( "&Advanced" ) );
    gridlayout = new QGridLayout( page );
    
    tmplabel = new QLabel( i18n( "uniqueName:" ), page );
    gridlayout->addWidget( tmplabel, 0, 0 );
    uniqueNameEdit = new QLineEdit( ann->uniqueName, page );
    gridlayout->addWidget( uniqueNameEdit, 0, 1 );
    
    tmplabel = new QLabel( i18n( "contents:" ), page );
    gridlayout->addWidget( tmplabel, 1, 0 );
    contentsEdit = new QLineEdit( ann->contents, page );
    gridlayout->addWidget( contentsEdit, 1, 1 );

    tmplabel = new QLabel( i18n( "flags:" ), page );
    gridlayout->addWidget( tmplabel, 2, 0 );
    flagsEdit = new QLineEdit( QString::number( m_annot->flags ), page );
    gridlayout->addWidget( flagsEdit, 2, 1 );

    QString tmpstr = QString( "%1,%2,%3,%4" ).arg( m_annot->boundary.left ).arg( m_annot->boundary.top ).arg( m_annot->boundary.right ).arg( m_annot->boundary.bottom );
    tmplabel = new QLabel( i18n( "boundary:" ), page );
    gridlayout->addWidget( tmplabel, 3, 0 );
    boundaryEdit = new QLineEdit( tmpstr, page );
    boundaryEdit->setReadOnly( true );
    gridlayout->addWidget( boundaryEdit, 3, 1 );

    gridlayout->addItem( new QSpacerItem( 5, 5, QSizePolicy::Fixed, QSizePolicy::Expanding ), 4, 0 );
    //END advance

    //BEGIN connections
    connect( colorBn, SIGNAL( changed( const QColor& ) ), this, SLOT( setModified() ) );
    connect( m_opacity, SIGNAL( valueChanged( int ) ), this, SLOT( setModified() ) );
    connect( AuthorEdit, SIGNAL( textChanged ( const QString& ) ), this, SLOT( setModified() ) );
    connect( uniqueNameEdit, SIGNAL( textChanged ( const QString& ) ), this, SLOT( setModified() ) );
    connect( contentsEdit, SIGNAL( textChanged ( const QString& ) ), this, SLOT( setModified() ) );
    connect( flagsEdit, SIGNAL( textChanged ( const QString& ) ), this, SLOT( setModified() ) );
    if ( m_annotWidget )
    {
        connect( m_annotWidget, SIGNAL( dataChanged() ), this, SLOT( setModified() ) );
    }
    //END

    resize( sizeHint() );
}
AnnotsPropertiesDialog::~AnnotsPropertiesDialog()
{
}


void AnnotsPropertiesDialog::setCaptionTextbyAnnotType()
{
    Annotation::SubType type=m_annot->subType();
    QString captiontext;
    switch(type)
    {
        case Annotation::AText:
            if(((TextAnnotation*)m_annot)->textType==TextAnnotation::Linked)
                captiontext = i18n( "Note Properties" );
            else
                captiontext = i18n( "FreeText Properties" );
            break;
        case Annotation::ALine:
            captiontext = i18n( "Line Properties" );
            break;
        case Annotation::AGeom:
            captiontext = i18n( "Geom Properties" );
            break;
        case Annotation::AHighlight:
            captiontext = i18n( "Highlight Properties" );
            break;
        case Annotation::AStamp:
            captiontext = i18n( "Stamp Properties" );
            break;
        case Annotation::AInk:
            captiontext = i18n( "Ink Properties" );
            break;
        default:
            captiontext = i18n( "Annotation Properties" );
            break;
    }
        setCaption( captiontext );
}

void AnnotsPropertiesDialog::setModified()
{
    modified = true;
    enableButton( Apply, true );
}

void AnnotsPropertiesDialog::slotapply()
{
    if ( !modified )
        return;

    m_annot->author=AuthorEdit->text();
    m_annot->contents=contentsEdit->text();
    m_annot->style.color = colorBn->color();
    m_annot->modifyDate=QDateTime::currentDateTime();
    m_annot->flags=flagsEdit->text().toInt();
    m_annot->style.opacity = (double)m_opacity->value() / 100.0;

    if ( m_annotWidget )
        m_annotWidget->applyChanges();

    m_document->modifyPageAnnotation( m_page, m_annot );

    modified = false;
    enableButton( Apply, false );
}
    
#include "annotationpropertiesdialog.moc"
    
