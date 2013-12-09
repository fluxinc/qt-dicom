/***************************************************************************
 *   Copyright © 2013 by Flux Inc.                                         *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#ifndef QTDICOM_QDICOMTAG_HPP
#define QTDICOM_QDICOMTAG_HPP

#include <QtCore/QtGlobal>
#include <QtCore/QMetaType>

#include <QtDicom/Globals.hpp>


class DcmTag;


class QDICOM_DLLSPEC QDicomTag {
	public :
		enum Id : quint32 {

#define ENTRY( NAME, G, E ) \
			NAME = 0x ## G ## E,
			
			ENTRY( Unknown, FFFF, FFFF )

			ENTRY( InstanceCreationDate,     0008, 0012 )
			ENTRY( InstanceCreationTime,     0008, 0013 )
			ENTRY( InstanceCreatorUid,       0008, 0014 )
			ENTRY( SopClassUid,              0008, 0016 )
			ENTRY( SopInstanceUid,           0008, 0018 )
			ENTRY( StudyDate,                0008, 0020 )
			ENTRY( SeriesDate,               0008, 0021 )
			ENTRY( AcquisitionDate,          0008, 0022 )
			ENTRY( ContentDate,              0008, 0023 )
			ENTRY( AcquisitionDateTime,      0008, 002a )
			ENTRY( StudyTime,                0008, 0030 )
			ENTRY( SeriesTime,               0008, 0031 )
			ENTRY( AcquisitionTime,          0008, 0032 )
			ENTRY( ContentTime,              0008, 0033 )
			ENTRY( AccessionNumber,          0008, 0050 )
			ENTRY( Modality,                 0008, 0060 )
			ENTRY( Manufacturer,             0008, 0070 )
			ENTRY( InstitutionName,          0008, 0080 )
			ENTRY( InstitutionAddress,       0008, 0081 )
			ENTRY( ReferringPhysicianName,   0008, 0090 )
			ENTRY( StudyDescription,         0008, 1030 )
			ENTRY( SeriesDescription,        0008, 103e )
			ENTRY( ReferencedSopClassUid,    0008, 1150 )
			ENTRY( ReferencedSopInstanceUid, 0008, 1155 )

			ENTRY( PatientName,      0010, 0010 )
			ENTRY( PatientId,        0010, 0020 )
			ENTRY( PatientBirthDate, 0010, 0030 )
			ENTRY( PatientBirthTime, 0010, 0032 )
			ENTRY( PatientSex,       0010, 0040 )

			ENTRY( SamplesPerPixel,           0028, 0002 )
			ENTRY( PhotometricInterpretation, 0028, 0004 )
			ENTRY( Rows,                      0028, 0010 )
			ENTRY( Columns,                   0028, 0011 )
			ENTRY( BitsAllocated,             0028, 0100 )
			ENTRY( BitsStored,                0028, 0101 )
			ENTRY( HighBit,                   0028, 0102 )
			ENTRY( PixelRepresentation,       0028, 0103 )

			ENTRY( NumberOfCopies,   2000, 0010 )
			ENTRY( PrintPriority,    2000, 0020 )
			ENTRY( MediumType,       2000, 0030 )
			ENTRY( FilmDestination,  2000, 0040 )
			ENTRY( FilmSessionLabel, 2000, 0050 )
			ENTRY( MemoryAllocation, 2000, 0060 )

			ENTRY( ImageDisplayFormat,            2010, 0010 )
			ENTRY( FilmOrientation,               2010, 0040 )
			ENTRY( FilmSizeId,                    2010, 0050 )
			ENTRY( MagnificationType,             2010, 0060 )
			ENTRY( SmoothingType,                 2010, 0080 )
			ENTRY( BorderDensity,                 2010, 0100 )
			ENTRY( EmptyImageDensity,             2010, 0110 )
			ENTRY( MinDensity,                    2010, 0120 )
			ENTRY( MaxDensity,                    2010, 0130 )
			ENTRY( Trim,                          2010, 0140 )
			ENTRY( ReferencedFilmSessionSequence, 2010, 0500 )
			ENTRY( ReferencedImageBoxSequence,    2010, 0510 )

			ENTRY( ImagePosition,                 2020, 0010 )
			ENTRY( Polarity,                      2020, 0020 )
			ENTRY( RequestedImageSize,            2020, 0030 )
			ENTRY( RequestedDecimateCropBehavior, 2020, 0040 )
			ENTRY( RequestedResolutionId,         2020, 0050 )
			ENTRY( BasicGrayscaleImageSequence,   2020, 0110 )

			ENTRY( OwnerId, 2100, 0160 )

			ENTRY( PixelData, 7FE0, 0010 )
#undef ENTRY

		};

	public :
		static QDicomTag fromString( const QString & );

	public :
		QDicomTag();
		QDicomTag( const Id & );
		QDicomTag( const quint16 & group, const quint16 & element );
		~QDicomTag();
		operator DcmTag() const;
		operator Id() const;
			
		quint16 element() const;
		quint16 group() const;

		bool isNull() const;
		bool isValid() const;

		const QString & keyword() const;

		void setElement( quint16 );
		void setGroup( quint16 );

		DcmTag toDcmTag() const;
		QString toString() const;
		quint32 toUInt32() const;

	private :
		Id id_;
};


Q_DECLARE_METATYPE( QDicomTag );

#endif // ! QTDICOM_QDICOMTAG_HPP
