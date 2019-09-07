#include <QtWidgets>
#include "EngineParamsDialog.h"

EngineParamsDialog::EngineParamsDialog(QWidget *parent,
	const EngineOptions& options)
	: QDialog(parent)
{
	okButton = new QPushButton(tr("OK"));
	cancelButton = new QPushButton(tr("Cancel"));
	
	QFormLayout
		*checkLayout = new QFormLayout,
		*comboLayout = new QFormLayout,
		*spinLayout = new QFormLayout,
		*stringLayout = new QFormLayout;
	QGroupBox
		*checkGB = new QGroupBox(tr("Check options")),
		*comboGB = new QGroupBox(tr("Combo options")),
		*spinGB = new QGroupBox(tr("Spin options")),
		*stringGB = new QGroupBox(tr("String options"));
	QCheckBox* checkEdit;
	QComboBox* comboEdit;
	QSpinBox* spinEdit;
	QLineEdit* stringEdit;
	QWidget* editWidget;
	for (const auto& [name, option] : options)
	{
		const QString qname = QString::fromStdString(name) + ":";
		switch (option.getType())
		{
		case UciOption::Type::Check:
			editWidget = checkEdit = new QCheckBox;
			checkEdit->setChecked(option.getDefaultBool());
			checkLayout->addRow(qname, checkEdit);
			break;
		case UciOption::Type::Combo:
			editWidget = comboEdit = new QComboBox;
			for (const auto& comboVar : option.getComboVars())
				comboEdit->addItem(QString::fromStdString(comboVar));
			comboEdit->setCurrentText(QString::fromStdString(option.getDefaultString()));
			comboLayout->addRow(qname, comboEdit);
			break;
		case UciOption::Type::Spin:
			editWidget = spinEdit = new QSpinBox;
			spinEdit->setMinimum(option.getMin());
			spinEdit->setMaximum(option.getMax());
			spinEdit->setValue(option.getDefaultInt());
			spinLayout->addRow(qname, spinEdit);
			break;
		case UciOption::Type::String:
			editWidget = stringEdit = new QLineEdit;
			stringEdit->setText(QString::fromStdString(option.getDefaultString()));
			stringLayout->addRow(qname, stringEdit);
			break;
		}
		m_optionEdits.emplace(name, std::pair{ option.getType(), editWidget });
	}

	checkGB->setLayout(checkLayout);
	comboGB->setLayout(comboLayout);
	spinGB->setLayout(spinLayout);
	stringGB->setLayout(stringLayout);

	QHBoxLayout* buttonsLayout = new QHBoxLayout;
	buttonsLayout->addWidget(okButton);
	buttonsLayout->addWidget(cancelButton);

	QHBoxLayout* optionsLayout = new QHBoxLayout;
	optionsLayout->addWidget(checkGB);
	optionsLayout->addWidget(comboGB);
	optionsLayout->addWidget(spinGB);
	optionsLayout->addWidget(stringGB);
	QVBoxLayout* mainLayout = new QVBoxLayout;
	mainLayout->addLayout(optionsLayout);
	mainLayout->addLayout(buttonsLayout);
	setLayout(mainLayout);

	if (checkLayout->isEmpty())
		checkGB->hide();
	if (comboLayout->isEmpty())
		comboGB->hide();
	if (spinLayout->isEmpty())
		spinGB->hide();
	if (stringLayout->isEmpty())
		stringGB->hide();

	connect(okButton, &QPushButton::clicked, this, &EngineParamsDialog::accept);
	connect(cancelButton, &QPushButton::clicked, this, &EngineParamsDialog::reject);
}

EngineParamsDialog::~EngineParamsDialog() = default;

UciOption::ValueType EngineParamsDialog::getOptionValue(const std::string& name)
{
	
}
