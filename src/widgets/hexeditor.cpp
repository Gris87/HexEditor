#include "hexeditor.h"

#include <QPainter>
#include <QScrollBar>
#include <QKeyEvent>

#include <math.h>

#define LINE_INTERVAL 2
#define CHAR_INTERVAL 2

HexEditor::HexEditor(QWidget *parent) :
    QAbstractScrollArea(parent)
{
    for (int i=0; i<256; ++i)
    {
        char aChar=i;
        mAsciiChars.append(QString::fromAscii(&aChar, 1));
    }

    mAsciiChars[9] =QChar(26);
    mAsciiChars[10]=QChar(8629);
    mAsciiChars[13]=QChar(8601);
    mAsciiChars[32]=QChar(183);

    for (int i=128; i<=160; ++i)
    {
        mAsciiChars[i]=QChar(9734);
    }

    mMode=INSERT;
    mReadOnly=false;
    mCursorPosition=0;

    mFont=QFont("Courier new", 1);     //Special action to calculate mCharWidth and mCharHeight at the next step
    setFont(QFont("Courier new", 10));

    mCursorVisible=true;
    mCursorAtTheLeft=true;
    connect(&mCursorTimer, SIGNAL(timeout()), this, SLOT(cursorBlicking()));
    mCursorTimer.start(500);
}

void HexEditor::undo()
{
    // TODO: Implement undo
}

void HexEditor::redo()
{
    // TODO: Implement redo
}

