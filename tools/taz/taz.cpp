/*
 * Scattering Triangle
 * @author tweber
 * @date feb-2014
 */

#include "taz.h"

#include <iostream>
#include <algorithm>

#include <QtGui/QApplication>


TazDlg::TazDlg(QWidget* pParent) : QDialog(pParent)
{
	this->setupUi(this);

	gfxRecip->setRenderHints(QPainter::Antialiasing);
	gfxRecip->setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
	gfxRecip->setDragMode(QGraphicsView::ScrollHandDrag);
	gfxRecip->setScene(&m_sceneRecip);

	gfxReal->setRenderHints(QPainter::Antialiasing);
	gfxReal->setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
	gfxReal->setDragMode(QGraphicsView::ScrollHandDrag);
}

TazDlg::~TazDlg()
{}


#ifdef STANDALONE_TAZ
int main(int argc, char** argv)
{
	QApplication app(argc, argv);

	TazDlg dlg(0);
	return dlg.exec();
}
#endif


#include "taz.moc"
