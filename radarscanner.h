#ifndef RADARSCANNER_H
#define RADARSCANNER_H

#include <QtCore>
#include <QtGui>

#include <QWidget>
#include <QTimer>
#include <QPoint>
#include <QImage>


/**
 * 一个像雷达扫描的动态图形。可以嵌入到其他界面中。
 * 在开放桌面系统KDE上有一个屏幕保护是一个雷达扫描图，该类及其实现特征源于此。
 */
class RadarScanner : public QWidget
{
	Q_OBJECT

public:
    RadarScanner(QWidget *parent);
    ~RadarScanner();

private:

	QWidget * mParent ;	//该物件的父物件。放在其上，可以0覆覆盖该物件，以保护调用界面的布局不会被打乱。
	
	QTimer mScannerTimer ;
    
	QPoint mCentrolPoint ;	//雷达中心坐标。
	int mRadarRadius ;	//雷达半径

	QImage * mRadarImageBuffer ;	//雷达的图片表示，可以很容易的保存了。

	int mCurrentAngle;		//当前扫描线所位于的角度。
	int mScanRate ;		//扫描速度，度/s
	bool mDirection ;	//扫描的方向，正向还是反向的。

	QRect mRect ;		//本窗口的大小。
	QBrush brush ;
	QColor cl ;

public slots:
	void onStartScan() ;
	void onStopScan();
	
	void onRestartScan() ;	//相当于重置一下。
	void onSetScanSpeed( int rate ); //设置扫描速度，即扫描线的旋旋转速度。即计时器的时间间隔变化一下。
	
	void setDirection(bool dr ) ;

private slots:
	void onUpdateRadar() ;

protected:
	virtual void paintEvent(QPaintEvent * event);
	
};

#endif // RADARSCANNER_H
