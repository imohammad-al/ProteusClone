/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.8.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *actionSelect;
    QAction *actionWire;
    QAction *actionResistor;
    QWidget *centralwidget;
    QHBoxLayout *horizontalLayout;
    QWidget *leftPanel;
    QVBoxLayout *verticalLayout;
    QLineEdit *lineEditSearch;
    QTreeWidget *treeWidgetComponents;
    QGraphicsView *graphicsView;
    QMenuBar *menubar;
    QStatusBar *statusbar;
    QToolBar *mainToolBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(1200, 750);
        actionSelect = new QAction(MainWindow);
        actionSelect->setObjectName("actionSelect");
        actionWire = new QAction(MainWindow);
        actionWire->setObjectName("actionWire");
        actionResistor = new QAction(MainWindow);
        actionResistor->setObjectName("actionResistor");
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        horizontalLayout = new QHBoxLayout(centralwidget);
        horizontalLayout->setObjectName("horizontalLayout");
        leftPanel = new QWidget(centralwidget);
        leftPanel->setObjectName("leftPanel");
        verticalLayout = new QVBoxLayout(leftPanel);
        verticalLayout->setObjectName("verticalLayout");
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        lineEditSearch = new QLineEdit(leftPanel);
        lineEditSearch->setObjectName("lineEditSearch");

        verticalLayout->addWidget(lineEditSearch);

        treeWidgetComponents = new QTreeWidget(leftPanel);
        treeWidgetComponents->setObjectName("treeWidgetComponents");
        treeWidgetComponents->setHeaderHidden(true);

        verticalLayout->addWidget(treeWidgetComponents);


        horizontalLayout->addWidget(leftPanel);

        graphicsView = new QGraphicsView(centralwidget);
        graphicsView->setObjectName("graphicsView");

        horizontalLayout->addWidget(graphicsView);

        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName("menubar");
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName("statusbar");
        MainWindow->setStatusBar(statusbar);
        mainToolBar = new QToolBar(MainWindow);
        mainToolBar->setObjectName("mainToolBar");
        MainWindow->addToolBar(Qt::ToolBarArea::TopToolBarArea, mainToolBar);

        mainToolBar->addAction(actionSelect);
        mainToolBar->addAction(actionWire);
        mainToolBar->addSeparator();
        mainToolBar->addAction(actionResistor);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "Proteus Clone", nullptr));
        actionSelect->setText(QCoreApplication::translate("MainWindow", "Select", nullptr));
        actionWire->setText(QCoreApplication::translate("MainWindow", "Wire", nullptr));
        actionResistor->setText(QCoreApplication::translate("MainWindow", "Resistor", nullptr));
        lineEditSearch->setPlaceholderText(QCoreApplication::translate("MainWindow", "Search component...", nullptr));
        QTreeWidgetItem *___qtreewidgetitem = treeWidgetComponents->headerItem();
        ___qtreewidgetitem->setText(0, QCoreApplication::translate("MainWindow", "Components", nullptr));
        mainToolBar->setWindowTitle(QCoreApplication::translate("MainWindow", "Main Toolbar", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
