#include <QApplication>
#include <QDebug>
#include <QKeyEvent>
#include <QResizeEvent>

#include "SimpleView.h"


SimpleView::SimpleView( const QString & photoPath, const QStringList & photoList )
    : QLabel()
    , m_photoPath( photoPath )
    , m_photoList( photoList )
    , m_current( -1 )
{
    setWindowState( Qt::WindowFullScreen );
    setAlignment( Qt::AlignHCenter | Qt::AlignVCenter );
    setPalette( Qt::black );
    resize( 600, 400 );
}


void SimpleView::showPhoto( int index, bool force )
{
    index = qBound( 0, index, m_photoList.size()-1 );

    if ( index == m_current && ! force )
	return;

    QString photoPath = m_photoPath + "/" + m_photoList.at( index );
    // qDebug() << "Showing" << photoPath;
    clear();
    QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
    QPixmap photo( photoPath );
    setWindowTitle( photoPath );
    setPixmap( photo.scaled( this->size(), Qt::KeepAspectRatio, Qt::FastTransformation ) );
    // setPixmap( photo.scaled( this->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation ) );
    QApplication::restoreOverrideCursor();
    m_current = index;
}


void SimpleView::resizeEvent( QResizeEvent * event )
{
    if ( ! event )
	return;

    showPhoto( m_current, true );
}


void SimpleView::keyPressEvent( QKeyEvent * event )
{
    if ( ! event )
	return;

    switch ( event->key() )
    {
	case Qt::Key_PageDown:
	case Qt::Key_Space:
	    showPhoto( m_current+1 );
	    break;

	case Qt::Key_PageUp:
	case Qt::Key_Backspace:
	    showPhoto( m_current-1 );
	    break;

	case Qt::Key_Home:
	    showPhoto( 0 );
	    break;

	case Qt::Key_End:
	    showPhoto( m_photoList.size()-1 );
	    break;

	case Qt::Key_Q:
	case Qt::Key_Escape:
	    qApp->quit();
	    break;

	case Qt::Key_Return:
	    setWindowState( windowState() ^ Qt::WindowFullScreen );
	    break;
	    
	default:
	    QLabel::keyPressEvent( event );
    }
}
