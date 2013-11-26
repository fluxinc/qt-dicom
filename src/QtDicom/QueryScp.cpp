/***************************************************************************
 *   Copyright (C) 2011 by Flux Inc.                                       *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#include "DataSource.hpp"
#include "QueryScp.hpp"
#include "QueryScpReceiverThread.hpp"

#include <dcmtk/dcmnet/dimse.h>

namespace Dicom {

QueryScp::QueryScp( QObject * parent ) :
	QObject( parent ),
	associationServer_( parent ),
	dataSource_( 0 )
{
}


QueryScp::QueryScp( DataSource * source, QObject * parent ) :
	QObject( parent ),
	associationServer_( parent ),
	dataSource_( source )
{
}


QueryScp::~QueryScp() {
	stop();
}


const AssociationServer & QueryScp::associationServer() const {
	return associationServer_;
}


AssociationServer & QueryScp::associationServer() {
	return associationServer_;
}


void QueryScp::createReceiverThread() {
	Q_ASSERT( associationServer().hasPendingConnections() );

	ReceiverThread * thread = new ReceiverThread( 
		reinterpret_cast< AcceptorAssociation * >(
			associationServer().nextPendingAssociation()
		)
	);

	connect( 
		thread, SIGNAL( newQuery( Dataset, ReceiverThread * ) ),
		this, SLOT( match( Dataset, ReceiverThread * ) )
	);
	connect( 
		thread, SIGNAL( newQuery( Dataset, ReceiverThread *  ) ),
		SIGNAL( newQuery( Dataset ) )
	);
	connect(
		thread, SIGNAL( finished() ),
		thread, SLOT( deleteLater() )
	);
	connect( 
		thread, SIGNAL( failedToQuery( QString, ReceiverThread * ) ),
		SIGNAL( failedToQuery( QString ) )
	);
	thread->start();
}


DataSource * QueryScp::dataSource() {
	return dataSource_;
}


bool QueryScp::isRunning() const {
	return associationServer().isListening();
}


void QueryScp::match( Dataset mask, ReceiverThread * thread ) {
	dataSource()->refresh();

	for ( int i = 0; i < dataSource()->size(); ++i ) {
		const Dataset Rsp = dataSource()->dataset( i ).match( mask );
		if ( ! Rsp.isEmpty() ) {
			thread->queueIdentifier( Rsp );
		}
	}

	thread->finish( STATUS_Success );
}


void QueryScp::setDataSource( DataSource * source ) {
	dataSource_ = source;
}


bool QueryScp::start( const ConnectionParameters & Parameters ) {
	if ( isRunning() ) {
		qDebug( "Query SCP has already been started." );

		return false;
	}

	if ( ! dataSource() ) {
		qDebug( "No data source for Query SCP." );

		return false;
	}

	associationServer().setAbstractSyntaxes( 
		UidList::queryRetrieveSopClasses() + UidList::echoSopClass()
	);
	associationServer().setTransferSyntaxes( UidList::supportedTransferSyntaxes() );
	if ( associationServer().listen( Parameters ) ) {
		connect( 
			&associationServer(), SIGNAL( newAssociation() ),
			SLOT( createReceiverThread() )
		);
		connect(
			&associationServer(), SIGNAL( newAssociationError( QString ) ),
			SIGNAL( failedToQuery( QString ) )
		);
		return true;
	}
	else {
		qWarning( qPrintable( associationServer().errorString() ) );
		return false;
	}
}


void QueryScp::stop() {
	associationServer().close();
}


} // Namespace DICOM ends here.
