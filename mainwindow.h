#ifndef MAINWINDOW_H
#define MAINWINDOW_H

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
    Ui::MainWindow *ui;
    QVector<QWidget*> input_list;
    QChartView  *m_chartView;
    QChart      *m_chart;
    QLineSeries *m_series;
    QLineSeries *m_seriesfirst;
    QLineSeries *m_seriessecond;
    QLineSeries *m_seriestangent;
    QScatterSeries *m_selectedPoint;
    QValueAxis  *m_axisX;
    QValueAxis  *m_axisY;
    QGraphicsLineItem *m_cursorLine = nullptr;  //vertical line for graph
    QList<QPointF> m_currentPoints;             // display points

    void init_graph();
    void refresh_graph();
    double polynomial_function(double x);
    QString polynomialToString(const QList<double> &coeffs);
    bool eventFilter(QObject *obj, QEvent *event);
    QList<double> derivateCoefficients(QList<double> coeffs, int order);
    double polynomial_derivative(double x,int order);
    QString derivativeToString(const QList<double> &coeffs,int order);
    double tangent(double x, double a);
    void move_select_point();
};
#endif // MAINWINDOW_H
