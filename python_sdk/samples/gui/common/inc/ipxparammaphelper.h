#ifndef IPXPARAMMAPHELPER_H
#define IPXPARAMMAPHELPER_H

#include "IpxCameraApi.h"

#include <QHash>
#include <QDebug>
#include <QTimer>
#include <QLabel>
#include <QVector>
#include <QString>
#include <QSlider>
#include <QSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QTabWidget>
#include <QPushButton>
#include <QMessageBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QStandardItemModel>

#include <limits>

//#define CHECK_INT_RANGE
//#undef max

namespace IpxGui
{
    class IIpxCamConfigIntConverter
    {
    public:
        virtual int64_t ToControl(int64_t value, IpxGenParam::Int *param = nullptr) = 0;
        virtual int64_t ToDevice(int64_t value, IpxGenParam::Int *param = nullptr) = 0;
        virtual ~IIpxCamConfigIntConverter(){}
    };

    enum class IpxCamConfigControlType
    {
        Undefined = -1,
        EnumComboBox,
        EnumCheckBox,
        EnumRadioButton,
        EnumTabWidget,
        CommandButton,
        BoolButton,
        IntEditBox,
        IntSpinBox,
        IntSlider
    };

    struct IpxCamConfigMapData
    {
        IpxCamConfigMapData()
            : controlType(IpxCamConfigControlType::Undefined)
            , control(nullptr)
            , userData(nullptr) {}

        IpxCamConfigMapData(IpxCamConfigControlType controlType, QWidget *control, void *userData = nullptr)
            : controlType(controlType)
            , control(control)
            , userData(userData) {}

        // we do not need copy and move operations
        // because it is simple structure and default one is enough

        // NOTE: Move operations will not be generated for any class that explicitly
        // declares a copy operation and declaring a move operation (construction or
        // assignment) in a class causes compilers to disable the copy operations
        //
        // C++11 deprecates the automatic generation of copy operations for classes declaring
        // copy operations or a destructor. This means that if you have code that depends on
        // the generation of copy operations in classes declaring a destructor or one of the copy
        // operations, you should consider upgrading these classes to eliminate the dependence.
        //
        // If Destructor is declared, move and copy operations will not be generated. (use '= default')

        IpxCamConfigControlType controlType;
        QWidget *control;
        void *userData;
    };

    // help class to facilitate developing
    class IpxCamConfigMapper : public IpxGenParam::ParamEventSink
    {

    public:

        IpxCamConfigMapper()
            : m_maxInt((std::numeric_limits<int>().max)())
        {}

        // clear map
        void clear(IpxGenParam::Array *params = nullptr)
        {
            if (params)
            {
                // it is faster to have two for loops than have one with if condition (params!=nullptr)
                for (auto iter = m_mapper.constBegin(); iter != m_mapper.constEnd(); ++iter)
                {
                    if (IpxGenParam::Param *param = params->GetParam(iter.key().toUtf8(), nullptr))
                        param->UnregisterEventSink(this);
                }
            }

            // disable control and disconnect all the slots
            for (auto iter = m_mapper.constBegin(); iter != m_mapper.constEnd(); ++iter)
                unmapControl(iter.value().first());

            m_mapper.clear();
            m_tabMapper.clear();
            m_radioButtonMapper.clear();
        }

        // returns a reference to map
        QHash<QString, QVector<IpxCamConfigMapData>> & paramMap() { return m_mapper; }

        // initialize a control representing combo box and
        // map a parameter with it for further easy work with it.
        // you can connect different slot to QComboBox::currentIndexChanged(const QString &),
        // but do not forget to disconnect first if you do not need default one (param->SetValueStr(value)).
        // note: all previous comboBox connections will be disconnected
        bool mapEnumParam(IpxGenParam::Enum *param, QComboBox *comboBox)
        {
            if (param)
            {
                // unregister for parameter update if it was regestered before
                param->UnregisterEventSink(this);

                // disconnect first so not to call slots on current index changed after clearing of the combo
                comboBox->disconnect();

                if (!param->IsAvailable())
                {
                    comboBox->setEnabled(false);
                    qDebug() << "mapEnumParam:" << param->GetDisplayName() << "is not available!";
                }
                else if (param->IsWritable())
                    comboBox->setEnabled(true);

                // clear combo box items ans fill in the new ones
                comboBox->clear();
                auto count = param->GetEnumEntriesCount();

                // go through and add items to the combo box
                for (decltype(count) i = 0; i < count; ++i)
                {
                    auto entry = param->GetEnumEntryByIndex(i);
                    if (entry)
                        comboBox->addItem(QString::fromUtf8(entry->GetValueStr()));
                }

                // add data to hash table of elements
                QString paramName = QString::fromUtf8(param->GetDisplayName());

                // if already mapped, unmap it
                if (m_mapper.contains(paramName))
                    unmapControl(m_mapper[paramName].first());

                m_mapper.insert(paramName, QVector<IpxCamConfigMapData>(1,
                    IpxCamConfigMapData(IpxCamConfigControlType::EnumComboBox, comboBox)));

                // update combo box
                updateComboBox(param, comboBox);

                // connect signal to lambda
                // if you want to receice signals when the change is caused by user interaction only,
                // then use activated() signal instead of currentIndexChanged
                QObject::connect(comboBox, static_cast<void(QComboBox::*)(const QString &)>(&QComboBox::currentIndexChanged),
                    [param](const QString &text){
                        IpxCamErr err = param->SetValueStr(text.toUtf8());
                        qDebug() << "on QComboBox::currentIndexChanged";
                        qDebug() << "errcode:" << err << "=" << param->GetDisplayName() << "-> SetValueStr(" << text.toUtf8() << ")";
                        if (err != IPX_CAM_ERR_OK)
                            QMessageBox::critical(nullptr, QString(),
                                QString("Error while setting the %1 enumeration value!\r\nError Code: %2").arg(param->GetDisplayName()).arg(err));
                });

                // register for parameter update
                param->RegisterEventSink(this);
            }
            else
            {
                comboBox->setEnabled(false);
                qDebug() << "mapEnumParam: IpxGenParam::Enum param is not found!";
                return false;
            }

            return true;
        }

