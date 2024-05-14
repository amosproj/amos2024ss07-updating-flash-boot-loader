#include "editablecombobox.h"

EditableComboBox::EditableComboBox(QWidget *parent) : QComboBox(parent)
{
    // Create a line edit for manual text input
    lineEdit = new QLineEdit(this);
    setLineEdit(lineEdit);
}

// Tries to handle all key presses
// might need additional work if bugs show up
void EditableComboBox::keyPressEvent(QKeyEvent *event)
{
    //qDebug() << "KEY:" << event->key();
    //qDebug() << " ";

    // Check if the event key corresponds to a number input key
    if ((event->key() >= 48 && event->key() <= 57) ||
        event->key() == Qt::Key_Comma || event->key() == Qt::Key_Period ||
        event->key() == Qt::Key_Backspace || event->key() == Qt::Key_Delete ||
        event->key() == Qt::Key_Tab ) {


        lineEdit->setFocus();
        QKeyEvent newEvent(event->type(), event->key(), event->modifiers(),
                           event->text(), event->isAutoRepeat(), event->count());
        QApplication::sendEvent(lineEdit, &newEvent);
    }
    // Additional logic to enter the value after typing
    else if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter ||
            event->key() == Qt::Key_Escape){

        setEditText(lineEdit->text());
        setFocus();
        lineEdit->clearFocus();
    }
    // Other logic that might be interesting
    else {
        // Non-text input key, handle it accordingly
        if (event->key() == Qt::Key_Up || event->key() == Qt::Key_Down ||
            event->key() == Qt::Key_Left || event->key() == Qt::Key_Right) {

            QComboBox::keyPressEvent(event);
        }
    }
}

// Handles mouse press to the arrow to the right
void EditableComboBox::mousePressEvent(QMouseEvent *event)
{
    QComboBox::mousePressEvent(event);
}
