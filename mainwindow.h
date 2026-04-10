#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "polynomialcalculator.h"

#include <QMainWindow>
#include <QtCharts/QLineSeries>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QChart>
#include <QtCharts/QValueAxis>
#include <QtCharts/QChartView>
#include <QLayout>
#include <QSpinBox>
#include <QLabel>
#include <QLegendMarker>

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
    void clearLayout(QLayout *layout);

private slots:
    void on_input_input_valueChanged(int value);
    void on_input_min_valueChanged(double arg1);
    void on_input_max_valueChanged(double arg1);
    void on_box_derivate_first_checkStateChanged(const Qt::CheckState &arg1);
    void on_box_derivate_second_checkStateChanged(const Qt::CheckState &arg1);
    void on_selected_x_valueChanged(double x);
    void on_box_tangent_checkStateChanged(const Qt::CheckState &arg1);

private:
    PolynomialCalculator m_calculator;

    Ui::MainWindow *ui;
    QVector<QWidget*> input_list;
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

    //functions
    void init_graph();
    void refresh_graph();
    void move_select_point();
    void updateCalculatorCoefficients();
    bool eventFilter(QObject *obj, QEvent *event);
};
#endif // MAINWINDOW_H