        // initialize a control representing check box and
        // map a parameter with it for further easy work with it.
        // enum parameter should have two states off and on
        // you can connect different slot to QCheckBox::stateChanged(int),
        // but do not forget to disconnect first if you do not need default one
        // (param->SetValue((int64_t)(state == 0 ? 0 : 1))).
        // note: all previous checkBox connections will be disconnected
        bool mapEnumParam(IpxGenParam::Enum *param, QCheckBox *checkBox)
        {
            if (param)
            {
                // unregister for parameter update if it was regestered before
                param->UnregisterEventSink(this);

                // disconnect first so not to call slots
                checkBox->disconnect();

                if (!param->IsAvailable())
                {
                    checkBox->setEnabled(false);
                    qDebug() << "mapEnumParam:" << param->GetDisplayName() << "is not available!";
                }
                else if (param->IsWritable())
                    checkBox->setEnabled(true);

                // add data to hash table of elements
                QString paramName = QString::fromUtf8(param->GetDisplayName());

                // if already mapped, unmap it
                if (m_mapper.contains(paramName))
                    unmapControl(m_mapper[paramName].first());

                m_mapper.insert(paramName, QVector<IpxCamConfigMapData>(1,
                    IpxCamConfigMapData(IpxCamConfigControlType::EnumCheckBox, checkBox)));

                // update combo box
                updateCheckBox(param, checkBox);

                // connect signal to lambda
                QObject::connect(checkBox, static_cast<void(QAbstractButton::*)(bool)>(&QAbstractButton::clicked),
                    [param](bool checked){
                        IpxCamErr err = param->SetValue((int64_t)checked);
                        qDebug() << "on QAbstractButton::clicked";
                        qDebug() << "errcode:" << err << "=" << param->GetDisplayName() << "-> SetValue(" << (int64_t)checked << ")";
                        if (err != IPX_CAM_ERR_OK)
                            QMessageBox::critical(nullptr, QString(),
                                QString("Error while setting the %1 enumeration value!\r\nError Code: %2").arg(param->GetDisplayName()).arg(err));
                });

                // register for parameter update
                param->RegisterEventSink(this);
            }
            else
            {
                checkBox->setEnabled(false); 
                qDebug() << "mapEnumParam: IpxGenParam::Enum param is not found!";
                return false;
            }

            return true;
        }

        // initialize a control representing radio buttons and
        // map a parameter with it for further easy work with it.
        // button group buttons will have text the same as enum entry it maps to
        // every button in a group will be mapped to id that is value of an enum entry
        // you can connect different slot to QButtonGroup::buttonToggled(int, bool),
        // but do not forget to disconnect first if you do not need default one
        // (param->SetValue((int64_t)id)).
        // note: all previous button group connections will be disconnected
        bool mapEnumParam(IpxGenParam::Enum *param, QButtonGroup *radios)
        {
            if (param)
            {
                QString paramName = QString::fromUtf8(param->GetDisplayName());

                // unregister for parameter update if it was regestered before
                param->UnregisterEventSink(this);

                // disconnect first so not to call slots on current index changed after clearing of the combo
                radios->disconnect();

                QList<QAbstractButton*> radioButtons = radios->buttons();
                if (!param->IsAvailable())
                {
                    for (auto x: radioButtons)
                    {
                        x->setVisible(true);
                        x->setEnabled(false);
                    }

                    qDebug() << "mapEnumParam:" << param->GetDisplayName() << "is not available!";
                }
                else
                {
                    if (param->IsWritable())
                    {
                        for (auto x: radioButtons)
                        {
                            x->setVisible(true);
                            x->setEnabled(true);
                        }
                    }
                }

                // check the sizes first
                auto countRadios = radioButtons.size();
                auto count = (decltype(countRadios))param->GetEnumEntriesCount();
                if (count > countRadios)
                    count = countRadios;
                else if (count < countRadios) // hide other radio buttons
                {
                    for (decltype(countRadios) i = count; i < countRadios; ++i)
                        radioButtons.at(i)->setVisible(false);
                }

                // assign ids to radio buttons
                for (decltype(count) i = 0; i < count; ++i)
                {
                    auto entry = param->GetEnumEntryByIndex(i);
                    if (entry)
                    {
                        radioButtons.at(i)->setText(QString::fromUtf8(entry->GetValueStr()));
                        radios->setId(radioButtons.at(i), (int)entry->GetValue());
                    }
                    else
                        radioButtons.at(i)->setVisible(false);
                }

                // if already mapped, unmap it
                if (m_mapper.contains(paramName))
                    unmapControl(m_mapper[paramName].first());

                m_mapper.insert(paramName, QVector<IpxCamConfigMapData>(1,
                    IpxCamConfigMapData(IpxCamConfigControlType::EnumRadioButton, nullptr, radios)));

                updateRadioButtons(param, radios);

                // connect signal to lambda
                QObject::connect(radios, static_cast<void(QButtonGroup::*)(int, bool)>(&QButtonGroup::buttonToggled),
                    [param](int id, bool checked){
                        if (checked)
                        {
                            IpxCamErr err = param->SetValue((int64_t)id);
                            qDebug() << "on QButtonGroup::buttonToggled";
                            qDebug() << "errcode:" << err << "=" << param->GetDisplayName() << "-> SetValue(" << id << ")";
                            if (err != IPX_CAM_ERR_OK)
                                QMessageBox::critical(nullptr, QString(),
                                    QString("Error while setting the %1 enumeration value!\r\nError Code: %2").arg(param->GetDisplayName()).arg(err));
                        }
                });

                // register for parameter update
                param->RegisterEventSink(this);
            }
            else
            {
                for (auto x: radios->buttons())
                    x->setEnabled(false);

                qDebug() << "mapEnumParam: IpxGenParam::Enum param is not found!";
                return false;
            }

            return true;
        }

