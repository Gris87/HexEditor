#ifndef HEXEDITOR_H
#define HEXEDITOR_H

#include <QScrollArea>

class HexEditor : public QScrollArea
{
    Q_OBJECT

public:
    Q_PROPERTY(QByteArray   Data                     READ data                     WRITE setData)
    Q_PROPERTY(Mode         Mode                     READ mode                     WRITE setMode)
    Q_PROPERTY(bool         ReadOnly                 READ isReadOnly               WRITE setReadOnly)
    Q_PROPERTY(QFont        Font                     READ font                     WRITE setFont)
    Q_PROPERTY(int          AddressOffset            READ addressOffset            WRITE setAddressOffset)
    Q_PROPERTY(int          CursorPosition           READ cursorPosition           WRITE setCursorPosition)
    Q_PROPERTY(QColor       AddressBackgroundColor   READ addressBackgroundColor   WRITE setAddressBackgroundColor)
    Q_PROPERTY(QColor       HighlightingColor        READ highlightingColor        WRITE setHighlightingColor)
    Q_PROPERTY(QColor       SelectionColor           READ selectionColor           WRITE setSelectionColor)



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



    QByteArray data() const;
    void setData(QByteArray const &aData);

    Mode mode() const;
    void setMode(const Mode &aMode);

    bool isReadOnly() const;
    void setReadOnly(const bool &aReadOnly);

    QFont font() const;
    void setFont(const QFont &aFont);

    int addressOffset() const;
    void setAddressOffset(int aOffset);

    int cursorPosition() const;
    void setCursorPosition(int cusorPos);

    QColor addressBackgroundColor() const;
    void setAddressBackgroundColor(QColor const &color);

    QColor highlightingColor() const;
    void setHighlightingColor(QColor const &color);

    QColor selectionColor() const;
    void setSelectionColor(QColor const &color);

protected:

public slots:
    void undo();
    void redo();
    void setAddressVisible(const bool &aAddressVisible);
    void setTextVisible(const bool &aTextVisible);
    void setHighlightingEnabled(const bool &aEnable);

signals:
    void positionChanged(int aPosition);
    void sizeChanged(int aSize);
    void dataChanged();
    void modeChanged(bool aMode);
};

#endif // HEXEDITOR_H
