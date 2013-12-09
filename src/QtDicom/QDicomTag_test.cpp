/***************************************************************************
 *   Copyright © 2013 by Flux Inc.                                         *
 *   Author: Pawel Zak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#ifdef _DEBUG

#include "QDicomTag.hpp"

#include <dcmtk/dcmdata/dctag.h>
#include <dcmtk/dcmdata/dcdeftag.h>


extern bool TestQDicomTag() {
	const QDicomTag NullTag;
	Q_ASSERT( NullTag == QDicomTag::Unknown );
	Q_ASSERT( NullTag != QDicomTag::PatientId );
	Q_ASSERT( NullTag.isNull() );
	Q_ASSERT( ! NullTag.isValid() );
	Q_ASSERT( NullTag.keyword().contains( "Unknown" ) );

	const QDicomTag PatientId = QDicomTag::PatientId;
	Q_ASSERT( PatientId == QDicomTag::PatientId );
	Q_ASSERT( PatientId != QDicomTag::Unknown );
	Q_ASSERT( ! PatientId.isNull() );
	Q_ASSERT( PatientId.isValid() );
	Q_ASSERT( PatientId.keyword() == "Patient ID" );
	Q_ASSERT( PatientId.keyword() == "Patient ID" );

#define COMPARE2( LOCAL, DCMTK ) \
		Q_ASSERT( QDicomTag( QDicomTag::LOCAL ).toDcmTag() == DCM_ ## DCMTK ); \
		Q_ASSERT( QDicomTag( QDicomTag::LOCAL ).group() == ( DCM_## DCMTK ).getGroup() ); \
		Q_ASSERT( QDicomTag( QDicomTag::LOCAL ).element() == ( DCM_ ## DCMTK ).getElement() )

#define COMPARE( LABEL ) \
		COMPARE2( LABEL, LABEL )


	// 0008 group
	COMPARE( InstanceCreationDate   );
	COMPARE( InstanceCreationTime   );
	COMPARE( StudyDate              );
	COMPARE( SeriesDate             );
	COMPARE( AcquisitionDate        );
	COMPARE( ContentDate            );
	COMPARE( AcquisitionDateTime    );
	COMPARE( StudyTime              );
	COMPARE( SeriesTime             );
	COMPARE( AcquisitionTime        );
	COMPARE( ContentTime            );
	COMPARE( AccessionNumber        );
	COMPARE( Modality               );
	COMPARE( Manufacturer           );
	COMPARE( InstitutionName        );
	COMPARE( InstitutionAddress     );
	COMPARE( ReferringPhysicianName );
	COMPARE( StudyDescription       );
	COMPARE( SeriesDescription      );

	COMPARE2( InstanceCreatorUid,       InstanceCreatorUID );
	COMPARE2( SopClassUid,              SOPClassUID );
	COMPARE2( SopInstanceUid,           SOPInstanceUID );
	COMPARE2( ReferencedSopClassUid,    ReferencedSOPClassUID );
	COMPARE2( ReferencedSopInstanceUid, ReferencedSOPInstanceUID );

	// 0010 group
	COMPARE( PatientName      );
	COMPARE( PatientBirthDate );
	COMPARE( PatientBirthTime );
	COMPARE( PatientSex       );

	COMPARE2( PatientId, PatientID );

	// 0028 group
	COMPARE( SamplesPerPixel );
	COMPARE( PhotometricInterpretation );
	COMPARE( Rows );
	COMPARE( Columns );
	COMPARE( BitsAllocated );
	COMPARE( BitsStored );
	COMPARE( HighBit );
	COMPARE( PixelRepresentation );

	// 2010 group
	COMPARE( ImageDisplayFormat );
	COMPARE( FilmOrientation );
	COMPARE( MagnificationType  );
	COMPARE( SmoothingType );
	COMPARE( BorderDensity );
	COMPARE( EmptyImageDensity );
	COMPARE( MinDensity );
	COMPARE( MaxDensity );
	COMPARE( Trim );
	COMPARE( ReferencedFilmSessionSequence );
	COMPARE( ReferencedImageBoxSequence );

	COMPARE2( FilmSizeId, FilmSizeID );

	// 2020 group
	COMPARE( Polarity                      );
	COMPARE( RequestedImageSize            );
	COMPARE( RequestedDecimateCropBehavior );
	COMPARE( BasicGrayscaleImageSequence   );

	COMPARE2( RequestedResolutionId, RequestedResolutionID );
	COMPARE2( ImagePosition,         ImageBoxPosition );

	// 2100 group
	COMPARE2( OwnerId, OwnerID );

	// 7FE0 group
	COMPARE( PixelData );

#undef COMPARE2
#undef COMPARE

	return true;
}

#endif // _DEBUG

 