        // initialize a control representing tab widget with tabs and
        // map a parameter with it for further easy work with it.
        // tab names will have text the same as enum entry it maps to
        // you can connect different slot to QTabWidget::currentChanged(int),
        // but do not forget to disconnect first if you do not need default one (param->SetValueStr(tabName)).
        // note: all previous tab widget connections will be disconnected
        bool mapEnumParam(IpxGenParam::Enum *param, QTabWidget *tabs)
        {
            if (param)
            {
                // unregister for parameter update if it was regestered before
                param->UnregisterEventSink(this);

                // disconnect first so not to call slots on current index changed after clearing of the combo
                tabs->disconnect();

                if (!param->IsAvailable())
                {
                    tabs->setEnabled(false);
                    qDebug() << "mapEnumParam:" << param->GetDisplayName() << "is not available!";
                }
                else if (param->IsWritable())
                    tabs->setEnabled(true);

                // check the sizes first
                auto countTabs = tabs->count();
                auto count = (decltype(countTabs))param->GetEnumEntriesCount();
                if (count > countTabs)
                    count = countTabs;
                else if (count < countTabs) // hide other tabs
                {
                    for (decltype(countTabs) i = count; i < countTabs; ++i)
                    {
                        tabs->widget(i)->setVisible(false);
                        tabs->removeTab(i);
                    }
                }

                // add data to hash table of elements
                QString paramName = QString::fromUtf8(param->GetDisplayName());

                // go through and assign
                for (decltype(count) i = 0; i < count; ++i)
                {
                    auto entry = param->GetEnumEntryByIndex(i);
                    if (entry)
                    {
                        QString entryName = QString::fromUtf8(entry->GetValueStr());
                        tabs->setTabText(i, entryName);
                        m_tabMapper[paramName][entryName] = i;
                    }
                    else
                        tabs->widget(i)->setVisible(false);
                }

                // if already mapped, unmap it
                if (m_mapper.contains(paramName))
                    unmapControl(m_mapper[paramName].first());

                m_mapper.insert(paramName, QVector<IpxCamConfigMapData>(1,
                    IpxCamConfigMapData(IpxCamConfigControlType::EnumTabWidget, tabs)));

                // update combo box
                updateTabWidget(param, tabs);

                // connect signal to lambda
                QObject::connect(tabs, &QTabWidget::currentChanged,
                    [param, tabs](int index){
                        IpxCamErr err = param->SetValueStr(tabs->tabText(index).toUtf8());
                        qDebug() << "on QComboBox::currentIndexChanged";
                        qDebug() << "errcode:" << err << "=" << param->GetDisplayName()
                            << "-> SetValueStr(" << tabs->tabText(index).toUtf8() << ")";
                        if (err != IPX_CAM_ERR_OK)
                            QMessageBox::critical(nullptr, QString(),
                                QString("Error while setting the %1 enumeration value!\r\nError Code: %2").arg(param->GetDisplayName()).arg(err));
                });

                // register for parameter update
                param->RegisterEventSink(this);
            }
            else
            {
                tabs->setEnabled(false);
                qDebug() << "mapEnumParam: IpxGenParam::Enum param is not found!";
                return false;
            }

            return true;
        }

        // initialize a control representing spin box and
        // map a parameter with it for further easy work with it.
        // you can connect different slot to QSpinBox::valueChanged(int),
        // but do not forget to disconnect first if you do not need default one (param->SetValue(value)).
        // note: all previous spinBox connections will be disconnected
        bool mapIntParam(IpxGenParam::Int *param, QSpinBox *spinBox)
        {
            if (param)
            {
                // unregister for parameter update if it was regestered before
                param->UnregisterEventSink(this);

                // disconnect so not to call slots
                spinBox->disconnect();

                if (!param->IsAvailable())
                {
                    spinBox->setEnabled(false);
                    qDebug() << "mapIntParam:" << param->GetDisplayName() << "is not available!";
                }
                else if (param->IsWritable())
                    spinBox->setEnabled(true);

                // add data to hash table of elements
                QString paramName = QString::fromUtf8(param->GetDisplayName());

                // if already mapped, unmap it
                if (m_mapper.contains(paramName))
                    unmapControl(m_mapper[paramName].first());

                m_mapper.insert(paramName, QVector<IpxCamConfigMapData>(1,
                    IpxCamConfigMapData(IpxCamConfigControlType::IntSpinBox, spinBox)));

                // update spin box
                updateSpinBox(param, spinBox);

                // connect signal to lambda
                QObject::connect(spinBox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),
                    [param](int value){
                        IpxCamErr err = param->SetValue(value);
                        qDebug() << "on QSpinBox::valueChanged";
                        qDebug() << "errcode:" << err << "=" << param->GetDisplayName() << "-> SetValue(" << value << ")";
                        if (err != IPX_CAM_ERR_OK)
                            QMessageBox::critical(nullptr, QString(),
                                QString("Error while setting the %1 integer value!\r\nError Code: %2").arg(param->GetDisplayName()).arg(err));
                });

                // register for parameter update
                param->RegisterEventSink(this);
            }
            else
            {
                spinBox->setEnabled(false);
                qDebug() << "mapIntParam: IpxGenParam::Int param is not found!";
                return false;
            }

