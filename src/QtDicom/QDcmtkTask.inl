/***************************************************************************
 *   Copyright © 2013 by Flux Inc.                                         *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#include <QtCore/QThreadPool>


QDcmtkTask::QDcmtkTask( const Functor * Functor, QObject * parent ) :
	QObject( parent ),
	Functor_( Functor ),
	ran_( false )
{
	setAutoDelete( false );
	Q_ASSERT( Functor_ != NULL );
	
	static const int ResultId = qRegisterMetaType< QDcmtkResult >(
		"QDcmtkResult"
	);
	Q_ASSERT( ResultId > 0 );
}

QDcmtkTask::~QDcmtkTask() {
	if ( ran_ ) {
		QThreadPool::globalInstance()->releaseThread();
	}

	delete Functor_;
}

template< typename P >
QDcmtkTask * QDcmtkTask::create(
	OFCondition ( *f )( P ), P p, QObject * parent
) {
	return new QDcmtkTask( new Functor1< P >( f, p ), parent );
}

template< typename P1, typename P2 >
QDcmtkTask * QDcmtkTask::create(
	OFCondition ( *f )( P1, P2 ), P1 p1, P2 p2, QObject * parent
) {
	return new QDcmtkTask( new Functor2< P1, P2 >( f, p1, p2 ), parent );
}

template< typename P1, typename P2, typename P3, typename P4 >
QDcmtkTask * QDcmtkTask::create(
	OFCondition ( *f )( P1, P2, P3, P4 ), P1 p1, P2 p2, P3 p3, P4 p4, QObject * parent
) {
	return new QDcmtkTask( new Functor4< P1, P2, P3, P4 >( f, p1, p2, p3, p4 ), parent );
}


const QDcmtkResult & QDcmtkTask::result() const {
	return result_;
}


QDcmtkTask::Functor::~Functor() {
}


template < typename P >
QDcmtkTask::Functor1< P >::Functor1( OFCondition ( *f )( P ), P p ) :
	F_( f ),
	P_( p )
{
	Q_ASSERT( F_ != NULL );
}

template < typename P >
QDcmtkResult QDcmtkTask::Functor1< P >::execute() const {
	return F_( P_ );
}


template < typename P1, typename P2 >
QDcmtkTask::Functor2< P1, P2 >::Functor2( OFCondition ( *f )( P1, P2 ), P1 p1, P2 p2 ) :
	F_( f ),
	P1_( p1 ),
	P2_( p2 )
{
	Q_ASSERT( F_ != NULL );
}

template < typename P1, typename P2 >
QDcmtkResult QDcmtkTask::Functor2< P1, P2 >::execute() const {
	return F_( P1_, P2_ );
}


template < typename P1, typename P2, typename P3, typename P4 >
QDcmtkTask::Functor4< P1, P2, P3, P4 >::Functor4( OFCondition ( *f )( P1, P2, P3, P4 ), P1 p1, P2 p2, P3 p3, P4 p4 ) :
	F_( f ),
	P1_( p1 ),
	P2_( p2 ),
	P3_( p3 ),
	P4_( p4 )
{
	Q_ASSERT( F_ != NULL );
}

template < typename P1, typename P2, typename P3, typename P4 >
QDcmtkResult QDcmtkTask::Functor4< P1, P2, P3, P4 >::execute() const {
	return F_( P1_, P2_, P3_, P4_ );
}