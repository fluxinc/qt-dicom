/***************************************************************************
 *   Copyright © 2013 by Flux Inc.                                         *
 *   Author: Pawel Zak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#ifndef QTDICOM_QDICOMPRINTENGINE_SCU_HPP
#define QTDICOM_QDICOMPRINTENGINE_SCU_HPP

#include "ConnectionParameters.hpp"
#include "QDicomPrintEngine.hpp"
#include "RequestorAssociation.hpp"
#include "ServiceUser.hpp"


class DcmSequenceOfItems;
class QDicomAttribute;


class QDicomPrintEngine::Scu : public Dicom::ServiceUser {
	public :
		Scu();

		bool beginSession( const QDicomPrinter * );
		bool printImage( const QImage & );
		void endSession();
		bool isOpen() const;

	private :
		bool createFilmBox( const QImage & );
		bool createSession();
		bool setImageBox( const QImage & );

	private :
		static DcmSequenceOfItems * createReference(
			const QDicomTag &, const QSopClass &, const QByteArray &
		);
		DcmSequenceOfItems * prepareImageSequence(
			const QImage &, const quint8 &
		);
		static QByteArray readReferencedInstance(
			const QDicomTag & sequence, const Dicom::Dataset & 
		);

	private :
		const Dicom::RequestorAssociation & association() const;
		Dicom::RequestorAssociation & association();
		Dicom::RequestorAssociation association_;

		const QByteArray & filmBoxUid() const;
		void setFilmBoxUid( const QByteArray & );
		QByteArray filmBoxUid_;

		const QByteArray & imageBoxUid() const;
		void setImageBoxUid( const QByteArray & );
		QByteArray imageBoxUid_;

		const QDicomPrinter & printer() const;
		void setPrinter( const QDicomPrinter & );
		const QDicomPrinter * printer_;

		const QByteArray & sessionUid() const;
		void setSessionUid( const QByteArray & );
		QByteArray sessionUid_;
};

#endif // QTDICOM_QDICOMPRINTENGINE_SCU_HPP
