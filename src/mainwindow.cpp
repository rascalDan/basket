/***************************************************************************
 *   Copyright (C) 2003 by S�astien Laot                                 *
 *   slaout@linux62.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "settings.h"
#include <qtabwidget.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qcursor.h>
#include <qwhatsthis.h>
#include <QList>
#include <qregexp.h>
#include <qbuttongroup.h>
#include <kstringhandler.h>
#include <ktoggleaction.h>

#include <ksqueezedtextlabel.h>
#include <qpoint.h>
#include <qpixmap.h>
#include <qinputdialog.h>
#include <kmenu.h>
#include <kiconloader.h>
#include <kiconeffect.h>
#include <qiconset.h>
#include <kaction.h>
#include <kapplication.h>
#include <klocale.h>
#include <kmenubar.h>
#include <kedittoolbar.h>
#include <kdebug.h>
#include <qsignalmapper.h>
#include <qstringlist.h>

#include <qpainter.h>
#include <qstyle.h>
#include <kglobalsettings.h>
#include <kstandarddirs.h>
#include <qdir.h>
#include <qstringlist.h>
#include <kmessagebox.h>
#include <kstatusbar.h>
#include <qtimer.h>
#include <qaction.h>
#include <kstdaccel.h>
#include <kglobalaccel.h>
#include <kshortcutsdialog.h>
#include <kpassivepopup.h>
#include <kconfig.h>
#include <kcolordialog.h>
#include <kaboutdata.h>

#include <kactioncollection.h>

#include <kdeversion.h>
#include <qdesktopwidget.h>
#include <kwindowsystem.h>

#include <QProgressBar>

#include "mainwindow.h"
#include "basket.h"
#include "basketproperties.h"
#include "note.h"
#include "noteedit.h"
#include "global.h"
//#include "addbasketwizard.h"
#include "newbasketdialog.h"
#include "basketfactory.h"
#include "popupmenu.h"
#include "xmlwork.h"
#include "debugwindow.h"
#include "notefactory.h"
#include "notedrag.h"
#include "tools.h"
#include "tag.h"
#include "formatimporter.h"
#include "softwareimporters.h"
#include "regiongrabber.h"
#include "password.h"
#include "bnpview.h"
#include "systemtray.h"
#include "clickablelabel.h"
#include "basketstatusbar.h"
#include <iostream>
#include <ksettings/dialog.h>
#include <kcmultidialog.h>
#include <KShortcutsDialog>
/** Container */

MainWindow::MainWindow(QWidget *parent, const char *name)
	: //TODO KMainWindow(parent, name != 0 ? name : "MainWindow")
	KXmlGuiWindow( parent )
	, m_settings(0)
	, m_quit(false)
{
	BasketStatusBar* bar = new BasketStatusBar(statusBar());
	//TODO m_baskets = new BNPView(this, "BNPViewApp", this, actionCollection(), bar);
	m_baskets = new BNPView(this, "BNPViewApp", dynamic_cast<KXMLGUIClient*>(this), actionCollection(), bar );
	setCentralWidget(m_baskets);

	setupActions();
	statusBar()->show();
	statusBar()->setSizeGripEnabled(true);

	setAutoSaveSettings(/*groupName=*/QString::fromLatin1("MainWindow"), /*saveWindowSize=*//*FIXME:false:Why was it false??*/true);

	//m_actShowToolbar->setChecked(   toolBar()->isVisible()   );
	m_actShowStatusbar->setChecked( statusBar()->isVisible() );
	connect( m_baskets,      SIGNAL(setWindowCaption(const QString &)), this, SLOT(setCaption(const QString &)));

//	InlineEditors::instance()->richTextToolBar();
	setStandardToolBarMenuEnabled(true);

	createGUI("basketui.rc");
	applyMainWindowSettings( KConfigGroup( KGlobal::config(), autoSaveGroup() )  );
	//FIXME applyMainWindowSettings( KGlobal::config(), autoSaveGroup() );
}

