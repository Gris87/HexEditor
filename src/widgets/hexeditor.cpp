#include "hexeditor.h"

HexEditor::HexEditor(QWidget *parent) :
    QScrollArea(parent)
{
    mMode=INSERT;
    mReadOnly=false;
    mCursorPosition=0;
    mAddressBackgroundColor.setRgb(128, 128, 128);
    mHighlightingColor.setRgb(255, 0, 255);
    mSelectionColor.setRgb(0, 0, 255);
    mAddressVisible=true;
    mTextVisible=true;
    mHighlightingEnabled=true;
}

int HexEditor::indexOf(const QByteArray &aArray, int aFrom) const
{
    return mData.indexOf(aArray, aFrom);
}

int HexEditor::lastIndexOf(const QByteArray &aArray, int aFrom) const
{
    return mData.lastIndexOf(aArray, aFrom);
}

void HexEditor::insert(int aIndex, const QByteArray &aArray)
{
    // TODO: Implement insert
}

void HexEditor::insert(int aIndex, char aChar)
{
    // TODO: Implement insert
}

void HexEditor::remove(int aPos, int aLength)
{
    // TODO: Implement remove
}

void HexEditor::replace(int aPos, int aLength, const QByteArray &aArray)
{
    // TODO: Implement replace
}

QString HexEditor::toString()
{
    return QString::fromAscii(mData);
}

void HexEditor::undo()
{
    // TODO: Implement undo
}

void HexEditor::redo()
{
    // TODO: Implement redo
}

void HexEditor::updateScrollBars()
{
    // TODO: Implement updateScrollBars
}

QByteArray HexEditor::data() const
{
    return mData;
}

void HexEditor::setData(QByteArray const &aData)
{
    if (mData!=aData)
    {
        mData=aData;
        updateScrollBars();
        // TODO: Update cursor position
        update();

        emit dataChanged();
    }
}

HexEditor::Mode HexEditor::mode() const
{
    return mMode;
}

void HexEditor::setMode(const Mode &aMode)
{
    if (mMode!=aMode)
    {
        mMode=aMode;
        emit modeChanged(mMode);
    }
}

bool HexEditor::isReadOnly() const
{
    return mReadOnly;
}

void HexEditor::setReadOnly(const bool &aReadOnly)
{
    mReadOnly=aReadOnly;
}

int HexEditor::position() const
{
    return mCursorPosition/2;
}

void HexEditor::setPosition(int aPosition)
{
    if (mCursorPosition/2!=aPosition)
    {
        setCursorPosition(aPosition*2);
    }
}

qint64 HexEditor::cursorPosition() const
{
    return mCursorPosition;
}

void HexEditor::setCursorPosition(qint64 aCursorPos)
{
    if (mCursorPosition!=aCursorPos)
    {
        bool aSamePos=(mCursorPosition/2==aCursorPos/2);

        mCursorPosition=aCursorPos;
        update();

        if (!aSamePos)
        {
            emit positionChanged(mCursorPosition/2);
        }
    }
}

QColor HexEditor::addressBackgroundColor() const
{
    return mAddressBackgroundColor;
}

void HexEditor::setAddressBackgroundColor(QColor const &aColor)
{
    if (mAddressBackgroundColor!=aColor)
    {
        mAddressBackgroundColor=aColor;
        update();
    }
}

QColor HexEditor::highlightingColor() const
{
    return mHighlightingColor;
}

void HexEditor::setHighlightingColor(QColor const &aColor)
{
    if (mHighlightingColor!=aColor)
    {
        mHighlightingColor=aColor;
        update();
    }
}

QColor HexEditor::selectionColor() const
{
    return mSelectionColor;
}

void HexEditor::setSelectionColor(QColor const &aColor)
{
    if (mSelectionColor!=aColor)
    {
        mSelectionColor=aColor;
        update();
    }
}

bool HexEditor::isAddressVisible() const
{
    return mAddressVisible;
}

void HexEditor::setAddressVisible(const bool &aAddressVisible)
{
    if (mAddressVisible!=aAddressVisible)
    {
        mAddressVisible=aAddressVisible;
        updateScrollBars();
        update();
    }
}

bool HexEditor::isTextVisible() const
{
    return mTextVisible;
}

void HexEditor::setTextVisible(const bool &aTextVisible)
{
    if (mTextVisible!=aTextVisible)
    {
        mTextVisible=aTextVisible;
        updateScrollBars();
        update();
    }
}

bool HexEditor::isHighlightingEnabled() const
{
    return mHighlightingEnabled;
}

void HexEditor::setHighlightingEnabled(const bool &aEnable)
{
    if (mHighlightingEnabled!=aEnable)
    {
        mHighlightingEnabled=aEnable;
        updateScrollBars();
        update();
    }
}
