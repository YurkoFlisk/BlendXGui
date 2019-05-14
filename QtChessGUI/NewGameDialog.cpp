#include "NewGameDialog.h"

using namespace BlendXChess;

NewGameDialog::NewGameDialog(QWidget *parent)
	: QDialog(parent)
{
	QPushButton* okButton = new QPushButton("&Ok");
	QPushButton* cancelButton = new QPushButton("&Cancel");
	m_sideCB = new QComboBox;

	m_sideCB->addItem("White", BlendXChess::WHITE);
	m_sideCB->addItem("Black", BlendXChess::BLACK);
	m_sideCB->addItem("Random", BlendXChess::NULL_COLOR);

	connect(okButton, &QPushButton::clicked, this, &NewGameDialog::accept);
	connect(cancelButton, &QPushButton::clicked, this, &NewGameDialog::reject);

	QFormLayout* mainLayout = new QFormLayout;
	mainLayout->addRow("Side: ", m_sideCB);
	mainLayout->addRow(okButton, cancelButton);
	setLayout(mainLayout);
}

NewGameDialog::~NewGameDialog()
{}

Side NewGameDialog::getSelectedSide(void) const
{
	return (Side)m_sideCB->currentData().toInt();
}
