#ifndef TIMECHART_HH
#define TIMECHART_HH

#include <QtWidgets>
#include <QtCharts>

QT_CHARTS_USE_NAMESPACE

class timeChart : public QChartView {
    Q_OBJECT
public:
    using QChartView::QChartView;
    timeChart(QWidget *p);
    void setRange(qreal xmin, qreal xmax, qreal ymin, qreal ymax);
    void setLimits(qreal min, qreal max, Qt::Orientation orientation);
    void addPoint(qreal xvalue, qreal yvalue);

signals:
    void doUpdate(bool);

private slots:


protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

private:
    QPointF m_lastMousePos;
    qreal m_limit_min;
    qreal m_limit_max;
    Qt::Orientation m_orientation;

    QtCharts::QLineSeries   *fSeries;
    QtCharts::QChart        *fChart;
    QtCharts::QValueAxis    *fAxisY, *fAxisX;


};

#endif // TIMECHART_HH