void HexEditor::cursorBlicking()
{
    mCursorVisible=!mCursorVisible;
    viewport()->update();
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

void HexEditor::resetCursorTimer()
{
    mCursorVisible=true;

    mCursorTimer.stop();
    mCursorTimer.start(500);

    viewport()->update();
}

void HexEditor::resetSelection()
{
    // TODO: Implement resetSelection
}

void HexEditor::updateSelection()
{
    // TODO: Implement updateSelection
}

void HexEditor::scrollToCursor()
{
    int aOffsetX=-horizontalScrollBar()->value();
    int aOffsetY=-verticalScrollBar()->value();
    int aViewWidth=viewport()->width();
    int aViewHeight=viewport()->height();



    int aCurCol=mCursorPosition % 32;
    bool aIsSecondChar=(aCurCol & 1);
    aCurCol>>=1;

    int aCurRow=floor(mCursorPosition/32.0f);

    int aCursorX;
    int aCursorY=aCurRow*(mCharHeight+LINE_INTERVAL);

    if (mCursorAtTheLeft)
    {
        aCursorX=(mAddressWidth+1+aCurCol*3)*mCharWidth;

        if (aIsSecondChar)
        {
            aCursorX+=mCharWidth;
        }
    }
    else
    {
        aCursorX=(mAddressWidth+50+aCurCol)*mCharWidth;
    }




}

void HexEditor::resizeEvent(QResizeEvent *event)
{
    QAbstractScrollArea::resizeEvent(event);
    updateScrollBars();
}

void HexEditor::paintEvent(QPaintEvent * /*event*/)
{
    QPainter painter(viewport());
    QPalette aPalette=palette();

    int aOffsetX=-horizontalScrollBar()->value();
    int aOffsetY=-verticalScrollBar()->value();
    int aViewWidth=viewport()->width();
    int aViewHeight=viewport()->height();

    painter.setFont(mFont);
    painter.setPen(aPalette.color(QPalette::Text));

    // Draw background for chars (Selection and cursor)
    {
        // Check for selection
        if (false)
        {
            // Draw selection
        }
        else
        {
            // Draw cursor
            int aCurRow=floor(mCursorPosition/32.0f);
            int aCursorY=aCurRow*(mCharHeight+LINE_INTERVAL)+aOffsetY;

            if (aCursorY+mCharHeight>=0 && aCursorY<=aViewHeight)
            {
                int aCurCol=mCursorPosition % 32;
                bool aIsSecondChar=(aCurCol & 1);
                aCurCol>>=1;

                int aCursorX=(mAddressWidth+1+aCurCol*3)*mCharWidth+aOffsetX;

                if (aIsSecondChar)
                {
                    aCursorX+=mCharWidth;
                }

                if (
                    (
                     !mCursorAtTheLeft
                     ||
                     mCursorVisible
                    )
                    &&
                    (
                     aCursorX>=(mAddressWidth-1)*mCharWidth
                     &&
                     aCursorX<=aViewWidth
                    )
                   )
                {
                    painter.fillRect(aCursorX, aCursorY, mCharWidth, mCharHeight, aPalette.color(QPalette::Highlight));
                }

                aCursorX=(mAddressWidth+50+aCurCol)*mCharWidth+aOffsetX;

                if (
                    (
                     mCursorAtTheLeft
                     ||
                     mCursorVisible
                    )
                    &&
                    (
                     aCursorX>=(mAddressWidth-1)*mCharWidth
                     &&
                     aCursorX<=aViewWidth
                    )
                   )
                {
                    painter.fillRect(aCursorX, aCursorY, mCharWidth, mCharHeight, aPalette.color(QPalette::Highlight));
                }
            }
        }
    }

    // HEX data and ASCII characters
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

            // -----------------------------------------------------------------------------------------------------------------

            quint8 aAsciiChar=mData.at(i);
            QString aHexChar=QString::number(aAsciiChar, 16).toUpper();

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

            // -----------------------------------------------------------------------------------------------------------------

            aCharX=(mAddressWidth+50+aCurCol)*mCharWidth+aOffsetX;

            if (aCharX>=(mAddressWidth-2)*mCharWidth && aCharX<=aViewWidth)
            {
                painter.drawText(aCharX, aCharY, mCharWidth, mCharHeight, Qt::AlignCenter, mAsciiChars.at(aAsciiChar));
            }

            // -----------------------------------------------------------------------------------------------------------------

            ++aCurCol;

            if (aCurCol==16)
            {
                ++aCurRow;
                aCurCol=0;
            }
        }
    }

    // Line delimeter
    {
        int aLineX=(mAddressWidth+49)*mCharWidth+aOffsetX; // mAddressWidth + 1+16*2+15+1
        painter.drawLine(aLineX, 0, aLineX, aViewHeight);
    }

    // Address field at the left side
    {
        painter.fillRect(0, 0, mAddressWidth*mCharWidth, aViewHeight, aPalette.color(QPalette::AlternateBase));

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

void HexEditor::keyPressEvent(QKeyEvent *event)
{
    // =======================================================================================
    //                                     Movements
    // =======================================================================================

    if (event->matches(QKeySequence::MoveToNextChar))
    {
        setCursorPosition(mCursorPosition+1);
        resetSelection();
        resetCursorTimer();
        scrollToCursor();
    }
    else
    if (event->matches(QKeySequence::MoveToPreviousChar))
    {
        setCursorPosition(mCursorPosition - 1);
        resetSelection();
        resetCursorTimer();
        scrollToCursor();
    }
    else
    if (event->matches(QKeySequence::MoveToPreviousLine))
    {
        setCursorPosition(mCursorPosition-32);
        resetSelection();
        resetCursorTimer();
        scrollToCursor();
    }
    else
    if (event->matches(QKeySequence::MoveToNextLine))
    {
        setCursorPosition(mCursorPosition+32);
        resetSelection();
        resetCursorTimer();
        scrollToCursor();
    }
    else
    if (event->matches(QKeySequence::MoveToEndOfLine))
    {
        setCursorPosition(mCursorPosition | 31);
        resetSelection();
        resetCursorTimer();
        scrollToCursor();
    }
    else
    if (event->matches(QKeySequence::MoveToStartOfLine))
    {
        setCursorPosition(mCursorPosition-(mCursorPosition % 32));
        resetSelection();
        resetCursorTimer();
        scrollToCursor();
    }
    else
    if (event->matches(QKeySequence::MoveToNextPage))
    {
        setCursorPosition(mCursorPosition+viewport()->height()/(mCharHeight+LINE_INTERVAL)*32);
        resetSelection();
        resetCursorTimer();
        scrollToCursor();
    }
    else
    if (event->matches(QKeySequence::MoveToPreviousPage))
    {
        setCursorPosition(mCursorPosition-viewport()->height()/(mCharHeight+LINE_INTERVAL)*32);
        resetSelection();
        resetCursorTimer();
        scrollToCursor();
    }
    else
    if (event->matches(QKeySequence::MoveToEndOfDocument))
    {
        setCursorPosition(mData.size()*2);
        resetSelection();
        resetCursorTimer();
        scrollToCursor();
    }
    else
    if (event->matches(QKeySequence::MoveToStartOfDocument))
    {
        setCursorPosition(0);
        resetSelection();
        resetCursorTimer();
        scrollToCursor();
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
        viewport()->update();

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
    if (aCursorPos>mData.size()<<1)
    {
        aCursorPos=mData.size()<<1;
    }

    if (mCursorPosition!=aCursorPos)
    {
        bool aSamePos=((mCursorPosition>>1)==(aCursorPos>>1));

        mCursorPosition=aCursorPos;
        viewport()->update();

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
        viewport()->update();
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
