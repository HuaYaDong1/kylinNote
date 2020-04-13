#include "notewidgetdelegate.h"
#include "noteview.h"
#include <QPainter>
#include <QEvent>
#include <QDebug>
#include <QApplication>
#include <QFontDatabase>
#include <QtMath>
#include "notemodel.h"


NoteWidgetDelegate::NoteWidgetDelegate(QObject *parent)
    : QStyledItemDelegate(parent),
      m_titleFont(QStringLiteral(""), 14, 30),        //标题字体
      m_titleSelectedFont(QStringLiteral(""), 14),    //
      m_dateFont(QStringLiteral(""), 10),             //日期字体
      m_titleColor(255, 255, 255),                          //标题颜色
      m_dateColor(255, 255, 255),                           //日期颜色
      m_ActiveColor(218, 233, 239),
      m_notActiveColor(175, 212, 228),                      //默认选中背景色
      m_hoverColor(80, 80, 80),                          //悬停颜色
      m_selectColor(40,40,40),                                      //选中颜色
      m_applicationInactiveColor(207, 207, 207),            //应用程序可见，但未选择显示在前面时背景色
      m_separatorColor(221, 221, 221),
      m_defaultColor(0, 0, 0),
      m_rowHeight(82),                                      //item宽度
      m_maxFrame(200),
      m_rowRightOffset(0),
      m_state(Normal),
      m_isActive(false)
{
    //QTimeLine拥有一个时间轴来控制时间动画和事件
    //参数1为毫秒级 总运行时间0.3秒
    m_timeLine = new QTimeLine(300, this);

    //对QTImeLine的时间轴设置动画帧数,共200张静态画面图组成
    //Qt默认40ms一帧，也就是一秒可以最多走25帧
    m_timeLine->setFrameRange(0,m_maxFrame);
    //时间轴走的时候，会不断发出frameChanged信号，setUpdateInterval控制多少时间发一次
    m_timeLine->setUpdateInterval(10);
    //设置你的时间变换曲线，即明确你的时间是先快后慢，还是先慢后快，或者线性
    m_timeLine->setCurveShape(QTimeLine::EaseInCurve);

    connect( m_timeLine, &QTimeLine::frameChanged, [this](){
        emit sizeHintChanged(m_animatedIndex);
    });

    connect(m_timeLine, &QTimeLine::finished, [this](){
        m_animatedIndex = QModelIndex();
        m_state = Normal;
    });
}

void NoteWidgetDelegate::setState(States NewState, QModelIndex index)
{
    m_animatedIndex = index;

    auto startAnimation = [this](QTimeLine::Direction diretion, int duration){
        m_timeLine->setDirection(diretion);
        m_timeLine->setDuration(duration);
        m_timeLine->start();
    };

    switch ( NewState ){
    case Insert:
        startAnimation(QTimeLine::Forward, m_maxFrame);
        break;
    case Remove:
        startAnimation(QTimeLine::Backward, m_maxFrame);
        break;
    case MoveOut:
        startAnimation(QTimeLine::Backward, m_maxFrame);
        break;
    case MoveIn:
        startAnimation(QTimeLine::Backward, m_maxFrame);
        break;
    case Normal:
        m_animatedIndex = QModelIndex();
        break;
    }

    m_state = NewState;
}

void NoteWidgetDelegate::setAnimationDuration(const int duration)
{
    m_timeLine->setDuration(duration);
}

void NoteWidgetDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    //qDebug() << "当前文件 :" << __FILE__ << "当前函数 :" << __FUNCTION__ << "当前行号 :" << __LINE__;
    QStyleOptionViewItem opt = option;
    opt.rect.setWidth(option.rect.width() - m_rowRightOffset);      //678

    int currentFrame = m_timeLine->currentFrame();
    //qDebug() << "currentFrame" << currentFrame;  //0
    double rate = (currentFrame/(m_maxFrame * 1.0));
    //qDebug() << "rate" << rate;  //0
    double height = m_rowHeight * rate;  //0
    //qDebug() << "height" << height;
    //qDebug() << m_state;
    //默认Normal
    switch(m_state){
    case Insert:
    case Remove:
    case MoveOut:
        if(index == m_animatedIndex){
            //qDebug() << "当前文件 :" << __FILE__ << "当前函数 :" << __FUNCTION__ << "当前行号 :" << __LINE__;
            opt.rect.setHeight(int(height));
            opt.backgroundBrush.setColor(m_notActiveColor);
        }
        break;
    case MoveIn:
        qDebug() << "当前文件 :" << __FILE__ << "当前函数 :" << __FUNCTION__ << "当前行号 :" << __LINE__;
        if(index == m_animatedIndex){
            opt.rect.setY(int(height));
        }
        break;
    case Normal:
        break;
    }

    //第一层
    painter->setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    painter->setBrush(QBrush(QColor(0, 170, 255)));   //0, 170, 255
    painter->setPen(Qt::transparent);
    opt.rect.setWidth(678);
    opt.rect.setHeight(opt.rect.height() - 10);
    {
        QPainterPath painterPath;
        painterPath.addRoundedRect(opt.rect, 7, 7);
        painter->drawPath(painterPath);
    }

    //第二层
    painter->setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    painter->setBrush(QBrush(QColor(43, 49, 56)));  //43, 49, 56
    painter->setPen(Qt::transparent);

    opt.rect.setWidth(678);//5
    opt.rect.setHeight(opt.rect.height() - 0);
    opt.rect.setLeft(opt.rect.left() + 5);
    {
        QPainterPath painterPath;
        painterPath.addRoundedRect(opt.rect, 0, 0);
        painter->drawPath(painterPath);
    }

    paintBackground2(painter, opt, index);
    paintLabels(painter, option, index);

}

QSize NoteWidgetDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QSize result = QStyledItemDelegate::sizeHint(option, index);
    //qDebug() << "159" << result;  //QSize(0, 23)
    if(index == m_animatedIndex){
        //qDebug() << "当前文件 :" << __FILE__ << "当前函数 :" << __FUNCTION__ << "当前行号 :" << __LINE__;
        //qDebug() << index;
        if(m_state == MoveIn){
            result.setHeight(m_rowHeight);
            //qDebug() << m_rowHeight;
        }else{
            double rate = m_timeLine->currentFrame()/(m_maxFrame * 1.0);
            double height = m_rowHeight * rate;
            //qDebug() << height;
            result.setHeight(int(height));
        }
    }else{
        //qDebug() << "当前文件 :" << __FILE__ << "当前函数 :" << __FUNCTION__ << "当前行号 :" << __LINE__;
        result.setHeight(m_rowHeight);
    }

    return result;
}

QTimeLine::State NoteWidgetDelegate::animationState()
{
    return m_timeLine->state();
}

void NoteWidgetDelegate::paintBackground2(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem opt = option;
    opt.rect.setWidth(option.rect.width() - m_rowRightOffset);
    if((option.state & QStyle::State_Selected) == QStyle::State_Selected)
    {
        //应用程序是可见的，并被选择在前面。
        if(qApp->applicationState() == Qt::ApplicationActive){      //返回应用程序的当前状态。
            if(m_isActive){//用指定的画笔填充给定的矩形。
                painter->setRenderHint(QPainter::Antialiasing);  // 反锯齿;
                painter->setBrush(QBrush(m_selectColor));
                painter->setPen(Qt::transparent);
                QPainterPath painterPath;
                painterPath.addRoundedRect(opt.rect, 0, 0);
                painter->drawPath(painterPath);
                //painter->fillRect(option.rect, QBrush(m_ActiveColor));//浅蓝
            }else{
                painter->setRenderHint(QPainter::Antialiasing);  // 反锯齿;
                painter->setBrush(QBrush(m_selectColor));
                painter->setPen(Qt::transparent);
                QPainterPath painterPath;
                painterPath.addRoundedRect(opt.rect, 0, 0);
                painter->drawPath(painterPath);
                //painter->fillRect(option.rect, QBrush(m_notActiveColor));//深蓝
            }
            //应用程序可见，但未选择显示在前面
        }else if(qApp->applicationState() == Qt::ApplicationInactive){
            painter->setRenderHint(QPainter::Antialiasing);  // 反锯齿;
            painter->setBrush(QBrush(m_defaultColor));
            painter->setPen(Qt::transparent);
            QPainterPath painterPath;
            painterPath.addRoundedRect(opt.rect, 0, 0);
            painter->drawPath(painterPath);
            //painter->fillRect(option.rect, QBrush(m_defaultColor));//
        }
    }
    //鼠标悬停时颜色
    //用于指示小部件是否在鼠标下。
    else if((option.state & QStyle::State_MouseOver) == QStyle::State_MouseOver){
        //qDebug() << "当前文件 :" << __FILE__ << "当前函数 :" << __FUNCTION__ << "当前行号 :" << __LINE__;
        painter->setRenderHint(QPainter::Antialiasing);  // 反锯齿;
        painter->setBrush(QBrush(m_hoverColor));
        painter->setPen(Qt::transparent);
        QPainterPath painterPath;
        painterPath.addRoundedRect(opt.rect, 0, 0);
        painter->drawPath(painterPath);
        //painter->fillRect(option.rect, QBrush(m_hoverColor));//灰色
    //当前item未选中 未悬停时颜色
    }else if((index.row() !=  m_currentSelectedIndex.row() - 1)
             && (index.row() !=  m_hoveredIndex.row() - 1)){
        //qDebug() << "当前文件 :" << __FILE__ << "当前函数 :" << __FUNCTION__ << "当前行号 :" << __LINE__;
        painter->setRenderHint(QPainter::Antialiasing);  // 反锯齿;
        painter->fillRect(option.rect, QBrush(m_defaultColor));//黑色
    }

}

