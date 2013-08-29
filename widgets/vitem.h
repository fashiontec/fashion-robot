#ifndef VITEM_H
#define VITEM_H

#include <QGraphicsPathItem>
#include <QObject>

/**
 * @brief VItem клас, що реалізує деталь на сцені.
 */
class VItem : public QObject, public QGraphicsPathItem{
    Q_OBJECT
public:
    /**
     * @brief VItem конструктор за замовчуванням
     *Конструктор генерує пусту деталь з номером в списку, що дорівнює 0.
     */
             VItem ();
    /**
    * @brief VItem конструктор
    * @param numInList номер в списку деталей, що передається у вікно де
    *укладаються деталі.
    * @param parent батьківський об'єкт на сцені для даного. За замовчуванням немає.
    */
             VItem (int numInList, QGraphicsItem * parent = 0);
    /**
    * @brief VItem конструктор
    * @param path зображення що буде показуватися на сцені - об’єкт класу QPainterPath.
    * @param numInList номер в списку деталей, що передається у вікно де
    *укладаються деталі.
    * @param parent батьківський об'єкт на сцені для даного. За замовчуванням немає.
    */
             VItem ( const QPainterPath & path, int numInList, QGraphicsItem * parent = 0 );
    /**
    * @brief Rotate повертає об'єкт на кут в градусах
    * @param angle кут в градусах на який повертається деталь.
    */
    void     Rotate ( qreal angle );
public slots:
    /**
     * @brief LengthChanged слот який обробляє сигнал зміни довжини листа.
     */
    void     LengthChanged();
    /**
     * @brief SetIndexInList встановлює номер деталі в списку деталей.
     * @param index номер в списку.
     */
    void     SetIndexInList( qint32 index );
protected:
    /**
     * @brief itemChange модифікація стандартного методу itemChange. Виконується перехоплення зміни
     *положення і зміни батька.
     * @param change
     * @param value
     * @return
     */
    QVariant itemChange ( GraphicsItemChange change, const QVariant &value );
    /**
     * @brief checkItemChange перевіряє вихід деталі за рамки листа і факт колізії. Посилає відповідні
     *сигнали.
     */
    void     checkItemChange ();
private:
    /**
     * @brief numInOutList для зберігання інформації про колізії від кожної деталі необхідно знати її
     *номер.
     */
    qint32      numInOutList;
signals:
    /**
     * @brief itemOut сигнал виходу за межі листа. Посилається у будь-якому випадку.
     * @param numInOutList номер деталі яка вийшла за межі листа або тепер знаходиться в межах листа.
     * @param flag був вихід чи ні.
     */
    void     itemOut ( int numInOutList, bool flag );
    /**
     * @brief itemColliding сигнал колізії деталі з іншими. Посилається як для додавання деталі до
     *списку тих що перетинаються, так і для виключення його з такого.
     * @param list список усіх деталей які приймають участь в колізії включаючи самого себе.
     * @param number 1 - перетин є, 0 - перетину немає.
     */
    void     itemColliding ( QList<QGraphicsItem *> list, int number );
};
#endif // VITEM_H
