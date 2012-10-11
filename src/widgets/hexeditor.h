#ifndef HEXEDITOR_H
#define HEXEDITOR_H

#include <QAbstractScrollArea>

#include <QTimer>

class HexEditor : public QAbstractScrollArea
{
    Q_OBJECT

public:
    Q_PROPERTY(QByteArray   Data                     READ data                     WRITE setData)
    Q_PROPERTY(Mode         Mode                     READ mode                     WRITE setMode)
    Q_PROPERTY(bool         ReadOnly                 READ isReadOnly               WRITE setReadOnly)
    Q_PROPERTY(qint64       Position                 READ position                 WRITE setPosition)
    Q_PROPERTY(int          CursorPosition           READ cursorPosition           WRITE setCursorPosition)
    Q_PROPERTY(QFont        Font                     READ font                     WRITE setFont)

    Q_PROPERTY(int      charWidth      READ charWidth)
    Q_PROPERTY(int      charHeight     READ charHeight)
    Q_PROPERTY(quint8   addressWidth   READ addressWidth)
    Q_PROPERTY(int      linesCount     READ linesCount)



    Q_ENUMS(Mode)

    enum Mode
    {
        INSERT,
        OVERWRITE
    };



    HexEditor(QWidget *parent = 0);



    int indexOf(const QByteArray &aArray, int aFrom=0) const;
    int lastIndexOf(const QByteArray &aArray, int aFrom=0) const;
    void insert(int aIndex, const QByteArray &aArray);
    void insert(int aIndex, char aChar);
    void remove(int aPos, int aLength=1);
    void replace(int aPos, int aLength, const QByteArray &aArray);
    QString toString();

    // ------------------------------------------------------------------

    QByteArray data() const;
    void setData(QByteArray const &aData);

    Mode mode() const;
    void setMode(const Mode &aMode);

    bool isReadOnly() const;
    void setReadOnly(const bool &aReadOnly);

    int position() const;
    void setPosition(int aPosition);

    qint64 cursorPosition() const;
    void setCursorPosition(qint64 aCursorPos);

    QFont font() const;
    void setFont(const QFont &aFont);

    int    charWidth();
    int    charHeight();
    quint8 addressWidth();
    int    linesCount();

protected:
    QByteArray mData;
    Mode       mMode;
    bool       mReadOnly;
    qint64     mCursorPosition;
    QFont      mFont;

    QString mAsciiChars;
    int     mCharWidth;
    int     mCharHeight;
    quint8  mAddressWidth;
    int     mLinesCount;

    QTimer  mCursorTimer;
    bool    mCursorVisible;

    void updateScrollBars();
    void resizeEvent(QResizeEvent *event);
    void paintEvent(QPaintEvent *event);

public slots:
    void undo();
    void redo();

protected slots:
    void cursorBlicking();

signals:
    void dataChanged();
    void sizeChanged(int aSize);
    void modeChanged(Mode aMode);
    void positionChanged(int aPosition);
};

#endif // HEXEDITOR_H
