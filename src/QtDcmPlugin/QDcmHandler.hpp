/***************************************************************************
 *   Copyright © 2010-2014 by Flux Inc.                                    *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#ifndef QDCMHANDLER_HPP
#define QDCMHANDLER_HPP

#include <QtCore/QCache>
#include <QtCore/QHash>
#include <QtCore/QSize>
#include <QtCore/QVariant>

#include <QtGui/QColor>
#include <QtGui/QImageIOHandler>

class DcmDataset;
class DcmFileFormat;
class DcmItem;
class DcmSequenceOfItems;
class DcmTagKey;


/**
 * \em QDcmHandler provides methods allowing to access, write and validate
 * DICOM image files, along with their tag values.
 *
 * \author Paweł Żak <pawel.zak@fluxinc.ca>
 */
class QDcmHandler : public QImageIOHandler {
	public :
		/**
		 * Creates a DICOM file handler associated with the \a device
		 * and reading the image data from it using the image \a format.
		 *
		 * Constructor invokes \ref loadDicomFile() method, which
		 * loads the file header from \em DcmInputStream created
		 * upon the data read from the \a device. It doesn't (according to
		 * the DCMTK manual) load pixel data, those are later fetched with
		 * \ref readFrame() method so it is relatively fast (depending
		 * on the transfer syntax used to encode the file).
		 * 
		 * \todo Investigate performance of this constructor. Perhaps
		 *       holding with read operation until \ref read() or \ref option()
		 *       method was called would make the handler faster?
		 */
		QDcmHandler( 
			QIODevice * device, const QByteArray & format
		);

		/**
		 * Destroys the DICOM image file handler.
		 *
		 * De-allocates DICOM image file and cache objects.
		 */
		~QDcmHandler();

		/**
		 * Returns \c true if handler can read from the device set in the
		 * constructor.
		 */
		bool canRead() const;

		/**
		 * Returns \c true when the \a device contains DICOM image file data.
		 *
		 * Since this method should only be used to glance a data stream and
		 * perform a quick check, it only checks whether the \c DICM tag is
		 * present at the 128th byte.
		 */
		static bool canRead( QIODevice * device );

		/**
		 * Returns current frame.
		 */
		int currentImageNumber() const;

		/**
		 * Returns the number of frames in the DICOM file.
		 *
		 * Functions reads the \c (0028,0008) Number of Frames tag and returns
		 * the integer value extracted from it.
		 * When the tag is not present but DICOM file was loaded successfully, 
		 * a value of \c 1 is returned. Otherwise function returns \c 0.
		 *
		 * Internal cache is employed to ensure non redundant calls are taken
		 * to DCMTK internal routines.
		 */
		int imageCount() const;	

		/**
		 * Jumps to \a n-th frame.
		 *
		 * Method returns \c true when DICOM file was successfully loaded and 
		 * the \a n is less than \ref imageCount(). Otherwise \c false is 
		 * returned.
		 *
		 * \sa jumpToNextImage()
		 */
		bool jumpToImage( int n );

		/**
		 * Jumps to next frame if one exists.
		 *
		 * Returns \c true when DICOM file was successfully loaded and
		 * next frame number is still less than \ref imageCount(). Otherwise
		 * \c false is returned.
		 */
		bool jumpToNextImage();

		/**
		 * Returns 1.
		 *
		 * DICOM standard doesn't define looping through frames.
		 */
		int loopCount() const;

		/**
		 * Returns the time each frame should be displayed in miliseconds.
		 *
		 * Functions reads the \c (0018,1063) Frame Time tag and returns
		 * the integer value extracted from it.
		 * When the tag is not present but DICOM file was loaded successfully, 
		 * a value of \c 1000 is returned. Otherwise function returns \c 0.
		 *
		 * Internal cache is employed to ensure non redundant calls are taken
		 * to DCMTK internal routines.
		 *
		 * \todo Make sure that the \c (0018,1063) Frame Time tag is meant to
		 *       to be used in this context.
		 */
		int nextImageDelay() const;

		/**
		 * Returns \a option value.
		 *
		 * As for version 0.0.1, \em QDcmHandler supports the following options:
		 * - \em QImageIOHandler::Size returns what the \ref imageSize() 
		 *                             provides,
		 * - \em QImageIOHandler::Description returns what the 
		 *                                    \ref serializeTags() provides,
		 * - \em QImageIOHandler::Animation returns \c true.
		 *
		 * For other options a null \em QVariant is returned.
		 */
		QVariant option( QImageIOHandler::ImageOption option ) const;

		/**
		 * Reads current frame into the \a image.
		 *
		 * Returns \c true when the \a image is valid. When DICOM image file
		 * wasn't loaded or when any other error occurs, returns \c false.
		 */
		bool read( QImage * image );

		/**
		 * Returns \c true if \em QDcmHandler supports the \a option.
		 *
		 * As for version 0.0.1, \em QDcmHandler supports the following options:
		 * - \em QImageIOHandler::Size,
		 * - \em QImageIOHandler::Description,
		 * - \em QImageIOHandler::Animation.
		 *
		 * For other options \c false is returned.
		 */
		bool supportsOption( QImageIOHandler::ImageOption option ) const;