MainWindow::~MainWindow()
{
	//FIXME saveMainWindowSettings( KGlobal::config(), autoSaveGroup());
	saveMainWindowSettings( KConfigGroup( KGlobal::config(), autoSaveGroup() ) );
	delete m_settings;
}

void MainWindow::setupActions()
{

	actQuit         = KStandardAction::quit( this, SLOT(quit()), actionCollection() );
	/*FIXME new KAction(i18n("Minimize"), "", 0,
				this, SLOT(minimizeRestore()), x, "minimizeRestore" );*/
	/** Settings : ************************************************************/
//	m_actShowToolbar   = KStandardAction::showToolbar(   this, SLOT(toggleToolBar()),   actionCollection());
	m_actShowStatusbar = KStandardAction::showStatusbar( this, SLOT(toggleStatusBar()), actionCollection());

//	m_actShowToolbar->setCheckedState( KGuiItem(i18n("Hide &Toolbar")) );

	(void) KStandardAction::keyBindings( this, SLOT(showShortcutsSettingsDialog()), actionCollection() );

	(void) KStandardAction::configureToolbars(this, SLOT(configureToolbars()), actionCollection() );

	//KAction *actCfgNotifs = KStandardAction::configureNotifications(this, SLOT(configureNotifications()), actionCollection() );
	//actCfgNotifs->setEnabled(false); // Not yet implemented !

	actAppConfig = KStandardAction::preferences( this, SLOT(showSettingsDialog()), actionCollection() );
}

/*void MainWindow::toggleToolBar()
{
	if (toolBar()->isVisible())
		toolBar()->hide();
	else
		toolBar()->show();

	saveMainWindowSettings( KGlobal::config(), autoSaveGroup() );
}*/

void MainWindow::toggleStatusBar()
{
	if (statusBar()->isVisible())
		statusBar()->hide();
	else
		statusBar()->show();

	//FIXME saveMainWindowSettings( KGlobal::config(), autoSaveGroup() );
	saveMainWindowSettings( KConfigGroup( KGlobal::config(), autoSaveGroup() ) );
}

void MainWindow::configureToolbars()
{
	saveMainWindowSettings( KConfigGroup( KGlobal::config(), autoSaveGroup() ) );

	KEditToolBar dlg(actionCollection());
	connect( &dlg, SIGNAL(newToolbarConfig()), this, SLOT(slotNewToolbarConfig()) );
	dlg.exec();
}

void MainWindow::configureNotifications()
{
	//FIXME
	//KNotifyDialog *dialog = new KNotifyDialog(this, "KNotifyDialog", false);
	//dialog->show();
}

void MainWindow::slotNewToolbarConfig() // This is called when OK or Apply is clicked
{
	// ...if you use any action list, use plugActionList on each here...
	createGUI("basketui.rc"); // TODO: Reconnect tags menu aboutToShow() ??
	if (!Global::bnpView->isPart())
		Global::bnpView->connectTagsMenu(); // The Tags menu was created again!
	//TODO plugActionList( QString::fromLatin1("go_baskets_list"), actBasketsList);
	applyMainWindowSettings( KConfigGroup( KGlobal::config(), autoSaveGroup() ) );
}

void MainWindow::showSettingsDialog()
{
	if(m_settings == 0)
		m_settings = new KSettings::Dialog(kapp->activeWindow());
	if (Global::mainWindow()) {
//TODO		m_settings->dialog()->showButton(KDialog::Help,    false); // Not implemented!
//TODO		m_settings->dialog()->showButton(KDialog::Default, false); // Not implemented!
//TODO		m_settings->dialog()->exec();
	} else
		m_settings->show();
}

void MainWindow::showShortcutsSettingsDialog()
{
	//TODO KShortcutsDialog::configure(actionCollection(), "basketui.rc");
	//.setCaption(..)
	//actionCollection()->writeSettings();
}

