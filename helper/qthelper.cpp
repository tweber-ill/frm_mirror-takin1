/*
 * qt helper
 * @author Tobias Weber
 * @date feb-2016
 * @license GPLv2
 */

#include "qthelper.h"
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <memory>
#include "tlibs/math/math.h"


bool save_table(const char* pcFile, const QTableWidget* pTable)
{
	std::ofstream ofstr(pcFile);
	if(!ofstr)
		return false;

	const int iNumCols = pTable->columnCount();
	const int iNumRows = pTable->rowCount();

	// item lengths
	std::unique_ptr<int[]> ptrMaxTxtLen(new int[iNumCols]);
	for(int iCol=0; iCol<iNumCols; ++iCol)
		ptrMaxTxtLen[iCol] = 0;

	for(int iCol=0; iCol<iNumCols; ++iCol)
	{
		const QTableWidgetItem *pItem = pTable->horizontalHeaderItem(iCol);
		ptrMaxTxtLen[iCol] = std::max(pItem ? pItem->text().length() : 0, ptrMaxTxtLen[iCol]);
	}

	for(int iRow=0; iRow<iNumRows; ++iRow)
		for(int iCol=0; iCol<iNumCols; ++iCol)
		{
			const QTableWidgetItem *pItem = pTable->item(iRow, iCol);
			ptrMaxTxtLen[iCol] = std::max(pItem ? pItem->text().length() : 0, ptrMaxTxtLen[iCol]);
		}


	// write items
	for(int iCol=0; iCol<iNumCols; ++iCol)
	{
		const QTableWidgetItem *pItem = pTable->horizontalHeaderItem(iCol);
		ofstr << std::setw(ptrMaxTxtLen[iCol]+4) << (pItem ? pItem->text().toStdString() : "");
	}
	ofstr << "\n";

	for(int iRow=0; iRow<iNumRows; ++iRow)
	{
		for(int iCol=0; iCol<iNumCols; ++iCol)
		{
			const QTableWidgetItem *pItem = pTable->item(iRow, iCol);
			ofstr << std::setw(ptrMaxTxtLen[iCol]+4) << (pItem ? pItem->text().toStdString() : "");
		}

		ofstr << "\n";
	}

	return true;
}


void set_zoomer_base(QwtPlotZoomer *pZoomer,
	const std::vector<double>& vecX, const std::vector<double>& vecY)
{
	if(!pZoomer)
		return;

	const auto minmaxX = std::minmax_element(vecX.begin(), vecX.end());
	const auto minmaxY = std::minmax_element(vecY.begin(), vecY.end());
	//tl::log_debug("min: ", *minmaxX.first, " ", *minmaxY.first);
	//tl::log_debug("max: ", *minmaxX.second, " ", *minmaxY.second);

	double dminmax[] = {*minmaxX.first, *minmaxX.second,
		*minmaxY.first, *minmaxY.second};

	if(tl::float_equal(dminmax[0], dminmax[1]))
	{
		dminmax[0] -= dminmax[0]/10.;
		dminmax[1] += dminmax[1]/10.;
	}
	if(tl::float_equal(dminmax[2], dminmax[3]))
	{
		dminmax[2] -= dminmax[2]/10.;
		dminmax[3] += dminmax[3]/10.;
	}

	QRectF rect;
	rect.setLeft(dminmax[0]);
	rect.setRight(dminmax[1]);
	rect.setBottom(dminmax[3]);
	rect.setTop(dminmax[2]);

	pZoomer->zoom(rect);
	pZoomer->setZoomBase(rect);
}
