/***************************************************************************
 *   Copyright © 2013 by Flux Inc.                                         *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#ifndef QTDICOM_QDICOMTAG_HPP
#define QTDICOM_QDICOMTAG_HPP

#include <QtCore/QtGlobal>
#include <QtCore/QMutex>

#include <QtDicom/Globals.hpp>


class QDICOM_DLLSPEC QDicomTag {
	public :
		enum Id : quint32 {

#define ENTRY( NAME, G, E ) \
			NAME = 0x ## G ## E,
			
			ENTRY( InstanceCreationDate,   0008, 0012 )
			ENTRY( InstanceCreationTime,   0008, 0013 )
			ENTRY( InstanceCreatorUid,     0008, 0014 )
			ENTRY( SopClassUid,            0008, 0016 )
			ENTRY( SopInstanceUid,         0008, 0018 )
			ENTRY( StudyDate,              0008, 0020 )
			ENTRY( SeriesDate,             0008, 0021 )
			ENTRY( AcquisitionDate,        0008, 0022 )
			ENTRY( ContentDate,            0008, 0023 )
			ENTRY( AcquisitionDateTime,    0008, 002a )
			ENTRY( StudyTime,              0008, 0030 )
			ENTRY( SeriesTime,             0008, 0031 )
			ENTRY( AcquisitionTime,        0008, 0032 )
			ENTRY( ContentTime,            0008, 0033 )
			ENTRY( AccessionNumber,        0008, 0050 )
			ENTRY( Modality,               0008, 0060 )
			ENTRY( Manufacturer,           0008, 0070 )
			ENTRY( InstitutionName,        0008, 0080 )
			ENTRY( InstitutionAddress,     0008, 0081 )
			ENTRY( ReferringPhysicianName, 0008, 0090 )
			ENTRY( StudyDescription,       0008, 1030 )
			ENTRY( SeriesDescription,      0008, 103e )
			ENTRY( PatientName,            0010, 0010 )
			ENTRY( PatientId,              0010, 0020 )
			ENTRY( PatientBirthDate,       0010, 0030 )
			ENTRY( PatientBirthTime,       0010, 0032 )
			ENTRY( PatientSex,             0010, 0040 )
#undef ENTRY

		};

	public :
		static QDicomTag fromString( const QString & );
		static QDicomTag fromUInt32( quint32 );

	public :
		QDicomTag();
		QDicomTag( const Id & );
		QDicomTag( quint16 group, quint16 element );
		~QDicomTag();
		operator quint32() const;
			
		quint16 element() const;
		quint16 group() const;

		bool isNull() const;
		bool isValid() const;

		const QString & keyword() const;

		void setElement( quint16 );
		void setGroup( quint16 );

		QString toString() const;

	private :
		quint32 value_;
};

#endif // ! QTDICOM_QDICOMTAG_HPP
