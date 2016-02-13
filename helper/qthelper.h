/*
 * qt helper
 * @author Tobias Weber
 * @date feb-2016
 * @license GPLv2
 */

#ifndef __QT_HELPER_H__
#define __QT_HELPER_H__

#include <vector>
#include <QTableWidget>
#include <qwt_plot.h>
#include <qwt_plot_zoomer.h>


extern bool save_table(const char* pcFile, const QTableWidget* pTable);

extern void set_zoomer_base(QwtPlotZoomer *pZoomer,
	const std::vector<double>& vecX, const std::vector<double>& vecY);


#endif
