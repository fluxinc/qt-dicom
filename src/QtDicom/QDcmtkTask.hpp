/***************************************************************************
 *   Copyright © 2013 by Flux Inc.                                         *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#ifndef QTDICOM_QDCMTKTASK_HPP
#define QTDICOM_QDCMTKTASK_HPP

#include "Globals.hpp"

#include <QtCore/QObject>
#include <QtCore/QRunnable>

#include <QtDicom/QDcmtkResult>

class OFCondition;

/**
 * The \em QDcmtkTask class is responsible for running DCMTK methods in 
 * background threads, without blocking the main event loop.
 *
 * You can create a task only by using one of static \ref create() methods, 
 * which require you to provide DCMTK routine address and its parameters.
 * Each \ref create() method is responsible for creating a so-called \em 
 * functor, holding both the function pointer and the arguments. It is essential
 * therefore, to keep the arguments intact before starting the task. In fact 
 * in most cases the arguments should remain valid throughout the entire time
 * task is running.
 *
 * Tasks can be started with the \ref start() slot. The slot reserves a thread 
 * from global thread pool and runs task's body (the \ref run() method) in it.
 * Inside the \ref run() method, stored functor is being executed. After DCMTK
 * finishes operation, the result is saved in task's local variable and the
 * \ref finished() signal is emitted. The signal passes a copy of the result as
 * well.
 *
 * Task results can be later queried thanks to the \ref result() accessor. Note,
 * however, that the \ref run() method implementation doesn't use any form of
 * data access synchronization for performance reasons, hence it is forbidden
 * to use the \ref result() accessor when the task is running.
 *
 * Since \ref create() methods family allocates each task on stack and returns 
 * pointers to them, it is caller's duty to free acquired memory. It is 
 * possible to connect the \ref finished() signal with \em QObject's \em
 * deleteLater(). It is also possible to provide parent's object pointer during
 * task construction, for task to be removed when its parent dies. During 
 * destruction, task releases acquired thread.
 *
 * The following example illustrates a typical usage scenario of \em QDcmtkTask
 * objects:
 *
 * \code
 * Task * task = QDcmtkTask::create( ::ASC_releaseAssociation, association );
 * connect(
 *     task, SIGNAL( finished( QDcmtkResult ) ),
 *     SLOT( checkAscReleaseResult( QDcmtkResult ) )
 * );
 * connect(
 *     task, SIGNAL( finished( QDcmtkResult ) ),
 *     task, SLOT( deleteLater() )
 * );
 * task->start();
 * \endcode
 *
 * \author Paweł Żak <pawel.zak@fluxinc.ca>
 */
class QDICOM_DLLSPEC QDcmtkTask : public QObject, public QRunnable {
	Q_OBJECT;

	public :
		// Forward declaration of the \em Functor class.
		class Functor;

	public :
		/**
		 * Creates a task, launching DCMTK function \a f with a single
		 * parameter \a p.
		 */
		template< typename P >
		inline static QDcmtkTask * create( 
			OFCondition ( *f )( P ), P p,
			QObject * parent = NULL
		);

		/**
		 * Creates a task, launching DCMTK function \a f with two parameters:
		 * \a p1 and \a p2.
		 */
		template< typename P1, typename P2 >
		inline static QDcmtkTask * create( 
			OFCondition ( *f )( P1, P2 ), P1 p1, P2 p2,
			QObject * parent = NULL
		);

		/**
		 * Creates a task, launching DCMTK function \a f with four parameters:
		 * \a p1, \a p2, \a p3 and \a p4.
		 */
		template< typename P1, typename P2, typename P3, typename P4 >
		inline static QDcmtkTask * create( 
			OFCondition ( *f )( P1, P2, P3, P4 ), P1 p1, P2 p2, P3 p3, P4 p4,
			QObject * parent = NULL
		);

	public :
		/**
		 * Destroys the task, releasing thread back to the pool.
		 */
		inline ~QDcmtkTask();

		/**
		 * Returns result of the task. The result is undefined if the task
		 * had not been started and finished already.
		 */
		inline const QDcmtkResult & result() const;

		/**
		 * Thread body, executes a functor created by one of the \ref create() 
		 * methods and stores result in local variable.
		 */
		void run();

	public slots :
		/**
		 * Starts the task. When specified routine finishes exectution, the \ref
		 * finished() signal is emitted.
		 */
		void start();

	signals :
		/**
		 * Emitted when task finishes and thread exits.
		 */
		void finished( QDcmtkResult );

	private :		
		template < typename P > class Functor1;
		template < typename P1, typename P2 > class Functor2;
		template < typename P1, typename P2, typename P3, typename P4 > class Functor4;

	private :
		inline QDcmtkTask( const Functor * f, QObject * parent );

	private :
		const Functor * Functor_;
		bool ran_;
		QDcmtkResult result_;

};


class QDcmtkTask::Functor {
	public :
		virtual ~Functor();
		virtual QDcmtkResult execute() const = 0;
};


template < typename P >
class QDcmtkTask::Functor1 : public QDcmtkTask::Functor {
	public :
		Functor1( OFCondition ( *f )( P ), P p );
		QDcmtkResult execute() const;

	private :
		OFCondition ( * const F_ )( P );
		const P P_;
};


template < typename P1, typename P2 >
class QDcmtkTask::Functor2 : public QDcmtkTask::Functor {
	public :
		Functor2( OFCondition ( *f )( P1, P2 ), P1 p1, P2 p2 );
		QDcmtkResult execute() const;

	private :
		OFCondition ( * const F_ )( P1, P2 );
		const P1 P1_;
		const P2 P2_;
};


template < typename P1, typename P2, typename P3, typename P4 >
class QDcmtkTask::Functor4 : public QDcmtkTask::Functor {
	public :
		Functor4( OFCondition ( *f )( P1, P2, P3, P4 ), P1 p1, P2 p2, P3 p3, P4 p4 );
		QDcmtkResult execute() const;

	private :
		OFCondition ( * const F_ )( P1, P2, P3, P4 );
		const P1 P1_;
		const P2 P2_;
		const P3 P3_;
		const P4 P4_;
};


#include "QDcmtkTask.inl"
#include "QDcmtkTask.moc.inl"

#endif
