#include "hexeditor.h"

#include <QPainter>
#include <QScrollBar>

#include <math.h>

#define LINE_INTERVAL 2
#define CHAR_INTERVAL 2

HexEditor::HexEditor(QWidget *parent) :
    QAbstractScrollArea(parent)
{
    mMode=INSERT;
    mReadOnly=false;
    mCursorPosition=0;
    mAddressBackgroundColor.setRgb(192, 192, 192);
    mHighlightingColor.setRgb(255, 0, 255);
    mSelectionColor.setRgb(0, 0, 255);
    mHighlightingEnabled=true;

    mFont=QFont("Courier new", 1);     //Special action to calculate mCharWidth and mCharHeight at the next step
    setFont(QFont("Courier new", 10));
}

void HexEditor::undo()
{
    // TODO: Implement undo
}

void HexEditor::redo()
{
    // TODO: Implement redo
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

// ------------------------------------------------------------------

void HexEditor::updateScrollBars()
{
    mAddressWidth=1;
    int aDataSize=mData.size();

    while (aDataSize>16)
    {
        ++mAddressWidth;
        aDataSize>>=4;
    }

    mLinesCount=floor(mData.size()/16.0f);

    if (mData.size() % 16!=0)
    {
        ++mLinesCount;
    }



    int aTotalWidth=(mAddressWidth+66)*mCharWidth; // mAddressWidth + 1+16*2+15+1 + 1+16
    int aTotalHeight=mLinesCount*mCharHeight;

    if (mLinesCount>0)
    {
        aTotalHeight+=(mLinesCount-1)*LINE_INTERVAL;
    }



    QSize areaSize=viewport()->size();

    horizontalScrollBar()->setPageStep(areaSize.width());
    verticalScrollBar()->setPageStep(areaSize.height());

    horizontalScrollBar()->setRange(0, aTotalWidth  - areaSize.width()  + 1);
    verticalScrollBar()->setRange(  0, aTotalHeight - areaSize.height() + 1);
}

void HexEditor::resizeEvent(QResizeEvent *event)
{
    QAbstractScrollArea::resizeEvent(event);
    updateScrollBars();
}

void HexEditor::paintEvent(QPaintEvent *event)
{
    QPainter painter(viewport());

    int aOffsetX=-horizontalScrollBar()->value();
    int aOffsetY=-verticalScrollBar()->value();
    int aViewWidth=viewport()->width();
    int aViewHeight=viewport()->height();

    painter.setFont(mFont);

    // HEX data
    {
        int aDataSize=mData.size();
        int aCurRow=0;
        int aCurCol=0;

        for (int i=0; i<aDataSize; ++i)
        {
            int aCharY=aCurRow*(mCharHeight+LINE_INTERVAL)+aOffsetY;

            if (aCharY+mCharHeight<0)
            {
                i+=15;
                ++aCurRow;
                continue;
            }
            else
            if (aCharY>aViewHeight)
            {
                break;
            }



            QString aHexChar=QString::number(mData.at(i), 16).toUpper();

            int aCharX=(mAddressWidth+1+aCurCol*3)*mCharWidth+aOffsetX;

            if (aCharX>=(mAddressWidth-2)*mCharWidth && aCharX<=aViewWidth)
            {
                if (aHexChar.length()<2)
                {
                    painter.drawText(aCharX,            aCharY, mCharWidth, mCharHeight, Qt::AlignCenter, "0");
                    painter.drawText(aCharX+mCharWidth, aCharY, mCharWidth, mCharHeight, Qt::AlignCenter, aHexChar.at(0));
                }
                else
                {
                    painter.drawText(aCharX,            aCharY, mCharWidth, mCharHeight, Qt::AlignCenter, aHexChar.at(0));
                    painter.drawText(aCharX+mCharWidth, aCharY, mCharWidth, mCharHeight, Qt::AlignCenter, aHexChar.at(1));
                }
            }



            ++aCurCol;

            if (aCurCol==16)
            {
                ++aCurRow;
                aCurCol=0;
            }
        }
    }

    // ASCII characters
    {
    }

    // Address field at the left side
    {
        painter.setBrush(QBrush(mAddressBackgroundColor));
        painter.fillRect(0, 0, mAddressWidth*mCharWidth, aViewHeight, mAddressBackgroundColor);

        for (int i=0; i<mLinesCount; ++i)
        {
            int aCharY=i*(mCharHeight+LINE_INTERVAL)+aOffsetY;

            if (aCharY+mCharHeight<0)
            {
                continue;
            }
            else
            if (aCharY>aViewHeight)
            {
                break;
            }



            QString aHexAddress=QString::number(i, 16).toUpper();

            for (int j=0; j<mAddressWidth; ++j)
            {
                int aCharX=j*mCharWidth;

                if (aCharX>aViewWidth)
                {
                    break;
                }

                QChar aHexChar;

                if (j<mAddressWidth-1 && mAddressWidth-j-2<aHexAddress.length())
                {
                    aHexChar=aHexAddress.at(mAddressWidth-j-2);
                }
                else
                {
                    aHexChar='0';
                }

                painter.drawText(aCharX, aCharY, mCharWidth, mCharHeight, Qt::AlignCenter, aHexChar);
            }
        }
    }
}

// ------------------------------------------------------------------

QByteArray HexEditor::data() const
{
    return mData;
}

void HexEditor::setData(QByteArray const &aData)
{
    if (mData!=aData)
    {
        mData=aData;

        if ((mCursorPosition>>1)>mData.size())
        {
            mCursorPosition=mData.size()<<1;
        }

        updateScrollBars();
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
    if (aCursorPos<0)
    {
        aCursorPos=0;
    }
    else
    if ((aCursorPos>>1)>mData.size())
    {
        aCursorPos=mData.size()<<1;
    }

    if (mCursorPosition!=aCursorPos)
    {
        bool aSamePos=((mCursorPosition>>1)==(aCursorPos>>1));

        mCursorPosition=aCursorPos;
        update();

        if (!aSamePos)
        {
            emit positionChanged(mCursorPosition>>1);
        }
    }
}

QFont HexEditor::font() const
{
    return mFont;
}

void HexEditor::setFont(const QFont &aFont)
{
    if (mFont!=aFont)
    {
        mFont=aFont;

        QFontMetrics aFontMetrics(mFont);

        mCharWidth=0;

        for (int i=0; i<16; ++i)
        {
            int aCharWidth=aFontMetrics.width(QString::number(i, 16).at(0));

            if (aCharWidth>mCharWidth)
            {
                mCharWidth=aCharWidth;
            }
        }

        mCharWidth+=CHAR_INTERVAL;
        mCharHeight=aFontMetrics.height()+CHAR_INTERVAL;

        updateScrollBars();
        update();
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

bool HexEditor::isHighlightingEnabled() const
{
    return mHighlightingEnabled;
}

void HexEditor::setHighlightingEnabled(const bool &aEnable)
{
    if (mHighlightingEnabled!=aEnable)
    {
        mHighlightingEnabled=aEnable;
        update();
    }
}

int HexEditor::charWidth()
{
    return mCharWidth;
}

int HexEditor::charHeight()
{
    return mCharHeight;
}

quint8 HexEditor::addressWidth()
{
    return mAddressWidth;
}

int HexEditor::linesCount()
{
    return mLinesCount;
}
