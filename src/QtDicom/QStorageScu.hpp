/***************************************************************************
 *   Copyright (C) 2011 by Flux Inc.                                       *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#ifndef QTDICOM_QSTORAGESCU_HPP
#define QTDICOM_QSTORAGESCU_HPP

#include <QtCore/QObject>

#include <QtDicom/Globals.hpp>
#include <QtDicom/ServiceUser.hpp>
#include <QtDicom/QTransferSyntax>
#include <QtDicom/QUid>


namespace Dicom {
	class ConnectionParameters;
	class RequestorAssociation;
}

using Dicom::Dataset;


class QDICOM_DLLSPEC QStorageScu : public QObject {
	Q_OBJECT;

	public :
		enum Error {
			NoError = 0,
			InvalidConnectionParameters,
			Timeout,
			SopClassNotSupported,
			AssociationError,
			DimseError,
			UnknownError
		};

		enum State {
			Disconnected,
			Requesting,
			Connected,
			Sending
		};

	public :
		QStorageScu( QObject * parent = 0 );
		~QStorageScu();

		QString associationErrorString() const;
		QString dimseErrorString() const;

		void connectToAe(
			const QUid & SOPClass,
			const QTransferSyntax & TS
		);
		void connectToAe(
			const Dicom::ConnectionParameters & params,
			const QUid & SOPClass,
			const QTransferSyntax & TS
		);
		void disconnectFromAe();

		Error error() const;
		const char * errorString() const;
		bool hasError() const;
		void setConnectionParameters( const Dicom::ConnectionParameters & params );
		State state() const;
		void store( const Dicom::Dataset & dataset );

	signals :
		void connected();
		void disconnected();
		void error( int error );
		void error( QString message );
		void stored( QByteArray instanceUid );

	private slots :
		void releaseAssociation();
		void requestAssociation();
		void storeDataset( Dataset dataset );

	private :
		inline void setError( Error e );
		inline void setState( State s );

	private :
		inline Dicom::RequestorAssociation & association();
		inline const Dicom::RequestorAssociation & association() const;
		Dicom::RequestorAssociation * association_;		
		Dicom::ServiceUser dimseClient_;
		Error error_;
		QUid sopClass_;
		State state_;
		QTransferSyntax transferSyntax_;
};

#endif
