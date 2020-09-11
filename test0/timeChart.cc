#include <iostream>

#include "timeChart.hh"

using namespace std;

timeChart::timeChart(QWidget *p) : QChartView(p) {
    fChart = new QChart();
    fSeries = new QLineSeries();
    fDoUpdate = true;

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
    //    connect(fAxisX, SIGNAL(rangeChanged(qreal, qreal)), this, SLOT(on_rangeChanged(qreal,qreal)) );

    fAxisX->setTickCount(11);
    fAxisX->setMax(1000.);


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
    fDoUpdate = false;
    emit doUpdate(false);
}

// -------------------------------------------------------------------------
void timeChart::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton && chart()){
        QPointF newValue = mapToScene(event->pos());
        QPointF delta = newValue - m_lastMousePos;
        std::cout << "delta = " << delta.x() << std::endl;
        chart()->scroll(-delta.x(), 0);
        fDoUpdate = false;
        emit doUpdate(false);
    }
    QGraphicsView::mouseMoveEvent(event);
}


// ----------------------------------------------------------------------
void timeChart::addPoint(qreal x, qreal y) {
    // cout << "timeChart::addPoint: " << fSeries << endl;
    fSeries->append(x,y);

    if ((x > fAxisX->max() - 100) && (x < fAxisX->max() + 100)) fDoUpdate = true;


    if (fDoUpdate) {
        fChart->removeSeries(fSeries);
        fChart->addSeries(fSeries);

        fAxisX->setMax(x);
        if (x < 43200) {
            fAxisX->setMin(0);
            if (x > 10000) {
                fAxisX->setTickCount(13);
                fAxisX->setMax(43200.);
            }
        } else {
            fAxisX->setMin(x-43200);
        }
        fSeries->attachAxis(fAxisY);
        fSeries->attachAxis(fAxisX);
    }
    repaint();
}
