#include "hexeditor.h"

HexEditor::HexEditor(QWidget *parent) :
    QScrollArea(parent)
{
}

int HexEditor::indexOf(const QByteArray &aArray, int aFrom) const
{
    return 0;
}

int HexEditor::lastIndexOf(const QByteArray &aArray, int aFrom) const
{
    return 0;
}

void HexEditor::insert(int aIndex, const QByteArray &aArray)
{

}

void HexEditor::insert(int aIndex, char aChar)
{

}

void HexEditor::remove(int aPos, int aLength)
{

}

void HexEditor::replace(int aPos, int aLength, const QByteArray &aArray)
{

}

QString HexEditor::toString()
{
    return "";
}

void HexEditor::undo()
{

}

void HexEditor::redo()
{

}

void HexEditor::setAddressVisible(const bool &aAddressVisible)
{

}

void HexEditor::setTextVisible(const bool &aTextVisible)
{

}

void HexEditor::setHighlightingEnabled(const bool &aEnable)
{

}

QByteArray HexEditor::data() const
{
    return QByteArray();
}

void HexEditor::setData(QByteArray const &aData)
{

}

HexEditor::Mode HexEditor::mode() const
{
    return INSERT;
}

void HexEditor::setMode(const Mode &aMode)
{

}

bool HexEditor::isReadOnly() const
{
    return false;
}

void HexEditor::setReadOnly(const bool &aReadOnly)
{

}

QFont HexEditor::font() const
{
    return QFont();
}

void HexEditor::setFont(const QFont &aFont)
{

}

int HexEditor::addressOffset() const
{
    return 0;
}

void HexEditor::setAddressOffset(int aOffset)
{

}

int HexEditor::cursorPosition() const
{
    return 0;
}

void HexEditor::setCursorPosition(int cusorPos)
{

}

QColor HexEditor::addressBackgroundColor() const
{
    return QColor();
}

void HexEditor::setAddressBackgroundColor(QColor const &color)
{

}

QColor HexEditor::highlightingColor() const
{
    return QColor();
}

void HexEditor::setHighlightingColor(QColor const &color)
{

}

QColor HexEditor::selectionColor() const
{
    return QColor();
}

void HexEditor::setSelectionColor(QColor const &color)
{

}
