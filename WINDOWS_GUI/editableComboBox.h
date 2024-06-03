// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Leon Wilms <leonwilms.wk@gmail.com>

#ifndef EDITABLECOMBOBOX_H
#define EDITABLECOMBOBOX_H

#include <QComboBox>
#include <QLineEdit>
#include <QKeyEvent>
#include <QApplication>
#include <QLabel>

class EditableComboBox : public QComboBox
{
    Q_OBJECT

public:
    EditableComboBox(QWidget *parent = nullptr);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    QLineEdit *lineEdit;
};

#endif // EDITABLECOMBOBOX_H
