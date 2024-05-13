#include "editablecombobox.h"

EditableComboBox::EditableComboBox(QWidget *parent) : QComboBox(parent), editMode(false)
{
    // Create a line edit for manual text input
    lineEdit = new QLineEdit(this);
    setLineEdit(lineEdit);

    // Connect line edit's textChanged signal to update the combo box's text
    connect(lineEdit, &QLineEdit::textChanged, this, &EditableComboBox::updateComboBoxText);

    // Install event filter on line edit
    lineEdit->installEventFilter(this);
}


bool EditableComboBox::eventFilter(QObject *obj, QEvent *event)
{


    if (obj == lineEdit && event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent->button() == Qt::LeftButton) {


            editMode = true;


            qDebug() << "LineEdit part of combo box clicked";
            return true; // Return true to indicate the event has been handled
        }
    }

    // Pass the event on to the parent class
    return QComboBox::eventFilter(obj, event);
}


void EditableComboBox::keyPressEvent(QKeyEvent *event)
{

    /*
    qDebug() << "KEY" << event->key();

    if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {

        setEditText(lineEdit->text());
        //editMode = false;
        //lineEdit->setEnabled(false); // Disable line edit
        setFocus();
        lineEdit->clearFocus(); // Clear focus from line edit
        return;
    }


    // Handle arrow key events separately to prevent recursion
    if (event->key() == Qt::Key_Up || event->key() == Qt::Key_Down ||
        event->key() == Qt::Key_Left || event->key() == Qt::Key_Right) {

        qDebug() << "HEY ARROW";


        QComboBox::keyPressEvent(event); // Let the base class handle arrow key events
        return;
    }

    // For other key events, handle as before
    lineEdit->setFocus();
    QKeyEvent newEvent(event->type(), event->key(), event->modifiers(), event->text(), event->isAutoRepeat(), event->count());
    QApplication::sendEvent(lineEdit, &newEvent);
    */


    qDebug() << "KEY NEU:" << event->key();
    qDebug() << " ";


    // Check if the event key corresponds to a text input key
    if (event->key() == Qt::Key_Comma || event->key() == Qt::Key_Period ||
        event->key() == Qt::Key_Backspace || event->key() == Qt::Key_Delete ||
        event->key() == Qt::Key_Tab || (event->key() >= 48 && event->key() <= 57)) {
        // Text input key, handle it accordingly




        lineEdit->setFocus();
        QKeyEvent newEvent(event->type(), event->key(), event->modifiers(), event->text(), event->isAutoRepeat(), event->count());
        QApplication::sendEvent(lineEdit, &newEvent);
    }
    else if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter ||
            event->key() == Qt::Key_Escape){

        setEditText(lineEdit->text());
        setFocus();
        lineEdit->clearFocus();
    }
    else {
        // Non-text input key, handle it accordingly
        if (event->key() == Qt::Key_Up || event->key() == Qt::Key_Down ||
            event->key() == Qt::Key_Left || event->key() == Qt::Key_Right) {

            QComboBox::keyPressEvent(event);
        }
    }


/*
    // Check if the event key corresponds to a printable character
    if (!event->text().isEmpty()) {

        qDebug() << "KEY NOT E" << event->key();

        if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return || event->key() == Qt::Key_Escape) {

            setEditText(lineEdit->text());
            setFocus();
            lineEdit->clearFocus();
        }
        else{

            // Printable character, handle text input
            lineEdit->setFocus();
            QKeyEvent newEvent(event->type(), event->key(), event->modifiers(), event->text(), event->isAutoRepeat(), event->count());
            QApplication::sendEvent(lineEdit, &newEvent);
        }
    }
    else {

        qDebug() << "KEY EMPTY" << event->key();
        // Non-text input key, handle it accordingly


        if (event->key() == Qt::Key_Up || event->key() == Qt::Key_Down ||
                   event->key() == Qt::Key_Left || event->key() == Qt::Key_Right) {

            QComboBox::keyPressEvent(event);
        }
    }

*/

}

void EditableComboBox::updateComboBoxText(const QString &text)
{
    setEditText(text);
}

void EditableComboBox::mousePressEvent(QMouseEvent *event)
{

    qDebug() << "HEY MOUSE" << event->button();

    editMode = true;
    if (!editMode)
        showPopup();

    QComboBox::mousePressEvent(event);
}