	private :
		/**
		 * Returns the DICOM file.
		 *
		 * Asserts DICOM file object is allocated and loaded, and returns a
		 * reference to it. Method is defined inline as it is provided just
		 * for coneniance.
		 */
		DcmFileFormat & dicomFile() const;

		/**
		 * Returns a dataset from DICOM file.
		 *
		 * Asserts DICOM file is loaded and returns its dataset. Implementation
		 * is defiend \c inline as this method is provided just for conveniance.
		 */
		DcmDataset & dicomFileDataset() const;

		/**
		 * Loads DICOM file from the device set in the constructor.
		 *
		 * Creates \em DcmInputBufferStream out of all data in the
		 * device specified in the constructor, and then reads a
		 * \em DcmFileFormat object from it (\ref dicomFile_ member).
		 *
		 * When function fails, the \ref dicomFile_ pointer is null.
		 */
		void loadDicomFile();

		/**
		 * Calculates the resolution specified in the DICOM file \em dataset.
		 *
		 * Method attempts to read values of the following tags:
		 * - \c (0028,0030) Pixel Spacing,
		 * - \c (0018,1164) Imager Pixel Spacing,
		 * - \c (0018,2010) Nominal Scanned Pixel Spacing.
		 * and if any one of these is present, uses retrieved value to determine
		 * dots-per-meter-of-patient-body, which is then returned in the 
		 * \em QSize object for respective axis separately.
		 *
		 * When none of those tags is found, returned QSize is invalid.
		 *
		 * Employs internal cache to ensure no redundant calls are taken.
		 *
		 * \todo Use IOD definitions for particular modalities to calculate the 
		 *       resolution. Right now method just iterates over pixel spacing 
		 *       tags and, once a value is found, uses it to calculate the 
		 *       result directly. This is invalid for some modalities 
		 *       (e.g. CT) where you need to take the distance between a patient
		 *       and an imaging device into account.
		 */
		QSize imageResolution() const;

		/**
		 * Returns the size of the image.
		 * 
		 * Reads \c (0028,0010) Rows and \c (0028,0011) Columns tags from the
		 * \em dataset and returns their value.
		 * If DICOM file failed to load or the \em dataset doesn't contain 
		 * required fields, invalid \em QSize is returned.
		 *
		 * Employs cache so that no redundant calls to DCMTK's internal getter 
		 * methods is necessary.
		 */
		QSize imageSize() const;

		/**
		 * Returns \c true if DICOM file was successfully loaded by the 
		 * constructor.
		 *
		 * Implementation of this methods just checks whether \ref dicomFile_ 
		 * member is initialized.
		 */
		bool isDicomFileLoaded() const;

		/**
		 * Returns \a n-th frame of the image.
		 *
		 * Method caches frames loaded already in the \ref frames_ member, 
		 * the latter however should not be accessed by other methods.
		 *
		 * Apart from pixel data, this method also stores image text into
		 * the \em QImage using appropriate methods.
		 *
		 * When handler fails to load the DICOM file or some other error
		 * occurs, null image is returned.
		 *
		 * \note \em QImage doesn't support grayscale images of over than 8 bits
		 *       depth, therefore DICOM images that store data using such
		 *       formats will be re-scaled to 256 shades range.
		 *       This also implies all stored VOI LUT transformation be removed
		 *       and replaced with MinMax window.
		 *
		 * \todo Maybe ask Qt Center for 16 bits grayscale format?
		 */
		QImage readFrame( int n );

		/**
		 * Returns \a tag value as a \em QString. When DICOM dataset doesn't
		 * contain the \a tag or some error occures, empty strinng is returned.
		 *
		 * The \a suppressOutput flag allows the user to inhibit printing 
		 * debug message generated by the method on failure.
		 */
		QString tagValue(
			const DcmTagKey & tag, bool suppressOutput = false
		) const;

		/**
		 * Returns a hash table mapping tag \em designators to tag values stored
		 * as a list of strings.
		 *
		 * A tag \a designator is a list of tag group and element markers that 
		 * lead directly to the tag and allow to reconstruct a dataset, e.g.:
		 * \c (0008,0060) is a \em designator of a Modality in the main dataset,
		 * whereas \c (0040,0100)(0008,0060) is a \em designator of a Modality
		 * in the Scheduled Procedure Step Sequence.
		 *
		 * In case of tags containing multiple values, associated string list
		 * holds more than one element.
		 */
		QHash< QString, QVariant > tagsValues() const;

	private :
		/**
		 * Returns grayscale 8 bits color table.
		 */
		static const QVector< QRgb > & grayscaleColorTable();

		/**
		 * Returns multiple values separator (a backslash).
		 */
		static const QString & multipleValuesSeparator();

	private :
		/**
		 * Current frame.
		 */
		int currentFrame_;

		/**
		 * DICOM file.
		 *
		 * \note It needs to be \c mutable because apparently DCMTK 
		 *       developers haven't heard about constant class members.
		 */
		mutable DcmFileFormat * dicomFile_;

		/**
		 * Cache for every frame.
		 */
		QCache< unsigned, QImage > frames_;

		/**
		 * Cache for all tag values, size, image count, etc.
		 */
		mutable QCache< QString, QVariant > cache_;
};

#endif
