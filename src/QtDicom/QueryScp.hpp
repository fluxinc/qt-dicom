/***************************************************************************
 *   Copyright (C) 2011 by Flux Inc.                                       *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#ifndef DICOM_QUERYSCP_HPP
#define DICOM_QUERYSCP_HPP

#include <QtCore/QObject>

#include "QtDicom/AssociationServer.hpp"
#include "QtDicom/Dataset.hpp"
#include "QtDicom/Globals.hpp"


class QDir;

namespace Dicom {

class ConnectionParameters;
class DataSource;


class QDICOM_DLLSPEC QueryScp : public QObject {
	Q_OBJECT;

	public :
		QueryScp( QObject * parent = 0 );
		QueryScp( DataSource * source, QObject * parent = 0 );
		~QueryScp();

		DataSource * dataSource();
		bool isRunning() const;
		void setDataSource( DataSource * source );
		bool start( const ConnectionParameters & parameters );
		void stop();

	private :
		class ReceiverThread;

	private :
		const AssociationServer & associationServer() const;
		AssociationServer & associationServer();

	private slots :
		void createReceiverThread();
		void match( Dataset dataset, ReceiverThread * );

	private :
		AssociationServer associationServer_;
		DataSource * dataSource_;

	signals :
		void failedToQuery( QString message );
		void newQuery( Dataset dataset );
};


}; // Namespace DICOM ends here.

#endif
