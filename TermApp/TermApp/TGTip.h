#pragma once
#include<QDialog>
#include<QTimer>
#include<QPropertyAnimation>

namespace Ui
{
	class TGTip;
}
class TGTip :public QDialog
{
	Q_OBJECT
public:
	void AddStr(std::wstring,int);
	explicit TGTip(QWidget *parent = 0);
	~TGTip();
signals:
	void SendMsg(QString,int);
public slots:
	void ShowString(QString,int);
	void DisAppearTip();
private:
	QString SetLineMargin(QString &);
	QString SetFileNameColor(QString &);
	void enterEvent(QEvent *event) override;
	void leaveEvent(QEvent *event)override;
private:
	QTimer timer;
	Ui::TGTip *ui;
	int m_scwidth;
	int m_scheight;
	QPropertyAnimation *disappear_ani;
	QPropertyAnimation *appear_ani;
	QPoint m_endpos;
};