            return true;
        }

        // initialize a control representing spin box and associated with it controls that
        // sets default value to it and also map a parameter with it for further easy work with it.
        // you can connect different slot to QSpinBox::valueChanged(int),
        // but do not forget to disconnect first if you do not need default one (param->SetValue(value)).
        // note: all previous spinBox connections will be disconnected
        bool mapIntParam(IpxGenParam::Int *param, QSpinBox *spinBox, const QVector<QWidget *> &boundControls)
        {
            if (param)
            {
                // unregister for parameter update if it was regestered before
                param->UnregisterEventSink(this);

                // disconnect so not to call slots
                spinBox->disconnect();

                if (!param->IsAvailable())
                {
                    spinBox->setEnabled(false);
                    for (auto control: boundControls)
                        control->setEnabled(false);

                    qDebug() << "mapIntParam:" << param->GetDisplayName() << "is not available!";
                }
                else if (param->IsWritable())
                {
                    spinBox->setEnabled(true);
                    for (auto control: boundControls)
                        control->setEnabled(true);
                }

                // add data to hash table of elements
                QString paramName = QString::fromUtf8(param->GetDisplayName());

                // if already mapped, unmap it
                if (m_mapper.contains(paramName))
                    unmapControl(m_mapper[paramName].first());

                auto iter = m_mapper.insert(paramName, QVector<IpxCamConfigMapData>(1,
                    IpxCamConfigMapData(IpxCamConfigControlType::IntSpinBox, spinBox)));
                for (auto control: boundControls)
                    iter->append(IpxCamConfigMapData(IpxCamConfigControlType::Undefined, control));

                // update spin box
                updateSpinBox(param, spinBox);

                // connect signal to lambda
                QObject::connect(spinBox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),
                    [param](int value){
                        IpxCamErr err = param->SetValue(value);
                        qDebug() << "on QtIpxSpinBox::valueChanged";
                        qDebug() << "errcode:" << err << "=" << param->GetDisplayName() << "-> SetValue(" << value << ")";
                        if (err != IPX_CAM_ERR_OK)
                            QMessageBox::critical(nullptr, QString(),
                                QString("Error while setting the %1 integer value!\r\nError Code: %2").arg(param->GetDisplayName()).arg(err));
                });

                // register for parameter update
                param->RegisterEventSink(this);
            }
            else
            {
                spinBox->setEnabled(false);
                for (auto control: boundControls)
                    control->setEnabled(false);

                qDebug() << "mapIntParam: IpxGenParam::Int param is not found!";
                return false;
            }

