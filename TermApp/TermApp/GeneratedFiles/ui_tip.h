/********************************************************************************
** Form generated from reading UI file 'tip.ui'
**
** Created by: Qt User Interface Compiler version 5.9.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TIP_H
#define UI_TIP_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>

QT_BEGIN_NAMESPACE

class Ui_TGTip
{
public:
    QLabel *content_label;
    QLabel *color;

    void setupUi(QDialog *TGTip)
    {
        if (TGTip->objectName().isEmpty())
            TGTip->setObjectName(QStringLiteral("TGTip"));
        TGTip->resize(351, 194);
        QFont font;
        font.setPointSize(14);
        TGTip->setFont(font);
        TGTip->setStyleSheet(QStringLiteral(""));
        content_label = new QLabel(TGTip);
        content_label->setObjectName(QStringLiteral("content_label"));
        content_label->setGeometry(QRect(20, 60, 311, 111));
        QFont font1;
        font1.setFamily(QStringLiteral("Segoe UI"));
        font1.setPointSize(14);
        font1.setKerning(true);
        content_label->setFont(font1);
        content_label->setStyleSheet(QStringLiteral(""));
        content_label->setLineWidth(0);
        color = new QLabel(TGTip);
        color->setObjectName(QStringLiteral("color"));
        color->setGeometry(QRect(0, -1, 351, 42));
        QFont font2;
        font2.setFamily(QString::fromUtf8("\345\276\256\350\275\257\351\233\205\351\273\221"));
        font2.setBold(false);
        font2.setItalic(false);
        font2.setWeight(50);
        color->setFont(font2);
        color->setAutoFillBackground(false);
        color->setStyleSheet(QString::fromUtf8("color: white;\n"
"font: 14px \"\345\276\256\350\275\257\351\233\205\351\273\221\";\n"
"background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 rgba(1, 88, 255, 255), stop:1 rgba(78, 183, 239, 255));"));
        color->setTextFormat(Qt::AutoText);
        color->setMargin(0);

        retranslateUi(TGTip);

        QMetaObject::connectSlotsByName(TGTip);
    } // setupUi

    void retranslateUi(QDialog *TGTip)
    {
        TGTip->setWindowTitle(QApplication::translate("TGTip", "Dialog", Q_NULLPTR));
        content_label->setText(QString());
        color->setText(QApplication::translate("TGTip", "  \350\255\246\345\221\212", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class TGTip: public Ui_TGTip {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TIP_H
