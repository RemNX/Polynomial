#ifndef GRAPHMANAGER_H
#define GRAPHMANAGER_H

#include <QObject>
#include <QtCharts/QLineSeries>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QChart>
#include <QtCharts/QValueAxis>
#include <QtCharts/QChartView>
#include <QLayout>

#include "polynomialcalculator.h"

class GraphManager : public QObject
{
    Q_OBJECT
public:
    GraphManager(QChartView *chartView,
                 QLayout *layout,
                 QObject *parent = nullptr);

    void init_graph();
    void refresh_graph();
    void move_select_point();

    void setCalculator(PolynomialCalculator *calc);

private:
    //graph construction
    QChartView  *m_chartView;
    QChart      *m_chart;
    QValueAxis  *m_axisX;
    QValueAxis  *m_axisY;
    //seriers or object for graph display
    QLineSeries *m_series;
    QLineSeries *m_seriesfirst;
    QLineSeries *m_seriessecond;
    QLineSeries *m_seriestangent;
    QScatterSeries *m_selectedPoint;
    QGraphicsLineItem *m_cursorLine = nullptr;  //vertical line for graph

    PolynomialCalculator *m_calculator = nullptr;

    QLayout *m_layout;
};

#endif // GRAPHMANAGER_H
