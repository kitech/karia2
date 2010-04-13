#ifndef MMEX_H
#define MMEX_H

#include <QtCore>
#include <QThread>

#include <QMap>
#include <QVector>
#include <QPair>
#include <QMutex>

/**
 * 这个类像是对计算机的内存管理机制有点相似之处，返回的只是一个指针，而由该类维护当前的该块内存用量。
 * 这个结束指针可能不需要吧。
 *
 */

class MMEX : public QThread
{
	Q_OBJECT

private:
	struct _mem_pointer_s
	{
		quint64 SP ;	//起始位置		
		quint64 CP ;	//当前位置
		//quint64 EP ;	//未尾位置	,默认为下一指针的起始位置，或者为0，或者为内存总容量。由该类维护管理。
		bool   busy ;	//指的是当前这一段有没有拥有者。
	};

	typedef struct _mem_pointer_s MPT ;

public:
	enum{MM_NOT_EXSIT,MM_NULL} ;
    MMEX( quint64 capacity , QObject *parent);
    ~MMEX();

	void run() ;
	
	/**
	 * -1 表示还没有设置最大长度。????
	 * 可能的返回值 0 ,无内存，>0 并且 <size ,没有总够的内存，=size有足够多的内存，
	 * 返回的值就是指针起点。
	 */
	quint64 malloc( /*quint64 size*/ );

	/**
	 * 可能的返回值　0　，　数据已经释放过了，表示内存释放完成，
	 * >0 并且 <size 部分数据释放过了，也表示内存释放完成
	 * == size ，内存释放正常，可能还有内存，需要继承试着翻译内存。
	 */
	quint64 free( quint64 SP , quint64 size  );
	
	/**
	 * 
	 */
	quint64 release( quint64 SP ) ;

private:
    quint64  mMemCapacity ;	
	QVector< MPT > mPointerMap ;
	QMutex  mAtomMutex ;
	//minBlock == 1 
	int mMaxBlock ;
};

#endif // MMEX_H


