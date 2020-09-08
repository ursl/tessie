#include <iostream>

#include "timeChart.hh"

using namespace std;

timeChart::timeChart(QWidget *p) : QChartView(p) {
    fChart = new QChart();
    fSeries = new QLineSeries();
    fSeries->append(0., 0.);

    cout << "hallo in timeChart" << endl;

    fChart->addSeries(fSeries);
    fChart->legend()->hide();

    fChart->setMargins(QMargins(0,0,0,0));
    fChart->setBackgroundRoundness(0);

    fAxisY = new QValueAxis();
    fAxisY->setLinePenColor(fSeries->pen().color());
    fAxisY->setTitleText("CPU");
    fAxisY->setMin(0.);
    fAxisY->setMax(4.);

    fAxisX = new QValueAxis();
    fAxisX->setTitleText("Time since tessie start [sec]");
    fAxisX->setLabelFormat("%.0f");
    fAxisX->setTickCount(10);
//    connect(fAxisX, SIGNAL(rangeChanged(qreal, qreal)), this, SLOT(on_rangeChanged(qreal,qreal)) );


    fChart->addAxis(fAxisY, Qt::AlignLeft);
    fChart->addAxis(fAxisX, Qt::AlignBottom);

    fSeries->attachAxis(fAxisY);
    fSeries->attachAxis(fAxisX);


    setChart(fChart);
    setRenderHint(QPainter::Antialiasing);
    setRubberBand( QChartView::HorizontalRubberBand);
    repaint();
}

// -------------------------------------------------------------------------
void timeChart::setRange(qreal xmin, qreal xmax, qreal ymin, qreal ymax) {
    if(!chart()) return;
    if(QValueAxis *xaxis = qobject_cast<QValueAxis *>(chart()->axes(Qt::Horizontal).first())){
        xaxis->setRange(xmin, xmax);
    }
    if(QValueAxis *yaxis = qobject_cast<QValueAxis *>(chart()->axes(Qt::Vertical).first())){
        yaxis->setRange(ymin, ymax);
    }
}

// -------------------------------------------------------------------------
void timeChart::setLimits(qreal min, qreal max, Qt::Orientation orientation) {
    m_limit_min = min;
    m_limit_max = max;
    m_orientation = orientation;
}

// -------------------------------------------------------------------------
void timeChart::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton && chart())
        m_lastMousePos = mapToScene(event->pos());
    QGraphicsView::mousePressEvent(event);
}

// -------------------------------------------------------------------------
void timeChart::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton && chart()){
        QPointF newValue = mapToScene(event->pos());
        QPointF delta = newValue - m_lastMousePos;
        std::cout << "delta = " << delta.x() << std::endl;
        chart()->scroll(-delta.x(), 0);
        if (0) {
            emit doUpdate(false);
        } else {
            emit doUpdate(true);
        }
        //        if(QValueAxis * axis = qobject_cast<QValueAxis *>(chart()->axes(m_orientation).first()) ){
        //            qreal deltaX = axis->max() - axis->min();
        //            if(axis->min() < m_limit_min){
        //                axis->setRange(m_limit_min, m_limit_min + deltaX);
        //            }
        //            else if(axis->max() > m_limit_max){
        //                axis->setRange(m_limit_max - deltaX, m_limit_max);
        //            }
        //        }
        //        m_lastMousePos = newValue;
    }
    QGraphicsView::mouseMoveEvent(event);
}


// ----------------------------------------------------------------------
void timeChart::addPoint(qreal x, qreal y) {
   // cout << "timeChart::addPoint: " << fSeries << endl;
    fSeries->append(x,y);

    fChart->removeSeries(fSeries);
    fChart->addSeries(fSeries);

 //   fAxisX0->setRange(fStartTime/1000., (momentInTime.toMSecsSinceEpoch()-fStartTime)/1000.);
    fAxisX->setMin(0);
    fAxisX->setMax(x);
    fSeries->attachAxis(fAxisY);
    fSeries->attachAxis(fAxisX);

    repaint();
}
