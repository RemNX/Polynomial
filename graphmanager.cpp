#include "graphmanager.h"

GraphManager::GraphManager(QChartView *chartView,
                           QLayout *layout,
                           QObject *parent)
    : QObject(parent),
    m_chartView(chartView),
    m_layout(layout)
{
}

//connection to the calculor file
void GraphManager::setCalculator(PolynomialCalculator *calc)
{
    m_calculator = calc;
}


