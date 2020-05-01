#include "TGTip.h"
#include"ui_tip.h"
#include<windows.h>
#include<global.h>

TGTip::TGTip(QWidget *parent /* = 0 */):ui(new Ui::TGTip)
{
	ui->setupUi(this);
	setWindowFlags(Qt::SubWindow | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint |Qt::Tool);
	setStyleSheet("border:1px groove rgb(10,10,10)");
	ui->content_label->setAlignment(Qt::AlignLeft);
	ui->content_label->setStyleSheet("border:0px;font-size:14px;");
	m_scwidth = GetSystemMetrics(SM_CXSCREEN);
	m_scheight = GetSystemMetrics(SM_CYSCREEN);
	setFixedSize(width(),height());

	m_endpos = QPoint(m_scwidth - this->width(), m_scheight - this->height() - 30);
	timer.setInterval(5000);

	//窗口消失效果
	disappear_ani = new QPropertyAnimation(this, "windowOpacity");
	disappear_ani->setDuration(3000);
	disappear_ani->setStartValue(1);
	disappear_ani->setEndValue(0);

	//窗口进入效果
	appear_ani = new QPropertyAnimation(this, "pos");
	appear_ani->setDuration(2000);
	appear_ani->setStartValue(QPoint(m_scwidth - this->width(), m_scheight));
	appear_ani->setEndValue(m_endpos);

	connect(this, SIGNAL(SendMsg(QString,int)), this, SLOT(ShowString(QString,int)));
	connect(appear_ani, SIGNAL(finished()), &timer, SLOT(start()));
	connect(&timer, SIGNAL(timeout()), this, SLOT(DisAppearTip()));
	connect(disappear_ani, SIGNAL(finished()), this, SLOT(hide()));
}
TGTip::~TGTip()
{
	delete ui;
}

void TGTip::AddStr(int type, std::wstring wstr)
{
	QString qstr = QString::fromStdWString(wstr);
	emit SendMsg(qstr,type);
}

void TGTip::ShowString(QString str,int type)
{
	QString tip_str;
	QString line;
	switch (type)
	{
	case HOOK_TYPE::HOOK_TYPE_OPEN_FILE:
		line.append(QString::fromStdWString(L"你打开了敏感文件: "));
		break;
	case HOOK_TYPE::HOOK_TYPE_DELETE_FILE:
		line.append(QString::fromStdWString(L"你删除了敏感文件: "));
		break;
	case HOOK_TYPE::HOOK_TYPE_COPY_FILE:
		line.append(QString::fromStdWString(L"你拷贝了敏感文件: "));
		break;
	case HOOK_TYPE::HOOK_TYPE_MOVE_FILE:
		line.append(QString::fromStdWString(L"你剪切了敏感文件: "));
		break;
	case HOOK_TYPE::HOOK_TYPE_RENAME_FILE:
		line.append(QString::fromStdWString(L"你重命名了敏感文件: "));
		break;
	case HOOK_TYPE::HOOK_TYPE_PRINT_DATA:
		line.append(QString::fromStdWString(L"你打印的文件含有敏感数据"));
		break;
	case HOOK_TYPE::HOOK_TYPE_COPY_DATA:
		line.append(QString::fromStdWString(L"你复制的内容含有敏感数据"));
		break;
	default:
		break;
	}
	tip_str.append(SetLineMargin(line));
	if (str != "")
	{
		tip_str.append(SetLineMargin(SetFileNameColor(str)));
	}
	line.clear();
	line.append(QString::fromStdWString(L"该行为将被记录至服务器"));
	tip_str.append(SetLineMargin(line));

	ui->content_label->setText(tip_str);
	ui->content_label->setWordWrap(true);
	if (isVisible())
	{
		timer.stop();
		disappear_ani->stop();
	}
	setWindowOpacity(1);
	show();
	appear_ani->start();
}

QString TGTip::SetLineMargin(QString &str)
{
	QString resLine;
	resLine.append("<p style=\'line-height:20px;\'>");
	resLine.append(str);
	resLine.append("</p>");
	return resLine;
}

QString TGTip::SetFileNameColor(QString &fileName)
{
	QString res;
	res.append("<a style=\'color:blue;\'>");
	res.append(fileName);
	return res;
}

void TGTip::enterEvent(QEvent *event)
{
	timer.stop();
	disappear_ani->stop();
	setWindowOpacity(1);
}

void TGTip::leaveEvent(QEvent *event)
{
	disappear_ani->start();
	timer.start();
}

void TGTip::DisAppearTip()
{
	appear_ani->stop();
	disappear_ani->start();
	timer.stop();
}
