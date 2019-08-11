#include <QtWidgets>
#include "EngineParamsDialog.h"

EngineParamsDialog::EngineParamsDialog(QWidget *parent,
	const EngineOptions& options)
	: QDialog(parent)
{
	okButton = new QPushButton("OK");
	cancelButton = new QPushButton("Cancel");
	
	QFormLayout
		*checkLayout = new QFormLayout,
		*comboLayout = new QFormLayout,
		*spinLayout = new QFormLayout,
		*stringLayout = new QFormLayout;
	QGroupBox
		*checkGB = new QGroupBox("Check options"),
		*comboGB = new QGroupBox("Combo options"),
		*spinGB = new QGroupBox("Spin options"),
		*stringGB = new QGroupBox("String options");
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
		optionEdits.emplace(name, std::pair{ option.getType(), editWidget });
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

EngineParamsDialog::~EngineParamsDialog(void)
{}

UciOption::ValueType EngineParamsDialog::getOptionValue(const std::string& name)
{
	auto it = optionEdits.find(name);
	if (it == optionEdits.end())
		throw std::invalid_argument("No option named '" + name + "'");
	QCheckBox* checkEdit;
	QComboBox* comboEdit;
	QSpinBox* spinEdit;
	QLineEdit* stringEdit;
	QWidget* editWidget = it->second.second;
	switch (it->second.first)
	{
	case UciOption::Type::Check:
		checkEdit = static_cast<QCheckBox*>(editWidget);
		return checkEdit->isChecked();
	case UciOption::Type::Combo:
		comboEdit = static_cast<QComboBox*>(editWidget);
		return comboEdit->currentText().toStdString();
	case UciOption::Type::Spin:
		spinEdit = static_cast<QSpinBox*>(editWidget);
		return spinEdit->value();
	case UciOption::Type::String:
		stringEdit = static_cast<QLineEdit*>(editWidget);
		return stringEdit->text().toStdString();
	default:
		throw std::runtime_error("Unexpected option type for editing");
	}
}