            return true;
        }

        // initialize a control representing edit box and
        // map a parameter with it for further easy work with it.
        // you can connect different slot to QLineEdit::editingFinished(),
        // but do not forget to disconnect first if you do not need default one (param->SetValue(value)).
        // note: all previous spinBox connections will be disconnected
        bool mapIntParam(IpxGenParam::Int *param, QLineEdit *editBox)
        {
            if (param)
            {
                // unregister for parameter update if it was regestered before
                param->UnregisterEventSink(this);

                // disconnect so not to call slots
                editBox->disconnect();

                if (!param->IsAvailable())
                {
                    editBox->setEnabled(false);
                    qDebug() << "mapIntParam:" << param->GetDisplayName() << "is not available!";
                }
                else if (param->IsWritable())
                    editBox->setEnabled(true);

                // add data to hash table of elements
                QString paramName = QString::fromUtf8(param->GetDisplayName());

                // if already mapped, unmap it
                if (m_mapper.contains(paramName))
                    unmapControl(m_mapper[paramName].first());

                m_mapper.insert(paramName, QVector<IpxCamConfigMapData>(1,
                    IpxCamConfigMapData(IpxCamConfigControlType::IntEditBox, editBox)));

                // update spin box
                updateEditBox(param, editBox);

                // connect signal to lambda
                QObject::connect(editBox, &QLineEdit::editingFinished,
                    [param, editBox](){
                        bool ok = false;
                        int value = editBox->text().toInt(&ok);
                        if (ok)
                        {
                            int64_t val = validate(value, param);
                            IpxCamErr err = param->SetValue(val);
                            qDebug() << "on QLineEdit::editingFinished";
                            qDebug() << "errcode:" << err << "=" << param->GetDisplayName() << "-> SetValue(" << val << ")";
                            if (val != (int64_t)value) // because we validate values can be different
                                editBox->setText(QString::number(param->GetValue()));

                            if (err != IPX_CAM_ERR_OK)
                                QMessageBox::critical(nullptr, QString(),
                                    QString("Error while setting the %1 integer value!\r\nError Code: %2").arg(param->GetDisplayName()).arg(err));
                        }
                        else
                        {
                            qDebug() << "on QLineEdit::editingFinished";
                            qDebug() << "Value cannot be converted from string to integer. Value:" << editBox->text();
                            editBox->setText(QString::number(param->GetValue()));
                        }
                });

                // register for parameter update
                param->RegisterEventSink(this);
            }
            else
            {
                editBox->setEnabled(false);
                qDebug() << "mapIntParam: IpxGenParam::Int param is not found!";
                return false;
            }

            return true;
        }

        // initialize a control representing slider
        // map a parameter with it for further easy work with it.
        // you can connect different slot to QSlider::valueChanged(int),
        // but do not forget to disconnect first if you do not need default one (param->SetValue(value)).
        // note: all previous spinBox connections will be disconnected
        bool mapIntParam(IpxGenParam::Int *param, QSlider *slider, IIpxCamConfigIntConverter *converter = nullptr)
        {
            if (param)
            {
                // unregister for parameter update if it was regestered before
                param->UnregisterEventSink(this);

                // disconnect so not to call slots
                slider->disconnect();

                if (!param->IsAvailable())
                {
                    slider->setEnabled(false);
                    qDebug() << "mapIntParam:" << param->GetDisplayName() << "is not available!";
                }
                else if (param->IsWritable())
                    slider->setEnabled(true);

                // add data to hash table of elements
                QString paramName = QString::fromUtf8(param->GetDisplayName());

                // if already mapped, unmap it
                if (m_mapper.contains(paramName))
                    unmapControl(m_mapper[paramName].first());

                m_mapper.insert(paramName, QVector<IpxCamConfigMapData>(1,
                    IpxCamConfigMapData(IpxCamConfigControlType::IntSlider, slider, converter)));

                // update slider
                updateSlider(param, slider, converter);

                if (converter)
                {
                    // connect signal to lambda
                    QObject::connect(slider, static_cast<void(QSlider::*)(int)>(&QSlider::valueChanged),
                        [param, converter](int value){
                            IpxCamErr err = param->SetValue(converter->ToDevice(value, param));
                            qDebug() << "on QSlider::valueChanged";
                            qDebug() << "errcode:" << err << "=" << param->GetDisplayName()
                                << "-> SetValue(converter->ToDevice(" << value << ")";
                            qDebug() << "converter->ToDevice(" << value << ") =" << converter->ToDevice(value, param);
                            if (err != IPX_CAM_ERR_OK)
                                QMessageBox::critical(nullptr, QString(),
                                    QString("Error while setting the %1 integer value!\r\nError Code: %2").arg(param->GetDisplayName()).arg(err));
                    });
                }
                else
                {
                    // connect signal to lambda
                    QObject::connect(slider, static_cast<void(QSlider::*)(int)>(&QSlider::valueChanged),
                        [param](int value){
                            IpxCamErr err = param->SetValue(value);
                            qDebug() << "on QSlider::valueChanged";
                            qDebug() << "errcode:" << err << "=" << param->GetDisplayName() << "-> SetValue(" << value << ")";
                            if (err != IPX_CAM_ERR_OK)
                                QMessageBox::critical(nullptr, QString(),
                                    QString("Error while setting the %1 integer value!\r\nError Code: %2").arg(param->GetDisplayName()).arg(err));
                    });
                }

                // register for parameter update
                param->RegisterEventSink(this);
            }
            else
            {
                slider->setEnabled(false);
                qDebug() << "mapIntParam: IpxGenParam::Int param is not found!";
                return false;
            }

            return true;
        }

        // initialize a control representing button and
        // map a parameter with it for further easy work with it.
        // you can connect different slot to QPushButton::clicked(bool checked),
        // but do not forget to disconnect first if you do not need default one (param->Execute()).
        // note: all previous button connections will be disconnected
        // if a timer is not nullptr, it will be start by calling start() method, you can
        // use timer to check if command is IsDone() and enable(disable) the button.
        // if a timer is not nullptr while you click button this button gets disabled so
        // in the timer slot you need to enable it if either IsDone is true or without checking anything
        bool mapCommandParam(IpxGenParam::Command *param, QPushButton *button, QTimer *timer = nullptr)
        {
            if (param)
            {
                // unregister for parameter update if it was regestered before
                param->UnregisterEventSink(this);

                // disconnect so not to call slots
                button->disconnect();

                if (!param->IsAvailable())
                {
                    button->setEnabled(false);
                    qDebug() << "mapCommandParam:" << param->GetDisplayName() << "is not available!";
                }
                else if (param->IsWritable())
                    button->setEnabled(true);

                // add data to hash table of elements
                QString paramName = QString::fromUtf8(param->GetDisplayName());

                // if already mapped, unmap it
                if (m_mapper.contains(paramName))
                    unmapControl(m_mapper[paramName].first());

                m_mapper.insert(paramName, QVector<IpxCamConfigMapData>(1,
                    IpxCamConfigMapData(IpxCamConfigControlType::CommandButton, button)));

                // connect signal to lambda
                QObject::connect(button, static_cast<void(QPushButton::*)(bool checked)>(&QPushButton::clicked),
                    [param, timer, button](bool checked){
                        IpxCamErr err = param->Execute();
                        qDebug() << "on QPushButton::clicked";
                        qDebug() << "errcode:" << err << "=" << param->GetDisplayName() << "-> Execute()";
                        if (timer)
                        {
                            button->setEnabled(false);
                            timer->start();
                        }

                        if (err != IPX_CAM_ERR_OK)
                            QMessageBox::critical(nullptr, QString(),
                                QString("Error while executing the %1 command!\r\nError Code: %2").arg(param->GetDisplayName()).arg(err));
                });

                // register for parameter update
                param->RegisterEventSink(this);
            }
            else
            {
                button->setEnabled(false);
                qDebug() << "mapCommandParam: IpxGenParam::Command param is not found!";
                return false;
            }

            return true;
        }

        // initialize a control representing abstract button (push button, check box, radio button) and
        // map a parameter with it for further easy work with abstract button.
        // note: all previous button connections will be disconnected
        // button should be checkable
        bool mapBooleanParam(IpxGenParam::Boolean *param, QAbstractButton *button)
        {
            if (param)
            {
                // unregister for parameter update if it was regestered before
                param->UnregisterEventSink(this);

                // disconnect so not to call slots
                button->disconnect();

                if (!param->IsAvailable())
                {
                    button->setEnabled(false);
                    qDebug() << "mapBooleanParam:" << param->GetDisplayName() << "is not available!";
                }
                else if (param->IsWritable())
                    button->setEnabled(true);

                // add data to hash table of elements
                QString paramName = QString::fromUtf8(param->GetDisplayName());

                // if already mapped, unmap it
                if (m_mapper.contains(paramName))
                    unmapControl(m_mapper[paramName].first());

                m_mapper.insert(paramName, QVector<IpxCamConfigMapData>(1,
                    IpxCamConfigMapData(IpxCamConfigControlType::BoolButton, button)));

                // update abstract button
                updateAbstractButton(param, button);

                // connect signal to lambda
                QObject::connect(button, static_cast<void(QAbstractButton::*)(bool)>(&QAbstractButton::clicked),
                    [param](bool checked){
                        IpxCamErr err = param->SetValue(checked);
                        qDebug() << "on QAbstractButton::clicked";
                        qDebug() << "errcode:" << err << "=" << param->GetDisplayName()
                            << "-> SetValue(" << (checked ? "true" : "false")  << ")";
                        if (err != IPX_CAM_ERR_OK)
                            QMessageBox::critical(nullptr, QString(),
                                QString("Error while setting the %1 boolean value!\r\nError Code: %2").arg(param->GetDisplayName()).arg(err));
                });

                // register for parameter update
                param->RegisterEventSink(this);
            }
            else
            {
                button->setEnabled(false);
                qDebug() << "mapBooleanParam: IpxGenParam::Boolean param is not found!";
                return false;
            }

            return true;
        }

        // unmap parameter and disable control
        void unmapParam(IpxGenParam::Param *param, QWidget *control)
        {
            if (param)
            {
                QString paramName = QString::fromUtf8(param->GetDisplayName());
                if (m_mapper.contains(paramName))
                {
                    // disable control and disconnect all the slots
                    unmapControl(m_mapper[paramName].first());
                    param->UnregisterEventSink(this);
                    m_mapper.remove(paramName);
                }
            }
            else
            {
                control->setEnabled(false);
                qDebug() << "unmapParam: param is not found!";
            }
        }

        void unmapControl(const IpxCamConfigMapData &mapData)
        {
            if (mapData.controlType == IpxCamConfigControlType::EnumRadioButton)
            {
                QButtonGroup *gr = static_cast<QButtonGroup*>(mapData.userData);
                if (gr)
                {
                    gr->disconnect();
                    for (auto x: gr->buttons())
                        x->setEnabled(false);
                }
            }
            else
            {
                if (mapData.control)
                {
                    mapData.control->setEnabled(false);
                    mapData.control->disconnect();
                }
            }
        }

        // update combo box representation
        void updateComboBox(IpxGenParam::Enum *param, QComboBox *comboBox)
        {
            if (param && comboBox)
            {
                // check if any entry status has been changed
                // we take quantity of combo box items because of the situation that should never happen
                // when the number of enum entries is not equal to the number of combo box items
                auto count = comboBox->count();
                if (!count)
                    return;

                // get model to check item visibility in the combo box
                // by default combo box is using QStandardItemModel
                auto cbModel = dynamic_cast<QStandardItemModel*>(comboBox->model());
                if (cbModel)
                {
                    // go through and check items of the combo box
                    for (decltype(count) i = 0; i < count; ++i)
                    {
                        auto entry = param->GetEnumEntryByIndex(i);
                        auto item = cbModel->item(i);
                        if (entry && item)
                        {
                            // enable the item if the entry is available, otherwise disable it
                            if (entry->IsAvailable())
                                item->setEnabled(true);
                            else
                                item->setEnabled(false);
                        }
                    }
                }

                IpxCamErr err = IPX_CAM_ERR_OK;
                comboBox->setCurrentText(QString::fromUtf8(param->GetValueStr(&err)));
                qDebug() << err << "=" << param->GetDisplayName() << "-> GetValueStr() ="
                    << QString::fromUtf8(param->GetValueStr());
            }
        }

        // update check box representation
        void updateCheckBox(IpxGenParam::Enum *param, QCheckBox *checkBox)
        {
            if (param && checkBox)
            {
                IpxCamErr err = IPX_CAM_ERR_OK;
                checkBox->setChecked(param->GetValue(&err));
                qDebug() << err << "=" << param->GetDisplayName() << "-> GetValue() ="
                    << (param->GetValue() ? "true" : "false");
            }
        }

        // update radio buttons representation
        void updateRadioButtons(IpxGenParam::Enum *param, QButtonGroup *radios)
        {
            if (param && radios)
            {
                IpxCamErr err = IPX_CAM_ERR_OK;
                int id = static_cast<int>(param->GetValue(&err));
                qDebug() << err << "=" << param->GetDisplayName() << "-> GetValue() =" << id;
                if (auto btn = radios->button(id))
                {
                    btn->setChecked(true);
                    qDebug() << "Radio Button with id =" << id << "was checked";
                }

                // go through and check availability of entries
                auto count = param->GetEnumEntriesCount();
                for (decltype(count) i = 0; i < count; ++i)
                {
                    auto entry = param->GetEnumEntryByIndex(i);
                    if (entry)
                    {
                        auto btn = radios->button((int)entry->GetValue());
                        if (btn)
                        {
                            // enable the button if the entry is available, otherwise disable it
                            if (entry->IsAvailable())
                                btn->setEnabled(true);
                            else
                                btn->setEnabled(false);
                        }
                    }
                }
            }
        }

        // update tab widget representation
        void updateTabWidget(IpxGenParam::Enum *param, QTabWidget *tabs)
        {
            if (param && tabs)
            {
                IpxCamErr err = IPX_CAM_ERR_OK;
                auto& internalHash = m_tabMapper[QString::fromUtf8(param->GetDisplayName())];
                auto index = internalHash[QString::fromUtf8(param->GetValueStr(&err))];
                tabs->setCurrentIndex(index);
                qDebug() << err << "=" << param->GetDisplayName() << "-> GetValueStr() ="
                    << QString::fromUtf8(param->GetValueStr());
                qDebug() << "Tab Widget with index =" << index << "was checked";

                // go through and check availability of entries
                size_t count = param->GetEnumEntriesCount();
                size_t countTabs = (size_t)tabs->count();
                if (count > countTabs)
                    count = countTabs;
                for (decltype(count) i = 0; i < count; ++i)
                {
                    auto entry = param->GetEnumEntryByIndex(i);
                    if (entry)
                    {
                        auto tab = tabs->widget(internalHash[QString::fromUtf8(entry->GetValueStr())]);
                        if (tab)
                        {
                            // enable the tab if the entry is available, otherwise disable it
                            if (entry->IsAvailable())
                                tab->setEnabled(true);
                            else
                                tab->setEnabled(false);
                        }
                    }
                }
            }
        }

        // update spin box representation
        void updateSpinBox(IpxGenParam::Int *param, QSpinBox *spinBox)
        {
            if (param && spinBox)
            {
                IpxCamErr err = IPX_CAM_ERR_OK;
#ifndef CHECK_INT_RANGE
                spinBox->setRange((int)param->GetMin(), (int)param->GetMax());

                // set step (increment)
                int step = param->GetIncrement();
                spinBox->setSingleStep(step == 0 ? 1 : step);

                spinBox->setValue((int)param->GetValue(&err));
                qDebug() << err << "=" << param->GetDisplayName() << "-> GetValue() ="
                    << param->GetValue();
#else
                int64_t max = param->GetMax();
                spinBox->setRange((int)param->GetMin(), (int)(m_maxInt > max ? max : m_maxInt));

                // set step (increment)
                int step = param->GetIncrement();
                spinBox->setSingleStep(step == 0 ? 1 : step);

                int64_t val = param->GetValue(&err);
                spinBox->setValue((int)(m_maxInt > val ? val : m_maxInt));
                qDebug() << err << "=" << param->GetDisplayName() << "-> GetValue() =" << val;
#endif
            }
        }

        // update spin box representation
        void updateEditBox(IpxGenParam::Int *param, QLineEdit *editBox)
        {
            if (param && editBox)
            {
                IpxCamErr err = IPX_CAM_ERR_OK;
#ifndef CHECK_INT_RANGE
                QIntValidator *validator = nullptr;
                if ((validator = dynamic_cast<QIntValidator*>(const_cast<QValidator*>(editBox->validator()))))
                    validator->setRange((int)param->GetMin(), (int)param->GetMax());

                editBox->setText(QString::number((int)param->GetValue(&err)));
                qDebug() << err << "=" << param->GetDisplayName() << "-> GetValue() ="
                    << param->GetValue();
#else
                int64_t max = param->GetMax();
                QIntValidator *validator = nullptr;
                if ((validator = dynamic_cast<QIntValidator *>(const_cast<QValidator *>(editBox->validator()))))
                    validator->setRange((int)param->GetMin(), (int)(m_maxInt > max ? max : m_maxInt));

                int64_t val = param->GetValue(&err);
                editBox->setText(QString::number((int)(m_maxInt > val ? val : m_maxInt)));
                qDebug() << err << "=" << param->GetDisplayName() << "-> GetValue() =" << val;
#endif
            }
        }

        // update slider spin box representation
        void updateSlider(IpxGenParam::Int *param, QSlider *slider,
            IIpxCamConfigIntConverter *converter = nullptr)
        {
            if (param && slider)
            {
                IpxCamErr err = IPX_CAM_ERR_OK;
#ifndef CHECK_INT_RANGE
                if (converter)
                {
                    slider->setRange((int)converter->ToControl(param->GetMin(), param),
                        (int)converter->ToControl(param->GetMax(), param));

                    // set step (increment)
                    int step = converter->ToControl(param->GetIncrement(), param);
                    slider->setSingleStep(step == 0 ? 1 : step);

                    slider->setValue((int)converter->ToControl(param->GetValue(&err), param));
                    qDebug() << err << "=" << param->GetDisplayName() << "-> GetValue() ="
                        << param->GetValue();
                    qDebug()  << "converter->ToControl(" << param->GetValue()
                        << ") = " << converter->ToControl(param->GetValue(), param);
                }
                else
                {
                    slider->setRange((int)param->GetMin(), (int)param->GetMax());

                    // set step (increment)
                    int step = param->GetIncrement();
                    slider->setSingleStep(step == 0 ? 1 : step);

                    slider->setValue((int)param->GetValue(&err));
                    qDebug() << err << "=" << param->GetDisplayName() << "-> GetValue() ="
                        << param->GetValue();
                }
#else
                int64_t max = param->GetMax();
                int64_t val = param->GetValue(&err);
                if (converter)
                {
                    int64_t maxConvert = converter->ToControl(max, param);
                    slider->setRange((int)converter->ToControl(param->GetMin(), param),
                        (int)(m_maxInt > maxConvert ? maxConvert : m_maxInt));

                    // set step (increment)
                    int step = converter->ToControl(param->GetIncrement(), param);
                    slider->setSingleStep(step == 0 ? 1 : step);

                    int64_t valConvert = converter->ToControl(val, param);
                    slider->setValue((int)(m_maxInt > valConvert ? valConvert : m_maxInt));
                    qDebug() << err << "=" << param->GetDisplayName() << "-> GetValue() =" << val;
                    qDebug()  << "converter->ToControl(" << val << ") = " << valConvert;
                }
                else
                {
                    slider->setRange((int)param->GetMin(), (int)(m_maxInt > max ? max : m_maxInt));

                    // set step (increment)
                    int step = param->GetIncrement();
                    slider->setSingleStep(step == 0 ? 1 : step);

                    slider->setValue((int)(m_maxInt > val ? val : m_maxInt));
                    qDebug() << err << "=" << param->GetDisplayName() << "-> GetValue() =" << val;
                }
#endif
            }
        }

        // update abstract button representation
        void updateAbstractButton(IpxGenParam::Boolean *param, QAbstractButton *button)
        {
            if (param && button)
            {
                IpxCamErr err = IPX_CAM_ERR_OK;
                button->setChecked(param->GetValue(&err));
                qDebug() << err << "=" << param->GetDisplayName() << "-> GetValue() ="
                    << (param->GetValue() ? "true" : "false");
            }
        }

        // general on parameter update processor
        void OnParameterUpdate(IpxGenParam::Param *param)
        {
            // check if we need to track changing of this parameter
            QString paramName = QString::fromUtf8(param->GetDisplayName());
            if (m_mapper.contains(paramName))
            {
                // get a reference to the vector of controls(representations)
                auto& value = m_mapper[paramName];

                // if parameter is not available, disable its representation
                if (!param->IsAvailable())
                {
                    for (auto& v: value)
                    {
                        if (v.controlType == IpxCamConfigControlType::EnumRadioButton)
                        {
                            QButtonGroup *gr = static_cast<QButtonGroup *>(v.userData);
                            for (auto x: gr->buttons())
                                x->setEnabled(false);
                        }
                        else
                            v.control->setEnabled(false);
                    }
                }
                else
                {
                    // if parameter is readable, update its representation
                    if (param->IsReadable() && !value.isEmpty())
                    {
                        qDebug() << "OnParameterUpdate";
                        for (auto& v: value)
                        {
                            switch (v.controlType)
                            {

                            case IpxCamConfigControlType::EnumComboBox:
                                updateComboBox(dynamic_cast<IpxGenParam::Enum*>(param),
                                    dynamic_cast<QComboBox*>(v.control));
                                break;

                            case IpxCamConfigControlType::EnumCheckBox:
                                updateCheckBox(dynamic_cast<IpxGenParam::Enum*>(param),
                                    dynamic_cast<QCheckBox*>(v.control));
                                break;

                            case IpxCamConfigControlType::EnumRadioButton:
                                updateRadioButtons(dynamic_cast<IpxGenParam::Enum*>(param),
                                    static_cast<QButtonGroup*>(v.userData));
                                break;

                            case IpxCamConfigControlType::EnumTabWidget:
                                updateTabWidget(dynamic_cast<IpxGenParam::Enum*>(param),
                                    dynamic_cast<QTabWidget*>(v.control));
                                break;

                            case IpxCamConfigControlType::CommandButton:
                                break;

                            case IpxCamConfigControlType::BoolButton:
                                updateAbstractButton(dynamic_cast<IpxGenParam::Boolean*>(param),
                                    dynamic_cast<QAbstractButton*>(v.control));
                                break;

                            case IpxCamConfigControlType::IntEditBox:
                                updateEditBox(dynamic_cast<IpxGenParam::Int*>(param),
                                    dynamic_cast<QLineEdit*>(v.control));
                                break;

                            case IpxCamConfigControlType::IntSpinBox:
                                updateSpinBox(dynamic_cast<IpxGenParam::Int*>(param),
                                    dynamic_cast<QSpinBox*>(v.control));
                                break;

                            case IpxCamConfigControlType::IntSlider:
                                updateSlider(dynamic_cast<IpxGenParam::Int*>(param),
                                    dynamic_cast<QSlider*>(v.control),
                                    static_cast<IIpxCamConfigIntConverter*>(v.userData));
                                break;

                            default:
                                qDebug() << param->GetDisplayName() << "| control type is undefined!";
                            }
                        }
                    }

                    // if parameter is writable, make it enabled to allow changing
                    // by changing its representation
                    if (param->IsWritable())
                    {
                        for (auto& v: value)
                        {
                            if (v.controlType == IpxCamConfigControlType::EnumRadioButton)
                            {
                                QButtonGroup *gr = static_cast<QButtonGroup*>(v.userData);
                                for (auto x: gr->buttons())
                                    x->setEnabled(true);
                            }
                            else
                                v.control->setEnabled(true);
                        }
                    }
                }
            }
        }

        // validate integer value
        static int64_t validate(int64_t value, IpxGenParam::Int *param)
        {
            int64_t min = param->GetMin();
            if (value <= min)
                return min;

            int64_t max = param->GetMax();
            int64_t step = param->GetIncrement();

            // to have normalized value all the time;
            value -= min;
            int64_t residual = value % step;
            if (residual)
            {
                if (residual < (step >> 1))
                    value -= residual;
                else
                    value += (step - residual);
            }

            value += min;
            if (value > max)
                return max;

            return value;
        }

    private:

        // for working in the method OnParameterUpdate so to have all the needed info on control
        QHash<QString, QVector<IpxCamConfigMapData>> m_mapper;

        // simple map for mapping names with tab index
        QHash<QString, QHash<QString, int>> m_tabMapper;

        // used for sorting radio buttons according values
        QMap<int, QString> m_radioButtonMapper;

        // used to keep maximum allowed 64 integer value
        int64_t m_maxInt;
    };
}

#endif // IPXPARAMMAPHELPER_H
