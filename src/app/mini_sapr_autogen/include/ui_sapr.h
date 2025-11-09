/********************************************************************************
** Form generated from reading UI file 'sapr.ui'
**
** Created by: Qt User Interface Compiler version 6.10.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SAPR_H
#define UI_SAPR_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *action;
    QAction *action_2;
    QAction *action_3;
    QAction *action_5;
    QWidget *centralwidget;
    QGridLayout *gridLayout;
    QScrollArea *scrollArea;
    QWidget *scrollAreaWidgetContents;
    QVBoxLayout *verticalLayout;
    QGroupBox *Anchors;
    QHBoxLayout *horizontalLayout;
    QCheckBox *checkBoxLeft;
    QCheckBox *checkBoxRight;
    QGroupBox *Bars;
    QVBoxLayout *verticalLayout_3;
    QGridLayout *BarsGrid;
    QPushButton *BarsAdd;
    QGroupBox *Forces;
    QVBoxLayout *verticalLayout_4;
    QTabWidget *ForcesTabs;
    QWidget *NodeForces;
    QWidget *BarForces;
    QTabWidget *tabWidget;
    QWidget *SchemaTab;
    QStatusBar *statusbar;
    QMenuBar *menubar;
    QMenu *menu;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(980, 648);
        action = new QAction(MainWindow);
        action->setObjectName("action");
        action_2 = new QAction(MainWindow);
        action_2->setObjectName("action_2");
        action_3 = new QAction(MainWindow);
        action_3->setObjectName("action_3");
        action_5 = new QAction(MainWindow);
        action_5->setObjectName("action_5");
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        gridLayout = new QGridLayout(centralwidget);
        gridLayout->setObjectName("gridLayout");
        scrollArea = new QScrollArea(centralwidget);
        scrollArea->setObjectName("scrollArea");
        QSizePolicy sizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
        sizePolicy.setHorizontalStretch(2);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(scrollArea->sizePolicy().hasHeightForWidth());
        scrollArea->setSizePolicy(sizePolicy);
        scrollArea->setMinimumSize(QSize(300, 0));
        scrollArea->setWidgetResizable(true);
        scrollAreaWidgetContents = new QWidget();
        scrollAreaWidgetContents->setObjectName("scrollAreaWidgetContents");
        scrollAreaWidgetContents->setGeometry(QRect(0, 0, 381, 581));
        QSizePolicy sizePolicy1(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Expanding);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(scrollAreaWidgetContents->sizePolicy().hasHeightForWidth());
        scrollAreaWidgetContents->setSizePolicy(sizePolicy1);
        verticalLayout = new QVBoxLayout(scrollAreaWidgetContents);
        verticalLayout->setObjectName("verticalLayout");
        verticalLayout->setSizeConstraint(QLayout::SizeConstraint::SetNoConstraint);
        Anchors = new QGroupBox(scrollAreaWidgetContents);
        Anchors->setObjectName("Anchors");
        horizontalLayout = new QHBoxLayout(Anchors);
        horizontalLayout->setObjectName("horizontalLayout");
        checkBoxLeft = new QCheckBox(Anchors);
        checkBoxLeft->setObjectName("checkBoxLeft");
        checkBoxLeft->setAutoRepeat(false);
        checkBoxLeft->setAutoExclusive(false);
        checkBoxLeft->setTristate(false);

        horizontalLayout->addWidget(checkBoxLeft);

        checkBoxRight = new QCheckBox(Anchors);
        checkBoxRight->setObjectName("checkBoxRight");
        checkBoxRight->setAutoRepeat(false);
        checkBoxRight->setAutoExclusive(false);
        checkBoxRight->setTristate(false);

        horizontalLayout->addWidget(checkBoxRight);


        verticalLayout->addWidget(Anchors);

        Bars = new QGroupBox(scrollAreaWidgetContents);
        Bars->setObjectName("Bars");
        QSizePolicy sizePolicy2(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::MinimumExpanding);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(Bars->sizePolicy().hasHeightForWidth());
        Bars->setSizePolicy(sizePolicy2);
        Bars->setMinimumSize(QSize(0, 200));
        Bars->setCheckable(false);
        verticalLayout_3 = new QVBoxLayout(Bars);
        verticalLayout_3->setObjectName("verticalLayout_3");
        verticalLayout_3->setSizeConstraint(QLayout::SizeConstraint::SetMinimumSize);
        BarsGrid = new QGridLayout();
        BarsGrid->setObjectName("BarsGrid");

        verticalLayout_3->addLayout(BarsGrid);

        BarsAdd = new QPushButton(Bars);
        BarsAdd->setObjectName("BarsAdd");

        verticalLayout_3->addWidget(BarsAdd);


        verticalLayout->addWidget(Bars);

        Forces = new QGroupBox(scrollAreaWidgetContents);
        Forces->setObjectName("Forces");
        sizePolicy2.setHeightForWidth(Forces->sizePolicy().hasHeightForWidth());
        Forces->setSizePolicy(sizePolicy2);
        Forces->setMinimumSize(QSize(0, 200));
        verticalLayout_4 = new QVBoxLayout(Forces);
        verticalLayout_4->setObjectName("verticalLayout_4");
        verticalLayout_4->setSizeConstraint(QLayout::SizeConstraint::SetMinimumSize);
        ForcesTabs = new QTabWidget(Forces);
        ForcesTabs->setObjectName("ForcesTabs");
        NodeForces = new QWidget();
        NodeForces->setObjectName("NodeForces");
        NodeForces->setEnabled(true);
        ForcesTabs->addTab(NodeForces, QString());
        BarForces = new QWidget();
        BarForces->setObjectName("BarForces");
        BarForces->setEnabled(true);
        ForcesTabs->addTab(BarForces, QString());

        verticalLayout_4->addWidget(ForcesTabs);


        verticalLayout->addWidget(Forces);

        scrollArea->setWidget(scrollAreaWidgetContents);

        gridLayout->addWidget(scrollArea, 0, 0, 1, 1);

        tabWidget = new QTabWidget(centralwidget);
        tabWidget->setObjectName("tabWidget");
        QSizePolicy sizePolicy3(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
        sizePolicy3.setHorizontalStretch(3);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(tabWidget->sizePolicy().hasHeightForWidth());
        tabWidget->setSizePolicy(sizePolicy3);
        SchemaTab = new QWidget();
        SchemaTab->setObjectName("SchemaTab");
        tabWidget->addTab(SchemaTab, QString());

        gridLayout->addWidget(tabWidget, 0, 1, 1, 1);

        MainWindow->setCentralWidget(centralwidget);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName("statusbar");
        MainWindow->setStatusBar(statusbar);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 980, 29));
        menubar->setDefaultUp(false);
        menu = new QMenu(menubar);
        menu->setObjectName("menu");
        MainWindow->setMenuBar(menubar);

        menubar->addAction(menu->menuAction());
        menu->addAction(action_2);
        menu->addAction(action_3);
        menu->addSeparator();
        menu->addAction(action_5);

        retranslateUi(MainWindow);

        ForcesTabs->setCurrentIndex(0);
        tabWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "Mini-SAPR", nullptr));
        action->setText(QCoreApplication::translate("MainWindow", "\320\244\320\260\320\271\320\273", nullptr));
        action_2->setText(QCoreApplication::translate("MainWindow", "\320\241\320\276\321\205\321\200\320\260\320\275\320\270\321\202\321\214 \320\272\320\260\320\272", nullptr));
        action_3->setText(QCoreApplication::translate("MainWindow", "\320\236\321\202\320\272\321\200\321\213\321\202\321\214", nullptr));
        action_5->setText(QCoreApplication::translate("MainWindow", "\320\222\321\213\320\271\321\202\320\270", nullptr));
        Anchors->setTitle(QCoreApplication::translate("MainWindow", "\320\236\320\277\320\276\321\200\321\213", nullptr));
        checkBoxLeft->setText(QCoreApplication::translate("MainWindow", "\320\233\320\265\320\262\320\260\321\217", nullptr));
        checkBoxRight->setText(QCoreApplication::translate("MainWindow", "\320\237\321\200\320\260\320\262\320\260\321\217", nullptr));
        Bars->setTitle(QCoreApplication::translate("MainWindow", "\320\241\321\202\320\265\321\200\320\266\320\275\320\270", nullptr));
        BarsAdd->setText(QCoreApplication::translate("MainWindow", "\320\224\320\276\320\261\320\260\320\262\320\270\321\202\321\214 \321\201\321\202\320\265\321\200\320\266\320\265\320\275\321\214", nullptr));
        Forces->setTitle(QCoreApplication::translate("MainWindow", "\320\235\320\260\320\263\321\200\321\203\320\267\320\272\320\270", nullptr));
        ForcesTabs->setTabText(ForcesTabs->indexOf(NodeForces), QCoreApplication::translate("MainWindow", "\320\241\320\276\321\201\321\200\320\265\320\264\320\276\321\202\320\276\321\207\320\265\320\275\320\275\321\213\320\265", nullptr));
        ForcesTabs->setTabText(ForcesTabs->indexOf(BarForces), QCoreApplication::translate("MainWindow", "\320\240\320\260\321\201\320\277\321\200\320\265\320\264\320\265\320\273\320\265\320\275\320\275\321\213\320\265", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(SchemaTab), QCoreApplication::translate("MainWindow", "\320\241\321\205\320\265\320\274\320\260", nullptr));
        menu->setTitle(QCoreApplication::translate("MainWindow", "\320\244\320\260\320\271\320\273", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SAPR_H