void MainWindow::polish()
{
	bool shouldSave = false;

	// If position and size has never been set, set nice ones:
	//  - Set size to sizeHint()
	//  - Keep the window manager placing the window where it want and save this
	if (Settings::mainWindowSize().isEmpty()) {
//		std::cout << "Main Window Position: Initial Set in show()" << std::endl;
		int defaultWidth  = kapp->desktop()->width()  * 5 / 6;
		int defaultHeight = kapp->desktop()->height() * 5 / 6;
		resize(defaultWidth, defaultHeight); // sizeHint() is bad (too small) and we want the user to have a good default area size
		shouldSave = true;
	} else {
//		std::cout << "Main Window Position: Recall in show(x="
//		          << Settings::mainWindowPosition().x() << ", y=" << Settings::mainWindowPosition().y()
//		          << ", width=" << Settings::mainWindowSize().width() << ", height=" << Settings::mainWindowSize().height()
//		          << ")" << std::endl;
		//move(Settings::mainWindowPosition());
		//resize(Settings::mainWindowSize());
	}

	//KMainWindow::polish();

	if (shouldSave) {
//		std::cout << "Main Window Position: Save size and position in show(x="
//		          << pos().x() << ", y=" << pos().y()
//		          << ", width=" << size().width() << ", height=" << size().height()
//		          << ")" << std::endl;
		Settings::setMainWindowPosition(pos());
		Settings::setMainWindowSize(size());
		Settings::saveConfig();
	}
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
//	std::cout << "Main Window Position: Save size in resizeEvent(width=" << size().width() << ", height=" << size().height() << ") ; isMaximized="
//	          << (isMaximized() ? "true" : "false") << std::endl;
	Settings::setMainWindowSize(size());
	Settings::saveConfig();

	// Added to make it work (previous lines do not work):
	//saveMainWindowSettings( KGlobal::config(), autoSaveGroup() );
	KMainWindow::resizeEvent(event);
}

void MainWindow::moveEvent(QMoveEvent *event)
{
//	std::cout << "Main Window Position: Save position in moveEvent(x=" << pos().x() << ", y=" << pos().y() << ")" << std::endl;
	Settings::setMainWindowPosition(pos());
	Settings::saveConfig();

	// Added to make it work (previous lines do not work):
	//saveMainWindowSettings( KGlobal::config(), autoSaveGroup() );
	KMainWindow::moveEvent(event);
}

bool MainWindow::queryExit()
{
	hide();
	return true;
}

void MainWindow::quit()
{
	m_quit = true;
	close();
}

bool MainWindow::queryClose()
{
/*	if (m_shuttingDown) // Set in askForQuit(): we don't have to ask again
	return true;*/

	if (kapp->sessionSaving()) {
		Settings::setStartDocked(false); // If queryClose() is called it's because the window is shown
		Settings::saveConfig();
		return true;
	}

	if (Settings::useSystray() && !m_quit) {
		Global::systemTray->displayCloseMessage(i18n("Basket"));
		hide();
		return false;
	} else
		return askForQuit();
}

bool MainWindow::askForQuit()
{
	//TODO QString message = i18n("<p>Do you really want to quit %1?</p>").arg(KCmdLineArgs::aboutData( )->programName());
	QString message = i18n("<p>Do you really want to quit ?</p>");
	if (Settings::useSystray())
		message += i18n("<p>Notice that you do not have to quit the application before ending your KDE session. "
				"If you end your session while the application is still running, the application will be reloaded the next time you log in.</p>");

	/*int really = KMessageBox::warningContinueCancel( this, message, i18n("Quit Confirm"),
			KStandardGuiItem::quit(), "confirmQuitAsking" );*/
	int really = KMessageBox::warningContinueCancel( this, message, i18n("Quit Confirm"),
			KStandardGuiItem::quit() );
	if (really == KMessageBox::Cancel)
	{
		m_quit = false;
		return false;
	}

	return true;
}

void MainWindow::minimizeRestore()
{
	if(isVisible())
		hide();
	else
		show();
}

void MainWindow::changeActive()
{
#if KDE_IS_VERSION( 3, 2, 90 ) // KDE 3.3.x
	kapp->updateUserTimestamp(); // If "activate on mouse hovering systray", or "on drag throught systray"
//	Global::systemTray->toggleActive();
#else
	setActive( ! isActiveWindow() );
#endif
}

#include "mainwindow.moc"
