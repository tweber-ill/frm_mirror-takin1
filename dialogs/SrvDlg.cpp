/*
 * Server Dialog
 * @author Tobias Weber
 * @date 27-aug-2014
 */

#include "SrvDlg.h"

SrvDlg::SrvDlg(QWidget* pParent, QSettings *pSett)
		: QDialog(pParent), m_pSettings(pSett)
{
	this->setupUi(this);

	if(m_pSettings)
	{
		if(m_pSettings->contains("server/nicos-host"))
			editHost->setText(m_pSettings->value("server/nicos-host").toString());
		if(m_pSettings->contains("server/nicos-port"))
			editPort->setText(m_pSettings->value("server/nicos-port").toString());
	}
}

SrvDlg::~SrvDlg()
{
}

void SrvDlg::accept()
{
	QString strHost = editHost->text();
	QString strPort = editPort->text();

	if(m_pSettings)
	{
		m_pSettings->setValue("server/nicos-host", strHost);
		m_pSettings->setValue("server/nicos-port", strPort);
	}

	emit ConnectTo(strHost, strPort);
	QDialog::accept();
}

#include "SrvDlg.moc"
