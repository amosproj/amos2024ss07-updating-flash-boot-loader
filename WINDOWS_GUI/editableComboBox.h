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
    bool editMode;

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void updateComboBoxText(const QString &text);

private:
    QLineEdit *lineEdit;

    QLabel *statusLabel;
};

#endif // EDITABLECOMBOBOX_H
