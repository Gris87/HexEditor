#include "hexeditor.h"

#include <QPainter>
#include <QScrollBar>
#include <QKeyEvent>
#include <QApplication>
#include <QClipboard>

#include <math.h>

#define LINE_INTERVAL 2
#define CHAR_INTERVAL 2

HexEditor::HexEditor(QWidget *parent) :
    QAbstractScrollArea(parent)
{
    for (int i=0; i<256; ++i)
    {
        char aChar=i;
        mAsciiChars.append(QString::fromLatin1(&aChar, 1));
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

    mFont=QFont("Courier new", 1);     // Special action to calculate mCharWidth and mCharHeight at the next step
    setFont(QFont("Courier new", 10));

    mSelectionStart=0;
    mSelectionEnd=0;
    mSelectionInit=0;

    mCursorVisible=true;
    mCursorAtTheLeft=true;
    connect(&mCursorTimer, SIGNAL(timeout()), this, SLOT(cursorBlicking()));
    mCursorTimer.start(500);

    mLeftButtonPressed=false;
    mOneMoreSelection=false;
}

void HexEditor::undo()
{
    mUndoStack.undo();
    emit dataChanged();

    setCursorPosition(mCursorPosition);
    cursorMoved(false);

    updateScrollBars();
    viewport()->update();
}

void HexEditor::redo()
{
    mUndoStack.redo();
    emit dataChanged();

    setCursorPosition(mCursorPosition);
    cursorMoved(false);

    updateScrollBars();
    viewport()->update();
}

void HexEditor::cursorBlicking()
{
    mCursorVisible=!mCursorVisible;
    viewport()->update();
}

void HexEditor::scrollToCursor()
{
    int aOffsetX=horizontalScrollBar()->value();
    int aOffsetY=verticalScrollBar()->value();
    int aViewWidth=viewport()->width();
    int aViewHeight=viewport()->height();



    int aCurCol=(mCursorPosition % 32) >> 1;
    int aCurRow=floor(mCursorPosition/32.0f);

    int aCursorWidth;
    int aCursorX;
    int aCursorY=aCurRow*(mCharHeight+LINE_INTERVAL);

    if (mCursorAtTheLeft)
    {
        aCursorX=(mAddressWidth+1+aCurCol*3)*mCharWidth; // mAddressWidth + 1+aCurCol*3
        aCursorWidth=2*mCharWidth;
    }
    else
    {
        aCursorX=(mAddressWidth+50+aCurCol)*mCharWidth; // mAddressWidth + 1+16*2+15+1 + 1+aCurCol
        aCursorWidth=mCharWidth;
    }



    if (aCursorX-(mAddressWidth+1)*mCharWidth<aOffsetX)
    {
        horizontalScrollBar()->setValue(aCursorX-(mAddressWidth+1)*mCharWidth);
    }
    else
    if (aCursorX+aCursorWidth>aOffsetX+aViewWidth)
    {
        horizontalScrollBar()->setValue(aCursorX+aCursorWidth-aViewWidth);
    }

    if (aCursorY<aOffsetY)
    {
        verticalScrollBar()->setValue(aCursorY);
    }
    else
    if (aCursorY+mCharHeight+LINE_INTERVAL>aOffsetY+aViewHeight)
    {
        verticalScrollBar()->setValue(aCursorY+mCharHeight+LINE_INTERVAL-aViewHeight);
    }
}

int HexEditor::charAt(QPoint aPos, bool *aAtLeftPart)
{
    int aOffsetX=horizontalScrollBar()->value();
    int aOffsetY=verticalScrollBar()->value();

    int aRow         = floor((aPos.y()+aOffsetY)/((double)(mCharHeight+LINE_INTERVAL)));
    int aLeftColumn  = ((int)floor((aPos.x()+aOffsetX-(mAddressWidth+1)*mCharWidth)/((double)(mCharWidth*3))))<<1;
    int aRightColumn = floor((aPos.x()+aOffsetX-(mAddressWidth+50)*mCharWidth)/((double)mCharWidth));

    if (aPos.x()+aOffsetX-(mAddressWidth+1)*mCharWidth > mCharWidth*(aLeftColumn+(aLeftColumn>>1)+1))
    {
        ++aLeftColumn;
    }

    if (aAtLeftPart)
    {
        *aAtLeftPart=true;
    }

    if (aLeftColumn<0)
    {
        return aRow*32;
    }
    else
    if (aLeftColumn>=32)
    {
        if (aAtLeftPart)
        {
            *aAtLeftPart=false;
        }

        if (aRightColumn<0)
        {
            return aRow*32;
        }
        else
        if (aRightColumn>=15)
        {
            return aRow*32+30;
        }

        return aRow*32+(aRightColumn<<1);
    }
    else
    {
        return aRow*32+aLeftColumn;
    }
}

int HexEditor::indexOf(const QByteArray &aArray, int aFrom) const
{
    return mData.indexOf(aArray, aFrom);
}

int HexEditor::lastIndexOf(const QByteArray &aArray, int aFrom) const
{
    return mData.lastIndexOf(aArray, aFrom);
}

void HexEditor::insert(int aIndex, char aChar)
{
    SingleUndoCommand *aCommand=new SingleUndoCommand(this, SingleUndoCommand::Insert, aIndex, aChar);
    mUndoStack.push(aCommand);
    emit dataChanged();

    setCursorPosition(mCursorPosition);
    resetSelection();

    updateScrollBars();
    viewport()->update();
}

void HexEditor::insert(int aIndex, const QByteArray &aArray)
{
    if (aArray.length()==0)
    {
        return;
    }

    MultipleUndoCommand *aCommand;

    if (mMode==INSERT)
    {
        aCommand=new MultipleUndoCommand(this, MultipleUndoCommand::Insert, aIndex, aArray.length(), aArray);
    }
    else
    {
        aCommand=new MultipleUndoCommand(this, MultipleUndoCommand::Replace, aIndex, aArray.length(), aArray);
    }

    mUndoStack.push(aCommand);
    emit dataChanged();

    setCursorPosition(mCursorPosition);
    resetSelection();

    updateScrollBars();
    viewport()->update();
}

void HexEditor::remove(int aPos, int aLength)
{
    if (aLength<=0)
    {
        return;
    }

    QUndoCommand *aCommand;

    if (aLength==1)
    {
        if (mMode==INSERT)
        {
            aCommand=new SingleUndoCommand(this, SingleUndoCommand::Remove, aPos);
        }
        else
        {
            aCommand=new SingleUndoCommand(this, SingleUndoCommand::Replace, aPos, 0);
        }
    }
    else
    {
        if (mMode==INSERT)
        {
            aCommand=new MultipleUndoCommand(this, MultipleUndoCommand::Remove, aPos, aLength);
        }
        else
        {
            QByteArray aArray=QByteArray(aLength, 0);
            aCommand=new MultipleUndoCommand(this, MultipleUndoCommand::Replace, aPos, aLength, aArray);
        }
    }

    mUndoStack.push(aCommand);
    emit dataChanged();

    setCursorPosition(mCursorPosition);
    resetSelection();

    updateScrollBars();
    viewport()->update();
}

void HexEditor::replace(int aPos, char aChar)
{
    SingleUndoCommand *aCommand=new SingleUndoCommand(this, SingleUndoCommand::Replace, aPos, aChar);
    mUndoStack.push(aCommand);
    emit dataChanged();

    setCursorPosition(mCursorPosition);
    resetSelection();

    updateScrollBars();
    viewport()->update();
}

void HexEditor::replace(int aPos, const QByteArray &aArray)
{
    MultipleUndoCommand *aCommand=new MultipleUndoCommand(this, MultipleUndoCommand::Replace, aPos, aArray.length(), aArray);
    mUndoStack.push(aCommand);
    emit dataChanged();

    setCursorPosition(mCursorPosition);
    resetSelection();

    updateScrollBars();
    viewport()->update();
}

void HexEditor::replace(int aPos, int aLength, const QByteArray &aArray)
{
    MultipleUndoCommand *aCommand=new MultipleUndoCommand(this, MultipleUndoCommand::Replace, aPos, aLength, aArray);
    mUndoStack.push(aCommand);
    emit dataChanged();

    setCursorPosition(mCursorPosition);
    resetSelection();

    updateScrollBars();
    viewport()->update();
}

void HexEditor::setSelection(int aPos, int aCount)
{
    if (aCount<0)
    {
        aCount=0;
    }

    qint64 aPrevPos=mCursorPosition;

    mCursorPosition=aPos<<1;
    resetSelection();

    mCursorPosition+=(aCount+1)<<1;
    updateSelection();

    mCursorPosition=aPrevPos;
}

void HexEditor::cut()
{
    copy();

    if (mSelectionStart==mSelectionEnd)
    {
        remove(mSelectionStart, 1);
    }
    else
    {
        remove(mSelectionStart, mSelectionEnd-mSelectionStart);
    }

    setPosition(mSelectionStart);
    cursorMoved(false);
}

void HexEditor::copy()
{
    QString aToClipboard;

    if (mCursorAtTheLeft)
    {
        if (mSelectionStart==mSelectionEnd)
        {
            if (mSelectionStart<mData.size())
            {
                quint8 aChar=mData.at(mSelectionStart);
                aToClipboard=QString::number(aChar, 16).toUpper();

                if (aToClipboard.length()==1)
                {
                    aToClipboard.insert(0, "0");
                }
            }
        }
        else
        {
            for (int i=mSelectionStart; i<mSelectionEnd && i<mData.size(); ++i)
            {
                quint8 aChar=mData.at(i);
                QString aHexChar=QString::number(aChar, 16).toUpper();

                if (aHexChar.length()==1)
                {
                    aToClipboard.append("0");
                }

                aToClipboard.append(aHexChar);
            }
        }
    }
    else
    {
        if (mSelectionStart==mSelectionEnd)
        {
            if (mSelectionStart<mData.size())
            {
                char aChar=mData.at(mSelectionStart);
                aToClipboard=QString::fromLatin1(&aChar, 1);
            }
        }
        else
        {
            for (int i=mSelectionStart; i<mSelectionEnd && i<mData.size(); ++i)
            {
                char aChar=mData.at(i);

                if (aChar)
                {
                    aToClipboard.append(QString::fromLatin1(&aChar, 1));
                }
            }
        }
    }

    QApplication::clipboard()->setText(aToClipboard);
}

void HexEditor::paste()
{
    int aSelStart=mSelectionStart;

    remove(mSelectionStart, mSelectionEnd-mSelectionStart);

    QString aText=QApplication::clipboard()->text();
    QByteArray aArray;

    if (mCursorAtTheLeft)
    {
        aArray=QByteArray::fromHex(aText.toUtf8());
    }
    else
    {
        aArray=aText.toLatin1();
    }

    insert(aSelStart, aArray);
    setPosition(aSelStart+aArray.length());
    cursorMoved(false);
}

QString HexEditor::toString()
{
    return QString::fromLatin1(mData);
}

// ------------------------------------------------------------------

void HexEditor::updateScrollBars()
{
    mAddressWidth=0;
    int aDataSize=mData.size();
    int aCurSize=1;

    while (aCurSize<aDataSize)
    {
        ++mAddressWidth;
        aCurSize<<=4;
    }

    if (mAddressWidth==0)
    {
        mAddressWidth=1;
    }

    mLinesCount=floor(aDataSize/16.0f);

    if (aDataSize % 16!=0)
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
    int aCurPosition=mCursorPosition>>1;

    bool aSelectionChanged=(mSelectionStart!=aCurPosition) || (mSelectionEnd!=aCurPosition);

    mSelectionInit=aCurPosition;
    mSelectionStart=aCurPosition;
    mSelectionEnd=aCurPosition;

    if (aSelectionChanged)
    {
        viewport()->update();
        emit selectionChanged(mSelectionStart, mSelectionEnd);
    }
}

void HexEditor::updateSelection()
{
    int aCurPosition=mCursorPosition>>1;

    bool aSelectionChanged=false;

    if (aCurPosition<mSelectionInit)
    {
        if (mSelectionStart!=aCurPosition || mSelectionEnd!=mSelectionInit+(mOneMoreSelection ? 1 : 0))
        {
            mSelectionStart=aCurPosition;
            mSelectionEnd=mSelectionInit+(mOneMoreSelection ? 1 : 0);

            aSelectionChanged=true;

            mOneMoreSelection=false;
        }
    }
    else
    {
        if (mSelectionStart!=mSelectionInit || mSelectionEnd!=aCurPosition)
        {
            mSelectionStart=mSelectionInit;
            mSelectionEnd=aCurPosition;

            aSelectionChanged=true;
        }
    }

    if (aSelectionChanged)
    {
        viewport()->update();
        emit selectionChanged(mSelectionStart, mSelectionEnd);
    }
}

void HexEditor::cursorMoved(bool aKeepSelection)
{
    if (aKeepSelection)
    {
        updateSelection();
    }
    else
    {
        resetSelection();
    }

    resetCursorTimer();
    scrollToCursor();
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

    QColor aTextColor=aPalette.color(QPalette::Text);
    QColor aHighlightColor=aPalette.color(QPalette::Highlight);
    QColor aHighlightedTextColor=aPalette.color(QPalette::HighlightedText);
    QColor aAlternateBaseColor=aPalette.color(QPalette::AlternateBase);

    int aOffsetX=-horizontalScrollBar()->value();
    int aOffsetY=-verticalScrollBar()->value();
    int aViewWidth=viewport()->width();
    int aViewHeight=viewport()->height();

    painter.setFont(mFont);

    // Draw background for chars (Selection and cursor)
    {
        // Check for selection
        if (mSelectionStart!=mSelectionEnd)
        {
            // Draw selection
            int aStartRow=floor(mSelectionStart/16.0f);
            int aStartCol=mSelectionStart % 16;

            int aEndRow=floor((mSelectionEnd-1)/16.0f);
            int aEndCol=(mSelectionEnd-1) % 16;

            int aStartLeftX=(mAddressWidth+1+aStartCol*3)*mCharWidth+aOffsetX; // mAddressWidth + 1+aStartCol*3
            int aStartRightX=(mAddressWidth+50+aStartCol)*mCharWidth+aOffsetX; // mAddressWidth + 1+16*2+15+1 + 1+aStartCol
            int aStartY=aStartRow*(mCharHeight+LINE_INTERVAL)+aOffsetY;

            int aEndLeftX=(mAddressWidth+1+aEndCol*3)*mCharWidth+aOffsetX; // mAddressWidth + 1+aEndCol*3
            int aEndRightX=(mAddressWidth+50+aEndCol)*mCharWidth+aOffsetX; // mAddressWidth + 1+16*2+15+1 + 1+aEndCol
            int aEndY=aEndRow*(mCharHeight+LINE_INTERVAL)+aOffsetY;

            if (aStartRow==aEndRow)
            {
                painter.fillRect(aStartLeftX, aStartY, aEndLeftX-aStartLeftX+mCharWidth*2, mCharHeight, aHighlightColor);
                painter.fillRect(aStartRightX, aStartY, aEndRightX-aStartRightX+mCharWidth, mCharHeight, aHighlightColor);
            }
            else
            {
                QRect aHexRect(
                               mAddressWidth*mCharWidth+aOffsetX,
                               (aStartRow+1)*(mCharHeight+LINE_INTERVAL)-LINE_INTERVAL+aOffsetY,
                               49*mCharWidth,
                               (aEndRow-aStartRow-1)*(mCharHeight+LINE_INTERVAL)+LINE_INTERVAL
                              );

                QRect aTextRect(
                                (mAddressWidth+50)*mCharWidth+aOffsetX,
                                (aStartRow+1)*(mCharHeight+LINE_INTERVAL)-LINE_INTERVAL+aOffsetY,
                                16*mCharWidth,
                                (aEndRow-aStartRow-1)*(mCharHeight+LINE_INTERVAL)+LINE_INTERVAL
                               );

                if (aEndRow>aStartRow+1)
                {
                    painter.fillRect(aHexRect,  aHighlightColor);
                    painter.fillRect(aTextRect, aHighlightColor);
                }

                painter.fillRect(aStartLeftX, aStartY, aHexRect.right()-aStartLeftX+1, mCharHeight, aHighlightColor);
                painter.fillRect(aHexRect.left(), aEndY-LINE_INTERVAL, aEndLeftX-aHexRect.left()+mCharWidth*2, mCharHeight+LINE_INTERVAL, aHighlightColor);

                painter.fillRect(aStartRightX, aStartY, aTextRect.right()-aStartRightX+1, mCharHeight, aHighlightColor);
                painter.fillRect(aTextRect.left(), aEndY-LINE_INTERVAL, aEndRightX-aTextRect.left()+mCharWidth, mCharHeight+LINE_INTERVAL, aHighlightColor);
            }
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
                    if (mMode==INSERT)
                    {
                        painter.fillRect(aCursorX, aCursorY+mCharHeight, mCharWidth, LINE_INTERVAL, aHighlightColor);
                    }
                    else
                    {
                        painter.fillRect(aCursorX, aCursorY, mCharWidth, mCharHeight, aHighlightColor);
                    }
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
                    if (mMode==INSERT)
                    {
                        painter.fillRect(aCursorX, aCursorY+mCharHeight, mCharWidth, LINE_INTERVAL, aHighlightColor);
                    }
                    else
                    {
                        painter.fillRect(aCursorX, aCursorY, mCharWidth, mCharHeight, aHighlightColor);
                    }
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
                if (i>=mSelectionStart && i<mSelectionEnd)
                {
                    painter.setPen(aHighlightedTextColor);
                }
                else
                {
                    painter.setPen(aTextColor);
                }

                if (aHexChar.length()<2)
                {
                    aHexChar.insert(0, "0");
                }

                if (i==mSelectionStart && i==mSelectionEnd && mMode==OVERWRITE)
                {
                    if ((mCursorAtTheLeft && !mCursorVisible) || (mCursorPosition & 1))
                    {
                        painter.setPen(aTextColor);
                    }
                    else
                    {
                        painter.setPen(aHighlightedTextColor);
                    }
                }

                painter.drawText(aCharX,            aCharY, mCharWidth, mCharHeight, Qt::AlignCenter, aHexChar.at(0));

                if (i==mSelectionStart && i==mSelectionEnd && mMode==OVERWRITE)
                {
                    if ((!mCursorAtTheLeft || mCursorVisible) && (mCursorPosition & 1))
                    {
                        painter.setPen(aHighlightedTextColor);
                    }
                    else
                    {
                        painter.setPen(aTextColor);
                    }
                }

                painter.drawText(aCharX+mCharWidth, aCharY, mCharWidth, mCharHeight, Qt::AlignCenter, aHexChar.at(1));
            }

            // -----------------------------------------------------------------------------------------------------------------

            aCharX=(mAddressWidth+50+aCurCol)*mCharWidth+aOffsetX;

            if (aCharX>=(mAddressWidth-2)*mCharWidth && aCharX<=aViewWidth)
            {
                if (
                    (
                     i==mSelectionStart
                     &&
                     i==mSelectionEnd
                     &&
                     mMode==OVERWRITE
                     &&
                     (
                      mCursorAtTheLeft
                      ||
                      mCursorVisible
                     )
                    )
                    ||
                    (
                     i>=mSelectionStart
                     &&
                     i<mSelectionEnd
                    )
                   )
                {
                    painter.setPen(aHighlightedTextColor);
                }
                else
                {
                    painter.setPen(aTextColor);
                }

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

    // Line delimeters
    {
        painter.setPen(QColor(0, 0, 0));

        int aLineX;

        aLineX=mAddressWidth*mCharWidth;
        painter.drawLine(aLineX, 0, aLineX, aViewHeight);

        aLineX=(mAddressWidth+49)*mCharWidth+aOffsetX; // mAddressWidth + 1+16*2+15+1
        painter.drawLine(aLineX, 0, aLineX, aViewHeight);
    }

    // Address field at the left side
    {
        painter.setPen(aTextColor);
        painter.fillRect(0, 0, mAddressWidth*mCharWidth, aViewHeight, aAlternateBaseColor);

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
                    aHexChar=aHexAddress.at(aHexAddress.length()-mAddressWidth+j+1);
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
    if (event->matches(QKeySequence::MoveToPreviousChar))
    {
        if (mCursorAtTheLeft)
        {
            setCursorPosition(mCursorPosition-1);
        }
        else
        {
            setPosition(position()-1);
        }

        cursorMoved(false);
    }
    else
    if (event->matches(QKeySequence::MoveToNextChar))
    {
        if (mCursorAtTheLeft)
        {
            setCursorPosition(mCursorPosition+1);
        }
        else
        {
            setPosition(position()+1);
        }

        cursorMoved(false);
    }
    else
    if (event->matches(QKeySequence::MoveToPreviousLine))
    {
        setCursorPosition(mCursorPosition-32);
        cursorMoved(false);
    }
    else
    if (event->matches(QKeySequence::MoveToNextLine))
    {
        setCursorPosition(mCursorPosition+32);
        cursorMoved(false);
    }
    else
    if (event->matches(QKeySequence::MoveToStartOfLine))
    {
        setCursorPosition(mCursorPosition-(mCursorPosition % 32));
        cursorMoved(false);
    }
    else
    if (event->matches(QKeySequence::MoveToEndOfLine))
    {
        setCursorPosition(mCursorPosition | 31);
        cursorMoved(false);
    }
    else
    if (event->matches(QKeySequence::MoveToPreviousPage))
    {
        setCursorPosition(mCursorPosition-viewport()->height()/(mCharHeight+LINE_INTERVAL)*32);
        cursorMoved(false);
    }
    else
    if (event->matches(QKeySequence::MoveToNextPage))
    {
        setCursorPosition(mCursorPosition+viewport()->height()/(mCharHeight+LINE_INTERVAL)*32);
        cursorMoved(false);
    }
    else
    if (event->matches(QKeySequence::MoveToStartOfDocument))
    {
        setCursorPosition(0);
        cursorMoved(false);
    }
    else
    if (event->matches(QKeySequence::MoveToEndOfDocument))
    {
        setCursorPosition(mData.size()*2);
        cursorMoved(false);
    }
    // =======================================================================================
    //                                    Selecting
    // =======================================================================================
    else
    if (event->matches(QKeySequence::SelectAll))
    {
        mSelectionInit=0;
        setCursorPosition(mData.size()*2);
        cursorMoved(true);
    }
    else
    if (event->matches(QKeySequence::SelectPreviousChar))
    {
        setPosition(position()-1);
        cursorMoved(true);
    }
    else
    if (event->matches(QKeySequence::SelectNextChar))
    {
        setPosition(position()+1);
        cursorMoved(true);
    }
    else
    if (event->matches(QKeySequence::SelectPreviousLine))
    {
        setCursorPosition(mCursorPosition-32);
        cursorMoved(true);
    }
    else
    if (event->matches(QKeySequence::SelectNextLine))
    {
        setCursorPosition(mCursorPosition+32);
        cursorMoved(true);
    }
    else
    if (event->matches(QKeySequence::SelectStartOfLine))
    {
        setCursorPosition(mCursorPosition-(mCursorPosition % 32));
        cursorMoved(true);
    }
    else
    if (event->matches(QKeySequence::SelectEndOfLine))
    {
        setCursorPosition(mCursorPosition | 31);
        cursorMoved(true);
    }
    else
    if (event->matches(QKeySequence::SelectPreviousPage))
    {
        setCursorPosition(mCursorPosition-viewport()->height()/(mCharHeight+LINE_INTERVAL)*32);
        cursorMoved(true);
    }
    else
    if (event->matches(QKeySequence::SelectNextPage))
    {
        setCursorPosition(mCursorPosition+viewport()->height()/(mCharHeight+LINE_INTERVAL)*32);
        cursorMoved(true);
    }
    else
    if (event->matches(QKeySequence::SelectStartOfDocument))
    {
        setCursorPosition(0);
        cursorMoved(true);
    }
    else
    if (event->matches(QKeySequence::SelectEndOfDocument))
    {
        setCursorPosition(mData.size()*2);
        cursorMoved(true);
    }
    // =======================================================================================
    //                                      Others
    // =======================================================================================
    else
    if (event->matches(QKeySequence::Copy))
    {
        copy();
    }
    else
    if ((event->key() == Qt::Key_Tab) && (event->modifiers() == Qt::NoModifier))
    {
        mCursorAtTheLeft=!mCursorAtTheLeft;
        cursorMoved(true);
    }
    else
    if ((event->key() == Qt::Key_Insert) && (event->modifiers() == Qt::NoModifier))
    {
        if (mMode==INSERT)
        {
            setMode(OVERWRITE);
        }
        else
        {
            setMode(INSERT);
        }
    }
    // =======================================================================================
    //                                     Editing
    // =======================================================================================
    else
    if (!mReadOnly)
    {
        if (event->matches(QKeySequence::Undo))
        {
            undo();
        }
        else
        if (event->matches(QKeySequence::Redo))
        {
            redo();
        }
        else
        if (event->matches(QKeySequence::Delete))
        {
            int aSelStart=mSelectionStart;

            if (mSelectionStart==mSelectionEnd)
            {
                if (mSelectionStart<mData.size())
                {
                    if (mMode==INSERT)
                    {
                        remove(mSelectionStart, 1);
                    }
                    else
                    {
                        replace(mSelectionStart, 0);
                        ++aSelStart;
                    }
                }
            }
            else
            {
                remove(mSelectionStart, mSelectionEnd-mSelectionStart);
            }

            setPosition(aSelStart);
            cursorMoved(false);
        }
        else
        if ((event->key() == Qt::Key_Backspace) && (event->modifiers() == Qt::NoModifier))
        {
            int aSelStart=mSelectionStart;

            if (mSelectionStart==mSelectionEnd)
            {
                if (mSelectionStart>0)
                {
                    if (mMode==INSERT)
                    {
                        remove(mSelectionStart-1, 1);
                    }
                    else
                    {
                        replace(mSelectionStart-1, 0);
                    }

                    --aSelStart;
                }
            }
            else
            {
                remove(mSelectionStart, mSelectionEnd-mSelectionStart);
            }

            setPosition(aSelStart);
            cursorMoved(false);
        }
        else
        if (event->matches(QKeySequence::Cut))
        {
            cut();
        }
        else
        if (event->matches(QKeySequence::Paste))
        {
            paste();
        }
        else
        {
            QString aKeyText=event->text();

            if (aKeyText.length()>0)
            {
                char aKey=aKeyText.at(0).toLatin1();

                if (mCursorAtTheLeft)
                {
                    if (
                        (
                         aKey>='0'
                         &&
                         aKey<='9'
                        )
                        ||
                        (
                         aKey>='a'
                         &&
                         aKey<='f'
                        )
                        ||
                        (
                         aKey>='A'
                         &&
                         aKey<='F'
                        )
                       )
                    {
                        if (mSelectionStart!=mSelectionEnd)
                        {
                            remove(mSelectionStart, mSelectionEnd-mSelectionStart);
                            setPosition(mSelectionStart);
                            cursorMoved(false);
                        }

                        if (
                            mSelectionStart==mData.size()
                            ||
                            (
                             mMode==INSERT
                             &&
                             (mCursorPosition & 1)==0
                            )
                           )
                        {
                            insert(mSelectionStart, 0);
                        }

                        if (mSelectionStart<mData.size())
                        {
                            QByteArray aHexChar=QString::number((quint8)mData.at(mSelectionStart), 16).toLatin1();

                            if (aHexChar.length()<2)
                            {
                                aHexChar.insert(0, 48); // 48 == "0"
                            }

                            if (mCursorPosition & 1)
                            {
                                aHexChar[1]=aKey;
                            }
                            else
                            {
                                aHexChar[0]=aKey;
                            }

                            replace(mSelectionStart, QByteArray::fromHex(aHexChar).at(0));

                            setCursorPosition(mCursorPosition+1);
                            cursorMoved(false);
                        }
                    }
                }
                else
                {
                    if (mSelectionStart!=mSelectionEnd)
                    {
                        remove(mSelectionStart, mSelectionEnd-mSelectionStart);
                        setPosition(mSelectionStart);
                        cursorMoved(false);
                    }

                    if (
                        mSelectionStart==mData.size()
                        ||
                        mMode==INSERT
                       )
                    {
                        insert(mSelectionStart, 0);
                    }

                    if (mSelectionStart<mData.size())
                    {
                        replace(mSelectionStart, aKey);

                        setPosition(mSelectionStart+1);
                        cursorMoved(false);
                    }
                }
            }
        }
    }
}

void HexEditor::mousePressEvent(QMouseEvent *event)
{
    mLeftButtonPressed=(event->button()==Qt::LeftButton);

    if (mLeftButtonPressed)
    {
        bool aShift=event->modifiers() & Qt::ShiftModifier;
        int aPosition=charAt(event->pos(), &mCursorAtTheLeft);

        if (aShift)
        {
            if ((aPosition>>1)>=mSelectionInit)
            {
                aPosition+=2;
            }
            else
            {
                mOneMoreSelection=true;
            }
        }

        setCursorPosition(aPosition);
        cursorMoved(aShift);
    }

    QAbstractScrollArea::mousePressEvent(event);
}

void HexEditor::mouseMoveEvent(QMouseEvent *event)
{
    if (mLeftButtonPressed)
    {
        int aPosition=charAt(event->pos(), &mCursorAtTheLeft);

        if ((aPosition>>1)>=mSelectionInit)
        {
            aPosition+=2;
        }
        else
        {
            mOneMoreSelection=true;
        }

        setCursorPosition(aPosition);
        cursorMoved(true);
    }

    QAbstractScrollArea::mouseMoveEvent(event);
}

void HexEditor::mouseReleaseEvent(QMouseEvent *event)
{
    mLeftButtonPressed=false;

    QAbstractScrollArea::mouseReleaseEvent(event);
}

void HexEditor::wheelEvent(QWheelEvent *event)
{
    if (event->delta()>=0)
    {
        verticalScrollBar()->setValue(verticalScrollBar()->value()-10*(mCharHeight+LINE_INTERVAL));
    }
    else
    {
        verticalScrollBar()->setValue(verticalScrollBar()->value()+10*(mCharHeight+LINE_INTERVAL));
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
        setCursorPosition(mCursorPosition);
        mUndoStack.clear();

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

        resetCursorTimer();

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
    return mCursorPosition>>1;
}

void HexEditor::setPosition(int aPosition)
{
    if ((mCursorPosition>>1)!=aPosition)
    {
        setCursorPosition(aPosition<<1);
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

int HexEditor::selectionStart()
{
    return mSelectionStart;
}

int HexEditor::selectionEnd()
{
    return mSelectionEnd;
}

bool HexEditor::isCursorAtTheLeft()
{
    return mCursorAtTheLeft;
}

// *********************************************************************************
//                                 SingleUndoCommand
// *********************************************************************************

SingleUndoCommand::SingleUndoCommand(HexEditor *aEditor, Type aType, int aPos, char aNewChar, QUndoCommand *parent) :
    QUndoCommand(parent)
{
    mEditor=aEditor;
    mType=aType;
    mPos=aPos;
    mNewChar=aNewChar;
}

void SingleUndoCommand::undo()
{
    switch (mType)
    {
        case Insert:
        {
            mEditor->mData.remove(mPos, 1);
        }
        break;
        case Replace:
        {
            QByteArray aArray;
            aArray.append(mOldChar);
            mEditor->mData.replace(mPos, 1, aArray);
        }
        break;
        case Remove:
        {
            mEditor->mData.insert(mPos, mOldChar);
        }
        break;
    }

    mEditor->setCursorPosition(mPrevPosition);
}

void SingleUndoCommand::redo()
{
    mPrevPosition=mEditor->mCursorPosition;

    switch (mType)
    {
        case Insert:
        {
            mEditor->mData.insert(mPos, mNewChar);
        }
        break;
        case Replace:
        {
            mOldChar=mEditor->mData.at(mPos);
            QByteArray aArray;
            aArray.append(mNewChar);
            mEditor->mData.replace(mPos, 1, aArray);
        }
        break;
        case Remove:
        {
            mOldChar=mEditor->mData.at(mPos);
            mEditor->mData.remove(mPos, 1);
        }
        break;
    }
}

bool SingleUndoCommand::mergeWith(const QUndoCommand *command)
{
    SingleUndoCommand *aAnotherCommand=(SingleUndoCommand *)command;

    if (
        mType!=Remove
        &&
        aAnotherCommand->mType==Replace
        &&
        aAnotherCommand->mPos==mPos
       )
    {
        mNewChar=aAnotherCommand->mNewChar;
        return true;
    }

    return false;
}

int SingleUndoCommand::id() const
{
    return 1;
}

// *********************************************************************************
//                                MultipleUndoCommand
// *********************************************************************************

MultipleUndoCommand::MultipleUndoCommand(HexEditor *aEditor, Type aType, int aPos, int aLength, QByteArray aNewArray, QUndoCommand *parent) :
    QUndoCommand(parent)
{
    mEditor=aEditor;
    mType=aType;
    mPos=aPos;
    mLength=aLength;
    mNewArray=aNewArray;
}

void MultipleUndoCommand::undo()
{
    switch (mType)
    {
        case Insert:
        {
            mEditor->mData.remove(mPos, mNewArray.length());
        }
        break;
        case Replace:
        {
            mEditor->mData.replace(mPos, mNewArray.length(), mOldArray);
        }
        break;
        case Remove:
        {
            mEditor->mData.insert(mPos, mOldArray);
        }
        break;
    }

    mEditor->setCursorPosition(mPrevPosition);
}

void MultipleUndoCommand::redo()
{
    mPrevPosition=mEditor->mCursorPosition;

    switch (mType)
    {
        case Insert:
        {
            mEditor->mData.insert(mPos, mNewArray);
        }
        break;
        case Replace:
        {
            mOldArray=mEditor->mData.mid(mPos, mLength);
            mEditor->mData.replace(mPos, mLength, mNewArray);
        }
        break;
        case Remove:
        {
            mOldArray=mEditor->mData.mid(mPos, mLength);
            mEditor->mData.remove(mPos, mLength);
        }
        break;
    }
}
