/***************************************************************************
 *   Copyright © 2013 by Flux Inc.                                         *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#ifndef	QTDICOM_QDCMTKRESULT_HPP
#define QTDICOM_QDCMTKRESULT_HPP

#include "Globals.hpp"

#include <QtCore/QSharedData>

class OFCondition;

/**
 * The \em QDcmtkResult class is a wrapper for DCMTK own result class, the
 * \em OFCondition.
 *
 * The DICOM toolkit employs the \em OFCondition class to pass its routines
 * status. Usually, they contain identification numbers and text. Since QtDicom 
 * cannot expose any DCMTK object directly (it is meant to be used without 
 * access to DCMTK), the \em OFCondition has to be wrapped. Further more, 
 * \em OFCondition objects can use a substantial amount of memory, the wrapper
 * allows to use implicitly shared pattern.
 *
 * \author Paweł Żak <pawel.zak@fluxinc.ca>
 */
class QDICOM_DLLSPEC QDcmtkResult {
	public :
		/**
		 * Creates a result with DCMTK status set to EC_Normal.
		 */
		inline QDcmtkResult();

		/**
		 * Creates a result with DCMTK status set to \a status, and message
		 * to \a text.
		 */
		inline QDcmtkResult( int status, const char * text = "" );

		/**
		 * Creates a result from DCMTK status \a status.
		 */
		inline QDcmtkResult( const OFCondition & status );

		/**
		 * Copy constructor, copies shared data pointer.
		 */
		inline QDcmtkResult( const QDcmtkResult & other );

		/**
		 * Does nothing special.
		 */
		inline ~QDcmtkResult();

		/**
		 * Returns DCMTK status wrapped by this object.
		 */
		inline const OFCondition & ofCondition() const;

		/**
		 * Returns DCMTK status wrapped by this object.
		 */
		inline OFCondition & ofCondition();

		/**
		 * Returns DCMTK status wrapped by this object.
		 */
		inline operator const OFCondition & () const;

		/**
		 * Returns DCMTK status wrapped by this object.
		 */
		inline operator OFCondition & ();

	private :
		class Data : public QSharedData {
			public :
				Data();
				Data( int status, const char * text );
				Data( const OFCondition & status );
				Data( const Data & other );
				~Data();
				Data & operator = ( const Data & other );

				inline const OFCondition & ofCondition() const;
				inline OFCondition & ofCondition();

			private :
				OFCondition * status_;
		};

	private :
		QSharedDataPointer< Data > data_;
};

#include "QDcmtkResult.inl"

#endif
