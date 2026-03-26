#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtCharts/QLineSeries>
#include <QtCharts/QChart>
#include <QtCharts/QValueAxis>
#include <QtCharts/QChartView>
#include <QLayout>
#include <QSpinBox>
#include <QLabel>

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

private:
    Ui::MainWindow *ui;
    QChartView  *m_chartView;
    QChart      *m_chart;
    QLineSeries *m_series;
    QValueAxis  *m_axisX;
    QValueAxis  *m_axisY;

    void init_graph();
    void refresh_graph();
    double polynomial_function(double x);
    QString polynomialToString(const QList<double> &coeffs);
};
#endif // MAINWINDOW_H
