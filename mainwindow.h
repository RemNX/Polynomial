#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "polynomialcalculator.h"

#include <QMainWindow>
#include <QtCharts/QLineSeries>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QChart>
#include <QtCharts/QValueAxis>
#include <QtCharts/QChartView>
#include <QtCharts/QAreaSeries>
#include <QLayout>
#include <QSpinBox>
#include <QLabel>
#include <QLegendMarker>
#include <QDoubleSpinBox>
#include <QStandardItemModel>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    /**
     * @brief Recursively clears a layout by deleting all its child widgets and sub-layouts
     * @param layout: the layout to clear
     */
    void clearLayout(QLayout *layout);

private slots:
    void on_input_input_valueChanged(int value);

    /**
     * @brief when the spinbox for selected x changes, adjust the point position
     */
    void on_selected_x_valueChanged();

    /**
     * @brief when xmin changes, if xmax < xmin → set xmax = xmin + 1
     * @param value of xmin
     */
    void on_input_min_valueChanged(double value);

    /**
     * @brief when xmax changes, if xmax < xmin → set xmin = xmax - 1
     * @param value of xmax
     */
    void on_input_max_valueChanged(double value);

private:
    static constexpr int MAX_DEGREE=6;
    static constexpr int NUMBER_POINTS=1000;

    int m_integration_point_counter=0;

    PolynomialCalculator m_calculator;

    Ui::MainWindow *ui;
    QVector<QWidget*> input_list;
    //graph construction
    QChartView  *m_chartView;
    QChart      *m_chart;
    QValueAxis  *m_axisX;
    QValueAxis  *m_axisY;
    //series or object for graph display
    QLineSeries *m_series;
    QLineSeries *m_seriesfirst;
    QLineSeries *m_seriessecond;
    QLineSeries *m_seriestangent;

    QScatterSeries *m_selectedPoint;    
    QGraphicsLineItem *m_cursorLine = nullptr;  //vertical line for graph

    QLineSeries *m_zeroSeries = nullptr;

    QGraphicsLineItem *m_startIntegrationLine = nullptr;
    QGraphicsLineItem *m_endIntegrationLine = nullptr;
    QLineSeries *m_integrationBaseSeries = nullptr;
    QLineSeries *m_integrationSeries = nullptr;

    QAreaSeries *m_areaSeries = nullptr;



    /**
     * @brief Initialize the chart and UI components
     *
     * -creating all data series (function, derivatives, tangent)
     * - configuring axes
     * - coefficients input
     *
     */
    void initGraph();

    /**
     * @brief Recompute and redraw all graph elements
     *
     * - update polynomial coefficients
     * - recompute function, derivatives, and tangent values
     * - adjusts axis ranges
     * - updates UI outputs
     */
    void refreshGraph();
    void refreshSeries();
    void refreshAxes();
    void refreshEquations();

    /**
     * @brief Update the selected point on the graph
     *
     * - graph point
     * - y value
     * - tangent slope
     */
    void move_select_point();

    /**
     * @brief Extract coefficients from the UI and update the calculator
     *
     * Coefficients are reversed to match polynomial order
     */
    void updateCalculatorCoefficients();

    /**
     * @brief Detect mouse activity on the graph (move or click)
     * @param obj target object, typically the graph
     * @param event type of interaction
     *
     * - mouse move: displays currend coordinates with a vertical line
     * - mouse click: update selected x value
     *
     * @return true if there is interaction
     */
    bool eventFilter(QObject *obj, QEvent *event);

    void updateIntegrationArea();
};
#endif // MAINWINDOW_H