void NoteWidgetDelegate::paintBackground(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    //qDebug() << "m_isActive" << m_isActive;  //fulse
    //option.state 此变量保存绘制控件时使用的样式标志 默认值是QStyle：：State_None
    if((option.state & QStyle::State_Selected) == QStyle::State_Selected){
        //应用程序是可见的，并被选择在前面。
        if(qApp->applicationState() == Qt::ApplicationActive){      //返回应用程序的当前状态。
            if(m_isActive){//用指定的画笔填充给定的矩形。
                qDebug() << "当前文件 :" << __FILE__ << "当前函数 :" << __FUNCTION__ << "当前行号 :" << __LINE__;
                painter->fillRect(option.rect, QBrush(m_ActiveColor));//浅蓝
            }else{
                qDebug() << "当前文件 :" << __FILE__ << "当前函数 :" << __FUNCTION__ << "当前行号 :" << __LINE__;
                painter->fillRect(option.rect, QBrush(m_notActiveColor));//深蓝
            }
        //应用程序可见，但未选择显示在前面
        }else if(qApp->applicationState() == Qt::ApplicationInactive){
            qDebug() << "当前文件 :" << __FILE__ << "当前函数 :" << __FUNCTION__ << "当前行号 :" << __LINE__;
            painter->fillRect(option.rect, QBrush(m_applicationInactiveColor));//灰色
        }
    //鼠标悬停时颜色
    //用于指示小部件是否在鼠标下。
    }else if((option.state & QStyle::State_MouseOver) == QStyle::State_MouseOver){
        qDebug() << "当前文件 :" << __FILE__ << "当前函数 :" << __FUNCTION__ << "当前行号 :" << __LINE__;
        painter->fillRect(option.rect, QBrush(m_hoverColor));//灰色
    //当前item未选中 未悬停时颜色
    }else if((index.row() !=  m_currentSelectedIndex.row() - 1)
             && (index.row() !=  m_hoveredIndex.row() - 1)){
        qDebug() << "当前文件 :" << __FILE__ << "当前函数 :" << __FUNCTION__ << "当前行号 :" << __LINE__;
        painter->fillRect(option.rect, QBrush(m_defaultColor));//黑色
        //paintSeparator(painter, option, index);  //绘制分割线
    }
}

