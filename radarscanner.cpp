
#include <math.h>

#include "radarscanner.h"

RadarScanner::RadarScanner(QWidget *parent)
	: QWidget(parent) , mParent( parent)
{
	QRect rc = parent->geometry() ;
	qDebug()<< rc ;

	this->setMinimumHeight(rc.height());
	this->setMinimumWidth(rc.width() );

	this->mRadarRadius = rc.width()>rc.height()? (rc.height()/2) : (rc.width()/2) ;
	this->mCentrolPoint.setX(rc.width()/2);
	this->mCentrolPoint.setY(rc.height()/2);
	this->mCurrentAngle = 360  ;
	this->mScanRate = 1;
	this->mDirection = true ;	//正向
	this->mRect = rc ;
	this->cl = QColor(0,0,0);
	this->brush = QBrush(cl);

	this->mRadarImageBuffer = new QImage(rc.width(),rc.height(),QImage::Format_ARGB32);
	QPainter painter(this->mRadarImageBuffer);
	painter.setBrush(QBrush(QColor(0,0,0)));
	painter.drawPie(0,0,this->rect().width(),this->rect().height(),0,360*16 );

	QObject::connect( & this->mScannerTimer,SIGNAL(timeout()),this,SLOT(onUpdateRadar()));

	this->onStartScan();

}

RadarScanner::~RadarScanner()
{
	if( this->mScannerTimer.isActive() )
		this->mScannerTimer.stop();

	delete this->mRadarImageBuffer ; this->mRadarImageBuffer = 0 ; 
}

void RadarScanner::onStartScan() 
{
	if( this->mScannerTimer.isActive()) return ;
	this->mScannerTimer.start(50);
}

void RadarScanner::onStopScan()
{
	if( this->mScannerTimer.isActive() )
		this->mScannerTimer.stop() ;
}


void RadarScanner::onRestartScan() 	//相当于重置一下。
{
	this->onStopScan();
	this->onStartScan() ;

}
void RadarScanner::onSetScanSpeed( int rate ) //设置扫描速度，即扫描线的旋旋转速度。即计时器的时间间隔变化一下。
{
	this->mScanRate = rate ;	//是不是要有一种好的直观的计算方式，给用户显示的是一种，而程序中使用的又是另外一种呢。　
}

void RadarScanner::paintEvent(QPaintEvent * event)
{
	
	QPainter painter(this);

	painter.drawImage( 0 , 0 , * this->mRadarImageBuffer);

}

void RadarScanner::setDirection(bool dr ) 
{
	this->mDirection = dr ;
}

/**
 * 将图象先在内存里面画出来，然后调用Update方法输出到用户界面上。
 */
void RadarScanner::onUpdateRadar()
{
	int color  ;

	if( this->mDirection ) //正向
	{
		this->mCurrentAngle -= 3 ;
		if( this->mCurrentAngle < 0  )
			this->mCurrentAngle = 360 ;
		color = 255 ;
	}
	else
	{
		this->mCurrentAngle += 3  ;
		if( this->mCurrentAngle > 360   )
			this->mCurrentAngle = 0 ;
		color = 0 ;
	}

	//
	//QColor cl(0,0,0);
	//QBrush brush( cl , Qt::SolidPattern );

	QPainter painter(this->mRadarImageBuffer);
	//painter.save();
	//painter.setBrush( brush );
	//painter.drawPie(0,0,this->mRect.width(),this->mRect.height(),0,360*16 );	//画背景。


	//painter.setBrush(brush);
	//painter.restore();

	//painter.setBrush(brush);
	//painter.setPen(QColor(0,255,0));	

	int tmpAngle = this->mCurrentAngle * 16 + 16 ;	
	int maxAngle = (this->mCurrentAngle + 50) * 16 ;
	for(  ; tmpAngle <= maxAngle ; tmpAngle +=48 )
	{
		//tmpAngle = angle + this->mCurrentAngle ;
		
		cl.setGreen(color);
		//painter.drawLine(this->mCentrolPoint,QPoint(endX , endY ) );
		brush.setColor(cl);
		painter.setBrush( brush );		
		painter.setPen(cl);
		//painter.setPen(Qt::SolidLine);

		painter.drawPie( this->mRect , tmpAngle   ,  48 );
		//qDebug()<< color << tmpAngle/16  ;
		if( this->mDirection)
			color -= 15 ;
		else 
			color += 15 ;
	}
	
	//painter.drawPie(this->rect(),0 * 16   ,  45 * 16 );

	this->update();

}