void NoteWidgetDelegate::paintLabels(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const int leftOffsetX = 20;
    const int topOffsetY = 18;   // 标题上方的空格
    const int spaceY = 5;       // 标题和日期之间的空格

    QString title{index.data(NoteModel::NoteFullTitle).toString()};
    QFont titleFont = (option.state & QStyle::State_Selected) == QStyle::State_Selected ? m_titleSelectedFont : m_titleFont;
    QFontMetrics fmTitle(titleFont);
    QRect fmRectTitle = fmTitle.boundingRect(title);

    QString date = parseDateTime(index.data(NoteModel::NoteLastModificationDateTime).toDateTime());
    QFontMetrics fmDate(m_dateFont);
    QRect fmRectDate = fmDate.boundingRect(title);

    double rowPosX = option.rect.x();
    double rowPosY = option.rect.y();
    double rowWidth = option.rect.width();

    double titleRectPosX = rowPosX + leftOffsetX;
    double titleRectPosY = rowPosY;
    double titleRectWidth = rowWidth - 2.0 * leftOffsetX;
    double titleRectHeight = fmRectTitle.height() + topOffsetY;

    double dateRectPosX = rowPosX + leftOffsetX;
    double dateRectPosY = rowPosY + fmRectTitle.height() + topOffsetY;
    double dateRectWidth = rowWidth - 2.0 * leftOffsetX;
    double dateRectHeight = fmRectDate.height() + spaceY;

    double rowRate = m_timeLine->currentFrame()/(m_maxFrame * 1.0);
    double currRowHeight = m_rowHeight * rowRate;

    auto drawStr = [painter](double posX, double posY, double width, double height, QColor color, QFont font, QString str){
        QRectF rect(posX, posY, width, height);
        painter->setPen(color);
        painter->setFont(font);
        painter->drawText(rect, Qt::AlignBottom, str);
    };

    // 设置标题和日期字符串的边界矩形
    if(index.row() == m_animatedIndex.row()){
        if(m_state == MoveIn){
            titleRectHeight = topOffsetY + fmRectTitle.height() + currRowHeight;

            dateRectPosY = titleRectHeight;
            dateRectHeight = fmRectDate.height() + spaceY;

        }else{

            if((fmRectTitle.height() + topOffsetY) >= ((1.0 - rowRate) * m_rowHeight)){
                titleRectHeight = (fmRectTitle.height() + topOffsetY) - (1.0 - rowRate) * m_rowHeight;
            }else{
                titleRectHeight = 0;

                double labelsSumHeight = fmRectTitle.height() + topOffsetY + fmRectDate.height() + spaceY;
                double bottomSpace = m_rowHeight - labelsSumHeight;

                if(currRowHeight > bottomSpace){
                    dateRectHeight = currRowHeight - bottomSpace;
                }else{
                    dateRectHeight = 0;
                }
            }

            dateRectPosY = titleRectHeight + rowPosY;
        }
    }

    // 绘图标题和日期
    title = fmTitle.elidedText(title, Qt::ElideRight, int(titleRectWidth));
    drawStr(titleRectPosX, titleRectPosY, titleRectWidth, titleRectHeight, m_titleColor, titleFont, title);
    drawStr(dateRectPosX, dateRectPosY, dateRectWidth, dateRectHeight, m_dateColor, m_dateFont, date);
}

void NoteWidgetDelegate::paintSeparator(QPainter*painter, const QStyleOptionViewItem&option, const QModelIndex&index) const
{
    Q_UNUSED(index)

    painter->setPen(QPen(m_separatorColor));
    const int leftOffsetX = 11;
    int posX1 = option.rect.x() + leftOffsetX;
    int posX2 = option.rect.x() + option.rect.width() - leftOffsetX - 1;
    int posY = option.rect.y() + option.rect.height() - 1;

    painter->drawLine(QPoint(posX1, posY),
                      QPoint(posX2, posY));
}

QString NoteWidgetDelegate::parseDateTime(const QDateTime &dateTime) const
{
    QLocale usLocale = QLocale::system();
    QString d;
    auto currDateTime = QDateTime::currentDateTime();

    if(dateTime.date() == currDateTime.date()){
        d = tr("Today ");
        d.append(usLocale.toString(dateTime.time(),"hh:mm"));
        return d;
    }else if(dateTime.daysTo(currDateTime) == 1){
        d = tr("Yesterday ");
        d.append(usLocale.toString(dateTime.time(),"hh:mm"));
        return d;
    }
//    else if(dateTime.daysTo(currDateTime) >= 2 &&
//             dateTime.daysTo(currDateTime) <= 7){
//        return usLocale.toString(dateTime.date(), "dddd");
//    }

    return dateTime.toString("yyyy/MM/dd hh:mm");
}

void NoteWidgetDelegate::setActive(bool isActive)
{
    m_isActive = isActive;
}

void NoteWidgetDelegate::setRowRightOffset(int rowRightOffset)
{
    m_rowRightOffset = rowRightOffset;
}

void NoteWidgetDelegate::setHoveredIndex(const QModelIndex &hoveredIndex)
{
    m_hoveredIndex = hoveredIndex;
}

void NoteWidgetDelegate::setCurrentSelectedIndex(const QModelIndex &currentSelectedIndex)
{
    m_currentSelectedIndex = currentSelectedIndex;
}